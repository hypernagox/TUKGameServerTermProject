#pragma once
#include "ServerCorePch.h"
#include "ObjectPool.hpp"
#include "SRWLock.hpp"

namespace ServerCore
{
	
	template <typename T>
	class S_ptr;

	class RefCountable
	{
	public:
		using DeleterFunc = void(*)(RefCountable* const)noexcept;
		friend static inline RefCountable* const IncAndGetPtrExternal(const RefCountable* const ref_ptr)noexcept;
		friend static inline void DecRefExternal(const RefCountable* const ref_ptr) noexcept;
		friend static inline void SetDeleterExternal(RefCountable* const ptr, const DeleterFunc deleter) noexcept;
		virtual ~RefCountable()noexcept = default;
	public:
		template<typename T = RefCountable>
		S_ptr<T> SharedFromThis()const noexcept { return S_ptr<T>{this}; }
		inline const uint64_t UseCount()const noexcept { return m_refCount.load(std::memory_order_relaxed) >> 48; }
		inline void IncRef()const noexcept { m_refCount.fetch_add(1ULL << 48, std::memory_order_relaxed); }
		void DecRef()const noexcept;
	private:
		inline void SetDeleter(const DeleterFunc deleter) noexcept {
			m_refCount.store((m_refCount.load(std::memory_order_relaxed) & (0xFFFFULL << 48)) | (reinterpret_cast<const uint64_t>(deleter) & ((1ULL << 48) - 1)), std::memory_order_relaxed);
		}
		inline const RefCountable* const IncAndGetPtrInternal()const noexcept { IncRef(); return this; }
		inline RefCountable* const IncAndGetPtr()const noexcept {
			return const_cast<RefCountable* const>(IncAndGetPtrInternal());
		}
	private:
		mutable std::atomic_uint64_t m_refCount = 0;
	};

	static inline RefCountable* const IncAndGetPtrExternal(const RefCountable* const ref_ptr)noexcept { return ref_ptr->IncAndGetPtr(); }
	static inline void DecRefExternal(const RefCountable* const ref_ptr) noexcept { ref_ptr->DecRef(); }
	static inline void SetDeleterExternal(RefCountable* const ptr, const RefCountable::DeleterFunc deleter) noexcept { ptr->SetDeleter(deleter); }

	template <typename T>
	class S_ptr
	{
	public:
		S_ptr()noexcept = default;
		S_ptr(std::nullptr_t) noexcept : m_count_ptr{ nullptr } {}
		~S_ptr()noexcept { DecRef(); }
		explicit S_ptr(const RefCountable* const ptr)noexcept
			:m_count_ptr{ IncAndGetPtrExternal(ptr)}
		{}
		explicit S_ptr(RefCountable* const ptr)noexcept
			:m_count_ptr{ IncAndGetPtrExternal(ptr) }
		{}
	public:
		S_ptr(const S_ptr& other)noexcept
			:m_count_ptr{ other.IncRef() }
		{}
		S_ptr& operator=(const S_ptr& other)noexcept {
			if (m_count_ptr != other.m_count_ptr) {
				DecRef();
				m_count_ptr = other.IncRef();
			}
			return *this;
		}
		S_ptr(S_ptr&& other)noexcept
			:m_count_ptr{ std::exchange(other.m_count_ptr,nullptr) }
		{}
		S_ptr& operator=(S_ptr&& other)noexcept {
			if (m_count_ptr != other.m_count_ptr) {
				DecRef();
				m_count_ptr = other.m_count_ptr;
				other.m_count_ptr = nullptr;
			}
			return *this;
		}
	public:
		template <typename U> requires std::derived_from<U,T>
		S_ptr(const S_ptr<U>& other)noexcept
			:m_count_ptr{ other.IncRef() }
		{}
		template <typename U> requires std::derived_from<U, T>
		S_ptr& operator=(const S_ptr<U>& other)noexcept {
			if (m_count_ptr != other.m_count_ptr) {
				DecRef();
				m_count_ptr = other.IncRef();
			}
			return *this;
		}
		template <typename U> requires std::derived_from<U, T>
		S_ptr(S_ptr<U>&& other)noexcept
			:m_count_ptr{ std::exchange(other.m_count_ptr,nullptr) }
		{}
		template <typename U> requires std::derived_from<U, T>
		S_ptr& operator=(S_ptr<U>&& other)noexcept {
			if (m_count_ptr != other.m_count_ptr) {
				DecRef();
				m_count_ptr = other.m_count_ptr;
				other.m_count_ptr = nullptr;
			}
			return *this;
		}
		S_ptr(RefCountable** ptr)noexcept
			:m_count_ptr{ std::exchange(*ptr,nullptr) }
		{}
	public:
		const uint64_t UseCount()const noexcept { return m_count_ptr ? m_count_ptr->UseCount() : 0; }
		void reset()noexcept { DecRef(); release(); }
		void release()noexcept { m_count_ptr = nullptr; }
		template <typename U> requires std::derived_from<U, T>
		void swap(S_ptr<U>& ptr)noexcept { std::swap(m_count_ptr, ptr.m_count_ptr); }
	public:
		T& operator*()const noexcept { return *static_cast<T* const>(const_cast<RefCountable* const>(m_count_ptr)); }
		T* const operator->()const noexcept { return static_cast<T* const>(const_cast<RefCountable*const>(m_count_ptr)); }
		T* const get()const noexcept { return static_cast<T* const>(const_cast<RefCountable* const>(m_count_ptr)); }
		const auto operator<=>(const S_ptr&)const noexcept = default;
		operator bool()const noexcept { return m_count_ptr; }
	public:
		void DecRef()const noexcept {
			if (m_count_ptr)
				DecRefExternal(m_count_ptr);
		}
		RefCountable* const IncRef()const noexcept {
			return m_count_ptr ? IncAndGetPtrExternal(m_count_ptr) : nullptr;
		}
	public:
		RefCountable* m_count_ptr = nullptr;
	};

	template <typename T, typename... Args> requires std::derived_from<T,RefCountable>
	S_ptr<T> MakePoolShared(Args&&... args) {
		const auto ptr = PoolNew<T>(std::forward<Args>(args)...);
		SetDeleterExternal(ptr,[](RefCountable* const p)noexcept {
			std::destroy_at(static_cast<T* const>(p));
			ObjectPoolAllocator<T>::deallocate(static_cast<T* const>(p), 1);
			});
		return S_ptr<T>{ptr};
	}

	template<typename Type, typename... Args> requires std::derived_from<Type, RefCountable>
	constexpr S_ptr<Type> MakeShared(Args&&... args)noexcept
	{
		return S_ptr<Type>{xnew<Type>(std::forward<Args>(args)...)};
	}

	template <typename U, typename T> requires std::derived_from<U, T>
	static S_ptr<U> StaticCast(const S_ptr<T>& ptr)noexcept { return S_ptr<U>{ptr.get()}; }
	template <typename U, typename T> requires std::derived_from<U,T>
	static S_ptr<U> StaticCast(S_ptr<T>&& ptr)noexcept { return S_ptr<U>{&ptr.m_count_ptr}; }

	template<typename T>
	class AtomicS_ptr
	{
	public:
		template <typename U>
		void store(const S_ptr<U>& p)noexcept {
			m_srwLock.lock();
			m_ptr = p;
			m_srwLock.unlock();
		}
		template <typename U>
		void store(S_ptr<U>&& p)noexcept {
			m_srwLock.lock();
			m_ptr = std::move(p);
			m_srwLock.unlock();
		}
		S_ptr<T> load()const noexcept {
			S_ptr<T> temp;
			m_srwLock.lock_shared();
			std::construct_at(&temp, m_ptr);
			m_srwLock.unlock_shared();
			return temp;
		}
		void reset()noexcept {
			m_srwLock.lock();
			m_ptr.reset();
			m_srwLock.unlock();
		}
		void store(std::nullptr_t)noexcept { reset(); }
		operator S_ptr<T>()const noexcept { return load(); }
	private:
		mutable SRWLock m_srwLock;
		S_ptr<T> m_ptr;
	};
}

namespace std {
	template <typename T>
	struct hash<ServerCore::S_ptr<T>> {
		const std::size_t operator()(const ServerCore::S_ptr<T>& ptr) const noexcept {
			return std::hash<T*>{}(ptr.get());
		}
	};
}