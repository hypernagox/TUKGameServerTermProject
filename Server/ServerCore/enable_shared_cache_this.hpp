#pragma once
#include <memory>
#include "Container.h"

namespace ServerCore
{
	//template <typename T>
	//class enable_shared_cache_this
	//	:public std::enable_shared_from_this<T>
	//{
	//public:
	//	void register_cache_shared()noexcept {
	//		//std::atomic_thread_fence(std::memory_order_acquire);
	//		//if (nullptr != m_forSharedFromThis) [[unlikely]]
	//		//	return;
	//		//	m_forSharedFromThis = std::enable_shared_from_this<T>::shared_from_this();
	//		//	std::atomic_thread_fence(std::memory_order_release);
	//	}
	//public:
	//	//constexpr inline S_ptr<T> shared_from_this()noexcept {
	//	//	S_ptr<T> temp;
	//	//	m_srwLock.lock_shared();
	//	//	std::construct_at(&temp, m_forSharedFromThis);
	//	//	m_srwLock.unlock_shared();
	//	//	return temp;
	//	//}
	//	//constexpr inline S_ptr<const T> shared_from_this()const noexcept {
	//	//	S_ptr<const T> temp;
	//	//	m_srwLock.lock_shared();
	//	//	std::construct_at(&temp, m_forSharedFromThis);
	//	//	m_srwLock.unlock_shared();
	//	//	return temp;
	//	//}
	//	//constexpr inline W_ptr<T> weak_from_this()noexcept {
	//	//	W_ptr<T> temp;
	//	//	m_srwLock.lock_shared();
	//	//	std::construct_at(&temp, m_forSharedFromThis);
	//	//	m_srwLock.unlock_shared();
	//	//	return temp;
	//	//}
	//	//constexpr inline W_ptr<const T> weak_from_this()const noexcept {
	//	//	W_ptr<const T> temp;
	//	//	m_srwLock.lock_shared();
	//	//	std::construct_at(&temp, m_forSharedFromThis);
	//	//	m_srwLock.unlock_shared();
	//	//	return temp;
	//	//}
	//public:
	//	template<typename U>
	//	constexpr inline S_ptr<U> SharedCastThis() noexcept {
	//		if constexpr (std::same_as<U, T>)
	//			return std::enable_shared_from_this<T>::shared_from_this();
	//		else
	//			return S_ptr<U>{std::enable_shared_from_this<T>::shared_from_this(), static_cast<U* const>(this)};
	//		//if constexpr (std::same_as<U, T>)
	//		//	return shared_from_this();
	//		//S_ptr<U> temp;
	//		//m_srwLock.lock_shared();
	//		//std::construct_at(&temp, m_forSharedFromThis, static_cast<U* const>(this));
	//		//m_srwLock.unlock_shared();
	//		//return temp;
	//	}
	//	template<typename U>
	//	constexpr inline S_ptr<const U> SharedCastThis() const noexcept {
	//		if constexpr (std::same_as<U, T>)
	//			return std::enable_shared_from_this<T>::shared_from_this();
	//		else
	//			return S_ptr<const U>{std::enable_shared_from_this<T>::shared_from_this(), static_cast<const U* const>(this)};
	//		//if constexpr (std::same_as<U, T>)
	//		//	return shared_from_this();
	//		//S_ptr<const U> temp;
	//		//m_srwLock.lock_shared();
	//		//std::construct_at(&temp, m_forSharedFromThis, static_cast<const U* const>(this));
	//		//m_srwLock.unlock_shared();
	//		//return temp;
	//	}
	//public:
	//	void reset_cache_shared(const class SessionManageable& pSessionRoom)noexcept {
	//		//auto& reset_ptr = m_forSharedFromThis;
	//		//m_srwLock.lock();
	//		//reset_ptr.reset();
	//		//m_srwLock.unlock();
	//	}
	//	void reset_cache_shared(const class Listener& pListener)noexcept {
	//		//auto& reset_ptr = m_forSharedFromThis;	
	//		//m_srwLock.lock();
	//		//reset_ptr.reset();
	//		//m_srwLock.unlock();
	//	}
	//public:
	//	void reset_cache_shared()noexcept {
	//		//auto& reset_ptr = m_forSharedFromThis;
	//		//m_srwLock.lock();
	//		//reset_ptr.reset();
	//		//m_srwLock.unlock();
	//	}
	//private:
	//	//SRWLock m_srwLock;
	//	//S_ptr<T> m_forSharedFromThis = nullptr;
	//};
	//
	//template <typename T>
	//class enable_shared_cache_this_core
	//	:public std::enable_shared_from_this<T>
	//{
	//public:
	//	void register_cache_shared_core(const S_ptr<T>& pForRegisterSharedThis)noexcept {
	//		//NAGOX_ASSERT(this == pForRegisterSharedThis.get());
	//		//m_srwLock.lock_shared();
	//		//if (nullptr != m_forSharedFromThis) [[unlikely]]
	//		//	return;
	//		//	m_forSharedFromThis = pForRegisterSharedThis;
	//		//	m_srwLock.unlock_shared();
	//	}
	//public:
	//	//constexpr inline S_ptr<T> shared_from_this()noexcept {
	//	//	S_ptr<T> temp;
	//	//	m_srwLock.lock_shared();
	//	//	std::construct_at(&temp, m_forSharedFromThis);
	//	//	m_srwLock.unlock_shared();
	//	//	return temp;
	//	//}
	//	//constexpr inline S_ptr<const T> shared_from_this()const noexcept {
	//	//	S_ptr<const T> temp;
	//	//	m_srwLock.lock_shared();
	//	//	std::construct_at(&temp, m_forSharedFromThis);
	//	//	m_srwLock.unlock_shared();
	//	//	return temp;
	//	//}
	//	//constexpr inline W_ptr<T> weak_from_this()noexcept {
	//	//	W_ptr<T> temp;
	//	//	m_srwLock.lock_shared();
	//	//	std::construct_at(&temp, m_forSharedFromThis);
	//	//	m_srwLock.unlock_shared();
	//	//	return temp;
	//	//}
	//	//constexpr inline W_ptr<const T> weak_from_this()const noexcept {
	//	//	W_ptr<const T> temp;
	//	//	m_srwLock.lock_shared();
	//	//	std::construct_at(&temp, m_forSharedFromThis);
	//	//	m_srwLock.unlock_shared();
	//	//	return temp;
	//	//}
	//public:
	//	template<typename U>
	//	constexpr inline S_ptr<U> SharedCastThis() noexcept {
	//		if constexpr (std::same_as<U, T>)
	//			return std::enable_shared_from_this<T>::shared_from_this();
	//		else
	//			return S_ptr<U>{std::enable_shared_from_this<T>::shared_from_this(), static_cast<U* const>(this)};
	//		//if constexpr (std::same_as<U, T>)
	//		//	return shared_from_this();
	//		//S_ptr<U> temp;
	//		//m_srwLock.lock_shared();
	//		//std::construct_at(&temp, m_forSharedFromThis, static_cast<U* const>(this));
	//		//m_srwLock.unlock_shared();
	//		//return temp;
	//	}
	//	template<typename U>
	//	constexpr inline S_ptr<const U> SharedCastThis() const noexcept {
	//		if constexpr (std::same_as<U, T>)
	//			return std::enable_shared_from_this<T>::shared_from_this();
	//		else
	//			return S_ptr<const U>{std::enable_shared_from_this<T>::shared_from_this(), static_cast<const U* const>(this)};
	//		//if constexpr (std::same_as<U, T>)
	//		//	return shared_from_this();
	//		//S_ptr<const U> temp;
	//		//m_srwLock.lock_shared();
	//		//std::construct_at(&temp, m_forSharedFromThis, static_cast<const U* const>(this));
	//		//m_srwLock.unlock_shared();
	//		//return temp;
	//	}
	//public:
	//	void reset_cache_shared(const class SessionManageable& pSessionRoom)noexcept {
	//		//auto& reset_ptr = m_forSharedFromThis;
	//		//m_srwLock.lock();
	//		//reset_ptr.reset();
	//		//m_srwLock.unlock();
	//	}
	//	void reset_cache_shared(const class Listener& pListener)noexcept {
	//		//auto& reset_ptr = m_forSharedFromThis;
	//		//m_srwLock.lock();
	//		//reset_ptr.reset();
	//		//m_srwLock.unlock();
	//	}
	//public:
	//	void reset_cache_shared()noexcept {
	//		//auto& reset_ptr = m_forSharedFromThis;
	//		//m_srwLock.lock();
	//		//reset_ptr.reset();
	//		//m_srwLock.unlock();
	//	}
	//private:
	//	//SRWLock m_srwLock;
	//	//S_ptr<T> m_forSharedFromThis = nullptr;
	//};
}