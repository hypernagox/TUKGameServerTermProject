#pragma once
#include "ClientNetworkPch.h"
#include "Protocol.pb.h"

namespace NetHelper
{
	template<typename T>requires std::is_enum_v<T>
	static inline constexpr const uint16 net_etoi(const T eType_)noexcept { return static_cast<const uint16>(eType_); }

	enum class PKT_ID: uint16
	{
		c2s_LOGIN = 1000,
		s2c_LOGIN = 1001,
		c2s_BREAK_TILE = 1002,
		s2c_BREAK_TILE = 1003,
	};
	
	class PacketSession;
	class Session;
	class SendBuffer;
	class SessionManageable;
	
	const bool Handle_Invalid(const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_);
	const bool Handle_s2c_LOGIN(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_LOGIN& pkt_);
	const bool Handle_s2c_BREAK_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_BREAK_TILE& pkt_);
	
	class s2c_PacketHandler
	{
		using PacketHandlerFunc = const bool (*)(const S_ptr<PacketSession>&, BYTE* const, c_int32);
		static inline PacketHandlerFunc g_fpPacketHandler[UINT16_MAX] = {};
	public:
		static void Init() noexcept
		{
			g_fpPacketHandler[net_etoi(PKT_ID::s2c_LOGIN)] = [](const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)->const bool { return HandlePacket<Protocol::s2c_LOGIN>(Handle_s2c_LOGIN, pSession_, pBuff_, len_); };
			g_fpPacketHandler[net_etoi(PKT_ID::s2c_BREAK_TILE)] = [](const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)->const bool { return HandlePacket<Protocol::s2c_BREAK_TILE>(Handle_s2c_BREAK_TILE, pSession_, pBuff_, len_); };
			for (auto& fpHandlerFunc : g_fpPacketHandler) 
			{
				if (nullptr == fpHandlerFunc)
					fpHandlerFunc = Handle_Invalid;
			}
		}
		
		static const PacketHandlerFunc* const GetPacketHandlerList()noexcept { return g_fpPacketHandler; }

		static void AddProtocol(const uint16 pktID_, const PacketHandlerFunc fpPacketHandler_)noexcept
		{
			NET_NAGOX_ASSERT(nullptr == g_fpPacketHandler[pktID_] || Handle_Invalid == g_fpPacketHandler[pktID_]);
			g_fpPacketHandler[pktID_] = fpPacketHandler_;
		}

		static const bool HandlePacket(const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)
		{
			const PacketHeader* const __restrict header = reinterpret_cast<const PacketHeader* const>(pBuff_);
			return g_fpPacketHandler[header->pkt_id](pSession_, pBuff_, len_);
		}
		static S_ptr<SendBuffer> MakeSendBuffer(Protocol::c2s_LOGIN& pkt)noexcept { return MakeSendBuffer(pkt, PKT_ID::c2s_LOGIN); }
		static S_ptr<SendBuffer> MakeSendBuffer(Protocol::c2s_BREAK_TILE& pkt)noexcept { return MakeSendBuffer(pkt, PKT_ID::c2s_BREAK_TILE); }
	
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
			NET_NAGOX_ASSERT(pkt.SerializeToArray(header + 1, dataSize));
			sendBuffer->Close(packetSize);
	
			return sendBuffer;
		}
	public:
		s2c_PacketHandler() = delete;                 
		s2c_PacketHandler(const s2c_PacketHandler&) = delete;
		s2c_PacketHandler(s2c_PacketHandler&&) = delete;
		s2c_PacketHandler& operator=(const s2c_PacketHandler&) = delete;
		s2c_PacketHandler& operator=(s2c_PacketHandler&&) = delete;
		~s2c_PacketHandler() = delete;
	};
}