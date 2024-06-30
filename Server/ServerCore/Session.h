#pragma once
#include "IocpObject.h"
#include "RecvBuffer.h"
#include "ID_Ptr.hpp"

namespace ServerCore
{
	class RecvEvent;
	class Service;
	class DisconnectEvent;
	class ConnectEvent;
	class RecvBuffer;
	class SendBuffer;
	class SendEvent;
	class PacketSession;

	/*--------------
		 Session
	---------------*/


	class Session
		:public IocpEntity
	{
		friend class Service;
		friend class Listener;
		friend class IocpCore;
		friend class PacketSession;
		using PacketHandleFunc = const bool(*)(const S_ptr<PacketSession>&, BYTE* const, c_int32);
	public:
		Session(const PacketHandleFunc* const sessionPacketHandler_)noexcept;
		~Session();
		Session(const Session&) = delete;
		Session& operator=(const Session&) = delete;
		template <typename T> requires std::convertible_to<T, S_ptr<PacketSession>>
		static inline const uint64 GetID(const T& __restrict pSession_)noexcept { return pSession_->GetSessionID(); }
	public:
		virtual const bool IsValid()const noexcept override { return IsConnectedAtomic(); }
		void SendAsync(S_ptr<SendBuffer> pSendBuff_)noexcept
		{
			m_sendQueue.emplace(std::move(pSendBuff_));
			TrySend();
		}
		inline void TrySend()noexcept
		{
			if (false == m_bIsSendRegistered.exchange(true, std::memory_order_relaxed))
			{
				std::atomic_thread_fence(std::memory_order_acquire);
				m_pSendEvent->m_registerSendEvent.SetIocpObject(SharedFromThis());
				::PostQueuedCompletionStatus(Mgr(ThreadMgr)->GetIocpHandle(), 0, 0, reinterpret_cast<IocpEvent* const>(reinterpret_cast<char* const>(m_pSendEvent.get()) + sizeof(IocpEvent) + sizeof(Vector<S_ptr<SendBuffer>>)));
			}
		}
		template <typename S_ptr_SendBuffer>
		void SendOnlyEnqueue(S_ptr_SendBuffer&& pSendBuff_)noexcept
		{
			m_sendQueue.emplace(std::forward<S_ptr_SendBuffer>(pSendBuff_));
		}
		bool Connect();

		bool Disconnect(const std::wstring_view cause)noexcept;

		const uint64 GetSessionID()const noexcept { return GetObjectID(); }
		const uint64 GetSessionIDAndAlive()const noexcept {
			std::atomic_thread_fence(std::memory_order_acquire);
			return GetObjectID() * m_bConnected;
		}
		Service* const GetService()const noexcept { return m_pService; }
		void SetService(Service* const pService_)noexcept { m_pService = pService_; }
		bool SetNagle(const bool bTrueIsOff_FalseIsOn)const noexcept;
		const bool CanRegisterSend()const noexcept { return !m_bIsSendRegistered.load(std::memory_order_relaxed); }
	public:
		void SetNetAddress(NetAddress netAddr_)noexcept { m_sessionAddr = netAddr_; }
		NetAddress GetAddress()const noexcept{ return m_sessionAddr; }
		SOCKET GetSocket()const noexcept{ return m_sessionSocket; }
		const bool IsConnected()const noexcept {
			return m_bConnectedNonAtomic;
		}
		const bool IsConnectedAtomic()const noexcept { return m_bConnected.load(std::memory_order_relaxed); }
		const bool IsHeartBeatAlive()const noexcept {
			return m_bHeartBeatAlive;
		}
		void SetHeartBeat(const bool bHeartBeat_)noexcept {
			m_bHeartBeatAlive = bHeartBeat_;
		}
		void SetLastError(c_int32 errCode_)noexcept { m_iLastErrorCode *= errCode_; }
	private:
		virtual HANDLE GetHandle()const noexcept override { return  reinterpret_cast<HANDLE>(m_sessionSocket); }
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;
	private:
		bool RegisterConnect();
		void ProcessConnect(const S_ptr<PacketSession>& pThisSessionPtr, c_int32 numofBytes_ = 0)noexcept;


		bool RegisterDisconnect()noexcept;
		void ProcessDisconnect(const S_ptr<PacketSession>& pThisSessionPtr, c_int32 numofBytes_ = 0)noexcept;

		void RegisterRecv(const S_ptr<PacketSession>& pThisSessionPtr)noexcept;
		void ProcessRecv(const S_ptr<PacketSession>& pThisSessionPtr, c_int32 numofBytes_)noexcept;

		void RegisterSend(const S_ptr<PacketSession>& pThisSessionPtr)noexcept;
		void ProcessSend(const S_ptr<PacketSession>& pThisSessionPtr, c_int32 numofBytes_)noexcept;

		void HandleError(c_int32 errorCode);

		inline void TryRegisterSend(const S_ptr<PacketSession>& pThisSessionPtr, c_int32 numofBytes_ = 0)noexcept
		{
			RegisterSend(pThisSessionPtr);
		}
	protected:
		// 컨텐츠단에서 구현 할 내용들 (오버라이딩)
		virtual void OnConnected() abstract;
		virtual const RecvStatus OnRecv(BYTE* const buffer, c_int32 len, const S_ptr<PacketSession>& pThisSessionPtr)noexcept abstract;
		virtual void OnSend(c_int32 len)noexcept abstract;
		virtual void OnDisconnected()abstract;

	private:
		MPSCQueue<S_ptr<SendBuffer>> m_sendQueue;
		const U_Pptr<SendEvent> m_pSendEvent;
		Atomic<bool> m_bIsSendRegistered = false;
		volatile bool m_bConnectedNonAtomic = false;
		SOCKET m_sessionSocket = INVALID_SOCKET;

		Service* m_pService;
		const U_Pptr<DisconnectEvent> m_pDisconnectEvent;
		NetAddress m_sessionAddr;

		volatile bool m_bConnectedNonAtomicForRecv = false;
		SOCKET m_sessionSocketForRecv = INVALID_SOCKET;
		const U_Pptr<RecvEvent> m_pRecvEvent;
		const U_Pptr<RecvBuffer> m_pRecvBuffer;
		const PacketHandleFunc* const __restrict m_sessionPacketHandler;
	private:
		std::atomic_int m_serviceIdx = -1;
		Atomic<bool>m_bConnected = false;
		volatile bool m_bHeartBeatAlive = true;
		int32 m_iLastErrorCode = 1;
		const U_ptr<ConnectEvent> m_pConnectEvent;
	private:
		constexpr static inline void (Session::* const g_sessionLookupTable[etoi(EVENT_TYPE::REGISTER_SEND) + 1])(const S_ptr<PacketSession>&, c_int32)noexcept =
		{
			&Session::ProcessConnect,
			&Session::ProcessDisconnect,
			&Session::ProcessRecv,
			&Session::ProcessSend,
			&Session::TryRegisterSend,
		};
	};
}
