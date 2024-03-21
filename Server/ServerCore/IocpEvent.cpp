#include "ServerCorePch.h"
#include "IocpEvent.h"
#include "IocpObject.h"
#include "Session.h"

namespace ServerCore
{
	IocpEvent::IocpEvent(const EVENT_TYPE eType_)
		:m_eEVENT_TYPE{ eType_ }
	{
		Init();
	}

	IocpEvent::~IocpEvent()
	{
	}

	AcceptEvent::~AcceptEvent()
	{
		ReleaseIocpObject();
		ReleaseSession();
	}

	ConnectEvent::~ConnectEvent()
	{
		ReleaseIocpObject();
	}

	DisconnectEvent::~DisconnectEvent()
	{
		ReleaseIocpObject();
	}

	RecvEvent::~RecvEvent()
	{
		ReleaseIocpObject();
	}

	SendEvent::~SendEvent()
	{
		ReleaseIocpObject();
	}
}