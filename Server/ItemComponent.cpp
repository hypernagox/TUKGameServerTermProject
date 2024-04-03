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
	// 1. ������ ���� ��Ŷ
	// 2. ���� �������� ����?
	// 3. �κ��丮 -> ������Ʈ -> Attackable ã��
	// 4. ��Ÿ�� �¾�?
	// 5. ������ ���ѹ��� ���͵�� �浹�˻�
	// 6. �浹�ߴٸ� �浹��Ŷ
	// 7. ��Ÿ�� ����
}
