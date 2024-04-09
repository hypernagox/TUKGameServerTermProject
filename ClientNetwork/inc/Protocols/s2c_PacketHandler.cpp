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
        NetMgr(NetworkMgr)->SetSessionID(pkt_.id());
        //g_TRWorld->GetPlayer()->SetPlayerID(pkt_.id());

        return true;
    }

    const bool Handle_s2c_ENTER(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_ENTER& pkt_)
    {
        constexpr const int x = TRWorld::WORLD_WIDTH / 2;
        const Vec2 vPos = TRWorld::WorldToGlobal(Vec2Int(x, g_TRWorld->GetTileMap()->GetTopYpos(x))) - Vec2(20.0f, 28.0f);
        g_TRWorld->AddNewPlayer(pkt_.player_id(),0,vPos);
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
        return true;
    }

    const bool NetHelper::Handle_s2c_PLACE_TILE_WALL(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_PLACE_TILE_WALL& pkt_)
    {
        const auto key = Utf8ToWide(pkt_.tile_key());
        if (pkt_.success())
        {
            g_TRWorld->PlaceTileWall(pkt_.tile_x(), pkt_.tile_y(), TRTileManager::GetInst()->GetTileWallByKey(key));
            g_TRWorld->FindAndModifyItemStack(pkt_.tile_key(), -1,true);
        }
        return true;
    }

    const bool NetHelper::Handle_s2c_MOVE(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_MOVE& pkt_)
    {
        if (const auto other = g_TRWorld->GetOtherPlayer(pkt_.obj_id(),sector))
        {
            other->SetNewMoveData(pkt_);
        }
        else if(NetMgr(NetworkMgr)->GetSessionID()==pkt_.obj_id())
        {
            g_TRWorld->GetPlayer()->SetNewMoveData(pkt_);
        }
        else
        {
            if (const auto pItem = g_TRWorld->GetServerObject(pkt_.obj_id()))
            {
                pItem->SetNewMoveData(pkt_);
            }
        }
        return true;
    }

    const bool NetHelper::Handle_s2c_CREATE_ITEM(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_CREATE_ITEM& pkt_)
    {
        g_TRWorld->CreateItem(pkt_.obj_id(), ::ToOriginVec2(pkt_.pos()), pkt_.item_name(),pkt_.sector());
        return true;
    }

    const bool NetHelper::Handle_s2c_GET_ITEM(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_GET_ITEM& pkt_)
    {
        if (const auto pItem = g_TRWorld->GetServerObject(pkt_.item_id()))
        {
            g_TRWorld->EraseItem(pkt_.item_id());
            if(pSession_->GetSessionID() == pkt_.obj_id())
                pItem->OnCollisionEnter(g_TRWorld->GetPlayer()->GetComp<CCollider>());
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
        Mgr(CEventMgr)->AddEvent([nextSector]()
            {
               if(const auto player = g_TRWorld->GetPlayer())
                {
                    const int prevSector = sector;
                    sector = nextSector;
                    Mgr(CSceneMgr)->GetCurScene()->ChangeSector(nextSector);
                    const int s = nextSector > prevSector ? 1 : -1;
                    player->SetPos(player->GetPos() + Vec2{ 100.f, 0.f } *(float)s + Vec2{ 0.f,-100.f });
                    player->SetWillPos(player->GetWillPos() + Vec2{ 100.f, 0.f } *(float)s + Vec2{ 0.f,-100.f });
                }
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
           const auto player = g_TRWorld->AddNewPlayer(pkt_.obj_id(), pkt_.sector(),ToOriginVec2(pkt_.appear_pos()));
           player->SetPos(ToOriginVec2(pkt_.appear_pos()));
           player->GetInterPolator().GetCurData().pos = ToOriginVec2(pkt_.appear_pos());
           player->GetInterPolator().GetNewData().pos = ToOriginVec2(pkt_.appear_pos());
           player->GetInterPolator().UpdateOnlyTimeStamp(pkt_.time_stamp());
        }
        return true;
    }

    const bool NetHelper::Handle_s2c_LEAVE_OBJECT(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_LEAVE_OBJECT& pkt_)
    {
        g_TRWorld->EraseOtherPlayer(pkt_.obj_id(), pkt_.sector());
        return true;
    }

    const bool NetHelper::Handle_s2c_CREATE_MISSILE(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_CREATE_MISSILE& pkt_)
    {
        g_TRWorld->CreateMissle(pkt_.obj_id(), ToOriginVec2(pkt_.obj_pos()));
        return true;
    }

    const bool NetHelper::Handle_s2c_CREATE_MONSTER(const S_ptr<PacketSession>& pSession_, const Protocol::s2c_CREATE_MONSTER& pkt_)
    {
        g_TRWorld->CreateMonster(pkt_.obj_id(), ToOriginVec2(pkt_.obj_pos()), pkt_.mon_name(), pkt_.sector());
        return true;
    }
}
