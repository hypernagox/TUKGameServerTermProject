#pragma once
#include "CObject.h"

class Missle
	:public CObject
{
public:
	Missle(const Vec2 vPos_);
public:
	void update()override;
	void component_update()override;
	void render(HDC dc_)const override;
public:
	Missle* Clone()const override {
		auto missle = new Missle{ GetPos()};
		return missle;
	}
	auto& GetInterpolator() { return m_interpolator; }
	void SetMoveData(Protocol::s2c_MOVE movePkt_);
private:
	const CImage* m_pMissleImg = nullptr;
	MoveInterpolator m_interpolator;
	bool m_bFirst = false;
};

