#pragma once

namespace ServerCore
{
	class Session;
	class SendBuffer;
	using drop_range = std::ranges::drop_view<std::ranges::ref_view<std::list<ServerCore::Session*, ServerCore::AtomicAllocator<ServerCore::Session*>>>>;
	class SessionManageable
		:public TaskQueueable
	{
		enum { HEART_BEAT_TICK = 15000 };
	public:
		SessionManageable(const uint16_t roomID_)noexcept;
		virtual ~SessionManageable()noexcept;
	public:
		const uint16_t GetRoomID()const noexcept { return m_roomID; }
		const auto& GetSessionList()const noexcept { return m_linkedHashMapForSession.GetItemListRef(); }
		void SendEnqueue(S_ptr<Session> pSession_, S_ptr<SendBuffer> pSendBuffer_)noexcept;
		void EnterEnqueue(S_ptr<Session> pSession_)noexcept;
		void LeaveAndDisconnectEnqueue(S_ptr<Session> pSession_)noexcept;
		void BroadCastEnqueue(S_ptr<SendBuffer> pSendBuffer)noexcept;
		void BroadCastEnqueue(S_ptr<SendBuffer> pSendBuffer, c_uint64 exceptSessionNumber)noexcept;
		S_ptr<Session> FindSession(const uint64 sessionID) const noexcept;
		void ImmigrationEnqueue(S_ptr<SessionManageable> pOtherRoom, c_uint64 sessionID)noexcept;
		void ImmigrationAllEnqueue(S_ptr<SessionManageable> pOtherRoom, const bool bDestroyCurrentRoom)noexcept;
	protected:
		drop_range GetSessionRangeExceptOne(c_uint64 exceptSessionNumber_)noexcept;
	private:
		void Enter(S_ptr<Session> pSession_)noexcept;
		void LeaveAndDisconnect(const S_ptr<Session> pSession_)noexcept;
		void Immigration(const S_ptr<SessionManageable> pOtherRoom, const uint64 sessionID) noexcept;
		void ImmigrationAll(const S_ptr<SessionManageable> pOtherRoom, const bool bDestroyCurrentRoom)noexcept;
		void BroadCast(const S_ptr<SendBuffer> pSendBuffer)noexcept;
		void BroadCastExceptOne(const S_ptr<SendBuffer> pSendBuffer, const uint64 exceptSessionNumber)noexcept;
		void RegisterHeartBeat()noexcept;
		void ListenHeartBeat()noexcept;
	private:
		LinkedHashMap<uint64, Session> m_linkedHashMapForSession;
		Vector<Task*> m_vecTaskBulks;
		const uint16_t m_roomID;
	};
}
