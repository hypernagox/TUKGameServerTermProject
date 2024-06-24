#pragma once

namespace ServerCore
{
	class AtomicNonTemplate;
	class SendBufferChunk;
	class SendBuffer;

	/*--------------
		SendBufferMgr
	---------------*/

	namespace SendBufferMgrHelper {
		constexpr static inline AtomicNonTemplate* const GetSendBufferMgrPool()noexcept;
	};

	class SendBufferMgr
		:public Singleton<SendBufferMgr>
	{
		friend constexpr static inline AtomicNonTemplate* const SendBufferMgrHelper::GetSendBufferMgrPool()noexcept;
		friend class Singleton;
		friend class ThreadMgr;
		SendBufferMgr();
		~SendBufferMgr();
	public:
		static S_ptr<SendBuffer> Open(c_uint32 size_)noexcept;
		static S_ptr<SendBufferChunk> Pop()noexcept;
		AtomicNonTemplate m_pSendBufferPool;
	};

	namespace SendBufferMgrHelper {
		constexpr static inline AtomicNonTemplate* const GetSendBufferMgrPool()noexcept { return &Mgr(SendBufferMgr)->m_pSendBufferPool; }
	};
}