#pragma once

class Object;

class BaseComponent
{
public:
	BaseComponent(std::string_view compName_, Object* const pOwner_)noexcept;
	virtual ~BaseComponent();
public:
	const std::string& GetCompName()const noexcept { return m_strCompName; }
	Object* const GetOwner()noexcept { return m_pOwner; }
	const Object* const GetOwner()const noexcept { return m_pOwner; }

	template <typename T> requires std::derived_from<T, BaseComponent>
	T* const Cast()noexcept { return static_cast<T* const>(this); }

	template <typename T> requires std::derived_from<T, BaseComponent>
	const T* const Cast()const noexcept { return static_cast<const T* const>(this); }
protected:
	const std::string m_strCompName;
	Object* const m_pOwner;
};

class Component
	:public BaseComponent
{
public:
	Component(std::string_view compName_, Object* const pOwner_)noexcept :BaseComponent{ compName_,pOwner_ } {}
public:
	virtual void Update(const float dt_) = 0;
	virtual void PostUpdate(const float dt_)noexcept{}
};

