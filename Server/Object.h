#pragma once
#include "PhysicsComponent.h"

class ClientSession;

class SessionObject
	:public Component
{
public:
	SessionObject(S_ptr<ClientSession>&& pSession_,Object* const pOwner_)
		: Component{ "SESSIONOBJECT",pOwner_ }
		, m_pSession{ std::move(pSession_) }
	{}
public:
	void Update(const float)override{}

	const S_ptr<ClientSession>& GetSession()const noexcept { return m_pSession; }
	
private:
	const S_ptr<ClientSession> m_pSession;
};

class Object
	:public ServerCore::enable_shared_cache_this_core<Object>
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
		//m_positionComponent.Update(dt_);
		for (const auto& pComp : m_vecComponentList)
			pComp->Update(dt_);
	}
	void PostUpdate(const float dt_)noexcept {
		m_positionComponent.PostUpdate(dt_);
		for (const auto& pComp : m_vecComponentList)
			pComp->PostUpdate(dt_);
	}
	Component* const GetComp(std::string_view compName)const noexcept {
		const auto iter = m_mapComponent.find(compName.data());
		return m_mapComponent.end() != iter ? iter->second : nullptr;
	}

	template<typename T> requires std::derived_from<T,Component>
	T* const AddComponent(S_ptr<T>&& pComp)noexcept {
		NAGOX_ASSERT(nullptr == GetComp(pComp->GetCompName()));
		const auto temp_ptr = static_cast<T* const>(m_vecComponentList.emplace_back(std::move(pComp)).get());
		m_mapComponent.try_emplace(temp_ptr->GetCompName(), temp_ptr);
		return temp_ptr;
	}

	const uint64 GetObjID()const noexcept { return m_objID; }
	const GROUP_TYPE GetObjectGroup()const noexcept { return m_eObjectGroup; }
	const std::string& GetObjectName()const noexcept { return m_strObjectName; }
	const std::string& GetImgName()const noexcept { return m_strImgName; }
	PositionComponent& GetPositionComponent()noexcept { return m_positionComponent; }
public:
	void SetPrevPos(const Vec2 v_)noexcept { m_positionComponent.SetPrevPos(v_); }
	void SetPos(const Vec2 v_)noexcept { m_positionComponent.SetPos(v_); }
	void SetWillPos(const Vec2 v_)noexcept { m_positionComponent.SetWillPos(v_); }
	void SetScale(const Vec2 v_)noexcept { m_positionComponent.SetScale(v_); }
	void SetImageName(std::string_view imgName_)noexcept { m_strImgName = imgName_; }
public:
	const Vec2 GetPos()const noexcept { return m_positionComponent.GetPos(); }
	const Vec2 GetWillPos()const noexcept { return m_positionComponent.GetWillPos(); }
	const Vec2 GetScale()const noexcept { return m_positionComponent.GetScale(); }
	const Vec2 GetPrevPos()const noexcept { return m_positionComponent.GetPrevPos(); }
	
	const bool IsValid()const noexcept { return m_bIsValid.load(std::memory_order_acquire); }
	const bool SetInvalid()noexcept { return m_bIsValid.exchange(false,std::memory_order_acq_rel); }
private:
	ServerCore::Vector<S_ptr<Component>> m_vecComponentList;
	ServerCore::HashMap<std::string, Component*> m_mapComponent;

	PositionComponent m_positionComponent;
	std::atomic_bool m_bIsValid = true;
	const uint64 m_objID;
	const GROUP_TYPE m_eObjectGroup;
	const std::string m_strObjectName;
	std::string m_strImgName;
};

