#pragma once

namespace ServerCore
{
	class IocpEntity;
	class Session;
	class SendBuffer;
	class PacketSession;
	class Sector;

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
		using HuristicFunc = bool(*)(const IocpEntity* const, const IocpEntity* const)noexcept;
		using PacketFunc = S_ptr<SendBuffer>(*)(const S_ptr<IocpEntity>&)noexcept;
	public:
		const int BroadcastMove(
			const S_ptr<SendBuffer>& in_pkt,
			const S_ptr<SendBuffer>& out_pkt,
			const S_ptr<SendBuffer>& move_pkt,
			const S_ptr<IocpEntity>& thisSession,
			const Vector<Sector*>& sectors
		)noexcept;
		
		void ReleaseViewList()noexcept
		{
			m_spinLock.lock();
			const S_ptr<ViewListWrapper> temp{ std::move(m_viewListPtr) };
			m_spinLock.unlock();
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
		struct ViewListWrapper
			:public RefCountable{
			HashSet<S_ptr<IocpEntity>> view_list;
		};
	private:
		SpinLock m_spinLock;
		S_ptr<ViewListWrapper> m_viewListPtr = MakeShared<ViewListWrapper>();
	private:
		static inline HuristicFunc g_huristic = {};
		static inline PacketFunc g_create_in_pkt = {};
		static inline PacketFunc g_create_out_pkt = {};
	};
}
