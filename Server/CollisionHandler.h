#pragma once

class Object;

class CollisionHandler
	:public ServerCore::RefCountable
{
public:
	virtual void OnCollisionEnter(Object* const a, Object* const b) {}
	virtual void OnCollisionStay(Object* const a, Object* const b) {}
	virtual void OnCollisionExit(Object* const a, Object* const b) {}
};

class PlayerCollisionHandler
	:public CollisionHandler
{
public:
	virtual void OnCollisionEnter(Object* const a, Object* const b)override;
	virtual void OnCollisionStay(Object* const a, Object* const b) override;
	virtual void OnCollisionExit(Object* const a, Object* const b)override;
};

