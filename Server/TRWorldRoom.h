#pragma once
#include "TRWorld.h"
#include "CollisionChecker.h"
#include "EventHandler.h"

class Object;
class ClientSession;

class TRWorldRoom
	: public ServerCore::SessionManageable
{
	friend class TRWorldMgr;
public:
	static inline TRWorld g_trWorld;
	static inline TRWorld g_trWorldMain{ 1 };
	static constexpr inline const uint64 CONTAINER_SIZE = ServerCore::ThreadMgr::NUM_OF_THREADS;
	static inline ServerCore::LinkedHashMap<uint64, ClientSession> g_allPlayers[CONTAINER_SIZE];
public:
	TRWorldRoom(const SECTOR sector_);
	TRWorldRoom(const uint16_t sector_);
	~TRWorldRoom();
	//static TRWorld* const GetTRWorldMain()noexcept { return &g_trWorldMain; }
	static TRWorld* const GetTRWorldMain()noexcept { return &g_trWorldMain; }
	void SetTRWorld(TRWorld* p) { m_pTRWorld = p; }
	TRWorld* const GetTRWorld()noexcept { return m_pTRWorld; }
public:
	void Update(const uint64 tick_ms = 100);

	void Init();
public:
	const float GetSectorDT()const noexcept { return m_timer.GetDT(); }
	const auto& GetGroupObject(const int32 thID_, const GROUP_TYPE eType_)const noexcept {
		return m_worldObjectList[thID_][etoi(eType_)].GetItemListRef();
	}
public:
	auto& GetAdjSector8()noexcept { return m_adjSector8; }
	auto& GetAdjSector4()noexcept { return m_adjSector4; }
	void AddObjectEnqueue(const GROUP_TYPE eType_, S_ptr<Object> pObj_);
	//void DeleteObjectEnqueue(const GROUP_TYPE eType_, const uint64 objID_);
	void TickTileCollision();
	bool TryGetItem(const S_ptr<Object>& pPlayer,const Vec2 offset_ = Vec2{});

	void TryGetItemSector(const S_ptr<Object>& pPlayer, const Vec2 offset_ = Vec2{});

	void BroadCastToWorld(const S_ptr<ServerCore::SendBuffer> pSendBuffer);
	void LeavePlayerEnqueue(const uint64 playerID)noexcept;
private:
	void LeavePlayer(const uint64 playerID)noexcept;
	void AddObject(const GROUP_TYPE eType_, S_ptr<Object> pObj_, const uint64 exceptThreadID, const bool bIsSession = false);
	//void DeleteObject(const GROUP_TYPE eType_, const uint64 objID_, const uint64 exceptThreadID);
private:
	void UpdateWorldCollision();
	
	virtual void ImigrationAfterBehavior(const S_ptr<ServerCore::SessionManageable> beforeRoom, const S_ptr<ServerCore::IocpEntity> pSession_)noexcept override;
	

	void UpdateTileCollisionForTick(const S_ptr<Object> pObj_)const noexcept;

	void RegisterGroup(const GROUP_TYPE _eLeft, const GROUP_TYPE _eRight) {
		m_collisionChecker.RegisterGroup(_eLeft, _eRight);
	}
private:
	ServerCore::LinkedHashMap<uint64, Object> m_worldObjectList[CONTAINER_SIZE][ServerCore::etoi(GROUP_TYPE::END)];
	ServerCore::Timer m_timer;
	TRWorld* m_pTRWorld;

	CollisionChecker m_collisionChecker;
	EventHandler m_eventHandler;
	ServerCore::Vector<ServerCore::Task> m_vecCollisionTask;
	std::atomic_int m_jobCount = 0;

	ServerCore::Vector<ServerCore::Task*> m_vecForBroadCastToWorld;
	ServerCore::Vector <ServerCore::SessionManageable*> m_adjSector8;
	ServerCore::Vector <ServerCore::SessionManageable*> m_adjSector4;
};

