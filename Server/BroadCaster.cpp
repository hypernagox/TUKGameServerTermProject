#include "pch.h"
#include "BroadCaster.h"
#include "Object.h"
#include "SessionManageable.h"
#include "c2s_PacketHandler.h"
#include "PhysicsComponent.h"

void MoveBroadCaster::PostUpdate(const float) noexcept
{
	const auto obj = GetOwner();
	const auto pRigid = obj->GetComp("RIGIDBODY")->Cast<RigidBody>();
	Protocol::s2c_MOVE pkt;

	*pkt.mutable_obj_pos() = ::ToProtoVec2(obj->GetPos());
	*pkt.mutable_wiil_pos() = ToProtoVec2(obj->GetWillPos());
	*pkt.mutable_vel() = ::ToProtoVec2(pRigid->GetVelocity());
	pkt.set_time_stamp(ServerCore::GetTimeStampMilliseconds());

	m_pCurRoom << pkt;
}
