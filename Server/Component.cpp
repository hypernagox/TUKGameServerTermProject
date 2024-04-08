#include "pch.h"
#include "Component.h"

BaseComponent::BaseComponent(std::string_view compName_, Object* const pOwner_)noexcept
	: m_strCompName{ compName_ }
	, m_pOwner{ pOwner_ }
{
}

BaseComponent::~BaseComponent()
{
}
