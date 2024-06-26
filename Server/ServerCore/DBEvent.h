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
		virtual const bool IsValid()const noexcept override { return m_bSuccess; }
		virtual void BindData()noexcept {}
		virtual void ExecuteQuery()noexcept abstract;
		virtual void Dispatch(ServerCore::IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept  abstract;
	public:
		void SetEventPtr()noexcept { m_dbEvent.SetIocpObject(SharedFromThis<DBEvent>()); }
	protected:
		bool m_bSuccess = false;
		ServerCore::IocpEvent m_dbEvent{ ServerCore::EVENT_TYPE::DB };
	};
}