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

	// �浹ó���� ���� , �÷��̾� , ������ ���� ���� �����̳� �ʿ�
	// �׷�ȭ�ؼ� ������ �ּ�ȭ��
	// ť �����
	// 
	// ��������
	// ���� + ������ + ����
	// �θ� ����.

	// IocpEntity
	// SessionManageable �ٲٱ�
	// ��ε�ĳ���� �ٲٱ�
	// ���ٲٱ�, ��ũ���ؽ� �ٲٱ�
	// 
};

