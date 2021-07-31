#include "stdhdrs.h"
#include "DBCmd.h"
#include "DratanCastle.h"

#ifdef DRATAN_CASTLE

#ifdef DYNAMIC_DUNGEON
CDungeon::CDungeon()
{
	m_nEnterFeeRate = 100;
	m_nHuntRate = 0;
	m_nMobRate = 50;
	m_nEnvRate = 50;
	m_pZone = 0;
	m_nChangeTime = 0;
}

CDungeon::~CDungeon()
{

}

void CDungeon::SetChangeTime(int nTime)
{
	m_nChangeTime = nTime;
}

CZone * CDungeon::GetZone()
{
	return m_pZone;
}

void CDungeon::ChangeFeeRate(int nRate)
{
	int newRate = nRate;
	if( newRate > 150 )
		newRate = 150;
	else if ( newRate < 70 )
		newRate = 70;

	SetFeeRate(newRate);
}

void CDungeon::ChangeHuntRate(int nRate)
{
	int newRate = nRate;
	if( newRate > 20 )
		newRate = 20;
	else if ( newRate < 0 )
		newRate = 0;

	SetHuntRate(newRate);
}

void CDungeon::ChangeMobRate(int nRate)
{
	int newRate = GetMobRate() + nRate;
	if( newRate > 100 )
		newRate = 100;
	else if ( newRate < 0 )
		newRate = 0;

	SetMobRate(newRate);
}

void CDungeon::ChangeEnvRate(int nRate)
{
	int newRate = GetEnvRate() + nRate;
	if( newRate > 100 )
		newRate = 100;
	else if ( newRate < 0 )
		newRate = 0;

	SetEnvRate(newRate);
}

int CDungeon::GetChangeTime()
{
	return m_nChangeTime;
}

void CDungeon::SetZone(CZone * pZone)
{
	m_pZone = pZone;
}

void CDungeon::ChangeSetting()
{
	if(m_pZone == 0)
	{
		return;
	}

	int nMobRate[10][4] = {	// 공격력, 방어력, 최대체력, 시야범위
		{20, 20, 30, 0},
		{10, 10, 15, 0},
		{5, 5, 10, 0},
		{0, 0, 0, 0},
		{-5, -5, -5, 0},
		{-7, -7, -7, 0},
		{-10, -10, -10, 0},
		{-15, -15, -15, 0},
		{-20, -20, -20, 0},
		{-25, -25, -25, -80},
	};

	// 몹 적용
	CNPC * npc = m_pZone->m_area->m_npcList;

	while(npc != 0)
	{
		int midx = GetMobRate()/10;
		if(midx > 9)
		{
			midx = 9;
		}

		// 공격력 적용
		if(npc->m_attackType == MSG_DAMAGE_MELEE)
		{
			npc->m_eqMelee = npc->m_proto->m_attack + npc->m_proto->m_attack * nMobRate[midx][0] / 100;
			if(npc->m_eqMelee < 0)
			{
				npc->m_eqMelee = 1;
			}
		}
		else if(npc->m_attackType == MSG_DAMAGE_RANGE)
		{
			npc->m_eqRange = npc->m_proto->m_attack + npc->m_proto->m_attack * nMobRate[midx][0] / 100;
			if(npc->m_eqRange < 0)
			{
				npc->m_eqRange = 1;
			}
		}
		else if(npc->m_attackType == MSG_DAMAGE_MAGIC)
		{
			npc->m_eqMagic = npc->m_proto->m_magic + npc->m_proto->m_magic * nMobRate[midx][0] / 100;
			if(npc->m_eqMagic < 0)
			{
				npc->m_eqMagic = 1;
			}
		}			

		// 방어력 적용
		npc->m_eqDefense = npc->m_proto->m_defense + npc->m_proto->m_defense * nMobRate[midx][1] / 100;

		// 최대 체력 적용
		npc->m_maxHP = npc->m_proto->m_hp + npc->m_proto->m_hp * nMobRate[midx][2] / 100;
		if(npc->m_maxHP < 0)
		{
			npc->m_maxHP = 1;
		}

		// 시야 거리 적용
		if(nMobRate[midx][3] != 0 && npc->m_attackRange != 0)
		{
			npc->m_attackRange = npc->m_proto->m_attackArea + npc->m_proto->m_attackArea * nMobRate[midx][3] / 100;
		}

		npc = npc->m_pNPCNext;
	}
}
#endif // DYNAMIC_DUNGEON

CGuardList::CGuardList()
{
	maxcount = 0;
	maxkind = 0;
	m_pGardList = NULL;
	m_cnt = 0;
	m_maxcnt = 0;
}

CGuardList::~CGuardList()
{
	if( m_pGardList != NULL)
	{
		delete [] m_pGardList;
		m_pGardList = NULL;
	}
}

void CGuardList::clean()
{
	if( m_pGardList != NULL)
	{
		memset(m_pGardList, 0, m_maxcnt*sizeof(GUARD));
	}
}

CGuardList::CGuardList(int kind, int count)
{
	init(kind, count);
}

void CGuardList::init(int kind, int count)
{
	maxcount = count;
	maxkind = kind;
	m_maxcnt = maxcount*maxkind;
	m_pGardList = new GUARD[maxcount*maxkind];

	m_cnt = 0;
}

bool CGuardList::InsertGuard(int idx, float x, float z, float h)
{
	if(m_cnt > m_maxcnt)
	{
		return false;
	}

	if(GetCount(idx) > maxcount)
	{
		return false;
	}
	
	m_pGardList[m_cnt].mobindex = idx;
	m_pGardList[m_cnt].pos_x = x;
	m_pGardList[m_cnt].pos_z = z;
	m_pGardList[m_cnt].pos_h = h;

	m_cnt++;

	return true;
}

int CGuardList::GetCount(int idx)
{
	int ret = 0;
	for(int i =0; i < m_maxcnt; i++)
	{
		if(m_pGardList[i].mobindex == idx)
		{
			ret++;
		}
	}

	return ret;
}

int CGuardList::GetCount()
{
	return m_cnt;
}

int CGuardList::GetMaxCount()
{
	return m_maxcnt;
}

GUARD * CGuardList::GetGuardList(int idx)
{
	if(idx >= m_cnt)
	{
		return NULL;
	}

	return  &m_pGardList[idx];
}

CDratanCastle * CDratanCastle::pInstance = 0;

CDratanCastle::CDratanCastle() : CWarCastle()
{
	memset(m_gateNPC, 0, sizeof(CNPC*) * COUNT_DRATAN_GATE_NPC);
	
	InitMasterTower();
	InitRespond();
	InitReinforce();
	InitWarpNPC();
	InitRebrithNPC();

	m_bMaster = false;
	m_bLoop = true;

	m_pDungeonNPC = NULL;
}

void CDratanCastle::InitRebrithNPC()
{
	for(int i = 0; i < 7; i++)
	{
		m_pRebrithNPC[i] = NULL;	
		m_nRebrithGuild[i] = -1;
		m_strRebrithGuild[i] = new char[51];
		memset((void *)m_strRebrithGuild[i], 0, 51);
	}
}

void CDratanCastle::ResetRebrithNPC()
{
	for(int i = 0; i < 7; i++)
	{	
		m_nRebrithGuild[i] = -1;
		memset((void *)m_strRebrithGuild[i], 0, 51);
	}
}

void CDratanCastle::InitWarpNPC()
{
	for(int i = 0; i < 5; i++)
	{
		m_pWarpNPC[i] = NULL;
	}
}

void CDratanCastle::InitReinforce()
{
	memset(m_reinforce, 0, 3);
}

void CDratanCastle::InitRespond()
{
	for(int i = 0; i < 5; i++)
	{
		if(m_RespondMember[i] != 0)
		{
			CPC * pc = gserver.m_playerList.Find(m_RespondMember[i]);
			if( pc != NULL)
			{
				pc->ResetPlayerState(PLAYER_CRISTAL_RESPOND);
				pc->SetRespondTime(0);
			}
			m_RespondMember[i] = 0;
		}
	}

	m_respondCnt = 0;
}

bool CDratanCastle::SetRespondMember(CPC * pc)
{
	if(m_respondCnt > 5)
	{
		return false;
	}
	else
	{
		for(int i = 0; i < 5; i++)
		{
			if(m_RespondMember[i] == 0)
			{
				m_RespondMember[i] = pc->m_index;
				return true;
			}
		}
	}

	return false;
}

void CDratanCastle::ResetRespondMeber(CPC * pc)
{
	for(int i = 0; i < 5; i++)
	{
		if(m_RespondMember[i] == pc->m_index)
		{
			m_RespondMember[i] = 0;
			m_respondCnt--;

			pc->ResetPlayerState(PLAYER_CRISTAL_RESPOND);
			pc->SetRespondTime(0);

			return;
		}
	}
}

void CDratanCastle::InitMasterTower()
{
	memset( m_nMasterTower, 0, 7);
}

char CDratanCastle::GetTowerStatus(int i)
{
	if( i < 7 
		&& m_nMasterTower[i] <= 5)
	{
		return m_nMasterTower[i];
	}

	return 0;
}

bool CDratanCastle::SetTowerStatus(int i, char values)
{
	if( i < 7 
		&& values <= 5)
	{
		m_nMasterTower[i] = values;
		return true;
	}
	else
	{
		return false;
	}
}

void CDratanCastle::SetNextWarTime(int nextWarTime)
{
	time_t curtime;
	time(&curtime);

	// 공성 시작 시간이 현재 시간이후이면 그대로 적용
	if (GetState() == WCSF_NORMAL && nextWarTime != 0)
	{
		if (nextWarTime > curtime)
		{
			m_nextWarTime = nextWarTime;
			return ;
		}
		else
		{
			GAMELOG << init("WAR CASTLE : WARN SET TIME")
					<< "CUR TIME : " << (int)curtime << delim
					<< "NEXT TIME : " << nextWarTime
					<< end;
		}
	}

	// 공성 시간을 기본 값으로 설정
	// 현재 요일을 구함
	struct tm curtm = NOW();

#if defined (LC_KOR) || defined (LC_MAL) || defined (LC_BRZ) 
	// 다음 일요일을 구함
	int next = 7 - curtm.tm_wday;
	if (next < 7)
		next += 7;
#else
	// 다음 토요일을 구함
	int next = 6 - curtm.tm_wday;
	if (next <= 0)
		next += 7;
#endif

	// 다음 토요일 20시에 해당하는 time_t 값을 구함
	struct tm nexttm = curtm;

	nexttm.tm_hour = 20;
	nexttm.tm_min = 0;
	nexttm.tm_sec = 0;
	nexttm.tm_mday += next;
	time_t nexttime = mktime(&nexttm);

#ifdef LC_JPN  // 일본의 경우 드라탄 공성전 자동 셋팅 안되게~
	// 일본은 1000일을 늘려버림..
	nexttm.tm_mday += 1000;
	nexttime = mktime(&nexttm);
#endif

	// 현재 시간보다 작으면 다음주로
	if (nexttime < curtime)
	{
		nexttm.tm_mday += 7;
		nexttime = mktime(&nexttm);
	}

	m_nextWarTime = nexttime;

	if (nextWarTime != 0)
	{
		GAMELOG << init("WAR CASTLE : SET NEXT TIME FOR LOAD")
				<< "IN : " << nextWarTime << delim
				<< "SET : " << m_nextWarTime
				<< end;
	}
}

bool CDratanCastle::IsJoinTime()
{
#ifndef TEST_SERVER
	if (m_bMoveToRegen)
		return false;

	struct tm tmJoinTime;
	GetJoinTime(&tmJoinTime, false);
	time_t jointime = mktime(&tmJoinTime);

	// 마지막 공성시간과 비교
	time_t curtime;
	time(&curtime);
	if (m_lastWarTime < curtime && curtime < jointime)
		return false;
#endif // KJTEST
	return true;
}

void CDratanCastle::GetJoinTime(struct tm* joinTime, bool bHumanable)
{
	// 다음 공성을 구하고
	struct tm tmNext;
	GetNextWarTime(&tmNext, false);

	// 월요일 구하기
	int nDay = tmNext.tm_wday;
	if (nDay == 0)
		nDay = 7;
	nDay -= 1;

	// 다음 공성에서 nDay일을 빼고
	// 그 날 20시로 설정 : 7시 
	tmNext.tm_hour = 20;
	tmNext.tm_min = 0;
	tmNext.tm_sec = 0;
	tmNext.tm_mday -= nDay;
	time_t tJoin = mktime(&tmNext);

	*joinTime = *localtime(&tJoin);

	if (bHumanable)
	{
		joinTime->tm_year += 1900;
		joinTime->tm_mon += 1;
	}
}

int CDratanCastle::GetRegenPoint(int joinflag, CPC* ch)
{
	switch (joinflag)
	{
	case WCJF_OWNER:
	case WCJF_DEFENSE_GUILD:
		return 25;

	case WCJF_ATTACK_GUILD:
		if (ch && ch->m_guildInfo && ch->m_guildInfo->guild())
		{
			int i;
			for (i = 0; i < 7; i++)
			{
				if (m_nRebrithGuild[i] == ch->m_guildInfo->guild()->index())
				{
					switch (i)
					{	///===
					case 0:		return 18;
					case 1:		return 19;
					case 2:		return 20;
					case 3:		return 21;
					case 4:		return 22;
					case 5:		return 23;
					case 6:		return 24;
					}
				}
			}
		}
		return 27;

	default:
		return 27;
	}
}

void CDratanCastle::SetRegenPoint(int nRegenPos, int nGuildIndex)
{
	if (nRegenPos > 9)
	{
		return;
	}

	m_posRegenPoint[nRegenPos] = nGuildIndex;
}

void CDratanCastle::SetGateNPC(CNPC* npc)
{
	if (npc->m_proto->CheckFlag(NPC_CASTLE_TOWER))
	{
		switch (npc->m_proto->m_index)
		{
		case 388:	m_gateNPC[0] = npc;	return;		// 외성문
		case 389:	m_gateNPC[1] = npc;	return;		// 내성문 1
		case 404:	m_gateNPC[2] = npc;	return;		// 내성문 2

		default:			return ;
		}
	}
}

void CDratanCastle::SetWarpNPC(CNPC* npc)
{
	if (npc->m_proto->CheckFlag(NPC_CASTLE_TOWER)
		&& npc->m_proto->m_index >= 382
		&& npc->m_proto->m_index <= 386)
	{
		m_pWarpNPC[npc->m_proto->m_index - 382] = npc;
	}
}

void CDratanCastle::SetRebrithNPC(CNPC* npc)
{
	if (npc->m_proto->CheckFlag(NPC_WARCASTLE)
		&& npc->m_proto->CheckFlag(NPC_CASTLE_TOWER)
		&& npc->m_proto->m_index >= 390
		&& npc->m_proto->m_index <= 396)
	{
		if( m_pRebrithNPC[npc->m_proto->m_index - 390] == NULL )
		{
			m_pRebrithNPC[npc->m_proto->m_index - 390] = npc;
		}
	}
}

void CDratanCastle::UpdateGateState(CNPC* npc)
{
	int i;
	for (i = 0; i < COUNT_DRATAN_GATE_NPC; i++)
	{
		if (m_gateNPC[i] == npc)
		{
			m_gateState |= (1 << (i + 6));
			
			GAMELOG << init("DRATAN GATE NPC DESTROY")
				<< npc->m_proto->m_index << end;
			return;
		}
	}
}

bool CDratanCastle::IsOpenGate()
{
	// 외성문
	int outer = (1 << 6);

	// 내성문 1
	int inner1 = (1 << 7);

	// 내성문 2
	int inner2 = (1 << 8);

	if (GetGateState(outer) == outer)
		return true;

	if (GetGateState(inner1) == inner1)
		return true;

	if (GetGateState(inner2) == inner2)
		return true;

	return false;
}

bool CDratanCastle::SetNextWarTime(int wday, int hour)
{
	int minHour = 9999;
	int maxHour = -1;
	switch (wday)
	{
	case 0:		// 일요일 : 13 ~ 23
#ifdef LC_USA	// 미국 : 11 ~ 20
		minHour = 11;
		maxHour = 20;
#else
		minHour = 13;
		maxHour = 23;
#endif // LC_USA
		break;

	case 6:		// 토요일 : 18 ~ 23
#ifdef LC_USA	// 미국 : 11 ~ 23
		minHour = 11;
		maxHour = 23;
#else
		minHour = 18;
		maxHour = 23;
#endif // LC_USA
		break;

	default:
		return false;
	}
	if (hour < minHour || hour > maxHour)
		return false;

	time_t curtime;
	time(&curtime);
	struct tm curtm = NOW();

	// 다음 해당 요일까지 차이를 구함
	int next = wday - curtm.tm_wday;
	if (next <= 0)
		next += 7;
	if (next <= 4)
		next += 7;

	// 그날 해당 시간의 time_t 값을 구함
	struct tm nexttm = curtm;
	nexttm.tm_hour = hour;
	nexttm.tm_min = 0;
	nexttm.tm_sec = 0;
	nexttm.tm_mday += next;
	time_t nexttime = mktime(&nexttm);

	// 현재 시간보다 작으면 ?	if (nexttime < curtime)
		return false;

#ifdef LC_JPN  // 일본의 경우 드라탄 공성전 자동 셋팅 안되게~
	return false;
#else
	m_nextWarTime = nexttime;
#endif

	return true;
}

bool CDratanCastle::GiveLordItem(CPC* pc)
{
	CNetMsg rmsg;
	CItem* lordItem = NULL;

	int lordItemIndex = GetLordItemIndex(pc->m_job, 1);
	int r, c;
	if (pc->m_invenNormal.FindItem(&r, &c, lordItemIndex, 0, 0))
	{
		m_bGiveLordWeapon = true;
		return true;
	}
	lordItemIndex = GetLordItemIndex(pc->m_job, 2);
	if (pc->m_invenNormal.FindItem(&r, &c, lordItemIndex, 0, 0))
	{
		m_bGiveLordWeapon = true;
		return true;
	}
	lordItemIndex = GetLordItemIndex(pc->m_job, pc->m_job2);

	if (lordItemIndex > 0)
	{
		lordItem = gserver.m_itemProtoList.CreateItem(lordItemIndex, -1, 0, 0, 1);
		if (lordItem)
		{
			if (AddToInventory(pc, lordItem, true, true))
			{
				ItemAddMsg(rmsg, lordItem);
				SEND_Q(rmsg, pc->m_desc);
			}
			else
			{
				delete lordItem;
				lordItem = NULL;
			}
		}
	}

	if (lordItem == NULL)
	{
		GAMELOG << init("WAR CASTLE : CANT GIVE WEAPON", pc)
				<< "ZONE" << delim
				<< GetZoneIndex() << delim
				<< "JOB" << delim
				<< pc->m_job << delim
				<< pc->m_job2
				<< end;

		return false;
	}
	else
	{
		GAMELOG << init("WAR CASTLE : GIVE WEAPON", pc)
				<< "ZONE" << delim
				<< GetZoneIndex() << delim
				<< "JOB" << delim
				<< pc->m_job << delim
				<< pc->m_job2 << delim
				<< "ITEM" << delim
				<< itemlog(lordItem)
				<< end;

		m_bGiveLordWeapon = true;

		return true;
	}
}

bool CDratanCastle::TakeLordItem(CPC* pc)
{
	CNetMsg rmsg;
	int r, c;
	if (pc->m_invenNormal.FindItem(&r, &c, GetLordItemIndex(pc->m_job, 1), 0, 0) ||
		pc->m_invenNormal.FindItem(&r, &c, GetLordItemIndex(pc->m_job, 2), 0, 0))
	{
		CItem* item = pc->m_invenNormal.GetItem(r, c);
		if (item->m_wearPos != WEARING_NONE)
		{
			ItemWearMsg(rmsg, item->m_wearPos, NULL, item);
			SEND_Q(rmsg, pc->m_desc);
			if (item->m_wearPos >= WEARING_SHOW_START && item->m_wearPos <= WEARING_SHOW_END && pc->m_pArea)
			{
				WearingMsg(rmsg, pc, item->m_wearPos, -1, 0);
				pc->m_pArea->SendToCell(rmsg, pc, true);
			}
			pc->m_wearing[item->m_wearPos] = NULL;
			item->m_wearPos = WEARING_NONE;
		}

		ItemDeleteMsg(rmsg, item);
		SEND_Q(rmsg, pc->m_desc);
		RemoveFromInventory(pc, item, true, true);
	}

	if (GetOwnerCharIndex() == pc->m_index)
	{
		m_bGiveLordWeapon = false;
	}
	return true;
}

int CDratanCastle::GetLordItemIndex(char job1, char job2)
{
	if (job2 == 0)
	{
		job2 = GetRandom(1, 2);
	}
	if (job2 < 1 || job2 > 2)
		return -1;

	const int itemindex[JOBCOUNT][2] = {
		{861, 862},
		{864, 863},
		{865, 866},
		{868, 867},
		{869, 870},
		{1070, 1071},
// NIGHT_SHADOW 공성 무기 필요
#ifdef NIGHT_SHADOW
		{864, 863},
#endif //NIGHT_SHADOW
	};

	switch (job1)
	{
	case JOB_TITAN:
	case JOB_KNIGHT:
	case JOB_HEALER:
	case JOB_MAGE:
	case JOB_ROGUE:
	case JOB_SORCERER:
#ifdef NIGHT_SHADOW
	case JOB_NIGHTSHADOW:
#endif //NIGHT_SHADOW
		return itemindex[job1][job2 - 1];
	default:
		return -1;
	}
}

bool CDratanCastle::IsDefenseWarpPoint(int pos)
{
	if (pos < 13 || pos > 17)
	{
		return false;
	}
	
	return true;
}

void CDratanCastle::InitGateNPC()
{
	memset(m_gateNPC, 0, sizeof(CNPC*) * COUNT_DRATAN_GATE_NPC);
	m_gateState = 0;
}

void CDratanCastle::GetInnerCastleRect(char* nYlayer, int* nX0x2, int* nZ0x2, int* nX1x2, int* nZ1x2)
{
	int i = gserver.FindZone(GetZoneIndex());
	if (i == -1)
		return ;

	CZone*	pZone	= gserver.m_zones + i;

	*nYlayer		= (char)(pZone->m_zonePos[26][0]);
	*nX0x2			= pZone->m_zonePos[26][1];
	*nZ0x2			= pZone->m_zonePos[26][2];
	*nX1x2			= pZone->m_zonePos[26][3];
	*nZ1x2			= pZone->m_zonePos[26][4];
}


CDratanCastle::CDratanCastle(const CDratanCastle & rhs) 
: CWarCastle()
{

}

CDratanCastle * CDratanCastle::CreateInstance()
{
	if (pInstance == NULL)
	{
		pInstance = new CDratanCastle();
	}

	return pInstance;
}

void CDratanCastle::CheckWarCastle()
{	///=== 공성 막기 kjtest
//#ifdef KJTEST	
#ifdef DYNAMIC_DUNGEON
	// 공성 던전 난이도 조정
	CheckDVD();
#endif // DYNAMIC_DUNGEON

	CHECK_SUBSERVER(this);

	// 마스터 타워, 부활진지 
	CheckCastleControlNPC();

	// 공성 시작 알림
	CheckWarCastleNotice();

	// 공성 시작
	CheckWarCastleStartWarCastle();

	// 공성 끝
	CheckWarCastleEnd();

	// 공성 NPC 처리
	CheckWarCastleNPC();

	// 권좌 생성
	RegenLordSymbol();

	// 상인 생성
	RegenShop();
//#endif // KJTEST
}

#ifdef DYNAMIC_DUNGEON
void CDratanCastle::CheckDVD()
{
#ifdef TEST_SERVER
	if(gserver.m_pulse - m_dvd.GetChangeTime() >= PULSE_REAL_MIN)		// 10분 마다 10% 하락
#else //TEST_SERVER
	#if defined(LC_KOR) || defined ( LC_BRZ ) 
		if(gserver.m_pulse - m_dvd.GetChangeTime() >= PULSE_REAL_HOUR*6)	// 6시간 마다 10% 하락
	#else
		if(gserver.m_pulse - m_dvd.GetChangeTime() >= PULSE_REAL_HOUR)		// 한시간 마다 10% 하락
	#endif
#endif //TEST_SERVER	
	{	
		CNetMsg rmsg;
		
		if(m_dvd.GetMobRate() - 10 > 0)
		{
			m_dvd.ChangeMobRate(-10);
		}
		else
		{
			m_dvd.SetMobRate(0);
		}

		if(m_dvd.GetEnvRate() - 10 > 0)
		{
			m_dvd.ChangeEnvRate(-10);
		}
		else
		{
			m_dvd.SetEnvRate(0);
		}
		//
		m_dvd.SetChangeTime(gserver.m_pulse);

		//
		m_dvd.ChangeSetting();
		
		GAMELOG << init("DRATAN DYNAMIC DUNGEON EVN CHANGE") 
				<< "Env:" << m_dvd.GetEnvRate() << delim
				<< "Mob:" << m_dvd.GetMobRate() << end;
		
		HelperDVDRateChangeMsg(rmsg, MSG_MANAGEMENT_ENV_CHANGE , m_dvd.GetEnvRate() );
		SEND_Q(rmsg, gserver.m_helper);

		HelperDVDRateChangeMsg(rmsg, MSG_MANAGEMENT_STATE_CHANGE , m_dvd.GetMobRate() );
		SEND_Q(rmsg, gserver.m_helper);
	}
}
#endif // DYNAMIC_DUNGEON

bool CDratanCastle::CheckSubServer()
{
	return (GetZoneIndex() == GetCurSubServerCastleZoneIndex());
}

int CDratanCastle::GetCurSubServerCastleZoneIndex()
{
	switch (gserver.m_subno)
	{
	case WAR_CASTLE_SUBNUMBER_DRATAN:
		return ZONE_DRATAN;

	default:
		return -1;
	}
}

int CDratanCastle::CheckJoin(CPC* ch)
{
	if (ch->m_guildInfo)
	{
		CGuild * guild = ch->m_guildInfo->guild();
		// 성주 길드 검사
		if (guild->index() == GetOwnerGuildIndex())
			return WCJF_OWNER;
	
		// 공성 길드 검사
		if (IsAttackGuild(guild->index()))
			return WCJF_ATTACK_GUILD;
	}

	return WCJF_NONE;
}

// 공성 시작 알림
void CDratanCastle::CheckWarCastleNotice()
{
	switch (GetState())
	{
	case WCSF_NORMAL:
		// 확정 알리는 것인지 아니면 시작 10분전에 알리는 것인지 검사
		if (!m_bNoticeWarTime)
		{
			// 공성 신청 가능 시간대이고
			if (!IsJoinTime())
				return ;

			// 게임내 모든 유저에게 알리기
			if (IS_RUNNING_HELPER)
			{
				struct tm nextWarTime;
				GetNextWarTime(&nextWarTime, true);

				CNetMsg rmsg;
				HelperWarNoticeTimeMsg(rmsg, GetZoneIndex(), nextWarTime.tm_mon, nextWarTime.tm_mday, nextWarTime.tm_hour, nextWarTime.tm_min);
				SEND_Q(rmsg, gserver.m_helper);
				m_bNoticeWarTime = true;
			}
		}
		else
		{
			struct tm nextWarTime;
			GetNextWarTime(&nextWarTime, true);
			int nexttime = GetNextWarTime();
			time_t curtime;
			time(&curtime);

			// 시작 10분전에 매분마다
			if (curtime + 10 * 60 >= nexttime)
			{
				// 남은 시간 구하기
				int remain = (nexttime - curtime) / 60;
				int modnum = (nexttime - curtime) % 60;
				if (modnum)
					remain++;
				if (remain != 0 && m_noticeRemain > remain && IS_RUNNING_HELPER)
				{
					CNetMsg rmsg;
					HelperWarNoticeRemainMsg(rmsg, GetZoneIndex(), remain);
					SEND_Q(rmsg, gserver.m_helper);
					m_noticeRemain = remain;

					// 길드전 중지 요청
					if (curtime + 60 >= nexttime)
					{
						// 서버내의 길드 리스트를 돌면서
						// 길드전 중이면 헬퍼에 요청
						CNetMsg stopmsg;
						CGuild* p;
						for (p = gserver.m_guildlist.head(); p; p = p->nextguild())
						{
							if (p->battleZone() == GetZoneIndex())
							{
								switch (p->battleState())
								{
								case GUILD_BATTLE_STATE_WAIT:
								case GUILD_BATTLE_STATE_ING:
								case GUILD_BATTLE_STATE_STOP_WAIT:
									if (IS_RUNNING_HELPER)
									{
										HelperGuildBattleStopReqMsg(stopmsg, p->index(), p->battleIndex());
										SEND_Q(stopmsg, gserver.m_helper);
									}
									break;
								} // switch (p->battleState())
							} // if (p->battleZone() == GetZoneIndex())
						} // for (p = gserver.m_guildlist.head(); p; p = p->nextguild())
					} // if (curtime + 60 >= nexttime)
				} // if (remain != 0 && m_noticeRemain > remain && IS_RUNNING_HELPER)
			} // if (curtime + 10 * 60 >= nexttime)

			// 시작 3분전 이동
			if (!m_bMoveToRegen && curtime + 3 * 60 >= nexttime)
			{
				// 리젠 포인트 할당
				InitRegenPosition();

				// 강제 존이동 시키기
				m_bMoveToRegen = true;
				MoveToRegenPoint();
			}

			// 공성전 시작 1분 전부터는 메모리 스크롤을 사용할 수 없도록 하기 위해..
			if (!m_bWarCastleReady && (curtime + 1 * 60) >= nexttime)
			{
				m_bWarCastleReady = true;
			}
		}
		break;

	case WCSF_WAR_CASTLE:
		{
			if (m_bWarCastleReady)
				m_bWarCastleReady = false;
		} break;
	}

}

// 공성 시작
void CDratanCastle::CheckWarCastleStartWarCastle()
{
	// 야전 상태 검사
	if (GetState() != WCSF_NORMAL)
		return;

	// 공성 시작 시간이상이어야 하고
	time_t curTime;
	time(&curTime);
	int nextTime = GetNextWarTime();
	if (nextTime > curTime)
		return ;

	// 알림 메시지 시간 저장 변수를 초기화
	m_noticeRemain = 9999;

	// 공성 신청 길드가 없으면 다음으로 넘어감
	if (m_attackGuildList == NULL)
	{
		SetState(WCSF_WAR_CASTLE);
		GAMELOG << init("WAR CASTLE : SKIP WAR") << GetZoneIndex() << end;
		return;
	}

	// 수성측 제외 성에서 내보내기
	GetOutNonDefense();

	// 리젠 포인트 할당
	InitRegenPosition();

	// 강제 존이동 시키기
	m_bMoveToRegen = true;
	MoveToRegenPoint();

	// 상태를 바꾸고
	SetState(WCSF_WAR_CASTLE);

	// 워프 게이트 생성
//	RegenWarpNPC();		// 수호탑 생성 할때 같이 생성

	// 수호탑 생성
	RegenCastleTower();

	GAMELOG << init("WAR CASTLE : START WAR")
			<< GetZoneIndex() << end;
	
	// 참여 길드 정보 기록
	CWarCastleJoin * pJoin = m_attackGuildList;
	while( pJoin != NULL )
	{
		GAMELOG << init("WAR CASTLE : JOIN ATTACK GUILD")
			<< pJoin->GetIndex() << end;
		pJoin = pJoin->GetNext();
	}
	
	pJoin = m_defenseGuildList;
	while( pJoin != NULL )
	{
		GAMELOG << init("WAR CASTLE : JOIN DEFANSE GUILD")
			<< pJoin->GetIndex() << end;
		pJoin = pJoin->GetNext();
	}

	// 마스터 타워 설정 로그
	GAMELOG << init("WAR CASTLE : REINFORCE SETTING");
	for( int i = 0; i < 7; i++ )
	{
		 
		GAMELOG << delim << (int)m_reinforce[i];
	}
	GAMELOG << end;

		
	// 알림 메시지 시간 저장 변수를 초기화
	m_noticeRemain = 9999;

	// 운영자 명령으로 멈춤 초기화
	m_bLoop = true;

	// 공성 시작을 헬퍼를 통해 알리기
	CNetMsg rmsg;
	if (IS_RUNNING_HELPER)
	{
		HelperWarNoticeStartAttackCastleMsg(rmsg, GetZoneIndex(), GetRemainWarTime(), 0, 0, 0);
		SEND_Q(rmsg, gserver.m_helper);
	}
	
	// 부활 진지 정보 전송
	CastletowerQuartersListMsg(rmsg, this);
	GetArea()->SendToAllClient(rmsg);

#ifdef DYNAMIC_DUNGEON
	// 공성 보상 던전의 유저 강제 이동
	MoveingDVD();
#endif // DYNAMIC_DUNGEON
}

#ifdef DYNAMIC_DUNGEON
// 공성 보상 던전의 유저 강제 이동
void CDratanCastle::MoveingDVD()
{	// 공성 대기 간이 마을로 이동
	if (m_dvd.GetZone() == 0)
	{
		return;
	}

	m_dvd.GetZone()->m_area->MoveAllChar( ZONE_DRATAN, 27 );
}
#endif // DYNAMIC_DUNGEON

void CDratanCastle::CheckWarCastleEnd()
{
	// 진행 상태 검사
	if (GetState() != WCSF_WAR_CASTLE)
		return;

	bool bFinished = false;

	// 시간이 종료되었나?

	int endWarTime = GetNextWarTime() + CDratanCastle::WAR_TIME;

	time_t curTime;
	time(&curTime);
	if (endWarTime  <= curTime)
	{
		bFinished = true;
	}

	// 공성 신청 길드가 없으면 다음으로 넘어감
	if (m_attackGuildList == NULL)
	{		
		bFinished = true;
	}

	if (bFinished == false && m_bLoop == true)
		return;

	///== 승리자 구함
	int bWinDefense = GetOwnerGuildIndex();
	

	// 권좌/수호탑/워프게이트/가디언 삭제
	RemoveLordSymbol();
	RemoveCastleTower();
//	RemoveWarpNPC();
	RemoveGuardNPC();

	// 성 관리인  리젠
	RegenCastleNPC();
	
	GAMELOG << init("WAR CASTLE : END WAR")
			<< GetZoneIndex();

	// 성 정보 초기화
	SetState(WCSF_NORMAL);
	m_lastWarTime = endWarTime;
	SetNextWarTime(0);

	m_bNoticeWarTime = false;
	m_noticeRemain = 9999;
	m_gateState = 0;
	m_bMoveToRegen = false;

	m_countAttackGuild = 0;

	// DB 초기화
	CDBCmd cmd;
	cmd.Init(&gserver.m_dbcastle);

	sprintf(g_buf,
			"UPDATE t_castle"
			" SET a_last_war_time = %d, a_state = 0, a_next_war_time = %d, a_owner_guild_index = %d, a_owner_guild_name = '%s', a_owner_char_index=%d, a_owner_char_name='%s'"
			" WHERE a_zone_index = %d"
			, m_lastWarTime, GetNextWarTime(), GetOwnerGuildIndex(), GetOwnerGuildName(), GetOwnerCharIndex(), GetOwnerCharName()
			, GetZoneIndex());
	cmd.SetQuery(g_buf);
	if (!cmd.Update())
	{
		GAMELOG << init("SYS_ERR: CANNOT SAVE CASTLE INFO")
				<< "ZONE" << delim
				<< GetZoneIndex() << delim
				<< "OWNER" << delim
				<< GetOwnerGuildIndex() << delim
				<< GetOwnerGuildName() << delim
				<< "LASTTIME" << delim
				<< m_lastWarTime
				<< end;
	}
	else
	{
		GAMELOG << init("WAR CASTLE : SAVE WAR END")
				<< "ZONE"			<< delim << GetZoneIndex()											<< delim
				<< "LAST TIME"		<< delim << m_lastWarTime											<< delim
				<< "NEXT TIME"		<< delim << m_nextWarTime											<< delim
				<< "OWNER GUILD"	<< delim << GetOwnerGuildIndex() << delim << GetOwnerGuildName()	<< delim
				<< "OWNER CHAR"		<< delim << GetOwnerCharIndex() << delim << GetOwnerCharName()
				<< end;
	}

	sprintf(g_buf,
			"DELETE"
			" FROM t_castle_join"
			" WHERE a_zone_index = %d"
			, GetZoneIndex());
	cmd.SetQuery(g_buf);
	if (!cmd.Update())
	{
		GAMELOG << init("SYS_ERR: CANNOT DELETE CASTLE JOIN INFO")
				<< "ZONE" << delim
				<< GetZoneIndex()
				<< end;
	}

	//  가디언 초기화
	m_GuardList.clean();
	sprintf(g_buf,
		"DELETE"
		" FROM t_castle_guard"
		" WHERE a_zone_num = %d"
		, GetZoneIndex());
	cmd.SetQuery(g_buf);
	if (!cmd.Update())
	{
		GAMELOG << init("SYS_ERR: CANNOT DELETE CASTLE GUARD INFO")
				<< "ZONE" << delim
				<< GetZoneIndex()
				<< end;
	}

	// 참여정보 초기화
	ResetJoinFlag();

	// 참여 리스트 초기화
	RemoveAllJoinList();

	// 마스터 타워 초기화
	InitMasterTower();
	sprintf(g_buf,
		"DELETE"
		" FROM t_castle_tower"
		" WHERE a_zone_num = %d"
		, GetZoneIndex());
	cmd.SetQuery(g_buf);
	if (!cmd.Update())
	{
		GAMELOG << init("SYS_ERR: CANNOT DELETE CASTLE GUARD INFO")
				<< "ZONE" << delim
				<< GetZoneIndex()
				<< end;
	}

	// 수호탑 강화 초기화
	InitReinforce();
	sprintf(g_buf,
		"DELETE"
		" FROM t_castle_reinfoce"
		" WHERE a_zone_num = %d"
		, GetZoneIndex());
	cmd.SetQuery(g_buf);
	if (!cmd.Update())
	{
		GAMELOG << init("SYS_ERR: CANNOT DELETE CASTLE TOWER REINFORCE")
				<< "ZONE" << delim
				<< GetZoneIndex()
				<< end;
	}

	// 부활 진지 초기화
	ResetRebrithNPC();
	sprintf(g_buf,
		"DELETE"
		" FROM t_castle_rebrith"
		" WHERE a_zone_num = %d"
		, GetZoneIndex());
	cmd.SetQuery(g_buf);
	if (!cmd.Update())
	{
		GAMELOG << init("SYS_ERR: CANNOT DELETE CASTLE REBRITH")
				<< "ZONE" << delim
				<< GetZoneIndex()
				<< end;
	}

	// 마스터 타워와 부활진지 다시 생성
	RemoveCastleControlNPC();
	RegenCastleControlNPC();

	// 공성 결과 알리기
	struct tm tmJoin;
	GetJoinTime(&tmJoin, true);
	CNetMsg rmsg;
	if (IS_RUNNING_HELPER)
	{
		HelperWarNoticeEndMsg(rmsg, GetZoneIndex(), bWinDefense, GetOwnerGuildIndex(), GetOwnerGuildName(), GetOwnerCharIndex(), GetOwnerCharName(), tmJoin.tm_mon, tmJoin.tm_mday, tmJoin.tm_hour, tmJoin.tm_wday); 
		SEND_Q(rmsg, gserver.m_helper);
	}
	else
	{
		GuildWarEndMsg(rmsg, GetZoneIndex(), ((bWinDefense) ? (char)1 : (char)0), GetOwnerGuildIndex(), GetOwnerGuildName(), GetOwnerCharIndex(), GetOwnerCharName(), tmJoin.tm_mon, tmJoin.tm_mday, tmJoin.tm_hour, tmJoin.tm_wday);
		gserver.m_playerList.SendToAll(rmsg);
		
		
		EndWarRegenPoint();

		for( int i = 0; i < gserver.m_playerList.m_max; i++ )
		{
			CPC* pc = gserver.m_playerList.m_pcList[i];
			if (pc)
			{
				// 바로 보내기
				CNetMsg rmsg;		
				StatusMsg(rmsg, pc);
				SEND_Q(rmsg, pc->m_desc );
			}
		}

	}

#ifdef ADULT_SERVER
	// 공성 후 진영 변경 반영
	CGuild * pGuild = gserver.m_guildlist.findguild(GetOwnerGuildIndex());
	if( pGuild != NULL)
	{
		// 마을속성 0 :일반, 1:헌터, 2:카오
		int mainV = 0, subV = 0;
		switch(pGuild->GetIncline())
		{
		case  GUILD_INCLINE_GENERAL: // 일반 길드 0
			mainV = 1, subV = 2;
			break;

		case GUILD_INCLINE_CHAO: // 카오 길드 1
			mainV = 2, subV = 1;
			break;

		case GUILD_INCLINE_HUNTER: // 헌터 길드 2
			mainV = 1, subV = 2;
			break;
		}

		int nZoneIndex[2] = { ZONE_DRATAN, ZONE_START };
		CNetMsg rmsg;

		for( int changeZoneIndex=0; changeZoneIndex<2 ; changeZoneIndex++ )
		{
			rmsg.Init( MSG_HELPER_COMMAND );

			int nIndex = gserver.FindZone(nZoneIndex[changeZoneIndex]);
			if( nIndex >= 0 )
			{
				int count = gserver.m_zones[ nIndex ].m_CountChaoAttZonePos;
				rmsg << MSG_HELPER_ZONE_CHAO_ATTR 
					 << (char) MSG_HELPER_ZONE_CHAO_ATT_CHANGE
					 << nZoneIndex[changeZoneIndex] << count;
				for( int i=0; i<count; i++ )
				{
					char cAtt = ( i % 2 == 0 ) ? mainV : subV;
					rmsg << cAtt;
				}
				SEND_Q( rmsg, gserver.m_helper );
			}								
		}
	}
#endif // ADULT_SERVER
}

// 공성전 기간 동안 성 관리인 삭제
void CDratanCastle::CheckWarCastleNPC()
{
	CHECK_SUBSERVER(this);

	switch (m_state)
	{
	case WCSF_NORMAL:
		if (m_castleNPC == NULL || m_pDungeonNPC == NULL)
			RegenCastleNPC();
		break;

	default:
		if (m_castleNPC != NULL)
			RemoveCastleNPC();
		if ( m_pDungeonNPC != NULL )
			RemoveDungeonNPC();
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// 리젠 관련
//////////////////////////////////////////////////////////////////////////

void CDratanCastle::CheckCastleControlNPC()
{
	if(m_bMaster == false)
	{
		RegenCastleControlNPC();
		m_bMaster = true;
	}
}

void CDratanCastle::RegenCastleControlNPC()
{	// 마스터 타워와 부활진지
	CHECK_SUBSERVER(this);

	CArea* area = GetArea();
	if (!area)
		return ;

	for (int i = 0; i < area->m_npcRegenList.m_nCount; i++)
	{
		CNPCRegenInfo* p = area->m_npcRegenList.m_infoList + i;
		if (p->m_totalNum != 0)
			continue ;

		CNPCProto* proto = gserver.m_npcProtoList.FindProto(p->m_npcIdx);
		if (proto != NULL 
			&& proto->CheckFlag(NPC_WARCASTLE) != 0 
			&& proto->CheckFlag(NPC_CASTLE_TOWER) != 0)
		{
			CNPC * npc = gserver.m_npcProtoList.Create(proto->m_index, p);
			if (!npc)
				return ;

			GET_X(npc) = p->m_regenX;
			GET_Z(npc) = p->m_regenZ;
			GET_R(npc) = p->m_regenR;
			GET_YLAYER(npc) = p->m_regenY;

			npc->m_regenX = GET_X(npc);
			npc->m_regenZ = GET_Z(npc);
			npc->m_regenY = GET_YLAYER(npc);

			npc->m_recoverPulse = gserver.m_pulse;
#ifndef NEW_DIVISION_EXPSP
			npc->m_totalDamage = 0;
#endif // #ifndef NEW_DIVISION_EXPSP

			p->m_bAlive = true;
			p->m_lastDieTime = 0;
			npc->m_disableTime = 0;

			npc->InitPointsToMax();
			area->AddNPC(npc);
			npc->m_postregendelay = 0;

			int cx, cz;
			area->PointToCellNum(GET_X(npc), GET_Z(npc), &cx, &cz);
			area->CharToCell(npc, GET_YLAYER(npc), cx, cz);

			CNetMsg rmsg;
			AppearMsg(rmsg, npc, true);
			area->SendToCell(rmsg, npc);
			
			// 부활 진지 등록
			SetRebrithNPC(npc);
		}
	}
}

void CDratanCastle::RemoveCastleControlNPC()
{
	CHECK_SUBSERVER(this);

	CArea* area = GetArea();
	if (!area)
		return ;

	CNPC* pNext = area->m_npcList;
	CNPC* p;
	while ((p = pNext))
	{
		pNext = p->m_pNPCNext;
		if (p->m_proto->CheckFlag(NPC_CASTLE_TOWER) != 0
			&& p->m_proto->CheckFlag(NPC_WARCASTLE) != 0)
		{
			p->SendDisappearAllInCell(true);

			// 어택리스트에서 삭제
			DelAttackList(p);
			
			// npc 삭제
			area->CharFromCell(p, true);
			area->DelNPC(p);
		}
	}

	InitMasterTower();
	InitRebrithNPC();
}

// 공성 NPC 리젠
void CDratanCastle::RegenCastleNPC()
{
	CHECK_SUBSERVER(this);

	if (m_castleNPC != NULL || m_pDungeonNPC != NULL )
		return;

	CArea* area = GetArea();
	if (!area)
		return ;

	int i;
	for (i = 0; i < area->m_npcRegenList.m_nCount; i++)
	{
		CNPCRegenInfo* p = area->m_npcRegenList.m_infoList + i;
		if (p->m_totalNum != 0)
			continue ;

		CNPCProto* proto = gserver.m_npcProtoList.FindProto(p->m_npcIdx);
		if (proto != NULL 
			&& proto->CheckFlag(NPC_WARCASTLE) != 0
			&& ( proto->m_index == 387 || proto->m_index == 468 ) )
		{
			CNPC* pTempNPC;
			if( !m_pDungeonNPC && proto->m_index == 468 )
			{
				pTempNPC = m_pDungeonNPC = gserver.m_npcProtoList.Create(proto->m_index, p);
			}
			else if( !m_castleNPC && proto->m_index == 387 )
			{
				pTempNPC = m_castleNPC = gserver.m_npcProtoList.Create(proto->m_index, p);
			}

			if (!pTempNPC)
				return ;

			GET_X(pTempNPC) = p->m_regenX;
			GET_Z(pTempNPC) = p->m_regenZ;
			GET_R(pTempNPC) = p->m_regenR;
			GET_YLAYER(pTempNPC) = p->m_regenY;

			pTempNPC->m_regenX = GET_X(pTempNPC);
			pTempNPC->m_regenZ = GET_Z(pTempNPC);
			pTempNPC->m_regenY = GET_YLAYER(pTempNPC);

			pTempNPC->m_recoverPulse = gserver.m_pulse;
#ifndef NEW_DIVISION_EXPSP
			pTempNPC->m_totalDamage = 0;
#endif // #ifndef NEW_DIVISION_EXPSP

			p->m_bAlive = true;
			p->m_lastDieTime = 0;
			pTempNPC->InitPointsToMax();
			pTempNPC->m_disableTime = 0;

			pTempNPC->InitPointsToMax();
			area->AddNPC(pTempNPC);
			pTempNPC->m_postregendelay = 0;

			int cx, cz;
			area->PointToCellNum(GET_X(pTempNPC), GET_Z(pTempNPC), &cx, &cz);
			area->CharToCell(pTempNPC, GET_YLAYER(pTempNPC), cx, cz);

			CNetMsg rmsg;
			AppearMsg(rmsg, pTempNPC, true);
			area->SendToCell(rmsg, pTempNPC);

			GAMELOG << init("CASTLE NPC REGEN")
					<< "ZONE" << delim
					<< GetZoneIndex()
					<< delim << pTempNPC->m_idNum
					<< end;
			
			pTempNPC = NULL;
		}
	}
}

void CDratanCastle::RemoveCastleNPC()
{
	CHECK_SUBSERVER(this);

	if (m_castleNPC == NULL)
		return ;
	CArea* area = m_castleNPC->m_pArea;
	if (area == NULL)
	{
		DelAttackList(m_castleNPC);
		delete m_castleNPC;
	}
	else
	{
		if( m_castleNPC )
		{
			m_castleNPC->SendDisappearAllInCell(true);
			DelAttackList(m_castleNPC);
			area->CharFromCell(m_castleNPC, true);
			area->DelNPC(m_castleNPC);
		}
	}
	m_castleNPC = NULL;
}

void CDratanCastle::RemoveDungeonNPC()
{
	CHECK_SUBSERVER(this);

	if (m_pDungeonNPC == NULL)
		return ;
	CArea* area = m_pDungeonNPC->m_pArea;
	if (area == NULL)
	{
		DelAttackList(m_pDungeonNPC);
		delete m_pDungeonNPC;
	}
	else
	{
		if( m_pDungeonNPC )
		{
			m_pDungeonNPC->SendDisappearAllInCell(true);
			DelAttackList(m_pDungeonNPC);
			area->CharFromCell(m_pDungeonNPC, true);
			area->DelNPC(m_pDungeonNPC);
		}
	}
	m_pDungeonNPC = NULL;
}

// 공성 타워 리젠
void CDratanCastle::RegenCastleTower()
{
	CHECK_SUBSERVER(this);

	if (GetState() != WCSF_WAR_CASTLE)
		return ;

	CArea* area = GetArea();
	if (!area)
		return ;

	int regencount = 0;

	for (int i = 0; i < area->m_npcRegenList.m_nCount; i++)
	{
		CNPCRegenInfo* p = area->m_npcRegenList.m_infoList + i;

		if (p->m_totalNum != 0 || p->m_bAlive || (gserver.m_pulse - p->m_lastDieTime < p->m_regenSec))
			continue ;

		CNPCProto* proto = gserver.m_npcProtoList.FindProto(p->m_npcIdx);
		if (proto && proto->CheckFlag(NPC_CASTLE_TOWER) && proto->CheckFlag(NPC_WARCASTLE) == 0)
		{
			// 마스터 타워 설정 반영
			if (CheckBuyTower(proto->m_index) == false)
			{
				continue;
			}


			CNPC* npc = gserver.m_npcProtoList.Create(proto->m_index, p);
			if (!npc)
			{
				return;
			}

			GET_X(npc) = p->m_regenX;
			GET_Z(npc) = p->m_regenZ;
			GET_R(npc) = p->m_regenR;
			GET_YLAYER(npc) = p->m_regenY;

			npc->m_regenX = GET_X(npc);
			npc->m_regenZ = GET_Z(npc);
			npc->m_regenY = GET_YLAYER(npc);

			npc->m_recoverPulse = gserver.m_pulse;
#ifndef NEW_DIVISION_EXPSP
			npc->m_totalDamage = 0;
#endif // #ifndef NEW_DIVISION_EXPSP

			p->m_bAlive = true;
			p->m_lastDieTime = 0;
			npc->m_disableTime = 0;


			// 수호탑 강화 적용
			SetTowerReinforce(npc);

			npc->InitPointsToMax();
			area->AddNPC(npc);
			npc->m_postregendelay = 0;

			int cx, cz;
			area->PointToCellNum(GET_X(npc), GET_Z(npc), &cx, &cz);
			area->CharToCell(npc, GET_YLAYER(npc), cx, cz);

			CNetMsg rmsg;
			AppearMsg(rmsg, npc, true);
			area->SendToCell(rmsg, npc);

			// 성문지기 NPC 설정
			SetGateNPC(npc);

			// 워프 타워 NPC 설정
			SetWarpNPC(npc);

			regencount++;
		}
	}

	if (regencount > 0)
	{
		GAMELOG << init("CASTLE TOWER REGEN")
				<< "ZONE" << delim
				<< GetZoneIndex() << delim
				<< "COUNT" << delim
				<< regencount
				<< end;
	}
}


// 공성 타워 체력 1/2 리젠
void CDratanCastle::RegenCastleTowerHalf()
{
	if (GetState() != WCSF_WAR_CASTLE)
		return;

	RemoveCastleTower();

	CArea* area = GetArea();
	if (!area)
		return ;

	int regencount = 0;

	for (int i = 0; i < area->m_npcRegenList.m_nCount; i++)
	{
		CNPCRegenInfo* p = area->m_npcRegenList.m_infoList + i;

		if (p->m_totalNum != 0 
			|| p->m_bAlive											
//			|| (gserver.m_pulse - p->m_lastDieTime < p->m_regenSec)		// 타워들은 리젠 시간에 관계 없이 리젠
			)
		{
			continue ;
		}

		CNPCProto* proto = gserver.m_npcProtoList.FindProto(p->m_npcIdx);
		if (proto && proto->CheckFlag(NPC_CASTLE_TOWER) != 0 && proto->CheckFlag(NPC_WARCASTLE) == 0)
		{
			// 마스터 타워 설정 반영
			if (CheckBuyTower(proto->m_index) == false)
			{
				continue;
			}

			CNPC * npc = gserver.m_npcProtoList.Create(proto->m_index, p);
			if (!npc)
			{
				return;
			}

			GET_X(npc) = p->m_regenX;
			GET_Z(npc) = p->m_regenZ;
			GET_R(npc) = p->m_regenR;
			GET_YLAYER(npc) = p->m_regenY;

			npc->m_regenX = GET_X(npc);
			npc->m_regenZ = GET_Z(npc);
			npc->m_regenY = GET_YLAYER(npc);

			npc->m_recoverPulse = gserver.m_pulse;
#ifndef NEW_DIVISION_EXPSP
			npc->m_totalDamage = 0;
#endif // #ifndef NEW_DIVISION_EXPSP

			p->m_bAlive = true;
			p->m_lastDieTime = 0;
			npc->m_disableTime = 0;

			// 수호탑 강화 적용
			SetTowerReinforce(npc);

			npc->InitPointsToMax();
			area->AddNPC(npc);
			npc->m_postregendelay = 0;

			int cx, cz;
			area->PointToCellNum(GET_X(npc), GET_Z(npc), &cx, &cz);
			area->CharToCell(npc, GET_YLAYER(npc), cx, cz);

			// 체력 1/2 적용
			npc->m_hp = npc->m_proto->m_hp / 2;
			npc->m_maxHP = npc->m_hp;

			CNetMsg rmsg;
			AppearMsg(rmsg, npc, true);
			area->SendToCell(rmsg, npc);

			// 성문지기 NPC 설정
			SetGateNPC(npc);

			// 워프 타워 NPC 설정
			SetWarpNPC(npc);

			regencount++;
		}
	}

	if (regencount > 0)
	{
		GAMELOG << init("CASTLE TOWER REGEN HALF")
				<< "ZONE" << delim
				<< GetZoneIndex() << delim
				<< "COUNT" << delim
				<< regencount
				<< end;
	}
}

// 공성 타워 제거
void CDratanCastle::RemoveCastleTower()
{
	CHECK_SUBSERVER(this);

	CArea* area = GetArea();
	if (!area)
		return ;

	CNPC* pNext = area->m_npcList;
	CNPC* p;
	while ((p = pNext))
	{
		pNext = p->m_pNPCNext;
		if (p->m_proto->CheckFlag(NPC_CASTLE_TOWER) != 0
			&& p->m_proto->CheckFlag(NPC_WARCASTLE) == 0)
		{
			p->SendDisappearAllInCell(true);

			// 어택리스트에서 삭제
			DelAttackList(p);
			
			// npc 삭제
			area->CharFromCell(p, true);
			area->DelNPC(p);
		}
	}

	InitGateNPC();
	InitWarpNPC();
}


// 공성 타워, 부활진지 삭제
void CDratanCastle::RemoveCastleTowerHalf()
{
	CArea* area = GetArea();
	if (!area)
		return ;

	CNPC* pNext = area->m_npcList;
	CNPC* p;
	while ((p = pNext))
	{
		pNext = p->m_pNPCNext;
		if (p->m_proto->CheckFlag(NPC_CASTLE_TOWER | NPC_WARCASTLE) != 0)
		{
			p->SendDisappearAllInCell(true);

			// 어택리스트에서 삭제
			DelAttackList(p);
			
			// npc 삭제
			area->CharFromCell(p, true);
			area->DelNPC(p);
		}
	}

	InitGateNPC();
	InitWarpNPC();
}

// 공성 상인 리젠
void CDratanCastle::RegenShop()
{
	CHECK_SUBSERVER(this);

	if (!m_bRegenShop && GetState() == WCSF_NORMAL)
	{
		m_bRegenShop = true;

		CArea* area = GetArea();
		if (!area)
			return ;

		int i;
		for (i = 0; i < area->m_zone->m_nShopCount; i++)
		{
			if (area->GetAttr(area->m_zone->m_shopList[i].m_yLayer, area->m_zone->m_shopList[i].m_x, area->m_zone->m_shopList[i].m_z) != MAPATT_WARZONE)
				continue ;

			CNPC* npc = gserver.m_npcProtoList.Create(area->m_zone->m_shopList[i].m_keeperIdx, NULL);
			if (!npc)
				return ;

			GET_X(npc) = area->m_zone->m_shopList[i].m_x;
			GET_Z(npc) = area->m_zone->m_shopList[i].m_z;
			GET_R(npc) = area->m_zone->m_shopList[i].m_r;
			GET_YLAYER(npc) = area->m_zone->m_shopList[i].m_yLayer;

			npc->m_regenX = GET_X(npc);
			npc->m_regenZ = GET_Z(npc);
			npc->m_regenY = GET_YLAYER(npc);

			npc->m_recoverPulse = gserver.m_pulse;
#ifndef NEW_DIVISION_EXPSP
			npc->m_totalDamage = 0;
#endif // #ifndef NEW_DIVISION_EXPSP

			npc->InitPointsToMax();
			npc->m_disableTime = 0;

			npc->InitPointsToMax();
			area->AddNPC(npc);
			npc->m_postregendelay = 0;

			int cx, cz;
			area->PointToCellNum(GET_X(npc), GET_Z(npc), &cx, &cz);
			area->CharToCell(npc, GET_YLAYER(npc), cx, cz);

			CNetMsg rmsg;
			AppearMsg(rmsg, npc, true);
			area->SendToCell(rmsg, npc);

			GAMELOG << init("CASTLE SHOP REGEN")
					<< "ZONE" << delim
					<< GetZoneIndex() << delim
					<< "SHOP" << delim
					<< npc->m_name << delim
					<< GET_X(npc) << delim
					<< GET_Z(npc)
					<< end;
		}
	}
	else if (m_bRegenShop && GetState() == WCSF_WAR_CASTLE)
	{
		m_bRegenShop = false;

		CArea* area = GetArea();
		if (!area)
			return ;

		CNPC* pNext = area->m_npcList;
		CNPC* p;
		while ((p = pNext))
		{
			pNext = p->m_pNPCNext;
			if (p->m_proto->CheckFlag(NPC_SHOPPER) && p->GetMapAttr() == MAPATT_WARZONE)
			{
				p->SendDisappearAllInCell(true);

				// 어택리스트에서 삭제
				DelAttackList(p);
				
				// npc 삭제
				area->CharFromCell(p, true);
				area->DelNPC(p);
			}
		}
	}
}

//  공성 크리스탈 리젠
void CDratanCastle::RegenLordSymbol()
{
	CHECK_SUBSERVER(this);

	if (m_lordSymbol != NULL || GetState() != WCSF_WAR_CASTLE)
		return ;

	CArea* area = GetArea();
	if (!area)
		return ;

	int i;
	for (i = 0; i < area->m_npcRegenList.m_nCount; i++)
	{
		CNPCRegenInfo* p = area->m_npcRegenList.m_infoList + i;
		if (p->m_totalNum != 0 || p->m_bAlive || (gserver.m_pulse - p->m_lastDieTime < p->m_regenSec))
			continue ;

		CNPCProto* proto = gserver.m_npcProtoList.FindProto(p->m_npcIdx);
		if (proto && proto->CheckFlag(NPC_LORD_SYMBOL))
		{
			m_lordSymbol = gserver.m_npcProtoList.Create(proto->m_index, p);
			if (!m_lordSymbol)
				return ;

			GET_X(m_lordSymbol) = p->m_regenX;
			GET_Z(m_lordSymbol) = p->m_regenZ;
			GET_R(m_lordSymbol) = p->m_regenR;
			GET_YLAYER(m_lordSymbol) = p->m_regenY;

			m_lordSymbol->m_regenX = GET_X(m_lordSymbol);
			m_lordSymbol->m_regenZ = GET_Z(m_lordSymbol);
			m_lordSymbol->m_regenY = GET_YLAYER(m_lordSymbol);

			m_lordSymbol->m_recoverPulse = gserver.m_pulse;
#ifndef NEW_DIVISION_EXPSP
			m_lordSymbol->m_totalDamage = 0;
#endif // #ifndef NEW_DIVISION_EXPSP

			p->m_bAlive = true;
			p->m_lastDieTime = 0;
			m_lordSymbol->InitPointsToMax();
			m_lordSymbol->m_disableTime = 0;

			m_lordSymbol->InitPointsToMax();
			area->AddNPC(m_lordSymbol);
			m_lordSymbol->m_postregendelay = 0;

			int cx, cz;
			area->PointToCellNum(GET_X(m_lordSymbol), GET_Z(m_lordSymbol), &cx, &cz);
			area->CharToCell(m_lordSymbol, GET_YLAYER(m_lordSymbol), cx, cz);

			CNetMsg rmsg;
			AppearMsg(rmsg, m_lordSymbol, true);
			area->SendToCell(rmsg, m_lordSymbol);

			GAMELOG << init("LORD SYMBOL REGEN")
					<< "ZONE" << delim
					<< GetZoneIndex()
					<< end;

			// TODO : 권좌를 2개이상 설치시 break 제거 및 변수 수정
			break;
		}
	}
}

void CDratanCastle::RemoveLordSymbol()
{
	CHECK_SUBSERVER(this);

	if (m_lordSymbol == NULL)
		return ;
	CArea* area = m_lordSymbol->m_pArea;
	if (!area)
	{
		DelAttackList(m_lordSymbol);
		delete m_lordSymbol;
	}
	else
	{
		m_lordSymbol->SendDisappearAllInCell(true);
		DelAttackList(m_lordSymbol);
		area->CharFromCell(m_lordSymbol, true);
		area->DelNPC(m_lordSymbol);
	}
	m_lordSymbol = NULL;
}

// 공성 가디언 리젠
void CDratanCastle::RegenGuardNPC()
{
	CHECK_SUBSERVER(this);

//	if (GetState() == WCSF_NORMAL)
//		return;

	CArea * area = GetArea();
	if (area == NULL)
	{
		return;
	}

	int regencount = 0;

	int i;
	for (i = 0; i < area->m_npcRegenList.m_nCount; i++)
	{
		CNPCRegenInfo * p = area->m_npcRegenList.m_infoList + i;
//		if (gserver.m_pulse - p->m_lastDieTime < p->m_regenSec)
//		{	// 리젠 시간 체크
//			continue;
//		}

		if (p == NULL)
		{
			continue;
		}

		CNPCProto* proto = gserver.m_npcProtoList.FindProto(p->m_npcIdx);
		if (proto && proto->CheckFlag(NPC_CASTLE_GUARD))
		{
			CNPC* npc = gserver.m_npcProtoList.Create(proto->m_index, p);
			if (!npc)
				return ;

			GET_X(npc) = p->m_regenX;
			GET_Z(npc) = p->m_regenZ;
			GET_R(npc) = p->m_regenR;
			GET_YLAYER(npc) = p->m_regenY;

			npc->m_regenX = GET_X(npc);
			npc->m_regenZ = GET_Z(npc);
			npc->m_regenY = GET_YLAYER(npc);

			npc->m_recoverPulse = gserver.m_pulse;
#ifndef NEW_DIVISION_EXPSP
			npc->m_totalDamage = 0;
#endif // #ifndef NEW_DIVISION_EXPSP

			p->m_bAlive = true;
			p->m_lastDieTime = 0;
			npc->InitPointsToMax();
			npc->m_disableTime = 0;

			npc->InitPointsToMax();
			area->AddNPC(npc);
			npc->m_postregendelay = 0;

			int cx, cz;
			area->PointToCellNum(GET_X(npc), GET_Z(npc), &cx, &cz);
			area->CharToCell(npc, GET_YLAYER(npc), cx, cz);

			CNetMsg rmsg;
			AppearMsg(rmsg, npc, true);
			area->SendToCell(rmsg, npc);

			regencount++;
		}
	}

	if (regencount > 0)
	{
		GAMELOG << init("GUARD NPC REGEN")
				<< "ZONE" << delim
				<< GetZoneIndex() << delim
				<< "COUNT" << delim
				<< regencount
				<< end;
	}
}

void CDratanCastle::RemoveGuardNPC()
{
	CHECK_SUBSERVER(this);

	CArea* area = GetArea();
	if (!area)
		return ;

	CNPC* pNext = area->m_npcList;
	CNPC* p;
	while ((p = pNext))
	{
		pNext = p->m_pNPCNext;
		if (p->m_proto->CheckFlag(NPC_CASTLE_GUARD))
		{
			p->SendDisappearAllInCell(true);

			// 어택리스트에서 삭제
			DelAttackList(p);
			
			// npc 삭제
			area->CharFromCell(p, true);
			area->DelNPC(p);
		}
	}
	
	for (int i = 0; i < area->m_npcRegenList.m_wPos; i++)
	{
		CNPCRegenInfo * p = area->m_npcRegenList.m_infoList + i;
		if (p == NULL)
		{
			continue;
		}

		CNPCProto* proto = gserver.m_npcProtoList.FindProto(p->m_npcIdx);
		if (proto && proto->CheckFlag(NPC_CASTLE_GUARD))
		{
			if(area->m_npcRegenList.DelRegenInfo(p) == true)
			{
				i--;	// 지웠으면 그 위치부터 다시 검색
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// 시간 관련
//////////////////////////////////////////////////////////////////////////
// 다음 공성 시간 가져오기
void CDratanCastle::GetNextWarTime(struct tm* nextWarTime, bool bHumanable)
{
	time_t tNext = (time_t)GetNextWarTime();
	memcpy(nextWarTime, localtime(&tNext), sizeof(struct tm));
	if (bHumanable)
	{
		nextWarTime->tm_year += 1900;
		nextWarTime->tm_mon += 1;
	}
}

//////////////////////////////////////////////////////////////////////////
// 초기화
//////////////////////////////////////////////////////////////////////////
CArea * CDratanCastle::GerArea()
{
	int idx = gserver.FindZone(ZONE_DRATAN);
	if (idx < 0)
		return NULL;
	else
		return gserver.m_zones[idx].m_area;
}

bool CDratanCastle::LoadCastleData()
{
	CWarCastle::LoadCastleData();

	// 유저가 지정한 가디언 정보 로딩
	m_GuardList.init(3, 10); // 3종류 각각 10개씩

	CDBCmd cmd;
	cmd.Init(&gserver.m_dbcastle);

	sprintf(g_buf, "SELECT * FROM t_castle_guard WHERE a_zone_num=%d", ZONE_DRATAN);
	cmd.SetQuery(g_buf);

	if (cmd.Open() == false)	
	{
		return false;
	}

	int regensec = 20*60*10;
	int mobindex;
	float x, z, r;
//	float h;

	CArea * pArea = GetArea();
	if( pArea == NULL)
	{
		return false;
	}

	while (cmd.MoveNext() == true)
	{
		cmd.GetRec("a_npc_idx", mobindex);
		cmd.GetRec("a_pos_x", x);
		cmd.GetRec("a_pos_z", z);
		
		r = GetRandom(0, (int) (PI_2 * 10000)) / 10000;
			
		m_GuardList.InsertGuard(mobindex, x, z, 0);

//		pArea->m_npcRegenList.AddRegenInfo(pArea->m_npcRegenList.m_wPos, mobindex, regensec, x, 0, z, r, 0, ZONE_DRATAN);
		pArea[0].m_npcRegenList.AddRegenInfo(pArea->m_npcRegenList.m_wPos, mobindex, regensec, x, 0, z, r, 1, ZONE_DRATAN);
	}

	// 수호병 NPC 처리
	RegenGuardNPC();

	// 마스터 타워 정보 로딩
	sprintf(g_buf, "SELECT * FROM t_castle_tower WHERE a_zone_num=%d", ZONE_DRATAN);
	cmd.SetQuery(g_buf);
	if (cmd.Open() == false)	
	{
		return false;
	}
	int rowid = 0, colid = 0;
	while (cmd.MoveNext() == true)
	{
		cmd.GetRec("rowid", rowid);
		cmd.GetRec("status", colid);
		SetTowerStatus(rowid, (char)colid);
	}
	
	// 수호탑 강화 정보 로딩
	sprintf(g_buf, "SELECT * FROM t_castle_reinforce WHERE a_zone_num=%d", ZONE_DRATAN);
	cmd.SetQuery(g_buf);
	if (cmd.Open() == false)	
	{
		return false;
	}
	char type = 0, step = 0;
	while (cmd.MoveNext() == true)
	{
		cmd.GetRec("a_type", type);
		cmd.GetRec("a_step", step);
		SetReinforceStep(type, step);
	}

	// 부활 진지 정보 로딩
	sprintf(g_buf, "SELECT * FROM t_castle_rebrith WHERE a_zone_num=%d", ZONE_DRATAN);
	cmd.SetQuery(g_buf);
	if (cmd.Open() == false)	
	{
		return false;
	}
	int qindex = 0, gindex = 0;
	CLCString gname(MAX_CHAR_NAME_LENGTH + 1);
	while (cmd.MoveNext() == true)
	{
		cmd.GetRec("a_npc_idx", qindex);
		cmd.GetRec("a_guild_idx", gindex);
		cmd.GetRec("a_guild_name", gname);
		
		m_nRebrithGuild[qindex-390] = gindex;
		strncpy(m_strRebrithGuild[qindex-390], (const char *)gname, 51);
	}

#ifdef DYNAMIC_DUNGEON
	// 공성 던전 데이터 로딩
	int nEnvRate , nMopRate, nTaxRate, nHuntRate;
	sprintf(g_buf, "SELECT * FROM t_castle_dungeon WHERE a_zone_index=%d", ZONE_DRATAN_CASTLE_DUNGEON );
	cmd.SetQuery(g_buf);
	if (cmd.Open() && cmd.MoveFirst() )	
	{
		if( cmd.GetRec("a_env_rate", nEnvRate) )
			m_dvd.SetEnvRate( nEnvRate );
		if( cmd.GetRec("a_mop_rate", nMopRate) )
			m_dvd.SetMobRate( nMopRate );
		if( cmd.GetRec("a_tax_rate", nTaxRate) )
			m_dvd.SetFeeRate( nTaxRate );
		if( cmd.GetRec("a_hunt_rate", nHuntRate) )		
			m_dvd.SetHuntRate( nHuntRate );
	}
#endif //DYNAMIC_DUNGEON
	
	return true;
}

void CDratanCastle::InitRegenPosition()
{
	if (m_posRegenPoint)
		delete [] m_posRegenPoint;
	m_nRegenPoint = 0;
	m_posRegenPoint = NULL;

	if (m_countAttackGuild > 0)
	{
		int* tmp = new int[m_countAttackGuild];
		memset(tmp, 0, sizeof(int) * m_countAttackGuild);

		CWarCastleJoin* p = m_attackGuildList;
		int i = 0;
		while (p)
		{
			tmp[i] = p->GetIndex();
			i++;
			p = p->GetNext();
		}

		// 랜덤으로 섞기
		for (i = 0; i < m_countAttackGuild; i++)
		{
			int j = GetRandom(0, m_countAttackGuild - 1);
			int s = tmp[i];
			tmp[i] = tmp[j];
			tmp[j] = s;
		}

		// 리젠 지역 최대 8개
		m_nRegenPoint = (m_countAttackGuild < 8) ? m_countAttackGuild : 8;
		m_posRegenPoint = new int[m_nRegenPoint];

		memcpy(m_posRegenPoint, tmp, sizeof(int) * m_nRegenPoint);

		delete [] tmp;
	}
}

void CDratanCastle::ResetOwner()
{
	m_ownerGuildIndex = 0;
	m_ownerGuildName = "";
	m_ownerCharIndex = 0;
	m_ownerCharName = "";
}

void CDratanCastle::SetOwner(CGuild* guild)
{
	if (guild)
	{
		m_ownerGuildIndex = guild->index();
		m_ownerGuildName = guild->name();
		m_ownerCharIndex = guild->boss()->charindex();
		m_ownerCharName = guild->boss()->GetName();
	}
	else
	{
		ResetOwner();
	}
}

void CDratanCastle::CheckRespond(CPC * ch)
{
	CNetMsg rmsg;

	// 교감 중이면 교감 취소
	if(ch->IsSetPlayerState(PLAYER_CRISTAL_RESPOND) == true)
	{
		
		if (ch->m_pZone != NULL
			&& ch->m_pZone->m_index == ZONE_DRATAN)
		{
			if(GetState() == WCSF_WAR_CASTLE)
			{	// 공성중이면 교감 해제 이펙트 보이기
				CastleCristalRespondFailMsg(rmsg, ch);
				ch->m_pArea->SendToCell(rmsg, ch, true);
			}
		}
		// 교감 해제
		ResetRespondMeber(ch);
	}
}

char CDratanCastle::GetReinforceStep(int i)
{
	if( i >= 0 && i < 3)
	{
			return m_reinforce[i];
	}
	return -1;
}

void CDratanCastle::SetReinforceStep(char i, char v)
{
	if( i >= 0 && i < 3 && v > -1 && v < 4)
	{
		m_reinforce[(int)i] = v;
	}
}

bool CDratanCastle::CheckBuyTower(int index)
{
	if( index >= 352 
			&& index <= 386)
	{	// 수호탑
		int rows = (index - 352)/5;
		int limit_npc = (352 + rows*5 ) + (int)(GetTowerStatus(rows));
		if(index >= limit_npc)
		{
			return false;
		}
	}

	return true;
}

void CDratanCastle::SetTowerReinforce(CNPC * npc)
{
	int type = (npc->m_proto->m_index - 352) / 5;
	if (type >=0 && type < 3)
	{
		type = 0;
	}
	else if (type >=3 && type < 6)
	{
		type = 1;
	}
	else if(npc->m_proto->m_index == 388
		|| npc->m_proto->m_index == 389
		|| npc->m_proto->m_index == 404) 
	{
		type = 2;
	}
	else
	{
		return;
	}

	switch (m_reinforce[type])
	{
	case 1:
		npc->m_opMelee = npc->m_opMelee + (npc->m_opMelee*30)/100;
		return;

	case 2:
		npc->m_opMelee = npc->m_opMelee + (npc->m_opMelee*50)/100;
		npc->m_maxHP =  npc->m_maxHP + (npc->m_maxHP*50)/100;
		return;

	case 3:
		npc->m_opMelee = npc->m_opMelee*2;
		npc->m_maxHP =  npc->m_maxHP*2;
		return;

	default:
		return;
	}
}

void CDratanCastle::ChangeSide(int gindex)
{
	// 길드 얻어서 길드 멤버 참여정보 초기화
	CGuild * pGuild = gserver.m_guildlist.findguild(gindex);
	// 공성 길드에서 제외
	RemoveAttackGuild(gindex);

	CNetMsg rmsg;
	if (pGuild)
	{	// 길드원 공성 참여 정보 변환
		int j;
		for (j = 0; j < pGuild->maxmember(); j++)
		{
			if (pGuild->member(j) && pGuild->member(j)->GetPC())
			{
				pGuild->member(j)->GetPC()->SetJoinFlag(ZONE_DRATAN, WCJF_OWNER);
//				pGuild->member(j)->GetPC()->m_bChangeStatus = true;
				// 바로 보내기
				StatusMsg(rmsg, pGuild->member(j)->GetPC());
				SEND_Q(rmsg, pGuild->member(j)->GetPC()->m_desc);
			}
		}
	}

	// 참여 정보 DB에 저장
	// a_index 길드 인덱스 or 캐릭터 인덱스 
	// a_guild 1이면 길드 참가 0이면 개인
	// a_attack 1이면 공성 0 이면 수성
	sprintf(g_buf, "DELETE FROM t_castle_join WHERE a_zone_index=%d AND a_index=%d AND a_guild=1", ZONE_DRATAN, gindex);
	CDBCmd cmd;
	cmd.Init(&gserver.m_dbcastle);
	cmd.SetQuery(g_buf);
	cmd.Update();

	// 수성 길드 변경
	if(m_defenseGuildList != NULL)
	{
		CWarCastleJoin * pJoin = m_defenseGuildList;
		while ( pJoin != NULL && pJoin->GetIndex() > 0)
		{
			int tindex = pJoin->GetIndex();
			ChangeAttackGuild(tindex);
			
			pJoin = pJoin->GetNext();

			RemoveDefenseGuild(tindex);
		}
	}
	
	// 기존 성주 길드 처리
	ChangeAttackGuild(GetOwnerGuildIndex());
}

void CDratanCastle::ChangeAttackGuild(int gindex)
{
	CGuild * pGuild = gserver.m_guildlist.findguild(gindex);
	AddAttackGuild(gindex);

	CNetMsg rmsg;

	if (pGuild)
	{	// 길드원 공성 참여 정보 변환
		int j;
		for (j = 0; j < pGuild->maxmember(); j++)
		{
			if (pGuild->member(j) && pGuild->member(j)->GetPC())
			{
				pGuild->member(j)->GetPC()->SetJoinFlag(ZONE_DRATAN, WCJF_ATTACK_GUILD);

				// 바로 보내기
				StatusMsg(rmsg, pGuild->member(j)->GetPC());
				SEND_Q(rmsg, pGuild->member(j)->GetPC()->m_desc);
			}
		}
	}

	// 참여 정보 DB에 저장
	CDBCmd cmd;
	cmd.Init(&gserver.m_dbcastle);

	sprintf(g_buf, "DELETE FROM t_castle_join WHERE a_index=%d", gindex);
	cmd.SetQuery(g_buf);
	cmd.Update();

	sprintf(g_buf, "INSERT INTO t_castle_join (a_zone_index, a_index, a_guild, a_attack) VALUES (%d, %d, 1, 1)", ZONE_DRATAN, gindex);
	cmd.SetQuery(g_buf);
	cmd.Update();
}

int CDratanCastle::GetRemainWarTime()
{
	int startTime = GetNextWarTime();
	int endTime = startTime + CDratanCastle::WAR_TIME;
	time_t curTime;
	time(&curTime);

	return endTime - curTime;
}

void CDratanCastle::GetOutNonDefense()
{
	CNetMsg rmsg;

	int i = gserver.FindZone(GetZoneIndex());
	if (i == -1)
		return ;

	CZone*		pZone	= gserver.m_zones + i;
	CArea*		pArea	= pZone->m_area;

	char		nYlayer	= 0;
	int			nX0x2	= 0;
	int			nZ0x2	= 0;
	int			nX1x2	= 0;
	int			nZ1x2	= 0;
	GetInnerCastleRect(&nYlayer, &nX0x2, &nZ0x2, &nX1x2, &nZ1x2);

	int			nCellX0	= 0;
	int			nCellZ0	= 0;
	int			nCellX1	= 0;
	int			nCellZ1	= 0;
	pArea->PointToCellNum(nX0x2 / 2.0f, nZ0x2 / 2.0f, &nCellX0, &nCellZ0);
	pArea->PointToCellNum(nX1x2 / 2.0f, nZ1x2 / 2.0f, &nCellX1, &nCellZ1);

	int cx, cz;
	for (cx = nCellX0; cx <= nCellX1; cx++)
	{
		if (cx < 0 || cx >= pArea->m_size[0])
			continue ;

		for (cz = nCellZ0; cz <= nCellZ1; cz++)
		{
			if (cz < 0 || cz >= pArea->m_size[1])
				continue ;

			CCell& rCell = pArea->m_cell[cx][cz];
			CCharacter* pCharNext = rCell.m_listChar;
			CCharacter* pChar = NULL;
			while ((pChar = pCharNext))
			{
				pCharNext = pCharNext->m_pCellNext;

				if (IS_PC(pChar))
				{
					CPC* pPC = TO_PC(pChar);
					int nJoinFlag = pPC->GetJoinFlag(pZone->m_index);

//					switch (nJoinFlag)
//					{
//					case WCJF_OWNER:
//					case WCJF_DEFENSE_GUILD:
//						pPC = NULL;
//						break;
//					default:
//						break;
//					}

					if (pPC)
					{
						if (IsInInnerCastle(pPC))
						{
							int extra = GetRegenPoint(nJoinFlag, pPC);

#ifdef EX_GO_ZONE_REBIRTH
							if (extra == 0)
							{
								extra = gserver.FindExtraByZoneIndex(pPC->IsChaotic(), ZONE_DRATAN);
							}
#endif // EX_GO_ZONE_REBIRTH
								
							GoZone(pPC, pZone->m_index,
									pZone->m_zonePos[extra][0],
									GetRandom(pZone->m_zonePos[extra][1], pZone->m_zonePos[extra][3]) / 2.0f,
									GetRandom(pZone->m_zonePos[extra][2], pZone->m_zonePos[extra][4]) / 2.0f);
							pPC->SetDisableTime(1);
						}
					}
				}
			}
		}
	}
}

bool CDratanCastle::IsInInnerCastle(CCharacter* pChar)
{
	char		nYlayer	= 0;
	int			nX0x2	= 0;
	int			nZ0x2	= 0;
	int			nX1x2	= 0;
	int			nZ1x2	= 0;
	GetInnerCastleRect(&nYlayer, &nX0x2, &nZ0x2, &nX1x2, &nZ1x2);

	if (nX0x2 / 2.0f > GET_X(pChar) || GET_X(pChar) > nX1x2 / 2.0f || nZ0x2 / 2.0f > GET_Z(pChar) || GET_Z(pChar) > nZ1x2 / 2.0f)
		return false;
	else
		return true;
}

bool CDratanCastle::CanLordChat(CPC* pc)
{
	switch (gserver.m_subno)
	{
	case WAR_CASTLE_SUBNUMBER_DRATAN:
		if (GetState() != WCSF_NORMAL)
			return false;
		if (GetOwnerCharIndex() != pc->m_index)
			return false;
		return true;

	default:
		return false;
	}
}

bool CDratanCastle::AddAttackGuild(int index)
{
	CWarCastleJoin * p = new CWarCastleJoin(index, 1);
	if (m_attackGuildList == NULL)
	{
		m_attackGuildList = p;
	}
	else
	{
		p->SetNext(m_attackGuildList);
		m_attackGuildList->SetPrev(p);
		m_attackGuildList = p;
	}

	m_countAttackGuild++;

	return true;
}

bool CDratanCastle::AddDefenseGuild(int index)
{
	CWarCastleJoin* p = new CWarCastleJoin(index, 1);
	if (m_defenseGuildList == NULL)
	{
		m_defenseGuildList = p;
	}
	else
	{
		p->SetNext(m_defenseGuildList);
		m_defenseGuildList->SetPrev(p);
		m_defenseGuildList = p;
	}

	return true;
}

bool CDratanCastle::IsAttackGuild(int index)
{
	return (FindAttackGuild(index)) ? true : false;
}

bool CDratanCastle::IsDefenseGuild(int index)
{
	return (FindDefenseGuild(index)) ? true : false;
}

void CDratanCastle::MoveToRegenPoint()
{
	CNetMsg rmsg;

	int i = gserver.FindZone(ZONE_DRATAN);
	if (i == -1)
		return ;

	CZone* pZone = gserver.m_zones + i;

	for (i = 0; i < gserver.m_playerList.m_max; i++)
	{
		CPC * pc = gserver.m_playerList.m_pcList[i];
		if( pc )
		{
			int extra = GetRegenPoint(pc->GetJoinFlag(ZONE_DRATAN), pc);
			if (extra == 0)
				continue ;

			// 확인 메시지 전송
			WarpPromptMsg(rmsg, pZone->m_index, extra);
			SEND_Q(rmsg, pc->m_desc);
		}
	}
}

void CDratanCastle::StopCastleTower()
{
	CHECK_SUBSERVER(this);

	CArea* area = GetArea();
	if (!area)
		return ;

	CNPC* pNext = area->m_npcList;
	CNPC* p;
	while ((p = pNext))
	{
		pNext = p->m_pNPCNext;
		if (p->m_proto->CheckFlag(NPC_CASTLE_TOWER) != 0
			&& p->m_proto->CheckFlag(NPC_WARCASTLE) == 0
			&& p->m_proto->m_index != 388
			&& p->m_proto->m_index != 389 
			&& p->m_proto->m_index != 404 )
		{	// 마스터 타워에서 설정한 타워들 정지
			p->m_hp = 0;
		}
	}

	// 워프 타워 정보 초기화
	InitWarpNPC();
}

void CDratanCastle::EndWarRegenPoint()
{
	int i = gserver.FindZone(GetZoneIndex());
	if (i == -1)
		return ;

	CZone*		pZone	= gserver.m_zones + i;
	CArea*		pArea	= pZone->m_area;

	char		nYlayer	= 0;
	int			nX0x2	= 0;
	int			nZ0x2	= 0;
	int			nX1x2	= 0;
	int			nZ1x2	= 0;
	GetInnerCastleRect(&nYlayer, &nX0x2, &nZ0x2, &nX1x2, &nZ1x2);

	int			nCellX0	= 0;
	int			nCellZ0	= 0;
	int			nCellX1	= 0;
	int			nCellZ1	= 0;
	pArea->PointToCellNum(nX0x2 / 2.0f, nZ0x2 / 2.0f, &nCellX0, &nCellZ0);
	pArea->PointToCellNum(nX1x2 / 2.0f, nZ1x2 / 2.0f, &nCellX1, &nCellZ1);

	int cx, cz;
	for (cx = nCellX0; cx <= nCellX1; cx++)
	{
		if (cx < 0 || cx >= pArea->m_size[0])
			continue ;

		for (cz = nCellZ0; cz <= nCellZ1; cz++)
		{
			if (cz < 0 || cz >= pArea->m_size[1])
				continue ;

			CCell& rCell = pArea->m_cell[cx][cz];
			CCharacter* pCharNext = rCell.m_listChar;
			CCharacter* pChar = NULL;
			while ((pChar = pCharNext))
			{
				pCharNext = pCharNext->m_pCellNext;

				if (IS_PC(pChar))
				{
					CPC* pPC = TO_PC(pChar);
					int nJoinFlag = pPC->GetJoinFlag(pZone->m_index);

					switch (nJoinFlag)
					{
					case WCJF_OWNER:
					case WCJF_DEFENSE_GUILD:
						pPC = NULL;
						break;
					}

					if (pPC)
					{
						if (IsInInnerCastle(pPC))
						{
							int extra = 0;
								
							GoZone(pPC, pZone->m_index,
									pZone->m_zonePos[extra][0],
									GetRandom(pZone->m_zonePos[extra][1], pZone->m_zonePos[extra][3]) / 2.0f,
									GetRandom(pZone->m_zonePos[extra][2], pZone->m_zonePos[extra][4]) / 2.0f);
							pPC->SetDisableTime(1);						
						}
					}
				}
			}
		}
	}
}
#endif // DRATAN_CASTLE