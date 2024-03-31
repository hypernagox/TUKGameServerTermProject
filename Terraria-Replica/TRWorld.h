#pragma once

#include "TRTile.h"
#include "TRTileMap.h"
#include "TRWorldGeneration.h"
#include "Hero.h"
#include "CScene.h"

#include "TRItemContainer.h"
#include "TRItemStack.h"


class TRItemContainer;
class CQuickBarVisualizer;
class CHealthIndicator;
class CInventoryVisualizer;
class CDropItem;

class TRWorld
{
public:
	static constexpr int WORLD_WIDTH = 512;
	static constexpr int WORLD_HEIGHT = 256;

private:
	TRTileMap* tile_map;
	Hero* player;
	CScene* m_pScene;

	std::array<TRItemContainer*,50> player_inventory;

	TRItemContainer* player_armor[3];

	std::array<TRItemContainer*,10> quick_bar;

	CQuickBarVisualizer* quick_bar_visualizer;
	CInventoryVisualizer* inventory_visualizer;

	CHealthIndicator* health_indicator;
	int quick_bar_index;
	bool toggle_inventory;

	std::unordered_map<uint64_t, CPlayer*> m_mapOtherPlayer[etoi(SECTOR::END)];

	std::unordered_map<uint64_t, CDropItem*> m_mapItem;
public:
	void FindAndModifyItemStack(std::string_view itemName, const int mount_,const bool bIsWall = false)noexcept;
public:
	TRWorld();
	~TRWorld();

	void Update();

	
	void CreateWorld(int seed);
	void OnSceneCreate(CScene* scene);

	static Vec2 WorldToGlobal(const Vec2& v);
	static Vec2 GlobalToWorld(const Vec2& v);

	TRTileMap* GetTileMap() const;
	CPlayer* GetPlayer() const;

	bool PlaceTile(int x, int y, TRTile* new_tile);
	void BreakTile(int x, int y);
	bool PlaceTileWall(int x, int y, TRTileWall* new_tile);
	void BreakTileWall(int x, int y);

	void DropItem(Vec2 world_pos, TRItemStack item);

	void SetToggleInventory(bool value);
	void AddItemToInventory(TRItemStack item);
	void SwitchQuickBarIndex(int value);

	auto& GetQuickBarList() { return quick_bar; }
	int GetQuickBarIdx()const { return quick_bar_index; }
	int GetArmorPoint() const;

	void FloatDamageText(int value, Vec2 vPos, COLORREF color);
	void SpawnBoss();

	void CreateItem(const uint64_t item_id,Vec2 world_pos, std::string_view item_key);
	CDropItem* const GetDropItem(const uint64_t item_id)const {
		const auto iter = m_mapItem.find(item_id);
		return m_mapItem.end() != iter ? iter->second : nullptr;
	}
	
	void EraseOtherPlayer(const uint64_t otherPlayerId,const uint64_t sector)noexcept;
	void EraseItem(const uint64_t item_id)noexcept;
public:
	void AddNewPlayer(const uint64_t id, const uint64_t sector);
	CPlayer* GetOtherPlayer(const uint64_t id,const uint64 sector_)noexcept { return m_mapOtherPlayer[sector_][id]; }
};