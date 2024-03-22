#include "../pch.h"
#include "c2s_PacketHandler.h"
#include "../ClientSession.h"
#include "../TRWorld.h"
#include "../TRTileManager.h"

static ClientSession* const GetClientSession(const std::shared_ptr<ServerCore::PacketSession>& pSession_)noexcept {
    return static_cast<ClientSession* const>(pSession_.get());
}

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
        pkt.set_id(pSession_->GetSessionID());
        pSession_ << pkt;

       

        return true;
    }

    const bool Handle_c2s_ENTER(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_ENTER& pkt_)
    {
        TRMgr(TRWorld)->m_room.EnqueueAsync([pSession_]()
            {
                Protocol::s2c_ENTER pkt;
                pkt.set_player_id(pSession_->GetSessionID());
                for (const auto others : TRMgr(TRWorld)->m_room.GetSessionList())
                {
                    {
                        others << pkt;
                    }
                    {
                        Protocol::s2c_ENTER pkt;
                        pkt.set_player_id(others->GetSessionID());
                        pSession_ << pkt;
                    }
                }
            });

        TRMgr(TRWorld)->m_room.EnterEnqueue(pSession_);

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

            &TRMgr(TRWorld)->m_room << pkt;
        }
        return true;
    }

    const bool Handle_c2s_BREAK_TILE_WALL(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_BREAK_TILE_WALL& pkt_)
    {
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        if (TRMgr(TRWorld)->BreakTileWall(x, y))
        {
            Protocol::s2c_BREAK_TILE_WALL pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);

            &TRMgr(TRWorld)->m_room << pkt;
        }
        return true;
    }

    const bool Handle_c2s_PLACE_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_PLACE_TILE& pkt_)
    {
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        const auto key = ::Utf8ToWide(pkt_.tile_key());

        if (TRMgr(TRWorld)->PlaceTile(x,y,TRMgr(TRTileManager)->GetTileByKey(key)))
        {
            Protocol::s2c_PLACE_TILE pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);
            pkt.set_tile_key(pkt_.tile_key());

            &TRMgr(TRWorld)->m_room << pkt;
        }
        return true;
    }

    const bool Handle_c2s_PLACE_TILE_WALL(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_PLACE_TILE_WALL& pkt_)
    {
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        const auto key = ::Utf8ToWide(pkt_.tile_key());

        if (TRMgr(TRWorld)->PlaceTileWall(x, y, TRMgr(TRTileManager)->GetTileWallByKey(key)))
        {
            Protocol::s2c_PLACE_TILE_WALL pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);
            pkt.set_tile_key(pkt_.tile_key());

            &TRMgr(TRWorld)->m_room << pkt;
        }
        return true;
    }

    const bool Handle_c2s_MOVE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_MOVE& pkt_)
    {
        auto pkt = TRMgr(TRWorld)->updateTileCollision(pkt_);
        pkt.set_obj_id(pSession_->GetSessionID());
        pkt.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
        pkt.set_anim_dir(pkt_.anim_dir());
        &TRMgr(TRWorld)->m_room << pkt - pSession_;
      
        return true;
    }

}

