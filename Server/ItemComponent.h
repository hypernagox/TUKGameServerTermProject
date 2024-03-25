#pragma once
#include "Component.h"

class Useable
	:public Component
{
public:
	void Update(const float dt_)override{}
	virtual void Use(const float dt_) = 0;
};