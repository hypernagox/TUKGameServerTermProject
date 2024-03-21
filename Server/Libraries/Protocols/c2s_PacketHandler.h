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
		c2s_ENTER_GAME = 1002,
		s2c_ENTER_GAME = 1003,
		c2s_CHAT = 1004,
		s2c_CHAT = 1005,
	};
	
	class PacketSession;
	class Session;
	class SendBuffer;
	class SessionManageable;
	
	const bool Handle_Invalid(const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_);
	//const bool Handle_c2s_LOGIN(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_LOGIN& pkt_);
	//const bool Handle_c2s_ENTER_GAME(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_ENTER_GAME& pkt_);
	//const bool Handle_c2s_CHAT(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_CHAT& pkt_);
	
	class c2s_PacketHandler
	{
		using PacketHandlerFunc = const bool (*)(const S_ptr<PacketSession>&, BYTE* const, c_int32);
		static inline PacketHandlerFunc g_fpPacketHandler[UINT16_MAX] = {};
	public:
		static void Init() noexcept
		{
			//g_fpPacketHandler[net_etoi(PKT_ID::c2s_LOGIN)] = [](const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)->const bool { return HandlePacket<Protocol::c2s_LOGIN>(Handle_c2s_LOGIN, pSession_, pBuff_, len_); };
			//g_fpPacketHandler[net_etoi(PKT_ID::c2s_ENTER_GAME)] = [](const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)->const bool { return HandlePacket<Protocol::c2s_ENTER_GAME>(Handle_c2s_ENTER_GAME, pSession_, pBuff_, len_); };
			//g_fpPacketHandler[net_etoi(PKT_ID::c2s_CHAT)] = [](const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)->const bool { return HandlePacket<Protocol::c2s_CHAT>(Handle_c2s_CHAT, pSession_, pBuff_, len_); };
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
		static S_ptr<SendBuffer> MakeSendBuffer(Protocol::s2c_ENTER_GAME& pkt)noexcept { return MakeSendBuffer(pkt, PKT_ID::s2c_ENTER_GAME); }
		static S_ptr<SendBuffer> MakeSendBuffer(Protocol::s2c_CHAT& pkt)noexcept { return MakeSendBuffer(pkt, PKT_ID::s2c_CHAT); }
	
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