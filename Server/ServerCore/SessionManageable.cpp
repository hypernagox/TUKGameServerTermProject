#include "ServerCorePch.h"
#include "SessionManageable.h"
#include "PacketSession.h"
#include "MoveBroadcaster.h"
#include "../Object.h"

namespace ServerCore
{
	SessionManageable::SessionManageable(const uint16_t roomID_)noexcept
		: m_roomID{ roomID_ }
	{
		//Mgr(TaskTimerMgr)->ReserveAsyncTask(HEART_BEAT_TICK, [this]()noexcept
		//	{
		//		if (const auto ptr = this->shared_from_this())
		//		{
		//			RegisterHeartBeat();
		//		}
		//	});
	}

	SessionManageable::~SessionManageable()noexcept
	{
	}

	//drop_range SessionManageable::GetSessionRangeExceptOne(c_uint64 exceptSessionNumber_)noexcept
	//{
	//	if (!m_linkedHashMapForSession.HasItem(exceptSessionNumber_))
	//	{
	//		return drop_range{ m_linkedHashMapForSession.GetItemListRef(),0 };
	//	}
	//	m_linkedHashMapForSession.SwapElement((*m_linkedHashMapForSession.cbegin())->GetSessionID(), exceptSessionNumber_);
	//	return  drop_range{ m_linkedHashMapForSession.GetItemListRef(),1 };
	//}

	void SessionManageable::EnterEnqueue(S_ptr<IocpEntity> pSession_) noexcept
	{
		EnqueueAsync(&SessionManageable::Enter, std::move(pSession_));
	}

	void SessionManageable::LeaveAndDisconnectEnqueue(const uint64_t obj_id)noexcept
	{
		EnqueueAsync(&SessionManageable::LeaveAndDisconnect, uint64_t{ obj_id });
	}

	void SessionManageable::BroadCastEnqueue(S_ptr<SendBuffer> pSendBuffer) noexcept
	{
		//BroadCast(std::move(pSendBuffer));
		EnqueueAsync(&SessionManageable::BroadCast, std::move(pSendBuffer));
	}

	void SessionManageable::BroadCastEnqueue(S_ptr<SendBuffer> pSendBuffer, c_uint64 exceptSessionNumber)noexcept
	{
		EnqueueAsync(&SessionManageable::BroadCastExceptOne, std::move(pSendBuffer), c_uint64{ exceptSessionNumber });
	}

	S_ptr<IocpEntity> SessionManageable::FindSession(const uint64 sessionID)const noexcept
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

	const int SessionManageable::MoveBroadCast(
		const S_ptr<IocpEntity>& move_session,
		const S_ptr<SendBuffer>& in_pkt,
		const S_ptr<SendBuffer>& out_pkt,
		const S_ptr<SendBuffer>& move_pkt,
		const Vector<SessionManageable*>* sectors
	) noexcept
	{
		return move_session->m_broadCaster->BroadcastMove(in_pkt, out_pkt, move_pkt, move_session, sectors);
	}

	void SessionManageable::Immigration(const S_ptr<SessionManageable> pOtherRoom, const uint64 sessionID) noexcept
	{
		if (auto session = m_linkedHashMapForSession.ExtractItemSafe(sessionID))
		{
			if (session->IsSession())
				m_linkedHashMapForClient.EraseItemSafe(sessionID);

			pOtherRoom->EnqueueAsync([beforeRoom = S_ptr<SessionManageable>{this}, pOtherRoom, session]()mutable noexcept
				{
					pOtherRoom->Enter(session);
					pOtherRoom->ImigrationAfterBehavior(std::move(beforeRoom), std::move(session));
				});
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
			pOtherRoom->EnqueueAsync([beforeRoom = S_ptr<SessionManageable>{ this }, pOtherRoom, pSession = S_ptr<IocpEntity>{pSession}]()mutable noexcept
				{
					pOtherRoom->Enter(pSession);
					pOtherRoom->ImigrationAfterBehavior(std::move(beforeRoom), std::move(pSession));
				});
		}
		
		m_linkedHashMapForSession.GetSRWLock().lock();
		m_linkedHashMapForSession.clear_unsafe();
		m_linkedHashMapForSession.GetSRWLock().unlock();

		m_linkedHashMapForClient.GetSRWLock().lock();
		m_linkedHashMapForClient.clear_unsafe();
		m_linkedHashMapForClient.GetSRWLock().unlock();
		
		if (bDestroyCurrentRoom)
			pOtherRoom->EnqueueAsync(&TaskQueueable::StopExecute);
		if (bDestroyCurrentRoom)
			pOtherRoom->StopExecute();
	}

	void SessionManageable::RegisterHeartBeat() noexcept
	{
		EnqueueAsync(&SessionManageable::ListenHeartBeat);
	}

	void SessionManageable::ListenHeartBeat() noexcept
	{
		//CREATE_FUNC_LOG(L"HeartBeat");
		//const S_ptr<SendBuffer> sendBuffer = CreateHeartBeatSendBuffer(HEART_BEAT::s2c_HEART_BEAT);
		//for (const auto session : m_linkedHashMapForSession)
		//{
		//	if (session->IsHeartBeatAlive())
		//	{
		//		session->SetHeartBeat(false);
		//		//session->SendOnlyEnqueue(sendBuffer);
		//		//session << sendBuffer;
		//	}
		//	else
		//	{
		//		// TODO: 하트비트 반응없으면 쳐내야함 
		//		//LeaveAndDisconnectEnqueue(session);
		//	}
		//}
		//EnqueueAsyncTimer(HEART_BEAT_TICK, &SessionManageable::ListenHeartBeat);
	}

	void SessionManageable::Enter(S_ptr<IocpEntity> pSession_)noexcept
	{
		const uint64 sessionID = pSession_->GetObjectID();
		const auto temp_session_ptr = pSession_->IsSession();
		if (temp_session_ptr)
		{
			if (false == temp_session_ptr->IsConnectedAtomic())
			{
				return;
			}
			else
			{
				temp_session_ptr->SetSessionRoomInfo(m_roomID, this);
				m_linkedHashMapForClient.AddItem(sessionID, S_ptr<PacketSession>{temp_session_ptr});
			}
		}
		const auto temp_ptr = m_linkedHashMapForSession.AddItem(sessionID, pSession_);
		if (!temp_ptr)
		{
			// TODO: 이미 방에 있는데 또 들어오려한거임
			std::cout << "Alread Exist in Room" << std::endl;
			return;
		}
		else
		{
			temp_ptr->GetContentsEntity()->m_pCurSector = this;
		}
	}

	void SessionManageable::BroadCast(const S_ptr<SendBuffer> pSendBuffer)noexcept
	{
		for (const auto pSession : m_linkedHashMapForClient)
		{
			pSession->SendAsync(pSendBuffer);
		}
	}

	void SessionManageable::BroadCastExceptOne(const S_ptr<SendBuffer> pSendBuffer, const uint64 exceptSessionNumber) noexcept
	{
		//const auto end_iter = m_linkedHashMapForSession.cend();
		//auto start_iter = m_linkedHashMapForSession.cbegin();
		//if (m_linkedHashMapForSession.HasItem(exceptSessionNumber))
		//{
		//	m_linkedHashMapForSession.SwapElement((*start_iter)->GetSessionID(), exceptSessionNumber);
		//	++start_iter;
		//}
		//for (; start_iter != end_iter; ++start_iter)
		//{
		//	(*start_iter)->SendAsync(pSendBuffer);
		//	//auto pSession = (*start_iter)->SharedCastThis<Session>();
		//	//pSession->SendOnlyEnqueue(pSendBuffer);
		//	//if (pSession->CanRegisterSend())
		//	//	m_vecTaskBulks.emplace_back(PoolNew<Task>(&Session::TryRegisterSend, std::move(pSession)));
		//}
		//if (const auto num = m_vecTaskBulks.size())
		//{
		//	Mgr(ThreadMgr)->EnqueueGlobalTaskBulk(m_vecTaskBulks.data(), num);
		//	m_vecTaskBulks.clear();
		//}
	}

	void SessionManageable::LeaveAndDisconnect(const uint64_t obj_id)noexcept
	{
		const uint64 sessionID = obj_id;
		if(m_linkedHashMapForSession.HasItem(sessionID))
		{
			if (const auto pObj = m_linkedHashMapForSession.ExtractItemSafe(sessionID))
			{
				if (pObj->IsSession())
				{
					if (const auto pSession = m_linkedHashMapForClient.ExtractItemSafe(sessionID)){
						//pSession->DecRef();
						std::cout << pSession->UseCount() << std::endl;
						
					}
						//pSession->reset_cache_shared(*this);
				}
				else
				{
					//pObj->reset_cache_shared(*this);
				}
			}
			else
			{
				//std::cout << "Cannot Find Session in Leave" << std::endl;
			}
		}
	}
}