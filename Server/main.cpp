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
#include "TimerObject.h"
#include "Store.h"
#include "DBMgr.h"

using namespace ServerCore;

//constexpr int VIEW_RANGE = 128/4;
int main()
{
	TRMgr(Store)->Init();
	Mgr(CoreGlobal)->Init();
	ServerCore::c2s_PacketHandler::Init();

	std::wstring inputIP;
	std::wcout << L"Input DB IP Address: ";
	std::wcin >> inputIP;
	
	NAGOX_ASSERT(Mgr(DBMgr)->Connect(L"DSN=GS_TERM_2024;UID=Nagox;PWD=gurdl3;"));
	NAGOX_ASSERT(Mgr(DBMgr)->ConnectQueryServer(inputIP, 8888));
	//NAGOX_ASSERT(Mgr(DBMgr)->Connect(L"DSN=GS_TERM_2024;UID=Nagox;PWD=gurdl3;"));
	//NAGOX_ASSERT(Mgr(DBMgr)->ConnectQueryServer(L"127.0.0.8", 8888));
	TRMgr(TRWorldMgr)->Init();
	//TRMgr(TRWorldMgr)->GetWorldRoom(SECTOR::SECTOR_0)->GetTRWorld()->CreateWorld(10);

	//TRMgr(TRWorldMgr)->GetStartWorld()->CreateWorld(10 + 2024 * 2024);
	int trseed = 10;
	for (int i = 0; i < etoi(CHUNK::END); ++i)
	{
		if (i == 3)
			trseed = 123456789;
		if (i >= 4)
			trseed = 987654321 + i;
		TRMgr(TRWorldMgr)->GetWorldChunk((CHUNK)i)->CreateWorld(trseed + (int)std::pow(2024, i) * (0 != i));
		TRMgr(TRWorldMgr)->GetWorldChunk((CHUNK)i)->InitMonsters((CHUNK)i);
	}
	//TRMgr(TRWorldMgr)->GetStartWorld()->InitMonsters();
	//TRMgr(TRWorldMgr)->GetWorldChunk(CHUNK::CHUNK_0)->CreateWorld(10);
	//TRMgr(TRWorldMgr)->GetWorldChunk(CHUNK::CHUNK_1)->CreateWorld(10 + 2024 * 2024);

	ServerCore::MoveBroadcaster::RegisterHuristicFunc([](const IocpEntity* const a, const IocpEntity* const b)
		{
			const auto aobj = a->GetContentsEntity()->ObjectCast();
			const auto bobj = b->GetContentsEntity()->ObjectCast();
			if (!aobj->IsValid() || !bobj->IsValid())
				return false;
			const auto aa = aobj->GetPos();
			const auto bb = bobj->GetPos();

			const int dist = (int)((aa.x - bb.x) * (aa.x - bb.x) + (aa.y - bb.y) * (aa.y - bb.y));
			const bool flag = dist <= VIEW_RANGE * VIEW_RANGE;
			const auto types = (bool)a->GetObjectType() + (bool)b->GetObjectType();
			
			if (flag && 1 == types)
			{
				const auto aa = a->ObjectCast<TimerObject>(2);
				const auto bb = b->ObjectCast<TimerObject>(2);
				if (aa) 
				{
					aa->ExecuteTimer(b);
				}
				else if (bb)
				{
					bb->ExecuteTimer(a);
				}
			}
			return flag;
			
		});

	ServerCore::MoveBroadcaster::RegisterInPacketFunc([](const S_ptr<IocpEntity>& p)
		{
			Protocol::s2c_APPEAR_NEW_OBJECT pkt;
			*pkt.mutable_appear_pos() = ((Object*)p->GetContentsEntity().get())->GetPos();
			
			pkt.set_is_player(p->IsSession());
			pkt.set_obj_id(p->GetObjectID());
			if (const auto s = (ClientSession*)p->IsSession())
				pkt.set_obj_name(s->GetName());
			else
				pkt.set_obj_name(((Object*)p->GetContentsEntity().get())->GetObjectName());

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