#pragma once
#include "IocpObject.h"
#include "NetAddress.h"

namespace ServerCore
{
	class AcceptEvent;
	class ServerService;

	/*--------------
		Listener
	---------------*/

	// Accept �޾��ִ� Ŭ���� (������)
	class Listener
		:public IocpObject
	{
	public:
		Listener();
		~Listener();
	public:
		bool StartAccept(ServerService* const  pServerService_);
		void CloseAccept();
		void FinishServer()noexcept;
		const bool CanAccept()const noexcept { return m_bCanAccept.load(); }
	public:
		virtual HANDLE GetHandle()const noexcept override { return reinterpret_cast<HANDLE>(m_socket); }
		virtual void Dispatch(IocpEvent* const iocpEvent_, c_int32 numOfBytes)noexcept override;
	private:
		void RegisterAccept(AcceptEvent* const acceptEvent)noexcept;
		void ProcessAccept(const S_ptr<class PacketSession>& pSession, AcceptEvent* const acceptEvent)noexcept;
	protected:
		SOCKET m_socket = INVALID_SOCKET; // ������ ��������
		Vector<std::shared_ptr<AcceptEvent>> m_vecAcceptEvent;
		ServerService* m_pServerService = nullptr;

		std::atomic_bool m_bCanAccept = true;
	};
}
