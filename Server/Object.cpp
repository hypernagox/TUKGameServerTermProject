#include "pch.h"
#include "Object.h"
#include "Component.h"
#include "ClientSession.h"

Object::~Object()
{
	std::cout << "destroy object" << std::endl;
}
