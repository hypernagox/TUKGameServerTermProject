#include "pch.h"
#include "CollisionResolver.h"
#include "Object.h"
#include "MoveBroadcaster.h"
#include "PacketSession.h"
#include "c2s_PacketHandler.h"
#include "TRWorldRoom.h"
#include "ItemComponent.h"
#include "Status.h"
#include "ObjectFactory.h"

void MonsterCollisionResolver::ResolveCollision(Object* const other, Attackable* const atk) noexcept
{
	const auto sector = (TRWorldRoom*)other->m_pCurSector.load(std::memory_order_relaxed);
	if (!sector)
		return;
	const auto hp = m_pOwner->GetComp<HP>();
	Protocol::s2c_DMG_INFO pkt;
	*pkt.mutable_dmg_pos() = m_pOwner->GetPos();
	pkt.set_dmg(atk->GetAtk());
	sector->BroadCastEnqueue(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
	if(0>= hp->DecHP(atk->GetAtk()))
	{ 
		if (m_pOwner->SetInvalid())
		{
			if (const auto pSession = other->GetIocpEntity()->IsSession())
			{
				const auto pkt1 = ServerCore::MoveBroadcaster::CreateOutPacket(m_pOwner->GetIocpEntity());
				for (const auto s : sector->GetAdjSector4())
				{
					s->BroadCastEnqueue(pkt1);
				}
				other->GetComp<Status>()->IncExp(6);
			}
			ObjectBuilder b;
			b.str = "GOLD";
			b.pos = m_pOwner->GetPos();
			auto gold = ObjectFactory::CreateDropItem(b);
			gold->GetComp<AcquireItem>()->SetIsGold();
			sector->AddEnterEnqueue(GROUP_TYPE::DROP_ITEM, std::move(gold));
		}
	}
}
