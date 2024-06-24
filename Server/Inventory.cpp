#include "pch.h"
#include "Inventory.h"
#include "ObjectFactory.h"
#include "DBMgr.h"
#include "DBPacket.h"
#include "Object.h"

const S_ptr<Item>& Inventory::AddItem(const std::string_view itemName, S_ptr<Item> pItem, const bool dbCall) noexcept
{
	pItem->SetItemName(itemName);
	if (dbCall)
	{
		s2q_ADD_OR_UPDATE_ITEM pkt;
		strcpy_s(pkt.itemName, itemName.data());
		pkt.user_id = GetOwner()->GetObjID();
		pkt.quantity = -1;
		RequestQueryServer(pkt);
	}
	return m_mapItems.try_emplace(itemName.data(), std::move(pItem)).first->second;
}

void Inventory::IncItem(const std::string_view itemName, const bool dbCall) noexcept
{
	const auto iter = m_mapItems.find(itemName.data());
	if (m_mapItems.end() == iter)
	{
		ItemBuilder b;
		b.itemName = itemName;
		b.owner = GetOwner();
		AddItem(itemName, ItemFactory::CreateCountableItem(b),false);
		//std::cout << itemName << "»ý±è" << std::endl;
	}
	else
	{
		static_cast<CountableItem* const>(iter->second.get())->IncCount();
		//std::cout << itemName <<"°¹¼ö¿Ã¸²" << std::endl;
	}
	if (!dbCall)return;
	s2q_ADD_OR_UPDATE_ITEM pkt;
	strcpy_s(pkt.itemName, itemName.data());
	pkt.user_id = GetOwner()->GetObjID();
	RequestQueryServer(pkt);
}
