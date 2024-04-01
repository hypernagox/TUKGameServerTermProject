#pragma once
#include "pch.h"

class CObject;
class CTexture;
class CLayer;
class CTileLayer;
class CPlayer;
class CCthulhuEye;
class Hero;

enum class SECTOR
{
	SECTOR_0,
	SECTOR_1,
	SECTOR_2,
	SECTOR_3,
	SECTOR_4,

	END,
};

class CScene
{
	friend class CDebugMgr;
	friend class CCollisionMgr;
	friend class CMiniMap;
private:
	const auto& GetSceneObj()const { return m_vecObj[m_iSectorNum]; }
public:
	CScene();
	virtual ~CScene();
protected:
	vector<unique_ptr<CLayer>> m_vecLayer;
	vector<unique_ptr<CTileLayer>> m_vecTileLayer;
	Vec2 m_vRes = {};
private:
	vector<unique_ptr<CObject>>			m_vecObj[etoi(SECTOR::END)][(UINT)GROUP_TYPE::END];
	wstring								m_strName;	
	CObject*							m_pPlayer = {};
	HDC	m_hSceneThreadDC[THREAD::END + 1];
	HBITMAP	m_hSceneThreadBit[THREAD::END + 1];
	int m_iSectorNum = 0;
public: 
	void RegisterPlayer(CObject* const _pPlayer) { m_pPlayer = _pPlayer; }
	CObject* GetPlayer()const { return m_pPlayer; }
	Hero* GetPlayerCast()const { return reinterpret_cast<Hero*>(m_pPlayer); }
	void AddObject(CObject* const _pObj, GROUP_TYPE _eType);
	const vector<unique_ptr<CObject>>& GetGroupObject(GROUP_TYPE _eType)const;
	vector<unique_ptr<CObject>>& GetUIGroup();
	auto& GetPlayerWeapon() { return m_vecObj[m_iSectorNum][etoi(GROUP_TYPE::PLAYER_WEAPON)]; }
public:

	void SetSector(const int sector)noexcept { m_iSectorNum = sector; }
	const int GetSectorNum()const noexcept { return m_iSectorNum; }
	void component_update()const;
	
	void DeleteGroup(GROUP_TYPE _eTarget);
	void Reset();


	virtual void render(HDC _dc);
	virtual void update();
	virtual void Enter();
	virtual void Exit();

public:
	void SetName(wstring_view _strName) { m_strName = _strName; }
	const wstring& GetName() { return m_strName; }
	void AddTileLayer(CTileLayer* const _pTileLayer);

	void ChangeSector(const int targetSector);

	auto& GetSectorObject(const uint64_t sector_)noexcept { return m_vecObj[sector_]; }
};
