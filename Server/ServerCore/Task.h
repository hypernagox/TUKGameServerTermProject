#pragma once

namespace ServerCore
{
	template<typename T>
	concept EnableSharedFromThis = requires(T t) {
		{ t.shared_from_this() };
	};

	template<typename T>
	concept IsNotMemFunc = !std::is_member_function_pointer_v<T> && !std::is_member_function_pointer_v<std::decay_t<T>>;

	class Session;

#define SIZE_OF_CALLBACK (104)

	class Task
	{
	public:
		Task()noexcept = delete;
		Task(const Task&) = delete;
		Task(Task&&)noexcept = delete;
		inline constexpr ~Task()noexcept { m_fpTaskDeleter(static_cast<void* const>(argBuff)); }

		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		constexpr Task(Ret(T::* const memFunc)(Args...)noexcept, const std::shared_ptr<U>& memFuncInstance, Args&&... args)noexcept
		{
			struct CallBack
			{
				Ret(T::* const memFunc)(Args...)noexcept;
				mutable std::tuple<std::decay_t<Args>...> args;
				const S_ptr<T> memFuncCaller;
				constexpr CallBack(Ret(T::* const memFunc_)(Args...)noexcept, const std::shared_ptr<U>& memFuncInstance, Args&&... args_)noexcept
					: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ memFuncInstance } {}

				inline constexpr const void operator()()const noexcept
				{
					if constexpr (std::derived_from<T, Session>)
					{
						if (!memFuncCaller->IsConnected())
							return;
					}
					invokeMemberFunction(memFunc, memFuncCaller, std::move(args));
				}
			};
			static_assert(SIZE_OF_CALLBACK >= sizeof(CallBack));
			std::construct_at<CallBack>(reinterpret_cast<CallBack* const>(argBuff), memFunc, memFuncInstance, std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {std::destroy_at<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			m_fpTask = [](const void* const callBackPtr_)noexcept {return static_cast<const CallBack* const>(callBackPtr_)->operator()(); };
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		constexpr Task(Ret(T::* const memFunc)(Args...)noexcept, std::shared_ptr<U>&& memFuncInstance, Args&&... args)noexcept
		{
			struct CallBack
			{
				Ret(T::* const memFunc)(Args...)noexcept;
				mutable std::tuple<std::decay_t<Args>...> args;
				const S_ptr<T> memFuncCaller;
				constexpr CallBack(Ret(T::* const memFunc_)(Args...)noexcept, std::shared_ptr<U>&& memFuncInstance, Args&&... args_)noexcept
					: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ std::move(memFuncInstance) } {}

				inline constexpr const void operator()()const noexcept
				{
					if constexpr (std::derived_from<T, Session>)
					{
						if (!memFuncCaller->IsConnected())
							return;
					}
					invokeMemberFunction(memFunc, memFuncCaller, std::move(args));
				}
			};
			static_assert(SIZE_OF_CALLBACK >= sizeof(CallBack));
			std::construct_at<CallBack>(reinterpret_cast<CallBack* const>(argBuff), memFunc, std::move(memFuncInstance), std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {std::destroy_at<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			m_fpTask = [](const void* const callBackPtr_)noexcept {return static_cast<const CallBack* const>(callBackPtr_)->operator()(); };
		}
		template<typename T, typename U, typename Ret, typename... Args> requires EnableSharedFromThis<U>&& std::derived_from<U, T>
		constexpr Task(Ret(T::* const memFunc)(Args...)noexcept, U* const memFuncInstance, Args&&... args)noexcept
		{
			struct CallBack
			{
				Ret(T::* const memFunc)(Args...)noexcept;
				mutable std::tuple<std::decay_t<Args>...> args;
				const S_ptr<T> memFuncCaller;
				constexpr CallBack(Ret(T::* const memFunc_)(Args...)noexcept, U* const memFuncInstance, Args&&... args_)noexcept
					: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ memFuncInstance->shared_from_this(), static_cast<T* const>(memFuncInstance) } {}

				inline constexpr const void operator()()const noexcept
				{
					if constexpr (std::derived_from<T, Session>)
					{
						if (!memFuncCaller->IsConnected())
							return;
					}
					invokeMemberFunction(memFunc, memFuncCaller, std::move(args));
				}
			};
			static_assert(SIZE_OF_CALLBACK >= sizeof(CallBack));
			std::construct_at<CallBack>(reinterpret_cast<CallBack* const>(argBuff), memFunc, memFuncInstance, std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {std::destroy_at<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			m_fpTask = [](const void* const callBackPtr_)noexcept {return static_cast<const CallBack* const>(callBackPtr_)->operator()(); };
		}

		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		constexpr Task(Ret(T::* const memFunc)(Args...), const std::shared_ptr<U>& memFuncInstance, Args&&... args)noexcept
		{
			struct CallBack
			{
				Ret(T::* const memFunc)(Args...);
				mutable std::tuple<std::decay_t<Args>...> args;
				const S_ptr<T> memFuncCaller;
				constexpr CallBack(Ret(T::* const memFunc_)(Args...), const std::shared_ptr<U>& memFuncInstance, Args&&... args_)noexcept
					: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ memFuncInstance } {}

				inline constexpr const void operator()()const noexcept
				{
					if constexpr (std::derived_from<T, Session>)
					{
						if (!memFuncCaller->IsConnected())
							return;
					}
					invokeMemberFunction(memFunc, memFuncCaller, std::move(args));
				}
			};
			static_assert(SIZE_OF_CALLBACK >= sizeof(CallBack));
			std::construct_at<CallBack>(reinterpret_cast<CallBack* const>(argBuff), memFunc, memFuncInstance, std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {std::destroy_at<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			//if constexpr (SIZE_OF_CALLBACK >= sizeof(CallBack))
			//{
			//	argPtr = std::construct_at<CallBack>(reinterpret_cast<CallBack* const>(argBuff), memFunc, memFuncInstance, std::forward<Args>(args)...);
			//	m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {std::destroy_at<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			//}
			//else
			//{
			//	argPtr = xnew<CallBack>(memFunc, memFuncInstance, std::forward<Args>(args)...);
			//	m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {xdelete<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			//}
			m_fpTask = [](const void* const callBackPtr_)noexcept {return static_cast<const CallBack* const>(callBackPtr_)->operator()(); };
		}
		template<typename T, typename U, typename Ret, typename... Args> requires std::derived_from<U, T>
		constexpr Task(Ret(T::* const memFunc)(Args...), std::shared_ptr<U>&& memFuncInstance, Args&&... args)noexcept
		{
			struct CallBack
			{
				Ret(T::* const memFunc)(Args...);
				mutable std::tuple<std::decay_t<Args>...> args;
				const S_ptr<T> memFuncCaller;
				constexpr CallBack(Ret(T::* const memFunc_)(Args...), std::shared_ptr<U>&& memFuncInstance, Args&&... args_)noexcept
					: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ std::move(memFuncInstance) } {}

				inline constexpr const void operator()()const noexcept
				{
					if constexpr (std::derived_from<T, Session>)
					{
						if (!memFuncCaller->IsConnected())
							return;
					}
					invokeMemberFunction(memFunc, memFuncCaller, std::move(args));
				}
			};
			static_assert(SIZE_OF_CALLBACK >= sizeof(CallBack));
			std::construct_at<CallBack>(reinterpret_cast<CallBack* const>(argBuff), memFunc, std::move(memFuncInstance), std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {std::destroy_at<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			//if constexpr (SIZE_OF_CALLBACK >= sizeof(CallBack))
			//{
			//	argPtr = std::construct_at<CallBack>(reinterpret_cast<CallBack* const>(argBuff), memFunc, std::move(memFuncInstance), std::forward<Args>(args)...);
			//	m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {std::destroy_at<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			//}
			//else
			//{
			//	argPtr = xnew<CallBack>(memFunc, std::move(memFuncInstance), std::forward<Args>(args)...);
			//	m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {xdelete<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			//}
			m_fpTask = [](const void* const callBackPtr_)noexcept {return static_cast<const CallBack* const>(callBackPtr_)->operator()(); };
		}
		template<typename T, typename U, typename Ret, typename... Args> requires EnableSharedFromThis<U> && std::derived_from<U, T>
		constexpr Task(Ret(T::* const memFunc)(Args...), U* const memFuncInstance, Args&&... args)noexcept
		{
			struct CallBack
			{
				Ret(T::* const memFunc)(Args...);
				mutable std::tuple<std::decay_t<Args>...> args;
				const S_ptr<T> memFuncCaller;
				constexpr CallBack(Ret(T::* const memFunc_)(Args...), U* const memFuncInstance, Args&&... args_)noexcept
					: memFunc{ memFunc_ }, args{ std::forward<Args>(args_)... }, memFuncCaller{ memFuncInstance->shared_from_this(), static_cast<T* const>(memFuncInstance) } {}

				inline constexpr const void operator()()const noexcept
				{
					if constexpr (std::derived_from<T, Session>)
					{
						if (!memFuncCaller->IsConnected())
							return;
					}
					invokeMemberFunction(memFunc, memFuncCaller, std::move(args));
				}
			};
			static_assert(SIZE_OF_CALLBACK >= sizeof(CallBack));
			std::construct_at<CallBack>(reinterpret_cast<CallBack* const>(argBuff), memFunc, memFuncInstance, std::forward<Args>(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {std::destroy_at<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			//if constexpr (SIZE_OF_CALLBACK >= sizeof(CallBack))
			//{
			//	argPtr = std::construct_at<CallBack>(reinterpret_cast<CallBack* const>(argBuff), memFunc, memFuncInstance, std::forward<Args>(args)...);
			//	m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {std::destroy_at<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			//}
			//else
			//{
			//	argPtr = xnew<CallBack>(memFunc, memFuncInstance, std::forward<Args>(args)...);
			//	m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {xdelete<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			//}
			m_fpTask = [](const void* const callBackPtr_)noexcept {return static_cast<const CallBack* const>(callBackPtr_)->operator()(); };
		}
		template<typename Func, typename... Args> requires std::invocable<Func, Args...> && IsNotMemFunc<Func>
		constexpr Task(Func&& fp, std::decay_t<Args>... args)noexcept
		{
			static_assert(!std::is_member_pointer_v<Func>);
			static_assert(IsNotMemFunc<Func>);
			struct CallBack
			{
				mutable Func fp;
				mutable std::tuple<std::decay_t<Args>...> args;
				constexpr CallBack(Func&& fp_, std::decay_t<Args>&&... args_)noexcept
					: fp{ std::forward<Func>(fp_) }, args{ std::move(args_)... } {}
				inline constexpr void operator()()const noexcept
				{
					std::apply(std::move(fp), std::move(args));
				}
			};
			static_assert(SIZE_OF_CALLBACK >= sizeof(CallBack));
			std::construct_at<CallBack>(reinterpret_cast<CallBack* const>(argBuff), std::forward<Func>(fp), std::move(args)...);
			m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {std::destroy_at<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			//if constexpr (SIZE_OF_CALLBACK >= sizeof(CallBack))
			//{
			//	argPtr = std::construct_at<CallBack>(reinterpret_cast<CallBack* const>(argBuff), std::forward<Func>(fp), std::forward<Args>(args)...);
			//	m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {std::destroy_at<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			//}
			//else
			//{
			//	argPtr = xnew<CallBack>(std::forward<Func>(fp), std::forward<Args>(args)...);
			//	m_fpTaskDeleter = [](void* const callBackPtr_)noexcept {xdelete<CallBack>(static_cast<CallBack* const>(callBackPtr_)); };
			//}
			m_fpTask = [](const void* const callBackPtr_)noexcept {return static_cast<const CallBack* const>(callBackPtr_)->operator()(); };
		}
		inline constexpr void ExecuteTask()const noexcept { m_fpTask(static_cast<const void* const>(argBuff)); }
	private:
		//void* argPtr;
		void(*m_fpTask)(const void* const)noexcept;
		void(*m_fpTaskDeleter)(void* const)noexcept;
		BYTE argBuff[SIZE_OF_CALLBACK];

	private:
		template<typename Function, typename T, typename Tuple, size_t... I>
		constexpr static inline const auto CallFunctionWithTuple(const Function func, T&& obj, Tuple&& tup, std::index_sequence<I...>) noexcept {
			return (*std::forward<T>(obj).*func)(std::get<I>(std::forward<Tuple>(tup))...);
		}

		template<typename Function, typename T>
		constexpr static inline const auto CallFunctionWithTuple(const Function func, T&& obj, std::tuple<>&&, std::index_sequence<>) noexcept {
			return (*std::forward<T>(obj).*func)();
		}

		template<typename Function, typename T, typename Tuple, typename Indices = std::make_index_sequence<std::tuple_size_v<std::decay_t<Tuple>>>>
		constexpr static inline const auto invokeMemberFunction(const Function func, T&& obj, Tuple&& tup) noexcept {
			return CallFunctionWithTuple(func, std::forward<T>(obj), std::forward<Tuple>(tup), Indices{});
		}
	};
}
