#include "pch.h"
#include "CollisionChecker.h"
#include "PhysicsComponent.h"
#include "Object.h"

bool CollisionChecker::IsCollision(const Collider* const _pLeftCol, const Collider* const _pRightCol, const Vec2 offSet_) noexcept
{
	const Vec2 vLeftPos = _pLeftCol->GetFinalPos() + offSet_;
	const Vec2 vLeftScale = _pLeftCol->GetScale();

	const Vec2 vRightPos = _pRightCol->GetFinalPos();
	const Vec2 vRightScale = _pRightCol->GetScale();

	if (bitwise_absf(vLeftPos.x - vRightPos.x) <= (vLeftScale.x + vRightScale.x) / 2.f
		&& bitwise_absf(vLeftPos.y - vRightPos.y) <= (vLeftScale.y + vRightScale.y) / 2.f)
	{
		return true;
	}

	return false;
}

void CollisionChecker::CollisionUpdateGroup(const ServerCore::LinkedHashMap<uint64, Object>& a, const ServerCore::LinkedHashMap<uint64, Object>& b)
{
	const auto& left_list = a.GetItemListRef();
	const auto& right_list = b.GetItemListRef();

	for (const auto iter_a : left_list)
	{
		const auto pLeftCol = iter_a->GetComp("COLLIDER")->Cast<Collider>();

		if (nullptr == pLeftCol)
			continue;

		for (const auto iter_b : right_list)
		{
			const auto pRightCol = iter_b->GetComp("COLLIDER")->Cast<Collider>();
			
			if (nullptr == pRightCol || pLeftCol == pRightCol)
				continue;

			const COLLIDER_ID ID{
				(static_cast<const ULONGLONG>(pLeftCol->GetID()) << 32)
				| static_cast<const ULONGLONG>(pRightCol->GetID())
			};

			const auto iter = m_mapColPrev.try_emplace(ID.ID, false).first;
			const bool nowCollision = IsCollision(pLeftCol, pRightCol);

			if (nowCollision)
			{
				if (iter->second)
				{
					if (pLeftCol->IsDead() || pRightCol->IsDead())
					{
						pLeftCol->OnCollisionExit(pRightCol);
						pRightCol->OnCollisionExit(pLeftCol);
					}
					else
					{
						pLeftCol->OnCollisionStay(pRightCol);
						pRightCol->OnCollisionStay(pLeftCol);
					}
				}
				else
				{
					if (!pLeftCol->IsDead() && !pRightCol->IsDead())
					{
						pLeftCol->OnCollisionEnter(pRightCol);
						pRightCol->OnCollisionEnter(pLeftCol);
						iter->second = true;
					}
				}
			}
			else
			{
				if (iter->second)
				{
					pLeftCol->OnCollisionExit(pRightCol);
					pRightCol->OnCollisionExit(pLeftCol);
					iter->second = false;
				}
			}
		}
	}
	
}

void CollisionChecker::RegisterGroup(const GROUP_TYPE _eLeft, const GROUP_TYPE _eRight)
{
	int iRow = static_cast<int>(_eLeft);
	int iCol = static_cast<int>(_eRight);

	if (iRow > iCol)
	{
		std::swap(iRow, iCol);
	}

	m_bitColTable[iRow][iCol].flip();
}
