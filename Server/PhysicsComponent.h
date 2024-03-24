#pragma once
#include "pch.h"
#include "Component.h"
#include "CollisionHandler.h"

class PositionComponent
	:public Component
{
public:
	PositionComponent(Object* const pOwner_)
		:Component{ "POSITION",pOwner_ }
	{}
public:
	void Update()override{}
public:
	void SetPos(const Vec2 v_)noexcept { m_vPos = v_; }
	void SetWillPos(const Vec2 v_)noexcept { m_vWillPos = v_; }
	void SetScale(const Vec2 v_)noexcept { m_vScale = v_; }
public:
	const Vec2 GetPos()const noexcept { return m_vPos; }
	const Vec2 GetWillPos()const noexcept { return m_vWillPos; }
	const Vec2 GetScale()const noexcept { return m_vScale; }
private:
	Vec2 m_vPos;
	Vec2 m_vWillPos;
	Vec2 m_vScale;
};

class Collider
	:public Component
{
	static inline std::atomic<UINT>		g_iNextID;
public:
	Collider(Object* const pOwner_);
	~Collider();
private:
	Vec2				m_vOffsetPos = {};
	Vec2				m_vFinalPos = {};
	Vec2				m_vScale = {};
	const UINT			m_iID;
	int			m_iColCnt = {};
	S_ptr<CollisionHandler> m_pCollisionHandler;
public:
	void SetCollisionHandler(S_ptr<CollisionHandler>&& pCollisionHandler)noexcept { m_pCollisionHandler = std::move(pCollisionHandler); }
	constexpr const UINT GetID()const noexcept{ return m_iID; }
	constexpr inline Vec2 GetFinalPos()const noexcept{ return m_vFinalPos; }
	constexpr inline void SetOffsetPos(Vec2 _vPos) noexcept{ m_vOffsetPos = _vPos; }
	constexpr inline void SetScale(Vec2 _vScale) noexcept{ m_vScale = _vScale; }
	constexpr inline Vec2 GetOffsetPos()const noexcept{ return m_vOffsetPos; }
	constexpr inline Vec2 GetScale()const noexcept{ return m_vScale; }
	int GetColCnt()const noexcept{ return m_iColCnt; }
public:
	void Update()override;
	
public:
	void OnCollisionEnter(Collider* const _pOther) {
		m_pCollisionHandler->OnCollisionEnter(GetOwner(), _pOther->GetOwner());
	}

	void OnCollisionStay(Collider* const _pOther) {
		m_pCollisionHandler->OnCollisionStay(GetOwner(), _pOther->GetOwner());
	}

	void OnCollisionExit(Collider* const _pOther) {
		m_pCollisionHandler->OnCollisionExit(GetOwner(), _pOther->GetOwner());
	}


};

