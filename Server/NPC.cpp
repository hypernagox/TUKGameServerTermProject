#include "pch.h"
#include "NPC.h"
#include "Object.h"

NPC::NPC()
	:ServerCore::IocpEntity{ 1 }
{
}

const bool NPC::IsValid() const noexcept
{
	return GetContentsEntity()->ObjectCast()->IsValid();
}
