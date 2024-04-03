#include "pch.h"
#include "ItemComponent.h"

Attackable::Attackable(std::string_view compName_, Object* const pOwner_)
	:Useable{ "ATTACKABLE",pOwner_ }
{
}

Attackable::~Attackable()
{
}

void Attackable::Update(const float dt_)
{
	
}

void Attackable::Use(const float dt_)
{
	// 1. 유저의 공격 패킷
	// 2. 지금 아이템이 뭐지?
	// 3. 인벤토리 -> 오브젝트 -> Attackable 찾음
	// 4. 쿨타임 맞아?
	// 5. 유저의 속한방의 몬스터들과 충돌검사
	// 6. 충돌했다면 충돌패킷
	// 7. 쿨타임 걸음
}
