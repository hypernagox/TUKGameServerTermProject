#pragma once


class Collider;
class Object;

union COLLIDER_ID
{
	struct {
		UINT Left_id;
		UINT Right_id;
	};
	ULONGLONG ID;
};

class CollisionChecker
{
private:
	ServerCore::HashMap<ULONGLONG, bool> m_mapColPrev;
	std::bitset<etoi(GROUP_TYPE::END)> m_bitColTable[etoi(GROUP_TYPE::END)];
public:
	static bool IsCollision(const Collider* const _pLeftCol, const Collider* const _pRightCol, const Vec2 offSet_ = Vec2{})noexcept;
	const bool GetCollisionBit(const uint16 iRow_, const uint16 iCol_)const noexcept { return m_bitColTable[iRow_][iCol_]; }
public:
	
	void CollisionUpdateGroup(const ServerCore::LinkedHashMap<uint64, Object>& a, const ServerCore::LinkedHashMap<uint64, Object>& b);
	void RegisterGroup(const GROUP_TYPE _eLeft, const GROUP_TYPE _eRight);

	void Reset()
	{
		m_mapColPrev.clear();
		for (auto& bit : m_bitColTable)
		{
			bit.reset();
		}
	}
};

