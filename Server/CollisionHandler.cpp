#include "pch.h"
#include "CollisionHandler.h"
#include "Object.h"
#include "c2s_PacketHandler.h"
#include "ClientSession.h"
#include "TRWorldMgr.h"
#include "TRWorldRoom.h"

void PlayerCollisionHandler::OnCollisionEnter(Object* const a, Object* const b)
{
	if (a->GetObjectGroup() == GROUP_TYPE::PROJ_PLAYER || b->GetObjectGroup() == GROUP_TYPE::PROJ_PLAYER)
	{
		//if (const auto pSession = a->GetComp("SESSIONOBJECT")->Cast<SessionObject>()->GetSession())
		//{
		//	Protocol::s2c_GET_ITEM pkt;
		//	pkt.set_obj_id(b->GetObjID());
		//	pkt.set_item_name(b->GetObjectName());
		//
		//	pSession << pkt;
		//}
		Protocol::s2c_GET_ITEM pkt;
		pkt.set_obj_id(a->GetObjID());
		pkt.set_item_name(b->GetObjectName());
		pkt.set_sector(0);
		pkt.set_item_id(b->GetObjID());
		TRMgr(TRWorldMgr)->GetWorldRoom(SECTOR::SECTOR_0) << pkt;
		std::cout << "!!" << std::endl;
	}
	
}

void PlayerCollisionHandler::OnCollisionStay(Object* const a, Object* const b)
{
}

void PlayerCollisionHandler::OnCollisionExit(Object* const a, Object* const b)
{
}
