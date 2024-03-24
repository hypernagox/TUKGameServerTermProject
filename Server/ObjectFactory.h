#pragma once

class Object;

class ObjectFactory
{
public:
	ObjectFactory() = delete;
	~ObjectFactory() = delete;
public:

	static S_ptr<Object> CreatePlayer(const uint64 id);

	static S_ptr<Object> CreateDropItem(const uint64 id, std::string_view strName);
};

