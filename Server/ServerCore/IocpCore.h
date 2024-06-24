#pragma once

namespace ServerCore
{
	class IocpObject;

	/*--------------
		IocpCore
	---------------*/


	class IocpCore
	{
	public:
		IocpCore();
		~IocpCore();

		HANDLE GetIocpHandle()const noexcept { return m_iocpHandle; }

		bool RegisterIOCP(const IocpObject* const iocpObject_, const uint64 iocpKey_ = 0)const noexcept; // 클라 입장시 iocp에 등록
		bool Dispatch(c_uint32 timeOutMs = INFINITE)const noexcept; // gqcs로 일감을 빼내서 일을 처리하는 스레드함수

	private:
		const HANDLE m_iocpHandle;
	};
}