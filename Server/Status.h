#pragma once
#include "Component.h"
#include "c2s_PacketHandler.h"

class HP
	:public BaseComponent
{
public:
	CONSTRUCTOR_BASE_COMPONENT(HP)
public:
	void SetMaxHP(const int maxHP_) { m_maxHP = maxHP_; }
public:
	int IncHP(const int incHP_)noexcept {
		int curHP = m_curHP.load(std::memory_order_relaxed);
		int afterHP = std::min(curHP + incHP_, m_maxHP);
		while (!m_curHP.compare_exchange_weak(curHP, afterHP)) {
			afterHP = std::min(curHP + incHP_, m_maxHP);
		}
		return afterHP;
	}
	int DecHP(const int decHP_)noexcept {
		int curHP = m_curHP.load(std::memory_order_relaxed);
		int afterHP = std::max(curHP - decHP_, 0);
		while (!m_curHP.compare_exchange_weak(curHP, afterHP)) {
			afterHP = std::max(curHP - decHP_, 0);
		}
		return afterHP;
	}
	void SetHP(const int hp_) { m_curHP = hp_; }
	void InitMaxHp(const int val) { m_maxHP = val; }
	const int GetHP()const noexcept { return m_curHP; }
private:
	std::atomic_int m_curHP = 200;
	int m_maxHP = 200;
};

class Status
	:public BaseComponent
{
public:
	Status(Object* const pOwner_) noexcept 
		: BaseComponent(COMP_TYPE::Status, pOwner_)
		, m_hp{ pOwner_ }
	{} 
	static inline consteval const COMP_TYPE GetCompTypeNameGlobal() noexcept {
		return COMP_TYPE::Status;
	}
public:
	void ModifyHP(const int val);

	void IncExp(const int val,const bool updateParty=true);
	void HalfExp();

	void ModifyGold(const int gold,const bool updateParty=true);
	const int GetCurrentGold()const noexcept { return m_gold; }

	void InitExp(const int val) { m_exp = val; }
	void InitMaxExp(const int val) { m_maxExp = val; }
	void InitLevel(const int val) { m_level = val; }
	void InitGold(const int val) { m_gold = val; }
	void InitHP(const int val) { m_hp.SetHP(val); }
	void InitMaxHP(const int val) { m_hp.InitMaxHp(val); }
private:
	void SendStatPacket(const Protocol::STAT s, const int val);
private:
	HP m_hp;
	std::atomic_int m_exp = 0;
	std::atomic_int m_maxExp = 10;
	std::atomic_int m_level = 1;
	std::atomic_int m_gold = 10;
};

