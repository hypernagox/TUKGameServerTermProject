#pragma once
#include <stack>
#include <random>
#include "RefCountable.h"

namespace ServerCore
{
	extern thread_local uint32 LThreadId;
	extern thread_local std::stack<int32> LLockStack;
	extern thread_local S_ptr<class SendBufferChunk> LSendBufferChunk;
	extern thread_local uint64				LEndTickCount;
	extern thread_local class TaskQueueable* LCurTaskQueue;
	extern thread_local uint64				LCurHandleSessionID;
	extern thread_local uint32_t			LRandSeed;
}

extern std::mt19937 g_RandEngine;