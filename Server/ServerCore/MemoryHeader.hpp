#pragma once

/*-----------------
	MemoryHeader
------------------*/

namespace ServerCore
{
	struct alignas(8) MemoryHeader
	{
		// [MemoryHeader][Data]
		constexpr MemoryHeader(const size_t size)noexcept : allocSize{ size } {}

		constexpr static void* const AttachHeader(MemoryHeader* const header, const size_t size)noexcept
		{
			return new(header)MemoryHeader(size) + 1;
		}

		constexpr static MemoryHeader* const DetachHeader(void* const ptr)noexcept
		{
			return static_cast<MemoryHeader* const>(ptr) - 1;
		}
		alignas(8) size_t allocSize;
	};
}