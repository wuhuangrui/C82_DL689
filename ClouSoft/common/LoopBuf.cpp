/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：LoopBuf.cpp
 * 摘    要：本文件实现循环缓冲区类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年12月
 * 备    注：
 *********************************************************************************************************/
#include "stdafx.h"
#include "string.h"
#include "LoopBuf.h"
#include "Trace.h"
#include "FaCfg.h"

////////////////////////////////////////////////////////////////////////////////////////////
//CLoopBuf

CLoopBuf::CLoopBuf()
{
	m_pbLoopBuf = NULL;	
}

CLoopBuf::~CLoopBuf()
{
	if (m_pbLoopBuf != NULL)
		delete [] m_pbLoopBuf;
}

bool CLoopBuf::Init(WORD wBufSize)
{
	m_wBufSize = wBufSize;
	m_pbLoopBuf = new BYTE[m_wBufSize]; //用来存放一个个的帧
	if (m_pbLoopBuf == NULL)
	{
		DTRACE(DB_CRITICAL, ("CLoopBuf::Init: critical error : sys out of memory !!!!!!\r\n"));
		return false;
	}

	m_wRxHead = 0;
	m_wRxTail = 0;	
	return true;
}

WORD CLoopBuf::GetBufLen()
{
	if (m_wRxHead >= m_wRxTail)
		return m_wRxHead - m_wRxTail;
	else
		return m_wRxHead + m_wBufSize - m_wRxTail;
}

//描述:把串口接收到的数据放到接收循环缓冲区中去
void CLoopBuf::PutToBuf(BYTE* pbBuf, WORD wLen)
{
	WORD WBuffLen=0;
	if (wLen > m_wBufSize)
	{
		DTRACE(DB_FAPROTO, ("CLoopBuf::PutToLoopBuf over,wLen>m_wBufSize.\n"));
		return;
	}
	WBuffLen=(m_wRxHead+m_wBufSize-m_wRxTail)%m_wBufSize;
	if (wLen+WBuffLen > m_wBufSize)
	{
		DTRACE(DB_FAPROTO, ("CLoopBuf::PutToLoopBuf over,(wLen+WBuffLen)>m_wBufSize.\n"));
		return;
	}		

	if (m_wRxHead+wLen <= m_wBufSize)
	{
		memcpy(&m_pbLoopBuf[m_wRxHead], pbBuf, wLen);
	}
	else
	{
		memcpy(&m_pbLoopBuf[m_wRxHead], pbBuf, m_wBufSize-m_wRxHead);
		memcpy(m_pbLoopBuf, pbBuf+m_wBufSize-m_wRxHead, wLen-(m_wBufSize-m_wRxHead));
	}

	m_wRxHead += wLen;
	if (m_wRxHead >= m_wBufSize)
		m_wRxHead -= m_wBufSize;
}


WORD CLoopBuf::RxFromBuf(BYTE* pbRxBuf, WORD wBufSize)
{
	WORD wRetLen;
	if (m_wRxHead != m_wRxTail)   //现在认为接收缓冲区大于等于循环缓冲区
	{
		if (m_wRxHead > m_wRxTail)
		{
			wRetLen = m_wRxHead - m_wRxTail;
			if (wRetLen > wBufSize)
				wRetLen = wBufSize;

			memcpy(pbRxBuf, &m_pbLoopBuf[m_wRxTail], wRetLen);
			return wRetLen;
		}
		else
		{
			//这里把数据的循序从循环缓冲区转换成一般缓冲区的顺序
			wRetLen = m_wBufSize - m_wRxTail;
			if (wRetLen >= wBufSize)
			{
				memcpy(pbRxBuf, &m_pbLoopBuf[m_wRxTail], wBufSize);
				return wBufSize;
			}
			else
			{
				memcpy(pbRxBuf, &m_pbLoopBuf[m_wRxTail], wRetLen);

				if (wRetLen + m_wRxHead > wBufSize)
				{
					memcpy(pbRxBuf+wRetLen, m_pbLoopBuf, wBufSize-wRetLen);
					return wBufSize;
				}
				else
				{
					memcpy(pbRxBuf+wRetLen, m_pbLoopBuf, m_wRxHead);
					return wRetLen+m_wRxHead;
				}
			}
		}

		//取走接收循环缓冲区中的数据后,还不更新循环缓冲区的头尾指针,
		//等RcvBlock()函数真正消费掉后才把该数据从缓冲区中删除
	}
	else
	{
		return 0;
	}
}

//描述:删除已经扫描的数据
void CLoopBuf::DeleteFromBuf(WORD wLen)
{
	WORD wLeft = GetBufLen();
    if (wLen > wLeft)
        wLen = wLeft;
            
	m_wRxTail = (m_wRxTail + wLen) % m_wBufSize;	
}

//描述:清除循环缓冲区现有的数据
void CLoopBuf::ClrBuf()
{
	m_wRxHead = 0;
	m_wRxTail = 0;	
}
