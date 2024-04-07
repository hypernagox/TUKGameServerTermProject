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

        const auto pClientSession = GetClientSession(pSession_);

        auto player = ObjectFactory::CreatePlayer(pClientSession, pSession_->GetSessionID());

        pClientSession->SetPlayer(player);

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
        const auto item_pos = Vec2{ 4100.f/4.f,0.f };;
        *pkt.mutable_pos() = item_pos;
        pkt.set_item_name("armor_iron_head");
        pkt.set_obj_id(IDGenerator::GenerateID());

        start_room << pkt;

        start_room->AddObjectEnqueue(GROUP_TYPE::PLAYER, std::move(player));

        for(int i=0;i<1;++i)
            start_room->AddObjectEnqueue(GROUP_TYPE::DROP_ITEM, ObjectFactory::CreateDropItem(pkt.obj_id(), "armor_iron_head", item_pos, start_room.get()));

       const auto tile_map = TRMgr(TRWorldMgr)->GetWorldRoom(SECTOR::SECTOR_0)->GetTRWorld()->GetTileMap();
       constexpr const int x = TRWorld::WORLD_WIDTH / 2;
       Protocol::s2c_MOVE pkt2;
       *pkt2.mutable_obj_pos() = TRWorld::WorldToGlobal(Vec2Int(x, tile_map->GetTopYpos(x))) - Vec2(20.0f, 28.0f);
       pkt2.set_obj_id(pSession_->GetSessionID());
       pkt2.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
       //*pkt2.mutable_vel()
       pSession_ << pkt2;
        return true;    
    }

    const bool Handle_c2s_BREAK_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_BREAK_TILE& pkt_)
    {
        //const auto room_id = pSession_->GetCurrentSessionRoomInfo().GetID();
        //const auto session_room = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(room_id));
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurrentSessionRoomInfo().GetPtr());
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        const Vec2 item_pos = TRWorld::WorldToGlobal(Vec2{ (float)x,(float)y});

        std::string temp;

        if (session_room->GetTRWorld()->BreakTile(x, y,temp))
        {
            Protocol::s2c_BREAK_TILE pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);

            session_room->BroadCastToWorld(c2s_PacketHandler::MakeSendBuffer(pkt));

            //session_room << pkt;

            if (!temp.empty())
            {
                Protocol::s2c_CREATE_ITEM item_pkt;
                *item_pkt.mutable_pos() = Vec2{ (float)x ,(float)y + .5f };
                item_pkt.set_item_name(temp);
                item_pkt.set_obj_id(IDGenerator::GenerateID());
                item_pkt.set_sector(session_room->GetRoomID());

                session_room->AddObjectEnqueue(GROUP_TYPE::DROP_ITEM, ObjectFactory::CreateDropItem(item_pkt.obj_id(), temp, item_pos, session_room));

                session_room->BroadCastToWorld(c2s_PacketHandler::MakeSendBuffer(item_pkt));

                Protocol::s2c_MOVE pkt2;
                *pkt2.mutable_obj_pos() = item_pos;
                pkt2.set_obj_id(item_pkt.obj_id());
                pkt2.set_time_stamp(ServerCore::GetTimeStampMilliseconds());

                session_room << pkt2;
            }
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
        if (session_room->GetTRWorld()->BreakTileWall(x, y))
        {
            Protocol::s2c_BREAK_TILE_WALL pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);

            session_room->BroadCastToWorld(c2s_PacketHandler::MakeSendBuffer(pkt));
           // session_room << pkt;
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

        if (session_room->GetTRWorld()->PlaceTile(x,y,TRMgr(TRTileManager)->GetTileByKey(key)))
        {
            Protocol::s2c_PLACE_TILE pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);
            pkt.set_tile_key(pkt_.tile_key());

            session_room->BroadCastToWorld(c2s_PacketHandler::MakeSendBuffer(pkt));
            //session_room << pkt;
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

        if (session_room->GetTRWorld()->PlaceTileWall(x, y, TRMgr(TRTileManager)->GetTileWallByKey(key)))
        {
            Protocol::s2c_PLACE_TILE_WALL pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);
            pkt.set_tile_key(pkt_.tile_key());

            session_room->BroadCastToWorld(c2s_PacketHandler::MakeSendBuffer(pkt));
            //session_room << pkt;
        }
        return true;
    }

    const bool Handle_c2s_MOVE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_MOVE& pkt_)
    {
        //const auto room_id = pSession_->GetCurrentSessionRoomInfo().GetID();
        //const auto session_room = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(room_id));
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurrentSessionRoomInfo().GetPtr());

        auto pkt = session_room->GetTRWorld()->updateTileCollision(pkt_);
        pkt.set_obj_id(pSession_->GetSessionID());
        pkt.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
        pkt.set_anim_dir(pkt_.anim_dir());
       // &TRMgr(TRWorld)->m_room << pkt - pSession_;

       //Protocol::s2c_MOVE pkt;
       //
       //*pkt.mutable_obj_pos() = pkt_.obj_pos();
       //*pkt.mutable_vel() = pkt_.vel();
       // pkt.set_state(pkt_.state());
       // 
       // pkt.set_obj_id(pSession_->GetSessionID());
       // pkt.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
       // pkt.set_anim_dir(pkt_.anim_dir());

        //session_room << pkt;
        pSession_ << pkt;
        const auto& player = GetClientSession(pSession_)->GetPlayer();
        if (player)
        {
            player->SetPos(::ToOriginVec2(pkt.obj_pos()));
            player->SetWillPos(::ToOriginVec2(pkt.wiil_pos()));
            player->GetComp("RIGIDBODY")->Cast<RigidBody>()->SetVelocity(::ToOriginVec2(pkt.vel()));
            player->GetComp("RIGIDBODY")->Cast<RigidBody>()->SetIsGround(pkt.ground());
            player->SetState(pkt.state());
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

    const bool Handle_c2s_INPUT_KEY(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_INPUT_KEY& pkt_)
    {
        const auto& player = GetClientSession(pSession_)->GetPlayer();
        if (player)
        {
           player->GetComp("KEYINPUTHANDLER")->Cast<KeyInputHandler>()->SetKeyState(pkt_.vk_key(), (KeyInputHandler::KEY_STATE)(pkt_.key_state()));
        }
        return true;
    }

    const bool Handle_c2s_TRY_GET_ITEM(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_TRY_GET_ITEM& pkt_)
    {
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurrentSessionRoomInfo().GetPtr());

        if (const auto& player = GetClientSession(pSession_)->GetPlayer())
        {
            session_room->TryGetItem(GetClientSession(pSession_)->GetPlayer(),ToOriginVec2(pkt_.obj_pos()));
        }
        return true;
    }

    const bool Handle_c2s_TRY_NEW_ROOM(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_TRY_NEW_ROOM& pkt_)
    {
       const auto pClientSession = GetClientSession(pSession_);

       const auto nextSector = pkt_.next_sector_num();

       const auto pCurRoom = static_cast<TRWorldRoom*>(pClientSession->GetCurrentSessionRoomInfo().GetPtr());
       auto pNextRoom = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(nextSector));


       pCurRoom->ImmigrationEnqueue(std::move(pNextRoom), pClientSession->GetSessionID());

        return true;
    }

    const bool Handle_c2s_ARRIVE_NEW_ROOM(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_ARRIVE_NEW_ROOM& pkt_)
    {
        return true;
    }

    const bool Handle_c2s_APPEAR_NEW_OBJECT(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_APPEAR_NEW_OBJECT& pkt_)
    {
        return false;
    }

    const bool Handle_c2s_LEAVE_OBJECT(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_LEAVE_OBJECT& pkt_)
    {
        return false;
    }

    const bool Handle_c2s_CREATE_MISSILE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_CREATE_MISSILE& pkt_)
    {
        const auto cur_room = static_cast<TRWorldRoom* const>(pSession_->GetCurrentSessionRoomInfo().GetPtr());
        auto pMissle = ObjectFactory::CreateMissle(ToOriginVec2(pkt_.obj_pos()), cur_room);
        

        cur_room->AddObjectEnqueue(GROUP_TYPE::PROJ_PLAYER, pMissle);

        Protocol::s2c_CREATE_MISSILE pkt;
        pkt.set_obj_id(pMissle->GetObjID());
        *pkt.mutable_obj_pos() = pkt_.obj_pos();

        //pMissle->PostUpdate(.2f);
        cur_room << pkt;

       //Protocol::s2c_MOVE pkt2;
       //pkt2.set_obj_id(pMissle->GetObjID());
       //*pkt2.mutable_obj_pos() = pMissle->GetPos() + Vec2{ 1000.f,0.f }*0.1f;
       //
       //cur_room << pkt2;

        return true;
    }

}

