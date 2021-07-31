#include "stdhdrs.h"
#include "Server.h"
#include "doFunc.h"
#include "CmdMsg.h"

void do_Request(CNetMsg& msg, CDescriptor* dest)
{
	msg.MoveFirst();

	int seq;
	int serverno, subno, zone;
	unsigned char subtype;

	msg >> seq
		>> serverno >> subno >> zone
		>> subtype;

	switch (subtype)
	{
	case MSG_MSGR_RECOMMEND:
#ifdef RECOMMEND_SERVER_SYSTEM
		do_Request_Recommend(msg);
#endif // RECOMMEND_SERVER_SYSTEM
		break;

	case MSG_MSGR_EVENT_GOLDENBALL:
		do_Request_EventGoldenball(msg);
		break;
	}

	CMsgListNode* node;
	if (seq == -1)
	{
		node = gserver.m_msgList.Add(-1, false, dest, msg);
	}
	else
	{
		node = gserver.m_msgList.Add(gserver.GetNextSeqNo(), true, dest, msg);
	}

	CDescriptor* pDesc;
	CDescriptor* pNext = gserver.m_desclist;
	while ((pDesc = pNext))
	{
		pNext = pDesc->m_pNext;

		if (STATE(pDesc) != CON_PLAYING)
			continue ;

		// serverno가 -1(모두)이거나
		// 해당 서버군 이고
		// 그 서버에 존재하는 존이거나
		// 모든 존이면 추가
		if (((serverno == -1/* && pDesc->m_serverNo != LOGIN_SERVER_NUM && pDesc->m_serverNo != CONNECTOR_SERVER_NUM*/)	// 컨넥터, 로그인서버 제외한
			|| pDesc->m_serverNo == serverno) && (subno == -1 || pDesc->m_subNo == subno) && (zone == -1 || pDesc->FindZone(zone) != -1))
			node->Add(pDesc);
	}
}

void do_Reply(CNetMsg& msg, CDescriptor* dest)
{
	msg.MoveFirst();

	int seq;
	int serverno, subno;
	int zoneno;
	unsigned char subtype;

	msg >> seq
		>> serverno
		>> subno
		>> zoneno
		>> subtype;

	// seq를 찾고
	CMsgListNode* node;
	CMsgListNode* nodeNext = gserver.m_msgList.m_head;
	CDescriptor* pServer = NULL;
	while ((node = nodeNext))
	{
		nodeNext = node->m_pNext;

		if (node->m_seq == seq)
		{
			bool bAllReceived = true;
			// 서버를 찾고
			CMsgListNodeServerNode* snode;
			CMsgListNodeServerNode* snodeNext = node->m_head;
			while ((snode = snodeNext))
			{
				snodeNext = snode->m_pNext;
				if (snode->m_server->m_serverNo == dest->m_serverNo && snode->m_server->m_subNo == dest->m_subNo)
				{
					// 서버 응답 받음으로 갱신
					snode->m_recieved = true;
					pServer = snode->m_server;
					continue ;
				}

				// 모두 응답 받았는지 검사
				if (!snode->m_recieved)
					bAllReceived = false;
			}

			// 요청한 서버가 없으면
			if (!pServer)
				continue ;

			switch (subtype)
			{
			case MSG_MSGR_WHISPER_REP:
				{
					// 수신자가 있어서 제대로 받았다
					int sidx;
					CLCString sname(MAX_CHAR_NAME_LENGTH + 1);
					CLCString rname(MAX_CHAR_NAME_LENGTH + 1);
					CLCString chat(1000);

					msg	>> sidx >> sname
						>> rname
						>> chat;

					CNetMsg mmsg;
					MsgrWhisperRep(mmsg, seq, serverno, subno, zoneno, sidx, sname, rname, chat);

					SEND_Q(mmsg, node->m_reqServer);

					// 제대로 받았으니 지워버리자
					bAllReceived = true;
				}
				break ;


			case MSG_MSGR_WHISPER_NOTFOUND:
				if (bAllReceived)
				{
					// 다 받았는데 귓말 대상이 없었다
					int sidx;
					CLCString sname(MAX_CHAR_NAME_LENGTH + 1);
					CNetMsg mmsg;

					msg >> sidx
						>> sname;

					MsgrWhisperNotfoundMsg(mmsg, seq, serverno, subno, zoneno, sidx, sname);

					SEND_Q(mmsg, node->m_reqServer);
				}
				break ;


			case MSG_MSGR_PLAYER_COUNT_REP:
				{
					// 받은 거 그대로 에코
					SEND_Q(msg, node->m_reqServer);
				}
				break;


			case MSG_MSGR_LOGOUT_REP:
				{
					char success;
					CLCString id(MAX_ID_NAME_LENGTH + 1);
					msg >> success >> id;

					CNetMsg rmsg;
					if (success)
					{
						// 로그아웃 한 서버가 있다면 성공한 것
						bAllReceived = true;
						MsgrLogoutRepMsg(rmsg, seq, serverno, subno, zoneno, (char)1, id);
						SEND_Q(rmsg, node->m_reqServer);
					}
					else if (bAllReceived)
					{
						// 다 받아도 없다...
						MsgrLogoutRepMsg(rmsg, seq, serverno, subno, zoneno, (char)0, id);
						SEND_Q(rmsg, node->m_reqServer);
					}
				}
				break ;

#ifdef NEW_DOUBLE_GM
			case MSG_MSGR_DOUBLE_EVENT_REP:
				{
					CNetMsg rmsg;
					int charindex, tserver, tsub, cmd, nas, nasget, exp, sp, produce, pronum;
					msg >> charindex
						>> tserver
						>> tsub
						>> cmd
						>> nas
						>> nasget
						>> exp
						>> sp
						>> produce
						>> pronum;

					//if( nas < 0 || nas > 200 )
					//	nas = 100;

					MsgrDoubleEventRepMsg(rmsg, seq, serverno, subno, zoneno, charindex, tserver, tsub, cmd, nas, nasget, exp, sp, produce, pronum);
					SEND_Q(rmsg, node->m_reqServer);
				}
				break;
#endif
#ifdef NEW_DOUBLE_EVENT_AUTO
			case MSG_MSGR_DOUBLE_EVENT_AUTO_REP:
				{
					CNetMsg rmsg;
					int charindex, tserver, tsub;
					char cmd, state;
					msg >> charindex
						>> tserver
						>> tsub
						>> cmd
						>> state;
					
					MsgrDoubleEventAutoRepMsg(rmsg, seq, serverno, subno, zoneno, charindex, tserver, tsub, cmd, state);
					SEND_Q(rmsg, node->m_reqServer);
				}
				break;
#endif // NEW_DOUBLE_EVENT_AUTO

#ifdef UPGRADE_EVENT
			case MSG_MSGR_UPGRADE_EVENT_REP:
				{
					CNetMsg rmsg;
					int charindex, tserver, tsub, prob;
					char cmd;

					msg >> charindex
						>> tserver
						>> tsub
						>> cmd
						>> prob;

					MsgrUpgradeEventRepMsg(rmsg, seq, serverno, subno, zoneno, charindex, tserver, tsub, cmd, prob);
					SEND_Q(rmsg, node->m_reqServer);
				}
				break;
#endif // UPGRADE_EVENT
				
#ifdef CHANCE_EVENT
			case MSG_MSGR_CHANCE_EVENT_REP:
				{
					CNetMsg rmsg;
					int charindex, tserver, tsub, cmd, slevel, elevel, nas, nasget, exp, sp, produce, pronum;
					msg >> charindex
						>> tserver
						>> tsub
						>> cmd
						>> slevel
						>> elevel
						>> nas
						>> nasget
						>> exp
						>> sp
						>> produce
						>> pronum;
					MsgrChanceEventRepMsg(rmsg, seq, serverno, subno, zoneno, charindex, tserver, tsub, cmd, slevel, elevel, nas, nasget, exp, sp, produce, pronum);
					SEND_Q(rmsg, node->m_reqServer);
				}
				break;
#endif // CHANCE_EVENT

			case MSG_MSGR_DOUBLE_EXP_EVENT_REP:
				{
					CNetMsg rmsg;
					int charindex, tserver, tsub, cmd, expPercent;
					msg >> charindex
						>> tserver
						>> tsub
						>> cmd
						>> expPercent;

					if( expPercent < 0 || expPercent > 200 )
						expPercent = 100;

					MsgrDoubleExpEventRepMsg(rmsg, seq, serverno, subno, zoneno, charindex, tserver, tsub, cmd, expPercent);
					SEND_Q(rmsg, node->m_reqServer);
				}
				break;
// 태국 오픈베타 메세지
			case MSG_MSGR_OPEN_BETA_EVENT_REP:
			case MSG_MSGR_FLOWER_EVENT_REP:
			case MSG_MSGR_MARGADUM_PVP_REP:
				{
					CNetMsg rmsg;
					int charindex, tserver, tsub, cmd, drop;
					msg >> charindex
						>> tserver
						>> tsub
						>> cmd
						>> drop;
					MsgrEventRepMsg(rmsg, subtype, seq, serverno, subno, zoneno, charindex, tserver, tsub, cmd, drop);
					SEND_Q(rmsg, node->m_reqServer);
				}
				break;

			case MSG_MSGR_MESSENGERCHAT_REP:
				{
					// 수신자가 있어서 제대로 받았다
					int sidx;
					CLCString sname(MAX_CHAR_NAME_LENGTH + 1);
					CLCString rname(MAX_CHAR_NAME_LENGTH + 1);
					CLCString chat(1000);

					msg	>> sidx >> sname
						>> rname
						>> chat;

					CNetMsg mmsg;
					MsgrFriendChatRep(mmsg, seq, serverno, subno, zoneno, sidx, sname, rname, chat);

					SEND_Q(mmsg, node->m_reqServer);

					// 제대로 받았으니 지워버리자
					bAllReceived = true;
				}
				break ;


			case MSG_MSGR_MESSENGERCHAT_NOTFOUND:
				if (bAllReceived)
				{
					// 다 받았는데 귓말 대상이 없었다
					int sidx;
					CLCString sname(MAX_CHAR_NAME_LENGTH + 1);
					CNetMsg mmsg;

					msg >> sidx
						>> sname;

					MsgrFriendChatNotfoundMsg(mmsg, seq, serverno, subno, zoneno, sidx, sname);

					SEND_Q(mmsg, node->m_reqServer);
				}
				break ;
#ifdef GMTOOL
			case MSG_MSGR_GMTOOL_KICKID_REP:
				{
					char success;
					int charindex;
					CLCString id(MAX_ID_NAME_LENGTH + 1);
					msg >> success >> charindex >> id;

					CNetMsg rmsg;
			//		if (success)
			//		{
						// 로그아웃 한 서버가 있다면 성공한 것
						bAllReceived = true;
						MsgrGmToolKickIDRepMsg(rmsg, seq, serverno, subno, zoneno, success, charindex, id);
						SEND_Q(rmsg, node->m_reqServer);
			//		}
			//		else if (bAllReceived)
			//		{
			//			// 다 받아도 없다...
			//			MsgrGmToolKickRepMsg(rmsg, seq, serverno, subno, zoneno, (char)0, charindex, id);
			//			SEND_Q(rmsg, node->m_reqServer);
			//		}
				}
				break;
			case MSG_MSGR_GMTOOL_KICK_REP:
				{
					char success;
					int charindex;
					CLCString name(MAX_CHAR_NAME_LENGTH + 1);
					msg >> success >> charindex >> name;

					CNetMsg rmsg;
					bAllReceived = true;
					MsgrGmToolKickRepMsg(rmsg, seq, serverno, subno, zoneno, success, charindex, name);
					SEND_Q(rmsg, node->m_reqServer);
				}
				break;
			case MSG_MSGR_GMTOOL_CHATMONITOR_REP:
				{
					char success;
					int gmcharindex;
					CLCString id(MAX_ID_NAME_LENGTH + 1);
					msg >> success
						>> gmcharindex
						>> id;

					CNetMsg rmsg;
					MsgrGmToolChatMonitorRepMsg(rmsg, seq, serverno, subno, zoneno, success, gmcharindex, id);
					SEND_Q(rmsg, node->m_reqServer);

					bAllReceived = true;
				}
				break;
			case MSG_MSGR_GM_WHISPER_REP:
				{
					CLCString rname(MAX_ID_NAME_LENGTH + 1);
					CLCString chat(1000);
					int charindex, serverno, subno, sindex;

					msg >> charindex
						>> rname
						>> chat
						>> serverno
						>> subno
						>> sindex;

					CNetMsg rmsg;

					MsgrGMWhisperRep(rmsg, seq, 1, 1, 0, charindex, rname, chat, serverno, subno, sindex);

					SEND_Q(rmsg, node->m_reqServer);

					bAllReceived = true;
				}
				break;
			case MSG_MSGR_GMTOOL_WHISPER_REP:
				{
					CLCString sname(MAX_ID_NAME_LENGTH + 1), rname(MAX_ID_NAME_LENGTH + 1);
					CLCString chat(1000);
					int sindex;

					msg >> sindex
						>> sname
						>> rname
						>> chat;

					CNetMsg rmsg;
					MsgrGMToolWhisperRep(rmsg, seq, serverno, subno, zoneno, sindex, sname, rname, chat);
					SEND_Q(rmsg, node->m_reqServer);
				}
				break;
			case MSG_MSGR_GM_WHISPER_NOTFOUND:
				{
					int sindex;
					msg >> sindex;

					CNetMsg mmsg;
					MsgrGMWhisperNotfoundMsg(mmsg, seq, serverno, subno, zoneno, sindex);
					SEND_Q(mmsg, node->m_reqServer);
				}
				break;
			case MSG_MSGR_GMTOOL_SILENCE_REP:
				{
					CLCString name(MAX_ID_NAME_LENGTH+1);
					int server, sub;
					int success;
					int gmcharindex;

					msg >> server
						>> sub
						>> success
						>> gmcharindex
						>> name;

					CNetMsg mmsg;
					MsgrGMSilenceRep(mmsg, seq, serverno, subno, zoneno, server, sub, success, gmcharindex, (const char*)name);
					SEND_Q(mmsg, node->m_reqServer);
				}
				break;
#endif // GMTOOL

#ifdef	DOUBLE_ITEM_DROP
			case MSG_MSGR_DOUBLE_ITEM_EVENT_REP :
				{
					CNetMsg rmsg;
					int charindex, tserver, tsub, cmd, itemPercent;
					msg >> charindex
						>> tserver
						>> tsub
						>> cmd
						>> itemPercent;

					if( itemPercent < 0 || itemPercent > 200 )
						itemPercent = 100;

					MsgrDoubleItemEventRepMsg(rmsg, seq, serverno, subno, zoneno, charindex, tserver, tsub, cmd, itemPercent);
					SEND_Q(rmsg, node->m_reqServer);
				}
				break;
#endif // DOUBLE_ITEM_DROP

#ifdef	DOUBLE_PET_EXP
			case MSG_MSGR_DOUBLE_PET_EXP_EVENT_REP:
				{
					CNetMsg rmsg;
					int charindex, tserver, tsub, cmd, PetExpPercent;
					msg >> charindex
						>> tserver
						>> tsub
						>> cmd
						>> PetExpPercent;

					if( PetExpPercent < 0 || PetExpPercent > 200 )
						PetExpPercent = 100;

					MsgrDoublePetExpEventRepMsg(rmsg, seq, serverno, subno, zoneno, charindex, tserver, tsub, cmd, PetExpPercent);
					SEND_Q(rmsg, node->m_reqServer);
				}
				break;
#endif // DOUBLE_PET_EXP
#ifdef DOUBLE_ATTACK
			case MSG_MSGR_DOUBLE_ATTACK_EVENT_REP:
				{
					CNetMsg rmsg;
					int charindex, tserver, tsub, cmd, AttackPercent;
					msg >> charindex
						>> tserver
						>> tsub
						>> cmd
						>> AttackPercent;

					if( AttackPercent < 0 || AttackPercent > 200 )
						AttackPercent = 100;

					MsgrDoubleAttackEventRepMsg(rmsg, seq, serverno, subno, zoneno, charindex, tserver, tsub, cmd, AttackPercent);
					SEND_Q(rmsg, node->m_reqServer);
				}
				break;
#endif // DOUBLE_ATTACK

#ifdef EVENT_DROPITEM
			case MSG_MSGR_EVENT_DROPITEM_REP:
				{
					CNetMsg rmsg;
					int charindex, npcidx, itemidx, prob, thisServer, thisSub;
					char type;

					msg >> charindex
						>> type
						>> npcidx
						>> itemidx
						>> prob
						>> thisServer
						>> thisSub;

					if( prob < 1 || prob > 100)
						prob = 1;

					MsgrEventDropItemRepMsg(rmsg, seq, serverno, subno, -1, charindex, type, npcidx, itemidx, prob, thisServer, thisSub);
					SEND_Q(rmsg, node->m_reqServer);
				}
				break;
#endif // EVENT_DROPITEM
			}

			// 다 받았으면 노드에서 제거
			if (bAllReceived)
			{
				gserver.m_msgList.Remove(node);
			}

			break;
		}
	}
}

#ifdef RECOMMEND_SERVER_SYSTEM
void do_Request_Recommend(CNetMsg& msg)
{
	int nGMCharIndex;
	int nRecommendServer;
	msg >> nGMCharIndex
		>> nRecommendServer;

	gserver.m_nRecommendServer = nRecommendServer;

	FILE* fp = fopen(FILENAME_RECOMMEND, "wb");
	if (fp)
	{
		fwrite(&gserver.m_nRecommendServer, sizeof(gserver.m_nRecommendServer), 1, fp);
		fclose(fp);
	}
}
#endif // RECOMMEND_SERVER_SYSTEM

void do_Request_EventGoldenball(CNetMsg& msg)
{
	int			nSubtype;
	int			nGMCharIndex;
	CLCString	strTeam1(GOLDENBALL_TEAM_NAME_LENGTH + 1);
	CLCString	strTeam2(GOLDENBALL_TEAM_NAME_LENGTH + 1);
	int			nTeam1Score = 0;
	int			nTeam2Score = 0;
	int			nYear = 0;
	int			nMonth = 0;
	int			nDay = 0;
	int			nHour = 0;
	int			nMin = 0;
	int			nEndVoteTime = 0;

	struct tm	tmEndVoteTime;

	msg >> nSubtype;

	switch (nSubtype)
	{
	case MSG_MSGR_EVENT_GOLDENBALL_VOTE:
		// 골든볼 이벤트 응모 설정	: gmcharindex(n) team1(str) team2(str) year(n) month(n) day(n) hour(n) minute(n)
		msg >> nGMCharIndex
			>> strTeam1
			>> strTeam2
			>> nYear
			>> nMonth
			>> nDay
			>> nHour
			>> nMin;
		gserver.m_sGoldenBallData.nStatus = GOLDENBALL_STATUS_VOTE;
		strcpy(gserver.m_sGoldenBallData.strTeam1, strTeam1);
		strcpy(gserver.m_sGoldenBallData.strTeam2, strTeam2);
		gserver.m_sGoldenBallData.nTeam1Score = 0;
		gserver.m_sGoldenBallData.nTeam2Score = 0;
		gserver.m_sGoldenBallData.nYear = nYear;
		gserver.m_sGoldenBallData.nMonth = nMonth;
		gserver.m_sGoldenBallData.nDay = nDay;
		gserver.m_sGoldenBallData.nHour = nHour;
		gserver.m_sGoldenBallData.nMin = nMin;
		
		memset(&tmEndVoteTime, 0, sizeof(tmEndVoteTime));
		tmEndVoteTime.tm_year	= nYear - 1900;
		tmEndVoteTime.tm_mon	= nMonth - 1;
		tmEndVoteTime.tm_mday	= nDay;
		tmEndVoteTime.tm_hour	= nHour;
		tmEndVoteTime.tm_min	= nMin;
		gserver.m_sGoldenBallData.timeEndVote = mktime(&tmEndVoteTime);
		break;

	case MSG_MSGR_EVENT_GOLDENBALL_GIFT:
		// 골든볼 이벤트 보상 설정	: gmcharindex(n) team1(str) team1score(n) team2(str) team2score(n) year(n) month(n) day(n) hour(n) minute(n) endVoteTime(n)
		msg >> nGMCharIndex
			>> strTeam1
			>> nTeam1Score
			>> strTeam2
			>> nTeam2Score
			>> nYear
			>> nMonth
			>> nDay
			>> nHour
			>> nMin
			>> nEndVoteTime;
		gserver.m_sGoldenBallData.nStatus = GOLDENBALL_STATUS_GIFT;
		strcpy(gserver.m_sGoldenBallData.strTeam1, strTeam1);
		strcpy(gserver.m_sGoldenBallData.strTeam2, strTeam2);
		gserver.m_sGoldenBallData.nTeam1Score = nTeam1Score;
		gserver.m_sGoldenBallData.nTeam2Score = nTeam2Score;
		gserver.m_sGoldenBallData.nYear = nYear;
		gserver.m_sGoldenBallData.nMonth = nMonth;
		gserver.m_sGoldenBallData.nDay = nDay;
		gserver.m_sGoldenBallData.nHour = nHour;
		gserver.m_sGoldenBallData.nMin = nMin;
		break;

	case MSG_MSGR_EVENT_GOLDENBALL_END:
		// 골든볼 이벤트 종료		: gmcharindex(n)
		msg >> nGMCharIndex;
		memset(&gserver.m_sGoldenBallData, 0, sizeof(gserver.m_sGoldenBallData));
		gserver.m_sGoldenBallData.nStatus = GOLDENBALL_STATUS_NOTHING;
		break;
	}

	FILE* fp = fopen(FILENAME_GOLDENBALL, "wb");
	if (fp)
	{
		fwrite(&gserver.m_sGoldenBallData, sizeof(gserver.m_sGoldenBallData), 1, fp);
		fclose(fp);
	}
}
