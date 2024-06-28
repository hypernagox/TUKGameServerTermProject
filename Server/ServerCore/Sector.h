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
		constexpr inline auto& GetSectorSRWLock()noexcept { return m_linkedHashMapForIocpEntity.GetSRWLock(); }
		inline void sector_lock()noexcept { GetSectorSRWLock().lock(); }
		inline void sector_unlock()noexcept { GetSectorSRWLock().unlock(); }
		inline void sector_lock_shared()noexcept { GetSectorSRWLock().lock_shared(); }
		inline void sector_unlock_shared()noexcept { GetSectorSRWLock().unlock_shared(); }

		constexpr inline auto& GetSessionSRWLock()noexcept { return m_linkedHashMapForSession.GetSRWLock(); }
		inline void session_lock()noexcept { GetSessionSRWLock().lock(); }
		inline void session_unlock()noexcept { GetSessionSRWLock().unlock(); }
		inline void session_lock_shared()noexcept { GetSessionSRWLock().lock_shared(); }
		inline void session_unlock_shared()noexcept { GetSessionSRWLock().unlock_shared(); }
		const uint16_t GetSectorID()const noexcept { return m_sectorID; }
	public:
		const auto& GetSessionList()const noexcept { return m_linkedHashMapForSession.GetItemListRef(); }
		const auto& GetEntityList()const noexcept { return m_linkedHashMapForIocpEntity.GetItemListRef(); }
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
			const Vector<Sector*>& sectors
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
