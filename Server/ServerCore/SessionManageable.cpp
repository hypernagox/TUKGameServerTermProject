#include "ServerCorePch.h"
#include "SessionManageable.h"
#include "Session.h"

namespace ServerCore
{
	SessionManageable::SessionManageable(const uint16_t roomID_)noexcept
		: m_roomID{ roomID_ }
	{
		Mgr(TaskTimerMgr)->ReserveAsyncTask(HEART_BEAT_TICK, [this]()noexcept
			{
				if (const auto ptr = this->shared_from_this())
				{
					RegisterHeartBeat();
				}
			});
	}

	SessionManageable::~SessionManageable()noexcept
	{
	}

	drop_range SessionManageable::GetSessionRangeExceptOne(c_uint64 exceptSessionNumber_)noexcept
	{
		if (!m_linkedHashMapForSession.HasItem(exceptSessionNumber_))
		{
			return drop_range{ m_linkedHashMapForSession.GetItemListRef(),0 };
		}
		m_linkedHashMapForSession.SwapElement((*m_linkedHashMapForSession.cbegin())->GetSessionID(), exceptSessionNumber_);
		return  drop_range{ m_linkedHashMapForSession.GetItemListRef(),1 };
	}

	void SessionManageable::SendEnqueue(S_ptr<Session> pSession_, S_ptr<SendBuffer> pSendBuffer_) noexcept
	{
		EnqueueAsync(&Session::SendAsync, std::move(pSession_), std::move(pSendBuffer_));
	}

	void SessionManageable::EnterEnqueue(S_ptr<Session> pSession_) noexcept
	{
		EnqueueAsync(&SessionManageable::Enter, std::move(pSession_));
	}

	void SessionManageable::LeaveAndDisconnectEnqueue(S_ptr<Session> pSession_)noexcept
	{
		EnqueueAsync(&SessionManageable::LeaveAndDisconnect, std::move(pSession_));
	}

	void SessionManageable::BroadCastEnqueue(S_ptr<SendBuffer> pSendBuffer) noexcept
	{
		EnqueueAsync(&SessionManageable::BroadCast, std::move(pSendBuffer));
	}

	void SessionManageable::BroadCastEnqueue(S_ptr<SendBuffer> pSendBuffer, c_uint64 exceptSessionNumber)noexcept
	{
		EnqueueAsync(&SessionManageable::BroadCastExceptOne, std::move(pSendBuffer), c_uint64{ exceptSessionNumber });
	}

	S_ptr<Session> SessionManageable::FindSession(const uint64 sessionID)const noexcept
	{
		return m_linkedHashMapForSession.FindItem(sessionID);
	}

	void SessionManageable::ImmigrationEnqueue(S_ptr<SessionManageable> pOtherRoom, c_uint64 sessionID) noexcept
	{
		EnqueueAsync(&SessionManageable::Immigration, std::move(pOtherRoom), c_uint64{ sessionID });
	}

	void SessionManageable::ImmigrationAllEnqueue(S_ptr<SessionManageable> pOtherRoom, const bool bDestroyCurrentRoom) noexcept
	{
		EnqueueAsync(&SessionManageable::ImmigrationAll, std::move(pOtherRoom), bool{ bDestroyCurrentRoom });
	}

	void SessionManageable::Immigration(const S_ptr<SessionManageable> pOtherRoom, const uint64 sessionID) noexcept
	{
		if (auto session = m_linkedHashMapForSession.ExtractItem(sessionID))
		{
			pOtherRoom->EnqueueAsync(&SessionManageable::Enter, std::move(session));
		}
		else
		{
			std::cout << "Cannot Find Session" << std::endl;
		}
	}

	void SessionManageable::ImmigrationAll(const S_ptr<SessionManageable> pOtherRoom, const bool bDestroyCurrentRoom)noexcept
	{
		for (const auto pSession : m_linkedHashMapForSession)
		{
			pOtherRoom->EnqueueAsync(&SessionManageable::Enter, pSession->SharedCastThis<Session>());
		}
		m_linkedHashMapForSession.clear_unsafe();
		if (bDestroyCurrentRoom)
			pOtherRoom->EnqueueAsync(&TaskQueueable::StopExecute, shared_from_this_ref());
	}

	void SessionManageable::RegisterHeartBeat() noexcept
	{
		EnqueueAsync(&SessionManageable::ListenHeartBeat);
	}

	void SessionManageable::ListenHeartBeat() noexcept
	{
		CREATE_FUNC_LOG(L"HeartBeat");
		const S_ptr<SendBuffer> sendBuffer = CreateHeartBeatSendBuffer(HEART_BEAT::s2c_HEART_BEAT);
		for (const auto session : m_linkedHashMapForSession)
		{
			if (session->IsHeartBeatAlive())
			{
				session->SetHeartBeat(false);
				session->SendOnlyEnqueue(sendBuffer);
			}
			else
			{
				// TODO: 하트비트 반응없으면 쳐내야함 
				//LeaveAndDisconnectEnqueue(session);
			}
		}
		EnqueueAsyncTimer(HEART_BEAT_TICK, &SessionManageable::ListenHeartBeat);
	}

	void SessionManageable::Enter(S_ptr<Session> pSession_)noexcept
	{
		const uint64 sessionID = pSession_->GetSessionID();
		const auto temp_ptr = m_linkedHashMapForSession.AddItem(sessionID, std::move(pSession_));
		if (!temp_ptr)
		{
			// TODO: 이미 방에 있는데 또 들어오려한거임
			std::cout << "Alread Exist in Room" << std::endl;
			return;
		}
		temp_ptr->SetSessionRoomInfo(m_roomID, this);
	}

	void SessionManageable::BroadCast(const S_ptr<SendBuffer> pSendBuffer)noexcept
	{
		//CREATE_FUNC_LOG(L"BroadCast");
		for (const auto pSession : m_linkedHashMapForSession)
		{
			pSession->SendOnlyEnqueue(pSendBuffer);
			m_vecTaskBulks.emplace_back(PoolNew<Task>(&Session::TryRegisterSend, pSession->SharedCastThis<Session>()));
		}
		if (const auto num = m_vecTaskBulks.size()) [[likely]]
		{
			Mgr(ThreadMgr)->EnqueueGlobalTaskBulk(m_vecTaskBulks.data(), num);
			m_vecTaskBulks.clear();
		}
	}

	void SessionManageable::BroadCastExceptOne(const S_ptr<SendBuffer> pSendBuffer, const uint64 exceptSessionNumber) noexcept
	{
		const auto end_iter = m_linkedHashMapForSession.cend();
		auto start_iter = m_linkedHashMapForSession.cbegin();
		if (m_linkedHashMapForSession.HasItem(exceptSessionNumber))
		{
			m_linkedHashMapForSession.SwapElement((*start_iter)->GetSessionID(), exceptSessionNumber);
			++start_iter;
		}
		for (; start_iter != end_iter; ++start_iter)
		{
			auto pSession = (*start_iter)->SharedCastThis<Session>();
			pSession->SendOnlyEnqueue(pSendBuffer);
			m_vecTaskBulks.emplace_back(PoolNew<Task>(&Session::TryRegisterSend, std::move(pSession)));
		}
		if (const auto num = m_vecTaskBulks.size())
		{
			Mgr(ThreadMgr)->EnqueueGlobalTaskBulk(m_vecTaskBulks.data(), num);
			m_vecTaskBulks.clear();
		}
	}

	void SessionManageable::LeaveAndDisconnect(const S_ptr<Session> pSession_)noexcept
	{
		const uint64 sessionID = pSession_->GetSessionID();
		pSession_->Disconnect(L"Leave");
		if (!m_linkedHashMapForSession.EraseItem(sessionID))
		{
			std::cout << "Cannot Find Session in Leave" << std::endl;
		}
		pSession_->reset_cache_shared(*this);
	}
}