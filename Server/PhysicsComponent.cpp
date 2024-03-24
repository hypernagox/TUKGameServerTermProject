#include "pch.h"
#include "PhysicsComponent.h"
#include "Object.h"

Collider::Collider(Object* const pOwner_)
	: Component{ "COLLIDER",pOwner_ }
	, m_iID{ Collider::g_iNextID.fetch_add(1,std::memory_order_acq_rel) }
{
}

Collider::~Collider()
{
}

void Collider::Update(const float dt_)
{
	m_vFinalPos = GetOwner()->GetPos() + m_vOffsetPos;
}

//////////////////////////////////////////////////

RigidBody::RigidBody(Object* const pOwner_)
	:Component{ "RIGIDBODY",pOwner_ }
{
}

RigidBody::~RigidBody()
{
}

void RigidBody::Move(const float dt_)
{
	if (m_vVelocity.IsZero())
	{
		return;
	}
	if (m_bIsGround)
	{
		m_vVelocity.y = 0.f;
	}
	
	Vec2 vPos = GetOwner()->GetPos();
	const Vec2 vScale = GetOwner()->GetScale() / 2.f;


	vPos += m_vVelocity * dt_;

	

	GetOwner()->SetWillPos(vPos);
}

void RigidBody::Update(const float dt_)
{
	if (bitwise_absf(m_vVelocity.x) <= bitwise_absf(m_vMaxVelocityOrigin.x))
	{
		//SetLimitOrigin();
	}
	update_gravity();
	m_vAccel = m_vForce / m_fMass;
	m_vVelocity += m_vAccel * dt_;
	if (!m_vVelocity.IsZero())
	{
		Vec2 vFriction = m_vVelocity.Normalize();
		vFriction *= m_fFriction * dt_;
		if (m_vVelocity.length() <= vFriction.length())
		{
			m_vVelocity = Vec2{ 0.,0. };
		}
		else
		{
			m_vVelocity -= vFriction;
		}
	}
	if (bitwise_absf(m_vMaxVelocity.x) < bitwise_absf(m_vVelocity.x))
	{
		m_vVelocity.x = m_vVelocity.x / bitwise_absf(m_vVelocity.x) * bitwise_absf(m_vMaxVelocity.x);
	}
	if (bitwise_absf(m_vMaxVelocity.y) < bitwise_absf(m_vVelocity.y))
	{
		m_vVelocity.y = m_vVelocity.y / bitwise_absf(m_vVelocity.y) * bitwise_absf(m_vMaxVelocity.y);
	}
	Move(dt_);
	m_vForce = Vec2{ 0.,0. };
}

void RigidBody::update_gravity()
{
	const auto vLow = GetOwner()->GetPos().y + GetOwner()->GetScale().y / 2.f;

	if (vLow >= 4096.f)
	{
		m_bIsGround = true;
	}
	if (m_bGravity && !m_bIsGround)
	{
		AddForce(Vec2{ 0.0f,2000.0f });
	}
}
