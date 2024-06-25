#pragma once
#include "TimerObject.h"

class AIComponent;

class TimerNPC
	:public ServerCore::TimerObject
{
public:
	TimerNPC()noexcept
		:TimerObject{ 2 }
	{}
public:
	virtual void InitTimer(const uint64 tick_ms)noexcept override;
	virtual void ToAwaker(const IocpEntity* const awaker)noexcept override;
	virtual const ServerCore::TIMER_STATE TimerUpdate()noexcept override;
public:
	void AddAIComponent(S_ptr<AIComponent> ai)noexcept;
	const uint64_t GetCurChaseUser()const noexcept { return m_curChaseUser; }
	virtual const bool CanAwake(const IocpEntity* const awaker)const noexcept override;
private:
	S_ptr<AIComponent> m_aiComponent;
	uint64_t m_curChaseUser = 0;
};

