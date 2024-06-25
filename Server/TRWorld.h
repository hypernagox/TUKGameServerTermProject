#pragma once

#include "TRTile.h"
#include "TRTileMap.h"
#include "TRWorldGeneration.h"
#include "Protocol.pb.h"

//#include "CPlayer.h"
//#include "CScene.h"
//
//#include "TRItemContainer.h"
//#include "TRItemStack.h"

class TRItemContainer;
class CQuickBarVisualizer;
class CHealthIndicator;
class CInventoryVisualizer;

struct TileInfo
{
	uint16_t x, y;
	std::string tile_name;
	TileInfo(const int x_, const int y_) :x{ (uint16_t)(x_) }, y{ (uint16_t)(y_) } {}
	TileInfo(const int x_, const int y_, std::string_view str) :x{ (uint16_t)(x_) }, y{ (uint16_t)(y_) }, tile_name{ str }
	{}
};

struct alignas(64) CacheLineWrapper
{
	ServerCore::Vector<TileInfo> tile_record;
};

struct LevelNode
{
	int level;
	ServerCore::Vector<Vec2Int> pos;
};

class TRWorld
	: public ServerCore::RefCountable
{
public:
	static constexpr int WORLD_WIDTH = 512;
	static constexpr int WORLD_HEIGHT = 256;

	LevelNode level_nodes[WORLD_WIDTH];
	std::vector<Vec2Int> nodes;
private:
	TRTileMap* tile_map;
	std::bitset<WORLD_WIDTH> m_bitSolid[WORLD_HEIGHT] = {};

	ServerCore::HashMap<Vec2Int, TileInfo> m_mapCreateRecords;
	ServerCore::HashSet<Vec2Int> m_setEraseRecords;

	//CPlayer* player;
	//CScene* m_pScene;

	TRItemContainer* player_inventory[50];
	TRItemContainer* player_armor[3];
	TRItemContainer* quick_bar[10];

	CQuickBarVisualizer* quick_bar_visualizer;
	CInventoryVisualizer* inventory_visualizer;

	CHealthIndicator* health_indicator;
	int quick_bar_index;
	bool toggle_inventory;
	ServerCore::SRWLock m_tileWorldLock;
	ServerCore::SRWLock m_tileWallWorldLock;
public:
	TRWorld(int seed=0);
	~TRWorld();

	void Update();
	void InitMonsters(const CHUNK eChunk);

	void CreateWorld(int seed);
	void OnSceneCreate();

	static constexpr Vec2 WorldToGlobal(const Vec2& v)noexcept
	{
		return Vec2(v.x * PIXELS_PER_TILE, (static_cast<float>(TRWorld::WORLD_HEIGHT) - v.y) * PIXELS_PER_TILE);
	}

	static constexpr Vec2 GlobalToWorld(const Vec2& v)noexcept
	{
		return Vec2(v.x / PIXELS_PER_TILE, static_cast<float>(TRWorld::WORLD_HEIGHT) - v.y / PIXELS_PER_TILE);
	}

	TRTileMap* const GetTileMap()noexcept { return tile_map; }
	const TRTileMap* const GetTileMap()const noexcept { return tile_map; }
	const bool GetTileSolid(const int x_, const int y_)const noexcept { return m_bitSolid[y_][x_]; }
	//CPlayer* GetPlayer() const;

	bool PlaceTile(int x, int y, TRTile* new_tile,std::string_view tile_key,std::wstring& name);
	bool BreakTile(int x, int y,std::string& outName);
	bool PlaceTileWall(int x, int y, TRTileWall* new_tile);
	bool BreakTileWall(int x, int y);

	//void DropItem(Vec2 world_pos, TRItemStack item);

	void SetToggleInventory(bool value);
	//void AddItemToInventory(TRItemStack item);
	void SwitchQuickBarIndex(int value);

	auto& GetQuickBarList() { return quick_bar; }
	int GetQuickBarIdx()const { return quick_bar_index; }
	int GetArmorPoint() const;

	void FloatDamageText(int value, Vec2 vPos, COLORREF color);
	void SpawnBoss();

	void TransmitTileRecord(const ServerCore::S_ptr<ServerCore::Session>& pSession_)const noexcept;
public:
	Protocol::s2c_MOVE updateTileCollision(const Protocol::c2s_MOVE& pkt_)const noexcept;
};