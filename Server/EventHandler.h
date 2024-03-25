#pragma once

class Object;

class EventHandler
{
private:
	ServerCore::Vector<std::function<void(void)>> m_vecEvent;
	
	ServerCore::Vector<S_ptr<Object>> m_vecDeadObj;

	ServerCore::Vector<std::function<void(void)>> m_vecInternalEventBuffer;
	
public:
	void UpdateEvent();

	template<typename Func, typename... Args>
		requires std::invocable<Func, Args...>
	void AddEvent(Func&& fp, Args&&... args) { m_vecEvent.emplace_back([fp = std::forward<Func>(fp), ...args = std::forward<Args>(args)]()mutable noexcept{std::invoke(std::forward<Func>(fp), std::forward<Args>(args)...); }); }
	
	
	void AddDeadObj(S_ptr<Object>&& pDeadObj_)noexcept {
		m_vecDeadObj.emplace_back(std::move(pDeadObj_));
	}
};


