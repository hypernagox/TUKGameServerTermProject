#pragma once
#include "ServerCorePch.h"

namespace ServerCore
{
	template <typename T>
	class MPSCQueue
	{

	private:
		struct Node {
			T data;
			std::atomic<Node*> next = nullptr;
			template<typename... Args>
			constexpr Node(Args&&... args)noexcept :data{ std::forward<Args>(args)... }, next{ nullptr } {}
		};
		std::atomic<Node*> head;
		SpinLock headLock;
		std::atomic<Node*> tail;
	private:
		void reset()noexcept {
			Node* curHead = head.load(std::memory_order_acquire);
			const Node* const curTail = tail.load(std::memory_order_acquire);
			while (curTail != curHead)
			{
				Node* const delHead = curHead;
				curHead = curHead->next.load(std::memory_order_acquire);
				xdelete<Node>(delHead);
			}
			head.store(curHead, std::memory_order_release);
		}
	public:
		MPSCQueue()noexcept :head{ xnew<Node>() } {
			tail.store(head, std::memory_order_relaxed);
			head.load(std::memory_order_relaxed)->next.store(nullptr, std::memory_order_relaxed);
		}
		~MPSCQueue()noexcept {
			reset();
			xdelete<Node>(head.load(std::memory_order_relaxed));
		}
		template <typename... Args>
		void emplace(Args&&... args) noexcept {
			Node* const value = xnew<Node>(std::forward<Args>(args)...);
			Node* __restrict oldTail = tail.load(std::memory_order_relaxed);
			while (!tail.compare_exchange_weak(oldTail, value
				, std::memory_order_relaxed
				, std::memory_order_relaxed))
			{
			}
			oldTail->next.store(value, std::memory_order_release);
		}
		const bool try_pop(T& _target)noexcept {
			Node* oldHead;
			Node* newHead;
			headLock.lock();
			if (!(newHead = head->next))
			{
				headLock.unlock();
				return false;
			}
			oldHead = head;
			head = newHead;
			if constexpr (std::swappable<T>)
				_target.swap(newHead->data);
			else
				std::swap(_target, newHead->data);
			headLock.unlock();
			xdelete<Node>(oldHead);
			return true;
		}
		const bool try_pop(Vector<T>& _targetForPushBack)noexcept {
			Node* oldHead;
			Node* newHead;
			headLock.lock();
			if (!(newHead = head->next))
			{
				headLock.unlock();
				return false;
			}
			oldHead = head;
			head = newHead;
			_targetForPushBack.emplace_back(std::move(newHead->data));
			headLock.unlock();
			xdelete<Node>(oldHead);
			return true;
		}
		const bool try_pop_for_cv(T& _target, std::unique_lock<SpinLock>& cvLock)noexcept {
			Node* const head_temp = head.load(std::memory_order_relaxed);
			if (Node* const __restrict newHead = head_temp->next.load(std::memory_order_acquire))
			{
				Node* const oldHead = head_temp;
				head.store(newHead, std::memory_order_release);
				if constexpr (std::swappable<T>)
					_target.swap(newHead->data);
				else
					std::swap(_target, newHead->data);
				cvLock.unlock();
				xdelete<Node>(oldHead);
				return true;
			}
			return false;
		}
		const bool try_pop_single(T& _target)noexcept {
			Node* const head_temp = head.load(std::memory_order_relaxed);
			if (Node* const __restrict newHead = head_temp->next.load(std::memory_order_acquire))
			{
				Node* const oldHead = head_temp;
				head.store(newHead, std::memory_order_release);
				if constexpr (std::swappable<T>)
					_target.swap(newHead->data);
				else
					std::swap(_target, newHead->data);
				xdelete<Node>(oldHead);
				return true;
			}
			return false;
		}
		const bool try_pop_single(Vector<T>& _targetForPushBack)noexcept {
			Node* const head_temp = head.load(std::memory_order_relaxed);
			if (Node* const __restrict newHead = head_temp->next.load(std::memory_order_acquire))
			{
				Node* const oldHead = head_temp;
				head.store(newHead, std::memory_order_release);
				_targetForPushBack.emplace_back(std::move(newHead->data));
				xdelete<Node>(oldHead);
				return true;
			}
			return false;
		}
		const bool try_pop_single(T& _target,Node*& head_temp)noexcept {
			if (Node* const __restrict newHead = head_temp->next.load(std::memory_order_acquire))
			{
				Node* const oldHead = head_temp;
				head_temp = newHead;
				if constexpr (std::swappable<T>)
					_target.swap(newHead->data);
				else
					std::swap(_target, newHead->data);
				xdelete<Node>(oldHead);
				return true;
			}
			return false;
		}
		const bool try_pop_single(Vector<T>& _targetForPushBack, Node*& head_temp)noexcept {
			if (Node* const __restrict newHead = head_temp->next.load(std::memory_order_acquire))
			{
				Node* const oldHead = head_temp;
				head_temp = newHead;
				_targetForPushBack.emplace_back(std::move(newHead->data));
				xdelete<Node>(oldHead);
				return true;
			}
			return false;
		}
		std::optional<T> pop()noexcept {
			Node* oldHead;
			Node* newHead;
			std::optional<T> temp = std::nullopt;
			headLock.lock();
			if (!(newHead = head->next))
			{
				headLock.unlock();
				return temp;
			}
			oldHead = head;
			head = newHead;
			temp.emplace(std::move(newHead->data));
			headLock.unlock();
			xdelete<Node>(oldHead);
			return temp;
		}
		Vector<T> try_flush()noexcept {
			Vector<T> vec; vec.reserve(32); while (try_pop(vec));
			return vec;
		}
		void try_flush(Vector<T>& vec_)noexcept {
			while (try_pop(vec_));
		}
		Vector<T> try_flush_single()noexcept {
			Node* head_temp = head.load(std::memory_order_acquire);
			Vector<T> vec; vec.reserve(32); while (try_pop_single(vec, head_temp));
			head.store(head_temp, std::memory_order_release);
			return vec;
		}
		void try_flush_single(Vector<T>& vec_)noexcept {
			Node* head_temp = head.load(std::memory_order_acquire);
			while (try_pop_single(vec_, head_temp));
			head.store(head_temp, std::memory_order_release);
		}
		const bool empty() noexcept {
			bool bIsEmpty;
			{
				std::lock_guard<SpinLock> lock{ headLock };
				bIsEmpty = !head->next;
			}
			return bIsEmpty;
		}
		const bool empty_single()const noexcept {
			return tail.load(std::memory_order_relaxed) == head.load(std::memory_order_relaxed);
		}
		void clear() noexcept {
			{
				std::lock_guard<SpinLock> lock{ headLock };
				reset();
			}
		}
		void clear_single()noexcept { reset(); }
	};
}