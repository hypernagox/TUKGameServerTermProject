#pragma once
#include "Component.h"
#include "IocpObject.h"
#include "IocpEvent.h"
#include "TRWorld.h"

class Astar
	:public Component
	,public ServerCore::IocpObject
{
public:
	Astar(Object* const pOwner_);
	~Astar();
	struct AstarNode
	{
		AstarNode(const Vec2Int pos_, const uint16 cost_) :pos{ pos_ }, cost{ cost_ } {}
		auto operator<=>(const AstarNode& other)const noexcept { return cost <=> other.cost; }
		Vec2Int pos;
		uint16 cost;
	};
	static const uint16_t calculateH(const Vec2Int src,const Vec2Int dest)noexcept {
		return ::bitwise_absi(src.x - dest.x) + ::bitwise_absi(src.y - dest.y);
	}
	void InitAI(const S_ptr<Astar>& forCacheThis_)noexcept;
public:
	void Update(const float)override;
	void PostUpdate(const float)noexcept override;

	virtual HANDLE GetHandle()const noexcept override { return nullptr; }
	virtual void Dispatch(ServerCore::IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;
private:
	S_ptr<Object> m_ownerForValid;
	std::priority_queue<AstarNode, ServerCore::Vector<AstarNode>, std::greater<AstarNode>> m_pqAstar;
	Vec2Int m_dest{65,188};
	Vec2Int m_start{106,197};
	Vec2Int m_prev;
	ServerCore::IocpEvent m_timerEvent{ ServerCore::EVENT_TYPE::TIMER };
	ServerCore::HashMap<Vec2Int, Vec2Int> m_parent;
	uint16 best[TRWorld::WORLD_HEIGHT][TRWorld::WORLD_WIDTH];
	ServerCore::Vector<Vec2Int> m_path;
	ServerCore::SpinLock m_lock;
	float m_accTime = 0.f;
	float m_dist = 0.f;
	Vec2Int m_prevPos;
	Vec2Int m_curPos;
};

