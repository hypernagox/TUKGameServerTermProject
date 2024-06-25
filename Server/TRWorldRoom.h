#pragma once
#include "ServerCorePch.h"
#include "TRWorld.h"
#include "CollisionChecker.h"
#include "EventHandler.h"

class Object;
class ClientSession;
class TRWorldChunk;
class Attackable;
class RangeAttack;

class TRWorldRoom
	: public ServerCore::Sector
{
	friend class TRWorldMgr;
public:
	//static inline TRWorld g_trWorld;
	//static inline TRWorld g_trWorldMain{ 1 };
	static constexpr inline const uint64 CONTAINER_SIZE = ServerCore::ThreadMgr::NUM_OF_THREADS;
	//static inline ServerCore::LinkedHashMap<uint64, ClientSession> g_allPlayers[CONTAINER_SIZE];
public:
	TRWorldRoom(const uint16_t sector_, TRWorldChunk* const pChunk_);
	~TRWorldRoom();
	//static TRWorld* const GetTRWorldMain()noexcept { return &g_trWorldMain; }
	//static TRWorld* const GetTRWorldMain()noexcept { return &g_trWorldMain; }
	//void SetTRWorld(TRWorld* p) { m_pTRWorld = p; }
	//TRWorld* const GetTRWorld()noexcept { return m_pTRWorld; }
public:
	void Update(const uint64 tick_ms = 100);

	void Init();
public:
	const float GetSectorDT()const noexcept { return m_timer.GetDT(); }
	const auto& GetGroupObject(const int32 thID_, const GROUP_TYPE eType_)const noexcept {
		return m_worldObjectList[thID_][etoi(eType_)].GetItemListRef();
	}
	const auto& GetCurThreadGroupObject(const GROUP_TYPE eType_)const noexcept {
		thread_local const int32 thIdx = ServerCore::GetCurThreadIdx();
		return GetGroupObject(thIdx, eType_);
	}
public:
	auto& GetAdjSector8()noexcept { return m_adjSector8; }
	auto& GetAdjSector4()noexcept { return m_adjSector4; }
	void AddObjectEnqueue(const GROUP_TYPE eType_, S_ptr<Object> pObj_);
	void AddEnterEnqueue(const GROUP_TYPE eType_, S_ptr<Object> pObj_);
	//void DeleteObjectEnqueue(const GROUP_TYPE eType_, const uint64 objID_);
	void TickTileCollision();
	bool TryGetItem(const S_ptr<Object>& pPlayer,const Vec2 offset_ = Vec2{});

	void TryGetItemSector(const S_ptr<Object>& pPlayer, const Vec2 offset_ = Vec2{});

	void BroadCastToWorld(const S_ptr<ServerCore::SendBuffer> pSendBuffer);
	void LeaveObjectEnqueue(const uint64 playerID, const GROUP_TYPE eType)noexcept;
	TRWorldChunk* const GetWorldChunk()noexcept { return m_pParentChunk; }
	

	void TryCollisionCheckEnqueue(Attackable* pCol, const GROUP_TYPE targetGroup, const Vec2 offset = {});
	void TryCollisionCheckRangeEnqueue(RangeAttack* pCol, const GROUP_TYPE targetGroup, const int dir);
private:
	void TryCollisionCheckRange(const uint64 id, const S_ptr<RangeAttack> pCol, const GROUP_TYPE targetGroup, const int dir);
	void TryCollisionCheck(const uint64 id,const S_ptr<Attackable> pCol, const GROUP_TYPE targetGroup,const Vec2 offset);
	void LeaveObject(const uint64 playerID,const GROUP_TYPE eType)noexcept;
	void AddObject(const GROUP_TYPE eType_, S_ptr<Object> pObj_, const uint64 exceptThreadID, const bool bIsSession = false);
	//void DeleteObject(const GROUP_TYPE eType_, const uint64 objID_, const uint64 exceptThreadID);
private:
	void UpdateWorldCollision();
	
	virtual void ImigrationAfterBehavior(const S_ptr<ServerCore::Sector> beforeRoom, ServerCore::IocpEntity* const pEntity_)noexcept override;
	

	void UpdateTileCollisionForTick(const S_ptr<Object> pObj_)const noexcept;

	void RegisterGroup(const GROUP_TYPE _eLeft, const GROUP_TYPE _eRight) {
		m_collisionChecker.RegisterGroup(_eLeft, _eRight);
	}
	
private:
	ServerCore::LinkedHashMap<uint64, Object> m_worldObjectList[CONTAINER_SIZE][ServerCore::etoi(GROUP_TYPE::END)];
	ServerCore::Timer m_timer;
	TRWorld* m_pTRWorld;
	TRWorldChunk* const m_pParentChunk;
	CollisionChecker m_collisionChecker;
	EventHandler m_eventHandler;
	ServerCore::Vector<ServerCore::Task> m_vecCollisionTask;
	std::atomic_int m_jobCount = 0;

	ServerCore::Vector<ServerCore::Task*> m_vecForBroadCastToWorld;
	ServerCore::Vector <ServerCore::Sector*> m_adjSector8;
	ServerCore::Vector <ServerCore::Sector*> m_adjSector4;
};

