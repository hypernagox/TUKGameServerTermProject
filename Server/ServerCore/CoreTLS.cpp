#include "ServerCorePch.h"
#include "CoreTLS.h"

namespace ServerCore
{
	thread_local uint32 LThreadId = 1;
	thread_local std::stack<int32> LLockStack = {};
	thread_local S_ptr<class SendBufferChunk> LSendBufferChunk = {};
	thread_local uint64				LEndTickCount = 0;
	thread_local class TaskQueueable* LCurTaskQueue = nullptr;
	thread_local uint64				LCurHandleSessionID = 0;
}