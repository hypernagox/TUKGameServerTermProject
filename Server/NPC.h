#pragma once
#include "IocpObject.h"

class NPC
	:public ServerCore::IocpEntity
{
public:
	NPC();
	virtual ~NPC() = default;
public:
	virtual HANDLE GetHandle()const noexcept { return nullptr; }
	virtual void Dispatch(ServerCore::IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept {}
private:

};

