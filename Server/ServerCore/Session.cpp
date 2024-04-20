#include "ServerCorePch.h"
#include "Session.h"
#include "SocketUtils.h"
#include "Service.h"
#include "IocpEvent.h"
#include "RecvBuffer.h"
#include "SendBuffer.h"
#include "PacketSession.h"

namespace ServerCore
{
	Session::Session(const PacketHandleFunc* const sessionPacketHandler_)noexcept
		: m_pRecvEvent{ MakeUnique<RecvEvent>() }
		, m_pConnectEvent{ MakeUnique<ConnectEvent>() }
		, m_pDisconnectEvent{ MakeUnique<DisconnectEvent>() }
		, m_pSendEvent{ MakeUnique<SendEvent>() }
		, m_pRecvBuffer{ MakePoolUnique<RecvBuffer>(RecvBuffer::BUFFER_SIZE) }
		, m_sessionSocket{ SocketUtils::CreateSocket() }
		, m_iSessionID{ IDGenerator::GenerateID() }
		, m_sessionPacketHandler{ sessionPacketHandler_ }
		, m_sessionSocketForRecv{ m_sessionSocket }
	{
	}

	Session::~Session()
	{
		//CancelIoEx(reinterpret_cast<HANDLE>(m_sessionSocket), NULL);
		//shutdown(m_sessionSocket, SD_BOTH);
		SocketUtils::Close(m_sessionSocket);
		m_sessionSocketForRecv = INVALID_SOCKET;
	}

	bool Session::Connect()
	{
		return RegisterConnect();
	}

	void Session::Disconnect(std::wstring cause)
	{
		LOG_MSG(std::move(cause));
		if (false == m_bConnected.exchange(false,std::memory_order_relaxed))
			return;
		m_bConnectedNonAtomic = m_bConnectedNonAtomicForRecv = false;
		std::atomic_thread_fence(std::memory_order_seq_cst);
		RegisterDisconnect();
	}

	bool Session::SetNagle(const bool bTrueIsOff_FalseIsOn)const noexcept
	{
		return SocketUtils::SetTcpNoDelay(m_sessionSocket, bTrueIsOff_FalseIsOn);
	}

	void Session::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept
	{
		if (S_ptr<PacketSession> pThisSessionPtr{ iocpEvent_->PassIocpObject(),static_cast<PacketSession* const>(this) })
		{
			(this->*g_sessionLookupTable[static_cast<const uint8_t>(iocpEvent_->GetEventType())])(pThisSessionPtr, numOfBytes);
		}
	}

	bool Session::RegisterConnect()
	{
		if (IsConnected())
			return false;
		if (SERVICE_TYPE::CLIENT != GetService()->GetServiceType())
			return false;
		if (false == SocketUtils::SetReuseAddress(m_sessionSocket, true))
			return false;
		if (false == SocketUtils::BindAnyAddress(m_sessionSocket, 0))
			return false;

		//register_cache_shared(); 어디선가 누가 레지스터 해주긴 해야함
		m_pConnectEvent->Init();
		m_pConnectEvent->SetIocpObject(shared_from_this());

		const SOCKADDR_IN& sockAddr = GetService()->GetNetAddress().GetSockAddr(); // 내가 붙어야 할 서버쪽 주소임

		//DWORD numOfBytes = 0;

		if (false == SocketUtils::ConnectEx(m_sessionSocket, reinterpret_cast<const SOCKADDR* const>(&sockAddr), sizeof(sockAddr), NULL, NULL, NULL, m_pConnectEvent.get()))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				m_pConnectEvent->ReleaseIocpObject();
				return false;
			}
		}

		return true;
	}

	void Session::ProcessConnect(const S_ptr<PacketSession>& pThisSessionPtr, c_int32 numofBytes_)noexcept
	{
		// 세선 등록
		if (GetService()->AddSession(pThisSessionPtr))
		{
			pThisSessionPtr->register_cache_shared_core(pThisSessionPtr);

			m_pSendEvent->m_registerSendEvent.SetIocpObject(pThisSessionPtr);

			m_bConnectedNonAtomic = m_bConnectedNonAtomicForRecv = true;
			std::atomic_thread_fence(std::memory_order_seq_cst);
			m_bConnected.store(true);
			// 컨텐츠 코드에서 오버로딩 해야함
			// 입장시 해야할 일

			OnConnected();

			// 수신 등록(낚싯대 던짐)
			RegisterRecv(pThisSessionPtr);
		}
	}

	bool Session::RegisterDisconnect()noexcept
	{
		m_pDisconnectEvent->Init();
		m_pDisconnectEvent->SetIocpObject(shared_from_this());

		if (false == SocketUtils::DisconnectEx(m_sessionSocket, m_pDisconnectEvent.get(), TF_REUSE_SOCKET, 0))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				m_bConnectedNonAtomic = m_bConnectedNonAtomicForRecv = false;
				m_bConnected.store(false);

				HandleError(errorCode);
				m_pDisconnectEvent->ReleaseIocpObject();
				return false;
			}
		}

		return true;
	}

	void Session::ProcessDisconnect(const S_ptr<PacketSession>& pThisSessionPtr, c_int32 numofBytes_)noexcept
	{
		OnDisconnected();
		GetService()->ReleaseSession(pThisSessionPtr);
		CancelIoEx(reinterpret_cast<HANDLE>(m_sessionSocket), NULL);
		m_pSendEvent->m_registerSendEvent.ReleaseIocpObject();
		// 캐시 쉐어드 리셋 해야됌
	}

	void Session::RegisterRecv(const S_ptr<PacketSession>& pThisSessionPtr)noexcept
	{
		//std::atomic_thread_fence(std::memory_order_acquire);
		if (false == m_bConnectedNonAtomicForRecv)
			return;

		m_pRecvEvent->Init();
		m_pRecvEvent->SetIocpObject(std::move(const_cast<S_ptr<PacketSession>&>(pThisSessionPtr)));

		WSABUF wsaBuf{ static_cast<const ULONG>(m_pRecvBuffer->FreeSize()),reinterpret_cast<char* const>(m_pRecvBuffer->WritePos()) };
		//DWORD numOfBytes;
		DWORD flags = 0;
		if (SOCKET_ERROR == ::WSARecv(m_sessionSocket, &wsaBuf, 1, NULL, &flags, m_pRecvEvent.get(), nullptr))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				m_bConnectedNonAtomic = m_bConnectedNonAtomicForRecv = false;
				m_bConnected.store(false);

				HandleError(errorCode);
				m_pRecvEvent->ReleaseIocpObject();
			}
		}
	}

	void Session::ProcessRecv(const S_ptr<PacketSession>& pThisSessionPtr, c_int32 numofBytes_)noexcept
	{
		if (0 == numofBytes_)
		{
			Disconnect(L"Recv 0");
			return;
		}

		if (false == m_pRecvBuffer->OnWrite(numofBytes_)) [[unlikely]]
		{
			Disconnect(L"OnWrite Overflow");
			return;
		}
		const int32 dataSize = m_pRecvBuffer->DataSize(); // 더 읽어야할 데이터 w - r
		// 컨텐츠 쪽에서 오버로딩 해야함

		const RecvStatus recvStatus = static_cast<PacketSession* const>(this)->PacketSession::OnRecv(m_pRecvBuffer->ReadPos(), dataSize, pThisSessionPtr);

		if (false == recvStatus.bIsOK || recvStatus.processLen < 0 || dataSize < recvStatus.processLen || false == m_pRecvBuffer->OnRead(recvStatus.processLen))
		{
			Disconnect(L"OnRecv Overflow");
			return;
		}

		m_pRecvBuffer->Clear(); // 커서 정리

		RegisterRecv(pThisSessionPtr);
	}

	void Session::RegisterSend(const S_ptr<PacketSession>& pThisSessionPtr)noexcept
	{
		if (false == IsConnected())
			return;

		m_pSendEvent->Init();
		m_pSendEvent->SetIocpObject(std::move(const_cast<S_ptr<PacketSession>&>(pThisSessionPtr)));

		auto& sendBuffer = m_pSendEvent->sendBuffer;
		sendBuffer.clear();
		m_sendQueue.try_flush_single(sendBuffer);

		//int32 writeSize = 0;
		// 원기옥모아서한방에 보낸다
		m_wsaBufs.clear();
		for (const auto& sb : sendBuffer)
		{
			m_wsaBufs.emplace_back(static_cast<const ULONG>(sb->WriteSize()), reinterpret_cast<char* const>(sb->Buffer()));
			//writeSize += sb->WriteSize();
		}
		//DWORD numOfBytes;
		if (SOCKET_ERROR == ::WSASend(m_sessionSocket, m_wsaBufs.data(), static_cast<const DWORD>(m_wsaBufs.size()), NULL, 0, m_pSendEvent.get(), nullptr))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{

				m_bConnectedNonAtomic = m_bConnectedNonAtomicForRecv = false;
				m_bIsSendRegistered.store(false);

				HandleError(errorCode);
				m_pSendEvent->ReleaseIocpObject();
				sendBuffer.clear();
			}
		}
	}

	void Session::ProcessSend(const S_ptr<PacketSession>& pThisSessionPtr, c_int32 numofBytes_)noexcept
	{
		if (0 == numofBytes_)
		{
			Disconnect(L"Send 0");
			return;
		}
		m_pSendEvent->m_registerSendEvent.SetIocpObject(pThisSessionPtr);

		OnSend(numofBytes_);

		m_bIsSendRegistered.store(false, std::memory_order_release);

		if (!m_sendQueue.empty_single() && false == m_bIsSendRegistered.exchange(true, std::memory_order_relaxed))
			RegisterSend(pThisSessionPtr);
	}

	void Session::HandleError(c_int32 errorCode)
	{
		switch (errorCode)
		{
		case WSAECONNRESET:
		case WSAECONNABORTED:
			//Disconnect(L"HandleError");
			break;
		default:
			// TODO 로그찍기
			//LOG_MSG(std::format(L"Handle Error: {}", errorCode));
			break;
		}
	}
}
