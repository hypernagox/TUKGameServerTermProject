#pragma once
#include "PhysicsComponent.h"
#include "IocpObject.h"

class ClientSession;
class Object;

#define GET_COMP(object, type) (object->GetCompByEnum(COMP_TYPE::type)->Cast<type>())

class ContentsEntity
	:public ServerCore::RefCountable
{
public:
	virtual ~ContentsEntity() = default;
public:
	std::atomic<ServerCore::SessionManageable*> m_pCurSector = nullptr;
public:
	Object* const ObjectCast()noexcept { return reinterpret_cast<Object* const>(this); }
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
	ServerCore::S_ptr<Object> shared_from_this()noexcept { return SharedFromThis<Object>(); }
	ServerCore::S_ptr<const Object> shared_from_this()const noexcept { return SharedFromThis<const Object>(); }

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
	BaseComponent* const GetCompByEnum(const COMP_TYPE compName)const noexcept {
		const auto iter = m_mapComponent.find(compName);
		return m_mapComponent.end() != iter ? iter->second.get() : nullptr;
	}
	template <typename T>
	T* const GetComp()const noexcept {
		constexpr const COMP_TYPE type = T::GetCompTypeNameGlobal();
		return GetCompByEnum(type)->Cast<T>();
	}
	const S_ptr<BaseComponent>& GetCompByEnumShared(const COMP_TYPE compName)const noexcept {
		const auto iter = m_mapComponent.find(compName);
		return m_mapComponent.end() != iter ? iter->second : nullptr;
	}
	template <typename T>
	S_ptr<T> GetCompShared()const noexcept {
		constexpr const COMP_TYPE type = T::GetCompTypeNameGlobal();
		return std::static_pointer_cast<T>(GetCompByEnumShared(type));
	}
	template<typename T> requires std::convertible_to<T,S_ptr<Component>>
	const auto AddComponent(T&& pComp)noexcept {
		NAGOX_ASSERT(nullptr == GetCompByEnum(pComp->GetCompType()));
		m_mapComponent.try_emplace(pComp->GetCompType(), pComp);
		return static_cast<decltype(pComp.get())>(m_vecComponentList.emplace_back(std::forward<T>(pComp)).get());
	}
	template<typename T>  requires std::convertible_to<T, S_ptr<BaseComponent>>
	const auto AddBaseComponent(T&& pComp)noexcept {
		NAGOX_ASSERT(nullptr == GetCompByEnum(pComp->GetCompType()));
		const auto temp_ptr = pComp.get();
		m_mapComponent.try_emplace(temp_ptr->GetCompType(), std::forward<T>(pComp));
		return temp_ptr;
	}
	template<typename T>
	const auto AddComponent()noexcept {
		auto pComp = MakeShared<T>(this);
		m_mapComponent.try_emplace(pComp->GetCompType(), pComp);
		return static_cast<decltype(pComp.get())>(m_vecComponentList.emplace_back(std::move(pComp)).get());
	}
	template<typename T>
	const auto AddBaseComponent()noexcept {
		auto pComp = MakeShared<T>(this);
		const auto temp_ptr = pComp.get();
		m_mapComponent.try_emplace(temp_ptr->GetCompType(), std::move(pComp));
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
	const ServerCore::S_ptr<ServerCore::IocpEntity>& GetIocpEntity()noexcept { return m_pOwnerEntity; }
	//const ServerCore::IocpEntity* const GetIocpEntity()const noexcept { return m_pOwnerEntity; }
	//const S_ptr<ServerCore::IocpEntity>& GetIocpEntity()const noexcept { return m_pOwnerEntity; } 
	template <typename... Components>
	void AddComponents()noexcept {
		(AddComponent<Components>(), ...);
	}
	template <typename... Components>
	void AddBaseComponents()noexcept {
		(AddBaseComponent<Components>(), ...);
	}
	void ResetEntity()noexcept { m_pOwnerEntity.reset(); }
	void SetDir(const int i) { m_positionComponent.SetDir(i); }
	const int GetDir()const noexcept { return m_positionComponent.GetDir(); }
private:
	S_ptr<ServerCore::IocpEntity> m_pOwnerEntity;
	ServerCore::Vector<S_ptr<Component>> m_vecComponentList;
	ServerCore::HashMap<COMP_TYPE, S_ptr<BaseComponent>> m_mapComponent;

	PositionComponent m_positionComponent;
	int32 m_state = 0;
	std::atomic_bool m_bIsValid = true;
	const uint64 m_objID;
	const GROUP_TYPE m_eObjectGroup;
	const std::string m_strObjectName;
	std::string m_strImgName;
};

static int GetObjectDistancePow2(const Object* const a, const Object* const b)noexcept {
	const Vec2 apos = a->GetPos();
	const Vec2 bpos = b->GetPos();
	const int x = (int)(apos.x - bpos.x);
	const int y = (int)(apos.y - bpos.y);
	return x * x + y * y;
}