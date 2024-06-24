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
		, m_vecSession{ CreateDynamicSpan<AtomicSessionPtr>(maxSessionCount_ + 1) }
	{
		for (int i = 1; i <= m_maxSessionCount; ++i)
		{
			m_idxQueue.push(i);
		}
	}

	Service::~Service()
	{
		delete m_vecSession.data();
	}

	void Service::CloseService()
	{
		// TODO: ¾î¶»°Ô ¼Ò¸ê½ÃÅ³Áö °í¹Î
		IterateSession([](const S_ptr<Session>& p)noexcept {p->Disconnect(L"Bye"); });
		//reset_cache_shared();
	}

	S_ptr<Session> Service::CreateSession()noexcept
	{
		auto pSession = m_sessionFactory();
		pSession->SetService(this);
		
		return m_pIocpCore->RegisterIOCP(pSession.get(), pSession->GetSessionID()) ? std::move(pSession) : nullptr;
	}

	const bool Service::AddSession(S_ptr<Session> pSession_)noexcept
	{
		int32 idx;
		if (!m_idxQueue.try_pop(idx))
			return false;
		m_id2Index[static_cast<c_uint32>(pSession_->GetSessionID())] = static_cast<c_uint16>(idx);
		pSession_->m_serviceIdx.store(idx, std::memory_order_relaxed);
		m_vecSession[idx].ptr.store(std::move(pSession_));
		return true;

		return true;
	}

	void Service::ReleaseSession(Session* const pSession_) noexcept
	{
		const int32 idx = pSession_->m_serviceIdx.exchange(-1, std::memory_order_relaxed);
		if (-1 == idx)
			return;
		//m_vecSession[idx].ptr.store(nullptr, std::memory_order_relaxed);
		pSession_->DecRef();
		m_vecSession[idx].ptr.reset();
		
		m_idxQueue.push(idx);
	}

	S_ptr<Session> Service::GetSession(const uint64_t sessionID_)noexcept
	{
		const int32 idx = m_id2Index[static_cast<c_uint32>(sessionID_)];
		auto target = m_vecSession[idx].ptr.load();
		if (target && target->GetSessionID() == sessionID_)
			return target;
		else
			return nullptr;
	}

	void Service::IterateSession(std::function<void(const S_ptr<Session>&)> fpIterate_)noexcept
	{
		for (const auto& pSession_ : m_vecSession)
		{
			const auto pSession = pSession_.ptr.load();
			if (pSession)
				fpIterate_(pSession);
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
			//pSession->register_cache_shared_core(pSession);
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
		//m_pListener->register_cache_shared_core(m_pListener);
		return m_pListener->StartAccept(this);
	}

	void ServerService::CloseService()
	{
		m_pListener->CloseAccept();
		Service::CloseService();
	}
}