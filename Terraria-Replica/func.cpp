#include "pch.h"
#include "func.h"
#include "NetworkMgr.h"
#include "ServerSession.h"

ServerSession* GetServerSession()noexcept {
    static const auto pServerSession = static_cast<ServerSession* const>(NetMgr(NetworkMgr)->GetSession().get());
    return pServerSession;
}