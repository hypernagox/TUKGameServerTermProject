#include "pch.h"
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

extern void updateTileCollision(CObject* const _pObj, TRWorld* const _pTRWorld);

CPlayer::CPlayer(TRWorld* const _trWorld)
{
	m_pTRWolrd = _trWorld;
	m_bIsCamAffected = true;
	CreateComponent(COMPONENT_TYPE::COLLIDER, Vec2{ 24.0f, 48.0f });
	CreateComponent(COMPONENT_TYPE::ANIMATOR);
	CreateComponent(COMPONENT_TYPE::RIGIDBODY);
	SetName(L"Player_");
	auto pAnim = GetComp<CAnimator>();
	m_pAnimLeg = make_unique<CAnimator>();
	m_pAnimLeg->SetOwner(this);
	pAnim->CreateAnimation(L"Player_Torso_WALK", L"Player_Torso.png", Vec2{ 20 * 2, 28 }, Vec2{ 20,28 }, Vec2{ 20,0 }, 0.04f, 13);
	pAnim->CreateAnimation(L"Player_Torso_IDLE", L"Player_Torso.png", Vec2{ 0,0 }, Vec2{ 20,28 }, Vec2{ 20,0 }, 0.2f, 1);
	pAnim->CreateAnimation(L"Player_Torso_JUMP", L"Player_Torso.png", Vec2{ 0,28 }, Vec2{ 20,28 }, Vec2{ 20,0 }, 0.5f, 1);
	pAnim->CreateAnimation(L"Player_Torso_ATTACK", L"Player_Torso.png", Vec2{ 0,0 }, Vec2{ 20,28 }, Vec2{ 20,0 }, 0.05f, 5);
	m_pAnimLeg->CreateAnimation(L"Player_Leg_WALK", L"Player_Leg.png", Vec2{ 0,28 * 7 }, Vec2{ 20,28 }, Vec2{ 0,28 },0.04f, 13);
	m_pAnimLeg->CreateAnimation(L"Player_Leg_IDLE", L"Player_Leg.png", Vec2{ 0,0}, Vec2{ 20,28 }, Vec2{ 0,28 }, 0.1f, 4);
	m_pAnimLeg->CreateAnimation(L"Player_Leg_JUMP", L"Player_Leg.png", Vec2{ 0,28 * 5 }, Vec2{ 20,28 }, Vec2{ 0,28 }, 0.5f, 1);
	pAnim->Play(L"Player_Torso_IDLE",true);
	m_pAnimLeg->Play(L"Player_Leg_IDLE", true);
	m_pPrevAnim = pAnim->GetCurAnim();

	//auto pRigid = GetComp<CRigidBody>();
	//pRigid->SetIsGround(false);
	
	for (int i = 0; i < 10; ++i)
	{
		m_vecWeapon.emplace_back(new CWeapon{ _trWorld, this });
	}

	CreateDCBITMAP(m_hPlayerVeilDC, m_hPlayerVeilBit, Vec2{ 40.f, 56.f });
	PatBlt(m_hPlayerVeilDC, 0, 0, 40, 56, BLACKNESS);
}

CPlayer::CPlayer(const CPlayer& other)
	:CObject{other}
{
}

void CPlayer::update()
{
	if (m_bSlain)
	{
		return;
	}
	CObject::update();
	const auto pRigid = GetComp<CRigidBody>();
	//if (bitwise_absf(pRigid->GetVelocity().y) > FLT_EPSILON && PLAYER_STATE::ATTACK != m_eCurState)
	//{
	//	m_eCurState = PLAYER_STATE::JUMP;
	//	m_bIsIDLE = false;
	//}

	const auto pAnim = GetComp<CAnimator>();
	//
	//m_ePrevState = m_eCurState;
	//
	//updateDmgCoolDown();
	//
	//updateQuickBarState(m_pTRWolrd->GetQuickBarIdx());
	//
	//updateMove();
	//
	//updateState();
	
	const auto moveData = m_interpolator.GetInterPolatedData();
	//SetWillPos(moveData.will_pos);
	
	SetPos(moveData.pos);
	//pRigid->SetVelocity(moveData.vel);
	
	if (m_ePrevState == PLAYER_STATE::ATTACK && pAnim->IsFinish())
	{
		m_eCurState = PLAYER_STATE::IDLE;
		m_bIsAtk = false;
	}

	updateAnimation();
	
	if (PLAYER_STATE::ATTACK == m_eCurState)
	{
		m_vecWeapon[m_iCurQuickBarIdx]->update_weapon();
	}
	else
	{
		m_vecWeapon[m_iCurQuickBarIdx]->ReForm();
	}
}


void CPlayer::render(HDC _dc)const
{
	if (m_bSlain)
	{
		return;
	}
	
	//Mgr(CCore)->RotateTransform(_dc,m_iDegree,vLT + vScale/2);
	//Mgr(CResMgr)->renderImg(_dc, m_pWeapon, this, Vec2{ 0,0 }, Vec2{ 16,16 });
	//Mgr(CCore)->ResetTransform(_dc);
	//m_iDegree = (m_iDegree + 5)%360;
	//Mgr(CCore)->RotateTransform(m_iDegree,vLT + vScale/2);
	//m_pWeapon->render(_dc);
	CObject::component_render(_dc);
	m_pAnimLeg->component_render(_dc);

	
	//m_pWeapon->render(_dc);
	//Mgr(CCore)->ResetTransform();
	//m_iDegree = (m_iDegree + 1);
	
}



void CPlayer::updateAnimation()
{
	if (m_ePrevState == m_eCurState)
	{
		return;
	}

	auto pAnim = GetComp<CAnimator>();

	switch (m_eCurState)
	{
	case PLAYER_STATE::IDLE:
	{
		if (!m_bIsAtk)
		{
			pAnim->Play(L"Player_Torso_IDLE", true);
		}
		m_pAnimLeg->Play(L"Player_Leg_IDLE", true);
	}
	break;
	case PLAYER_STATE::WALK:
	{
		if (!m_bIsAtk)
		{
			pAnim->Play(L"Player_Torso_WALK", true);
		}
		m_pAnimLeg->Play(L"Player_Leg_WALK", true);
	}
	break;
	case PLAYER_STATE::JUMP:
	{
		if (!m_bIsAtk)
		{
			pAnim->Play(L"Player_Torso_JUMP", true);
		}
		m_pAnimLeg->Play(L"Player_Leg_JUMP", true);
		
	}
	break;
	case PLAYER_STATE::ATTACK:
	{
		if (!m_bIsAtk)
		{
			pAnim->Play(L"Player_Torso_ATTACK", false);
			m_bIsAtk = true;
		}
	}
	break;
	default:
		break;
	}
	m_pPrevAnim = pAnim->GetCurAnim();
}

void CPlayer::component_update()
{
	if (m_bSlain)
	{
		return;
	}
	CObject::component_update();
	m_pAnimLeg->component_update();
	//const auto pRigid = GetComp<CRigidBody>();
	//if (IsFloatZero(pRigid->GetVelocity().y) )
	//if(m_pTRWolrd &&m_pTRWolrd->GetTileMap())
	//{
	//	::updateTileCollision(this, m_pTRWolrd);
	//	const auto pRigid = GetComp<CRigidBody>();
	//	
	//	if (bitwise_absf(pRigid->GetVelocity().y) < FLT_EPSILON)
	//	{
	//		pRigid->SetIsGround(true);
	//	}
	//}
	//else
	//	SendMoveData();
}

void CPlayer::OnCollision(CCollider* const _pOther)
{

}

void CPlayer::OnCollisionEnter(CCollider* const _pOther)
{
	auto pObj = _pOther->GetOwner();
	const wstring wstrObjName = pObj->GetName().substr(0, pObj->GetName().find(L'_'));

	if (L"Monster" == wstrObjName)
	{
		++m_iMonColCnt;
		if (!Mgr(CCamera)->IsCamMove() && !Mgr(CCamera)->IsCamShake())
		{
			Mgr(CCamera)->SetShakeFlag(true);
		}
	}
}

void CPlayer::OnCollisionExit(CCollider* const _pOther)
{
	
	auto pObj = _pOther->GetOwner();
	const wstring wstrObjName = pObj->GetName().substr(0, pObj->GetName().find(L'_'));

	if (L"Monster" == wstrObjName)
	{
		--m_iMonColCnt;
		GetComp<CRigidBody>()->SetLimitOrigin();
	}
	
}


CoRoutine CPlayer::PlayerRebirthProcess()
{
	Mgr(CCamera)->FadeIn(2.f);
	Mgr(CCamera)->SetMoveFlag(true);
	int x = TRWorld::WORLD_WIDTH / 2;
	const Vec2 vPlayerDeadPos = TRWorld::WorldToGlobal(Vec2Int(x, m_pTRWolrd->GetTileMap()->GetTopYpos(x))) - Vec2(20.0f, 28.0f);
	StartCoEvent(Mgr(CCamera)->CamMoveCoRoutine(vPlayerDeadPos));
	co_await std::suspend_always{};
	while (Mgr(CCamera)->IsCamMove())
	{
		co_await std::suspend_always{};
	}
	SetHP(200);
	SetWillPos(vPlayerDeadPos);
	SetPos(vPlayerDeadPos);
	m_eCurState = PLAYER_STATE::IDLE;
	m_bSlain = false;
	Mgr(CCamera)->SetMoveFlag(false);
	Mgr(CCamera)->SetTarget(this);
	Mgr(CCamera)->update();
	co_return;
}

void CPlayer::updateDmgCoolDown()
{
	m_fDmgCoolDown -= DT;
	if (0.f >= m_fDmgCoolDown)
	{
		m_fDmgCoolDown = m_iMonColCnt == 0 ? 0.f : 1.f;
	}
}

void CPlayer::dmg_render(HDC _dc)
{
	if (!IsCanHit())
	{
	    const int i = (int)(m_fDmgCoolDown * 10.f);
		const int iAlpha = i & 1 ? 50 : 0;

		const auto [vLT, vScale] = Mgr(CCamera)->GetRenderPos(this);

		const BLENDFUNCTION bf = {
			.BlendOp = AC_SRC_OVER,
			.BlendFlags = 0,
			.SourceConstantAlpha = (BYTE)iAlpha,
			.AlphaFormat = 0,
		};

		

		AlphaBlend(_dc
			, (int)vLT.x + 15
			, (int)vLT.y + 10
			, (int)(vScale.x) - 22
			, (int)(vScale.y) - 10
			, m_hPlayerVeilDC
			, 0
			, 0
			, 40 - 22
			, 56 - 10
			, bf);
	}
}

void CPlayer::SetMoveData(const Protocol::s2c_MOVE& movePkt_) noexcept
{
	if (m_bIsHero)
	{
		const auto pRigid = GetComp<CRigidBody>();
		auto new_data = m_interpolator.GetNewData();
		//auto& cur_data = m_interpolator.GetCurData();

		//cur_data.pos = GetPos();
		//cur_data.vel = pRigid->GetVelocity();
		//cur_data.will_pos = GetWillPos();

		
		
		new_data.pos = ToOriginVec2(movePkt_.obj_pos());
		new_data.vel = ToOriginVec2(movePkt_.vel());
		new_data.will_pos = ToOriginVec2(movePkt_.wiil_pos());
		
		//
		//SetPos(cur_data.pos * 0.4f + new_data.pos * 0.6f);
		//SetWillPos(cur_data.will_pos * 0.4f + new_data.will_pos * 0.6f);
		//pRigid->SetVelocity(cur_data.vel * 0.4f + new_data.vel * 0.6f);
		m_interpolator.UpdateNewData(new_data, movePkt_.time_stamp());

		//m_interpolator.UpdateOnlyTimeStamp(movePkt_.time_stamp());
		
		//SetPos(ToOriginVec2(movePkt_.obj_pos()));
		//pRigid->SetVelocity(ToOriginVec2(movePkt_.vel()));
		//const bool bGround = movePkt_.ground();
		//pRigid->SetIsGround(movePkt_.ground());
		//SetState((PLAYER_STATE)movePkt_.state());
		//if (bGround)
		//{
		//	auto vPos = GetPos();
		//	const auto vPosY = GetPos().y - pRigid->GetVelocity().y * DT;
		//	vPos.y = vPosY;
		//	SetPos(vPos);
		//}
	}
	else
	{
		const Vec2 vFutureVel = ToOriginVec2(movePkt_.vel()) + ToOriginVec2(movePkt_.accel()) * DT;

		const Vec2 vFuturePos = ToOriginVec2(movePkt_.obj_pos()) + ToOriginVec2(movePkt_.vel()) * DT + ToOriginVec2(movePkt_.accel()) * DT * DT * 0.5f;
		//std::cout << vFuturePos.x << ", " << vFuturePos.y << std::endl;
		const MoveData moveData
		{
			.pos = vFuturePos, 
			.will_pos = ToOriginVec2(movePkt_.wiil_pos()), 
			.vel = vFutureVel 
		};
		m_interpolator.UpdateNewData(moveData, movePkt_.time_stamp());
		//SetPos(moveData.pos);
		SetState((PLAYER_STATE)movePkt_.state());
		GetComp<CAnimator>()->SetAnimDir(movePkt_.anim_dir());
		GetComp<CRigidBody>()->SetIsGround(movePkt_.ground());
		GetComp<CRigidBody>()->SetVelocity(moveData.vel);
	}
}

void CPlayer::updateQuickBarState(const int _idx)
{
	m_iCurQuickBarIdx = _idx;
	auto& quick_bar_list = m_pTRWolrd->GetQuickBarList();
	static const wstring wstrItem = L"Weapon_";
	for (int i = 0; i < 10; ++i)
	{
		if (quick_bar_list[i]->Blank())
		{
			m_vecWeapon[i]->SetActivate(false);
			m_vecWeapon[i]->SetDmg(1);
		}
		else
		{
			if (m_iCurQuickBarIdx != i)
			{
				m_vecWeapon[i]->SetActivate(false);
				m_vecWeapon[i]->ReForm();
			}
			else
			{
				m_vecWeapon[i]->SetActivate(true);
			}
			static size_t strPos = 0;
			if (wstring::npos != quick_bar_list[i]->GetItemStack().GetItem()->GetName().find(L' '))
			{
				strPos = quick_bar_list[i]->GetItemStack().GetItem()->GetName().find(L' ') + 1;
			}
			else
			{
				strPos = 0;
			}
			wstring tempName = std::move(quick_bar_list[i]->GetItemStack().GetItem()->GetName().substr(strPos));
			m_vecWeapon[i]->SetWeaponState(quick_bar_list[i]->GetItemStack().GetItem()->GetItemElement(), std::move(wstrItem + std::move(tempName)));
		}

	}
}