#include "ServerCorePch.h"
#include "ThreadMgr.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"
#include "TaskQueueable.h"
#include "Service.h"
#include "IocpCore.h"
#include "TaskTimerMgr.h"
#include "SendBufferMgr.h"

/*------------------
	ThreadMgr
-------------------*/

namespace ServerCore
{
	extern thread_local uint64				LEndTickCount;
	extern thread_local class TaskQueueable* LCurTaskQueue;

	//thread_local moodycamel::ProducerToken* LPro_token;
	//thread_local moodycamel::ConsumerToken* LCon_token;
	thread_local moodycamel::ProducerToken* LPro_tokenGlobalTask;
	thread_local moodycamel::ConsumerToken* LCon_tokenGlobalTask;

	ThreadMgr::ThreadMgr()
		:m_iocpHandle{ Mgr(CoreGlobal)->GetIocpCore()->GetIocpHandle() }
	{
		// Main Thread
		InitTLS();
		LThreadId = 1;
	}

	ThreadMgr::~ThreadMgr()
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);
		if (!m_bStopRequest)
		{
			Join();
		}
		Task task;
		while (m_globalTask.try_dequeue(*LCon_tokenGlobalTask, task)) { std::destroy_at<Task>(&task); }

		//xdelete<moodycamel::ProducerToken>(LPro_token);
		//xdelete<moodycamel::ConsumerToken>(LCon_token);

		xdelete<moodycamel::ProducerToken>(LPro_tokenGlobalTask);
		xdelete<moodycamel::ConsumerToken>(LCon_tokenGlobalTask);
	}

	void ThreadMgr::Launch(S_ptr<Service> pService)
	{
		m_pMainService = pService.get();
		for (int i = 0; i < NUM_OF_THREADS; ++i)
		{
			m_threads.emplace_back([this, pService = pService.get()]()noexcept
				{
					InitTLS();
					const bool& bStopRequest = m_bStopRequest;
					const auto pIocpCore = pService->GetIocpCore().get();
					const auto taskTimer = Mgr(TaskTimerMgr);
					for (;;)
					{
						if (bStopRequest) [[unlikely]]
							break;

						if (false == pIocpCore->Dispatch(INFINITE))
						{
							this->TryGlobalQueueTask();
						}
					}

					DestroyTLS();
				});
		}

		while (g_threadID.load(std::memory_order_seq_cst) <= NUM_OF_THREADS);
		std::atomic_thread_fence(std::memory_order_seq_cst);

		m_timerThread = std::jthread{ [this]()noexcept
			{
				InitTLS();
				const bool& bStopRequest = m_bStopRequest;
				const auto taskTimer = Mgr(TaskTimerMgr);
				for (;;)
				{
					if (bStopRequest) [[unlikely]]
						break;

					taskTimer->DistributeTask();

					std::this_thread::yield();
				}
				DestroyTLS();
			} };
		std::string strFin(32, 0);
		if (SERVICE_TYPE::SERVER == pService->GetServiceType())
		{
			static std::atomic_bool registerFinish = false;
			while (!m_bStopRequest)
			{
				std::cin >> strFin;

				if ("EXIT" == strFin)
				{
					if (false == registerFinish.exchange(true))
					{
						pService->CloseService();
						Mgr(Logger)->m_bStopRequest = true;
						std::this_thread::sleep_for(std::chrono::seconds(5));
						Join();
					}
				}
				std::this_thread::sleep_for(std::chrono::seconds(5));
				if (::GetAsyncKeyState(VK_END))
				{
					if (false == registerFinish.exchange(true))
					{
						pService->CloseService();
						Mgr(Logger)->m_bStopRequest = true;
						std::this_thread::sleep_for(std::chrono::seconds(5));
						Join();
					}
				}
			}
		}
	}

	void ThreadMgr::Join()
	{
		if (m_bStopRequest)
			return;
		m_bStopRequest = true;
		std::atomic_thread_fence(std::memory_order_seq_cst);
		for (int i = 0; i < NUM_OF_THREADS; ++i)
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
		for (auto& t : m_threads)
		{
			PostQueuedCompletionStatus(m_iocpHandle, 0, 0, 0);
			t.join();
		}
		m_timerThread.join();
	}

	void ThreadMgr::InitTLS()
	{
		LThreadId = g_threadID.fetch_add(1);

		//LPro_token = xnew<moodycamel::ProducerToken>(m_globalTaskQueue);
		LPro_tokenGlobalTask = xnew <moodycamel::ProducerToken>(m_globalTask);

		//LCon_token = xnew <moodycamel::ConsumerToken>(m_globalTaskQueue);
		LCon_tokenGlobalTask = xnew <moodycamel::ConsumerToken>(m_globalTask);

		if (NUM_OF_THREADS >= LThreadId)
			LSendBufferChunk = Mgr(SendBufferMgr)->Pop();

		const volatile auto init_rand_seed = LRandSeed;
	}

	void ThreadMgr::DestroyTLS()
	{
	}

	void ThreadMgr::TryGlobalQueueTask()noexcept
	{
		Task task;
		while (m_globalTask.try_dequeue(*LCon_tokenGlobalTask, task)) {
			task.ExecuteTask();
			std::destroy_at<Task>(&task);
		}
	}
}