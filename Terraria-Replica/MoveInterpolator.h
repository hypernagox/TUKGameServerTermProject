#pragma once
#include "Interpolator.hpp"

struct  MoveData
{
    Vec2 pos;
    Vec2 will_pos;
    Vec2 vel;
};

class MoveInterpolator
    :public NetHelper::Interpolator<MoveData>
{
public:
    MoveData GetInterPolatedData()noexcept
    {
        MoveData temp;
        UpdateInterpolationParam();
        temp.pos = SmoothLinearInterpolation(m_curData.pos, m_newData.pos);
        temp.will_pos = SmoothLinearInterpolation(m_curData.will_pos, m_newData.will_pos);
        temp.vel = SmoothLinearInterpolation(m_curData.vel, m_newData.vel);
        return temp;
    }
};

