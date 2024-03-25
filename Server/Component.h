#pragma once

class Object;

class Component
{
public:
	Component(std::string_view compName_, Object* const pOwner_);
	virtual ~Component();
public:
	virtual void Update(const float dt_) = 0;
	virtual void PostUpdate(const float dt_)noexcept{}

	const std::string& GetCompName()const noexcept { return m_strCompName; }
	Object* const GetOwner()noexcept { return m_pOwner; }
	const Object* const GetOwner()const noexcept { return m_pOwner; }

	template <typename T> requires std::derived_from<T,Component>
	T* const Cast()noexcept { return static_cast<T* const>(this); }

	template <typename T> requires std::derived_from<T, Component>
	const T* const Cast()const noexcept { return static_cast<const T* const>(this); }
protected:
	const std::string m_strCompName;
	Object* const m_pOwner;
};

