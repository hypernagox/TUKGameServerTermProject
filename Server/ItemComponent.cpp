#include "pch.h"
#include "ItemComponent.h"
#include "Object.h"
#include "PhysicsComponent.h"

Attackable::Attackable(Object* const pOwner_)
	:Useable{ "ATTACKABLE",pOwner_ }
{
}

Attackable::~Attackable()
{
}

void Attackable::Update(const float dt_)
{
	//m_pOwner->GetComp("RIGIDBODY")->Cast<RigidBody>()->AddVeloci(Vec2{ 1000.f,0.f }*dt_);
	//m_pOwner->SetPos(m_pOwner->GetPos() + Vec2{ 1000.f,0.f } *dt_);
	//m_pOwner->PostUpdate(dt_);
}

void Attackable::PostUpdate(const float dt_) noexcept
{
	m_pOwner->SetPos(m_pOwner->GetPos() + Vec2{ 1000.f,0.f } *dt_);
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
}

const bool AcquireItem::TryGetItem() noexcept
{
	m_pOwner->GetComp("COLLIDER")->Cast<Collider>()->SetDead();
	return m_bCanGet.exchange(false, std::memory_order_acq_rel);
}
