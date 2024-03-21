#include "ServerCorePch.h"
#include "SendBufferMgr.h"
#include "SendBuffer.h"
#include "SendBufferChunk.h"

namespace ServerCore
{
	extern thread_local std::shared_ptr<class SendBufferChunk> LSendBufferChunk;

	template<typename T>
	class SendBufferAllocator
	{
	public:
		using value_type = T;

		constexpr SendBufferAllocator()noexcept {}

		template<typename Other>
		SendBufferAllocator(const SendBufferAllocator<Other>&) { }

		constexpr static T* const allocate(const size_t size)noexcept
		{
			return static_cast<T* const>(SendBufferMgrHelper::GetSendBufferMgrPool()->allocate());
		}

		constexpr static void deallocate(T* const ptr, const size_t count)noexcept
		{
			SendBufferMgrHelper::GetSendBufferMgrPool()->deallocate(ptr);
		}
	};

	SendBufferMgr::SendBufferMgr()
		:m_pSendBufferPool{ MakeUnique<AtomicNonTemplate>(AllocateSharedActualSize<SendBufferChunk>()) }
	{
	}

	SendBufferMgr::~SendBufferMgr()
	{
	}

	S_ptr<SendBuffer> SendBufferMgr::Open(c_uint32 size_)noexcept
	{
		NAGOX_ASSERT(false == LSendBufferChunk->IsOpen());

		// 다쓰면 교체
		if (size_ > LSendBufferChunk->FreeSize())
		{
			LSendBufferChunk = Pop();
			LSendBufferChunk->Reset();
		}

		return LSendBufferChunk->Open(size_);
	}

	S_ptr<SendBufferChunk> SendBufferMgr::Pop()noexcept
	{
		return std::allocate_shared<SendBufferChunk>(SendBufferAllocator<SendBufferChunk>{});
	}
}