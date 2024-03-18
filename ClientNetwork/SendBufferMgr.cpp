#include "ClientNetworkPch.h"
#include "SendBufferMgr.h"
#include "SendBufferChunk.h"

namespace NetHelper
{
	SendBufferMgr::SendBufferMgr()
		:m_chunkPool(4)
	{
		for (auto& pChunk : m_chunkPool)pChunk = S_ptr<SendBufferChunk>{ new SendBufferChunk,PushGlobal };
	}

	SendBufferMgr::~SendBufferMgr()
	{
		const std::vector<S_ptr<SendBufferChunk>>* const temp = new std::vector<S_ptr<SendBufferChunk>>{ m_chunkPool };
	}

	S_ptr<SendBuffer> SendBufferMgr::Open(c_uint32 size_)noexcept
	{
		thread_local S_ptr<SendBufferChunk> LSendBufferChunk = Pop();

		NET_NAGOX_ASSERT(false == LSendBufferChunk->IsOpen());

		if (size_ > LSendBufferChunk->FreeSize())
		{
			LSendBufferChunk = Pop();
			LSendBufferChunk->Reset();
		}

		return LSendBufferChunk->Open(LSendBufferChunk, size_);
	}

	void SendBufferMgr::Push(S_ptr<SendBufferChunk>&& pChunk) noexcept
	{
		m_spinLock.lock();
		m_chunkPool.emplace_back(std::move(pChunk));
		m_spinLock.unlock();
	}

	S_ptr<SendBufferChunk> SendBufferMgr::Pop()noexcept
	{
		m_spinLock.lock();
		if (!m_chunkPool.empty())
		{
			auto sendBuffChunk = std::move(m_chunkPool.back());
			m_chunkPool.pop_back();
			m_spinLock.unlock();
			return std::move(sendBuffChunk);
		}
		m_spinLock.unlock();
		return S_ptr<SendBufferChunk>{ new SendBufferChunk, PushGlobal };
	}

	void SendBufferMgr::PushGlobal(SendBufferChunk* const buffer) noexcept
	{
		NetMgr(SendBufferMgr)->Push(S_ptr<SendBufferChunk>{ buffer, PushGlobal });
	}
}