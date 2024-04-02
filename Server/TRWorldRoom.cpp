#include "pch.h"
#include "TRWorldRoom.h"
#include "Object.h"
#include "PhysicsComponent.h"
#include "TRTileMap.h"
#include "ThreadMgr.h"
#include "ItemComponent.h"
#include "ClientSession.h"
#include "c2s_PacketHandler.h"
#include "ThreadMgr.h"

TRWorldRoom::TRWorldRoom(const SECTOR sector_)
	:SessionManageable{ static_cast<uint16>(sector_) }
{
}

TRWorldRoom::~TRWorldRoom()
{
}

void TRWorldRoom::Update(const uint64 tick_ms)
{
	const uint64 threadID = Mgr(ThreadMgr)->GetCurThreadID() - 1;

	m_timer.Update();

	const float dt = m_timer.GetDT();

	for (auto& obj_list : m_worldObjectList[threadID])
	{
		const auto end_iter = obj_list.cend();
		for (auto iter = obj_list.cbegin(); iter != end_iter; )
		{
			const auto obj = (*iter);
			if (obj->IsValid())
			{
				obj->Update(dt);
				m_vecCollisionTask.emplace_back(ServerCore::PoolNew<ServerCore::Task>(
					[this, obj = obj->SharedCastThis<Object>()]()mutable noexcept {
						this->UpdateTileCollisionForTick(std::move(obj));
						//m_jobCount.fetch_sub(1, std::memory_order_release);
					}
				));
				++iter;
			}
			else
			{
				obj->reset_cache_shared(*this);
				iter = obj_list.EraseItemAndGetIter(obj->GetObjID());
			}
		}
	}
	
	UpdateWorldCollision();

	//TickTileCollision();

	if (const auto num = m_vecCollisionTask.size())
	{
		Mgr(ThreadMgr)->EnqueueGlobalTaskBulk(m_vecCollisionTask.data(), num);
		//m_jobCount.fetch_add(static_cast<int>(num), std::memory_order_acq_rel);
		m_vecCollisionTask.clear();
		//while (0 != m_jobCount.load(std::memory_order_relaxed)) { std::this_thread::yield(); };
	}

	//for (const auto& obj_list : m_worldObjectList)
	//{
	//	for (const auto obj : obj_list.GetItemListRef())
	//	{
	//		obj->PostUpdate(dt);
	//	}
	//}

	EnqueueAsyncTimer(tick_ms, &TRWorldRoom::Update, uint64{ tick_ms });
}

void TRWorldRoom::Init()
{
	//EnqueueAsyncTimer(5000, &TRWorldRoom::Update, uint64{ 30 });

	//RegisterGroup(GROUP_TYPE::PLAYER, GROUP_TYPE::DROP_ITEM);
}

void TRWorldRoom::AddObjectEnqueue(const GROUP_TYPE eType_, S_ptr<Object> pObj_)
{
	const uint64 threadID = Mgr(ThreadMgr)->GetCurThreadID() - 1;
	const uint64 objID = pObj_->GetObjID();
	m_worldObjectList[threadID][etoi(eType_)].AddItem(objID, pObj_);
	const auto pIsSession = pObj_->GetComp("SESSIONOBJECT");
	if (pIsSession)
	{
		g_allPlayers[threadID].AddItem(objID, pIsSession->Cast<SessionObject>()->GetSession());
	}
	const auto& pSession = pObj_->GetComp("SESSIONOBJECT")->Cast<SessionObject>()->GetSession();
	EnqueueAsync(&TRWorldRoom::AddObject, GROUP_TYPE{ eType_ }, std::move(pObj_), uint64{ threadID }, bool(pIsSession));
}

//void TRWorldRoom::DeleteObjectEnqueue(const GROUP_TYPE eType_, const uint64 objID_)
//{
//	const uint64 threadID = Mgr(ThreadMgr)->GetCurThreadID();
//	m_worldObjectList[threadID][etoi(eType_)].EraseItem(objID_);
//
//	EnqueueAsync(&TRWorldRoom::DeleteObject, GROUP_TYPE{ eType_ }, uint64{ objID_ }, uint64{ threadID });
//}

void TRWorldRoom::LeavePlayer(const uint64 playerID) noexcept
{
	const uint64 thID = Mgr(ThreadMgr)->GetCurThreadID() - 1;
	const auto player = m_worldObjectList[thID][etoi(GROUP_TYPE::PLAYER)].ExtractItem(playerID);
	if (player)
	{
		const auto& pSession = player->GetComp("SESSIONOBJECT")->Cast<SessionObject>()->GetSession();

		for (const auto others : GetSessionList())
		{
			Protocol::s2c_LEAVE_OBJECT pkt;
			pkt.set_is_player(true);
			pkt.set_obj_id(others->GetSessionID());
			pkt.set_sector(GetRoomID());
			pSession << pkt;
		}
	}
	for (int i = 0; i < CONTAINER_SIZE; ++i)
	{
		if (thID == i)continue;
		m_worldObjectList[i][etoi(GROUP_TYPE::PLAYER)].EraseItem(playerID);
	}
}

void TRWorldRoom::AddObject(const GROUP_TYPE eType_, S_ptr<Object> pObj_, const uint64 exceptThreadID, const bool bIsSession)
{
	const auto obj_id = pObj_->GetObjID();
	if (bIsSession)
	{
		const auto& pSession = pObj_->GetComp("SESSIONOBJECT")->Cast<SessionObject>()->GetSession();
		for (int i = 0; i < CONTAINER_SIZE; ++i)
		{
			if (exceptThreadID == i)continue;
			m_worldObjectList[i][etoi(eType_)].AddItem_endLock(obj_id, pObj_);
			g_allPlayers[i].AddItem_endLock(obj_id, pSession);
		}
	}
	else
	{
		for (int i = 0; i < CONTAINER_SIZE; ++i)
		{
			if (exceptThreadID == i)continue;
			m_worldObjectList[i][etoi(eType_)].AddItem_endLock(obj_id, pObj_);
		}
	}
}

//void TRWorldRoom::DeleteObject(const GROUP_TYPE eType_, const uint64 objID_, const uint64 exceptThreadID)
//{
//	for (int i = 0; i < ServerCore::ThreadMgr::NUM_OF_THREADS; ++i)
//	{
//		if (exceptThreadID == i)continue;
//		m_worldObjectList[i][etoi(eType_)].EraseItem(objID_);
//	}
//}

void TRWorldRoom::UpdateWorldCollision()
{
	const uint64 threadID = Mgr(ThreadMgr)->GetCurThreadID() - 1;

	for (uint16 iRow = 0; iRow < etoi(GROUP_TYPE::END); ++iRow)
	{
		for (uint16  iCol = iRow; iCol < etoi(GROUP_TYPE::END); ++iCol)
		{
			if (m_collisionChecker.GetCollisionBit(iRow, iCol))
			{
				m_collisionChecker.CollisionUpdateGroup(m_worldObjectList[threadID][iRow], m_worldObjectList[threadID][iCol]);
			}
		}
	}
}

void TRWorldRoom::ImigrationAfterBehavior(const S_ptr<ServerCore::SessionManageable> beforeRoom, const S_ptr<ServerCore::PacketSession> pSession_) noexcept
{
	const uint64 thID = Mgr(ThreadMgr)->GetCurThreadID() - 1;
	const uint64 sessionID = pSession_->GetSessionID();
	
	const auto& player = static_cast<ClientSession* const>(pSession_.get())->GetPlayer();
	const auto trBefromRoom = static_cast<TRWorldRoom* const>(beforeRoom.get());


	trBefromRoom->LeavePlayerEnqueue(sessionID);

	Protocol::s2c_LEAVE_OBJECT pkt1;
	pkt1.set_is_player(true);
	pkt1.set_obj_id(sessionID);
	pkt1.set_sector(beforeRoom->GetRoomID());

	trBefromRoom << pkt1;


	Protocol::s2c_TRY_NEW_ROOM pkt3;
	pkt3.set_next_sector_num(GetRoomID());
	pSession_ << pkt3;

	Protocol::s2c_APPEAR_NEW_OBJECT pkt2;
	pkt2.set_is_player(true);
	pkt2.set_sector(GetRoomID());
	pkt2.set_obj_id(sessionID);
	*pkt2.mutable_appear_pos() = player->GetPos();
	pkt2.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
	this << pkt2;

	for (const auto others : GetSessionList())
	{
		Protocol::s2c_APPEAR_NEW_OBJECT pkt;
		pkt.set_is_player(true);
		pkt.set_sector(GetRoomID());
		pkt.set_obj_id(others->GetSessionID());
		*pkt.mutable_appear_pos() = static_cast<const ClientSession* const>(others)->GetPlayer()->GetPos();
		pkt.set_time_stamp(ServerCore::GetTimeStampMilliseconds());
		pSession_ << pkt;
	}

	AddObjectEnqueue(GROUP_TYPE::PLAYER, player);
}

void TRWorldRoom::BroadCastToWorld(const S_ptr<ServerCore::SendBuffer> pSendBuffer)
{
	const uint64 thID_ = Mgr(ThreadMgr)->GetCurThreadID() - 1;

	const auto end_iter = g_allPlayers[thID_].cend_safe();
	
	for (auto iter = g_allPlayers[thID_].cbegin(); iter != end_iter;)
	{
		const auto pSession = (*iter);
		if (pSession->IsConnected())
		{
			pSession->SendOnlyEnqueue(pSendBuffer);
			m_vecForBroadCastToWorld.emplace_back(PoolNew<ServerCore::Task>(&ServerCore::Session::TryRegisterSend, pSession->SharedCastThis<ServerCore::Session>()));
			++iter;
		}
		else
		{
			iter = g_allPlayers[thID_].EraseItemAndGetIter(pSession->GetSessionID());
		}
	}
	if (const auto num = m_vecForBroadCastToWorld.size())
	{
		Mgr(ThreadMgr)->EnqueueGlobalTaskBulk(m_vecForBroadCastToWorld.data(), num);
		m_vecForBroadCastToWorld.clear();
	}
}

void TRWorldRoom::LeavePlayerEnqueue(const uint64 playerID) noexcept
{
	EnqueueAsync(&TRWorldRoom::LeavePlayer, uint64{ playerID });
}

void TRWorldRoom::TickTileCollision()
{
	//for (const auto& obj_list : m_worldObjectList)
	//{
	//	for (const auto obj : obj_list.GetItemListRef())
	//	{
	//		//if (const auto pRigid = obj->GetComp("RIGIDBODY")->Cast<RigidBody>())
	//		//{
	//		//	Protocol::c2s_MOVE pkt;
	//		//	*pkt.mutable_obj_pos() = ::ToProtoVec2(obj->GetPos());
	//		//	*pkt.mutable_scale() = ::ToProtoVec2(obj->GetScale());
	//		//	*pkt.mutable_wiil_pos() = ToProtoVec2(obj->GetWillPos());
	//		//	*pkt.mutable_vel() = ::ToProtoVec2(pRigid->GetVelocity());
	//		//	updateTileCollision(pkt);
	//		//};
	//		m_vecCollisionTask.emplace_back(ServerCore::PoolNew<ServerCore::Task>(
	//			[this,obj]()noexcept {
	//				this->UpdateTileCollisionForTick(obj);
	//				m_jobCount.fetch_sub(1, std::memory_order_release);
	//			}
	//		));
	//		//UpdateTileCollisionForTick(obj);
	//		//Mgr(ThreadMgr)->EnqueueGlobalTask(ServerCore::PoolNew<ServerCore::Task>(
	//		//	[this, pObj = obj->shared_from_this()]()mutable noexcept {this->UpdateTileCollisionForTick(std::move(pObj)); }
	//		//));
	//	}
	//}
	//if (const auto num = m_vecCollisionTask.size())
	//{
	//	Mgr(ThreadMgr)->EnqueueGlobalTaskBulk(m_vecCollisionTask.data(), num);
	//	m_jobCount.fetch_add(static_cast<int>(num), std::memory_order_acq_rel);
	//	m_vecCollisionTask.clear();
	//	while (0 != m_jobCount.load(std::memory_order_relaxed)) { std::this_thread::yield(); };
	//}
}

void TRWorldRoom::TryGetItem(const S_ptr<Object>& pPlayer)
{
	const uint64 threadID = Mgr(ThreadMgr)->GetCurThreadID() - 1;
	const auto pPlayerCollider = pPlayer->GetComp("COLLIDER")->Cast<Collider>();
	const auto& pSession = pPlayer->GetComp("SESSIONOBJECT")->Cast<SessionObject>()->GetSession();

	auto begin_iter = m_worldObjectList[threadID][etoi(GROUP_TYPE::DROP_ITEM)].cbegin();
	const auto end_iter = m_worldObjectList[threadID][etoi(GROUP_TYPE::DROP_ITEM)].cend_safe();
	for (; begin_iter != end_iter; ++begin_iter)
	{
		const auto pItem = (*begin_iter);
		const auto pCol = pItem->GetComp("COLLIDER")->Cast<Collider>();

		if (CollisionChecker::IsCollision(pPlayerCollider, pCol))
		{
			if (pItem->GetComp("ACQUIREITEM")->Cast<AcquireItem>()->TryGetItem())
			{
				Protocol::s2c_GET_ITEM pkt;
				pkt.set_obj_id(pSession->GetSessionID());
				pkt.set_item_name(pItem->GetObjectName());
				pkt.set_item_id(pItem->GetObjID());
				pkt.set_sector(GetRoomID());

				BroadCastToWorld(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
				//this << pkt;

				pItem->SetInvalid();
			}
		}
	}
}

void TRWorldRoom::UpdateTileCollisionForTick(const S_ptr<Object> pObj_)const noexcept
{
	constexpr const int try_num = 4;
	//if (pObj_->GetObjectGroup() == GROUP_TYPE::PLAYER)
	//{
	//	if (pObj_->GetComp("KEYINPUTHANDLER")->Cast<KeyInputHandler>()->GetKeyState(VK_SPACE) == KeyInputHandler::KEY_STATE::KEY_TAP)
	//	{
	//		try_num = 10;
	//	}
	//}

	const auto pRigid = pObj_->GetComp("RIGIDBODY")->Cast<RigidBody>();
	const auto pCol = pObj_->GetComp("COLLIDER")->Cast<Collider>();

	if (pCol == nullptr || "Monster_CthulhuEye" == pObj_->GetObjectName())[[unlikely]]
	{
		pObj_->SetPos(pObj_->GetWillPos());
		return;
	}
	
	//if (pObj_->GetObjectGroup() == GROUP_TYPE::PLAYER)
	//{
	//	if (pObj_->GetComp("KEYINPUTHANDLER")->Cast<KeyInputHandler>()->GetKeyState(VK_SPACE) == KeyInputHandler::KEY_STATE::KEY_TAP)
	//	{
	//		//pRigid->SetPrevPos(pObj_->GetPos() + Vec2{ 0,-2.f });
	//		pRigid->SetPrevVelocity(pRigid->GetVelocity() + Vec2{ 0,-2.f });
	//		pRigid->SetIsGround(false);
	//	}
	//}

	const Vec2 prev_pos = pRigid->GetPrevPos();
	const Vec2 prev_vel = pRigid->GetPrevVelocity();

	const Vec2 delta_pos = (pObj_->GetPos() - pRigid->GetPrevPos()) / (float)try_num;
	const Vec2 delta_vel = (pRigid->GetVelocity() - pRigid->GetPrevVelocity()) / (float)try_num;

	const auto pTileMap = GetTRWorld()->GetTileMap();

	Vec2 temp_prev_pos;
	//Vec2 temp_prev_vel;
	bool prev_landed = pRigid->IsGround();
	bool bCollide = false;
	for (int i = 1; i <= try_num; ++i)
	{
		const Vec2 seq_pos = prev_pos + delta_pos * (float)i;
		const Vec2 seq_vel = prev_vel + delta_vel * (float)i;



		//const Vec2 world_pos = TRWorld::GlobalToWorld(pObj_->GetWillPos());
		//const Vec2 world_vel = pRigid->GetVelocity();

		const Vec2 world_pos = TRWorld::GlobalToWorld(seq_pos);
		const Vec2 world_vel = seq_vel;


		const float w = pCol->GetScale().x / (float)PIXELS_PER_TILE;
		const float h = pCol->GetScale().y / (float)PIXELS_PER_TILE;

		//const Vec2 pre_pos = TRWorld::GlobalToWorld(pObj_->GetPos());

		const Vec2 pre_pos = TRWorld::GlobalToWorld(seq_pos + delta_pos);

		Vec2 post_pos = world_pos;
		Vec2 post_vel = world_vel;

		bool landed = false;
		bool collided = false;

		int x_min = FloorToInt(pre_pos.x - w * 0.5f);
		int x_max = CeilToInt(pre_pos.x + w * 0.5f) - 1;
		int y_min = FloorToInt(world_pos.y - h * 0.5f);
		int y_max = CeilToInt(world_pos.y + h * 0.5f) - 1;

		if (x_min >= 0 && x_max < TRWorld::WORLD_WIDTH && y_min >= 0 && y_max < TRWorld::WORLD_HEIGHT)
		{
			for (int x = x_min; x <= x_max; ++x)
			{
				if (world_vel.y > 0.0f && pTileMap->GetTile(x, y_min)->Solid())
				{
					post_pos.y = y_min + 1.0f + h * 0.5f;
					post_vel.y = 0.0f;

					landed = true;
					collided = true;
					break;
				}
				if (world_vel.y < 0.0f && pTileMap->GetTile(x, y_max)->Solid())
				{
					post_pos.y = y_max - h * 0.5f;
					post_vel.y = 0.0f;

					collided = true;
					break;
				}
			}
		}

		if (world_pos.x - w * 0.5f < 0.0f)
		{
			post_pos.x = w * 0.5f;
			post_vel.x = 0.0f;
		}
		if (world_pos.x + w * 0.5f > TRWorld::WORLD_WIDTH)
		{
			post_pos.x = TRWorld::WORLD_WIDTH - w * 0.5f;
			post_vel.x = 0.0f;
		}

		x_min = FloorToInt(post_pos.x - w * 0.5f);
		x_max = CeilToInt(post_pos.x + w * 0.5f) - 1;
		y_min = FloorToInt(post_pos.y - h * 0.5f);
		y_max = CeilToInt(post_pos.y + h * 0.5f) - 1;

		if (x_min >= 0 && x_max < TRWorld::WORLD_WIDTH && y_min >= 0 && y_max < TRWorld::WORLD_HEIGHT)
		{
			bool collision_x = false;
			float reform_x = 0.0f;

			for (int y = y_min; y <= y_max; ++y)
			{
				if (world_vel.x < 0.0f && pTileMap->GetTile(x_min, y)->Solid())
				{
					reform_x = x_min + 1.0f + w * 0.5f;

					collided = true;
					collision_x = true;
					break;
				}
				if (world_vel.x > 0.0f && pTileMap->GetTile(x_max, y)->Solid())
				{
					reform_x = x_max - w * 0.5f;

					collided = true;
					collision_x = true;
					break;
				}
			}

			if (collision_x)
			{
				y_min = y_min + 1;
				y_max = CeilToInt(y_min + h) - 1;

				bool flag = false;
				for (int x = x_min; x <= x_max; ++x)
				{
					for (int y = y_min; y <= y_max; ++y)
						flag |= pTileMap->GetTile(x, y)->Solid();
				}
				if (flag)
				{
					post_pos.x = reform_x;
					post_vel.x = 0.0f;
				}
				else if (post_vel.y >= 0.0f)
					post_pos.y = y_min + h * 0.5f;
			}
		}

		//if (prev_landed != landed)
		//{
		//	pObj_->SetPos(TRWorld::WorldToGlobal((post_pos)));
		//	pRigid->SetVelocity((post_vel));
		//	//pRigid->SetIsGround(prev_landed);
		//	prev_landed = landed;
		//	break;
		//}
		if (landed)
		{
			pObj_->SetPos(TRWorld::WorldToGlobal((post_pos)));
			pRigid->SetVelocity((post_vel));
			//pRigid->SetIsGround(prev_landed);
			prev_landed = landed;
			bCollide = true;
			break;
		}
		

		prev_landed = landed;
		temp_prev_pos = post_pos;
		//temp_prev_vel = post_vel;
	}
	if (!bCollide)
		pObj_->SetPos(TRWorld::WorldToGlobal(temp_prev_pos));

	pRigid->SetIsGround(prev_landed);

	pObj_->SetPrevPos(pObj_->GetPos());
	pRigid->SetPrevVelocity(pRigid->GetVelocity());
	
	pObj_->PostUpdate(GetSectorDT());

	//pRigid->SetIsGround(landed);
	//if (!landed)
	//{
	//	pRigid->SetIsGround(false);
	//}
}
