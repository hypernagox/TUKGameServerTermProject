#pragma once
#include "IocpObject.h"

namespace ServerCore
{
	class DBEvent
		:public ServerCore::IocpObject
	{
		friend class DBMgr;
	public:
		DBEvent()noexcept = default;
		virtual ~DBEvent()noexcept = default;
		DBEvent(DBEvent&& other)noexcept
			:m_dbEvent{ std::move(other.m_dbEvent) }
		{}
	public:
		virtual void BindData()noexcept {}
		virtual void ExecuteQuery()noexcept abstract;
		virtual void Dispatch(ServerCore::IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept  abstract;
	public:
		void SetEventPtr(S_ptr<DBEvent> pDBEvent)noexcept {
			//m_dbEvent.SetIocpObject(std::move(pDBEvent));
			m_dbEvent.SetIocpObject(S_ptr<IocpObject>(pDBEvent));

		}
	protected:
		bool m_bSuccess = false;
		ServerCore::IocpEvent m_dbEvent{ ServerCore::EVENT_TYPE::DB };
	};
}