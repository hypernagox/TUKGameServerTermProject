#include "pch.h"
#include "TRWorldMgr.h"
#include "TRWorldRoom.h"
#include "TRWorldChunk.h"

TRWorldMgr::TRWorldMgr()
{
}

TRWorldMgr::~TRWorldMgr()
{
	for (auto& chunk : m_arrWorldChunk)
	{
		//chunk->reset_cache_shared();
	}
}

void TRWorldMgr::Init() noexcept
{
	int cnt = 0;
	for (auto& chunk : m_arrWorldChunk)
	{
		chunk = ServerCore::MakeShared<TRWorldChunk>(cnt++);
		chunk->Init();
	}
	//RegisterWorld(SECTOR::SECTOR_0, ServerCore::MakeShared<TRWorldRoom>(SECTOR::SECTOR_0));
	//RegisterWorld(SECTOR::SECTOR_1, ServerCore::MakeShared<TRWorldRoom>(SECTOR::SECTOR_1));
	//RegisterWorld(SECTOR::SECTOR_2, ServerCore::MakeShared<TRWorldRoom>(SECTOR::SECTOR_2));
	//RegisterWorld(SECTOR::SECTOR_3, ServerCore::MakeShared<TRWorldRoom>(SECTOR::SECTOR_3));
	//RegisterWorld(SECTOR::SECTOR_4, ServerCore::MakeShared<TRWorldRoom>(SECTOR::SECTOR_4));
	//
	//
	//RegisterWorld(SECTOR::SECTOR_5, ServerCore::MakeShared<TRWorldRoom>(SECTOR::SECTOR_5));

	//m_pMainWorld = ServerCore::MakeShared<TRWorldChunk>();
	//
	//m_pMainWorld->Init();
	//m_pMainWorld = m_arrRoom[etoi(SECTOR::SECTOR_5)];
	//
	//m_pMainWorld->SetTRWorld(&m_pMainWorld->g_trWorld);
}

//void TRWorldMgr::RegisterWorld(const SECTOR worldID_, std::shared_ptr<TRWorldRoom> world_) noexcept
//{
//	world_->StartExecute(world_);
//
//	world_->Init();
//
//	m_mapWorld.emplace_no_return(worldID_, world_);
//
//	world_->Update();
//	
//	world_->SetTRWorld(&world_->g_trWorld);
//
//	m_arrRoom[etoi(worldID_)] = std::move(world_);
//
//}

//const float TRWorldMgr::GetSectorDT(const SECTOR eType_) const noexcept
//{
//	return (*m_mapWorld.find(eType_))->GetSectorDT();
//}
