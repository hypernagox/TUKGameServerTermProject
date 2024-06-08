#pragma once
#include "ItemComponent.h"

class Inventory
	:public BaseComponent
{
public:
	Inventory(Object* const pOwner_)noexcept
		:BaseComponent{ "INVENTORY",pOwner_} {}
	const std::string_view GetCurSelectItemName()const noexcept { return m_strCurSelectItem; }
	Item* GetCurItem()noexcept {
		const auto iter = m_mapItems.find(m_strCurSelectItem);
		return m_mapItems.cend() != iter ? iter->second.get() : nullptr;
	}
	Item* AddItem(const std::string_view itemName, Item* const pItem)noexcept {
		m_mapItems.try_emplace(itemName.data(), pItem);
		return pItem;
	}
	void SetCurItem(const std::string_view itemName)noexcept {
		m_strCurSelectItem = itemName;
	}
private:
	String m_strCurSelectItem;
	ServerCore::HashMap<String, U_ptr<Item>> m_mapItems;
};

