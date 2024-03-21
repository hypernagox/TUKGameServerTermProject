#pragma once
#include <memory>
#include "Container.h"

namespace ServerCore
{
	template <typename T>
	class enable_shared_cache_this
		:public std::enable_shared_from_this<T>
	{
	public:
		void register_cache_shared()noexcept{
			std::atomic_thread_fence(std::memory_order_acquire);
			if (nullptr != m_forSharedFromThis) [[unlikely]]
				return;
			m_forSharedFromThis = std::enable_shared_from_this<T>::shared_from_this();
			std::atomic_thread_fence(std::memory_order_release);
		}
	public:
		const S_ptr<T>& shared_from_this_ref()const noexcept { return m_forSharedFromThis; }
		S_ptr<T> shared_from_this()noexcept { return m_forSharedFromThis; }
		S_ptr<const T> shared_from_this()const noexcept { return m_forSharedFromThis; }
		W_ptr<T> weak_from_this()noexcept { return m_forSharedFromThis; }
		W_ptr<const T> weak_from_this()const noexcept { return m_forSharedFromThis; }
	public:
		template<typename U>
		S_ptr<U> SharedCastThis() noexcept {
			if constexpr (std::same_as<U, T>)
				return shared_from_this();
			else
				return S_ptr<U>{m_forSharedFromThis, static_cast<U* const>(this)};
		}
		template<typename U>
		S_ptr<const U> SharedCastThis() const noexcept {
			if constexpr (std::same_as<U, T>)
				return shared_from_this();
			else
				return S_ptr<const U>{m_forSharedFromThis, static_cast<const U* const>(this)};
		}
	public:
		void reset_cache_shared(const class SessionManageable& pSessionRoom)noexcept { m_forSharedFromThis.reset(); }
		void reset_cache_shared(const class Listener& pListener)noexcept { m_forSharedFromThis.reset(); }
	protected:
		void reset_cache_shared()noexcept { m_forSharedFromThis.reset(); }
	private:
		S_ptr<T> m_forSharedFromThis = nullptr;
	};

	template <typename T>
	class enable_shared_cache_this_core
	{
	public:
		void register_cache_shared_core(const S_ptr<T>& pForRegisterSharedThis)noexcept {
			NAGOX_ASSERT(this == pForRegisterSharedThis.get());
			std::atomic_thread_fence(std::memory_order_acquire);
			if (nullptr != m_forSharedFromThis) [[unlikely]]
				return;
			m_forSharedFromThis = pForRegisterSharedThis;
			std::atomic_thread_fence(std::memory_order_release);
		}
	public:
		const S_ptr<T>& shared_from_this_ref()const noexcept { return m_forSharedFromThis; }
		S_ptr<T> shared_from_this()noexcept { return m_forSharedFromThis; }
		S_ptr<const T> shared_from_this()const noexcept { return m_forSharedFromThis; }
		W_ptr<T> weak_from_this()noexcept { return m_forSharedFromThis; }
		W_ptr<const T> weak_from_this()const noexcept { return m_forSharedFromThis; }
	public:
		template<typename U>
		S_ptr<U> SharedCastThis() noexcept {
			if constexpr (std::same_as<U, T>)
				return shared_from_this();
			else
				return S_ptr<U>{m_forSharedFromThis, static_cast<U* const>(this)};
		}
		template<typename U>
		S_ptr<const U> SharedCastThis() const noexcept {
			if constexpr (std::same_as<U, T>)
				return shared_from_this();
			else
				return S_ptr<const U>{m_forSharedFromThis, static_cast<const U* const>(this)};
		}
	public:
		void reset_cache_shared(const class SessionManageable& pSessionRoom)noexcept { m_forSharedFromThis.reset(); }
		void reset_cache_shared(const class Listener& pListener)noexcept { m_forSharedFromThis.reset(); }
	protected:
		void reset_cache_shared()noexcept { m_forSharedFromThis.reset(); }
	private:
		S_ptr<T> m_forSharedFromThis = nullptr;
	};
}