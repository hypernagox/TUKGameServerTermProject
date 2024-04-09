#pragma once
#include "ServerObject.h"

class Missile
	:public ServerObject
{
public:
	Missile(const Vec2 vPos_);
public:
	void update()override;
	void component_update()override;
	void render(HDC dc_)const override;
	void SetDir(const float dir_) { m_dir = dir_; }
private:
	const CImage* m_pMissleImg = nullptr;
	MoveInterpolator m_interpolator;
	bool m_bFirst = false;
	float m_dir = 1.f;
};

