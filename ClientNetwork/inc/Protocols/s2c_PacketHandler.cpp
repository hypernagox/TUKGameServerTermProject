#include "pch.h"
#include "s2c_PacketHandler.h"
#include "ServerSession.h"
#include "NetworkMgr.h"
#include "TRWorld.h"

extern int g_TR_SEED;
extern TRWorld* g_TRWorld;


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

    const bool Handle_s2c_BREAK_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_BREAK_TILE& pkt_)
    {
        if (pkt_.success())
        {
            g_TRWorld->BreakTile(pkt_.tile_x(), pkt_.tile_y());
        }

        return true;
    }
}
