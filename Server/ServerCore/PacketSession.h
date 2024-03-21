#pragma once
#include "Session.h"

namespace ServerCore
{
	class PacketSession
		:public Session
	{
	public:
		PacketSession(const PacketHandleFunc* const sessionPacketHandler_)noexcept;
		~PacketSession();
		virtual const RecvStatus OnRecv(BYTE* const buffer, c_int32 len, const S_ptr<PacketSession>& pThisSessionPtr)noexcept override sealed;
	protected:
		virtual void OnConnected() abstract;
		virtual void OnSend(c_int32 len)noexcept override {}
		virtual void OnDisconnected()abstract;
	};
}
