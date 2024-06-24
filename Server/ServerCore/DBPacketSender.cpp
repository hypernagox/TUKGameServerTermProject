#include "ServerCorePch.h"
#include "DBPacketSender.h"

namespace ServerCore
{
    void DBPacketSender::Init() noexcept
    {
		//m_iocpEvent.SetIocpObject(shared_from_this());
		m_iocpEvent.SetIocpObject(SharedFromThis<IocpObject>());
    }

	DBPacketSender::~DBPacketSender() noexcept
	{
	}
  
    void DBPacketSender::Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
    {
		m_bRegisterSend.store(false, std::memory_order_release);

		if (!m_sendQueue.empty_single() && false == m_bRegisterSend.exchange(true, std::memory_order_relaxed))
			RegisterDBPacket();
    }

    void DBPacketSender::RegisterDBPacket() noexcept
    {
		m_iocpEvent.Init();
		thread_local Vector<S_ptr<SendBuffer>> sendBuffer;
		sendBuffer.clear();

		m_sendQueue.try_flush_single(sendBuffer);
		Vector<WSABUF> wsaBufs;
		wsaBufs.reserve(sendBuffer.size());
		for (const auto& sb : sendBuffer)
		{
			wsaBufs.emplace_back(static_cast<const ULONG>(sb->WriteSize()), reinterpret_cast<char* const>(sb->Buffer()));
		}
		
		if (SOCKET_ERROR == ::WSASend(m_queryServerSocket, wsaBufs.data(), static_cast<const DWORD>(wsaBufs.size()), NULL, 0, &m_iocpEvent, nullptr))
		{
			const int32 errorCode = ::WSAGetLastError();
			if (errorCode != WSA_IO_PENDING)
			{
				// TODO:: 에러처리
			}
		}
    }
}