#include "ServerCorePch.h"
#include "SendBufferChunk.h"
#include "SendBuffer.h"
#include "ObjectPool.hpp"

namespace ServerCore
{
	extern thread_local std::shared_ptr<class SendBufferChunk> LSendBufferChunk;

	S_ptr<SendBuffer> SendBufferChunk::Open(c_uint32 allocSize)noexcept
	{
		NAGOX_ASSERT(allocSize <= SEND_BUFFER_CHUNK_SIZE);
		NAGOX_ASSERT(false == m_bOpen);

		m_bOpen = true;

		return MakePoolShared<SendBuffer>(LSendBufferChunk, Buffer(), allocSize);
	}
}
