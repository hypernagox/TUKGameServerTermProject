#include "pch.h"
#include "RefCounter.h"

RefCounter::RefCounter()noexcept
	:m_refCount{0}
{
}

RefCounter::~RefCounter()noexcept
{
	std::cout << typeid(this).name() << std::endl;
}

void RefCounter::AddRef() noexcept
{
	++m_refCount;
}

void RefCounter::ReleaseRef() noexcept
{
	if (nullptr == this)
		return;
	const int refCount = --m_refCount;
	if (0 == refCount)
	{
		//this->~RefCounter();
		//Mgr(Memory)->Release(this);
		::xdelete<RefCounter>(this);
		//::free(this);
		return;
	}
	//ASSERT_CRASH(0 <= refCount);
}

