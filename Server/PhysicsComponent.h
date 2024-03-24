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
	void Update(const float dt_)override{}
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
	void Update(const float dt_)override;
	
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

class RigidBody :
	public Component
{

private:
	Vec2			m_vLimitBreak = Vec2{ 240.0f, 720.0f } *5.f;
	Vec2            m_vForce;
	Vec2            m_vAccel;
	Vec2            m_vVelocity;
	Vec2            m_vMaxVelocity = { 240.0f, 720.0f };
	Vec2			m_vMaxVelocityOrigin = { 240.0f, 720.0f };
	float            m_fMass = 1.f;
	float           m_fFriction = 640.0f;
	bool            m_bGravity = true;
	bool            m_bIsGround = false;
private:
	void Move(const float dt_);
public:
	RigidBody(Object* const pOwner_);
	~RigidBody();
public:
	bool  IsGround()const { return m_bIsGround || IsFloatZero(m_vVelocity.y); }
	void SetIsGround(bool _b)
	{
		m_bIsGround = _b;
		if (m_bIsGround)
		{
			m_vForce.y = 0.f;
			m_vVelocity.y = 0.f;
		}
	}
	bool  IsGravity()const { return m_bGravity; }
	void  SetGravity(bool _b) { m_bGravity = _b; }
public:
	constexpr inline Vec2 GetVelocity()const { return m_vVelocity; }
	constexpr inline void SetVelocity(Vec2 _v) { m_vVelocity = _v; }
	constexpr inline void SetMaxVelocity(Vec2 _vMaxVel)
	{
		m_vLimitBreak = _vMaxVel * 5.f;
		m_vMaxVelocity = _vMaxVel;
		m_vMaxVelocityOrigin = _vMaxVel;
	}
	constexpr inline void SetFriction(float _value) { m_fFriction = _value; }
	constexpr inline float GetFriction() const { return m_fFriction; }
	constexpr inline void SetMass(float _fMass) { m_fMass = _fMass; }
	constexpr inline float GetMass()const { return m_fMass; }
	constexpr inline void AddForce(Vec2 _vF) { m_vForce += _vF; }
	constexpr inline void AddVelocity(Vec2 _v) { m_vVelocity += _v; }
	constexpr inline Vec2 GetForce()const { return m_vForce; }
	constexpr inline void SetForce(Vec2 _v) { m_vForce = _v; }
	void SetLimitBreak()
	{
		m_vMaxVelocity = m_vLimitBreak;
		m_fFriction = 0.f;
	}
	void SetLimitOrigin()
	{
		m_vMaxVelocity = m_vMaxVelocityOrigin;
		m_fFriction = 640.f;
	}
public:

	void Update(const float dt_)override;

	void update_gravity();
};
