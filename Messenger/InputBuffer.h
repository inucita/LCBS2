// InputBuffer.h: interface for the CInputBuffer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INPUTBUFFER_H__A5EDE24B_050E_4EE6_81E1_94310CE034ED__INCLUDED_)
#define AFX_INPUTBUFFER_H__A5EDE24B_050E_4EE6_81E1_94310CE034ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NetMsg.h"
#include "MemPool.h"
#include "Mutex.h"	// Added by ClassView

extern CInputBufferPool g_inbufpool;

class CInputBuffer
{
public:
	unsigned char*	m_buf;
	int				m_ptr;

	CInputBuffer()
	{
		m_buf = g_inbufpool.Get(false);
		m_ptr = 0;
	}

	virtual ~CInputBuffer()
	{
		g_inbufpool.Set(m_buf);
		m_buf = NULL;
		m_ptr = 0;
	}

	void Clear()
	{
		memset(m_buf, 0, MAX_INPUTBUFFER_SIZE);
		m_ptr = 0;
	}

	unsigned char* GetPoint()
	{
		return m_buf + m_ptr;
	}

	int GetRemain()
	{
		return MAX_INPUTBUFFER_SIZE - m_ptr;
	}

	void SetPoint(int dval)
	{
		m_ptr += dval;
	}



	/////////////////////
	// Function name	: GetMessage
	// Description	    : 입력 스트림 버퍼에서 메시지를 얻어온다
	// Return type		: int 
	//            		: 0: 정상, 1: 메시지 없음, -1: 오류
	// Argument         : CNetMsg& msg
	//                  : 얻어온 메시지
	int GetMessage(CNetMsg& msg)
	{
		msg.Init();

		if (m_ptr < (int)MAX_HEADER_SIZE)
			return 1;

		unsigned char* p = m_buf;

		// reliable, seq, id, size
		unsigned short	pack_reliable;
		int				pack_seq;
		unsigned short	pack_id;
		int				size;

		memcpy(&pack_reliable,	p, 2);			p += 2;
		memcpy(&pack_seq,		p, 4);			p += 4;
		memcpy(&pack_id,		p, 2);			p += 2;
		memcpy(&size,			p, 4);			p += 4;

		HTONS(pack_reliable);
		HTONL(pack_seq);
//		HTONS(pack_id);
		HTONL(size);

		int len = size + MAX_HEADER_SIZE;

#ifdef USING_CHECKSUM_MD5
		len += USING_CHECKSUM_MD5;
#endif // USING_CHECKSUM_MD5

#ifdef USING_CHECKSUM_CRC32
		len += USING_CHECKSUM_CRC32;
#endif // USING_CHECKSUM_CRC32

		if (len > m_ptr)
			return 1;

		if (size > MAX_MESSAGE_SIZE || size < 0)
			return -1;

		msg.WriteRaw(p, size);
		p += size;

#ifdef USING_CHECKSUM_MD5
		extern CLCMD5 gMD5;
		gMD5.Init();
		gMD5.Update(msg.m_buf, msg.m_size);
		const unsigned int* pMD5Result = gMD5.Final();
		unsigned int nRecvMD5Result[4] = {0,};
		memcpy(nRecvMD5Result,		p, 4);			p += 4;
		memcpy(nRecvMD5Result + 1,	p, 4);			p += 4;
		memcpy(nRecvMD5Result + 2,	p, 4);			p += 4;
		memcpy(nRecvMD5Result + 3,	p, 4);			p += 4;
		HTONL(nRecvMD5Result[0]);
		HTONL(nRecvMD5Result[1]);
		HTONL(nRecvMD5Result[2]);
		HTONL(nRecvMD5Result[3]);
		if (memcmp(pMD5Result, nRecvMD5Result, USING_CHECKSUM_MD5) != 0)
			return -1;
#endif // USING_CHECKSUM_MD5

#ifdef USING_CHECKSUM_CRC32
		extern CLCCRC32 gCRC32;
		unsigned int nCRCResult = gCRC32.CalcCRC32(msg.m_buf, msg.m_size);
		unsigned int nRecvCRCResult = 0;
		memcpy(&nRecvCRCResult,		p, 4);			p += 4;
		HTONL(nRecvCRCResult);
		if (nCRCResult != nRecvCRCResult)
			return -1;
#endif // USING_CHECKSUM_CRC32

//0707 
		msg.secretkey = pack_id;

		m_ptr -= len;
		if (m_ptr > 0)
		{
			unsigned char* tmp = g_inbufpool.Get(false);
			memcpy(tmp, p, m_ptr);
			g_inbufpool.Set(m_buf);
			m_buf = tmp;
		}
		return 0;
	}
};

#endif // !defined(AFX_INPUTBUFFER_H__A5EDE24B_050E_4EE6_81E1_94310CE034ED__INCLUDED_)
