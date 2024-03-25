#include "pch.h"
#include "CollisionHandler.h"
#include "Object.h"
#include "c2s_PacketHandler.h"
#include "ClientSession.h"

void PlayerCollisionHandler::OnCollisionEnter(Object* const a, Object* const b)
{
	if (b->GetObjectGroup() == GROUP_TYPE::DROP_ITEM)
	{
		if (const auto pSession = a->GetComp("SESSIONOBJECT")->Cast<SessionObject>()->GetSession())
		{
			Protocol::s2c_GET_ITEM pkt;
			pkt.set_obj_id(b->GetObjID());
			pkt.set_item_name(b->GetObjectName());

			pSession << pkt;
		}
	}
}

void PlayerCollisionHandler::OnCollisionStay(Object* const a, Object* const b)
{
}

void PlayerCollisionHandler::OnCollisionExit(Object* const a, Object* const b)
{
}
