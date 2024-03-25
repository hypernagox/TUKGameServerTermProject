#include "../pch.h"
#include "c2s_PacketHandler.h"
#include "../ClientSession.h"
#include "../TRTileManager.h"
#include "../TRWorldMgr.h"
#include "../TRWorldRoom.h"
#include "../ObjectFactory.h"
#include "../Object.h"

static ClientSession* const GetClientSession(const std::shared_ptr<ServerCore::PacketSession>& pSession_)noexcept {
    return static_cast<ClientSession* const>(pSession_.get());
}

static std::shared_ptr<TRWorldRoom> GetWorldRoom(const std::shared_ptr<ServerCore::PacketSession>& pSession_)noexcept {
    return TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(pSession_->GetCurrentSessionRoomInfo().GetID()));
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
        const auto start_room = TRMgr(TRWorldMgr)->GetWorldRoom(SECTOR::SECTOR_0);

        start_room->EnqueueAsync([pSession_, start_room]()noexcept
            {
                Protocol::s2c_ENTER pkt;
                pkt.set_player_id(pSession_->GetSessionID());
                for (const auto others : start_room->GetSessionList())
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

        start_room->EnterEnqueue(pSession_);

        Protocol::s2c_CREATE_ITEM pkt;
        const auto item_pos = Vec2{ 4100.f,0.f };;
        *pkt.mutable_pos() = item_pos;
        pkt.set_item_name("armor_iron_head");
        pkt.set_obj_id(IDGenerator::GenerateID());

        start_room << pkt;

        const auto pClientSession = GetClientSession(pSession_);

        auto player = ObjectFactory::CreatePlayer(pClientSession, pSession_->GetSessionID());

        pClientSession->SetPlayer(player.get());

        start_room->AddObjectEnqueue(GROUP_TYPE::PLAYER, std::move(player));

        start_room->AddObjectEnqueue(GROUP_TYPE::DROP_ITEM, ObjectFactory::CreateDropItem(pkt.obj_id(), "armor_iron_head", item_pos, start_room.get()));

        return true;
    }

    const bool Handle_c2s_BREAK_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_BREAK_TILE& pkt_)
    {
        //const auto room_id = pSession_->GetCurrentSessionRoomInfo().GetID();
        //const auto session_room = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(room_id));
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurrentSessionRoomInfo().GetPtr());
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        if (session_room->BreakTile(x, y))
        {
            Protocol::s2c_BREAK_TILE pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);

            session_room << pkt;
        }
        return true;
    }

    const bool Handle_c2s_BREAK_TILE_WALL(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_BREAK_TILE_WALL& pkt_)
    {
        //const auto room_id = pSession_->GetCurrentSessionRoomInfo().GetID();
        //const auto session_room = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(room_id));
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurrentSessionRoomInfo().GetPtr());
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        if (session_room->BreakTileWall(x, y))
        {
            Protocol::s2c_BREAK_TILE_WALL pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);

            session_room << pkt;
        }
        return true;
    }

    const bool Handle_c2s_PLACE_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_PLACE_TILE& pkt_)
    {
        //const auto room_id = pSession_->GetCurrentSessionRoomInfo().GetID();
        //const auto session_room = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(room_id));
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurrentSessionRoomInfo().GetPtr());
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        const auto key = ::Utf8ToWide(pkt_.tile_key());

        if (session_room->PlaceTile(x,y,TRMgr(TRTileManager)->GetTileByKey(key)))
        {
            Protocol::s2c_PLACE_TILE pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);
            pkt.set_tile_key(pkt_.tile_key());

            session_room << pkt;
        }
        return true;
    }

    const bool Handle_c2s_PLACE_TILE_WALL(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_PLACE_TILE_WALL& pkt_)
    {
        //const auto room_id = pSession_->GetCurrentSessionRoomInfo().GetID();
        //const auto session_room = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(room_id));
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurrentSessionRoomInfo().GetPtr());
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        const auto key = ::Utf8ToWide(pkt_.tile_key());

        if (session_room->PlaceTileWall(x, y, TRMgr(TRTileManager)->GetTileWallByKey(key)))
        {
            Protocol::s2c_PLACE_TILE_WALL pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);
            pkt.set_tile_key(pkt_.tile_key());

            session_room << pkt;
        }
        return true;
    }

    const bool Handle_c2s_MOVE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_MOVE& pkt_)
    {
        //const auto room_id = pSession_->GetCurrentSessionRoomInfo().GetID();
        //const auto session_room = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(room_id));
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurrentSessionRoomInfo().GetPtr());

        auto pkt = session_room->updateTileCollision(pkt_);
        pkt.set_obj_id(pSession_->GetSessionID());
        pkt.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
        pkt.set_anim_dir(pkt_.anim_dir());
        //&TRMgr(TRWorld)->m_room << pkt - pSession_;
        session_room << pkt;
        
        const auto player = GetClientSession(pSession_)->GetPlayer();
        if (player)
        {
            player->SetPos(::ToOriginVec2(pkt.obj_pos()));
            player->SetWillPos(::ToOriginVec2(pkt.wiil_pos()));
        }
        return true;
    }

    const bool Handle_c2s_CREATE_ITEM(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_CREATE_ITEM& pkt_)
    {
        return false;
    }

    const bool Handle_c2s_GET_ITEM(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_GET_ITEM& pkt_)
    {
        return false;
    }

}

