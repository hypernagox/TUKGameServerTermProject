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


    void SetQuickBarIdx(const int _idx) { m_iCurQuickBarIdx = _idx; }
    int GetQuickBarIdx()const { return m_iCurQuickBarIdx; }

    void SendMoveData() noexcept;
    
private:

};

