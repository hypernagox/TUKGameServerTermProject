#include "ServerCorePch.h"
#include "TaskQueueable.h"

namespace ServerCore
{
	extern thread_local uint64				LEndTickCount;
	extern thread_local class TaskQueueable* LCurTaskQueue;

	TaskQueueable::TaskQueueable()
	{
		m_taskEvent.SetIocpObject(SharedFromThis<IocpObject>());
	}

	TaskQueueable::~TaskQueueable()noexcept
	{
	}

	void TaskQueueable::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
	{
		Execute();
	}

	void TaskQueueable::EnqueueAsyncTask(Task&& task_, const bool pushOnly)noexcept
	{
		// 동적으로 생겼다가 사라질 수 있다면, 사라질때 절대 여기로 들어오지 못하게 해야한다.

		const int32 prevCount = m_taskCount.fetch_add(1, std::memory_order_relaxed);
		m_taskQueue.emplace(std::move(task_));
		if (0 == prevCount)
		{
			std::atomic_thread_fence(std::memory_order_acquire);
			// TODO: 타고타고 여기 들어올 수가 있나?
			// 이미 실행중인 JobQueue가 없으면 실행
			if (nullptr == LCurTaskQueue && false == pushOnly)
			{
				Execute();
			}
			else
			{
				// 여유 있는 다른 쓰레드가 실행하도록 GlobalQueue에 넘긴다
				//Mgr(ThreadMgr)->PushGlobalQueue(shared_from_this());
				::PostQueuedCompletionStatus(Mgr(ThreadMgr)->GetIocpHandle(), 0, 0, &m_taskEvent);
			}
		}
	}

	void TaskQueueable::Execute()noexcept
	{
		LCurTaskQueue = this;
		thread_local Vector<Task> taskVec;
		for (;;)
		{
			m_taskQueue.try_flush_single(taskVec);

			const int32 taskCount = static_cast<c_int32>(taskVec.size());
			for (const auto& task : taskVec)task.ExecuteTask();
			taskVec.clear();
			// 남은 일감이 0개라면 종료
			if (m_taskCount.fetch_sub(taskCount, std::memory_order_release) == taskCount)
			{
				break;
			}

			// TODO: 굳이 다른 스레드한테 넘겨야 돼나?
			if (::GetTickCount64() >= LEndTickCount)
			{
				// 여유 있는 다른 쓰레드가 실행하도록 GlobalQueue에 넘긴다
				//Mgr(ThreadMgr)->PushGlobalQueue(shared_from_this());
				::PostQueuedCompletionStatus(Mgr(ThreadMgr)->GetIocpHandle(), 0, 0, &m_taskEvent);
				break;
			}
		}
		LCurTaskQueue = nullptr;
	}
}