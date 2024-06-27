#include "pch.h"

#include "TRWorld.h"
#include "TRTileManager.h"
#include "TRTile.h"
#include "ThreadMgr.h"
#include "Session.h"
#include "c2s_PacketHandler.h"
#include "ObjectFactory.h"
#include "TRWorldMgr.h"
#include "TRWorldChunk.h"
#include "TRWorldRoom.h"
#include "Object.h"

//#include "CTimeMgr.h"
//#include "CKeyMgr.h"
//#include "CCamera.h"
//#include "CScene.h"
//#include "CQuickBarVisualizer.h"
//#include "CHealthIndicator.h"
//#include "CInventoryVisualizer.h"
//#include "CAcquireItemText.h"
//
//#include "CMonster.h"
//#include "TRItemManager.h"
//#include "TRItemStack.h"
//#include "CCollisionMgr.h"
//#include "CZombie.h"
//#include "CSlime.h"
//#include "CEyeMonster.h"
//#include "CCthulhuEye.h"
//
//#include "Vec2Int.hpp"
//#include "CustomMath.hpp"
//#include "CWeapon.h"
//#include "TRMonGenerator.h"
//#include "CMiniMap.h"
//#include "CDropItem.h"
//#include "CSoundMgr.h"
//
//#include "CParticleMgr.h"

TRWorld* g_TRWorld = nullptr;
TRWorld* g_TRWorldMain = nullptr;
//extern bool g_bStopToken;

static std::mt19937 randDigSound{std::random_device{}()};
static std::uniform_int_distribution<> uidDig{0, 2};

TRWorld::TRWorld(int seed)
{
	if (0 == seed)
	{
		g_TRWorld = this;
		TRMgr(TRTileManager)->LoadTiles();
		tile_map = new TRTileMap(TRWorld::WORLD_WIDTH, TRWorld::WORLD_HEIGHT);
	}
	else
	{
		g_TRWorldMain = this;
		tile_map = new TRTileMap(TRWorld::WORLD_WIDTH, TRWorld::WORLD_HEIGHT);
	}
	
	//for (int i = 0; i < 50; ++i)
	//	player_inventory[i] = new TRItemContainer();
	//for (int i = 0; i < 3; ++i)
	//	player_armor[i] = new TRItemContainer();
	//for (int i = 0; i < 10; ++i)
	//	quick_bar[i] = player_inventory[i];
	//
	//quick_bar_visualizer = new CQuickBarVisualizer(quick_bar);
	//quick_bar_visualizer->SetPos(Vec2Int(10, 24));
	//
	//inventory_visualizer = new CInventoryVisualizer(player_inventory, player_armor);
	//inventory_visualizer->SetPos(Vec2Int(10, 24));
	//
	//health_indicator = new CHealthIndicator();
	//health_indicator->SetPos(Vec2Int(1120, 10));
	//
	////for (int id = 0; id < 17; ++id)
	////	player_inventory[id]->Apply(TRItemStack(Mgr(TRItemManager)->GetItemByID(id), 10));
	//
	//quick_bar_index = 0;
	//SetToggleInventory(false);
	//
	//player_inventory[0]->Apply(TRItemStack(Mgr(TRItemManager)->GetItemByKey(L"pickaxe_iron"), 1));
	//player_inventory[1]->Apply(TRItemStack(Mgr(TRItemManager)->GetItemByKey(L"hammer_iron"), 1));
	//player_inventory[2]->Apply(TRItemStack(Mgr(TRItemManager)->GetItemByKey(L"longsword_iron"), 1));
	//
	//
	//Mgr(CParticleMgr)->Init();
}

TRWorld::~TRWorld()
{
	delete tile_map;

	//for (int i = 0; i < 50; ++i)
	//	delete player_inventory[i];
	//for (int i = 0; i < 3; ++i)
	//	delete player_armor[i];
	//
	//g_TRWorld = nullptr;
}

void TRWorld::Update()
{
	//if (KEY_TAP(KEY::ONE))
	//	SwitchQuickBarIndex(0);
	//else if (KEY_TAP(KEY::TWO))
	//	SwitchQuickBarIndex(1);
	//else if (KEY_TAP(KEY::THREE))
	//	SwitchQuickBarIndex(2);
	//else if (KEY_TAP(KEY::FOUR))
	//	SwitchQuickBarIndex(3);
	//else if (KEY_TAP(KEY::FIVE))
	//	SwitchQuickBarIndex(4);
	//else if (KEY_TAP(KEY::SIX))
	//	SwitchQuickBarIndex(5);
	//else if (KEY_TAP(KEY::SEVEN))
	//	SwitchQuickBarIndex(6);
	//else if (KEY_TAP(KEY::EIGHT))
	//	SwitchQuickBarIndex(7);
	//else if (KEY_TAP(KEY::NINE))
	//	SwitchQuickBarIndex(8);
	//else if (KEY_TAP(KEY::ZERO))
	//	SwitchQuickBarIndex(9);
	//else if (KEY_TAP(KEY::ESC))
	//	SetToggleInventory(!toggle_inventory);
	//
	//
	//if (Mgr(CKeyMgr)->GetMouseWheelUp())
	//{
	//	SwitchQuickBarIndex(wrapAround(quick_bar_index + 1, 0, 10));
	//}
	//
	//if (Mgr(CKeyMgr)->GetMouseWheelDown())
	//{
	//	SwitchQuickBarIndex(wrapAround(quick_bar_index - 1, 0, 10));
	//}
	//
	//health_indicator->SetHealthValue(player->GetHP());
	//
	//if (KEY_TAP(KEY::LBTN))
	//{
	//	bool result = inventory_visualizer->HandleMouseInput();
	//	if (result)
	//	{
	//		Mgr(CSoundMgr)->PlayEffect("Grab.wav", 0.5f);
	//	}
	//	else if (!quick_bar[quick_bar_index]->Blank())
	//	{
	//		Vec2 mouse_world_pos = TRWorld::GlobalToWorld(Mgr(CCamera)->GetRealPos(Mgr(CKeyMgr)->GetMousePos()));
	//		player->UseItem();
	//		bool use_item = quick_bar[quick_bar_index]->GetItemStack().GetItem()->OnUseItem(player, this, mouse_world_pos);
	//		if (use_item)
	//			quick_bar[quick_bar_index]->Apply(-1);
	//	}
	//}
	//
	//Mgr(CParticleMgr)->Update();
	//
	//TRMonGenerator::GenerateMonster();
}

void TRWorld::InitMonsters(const CHUNK eChunk)
{
	static std::default_random_engine dre;
	constexpr int CNT = 1000;
	std::vector<Vec2Int> arr;
	for (int i = 0; i < CNT; ++i)
	{
		std::sample(nodes.begin(), nodes.end(), std::back_inserter(arr), 1, dre);
	}
	for (const auto v : arr)
	{
		ObjectBuilder b;
		b.pos = TRWorld::WorldToGlobal(v);
		auto mon = ObjectFactory::CreateMonster(b);
		TRMgr(TRWorldMgr)->GetWorldChunk(eChunk)->GetWorldSector(b.pos)->AddObjectEnqueue(GROUP_TYPE::MONSTER, mon);
		TRMgr(TRWorldMgr)->GetWorldChunk(eChunk)->GetWorldSector(b.pos)->EnterEnqueue(mon->GetIocpEntity().get());
	}
}

void TRWorld::CreateWorld(int seed)
{
	TRWorldGeneration* generator = new TRWorldGeneration();

	TRWorldGenerationProcess* processes[] =
	{
		new TRWorldGenerationTerrainHeight(),
		new TRWorldGenerationAttachWall(),
		new TRWorldGenerationPinchCaves(),
		new TRWorldGenerationGrowOres(TRMgr(TRTileManager)->GetTileByKey(L"copper_ore"), -20),
		new TRWorldGenerationGrowOres(TRMgr(TRTileManager)->GetTileByKey(L"iron_ore"), 0),
		new TRWorldGenerationGrowOres(TRMgr(TRTileManager)->GetTileByKey(L"silver_ore"), 10),
		new TRWorldGenerationGrowOres(TRMgr(TRTileManager)->GetTileByKey(L"gold_ore"), 30),
		new TRWorldGenerationGrowGrass()
	};

	for (int i = 0; i < sizeof(processes) / sizeof(*processes); ++i)
		generator->AddProcess(processes[i]);
	generator->GenerateWorld(tile_map, TRWorld::WORLD_WIDTH, TRWorld::WORLD_HEIGHT, seed);
	delete generator;
	for (int i = 0; i < sizeof(processes) / sizeof(*processes); ++i)
		delete processes[i];

	for (int y = 0; y < TRWorld::WORLD_HEIGHT; ++y)
	{
		for (int x = 0; x < TRWorld::WORLD_WIDTH; ++x)
		{
			m_bitSolid[y][x] = tile_map->GetTile(x, y)->Solid();
		}
	}

	int prev_level = -1;
	std::bitset<WORLD_HEIGHT> flag{ 0 };

	//for (int x = 0; x < TRWorld::WORLD_WIDTH; ++x)
	//{
	//	for (int y = TRWorld::WORLD_HEIGHT-1; y >=0 ; --y)
	//	{
	//		if (!flag[y] && m_bitSolid[y][x])
	//		{
	//			flag[y] = 1;
	//			++y;
	//			level_nodes[x].level = y;
	//			level_nodes[x].pos.emplace_back(x, y);
	//			if (-1 != prev_level && prev_level != y)
	//			{
	//				const int target_x = x - (prev_level < y);
	//				int temp_y = prev_level < y ? prev_level + 1 : y + 1;
	//				level_nodes[target_x].pos.emplace_back(target_x, temp_y);
	//				flag[temp_y] = 1;
	//				temp_y += 1;
	//				while (prev_level > temp_y)
	//				{
	//					flag[temp_y] = 1;
	//					level_nodes[target_x].pos.emplace_back(target_x, temp_y);
	//					temp_y += 1;
	//				} 
	//			}
	//			--y;
	//			prev_level = y;
	//			break;
	//		}
	//	}
	//}
	for (int x = 0; x < TRWorld::WORLD_WIDTH; ++x)
	{
		for (int y = TRWorld::WORLD_HEIGHT - 1; y >= 0; --y)
		{
			if (m_bitSolid[y][x])
			{
				nodes.emplace_back(Vec2Int{ x,y + 1 });
				break;
			}
		}
	}
	std::cout << "!!" << std::endl;
}	

void TRWorld::OnSceneCreate()
{
	//if (g_bStopToken)
	//{
	//	return;
	//}

	//_pScene = scene;
	//gr(CCollisionMgr)->Reset();
	//layer = new CPlayer(this);
	//int x = TRWorld::WORLD_WIDTH / 2;
	//player->SetPos(TRWorld::WorldToGlobal(Vec2Int(x, tile_map->GetTopYpos(x))) - Vec2(20.0f, 28.0f));
	//player->SetScale(Vec2{ 40.f, 56.f });
	//scene->AddObject(player, GROUP_TYPE::PLAYER);
	//Mgr(CCamera)->SetTarget(player);
	//scene->RegisterPlayer(player);

	//TRItemStack dropitem_list[] = {
	//	TRItemStack(TRMgr(TRItemManager)->GetItemByKey(L"tile_planks_wood"), 100),
	//	TRItemStack(TRMgr(TRItemManager)->GetItemByKey(L"tile_bricks_stone"), 100),
	//	TRItemStack(TRMgr(TRItemManager)->GetItemByKey(L"tile_bricks_clay"), 100),
	//	TRItemStack(TRMgr(TRItemManager)->GetItemByKey(L"tile_torch"), 100),
	//	TRItemStack(TRMgr(TRItemManager)->GetItemByKey(L"wall_planks_wood"), 100),
	//	TRItemStack(TRMgr(TRItemManager)->GetItemByKey(L"wall_bricks_stone"), 100),
	//	TRItemStack(TRMgr(TRItemManager)->GetItemByKey(L"wall_bricks_clay"), 100),
	//	TRItemStack(TRMgr(TRItemManager)->GetItemByKey(L"armor_iron_head"), 1),
	//	TRItemStack(TRMgr(TRItemManager)->GetItemByKey(L"armor_iron_chestplate"), 1),
	//	TRItemStack(TRMgr(TRItemManager)->GetItemByKey(L"armor_iron_leggings"), 1),
	//	TRItemStack(TRMgr(TRItemManager)->GetItemByKey(L"summon_cthulhueye"), 1)
	//};

	//for (int i = 0; i < sizeof(dropitem_list) / sizeof(*dropitem_list); ++i)
	//	DropItem(Vec2Int(x - 10 + i * -4, 255), dropitem_list[i]);
	//
	//tile_map->OnSceneCreate(scene);
	//
	//Mgr(CCollisionMgr)->RegisterGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::MONSTER); 
	//Mgr(CCollisionMgr)->RegisterGroup( GROUP_TYPE::MONSTER,GROUP_TYPE::PLAYER_WEAPON);
	//Mgr(CCollisionMgr)->RegisterGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::DROP_ITEM);
	//scene->AddObject(quick_bar_visualizer, GROUP_TYPE::UI);
	//quick_bar_visualizer->AddContainerVisualizers(scene);
	//scene->AddObject(inventory_visualizer, GROUP_TYPE::UI);
	//inventory_visualizer->AddContainerVisualizers(scene);
	//scene->AddObject(health_indicator, GROUP_TYPE::UI);
	//
	//{
	//	auto pMap = new CMiniMap;
	//	pMap->CreateMiniMap();
	//	scene->AddObject(pMap, GROUP_TYPE::UI);
	//}

	//Mgr(CCamera)->update();
}

//CPlayer* TRWorld::GetPlayer() const
//{
//	return player;
//}

bool TRWorld::PlaceTile(int x, int y, TRTile* new_tile, std::string_view tile_key, std::wstring& name)
{
	const int th_idx = Mgr(ThreadMgr)->GetCurThreadIdx();

	TRTile* const tile = tile_map->GetTile(x, y);

	if (tile == nullptr)
		return false;
	if (tile->Solid())
		return false;

	constexpr const int dir[][2] = { 0, 1, 0, -1, -1, 0, 1, 0 };
	int bitmask = 0;

	for (int k = 0; k < 4; ++k)
	{
		const int xp = x + dir[k][0];
		const int yp = y + dir[k][1];

		TRTile* const tile_p = tile_map->GetTile(xp, yp);
		if (tile_p == nullptr)
			continue;

		if (tile_p->Solid())
			bitmask |= 1 << k;
	}

	TRTileWall* const tile_wall_p = tile_map->GetTileWall(x, y);

	if (bitmask == 0 && tile_wall_p == TRMgr(TRTileManager)->TileWallAir())
		return false;

	//switch (uidDig(randDigSound))
	//{
	//case 0:Mgr(CSoundMgr)->PlayEffect("Dig_0.wav", 0.5f); break;
	//case 1:Mgr(CSoundMgr)->PlayEffect("Dig_1.wav", 0.5f); break;
	//case 2:Mgr(CSoundMgr)->PlayEffect("Dig_2.wav", 0.5f); break;
	//default:
	//	break;
	//}

	{
		std::lock_guard<ServerCore::SRWLock> lock{ m_tileWorldLock };
		tile_map->SetTile(x, y, new_tile, true);
		m_bitSolid[y][x] = 1;
		const Vec2Int key{ x,y };
		const auto iter = m_setEraseRecords.find(key);
		if (m_setEraseRecords.cend() != iter)
		{
			m_setEraseRecords.erase(iter);
		}
		m_mapCreateRecords.emplace(key, TileInfo{ x, y, tile_key });
	}
	name = new_tile->DropItem();
	//m_recordLock[1][th_idx].lock();
	//m_vecCreateRecords[th_idx].tile_record.emplace_back(x, y, tile_key);
	//m_recordLock[1][th_idx].unlock();

	return true;
}

bool TRWorld::BreakTile(int x, int y, std::string& outName)
{
	const int th_idx = Mgr(ThreadMgr)->GetCurThreadIdx();
	const TRTile* tile;
	TRTile* const air_tile = TRTileManager::GetInst()->TileAir();
	{
		std::lock_guard<ServerCore::SRWLock> lock{ m_tileWorldLock };
		tile = tile_map->GetTile(x, y);

		if (tile == nullptr)
			return false;

		

		if (tile == air_tile)
			return false;

		//char buffer[16];
		//sprintf_s(buffer, "%s_%d.wav", tile->Rocky() ? "Tink" : "Dig", uidDig(randDigSound));
		//Mgr(CSoundMgr)->PlayEffect(buffer, 0.5f);

		tile_map->SetTile(x, y, air_tile, true);
		m_bitSolid[y][x] = 0;
		
		const Vec2Int key{ x,y };
		const auto iter = m_mapCreateRecords.find(key);
		if (m_mapCreateRecords.cend()!=iter)
		{
			m_mapCreateRecords.erase(iter);
		}
		else
		{
			m_setEraseRecords.emplace(key);
		}
	}
	const Vec2 vParticlePos = TRWorld::WorldToGlobal(Vec2{ (float)x,(float)y });
	//CAtlasElement* pImg = tile->GetTileImg();
	//Mgr(CParticleMgr)->SetParticles(vParticlePos, pImg);

	const auto& k_dropitem = tile->DropItem();
	if (k_dropitem == L"")
		return true;

	outName =  WideToUtf8(k_dropitem);

	//TRItem* p_dropitem = Mgr(TRItemManager)->GetItemByKey(k_dropitem);
	//DropItem(Vec2(x + 0.5f, y + 0.5f), TRItemStack(p_dropitem, 1));
	return true;
}

bool TRWorld::PlaceTileWall(int x, int y, TRTileWall* new_tile)
{
	TRTileWall* const tile = tile_map->GetTileWall(x, y);
	TRTileWall* const air_tile = TRMgr(TRTileManager)->TileWallAir();

	if (tile == nullptr)
		return false;
	if (tile != air_tile)
		return false;

	constexpr const int dir[][2] = { 0, 1, 0, -1, -1, 0, 1, 0 };
	int bitmask = 0;

	for (int k = 0; k < 4; ++k)
	{
		int xp = x + dir[k][0];
		int yp = y + dir[k][1];

		TRTile* tile_p = tile_map->GetTile(xp, yp);
		TRTileWall* tile_wall_p = tile_map->GetTileWall(xp, yp);

		if (tile_p != nullptr)
		{
			if (tile_p->Solid())
				bitmask |= 1 << k;
		}
		if (tile_wall_p != nullptr)
		{
			if (tile_wall_p != air_tile)
				bitmask |= 1 << k;
		}
	}

	TRTile* tile_p = tile_map->GetTile(x, y);

	if (bitmask == 0 && !tile_p->Solid())
		return false;
	
	//char buffer[16];
	//sprintf_s(buffer, "%s_%d.wav", "Dig", uidDig(randDigSound));
	//Mgr(CSoundMgr)->PlayEffect(buffer, 0.5f);
	//
	{
		std::lock_guard<ServerCore::SRWLock> lock{ m_tileWallWorldLock };
		tile_map->SetTileWall(x, y, new_tile, true);
	}
	return true;
}

bool TRWorld::BreakTileWall(int x, int y)
{
	const TRTileWall* tile;
	{
		std::lock_guard<ServerCore::SRWLock> lock{ m_tileWallWorldLock };

		tile = tile_map->GetTileWall(x, y);

		if (tile == nullptr)
			return false;

		TRTileWall* const air_tile = TRMgr(TRTileManager)->TileWallAir();

		if (tile == air_tile)
			return false;

		//char buffer[16];
		//sprintf_s(buffer, "%s_%d.wav", "Dig", uidDig(randDigSound));
		//Mgr(CSoundMgr)->PlayEffect(buffer, 0.5f);

		tile_map->SetTileWall(x, y, air_tile, true);
	}

	const auto& k_dropitem = tile->DropItem();
	if (k_dropitem == L"")
		return true;

	//TRItem* p_dropitem = Mgr(TRItemManager)->GetItemByKey(k_dropitem);
	//DropItem(Vec2(x + 0.5f, y + 0.5f), TRItemStack(p_dropitem, 1));
	return true;
}

//void TRWorld::DropItem(Vec2 world_pos, TRItemStack item)
//{
//	if (item.Null())
//		return;
//
//	CDropItem* drop_item = new CDropItem(this, item);
//	drop_item->SetPos(TRWorld::WorldToGlobal(world_pos));
//
//	m_pScene->AddObject(drop_item, GROUP_TYPE::DROP_ITEM);
//}

void TRWorld::SetToggleInventory(bool value)
{
	//if (toggle_inventory != value)
	//{
	//	if (value)
	//	{
	//		Mgr(CSoundMgr)->PlayEffect("Menu_Open.wav", 0.5f);
	//	}
	//	else
	//	{
	//		Mgr(CSoundMgr)->PlayEffect("Menu_Close.wav", 0.5f);
	//	}
	//}
	//toggle_inventory = value;
	//quick_bar_visualizer->SetVisible(!value);
	//inventory_visualizer->SetVisible(value);
}

//void TRWorld::AddItemToInventory(TRItemStack item)
//{
//	//TRItemStack return_item = item;
//	//
//	//for (auto& container : player_inventory)
//	//{
//	//	if (container->Blank() || container->GetItemStack().GetItem() == item.GetItem())
//	//		return_item = container->Apply(return_item);
//	//
//	//	if (return_item.Null())
//	//		break;
//	//}
//	//
//	//static wchar_t buffer[64];
//	//if (item.GetStackSize() > 1)
//	//	wsprintf(buffer, L"%s (%d)", item.GetItem()->GetName().c_str(), item.GetStackSize());
//	//else
//	//	wsprintf(buffer, L"%s", item.GetItem()->GetName().c_str());
//	//
//	//CAcquireItemText* text = new CAcquireItemText(buffer);
//	//text->SetPos(player->GetPos() - Vec2::up * 24.0f);
//	//m_pScene->AddObject(text, GROUP_TYPE::DROP_ITEM);
//}

void TRWorld::SwitchQuickBarIndex(int value)
{
	//if (quick_bar_index == value)
	//	return;
	//quick_bar_index = value;
	//
	//quick_bar_visualizer->SetSelectIndex(quick_bar_index);
	//Mgr(CSoundMgr)->PlayEffect("Menu_Tick.wav", 0.5f);
}

int TRWorld::GetArmorPoint() const
{
	//int armor_point = 0;
	//
	//for (TRItemContainer* container : player_armor)
	//{
	//	if (container->Blank())
	//		continue;
	//
	//	TRItemArmor* armor = dynamic_cast<TRItemArmor*>(container->GetItemStack().GetItem());
	//	if (armor != nullptr)
	//		armor_point += armor->GetArmorPoint();
	//}
	//
	return 0;
}

void TRWorld::FloatDamageText(int value, Vec2 vPos, COLORREF color)
{
	//static wchar_t buffer[64];
	//wsprintf(buffer, L"%d", value);
	//
	//CAcquireItemText* text = new CAcquireItemText(buffer, color);
	//text->SetPos(vPos);
	//m_pScene->AddObject(text, GROUP_TYPE::DROP_ITEM);
}

void TRWorld::SpawnBoss()
{
	//auto pMon = new CCthulhuEye{ this,L"Monster_CthulhuEye", L"NPC_4.png" };
	//pMon->SetPos(TRWorld::WorldToGlobal(Vec2(TRWorld::WORLD_WIDTH / 2, TRWorld::WORLD_HEIGHT)));
	//pMon->SetScale(Vec2{ 110.0f, 166.0f });
	//pMon->SetColliderScale(Vec2{ 110.0f, 110.0f });
	//
	//m_pScene->AddObject(pMon, GROUP_TYPE::MONSTER);
	//
	//Mgr(CSoundMgr)->PlayBGM("04. Boss 1.mp3", 0.5f);
	//
	//Mgr(CCamera)->FadeOut(1.f);
	//Mgr(CCamera)->FadeIn(1.f);
	//
	//StartCoEvent(Mgr(CCamera)->ZoomInBoss(pMon->GetPos()));
}

void TRWorld::TransmitTileRecord(const ServerCore::S_ptr<ServerCore::Session>& pSession_) const noexcept
{
	m_tileWorldLock.lock_shared();
	for (int i = 0; i < ServerCore::NUM_OF_THREADS; ++i)
	{
		for (const auto& [key, tile] : m_mapCreateRecords)
		{
			Protocol::s2c_PLACE_TILE pkt;
			pkt.set_success(true);
			pkt.set_tile_x(tile.x);
			pkt.set_tile_y(tile.y);
			pkt.set_tile_key(tile.tile_name);
			//pSession_ << pkt;
			pSession_->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
		}
		for (const auto key : m_setEraseRecords)
		{
			Protocol::s2c_BREAK_TILE pkt;
			pkt.set_success(true);
			pkt.set_tile_x(key.x);
			pkt.set_tile_y(key.y);
			//pSession_ << pkt;
			pSession_->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
		}
	}
	m_tileWorldLock.unlock_shared();
}

Protocol::s2c_MOVE TRWorld::updateTileCollision(const Protocol::c2s_MOVE& pkt_) const noexcept
{
	

	const auto pTileMap = GetTileMap();

	const Vec2 world_pos = TRWorld::GlobalToWorld(ToOriginVec2(pkt_.wiil_pos()));

	const Vec2 world_vel = ToOriginVec2(pkt_.vel());

	const auto vScale = ToOriginVec2(pkt_.scale());

	const float w = vScale.x / (float)PIXELS_PER_TILE;
	const float h = vScale.y / (float)PIXELS_PER_TILE;

	const Vec2 pre_pos = TRWorld::GlobalToWorld(ToOriginVec2(pkt_.obj_pos()));

	
	Vec2 post_pos = world_pos;
	Vec2 post_vel = world_vel;

	bool landed = false;
	bool collided = false;

	int x_min = FloorToInt(pre_pos.x - w * 0.5f);
	int x_max = CeilToInt(pre_pos.x + w * 0.5f) - 1;
	int y_min = FloorToInt(world_pos.y - h * 0.5f);
	int y_max = CeilToInt(world_pos.y + h * 0.5f) - 1;

	if (x_min >= 0 && x_max < TRWorld::WORLD_WIDTH && y_min >= 0 && y_max < TRWorld::WORLD_HEIGHT)
	{
		for (int x = x_min; x <= x_max; ++x)
		{
			if (world_vel.y > 0.0f && pTileMap->GetTile(x, y_min)->Solid())
			{
				post_pos.y = y_min + 1.0f + h * 0.5f;
				post_vel.y = 0.0f;

				landed = true;
				collided = true;
				break;
			}
			if (world_vel.y < 0.0f && pTileMap->GetTile(x, y_max)->Solid())
			{
				post_pos.y = y_max - h * 0.5f;
				post_vel.y = 0.0f;

				collided = true;
				break;
			}
		}
	}

	if (world_pos.x - w * 0.5f < 0.0f)
	{
		post_pos.x = w * 0.5f;
		post_vel.x = 0.0f;
	}
	if (world_pos.x + w * 0.5f > TRWorld::WORLD_WIDTH)
	{
		post_pos.x = TRWorld::WORLD_WIDTH - w * 0.5f;
		post_vel.x = 0.0f;
	}

	x_min = FloorToInt(post_pos.x - w * 0.5f);
	x_max = CeilToInt(post_pos.x + w * 0.5f) - 1;
	y_min = FloorToInt(post_pos.y - h * 0.5f);
	y_max = CeilToInt(post_pos.y + h * 0.5f) - 1;

	if (x_min >= 0 && x_max < TRWorld::WORLD_WIDTH && y_min >= 0 && y_max < TRWorld::WORLD_HEIGHT)
	{
		bool collision_x = false;
		float reform_x = 0.0f;

		for (int y = y_min; y <= y_max; ++y)
		{
			if (world_vel.x < 0.0f && pTileMap->GetTile(x_min, y)->Solid())
			{
				reform_x = x_min + 1.0f + w * 0.5f;

				collided = true;
				collision_x = true;
				break;
			}
			if (world_vel.x > 0.0f && pTileMap->GetTile(x_max, y)->Solid())
			{
				reform_x = x_max - w * 0.5f;

				collided = true;
				collision_x = true;
				break;
			}
		}

		if (collision_x)
		{
			y_min = y_min + 1;
			y_max = CeilToInt(y_min + h) - 1;

			bool flag = false;
			for (int x = x_min; x <= x_max; ++x)
			{
				for (int y = y_min; y <= y_max; ++y)
					flag |= pTileMap->GetTile(x, y)->Solid();
			}
			if (flag)
			{
				post_pos.x = reform_x;
				post_vel.x = 0.0f;
			}
			else if (post_vel.y >= 0.0f)
				post_pos.y = y_min + h * 0.5f;
		}
	}

	Protocol::s2c_MOVE pkt;
	*pkt.mutable_obj_pos() = ToProtoVec2(TRWorld::WorldToGlobal(post_pos));
	*pkt.mutable_vel() = ToProtoVec2(post_vel);
	pkt.set_ground(landed);
	pkt.set_state(pkt_.state());
	//if (!landed)
	//{
	//	pkt.set_ground(false);
	//}

	return pkt;
}
