#include "ServerCorePch.h"
#include "TimerObject.h"
#include "TaskTimerMgr.h"

namespace ServerCore
{
	void TimerObject::InitTimer(const S_ptr<TimerObject>& forCacheThis_, const uint64 tick_ms) noexcept
	{
		// 초기화시 필요한 추가동작은 여기에

		register_cache_shared_core(forCacheThis_);
		m_timerEvent.SetIocpObject(forCacheThis_);
		m_tickInterval = tick_ms;
		
		Mgr(TaskTimerMgr)->ReserveAsyncTask(tick_ms, &m_timerEvent);
	}

	void TimerObject::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
	{
		if (TimerUpdate())
		{
			Mgr(TaskTimerMgr)->ReserveAsyncTask(m_tickInterval, &m_timerEvent);
		}
		else
		{
			reset_cache_shared();
			m_timerEvent.ReleaseIocpObject();
		}
	}
}
