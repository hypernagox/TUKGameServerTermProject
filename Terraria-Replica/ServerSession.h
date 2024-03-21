#pragma once
#include "pch.h"
#include "PacketSession.h"

class ServerSession
	:public NetHelper::PacketSession
{
public:
	ServerSession();
	~ServerSession();
public:
	virtual void OnConnected() override;
	virtual void OnSend(c_int32 len)override;
	virtual void OnDisconnected()override;
private:

};

