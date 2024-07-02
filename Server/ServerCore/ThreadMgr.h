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

	//extern thread_local moodycamel::ProducerToken* LPro_token;
	//extern thread_local moodycamel::ConsumerToken* LCon_token;
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
		static constexpr const uint64 NUM_OF_THREADS = ServerCore::NUM_OF_THREADS;
		void Launch(const std::shared_ptr<Service> pService);
		static c_uint32 GetCurThreadID()noexcept { return LThreadId; }
		static c_uint32 GetCurThreadIdx()noexcept { return LThreadId - 1; }
		const bool IsServerFinish()const noexcept { return m_bStopRequest; }
		void NotifyThread()const noexcept { PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0); }
		const HANDLE GetIocpHandle()const noexcept { return m_iocpHandle; }
	public:
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...)noexcept, const S_ptr<U>& memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...)noexcept, S_ptr<U>&& memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires EnableSharedFromThis<U>&& std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...)noexcept, U* const memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...), const S_ptr<U>& memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...), S_ptr<U>&& memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename T, typename U, typename Ret, typename... Args> requires EnableSharedFromThis<U>&& std::derived_from<U, T>
		void EnqueueGlobalTask(Ret(T::* const memFunc)(Args...), U* const memFuncInstance, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(memFunc, memFuncInstance, std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename Func, typename... Args> requires std::invocable<Func, Args...>&& IsNotMemFunc<Func>
		void EnqueueGlobalTask(Func&& fp, Args&&... args)noexcept
		{
			m_globalTask.enqueue(*LPro_tokenGlobalTask, Task(std::forward<Func>(fp), std::forward<Args>(args)...));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		void EnqueueGlobalTask(Task&& task_)noexcept {
			m_globalTask.enqueue(*LPro_tokenGlobalTask, std::move(task_));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		void EnqueueGlobalTaskBulk(Task* const taskBulks, const std::size_t num_of_task)noexcept {
			m_globalTask.enqueue_bulk(*LPro_tokenGlobalTask, taskBulks, num_of_task);
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		}
		template<typename Func, typename... Args>
			requires std::invocable<Func, Args...>
		std::future<std::invoke_result_t<Func, Args...>> EnqueueGlobalTaskFuture(Func&& fp, Args&&... args) noexcept
		{
			using return_type = std::invoke_result_t<Func, Args...>;
			auto task = ::MakeUnique<std::packaged_task<return_type(void)>>(std::bind_front(std::forward<Func>(fp), std::forward<Args>(args)...));
			std::future<return_type> res_future = task->get_future();
			EnqueueGlobalTask(Task(([task = std::move(task)]() noexcept {(*task)(); })));
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
			return res_future;
		}
		Service* const GetMainService()noexcept { return m_pMainService; }
		const bool& GetStopFlagRef()const noexcept { return m_bStopRequest; }
	public:
		void InitTLS();
		void DestroyTLS();
		void Join();
		void TryGlobalQueueTask()noexcept;
	private:
		const HANDLE m_iocpHandle;
		Service* m_pMainService;
		moodycamel::ConcurrentQueue<Task, LFQueueAllocator> m_globalTask{ 32 };
		bool m_bStopRequest = false;
		std::vector<std::jthread>	m_threads;
		std::jthread m_timerThread;
		//SpinLock m_heartBeatFuncLock;
		static inline Atomic<uint32> g_threadID = 0;

		//moodycamel::ConcurrentQueue<S_ptr<TaskQueueable>, LFQueueAllocator> m_globalTaskQueue{ 32 };
		

		enum { WORKER_TICK = 64 };
	};
}

