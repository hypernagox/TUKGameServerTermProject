#pragma once
#include "ServerCorePch.h"
#include "Protocol.pb.h"

namespace ServerCore
{
	template<typename T>requires std::is_enum_v<T>
	static inline constexpr const uint16 net_etoi(const T eType_)noexcept { return static_cast<const uint16>(eType_); }

	enum class PKT_ID: uint16
	{
		c2s_LOGIN = 1000,
		s2c_LOGIN = 1001,
		c2s_ENTER = 1002,
		s2c_ENTER = 1003,
		c2s_BREAK_TILE = 1004,
		s2c_BREAK_TILE = 1005,
		c2s_BREAK_TILE_WALL = 1006,
		s2c_BREAK_TILE_WALL = 1007,
		c2s_PLACE_TILE = 1008,
		s2c_PLACE_TILE = 1009,
		c2s_PLACE_TILE_WALL = 1010,
		s2c_PLACE_TILE_WALL = 1011,
		c2s_MOVE = 1012,
		s2c_MOVE = 1013,
	};
	
	class PacketSession;
	class Session;
	class SendBuffer;
	class SessionManageable;
	
	const bool Handle_Invalid(const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_);
	const bool Handle_c2s_LOGIN(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_LOGIN& pkt_);
	const bool Handle_c2s_ENTER(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_ENTER& pkt_);
	const bool Handle_c2s_BREAK_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_BREAK_TILE& pkt_);
	const bool Handle_c2s_BREAK_TILE_WALL(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_BREAK_TILE_WALL& pkt_);
	const bool Handle_c2s_PLACE_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_PLACE_TILE& pkt_);
	const bool Handle_c2s_PLACE_TILE_WALL(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_PLACE_TILE_WALL& pkt_);
	const bool Handle_c2s_MOVE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_MOVE& pkt_);
	
	class c2s_PacketHandler
	{
		using PacketHandlerFunc = const bool (*)(const S_ptr<PacketSession>&, BYTE* const, c_int32);
		static inline PacketHandlerFunc g_fpPacketHandler[UINT16_MAX] = {};
	public:
		static void Init() noexcept
		{
			g_fpPacketHandler[net_etoi(PKT_ID::c2s_LOGIN)] = [](const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)->const bool { return HandlePacket<Protocol::c2s_LOGIN>(Handle_c2s_LOGIN, pSession_, pBuff_, len_); };
			g_fpPacketHandler[net_etoi(PKT_ID::c2s_ENTER)] = [](const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)->const bool { return HandlePacket<Protocol::c2s_ENTER>(Handle_c2s_ENTER, pSession_, pBuff_, len_); };
			g_fpPacketHandler[net_etoi(PKT_ID::c2s_BREAK_TILE)] = [](const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)->const bool { return HandlePacket<Protocol::c2s_BREAK_TILE>(Handle_c2s_BREAK_TILE, pSession_, pBuff_, len_); };
			g_fpPacketHandler[net_etoi(PKT_ID::c2s_BREAK_TILE_WALL)] = [](const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)->const bool { return HandlePacket<Protocol::c2s_BREAK_TILE_WALL>(Handle_c2s_BREAK_TILE_WALL, pSession_, pBuff_, len_); };
			g_fpPacketHandler[net_etoi(PKT_ID::c2s_PLACE_TILE)] = [](const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)->const bool { return HandlePacket<Protocol::c2s_PLACE_TILE>(Handle_c2s_PLACE_TILE, pSession_, pBuff_, len_); };
			g_fpPacketHandler[net_etoi(PKT_ID::c2s_PLACE_TILE_WALL)] = [](const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)->const bool { return HandlePacket<Protocol::c2s_PLACE_TILE_WALL>(Handle_c2s_PLACE_TILE_WALL, pSession_, pBuff_, len_); };
			g_fpPacketHandler[net_etoi(PKT_ID::c2s_MOVE)] = [](const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)->const bool { return HandlePacket<Protocol::c2s_MOVE>(Handle_c2s_MOVE, pSession_, pBuff_, len_); };
			for (auto& fpHandlerFunc : g_fpPacketHandler) 
			{
				if (nullptr == fpHandlerFunc)
					fpHandlerFunc = Handle_Invalid;
			}
		}

		static const PacketHandlerFunc* const GetPacketHandlerList()noexcept { return g_fpPacketHandler; }

		static void AddProtocol(const uint16 pktID_, const PacketHandlerFunc fpPacketHandler_)noexcept
		{
			NAGOX_ASSERT(nullptr == g_fpPacketHandler[pktID_] || Handle_Invalid == g_fpPacketHandler[pktID_]);
			g_fpPacketHandler[pktID_] = fpPacketHandler_;
		}
	
		static const bool HandlePacket(const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)noexcept
		{
			const PacketHeader* const __restrict header = reinterpret_cast<const PacketHeader* const>(pBuff_);
			return g_fpPacketHandler[header->pkt_id](pSession_, pBuff_, len_);
		}
		static S_ptr<SendBuffer> MakeSendBuffer(Protocol::s2c_LOGIN& pkt)noexcept { return MakeSendBuffer(pkt, PKT_ID::s2c_LOGIN); }
		static S_ptr<SendBuffer> MakeSendBuffer(Protocol::s2c_ENTER& pkt)noexcept { return MakeSendBuffer(pkt, PKT_ID::s2c_ENTER); }
		static S_ptr<SendBuffer> MakeSendBuffer(Protocol::s2c_BREAK_TILE& pkt)noexcept { return MakeSendBuffer(pkt, PKT_ID::s2c_BREAK_TILE); }
		static S_ptr<SendBuffer> MakeSendBuffer(Protocol::s2c_BREAK_TILE_WALL& pkt)noexcept { return MakeSendBuffer(pkt, PKT_ID::s2c_BREAK_TILE_WALL); }
		static S_ptr<SendBuffer> MakeSendBuffer(Protocol::s2c_PLACE_TILE& pkt)noexcept { return MakeSendBuffer(pkt, PKT_ID::s2c_PLACE_TILE); }
		static S_ptr<SendBuffer> MakeSendBuffer(Protocol::s2c_PLACE_TILE_WALL& pkt)noexcept { return MakeSendBuffer(pkt, PKT_ID::s2c_PLACE_TILE_WALL); }
		static S_ptr<SendBuffer> MakeSendBuffer(Protocol::s2c_MOVE& pkt)noexcept { return MakeSendBuffer(pkt, PKT_ID::s2c_MOVE); }
	
	private:
		template<typename PacketType, typename ProcessFunc>
		static const bool HandlePacket(ProcessFunc&& func, const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)
		{
			PacketType pkt;
			if (pkt.ParseFromArray(pBuff_ + sizeof(PacketHeader), len_ - sizeof(PacketHeader)) == false) [[unlikely]]
				return false;
	
			return func(pSession_, pkt);
		}
	
		template<typename T>
		static S_ptr<SendBuffer> MakeSendBuffer(T& pkt, const PKT_ID pktId)noexcept
		{
			const uint16 dataSize = static_cast<c_uint16>(pkt.ByteSizeLong());
			const uint16 packetSize = dataSize + static_cast<c_uint16>(sizeof(PacketHeader));
	
			S_ptr<SendBuffer> sendBuffer = Mgr(SendBufferMgr)->Open(packetSize);
			PacketHeader* const __restrict header = reinterpret_cast<PacketHeader*const>(sendBuffer->Buffer());
			header->pkt_size = packetSize;
			header->pkt_id = static_cast<c_uint16>(pktId);
			NAGOX_ASSERT(pkt.SerializeToArray(header + 1, dataSize));
			sendBuffer->Close(packetSize);
	
			return sendBuffer;
		}
	public:
		c2s_PacketHandler() = delete;                 
		c2s_PacketHandler(const c2s_PacketHandler&) = delete;
		c2s_PacketHandler(c2s_PacketHandler&&) = delete;
		c2s_PacketHandler& operator=(const c2s_PacketHandler&) = delete;
		c2s_PacketHandler& operator=(c2s_PacketHandler&&) = delete;
		~c2s_PacketHandler() = delete;
	};
}