#pragma once
#include "Singleton.hpp"

namespace ServerCore
{
	class CoreGlobal
		:public Singleton<CoreGlobal>
	{
		friend class Singleton;
	private:
		CoreGlobal();
		~CoreGlobal();
	};
}