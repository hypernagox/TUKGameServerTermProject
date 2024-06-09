#pragma once

class Object;

enum class COMP_TYPE : uint8
{
	PositionComponent,
	Collider,
	RigidBody,
	KeyInputHandler,
	Item,
	AcquireItem,
	Attackable,
	MoveBroadCaster,
	Astar,
	Inventory,

	END
};

class BaseComponent
{
public:
	BaseComponent(const COMP_TYPE compName_, Object* const pOwner_)noexcept;
	virtual ~BaseComponent();
public:
	constexpr const COMP_TYPE GetCompType()const noexcept { return m_eCompType; }
	Object* const GetOwner()noexcept { return m_pOwner; }
	const Object* const GetOwner()const noexcept { return m_pOwner; }

	template <typename T> requires std::derived_from<T, BaseComponent>
	T* const Cast()noexcept { return static_cast<T* const>(this); }

	template <typename T> requires std::derived_from<T, BaseComponent>
	const T* const Cast()const noexcept { return static_cast<const T* const>(this); }

protected:
	const COMP_TYPE m_eCompType;
	Object* const m_pOwner;
};

class Component
	:public BaseComponent
{
public:
	Component(const COMP_TYPE compName_, Object* const pOwner_)noexcept :BaseComponent{ compName_,pOwner_ } {}
public:
	virtual void Update(const float dt_) = 0;
	virtual void PostUpdate(const float dt_)noexcept{}
};

#define GET_COMP_TYPE_FUNC(ClassType) static inline consteval const COMP_TYPE GetCompTypeNameGlobal() noexcept { return COMP_TYPE::ClassType; }

#define CONSTRUCTOR_BASE_COMPONENT(ClassType) \
    ClassType(Object* const pOwner_) noexcept \
        : BaseComponent(COMP_TYPE::ClassType, pOwner_) {} \
    GET_COMP_TYPE_FUNC(ClassType)

#define CONSTRUCTOR_COMPONENT(ClassType) \
    ClassType(Object* const pOwner_) noexcept \
        : Component(COMP_TYPE::ClassType, pOwner_) {} \
    GET_COMP_TYPE_FUNC(ClassType)
