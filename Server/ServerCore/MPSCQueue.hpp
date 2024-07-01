#pragma once
#include "ServerCorePch.h"
#include "EBR.hpp"
#include "BackOff.h"

namespace ServerCore
{
	template <typename T>
	class MPSCQueue
	{
	private:
		struct Node
		{
			T data;
			std::atomic<Node*> next = nullptr;
			template<typename... Args>
			constexpr Node(Args&&... args)noexcept :data{ std::forward<Args>(args)... }, next{ nullptr } {}
		};
		alignas(64) Node* volatile tail;
		alignas(64) std::atomic<Node*> head;
	private:
		void reset()noexcept {
			Node* curHead = head.load(std::memory_order_acquire);
			const Node* const curTail = tail;
			while (curTail != curHead)
			{
				Node* const delHead = curHead;
				curHead = curHead->next.load(std::memory_order_acquire);
				xdelete<Node>(delHead);
			}
			head.store(curHead, std::memory_order_release);
		}
	public:
		MPSCQueue()noexcept
			: tail{ xnew<Node>() }
			, head{ tail }
		{
		}
		~MPSCQueue()noexcept {
			reset();
			xdelete<Node>(head.load(std::memory_order_relaxed));
		}
		template <typename... Args>
		void emplace(Args&&... args) noexcept {
			const BackOff bo{ NUM_OF_THREADS / 2 };
			Node* const value = xnew<Node>(std::forward<Args>(args)...);
			for (;;)
			{
				Node* oldTail = tail;
				if (oldTail != tail)continue;
				if(true == compareExchange(&tail, &oldTail, value))
				{
					oldTail->next.store(value, std::memory_order_release);
					return;
				}
				bo.delay();
			}
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
		const bool try_pop_single(T& _target, Node*& head_temp)noexcept {
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
		const bool try_pop_single(std::vector<T>& _targetForPushBack, Node*& head_temp)noexcept {
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
		void try_flush_single(std::vector<T>& vec_)noexcept {
			Node* head_temp = head.load(std::memory_order_acquire);
			while (try_pop_single(vec_, head_temp));
			head.store(head_temp, std::memory_order_release);
		}
		const bool empty_single()const noexcept {
			return tail == head.load(std::memory_order_relaxed);
		}
		void clear_single()noexcept { reset(); }
	};
}