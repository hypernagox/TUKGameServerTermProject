#include "ServerCorePch.h"
#include "MemoryMgr.h"

/*-------------
	Memory
---------------*/

namespace ServerCore
{
	MemoryMgr::MemoryMgr()
	{
		m_pools.reserve(64);

		int32 size = 0;
		int32 tableIndex = 0;

		for (size = 32; size <= 1024; size += 32)
		{
			AtomicNonTemplate* const pool = new (m_poolAllocator.allocate())AtomicNonTemplate(size);
			m_pools.emplace_back(pool);

			while (tableIndex <= size)
			{
				m_poolTable[tableIndex] = pool;
				tableIndex++;
			}
		}
		size -= 32;
		for (; size <= 2048; size += 128)
		{
			AtomicNonTemplate* const pool = new (m_poolAllocator.allocate())AtomicNonTemplate(size);
			m_pools.emplace_back(pool);

			while (tableIndex <= size)
			{
				m_poolTable[tableIndex] = pool;
				tableIndex++;
			}
		}
		size -= 128;
		for (; size <= 4096; size += 256)
		{
			AtomicNonTemplate* const pool = new (m_poolAllocator.allocate())AtomicNonTemplate(size);
			m_pools.emplace_back(pool);

			while (tableIndex <= size)
			{
				m_poolTable[tableIndex] = pool;
				tableIndex++;
			}
		}
	}

	MemoryMgr::~MemoryMgr()
	{
		for (auto& pool : m_pools)
		{
			pool->~AtomicNonTemplate();
			m_poolAllocator.deallocate(pool);
		}
	}

	void* const MemoryMgr::Allocate(const size_t size)const noexcept
	{
		const size_t allocSize = size + sizeof(MemoryHeader);
#ifdef _STOMP
		return MemoryHeader::AttachHeader(static_cast<MemoryHeader* const>(StompAllocator::Alloc(allocSize)), allocSize);
#else
		if (allocSize > MAX_ALLOC_SIZE)
		{
			// 메모리 풀링 최대 크기를 벗어나면 일반 할당
			return MemoryHeader::AttachHeader(static_cast<MemoryHeader* const>(::_aligned_malloc(allocSize, std::hardware_constructive_interference_size)),allocSize);
		}
		else
		{
			// 메모리 풀에서 꺼내온다
			return MemoryHeader::AttachHeader(static_cast<MemoryHeader* const>(m_poolTable[allocSize]->allocate()), allocSize);
		}
#endif	
	}

	void MemoryMgr::Release(void* const ptr)const noexcept
	{
		MemoryHeader* const __restrict header = MemoryHeader::DetachHeader(ptr);
		const size_t allocSize = header->allocSize;

		NAGOX_ASSERT(allocSize > 0);

		header->allocSize = 0;
#ifdef _STOMP
		StompAllocator::Release(header);
#else
		if (allocSize > MAX_ALLOC_SIZE)
		{
			// 메모리 풀링 최대 크기를 벗어나면 일반 해제
			::_aligned_free(header);
		}
		else
		{
			// 메모리 풀에 반납한다
			m_poolTable[allocSize]->deallocate(header);
		}
#endif	
	}
}