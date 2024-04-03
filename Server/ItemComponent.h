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
	const bool TryGetItem()noexcept {
		return m_bCanGet.exchange(false, std::memory_order_acq_rel);
	}
private:
	std::atomic_bool m_bCanGet = true;
};

class Attackable
	:public Useable
{
public:
	Attackable(std::string_view compName_, Object* const pOwner_);
	~Attackable();
public:
	void Update(const float dt_)override;
	void Use(const float dt_)override;

private:

};