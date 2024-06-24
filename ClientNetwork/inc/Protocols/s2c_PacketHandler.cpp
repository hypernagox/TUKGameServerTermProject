#include "pch.h"
#include "s2c_PacketHandler.h"
#include "ServerSession.h"
#include "NetworkMgr.h"
#include "TRWorld.h"
#include "TRItemManager.h"
#include "TRTileManager.h"
#include "CDropItem.h"
#include "CCollider.h"
#include "CEventMgr.h"
#include "CSceneMgr.h"
#include "CScene.h"
#include "Vec2Int.hpp"
#include "Missile.h"
#include "TRMain.h"
#include "CScene_Start.h"
#include "CSoundMgr.h"

extern int g_TR_SEED;
//extern TRWorld* g_TRWorld;
extern std::string id;

namespace NetHelper
{
    const bool Handle_Invalid(const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)
    {
        return false;
    }

    const bool Handle_s2c_LOGIN(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_LOGIN& pkt_)
    {
        g_TR_SEED = pkt_.seed();
        NetMgr(NetworkMgr)->SetSessionID(pkt_.id());
        //g_TRWorld->GetPlayer()->SetPlayerID(pkt_.id());
       
        return true;
    }

    const bool Handle_s2c_ENTER(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_ENTER& pkt_)
    {
        constexpr const int x = TRWorld::WORLD_WIDTH / 2;
        const Vec2 vPos = TRWorld::WorldToGlobal(Vec2Int(x, Mgr(TRMain)->active_world->GetTileMap()->GetTopYpos(x))) - Vec2(20.0f, 28.0f);
        //Mgr(TRMain)->scene_agent2->SetSector(5);
        //Mgr(TRMain)->active_world->AddNewPlayer(pkt_.player_id(),5,vPos);
        for (auto& p : Mgr(TRMain)->m_arrTRWorlds) {
            p->GetPlayer()->SetObjID(NetMgr(NetworkMgr)->GetSessionID());
            p->GetPlayer()->SetPos(::ToOriginVec2(pkt_.pos()));
            p->GetPlayer()->SetName(Utf8ToWide(id));
        }
        return true;
    }

    const bool Handle_s2c_SWING(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_SWING& pkt_)
    {
        if (const auto player = Mgr(TRMain)->active_world->GetOtherPlayer(pkt_.swing_user_id(),0))
        {
            player->SetCurWeapon(Utf8ToWide(pkt_.item_name()));
        }
        return false;
    }

    const bool Handle_s2c_BREAK_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_BREAK_TILE& pkt_)
    {
        if (pkt_.success())
        {
            Mgr(TRMain)->active_world->BreakTile(pkt_.tile_x(), pkt_.tile_y());
        }

        return true;
    }

    const bool NetHelper::Handle_s2c_BREAK_TILE_WALL(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_BREAK_TILE_WALL& pkt_)
    {
        if (pkt_.success())
        {
            Mgr(TRMain)->active_world->BreakTileWall(pkt_.tile_x(), pkt_.tile_y());
        }

        return true;
    }

    const bool NetHelper::Handle_s2c_PLACE_TILE(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_PLACE_TILE& pkt_)
    {
        const auto key = Utf8ToWide(pkt_.tile_key());
        if (pkt_.success())
        {
            const auto pTile = TRTileManager::GetInst()->GetTileByKey(key);
            Mgr(TRMain)->active_world->PlaceTile(pkt_.tile_x(), pkt_.tile_y(), pTile);
            Mgr(TRMain)->active_world->FindAndModifyItemStack(pkt_.tile_key(), -1);
        }
        return true;
    }

    const bool NetHelper::Handle_s2c_PLACE_TILE_WALL(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_PLACE_TILE_WALL& pkt_)
    {
        const auto key = Utf8ToWide(pkt_.tile_key());
        if (pkt_.success())
        {
            Mgr(TRMain)->active_world->PlaceTileWall(pkt_.tile_x(), pkt_.tile_y(), TRTileManager::GetInst()->GetTileWallByKey(key));
            Mgr(TRMain)->active_world->FindAndModifyItemStack(pkt_.tile_key(), -1,true);
        }
        return true;
    }

    const bool NetHelper::Handle_s2c_MOVE(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_MOVE& pkt_)
    {
        if (const auto other = Mgr(TRMain)->active_world->GetOtherPlayer(pkt_.obj_id(),5))
        {
            other->SetNewMoveData(pkt_);
        }
        else if(NetMgr(NetworkMgr)->GetSessionID()==pkt_.obj_id())
        {
            Mgr(TRMain)->active_world->GetPlayer()->SetNewMoveData(pkt_);
        }
        else
        {
          if (const auto pItem = Mgr(TRMain)->active_world->GetServerObject(pkt_.obj_id()))
          {
              pItem->SetNewMoveData(pkt_);
          }
        }
        return true;
    }

    const bool NetHelper::Handle_s2c_CREATE_ITEM(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_CREATE_ITEM& pkt_)
    {
       // Mgr(TRMain)->active_world->CreateItem(pkt_.obj_id(), ::ToOriginVec2(pkt_.pos()), pkt_.item_name(),pkt_.sector());
        return true;
    }

    const bool NetHelper::Handle_s2c_GET_ITEM(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_GET_ITEM& pkt_)
    {
        if (const auto pItem = Mgr(TRMain)->active_world->GetServerObject(pkt_.item_id()))
        {
            Mgr(TRMain)->active_world->EraseItem(pkt_.item_id());
            if (pSession_->GetSessionID() == pkt_.obj_id())
            {
                if (!pkt_.is_gold())
                    pItem->OnCollisionEnter(Mgr(TRMain)->active_world->GetPlayer()->GetComp<CCollider>());
                else
                {
                    Mgr(TRMain)->active_world->EraseItem(pkt_.item_id());
                    TRWorld::g_stat->GetGold();
                }
            }
        }
        return true;
    }

    const bool NetHelper::Handle_s2c_INPUT_KEY(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_INPUT_KEY& pkt_)
    {
        return false;
    }

    const bool NetHelper::Handle_s2c_TRY_GET_ITEM(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_TRY_GET_ITEM& pkt_)
    {
        return false;
    }

    const bool NetHelper::Handle_s2c_TRY_NEW_ROOM(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_TRY_NEW_ROOM& pkt_)
    {
        const int nextSector = pkt_.next_sector_num();
       // Mgr(CEventMgr)->AddEvent([nextSector]()
       //     {
       //        if(const auto player = Mgr(TRMain)->active_world->GetPlayer())
       //         {
       //             const int prevSector = sector;
       //             sector = nextSector;
       //             Mgr(CSceneMgr)->GetCurScene()->ChangeSector(nextSector);
       //             const int s = nextSector > prevSector ? 1 : -1;
       //             player->SetPos(player->GetPos() + Vec2{ 100.f, 0.f } *(float)s + Vec2{ 0.f,-100.f });
       //             player->SetWillPos(player->GetWillPos() + Vec2{ 100.f, 0.f } *(float)s + Vec2{ 0.f,-100.f });
       //         }
       //     });

        for (const auto p : Mgr(TRMain)->active_world->m_mapOtherPlayer | std::views::values)
            DeleteObj(p);
        Mgr(TRMain)->active_world->m_mapOtherPlayer.clear();
        for (const auto p : Mgr(TRMain)->active_world->m_mapServerObject | std::views::values)
            DeleteObj(p);
        Mgr(TRMain)->active_world->m_mapServerObject.clear();
        ChangeScene((SCENE_TYPE)sector);
       // Mgr(TRMain)->ChangeTRWorld();
       Mgr(CEventMgr)->AddEvent([]()
           {
               Mgr(TRMain)->ChangeTRWorld();
               Mgr(CEventMgr)->SetTRupdate(&TRMain::Update, Mgr(TRMain));
           });
        return true;
    }

    const bool NetHelper::Handle_s2c_ARRIVE_NEW_ROOM(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_ARRIVE_NEW_ROOM& pkt_)
    {
        return true;
    }

    const bool NetHelper::Handle_s2c_APPEAR_NEW_OBJECT(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_APPEAR_NEW_OBJECT& pkt_)
    {
        if (NetMgr(NetworkMgr)->GetSessionID() == pkt_.obj_id())
            return true;
        if (pkt_.is_player())
        {
           const auto player = Mgr(TRMain)->active_world->AddNewPlayer(pkt_.obj_id(), 0,ToOriginVec2(pkt_.appear_pos()),Utf8ToWide(pkt_.obj_name()));
           if (!player)return true;
           player->SetPos(ToOriginVec2(pkt_.appear_pos()));
           player->GetInterPolator().GetCurData().pos = ToOriginVec2(pkt_.appear_pos());
           player->GetInterPolator().GetNewData().pos = ToOriginVec2(pkt_.appear_pos());
           player->GetInterPolator().UpdateOnlyTimeStamp(pkt_.time_stamp());
        }
        else
        {
            const auto& str = pkt_.obj_name();
            if (str == "PLAYER")
                return true;
            if (str == "MONSTER")
            {
                Mgr(TRMain)->active_world->CreateMonster(pkt_.obj_id(), ToOriginVec2(pkt_.appear_pos()), str, 0);
                return true;
            }

            Mgr(TRMain)->active_world->CreateItem(pkt_.obj_id(), ::ToOriginVec2(pkt_.appear_pos()), str, 0);
        }
        return true;
    }

    const bool NetHelper::Handle_s2c_LEAVE_OBJECT(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_LEAVE_OBJECT& pkt_)
    {
        Mgr(TRMain)->active_world->EraseOtherPlayer(pkt_.obj_id(), pkt_.sector());
        Mgr(TRMain)->active_world->EraseItem(pkt_.obj_id());
       
        return true;
    }

    const bool NetHelper::Handle_s2c_CREATE_MISSILE(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_CREATE_MISSILE& pkt_)
    {
        Mgr(TRMain)->active_world->CreateMissle(pkt_.obj_id(), ToOriginVec2(pkt_.obj_pos()),pkt_.dir(),pkt_.speed());
        return true;
    }

    const bool NetHelper::Handle_s2c_CREATE_MONSTER(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_CREATE_MONSTER& pkt_)
    {
        Mgr(TRMain)->active_world->CreateMonster(pkt_.obj_id(), ToOriginVec2(pkt_.obj_pos()), pkt_.mon_name(), pkt_.sector());
        return true;
    }
    const bool Handle_s2c_DMG_INFO(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_DMG_INFO& pkt_)
    {
        Mgr(CSoundMgr)->PlayEffect("NPC_Hit_1.wav");
        Mgr(TRMain)->active_world->FloatDamageText(pkt_.dmg(), ::ToOriginVec2(pkt_.dmg_pos()), (COLORREF)0x000000D0);
        return true;
    }
    const bool Handle_s2c_STAT_MODIFY(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_STAT_MODIFY& pkt_)
    {
        const auto stat = pkt_.stat();
        const int val = pkt_.val();
        Mgr(TRMain)->active_world->g_stat->curID = pkt_.obj_id();
        switch (stat)
        {
        case Protocol::HP:
        {
            Mgr(TRMain)->active_world->g_stat->ModifyHP(val);
            Mgr(TRMain)->active_world->FindAndModifyItemStack("Item_28.png", -1);
            break;
        }
        case Protocol::EXP:
        {
            Mgr(TRMain)->active_world->g_stat->AddExp(val);
            break;
        }
        case Protocol::GOLD:
        {
            if(val >0)
                Mgr(TRMain)->active_world->g_stat->AddGold(val);
            else
                Mgr(TRMain)->active_world->g_stat->SubGold(val);
            break;
        }
        default:
            break;
        }
        return true;
    }
    const bool Handle_s2c_LEVEL_UP(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_LEVEL_UP& pkt_)
    {
        Mgr(TRMain)->active_world->g_stat->LevelUp(pkt_.exp());
        return true;
    }
    const bool Handle_s2c_BUY_ITEM(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_BUY_ITEM& pkt_)
    {
        const std::unordered_map<string,string> items = {
              {{"Item_Pickaxe.png"},{"pickaxe_iron"}},
              {{ "Item_Sword.png" },{"longsword_iron"} },
              {{ "Item_Hammer.png" }, {"hammer_iron"} },
              {{ "Item_Arrow.png" },{"Iron_Arrow"} },
              {{ "Item_28.png" }, {"Item_28.png"} }
        };

        const auto& itemName = pkt_.item_name();
        if ("NeedMoreGold" != itemName && "CannotBuyItem"!=itemName)
        {
            const auto iter = items.find(itemName);
            const auto ws = items.cend() != iter ? Utf8ToWide(iter->second) : Utf8ToWide(itemName);
            Mgr(TRMain)->active_world->AddItemToInventory(TRItemStack{ Mgr(TRItemManager)->GetItemByKey(ws),1 });
        }
        else
        {
            Mgr(TRMain)->active_world->FloatText(::Utf8ToWide(itemName), Mgr(TRMain)->active_world->GetPlayer()->GetPos(), RGB(255, 255, 0));
        }
        return true;
    }
    const bool Handle_s2c_INIT_ALL_STAT(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_INIT_ALL_STAT& pkt_)
    {
        auto& s = Mgr(TRMain)->active_world->g_stat;
        s->curID = pSession_->GetSessionID();
        s->m_exp = pkt_.exp();  
        s->m_gold = pkt_.gold();
        s->m_maxExp = pkt_.maxexp();
        s->m_level = pkt_.level();
        Mgr(TRMain)->active_world->GetPlayer()->SetHP(pkt_.hp());
        return true;
    }
    const bool Handle_s2c_CHAT(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_CHAT& pkt_)
    {
        if (const auto other = Mgr(TRMain)->active_world->GetOtherPlayer(pkt_.id(), 5))
        {
            Mgr(TRMain)->active_world->FloatText(Utf8ToWide(pkt_.msg()), other->GetPos(), RGB(0, 0, 0));
        }
        else if (NetMgr(NetworkMgr)->GetSessionID() == pkt_.id())
        {
            Mgr(TRMain)->active_world->FloatText(Utf8ToWide(pkt_.msg()), Mgr(TRMain)->active_world->GetPlayer()->GetPos(), RGB(0, 0, 0));
        }
        return true;
    }
    const bool Handle_s2c_LOGIN_FAIL(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_LOGIN_FAIL& pkt_)
    {
        const auto& m = pkt_.msg();
        if ("AlreadyConnected!" == m)
        {
            std::cout << m << std::endl;
            exit(0);
        }
        else if("Welcome!" == m)
        {
            std::cout << "Welcome !" << std::endl;
        }
        return true;
    }
    const bool Handle_s2c_PARTY_SINCHUNG(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_PARTY_SINCHUNG& pkt_)
    {
        const auto p = (Hero*)Mgr(TRMain)->active_world->GetPlayer();
        p->m_bPartyRequest = true;
        p->m_partyRequestUser = Utf8ToWide(pkt_.sinchungname());
        p->m_partySinchunza = pkt_.sinchungid();
        return true;
    }
    const bool Handle_s2c_PARTY_SURAK(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_PARTY_SURAK& pkt_)
    {
        const auto p = (Hero*)Mgr(TRMain)->active_world->GetPlayer();
        if (pkt_.is_surak() == false)
        {
            std::cout << "°ÅÀý !" << std::endl;
            p->m_bNowRequset = false;
            p->m_bPartyRequest = false;
            return true;
        }
        if (NetMgr(NetworkMgr)->GetSessionID() == pkt_.sinchungid())
        {
            p->m_bNowRequset = false;
            p->m_partyList.emplace(Utf8ToWide(pkt_.sinchungname()));
        }
        else
        {
            p->m_bPartyRequest = false;
            p->m_partyList.emplace(p->m_partyRequestUser);
        }
        return true;
    }
    const bool Handle_s2c_PARTY_OUT(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_PARTY_OUT& pkt_)
    {
        const auto p = (Hero*)Mgr(TRMain)->active_world->GetPlayer();
        if (NetMgr(NetworkMgr)->GetSessionID() == pkt_.sinchungid())
        {
            p->m_bNowRequset = false;
            p->m_partyList.erase(p->m_partyRequestUser);
        }
        else
        {
            p->m_bPartyRequest = false;
            p->m_partyList.erase(Utf8ToWide(pkt_.sinchungname()));
        }
        return true;
    }
}
