#pragma once

class TRWorldRoom;


class TRWorldMgr
	:public ServerCore::Singleton<TRWorldMgr>
{
	friend class Singleton;
	TRWorldMgr();
	~TRWorldMgr();
public:
	void Init()noexcept override;

	std::shared_ptr<TRWorldRoom> GetWorldRoom(const SECTOR worldID_)const noexcept{
		return m_mapWorld.find(worldID_).value();
	}

	void RegisterWorld(const SECTOR worldID_, std::shared_ptr<TRWorldRoom> world_)noexcept;

	const float GetSectorDT(const SECTOR eType_)const noexcept;
private:
	ServerCore::ConcurrentHashMap<SECTOR, S_ptr<TRWorldRoom>> m_mapWorld;
	std::array<S_ptr<TRWorldRoom>, etoi(SECTOR::END)> m_arrRoom;
};

