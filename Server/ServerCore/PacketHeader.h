#pragma once
#include "ServerCorePch.h"

namespace ServerCore
{
	struct PacketHeader
	{
		int32 size;
		int32 id;
	};

	struct RecvStatus
	{
		c_int32 processLen;
		const bool bIsOK;
	};
}