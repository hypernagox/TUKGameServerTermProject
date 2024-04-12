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

	void TaskQueueable::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
	{
		Execute();
	}

	void TaskQueueable::EnqueueAsyncTask(U_Pptr<Task>&& task_, const bool pushOnly)noexcept
	{
		// �������� ����ٰ� ����� �� �ִٸ�, ������� ���� ����� ������ ���ϰ� �ؾ��Ѵ�.

		const int32 prevCount = m_taskCount.fetch_add(1, std::memory_order_acquire);
		m_taskQueue.emplace(std::move(task_));
		if (0 == prevCount)
		{
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
		for (;;)
		{
			m_taskQueue.try_flush_single(m_taskVec);

			const int32 taskCount = static_cast<int32>(m_taskVec.size());
			for (const auto& task : m_taskVec)task->ExecuteTask();
			m_taskVec.clear();
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