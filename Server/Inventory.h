#pragma once
#include "ItemComponent.h"

class Inventory
	:public BaseComponent
{
public:
	CONSTRUCTOR_BASE_COMPONENT(Inventory)
	const std::string_view GetCurSelectItemName()const noexcept { return m_strCurSelectItem; }
	const S_ptr<Item>& GetCurItem()noexcept {
		const auto iter = m_mapItems.find(m_strCurSelectItem);
		return m_mapItems.cend() != iter ? iter->second : g_sentinel;
	}

	const S_ptr<Item>& AddItem(const std::string_view itemName, S_ptr<Item> pItem,const bool dbCall=true)noexcept;

	void SetCurItem(const std::string_view itemName)noexcept {
		m_strCurSelectItem = itemName;
	}
	void RemoveItem(const std::string_view strName_)noexcept {
		if (strName_ == m_strCurSelectItem)
			m_strCurSelectItem.clear();
		m_mapItems.erase(strName_.data());
	}
	void IncItem(const std::string_view itemName, const bool dbCall=true)noexcept;

	const S_ptr<Item>& GetItem(const std::string_view itemName)noexcept {
		const auto iter = m_mapItems.find(itemName.data());
		return m_mapItems.cend() != iter ? iter->second : g_sentinel;
	}
private:
	String m_strCurSelectItem;
	ServerCore::HashMap<String, S_ptr<Item>> m_mapItems;

	static inline const S_ptr<Item> g_sentinel = nullptr;
};

