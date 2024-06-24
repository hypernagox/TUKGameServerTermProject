#pragma once
#include "Component.h"

class Attackable;

class CollisionResolver
	:public BaseComponent
{
public:
	CONSTRUCTOR_BASE_COMPONENT(CollisionResolver)
public:
	virtual void ResolveCollision(Object* const other)noexcept {}
	virtual void ResolveCollision(Object* const other,Attackable* const atk)noexcept{}
private:
};

class MonsterCollisionResolver
	:public CollisionResolver
{
public:
	MonsterCollisionResolver(Object* const owner)
		:CollisionResolver{ owner } {}
public:
	void ResolveCollision(class Object* const other, Attackable* const atk)noexcept override;
};