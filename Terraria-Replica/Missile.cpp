#include "pch.h"
#include "Missile.h"
#include "CSceneMgr.h"
#include "CScene.h"
#include "CRigidBody.h"
#include "CResMgr.h"
#include "CTimeMgr.h"
#include "s2c_PacketHandler.h"
#include "CCore.h"
#include "CCamera.h"

Missile::Missile(const Vec2 vPos_)
{
	//SetPos(vPos_ + Vec2{ 8.f,0.f });
	SetPos(vPos_);
	//Mgr(CSceneMgr)->GetScene(SCENE_TYPE::START)->AddObject(this, GROUP_TYPE::PROJ_PLAYER);
	m_pMissleImg = Mgr(CResMgr)->GetImg(L"Wooden_Arrow.png");
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

void Missile::update()
{
	CObject::update();
	m_acc -= DT;
	if (0.f >= m_acc)
	{
		DeleteObj(this);
		return;
	}
	//if (!m_bFirst)
	{
		SetPos(GetPos() + Vec2{ m_speed,0.f }*DT * m_dir);
		//m_interpolator.GetNewData().pos = GetPos();
	}
	//else
	//{
	//	const auto move_data = m_interpolator.GetInterPolatedData();
	//	SetPos(move_data.pos);
	//}
}

void Missile::component_update()
{
	//CObject::component_update();
	
}

void Missile::render(HDC dc_) const
{
	//if (!m_bFirst)
	//	return;
	const auto [vLT, vScale] = Mgr(CCamera)->GetRenderPos(this);
	Mgr(CCore)->RotateTransform(dc_, 90.f * -m_dir, vLT + vScale / 2.f);
	Mgr(CResMgr)->renderImg(dc_, m_pMissleImg, this, {}, Vec2((float)m_pMissleImg->GetWidth(), (float)m_pMissleImg->GetHeight()));
	Mgr(CCore)->ResetTransform(dc_);
}

