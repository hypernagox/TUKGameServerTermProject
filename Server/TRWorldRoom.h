#pragma once
#include "TRWorld.h"
#include "CollisionChecker.h"
#include "EventHandler.h"

class Object;

class TRWorldRoom
	:public TRWorld
	,public ServerCore::SessionManageable
{
	static constexpr inline const uint64 CONTAINER_SIZE = ServerCore::ThreadMgr::NUM_OF_THREADS ;
public:
	TRWorldRoom(const SECTOR sector_);
	~TRWorldRoom();
public:
	void Update(const uint64 tick_ms = 100);

	void Init();
public:
	const float GetSectorDT()const noexcept { return m_timer.GetDT(); }
public:
	void AddObjectEnqueue(const GROUP_TYPE eType_, S_ptr<Object> pObj_);
	//void DeleteObjectEnqueue(const GROUP_TYPE eType_, const uint64 objID_);
	void TickTileCollision();
	void TryGetItem(Object* const pPlayer);
private:
	void AddObject(const GROUP_TYPE eType_, S_ptr<Object> pObj_, const uint64 exceptThreadID);
	//void DeleteObject(const GROUP_TYPE eType_, const uint64 objID_, const uint64 exceptThreadID);
private:
	void UpdateWorldCollision();
	

	

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
};

