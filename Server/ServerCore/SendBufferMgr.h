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
		constexpr static inline const U_ptr<AtomicNonTemplate>& GetSendBufferMgrPool()noexcept;
	};

	class SendBufferMgr
		:public Singleton<SendBufferMgr>
	{
		friend constexpr static inline const U_ptr<AtomicNonTemplate>& SendBufferMgrHelper::GetSendBufferMgrPool()noexcept;
		friend class Singleton;
		friend class ThreadMgr;
		SendBufferMgr();
		~SendBufferMgr();
	public:
		static S_ptr<SendBuffer> Open(c_uint32 size_)noexcept;
	private:
		static S_ptr<SendBufferChunk> Pop()noexcept;
		const U_ptr<AtomicNonTemplate> m_pSendBufferPool;
	};

	namespace SendBufferMgrHelper {
		constexpr static inline const U_ptr<AtomicNonTemplate>& GetSendBufferMgrPool()noexcept { return Mgr(SendBufferMgr)->m_pSendBufferPool; }
	};
}