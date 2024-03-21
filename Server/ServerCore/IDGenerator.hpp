#pragma once
#include "ServerCorePch.h"

class IDGenerator
{
public:
	IDGenerator() = delete;
	~IDGenerator() = delete;
public:
	static c_uint64 GenerateID()noexcept { return g_objectID.fetch_add(1, std::memory_order_acq_rel); }
private:
	static inline std::atomic<uint64> g_objectID = 1;
};