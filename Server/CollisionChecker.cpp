#include "pch.h"
#include "CollisionChecker.h"
#include "PhysicsComponent.h"
#include "Object.h"

bool CollisionChecker::IsCollision(const Collider* const _pLeftCol, const Collider* const _pRightCol, const Vec2 offSet_) noexcept
{
	//const Vec2 vLeftPos = _pLeftCol->GetFinalPos() + offSet_;
	const Vec2 vLeftPos = _pLeftCol->GetFinalPos();
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

bool CollisionChecker::IsCollisionCCD(const Collider* const _pLeftCol, const Collider* const _pRightCol,
	const int try_num ,
	const Vec2 aOffset, 
	const Vec2 bOffset) noexcept
{
	// 거리 너무 과하면 바로 쳐냄
	const Vec2 vPrev_A = _pLeftCol->GetPrevPos() + aOffset;
	const Vec2 vPrev_B = _pRightCol->GetPrevPos() + bOffset;

	const Vec2 vScale_A = _pLeftCol->GetScale() / 2.f;
	const Vec2 vScale_B = _pRightCol->GetScale() / 2.f;

	
	const int dx = (int)bitwise_absf(vPrev_B.x - vPrev_A.x);
	const int dy = (int)bitwise_absf(vPrev_B.y - vPrev_A.y);

	const int combinedHalfWidth =  ((int)(vScale_A.x + vScale_B.x)) * 2;
	const int combinedHalfHeight = ((int)(vScale_A.y + vScale_B.y)) * 2;

	if (!(dx <= combinedHalfWidth && dy <= combinedHalfHeight))
		return false;

	const Vec2 vDelta_A = (_pLeftCol->GetFinalPos() - vPrev_A) / (float)try_num;
	const Vec2 vDelta_B = (_pRightCol->GetFinalPos() - vPrev_B) / (float)try_num;

	for (int i = 1; i <= try_num; ++i)
	{
		const Vec2 seq_A = vPrev_A + vDelta_A * (float)i;

		for (int j = 1; j <= try_num; ++j)
		{
			const Vec2 seq_B = vPrev_B + vDelta_B * (float)j;

			if (bitwise_absf(seq_A.x - seq_B.x) <= vScale_A.x + vScale_B.x
				&& bitwise_absf(seq_A.y - seq_B.y) <= vScale_A.y + vScale_B.y)
			{
				return true;
			}
		}
	}

	return false;
}

bool CollisionChecker::IsCollisionRange(const Collider* const _pLeftCol, const Collider* const _pRightCol, const float range) noexcept
{
	const Vec2 LeftPos = _pLeftCol->GetFinalPos();
	const Vec2 vRangePos = LeftPos + Vec2{ range/2.f,0.f };
	const float x_range = bitwise_absf(LeftPos.x - vRangePos.x);
	const Vec2 vLeftPos = Vec2{ (LeftPos.x + vRangePos.x) / 2.f,LeftPos.y };
	const Vec2 vLeftScale = Vec2{ 0.f,_pLeftCol->GetScale().y } + Vec2{ x_range,0.f };

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
		const auto pLeftCol = iter_a->GetComp<Collider>();

		if (nullptr == pLeftCol)
			continue;

		for (const auto iter_b : right_list)
		{
			const auto pRightCol = iter_b->GetComp<Collider>();
			
			if (nullptr == pRightCol || pLeftCol == pRightCol)
				continue;

			const COLLIDER_ID ID{
				(static_cast<const ULONGLONG>(pLeftCol->GetID()) << 32)
				| static_cast<const ULONGLONG>(pRightCol->GetID())
			};

			if (pLeftCol->IsDead() || pRightCol->IsDead())
				continue;

			const auto iter = m_mapColPrev.try_emplace(ID.ID, false).first;
			const bool nowCollision = IsCollisionCCD(pLeftCol, pRightCol);

			if (nowCollision)
			{
				if (iter->second)
				{
					if (pLeftCol->IsDead() || pRightCol->IsDead())
					{
						pLeftCol->OnCollisionExit(pRightCol);
						pRightCol->OnCollisionExit(pLeftCol);
						m_mapColPrev.erase(iter);
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
					else
					{
						m_mapColPrev.erase(iter);
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
				if (pLeftCol->IsDead() || pRightCol->IsDead())
				{
					m_mapColPrev.erase(iter);
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
