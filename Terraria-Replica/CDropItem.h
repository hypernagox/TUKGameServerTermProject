#pragma once
#include "CObject.h"
#include "TRItemStack.h"
#include "Protocol.pb.h"

class CDropItem : public CObject
{
private:
    TRItemStack m_item;

    MoveInterpolator m_interpolator;
public:
    CDropItem(TRWorld* const _trWorld, TRItemStack _item);
    ~CDropItem();
    CDropItem* Clone()const override
    {
        auto p = new CDropItem{ *this };
        return p;
    }

    void update() override;
    void component_update()override;
    void render(HDC _dc) const override;

    virtual void OnCollision(CCollider* const _pOther) override;
    virtual void OnCollisionEnter(CCollider* const _pOther) override;
    virtual void OnCollisionExit(CCollider* const _pOther) override;

    void SetMoveData(Protocol::s2c_MOVE pkt_);
};