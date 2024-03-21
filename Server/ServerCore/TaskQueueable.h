#pragma once
#include "Task.h"
#include "TaskTimerMgr.h"
#include "ThreadMgr.h"
#include "enable_shared_cache_this.hpp"

namespace ServerCore
{
	class TaskQueueable
		:public enable_shared_cache_this<TaskQueueable>
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
			EnqueueAsyncTask(MakePoolUnique<Task>(memFunc, std::move(ptr), std::forward<Args>(args)...));
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...)noexcept, const S_ptr<U>& ptr, Args&&... args)noexcept
		{
			EnqueueAsyncTask(MakePoolUnique<Task>(memFunc, ptr, std::forward<Args>(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...)noexcept, Args&&... args)noexcept
		{
			EnqueueAsyncTask(MakePoolUnique<Task>(memFunc, this->SharedCastThis<T>(), std::forward<Args>(args)...));
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U,T>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...), S_ptr<U>&& ptr, Args&&... args)noexcept
		{
			EnqueueAsyncTask(MakePoolUnique<Task>(memFunc, std::move(ptr), std::forward<Args>(args)...));
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...), const S_ptr<U>& ptr, Args&&... args)noexcept
		{
			EnqueueAsyncTask(MakePoolUnique<Task>(memFunc, ptr, std::forward<Args>(args)...));
		}
		template<typename Func, typename... Args> requires std::invocable<Func, Args...>&& IsNotMemFunc<Func>
		void EnqueueAsync(Func&& fp, Args&&... args)noexcept
		{
			EnqueueAsyncTask(MakePoolUnique<Task>(std::forward<Func>(fp), std::forward<Args>(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void EnqueueAsync(Ret(T::* const memFunc)(Args...), Args&&... args)noexcept
		{
			EnqueueAsyncTask(MakePoolUnique<Task>(memFunc, this->SharedCastThis<T>(), std::forward<Args>(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void EnqueueAsyncTimer(c_uint64 tickAfter, Ret(T::* const memFunc)(Args...)noexcept, Args&&... args)noexcept
		{
			auto memFuncInstance = this->SharedCastThis<T>();
			Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfter, std::move(memFuncInstance),
				MakePoolUnique<Task>(memFunc, memFuncInstance, std::forward<Args>(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void EnqueueAsyncTimer(c_uint64 tickAfter, Ret(T::* const memFunc)(Args...), Args&&... args)noexcept
		{
			auto memFuncInstance = this->SharedCastThis<T>();
			Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfter, std::move(memFuncInstance),
				MakePoolUnique<Task>(memFunc, memFuncInstance, std::forward<Args>(args)...));
		}
		template<typename Func, typename... Args> requires std::invocable<Func, Args...>
		void EnqueueAsyncTimer(c_uint64 tickAfter, Func&& fp, Args&&... args)noexcept
		{
			Mgr(TaskTimerMgr)->ReserveAsyncTask(tickAfter, shared_from_this(), MakePoolUnique<Task>(std::forward<Func>(fp), std::forward<Args>(args)...));
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void PushAsyncGlobalQueue(Ret(T::* const memFunc)(Args...)noexcept, Args&&... args)noexcept
		{
			Mgr(ThreadMgr)->EnqueueGlobalTask(memFunc, this->SharedCastThis<T>(), std::forward<Args>(args)...);
		}
		template<typename T, typename Ret, typename... Args> requires std::derived_from<T, TaskQueueable>
		void PushAsyncGlobalQueue(Ret(T::* const memFunc)(Args...), Args&&... args)noexcept
		{
			Mgr(ThreadMgr)->EnqueueGlobalTask(memFunc, this->SharedCastThis<T>(), std::forward<Args>(args)...);
		}
		void StartExecute()noexcept { register_cache_shared(); }
		void StopExecute()noexcept { clear(); }
	private:
		void	EnqueueAsyncTask(U_Pptr<Task>&& task_, const bool pushOnly = false)noexcept;
		void	Execute()noexcept;
		void	clear()noexcept { m_taskQueue.clear_single(); m_taskVec.clear(); reset_cache_shared(); }
	private:
		MPSCQueue<U_Pptr<Task>> m_taskQueue;
		Vector<U_Pptr<Task>> m_taskVec;
		std::atomic<int32> m_taskCount = 0;
	};
}
