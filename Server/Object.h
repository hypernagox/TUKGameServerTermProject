#pragma once
#include "PhysicsComponent.h"

class Object
{
public:
	Object(const uint64 id_,const GROUP_TYPE eType_,std::string_view name_)
		: m_objID{id_}
		, m_eObjectGroup{eType_}
		, m_strObjectName{name_}
		, m_positionComponent{ this }
	{}
	~Object();
public:
	void Update(const float dt_) {
		for (const auto& pComp : m_vecComponentList)
			pComp->Update(dt_);
	}
	Component* const GetComp(std::string_view compName)const noexcept {
		const auto iter = std::ranges::find_if(m_vecComponentList, [&compName](const S_ptr<Component>& pComp)noexcept {
			return pComp->GetCompName() == compName;
			});
		return m_vecComponentList.end() != iter ? iter->get() : nullptr;
	}

	Component* const AddComponent(S_ptr<Component>&& pComp)noexcept {
		NAGOX_ASSERT(nullptr != GetComp(pComp->GetCompName()));
		return m_vecComponentList.emplace_back(std::move(pComp)).get();
	}

	const uint64 GetObjID()const noexcept { return m_objID; }
	const GROUP_TYPE GetObjectGroup()const noexcept { return m_eObjectGroup; }
	const std::string& GetObjectName()const noexcept { return m_strObjectName; }

public:
	void SetPos(const Vec2 v_)noexcept { m_positionComponent.SetPos(v_); }
	void SetWillPos(const Vec2 v_)noexcept { m_positionComponent.SetWillPos(v_); }
	void SetScale(const Vec2 v_)noexcept { m_positionComponent.SetScale(v_); }
public:
	const Vec2 GetPos()const noexcept { return m_positionComponent.GetPos(); }
	const Vec2 GetWillPos()const noexcept { return m_positionComponent.GetWillPos(); }
	const Vec2 GetScale()const noexcept { return m_positionComponent.GetScale(); }
private:
	ServerCore::Vector<S_ptr<Component>> m_vecComponentList;

	PositionComponent m_positionComponent;
	const uint64 m_objID;
	const GROUP_TYPE m_eObjectGroup;
	const std::string m_strObjectName;
};

