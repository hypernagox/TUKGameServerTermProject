#pragma once

namespace ServerCore
{
	class Session;
	class IocpObject;
	class Session;
	class SendBuffer;

	/*--------------
		IocpEvent
	---------------*/

	// �����Լ� ������� (�����Լ����̺� �����ͷ����� ����ü������� ��ȭ)
	class IocpEvent
		:public OVERLAPPED
	{
	public:
		IocpEvent(const EVENT_TYPE eType_);
		~IocpEvent();
	public:
		void Init()noexcept { ::memset(static_cast<OVERLAPPED* const>(this), NULL, sizeof(OVERLAPPED)); }
		constexpr const EVENT_TYPE GetEventType()const noexcept { return m_eEVENT_TYPE; }
		//void SetIocpObject(S_ptr<IocpObject>&& pIocp_)noexcept {
		//	//std::construct_at(&m_pIocpObject, std::move(pIocp_));
		////	m_pIocpObject.swap(pIocp_);
		//	m_pIocpObject = std::move(pIocp_);
		//}
		//void SetIocpObject(S_ptr<class PacketSession>&& pSession_)noexcept {
		//	//std::construct_at(&m_pIocpObject, std::move(pSession_), reinterpret_cast<IocpObject* const>(pSession_.get()));
		//	//m_pIocpObject.swap(pSession_);
		//	m_pIocpObject = std::move(pSession_);
		//}
		template <typename T> //requires //std::convertible_to<T,S_ptr<IocpObject>>
		void SetIocpObject(T&& pIocp_)noexcept {
			std::construct_at(&m_pIocpObject, std::forward<T>(pIocp_));
			//m_pIocpObject = std::forward<T>(pIocp_);
		}
		const S_ptr<IocpObject>& GetIocpObject()const noexcept { return m_pIocpObject; }
		void ReleaseIocpObject()noexcept { m_pIocpObject.reset(); }
		[[nodiscard]] constexpr __forceinline S_ptr<IocpObject>&& PassIocpObject()noexcept { return std::move(m_pIocpObject); }
		template<typename T> requires std::derived_from<T, IocpEvent>
		T* const Cast()noexcept;
		template<typename T> requires std::derived_from<T, IocpEvent>
		const T* const Cast()const noexcept;
	private:
		const EVENT_TYPE m_eEVENT_TYPE;
		S_ptr<IocpObject> m_pIocpObject = {};
	};

	template<typename T> requires std::derived_from<T, IocpEvent>
	inline T* const IocpEvent::Cast()noexcept { return static_cast<T* const>(this); }

	template<typename T> requires std::derived_from<T, IocpEvent>
	inline const T* const IocpEvent::Cast() const noexcept { return static_cast<const T* const>(this); }

	/*--------------
		ConnectEvent
	---------------*/

	class ConnectEvent
		:public IocpEvent
	{
	public:
		ConnectEvent() :IocpEvent{ EVENT_TYPE::CONNECT } {}
		~ConnectEvent();
	};

	/*--------------
		DisconnectEvent
	---------------*/

	class DisconnectEvent
		:public IocpEvent
	{
	public:
		DisconnectEvent() :IocpEvent{ EVENT_TYPE::DISCONNECT } {}
		~DisconnectEvent();
	};


	/*--------------
		AcceptEvent
	---------------*/

	class AcceptEvent
		:public IocpEvent
	{
	public:
		AcceptEvent() :IocpEvent{ EVENT_TYPE::ACCEPT } {}
		~AcceptEvent();
		const S_ptr<Session>& RegisterSession(S_ptr<Session>&& pSession_)noexcept {
			m_pSession.swap(pSession_);
			return m_pSession;
		}
		S_ptr<Session> ReleaseSession()noexcept { return S_ptr<Session>{std::move(m_pSession)}; }
		[[nodiscard]] constexpr __forceinline S_ptr<Session>&& PassSession()noexcept { return std::move(m_pSession); }
	private:
		S_ptr<Session> m_pSession;
	};

	/*--------------
		RecvEvent
	---------------*/

	class RecvEvent
		:public IocpEvent
	{
	public:
		RecvEvent() :IocpEvent{ EVENT_TYPE::RECV } {}
		~RecvEvent();
	};

	/*--------------
		SendEvent
	---------------*/

	class SendEvent
		:public IocpEvent
	{
	public:
		SendEvent() :IocpEvent{ EVENT_TYPE::SEND } {}
		~SendEvent();
		IocpEvent m_registerSendEvent{ EVENT_TYPE::REGISTER_SEND };
	};
}