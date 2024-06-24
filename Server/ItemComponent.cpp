#include "pch.h"
#include "ItemComponent.h"
#include "Object.h"
#include "PhysicsComponent.h"
#include "TRWorldRoom.h"
#include "PacketSession.h"
#include "c2s_PacketHandler.h"
#include "Inventory.h"
#include "Status.h"
#include "DBMgr.h"
#include "DBPacket.h"

Attackable::Attackable(Object* const pOwner_)
	: Item{ COMP_TYPE::Attackable,pOwner_ }
{
}

Attackable::~Attackable()
{
}


void Attackable::Use(const float dt_)
{
	// 1. 유저의 공격 패킷
	// 2. 지금 아이템이 뭐지?
	// 3. 인벤토리 -> 오브젝트 -> Attackable 찾음
	// 4. 쿨타임 맞아?
	// 5. 유저의 속한방의 몬스터들과 충돌검사
	// 6. 충돌했다면 충돌패킷
	// 7. 쿨타임 걸음
	
	if (const auto sector = m_pOwner->m_pCurSector.load(std::memory_order_acquire))
	{
		((TRWorldRoom*)sector)->TryCollisionCheckEnqueue(this, GROUP_TYPE::MONSTER,Vec2{10.f,0} * (float)m_pOwner->GetDir());
	}
}

const bool AcquireItem::TryGetItem() noexcept
{
	m_pOwner->GetComp<Collider>()->SetDead();
	return m_bCanGet.exchange(false, std::memory_order_acq_rel);
}

void AcquireItem::Update(const float) noexcept
{
	const auto elapsed_time = std::chrono::steady_clock::now() - m_create_time;
	if (3000 <= std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_time).count())
	{
		m_pOwner->SetInvalid();
	}
}

void RangeAttack::Use(const float dt_)
{
	// 시전 시, 범위에 맞는 몬스터 다 가져옴
	// 벡터로 만들고 플레이어와 x거리차이
	// ex)  플:300 몬:500
	// 속도가 10
	// 200ms 이후 이벤트
	if (const auto sector = (TRWorldRoom*)m_pOwner->m_pCurSector.load(std::memory_order_acquire))
	{
		Protocol::s2c_CREATE_MISSILE pkt;
		pkt.set_obj_id(0);
		*pkt.mutable_obj_pos() = ToProtoVec2(m_pOwner->GetPos());
		pkt.set_dir(m_pOwner->GetDir());
		pkt.set_speed(GetSpeed()*10.f);
		for(const auto s:sector->GetAdjSector8())
			s->BroadCastEnqueue(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
		((TRWorldRoom*)sector)->TryCollisionCheckRangeEnqueue(this, GROUP_TYPE::MONSTER,GetOwner()->GetDir());
	}
}

int CountableItem::DecCount() noexcept
{
	const int prev_count = --m_curCount;
	if (0 >= prev_count)
	{
		//std::cout << "사용" << std::endl;
		m_pOwner->GetComp<Inventory>()->RemoveItem(GetItemName());
	}
	s2q_CONSUME_ITEM pkt;
	strcpy_s(pkt.itemName, GetItemName().data());
	pkt.user_id = m_pOwner->GetObjID();
	pkt.quantity = 1;	
	RequestQueryServer(pkt);
	return prev_count;
}

void CountableItem::Use(const float dt_)
{
	//DecCount();
}

void HPPotion::Use(const float dt_)
{
	DecCount();

	m_pOwner->GetComp<Status>()->ModifyHP(50);
}
