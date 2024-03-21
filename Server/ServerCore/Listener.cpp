#include "ServerCorePch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpCore.h"
#include "IocpEvent.h"
#include "Session.h"
#include "Service.h"
#include "RecvBuffer.h"
#include "PacketSession.h"

namespace ServerCore
{
	Listener::Listener()
	{
	}

	Listener::~Listener()
	{
		CancelIoEx(reinterpret_cast<HANDLE>(m_socket), NULL);
		shutdown(m_socket, SD_BOTH);
		SocketUtils::Close(m_socket);
	}

	bool Listener::StartAccept(ServerService* const pServerService_)
	{
		m_pServerService = pServerService_;

		if (nullptr == m_pServerService)
			return false;
		// 영업개시
		m_socket = SocketUtils::CreateSocket();

		if (INVALID_SOCKET == m_socket)
			return false;
		if (false == m_pServerService->GetIocpCore()->RegisterIOCP(this))
			return false;
		if (false == SocketUtils::SetReuseAddress(m_socket, true))
			return false;
		if (false == SocketUtils::SetLinger(m_socket, 0, 0))
			return false;
		if (false == SocketUtils::Bind(m_socket, m_pServerService->GetNetAddress()))
			return false;
		if (false == SocketUtils::Listen(m_socket))
			return false;

		constexpr const int32 acceptCount = ThreadMgr::NUM_OF_THREADS;

		for (int i = 0; i < acceptCount; ++i)
		{
			auto acceptEvent = MakeShared<AcceptEvent>();
			acceptEvent->SetIocpObject(shared_from_this());
			RegisterAccept(acceptEvent.get());
			m_vecAcceptEvent.emplace_back(std::move(acceptEvent));
		}

		return true;
	}

	void Listener::CloseAccept()
	{
		// TODO: 연결 안받기 , 다시받기
		m_bCanAccept.store(false);
		std::atomic_thread_fence(std::memory_order_seq_cst);
	}

	void Listener::FinishServer() noexcept
	{
		for (auto& accepts : m_vecAcceptEvent)
		{
			if (const auto pSession = accepts->ReleaseSession())pSession->reset_cache_shared();
			accepts->ReleaseIocpObject();
		}
		reset_cache_shared();
	}

	void Listener::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept
	{
		NAGOX_ASSERT(iocpEvent_->GetEventType() == EVENT_TYPE::ACCEPT);
		const auto acceptEvent = iocpEvent_->Cast<AcceptEvent>();
		const S_ptr<PacketSession> pSession{ std::static_pointer_cast<PacketSession>(acceptEvent->PassSession()) };
		ProcessAccept(pSession, acceptEvent);
	}

	void Listener::RegisterAccept(AcceptEvent* const acceptEvent)noexcept
	{
		if (false == CanAccept())
			return;

		const S_ptr<Session> session = m_pServerService->CreateSession();

		acceptEvent->Init();
		acceptEvent->RegisterSession(session);

		DWORD bytesReceived;
		if (false == SocketUtils::AcceptEx(m_socket, session->GetSocket(), session->m_pRecvBuffer->WritePos(), 0,
			sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT & bytesReceived, acceptEvent))
		{
			const int32 errCode = ::WSAGetLastError();
			if (errCode != WSA_IO_PENDING)
			{
				// 일단 다시 Accept (입질이 왔는데 뭔가 문제가있음)
				acceptEvent->ReleaseSession();
				RegisterAccept(acceptEvent);
			}
		}
	}

	void Listener::ProcessAccept(const S_ptr<class PacketSession>& pSession, AcceptEvent* const acceptEvent)noexcept
	{
		if (!pSession)
		{
			return;
		}

		const SOCKET sessionSocket = pSession->GetSocket();

		if (false == SocketUtils::SetUpdateAcceptSocket(sessionSocket, m_socket))
		{
			RegisterAccept(acceptEvent);
			pSession->reset_cache_shared(*this);
			return;
		}

		if (false == SocketUtils::SetTcpNoDelay(sessionSocket, true))
		{
			RegisterAccept(acceptEvent);
			pSession->reset_cache_shared(*this);
			return;
		}

		SOCKADDR_IN sockAddress;
		int32 sizeOfSockAddr = sizeof(sockAddress);
		if (SOCKET_ERROR == ::getpeername(sessionSocket, reinterpret_cast<SOCKADDR* const>(&sockAddress), &sizeOfSockAddr))
		{
			RegisterAccept(acceptEvent);
			pSession->reset_cache_shared(*this);
			return;
		}

		pSession->SetNetAddress(NetAddress{ sockAddress });
		pSession->ProcessConnect(pSession);

		if (pSession->IsConnected())
		{
			LOG_MSG(L"client in");
		}
		else
		{
			LOG_MSG(L"Server Is Full");
			pSession->reset_cache_shared(*this);
			// TODO: 입장 정원 초과 메시지 보내기 또는 현재 더 받을 여유가 없음
			//std::this_thread::sleep_for(std::chrono::seconds(3));
		}

		RegisterAccept(acceptEvent);
	}
}