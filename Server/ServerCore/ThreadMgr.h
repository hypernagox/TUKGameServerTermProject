#pragma once
#include "Task.h"

/*------------------
	ThreadMgr
-------------------*/

namespace ServerCore
{
	class TaskQueueable;
	class Service;
	class Task;

	extern thread_local moodycamel::ProducerToken* LPro_token;
	extern thread_local moodycamel::ConsumerToken* LCon_token;
	extern thread_local moodycamel::ProducerToken* LPro_tokenGlobalTask;
	extern thread_local moodycamel::ConsumerToken* LCon_tokenGlobalTask;

	class ThreadMgr
		:public Singleton<ThreadMgr>
	{
		friend class Singleton;
		ThreadMgr();
		~ThreadMgr();
		struct LFQueueAllocator
			:public moodycamel::ConcurrentQueueDefaultTraits
		{
			static inline void* const malloc(const size_t size)noexcept { return Mgr(MemoryMgr)->Allocate(size); }
			static inline void free(void* const ptr)noexcept { Mgr(MemoryMgr)->Release(ptr); }
		};
	public:
		static constexpr uint64 NUM_OF_THREADS = 6;
		void Launch(S_ptr<Service> pService);
		c_uint32 GetCurThreadID()const noexcept { return LThreadId; }
		const bool IsServerFinish()const noexcept { return m_bStopRequest; }
	public:
		void PushGlobalQueue(S_ptr<TaskQueueable>&& qPtr_)noexcept
		{
			m_globalTaskQueue.enqueue(*LPro_token, std::move(qPtr_));
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...)noexcept, const S_ptr<U>& memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, PoolNew<Task>(memFunc, memFuncInstance, std::forward<Args>(args)...));
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...)noexcept, S_ptr<U>&& memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, PoolNew<Task>(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...));
		}
		template<typename T, typename U, typename Ret, typename... Args> requires EnableSharedFromThis<U>&& std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...)noexcept, U* const memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, PoolNew<Task>(memFunc, memFuncInstance, std::forward<Args>(args)...));
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U,T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...), const S_ptr<U>& memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, PoolNew<Task>(memFunc, memFuncInstance, std::forward<Args>(args)...));
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...), S_ptr<U>&& memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, PoolNew<Task>(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...));
		}
		template<typename T, typename U, typename Ret, typename... Args> requires EnableSharedFromThis<U> && std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...), U* const memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, PoolNew<Task>(memFunc, memFuncInstance, std::forward<Args>(args)...));
		}
		template<typename Func, typename... Args> requires std::invocable<Func, Args...>&& IsNotMemFunc<Func>
		void EnqueueGlobalTask(Func&& fp, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, PoolNew<Task>(std::forward<Func>(fp), std::forward<Args>(args)...));
		}
		void EnqueueGlobalTask(Task* const task_)noexcept {
			m_globalTask.enqueue(*LPro_tokenGlobalTask, task_);
		}
		void EnqueueGlobalTaskBulk(Task** const taskBulks, const std::size_t num_of_task)noexcept {
			m_globalTask.enqueue_bulk(*LPro_tokenGlobalTask, taskBulks, num_of_task);
		}
		template<typename Func, typename... Args>
			requires std::invocable<Func, Args...>
		std::future<std::invoke_result_t<Func, Args...>> EnqueueGlobalTaskFuture(Func&& fp, Args&&... args) noexcept
		{
			using return_type = std::invoke_result_t<Func, Args...>;
			auto task = ::MakeUnique<std::packaged_task<return_type(void)>>(std::bind_front(std::forward<Func>(fp), std::forward<Args>(args)...));
			std::future<return_type> res_future = task->get_future();
			EnqueueGlobalTask(PoolNew<Task>(([task = std::move(task)]() noexcept {(*task)(); })));
			return res_future;
		}
	private:
		void InitTLS();
		void DestroyTLS();
		void Join();
		void TryGlobalQueueTask()noexcept;
	private:
		bool m_bStopRequest = false;
		Vector<std::jthread>	m_threads;
		SpinLock m_heartBeatFuncLock;
		static inline Atomic<uint32> g_threadID = 1;

		moodycamel::ConcurrentQueue<S_ptr<TaskQueueable>, LFQueueAllocator> m_globalTaskQueue{ 32 };
		moodycamel::ConcurrentQueue<Task*, LFQueueAllocator> m_globalTask{ 1024 };

		enum { WORKER_TICK = 64 };
	};
}

