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
