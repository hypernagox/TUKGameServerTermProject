#include "pch.h"
#include "TRWorldRoom.h"
#include "Object.h"

TRWorldRoom::TRWorldRoom(const SECTOR sector_)
	:SessionManageable{ static_cast<uint16>(sector_) }
{
}

TRWorldRoom::~TRWorldRoom()
{
}

void TRWorldRoom::Update(const uint64 tick_ms)
{
	m_timer.Update();

	for (const auto& obj_list : m_worldObjectList)
	{
		for (const auto obj : obj_list.GetItemListRef())
			obj->Update(m_timer.GetDT());
	}
	
	EnqueueAsyncTimer(tick_ms, &TRWorldRoom::Update, uint64{ tick_ms });
}

void TRWorldRoom::Init()
{

}

void TRWorldRoom::AddObjectEnqueue(const GROUP_TYPE eType_, S_ptr<Object> pObj_)
{
	EnqueueAsync(&TRWorldRoom::AddObject, GROUP_TYPE{ eType_ }, std::move(pObj_));
}

void TRWorldRoom::DeleteObjectEnqueue(const GROUP_TYPE eType_, const uint64 objID_)
{
	EnqueueAsync(&TRWorldRoom::DeleteObject, GROUP_TYPE{ eType_ }, uint64{ objID_ });
}

void TRWorldRoom::AddObject(const GROUP_TYPE eType_, S_ptr<Object> pObj_)
{
	m_worldObjectList[etoi(eType_)].AddItem(pObj_->GetObjID(), std::move(pObj_));
}

void TRWorldRoom::DeleteObject(const GROUP_TYPE eType_, const uint64 objID_)
{
	m_worldObjectList[etoi(eType_)].EraseItem(objID_);
}
