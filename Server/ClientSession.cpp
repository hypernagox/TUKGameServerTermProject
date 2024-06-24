#include "ClientSession.h"
#include "c2s_PacketHandler.h"
#include "Object.h"
#include "SessionManageable.h"
#include "ObjectFactory.h"
#include "Inventory.h"
#include "ItemComponent.h"
#include "Status.h"
#include "DBMgr.h"
#include "DBBindRAII.h"
#include "DBPacket.h"
#include "TRWorldChunk.h"
#include "TRWorldMgr.h"
#include "TRWorldRoom.h"

using namespace ServerCore;
extern concurrency::concurrent_unordered_map<std::string, std::unique_ptr<std::atomic_bool>> g_connectHash;

ClientSession::ClientSession()
	:PacketSession{ ServerCore::c2s_PacketHandler::GetPacketHandlerList() }
{
}

ClientSession::~ClientSession()
{
	std::cout << "BYE" << std::endl;
}

void ClientSession::OnConnected()
{
}

void ClientSession::OnSend(c_int32 len)noexcept
{	
}

void ClientSession::OnDisconnected()
{
	if (m_pPlayer)
	{
		m_pPlayer->SetInvalid();
        m_pPlayer->ResetEntity();
		m_pPlayer.reset();
	}
	if (const auto pCurRoom = GetCurrentSessionRoomInfo().GetPtr())
	{
		GetCurrentSessionRoomInfo().GetPtr()->LeaveAndDisconnectEnqueue(GetSessionID());
	}
	else
	{
		//reset_cache_shared();
	}
    const auto iter = g_connectHash.find(m_name);
    if (g_connectHash.end() != iter)
    {
        auto& b = *iter->second;
        b.store(false);
    }
    if (const auto p = m_partyOne.load())
    {
        const auto target_id = p->GetObjID();
        Protocol::s2c_PARTY_OUT pkt;
        pkt.set_sinchungid(target_id);
        pkt.set_sinchungname(p->GetIocpEntity()->SharedFromThis<ClientSession>()->GetName());
        //this << pkt;
        this->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
        m_partyOne.reset();
    }
}

const std::string_view ClientSession::GetCurSelectItemName() const noexcept
{
	const auto inventory = m_pPlayer->GetComp<Inventory>();
	return inventory->GetCurSelectItemName();
}

void ClientSession::SetCurItemName(const std::string_view itemName) noexcept
{
	const auto inventory = m_pPlayer->GetComp<Inventory>();
	inventory->SetCurItem(itemName);
}

void ClientSession::UseCurItem(const float dt_) const noexcept
{
	if (const auto cur_item = m_pPlayer->GetComp<Inventory>()->GetCurItem())
	{
		cur_item->Use(dt_);
	}
}

Status*const ClientSession::GetStatus() const noexcept
{
	return m_pPlayer->GetComp<Status>();
}

void DB_PlayerLogin::ExecuteQuery() noexcept
{
    auto playerId = Utf8ToWide(m_pSession->GetName());

    dbCheck.BindParam(0, playerId);

    int count = 0;
    dbCheck.BindCol(0, count);

    ObjectBuilder b;
    b.session = m_pSession.get();
    b.id = m_pSession->GetObjectID();
    auto player = ObjectFactory::CreatePlayer(b);


    m_pSession->SetPlayer(player);
    static std::mutex mt;
    mt.lock();
    if (dbCheck.Execute() && dbCheck.Fetch() && count == 0)
    {
        dbCheck.UnBind();
        dbInsert.BindParam(0, playerId);
        if (!dbInsert.Execute())
        {
            mt.unlock();
            dbInsert.UnBind();
            m_bSuccess = false;
            return;
        }
        mt.unlock();
    }
    else
    {
        mt.unlock();
        dbCheck.UnBind();
        dbSelect.BindParam(0, playerId);

        int level, hp, exp, gold;

        dbSelect.BindCol(0, level);
        dbSelect.BindCol(1, hp);
        dbSelect.BindCol(2, exp);
        dbSelect.BindCol(3, gold);

        if (!dbSelect.Execute() || !dbSelect.Fetch())
        {
            dbSelect.UnBind();
            m_bSuccess = false;
            return;
        }
        dbSelect.UnBind();
        const auto stat = player->GetComp<Status>();

        stat->InitExp(exp);
        stat->InitGold(gold);
        stat->InitHP(hp);
        stat->InitLevel(level);
        stat->InitMaxExp(level * 10);

        Protocol::s2c_INIT_ALL_STAT statpkt;
        statpkt.set_exp(exp);
        statpkt.set_maxexp(level * 10);
        statpkt.set_hp(hp);
        statpkt.set_maxhp(200);
        statpkt.set_level(level);
        statpkt.set_gold(gold);

        m_pSession->SendOnlyEnqueue(c2s_PacketHandler::MakeSendBuffer(statpkt));

       

        const auto inventory = player->GetComp<Inventory>();


        dbItems.BindParam(0, playerId);

        int quantity = 0;
        
        char itemName[32]{};
      
        dbItems.BindCol(0, itemName, 32);
        dbItems.BindCol(1, quantity);
        dbItems.Execute();

        while (dbItems.Fetch())
        {
            if (0 > quantity)
            {
                if ("Item_Arrow.png" == std::string_view{ itemName })
                {
                    auto& item4 = inventory->AddItem("Item_Arrow.png", MakeShared<RangeAttack>(player.get()), false);
                    {
                        auto r = (RangeAttack*)item4.get();
                        r->SetAtk(200);
                        r->SetSpeed(100.f);
                        r->SetRange(800.f);
                    }
                }
                else
                {
                    if ("Item_Pickaxe.png" == std::string_view{ itemName })
                    {
                        auto& item = inventory->AddItem("Item_Pickaxe.png", MakeShared<Attackable>(player.get()), false);

                        ((Attackable*)item.get())->SetAtk(100);

                    }
                    else if ("Item_Sword.png" == std::string_view{ itemName })
                    {

                        auto& item2 = inventory->AddItem("Item_Sword.png", MakeShared<Attackable>(player.get()), false);
                        ((Attackable*)item2.get())->SetAtk(200);
                    }
                    else if ("Item_Hammer.png" == std::string_view{ itemName })
                    {
                        auto& item3 = inventory->AddItem("Item_Hammer.png", MakeShared<Attackable>(player.get()), false);
                        ((Attackable*)item3.get())->SetAtk(300);
                    }
                }
                Protocol::s2c_BUY_ITEM pkt;
                pkt.set_item_name(itemName);
                //m_pSession << pkt;
                m_pSession->SendAsync(c2s_PacketHandler::MakeSendBuffer(pkt));
            }
            else
            {
                if ("Item_28.png" == std::string_view{ itemName })
                {
                    auto item5 = (CountableItem*)inventory->AddItem("Item_28.png", MakeShared<HPPotion>(player.get()), false).get();
                    for (int i = 0; i < quantity; ++i) {
                        item5->IncCount();
                        Protocol::s2c_BUY_ITEM pkt;
                        pkt.set_item_name(itemName);
                       // m_pSession << pkt;
                        m_pSession->SendAsync(c2s_PacketHandler::MakeSendBuffer(pkt));
                    }
                }
                else
                {
                    auto item5 = (CountableItem*)inventory->AddItem(itemName, MakeShared<CountableItem>(player.get()), false).get();
                    for (int i = 0; i < quantity; ++i) {
                        item5->IncCount();
                        Protocol::s2c_BUY_ITEM pkt;
                        pkt.set_item_name(itemName);
                        //m_pSession << pkt;
                        m_pSession->SendAsync(c2s_PacketHandler::MakeSendBuffer(pkt));
                    }
                }
            }
           
        }
    }

    dbItems.UnBind();

    m_bSuccess = true;
}

void DB_PlayerLogin::Dispatch(ServerCore::IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
{
    if (!m_bSuccess)
    {
        m_pSession->Disconnect(L"");
        m_pSession->GetPlayer()->ResetEntity();
        return;
    }
    if (!m_pSession->IsConnectedAtomic())
    {
        m_pSession->GetPlayer()->ResetEntity();
        return;
    }
    //Protocol::s2c_LOGIN pkt3;
    //pkt3.set_seed(10);
    //pkt3.set_id(m_pSession->GetSessionID());
    //m_pSession << pkt3;
    //
    //s2q_LOGIN p;
    //p.user_id = m_pSession->GetSessionID();
    //strcpy_s(p.userName, m_pSession->GetName());
    //m_pSession->SendAsync(p.MakeSendBuffer());

    const auto tile_map = TRMgr(TRWorldMgr)->GetStartWorld()->GetTileMap();
    constexpr const int x = TRWorld::WORLD_WIDTH / 2;
    const auto init_pos = TRWorld::WorldToGlobal(Vec2Int(x, tile_map->GetTopYpos(x))) - Vec2(20.0f, 28.0f);
    const auto& player = m_pSession->GetPlayer();
    const auto pos = init_pos;
   
    const auto start_room = TRMgr(TRWorldMgr)->GetStartWorld()->GetWorldSector(pos);
    
    player->SetPos(init_pos);
    //
    //
    Protocol::s2c_ENTER pkt;
    pkt.set_player_id(m_pSession->GetObjectID());
    *pkt.mutable_pos() = pos;
    start_room->AddObjectEnqueue(GROUP_TYPE::PLAYER, player);
    start_room->EnterEnqueue(m_pSession->SharedFromThis<IocpEntity>());
    start_room->GetWorldChunk()->TransmitTileRecord(m_pSession);
    //

    m_pSession->SendAsync(c2s_PacketHandler::MakeSendBuffer(pkt));
   // m_pSession << pkt;

}
