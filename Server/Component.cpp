#include "pch.h"
#include "Component.h"

BaseComponent::BaseComponent(const COMP_TYPE compName_, Object* const pOwner_)noexcept
	: m_eCompType{ compName_ }
	, m_pOwner{ pOwner_ }
{
}

BaseComponent::~BaseComponent()
{
}
