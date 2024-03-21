#pragma once
#include "PacketSession.h"

class ClientSession
	:public ServerCore::PacketSession
{
public:
	ClientSession();
	~ClientSession();
public:
	virtual void OnConnected()override;
	virtual void OnSend(c_int32 len)noexcept override;
	virtual void OnDisconnected()override;
public:

private:

};

