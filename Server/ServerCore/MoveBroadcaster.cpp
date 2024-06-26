#include "ServerCorePch.h"
#include "MoveBroadcaster.h"
#include "PacketSession.h"
#include "Sector.h"

namespace ServerCore
{
	MoveBroadcaster::MoveBroadcaster()
		: m_viewListPtr{ MakeSharedSTD<HashSet<S_ptr<IocpEntity>>>() }
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
		const Vector<Sector*>* const sectors
	)noexcept
	{
		int sector_state = 0;
		
		const uint16 obj_type = thisSession_->GetObjectType();
		const auto thisSession = thisSession_->IsSession();
		if (thisSession)
		{
			if (false == thisSession->IsConnected())
				return NONE;
		}

		const std::shared_ptr<HashSet<S_ptr<IocpEntity>>> viewListPtr = m_viewListPtr.load(std::memory_order_acquire);

		if (!viewListPtr)
			return NONE;

		auto& m_viewList = *viewListPtr;
		const auto cache_obj_ptr = thisSession_.get();
		const bool bIsNPC = 0 != obj_type;

		thread_local HashSet<S_ptr<IocpEntity>> new_view_list;
		new_view_list.clear();
		thread_local HashSet<Session*> send_list;
		send_list.clear();
		thread_local Vector<IocpEntity*> entity_copy;
		entity_copy.clear();

		for (const auto sector : *sectors)
		{
			sector->lock_shared();
			for (const auto pEntity : sector->GetObjectList())
			{
				if (bIsNPC && !pEntity->IsSession())continue;
				pEntity->IncRef();
				entity_copy.emplace_back(pEntity);
			}
			sector->unlock_shared();
		}
		
		for (const auto pEntity : entity_copy)
		{
			if (g_huristic(cache_obj_ptr, pEntity))
			{
				const bool bFlag = static_cast<const bool>(pEntity->IsSession());
				new_view_list.emplace(pEntity);
				sector_state |= (bFlag + 1);
			}
			pEntity->DecRef();
		}

		new_view_list.erase(thisSession_);

		for (const auto& pEntity : new_view_list)
		{
			const auto pSession = pEntity->IsSession();
			if (!m_viewList.contains(pEntity))
			{
				
				if (pSession) {
					pSession->SendOnlyEnqueue(in_pkt);
					send_list.emplace(pSession);
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
					send_list.emplace(pSession);
				}
			}
		}

		for (auto iter = m_viewList.cbegin(); iter != m_viewList.cend();)
		{
			const auto& pEntity = *iter;
			if (false == new_view_list.contains(pEntity))
			{
				const auto pSession = pEntity->IsSession();
				if (pSession) {
					pSession->SendOnlyEnqueue(out_pkt);
					send_list.emplace(pSession);
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
		for (const auto pSession : send_list)
			pSession->TrySend();

		return 3 ^ sector_state;
	}
}