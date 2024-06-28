#include "ServerCorePch.h"
#include "RefCountable.h"
#include "Session.h"

namespace ServerCore
{
    void RefCountable::DecRef() const noexcept
	{
        const uint64_t old_value = m_refCount.fetch_sub(1ULL << 47, std::memory_order_acq_rel);
        if (0 == ((old_value >> 47) - 1))
        {
            if (const DeleterFunc deleter = reinterpret_cast<const DeleterFunc>((old_value & ((1ULL << 47) - 1)) << 1))
                deleter(const_cast<RefCountable* const>(this));
            else
                xdelete<RefCountable>(const_cast<RefCountable* const>(this));
        }
	}
}
