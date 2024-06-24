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
class ServerObject;


class TRWorld
{
public:
	static constexpr int WORLD_WIDTH = 512;
	static constexpr int WORLD_HEIGHT = 256;

private:
	TRTileMap* tile_map;
	static inline Hero* player;
	static inline std::array<TRItemContainer*,50> player_inventory;
	static inline TRItemContainer* player_armor[3];
	static inline std::array<TRItemContainer*,10> quick_bar;
	static inline CQuickBarVisualizer* quick_bar_visualizer;
	static inline CInventoryVisualizer* inventory_visualizer;
	static inline CHealthIndicator* health_indicator;
	static inline int quick_bar_index;
	static inline bool toggle_inventory;
public:
	static inline std::unordered_map<uint64_t, CPlayer*> m_mapOtherPlayer;
	static inline std::unordered_map<uint64_t, ServerObject*> m_mapServerObject;
	CScene* m_pScene;
	static class Status* g_stat;
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

	void FloatText(const wstring_view str, Vec2 vPos, COLORREF color);

	void SpawnBoss();

	void CreateItem(const uint64_t item_id,Vec2 world_pos, std::string_view item_key);
	void CreateItem(const uint64_t item_id, Vec2 world_pos, std::string_view item_key,const int sector_);
	
	void EraseOtherPlayer(const uint64_t otherPlayerId,const uint64_t sector)noexcept;
	void EraseItem(const uint64_t item_id)noexcept;

	void CreateMonster(const uint64_t mon_id, Vec2 world_pos, std::string_view mon_name,const int sector_);
public:
	CPlayer* const AddNewPlayer(const uint64_t id, const uint64_t sector,const Vec2 vPos_,const std::wstring_view name);
	CPlayer* GetOtherPlayer(const uint64_t id,const uint64 sector_)noexcept { 
		const auto iter = m_mapOtherPlayer.find(id);
		return m_mapOtherPlayer.end() != iter ? iter->second : nullptr;
	}
	void CreateMissle(const uint64_t id_, const Vec2 vPos_,const int dir,const float speed);


	ServerObject* GetServerObject(const uint64_t id_)const noexcept {
		const auto iter = m_mapServerObject.find(id_);
		return m_mapServerObject.end() != iter ? iter->second : nullptr;
	}
	
};

class Status
{
public:
	wstring GetLevelText()const noexcept {
		return std::format(L"LEVEL: {}", m_level);
	}
	wstring GetExpText()const noexcept {
		return std::format(L"EXP: {} / {}", m_exp, m_maxExp);
	}
	wstring GetGoldText()const noexcept {
		return std::format(L"GOLD: {}", m_gold);
	}
public:

	void AddGold(const int add_gold);
	void SubGold(const int sub_gold);
	void AddExp(const int add_exp);
	void HalfExp();

	void LevelUp(const int exp);
	void GetGold() { AddGold(1); }

	void ModifyHP(const int val);

	uint64_t curID = 0;

	
public:
	int m_level = 1;
	int m_exp = 0;
	int m_maxExp = 10;
	int m_gold = 10;
};