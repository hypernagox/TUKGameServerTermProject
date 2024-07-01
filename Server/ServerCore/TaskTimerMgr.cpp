#include "ServerCorePch.h"
#include "TaskTimerMgr.h"
#include "TaskQueueable.h"
#include "ThreadMgr.h"
#include "IocpEvent.h"

namespace ServerCore
{
	TaskTimerMgr::TaskTimerMgr()
	{
		for (int i = 0; i < 1024; ++i)
		{
			m_timerTaskQueue.emplace(TimerTask{});
		}
		m_timerTaskQueue.clear();
	}

	TaskTimerMgr::~TaskTimerMgr()
	{
	}

	void TaskTimerMgr::ReserveAsyncTask(c_uint64 tickAfter, S_ptr<TaskQueueable>&& memfuncInstance, Task&& task)noexcept
	{
		m_timerTaskQueue.emplace(
			TimerTask(::GetTickCount64() + tickAfter, Task
			([memfuncInstance = std::move(memfuncInstance), task = std::move(task)]()mutable noexcept
				{
					memfuncInstance->EnqueueAsyncTask(std::move(task), true);
				}
		)));
	}

	void TaskTimerMgr::ReserveAsyncTask(c_uint64 tickAfter, IocpEvent* const pTimerEvent_) noexcept
	{
		m_timerTaskQueue.emplace(
			TimerTask(::GetTickCount64() + tickAfter, Task
			([pTimerEvent_]()noexcept
				{
					::PostQueuedCompletionStatus(Mgr(ThreadMgr)->GetIocpHandle(), 0, 0, pTimerEvent_);
				}
		)));
	}

	void TaskTimerMgr::DistributeTask()noexcept
	{
		TimerTask task;
		while (m_timerTaskQueue.try_pop(task))
		{
			if (::GetTickCount64() < task.executeTime)
			{
				m_timerTaskQueue.emplace(std::move(task));
				return;
			}
			else
			{
				task.taskPtr.ExecuteTask();
				std::destroy_at<TimerTask>(&task);
			}
		}
	}
}