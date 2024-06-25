#include "pch.h"
#include "TRWorldChunk.h"
#include "TRWorld.h"
#include "TRWorldRoom.h"
#include "PacketSession.h"
#include "Object.h"
#include "ClientSession.h"
#include "TRWorldMgr.h"
#include "c2s_PacketHandler.h"

TRWorldChunk::~TRWorldChunk()
{
	for (auto& sector : m_sectors | std::views::join)
	{
		//sector->reset_cache_shared();
	}
}

void TRWorldChunk::Init()
{
	//register_cache_shared();
	uint16 count = 0;
	for (auto& sector : m_sectors | std::views::join)
	{
		sector = ServerCore::MakeShared<TRWorldRoom>(count++,this);
		
		sector->GetAdjSector8().reserve(9);
		sector->GetAdjSector4().reserve(5);

		//sector->StartExecute(sector);
		//sector->StartExecute();

		sector->Update();


		sector->Init();
	}

	constexpr static const int dx[9] = { 0,0, 1, 1, 1, 0, -1, -1, -1 };
	constexpr static const int dy[9] = { 0,-1, -1, 0, 1, 1, 1, 0, -1 };

	constexpr static const int dx4[5] = { 0,0,1,0,-1 };
	constexpr static const int dy4[5] = { 0,-1,0,1,0 };

	for (int i = 0; i < TR_WORLD_HEIGHT / SECTOR_HEIGHT; ++i)
	{
		for (int j = 0; j < TR_WORLD_WIDTH / SECTOR_WIDTH; ++j)
		{
			//m_sectors[i][j]->GetAdjSector().emplace_back(m_sectors[i][j].get());
			for (int k = 0; k < 9; ++k)
			{
				const int ny = i + dy[k];
				const int nx = j + dx[k];
				if (ny < 0 || nx < 0 || ny >= TR_WORLD_HEIGHT / SECTOR_HEIGHT || nx >= TR_WORLD_WIDTH / SECTOR_WIDTH)continue;
				m_sectors[i][j]->GetAdjSector8().emplace_back(m_sectors[ny][nx].get());
			}
			for (int k = 0; k < 5; ++k)
			{
				const int ny = i + dy4[k];
				const int nx = j + dx4[k];
				if (ny < 0 || nx < 0 || ny >= TR_WORLD_HEIGHT / SECTOR_HEIGHT || nx >= TR_WORLD_WIDTH / SECTOR_WIDTH)continue;
				m_sectors[i][j]->GetAdjSector4().emplace_back(m_sectors[ny][nx].get());
			}
			//m_sectors[i][j]->
		}

	}
}

const ServerCore::S_ptr<TRWorldRoom>& TRWorldChunk::GetWorldSector(const Vec2 global_pos) const noexcept
{
	const Vec2 world_pos = TRWorld::GlobalToWorld(global_pos);
	const int y = (int)std::ceil(world_pos.y);
	const int x = (int)std::ceil(world_pos.x);

	return m_sectors[std::clamp(y / SECTOR_HEIGHT,0,7)][x / SECTOR_WIDTH];
}

const ServerCore::S_ptr<TRWorldRoom>& TRWorldChunk::GetWorldSector(const S_ptr<ServerCore::IocpEntity>& pEntity) const noexcept
{
	return GetWorldSector(((Object*)pEntity->GetContentsEntity().get())->GetPos());
}

void TRWorldChunk::BroadCastToWorld(const S_ptr<ServerCore::SendBuffer> pSendBuffer) const noexcept
{
	for (const auto& sector : m_sectors | std::views::join)
	{
		sector->session_lock_shared();
		for (const auto pSession : sector->GetSessionList())
		{
			pSession->SendAsync(pSendBuffer);
		}
		sector->session_unlock_shared();
	}
}

void TRWorldChunk::ImigrationWorldChunk(const S_ptr<ServerCore::PacketSession>& pSession_, const CHUNK eType_) noexcept
{
	constexpr const int x = TRWorld::WORLD_WIDTH / 2;
	const auto& player = ((ClientSession*)pSession_.get())->GetPlayer();
	if (player)
	{
		const auto& next_sector = TRMgr(TRWorldMgr)->GetWorldChunk(eType_)->GetWorldSector(pSession_);
		//next_sector->GetWorldChunk()->TransmitTileRecord(pSession_);

		pSession_->GetCurSector()->ImmigrationEnqueue(next_sector, player->GetObjID());
		
		player->SetPos(TRWorld::WorldToGlobal(Vec2Int(x, GetTileMap()->GetTopYpos(x))) - Vec2(20.0f, 28.0f));

		Protocol::s2c_MOVE pkt;
		*pkt.mutable_obj_pos() = player->GetPos();
		pkt.set_obj_id(player->GetObjID());

		//pSession_ << pkt;
		pSession_->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
		Protocol::s2c_TRY_NEW_ROOM pkt2;
		//pSession_ << pkt2;
		pSession_->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt2));

	}

}
