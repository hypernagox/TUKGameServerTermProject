#include "ServerCorePch.h"
#include "IocpCore.h"
#include "IocpObject.h"
#include "IocpEvent.h"

namespace ServerCore
{
	IocpCore::IocpCore()
		:m_iocpHandle{ ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, NULL) }
	{
		NAGOX_ASSERT(INVALID_HANDLE_VALUE != m_iocpHandle);
	}

	IocpCore::~IocpCore()
	{
	}

	bool IocpCore::RegisterIOCP(const IocpObject* const iocpObject_)const noexcept
	{
		return ::CreateIoCompletionPort(iocpObject_->GetHandle(), m_iocpHandle, 0, 0);
	}

	bool IocpCore::Dispatch(c_uint32 timeOutMs)const noexcept
	{
		DWORD numOfBytes = 0;
		ULONG_PTR dummyKey = 0;
		IocpEvent* iocpEvent = nullptr;

		const BOOL bResult = ::GetQueuedCompletionStatus(m_iocpHandle, OUT & numOfBytes, OUT & dummyKey, OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeOutMs);

		if (iocpEvent)
		{
			if (const S_ptr<IocpObject>& iocpObject = iocpEvent->GetIocpObject())
			{
				iocpObject->Dispatch(iocpEvent, numOfBytes);
			}
		}
		else
		{
			const int32 errCode = ::WSAGetLastError();
			switch (errCode)
			{
			case WAIT_TIMEOUT:
				return false;
			default:
				// TODO 왜 여기로 왔는지 로그 찍기
				//iocpObject->Dispatch(iocpEvent, numOfBytes);
				break;
			}
		}

		return true;
	}
}