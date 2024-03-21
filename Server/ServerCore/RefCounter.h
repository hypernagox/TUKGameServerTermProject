#pragma once
#include "pch.h"
#include "Memory.h"

class RefCounter
{
public:
	//static void* operator new(const std::size_t size) = delete;
	//static void operator delete(void* const) = delete;
public:
	RefCounter()noexcept;
	virtual ~RefCounter()noexcept;
public:
	void AddRef()noexcept;
	void ReleaseRef()noexcept;

	//void Del();
private:
	std::atomic_int m_refCount = 0;
};

