#include "ClientSession.h"
#include "c2s_PacketHandler.h"

ClientSession::ClientSession()
	:PacketSession{ ServerCore::c2s_PacketHandler::GetPacketHandlerList() }
{
}

ClientSession::~ClientSession()
{
}

void ClientSession::OnConnected()
{
}

void ClientSession::OnSend(c_int32 len)noexcept
{
}

void ClientSession::OnDisconnected()
{
}
