#pragma once

class TRWorldRoom;
class TRWorldChunk;

class TRWorldMgr
	:public ServerCore::Singleton<TRWorldMgr>
{
	friend class Singleton;
	TRWorldMgr();
	~TRWorldMgr();
public:
	void Init()noexcept override;

	//std::shared_ptr<TRWorldRoom> GetWorldRoom(const SECTOR worldID_)const noexcept{
	//	return m_mapWorld.find(worldID_).value();
	//}

	//S_ptr<TRWorldChunk> GetWorldChunk(const CHUNK worldID_)const noexcept {
	//	return m_mapWorldChunk.find(worldID_).value();
	//}
	const S_ptr<TRWorldChunk>& GetWorldChunk(const CHUNK worldID_)const noexcept {
		return m_arrWorldChunk[etoi(worldID_)];
	}
	//void RegisterWorld(const SECTOR worldID_, std::shared_ptr<TRWorldRoom> world_)noexcept;

	//const auto& GetStartWorld()const noexcept { return m_arrWorldChunk[0]; }
	const auto& GetStartWorld()const noexcept { return m_arrWorldChunk[0]; }
	//const float GetSectorDT(const SECTOR eType_)const noexcept;

private:
	//ServerCore::ConcurrentHashMap<SECTOR, S_ptr<TRWorldRoom>> m_mapWorld;
	//ServerCore::ConcurrentHashMap<SECTOR, S_ptr<TRWorldChunk>> m_mapWorldChunk;

	//ServerCore::S_ptr<TRWorldChunk> m_pMainWorld;
	//ServerCore::S_ptr<TRWorldRoom> m_pMainWorld;
	//std::array<S_ptr<TRWorldRoom>, etoi(SECTOR::END)> m_arrRoom;

	std::array<S_ptr<TRWorldChunk>, etoi(CHUNK::END)> m_arrWorldChunk;
};

