#pragma once
#include "AtomicMemoryPool.hpp"

namespace ServerCore
{
	template<typename T>
	class ObjectPoolAllocator
	{
	public:
		using value_type = T;

		constexpr ObjectPoolAllocator()noexcept {}

		template<typename Other>
		ObjectPoolAllocator(const ObjectPoolAllocator<Other>&) { }

		constexpr static T* const allocate(const size_t size)noexcept
		{
			return static_cast<T* const>(allocator_once.allocate());
		}

		constexpr static void deallocate(T* const ptr, const size_t count)noexcept
		{
			allocator_once.deallocate(ptr);
		}

	private:
		static inline AtomicMemoryPool<T> allocator_once{ DEFAULT_MEM_POOL_SIZE };
	};

	template<typename T, typename... Args>
	constexpr std::shared_ptr<T> MakePoolShared(Args&&... args)noexcept
	{
		return std::allocate_shared<T>(ObjectPoolAllocator<T>{}, std::forward<Args>(args)...);
	}


	template<typename T, typename... Args>
	constexpr T* const PoolNew(Args&&... args)noexcept
	{
		return std::construct_at<T>(static_cast<T* const>(ObjectPoolAllocator<T>::allocate(sizeof(T))), std::forward<Args>(args)...);
	}

	template <typename T>
	constexpr void PoolDelete(T* const poolObjPtr)noexcept
	{
		std::destroy_at<T>(poolObjPtr);
		ObjectPoolAllocator<T>::deallocate(poolObjPtr, 1);
	}

	template <typename T>
	struct UPDeleter {
		constexpr inline void operator()(T* const ptr) const noexcept { PoolDelete<T>(ptr); }
	};

	template<typename T>
	using U_Pptr = std::unique_ptr<T, UPDeleter<T>>;

	template<typename T, typename... Args>
	constexpr U_Pptr<T> MakePoolUnique(Args&&... args)noexcept
	{
		return std::unique_ptr<T, UPDeleter<T>>{PoolNew<T>(std::forward<Args>(args)...), UPDeleter<T>{}};
	}
}
