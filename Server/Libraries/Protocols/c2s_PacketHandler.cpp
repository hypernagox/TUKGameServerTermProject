#include "../pch.h"
#include "c2s_PacketHandler.h"
#include "../ClientSession.h"
#include "../TRWorld.h"

namespace ServerCore
{
    const bool Handle_Invalid(const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)
    {
        return false;
    }

    const bool Handle_c2s_LOGIN(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_LOGIN& pkt_)
    {
        Protocol::s2c_LOGIN pkt;
        pkt.set_seed(10);

        pSession_ << pkt;

        return true;
    }

    const bool Handle_c2s_BREAK_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_BREAK_TILE& pkt_)
    {
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        if (TRMgr(TRWorld)->BreakTile(x, y))
        {
            Protocol::s2c_BREAK_TILE pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);

            pSession_ << pkt;
        }
        return true;
    }

}

