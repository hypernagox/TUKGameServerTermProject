#pragma once

class Object;
class ServerCore::SessionManageable;
class ClientSession;

class ObjectFactory
{
public:
	ObjectFactory() = delete;
	~ObjectFactory() = delete;
public:

	static S_ptr<Object> CreatePlayer(ClientSession* const pSession_, const uint64 id);

	static S_ptr<Object> CreateDropItem(const uint64 id, std::string_view strName,const Vec2 vPos, ServerCore::SessionManageable* const pRoom_);

};

