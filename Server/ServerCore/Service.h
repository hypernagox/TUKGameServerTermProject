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
		:public enable_shared_cache_this<Service>
	{
	public:
		Service(const std::shared_ptr<IocpCore>& pIocp_, SERVICE_TYPE eServiceType_, NetAddress addr_, SessionFactory factory_, c_int32 maxSessionCount_ = 1);
		virtual ~Service();

	public:
		virtual bool Start()abstract;
		virtual void CloseService();
		bool CanStart()const { return nullptr != m_sessionFactory; }

		S_ptr<Session> CreateSession()noexcept;
		const bool AddSession(S_ptr<Session>&& pSession_)noexcept;
		void ReleaseSession(const S_ptr<Session>& pSession_)noexcept;
		int32 GetCurrentSessionCount()const noexcept { return m_sessionCount; }
		int32 GetMaxSessionCount()const noexcept { return m_maxSessionCount; }
		SERVICE_TYPE GetServiceType()const noexcept{ return m_eServiceType; }
		NetAddress GetNetAddress()const noexcept{ return m_netAddr; }
		const S_ptr<IocpCore>& GetIocpCore()const noexcept { return m_pIocpCore; }
	public:
		void IterateSession(std::function<void(const S_ptr<Session>&)> fpIterate_)noexcept;
	protected:
		const S_ptr<IocpCore> m_pIocpCore;

		const SERVICE_TYPE m_eServiceType;
		const NetAddress m_netAddr;
		const SessionFactory m_sessionFactory;

		std::atomic<int32> m_sessionCount = 0;
		const int32 m_maxSessionCount;

		List<S_ptr<Session>> m_listSession;
		ConcurrentHashMap<uint64, decltype(m_listSession.begin())> m_mapFindSession;

		decltype(m_listSession.begin()) m_beginSentienl;
		decltype(m_listSession.begin()) m_endSentienl;
		SpinLock m_InsertLock[2];
		SpinLock m_eraseLock;
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

