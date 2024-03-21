#pragma once
#include "CoRoutine.hpp"
#include "ThreadMgr.h"

namespace ServerCore
{
	class Task;
	class TaskQueueable;

	struct TimerTask
	{
		uint64 executeTime = 0;
		mutable U_Pptr<Task> taskPtr = nullptr;
		constexpr TimerTask()noexcept = default;
		TimerTask(const TimerTask& other)noexcept :executeTime{ other.executeTime }, taskPtr{ std::move(other.taskPtr) } {}
		constexpr TimerTask& operator=(const TimerTask& other)noexcept
		{
			if (&other != this)
			{
				executeTime = other.executeTime;
				taskPtr = std::move(other.taskPtr);
			}
			return *this;
		}
		TimerTask(c_uint64 tickCount, U_Pptr<Task>&& task_)noexcept :executeTime{ tickCount }, taskPtr{ std::move(task_) } {}
	};

	struct TimerCompare
	{
		const bool operator () (const TimerTask& a, const TimerTask& b) const noexcept { return a.executeTime > b.executeTime; }
	};

	class TaskTimerMgr
		:public Singleton<TaskTimerMgr>
	{
		friend class Singleton;
		TaskTimerMgr();
		~TaskTimerMgr();
	public:
		void ReserveAsyncTask(c_uint64 tickAfter, S_ptr<TaskQueueable>&& memfuncInstance, U_Pptr<Task>&& task)noexcept;
		template<typename Func, typename... Args> requires std::invocable<Func, Args...>
		void ReserveAsyncTask(c_uint64 tickAfter, Func&& fp, Args&&... args)noexcept
		{
			m_timerTaskQueue.push(
				TimerTask(::GetTickCount64() + tickAfter, MakePoolUnique<Task>([task = MakePoolUnique<Task>(std::forward<Func>(fp), std::forward<Args>(args)...)]()mutable noexcept
					{
						Mgr(ThreadMgr)->EnqueueGlobalTask(task.release());
					})
				));
		}
		void DistributeTask()noexcept;
	private:
		Concurrency::concurrent_priority_queue<TimerTask, TimerCompare, StlAllocator<TimerTask>> m_timerTaskQueue;
		std::atomic_bool m_bIsDistribute = false;
	};
}