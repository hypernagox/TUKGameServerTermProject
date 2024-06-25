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
		// �������� ����ٰ� ����� �� �ִٸ�, ������� ���� ����� ������ ���ϰ� �ؾ��Ѵ�.

		const int32 prevCount = m_taskCount.fetch_add(1, std::memory_order_relaxed);
		m_taskQueue.emplace(std::move(task_));
		if (0 == prevCount)
		{
			std::atomic_thread_fence(std::memory_order_acquire);
			// TODO: Ÿ��Ÿ�� ���� ���� ���� �ֳ�?
			// �̹� �������� JobQueue�� ������ ����
			if (nullptr == LCurTaskQueue && false == pushOnly)
			{
				Execute();
			}
			else
			{
				// ���� �ִ� �ٸ� �����尡 �����ϵ��� GlobalQueue�� �ѱ��
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
			// ���� �ϰ��� 0����� ����
			if (m_taskCount.fetch_sub(taskCount, std::memory_order_release) == taskCount)
			{
				break;
			}

			// TODO: ���� �ٸ� ���������� �Ѱܾ� �ų�?
			if (::GetTickCount64() >= LEndTickCount)
			{
				// ���� �ִ� �ٸ� �����尡 �����ϵ��� GlobalQueue�� �ѱ��
				//Mgr(ThreadMgr)->PushGlobalQueue(shared_from_this());
				::PostQueuedCompletionStatus(Mgr(ThreadMgr)->GetIocpHandle(), 0, 0, &m_taskEvent);
				break;
			}
		}
		LCurTaskQueue = nullptr;
	}
}