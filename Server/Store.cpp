#include "pch.h"
#include "Store.h"

Store::Store()
{
}

Store::~Store()
{
}

void Store::Init() noexcept
{
    m_storeLua = luaL_newstate();
    luaL_openlibs(m_storeLua);
    if (luaL_dofile(m_storeLua, "store.lua") != LUA_OK)
    {
        std::cerr << "Failed to load Lua script: " << lua_tostring(m_storeLua, -1) << std::endl;
        lua_pop(m_storeLua, 1);
    }
}

const int Store::GetItemPrice(const std::string_view itemName) noexcept
{
    lua_getglobal(m_storeLua, "GetItemPrice");
    lua_pushstring(m_storeLua, itemName.data());

    if (lua_pcall(m_storeLua, 1, 1, 0) != LUA_OK)
    {
        std::cerr << "Error running function `GetItemPrice`: " << lua_tostring(m_storeLua, -1) << std::endl;
        lua_pop(m_storeLua, 1);
        return -1;
    }

    if (!lua_isnumber(m_storeLua, -1))
    {
        std::cerr << "Invalid price returned for item: " << itemName << std::endl;
        lua_pop(m_storeLua, 1);
        return -1;
    }

    const int price = (int)lua_tointeger(m_storeLua, -1);
    lua_pop(m_storeLua, 1);
    return price;
}
