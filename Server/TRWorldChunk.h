#pragma once
#include "ServerCorePch.h"
#include "SessionManageable.h"
#include "TRWorld.h"

class TRWorld;
class TRWorldRoom;
class ServerCore::IocpEntity;
class ServerCore::PacketSession;

class TRWorldChunk
	: public TRWorld
{
public:
	TRWorldChunk(const int seed) :TRWorld{ seed } {}
	~TRWorldChunk();
public:
	void Init();
public:
	const ServerCore::S_ptr<TRWorldRoom>& GetWorldSector(const Vec2 global_pos)const noexcept;
	const ServerCore::S_ptr<TRWorldRoom>& GetWorldSector(const S_ptr<ServerCore::IocpEntity>& pEntity)const noexcept;
	void BroadCastToWorld(const S_ptr<ServerCore::SendBuffer> pSendBuffer)const noexcept;
	void ImigrationWorldChunk(const S_ptr<ServerCore::PacketSession>& pSession_,const CHUNK eType_)noexcept;
private:
	ServerCore::S_ptr<TRWorldRoom> m_sectors[TR_WORLD_HEIGHT / SECTOR_HEIGHT][TR_WORLD_WIDTH / SECTOR_WIDTH];

	// 충돌처리용 몬스터 , 플레이어 , 아이템 전역 관리 컨테이너 필요
	// 그룹화해서 락을좀 최소화함
	// 큐 써야함
	// 
	// 인접섹터
	// 유저 + 아이템 + 몬스터
	// 부모 월드.

	// IocpEntity
	// SessionManageable 바꾸기
	// 브로드캐스터 바꾸기
	// 락바꾸기, 링크드해시 바꾸기
	// 
};

