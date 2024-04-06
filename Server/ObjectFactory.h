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

	////////////////////////////////////
	// 레지스터 캐시를 잊지말자 
	///////////////////////////////////

	static S_ptr<Object> CreatePlayer(ClientSession* const pSession_, const uint64 id);

	static S_ptr<Object> CreateDropItem(const uint64 id, std::string_view strName,const Vec2 vPos, ServerCore::SessionManageable* const pRoom_);

	static S_ptr<Object> CreateMissle(const Vec2 vPos, ServerCore::SessionManageable* const pRoom_);

};

