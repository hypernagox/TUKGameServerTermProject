#pragma once

class Object;
class ServerCore::SessionManageable;
class ClientSession;
class Item;

struct ObjectBuilder
{
	ClientSession* session;
	String str;
	uint64_t id;
	Vec2 pos;
	float dir;
	ServerCore::SessionManageable* sector;

	void ConvertProtoVec2AndSet(const Protocol::Vec2 v)noexcept { pos = ::ToOriginVec2(v); }
};

class ObjectFactory
{
public:
	ObjectFactory() = delete;
	~ObjectFactory() = delete;
public:

	////////////////////////////////////
	// 레지스터 캐시를 잊지말자 
	///////////////////////////////////

	static S_ptr<Object> CreatePlayer(ObjectBuilder& bulider);

	static S_ptr<Object> CreateDropItem(ObjectBuilder& bulider);

	static S_ptr<Object> CreateMissle(ObjectBuilder& bulider);

	static S_ptr<Object> CreateMonster(ObjectBuilder& bulider);
};

struct ItemBuilder
{
	String itemName;
	Object* owner;
};

class ItemFactory
{
public:
	ItemFactory() = delete;
	~ItemFactory() = delete;
public:

	////////////////////////////////////
	// 레지스터 캐시를 잊지말자 
	///////////////////////////////////

	static S_ptr<Item> CreateCountableItem(ItemBuilder& b);
};