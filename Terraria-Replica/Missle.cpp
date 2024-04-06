#include "pch.h"
#include "Missle.h"
#include "CSceneMgr.h"
#include "CScene.h"
#include "CRigidBody.h"
#include "CResMgr.h"
#include "CTimeMgr.h"
#include "s2c_PacketHandler.h"

uint64 time_stamp = 0;

Missle::Missle(const Vec2 vPos_)
{
	//SetPos(vPos_ + Vec2{ 8.f,0.f });
	SetPos(vPos_);
	//Mgr(CSceneMgr)->GetScene(SCENE_TYPE::START)->AddObject(this, GROUP_TYPE::PROJ_PLAYER);
	m_pMissleImg = Mgr(CResMgr)->GetImg(L"Item_Torch.png");
	SetScale(Vec2{ 32.f,32.f });
	
	//CreateComponent(COMPONENT_TYPE::RIGIDBODY);
	
	//GetComp<CRigidBody>()->SetGravity(false);
	MoveData data;
	data.pos = GetPos();
	//m_interpolator.GetCurData().pos = GetPos();
	m_interpolator.GetNewData().pos = GetPos();
	m_interpolator.UpdateNewData(data, NetHelper::GetTimeStampMilliseconds());
	//m_interpolator.UpdateOnlyTimeStamp(time_stamp);
}

void Missle::update()
{
	CObject::update();
	
	//if (!m_bFirst)
	{
		SetPos(GetPos() + Vec2{ 1000.f,0.f }*DT);
		//m_interpolator.GetNewData().pos = GetPos();
	}
	//else
	//{
	//	const auto move_data = m_interpolator.GetInterPolatedData();
	//	SetPos(move_data.pos);
	//}
}

void Missle::component_update()
{
	//CObject::component_update();
	
}

void Missle::render(HDC dc_) const
{
	//if (!m_bFirst)
	//	return;
	Mgr(CResMgr)->renderImg(dc_, m_pMissleImg, this, {}, Vec2((float)m_pMissleImg->GetWidth(), (float)m_pMissleImg->GetHeight()));
}

void Missle::SetMoveData(Protocol::s2c_MOVE movePkt_)
{
	m_bFirst = true;
	MoveData moveData
	{
		.pos = (ToOriginVec2(movePkt_.obj_pos())),
		.will_pos = (ToOriginVec2(movePkt_.wiil_pos())),
		.vel = (ToOriginVec2(movePkt_.vel()))
	};

	
	//if (false == std::exchange(m_bFirst, true))
	//{
	//	moveData.pos = GetPos();
	//	SetPos((moveData.pos));
	//	m_interpolator.GetNewData().pos = GetPos();
	//}
	//else
	{
		//SetPos((moveData.pos));
	}
	
	m_interpolator.UpdateNewData(moveData, movePkt_.time_stamp());
}
