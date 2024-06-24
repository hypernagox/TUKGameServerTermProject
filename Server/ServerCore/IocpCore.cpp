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

	bool IocpCore::RegisterIOCP(const IocpObject* const iocpObject_, const uint64 iocpKey_)const noexcept
	{
		return ::CreateIoCompletionPort(iocpObject_->GetHandle(), m_iocpHandle, (ULONG_PTR)(iocpKey_), 0);
	}

	bool IocpCore::Dispatch(c_uint32 timeOutMs)const noexcept
	{
		DWORD numOfBytes = 0;
		IocpEvent* iocpEvent = nullptr;

		const BOOL bResult = ::GetQueuedCompletionStatus(m_iocpHandle, OUT & numOfBytes, OUT reinterpret_cast<PULONG_PTR>(&LCurHandleSessionID), OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeOutMs);

		LEndTickCount = ::GetTickCount64() + 64;

		if (iocpEvent)
		{
			if (const S_ptr<IocpObject>& iocpObject = iocpEvent->GetIocpObject())
			{
				iocpObject->Dispatch(iocpEvent, numOfBytes);

				return true;
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

		return false;
	}
}