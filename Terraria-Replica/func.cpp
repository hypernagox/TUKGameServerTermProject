#include "pch.h"
#include "func.h"
#include "NetworkMgr.h"
#include "ServerSession.h"

ServerSession* const GetServerSession()noexcept {
    static const auto pServerSession = static_cast<ServerSession* const>(NetMgr(NetworkMgr)->GetSession().get());
    return pServerSession;
}

std::string WideToUtf8(std::wstring_view wstr) noexcept
{
    const int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
    return strTo;
}

std::wstring Utf8ToWide(std::string_view utf8Str)noexcept
{
    const int charsNeeded = MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), -1, NULL, 0);
    std::wstring wideStr(charsNeeded, 0);
    const int charsConverted = MultiByteToWideChar(CP_UTF8, 0, utf8Str.data(), -1, &wideStr[0], charsNeeded);
    wideStr.pop_back();
    return wideStr;
}