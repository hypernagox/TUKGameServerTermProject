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

	void RegisterWorld(const SECTOR worldID_,std::shared_ptr<TRWorldRoom> world_)noexcept {
		m_mapWorld.emplace_no_return(worldID_, std::move(world_));
	}

private:
	ServerCore::ConcurrentHashMap<SECTOR, S_ptr<TRWorldRoom>> m_mapWorld;
};

