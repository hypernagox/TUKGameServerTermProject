#pragma once
#include "ServerCorePch.h"
#include "SlimLinkedHashMap.hpp"

namespace ServerCore
{
	class Session;
	class IocpEntity;
	class SendBuffer;

	class Sector
		:public TaskQueueable
	{
		enum { HEART_BEAT_TICK = 15000 };
	public:
		Sector(const uint16_t sector_id)noexcept;
		virtual ~Sector()noexcept;
		constexpr inline auto& GetSRWLock()noexcept { return m_linkedHashMapForIocpEntity.GetSRWLock(); }
		void lock()noexcept { GetSRWLock().lock(); }
		void unlock()noexcept { GetSRWLock().unlock(); }
		void lock_shared()noexcept { GetSRWLock().lock_shared(); }
		void unlock_shared()noexcept { GetSRWLock().unlock_shared(); }

		constexpr inline auto& GetSessionSRWLock()noexcept { return m_linkedHashMapForSession.GetSRWLock(); }
		void session_lock()noexcept { GetSessionSRWLock().lock(); }
		void session_unlock()noexcept { GetSessionSRWLock().unlock(); }
		void session_lock_shared()noexcept { GetSessionSRWLock().lock_shared(); }
		void session_unlock_shared()noexcept { GetSessionSRWLock().unlock_shared(); }
		const uint16_t GetSectorID()const noexcept { return m_sectorID; }
	public:
		const auto& GetSessionList()const noexcept { return m_linkedHashMapForSession.GetItemListRef(); }
		const auto& GetObjectList()const noexcept { return m_linkedHashMapForIocpEntity.GetItemListRef(); }
		void EnterEnqueue(IocpEntity* const pEntity_)noexcept;
		void LeaveAndDisconnectEnqueue(const uint64_t obj_id)noexcept;
		void BroadCastEnqueue(S_ptr<SendBuffer> pSendBuffer)noexcept;
		void BroadCastEnqueue(S_ptr<SendBuffer> pSendBuffer, c_uint64 exceptSessionNumber)noexcept;
		void ImmigrationEnqueue(S_ptr<Sector> pOtherSector, c_uint64 obj_id)noexcept;
		void ImmigrationAllEnqueue(S_ptr<Sector> pOtherSector, const bool bDestroyCurrentSector)noexcept;
		const int MoveBroadCast(
			const S_ptr<IocpEntity>& move_session,
			const S_ptr<SendBuffer>& in_pkt,
			const S_ptr<SendBuffer>& out_pkt,
			const S_ptr<SendBuffer>& move_pkt,
			const Vector<Sector*>* sectors
		)noexcept;
	protected:
		virtual void ImigrationAfterBehavior(const S_ptr<ServerCore::Sector> beforeSector, IocpEntity* const pEntity_)noexcept abstract;
	private:
		void Enter(IocpEntity* const pEntity_)noexcept;
		void LeaveAndDisconnect(const uint64_t obj_id)noexcept;
		void Immigration(const S_ptr<Sector> pOtherSector, const uint64 obj_id) noexcept;
		void ImmigrationAll(const S_ptr<Sector> pOtherSector, const bool bDestroyCurrentSector)noexcept;
		void BroadCast(const S_ptr<SendBuffer> pSendBuffer)noexcept;
		void BroadCastExceptOne(const S_ptr<SendBuffer> pSendBuffer, const uint64 exceptSessionNumber)noexcept;
		void RegisterHeartBeat()noexcept;
		void ListenHeartBeat()noexcept;
	protected:
		SlimLinkedHashMap<uint64, IocpEntity> m_linkedHashMapForIocpEntity{ 128 };
		SlimLinkedHashMap<uint64, PacketSession> m_linkedHashMapForSession{ 128 };
		const uint16_t m_sectorID;
	};
}
