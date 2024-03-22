#pragma once
#include "pch.h"
#include "Struct.pb.h"

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

static inline Protocol::Vec2 ToProtoVec2(const Vec2& v_)noexcept {
	Protocol::Vec2 vec;
	vec.set_x(v_.x);
	vec.set_y(v_.y);
	return vec;
}

static inline Vec2 ToOriginVec2(const Protocol::Vec2& v_)noexcept {
	return { v_.x(),v_.y() };
}
