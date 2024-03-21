#pragma once

namespace ServerCore
{
	class MemoryMgr;

	/*-------------------
		StompAllocator
	-------------------*/

	class StompAllocator
	{
		enum { PAGE_SIZE = 0x1000 };

	public:
		static void* Alloc(const size_t size);
		static void		Release(void* ptr);
	};

	/*-------------------
		STL Allocator
	-------------------*/

	template<typename T>
	class StlAllocator
	{
	public:
		using value_type = T;

		constexpr StlAllocator()noexcept {}

		template<typename Other>
		StlAllocator(const StlAllocator<Other>&) {}

		constexpr static T* const allocate(const size_t count)noexcept
		{
			return static_cast<T* const>(Mgr(MemoryMgr)->Allocate(count * sizeof(T)));
		}

		constexpr static void deallocate(T* const ptr, const size_t count)noexcept
		{
			Mgr(MemoryMgr)->Release(ptr);
		}

		friend constexpr const bool operator==(const StlAllocator<T>&, const StlAllocator<T>&)noexcept { return true; }

		friend constexpr const bool operator!=(const StlAllocator<T>& a, const StlAllocator<T>& b)noexcept { return !(a == b); }
	};

	template<typename T>
	class SharedAllocator
	{
	public:
		using value_type = T;

		constexpr SharedAllocator()noexcept {}

		template<typename Other>
		SharedAllocator(const SharedAllocator<Other>&) { }

		constexpr static T* const allocate(const size_t size)noexcept
		{
			return static_cast<T* const>(Mgr(MemoryMgr)->Allocate(size * sizeof(T)));
		}

		constexpr static void deallocate(T* const ptr, const size_t count)noexcept
		{
			Mgr(MemoryMgr)->Release(ptr);
		}
	};

	template <typename T>
	struct UDeleter { constexpr inline void operator()(T* const ptr) const noexcept { xdelete<T>(ptr); } };
}