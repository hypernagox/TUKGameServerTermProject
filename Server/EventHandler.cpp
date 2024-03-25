#include "pch.h"
#include "EventHandler.h"

void EventHandler::UpdateEvent()
{
	
	m_vecInternalEventBuffer.swap(m_vecEvent);

	for (const auto& eve : m_vecInternalEventBuffer)
	{
		eve();
	}

	m_vecInternalEventBuffer.clear();

	for (const auto& eve : m_vecEvent)
	{
		eve();
	}

	for (const auto& eve : m_vecInternalEventBuffer)
	{
		eve();
	}

	m_vecInternalEventBuffer.clear();

	for (const auto& eve : m_vecEvent)
	{
		eve();
	}

	m_vecEvent.clear();
	m_vecDeadObj.clear();
}
