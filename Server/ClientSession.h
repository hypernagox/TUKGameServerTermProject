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
	const S_ptr<Object>& GetPlayer()const noexcept { return m_pPlayer; };
	void SetPlayer(const S_ptr<Object>& pPlayer)noexcept { m_pPlayer = pPlayer; }
private:
	S_ptr<Object> m_pPlayer;
};

