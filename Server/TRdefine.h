#pragma once

enum class SECTOR :uint16_t
{
	SECTOR_0,
	SECTOR_1,
	SECTOR_2,
	SECTOR_3,
	SECTOR_4,

	END,
};

using ServerCore::S_ptr;
using ServerCore::W_ptr;
using ServerCore::U_ptr;
using ServerCore::U_Pptr;

using ServerCore::etoi;


using ServerCore::MakeShared;
using ServerCore::MakePoolShared;
using ServerCore::MakeUnique;
using ServerCore::MakePoolUnique;