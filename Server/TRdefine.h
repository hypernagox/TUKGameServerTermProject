#pragma once

enum class SECTOR :uint16_t
{
	SECTOR_0,
	

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