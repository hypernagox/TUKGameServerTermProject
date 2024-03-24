#pragma once
#include "TRWorld.h"

class Object;

class TRWorldRoom
	:public TRWorld
	,public ServerCore::SessionManageable
{
public:
	TRWorldRoom(const SECTOR sector_);
	~TRWorldRoom();
public:
	void Update(const uint64 tick_ms = 30);

	void Init();
public:
	void AddObjectEnqueue(const GROUP_TYPE eType_, S_ptr<Object> pObj_);
	void DeleteObjectEnqueue(const GROUP_TYPE eType_, const uint64 objID_);

private:
	void AddObject(const GROUP_TYPE eType_, S_ptr<Object> pObj_);
	void DeleteObject(const GROUP_TYPE eType_, const uint64 objID_);
private:
	ServerCore::LinkedHashMap<uint64, Object> m_worldObjectList[ServerCore::etoi(GROUP_TYPE::END)];
};

