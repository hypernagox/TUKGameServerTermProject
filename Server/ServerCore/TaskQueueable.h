#pragma once
#include "Task.h"
#include "TaskTimerMgr.h"
#include "ThreadMgr.h"
#include "IocpObject.h"
#include "IocpEvent.h"

namespace ServerCore
{
	class TaskQueueable
		:public IocpObject
	{
		friend class ThreadMgr;
		friend class TaskTimerMgr;
	public:
		TaskQueueable();
		virtual ~TaskQueueable()noexcept;
	public:
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...)noexcept, S_ptr<U>&& ptr, Args&&... args)noexcept
		{
			EnqueueAsyncTask(Task(memFunc, std::move(ptr), std::forward<Args>(args)...));
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...)noexcept, const S_ptr<U>& ptr, Args&&... args)noexcept
		{
			EnqueueAsyncTask(Task(memFunc, ptr, std::forward<Args>(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...)noexcept, Args&&... args)noexcept
		{
			EnqueueAsyncTask(Task(memFunc, S_ptr<T>{this}, std::forward<Args>(args)...));
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...), S_ptr<U>&& ptr, Args&&... args)noexcept
		{
			EnqueueAsyncTask(Task(memFunc, std::move(ptr), std::forward<Args>(args)...));
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...), const S_ptr<U>& ptr, Args&&... args)noexcept
		{
			EnqueueAsyncTask(Task(memFunc, ptr, std::forward<Args>(args)...));
		}
		template<typename Func, typename... Args> requires std::invocable<Func, Args...>&& IsNotMemFunc<Func>
		void EnqueueAsync(Func&& fp, Args&&... args)noexcept
		{
			EnqueueAsyncTask(Task(std::forward<Func>(fp), std::forward<Args>(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...), Args&&... args)noexcept
		{
			EnqueueAsyncTask(Task(memFunc, S_ptr<T>{this}, std::forward<Args>(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void EnqueueAsyncTimer(c_uint64 tickAfter, Ret(T::* const memFunc)(Args...)noexcept, Args&&... args)noexcept
		{
			auto memFuncInstance = S_ptr<T>{ this };
			Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfter, std::move(memFuncInstance),
				Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void EnqueueAsyncTimer(c_uint64 tickAfter, Ret(T::* const memFunc)(Args...), Args&&... args)noexcept
		{
			auto memFuncInstance = S_ptr<T>{ this };
			Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfter, std::move(memFuncInstance),
				Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
		}
		template<typename Func, typename... Args> requires std::invocable<Func, Args...>
		void EnqueueAsyncTimer(c_uint64 tickAfter, Func&& fp, Args&&... args)noexcept
		{
			Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfter, S_ptr<TaskQueueable>{this}, Task(std::forward<Func>(fp), std::forward<Args>(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void PushAsyncGlobalQueue(Ret(T::* const memFunc)(Args...)noexcept, Args&&... args)noexcept
		{
			Mgr(ThreadMgr)->EnqueueGlobalTask(memFunc, S_ptr<T>{this}, std::forward<Args>(args)...);
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void PushAsyncGlobalQueue(Ret(T::* const memFunc)(Args...), Args&&... args)noexcept
		{
			Mgr(ThreadMgr)->EnqueueGlobalTask(memFunc, S_ptr<T>{this}, std::forward<Args>(args)...);
		}
		void StopExecute()noexcept {
			if (true == m_bIsValid.exchange(false, std::memory_order_relaxed))
				EnqueueAsync(&TaskQueueable::clear);
		}
	public:
		virtual const bool IsValid()const noexcept override { return m_bIsValid.load(std::memory_order_relaxed); }
		virtual HANDLE GetHandle()const noexcept override { return nullptr; }
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;
	private:
		void	EnqueueAsyncTask(Task&& task_, const bool pushOnly = false)noexcept;
		void	Execute()noexcept;
		void	clear()noexcept { m_taskQueue.clear_single(); m_taskEvent.ReleaseIocpObject(); }
	private:
		MPSCQueue<Task> m_taskQueue;
		std::atomic<int32> m_taskCount = 0;
		IocpEvent m_taskEvent{ EVENT_TYPE::TASK };
		std::atomic_bool m_bIsValid = true;
	};
}
