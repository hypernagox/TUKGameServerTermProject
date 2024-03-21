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

	thread_local moodycamel::ProducerToken* LPro_token;
	thread_local moodycamel::ConsumerToken* LCon_token;
	thread_local moodycamel::ProducerToken* LPro_tokenGlobalTask;
	thread_local moodycamel::ConsumerToken* LCon_tokenGlobalTask;

	ThreadMgr::ThreadMgr()
	{
		// Main Thread
		InitTLS();
	}

	ThreadMgr::~ThreadMgr()
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);
		if (!m_bStopRequest)
		{
			Join();
		}
		Task* task;
		while (m_globalTask.try_dequeue(*LCon_tokenGlobalTask, task))PoolDelete<Task>(task);

		xdelete<moodycamel::ProducerToken>(LPro_token);
		xdelete<moodycamel::ConsumerToken>(LCon_token);

		xdelete<moodycamel::ProducerToken>(LPro_tokenGlobalTask);
		xdelete<moodycamel::ConsumerToken>(LCon_tokenGlobalTask);
	}

	void ThreadMgr::Launch(S_ptr<Service> pService)
	{
		for (int i = 0; i < NUM_OF_THREADS; ++i)
		{
			m_threads.emplace_back([this, pService = pService.get()]()noexcept
				{
					InitTLS();
					const bool& bStopRequest = m_bStopRequest;
					const auto pIocpCore = pService->GetIocpCore().get();
					const auto taskTimer = Mgr(TaskTimerMgr);
					const auto threadMgr = Mgr(ThreadMgr);
					for (;;)
					{
						if (bStopRequest) [[unlikely]]
							break;

						LEndTickCount = ::GetTickCount64() + WORKER_TICK;

						pIocpCore->Dispatch(10);

						taskTimer->DistributeTask();

						threadMgr->TryGlobalQueueTask();
					}
					DestroyTLS();
				});
		}
		if (SERVICE_TYPE::SERVER == pService->GetServiceType())
		{
			static std::atomic_bool registerFinish = false;
			while (!m_bStopRequest)
			{
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
		for (auto& t : m_threads)
		{
			t.join();
		}
	}

	void ThreadMgr::InitTLS()
	{
		LThreadId = g_threadID.fetch_add(1);

		LPro_token = xnew<moodycamel::ProducerToken>(m_globalTaskQueue);
		LPro_tokenGlobalTask = xnew <moodycamel::ProducerToken>(m_globalTask);

		LCon_token = xnew <moodycamel::ConsumerToken>(m_globalTaskQueue);
		LCon_tokenGlobalTask = xnew <moodycamel::ConsumerToken>(m_globalTask);

		LSendBufferChunk = Mgr(SendBufferMgr)->Pop();
	}

	void ThreadMgr::DestroyTLS()
	{
	}

	void ThreadMgr::TryGlobalQueueTask()noexcept
	{
		while (::GetTickCount64() < LEndTickCount)
		{
			S_ptr<TaskQueueable> qPtr;
			if (!m_globalTaskQueue.try_dequeue(*LCon_token, qPtr))
				break;
			qPtr->Execute();
		}
		Task* task;
		while (m_globalTask.try_dequeue(*LCon_tokenGlobalTask, task)) {
			task->ExecuteTask();
			PoolDelete<Task>(task);
		}
	}
}