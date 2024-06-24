#pragma once

enum class CHUNK :uint16_t
{
	CHUNK_0,
	CHUNK_1,
	CHUNK_2,
	CHUNK_3,

	CHUNK_4,
	CHUNK_5,
	CHUNK_6,
	CHUNK_7,


	END,
};

constexpr int VIEW_RANGE = 512 + 128;


using ServerCore::S_ptr;
using ServerCore::W_ptr;
using ServerCore::U_ptr;
using ServerCore::U_Pptr;

using ServerCore::etoi;


using ServerCore::MakeShared;
using ServerCore::MakePoolShared;
using ServerCore::MakeUnique;
using ServerCore::MakePoolUnique;

using ServerCore::String;
using ServerCore::WString;