#include "pch.h"
#include "AIComponent.h"
#include "TRWorldRoom.h"
#include "Object.h"
#include "TaskTimerMgr.h"
#include "TRWorldMgr.h"
#include "ThreadMgr.h"
#include "TRWorldChunk.h"
#include "TimerNPC.h"
#include "ClientSession.h"

Astar::~Astar()
{
}

void Astar::ExecuteAI() noexcept
{
	const auto pOwner = static_pointer_cast<TimerNPC>(m_pOwner->GetIocpEntity());
	const auto player = ServerCore::GetSession(pOwner->GetCurChaseUser());
	
	if (!player)
		return;

	thread_local std::priority_queue<AstarNode, ServerCore::Vector<AstarNode>, std::greater<AstarNode>> m_pqAstar;
	thread_local ServerCore::HashMap<Vec2Int, Vec2Int> m_parent;
	thread_local uint16 best[TRWorld::WORLD_HEIGHT][TRWorld::WORLD_WIDTH];
	thread_local ServerCore::Vector<Vec2Int> m_path;

	m_parent.clear();
	m_path.clear();

	m_dest = TRWorld::GlobalToWorld(((ClientSession*)player.get())->GetPlayer()->GetPos());
	
	const auto& pworld = TRMgr(TRWorldMgr)->GetStartWorld();
	
	::memset(best, 0, sizeof(uint16) * TRWorld::WORLD_WIDTH * TRWorld::WORLD_HEIGHT);
	
	m_start = TRWorld::GlobalToWorld(m_pOwner->GetPos()) - Vec2Int{ 0,2 };
	m_start.x = std::clamp(m_start.x, 0, TRWorld::WORLD_WIDTH - 1);
	m_start.y = std::clamp(m_start.y, 0, TRWorld::WORLD_HEIGHT - 1);
	

	constexpr const int g = 0;
	const int h = calculateH(m_start, m_dest);

	m_pqAstar.emplace(m_start, (g + h));
	best[m_start.y][m_start.x] = g + h;
	m_parent[m_start] = m_start;

	constexpr const std::pair<Vec2Int, int> front[8] = {
	{{0, -1}, 1},
	{{0, 1}, 500},
	{{-1, 0}, 1},
	{{1, 0}, 1},
	{{-1, -1}, 2},
	{{-1, 1}, 500},
	{{1, -1}, 2},
	{{1, 1}, 500}
	};
	
	bool bFound = false;
	const auto start_time = std::chrono::steady_clock::now();
	while (!m_pqAstar.empty())
	{
		const auto end_time = std::chrono::steady_clock::now() - start_time;
		if (200 <= std::chrono::duration_cast<std::chrono::milliseconds>(end_time).count())
			break;
		
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
	
	Vec2Int pos = bFound ? m_dest : m_pqAstar.top().pos;

	while (false == m_pqAstar.empty()) { m_pqAstar.pop(); }
	
	for (;;)
	{
		m_path.emplace_back(pos);
		const auto iter = m_parent.find(pos);
		if (m_parent.end() != iter)
		{
			if (pos == m_parent[pos])
				break;
			pos = m_parent[pos];
		}
	}

	if(!m_path.empty())
	{
		m_start = m_path.size() > 1 ? *(m_path.end() - 2) : *m_path.begin();
	}

	int yy = pos.y;
	int delta_y = 0;
	while (!pworld->GetTileSolid(pos.x, yy)) {
		--yy;
		++delta_y;
		if (delta_y >= 3)
			break;
	}
	m_start.y = yy + 1;
	
	m_pOwner->SetPos(TRWorld::WorldToGlobal(m_start + Vec2Int{0,2}));
}
