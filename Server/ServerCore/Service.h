#pragma once
#include "NetAddress.h"

namespace ServerCore
{
	class Session;
	class IocpCore;
	class Listener;

	enum class SERVICE_TYPE : uint8
	{
		SERVER,
		CLIENT,
		END
	};

	/*-------------------
		Service
	-------------------*/

	using SessionFactory = std::function<S_ptr<Session>(void)>;

	class Service
	{
		struct alignas(64) AtomicSessionPtr
		{
			std::atomic<S_ptr<Session>> ptr;
		};
	public:
		Service(const std::shared_ptr<IocpCore>& pIocp_, SERVICE_TYPE eServiceType_, NetAddress addr_, SessionFactory factory_, c_int32 maxSessionCount_ = 1);
		virtual ~Service();

	public:
		virtual bool Start()abstract;
		virtual void CloseService();
		bool CanStart()const { return nullptr != m_sessionFactory; }

		S_ptr<Session> CreateSession()noexcept;
		const bool AddSession(S_ptr<Session>&& pSession_)noexcept;
		void ReleaseSession(Session* const pSession_)noexcept;
		//int32 GetCurrentSessionCount()const noexcept { return m_sessionCount; }
		int32 GetMaxSessionCount()const noexcept { return m_maxSessionCount; }
		SERVICE_TYPE GetServiceType()const noexcept { return m_eServiceType; }
		NetAddress GetNetAddress()const noexcept { return m_netAddr; }
		const S_ptr<IocpCore>& GetIocpCore()const noexcept { return m_pIocpCore; }
		S_ptr<Session> GetSession(const uint64_t sessionID_)noexcept;
	public:
		void IterateSession(std::function<void(const S_ptr<Session>&)> fpIterate_)noexcept;
	protected:
		Concurrency::concurrent_unordered_map<
			uint32_t,
			uint16_t,
			std::hash<uint32_t>,
			std::equal_to<uint32_t>,
			StlAllocator<std::pair<const uint32_t, uint16_t>>> m_id2Index;
		const std::span<AtomicSessionPtr> m_vecSession;
		Concurrency::concurrent_queue<int32, StlAllocator<int32>> m_idxQueue;

		const S_ptr<IocpCore> m_pIocpCore;
		const SERVICE_TYPE m_eServiceType;
		const NetAddress m_netAddr;
		const SessionFactory m_sessionFactory;
		const int32 m_maxSessionCount;

		template<typename T>
		static const std::span<T> CreateDynamicSpan(const size_t size_)noexcept {
			const auto ptr = new T[size_]{};
			return { ptr,ptr + size_ };
		}
	};


	/*-------------------
		ClientService
	-------------------*/
	class ClientService
		:public Service
	{
	public:
		ClientService(const std::shared_ptr<IocpCore>& pIocp_, NetAddress targetServerAddr_, SessionFactory factory_, c_int32 maxSessionCount_ = 1);
		virtual ~ClientService();
	public:
		virtual bool Start()override;
		virtual void CloseService()override;
	private:

	};


	/*-------------------
		ServerService
	-------------------*/

	class ServerService
		:public Service
	{
	public:
		ServerService(const std::shared_ptr<IocpCore>& pIocp_, NetAddress addr_, SessionFactory factory_, c_int32 maxSessionCount_ = 1);
		virtual ~ServerService();
	public:
		virtual bool Start()override;
		virtual void CloseService()override;
	private:
		const S_ptr<Listener> m_pListener;
	};
}

