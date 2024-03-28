#pragma once
#include "CObject.h"
#include "TRItemStack.h"
#include "Protocol.pb.h"

class CDropItem : public CObject
{
private:
    TRItemStack m_item;

    MoveInterpolator m_interpolator;
    uint64_t m_itemId;
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
   //void SetPos(const Vec2 v_) {
   //    CObject::SetPos(v_);
   //    MoveData data;
   //    data.pos = v_;
   //    m_interpolator.UpdateNewData(data, NetHelper::GetTimeStampMilliseconds());
   //}


    uint64_t GetItemId()const noexcept { return m_itemId; }
    void SetItemId(const uint64_t id_)noexcept { m_itemId = id_; }
};