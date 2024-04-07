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

	static inline TRWorld g_trWorld;
	static constexpr inline const uint64 CONTAINER_SIZE = ServerCore::ThreadMgr::NUM_OF_THREADS;
	static inline ServerCore::LinkedHashMap<uint64, ClientSession> g_allPlayers[CONTAINER_SIZE];
public:
	TRWorldRoom(const SECTOR sector_);
	~TRWorldRoom();
	static TRWorld* const GetTRWorld()noexcept { return &g_trWorld; }
public:
	void Update(const uint64 tick_ms = 100);

	void Init();
public:
	const float GetSectorDT()const noexcept { return m_timer.GetDT(); }
public:
	void AddObjectEnqueue(const GROUP_TYPE eType_, S_ptr<Object> pObj_);
	//void DeleteObjectEnqueue(const GROUP_TYPE eType_, const uint64 objID_);
	void TickTileCollision();
	void TryGetItem(const S_ptr<Object>& pPlayer,const Vec2 offset_ = Vec2{});
	void BroadCastToWorld(const S_ptr<ServerCore::SendBuffer> pSendBuffer);
	void LeavePlayerEnqueue(const uint64 playerID)noexcept;
private:
	void LeavePlayer(const uint64 playerID)noexcept;
	void AddObject(const GROUP_TYPE eType_, S_ptr<Object> pObj_, const uint64 exceptThreadID, const bool bIsSession = false);
	//void DeleteObject(const GROUP_TYPE eType_, const uint64 objID_, const uint64 exceptThreadID);
private:
	void UpdateWorldCollision();
	
	virtual void ImigrationAfterBehavior(const S_ptr<ServerCore::SessionManageable> beforeRoom, const S_ptr<ServerCore::PacketSession> pSession_)noexcept override;
	

	void UpdateTileCollisionForTick(const S_ptr<Object> pObj_)const noexcept;

	void RegisterGroup(const GROUP_TYPE _eLeft, const GROUP_TYPE _eRight) {
		m_collisionChecker.RegisterGroup(_eLeft, _eRight);
	}
private:
	ServerCore::LinkedHashMap<uint64, Object> m_worldObjectList[CONTAINER_SIZE][ServerCore::etoi(GROUP_TYPE::END)];
	ServerCore::Timer m_timer;
	

	CollisionChecker m_collisionChecker;
	EventHandler m_eventHandler;
	ServerCore::Vector<ServerCore::Task*> m_vecCollisionTask;
	std::atomic_int m_jobCount = 0;

	ServerCore::Vector<ServerCore::Task*> m_vecForBroadCastToWorld;
};

