#include "ServerCorePch.h"
#include "TaskTimerMgr.h"
#include "TaskQueueable.h"
#include "ThreadMgr.h"

namespace ServerCore
{
	TaskTimerMgr::TaskTimerMgr()
	{
	}

	TaskTimerMgr::~TaskTimerMgr()
	{
	}

	void TaskTimerMgr::ReserveAsyncTask(c_uint64 tickAfter, S_ptr<TaskQueueable>&& memfuncInstance, U_Pptr<Task>&& task)noexcept
	{
		m_timerTaskQueue.push(
			TimerTask(::GetTickCount64() + tickAfter, MakePoolUnique<Task>
				([memfuncInstance = std::move(memfuncInstance), task = std::move(task)]()mutable noexcept
					{
						if (1 == memfuncInstance.use_count())
							return;
						memfuncInstance->EnqueueAsyncTask(std::move(task), true);
					}
		)));
	}

	void TaskTimerMgr::DistributeTask()noexcept
	{
		if (true == m_bIsDistribute.exchange(true, std::memory_order_acq_rel))
			return;

		TimerTask task;
		while (m_timerTaskQueue.try_pop(task))
		{
			if (::GetTickCount64() < task.executeTime)
			{
				m_timerTaskQueue.push(task);
				break;
			}
			else
			{
				task.taskPtr->ExecuteTask();
			}
		}

		m_bIsDistribute.store(false, std::memory_order_release);
	}
}