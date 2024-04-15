#pragma once
#include "IocpObject.h"
#include "IocpEvent.h"

namespace ServerCore
{
	class TimerObject
		:public IocpObject
	{
	public:
		virtual void InitTimer(const S_ptr<TimerObject>& forCacheThis_, const uint64 tick_ms)noexcept;
		void SetTickInterval(const uint64 tick_ms)noexcept { m_tickInterval = tick_ms; }
	protected:
		virtual HANDLE GetHandle()const noexcept { return nullptr; }
		virtual const bool TimerUpdate()noexcept = 0;
	private:
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;
	private:
		uint64_t m_tickInterval = 0;
		IocpEvent m_timerEvent{ EVENT_TYPE::TIMER };
	};
}

