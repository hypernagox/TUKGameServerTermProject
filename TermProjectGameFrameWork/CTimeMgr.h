#pragma once

class CTimeMgr
	:public Singleton<CTimeMgr>
{
	friend class Singleton;
private:
	CTimeMgr();
	~CTimeMgr();
private:
	std::chrono::steady_clock::time_point m_PrevTime;
	std::chrono::duration<float> m_DeltaTime;
	float m_fAccTime = 0.f;
	float m_fCheckDT = 1.f;
public:
	void init();
	void update();
	float GetDT()const {
		const auto dt = m_DeltaTime.count();
		if (0.016f > dt) [[likely]]
			return dt;
		else [[unlikely]]
			return 0.016f;
	}
};

