#include "../pch.h"
#include "c2s_PacketHandler.h"
#include "../ClientSession.h"

namespace ServerCore
{
    const bool Handle_Invalid(const S_ptr<PacketSession>& pSession_, BYTE* const pBuff_, c_int32 len_)
    {
        return false;
    }

    const bool Handle_c2s_LOGIN(const S_ptr<PacketSession>& pSession_, const Protocol::c2s_LOGIN& pkt_)
    {
        Protocol::s2c_LOGIN pkt;
        pkt.set_seed(10);

        pSession_ << pkt;

        return true;
    }

}

