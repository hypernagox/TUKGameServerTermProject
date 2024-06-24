#pragma once

namespace ServerCore
{
	class IocpEntity;
	class Session;
	class SendBuffer;
	class PacketSession;

	enum SECTOR_STATE
	{
		NOT_EMPTY = 0,
		NPC_EMPTY = 1 << 0,
		USER_EMPTY = 1 << 1,
		EMPTY = 3,
		STOP = 1 << 2,

		IDLE,
		WORK,

		NONE,
	};

	class MoveBroadcaster
	{
	public:
		MoveBroadcaster();
		~MoveBroadcaster();
		using HuristicFunc = bool(*)(const IocpEntity* const, const IocpEntity* const);
		using PacketFunc = S_ptr<SendBuffer>(*)(const S_ptr<IocpEntity>&);
	public:

		const int BroadcastMove(
			const S_ptr<SendBuffer>& in_pkt,
			const S_ptr<SendBuffer>& out_pkt,
			const S_ptr<SendBuffer>& move_pkt,
			const S_ptr<IocpEntity>& thisSession,
			const Vector<SessionManageable*>* const sectors
		)noexcept;
		
		void ReleaseViewList()noexcept
		{
			//if (IDLE == m_work_flag.exchange(STOP, std::memory_order_relaxed))
			//{
			//	std::atomic_thread_fence(std::memory_order_acquire);
			//	//m_viewList.clear();
			//}
			m_viewListPtr.store(nullptr, std::memory_order_release);
		}
	public:
		static void RegisterHuristicFunc(const HuristicFunc fp_)noexcept {
			if (g_huristic)return;
			g_huristic = (fp_);
		}
		static void RegisterInPacketFunc(const PacketFunc fp_)noexcept {
			if (g_create_in_pkt)return;
			g_create_in_pkt = (fp_);
		}
		static void RegisterOutPacketFunc(const PacketFunc fp_)noexcept {
			if (g_create_out_pkt)return;
			g_create_out_pkt = (fp_);
		}
	public:
		static S_ptr<SendBuffer> CreateAddPacket(const S_ptr<IocpEntity>& pEntity_)noexcept {
			return g_create_in_pkt(pEntity_);
		}
		static S_ptr<SendBuffer> CreateOutPacket(const S_ptr<IocpEntity>& pEntity_)noexcept {
			return g_create_out_pkt(pEntity_);
		}
	private:
		//std::atomic<SECTOR_STATE> m_work_flag = IDLE; 
		//HashSet<S_ptr<IocpEntity>> m_viewList;
		std::atomic<std::shared_ptr<HashSet<S_ptr<IocpEntity>>>> m_viewListPtr = std::shared_ptr<HashSet<S_ptr<IocpEntity>>>();
		
	private:
		static inline HuristicFunc g_huristic = {};
		static inline PacketFunc g_create_in_pkt = {};
		static inline PacketFunc g_create_out_pkt = {};
	};
}
