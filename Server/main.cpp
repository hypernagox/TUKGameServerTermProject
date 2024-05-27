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
	//TRMgr(TRWorldMgr)->GetWorldRoom(SECTOR::SECTOR_0)->GetTRWorld()->CreateWorld(10);

	//TRMgr(TRWorldMgr)->GetStartWorld()->CreateWorld(10 + 2024 * 2024);
	TRMgr(TRWorldMgr)->GetWorldChunk(CHUNK::CHUNK_0)->CreateWorld(10 + 2024 * 2024);
	TRMgr(TRWorldMgr)->GetWorldChunk(CHUNK::CHUNK_1)->CreateWorld(10);

	ServerCore::MoveBroadcaster::RegisterHuristicFunc([](const IocpEntity* const a, const IocpEntity* const b)
		{
			if (!((Object*)a->GetContentsEntity().get())->IsValid() || !((Object*)b->GetContentsEntity().get())->IsValid())
				return false;
			const auto aa = ((Object*)a->GetContentsEntity().get())->GetPos();
			const auto bb = ((Object*)b->GetContentsEntity().get())->GetPos();

			const int dist = (int)((aa.x - bb.x) * (aa.x - bb.x) + (aa.y - bb.y) * (aa.y - bb.y));
			

			return dist <= VIEW_RANGE * VIEW_RANGE;
		});

	ServerCore::MoveBroadcaster::RegisterInPacketFunc([](const S_ptr<IocpEntity>& p)
		{
			Protocol::s2c_APPEAR_NEW_OBJECT pkt;
			*pkt.mutable_appear_pos() = ((Object*)p->GetContentsEntity().get())->GetPos();
			
			pkt.set_is_player(p->IsSession());
			pkt.set_sector(0);
			pkt.set_obj_id(p->GetObjectID());
			pkt.set_obj_name(((Object*)p->GetContentsEntity().get())->GetObjectName());
			pkt.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
			if (!p->IsSession())
			{
				
				if (pkt.obj_name() == "PLAYER")
				{
					auto m = p->GetContentsEntity();
					auto asaa = p->IsSession();
					int a = 10;
				}
			}
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