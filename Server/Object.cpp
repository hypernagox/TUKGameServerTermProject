#include "pch.h"
#include "Object.h"
#include "Component.h"


Object::~Object()
{
	std::cout << "destroy" << std::endl;
}

