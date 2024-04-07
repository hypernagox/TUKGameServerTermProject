#pragma once
#include "pch.h"
#include "ServerObject.h"

class CTexture;
class CAnimation;
class CAnimator;
class TRWorld;
class TRTile;

enum class PLAYER_STATE
{
    IDLE,
    WALK,
    JUMP,
    ATTACK,

    END
};

class CWeapon;

class CPlayer :
    public ServerObject
{
    friend class CScene_Intro;
protected:

    int m_iCurQuickBarIdx = 0;
    vector<CWeapon*>  m_vecWeapon;
    bool m_bIsHero = false;
    class CAnimation* m_pPrevAnim = {};
    std::unique_ptr<class CAnimator> m_pAnimLeg;
    mutable int m_iDegree = 0;
    PLAYER_STATE m_eCurState = PLAYER_STATE::IDLE;
    PLAYER_STATE m_ePrevState = PLAYER_STATE::IDLE;
    bool m_bIsAtk = false;
    bool m_bIsIDLE = false;
    bool m_bPrevCol = false;

    bool m_bDirtyFlag = false;

    bool m_bSlain = false;
    bool m_bRequestAttack = false;

    float m_fDmgCoolDown = 0.f;
    int m_iMonColCnt = 0;
    HDC m_hPlayerVeilDC; 
    HBITMAP m_hPlayerVeilBit;
public:
    bool IsHero()const noexcept { return m_bIsHero; }
    CPlayer(TRWorld* const _trWorld);
    CPlayer(const CPlayer& other);
    void update()override;
    void render(HDC _dc)const override;
    CPlayer* Clone()const override
    {
        auto p = new CPlayer{ *this };
        return p;
    }

public:
    void component_update()override;
    void updateAnimation();
    CoRoutine PlayerRebirthProcess();
    void dmg_render(HDC _dc);
    virtual void OnCollision(CCollider* const _pOther);
    virtual void OnCollisionEnter(CCollider* const _pOther);
    virtual void OnCollisionExit(CCollider* const _pOther);
   
    void updateDmgCoolDown();

    bool IsCanHit()const { return 0.f >= m_fDmgCoolDown || 1.f <= m_fDmgCoolDown; }

    void SetState(const PLAYER_STATE eType_)noexcept {
        m_ePrevState = m_eCurState;
        m_eCurState = eType_;
    }
public:
    void updateQuickBarState(const int _idx);
   
    virtual void SetNewMoveData(const Protocol::s2c_MOVE& movePkt_)noexcept override;
};

