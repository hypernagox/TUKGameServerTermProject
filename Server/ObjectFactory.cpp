#include "pch.h"
#include "ObjectFactory.h"
#include "Object.h"
#include "PhysicsComponent.h"
#include "CollisionHandler.h"
#include "TRWorldMgr.h"
#include "TRWorldRoom.h"
#include "BroadCaster.h"
#include "ItemComponent.h"
#include "ClientSession.h"
#include "IDGenerator.hpp"
#include "AIComponent.h"
#include "TRWorldChunk.h"
#include "NPC.h"
#include "Inventory.h"
#include "TimerNPC.h"
#include "CollisionResolver.h"
#include "Status.h"

S_ptr<Object> ObjectFactory::CreatePlayer(ObjectBuilder& builder)
{
	const auto pSession_ = builder.session;

	auto player = MakePoolShared<Object>(pSession_->SharedFromThis<ServerCore::IocpEntity>(), GROUP_TYPE::PLAYER, "PLAYER");
	
	pSession_->SetEntity(player);

	auto collider = MakeShared<Collider>(player.get());

	auto collisionHandler = MakeShared<PlayerCollisionHandler>();

	collider->SetCollisionHandler(std::move(collisionHandler));

	player->AddComponent(std::move(collider));

	auto rigidbody = MakeShared<RigidBody>(player.get());

	constexpr const int x = TRWorld::WORLD_WIDTH / 2;

	const auto tile_map = TRMgr(TRWorldMgr)->GetStartWorld()->GetTileMap();
	player->SetPos(TRWorld::WorldToGlobal(Vec2Int(x, tile_map->GetTopYpos(x))) - Vec2(20.0f, 28.0f) + Vec2{0.f,+1000.f});
	player->SetScale(Vec2{ 40.f, 56.f });
	
	//player->AddComponent(MakeShared<SessionObject>(pSession_->SharedCastThis<ClientSession>(), player.get()));

	player->AddComponent(MakeShared<RigidBody>(player.get()));

	//player->register_cache_shared_core(player);

	//auto broadcaster = MakeShared<MoveBroadCaster>(player.get(), TRMgr(TRWorldMgr)->GetWorldRoom(SECTOR::SECTOR_5).get());
	
	//auto broadcaster = MakeShared<MoveBroadCaster>(player.get(), TRMgr(TRWorldMgr)->GetStartWorld()->GetWorldSector(player->GetPos()).get());
	//
	//player->AddComponent(std::move(broadcaster));

	auto input_handler = MakeShared<KeyInputHandler>(player.get());

	input_handler->RegisterKeyHandleFunc('A', [](Object* const pObj,const KeyInputHandler::KEY_STATE eState_)
		{
			const auto pRigid = pObj->GetComp<RigidBody>();
			switch (eState_)
			{
			case KeyInputHandler::KEY_STATE::KEY_TAP:
				pRigid->AddVelocity(Vec2{ -20.0f, 0.0f });
				break;
			case KeyInputHandler::KEY_STATE::KEY_HOLD:
				pRigid->AddVelocity(Vec2{ -20.0f, 0.0f });
				break;
			case KeyInputHandler::KEY_STATE::KEY_AWAY:
				//pRigid->AddVelocity(Vec2{ -20.0f, 0.0f });
				break;
			default:
				break;
			}
		});

	input_handler->RegisterKeyHandleFunc('D', [](Object* const pObj, const KeyInputHandler::KEY_STATE eState_)
		{
			const auto pRigid = pObj->GetComp<RigidBody>();
			switch (eState_)
			{
			case KeyInputHandler::KEY_STATE::KEY_TAP:
				pRigid->AddVelocity(Vec2{ 20.0f, 0.0f });
				break;
			case KeyInputHandler::KEY_STATE::KEY_HOLD:
				pRigid->AddVelocity(Vec2{ 20.0f, 0.0f });
				break;
			case KeyInputHandler::KEY_STATE::KEY_AWAY:
				break;
			default:
				break;
			}
		});

	input_handler->RegisterKeyHandleFunc(VK_SPACE, [](Object* const pObj, const KeyInputHandler::KEY_STATE eState_)
		{
			const auto pRigid = pObj->GetComp<RigidBody>();
			switch (eState_)
			{
			case KeyInputHandler::KEY_STATE::KEY_TAP:
				pRigid->SetIsGround(false);
				pRigid->AddVelocity(Vec2::down * 720.0f);
				//pRigid->SetForce(Vec2{ 0, -1000.0f });
				break;
			case KeyInputHandler::KEY_STATE::KEY_HOLD:
				break;
			case KeyInputHandler::KEY_STATE::KEY_AWAY:
				break;
			default:
				break;
			}
		});

	auto inventory = MakeShared<Inventory>(player.get());
	//inventory->SetCurItem("Item_Sword.png");

	player->AddBaseComponent(inventory);
	
	
	//auto& item = inventory->AddItem("Item_Pickaxe.png", MakeShared<Attackable>(player.get()));
	//
	//((Attackable*)item.get())->SetAtk(100);
	//
	//inventory->SetCurItem("Item_Pickaxe.png");
	//
	player->AddComponent(std::move(input_handler));
	//
	//auto& item2 = inventory->AddItem("Item_Sword.png", MakeShared<Attackable>(player.get()));
	//((Attackable*)item2.get())->SetAtk(200);
	//
	//auto& item3 = inventory->AddItem("Item_Hammer.png", MakeShared<Attackable>(player.get()));
	//((Attackable*)item3.get())->SetAtk(300);
	//
	//auto& item4= inventory->AddItem("Item_Arrow.png", MakeShared<RangeAttack>(player.get()));
	//{
	//	auto r = (RangeAttack*)item4.get();
	//	r->SetAtk(200);
	//	r->SetSpeed(100.f);
	//	r->SetRange(800.f);
	//}

	player->AddBaseComponent<Status>();

	//auto& item5 = inventory->AddItem("Item_28.png", MakeShared<HPPotion>(player.get()));
	//Item_28.png
	return player;
}

S_ptr<Object> ObjectFactory::CreateDropItem(ObjectBuilder& builder)
{
	//const uint64 id, std::string_view strName, const Vec2 vPos, ServerCore::SessionManageable* const pRoom_
	const auto id = builder.id;
	auto strName = std::move(builder.str);
	const auto vPos = builder.pos;
	const auto pRoom_ = builder.sector;

	auto npc = MakePoolShared<NPC>();
	//npc->register_cache_shared_core(npc);
	auto drop_item = MakeShared<Object>((npc), GROUP_TYPE::DROP_ITEM, std::move(strName));

	auto collider = MakeShared<Collider>(drop_item.get());

	drop_item->GetIocpEntity()->SetEntity(drop_item);

	auto collisionHandler = MakeShared<PlayerCollisionHandler>();
	
	collider->SetCollisionHandler(std::move(collisionHandler));

	drop_item->SetPos(vPos);
	drop_item->SetWillPos(vPos);
	drop_item->SetScale(Vec2{ 16.f,16.f });
	collider->Update(0.f);

	drop_item->AddComponent(std::move(collider));

	
	
	//auto broadcaster = MakeShared<MoveBroadCaster>(drop_item.get(), pRoom_);
	////
	//drop_item->AddComponent(std::move(broadcaster));

	//const auto rigid = drop_item->AddComponent(MakeShared<RigidBody>(drop_item.get()));
	//
	//rigid->AddVelocity(Vec2::down * 240.f);

	//drop_item->register_cache_shared_core(drop_item);

	auto acq = MakeShared<AcquireItem>(drop_item.get());

	drop_item->AddComponent(std::move(acq));

	return drop_item;
}

S_ptr<Object> ObjectFactory::CreateMissle(ObjectBuilder& bulider)
{
	//const Vec2 vPos, ServerCore::SessionManageable* const pRoom_, const float dir_
	const Vec2 vPos = bulider.pos;
	const auto pRoom_ = bulider.sector;
	const auto dir_ = bulider.dir;

	auto missle = MakePoolShared<Object>(MakePoolShared<NPC>(), GROUP_TYPE::PROJ_PLAYER, "MISSLE");
	missle->SetPos(vPos);
	missle->SetScale(Vec2{ 32.f,32.f } * 2.f);

	auto rigidbody = MakeShared<RigidBody>(missle.get());

	rigidbody->SetGravity(false);

	rigidbody->SetFriction(0.f);

	rigidbody->SetLimitBreak();

	missle->AddComponent(std::move(rigidbody));

	auto collider = MakeShared<Collider>(missle.get());

	auto collisionHandler = MakeShared<PlayerCollisionHandler>();

	collider->SetCollisionHandler(std::move(collisionHandler));

	missle->AddComponent(std::move(collider));
	
	//auto broadcaster = MakeShared<MoveBroadCaster>(missle.get(), pRoom_);
	//
	//missle->AddComponent(std::move(broadcaster));

	//auto atk = MakeShared<Attackable>(missle.get(),dir_);
	
	//missle->AddComponent(std::move(atk));

	//missle->register_cache_shared_core(missle);

	return missle;
}

S_ptr<Object> ObjectFactory::CreateMonster(ObjectBuilder& builder)
{
	const auto id = builder.id;
	const auto vPos = builder.pos;
	const int sector = 0;
	//const uint64 id, const Vec2 vPos, const int sector

	auto npc = MakePoolShared<TimerNPC>();
	
	auto mon = MakePoolShared<Object>(npc, GROUP_TYPE::MONSTER, "MONSTER");
	npc->SetEntity(mon);
	
	npc->InitTimer(npc, 1000);

	auto collider = MakeShared<Collider>(mon.get());

	//auto collisionHandler = MakeShared<PlayerCollisionHandler>();
	//
	//
	//collider->SetCollisionHandler(std::move(collisionHandler));

	mon->AddComponent(std::move(collider));

	
	mon->SetPos(vPos);
	mon->SetWillPos(vPos);
	mon->SetScale(Vec2(24.0f, 48.0f));

	//auto broadcaster = MakeShared<MoveBroadCaster>(mon.get(), TRMgr(TRWorldMgr)->GetWorldChunk((CHUNK)sector)->GetWorldSector(vPos).get());
	////
	//mon->AddComponent(std::move(broadcaster));

	const auto rigid = mon->AddComponent(MakeShared<RigidBody>(mon.get()));

	rigid->AddVelocity(Vec2::down * 240.f);

	//mon->register_cache_shared_core(mon);


	auto astar = MakeShared<Astar>(mon.get());
	
	npc->AddAIComponent(astar);
	
	mon->AddBaseComponent<MonsterCollisionResolver>();

	//astar->InitTimer(astar, 1000);
	// 나중에 몬스터가 진짜 맵에 들어왔을때 타이머가 작동하도록 조정필요

	//mon->AddComponent(std::move(astar));
	auto hp = mon->AddBaseComponent<HP>();
	hp->SetMaxHP(500);
	hp->SetHP(500);

	return mon;
}

S_ptr<Item> ItemFactory::CreateCountableItem(ItemBuilder& b)
{
	auto item = MakeShared<CountableItem>(b.owner);
	item->SetItemName(std::move(b.itemName));
	return item;
}
