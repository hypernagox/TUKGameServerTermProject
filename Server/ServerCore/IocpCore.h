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

		bool RegisterIOCP(const IocpObject* const iocpObject_, const uint64 iocpKey_ = 0)const noexcept; // Ŭ�� ����� iocp�� ���
		bool Dispatch(c_uint32 timeOutMs = INFINITE)const noexcept; // gqcs�� �ϰ��� ������ ���� ó���ϴ� �������Լ�

	private:
		const HANDLE m_iocpHandle;
	};
}