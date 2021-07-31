#include "stdhdrs.h"
#include "Log.h"
#include "Server.h"
#include "Cmd.h"
#include "CmdMsg.h"
#include "DBCmd.h"

volatile LC_THREAD_T		gThreadIDGameThread;
volatile LC_THREAD_T		gThreadIDDBThread;

CServer gserver;
CCmdList gcmdlist;

void ServerSrandom(unsigned long initial_seed);

CServer::CServer()
{
	m_ssock = -1;
	m_desclist = NULL;
	m_nDesc = 0;
	FD_ZERO(&m_input_set);
	FD_ZERO(&m_output_set);
	FD_ZERO(&m_exc_set);
	FD_ZERO(&m_null_set);

	m_seqGen = 1;

	m_serverpath = GetServerPath();
	m_maxplayers = 0;
	m_bshutdown = false;
	m_nameserverisslow = true;
	m_breboot = false;

	m_pulse = 0;
	m_pulseServerTime = 0;

#ifdef RECOMMEND_SERVER_SYSTEM
	m_nRecommendServer = -1;
#endif // RECOMMEND_SERVER_SYSTEM

	memset(&m_sGoldenBallData, 0, sizeof(m_sGoldenBallData));
	m_sGoldenBallData.nStatus = GOLDENBALL_STATUS_NOTHING;
}

CServer::~CServer()
{
	if (m_serverpath)
		delete[] m_serverpath;
}

char* CServer::GetServerPath()
{
	char szBuffer[1024];
#ifdef CIRCLE_WINDOWS
	GetModuleFileName(::GetModuleHandle(NULL), szBuffer, 1000);
	int path_len = strlen(szBuffer);
	int i;

	for (i = path_len - 1; i >= 0; i-- ) {
		if (szBuffer[i] == '\\')  {
			szBuffer[i+1] = '\0';
			break;
		}
	}
	if (i < 0)
		return false;
#else
	getcwd (szBuffer, 512);
	strcat (szBuffer, "/");
#endif

	char* ret = new char[strlen(szBuffer) + 1];
	strcpy(ret, szBuffer);
	return ret;
}

bool CServer::LoadSettingFile()
{
	puts("Load setting file....");

	CLCString strTemp(1024);

	strTemp.Format("%s%s", m_serverpath, "newStobm.bin");

	if (!m_config.Load(strTemp))
		return false;

	return true;
}

bool CServer::InitGame()
{
	ServerSrandom(time(0));
	MakeMath();

#ifdef RECOMMEND_SERVER_SYSTEM
	GAMELOG << init("SYSTEM") << "Load Recommend Server Data" << delim;
	FILE* fpRecommend = fopen(FILENAME_RECOMMEND, "rb");
	if (fpRecommend)
	{
		if (fread(&gserver.m_nRecommendServer, sizeof(gserver.m_nRecommendServer), 1, fpRecommend) != 1)
			gserver.m_nRecommendServer = -1;
		fclose(fpRecommend);
		fpRecommend = NULL;
	}
	GAMELOG << gserver.m_nRecommendServer << end;
#endif // RECOMMEND_SERVER_SYSTEM

	GAMELOG << init("SYSTEM") << "Load Golden Ball Data" << end;
	FILE* fpGoldenBall = fopen(FILENAME_GOLDENBALL, "rb");
	if (fpGoldenBall)
	{
		if (fread(&gserver.m_sGoldenBallData, sizeof(gserver.m_sGoldenBallData), 1, fpGoldenBall) != 1)
		{
			memset(&gserver.m_sGoldenBallData, 0, sizeof(gserver.m_sGoldenBallData));
			gserver.m_sGoldenBallData.nStatus = GOLDENBALL_STATUS_NOTHING;
		}
		fclose(fpGoldenBall);
	}

	GAMELOG << init("SYSTEM") << "Finding player limit." << end;
	m_maxplayers = GetMaxPlayers();

	GAMELOG << init("SYSTEM") << "Opening mother connection." << end;
	m_ssock = InitSocket();

	GAMELOG << init("SYSTEM") << "Entering game loop." << end;

	return true;
}

int CServer::GetMaxPlayers()
{
#ifndef CIRCLE_UNIX
	return MAX_PLAYING;
#else
	int max_descs = 0;
	const char *method;
/*
 * First, we'll try using getrlimit/setrlimit.  This will probably work
 * on most systems.  HAS_RLIMIT is defined in sysdep.h.
 */
#ifdef HAS_RLIMIT
	{
		struct rlimit limit;

		/* find the limit of file descs */
		method = "rlimit";
		if (getrlimit(RLIMIT_NOFILE, &limit) < 0)
		{
			GAMELOG << init("SYS_ERR") << "calling getrlimit" << end;
			exit(1);
		}

		/* set the current to the maximum */
		limit.rlim_cur = limit.rlim_max;
		if (setrlimit(RLIMIT_NOFILE, &limit) < 0)
		{
			GAMELOG << init("SYS_ERR") << "calling setrlimit" << end;
			exit(1);
		}
#ifdef RLIM_INFINITY
		if (limit.rlim_max == RLIM_INFINITY)
			max_descs = MAX_PLAYING + NUM_RESERVED_DESCS;
		else
			max_descs = MIN(MAX_PLAYING + NUM_RESERVED_DESCS, limit.rlim_max);
#else
		max_descs = MIN(MAX_PLAYING + NUM_RESERVED_DESCS, limit.rlim_max);
#endif
	}
#elif defined (OPEN_MAX) || defined(FOPEN_MAX)
#if !defined(OPEN_MAX)
#define OPEN_MAX FOPEN_MAX
#endif
	method = "OPEN_MAX";
	max_descs = OPEN_MAX;		/* Uh oh.. rlimit didn't work, but we have
				 * OPEN_MAX */
#elif defined (_SC_OPEN_MAX)
   /*
	* Okay, you don't have getrlimit() and you don't have OPEN_MAX.  Time to
	* try the POSIX sysconf() function.  (See Stevens' _Advanced Programming
	* in the UNIX Environment_).
	*/
	method = "POSIX sysconf";
	errno = 0;
	if ((max_descs = sysconf(_SC_OPEN_MAX)) < 0) {
		if (errno == 0)
			max_descs = m_MaxPlaying + NUM_RESERVED_DESCS;
		else
		{
			GAMELOG << init("SYS_ERR") << "Error calling sysconf" << end;
			exit(1);
		}
	}
#else
	/* if everything has failed, we'll just take a guess */
	method = "random guess";
	max_descs = m_MaxPlaying + NUM_RESERVED_DESCS;
#endif

	/* now calculate max _players_ based on max descs */
	max_descs = MIN(MAX_PLAYING, max_descs - NUM_RESERVED_DESCS);

	if (max_descs <= 0)
	{
		GAMELOG << init("SYS_ERR") << "Non-positive max player limit!  (Set at" << max_descs << "using" << method << ")" << end;
		exit(1);
	}

	GAMELOG << init("SYSTEM") << "Setting player limit to" << max_descs << "using" << method << "." << end;

	return (max_descs);
#endif /* CIRCLE_UNIX */
}

socket_t CServer::InitSocket()
{
	socket_t s;
	struct sockaddr_in sa;
	int opt;

#ifdef CIRCLE_WINDOWS
	// 扩加 檬扁拳
	{
		WORD wVersionRequested;
		WSADATA wsaData;

		wVersionRequested = MAKEWORD(1, 1);

		if (WSAStartup(wVersionRequested, &wsaData) != 0)
		{
			puts("SYSERR: WinSock not available!");
			exit(1);
		}
		if ((wsaData.iMaxSockets - 4) < m_maxplayers)
		{
			m_maxplayers = wsaData.iMaxSockets - 4;
		}
		GAMELOG << init("SYSTEM") << "Max players set to" << m_maxplayers << end;

		// 家南 积己
		if ((s = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		{
			puts("SYSERR: Error opening network connection : Winsock error");
			exit(1);
		}
	}
#else
	// 家南 积己
	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		GAMELOG << init("SYS_ERR") << "Error creating socket" << end;
		exit(1);
	}
#endif	/* CIRCLE_WINDOWS */

#if defined(SO_REUSEADDR) && !defined(CIRCLE_MACINTOSH)
	opt = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &opt, sizeof(opt)) < 0)
	{
		puts("SYSERR: setsockopt REUSEADDR");
		exit(1);
	}
#endif

	SetSendbuf(s);

	/*
	 * The GUSI sockets library is derived from BSD, so it defines
	 * SO_LINGER, even though setsockopt() is unimplimented.
	 *	(from Dean Takemori <dean@UHHEPH.PHYS.HAWAII.EDU>)
	 */
#if defined(SO_LINGER) && !defined(CIRCLE_MACINTOSH)
	{
		struct linger ld;
		
		ld.l_onoff = 0;
		ld.l_linger = 0;
		if (setsockopt(s, SOL_SOCKET, SO_LINGER, (char *) &ld, sizeof(ld)) < 0)
			GAMELOG << init("SYS_ERR") << "setsockopt SO_LINGER" << end;	/* Not fatal I suppose. */
	}
#endif

	/* Clear the structure */
	memset((char *)&sa, 0, sizeof(sa));

	sa.sin_family = AF_INET;
	sa.sin_port = htons((unsigned short)atoi(gserver.m_config.Find("Server", "Port")));
	sa.sin_addr = *(GetBindAddr());

	if (bind(s, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		GAMELOG << init("SYS_ERR") << "bind" << end;
		CLOSE_SOCKET(s);
		FILE *fp = fopen (".shutdown", "w");
		fclose (fp);
		exit(1);
	}
	Nonblock(s);
	listen(s, 57);

	return (s);
}

int CServer::SetSendbuf(socket_t s)
{
#if defined(SO_SNDBUF) && !defined(CIRCLE_MACINTOSH)
	int opt = MAX_SOCK_BUF;

	if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (char *) &opt, sizeof(opt)) < 0)
	{
		GAMELOG << init("SYS_ERR") << "setsockopt SNDBUF" << end;
		return (-1);
	}

# if 0
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (char *) &opt, sizeof(opt)) < 0)
	{
		GAMELOG << init("SYS_ERR") << "setsockopt RCVBUF" << end;
		return (-1);
	}
# endif

#endif

	return (0);
}

struct in_addr* CServer::GetBindAddr()
{
	static struct in_addr bind_addr;

	memset(&bind_addr, 0, sizeof(bind_addr));

	if (strcmp(m_config.Find("Server", "IP"), "ALL") == 0)
		bind_addr.s_addr = htonl(INADDR_ANY);
	else
	{
		unsigned long addr = inet_addr(m_config.Find("Server", "IP"));
		if (addr < 0)
		{
			bind_addr.s_addr = htonl(INADDR_ANY);
			GAMELOG << init("SYS_ERR") << "Invalid IP address" << end;
		}
		else
			bind_addr.s_addr = addr;
	}

	/* Put the address that we've finally decided on into the logs */
	if (bind_addr.s_addr == htonl(INADDR_ANY))
		GAMELOG << init("SYSTEM") << "Binding to all IP interfaces on this m_host." << end;
	else
	{
		GAMELOG << init("SYSTEM") << "Binding only to IP address" << inet_ntoa(bind_addr) << end;
	}

	return (&bind_addr);
}

#if defined(CIRCLE_WINDOWS)
void CServer::Nonblock(socket_t s)
{
	unsigned long val = 1;
	ioctlsocket(s, FIONBIO, &val);
}
#else
#  ifndef O_NONBLOCK
#    define O_NONBLOCK O_NDELAY
#  endif
void CServer::Nonblock(socket_t s)
{
	int flags;
	flags = fcntl(s, F_GETFL, 0);
	flags |= O_NONBLOCK;
	if (fcntl(s, F_SETFL, flags) < 0)
	{
		GAMELOG << init("SYS_ERR") << "Fatal error executing nonblock (CServer.cpp)" << end;
		exit(1);
	}
}
#endif

#ifdef CIRCLE_WINDOWS
void CServer::ServerSleep(struct timeval* timeout)
{
	Sleep(timeout->tv_sec * 1000 + timeout->tv_usec / 1000);
}
#else
void CServer::ServerSleep(struct timeval* timeout)
{
	if (select(0, (fd_set *) 0, (fd_set *) 0, (fd_set *) 0, timeout) < 0)
	{
		if (errno != EINTR)
		{
			GAMELOG << init("SYS_ERR") << "Select sleep!!" << end;
			exit(1);
		}
	}
}
#endif /* end of CIRCLE_WINDOWS */

int CServer::NewDescriptor(int s)
{
	socket_t m_desc;
	socklen_t i;
	static int last_desc = 0;	/* last descriptor number */
	CDescriptor* newd;
	struct sockaddr_in peer;
	struct hostent *from;

	i = sizeof(peer);
	if ((m_desc = accept(s, (struct sockaddr *)&peer, &i)) == INVALID_SOCKET)
		return (-1);

	/* keep it from blocking */
	Nonblock(m_desc);

	/* set the send buffer size */
	if (SetSendbuf(m_desc) < 0)
	{
		CLOSE_SOCKET(m_desc);
		return (0);
	}

	/* create a new descriptor */
	newd = new CDescriptor;

	/* find the sitename */
	if (m_nameserverisslow
			|| !(from = gethostbyaddr((char*)&peer.sin_addr, sizeof(peer.sin_addr), AF_INET)))
	{
		/* resolution failed */
		if (!m_nameserverisslow)
			GAMELOG << init("SYS_ERR") << "gethostbyaddr" << end;

		/* find the numeric site address */
		newd->m_host.CopyFrom((char*)inet_ntoa(peer.sin_addr), HOST_LENGTH);
	} else {
		newd->m_host.CopyFrom(from->h_name, HOST_LENGTH);
	}

	/* initialize descriptor data */
	newd->m_desc = m_desc;
	STATE(newd) = CON_GET_LOGIN;

	/*
	* This isn't exactly optimal but allows us to make a design choice.
	* Do we embed the history in CDescriptor or keep it dynamically
	* allocated and allow a user defined history size?
	*/
	if (++last_desc == 100000000)
		last_desc = 1;
	newd->m_descnum = last_desc;

	/* prepend to list */
	ADD_TO_BILIST(newd, m_desclist, m_pPrev, m_pNext);

	return (0);
}

void CServer::CloseSocket(CDescriptor* d)
{
	if (d->m_logined >= 1 && STATE(d) != CON_DISCONNECT)
	{
		STATE(d) = CON_DISCONNECT;
		d->m_logined = 2;		// m_logined but not play ==> want to disconnect
		return;
	}
	else
	{
		if (STATE(d) != CON_CLOSE && STATE(d) != CON_DISCONNECT)
		{
			GAMELOG << init("SYS_ERR") << "Close Socket Error :" << STATE(d) << end;
		}
	}

	REMOVE_FROM_BILIST(d, m_desclist, m_pPrev, m_pNext);
	d->CloseSocket();

	m_msgList.RemoveServer(d);

	delete d;
}

void CServer::SendOutput(CDescriptor* d)
{
	if (d == NULL) return;
	if (d->m_bclosed) return;
	if (d->m_outBuf.GetNextPoint())
	{
		if (FD_ISSET(d->m_desc, &m_output_set))
		{
			if (d->ProcessOutput() < 0)
				d->m_bclosed = true;
		}
	}
}

bool CServer::DisConnectLogedIn(CDescriptor* my_d)
{
	CDescriptor* d;
	CDescriptor* dNext = m_desclist;

	while ((d = dNext))
	{
		dNext = d->m_pNext;
		if (d != my_d && STATE(d) != CON_GET_LOGIN)
		{
			if (STATE(d) != CON_DISCONNECT)
			{
				STATE(d) = CON_CLOSE;
			}
			return true;
		}
	}
	return false;
}

void CServer::CommandInterpreter(CDescriptor* d, CNetMsg& msg)
{
	int cmd_num = gcmdlist.Find(msg.m_mtype);
	if (cmd_num < 0) // invalid command.. 
	{
		GAMELOG << init("SYS_ERR") << "Invalid command:" << msg.m_mtype << end;
		return;
	}

	gcmdlist.Run(cmd_num, msg, d);
}

void CServer::SetServerTime()
{
	time_t ct;
	struct tm *ti;
	
	ct = time(0);
	ti = localtime(&ct);
	m_serverTime.year	= (ti->tm_year % 100) % 62;
	m_serverTime.month	= (ti->tm_mon + 1) % 62;
	m_serverTime.day	= ti->tm_mday % 62;
	m_serverTime.hour	= ti->tm_hour % 62;
	m_serverTime.min	= ti->tm_min % 62;
	m_serverTime.sec	= ti->tm_sec % 62;
}

void CServer::SendToLoginServer(CNetMsg& msg)
{
	CDescriptor* pDesc;
	CDescriptor* pNext = gserver.m_desclist;
	while ((pDesc = pNext))
	{
		pNext = pDesc->m_pNext;

		if (STATE(pDesc) != CON_PLAYING)
			continue ;

		if (pDesc->m_serverNo == LOGIN_SERVER_NUM)
			SEND_Q(msg, pDesc);
	}
}
