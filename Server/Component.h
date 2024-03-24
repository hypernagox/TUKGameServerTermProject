#pragma once

class Object;

class Component
{
public:
	Component(std::string_view compName_, Object* const pOwner_);
	virtual ~Component();
public:
	virtual void Update(const float dt_) = 0;
	
	const std::string& GetCompName()const noexcept { return m_strCompName; }
	Object* const GetOwner()noexcept { return m_pOwner; }
	const Object* const GetOwner()const noexcept { return m_pOwner; }

	template <typename T> requires std::derived_from<T,Component>
	Component* const Cast()noexcept { return static_cast<T* const>(this); }

	template <typename T> requires std::derived_from<T, Component>
	const Component* const Cast()const noexcept { return static_cast<const T* const>(this); }
protected:
	const std::string m_strCompName;
	Object* const m_pOwner;
};

