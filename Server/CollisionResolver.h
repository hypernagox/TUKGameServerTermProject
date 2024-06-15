#pragma once
#include "Component.h"

class CollisionResolver
	:public BaseComponent
{
public:
	CONSTRUCTOR_BASE_COMPONENT(CollisionResolver)
public:
	virtual void ResolveCollision(class Object* const other)noexcept = 0;
private:

};

class MonsterCollisionResolver
	:public CollisionResolver
{
public:
	MonsterCollisionResolver(Object* const owner)
		:CollisionResolver{ owner } {}
public:
	void ResolveCollision(class Object* const other)noexcept override;
};