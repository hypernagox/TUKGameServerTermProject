#pragma once
#include "pch.h"

class ServerSession;
class s2c_PacketHandler;

ServerSession* const GetServerSession()noexcept;

template<typename T>
void Send(T& pkt_)noexcept 
{
	NetHelper::Send(pkt_);
}

std::string WideToUtf8(std::wstring_view wstr)noexcept;

std::wstring Utf8ToWide(std::string_view utf8Str)noexcept;
