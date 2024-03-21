#include "ServerCorePch.h"
#include "CoreGlobal.h"
#include "ThreadMgr.h"
#include "MemoryMgr.h"
#include "DeadLockProfiler.h"
#include "SocketUtils.h"
#include "SendBufferMgr.h"
#include "TaskTimerMgr.h"
#include "Logger.h"

namespace ServerCore
{
	CoreGlobal::CoreGlobal()
	{
		SocketUtils::Init();
		Mgr(Logger)->Init();
		Mgr(MemoryMgr)->Init();
		Mgr(ThreadMgr)->Init();
		Mgr(DeadLockProfiler)->Init();
		Mgr(TaskTimerMgr)->Init();
		Mgr(SendBufferMgr)->Init();
	}

	CoreGlobal::~CoreGlobal()
	{
		SocketUtils::Clear();
	}
}