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
	const Vec2 vPrevPos = GetPrevPos();
	const Vec2 vCurPos = GetOwner()->GetPos();

	const Vec2 vDir = vCurPos - vPrevPos;



	m_vFinalPos = GetOwner()->GetPos() + m_vOffsetPos;
}

const Vec2 Collider::GetScale() const noexcept
{
	return m_pOwner->GetScale();
}

const Vec2 Collider::GetPrevPos() const noexcept
{
	return m_pOwner->GetPrevPos();
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
	
	const Vec2 vPos = GetOwner()->GetPos() + m_vVelocity * dt_;
	const Vec2 vScale = GetOwner()->GetScale() / 2.f;

	m_pOwner->SetPos(vPos);
	m_pOwner->SetWillPos(vPos);
}

void RigidBody::Update(const float dt_)
{
	//if (m_pOwner->GetObjectGroup() == GROUP_TYPE::DROP_ITEM)
	//{
	//	std::cout << m_pOwner->GetPos().x << ", " << m_pOwner->GetPos().y << std::endl;
	//}
	if (bitwise_absf(m_vVelocity.x) <= bitwise_absf(m_vMaxVelocityOrigin.x))
	{
		//SetLimitOrigin();
	}
	update_gravity();
	m_vAccel = m_vForce / m_fMass;
	m_vVelocity += m_vAccel * dt_;
	if (!m_vVelocity.IsZero())
	{
		const Vec2 vFriction = m_vVelocity.Normalize() * m_fFriction * dt_;

		if (m_vVelocity.length() <= vFriction.length())
		{
			m_vVelocity = Vec2{ 0.f,0.f };
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
	m_vForce = Vec2{ 0.f,0.f };
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

const Vec2 RigidBody::GetPrevPos()const noexcept
{
	return m_pOwner->GetPrevPos();
}

/////////////////////////////////////////////////////

KeyInputHandler::~KeyInputHandler()
{

}

void KeyInputHandler::Update(const float dt_)
{
	for (const auto& [key, info] : m_keyHandlerMap)
	{
		info.second(m_pOwner, info.first);
	}
	
}

void KeyInputHandler::PostUpdate(const float dt_)noexcept
{
	for (auto& [key, info] : m_keyHandlerMap)
	{
		if (KEY_STATE::KEY_TAP == info.first)
		{
			info.first = KEY_STATE::KEY_HOLD;
		}
		else if (KEY_STATE::KEY_AWAY == info.first)
		{
			info.first = KEY_STATE::KEY_NONE;
		}
	}
}