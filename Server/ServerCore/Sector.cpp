#include "ServerCorePch.h"
#include "Sector.h"
#include "PacketSession.h"
#include "MoveBroadcaster.h"

namespace ServerCore
{
	Sector::Sector(const uint16_t sector_id)noexcept
		: m_sectorID{ sector_id }
	{
		//Mgr(TaskTimerMgr)->ReserveAsyncTask(HEART_BEAT_TICK, [this]()noexcept
		//	{
		//		if (const auto ptr = this->shared_from_this())
		//		{
		//			RegisterHeartBeat();
		//		}
		//	});
	}

	Sector::~Sector()noexcept
	{
	}

	void Sector::EnterEnqueue(IocpEntity* const pEntity_) noexcept
	{
		// TODO: 만약 이렇게 할거면 EnterEnqueue는 첫 월드 입장시 단 한번만 불려야한다.
		pEntity_->IncRef();
		pEntity_->SetSectorInfo(GetSectorID(), this);
		EnqueueAsync(&Sector::Enter, static_cast<IocpEntity*const>(pEntity_));
	}

	void Sector::LeaveAndDisconnectEnqueue(const uint64_t obj_id)noexcept
	{
		EnqueueAsync(&Sector::LeaveAndDisconnect, uint64_t{ obj_id });
	}

	void Sector::BroadCastEnqueue(S_ptr<SendBuffer> pSendBuffer) noexcept
	{
		EnqueueAsync(&Sector::BroadCast, std::move(pSendBuffer));
	}

	void Sector::BroadCastEnqueue(S_ptr<SendBuffer> pSendBuffer, c_uint64 exceptSessionNumber)noexcept
	{
		EnqueueAsync(&Sector::BroadCastExceptOne, std::move(pSendBuffer), c_uint64{ exceptSessionNumber });
	}

	void Sector::ImmigrationEnqueue(S_ptr<Sector> pOtherSector, c_uint64 obj_id) noexcept
	{
		EnqueueAsync(&Sector::Immigration, std::move(pOtherSector), c_uint64{ obj_id });
	}

	void Sector::ImmigrationAllEnqueue(S_ptr<Sector> pOtherSector, const bool bDestroyCurrentRoom) noexcept
	{
		EnqueueAsync(&Sector::ImmigrationAll, std::move(pOtherSector), bool{ bDestroyCurrentRoom });
	}

	const int Sector::MoveBroadCast(
		const S_ptr<IocpEntity>& move_session,
		const S_ptr<SendBuffer>& in_pkt,
		const S_ptr<SendBuffer>& out_pkt,
		const S_ptr<SendBuffer>& move_pkt,
		const Vector<Sector*>& sectors
	) noexcept
	{
		return move_session->GetMoveBroadcaster()->BroadcastMove(in_pkt, out_pkt, move_pkt, move_session, sectors);
	}

	void Sector::Immigration(const S_ptr<Sector> pOtherSector, const uint64 obj_id) noexcept
	{
		if (const auto entity = m_linkedHashMapForIocpEntity.ExtractItemSafe(obj_id))
		{
			if (entity->IsSession())
				m_linkedHashMapForSession.EraseItemSafe(obj_id);
			pOtherSector->EnqueueAsync([beforeSector = S_ptr<Sector>{this}, pOtherSector, entity]()mutable noexcept
				{
					entity->SetSectorInfo(beforeSector->GetSectorID(), pOtherSector.get());
					pOtherSector->Enter(entity);
					pOtherSector->ImigrationAfterBehavior(std::move(beforeSector), entity);
				});
		}
		else
		{
			//std::cout << "Cannot Find Session" << std::endl;
		}
	}

	void Sector::ImmigrationAll(const S_ptr<Sector> pOtherSector, const bool bDestroyCurrentRoom)noexcept
	{
		for (const auto entity : m_linkedHashMapForIocpEntity)
		{
			pOtherSector->EnqueueAsync([beforeSector = S_ptr<Sector>{ this }, pOtherSector, entity]()mutable noexcept
				{
					entity->SetSectorInfo(beforeSector->GetSectorID(), pOtherSector.get());
					pOtherSector->Enter(entity);
					pOtherSector->ImigrationAfterBehavior(std::move(beforeSector), entity);
				});
		}
		
		m_linkedHashMapForIocpEntity.GetSRWLock().lock();
		m_linkedHashMapForIocpEntity.clear_unsafe();
		m_linkedHashMapForIocpEntity.GetSRWLock().unlock();

		m_linkedHashMapForSession.GetSRWLock().lock();
		m_linkedHashMapForSession.clear_unsafe();
		m_linkedHashMapForSession.GetSRWLock().unlock();
		
		if (bDestroyCurrentRoom)
			pOtherSector->StopExecute();
	}

	void Sector::RegisterHeartBeat() noexcept
	{
		EnqueueAsync(&Sector::ListenHeartBeat);
	}

	void Sector::ListenHeartBeat() noexcept
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

	void Sector::Enter(IocpEntity* const pEntity_)noexcept
	{
		const uint64 sessionID = pEntity_->GetObjectID();
		const auto temp_session_ptr = pEntity_->IsSession();
		if (temp_session_ptr)
		{
			if (false == temp_session_ptr->IsConnectedAtomic())
			{
				temp_session_ptr->DecRef();
				return;
			}
			else
			{
				m_linkedHashMapForSession.AddItem(sessionID, temp_session_ptr);
			}
		}
		else
		{
			if (false == pEntity_->IsValid())
			{
				pEntity_->DecRef();
				return;
			}
		}
		if (!m_linkedHashMapForIocpEntity.AddItem(sessionID, pEntity_))
		{
			// TODO: 이미 방에 있는데 또 들어오려한거임
			// std::cout << "Alread Exist in Room" << std::endl;
			return;
		}
	}

	void Sector::BroadCast(const S_ptr<SendBuffer> pSendBuffer)noexcept
	{
		for (const auto pSession : m_linkedHashMapForSession)
		{
			pSession->SendAsync(pSendBuffer);
		}
	}

	void Sector::BroadCastExceptOne(const S_ptr<SendBuffer> pSendBuffer, const uint64 exceptSessionNumber) noexcept
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
			(*start_iter)->SendAsync(pSendBuffer);
		}
	}

	void Sector::LeaveAndDisconnect(const uint64_t obj_id)noexcept
	{
		if(m_linkedHashMapForIocpEntity.HasItem(obj_id))
		{
			if (const auto entity = m_linkedHashMapForIocpEntity.ExtractItemSafe(obj_id))
			{
				if (entity->IsSession())
				{
					m_linkedHashMapForSession.EraseItemSafe(obj_id);
				}
				entity->DecRef();
			}
			else
			{
				//std::cout << "Cannot Find Session in Leave" << std::endl;
			}
		}
	}
}