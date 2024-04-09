#pragma once
#include "Component.h"

class Useable
	:public Component
{
public:
	Useable(std::string_view compName_,Object* const pOwner_)
		:Component{ compName_,pOwner_ }
	{}
public:
	void Update(const float dt_)override{}
	virtual void Use(const float dt_) = 0;
};

//class Inventory
//	:public Component
//{
//public:
//
//private:
//	ServerCore::Vector<S_ptr<Useable>> m_vecItemList;
//};


class AcquireItem
	:public Component
{
public:
	AcquireItem(Object* const pOwner_)
		:Component{ "ACQUIREITEM",pOwner_ }
	{}
public:
	void Update(const float dt_)override {}
	const bool TryGetItem()noexcept;
private:
	std::atomic_bool m_bCanGet = true;
};

class Attackable
	:public Useable
{
public:
	Attackable(Object* const pOwner_,const float dir_);
	~Attackable();
public:
	void Update(const float dt_)override;
	void PostUpdate(const float dt_)noexcept override;
	void Use(const float dt_)override;

private:
	const float m_dir;
};