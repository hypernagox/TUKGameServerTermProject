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
		// ��������
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
		// TODO: ���� �ȹޱ� , �ٽùޱ�
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

		//DWORD bytesReceived;
		if (false == SocketUtils::AcceptEx(m_socket, session->GetSocket(), session->m_pRecvBuffer->WritePos(), 0,
			sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, NULL, acceptEvent))
		{
			const int32 errCode = ::WSAGetLastError();
			if (errCode != WSA_IO_PENDING)
			{
				// �ϴ� �ٽ� Accept (������ �Դµ� ���� ����������)
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
		const auto session_ptr = pSession.get();
		const SOCKET sessionSocket = pSession->GetSocket();

		if (false == SocketUtils::SetUpdateAcceptSocket(sessionSocket, m_socket))
		{
			RegisterAccept(acceptEvent);
			session_ptr->reset_cache_shared(*this);
			return;
		}

		if (false == SocketUtils::SetTcpNoDelay(sessionSocket, true))
		{
			RegisterAccept(acceptEvent);
			session_ptr->reset_cache_shared(*this);
			return;
		}

		SOCKADDR_IN sockAddress;
		int32 sizeOfSockAddr = sizeof(sockAddress);
		if (SOCKET_ERROR == ::getpeername(sessionSocket, reinterpret_cast<SOCKADDR* const>(&sockAddress), &sizeOfSockAddr))
		{
			RegisterAccept(acceptEvent);
			session_ptr->reset_cache_shared(*this);
			return;
		}

		session_ptr->SetNetAddress(NetAddress{ sockAddress });
		session_ptr->ProcessConnect(pSession);

		if (session_ptr->IsConnected())
		{
			LOG_MSG(L"client in");
		}
		else
		{
			LOG_MSG(L"Server Is Full");
			session_ptr->reset_cache_shared(*this);
			// TODO: ���� ���� �ʰ� �޽��� ������ �Ǵ� ���� �� ���� ������ ����
			//std::this_thread::sleep_for(std::chrono::seconds(3));
		}

		RegisterAccept(acceptEvent);
	}
}