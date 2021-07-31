#include "stdhdrs.h"
#include "Log.h"
#include "Server.h"
#include "DBCmd.h"

#ifdef __GAME_SERVER__
// 시리얼 코드
static char		code62[62] = 
{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
#endif

static float	a_sin[360];
static float	a_cos[360];
struct timeval	a_NullTime;

char			g_buf[MAX_STRING_LENGTH];
char			g_buf2[MAX_STRING_LENGTH];
char			g_buf3[MAX_STRING_LENGTH];



/////////////////////////////////////////
// 수치 데이터에서 문자열로 변환하는 함수


////////////////////
// Function name	: __Itoa
// Description	    : 32비트 정수를 문자열로
// Return type		: char* 
//					: 변환 완료된 문자열 포인터
// Argument         : int value
//					: 변환 하고자 하는 정수
// Argument         : char *string
//					: 변환된 데이터를 저장할 문자열 포인터
char* __ltoa(int value, char *string)
{
	char p_buf[128];
	int idx, i;

	// 초기화
	*string = '\0';
	idx = 0;
	i = 0;

	// 음수면 -부호를 붙이고 value는 양수로 무조건
	if (value < 0)
	{
		strcpy(string, "-");
		value = -value;
		i = 1;
	}
	if (value == 0)
	{
		strcpy(string, "0");
		return string;
	}

	// 뒷자리부터 문자열로 변경
	while (value)
	{
		switch (value % 10)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			p_buf[idx++] = (char)((value % 10) + '0');
			break;
		default:
			p_buf[idx++] = '0';
			break;
		}

		value /= 10;
	}

	while (idx)
	{
		string[i++] = p_buf[--idx];
	}
	string[i++] = '\0';

	return string;
}



////////////////////
// Function name	: lltoa
// Description	    : 64비트 정수를 문자열로 변환
// Return type		: char* 
//					: 변환된 문자열
// Argument         : LONGLONG value
//					: 64비트 정수
// Argument         : char *string
//					: 변환에 사용하는 버퍼
char* __ltoa(LONGLONG value, char *string)
{
	char p_buf[128];
	int idx, i;

	// 초기화
	*string = '\0';
	idx = 0;
	i = 0;

	// 음수면 -부호를 붙이고 value는 양수로 무조건
	if (value < 0)
	{
		strcpy(string, "-");
		value = -value;
		i = 1;
	}
	if (value == 0)
	{
		strcpy(string, "0");
		return string;
	}

	// 뒷자리부터 문자열로 변경
	while (value)
	{
		switch (value % 10)
		{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			p_buf[idx++] = (char)((value % 10) + '0');
			break;
		default:
			p_buf[idx++] = '0';
			break;
		}
		
		value /= 10;
	}

	while (idx)
	{
		string[i++] = p_buf[--idx];
	}
	string[i++] = '\0';

	return string;
}


////////////////////
// Function name	: __ftoa
// Description	    : float 데이터를 문자열로 변환
// Return type		: char* 
//					: 변환된 문자열
// Argument         : float f
//					: 입력 실수
// Argument         : char *buf
//					: 출력 포인터
// Argument         : int place
//					: 자리수
char* __ftoa(float f, char *buf, int place)
{
	char p_buf[128];

	*buf = '\0';

	// 소수점 앞자리 문자열로
	// 양수 음수 가려서
	if (f < 0)
	{
		// 음수면 양수로 만들고
		f = -f;
		__ltoa((int)f, p_buf);
		// 문자열 앞에 -를 붙이고
		strcpy(buf, "-");
		strcat(buf, p_buf);
	}
	else
	{
		__ltoa((int)f, buf);
	}

	// 소수점 몇자리 까지?
	if (place <= 0)
	{
		return buf;
	}

	// 소수점 8자리까지만
	if (place > 8)
		place = 8;

	// 소수점 뒤부분만 남기고
	f -= (int)f;

	// 원하는 소수 자리까지 곱해서
	while (place > 0)
	{
		f *= 10;
		place--;
	}

	// 정수 부분만 다시 변환
	__ltoa((int)f, p_buf);
	strcat(buf, ".");
	strcat(buf, p_buf);

	return buf;
}


/////////////////////////////////////////////
// 수학 관련


////////////////////
// Function name	: MakeMath
// Description	    : 코사인, 사인 값을 1도 단위로 미리 계산해 둠
// Return type		: void 
//					: 
void MakeMath()
{
	for (int i = 0; i < 360; i++) {
		a_sin[i] = sinf((float) i / 360.0f * PI_2);
		a_cos[i] = cosf((float) i / 360.0f * PI_2);
	}
}


////////////////////
// Function name	: get_sin
// Description	    : 미리 구해둔 사인 값을 얻음
// Return type		: float 
//					: 사인값
// Argument         : float radian
//					: 각도, 단위는 도, 범위는 -180 ~ +180
float get_sin(float radian)
{
	int i = (int)(radian / PI_2 * 360);

	if (i < 0)
	{
		i %= 360;
		i += 360;
	}

	if (i >= 360)
	{
		i %= 360;
	}
	/*while (i >= 360) {
		i -= 360;
	}
	while (i < 0) {
		i += 360;
	}*/

	return a_sin[i];
}


////////////////////
// Function name	: get_cos
// Description	    : 미리 구해둔 코사인 값을 얻음
// Return type		: float 
//					: 사인값
// Argument         : float radian
//					: 각도, 단위는 도, 범위는 -180 ~ +180
float get_cos(float radian)
{
	int i = (int)(radian / PI_2 * 360);

	if (i < 0)
	{
		i %= 360;
		i += 360;
	}

	if (i >= 360)
	{
		i %= 360;
	}
	/*while (i >= 360) {
		i -= 360;
	}
	while (i < 0) {
		i += 360;
	}*/

	return a_cos[i];
}



///////////////////////////////////////////
// 시간 관련


////////////////////
// Function name	: GetTimeofDay
// Description	    : 현재 시간을 수함
// Return type		: void 
//					: 
// Argument         : struct timeval *t
//					: 구한 시간을 설정
// Argument         : struct timezone *dummy
//					: 사용하지 않음
void GetTimeofDay(struct timeval *t, struct timezone* dummy)
{
#ifdef CIRCLE_WINDOWS
	unsigned long millisec = GetTickCount();
	t->tv_sec = (int) (millisec / 1000);
	t->tv_usec = (millisec % 1000) * 1000;
#else
	gettimeofday (t, dummy);
#endif
}


////////////////////
// Function name	: TimeDiff
// Description	    : 시간 차이를 구한다, a - b = rslt
// Return type		: void 
//					: 
// Argument         : struct timeval *rslt
//					: 결과를 저장할 변수
// Argument         : struct timeval *a
//					: 시간 값, 기준 값
// Argument         : struct timeval *b
//					: 시간 값, 빼는 값
void TimeDiff(struct timeval *rslt, struct timeval *a, struct timeval *b)
{
	// a가 작으면 0
	if (a->tv_sec < b->tv_sec)
		*rslt = a_NullTime;

	// a와 b의 초단위 비교
	else if (a->tv_sec == b->tv_sec)
	{
		// 초단위 같을때
		// a가 작으면 0
		if (a->tv_usec < b->tv_usec)
			*rslt = a_NullTime;
		else
		{
			// 차이를 구함
			rslt->tv_sec = 0;
			rslt->tv_usec = a->tv_usec - b->tv_usec;
		}
	}

	else
	{
		// 초단위 다를때
		// 초단위 차이 구함
		rslt->tv_sec = a->tv_sec - b->tv_sec;
		// a가 마이크로초 단위는 작다면
		if (a->tv_usec < b->tv_usec)
		{
			// 마이크로초 단위에서 연산하고
			rslt->tv_usec = a->tv_usec + 1000000 - b->tv_usec;
			// 초단위는 자리내림으로 감소
			rslt->tv_sec--;
		}
		else
			rslt->tv_usec = a->tv_usec - b->tv_usec;
	}
}


////////////////////
// Function name	: TimeAdd
// Description	    : 시간의 함을 구한다, a + b = rslt
// Return type		: void 
//					: 
// Argument         : struct timeval *rslt
//					: 결과 값
// Argument         : struct timeval *a
//					: 시간 값, 기준 값
// Argument         : struct timeval *b
//					: 시간 값, 더해지는 값
void TimeAdd(struct timeval *rslt, struct timeval *a, struct timeval *b)
{
	rslt->tv_sec = a->tv_sec + b->tv_sec;
	rslt->tv_usec = a->tv_usec + b->tv_usec;

	// 자리올림 처리
	while (rslt->tv_usec >= 1000000)
	{
		rslt->tv_usec -= 1000000;
		rslt->tv_sec++;
	}
}



/////////////////////////////////////////
// 거리 계산 함수

////////////////////
// Function name	: GetDistance
// Description	    : 거리 계산
// Return type		: float 
//					: 거리 반환
// Argument         : float x1
//					: 시작 x 좌표
// Argument         : float x2
//					: 끝 x 좌표
// Argument         : float z1
//					: 시작 z 좌표
// Argument         : float z2
//					: 끝 z 좌표
// Argument         : float h1
//					: 시작 높이
// Argument         : float h2
//					: 끝 높이
float GetDistance(float x1, float x2, float z1, float z2, float h1, float h2)
{
	float dx = ABS(x1 - x2);
	float dz = ABS(z1 - z2);
	float dh = ABS(h1 - h2);

	float ret = (float)sqrt(dx*dx + dz*dz + dh*dh);
	return (ret > 0.0f) ? ret : 0.0f;
}




/////////////////////////////////
// 문자열 관련


////////////////////
// Function name	: SkipSpaces
// Description	    : 앞부분 공백 넘김, 원본에 변형 없음
// Return type		: char* 
//					: 처음 나오는 비공백 문자위 위치를 반환
// Argument         : char* string
//					: 공백을 넘기고자 하는 문자열
const char* SkipSpaces(const char* string)
{
	// 공백이면 다음으로 넘기기
	for (; string && *string && is_space(*string); (string)++)
		;
	// 포인터 반환
	return string;
}


////////////////////
// Function name	: RemoveTailSpaces
// Description	    : 끝의 공백을 제거, 원본을 수정
// Return type		: char* 
//					: 입력 문자열 포인터 반환
// Argument         : char* string
//					: 공백을 제거하고자 하는 문자열
char* RemoveTailSpaces(char* string)
{
	if(string == NULL) return NULL;

	// 길이를 구하고
	int len = strlen(string);

	// 끝부터 공백을 검사하여
	for (int i = len - 1; i >= 0; i++)
	{
		// 공백이면 NULL 문자로 대체
		if (is_space(string[i]))
			string[i] = '\0';
		else
			break;
	}

	return string;
}




////////////////////
// Function name	: TrimString
// Description	    : 앞뒤 공백을 제거, 원본이 수정됨
// Return type		: char* 
//					: 입력 문자열
// Argument         : char* string
//					: 공백을 제거하고자 하는 문자열
char* TrimString(char* string)
{
	if(string == NULL) return NULL;

	int len = strlen(string);
	int index = 0, i;

	// 공백이 아닌 처음 문자 구함
	while (string[index] == ' ')
		index ++;

	// 공백만큼 당기기
	if (index > 0)
	{
		for (i = 0; i <= (len-index); i++)
			string[i] = string[i+index];

		string[i] = '\0';
	}

	// 뒤의 공백 제거
	for (i = strlen(string) - 1; i > 0; i--)
	{
		if (string[i] != ' ')
			break;
	}

	string[i + 1] = '\0';

	return string;
}


////////////////////
// Function name	: AnyOneArg
// Description	    : 공백 단위로 문자열 토큰화
// Return type		: char* 
//					: 다음 토큰 위치
// Argument         : char* argument
//					: 원본 문자열
// Argument         : char* first_arg
//					: 토큰화된 문자열을 담을 곧
// Argument         : bool toLower
//					: 소문자로 강제 변환할지 여부
const char* AnyOneArg(const char* argument, char* first_arg, bool toLower)
{
	argument = SkipSpaces(argument);

	while (argument && *argument && !is_space(*argument))
	{
		if (toLower)
		{
			*(first_arg++) = LOWER(*argument);
		}
		else
		{
			*(first_arg++) = *argument;
		}
		argument++;
	}

	*first_arg = '\0';

	return (argument);
}

char* TransDateStr(int time, char* convbuf)//0627 // 광호 수정
{
	struct tm ti;
	
	time_t t = (long)time;
	
	ti = *localtime(&t);

	sprintf(convbuf, "%04d/%02d/%02d %02d:%02d:%02d", ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday, ti.tm_hour, ti.tm_min, ti.tm_sec);

	return convbuf;
}

#ifndef __BILLING_SERVER__
////////////////////
// Function name	: StrDup
// Description	    : 문자열 복사, C의 라이브러리와 다르게 new 연산자를 이용하여 메모리 할당
// Return type		: char* 
//					: 복사된 문자열 포인터
// Argument         : const char* source
//					: 원본 문자열
char* StrDup(const char* source)
{
	if(source == NULL) return NULL;

	char *new_z;
	new_z = new char[strlen(source) + 1];
	return (strcpy(new_z, source));
}


////////////////////
// Function name	: StrCat
// Description	    : 문자열 붙이기, 앞에 공백을 하나 삽입
// Return type		: char* 
//					: 붙여진 문자열
// Argument         : char* dest
//					: 대상 문자열
// Argument         : const char* src
//					: 붙여질 문자열
char* StrCat(char* dest, const char* src)
{
	strcat(dest, " ");
	strcat(dest, src);
	return dest;
}
#endif


////////////////////
// Function name	: IntCat
// Description	    : 문자열에 정수 붙이기
// Return type		: char* 
//					: 붙여진 문자열
// Argument         : char *dest
//					: 대상 문자열
// Argument         : int i
//					: 붙여질 정수
char* IntCat(char *dest, int i, bool bInsertSpace)
{
	char i_buf[128];
	if (bInsertSpace)
		strcat(dest, " ");
	strcat(dest, __ltoa(i, i_buf));
	return dest;
}


////////////////////
// Function name	: IntCat
// Description	    : 문자열에 정수 붙이기
// Return type		: char* 
//					: 붙여진 문자열
// Argument         : char *dest
//					: 대상 문자열
// Argument         : LONGLONG ll
//					: 붙여질 정수
char* IntCat(char *dest, LONGLONG ll, bool bInsertSpace)
{
	char i_buf[128];
	if (bInsertSpace)
		strcat(dest, " ");
	strcat(dest, __ltoa(ll, i_buf));
	return dest;
}


////////////////////
// Function name	: IntCat0
// Description	    : 정수를 문자열에 붙이기, 지정 폭만큼 0으로 채움
// Return type		: char* 
//					: 결과 문자열
// Argument         : char* dest
//					: 저장할 공간
// Argument         : int n
//					: 붙일 정수
// Argument         : int width
//					: 폭
char* IntCat0(char* dest, int n, int width, bool bInsertSpace)
{
	if(dest == NULL) return NULL;

	char tbuf[128];
	int len = strlen(__ltoa(n, tbuf));
	int wasLen = strlen(dest);

	if (bInsertSpace)
		strcat(dest, " ");
	if (width > len)
	{
		width -= len;
		for (int i = wasLen; i < width + wasLen ; i++)
		{
			dest[i] = '0';
			dest[i + 1] = '\0';
		}
	}
	strcat(dest, tbuf);
	return dest;
}


////////////////////
// Function name	: IntCat0
// Description	    : 정수를 문자열에 붙이기, 지정 폭만큼 0으로 채움
// Return type		: char* 
//					: 결과 문자열
// Argument         : char* dest
//					: 저장할 공간
// Argument         : LONGLONG n
//					: 붙일 정수
// Argument         : int width
//					: 폭
char* IntCat0(char* dest, LONGLONG n, int width, bool bInsertSpace)
{
	if(dest == NULL) return NULL;

	char tbuf[128];
	int len = strlen(__ltoa(n, tbuf));
	int wasLen = strlen(dest);

	if (bInsertSpace)
		strcat(dest, " ");
	if (width > len)
	{
		width -= len;
		for (int i = wasLen; i < width + wasLen ; i++)
		{
			dest[i] = '0';
			dest[i + 1] = '\0';
		}
	}
	strcat(dest, tbuf);
	return dest;
}


////////////////////
// Function name	: FloatCat
// Description	    : 문자열에 실수 붙이기
// Return type		: char* 
//					: 붙여진 문자열
// Argument         : char *dest
//					: 대상 문자열
// Argument         : float f
//					: 붙여질 실수
// Argument         : int place
//					: 소수점 이하 자리수
char* FloatCat(char *dest, float f, int place, bool bInsertSpace)
{
	char f_buf[128];
	if (bInsertSpace)
		strcat(dest, " ");
	strcat(dest, __ftoa(f, f_buf, place));
	return dest;
}


////////////////////
// Function name	: strinc
// Description	    : 특정 문자열을 포함하는지 검사
// Return type		: bool 
//					: 포함하면 true, 아니면 return
// Argument         : const char *str
//					: 기준 문자열
// Argument         : const char *inc
//					: 비교 문자열
bool strinc(const char *str, const char *inc)
{
	if(str == NULL || inc == NULL) return false;

	int str_len = strlen(str);
	int inc_len = strlen(inc);

	if (str_len < inc_len)
		return false;

	for (int i = 0; i <= str_len - inc_len; i++)
	{
		if (!strncmp(str+i, inc, inc_len))
			return true;
	}

	return false;
}



// 게임 서버 전용 함수들
#ifdef __GAME_SERVER__

//////////////////////////////////////////
// 아이템 시리얼 관련

////////////////////
// Function name	: GetSerial
// Description	    : 시리얼 넘버를 구함
// Return type		: void 
//					: 
// Argument         : char *buf
//					: 
// Argument         : int index
//					: 
void GetSerial(char *buf, int index)
{
	int temp = (gserver.m_serverno * 20 + gserver.m_subno) % 62; // (a_WorldSubNum % 50 + a_ServerNum);
	if (temp < 0) temp=0;
	buf[0] = code62[temp % 62];
	buf[1] = code62[gserver.m_serverTime.year % 62];
	buf[2] = code62[gserver.m_serverTime.month % 62];
	buf[3] = code62[gserver.m_serverTime.day % 62];
	buf[4] = code62[gserver.m_serverTime.hour % 62];
	buf[5] = code62[gserver.m_serverTime.min % 62];
	
	if (index < 0) index = -index;
	index %= 238328;
	buf[8] = code62[index % 62];	index /= 62;
	buf[7] = code62[index % 62];	index /= 62;
	buf[6] = code62[index % 62];
	buf[9] = '\0';
}
void GetSerial(CLCString& buf, int index)
{
	char tmp[MAX_SERIAL_LENGTH + 1];
	GetSerial(tmp, index);
	buf = tmp;
}


////////////////////
// Function name	: GetDistance
// Description	    : 캐릭터 사이의 거리 계산
// Return type		: float 
//					: 거리 반환
// Argument         : CCharacter* ch
//					: 기준 캐릭터
// Argument         : CCharacter* tch
//					: 측정 캐릭터
float GetDistance(CCharacter* ch, CCharacter* tch)
{
	if (ch == tch)
		return 0;
	if (ch == NULL || tch == NULL)
		return 0;
#ifdef NIGHTSHADOW_SKILL	// 나이트 쉐도우의 몬스터 시스템
	if (DEAD(tch))
		return 0;
#endif  // NIGHTSHADOW_SKILL

	float ret = GetDistance(GET_X(ch), GET_X(tch), GET_Z(ch), GET_Z(tch), GET_H(ch), GET_H(tch)) - ch->GetSize() - tch->GetSize();
	return (ret > 0.0f) ? ret : 0.0f;
}


////////////////////
// Function name	: GetDistance
// Description	    : 특정 좌표와 캐릭터 사이의 거리
float GetDistance(float x, float z, float h, CCharacter* tch)
{
	float ret = GetDistance(x, GET_X(tch), z, GET_Z(tch), h, GET_H(tch)) - tch->GetSize();
	return (ret > 0.0f) ? ret : 0.0f;
}

#endif

const char* DAnyOneArg(const char *argument, char *first_arg)
{
	while (*argument && (*argument != '.'))
	{
			*(first_arg++) = *argument;
		    argument++;
	}

	*first_arg = '\0';
	argument++;
	return (argument);
}

void IPtoi(const char *org_IP,int *i_one, int *i_two, int *i_thr, int *i_for)
{
	const char *p_IP;
	char tempStr[10];

	p_IP = org_IP;
	p_IP = DAnyOneArg(p_IP,tempStr);
	*i_one = atoi(tempStr);
	p_IP = DAnyOneArg(p_IP,tempStr);
	*i_two = atoi(tempStr);
	p_IP = DAnyOneArg(p_IP,tempStr);
	*i_thr = atoi(tempStr);
	*i_for = atoi(p_IP);

}

#if defined(LC_KOR) || defined(LC_CHN)

bool CheckIP(const char *u_IP)
{
	// 사내 아이피 대역은 제외
	// 대만 아이피 막기

	// BClassIp: B Class 하위 65536개 모두 막기 xxx.xxx.nnn.xxx
	// FirType: C Class 하위 256개 막기 xxx.xxx.xxx.nnn
	// SecType: B Class 일부 xxx.xxx.nnn.xxx ~ xxx.xxx.mmm.xxx
	// ThrType: A Class 일부 xxx.nnn.xxx.xxx ~ xxx.mmm.xxx.xxx
	// 순서는 thr, sec, fir, bclass
	int BClassIp[64][2] = {
		{222,250},		{210,85},		{218,184},		{218,35},
		{218,34},		{210,58},		{61,20},		{163,29},
		{210,69},		{211,23},		{210,65},		{203,69},
		{61,216},		{211,20},		{203,75},		{60,248},
		{61,217},		{211,22},		{211,21},		{210,61},
		{203,74},		{203,66},		{220,229},		{218,32},
		{61,66},		{220,228},		{219,80},		{219,81},
		{61,30},		{61,31},		{221,169},		{210,68},
		{203,73},		{210,64},		{203,67},		{61,59},
		{210,66},		{203,70},		{219,84},		{219,85},
		{61,62},		{140,135},		{140,132},		{140,129},
		{140,113},		{140,110},		{210,240},		{203,71},
		{140,138},		{140,133},		{140,130},		{140,115},
		{140,111},		{210,60},		{203,72},		{203,68},
		{140,114},		{140,134},		{140,131},		{140,128},
		{140,112},		{140,109},		{210,70},		{203,64}
	};	

	
	int FirType[2][3] = {
		{202,3,129},	{203,119,3}
	};

	int SecType[183][3] = {
		{202,8,14},		{202,5,4},		{202,6,104},	{202,5,12},
		{202,5,8},		{210,62,248},	{210,67,248},	{210,62,240},
		{211,79,192},	{61,64,32},		{61,57,240},	{61,57,192},
		{61,67,112},	{61,56,48},		{61,61,192},	{61,57,208},
		{61,60,240},	{61,58,48},		{61,58,16},		{211,79,96},
		{211,78,80},	{61,56,64},		{61,56,80},		{210,67,96},
		{211,78,240},	{202,168,192},	{61,64,48},		{210,62,176},
		{211,76,224},	{211,76,96},	{61,56,16},		{61,57,144},
		{61,67,48},		{61,56,0},		{211,78,64},	{61,67,0},
		{211,79,32},	{211,79,208},	{61,61,240},	{211,78,0},
		{211,78,16},	{61,57,160},	{61,57,176},	{61,60,208},
		{61,58,0},		{210,62,160},	{211,79,48},	{61,56,96},
		{211,76,160},	{211,76,240},	{211,76,112},	{61,58,112},
		{61,57,224},	{211,78,208},	{61,56,112},	{61,60,192},
		{211,76,144},	{61,56,128},	{61,56,144},	{211,78,192},
		{211,76,128},	{210,67,128},	{202,39,96},	{210,67,0},
		{210,67,112},	{210,67,16},	{210,62,224},	{211,78,224},
		{61,58,80},		{61,58,64},		{61,60,224},	{210,67,192},
		{210,67,208},	{210,67,144},	{61,58,32},		{61,56,32},
		{61,58,96},		{211,76,176},	{61,67,96},		{210,63,224},
		{210,63,192},	{211,76,192},	{211,73,32},	{61,63,128},
		{61,60,128},	{61,60,160},	{210,62,96},	{210,241,160},
		{211,73,128},	{210,241,192},	{203,65,160},	{210,62,0},
		{210,62,32},	{210,62,192},	{210,67,160},	{61,57,32},
		{211,79,128},	{61,67,64},		{211,79,160},	{61,60,0},
		{203,65,128},	{210,241,224},	{211,73,96},	{61,63,0},
		{211,78,32},	{210,244,192},	{211,76,0},		{210,244,224},
		{211,73,64},	{61,56,224},	{211,78,160},	{61,56,192},
		{211,78,128},	{211,73,192},	{203,65,192},	{211,73,224},
		{210,67,64},	{210,241,128},	{61,64,0},		{211,73,0},
		{211,79,64},	{210,67,32},	{210,63,0},		{210,63,32},
		{210,62,64},	{61,58,160},	{61,63,96},		{61,67,16},
		{61,58,128},	{211,78,96},	{202,160,64},	{210,243,96},
		{210,63,160},	{211,79,0},		{210,63,128},	{61,57,0},
		{210,243,64},	{61,56,160},	{211,76,32},	{61,61,208},
		{203,65,224},	{211,77,64},	{211,77,0},		{210,241,0},
		{210,241,64},	{61,65,192},	{220,128,0},	{61,63,32},
		{210,63,64},	{61,58,192},	{61,65,128},	{61,64,64},
		{210,243,0},	{61,57,64},		{61,63,160},	{61,60,32},
		{221,120,0},	{202,39,0},		{202,178,128},	{222,251,0},
		{211,77,128},	{203,65,0},		{211,75,128},	{210,59,128},
		{211,72,128},	{211,72,0},		{210,242,0},	{210,71,128},
		{202,39,128},	{211,75,0},		{210,242,128},	{61,67,128},
		{61,65,0},		{211,74,0},		{211,74,128},	{210,244,0},
		{210,243,128},	{61,64,128},	{210,71,0},		{210,59,0},
		{210,17,0},		{220,128,64},	{61,61,0}
	};
	int SecTypeLimit[183] = {
		15,		5,		105,	15,
		11,		255,	255,	247,
		207,	47,		255,	207,
		127,	63,		207,	223,
		255,	63,		31,		111,
		95,		79,		95,		111,
		255,	207,	63,		191,
		239,	111,	31,		159,
		63,		15,		79,		15,
		47,		223,	255,	15,
		31,		175,	191,	223,
		15,		175,	63,		111,
		175,	255,	127,	127,
		239,	223,	127,	207,
		159,	143,	159,	207,
		143,	143,	111,	15,
		127,	31,		239,	239,
		95,		79,		239,	207,
		223,	159,	47,		47,
		111,	191,	111,	255,
		223,	223,	63,		159,
		159,	191,	127,	191,
		159,	223,	191,	31,
		63,		223,	191,	63,
		159,	95,		191,	31,
		159,	255,	127,	31,
		63,		223,	31,		255,
		95,		255,	191,	223,
		159,	223,	223,	255,
		95,		159,	31,		31,
		95,		63,		31,		63,
		95,		191,	127,	47,
		159,	127,	95,		127,
		191,	31,		159,	31,
		95,		191,	63,		239,
		255,	127,	63,		63,
		127,	255,	63,		95,
		127,	255,	191,	127,
		63,		127,	255,	127,
		95,		95,		255,	127,
		255,	127,	255,	255,
		255,	127,	127,	255,
		255,	127,	255,	255,
		127,	127,	255,	127,
		255,	255,	127,	127,
		127,	255,	191
	};

	int ThrType[17][2] = {
		{218,166},		{61,218},		{218,164},		{59,104},
		{140,136},		{163,30},		{61,220},		{59,120},
		{61,224},		{218,160},		{61,228},		{140,116},
		{59,112},		{218,168},		{140,120},		{220,129},
		{163,13}
	};
	int ThrTypeLimit[17] = {
		167,		219,		165,		105,
		137,		32,			223,		123,
		227,		163,		231,		119,
		119,		175,		127,		143,
		28
	};

	char org_IP[20];
	int i_Ip[4];
	int i;
	
	strcpy(org_IP,u_IP);
	IPtoi(org_IP,&i_Ip[0],&i_Ip[1],&i_Ip[2],&i_Ip[3]);
	
	if (i_Ip[0] == 61 && i_Ip[1] == 104 && i_Ip[2] == 44) //t 피씨방은 풀어주자
		return true;

#ifdef LC_KOR
	// 예외 : 220.130.35.*
	if (i_Ip[0] == 220 && i_Ip[1] == 130 && i_Ip[2] == 35)
		return true;

	// 중국 장사치 IP ( 211.101.192.19 , 116.228.21.250 )
	if (i_Ip[0] == 211 && i_Ip[1] == 101 && i_Ip[2] == 192 && i_Ip[3] == 19 )
		return true;
	if (i_Ip[0] == 116 && i_Ip[1] == 228 && i_Ip[2] == 21 && i_Ip[3] == 250 )
		return true;
// 050225 : bs : 대만에서 테스트 서버 접속 허용
#ifdef TEST_SERVER
	// 예외 : 220.130.170.145 ~ 158
	//if (i_Ip[0] == 220 && i_Ip[1] == 130 && i_Ip[2] == 170 && i_Ip[3] >= 145 && i_Ip[3] <= 158)
	//	return true;
	if (i_Ip[0] == 60 && i_Ip[1] == 248 && i_Ip[2] == 77 && i_Ip[3] == 245)
		return true;
#endif
// --- 050225 : bs : 대만에서 테스트 서버 접속 허용
#endif

	for(i = 0; i < 17; i++)
	{
		if(ThrType[i][0] == i_Ip[0])
			if((ThrType[i][1] <= i_Ip[1]) && (i_Ip[1] <= ThrTypeLimit[i]))
				return false;
	}
	for(i = 0; i < 183; i++)
	{
		if(SecType[i][0] == i_Ip[0] && SecType[i][1] == i_Ip[1])
			if((SecType[i][2] <= i_Ip[2]) && (i_Ip[2] <= SecTypeLimit[i]))
				return false;
	}
	for(i = 0; i < 2; i++)
	{
		if(FirType[i][0] == i_Ip[0] && FirType[i][1] == i_Ip[1] 
										&& FirType[i][2] == i_Ip[2])
			return false;
	}
	for(i = 0; i < 64; i++)
	{
		if(BClassIp[i][0] == i_Ip[0] && BClassIp[i][1] == i_Ip[1])
			return false;
	}

	return true;

} //한국IP막기 여기까지 
/*
#elif defined(LC_TWN)

bool CheckIP(const char *u_IP)
{
	// 대만 아이피 대역만 가능하게 하기
	// BClassIp: B Class 하위 65536개 모두 막기 xxx.xxx.nnn.xxx
	// FirType: C Class 하위 256개 막기 xxx.xxx.xxx.nnn
	// SecType: B Class 일부 xxx.xxx.nnn.xxx ~ xxx.xxx.mmm.xxx
	// ThrType: A Class 일부 xxx.nnn.xxx.xxx ~ xxx.mmm.xxx.xxx
	// 순서는 thr, sec, fir, bclass
	int BClassIp[64][2] = {
		{222,250},		{210,85},		{218,184},		{218,35},
		{218,34},		{210,58},		{61,20},		{163,29},
		{210,69},		{211,23},		{210,65},		{203,69},
		{61,216},		{211,20},		{203,75},		{60,248},
		{61,217},		{211,22},		{211,21},		{210,61},
		{203,74},		{203,66},		{220,229},		{218,32},
		{61,66},		{220,228},		{219,80},		{219,81},
		{61,30},		{61,31},		{221,169},		{210,68},
		{203,73},		{210,64},		{203,67},		{61,59},
		{210,66},		{203,70},		{219,84},		{219,85},
		{61,62},		{140,135},		{140,132},		{140,129},
		{140,113},		{140,110},		{210,240},		{203,71},
		{140,138},		{140,133},		{140,130},		{140,115},
		{140,111},		{210,60},		{203,72},		{203,68},
		{140,114},		{140,134},		{140,131},		{140,128},
		{140,112},		{140,109},		{210,70},		{203,64}
	};	

	
	int FirType[2][3] = {
		{202,3,129},	{203,119,3}
	};

	int SecType[183][3] = {
		{202,8,14},		{202,5,4},		{202,6,104},	{202,5,12},
		{202,5,8},		{210,62,248},	{210,67,248},	{210,62,240},
		{211,79,192},	{61,64,32},		{61,57,240},	{61,57,192},
		{61,67,112},	{61,56,48},		{61,61,192},	{61,57,208},
		{61,60,240},	{61,58,48},		{61,58,16},		{211,79,96},
		{211,78,80},	{61,56,64},		{61,56,80},		{210,67,96},
		{211,78,240},	{202,168,192},	{61,64,48},		{210,62,176},
		{211,76,224},	{211,76,96},	{61,56,16},		{61,57,144},
		{61,67,48},		{61,56,0},		{211,78,64},	{61,67,0},
		{211,79,32},	{211,79,208},	{61,61,240},	{211,78,0},
		{211,78,16},	{61,57,160},	{61,57,176},	{61,60,208},
		{61,58,0},		{210,62,160},	{211,79,48},	{61,56,96},
		{211,76,160},	{211,76,240},	{211,76,112},	{61,58,112},
		{61,57,224},	{211,78,208},	{61,56,112},	{61,60,192},
		{211,76,144},	{61,56,128},	{61,56,144},	{211,78,192},
		{211,76,128},	{210,67,128},	{202,39,96},	{210,67,0},
		{210,67,112},	{210,67,16},	{210,62,224},	{211,78,224},
		{61,58,80},		{61,58,64},		{61,60,224},	{210,67,192},
		{210,67,208},	{210,67,144},	{61,58,32},		{61,56,32},
		{61,58,96},		{211,76,176},	{61,67,96},		{210,63,224},
		{210,63,192},	{211,76,192},	{211,73,32},	{61,63,128},
		{61,60,128},	{61,60,160},	{210,62,96},	{210,241,160},
		{211,73,128},	{210,241,192},	{203,65,160},	{210,62,0},
		{210,62,32},	{210,62,192},	{210,67,160},	{61,57,32},
		{211,79,128},	{61,67,64},		{211,79,160},	{61,60,0},
		{203,65,128},	{210,241,224},	{211,73,96},	{61,63,0},
		{211,78,32},	{210,244,192},	{211,76,0},		{210,244,224},
		{211,73,64},	{61,56,224},	{211,78,160},	{61,56,192},
		{211,78,128},	{211,73,192},	{203,65,192},	{211,73,224},
		{210,67,64},	{210,241,128},	{61,64,0},		{211,73,0},
		{211,79,64},	{210,67,32},	{210,63,0},		{210,63,32},
		{210,62,64},	{61,58,160},	{61,63,96},		{61,67,16},
		{61,58,128},	{211,78,96},	{202,160,64},	{210,243,96},
		{210,63,160},	{211,79,0},		{210,63,128},	{61,57,0},
		{210,243,64},	{61,56,160},	{211,76,32},	{61,61,208},
		{203,65,224},	{211,77,64},	{211,77,0},		{210,241,0},
		{210,241,64},	{61,65,192},	{220,128,0},	{61,63,32},
		{210,63,64},	{61,58,192},	{61,65,128},	{61,64,64},
		{210,243,0},	{61,57,64},		{61,63,160},	{61,60,32},
		{221,120,0},	{202,39,0},		{202,178,128},	{222,251,0},
		{211,77,128},	{203,65,0},		{211,75,128},	{210,59,128},
		{211,72,128},	{211,72,0},		{210,242,0},	{210,71,128},
		{202,39,128},	{211,75,0},		{210,242,128},	{61,67,128},
		{61,65,0},		{211,74,0},		{211,74,128},	{210,244,0},
		{210,243,128},	{61,64,128},	{210,71,0},		{210,59,0},
		{210,17,0},		{220,128,64},	{61,61,0}
	};
	int SecTypeLimit[183] = {
		15,		5,		105,	15,
		11,		255,	255,	247,
		207,	47,		255,	207,
		127,	63,		207,	223,
		255,	63,		31,		111,
		95,		79,		95,		111,
		255,	207,	63,		191,
		239,	111,	31,		159,
		63,		15,		79,		15,
		47,		223,	255,	15,
		31,		175,	191,	223,
		15,		175,	63,		111,
		175,	255,	127,	127,
		239,	223,	127,	207,
		159,	143,	159,	207,
		143,	143,	111,	15,
		127,	31,		239,	239,
		95,		79,		239,	207,
		223,	159,	47,		47,
		111,	191,	111,	255,
		223,	223,	63,		159,
		159,	191,	127,	191,
		159,	223,	191,	31,
		63,		223,	191,	63,
		159,	95,		191,	31,
		159,	255,	127,	31,
		63,		223,	31,		255,
		95,		255,	191,	223,
		159,	223,	223,	255,
		95,		159,	31,		31,
		95,		63,		31,		63,
		95,		191,	127,	47,
		159,	127,	95,		127,
		191,	31,		159,	31,
		95,		191,	63,		239,
		255,	127,	63,		63,
		127,	255,	63,		95,
		127,	255,	191,	127,
		63,		127,	255,	127,
		95,		95,		255,	127,
		255,	127,	255,	255,
		255,	127,	127,	255,
		255,	127,	255,	255,
		127,	127,	255,	127,
		255,	255,	127,	127,
		127,	255,	191
	};

	int ThrType[17][2] = {
		{218,166},		{61,218},		{218,164},		{59,104},
		{140,136},		{163,30},		{61,220},		{59,120},
		{61,224},		{218,160},		{61,228},		{140,116},
		{59,112},		{218,168},		{140,120},		{220,129},
		{163,13}
	};
	int ThrTypeLimit[17] = {
		167,		219,		165,		105,
		137,		32,			223,		123,
		227,		163,		231,		119,
		119,		175,		127,		143,
		28
	};

	char org_IP[20];
	int i_Ip[4];
	int i;
	
	strcpy(org_IP,u_IP);
	IPtoi(org_IP,&i_Ip[0],&i_Ip[1],&i_Ip[2],&i_Ip[3]);
	
	if (i_Ip[0] == 61 && i_Ip[1] == 104 && i_Ip[2] == 44) //나코 피씨방은 풀어주자
		return true;

	for(i = 0; i < 17; i++)
	{
		if(ThrType[i][0] == i_Ip[0])
			if((ThrType[i][1] <= i_Ip[1]) && (i_Ip[1] <= ThrTypeLimit[i]))
				return true;
	}
	for(i = 0; i < 183; i++)
	{
		if(SecType[i][0] == i_Ip[0] && SecType[i][1] == i_Ip[1])
			if((SecType[i][2] <= i_Ip[2]) && (i_Ip[2] <= SecTypeLimit[i]))
				return true;
	}
	for(i = 0; i < 2; i++)
	{
		if(FirType[i][0] == i_Ip[0] && FirType[i][1] == i_Ip[1] 
										&& FirType[i][2] == i_Ip[2])
			return true;
	}
	for(i = 0; i < 64; i++)
	{
		if(BClassIp[i][0] == i_Ip[0] && BClassIp[i][1] == i_Ip[1])
			return true;
	}

	return false;
}
*/
#elif defined(LC_JPN)

bool CheckIP(const char *u_IP)
{
	// 다음 대역에 속하는 ip는 접근을 제한한다
	// IPv4에서 각 아이피 자리를 숫자로 변경하여 그 범위를 검사한다.
	// 1.2.3.4는 (1 << 24) | (2 << 16) | (3 << 8) | 4로 계산한다
	static unsigned int tableBlockIP[][2] = {
		{3544711168UL, 3544973311UL},
		{3538944000UL, 3539468287UL},
		{3536846848UL, 3536928767UL},
		{3536322560UL, 3536347135UL},
		{3535798272UL, 3535863807UL},
		{3527081984UL, 3527933951UL},
		{3414491136UL, 3414507519UL},
		{3412787200UL, 3412803583UL},
		{3412033536UL, 3412066303UL},
		{3410952192UL, 3411017727UL},
		{3410886656UL, 3410894847UL},
		{3410821120UL, 3410853887UL},
		{3409969152UL, 3410755583UL},
		{3414491136UL, 3414507519UL},
		{3412787200UL, 3412803583UL},
		{3412033536UL, 3412066303UL},
		{3410952192UL, 3411017727UL},
		{3410886656UL, 3410894847UL},
		{3410821120UL, 3410853887UL},
		{3409969152UL, 3410755583UL},
		{3414491136UL, 3414507519UL},
		{3412787200UL, 3412803583UL},
		{3412033536UL, 3412066303UL},
		{3410952192UL, 3411017727UL},
		{3410886656UL, 3410894847UL},
		{3410821120UL, 3410853887UL},
		{3409969152UL, 3410755583UL},
		{3389917184UL, 3389919231UL},
		{3389382656UL, 3389390847UL},
		{3389142016UL, 3389143039UL},
		{2355953664UL, 2357919743UL},
		{2355101696UL, 2355167231UL},
		{2354839552UL, 2354905087UL},
		{2343501824UL, 2343567359UL},
		{2261778432UL, 2261843967UL},
		{2245984256UL, 2346713087UL},
		{3345708800UL, 3345709055UL},
		{3278942208UL, 3278942211UL},
		{3262474071UL, 3262474071UL},
		{3262473995UL, 3262473995UL},
		{3262474001UL, 3262474001UL},
		{3233590016UL, 3233590271UL},
		{3262474116UL, 3262474116UL},
		{3262474009UL, 3262474009UL},
		{2168848384UL, 2168913919UL},
		{889192448UL,  905969663UL},
		{2168782848UL, 2168848383UL},
		{2213937152UL, 2214002687UL},
		{3412721664UL, 3412738047UL},
		{2710437888UL, 2710503423UL},
		{2710450176UL, 2710450431UL},
		{3412721664UL, 3412738047UL},
		{3412721664UL, 3412738047UL},
		{3389603840UL, 3389612031UL},
		{3389021696UL, 3389021951UL},
		{3546152960UL, 3546808319UL},
		{3527933952UL, 3528458239UL},
		{3412000768UL, 3412017151UL},
		{3411935232UL, 3411951615UL},
		{3411804160UL, 3411820543UL},
		{3411673088UL, 3411689471UL},
		{3411607552UL, 3411623935UL},
		{3411533824UL, 3411558399UL},
		{3412000768UL, 3412017151UL},
		{3411935232UL, 3411951615UL},
		{3411804160UL, 3411820543UL},
		{3411738624UL, 3411755007UL},
		{3411673088UL, 3411689471UL},
		{3411533824UL, 3411558399UL},
		{3412000768UL, 3412017151UL},
		{3411935232UL, 3411951615UL},
		{3411804160UL, 3411820543UL},
		{3411738624UL, 3411755007UL},
		{3411673088UL, 3411689471UL},
		{3411533824UL, 3411558399UL},
		{3411533824UL, 3411558399UL},
		{3389025792UL, 3389026047UL},
		{2682388480UL, 2682454015UL},
		{2714697728UL, 2714763263UL},
		{2682388736UL, 2682389503UL},
		{3233589760UL, 3233590015UL},
		{3389957120UL, 3389957375UL},
		{3389956608UL, 3389957119UL},
		{3389945088UL, 3389945343UL},
		{3389939200UL, 3389939455UL},
		{3389938688UL, 3389939199UL},
		{3389938176UL, 3389938687UL},
		{3389813760UL, 3389814015UL},
		{3389808896UL, 3389809151UL},
		{3389803008UL, 3389803263UL},
		{3389802752UL, 3389803007UL},
		{3389540352UL, 3389541375UL},
		{3389538304UL, 3389540351UL},
		{3389419264UL, 3389419519UL},
		{3389418240UL, 3389418495UL},
		{3389416448UL, 3389416959UL},
		{3389413888UL, 3389414143UL},
		{3389027584UL, 3389028607UL},
		{3389026048UL, 3389026303UL},
		{3389017856UL, 3389018111UL},
		{3233590272UL, 3233590527UL},
		{3542089728UL, 3543400447UL},
		{3537371136UL, 3537895423UL},
		{3536584704UL, 3536846847UL},
		{3534880768UL, 3535298559UL},
		{3529113600UL, 3531603967UL},
		{3389957376UL, 3389957631UL},
		{3389941504UL, 3389941759UL},
		{3389415168UL, 3389415423UL},
		{3534749696UL, 3534815231UL},
		{3528980480UL, 3528982527UL},
		{3414417408UL, 3414425599UL},
		{3412656128UL, 3412672511UL},
		{3411730432UL, 3411738623UL},
		{3411017728UL, 3411476479UL},
		{3414417408UL, 3414425599UL},
		{3412656128UL, 3412672511UL},
		{3411730432UL, 3411738623UL},
		{3411017728UL, 3411476479UL},
		{3414417408UL, 3414425599UL},
		{3412656128UL, 3412672511UL},
		{3411730432UL, 3411738623UL},
		{3411017728UL, 3411476479UL},
		{3389971968UL, 3389972223UL},
		{3389935616UL, 3389935871UL},
		{3389932544UL, 3389932799UL},
		{3389932288UL, 3389932543UL},
		{3389785088UL, 3389786111UL},
		{3389489152UL, 3389497343UL},
		{3389300480UL, 3389317119UL},
		{3389145088UL, 3389153279UL},
		{3389136896UL, 3389142015UL},
		{3389064704UL, 3389065215UL},
		{3389043712UL, 3389044735UL},
		{3389038592UL, 3389043711UL},
		{3389030400UL, 3389035519UL},
		{3389028864UL, 3389029375UL},
		{3389026304UL, 3389026559UL},
		{3389024256UL, 3389024511UL},
		{3389023232UL, 3389023487UL},
		{3389017344UL, 3389017855UL},
		{3329272064UL, 3329272319UL},
		{3237335040UL, 3237335295UL},
		{3234814720UL, 3234814975UL},
		{3234588672UL, 3234588927UL},
		{2815950848UL, 2816016383UL},
		{2705326080UL, 2705391615UL},
		{2662727680UL, 2662793215UL},
		{2659450880UL, 2659516415UL},
		{2466775040UL, 2466840575UL},
		{2429943808UL, 2430009343UL},
		{2404974592UL, 2405040127UL},
		{2310864896UL, 2310930431UL},
		{180813824UL,  184418303UL},
		{3530844672UL,	3530844927UL},
		{3555278848UL,	3555279103UL},
		{3542863872UL,	3542864127UL},
		{3733454848UL,	3733979135UL},
		{1038352384UL,	1038614527UL},
		{3708125184UL,	3708157951UL},
		{3659005952UL,	3659137023UL},
		{3542863908UL,	3542863908UL},


	};
	char org_IP[20];
	int i_Ip[4];
	unsigned int i;
	
	strcpy(org_IP,u_IP);
	IPtoi(org_IP,&i_Ip[0],&i_Ip[1],&i_Ip[2],&i_Ip[3]);

	unsigned int nIPAddr	= ((unsigned int)i_Ip[0] << 24)
							| ((unsigned int)i_Ip[1] << 16)
							| ((unsigned int)i_Ip[2] <<  8)
							| ((unsigned int)i_Ip[3]      );
	
	if (i_Ip[0] == 61 && i_Ip[1] == 104 && i_Ip[2] == 44) //나코 피씨방은 풀어주자
		return true;

	for (i = 0; i < sizeof(tableBlockIP) / sizeof(unsigned int); i++)
	{
		if (tableBlockIP[i][0] <= nIPAddr && nIPAddr <= tableBlockIP[i][1])
			return false;
	}

	return true;
}

#elif defined( LC_USA )
bool CheckIP(const char *u_IP)
{
	// 다음 대역에 속하는 ip는 접근을 제한한다
//	// IPv4에서 각 아이피 자리를 숫자로 변경하여 그 범위를 검사한다.
//	// 1.2.3.4는 (1 << 24) | (2 << 16) | (3 << 8) | 4로 계산한다
//	static unsigned int tableBlockIP[][2] = {
//		{1441693696UL,1441693950UL},
//		{2337406976UL,2337407230UL},
//		{2402680832UL,2402681086UL},
//		{2406088704UL,2406088958UL},
//		{2406219776UL,2406220030UL},
//		{2417426432UL,2417426686UL},
//		{2458255360UL,2458255614UL},
//		{2460221440UL,2460221694UL},
//		{2470510592UL,2470510846UL},
//		{2527133696UL,2527133950UL},
//		{2527199232UL,2527199486UL},
//		{2527330304UL,2527330558UL},
//		{2555641856UL,2555642110UL},
//		{2556166144UL,2556166398UL},
//		{2614296576UL,2614296830UL},
//		{2639659008UL,2639659262UL},
//		{2702704640UL,2702704894UL},
//		{2706309120UL,2706309374UL},
//		{2710831104UL,2710831358UL},
//		{2754150400UL,2754150654UL},
//		{2757033984UL,2757034238UL},
//		{2856452096UL,2856452350UL},
//		{3170893824UL,3170894078UL},
//		{3226521856UL,3226522110UL},
//		{3228558592UL,3228558846UL},
//		{3228558848UL,3228559102UL},
//		{3229885184UL,3229885438UL},
//		{3230833920UL,3230834174UL},
//		{3230852352UL,3230852606UL},
//		{3230913024UL,3230913278UL},
//		{3230915072UL,3230915326UL},
//		{3231275008UL,3231275262UL},
//		{3231283200UL,3231283454UL},
//		{3231292160UL,3231292414UL},
//		{3231675392UL,3231675646UL},
//		{3231722752UL,3231723006UL},
//		{3231724032UL,3231724286UL},
//		{3231739648UL,3231739902UL},
//		{3231744000UL,3231744254UL},
//		{3231759360UL,3231759614UL},
//		{3233549056UL,3233549310UL},
//		{3233684992UL,3233685246UL},
//		{3234065664UL,3234065918UL},
//		{3234203648UL,3234203902UL},
//		{3234841088UL,3234841342UL},
//		{3234842624UL,3234842878UL},
//		{3234843648UL,3234843902UL},
//		{3234844160UL,3234844414UL},
//		{3235856384UL,3235856638UL},
//		{3236393472UL,3236393726UL},
//		{3236393984UL,3236394238UL},
//		{3236395008UL,3236395262UL},
//		{3236409088UL,3236409342UL},
//		{3236409344UL,3236409598UL},
//		{3322683392UL,3322683646UL},
//		{3323033600UL,3323033854UL},
//		{3323062016UL,3323062270UL},
//		{3323062272UL,3323062526UL},
//		{3325132800UL,3325133054UL},
//		{3325133824UL,3325134078UL},
//		{3325169664UL,3325169918UL},
//		{3325691904UL,3325692158UL},
//		{3325692928UL,3325693182UL},
//		{3333988608UL,3333988862UL},
//		{3355445248UL,3355445502UL},
//		{3355451392UL,3355451646UL},
//		{3355455488UL,3355455742UL},
//		{3355457536UL,3355457790UL},
//		{3355458560UL,3355458814UL},
//		{3355460352UL,3355460606UL},
//		{3355460608UL,3355460862UL},
//		{3355461632UL,3355461886UL},
//		{3355463936UL,3355464190UL},
//		{3355464960UL,3355465214UL},
//		{3355465216UL,3355465470UL},
//		{3355465984UL,3355466238UL},
//		{3355466240UL,3355466494UL},
//		{3355466752UL,3355467006UL},
//		{3355468800UL,3355469054UL},
//		{3355469312UL,3355469566UL},
//		{3355472384UL,3355472638UL},
//		{3355643904UL,3355644158UL},
//		{3355773184UL,3355773438UL},
//		{3355845376UL,3355845630UL},
//		{3355845632UL,3355845886UL},
//		{3355846656UL,3355846910UL},
//		{3355848704UL,3355848958UL},
//		{3355869184UL,3355869438UL},
//		{3355870208UL,3355870462UL},
//		{3355901952UL,3355902206UL},
//		{3355904000UL,3355904254UL},
//		{3355905024UL,3355905278UL},
//		{3356033024UL,3356033278UL},
//		{3356033536UL,3356033790UL},
//		{3356049664UL,3356049918UL},
//		{3356049920UL,3356050174UL},
//		{3356050432UL,3356050686UL},
//		{3356052480UL,3356052734UL},
//		{3356052992UL,3356053246UL},
//		{3356054528UL,3356054782UL},
//		{3356055552UL,3356055806UL},
//		{3356059136UL,3356059390UL},
//		{3356059648UL,3356059902UL},
//		{3356061696UL,3356061950UL},
//		{3356062208UL,3356062462UL},
//		{3356062720UL,3356062974UL},
//		{3356063744UL,3356063998UL},
//		{3356064512UL,3356064766UL},
//		{3356064768UL,3356065022UL},
//		{3356066048UL,3356066302UL},
//		{3356066304UL,3356066558UL},
//		{3356066816UL,3356067070UL},
//		{3356067840UL,3356068094UL},
//		{3356068864UL,3356069118UL},
//		{3356069632UL,3356069886UL},
//		{3356069888UL,3356070142UL},
//		{3356070912UL,3356071166UL},
//		{3356073472UL,3356073726UL},
//		{3356073984UL,3356074238UL},
//		{3356075008UL,3356075262UL},
//		{3356076288UL,3356076542UL},
//		{3356076544UL,3356076798UL},
//		{3356077056UL,3356077310UL},
//		{3356079360UL,3356079614UL},
//		{3356079616UL,3356079870UL},
//		{3356080128UL,3356080382UL},
//		{3356080640UL,3356080894UL},
//		{3356083968UL,3356084222UL},
//		{3356084224UL,3356084478UL},
//		{3356084736UL,3356084990UL},
//		{3356085760UL,3356086014UL},
//		{3356087808UL,3356088062UL},
//		{3356089344UL,3356089598UL},
//		{3356090368UL,3356090622UL},
//		{3356090880UL,3356091134UL},
//		{3356091648UL,3356091902UL},
//		{3356092928UL,3356093182UL},
//		{3356096768UL,3356097022UL},
//		{3356097024UL,3356097278UL},
//		{3356097536UL,3356097790UL},
//		{3356099584UL,3356099838UL},
//		{3356106752UL,3356107006UL},
//		{3356110848UL,3356111102UL},
//		{3356112896UL,3356113150UL},
//		{3356132352UL,3356132606UL},
//		{3356133376UL,3356133630UL},
//		{3356133888UL,3356134142UL},
//		{3356134656UL,3356134910UL},
//		{3356135424UL,3356135678UL},
//		{3356135936UL,3356136190UL},
//		{3356137728UL,3356137982UL},
//		{3356137984UL,3356138238UL},
//		{3356138496UL,3356138750UL},
//		{3356140288UL,3356140542UL},
//		{3356140544UL,3356140798UL},
//		{3356142848UL,3356143102UL},
//		{3356143104UL,3356143358UL},
//		{3356143616UL,3356143870UL},
//		{3356144640UL,3356144894UL},
//		{3356145408UL,3356145662UL},
//		{3356145920UL,3356146174UL},
//		{3356146432UL,3356146686UL},
//		{3356146944UL,3356147198UL},
//		{3356147456UL,3356147710UL},
//		{3356147712UL,3356147966UL},
//		{3356152064UL,3356152318UL},
//		{3356152320UL,3356152574UL},
//		{3356156672UL,3356156926UL},
//		{3356161280UL,3356161534UL},
//		{3356164096UL,3356164350UL},
//		{3356168192UL,3356168446UL},
//		{3356170240UL,3356170494UL},
//		{3356171264UL,3356171518UL},
//		{3356229632UL,3356229886UL},
//		{3356263168UL,3356263422UL},
//		{3356265216UL,3356265470UL},
//		{3356269824UL,3356270078UL},
//		{3356297216UL,3356297470UL},
//		{3356368896UL,3356369150UL},
//		{3356369664UL,3356369918UL},
//		{3356369920UL,3356370174UL},
//		{3356557312UL,3356557566UL},
//		{3356622848UL,3356623102UL},
//		{3356753920UL,3356754174UL},
//		{3361734656UL,3361734910UL},
//		{3363831808UL,3363832062UL},
//		{3372220416UL,3372220670UL},
//		{3376414720UL,3376414974UL},
//		{3470610432UL,3470610686UL},
//	};
//	char org_IP[20];
//	int i_Ip[4];
//	unsigned int i;
//	
//	strcpy(org_IP,u_IP);
//	IPtoi(org_IP,&i_Ip[0],&i_Ip[1],&i_Ip[2],&i_Ip[3]);
//
//	unsigned int nIPAddr	= ((unsigned int)i_Ip[0] << 24)
//							| ((unsigned int)i_Ip[1] << 16)
//							| ((unsigned int)i_Ip[2] <<  8)
//							| ((unsigned int)i_Ip[3]      );
//
//	if (i_Ip[0] == 61 && i_Ip[1] == 104 && i_Ip[2] == 44) //나코 피씨방은 풀어주자
//		return true;
//	
//	for (i = 0; i < sizeof(tableBlockIP) / sizeof(unsigned int); i++)
//	{
//		if (tableBlockIP[i][0] <= nIPAddr && nIPAddr <= tableBlockIP[i][1])
//			return false;
//	}

static int tableBlockIP[359][2] = {		
			{10,0}, {10,1}, {10,10}, {10,100}, {10,102},
			{10,102}, {10,103}, {10,104}, {10,12}, {10,121},
			{10,13}, {10,145}, {10,147}, {10,159}, {10,16},
			{10,2}, {10,20}, {10,200}, {10,21}, {10,233}, 
			{10,241}, {10,246}, {10,254}, {10,27}, {10,3},
			{10,30}, {10,32}, {10,5}, {10,6}, {10,60},
			{10,67}, {10,70}, {10,87}, {10,9}, {10,91},
			{10,97}, {122,24}, {125,3}, {127,0}, {128,1},
			{134,162}, {139,82}, {143,107}, {143,54}, {15,227},
			{157,86}, {160,15}, {166,79}, {170,0}, {172,0}, 
			{172,1}, {172,100}, {172,145}, {172,146}, {172,16},
			{172,168}, {172,17}, {172,18}, {172,20}, {172,22}, 
			{172,24},  {172,25}, {172,29}, {172,30}, {172,31},
			{172,99}, {189,0}, {189,1}, {189,10}, {189,11},
			{189,12}, {189,13}, {189,14}, {189,15}, {189,16},
			{189,17}, {189,18}, {189,2}, {189,3}, {189,4},
			{189,5}, {189,6}, {189,7}, {192,16}, {192,169},
			{194,247}, {195,212}, {195,8}, {20,0}, {200,0},
			{200,100}, {200,101}, {200,102}, {200,103}, {200,129},
			{200,136}, {200,137}, {200,138}, {200,139}, {200,140},
			{200,141}, {200,142}, {200,143}, {200,144}, {200,146},
			{200,147}, {200,148}, {200,149}, {200,150}, {200,152},
			{200,153}, {200,154},  {200,155}, {200,157}, {200,158},
			{200,159}, {200,160}, {200,161}, {200,162}, {200,163},
			{200,164}, {200,165}, {200,166}, {200,167}, {200,168},
			{200,169}, {200,17}, {200,170}, {200,171}, {200,172},
			{200,174}, {200,175}, {200,176}, {200,177}, {200,178},
			{200,179}, {200,180}, {200,181}, {200,182}, {200,183},
			{200,184}, {200,185}, {200,186}, {200,187}, {200,188},
			{200,189}, {200,191}, {200,192}, {200,193}, {200,194},
			{200,195}, {200,196}, {200,198}, {200,199}, {200,201},
			{200,202}, {200,203}, {200,204}, {200,205}, {200,206},
			{200,207}, {200,208}, {200,209}, {200,210}, {200,211},
			{200,212}, {200,213}, {200,214}, {200,215}, {200,216},
			{200,217}, {200,218}, {200,219}, {200,220}, {200,221},
			{200,222}, {200,223}, {200,225}, {200,226}, {200,227},
			{200,228}, {200,230}, {200,231}, {200,232}, {200,233},
			{200,234}, {200,236}, {200,241}, {200,242}, {200,243},
			{200,244}, {200,245}, {200,246}, {200,247}, {200,248},
			{200,249}, {200,250}, {200,251}, {200,252}, {200,253},
			{200,254}, {200,255}, {200,83}, {200,96}, {200,97},
			{200,98}, {200,99},  {201,0}, {201,1}, {201,10},
			{201,11}, {201,12}, {201,13}, {201,14}, {201,15},
			{201,16}, {201,17}, {201,18},  {201,19}, {201,2},
			{201,20}, {201,21}, {201,22}, {201,23}, {201,24}, 
			{201,25}, {201,26}, {201,27}, {201,28}, {201,29},
			{201,3}, {201,30}, {201,31}, {201,32}, {201,34},
			{201,35}, {201,36}, {201,37}, {201,38}, {201,39},
			{201,4}, {201,40}, {201,41}, {201,42}, {201,43},
			{201,44}, {201,45}, {201,46}, {201,47}, {201,48},
			{201,49}, {201,5}, {201,50}, {201,51}, {201,52},
			{201,53}, {201,54}, {201,55}, {201,56}, {201,57},
			{201,58}, {201,59}, {201,6}, {201,62}, {201,63},
			{201,64}, {201,65}, {201,66}, {201,67}, {201,68},
			{201,69}, {201,7}, {201,70}, {201,71}, {201,72},
			{201,73}, {201,74}, {201,75}, {201,76}, {201,77},
			{201,78}, {201,79}, {201,8}, {201,80}, {201,81},
			{201,82}, {201,83}, {201,84}, {201,86}, {201,87},
			{201,88}, {201,89}, {201,9}, {201,90}, {201,91},
			{201,92}, {201,93}, {201,94}, {201,95}, {205,181},
			{207,138}, {207,180}, {210,199}, {211,121}, {211,3},
			{211,6}, {213,22}, {213,58}, {216,114}, {217,165},
			{217,70}, {218,222}, {218,43}, {219,160}, {219,66},
			{220,105}, {221,27}, {222,11}, {222,2}, {222,3},
			{222,4},  {24,17}, {24,62}, {32,104}, {40,10},
			{41,223}, {60,36}, {61,123}, {61,209}, {61,214},
			{68,81}, {71,228}, {80,0}, {81,153}, {81,193},
			{81,20}, {81,84}, {82,120}, {82,154}, {82,155},
			{83,132},  {83,167}, {83,43}, {83,54}, {84,124},
			{84,90}, {85,140}, {85,240}, {85,241}, {85,242},
			{85,61}, {86,138}, {86,146}, {86,217}, {87,194},
			{87,196}, {88,157}, {89,0}, {89,26},
	};

	char org_IP[20];
	int i_Ip[4];
	unsigned int i;
	
	strcpy(org_IP,u_IP);
	IPtoi(org_IP,&i_Ip[0],&i_Ip[1],&i_Ip[2],&i_Ip[3]);

	if (i_Ip[0] == 61 && i_Ip[1] == 104 && i_Ip[2] == 44) //나코 피씨방은 풀어주자
	{
		return true;
	}
	
	for(i = 0; i < 359; i++)
	{
		if(tableBlockIP[i][0] == i_Ip[0] && tableBlockIP[i][1] == i_Ip[1])
		{
//			//gserver.BlockIPLog( u_IP );
//			CDBCmd dbUser;
//			dbUser.Init( &gserver.m_dbuser );
//
//			sprintf( g_buf, "INSERT INTO t_ip_log (a_ip, a_date) VALUES ( '%s', NOW() )", u_IP );
//			dbUser.SetQuery( g_buf );
//			dbUser.Update();
			return false;	// 미국 IP 블럭과 관련된 자료 조사하기 위해서 return 값을 true로 설정 - false로 변경
		}
		
	}
	return true;
}

#else

bool CheckIP(const char *u_IP)
{
	return true;
}

#endif // CheckIP


#ifdef CHECK_LIMIT_AGE
bool CheckBirthDay(const char* strJumin, int nLimitAge)
{
	bool bRet = true;
	char chy1 = strJumin[0];
	char chy2 = strJumin[1];
	char chm1 = strJumin[2];
	char chm2 = strJumin[3];
	char chd1 = strJumin[4];
	char chd2 = strJumin[5];
	char chc  = strJumin[7];
	if (	   isdigit(chy1)
			&& isdigit(chy2)
			&& isdigit(chm1)
			&& isdigit(chm2)
			&& isdigit(chd1)
			&& isdigit(chd2)
			&& isdigit(chc )
		)
	{
		// 정상 데이터이면 생년월일 계산
		int nJuminYear		= 0;
		int nJuminMonth		= 0;
		int nJuminDay		= 0;

		int y1 = chy1 - '0';
		int y2 = chy2 - '0';
		int m1 = chm1 - '0';
		int m2 = chm2 - '0';
		int d1 = chd1 - '0';
		int d2 = chd2 - '0';
		int c  = chc  - '0';

		switch (c)
		{
		case 1:
		case 2:
			nJuminYear	= 1900 + y1 * 10 + y2;
			nJuminMonth	= m1 * 10 + m2;
			nJuminDay	= d1 * 10 + d2;
			break;

		case 3:
		case 4:
			nJuminYear	= 2000 + y1 * 10 + y2;
			nJuminMonth	= m1 * 10 + m2;
			nJuminDay	= d1 * 10 + d2;
			break;

		default:
			bRet = false;
			break;
		}
		if (nJuminYear < 1 || nJuminMonth < 1 || nJuminDay < 1)
		{
			bRet = false;
		}
		else
		{
			// 생년월일이 정상이면 오늘과 비교
			struct tm tmCurrent = NOW();
			int nCurrentYear	= tmCurrent.tm_year + 1900;
			int nCurrentMonth	= tmCurrent.tm_mon + 1;
			int nCurrentDay		= tmCurrent.tm_mday;
			int nLimitAge		= 15;

			if (nCurrentYear - nJuminYear < nLimitAge)
			{
				// 아직 만 15세 안됨
				bRet = false;
			}
			else if (nCurrentYear - nJuminYear == nLimitAge)
			{
				// 올해 15세 됨
				if (nCurrentMonth - nJuminMonth < 0)
				{
					// 월이 안 지남
					bRet = false;
				}
				else if (nCurrentMonth - nJuminMonth == 0)
				{
					// 일 검사
					if (nCurrentDay - nJuminDay < 0)
					{
						bRet = false;
					}
				}
			}
		}
	}
	else
	{
		bRet = false;
	}

	return bRet;
}
#endif // CHECK_LIMIT_AGE
