#include "pch.h"
#include "TimerNPC.h"
#include "AIComponent.h"
#include "Object.h"
#include "TRWorldMgr.h"
#include "TRWorldChunk.h"
#include "TRWorldRoom.h"
#include "c2s_PacketHandler.h"

using namespace ServerCore;

const bool TimerNPC::IsValid() const noexcept
{
    return GetContentsEntity()->ObjectCast()->IsValid();
}

void TimerNPC::InitTimer(const uint64 tick_ms) noexcept
{
    ServerCore::TimerObject::InitTimer(tick_ms);
}

void TimerNPC::ToAwaker(const IocpEntity* const awaker) noexcept
{
    const auto cur_state = m_timer_state.load();
    if (ServerCore::TIMER_STATE::IDLE == cur_state)
    {
        m_curChaseUser = awaker->GetObjectID();
    }
}

const ServerCore::TIMER_STATE TimerNPC::TimerUpdate() noexcept
{
    const auto pEntity = (Object*)GetContentsEntity().get();

    if (!pEntity->IsValid())
    {
        StopTimer();
        return TIMER_STATE::IDLE;
    }

    if (!m_aiComponent->ExecuteAI())
        return TIMER_STATE::IDLE;

    const auto sector = (TRWorldRoom*)pEntity->m_pCurSector.load(std::memory_order_relaxed);
    
    Protocol::s2c_MOVE pkt;
    *pkt.mutable_obj_pos() = pEntity->GetPos();
    pkt.set_obj_id(GetObjectID());
    pkt.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
    
    Protocol::s2c_APPEAR_NEW_OBJECT add_pkt;
    {
        *add_pkt.mutable_appear_pos() = pEntity->GetPos();

        add_pkt.set_is_player(false);
        //add_pkt.set_sector(0);
        add_pkt.set_obj_name("MONSTER");
        add_pkt.set_obj_id(GetObjectID());
        add_pkt.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
    }
    Protocol::s2c_LEAVE_OBJECT remove_pkt;
    {
        remove_pkt.set_is_player(false);
        remove_pkt.set_obj_id(GetObjectID());
        remove_pkt.set_sector(0);
    }
    
    const int sector_state = sector->MoveBroadCast(SharedFromThis<IocpEntity>()
        , ServerCore::c2s_PacketHandler::MakeSendBuffer(add_pkt)
        , ServerCore::c2s_PacketHandler::MakeSendBuffer(remove_pkt)
        , ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt)
        , sector->GetAdjSector8());

    auto next_sector = sector->GetWorldChunk()->GetWorldSector(pEntity->GetPos());

    if (sector != next_sector.get())
    {
        sector->ImmigrationEnqueue(std::move(next_sector), GetObjectID());
    } 

    if (ServerCore::SECTOR_STATE::USER_EMPTY & sector_state)
    {
        return TIMER_STATE::IDLE;
    }
    else
    {
        return TIMER_STATE::RUN;
    }
}

void TimerNPC::AddAIComponent(S_ptr<AIComponent> ai) noexcept
{
    m_aiComponent = ai;
    ((Object*)GetContentsEntity().get())->AddBaseComponent(std::move(ai));
}

const bool TimerNPC::CanAwake(const IocpEntity* const awaker) const noexcept
{
    constexpr const int DIST = (VIEW_RANGE / 3) * 2;
    return GetObjectDistancePow2(awaker->GetContentsEntity()->ObjectCast(), GetContentsEntity()->ObjectCast())
        <= DIST * DIST;
}
