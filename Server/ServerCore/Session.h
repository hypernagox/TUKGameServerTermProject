#pragma once
#include "NetAddress.h"
#include "IocpObject.h"
#include "MPSCQueue.hpp"
#include "RecvBuffer.h"

namespace ServerCore
{
	class RecvEvent;
	class Service;
	class DisconnectEvent;
	class ConnectEvent;
	class RecvBuffer;
	class SendBuffer;
	class SendEvent;
	class SessionManageable;
	class PacketSession;

	/*--------------
		 Session
	---------------*/


	class Session
		:public IocpObject
	{
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
		void SendAsync(S_ptr<SendBuffer> pSendBuff_)noexcept
		{
			m_sendQueue.emplace(std::move(pSendBuff_));
			if (false == m_bIsSendRegistered.exchange(true, std::memory_order_relaxed))
			{
				std::atomic_thread_fence(std::memory_order_acquire);
				::PostQueuedCompletionStatus(Mgr(ThreadMgr)->GetIocpHandle(), 0, 0, reinterpret_cast<IocpEvent* const>(reinterpret_cast<char* const>(m_pSendEvent.get()) + sizeof(IocpEvent)));
			}
		}

		void DisconnectAsync(std::wstring cause)noexcept
		{
			Mgr(ThreadMgr)->EnqueueGlobalTask(PoolNew<Task>(&Session::Disconnect, this->SharedCastThis<Session>(), std::move(cause)));
		}
		template <typename S_ptr_SendBuffer>
		void SendOnlyEnqueue(S_ptr_SendBuffer&& pSendBuff_)noexcept
		{
			m_sendQueue.emplace(std::forward<S_ptr_SendBuffer>(pSendBuff_));
		}
		bool Connect();
		void Disconnect(std::wstring cause);
		constexpr const uint64 GetSessionID()const noexcept { return m_iSessionID; }
		const uint64 GetSessionIDAndAlive()const noexcept {
			std::atomic_thread_fence(std::memory_order_acquire);
			return m_iSessionID * m_bConnected;
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
			//std::atomic_thread_fence(std::memory_order_acquire);
			return m_bConnectedNonAtomic;
		}
		const bool IsHeartBeatAlive()const noexcept {
			//std::atomic_thread_fence(std::memory_order_acquire);
			return m_bHeartBeatAlive;
		}
		void SetHeartBeat(const bool bHeartBeat_)noexcept {
			m_bHeartBeatAlive = bHeartBeat_;
			//std::atomic_thread_fence(std::memory_order_release);
		}
		void SetLastError(c_int32 errCode_)noexcept { m_iLastErrorCode *= errCode_; }
		const ID_Ptr<SessionManageable> GetCurrentSessionRoomInfo()const noexcept { return m_CurrentSessionRoomInfo.load(std::memory_order_acquire); }
		void SetSessionRoomInfo(const uint16_t roomID_, const SessionManageable* const pSessionRoom_)noexcept { m_CurrentSessionRoomInfo.store(ID_Ptr<SessionManageable>{ roomID_, pSessionRoom_ }, std::memory_order_release); }
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
		const U_ptr<SendEvent> m_pSendEvent;
		Atomic<bool> m_bIsSendRegistered = false;
		volatile bool m_bConnectedNonAtomic = false;
		Vector<WSABUF> m_wsaBufs;
		SOCKET m_sessionSocket = INVALID_SOCKET;

		Service* m_pService;
		const U_ptr<DisconnectEvent> m_pDisconnectEvent;
		NetAddress m_sessionAddr;

		volatile bool m_bConnectedNonAtomicForRecv = false;
		SOCKET m_sessionSocketForRecv = INVALID_SOCKET;
		const U_ptr<RecvEvent> m_pRecvEvent;
		const U_Pptr<RecvBuffer> m_pRecvBuffer;
		const PacketHandleFunc* const __restrict m_sessionPacketHandler;

		const uint64 m_iSessionID;
		std::atomic<ID_Ptr<SessionManageable>> m_CurrentSessionRoomInfo;
	private:
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
