#pragma once
#include "Component.h"

class ServerCore::SessionManageable;
class Object;

class BroadCaster
	:public Component
{
public:
	BroadCaster(const COMP_TYPE compName_, Object* const pObj_, ServerCore::SessionManageable* const pRoom_)
		: Component{ compName_,pObj_ }
		, m_pCurRoom{ pRoom_ }
	{}

public:
	void Update(const float)override{}
	virtual void PostUpdate(const float)noexcept = 0;
protected:
	ServerCore::SessionManageable* const m_pCurRoom;
};

class MoveBroadCaster
	:public BroadCaster
{
public:
	MoveBroadCaster(Object* const pObj_, ServerCore::SessionManageable* const pRoom_)
		:BroadCaster{ COMP_TYPE::MoveBroadCaster,pObj_,pRoom_ }
	{}
	GET_COMP_TYPE_FUNC(MoveBroadCaster)
public:
	void Update(const float)override {}
	void PostUpdate(const float)noexcept override;
};

