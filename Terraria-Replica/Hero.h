#pragma once
#include "CPlayer.h"

extern int sector;

class Hero
	:public CPlayer
{
private:
    float m_fAccTime = 0.f;
public:
	Hero(TRWorld* const _trWorld);
	~Hero();
	Hero* Clone()const override
	{
		auto p = new Hero{ *this };
		return p;
	}
public:
    void update()override;
   

    void updateState();
    void updateMove();

    void component_update()override;


    //void updateQuickBarState(const int _idx);

    void SetSlane(bool _b) { m_bSlain = _b; }
    bool IsPlayerSlain()const { return m_bSlain; }
    void UseItem();
   
    void render(HDC _dc)const override;

    void SetQuickBarIdx(const int _idx) { m_iCurQuickBarIdx = _idx; }
    int GetQuickBarIdx()const { return m_iCurQuickBarIdx; }

    bool m_bNowRequset = false;
    bool m_bPartyRequest = false;
    wstring m_partyRequestUser;
    std::unordered_set<wstring> m_partyList;
    uint64_t m_partySinchunza = 0;
public:
    virtual void SetNewMoveData(const Protocol::s2c_MOVE& movePkt_)noexcept override;
    void SendMoveData()noexcept;
private:

};

