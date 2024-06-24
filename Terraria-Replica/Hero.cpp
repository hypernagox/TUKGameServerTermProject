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
#include "Missile.h"
#include "TRMain.h"
#include"CCore.h"
#include "CCamera.h"

extern void updateTileCollision(CObject* const _pObj, TRWorld* const _pTRWorld);

int sector = 0;

wstring inputBuffer;
static unordered_map<wchar_t, bool> keymap;
static bool enterPressed = false;
static bool prevEnterState = false;

std::unordered_map<wchar_t, wchar_t> alphabetToHangul = {
	{L'A', L'ㅁ'}, {L'B', L'ㅠ'}, {L'C', L'ㅊ'}, {L'D', L'ㅇ'},
	{L'E', L'ㄷ'}, {L'F', L'ㄹ'}, {L'G', L'ㅎ'}, {L'H', L'ㅗ'},
	{L'I', L'ㅑ'}, {L'J', L'ㅓ'}, {L'K', L'ㅏ'}, {L'L', L'ㅣ'},
	{L'M', L'ㅡ'}, {L'N', L'ㅜ'}, {L'O', L'ㅐ'}, {L'P', L'ㅔ'},
	{L'Q', L'ㅂ'}, {L'R', L'ㄱ'}, {L'S', L'ㄴ'}, {L'T', L'ㅅ'},
	{L'U', L'ㅕ'}, {L'V', L'ㅍ'}, {L'W', L'ㅈ'}, {L'X', L'ㅌ'},
	{L'Y', L'ㅛ'}, {L'Z', L'ㅋ'},
	{L'a', L'ㅁ'}, {L'b', L'ㅠ'}, {L'c', L'ㅊ'}, {L'd', L'ㅇ'},
	{L'e', L'ㄷ'}, {L'f', L'ㄹ'}, {L'g', L'ㅎ'}, {L'h', L'ㅗ'},
	{L'i', L'ㅑ'}, {L'j', L'ㅓ'}, {L'k', L'ㅏ'}, {L'l', L'ㅣ'},
	{L'm', L'ㅡ'}, {L'n', L'ㅜ'}, {L'o', L'ㅐ'}, {L'p', L'ㅔ'},
	{L'q', L'ㅂ'}, {L'r', L'ㄱ'}, {L's', L'ㄴ'}, {L't', L'ㅅ'},
	{L'u', L'ㅕ'}, {L'v', L'ㅍ'}, {L'w', L'ㅈ'}, {L'x', L'ㅌ'},
	{L'y', L'ㅛ'}, {L'z', L'ㅋ'}
};

bool IsMouseInsideRectangle(const Vec2& mousePos, const Vec2& center, float width, float height) {
	
	const float left = center.x - (width / 2);
	const float right = center.x + (width / 2);
	const float top = center.y - (height / 2);
	const float bottom = center.y + (height / 2);

	return (mousePos.x >= left && mousePos.x <= right &&
		mousePos.y >= top && mousePos.y <= bottom);
}

bool ProcessInput()
{
	
	static int enter_state = 0;
	
	if (KEY_TAP(KEY::ENTER))
		enter_state = (enter_state + 1) % 3;
	
	if (0 == enter_state)return true;
	
	for (int key = 0; key <= 127; ++key)
	{ 
		const auto k = static_cast<wchar_t>(key);
		if (GetAsyncKeyState(key) & 0x8000)
		{
			if (!keymap[k])
			{
				inputBuffer.push_back(k);
				keymap[k] = true;
			}
		}
		else
		{
			keymap[k] = false;
		}
	}

	if (KEY_TAP(KEY::BACK))
	{
		if (2 <= inputBuffer.size()) 
		{
			inputBuffer.pop_back();
			inputBuffer.pop_back();
		}
	}

	if (2==enter_state) 
	{
		std::string message = WideToUtf8(inputBuffer);
		inputBuffer.clear();
		Protocol::c2s_BUY_ITEM pkt;
		std::string temp;
		
		for (int i = 0; i < message.size(); ++i)
		{
			if (isdigit(message[i])) 
				temp.push_back(message[i]);
		}
		if (!temp.empty())
		{
			pkt.set_item_name(temp);
			Send(pkt);
		}
		else
		{
			Protocol::c2s_CHAT c;
			c.set_msg(message);
			Send(c);
		}

		enter_state = 0;
		return true;
	}
	return false;
}

Hero::Hero(TRWorld* const _trWorld)
	:CPlayer{ _trWorld }
{
	//m_pTRWolrd = Mgr(TRMain)->active_world;
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
	if (!ProcessInput())
		return;

	static float acc = 0.f;
	acc += DT;
	if (m_bSlain)
	{
		return;
	}
	if (m_partyList.empty() && false == m_bNowRequset && KEY_TAP(KEY::RBTN))
	{
		//const auto v = TRWorld::GlobalToWorld(MOUSE_POS);
		//std::cout << v.x << " , " << v.y << std::endl;
		//Protocol::c2s_CREATE_MISSILE pkt;
		//const int dir = GetComp<CAnimator>()->GetAnimDir() == 1 ? -1 : 1;
		//*pkt.mutable_obj_pos() = ToProtoVec2(GetPos());
		//pkt.set_dir(dir);
		//Send(pkt);
		const auto scale = GetScale();
		bool flag = false;
		uint64 targetId = 0;
		for (const auto [key, player] : m_pTRWolrd->m_mapOtherPlayer)
		{
			if (IsMouseInsideRectangle(MOUSE_POS, player->GetPos(), scale.x, scale.y))
			{
				targetId = key;
				flag = true;
				break;
			}
		}
		if (flag)
		{
			// 신청패킷
			m_partySinchunza = targetId;
			m_bNowRequset = true;
			Protocol::c2s_PARTY_SINCHUNG pkt;
			pkt.set_target_id(targetId);
			Send(pkt);
		}
	}
	if (KEY_TAP(KEY::P))
	{
		if (acc >= 1.f)
		{
			acc = 0.f;
			//Protocol::c2s_CREATE_MONSTER pkt;
			Protocol::c2s_BUY_ITEM pkt;
			pkt.set_item_name("Item_28.png");
			Send(pkt);
		}
	}
	if (KEY_TAP(KEY::W))
	{
		sector = (sector + 1) % (etoi(SCENE_TYPE::INTRO));
		Protocol::c2s_TRY_NEW_ROOM pkt;
		pkt.set_cur_sector_num(sector);
		//pkt.set_next_sector_num((sector + 1) % 5);
		Send(pkt);
		return;
		
		if (5 == sector)return;
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
		const auto wPos = TRWorld::GlobalToWorld(GetPos());
		std::cout << wPos.x << ", " << wPos.y << std::endl;
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

	if (!m_partyList.empty())
	{
		if (KEY_TAP(KEY::N))
		{
			// 파티탈퇴패킷
			Protocol::c2s_PARTY_OUT pkt;
			pkt.set_target_id(m_partySinchunza);
			pkt.set_target_name(WideToUtf8(m_partyRequestUser));
			Send(pkt);
		}
	}
}

void Hero::updateState()
{
	m_bIsIDLE = true;
	const auto pAnim = GetComp<CAnimator>();
	const auto pRigid = GetComp<CRigidBody>();

	Protocol::c2s_INPUT_KEY pkt;
	
	if (!m_pTRWolrd->GetQuickBarList()[m_iCurQuickBarIdx]->Blank())
	{
		const auto item_name = m_pTRWolrd->GetQuickBarList()[m_iCurQuickBarIdx]->GetItemStack().GetItem()->GetElementName();
		if (std::wstring::npos == item_name.find(L"Item") || L"Item_28.png" !=item_name)
		{
			m_curWeaponName
				= m_pTRWolrd->GetQuickBarList()[m_iCurQuickBarIdx]->GetItemStack().GetItem()->GetElementName();
		}
		else
		{
			m_curWeaponName
				= m_pTRWolrd->GetQuickBarList()[m_iCurQuickBarIdx]->GetItemStack().GetItem()->GetKeyName();
		}
	}

	if (KEY_TAP(KEY::Z))
	{
		Protocol::c2s_TRY_GET_ITEM pkt;
		pkt.set_time_stamp(NetHelper::GetTimeStampMilliseconds());
		//*pkt.mutable_obj_pos() = ::ToProtoVec2(Vec2{});
		*pkt.mutable_obj_pos() = ::ToProtoVec2(GetPos());
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
		Protocol::c2s_SWING pkt;
		pkt.set_item_name(::WideToUtf8(m_curWeaponName));
		pkt.set_dir(GetComp<CAnimator>()->GetAnimDir() == 0 ? 1 : -1);
		Send(pkt);

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

	//UpdateMoveData();
}

void Hero::UseItem()
{
	if (m_bIsAtk)
		return;

	m_bRequestAttack = true;
}

void Hero::render(HDC _dc) const
{
	CPlayer::render(_dc);
	renderText(Mgr(CCore)->GetMainDC(), RGB(255, 255, 255),Mgr(CCore)->GetResolutionV()/2.f, inputBuffer);

	if (m_bPartyRequest)
	{
		renderText(Mgr(CCore)->GetMainDC(), RGB(255, 0, 0), { 0,300 },
			std::format(L"Party Request From User: {} , Press Y/N", m_bPartyRequest));
		if (KEY_TAP(KEY::Y))
		{
			// 수락패킷
			Protocol::c2s_PARTY_SURAK pkt;
			pkt.set_is_surak(true);
			pkt.set_target_id(m_partySinchunza);
			Send(pkt);
			
		}
		if (KEY_TAP(KEY::N))
		{
			Protocol::c2s_PARTY_SURAK pkt;
			pkt.set_is_surak(false);
			pkt.set_target_id(m_partySinchunza);
			Send(pkt);
			// 거절패킷
		}
	}
}

void Hero::SetNewMoveData(const Protocol::s2c_MOVE& movePkt_) noexcept
{
	const MoveData moveData
	{
		.pos = ToOriginVec2(movePkt_.obj_pos()),
		.will_pos = ToOriginVec2(movePkt_.wiil_pos()),
		.vel = ToOriginVec2(movePkt_.vel())
	};

	GetInterPolator().UpdateNewData(moveData, movePkt_.time_stamp());
}

void Hero::SendMoveData() noexcept
{
	m_fAccTime += DT;
	
	if (m_bDirtyFlag || 0.15f <= m_fAccTime)
	{
		m_fAccTime = 0.f;
		m_bDirtyFlag = false;
		Protocol::c2s_MOVE pkt = ServerObject::MakeSendData();
		pkt.set_state((Protocol::PLAYER_STATE)m_eCurState);
		pkt.set_anim_dir(GetComp<CAnimator>()->GetAnimDir());
		SetCurrentTimeStampRTT();

		Send(pkt);
	}
}