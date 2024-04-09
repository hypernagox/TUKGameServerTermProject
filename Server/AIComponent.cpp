#include "pch.h"
#include "AIComponent.h"
#include "TRWorldRoom.h"
#include "Object.h"
#include "TaskTimerMgr.h"
#include "TRWorldMgr.h"
#include "ThreadMgr.h"

Astar::Astar(Object* const pOwner_)
	:Component{ "ASTAR",pOwner_ }
{
}

Astar::~Astar()
{
}

void Astar::InitAI(const S_ptr<Astar>& forCacheThis_) noexcept
{
	register_cache_shared_core(forCacheThis_);
	m_ownerForValid = m_pOwner->shared_from_this_ref();
	m_timerEvent.SetIocpObject(forCacheThis_);

	Mgr(TaskTimerMgr)->ReserveAsyncTask(1000, &m_timerEvent);
}

void Astar::Update(const float dt)
{
	return;
	m_accTime += 0.1f;
	//Vec2Int pos = TRWorld::GlobalToWorld(m_pOwner->GetPos());
	//if (m_accTime >= m_dist)
	//if(m_accTime >= 1.f)
	{
		m_accTime = 0.f;
		
		{
			std::lock_guard<ServerCore::SpinLock> lock{ m_lock };
			if (m_path.empty())
			{
				//const auto pworld = TRWorldRoom::GetTRWorld();
				//auto vpos = m_curPos;
				//
				//vpos.x = std::clamp(vpos.x, 0, TRWorld::WORLD_WIDTH - 1);
				//vpos.y = std::clamp(vpos.y-1, 0, TRWorld::WORLD_HEIGHT - 1);
				//int yy = vpos.y;
				//while (!pworld->GetTileSolid(vpos.x, vpos.y)) {
				//	vpos.y = std::clamp(vpos.y - 1, 0, TRWorld::WORLD_HEIGHT - 1);
				//}
				//vpos.y = yy + 1;
				//m_pOwner->SetPos(TRWorld::WorldToGlobal(vpos));
				return;
			}
			m_prevPos = m_curPos;
			m_curPos = m_path.back();
			m_path.pop_back();
			//const Vec2 vDir = TRWorld::WorldToGlobal(m_prevPos) - m_pOwner->GetPos();
			//m_dist = vDir.length();
			m_accTime = 0.f;
			m_pOwner->SetPos(TRWorld::WorldToGlobal(m_curPos));
			//m_start = m_curPos;
		}
		//if (m_prevPos != m_prevPos.zero)
		//{
		//	
		//	m_pOwner->SetPos(
		//		TRWorld::WorldToGlobal(m_curPos)*(1.f - m_accTime)+
		//		TRWorld::WorldToGlobal(m_prevPos) * m_accTime);
		//}
	}
	//if (pos != pos.zero)
	//{
	//	const Vec2 vDir = TRWorld::WorldToGlobal(pos) - m_pOwner->GetPos();
	//	m_accTime += vDir.Normalize().length() * 20.f;
	//	m_pOwner->SetPos(m_pOwner->GetPos() + vDir.Normalize() * 20.f);
	//}
	//m_start = TRWorld::GlobalToWorld(m_pOwner->GetPos());

	//m_pOwner->SetPos(TRWorld::WorldToGlobal(m_start));
}

void Astar::Dispatch(ServerCore::IocpEvent* const iocpEvent_, c_int32 numOfBytes) noexcept
{
	if (!m_ownerForValid->IsValid())
	{
		m_timerEvent.ReleaseIocpObject();
		reset_cache_shared();
		m_ownerForValid.reset();
		return;
	}
	if (const auto pRoom = TRMgr(TRWorldMgr)->GetWorldRoom(SECTOR::SECTOR_0))
	{
		const auto& player_list = pRoom->GetGroupObject(Mgr(ThreadMgr)->GetCurThreadIdx(), GROUP_TYPE::PLAYER);
		if (!player_list.empty())
		{
			m_dest = TRWorld::GlobalToWorld(player_list.front()->GetPos());
		}
	}
	m_parent.clear();
	m_prev = m_start;
	const auto pworld = TRWorldRoom::GetTRWorld();
	//std::fill(best, best + TRWorld::WORLD_WIDTH * TRWorld::WORLD_HEIGHT, UINT16_MAX);
	::memset(best, 0, sizeof(uint16) * TRWorld::WORLD_WIDTH * TRWorld::WORLD_HEIGHT);
	//m_pqAstar = {};
	//m_start = TRWorld::GlobalToWorld(m_pOwner->GetPos());
	//{
	//	std::lock_guard<ServerCore::SpinLock> lock{ m_lock };
	//	if (!m_path.empty())
	//	{
	//		m_start = m_path.back();
	//	}
	//}
	//m_start = TRWorld::GlobalToWorld(m_pOwner->GetPos());
	//m_start = m_curPos;
	m_start.x = std::clamp(m_start.x, 0, TRWorld::WORLD_WIDTH - 1);
	m_start.y = std::clamp(m_start.y, 0, TRWorld::WORLD_HEIGHT - 1);
	int yy = m_start.y - 1;
	while (!pworld->GetTileSolid(m_start.x, yy)) {
		--yy;
	}
	m_start.y = yy + 1;
	//if (pworld->GetTileSolid(m_start.x - 1, yy))
	//{
	//	m_start.x += 2;
	//}
	//ServerCore::Map<Vec2Int, int32> best;

	
	const int g = 0;
	const int h = calculateH(m_start, m_dest);

	m_pqAstar.emplace(m_start, (g + h));
	best[m_start.y][m_start.x] = g + h;
	m_parent[m_start] = m_start;

	constexpr const std::pair<Vec2Int, int> front[8] = {
	{{0, -1}, 1},
	{{0, 1}, 1},
	{{-1, 0}, 1},
	{{1, 0}, 1},
	{{-1, -1}, 2},
	{{-1, 1}, 2},
	{{1, -1}, 2},
	{{1, 1}, 2}
	};

	bool bFound = false;
	int cnt = 0;
	while (!m_pqAstar.empty())
	{

		const AstarNode node = m_pqAstar.top();

		m_pqAstar.pop();
		if (best[node.pos.y][node.pos.x] < node.cost)
			continue;

		if (node.pos == m_dest)
		{
			bFound = true;
			break;
		}

		for (const auto [npos, w] : front)
		{
			const Vec2Int nextPos = node.pos + npos;
			if (nextPos.x < 0 || nextPos.x >= TRWorld::WORLD_WIDTH || nextPos.y < 0 || nextPos.y >= TRWorld::WORLD_HEIGHT)
				continue;
			if (pworld->GetTileSolid(nextPos.x, nextPos.y))
				continue;
			const int g = best[node.pos.y][node.pos.x] + w;
			const int h = calculateH(nextPos, m_dest);

			const int bestCost = best[nextPos.y][nextPos.x];
			const int f = (g + h);
			if (0 == bestCost || bestCost > f)
			{
				best[nextPos.y][nextPos.x] = f;
				m_pqAstar.emplace(nextPos, f);
				m_parent[nextPos] = node.pos;
			}
		}
	}
	if (!bFound)
	{
		// 못찾으면할거
	}
	ServerCore::Vector<Vec2Int> path;
	path.reserve(m_parent.size());
	Vec2Int pos = m_dest;
	cnt = 0;
	{
		//std::lock_guard<ServerCore::SpinLock> lock{ m_lock };
		//m_path.clear();
		for (;;)
		{
			//if (cnt >= 100)
			//	break;
			path.emplace_back(pos);
			if (pos == m_parent[pos])
				break;
			pos = m_parent[pos];
			//++cnt;
		}
		//std::ranges::reverse(m_path);
	}
	cnt = 0;
	//for (auto [vv, vp] : parent)
	//{
	//	path.emplace_back(vv);
	//}
	//std::ranges::sort(path, [this](Vec2Int a,Vec2Int b) {
	//	return calculateH(a, m_dest) > calculateH(b, m_dest);
	//	});
	{
		//std::lock_guard<ServerCore::SpinLock> lock{ m_lock };
		m_path.clear();
		for (const auto vpos : path | std::views::reverse)
		{
			if (cnt == 2)
			{
				//m_pOwner->SetPrevPos(TRWorld::WorldToGlobal(vpos));
				//if (pos != m_prev)
				{
					m_start = pos;
					//if (pworld->GetTileSolid(m_start.x - 1, m_start.y))
					//{
					//	pos.x -= 1;
					//	pos.y += 3;
					//	m_start = pos;
					//}
					//m_path.emplace_back(m_start);
					break;
				}
				//else
				//{
				//	m_start.y += 2;
				//	//m_start.x -= 2;
				//	m_start = pos;
				//	break;
				//}
			}
			else
			{
				//else if (cnt >= 3)
				//{
				//	//m_pOwner->SetPos(TRWorld::WorldToGlobal(vpos));
				//	break;
				//}
				//std::cout << vpos.x << ", " << vpos.y << std::endl;
				//if (cnt >= 10)
				//{
				//
				//	break;
				//}
				++cnt;
				pos = vpos;
				//m_path.emplace_back(vpos);
			}
		}
		//m_start = m_path.back();
	}
	if (pworld->GetTileSolid(m_start.x-1, m_start.y))
	{
		pos.x -= 1;
		pos.y += 3;
		m_start = pos;
	}
	m_start = pos;
	//m_curPos = m_start;

	m_pOwner->SetPos(TRWorld::WorldToGlobal(pos));

	Mgr(TaskTimerMgr)->ReserveAsyncTask(100, &m_timerEvent);
}
