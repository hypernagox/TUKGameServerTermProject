#pragma once
#include "pch.h"

class ServerSession;
class s2c_PacketHandler;

ServerSession* GetServerSession()noexcept;

template<typename T>
void Send(T& pkt_)noexcept 
{
	GetServerSession()->Send(NetHelper::s2c_PacketHandler::MakeSendBuffer(pkt_));
}
