#include "pch.h"
#include "s2c_PacketHandler.h"
#include "ServerSession.h"
#include "NetworkMgr.h"
#include "TRWorld.h"
#include "TRItemManager.h"
#include "TRTileManager.h"

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

    const bool NetHelper::Handle_s2c_BREAK_TILE_WALL(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_BREAK_TILE_WALL& pkt_)
    {
        if (pkt_.success())
        {
            g_TRWorld->BreakTileWall(pkt_.tile_x(), pkt_.tile_y());
        }

        return true;
    }

    const bool NetHelper::Handle_s2c_PLACE_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_PLACE_TILE& pkt_)
    {
        const auto key = Utf8ToWide(pkt_.tile_key());
        if (pkt_.success())
        {
            g_TRWorld->PlaceTile(pkt_.tile_x(), pkt_.tile_y(), TRTileManager::GetInst()->GetTileByKey(key));
            g_TRWorld->FindAndModifyItemStack(pkt_.tile_key(), -1);
        }
        return false;
    }
}
