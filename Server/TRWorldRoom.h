#pragma once
#include "TRWorld.h"
#include "CollisionChecker.h"
#include "EventHandler.h"

class Object;

class TRWorldRoom
	:public TRWorld
	,public ServerCore::SessionManageable
{
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
	void DeleteObjectEnqueue(const GROUP_TYPE eType_, const uint64 objID_);
	void TickTileCollision();
private:
	void AddObject(const GROUP_TYPE eType_, S_ptr<Object> pObj_);
	void DeleteObject(const GROUP_TYPE eType_, const uint64 objID_);
private:
	void UpdateWorldCollision();
	

	void UpdateTileCollisionForTick(Object* const pObj_)const;

	void RegisterGroup(const GROUP_TYPE _eLeft, const GROUP_TYPE _eRight) {
		m_collisionChecker.RegisterGroup(_eLeft, _eRight);
	}
private:
	ServerCore::LinkedHashMap<uint64, Object> m_worldObjectList[ServerCore::etoi(GROUP_TYPE::END)];
	ServerCore::Timer m_timer;
	CollisionChecker m_collisionChecker;
	EventHandler m_eventHandler;
};

