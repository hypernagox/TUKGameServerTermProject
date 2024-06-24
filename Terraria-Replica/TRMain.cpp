#include "pch.h"
#include "TRMain.h"
#include "TRTileManager.h"
#include "TRItemManager.h"
#include "CSceneMgr.h"
#include "CLayer.h"
#include "CCamera.h"
#include "CEventMgr.h"
#include "CScene_Start.h"
#include "CResMgr.h"
#include <random>
#include <time.h>

int g_TR_SEED = 0;

TRMain::TRMain()
{
	std::default_random_engine dre((unsigned int)time(NULL));
	std::uniform_int_distribution<int> uid;

	Mgr(TRTileManager)->LoadTiles();
	Mgr(TRItemManager)->LoadItems();

	//m_dungeon = new TRWorld();
	//m_dungeon->CreateWorld(g_TR_SEED);
	//
	//scene_agent = Mgr(CSceneMgr)->GetScene(SCENE_TYPE::START);
	//m_dungeon->OnSceneCreate(scene_agent);
	//
	//m_main_world = new TRWorld();
	//m_main_world->CreateWorld(g_TR_SEED + 2024 * 2024);
	//
	//scene_agent2 = Mgr(CSceneMgr)->GetScene(SCENE_TYPE::STAGE_01);
	//m_main_world->OnSceneCreate(scene_agent2);
	
	//active_world = m_main_world;

	//active_world = m_dungeon;
	for (int i = 0; i < etoi(SCENE_TYPE::INTRO); ++i)
	{
		m_arrTRWorlds[i] = new TRWorld;
		auto temp = m_arrTRWorlds[i];
		if (i == 3)
			g_TR_SEED = 123456789;
		if (i >= 4)
			g_TR_SEED = 987654321 + i;

		temp->CreateWorld(g_TR_SEED + (int)std::pow(2024, i) * (0 != i));
		m_arrScene[i] = Mgr(CSceneMgr)->GetScene((SCENE_TYPE)i);
		m_arrTRWorlds[i]->OnSceneCreate(m_arrScene[i]);
	}
	active_world = m_arrTRWorlds[0];
	Mgr(CCamera)->SetTarget(active_world->GetPlayer());
}

TRMain::~TRMain()
{
	delete active_world;
}

void TRMain::Update()
{
	active_world->Update();
}

void TRMain::ChangeTRWorld()
{
	//for (const auto p : active_world->m_mapOtherPlayer | std::views::values)
	//	DeleteObj(p);
	//active_world->m_mapOtherPlayer.clear();
	//for (const auto p : active_world->m_mapServerObject | std::views::values)
	//	DeleteObj(p);
	//active_world->m_mapServerObject.clear();

	active_world = m_arrTRWorlds[sector];
	((CScene_Start*)Mgr(CSceneMgr)->GetScene((SCENE_TYPE)sector))->LoadWorld();
	Mgr(CCamera)->SetTarget(active_world->GetPlayer());
	Mgr(CEventMgr)->SetTRupdate(&TRMain::Update, this);
}
