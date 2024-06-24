#include "pch.h"
#include "Status.h"
#include "c2s_PacketHandler.h"
#include "PacketSession.h"
#include "Object.h"
#include "TRWorldRoom.h"
#include "DBMgr.h"
#include "DBPacket.h"
#include "ClientSession.h"

using namespace ServerCore;

void Status::ModifyHP(const int val)
{
	if (0 > val)
	{
		if (0 >= m_hp.DecHP(val))
		{
			// TODO : »ç¸Á
		}
	}
	else
	{
		m_hp.IncHP(val);
	}
	SendStatPacket(Protocol::HP, val);
}

void Status::IncExp(const int val, const bool updateParty)
{
	for (;;)
	{
		int cur_level = m_level.load();
		int max_exp = m_maxExp.load();
		if (cur_level * 10 != max_exp)
			continue;
		int old_exp = m_exp.load();
		int new_exp = old_exp + val;

		if (max_exp <= new_exp)
		{
			if (m_level.compare_exchange_strong(cur_level, cur_level + 1))
			{
				const int new_max = max_exp + 10;
				m_maxExp = new_max;
				for (;;)
				{
					new_exp = old_exp + val;
					const int exp = new_exp - max_exp > 0 ? new_exp - max_exp : new_exp;
					if (m_exp.compare_exchange_strong(old_exp, exp))
					{
						Protocol::s2c_LEVEL_UP pkt;
						pkt.set_exp(exp);
						const auto entity = m_pOwner->GetIocpEntity();
						if (const auto session = entity->IsSession())
						{
							//session << pkt;
							session->SendAsync(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
						}
						s2q_UPDATE_PLAYER_INFO p;
						p.user_id = entity->GetObjectID();
						p.level = 1;
						RequestQueryServer(p);
						if (updateParty)
						{
							if(const auto party = ((ClientSession*)m_pOwner->GetIocpEntity().get())->m_partyOne.load())
								party->GetComp<Status>()->IncExp(val,false);
						}
						return;
					}
				}
			}
		}
		else
		{
			if (m_exp.compare_exchange_strong(old_exp, new_exp))
			{
				SendStatPacket(Protocol::EXP, val);
				if (updateParty)
				{
					if (const auto party = ((ClientSession*)m_pOwner->GetIocpEntity().get())->m_partyOne.load())
						party->GetComp<Status>()->IncExp(val, false);
				}
				return;
			}
		}
	}
}

void Status::HalfExp()
{
	int old_exp = m_exp.load();
	int new_exp = old_exp / 2;	
	while(!m_exp.compare_exchange_weak(old_exp, new_exp)) {
		new_exp = old_exp / 2;
	}
	const int temp = old_exp - new_exp;
	SendStatPacket(Protocol::EXP, temp);
}

void Status::ModifyGold(const int gold,const bool updateParty)
{
	m_gold += gold;
	SendStatPacket(Protocol::GOLD, gold);
	if (updateParty)
	{
		if (const auto party = ((ClientSession*)m_pOwner->GetIocpEntity().get())->m_partyOne.load())
			party->GetComp<Status>()->ModifyGold(gold, false);
	}
}

void Status::SendStatPacket(const Protocol::STAT s, const int val)
{
	const auto entity = m_pOwner->GetIocpEntity();
	if (const auto session = entity->IsSession())
	{
		Protocol::s2c_STAT_MODIFY pkt;
		pkt.set_stat(s);
		pkt.set_val(val);
		pkt.set_obj_id(entity->GetObjectID());
		for (const auto s : ((TRWorldRoom*)session->GetContentsEntity()->m_pCurSector.load())->GetAdjSector4()) {
		//	s << pkt;
			s->BroadCastEnqueue(ServerCore::c2s_PacketHandler::MakeSendBuffer(pkt));
		}
		
		s2q_UPDATE_PLAYER_INFO p;
		p.user_id = entity->GetObjectID();
		switch (s)
		{
		case Protocol::EXP:
		{
			p.experience = val;
			break;
		}
		case Protocol::HP:
		{
			p.hp = val;
			break;
		}
		case Protocol::GOLD:
		{
			p.gold = val;
			break;
		}
		default:
			break;
		}
		
		RequestQueryServer(p);
	}
}
