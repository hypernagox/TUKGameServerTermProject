#pragma once
#include "PacketSession.h"
#include "DBEvent.h"

class Object;
class Status;

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
	const std::string_view GetCurSelectItemName()const noexcept;
	void SetCurItemName(const std::string_view itemName)noexcept;
	void UseCurItem(const float dt_ = 0.f)const noexcept;
	Status* const GetStatus()const noexcept;

	auto& GetName()noexcept { return m_name; }
	void SetName(const std::string_view str)noexcept {
		strcpy_s(m_name, str.data());
	}
	ServerCore::AtomicS_ptr<Object> m_partyOne;
private:
	S_ptr<Object> m_pPlayer;
	char m_name[32]{};
};

class DB_PlayerLogin
	:public ServerCore::DBEvent
{
public:
	DB_PlayerLogin(S_ptr<ClientSession> pPlayerSession)
		: m_pSession{ std::move(pPlayerSession) }
	{}
private:
	virtual void ExecuteQuery()noexcept override;
	virtual void Dispatch(ServerCore::IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;
private:
	S_ptr<ClientSession> m_pSession;
	ServerCore::DBBindRAII<1, 1> dbCheck{ L"SELECT COUNT(*) FROM Players WHERE PlayerID = ?" };
	ServerCore::DBBindRAII<1, 0> dbInsert{ L"INSERT INTO Players (PlayerID, Level, HP, Experience, Gold) VALUES (?, 1, 200, 0, 10)" };
	ServerCore::DBBindRAII<1, 4> dbSelect{ L"SELECT Level, HP, Experience, Gold FROM Players WHERE PlayerID = ?" };
	ServerCore::DBBindRAII<1, 2> dbItems{ L"SELECT ItemName, Quantity FROM PlayerItems WHERE PlayerID = ?" };
};

