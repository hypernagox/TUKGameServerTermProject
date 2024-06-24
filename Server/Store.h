#pragma once
#include "Lua/lua.hpp"

class Store
	:public ServerCore::Singleton<Store>
{
	friend class Singleton;
	Store();
	~Store();
public:
	void Init()noexcept override;
	const int GetItemPrice(const std::string_view itemName)noexcept;
private:
	lua_State* m_storeLua;
};

