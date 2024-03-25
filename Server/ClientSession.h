#pragma once
#include "PacketSession.h"

class Object;

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
	Object* const GetPlayer()const noexcept { return m_pPlayer; };
	void SetPlayer(Object* const pPlayer)noexcept { m_pPlayer = pPlayer; }
private:
	Object* m_pPlayer;
};

