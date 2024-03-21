#include "pch.h"
#include "s2c_PacketHandler.h"
#include "ServerSession.h"
#include "NetworkMgr.h"

extern int g_TR_SEED;



namespace NetHelper
{
    const bool Handle_Invalid(const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)
    {
        return false;
    }
    const bool Handle_s2c_LOGIN(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_LOGIN& pkt_)
    {
        g_TR_SEED = pkt_.seed();
        return true;
    }
}
