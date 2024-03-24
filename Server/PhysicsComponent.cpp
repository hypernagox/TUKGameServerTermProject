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

void Collider::Update()
{
	m_vFinalPos = GetOwner()->GetPos() + m_vOffsetPos;
}

