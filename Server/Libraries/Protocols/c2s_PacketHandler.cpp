#include "../pch.h"
#include "c2s_PacketHandler.h"
#include "../ClientSession.h"
#include "../TRTileManager.h"
#include "../TRWorldMgr.h"
#include "../TRWorldRoom.h"
#include "../ObjectFactory.h"
#include "../Object.h"
#include "../TRWorldChunk.h"
#include "../Store.h"
#include "../Status.h"
#include "../Inventory.h"
#include "DBMgr.h"
#include "DBPacket.h"

static ClientSession* const GetClientSession(const S_ptr<ServerCore::PacketSession>& pSession_)noexcept {
    return static_cast<ClientSession* const>(pSession_.get());
}

//static std::shared_ptr<TRWorldRoom> GetWorldRoom(const std::shared_ptr<ServerCore::PacketSession>& pSession_)noexcept {
//    return TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(pSession_->GetCurSector.GetID()));
//}

concurrency::concurrent_unordered_map<std::string, std::unique_ptr<std::atomic_bool>> g_connectHash;

namespace ServerCore
{
    const bool Handle_Invalid(const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)
    {
        return false;
    }

    const bool Handle_c2s_LOGIN(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_LOGIN& pkt_)
    {
        const auto iter = g_connectHash.find(pkt_.user_name());
        if (g_connectHash.end() != iter)
        {
            auto& b = *iter->second;
            bool old_b = false;
            bool new_b = true;
            if (!b.compare_exchange_strong(old_b, new_b)) 
            {
                Protocol::s2c_LOGIN_FAIL fail;
                fail.set_msg("AlreadyConnected!");
               // pSession_ << fail;
                pSession_->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(fail));
                return true;
            }
        }
        else
        {
            g_connectHash.insert(iter, std::make_pair( pkt_.user_name(),std::make_unique<std::atomic_bool>(true) ));
            Protocol::s2c_LOGIN_FAIL fail;
            fail.set_msg("Welcome!");
           // pSession_ << fail;
            pSession_->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(fail));
        }
        Protocol::s2c_LOGIN pkt3;
        pkt3.set_seed(10);
        pkt3.set_id(pSession_->GetSessionID());
       // pSession_ << pkt3;
        pSession_->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt3));
        GetClientSession(pSession_)->SetName(pkt_.user_name());
        RequestQuery(DB_PlayerLogin{ pSession_->SharedFromThis<ClientSession>() });
        s2q_LOGIN pkt2;
        pkt2.user_id = pSession_->GetObjectID();
        strcpy_s(pkt2.userName, pkt_.user_name().c_str());
        RequestQueryServer(pkt2);

        return true;
    }

    const bool Handle_c2s_ENTER(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_ENTER& pkt_)
    {
        //const auto start_room = TRMgr(TRWorldMgr)->GetWorldRoom(SECTOR::SECTOR_0);

       // const auto start_room = TRMgr(TRWorldMgr)->GetWorldRoom(SECTOR::SECTOR_5);

        if (const auto pStage = pSession_->GetCurSector()) 
        {
            ((TRWorldRoom*)pStage)->GetWorldChunk()->TransmitTileRecord(pSession_);
            return true;
        }
       const auto pClientSession = GetClientSession(pSession_);
       //
       //ObjectBuilder b;
       //b.session = pClientSession;
       //b.id = pClientSession->GetObjectID();
       //auto player = ObjectFactory::CreatePlayer(b);
       //
       //
       //pClientSession->SetPlayer(player);
       //
      //const auto pos = pClientSession->GetPlayer()->GetPos();
      //const auto start_room = TRMgr(TRWorldMgr)->GetStartWorld()->GetWorldSector(pos);
      ////
      ////
      //Protocol::s2c_ENTER pkt;
      //*pkt.mutable_pos() = pos;
      //start_room->AddObjectEnqueue(GROUP_TYPE::PLAYER, pClientSession->GetPlayer());
      //start_room->EnterEnqueue(pSession_);
      //start_room->GetWorldChunk()->TransmitTileRecord(pSession_);
      ////
      //
      //pSession_ << pkt;
      
      
        return true;    
    }

    const bool Handle_c2s_SWING(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_SWING& pkt_)
    {
        const auto pSession = GetClientSession(pSession_);
        pSession->SetCurItemName(pkt_.item_name());
        Protocol::s2c_SWING pkt;
        pkt.set_swing_user_id(pSession->GetSessionID());
        pkt.set_item_name(pkt_.item_name());
        pkt.set_dir(pkt_.dir());
        pSession->GetPlayer()->SetDir(pkt_.dir());
        if (const auto curSector = (TRWorldRoom*)pSession->GetContentsEntity()->m_pCurSector.load(std::memory_order_acquire))
        {
            for (const auto s : curSector->GetAdjSector8()) {
             //   s << pkt;
                s->BroadCastEnqueue(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
            }
            pSession->UseCurItem();
        }
        return true;
    }

    const bool Handle_c2s_BREAK_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_BREAK_TILE& pkt_)
    {
        //const auto room_id = pSession_->GetCurSector.GetID();
        //const auto session_room = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(room_id));
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurSector());
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        const Vec2 item_pos = TRWorld::WorldToGlobal(Vec2{ (float)x,(float)y});

        std::string temp;

        if (session_room->GetWorldChunk()->BreakTile(x, y, temp))
        {
            Protocol::s2c_BREAK_TILE pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);
            
            session_room->BroadCastToWorld(c2s_PacketHandler::MakeSendBuffer(pkt));
            
            //session_room << pkt;

            if (!temp.empty())
            {
                ObjectBuilder b;
                b.str = temp;
                b.pos = item_pos;
                b.sector = session_room;

                auto item = ObjectFactory::CreateDropItem(b);

                Protocol::s2c_APPEAR_NEW_OBJECT add_pkt;
                {
                    *add_pkt.mutable_appear_pos() = item->GetPos();

                    add_pkt.set_is_player(false);
                  //  add_pkt.set_sector(0);
                    add_pkt.set_obj_id(item->GetObjID());
                    add_pkt.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
                }
               // pSession_ << add_pkt;
                //Protocol::s2c_CREATE_ITEM item_pkt;
                //*item_pkt.mutable_pos() = Vec2{ (float)x ,(float)y + .5f };
                //item_pkt.set_item_name(temp);
                ////item_pkt.set_obj_id(IDGenerator::GenerateID());
                //item_pkt.set_obj_id(item->GetObjID());
                //item_pkt.set_sector(session_room->GetRoomID());

                session_room->AddEnterEnqueue(GROUP_TYPE::DROP_ITEM, item);
               // session_room->AddObjectEnqueue(GROUP_TYPE::DROP_ITEM, item);

                //session_room->BroadCastToWorld(c2s_PacketHandler::MakeSendBuffer(add_pkt));
               //
               //Protocol::s2c_MOVE pkt2;
               //*pkt2.mutable_obj_pos() = item_pos;
               //pkt2.set_obj_id(item_pkt.obj_id());
               //pkt2.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
               //
               //session_room << pkt2;
            }
        }
        return true;
    }

    const bool Handle_c2s_BREAK_TILE_WALL(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_BREAK_TILE_WALL& pkt_)
    {
        //const auto room_id = pSession_->GetCurSector.GetID();
        //const auto session_room = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(room_id));
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurSector());
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        if (session_room->GetWorldChunk()->BreakTileWall(x, y))
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
        //const auto room_id = pSession_->GetCurSector.GetID();
        //const auto session_room = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(room_id));
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurSector());
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        const auto key = ::Utf8ToWide(pkt_.tile_key());
        std::wstring temp;
        if (!GetClientSession(pSession_)->GetPlayer()->GetComp<Inventory>()->GetItem(WideToUtf8(TRMgr(TRTileManager)->GetTileByKey(key)->DropItem())))
            return true;
        if (session_room->GetWorldChunk()->PlaceTile(x,y,TRMgr(TRTileManager)->GetTileByKey(key),pkt_.tile_key(), temp))
        {
            Protocol::s2c_PLACE_TILE pkt;
            pkt.set_success(true);
            pkt.set_tile_x(x);
            pkt.set_tile_y(y);
            pkt.set_tile_key(pkt_.tile_key());
            if (!temp.empty())
            {
                if (const auto  pTile = (CountableItem*)GetClientSession(pSession_)->GetPlayer()->GetComp<Inventory>()->GetItem(WideToUtf8(temp)).get())
                {
                    pTile->DecCount();
                }
            }
            session_room->BroadCastToWorld(c2s_PacketHandler::MakeSendBuffer(pkt));
            //session_room << pkt;
        }
        return true;
    }

    const bool Handle_c2s_PLACE_TILE_WALL(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_PLACE_TILE_WALL& pkt_)
    {
        //const auto room_id = pSession_->GetCurSector.GetID();
        //const auto session_room = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(room_id));
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurSector());
        const int x = pkt_.tile_x();
        const int y = pkt_.tile_y();
        const auto key = ::Utf8ToWide(pkt_.tile_key());

        if (session_room->GetWorldChunk()->PlaceTileWall(x, y, TRMgr(TRTileManager)->GetTileWallByKey(key)))
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
        //const auto room_id = pSession_->GetCurSector.GetID();
        //const auto session_room = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(room_id));
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurSector());
       
        if (!session_room)return true;
        auto pkt = session_room->GetWorldChunk()->updateTileCollision(pkt_);
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
       // pSession_ << pkt;
        pSession_->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
        const auto& player = GetClientSession(pSession_)->GetPlayer();
        if (player)
        {
            player->SetPos(::ToOriginVec2(pkt.obj_pos()));
            player->SetWillPos(::ToOriginVec2(pkt.wiil_pos()));
            player->GetComp<RigidBody>()->SetVelocity(::ToOriginVec2(pkt.vel()));
            player->GetComp<RigidBody>()->SetIsGround(pkt.ground());
            player->SetState(pkt.state());
            player->Update(session_room->GetSectorDT());
        }

        Protocol::s2c_APPEAR_NEW_OBJECT add_pkt;
        {
            *add_pkt.mutable_appear_pos() = ((Object*)pSession_->GetContentsEntity().get())->GetPos();

            add_pkt.set_is_player(true);
           // add_pkt.set_sector(0);
            add_pkt.set_obj_name(GetClientSession(pSession_)->GetName());
            add_pkt.set_obj_id(pSession_->GetObjectID());
            add_pkt.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
        }
        Protocol::s2c_LEAVE_OBJECT remove_pkt;
        {
            remove_pkt.set_is_player(true);
            remove_pkt.set_obj_id(pSession_->GetObjectID());
            remove_pkt.set_sector(0);
        }
        //auto& v = session_room->GetAdjSector();
        session_room->MoveBroadCast(pSession_
            , c2s_PacketHandler::MakeSendBuffer(add_pkt)
            , c2s_PacketHandler::MakeSendBuffer(remove_pkt)
            , c2s_PacketHandler::MakeSendBuffer(pkt)
            , session_room->GetAdjSector8());

        const auto& pSector = session_room->GetWorldChunk()->GetWorldSector(player->GetPos());
        if (session_room != pSector.get() && session_room->GetWorldChunk() == pSector->GetWorldChunk())
        {
            session_room->ImmigrationEnqueue(pSector, pSession_->GetSessionID());
            const Vec2 world_pos = TRWorld::GlobalToWorld(player->GetPos());
            const int y = (int)std::ceil(world_pos.y);
            const int x = (int)std::ceil(world_pos.x);
           // std::cout << y / SECTOR_HEIGHT << " , " << x / SECTOR_WIDTH << std::endl;
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
           player->GetComp<KeyInputHandler>()->SetKeyState(pkt_.vk_key(), (KeyInputHandler::KEY_STATE)(pkt_.key_state()));
        }
        return true;
    }

    const bool Handle_c2s_TRY_GET_ITEM(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_TRY_GET_ITEM& pkt_)
    {
        const auto session_room = static_cast<TRWorldRoom* const>(pSession_->GetCurSector());

        if (const auto& player = GetClientSession(pSession_)->GetPlayer())
        {
           //player->SetPos(ToOriginVec2(pkt_.obj_pos()));
           //player->Update(0.f);
            session_room->TryGetItemSector(GetClientSession(pSession_)->GetPlayer());
        }
        return true;
    }

    const bool Handle_c2s_TRY_NEW_ROOM(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_TRY_NEW_ROOM& pkt_)
    {
       const auto pClientSession = GetClientSession(pSession_);

       //const auto nextSector = pkt_.next_sector_num();

       const auto pCurRoom = static_cast<TRWorldRoom*>(pClientSession->GetCurSector());

       pCurRoom->GetWorldChunk()->ImigrationWorldChunk(pSession_, (CHUNK)pkt_.cur_sector_num());
       //auto pNextRoom = TRMgr(TRWorldMgr)->GetWorldRoom(static_cast<SECTOR>(nextSector));


     //  pCurRoom->ImmigrationEnqueue(std::move(pNextRoom), pClientSession->GetSessionID());

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
        return true;
        const auto cur_room = static_cast<TRWorldRoom* const>(pSession_->GetCurSector());
        const auto player = GetClientSession(pSession_)->GetPlayer();

        //const float dir = (player->GetPos().x - player->GetPrevPos().x) > 0.f ? 1.f : -1.f;

        ObjectBuilder b;
        b.ConvertProtoVec2AndSet(pkt_.obj_pos());
        b.sector = cur_room;
        b.dir = (float)pkt_.dir();

        auto pMissle = ObjectFactory::CreateMissle(b);
        

        cur_room->AddObjectEnqueue(GROUP_TYPE::PROJ_PLAYER, pMissle);

        Protocol::s2c_CREATE_MISSILE pkt;
        pkt.set_obj_id(pMissle->GetObjID());
        *pkt.mutable_obj_pos() = pkt_.obj_pos();

        //pMissle->PostUpdate(.2f);
        //cur_room << pkt;
       cur_room->BroadCastEnqueue(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));

          //Protocol::s2c_MOVE pkt2;
       //pkt2.set_obj_id(pMissle->GetObjID());
       //*pkt2.mutable_obj_pos() = pMissle->GetPos() + Vec2{ 1000.f,0.f }*0.1f;
       //
       //cur_room << pkt2;

        return true;
    }

    const bool Handle_c2s_CREATE_MONSTER(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_CREATE_MONSTER& pkt_)
    {
        
        const auto mon_pos = GetClientSession(pSession_)->GetPlayer()->GetPos();
      
        ObjectBuilder b;
        b.id = IDGenerator::GenerateID();
        b.pos = mon_pos;
        
       auto mon = ObjectFactory::CreateMonster(b);
      
       const auto pRoom = TRMgr(TRWorldMgr)->GetStartWorld()->GetWorldSector(mon_pos);
      
       pRoom->AddEnterEnqueue(GROUP_TYPE::MONSTER, mon);
      
       Protocol::s2c_CREATE_MONSTER pkt;
       pkt.set_obj_id(mon->GetObjID());
       *pkt.mutable_obj_pos() = mon_pos;
       pkt.set_sector(0);
       pkt.set_mon_name("ZOMBIE");
      // pRoom << pkt;
      pRoom->BroadCastEnqueue(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
       //pSession_ << pkt;
        return true;
    }

    const std::pair<std::string,std::string> items[] = {
       {{"Item_Pickaxe.png"},{"pickaxe_iron"}},
       { { "Item_Sword.png" },{"longsword_iron"} },
       { { "Item_Hammer.png" }, {"hammer_iron"} },
       { { "Item_Arrow.png" },{"Iron_Arrow"} },
       { { "Item_28.png" }, {"Item_28.png"} }
    };
    constexpr const int dmgs[] = {
        {100},
        {200},
        {300},
        {200},
        {200}
    };
    const bool Handle_c2s_BUY_ITEM(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_BUY_ITEM& pkt_)
    {
        const auto obj = GetClientSession(pSession_)->GetContentsEntity()->ObjectCast();
        const auto s = obj->GetComp<Status>();
        const auto& str = pkt_.item_name();
        std::string temp;
        for (int i = 0; i < str.size(); ++i)
        {
            if (isdigit(str[i]))
                temp.push_back(str[i]);
        }
        if (temp.empty())return true;
        const int idx = std::stoi(temp);
        if (0 > idx || idx >= 5)return true;
        const int price = TRMgr(Store)->GetItemPrice(items[idx].first);
        if (-1 == price)return true;
        const int cur_gold = s->GetCurrentGold();
        const int diff = cur_gold - price;
        Protocol::s2c_BUY_ITEM pkt;
        if (0 <= diff)
        {
            const std::string_view itemName = items[idx].first;
            
            pkt.set_item_name(items[idx].second);
            if (4 == idx)
            {
                obj->GetComp<Inventory>()->IncItem(items[idx].second);
                s->ModifyGold(-price,false);
            }
            else if (3 == idx)
            {
                if (!obj->GetComp<Inventory>()->GetItem(itemName))
                {
                    auto r = MakeShared<RangeAttack>(obj);
                    r->SetAtk(200);
                    r->SetSpeed(100.f);
                    r->SetRange(800.f);
                    s->ModifyGold(-price,false);
                    obj->GetComp<Inventory>()->AddItem(itemName,r);
                }
                else
                    pkt.set_item_name("CannotBuyItem");
            }
            else
            {
                if (!obj->GetComp<Inventory>()->GetItem(itemName))
                {
                    auto r = MakeShared<Attackable>(obj);
                    r->SetAtk(dmgs[idx]);
                    s->ModifyGold(-price,false);
                    obj->GetComp<Inventory>()->AddItem(itemName, r);
                }
                else
                    pkt.set_item_name("CannotBuyItem");

            }
        }
        else
        {
            pkt.set_item_name("NeedMoreGold");
        }
       // pSession_ << pkt;
        pSession_->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
        return true;
    }

    const bool Handle_c2s_CHAT(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_CHAT& pkt_)
    {
        if (const auto sector = (TRWorldRoom*)GetClientSession(pSession_)->GetContentsEntity()->ObjectCast()->m_pCurSector.load())
        {
            Protocol::s2c_CHAT pkt;
            pkt.set_id(pSession_->GetObjectID());
            pkt.set_msg(pkt_.msg());
            for (const auto s : sector->GetAdjSector8()) {
               // s << pkt;
                s->BroadCastEnqueue(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
            }
        }
        return true;
    }

    const bool Handle_c2s_PARTY_SINCHUNG(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_PARTY_SINCHUNG& pkt_)
    {
        const auto target_id = pkt_.target_id();
        if (const auto s = GetSession(target_id))
        {
            const auto cs = (ClientSession*)s.get();
            Protocol::s2c_PARTY_SINCHUNG pkt;
            pkt.set_sinchungid(pSession_->GetObjectID());
            pkt.set_sinchungname(GetClientSession(pSession_)->GetName());
           // cs << pkt;
            cs->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
        }
        return true;
    }

    const bool Handle_c2s_PARTY_SURAK(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_PARTY_SURAK& pkt_)
    {
        const auto target_id = pkt_.target_id();
        const auto s = GetSession(target_id);
        if (s)
        {
            const auto cs = (ClientSession*)s.get();
            Protocol::s2c_PARTY_SURAK pkt;
            pkt.set_is_surak(pkt_.is_surak());
            pkt.set_sinchungid(pkt_.target_id());
            pkt.set_surakgid(pSession_->GetObjectID());
            pkt.set_sinchungname(GetClientSession(pSession_)->GetName());
            if(pkt_.is_surak())
                cs->m_partyOne.store(GetClientSession(pSession_)->GetPlayer());
           // cs << pkt;
            cs->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
        }
        if(s)
        {
            Protocol::s2c_PARTY_SURAK pkt;
            pkt.set_is_surak(pkt_.is_surak());
            pkt.set_sinchungid(pkt_.target_id());
            pkt.set_surakgid(pSession_->GetObjectID());
            pkt.set_sinchungname(((ClientSession*)(s.get()))->GetName());
            if (pkt_.is_surak())
                GetClientSession(pSession_)->m_partyOne.store(((ClientSession*)s.get())->GetPlayer());
           // pSession_ << pkt;
            pSession_->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
        }
        return true;
    }

    const bool Handle_c2s_PARTY_OUT(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_PARTY_OUT& pkt_)
    {
        const auto target_id = pkt_.target_id();
        const auto s = GetSession(target_id);
        if (s)
        {
            const auto cs = (ClientSession*)s.get();
            Protocol::s2c_PARTY_OUT pkt;
            pkt.set_sinchungid(pSession_->GetObjectID());
            pkt.set_sinchungname(GetClientSession(pSession_)->GetName());
           //cs->m_partyOne.reset(nullptr);
            cs->m_partyOne.reset();
          //  cs << pkt;
            cs->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
        }
        if (s)
        {
            Protocol::s2c_PARTY_OUT pkt;
            pkt.set_sinchungid(target_id);
            pkt.set_sinchungname(((ClientSession*)(s.get()))->GetName());
            //GetClientSession(pSession_)->m_partyOne.store(nullptr);
            GetClientSession(pSession_)->m_partyOne.reset();
           // pSession_ << pkt;
            pSession_->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
        }
        return true;
    }

}

