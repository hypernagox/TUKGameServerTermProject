#include "pch.h"
#include "ObjectFactory.h"
#include "Object.h"
#include "PhysicsComponent.h"
#include "CollisionHandler.h"
#include "TRWorldMgr.h"
#include "TRWorldRoom.h"

S_ptr<Object> ObjectFactory::CreatePlayer(const uint64 id)
{
	auto player = MakePoolShared<Object>(id, GROUP_TYPE::PLAYER, "PLAYER");
	
	auto collider = MakeShared<Collider>(player.get());

	auto collisionHandler = MakeShared<PlayerCollisionHandler>();

	collider->SetCollisionHandler(std::move(collisionHandler));

	player->AddComponent(std::move(collider));

	auto rigidbody = MakeShared<RigidBody>(player.get());

	constexpr const int x = TRWorld::WORLD_WIDTH / 2;

	const auto tile_map = TRMgr(TRWorldMgr)->GetWorldRoom(SECTOR::SECTOR_0)->GetTileMap();
	player->SetPos(TRWorld::WorldToGlobal(Vec2Int(x, tile_map->GetTopYpos(x))) - Vec2(20.0f, 28.0f));
	player->SetScale(Vec2{ 40.f, 56.f });
	

	return player;
}

S_ptr<Object> ObjectFactory::CreateDropItem(const uint64 id, std::string_view strName)
{
	return S_ptr<Object>();
}
