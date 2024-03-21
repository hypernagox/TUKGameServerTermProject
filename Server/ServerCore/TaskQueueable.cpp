#include "ServerCorePch.h"
#include "TaskQueueable.h"

namespace ServerCore
{
	extern thread_local uint64				LEndTickCount;
	extern thread_local class TaskQueueable* LCurTaskQueue;

	TaskQueueable::TaskQueueable()
	{
	}

	TaskQueueable::~TaskQueueable()noexcept
	{
	}

	void TaskQueueable::EnqueueAsyncTask(U_Pptr<Task>&& task_, const bool pushOnly)noexcept
	{
		const int32 prevCount = m_taskCount.fetch_add(1, std::memory_order_acq_rel);
		m_taskQueue.emplace(std::move(task_));
		if (0 == prevCount)
		{
			// TODO: 타고타고 여기 들어올 수가 있나?
			// 이미 실행중인 JobQueue가 없으면 실행
			if (nullptr == LCurTaskQueue && false == pushOnly)
			{
				Execute();
			}
			else
			{
				// 여유 있는 다른 쓰레드가 실행하도록 GlobalQueue에 넘긴다
				Mgr(ThreadMgr)->PushGlobalQueue(shared_from_this());
			}
		}
	}

	void TaskQueueable::Execute()noexcept
	{
		LCurTaskQueue = this;
		for (;;)
		{
			m_taskQueue.try_flush_single(m_taskVec);

			const int32 taskCount = static_cast<int32>(m_taskVec.size());
			for (const auto& task : m_taskVec)task->ExecuteTask();
			m_taskVec.clear();
			// 남은 일감이 0개라면 종료
			if (m_taskCount.fetch_sub(taskCount, std::memory_order_acq_rel) == taskCount)
			{
				break;
			}

			// TODO: 굳이 다른 스레드한테 넘겨야 돼나?
			if (::GetTickCount64() >= LEndTickCount)
			{
				// 여유 있는 다른 쓰레드가 실행하도록 GlobalQueue에 넘긴다
				Mgr(ThreadMgr)->PushGlobalQueue(shared_from_this());
				break;
			}
		}
		LCurTaskQueue = nullptr;
	}
}