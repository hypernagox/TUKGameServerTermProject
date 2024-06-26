#include "ServerCorePch.h"
#include "TimerObject.h"
#include "TaskTimerMgr.h"

namespace ServerCore
{
	void TimerObject::InitTimer(const uint64 tick_ms) noexcept
	{
		// �ʱ�ȭ�� �ʿ��� �߰������� ���⿡

		m_timerEvent.SetIocpObject(SharedFromThis<IocpObject>());
		m_tickInterval = tick_ms;

	}

	const bool TimerObject::ExecuteTimer(const IocpEntity* const awaker)noexcept
	{
		if (!CanAwake(awaker))
			return false;
		ToAwaker(awaker);
		const TIMER_STATE ePrevState = m_timer_state.exchange(TIMER_STATE::RUN, std::memory_order_relaxed);
		if (TIMER_STATE::IDLE == ePrevState)
		{
			if (true == m_bStopFlag.load(std::memory_order_relaxed))
				return false;
			::PostQueuedCompletionStatus(Mgr(ThreadMgr)->GetIocpHandle(), 0, 0, &m_timerEvent);
			return true;
		}
		else if (TIMER_STATE::PREPARE == ePrevState)
		{
			Mgr(TaskTimerMgr)->ReserveAsyncTask(m_tickInterval, &m_timerEvent);
			return true;
		}
		return false;
	}

	void TimerObject::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
	{
		const TIMER_STATE eCurState = TimerUpdate();
		std::atomic_thread_fence(std::memory_order_seq_cst);
		if (true == m_bStopFlag.load(std::memory_order_relaxed))
		{
			m_timer_state = eCurState;
			std::atomic_thread_fence(std::memory_order_acquire);
			m_timerEvent.ReleaseIocpObject();
			return;
		}

		m_timer_state.store(TIMER_STATE::PREPARE, std::memory_order_relaxed);
		const TIMER_STATE ePrevState = m_timer_state.exchange(eCurState, std::memory_order_relaxed);
		if (TIMER_STATE::RUN == eCurState && TIMER_STATE::PREPARE == ePrevState)
		{
			Mgr(TaskTimerMgr)->ReserveAsyncTask(m_tickInterval, &m_timerEvent);
		}
	}
}
