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
#include "CCamera.h"
#include "CSceneMgr.h"
#include "CScene.h"
#include "CEventMgr.h"

extern void updateTileCollision(CObject* const _pObj, TRWorld* const _pTRWorld);

int sector = 0;

Hero::Hero(TRWorld* const _trWorld)
	:CPlayer{ _trWorld }
{
	m_bIsHero = true;
	SetName(L"HERO");
	//CreateComponent(COMPONENT_TYPE::RIGIDBODY);
	//auto pRigid = GetComp<CRigidBody>();
	//pRigid->SetIsGround(false);
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
	
	if (KEY_TAP(KEY::W))
	{
		const auto vPos = GetPos();
		const auto vScale = GetScale();
		const auto vScaleX = vScale.x / 2.f;
		if (vPos.x + vScaleX >= 2000 + sector * 1350 && vPos.x <= 2000 + sector * 1350)
		{
			if (sector == 4)
				return;
			Protocol::c2s_TRY_NEW_ROOM pkt;
			pkt.set_cur_sector_num(sector);
			pkt.set_next_sector_num((sector + 1) % 5);
			Send(pkt);
		}
		else if (vPos.x - vScaleX <= 2000 + (sector - 1) * 1350 + 50.f && vPos.x >= 2000 + (sector - 1) * 1350 + 50.f)
		{
			if (sector == 0)
				return;
			Protocol::c2s_TRY_NEW_ROOM pkt;
			pkt.set_cur_sector_num(sector);
			pkt.set_next_sector_num(wrapAround(sector - 1, 0, 5));
			Send(pkt);
		}
		//Protocol::c2s_TRY_NEW_ROOM pkt;
		//pkt.set_cur_sector_num(sector);
		//pkt.set_next_sector_num((sector + 1) % 5);
		//Send(pkt);
		//std::cout << GetPos().x << ", " << GetPos().y << std::endl;
		

		//sector = (sector + 1) % 5;
	}
	
	//std::cout << GetPos().x << ", " << GetPos().y << std::endl;
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

	Protocol::c2s_INPUT_KEY pkt;
	
	if (KEY_TAP(KEY::Z))
	{
		Protocol::c2s_TRY_GET_ITEM pkt;
		pkt.set_time_stamp(NetHelper::GetTimeStampMilliseconds());
		*pkt.mutable_obj_pos() = ::ToProtoVec2(Vec2{});
		Send(pkt);
	}

	if (KEY_TAP(KEY::SPACE) && IsFloatZero(pRigid->GetVelocity().y))
	{
		pRigid->SetIsGround(false);

		pRigid->AddVelocity(Vec2::down * 720.0f);
		//pRigid->SetForce(Vec2{ 0, -1000.0f });
		m_eCurState = PLAYER_STATE::JUMP;
		m_bIsIDLE = false;

		m_bDirtyFlag = true;

		pkt.set_key_state(1);
		pkt.set_vk_key(VK_SPACE);
		Send(pkt);
	}

	if (m_ePrevState == PLAYER_STATE::ATTACK && pAnim->IsFinish())
	{
		m_eCurState = PLAYER_STATE::IDLE;
		m_bIsAtk = false;
	}

	if (KEY_TAP(KEY::A))
	{
		pAnim->SetAnimLeft();
		m_pAnimLeg->SetAnimLeft();
		m_eCurState = PLAYER_STATE::WALK;
		m_bIsIDLE = false;

		m_bDirtyFlag = true;

		pkt.set_key_state(1);
		pkt.set_vk_key('A');
		Send(pkt);
	}

	if (KEY_TAP(KEY::D))
	{
		pAnim->SetAnimRight();
		m_pAnimLeg->SetAnimRight();
		m_eCurState = PLAYER_STATE::WALK;
		m_bIsIDLE = false;

		m_bDirtyFlag = true;

		pkt.set_key_state(1);
		pkt.set_vk_key('D');
		Send(pkt);
	}

	if (KEY_AWAY(KEY::SPACE))
	{
		m_bDirtyFlag = true;
		pkt.set_key_state(3);
		pkt.set_vk_key(VK_SPACE);
		Send(pkt);
	}
	if (KEY_AWAY(KEY::A))
	{
		m_bDirtyFlag = true;
		pkt.set_key_state(3);
		pkt.set_vk_key('A');
		Send(pkt);
	}
	if (KEY_AWAY(KEY::D))
	{
		m_bDirtyFlag = true;
		pkt.set_key_state(3);
		pkt.set_vk_key('D');
		Send(pkt);
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
		//m_bDirtyFlag = true;
	}
}

void Hero::updateMove()
{
	const auto pRigid = GetComp<CRigidBody>();
	const auto vPos = GetPos();

	//const auto moveData = m_interpolator.GetInterPolatedData();
	//SetWillPos(moveData.will_pos);
	//SetPos(moveData.pos);
	//pRigid->SetVelocity(moveData.vel);

	if (KEY_HOLD(KEY::A))
	{
		pRigid->AddVelocity(Vec2{ -20.0f, 0.0f });

		//m_bDirtyFlag = true;
	}

	if (KEY_HOLD(KEY::D))
	{
		pRigid->AddVelocity(Vec2{ 20.0f, 0.0f });

		//m_bDirtyFlag = true;
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

	m_bDirtyFlag |= KEY_TAP(KEY::A) || KEY_TAP(KEY::D) || KEY_TAP(KEY::SPACE);
}

void Hero::component_update()
{
	if (m_bSlain)
	{
		return;
	}


	CObject::component_update();
	m_pAnimLeg->component_update();
	const auto pRigid = GetComp<CRigidBody>();
	//if ((PLAYER_STATE::JUMP == m_eCurState) && !m_bDirtyFlag)
	//{
	//	::updateTileCollision(this, m_pTRWolrd);
	//	//const auto pRigid = GetComp<CRigidBody>();
	//
	//	if (IsFloatZero(pRigid->GetVelocity().y))
	//	{
	//		pRigid->SetIsGround(true);
	//	}
	//}
	//else
	SendMoveData();

	const auto move_data = m_interpolator.GetInterPolatedData();
	SetPos(move_data.pos);
	//pRigid->SetVelocity(move_data.vel);
	//SetWillPos(move_data.will_pos);
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
	
	if (m_bDirtyFlag || 0.1f <= m_fAccTime)
	{
		const float dt_rtt = m_interpolator.GetPredictRTT();

		m_fAccTime = 0.f;
		m_bDirtyFlag = false;
		Protocol::c2s_MOVE pkt;
		const auto vVelocity = GetComp<CRigidBody>()->GetVelocity();
		*pkt.mutable_obj_pos() = ToProtoVec2(GetPos()+ GetComp<CRigidBody>()->GetVelocity() * dt_rtt);
		*pkt.mutable_wiil_pos() = ToProtoVec2(GetWillPos());
		*pkt.mutable_scale() = ToProtoVec2(GetComp<CCollider>()->GetScale());
		*pkt.mutable_vel() = ToProtoVec2(GetComp<CRigidBody>()->GetVelocity() + GetComp<CRigidBody>()->GetAccel()* dt_rtt);
		pkt.set_state((Protocol::PLAYER_STATE)m_eCurState);
		pkt.set_anim_dir(GetComp<CAnimator>()->GetAnimDir());
		pkt.set_ground(GetComp<CRigidBody>()->IsGround());

		m_interpolator.SetCurrentTimeStampRTT();

		Send(pkt);
	}
}