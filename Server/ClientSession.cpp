#include "ClientSession.h"
#include "c2s_PacketHandler.h"
#include "Object.h"
#include "SessionManageable.h"

ClientSession::ClientSession()
	:PacketSession{ ServerCore::c2s_PacketHandler::GetPacketHandlerList() }
{
}

ClientSession::~ClientSession()
{
	std::cout << "BYE" << std::endl;
}

void ClientSession::OnConnected()
{
}

void ClientSession::OnSend(c_int32 len)noexcept
{	
}

void ClientSession::OnDisconnected()
{
	if (m_pPlayer)
	{
		m_pPlayer->SetInvalid();
		m_pPlayer.reset();
	}
	if (const auto pCurRoom = GetCurrentSessionRoomInfo().GetPtr())
	{
		GetCurrentSessionRoomInfo().GetPtr()->LeaveAndDisconnectEnqueue(SharedCastThis<Session>());
	}
	else
	{
		reset_cache_shared();
	}
}
