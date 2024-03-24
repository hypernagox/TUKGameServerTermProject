#include "pch.h"
#include "ObjectFactory.h"
#include "Object.h"
#include "PhysicsComponent.h"
#include "CollisionHandler.h"

S_ptr<Object> ObjectFactory::CreatePlayer(const uint64 id)
{
	auto player = MakePoolShared<Object>(id, GROUP_TYPE::PLAYER, "PLAYER");
	
	auto collider = MakeShared<Collider>(player.get());

	auto collisionHandler = MakeShared<PlayerCollisionHandler>();

	collider->SetCollisionHandler(std::move(collisionHandler));

	player->AddComponent(std::move(collider));

	auto rigidbody = MakeShared<RigidBody>(player.get());

	return player;
}
