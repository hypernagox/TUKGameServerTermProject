#pragma once

#include "TRWorld.h"
#include "CScene.h"
#include "Singleton.hpp"

class TRMain
	:public Singleton<TRMain>
{
	friend class Singleton;
	TRMain();
	~TRMain();
public:
	TRWorld* active_world;
	CScene* scene_agent;
	CScene* scene_agent2;

	TRWorld* m_main_world = nullptr;
	TRWorld* m_dungeon = nullptr;

	TRWorld* m_arrTRWorlds[etoi(SCENE_TYPE::END) - 1];
	CScene* m_arrScene[etoi(SCENE_TYPE::END) - 1];
public:
	void Update();
	void ChangeTRWorld();
};