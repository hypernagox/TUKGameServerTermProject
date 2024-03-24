#include "pch.h"
#include "Component.h"

Component::Component(std::string_view compName_,Object* const pOwner_)
	:m_strCompName{ compName_ },
	 m_pOwner{ pOwner_ }
{
}

Component::~Component()
{
}
