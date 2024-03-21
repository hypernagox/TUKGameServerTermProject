#include "ServerCorePch.h"
#include "Service.h"
#include "IocpCore.h"
#include "Session.h"
#include "Listener.h"
#include "SendBufferMgr.h"
#include "SessionManageable.h"

namespace ServerCore
{
	Service::Service(const std::shared_ptr<IocpCore>& pIocp_, SERVICE_TYPE eServiceType_, NetAddress addr_, SessionFactory factory_, c_int32 maxSessionCount_)
		: m_pIocpCore{ pIocp_ }
		, m_eServiceType{ eServiceType_ }
		, m_netAddr{ addr_ }
		, m_sessionFactory{ std::move(factory_) }
		, m_maxSessionCount{ maxSessionCount_ }
	{
		m_listSession.emplace_back(nullptr);
		m_beginSentienl = std::prev(m_listSession.end());
		m_listSession.emplace_back(nullptr);
		m_endSentienl = std::prev(m_listSession.end());
	}

	Service::~Service()
	{

	}

	void Service::CloseService()
	{
		// TODO: ¾î¶»°Ô ¼Ò¸ê½ÃÅ³Áö °í¹Î
		IterateSession([](const S_ptr<Session>& p)noexcept {p->Disconnect(L"Bye"); });
		reset_cache_shared();
	}

	S_ptr<Session> Service::CreateSession()noexcept
	{
		auto pSession = m_sessionFactory();
		pSession->SetService(this);
		pSession->register_cache_shared_core(pSession);
		return m_pIocpCore->RegisterIOCP(pSession.get()) ? std::move(pSession) : nullptr;
	}

	const bool Service::AddSession(S_ptr<Session>&& pSession_)noexcept
	{
		if (m_maxSessionCount <= m_sessionCount.fetch_add(1, std::memory_order_acq_rel))
		{
			m_sessionCount.fetch_sub(1, std::memory_order_acq_rel);
			return false;
		}
		const bool threadNum = Mgr(ThreadMgr)->GetCurThreadID() & 1;
		const uint64 sessionID = pSession_->GetSessionID();
		decltype(m_listSession.begin()) iter;
		{
			std::scoped_lock lock{ m_InsertLock[threadNum],m_eraseLock };
			if (threadNum)
			{
				m_listSession.emplace_back(std::move(pSession_));
				iter = std::prev(m_listSession.end());
			}
			else
			{
				m_listSession.emplace_front(std::move(pSession_));
				iter = m_listSession.begin();
			}
		}
		m_mapFindSession.emplace_no_return(sessionID, std::move(iter));
		return true;
	}

	void Service::ReleaseSession(const S_ptr<Session>& pSession_)noexcept
	{
		const auto iter = m_mapFindSession.extract(pSession_->GetSessionID());
		{
			std::scoped_lock lock{ m_InsertLock[0],m_InsertLock[1],m_eraseLock };
			m_listSession.erase(iter->second);
		}
		m_sessionCount.fetch_sub(1, std::memory_order_acq_rel);
	}

	void Service::IterateSession(std::function<void(const S_ptr<Session>&)> fpIterate_)noexcept
	{
		{
			std::scoped_lock lock{ m_InsertLock[0],m_InsertLock[1],m_eraseLock };
			m_listSession.splice(m_endSentienl, m_listSession, std::next(m_endSentienl), m_listSession.end());
			m_listSession.splice(std::next(m_beginSentienl), m_listSession, m_listSession.begin(), m_beginSentienl);
		}
		{
			std::lock_guard<SpinLock> lock{ m_eraseLock };
			for (auto it = std::next(m_beginSentienl); it != m_endSentienl; ++it)
			{
				fpIterate_((*it));
			}
		}
	}



	ClientService::ClientService(const std::shared_ptr<IocpCore>& pIocp_, NetAddress targetServerAddr_, SessionFactory factory_, c_int32 maxSessionCount_)
		:Service{ pIocp_,SERVICE_TYPE::CLIENT,targetServerAddr_,std::move(factory_),maxSessionCount_ }
	{
	}

	ClientService::~ClientService()
	{
	}

	bool ClientService::Start()
	{
		if (false == CanStart())
			return false;
		const int32 sessionCount = GetMaxSessionCount();
		for (int i = 0; i < sessionCount; ++i)
		{
			auto pSession = CreateSession();
			pSession->register_cache_shared_core(pSession);
			if (false == pSession->Connect())
				return false;
		}
		return true;
	}

	void ClientService::CloseService()
	{
		Service::CloseService();
	}



	ServerService::ServerService(const std::shared_ptr<IocpCore>& pIocp_, NetAddress addr_, SessionFactory factory_, c_int32 maxSessionCount_)
		: Service{ pIocp_,SERVICE_TYPE::SERVER,addr_,std::move(factory_),maxSessionCount_ }
		, m_pListener{ MakeShared<Listener>() }
	{
	}

	ServerService::~ServerService()
	{
		m_pListener->FinishServer();
	}

	bool ServerService::Start()
	{
		if (false == CanStart())
			return false;
		if (!m_pListener)
			return false;
		m_pListener->register_cache_shared_core(m_pListener);
		return m_pListener->StartAccept(this);
	}

	void ServerService::CloseService()
	{
		m_pListener->CloseAccept();
		Service::CloseService();
	}
}