#pragma once
#include "Component.h"

class Item
	:public Component
{
public:
	Item(const COMP_TYPE compName,Object* const pOwner)noexcept
		:Component{compName,pOwner}
	{}
public:
	void Update(const float dt_)override{}
	virtual void Use(const float dt_) = 0;
	const std::string_view GetItemName()const noexcept { return m_strItemName; }
	void SetItemName(const std::string_view strName)noexcept { m_strItemName = strName; }
private:
	String m_strItemName;
};

class AcquireItem
	:public Component
{
public:
	CONSTRUCTOR_COMPONENT(AcquireItem)
public:
	const bool TryGetItem()noexcept;
	void SetIsGold()noexcept { m_bIsGold = true; }
	const bool IsGold()const noexcept { return m_bIsGold; }
	void Update(const float)noexcept override;
private:
	const std::chrono::steady_clock::time_point m_create_time = std::chrono::steady_clock::now();
	std::atomic_bool m_bCanGet = true;
	bool m_bIsGold = false;
};

class Attackable
	:public Item
{
public:
	Attackable(Object* const pOwner_);
	GET_COMP_TYPE_FUNC(Attackable)
	~Attackable();
public:
	void Use(const float dt_)override;
	void SetAtk(const int atk_)noexcept { m_atk = atk_; }
	int GetAtk()const noexcept { return m_atk; }
private:
	int m_atk = 0;
};

class RangeAttack
	:public Attackable
{
public:
	RangeAttack(Object* const pOwner)
		:Attackable{ pOwner } {}
public:
	void SetRange(const float range) { m_fRange = range; }
	void SetSpeed(const float speed) { m_fSpeed = speed; }
	float GetRange()const { return m_fRange; }
	float GetSpeed()const { return m_fSpeed; }
public:
	void Use(const float dt_)override;
private:
	// 사거리
	// 속도
	float m_fRange;
	float m_fSpeed;
};

class CountableItem
	:public Item
{
public:
	CountableItem(Object* const pOwner)
		:Item{ COMP_TYPE::CountableItem ,pOwner } {}
	GET_COMP_TYPE_FUNC(CountableItem)
public:
	void IncCount() noexcept { ++m_curCount; }
	int DecCount() noexcept;
	virtual void Use(const float dt_)override;
private:
	int m_curCount = 1;
};

class HPPotion
	:public CountableItem
{
public:
	HPPotion(Object* const pOwner)
		:CountableItem{ pOwner } {}
public:
	virtual void Use(const float dt_)override;

private:
};