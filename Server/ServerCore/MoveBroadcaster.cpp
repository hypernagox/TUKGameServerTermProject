#include "ServerCorePch.h"
#include "MoveBroadcaster.h"
#include "PacketSession.h"
#include "SessionManageable.h"

namespace ServerCore
{
	MoveBroadcaster::MoveBroadcaster()
		: m_viewListPtr{ ServerCore::MakeShared<HashSet<S_ptr<IocpEntity>>>() }
	{
	}

	MoveBroadcaster::~MoveBroadcaster()
	{
	}

	const int MoveBroadcaster::BroadcastMove(
		const S_ptr<SendBuffer>& in_pkt,
		const S_ptr<SendBuffer>& out_pkt,
		const S_ptr<SendBuffer>& move_pkt,
		const S_ptr<IocpEntity>& thisSession_,
		const Vector<SessionManageable*>* const sectors
	)noexcept
	{
		//if (IDLE != m_work_flag.exchange(WORK, std::memory_order_relaxed))
		//	return NONE;
		int sector_state = 0;
		
		const uint16 obj_type = thisSession_->GetObjectType();
		const auto thisSession = thisSession_->IsSession();
		if (thisSession)
		{
			if (false == thisSession->IsConnected())
				return NONE;
		}

		const S_ptr<HashSet<S_ptr<IocpEntity>>> viewListPtr = m_viewListPtr.load(std::memory_order_acquire);

		if (!viewListPtr)
			return NONE;

		auto& m_viewList = *viewListPtr;
		const auto cache_obj_ptr = thisSession_.get();
		const bool bIsNPC = 0 != obj_type;

		thread_local HashSet<S_ptr<IocpEntity>> new_view_list;
		new_view_list.clear();
		thread_local Vector<Session*> send_session_list;
		send_session_list.clear();
		
		for (const auto sector : *sectors)
		{
			sector->GetSRWLock().lock_shared();
			for (const auto pSession : sector->GetObjectList())
			{
				const bool bFlag = static_cast<const bool>(pSession->IsSession());
				if (bIsNPC && !bFlag)
					continue;
				if (auto pValid = pSession->GetSharedThis())
				{
					if (g_huristic(cache_obj_ptr, pSession))
					{
						new_view_list.emplace(std::move(pValid));
						sector_state |= (bFlag + 1);
					}
				}
			}
			sector->GetSRWLock().unlock_shared();
		}

		new_view_list.erase(thisSession_);

		for (const auto& pEntity : new_view_list)
		{
			const auto pSession = pEntity->IsSession();
			if (!m_viewList.contains(pEntity))
			{
				
				if (pSession) {
					pSession->SendOnlyEnqueue(in_pkt);
					send_session_list.emplace_back(pSession);
				}

				if (thisSession)
					thisSession->SendOnlyEnqueue(g_create_in_pkt(pEntity));
				
				m_viewList.emplace(pEntity);
			}
			else
			{
				if (thisSession)
					thisSession->SendOnlyEnqueue(move_pkt);

				if (pSession) {
					pSession->SendOnlyEnqueue(move_pkt);
					send_session_list.emplace_back(pSession);
				}
			}
		}

		const auto e_iter = new_view_list.cend();
		
		for (auto iter = m_viewList.cbegin(); iter != m_viewList.cend();)
		{
			const auto& pEntity = *iter;
			const auto target = new_view_list.find(pEntity);
			const auto pSession = pEntity->IsSession();
			if (e_iter == target)
			{
				if (pSession) {
					pSession->SendOnlyEnqueue(out_pkt);
					send_session_list.emplace_back(pSession);
				}

				if (thisSession)
					thisSession->SendOnlyEnqueue(g_create_out_pkt(pEntity));

				iter = m_viewList.erase(iter);
			}
			else
			{
				++iter;
			}
		}

		if (thisSession)
			thisSession->TrySend();
		for (const auto session : send_session_list)
			session->TrySend();

		//if (WORK != m_work_flag.exchange(IDLE, std::memory_order_release))
		//{
		//	m_viewList.clear();
		//	return STOP;
		//}
			
		return 3 ^ sector_state;
	}
}