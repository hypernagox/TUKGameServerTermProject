#include "pch.h"
#include "ServerCorePch.h"
#include "c2s_PacketHandler.h"
#include "Service.h"
#include "ClientSession.h"
#include "IocpCore.h"
#include "TRWorldMgr.h"
#include "TRWorldRoom.h"
#include "TRWorldChunk.h"
#include "BroadCaster.h"
#include "Object.h"

using namespace ServerCore;

constexpr int VIEW_RANGE = 512 + 128;
//constexpr int VIEW_RANGE = 128/4;
int main()
{
	Mgr(CoreGlobal)->Init();
	ServerCore::c2s_PacketHandler::Init();
	
	TRMgr(TRWorldMgr)->Init();
	TRMgr(TRWorldMgr)->GetWorldRoom(SECTOR::SECTOR_0)->GetTRWorld()->CreateWorld(10);

	TRMgr(TRWorldMgr)->GetMainWorld()->GetWorldSector(Vec2{})->g_trWorldMain.CreateWorld(10 + 2024 * 2024);

	ServerCore::MoveBroadcaster::RegisterHuristicFunc([](const IocpEntity* const a, const IocpEntity* const b)
		{
			const auto aa = ((Object*)a->GetContentsEntity())->GetPos();
			const auto bb = ((Object*)b->GetContentsEntity())->GetPos();

			const int dist = (int)((aa.x - bb.x) * (aa.x - bb.x) + (aa.y - bb.y) * (aa.y - bb.y));
			

			return dist <= VIEW_RANGE * VIEW_RANGE;
		});

	ServerCore::MoveBroadcaster::RegisterInPacketFunc([](const S_ptr<IocpEntity>& p)
		{
			Protocol::s2c_APPEAR_NEW_OBJECT pkt;
			*pkt.mutable_appear_pos() = ((Object*)p->GetContentsEntity())->GetPos();
			
			pkt.set_is_player(p->IsSession());
			pkt.set_sector(0);
			pkt.set_obj_id(p->GetObjectID());
		
			pkt.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
		
			return c2s_PacketHandler::MakeSendBuffer(pkt);
		});

	ServerCore::MoveBroadcaster::RegisterOutPacketFunc([](const S_ptr<IocpEntity>& p)
		{
			Protocol::s2c_LEAVE_OBJECT pkt;
			pkt.set_is_player(p->IsSession());
			pkt.set_obj_id(p->GetObjectID());
			//pkt.set_obj_id(0);
			pkt.set_sector(0);
			return c2s_PacketHandler::MakeSendBuffer(pkt);
		});
	
	const auto pServerService = ServerCore::MakeShared<ServerCore::ServerService>
		(
			  Mgr(CoreGlobal)->GetIocpCore()
			, ServerCore::NetAddress{ L"0.0.0.0",7777 }
			, ServerCore::MakePoolShared<ClientSession>
			, 100
		);

	ASSERT_CRASH(pServerService->Start());

	Mgr(ThreadMgr)->Launch(pServerService);

	google::protobuf::ShutdownProtobufLibrary();
}