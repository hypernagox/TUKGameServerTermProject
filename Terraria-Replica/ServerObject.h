#pragma once
#include "CObject.h"
#include "Protocol.pb.h"

class ServerObject
	:public CObject
{
public:
	ServerObject();
	~ServerObject();
	ServerObject* Clone()const override { return new ServerObject{ *this }; }
public:
	void SetObjID(const uint64_t id_)noexcept { m_objID = id_; }
	const uint64_t GetObjID()const noexcept { return m_objID; }

	virtual void SetNewMoveData(const Protocol::s2c_MOVE& movePkt_)noexcept;
	virtual Protocol::c2s_MOVE MakeSendData()const noexcept;
	auto& GetInterPolator()noexcept { return m_interpolator; }
protected:
	virtual void UpdateMoveData()noexcept;
	void SetCurrentTimeStampRTT()noexcept { m_interpolator.SetCurrentTimeStampRTT(); }
private:
	uint64_t m_objID = 0;
	MoveInterpolator m_interpolator;
};

