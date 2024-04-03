#include "pch.h"
#include "TRItem.h"
#include "CResMgr.h"
#include "TRTileManager.h"
#include "CustomMath.hpp"
#include "CSoundMgr.h"
#include "Protocol.pb.h"
#include "s2c_PacketHandler.h"
#include "CPlayer.h"
#include "CAnimator.h"

TRItem::TRItem(std::wstring name, std::wstring k_element)
{
	this->name = name;
	this->k_element = k_element;
	this->max_stacksize = 99;

	if (k_element != L"")
		CreateAtlasElements();
}

TRItem::~TRItem()
{
}

void TRItem::CreateAtlasElements()
{
	element = Mgr(CResMgr)->GetImg(k_element);
}

int TRItem::GetMaxStacksize() const
{
	return max_stacksize;
}

CImage* TRItem::GetItemElement() const
{
	return element;
}

bool TRItem::OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos)
{
	return false;
}

TRItemTile::TRItemTile(std::wstring name, std::wstring k_element, std::wstring k_tile) : TRItem(name, k_element)
{
	m_KeyName = k_tile;
	this->k_tile = k_tile;
}

TRItemTile::~TRItemTile()
{

}

bool TRItemTile::OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos)
{
	const Vec2 vPos = TRWorld::GlobalToWorld(user->GetPos());
	if ((target_pos - vPos).length() > 10.0f)
		return false;

	const int x = FloorToInt(target_pos.x);
	const int y = FloorToInt(target_pos.y);
	//bool result = world->PlaceTile(x, y, Mgr(TRTileManager)->GetTileByKey(k_tile));

	Protocol::c2s_PLACE_TILE pkt;
	pkt.set_tile_x(x);
	pkt.set_tile_y(y);
	pkt.set_tile_key(WideToUtf8(k_tile));

	Send(pkt);

	return false;
}

TRItemTileWall::TRItemTileWall(std::wstring name, std::wstring k_element, std::wstring k_tilewall) : TRItem(name, k_element)
{
	m_KeyName = k_tilewall;
	this->k_tilewall = k_tilewall;
}

TRItemTileWall::~TRItemTileWall()
{

}

bool TRItemTileWall::OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos)
{
	const Vec2 vPos = TRWorld::GlobalToWorld(user->GetPos());
	if ((target_pos - vPos).length() > 10.0f)
		return false;

	const int x = FloorToInt(target_pos.x);
	const int y = FloorToInt(target_pos.y);
	//bool result = world->PlaceTileWall(x, y, Mgr(TRTileManager)->GetTileWallByKey(k_tilewall));

	Protocol::c2s_PLACE_TILE_WALL pkt;
	pkt.set_tile_x(x);
	pkt.set_tile_y(y);
	pkt.set_tile_key(WideToUtf8(k_tilewall));

	Send(pkt);


	return false;
}

TRItemTool::TRItemTool(std::wstring name, std::wstring k_element) : TRItem(name, k_element)
{
	this->max_stacksize = 1;
}

TRItemTool::~TRItemTool()
{
}

TRItemPickaxe::TRItemPickaxe(std::wstring name, std::wstring k_element) : TRItemTool(name, k_element)
{
}

TRItemPickaxe::~TRItemPickaxe()
{
}

bool TRItemPickaxe::OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos)
{
	const Vec2 vPos = TRWorld::GlobalToWorld(user->GetPos());
	if ((target_pos - vPos).length() > 10.0f)
		return false;

	const int x = FloorToInt(target_pos.x);
	const int y = FloorToInt(target_pos.y);
	//world->BreakTile(x, y);
	Protocol::c2s_BREAK_TILE pkt;
	pkt.set_tile_x(x);
	pkt.set_tile_y(y);
	Send(pkt);

	return false;
}

TRItemHammer::TRItemHammer(std::wstring name, std::wstring k_element) : TRItemTool(name, k_element)
{
}

TRItemHammer::~TRItemHammer()
{
}

bool TRItemHammer::OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos)
{
	const Vec2 vPos = TRWorld::GlobalToWorld(user->GetPos());
	if ((target_pos - vPos).length() > 10.0f)
		return false;

	const int x = FloorToInt(target_pos.x);
	const int y = FloorToInt(target_pos.y);
	//world->BreakTileWall(x, y);

	Protocol::c2s_BREAK_TILE_WALL pkt;
	pkt.set_tile_x(x);
	pkt.set_tile_y(y);
	Send(pkt);

	return false;
}

TRItemSword::TRItemSword(std::wstring name, std::wstring k_element) : TRItemTool(name, k_element)
{
}

TRItemSword::~TRItemSword()
{
}

bool TRItemSword::OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos)
{
	// 무기의 좌표 ( 유저 코앞 )
	// 무기의 크기
	const int dir = user->GetComp<CAnimator>()->GetAnimDir() == 0 ? 1 : -1;

	const Vec2 wepon_pos = user->GetPos() + Vec2{ 8.f,0.f } *(float)dir;

	Protocol::c2s_TRY_GET_ITEM pkt;
	pkt.set_time_stamp(NetHelper::GetTimeStampMilliseconds());
	*pkt.mutable_obj_pos() = ::ToProtoVec2(Vec2{ user->GetScale().x,0.f } * (float)dir);

	Send(pkt);

	return false;
}

TRItemSummonBoss::TRItemSummonBoss(std::wstring name, std::wstring k_element) : TRItem(name, k_element)
{
	this->max_stacksize = 1;
}

TRItemSummonBoss::~TRItemSummonBoss()
{
}

bool TRItemSummonBoss::OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos)
{
	Mgr(CSoundMgr)->PlayEffect("Roar_0.wav", 0.5f);
	world->SpawnBoss();
	return true;
}

TRItemArmor::TRItemArmor(std::wstring name, std::wstring k_element, int armor_part, int armor_point) : TRItem(name, k_element)
{
	this->armor_part = armor_part;
	this->armor_point = armor_point;
	this->max_stacksize = 1;
}

TRItemArmor::~TRItemArmor()
{
}

int TRItemArmor::GetArmorPart() const
{
	return armor_part;
}

int TRItemArmor::GetArmorPoint() const
{
	return armor_point;
}
