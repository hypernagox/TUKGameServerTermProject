#pragma once
#include "Singleton.hpp"

namespace ServerCore
{
	class IocpCore;

	class CoreGlobal
		:public Singleton<CoreGlobal>
	{
		friend class Singleton;
		CoreGlobal();
		~CoreGlobal();
	public:
		const std::shared_ptr<IocpCore>& GetIocpCore()const noexcept { return m_iocpCore; }
		void Init()noexcept override;
	private:
		const std::shared_ptr<IocpCore> m_iocpCore;
	};
}