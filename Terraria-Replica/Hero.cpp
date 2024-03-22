#include "pch.h"
#include "Hero.h"
#include "CKeyMgr.h"
#include "CAnimation.h"
#include "CAnimator.h"
#include "CSoundMgr.h"
#include "TRWorld.h"
#include "CWeapon.h"
#include "CRigidBody.h"
#include "s2c_PacketHandler.h"
#include "TRItem.h"
#include "CTimeMgr.h"
#include "CCollider.h"

extern void updateTileCollision(CObject* const _pObj, TRWorld* const _pTRWorld);

Hero::Hero(TRWorld* const _trWorld)
	:CPlayer{ _trWorld }
{
	m_bIsHero = true;
}

Hero::~Hero()
{
}

void Hero::update()
{
	if (m_bSlain)
	{
		return;
	}

	CObject::update();
	auto pAnim = GetComp<CAnimator>();

	m_ePrevState = m_eCurState;

	updateDmgCoolDown();

	updateQuickBarState(m_pTRWolrd->GetQuickBarIdx());

	updateMove();

	updateState();

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

void Hero::updateState()
{
	m_bIsIDLE = true;
	const auto pAnim = GetComp<CAnimator>();
	const auto pRigid = GetComp<CRigidBody>();

	if (KEY_TAP(KEY::SPACE) && IsFloatZero(pRigid->GetVelocity().y))
	{
		pRigid->SetIsGround(false);

		pRigid->AddVelocity(Vec2::down * 720.0f);
		//pRigid->SetForce(Vec2{ 0, -1000.0f });
		m_eCurState = PLAYER_STATE::JUMP;
		m_bIsIDLE = false;

		m_bDirtyFlag = true;
	}

	if (m_ePrevState == PLAYER_STATE::ATTACK && pAnim->IsFinish())
	{
		m_eCurState = PLAYER_STATE::IDLE;
		m_bIsAtk = false;
	}

	if (KEY_HOLD(KEY::A))
	{
		pAnim->SetAnimLeft();
		m_pAnimLeg->SetAnimLeft();
		m_eCurState = PLAYER_STATE::WALK;
		m_bIsIDLE = false;

		m_bDirtyFlag = true;
	}

	if (KEY_HOLD(KEY::D))
	{
		pAnim->SetAnimRight();
		m_pAnimLeg->SetAnimRight();
		m_eCurState = PLAYER_STATE::WALK;
		m_bIsIDLE = false;

		m_bDirtyFlag = true;
	}

	if (m_bRequestAttack)
	{
		m_eCurState = PLAYER_STATE::ATTACK;
		m_bIsIDLE = false;
		m_bRequestAttack = false;
		Mgr(CSoundMgr)->PlayEffect("Item_1.wav", 1.f);
	}

	if (m_bIsAtk)
	{
		m_eCurState = PLAYER_STATE::ATTACK;
		m_bIsIDLE = false;
	}

	if (m_bIsIDLE)
	{
		m_eCurState = PLAYER_STATE::IDLE;
	}

	if (bitwise_absf(pRigid->GetVelocity().y) > 0.f && PLAYER_STATE::ATTACK != m_eCurState)
	{
		m_eCurState = PLAYER_STATE::JUMP;
		m_bIsIDLE = false;
	}
}

void Hero::updateMove()
{
	const auto pRigid = GetComp<CRigidBody>();
	const auto vPos = GetPos();

	if (KEY_HOLD(KEY::A))
	{
		pRigid->AddVelocity(Vec2{ -20.0f, 0.0f });

		m_bDirtyFlag = true;
	}

	if (KEY_HOLD(KEY::D))
	{
		pRigid->AddVelocity(Vec2{ 20.0f, 0.0f });

		m_bDirtyFlag = true;
	}

	/*if (KEY_HOLD(KEY::W))
	{
		SetPos({ GetPos().x,GetPos().y - 5.f });
		pRigid->AddVelocity(Vec2{ 0,-300 });
		pRigid->AddForce(Vec2{ 0,-300 });
	}
	if (KEY_HOLD(KEY::S))
	{
		SetPos({ GetPos().x,GetPos().y + 5.f });
		pRigid->AddVelocity(Vec2{ 0,300 });
		pRigid->AddForce(Vec2{ 0,300 });
	}*/
}

void Hero::component_update()
{
	if (m_bSlain)
	{
		return;
	}
	

	CObject::component_update();
	m_pAnimLeg->component_update();

	//if (PLAYER_STATE::JUMP == m_eCurState && !m_bDirtyFlag)
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
		SendMoveData();
}



void Hero::UseItem()
{
	if (m_bIsAtk)
		return;

	m_bRequestAttack = true;
}

void Hero::SendMoveData() noexcept
{
	m_fAccTime += DT;

	if (m_bDirtyFlag || 0.01f <=m_fAccTime)
	{
		m_fAccTime = 0.f;
		m_bDirtyFlag = false;
		Protocol::c2s_MOVE pkt;

		*pkt.mutable_obj_pos() = ToProtoVec2(GetPos());
		*pkt.mutable_wiil_pos() = ToProtoVec2(GetWillPos());
		*pkt.mutable_scale() = ToProtoVec2(GetComp<CCollider>()->GetScale());
		*pkt.mutable_vel() = ToProtoVec2(GetComp<CRigidBody>()->GetVelocity());
		pkt.set_state((Protocol::PLAYER_STATE)m_eCurState);
		pkt.set_anim_dir(GetComp<CAnimator>()->GetAnimDir());
		Send(pkt);
	}
}