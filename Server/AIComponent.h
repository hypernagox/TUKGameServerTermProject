#pragma once
#include "Component.h"
#include "TimerObject.h"
#include "TRWorld.h"

class AIComponent
	:public BaseComponent
{
public:
	AIComponent(const COMP_TYPE eType_, Object* const pOwner_)noexcept
		:BaseComponent{ eType_,pOwner_ } {}
public:
	virtual const bool ExecuteAI()noexcept abstract;
};

#define CONSTRUCTOR_AI_COMPONENT(ClassType) \
    ClassType(Object* const pOwner_) noexcept \
        : AIComponent(COMP_TYPE::ClassType, pOwner_) {} \
    GET_COMP_TYPE_FUNC(ClassType)

class Astar
	:public AIComponent
{
public:
	CONSTRUCTOR_AI_COMPONENT(Astar)
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
public:
	virtual const bool ExecuteAI()noexcept override;
private:
	Vec2Int m_dest{65,188};
	Vec2Int m_start{106,197};
};

