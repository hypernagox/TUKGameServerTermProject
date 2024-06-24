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
	// 1. ������ ���� ��Ŷ
	// 2. ���� �������� ����?
	// 3. �κ��丮 -> ������Ʈ -> Attackable ã��
	// 4. ��Ÿ�� �¾�?
	// 5. ������ ���ѹ��� ���͵�� �浹�˻�
	// 6. �浹�ߴٸ� �浹��Ŷ
	// 7. ��Ÿ�� ����
	
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
	// ���� ��, ������ �´� ���� �� ������
	// ���ͷ� ����� �÷��̾�� x�Ÿ�����
	// ex)  ��:300 ��:500
	// �ӵ��� 10
	// 200ms ���� �̺�Ʈ
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
		//std::cout << "���" << std::endl;
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
