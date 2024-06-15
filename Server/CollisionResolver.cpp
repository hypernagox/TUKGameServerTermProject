#include "pch.h"
#include "CollisionResolver.h"
#include "Object.h"
#include "MoveBroadcaster.h"
#include "PacketSession.h"
#include "c2s_PacketHandler.h"
#include "TRWorldRoom.h"

void MonsterCollisionResolver::ResolveCollision(Object* const other) noexcept
{
	if (m_pOwner->SetInvalid())
	{
		std::cout << "ÇÇ°Ý" << std::endl;
		if (const auto pSession = other->GetIocpEntity()->IsSession())
		{
			Protocol::s2c_DMG_INFO pkt;
			*pkt.mutable_dmg_pos() = m_pOwner->GetPos();
			pkt.set_dmg(10);
			const auto sector = (TRWorldRoom*)other->m_pCurSector.load(std::memory_order_relaxed);
			const auto pkt1 = ServerCore::MoveBroadcaster::CreateOutPacket(m_pOwner->GetIocpEntity());
			const auto pkt2 = ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt);
			sector->BroadCastEnqueue(pkt2);
			for (const auto s : sector->GetAdjSector4())
			{
				s->BroadCastEnqueue(pkt1);
			}
		}

	}
}
