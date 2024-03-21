#pragma once
#include "enable_shared_cache_this.hpp"

namespace ServerCore
{
	class IocpEvent;

	/*--------------
		IocpCore
	---------------*/

	// IOCP�� ��� ������ ��� ������Ʈ
	class IocpObject
		:public enable_shared_cache_this_core<IocpObject>
	{
	protected:
		IocpObject();
		virtual ~IocpObject();
	public:
		virtual HANDLE GetHandle()const noexcept abstract;
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept abstract;
	};
}