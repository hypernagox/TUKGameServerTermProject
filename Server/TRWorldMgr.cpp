#include "pch.h"
#include "TRWorldMgr.h"
#include "TRWorldRoom.h"

TRWorldMgr::TRWorldMgr()
{
}

TRWorldMgr::~TRWorldMgr()
{
}

void TRWorldMgr::Init() noexcept
{
	RegisterWorld(SECTOR::SECTOR_0, ServerCore::MakeShared<TRWorldRoom>(SECTOR::SECTOR_0));
}

void TRWorldMgr::RegisterWorld(const SECTOR worldID_, std::shared_ptr<TRWorldRoom> world_) noexcept
{
	world_->Init();

	world_->register_cache_shared();

	world_->Update();

	m_mapWorld.emplace_no_return(worldID_, std::move(world_));
}

const float TRWorldMgr::GetSectorDT(const SECTOR eType_) const noexcept
{
	return (*m_mapWorld.find(eType_))->GetSectorDT();
}
