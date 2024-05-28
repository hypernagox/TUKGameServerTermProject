#pragma once
#include "PhysicsComponent.h"
#include "IocpObject.h"

class ClientSession;


class ContentsEntity
	:public ServerCore::enable_shared_cache_this_core<ContentsEntity>
{
public:
	virtual ~ContentsEntity() = default;
public:
	std::atomic<ServerCore::SessionManageable*> m_pCurSector = nullptr;
};

class Object
	:public ContentsEntity
{
public:
	Object(S_ptr<ServerCore::IocpEntity> pOwnerEntity_ ,const GROUP_TYPE eType_,std::string_view name_)
		: m_pOwnerEntity{ pOwnerEntity_ }
		, m_objID{ pOwnerEntity_->GetObjectID() }
		, m_eObjectGroup{eType_}
		, m_strObjectName{name_}
		, m_positionComponent{ this }
	{}
	~Object();
public:
	ServerCore::S_ptr<Object> shared_from_this()noexcept { return SharedCastThis<Object>(); }
	ServerCore::S_ptr<const Object> shared_from_this()const noexcept { return SharedCastThis<const Object>(); }

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
	BaseComponent* const GetComp(std::string_view compName)const noexcept {
		const auto iter = m_mapComponent.find(compName.data());
		return m_mapComponent.end() != iter ? iter->second.get() : nullptr;
	}
	template<typename T> requires std::convertible_to<T,S_ptr<Component>>
	const auto AddComponent(T&& pComp)noexcept {
		NAGOX_ASSERT(nullptr == GetComp(pComp->GetCompName()));
		m_mapComponent.try_emplace(pComp->GetCompName(), pComp);
		return static_cast<decltype(pComp.get())>(m_vecComponentList.emplace_back(std::move(pComp)).get());
	}
	template<typename T>  requires std::convertible_to<T, S_ptr<BaseComponent>>
	const auto AddBaseComponent(T&& pComp)noexcept {
		NAGOX_ASSERT(nullptr == GetComp(pComp->GetCompName()));
		const auto temp_ptr = pComp.get();
		m_mapComponent.try_emplace(temp_ptr->GetCompName(), std::move(pComp));
		return temp_ptr;
	}
	const uint64 GetObjID()const noexcept { return m_objID; }
	const GROUP_TYPE GetObjectGroup()const noexcept { return m_eObjectGroup; }
	const std::string& GetObjectName()const noexcept { return m_strObjectName; }
	const std::string& GetImgName()const noexcept { return m_strImgName; }
	PositionComponent& GetPositionComponent()noexcept { return m_positionComponent; }
	const int32 GetState()const noexcept { return m_state; }
public:
	void SetPrevPos(const Vec2 v_)noexcept { m_positionComponent.SetPrevPos(v_); }
	void SetPos(const Vec2 v_)noexcept { m_positionComponent.SetPos(v_); }
	void SetWillPos(const Vec2 v_)noexcept { m_positionComponent.SetWillPos(v_); }
	void SetScale(const Vec2 v_)noexcept { m_positionComponent.SetScale(v_); }
	void SetImageName(std::string_view imgName_)noexcept { m_strImgName = imgName_; }
	void SetState(const int32 state_)noexcept { m_state = state_; }
public:
	const Vec2 GetPos()const noexcept { return m_positionComponent.GetPos(); }
	const Vec2 GetWillPos()const noexcept { return m_positionComponent.GetWillPos(); }
	const Vec2 GetScale()const noexcept { return m_positionComponent.GetScale(); }
	const Vec2 GetPrevPos()const noexcept { return m_positionComponent.GetPrevPos(); }
	
	const bool IsValid()const noexcept { return m_bIsValid.load(std::memory_order_acquire); }
	const bool SetInvalid()noexcept { return m_bIsValid.exchange(false,std::memory_order_acq_rel); }
	//ServerCore::IocpEntity* const GetIocpEntity()noexcept { return m_pOwnerEntity.get(); }
	ServerCore::S_ptr<ServerCore::IocpEntity> const GetIocpEntity()noexcept { return m_pOwnerEntity.lock(); }
	//const ServerCore::IocpEntity* const GetIocpEntity()const noexcept { return m_pOwnerEntity; }
	//const S_ptr<ServerCore::IocpEntity>& GetIocpEntity()const noexcept { return m_pOwnerEntity; }
private:
	const W_ptr<ServerCore::IocpEntity> m_pOwnerEntity;
	ServerCore::Vector<S_ptr<Component>> m_vecComponentList;
	ServerCore::HashMap<std::string, S_ptr<BaseComponent>> m_mapComponent;

	PositionComponent m_positionComponent;
	int32 m_state = 0;
	std::atomic_bool m_bIsValid = true;
	const uint64 m_objID;
	const GROUP_TYPE m_eObjectGroup;
	const std::string m_strObjectName;
	std::string m_strImgName;
};

