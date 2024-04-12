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
		void StartExecute(const S_ptr<TaskQueueable>& forCacheThis_)noexcept {
			register_cache_shared_core(forCacheThis_);
			m_taskEvent.SetIocpObject(shared_from_this());
		}
		void StopExecute()noexcept {
			if (true == m_bIsValid.exchange(false, std::memory_order_relaxed))
				EnqueueAsync(&TaskQueueable::clear);
		}
	public:
		virtual HANDLE GetHandle()const noexcept override { return nullptr; }
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;
	private:
		void	EnqueueAsyncTask(U_Pptr<Task>&& task_, const bool pushOnly = false)noexcept;
		void	Execute()noexcept;
		void	clear()noexcept { m_taskEvent.ReleaseIocpObject(); m_taskQueue.clear_single(); reset_cache_shared(); }
	private:
		MPSCQueue<U_Pptr<Task>> m_taskQueue;
		Vector<U_Pptr<Task>> m_taskVec;
		std::atomic<int32> m_taskCount = 0;
		IocpEvent m_taskEvent{ EVENT_TYPE::TASK };
		std::atomic_bool m_bIsValid = true;
	};
}
