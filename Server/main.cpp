#include "pch.h"
#include "ServerCorePch.h"
#include "c2s_PacketHandler.h"
#include "Service.h"
#include "ClientSession.h"
#include "IocpCore.h"
#include "TRWorldMgr.h"
#include "TRWorldRoom.h"

int main()
{
	Mgr(CoreGlobal)->Init();
	ServerCore::c2s_PacketHandler::Init();
	
	TRMgr(TRWorldMgr)->Init();
	TRMgr(TRWorldMgr)->GetWorldRoom(SECTOR::SECTOR_0)->GetTRWorld()->CreateWorld(10);

	const auto pServerService = ServerCore::MakeShared<ServerCore::ServerService>
		(
			  ServerCore::MakeShared<ServerCore::IocpCore>()
			, ServerCore::NetAddress{ L"0.0.0.0",7777 }
			, ServerCore::MakePoolShared<ClientSession>
			, 100
		);

	ASSERT_CRASH(pServerService->Start());

	Mgr(ThreadMgr)->Launch(pServerService);

	google::protobuf::ShutdownProtobufLibrary();
}