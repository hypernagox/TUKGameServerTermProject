#pragma once

#include <string>
#include <Windows.h>
#include "CAtlasElement.h"

#include "TRWorld.h"
#include "CPlayer.h"

class TRItem
{
protected:
	std::wstring name;
	std::wstring k_element;
	std::wstring m_KeyName;

	int max_stacksize;

	CImage* element;

public:
	TRItem(std::wstring name, std::wstring k_element);
	virtual ~TRItem();

	void CreateAtlasElements();
	const std::wstring& GetKeyName()const noexcept { return m_KeyName; }
	const std::wstring& GetName() const noexcept { return name; }
	int GetMaxStacksize() const;
	CImage* GetItemElement() const;
	virtual bool OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos);
	const std::wstring_view GetElementName()const noexcept { return k_element; }
};

class TRItemTile : public TRItem
{
protected:
	std::wstring k_tile;

public:
	TRItemTile(std::wstring name, std::wstring k_element, std::wstring k_tile);
	~TRItemTile();

	virtual bool OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos) override;
};

class TRItemTileWall : public TRItem
{
protected:
	std::wstring k_tilewall;

public:
	TRItemTileWall(std::wstring name, std::wstring k_element, std::wstring k_tilewall);
	~TRItemTileWall();

	virtual bool OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos) override;
};

class TRItemTool : public TRItem
{
public:
	TRItemTool(std::wstring name, std::wstring k_element);
	~TRItemTool();
};

class TRItemPickaxe : public TRItemTool
{
public:
	TRItemPickaxe(std::wstring name, std::wstring k_element);
	~TRItemPickaxe();

	virtual bool OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos) override;
};

class TRItemHammer : public TRItemTool
{
public:
	TRItemHammer(std::wstring name, std::wstring k_element);
	~TRItemHammer();

	virtual bool OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos) override;
};

class TRItemSword : public TRItemTool
{
public:
	TRItemSword(std::wstring name, std::wstring k_element);
	~TRItemSword();

	virtual bool OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos) override;
};

class TRItemSummonBoss : public TRItem
{
public:
	TRItemSummonBoss(std::wstring name, std::wstring k_element);
	~TRItemSummonBoss();

	virtual bool OnUseItem(CPlayer* user, TRWorld* world, const Vec2& target_pos) override;
};

class TRItemArmor : public TRItem
{
private:
	int armor_part;
	int armor_point;

public:
	TRItemArmor(std::wstring name, std::wstring k_element, int armor_part, int armor_point);
	~TRItemArmor();

	int GetArmorPart() const;
	int GetArmorPoint() const;
};