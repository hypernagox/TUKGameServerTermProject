#include "pch.h"
#include "ObjectFactory.h"
#include "Object.h"
#include "PhysicsComponent.h"
#include "CollisionHandler.h"
#include "TRWorldMgr.h"
#include "TRWorldRoom.h"
#include "BroadCaster.h"

S_ptr<Object> ObjectFactory::CreatePlayer(ClientSession* const pSession_, const uint64 id)
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
	
	player->AddComponent(MakeShared<SessionObject>(pSession_, player.get()));

	player->AddComponent(MakeShared<RigidBody>(player.get()));

	
	return player;
}

S_ptr<Object> ObjectFactory::CreateDropItem(const uint64 id, std::string_view strName, const Vec2 vPos, ServerCore::SessionManageable* const pRoom_)
{
	auto drop_item = MakePoolShared<Object>(id, GROUP_TYPE::DROP_ITEM, strName);

	auto collider = MakeShared<Collider>(drop_item.get());

	//auto collisionHandler = MakeShared<PlayerCollisionHandler>();
	//
	//
	//collider->SetCollisionHandler(std::move(collisionHandler));

	drop_item->AddComponent(std::move(collider));

	auto rigidbody = MakeShared<RigidBody>(drop_item.get());

	
	drop_item->SetPos(vPos);
	drop_item->SetWillPos(vPos);
	drop_item->SetScale(Vec2{ 16.f,16.f });
	
	auto broadcaster = MakeShared<MoveBroadCaster>(drop_item.get(), pRoom_);
	
	drop_item->AddComponent(std::move(broadcaster));

	const auto rigid = drop_item->AddComponent(MakeShared<RigidBody>(drop_item.get()));
	
	rigid->AddVelocity(Vec2::down * 240.f);

	return drop_item;
}
