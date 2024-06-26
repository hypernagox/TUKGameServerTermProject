#pragma once
#include "IocpObject.h"
#include "IocpEvent.h"

namespace ServerCore
{
	enum class TIMER_STATE : uint8
	{
		IDLE,
		RUN,
		PREPARE,

		END,
	};

	class TimerObject
		:public IocpEntity
	{
	public:
		TimerObject(const uint16_t type_id) noexcept
			:IocpEntity{ type_id }
		{}
	public:
		virtual const bool IsValid()const noexcept override { return m_bStopFlag.load(std::memory_order_relaxed); }
		virtual void ToAwaker(const IocpEntity* const awaker)noexcept abstract;
		virtual void InitTimer(const uint64 tick_ms)noexcept;
		const bool ExecuteTimer(const IocpEntity* const awaker)noexcept;
		void StopTimer()noexcept { m_bStopFlag.store(true, std::memory_order_release); }
		void SetTickInterval(const uint64 tick_ms)noexcept { m_tickInterval = tick_ms; }
		virtual const bool CanAwake(const IocpEntity* const awaker)const noexcept { return true; }
	protected:
		virtual HANDLE GetHandle()const noexcept { return nullptr; }
		virtual const ServerCore::TIMER_STATE TimerUpdate()noexcept = 0;
	private:
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;
	protected:
		std::atomic_bool m_bStopFlag = false;
		std::atomic<TIMER_STATE> m_timer_state = TIMER_STATE::IDLE;
		uint64_t m_tickInterval = 0;
		IocpEvent m_timerEvent{ EVENT_TYPE::TIMER };
	};
}

