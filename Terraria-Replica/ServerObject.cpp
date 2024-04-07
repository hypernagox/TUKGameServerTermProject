#include "pch.h"
#include "ServerObject.h"
#include "CPlayer.h"
#include "CKeyMgr.h"
#include "CTimeMgr.h"
#include "CCore.h"
#include "CResMgr.h"
#include "CTexture.h"
#include "CAnimator.h"
#include "CCamera.h"
#include "CRigidBody.h"
#include "CAnimation.h"
#include "TRWorld.h"
#include "CustomMath.hpp"
#include "Vec2Int.hpp"
#include "CWeapon.h"
#include "SimpleMath.hpp"
#include "CCollider.h"
#include "CSceneMgr.h"
#include "CScene.h"
#include "CCamera.h"
#include "CEventMgr.h"
#include "CDropItem.h"
#include "TRItem.h"
#include "CSoundMgr.h"
#include "Protocol.pb.h"
#include "s2c_PacketHandler.h"
#include "Enum.pb.h"

ServerObject::ServerObject()
{
}

ServerObject::~ServerObject()
{
}

void ServerObject::SetNewMoveData(const Protocol::s2c_MOVE& movePkt_) noexcept
{
	const Vec2 vFutureVel = ToOriginVec2(movePkt_.vel()) + ToOriginVec2(movePkt_.accel()) * DT;

	const Vec2 vFuturePos = ToOriginVec2(movePkt_.obj_pos()) + ToOriginVec2(movePkt_.vel()) * DT + ToOriginVec2(movePkt_.accel()) * DT * DT * 0.5f;

	const MoveData moveData
	{
		.pos = vFuturePos,
		.will_pos = ToOriginVec2(movePkt_.wiil_pos()),
		.vel = vFutureVel
	};

	m_interpolator.UpdateNewData(moveData, movePkt_.time_stamp());

	if (const auto pRigid = GetComp<CRigidBody>())
	{
		pRigid->SetIsGround(movePkt_.ground());
		pRigid->SetVelocity(moveData.vel);
	}
}

Protocol::c2s_MOVE ServerObject::MakeSendData() const noexcept
{
	Protocol::c2s_MOVE pkt;
	*pkt.mutable_obj_pos() = ToProtoVec2(GetPos());
	*pkt.mutable_wiil_pos() = ToProtoVec2(GetWillPos());
	*pkt.mutable_scale() = ToProtoVec2(GetComp<CCollider>()->GetScale());
	*pkt.mutable_vel() = ToProtoVec2(GetComp<CRigidBody>()->GetVelocity());
	pkt.set_ground(GetComp<CRigidBody>()->IsGround());
	*pkt.mutable_accel() = ToProtoVec2(GetComp<CRigidBody>()->GetAccel());

	return pkt;
}

void ServerObject::UpdateMoveData() noexcept
{
	const auto moveData = m_interpolator.GetInterPolatedData();
	SetPos(moveData.pos);
}
