/*********************************************************************************************************
* Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�Trace.cpp
* ժ    Ҫ�����ļ���Ҫ������ϵͳ�µ�������Ĺ����ӿ�
* ��ǰ�汾��1.0
* ��    �ߣ�᯼���
* ������ڣ�2007��8��
* ��    ע��
*********************************************************************************************************/
#include "stdafx.h"
#include "syscfg.h"
#include "sysfs.h"
#include <stdio.h>
#include <ctype.h> 
#include <stdlib.h>
#include <stdarg.h> 
#include "Trace.h"
#include "DbAPI.h"
#include "Comm.h"
#include "ComAPI.h"
#ifdef SYS_LINUX
#include <syslog.h>
#endif

#define DB_BUF_LEN	(1024*16)

TSem   g_semDebug;
BYTE* g_pbDbOffDelay;
BYTE* g_pbDbItem;
DWORD g_dwDebugClick;
bool g_fDbInited = false;
bool g_fTraceEnable = true; 
int  g_fHandle = 0; 
static BYTE g_bDebug2File = 0;
static DWORD g_dwLogLen = 0;
BYTE g_bDbBuf[DB_BUF_LEN];	//ʹ�ù���������������ÿ���̶߳�������ô��Ķ�ջ

//����:��ʼ����ͬϵͳ�µ�����Ͳ�Ĳ���
bool SysInitDebug()
{
	g_semDebug = NewSemaphore(1);

	g_dwDebugClick = GetClick();

#ifdef COMM_DEBUG
	//g_commDebug.Open(COMM_DEBUG, CBR_115200, 8, ONESTOPBIT, NOPARITY);
#endif 

#ifdef SYS_LINUX
	openlog("clou", LOG_PID | LOG_NDELAY, LOG_LOCAL2);
	setlogmask(LOG_UPTO(LOG_INFO));
#endif
	return true; 
}

bool InitDebug()
{
	//TDataItem di = GetItemEx(1, 0, 0x1000);  //0x1000 1 ������Ϣ����ܿ���,1��
	//g_pbDbAllOn = di.pbAddr;

	TDataItem di = GetItemEx(BN1, PN0, 0x1001);  //0x1001 2 ��������Զ��ض�ʱ��,��λ����,0���Զ��ض�
	g_pbDbOffDelay = di.pbAddr;

	di = GetItemEx(BN1, PN0, 0x1002);  //0x1002 16 ����ĵ����������
	g_pbDbItem = di.pbAddr;

	g_fDbInited = true;

	ReadItemEx(BN10, PN0, 0xa1c0, &g_bDebug2File);

	if(g_bDebug2File == 1)
	{
		TTime tm;
		GetCurTime(&tm);
		BYTE  bBuf[512] = {0};
		char pszPathName[64] = {0};
		sprintf(pszPathName, USER_PATH"log");

		g_fHandle = open(pszPathName, O_CREAT|O_RDWR|O_BINARY|O_APPEND, S_IREAD|S_IWRITE);

		g_dwLogLen = lseek(g_fHandle, 0, SEEK_END);

		if(g_dwLogLen>4*1000*1000)  //�ļ�̫�󣬳���4M��
		{
			close(g_fHandle);
			g_fHandle = open(pszPathName, O_CREAT|O_RDWR|O_BINARY|O_TRUNC, S_IREAD|S_IWRITE);
			g_dwLogLen = 0;
		}

		if (g_fHandle < 0)
			return  false;
		
		sprintf((char*)bBuf, "\r\n####################### CL818C7-LOG Start at %d-%d-%d %d:%d:%d #######################\r\n", tm.nYear,tm.nMonth,tm.nDay, tm.nHour,tm.nMinute,tm.nSecond);
		write(g_fHandle, bBuf, strlen((char*)bBuf));

        g_bDebug2File = 2; //��ʼ����ɣ�����д�ļ�û���ź�������������ֻ�е���ʼ����ɺ����������д�ļ�
	}

	return true;
}

bool IsDebugOn(BYTE bType)
{
	if (!g_fTraceEnable) //���ڱ���ͨѶ�ں͵���������ص������,ʹ�ñ��ӿ�������
		return false;

	if (g_fDbInited == false)
		return true;

	if (g_pbDbItem == NULL)//��Щ��g_pbDbItemû�г�ʼ����ʱ���ӡ������
		return false;

	if ((*g_pbDbItem & 0x01) == 0)     //�ܿ��عر�
		return false;

	BYTE flag = g_pbDbItem[bType/8];
	if ((flag & (1<<bType%8)) == 0)    //������������Ϣ
		return false;

	/*WORD wDelayMinutes = *g_pbDbOffDelay + (WORD )*(g_pbDbOffDelay + 1) * 0x100;
	if (wDelayMinutes != 0)
	{
		DWORD dwClick = GetClick();
		
	}*/

	//if (g_bDebug2File && 
	//	( (((GetClick()-g_dwDebugClick)/60)>3*24*60) || (g_dwLogLen>4*1000*1000))) //������3�졢���4M������Ϣ���ļ�/mnt/data/log�У����ȵ���Ϊ׼

	if (g_bDebug2File>0 && 
		(g_dwLogLen>4*1000*1000 || GetClick()>3*24*60*60)) //������3�졢���4M������Ϣ���ļ�/mnt/data/log�У����ȵ���Ϊ׼
	{
		g_bDebug2File = 0;  
		TTime tm;
		GetCurTime(&tm);
		BYTE  bBuf[512] = {0};

		sprintf((char*)bBuf, "\r\n####################### CL818C7-LOG End at %d-%d-%d %d:%d:%d at Click=%d g_dwLogLen=%d#######################\r\n", tm.nYear,tm.nMonth,tm.nDay, tm.nHour,tm.nMinute,tm.nSecond, GetClick(),g_dwLogLen);
		write(g_fHandle, bBuf, strlen((char*)bBuf));
		close(g_fHandle);
		
		WriteItemEx(BN10, PN0, 0xa1c0, &g_bDebug2File);
	}

	return true;
}


void debug_printf( const char *fmt, ... )
{
	va_list varg;

	WaitSemaphore(g_semDebug);

	va_start(varg, fmt ); //include <stdarg.h> va_start,va_arg,va_end����stdarg.h�б�����ɺ�
	vsprintf((char* )g_bDbBuf, fmt, varg);//�͸�ʽ�����������,int vsprintf(char *string, char *format, va_list param); 
	va_end(varg);
	DWORD dwlen = strlen((char* )g_bDbBuf);
	DTRACEOUT((char*)g_bDbBuf, dwlen);

	if (g_bDebug2File == 2)
	{			
		write(g_fHandle, g_bDbBuf, dwlen);
		g_dwLogLen += dwlen;
	}

	SignalSemaphore(g_semDebug);
}


WORD PrintBuf(BYTE* out, BYTE* in, WORD wInLen)
{
	for (int i=0; i<wInLen; i++)
	{
		BYTE b = *in++;
		BYTE hi = b >> 4;
		BYTE lo = b & 0x0f;
		*out++ = ' ';
		if (hi >= 0x0a)
			*out++ = hi - 0x0a + 'A';
		else
			*out++ = hi + '0';

		if (lo >= 0x0a)
			*out++ = lo - 0x0a + 'A';
		else
			*out++ = lo + '0';
	}

	*out++ = 0;
	return wInLen*3;
}


WORD PrintBuf(BYTE* out, WORD wOutLen, BYTE* in, WORD wInLen)
{
	WORD i;
	for (i=0; i<wInLen; i++)
	{
		if ((i+1)*3 > wOutLen)
			return i;

		BYTE b = *in++;
		BYTE hi = b >> 4;
		BYTE lo = b & 0x0f;
		*out++ = ' ';
		if (hi >= 0x0a)
			*out++ = hi - 0x0a + 'A';
		else
			*out++ = hi + '0';

		if (lo >= 0x0a)
			*out++ = lo - 0x0a + 'A';
		else
			*out++ = lo + '0';
	}

	return i;
}


void TraceBuf(WORD wSwitch, char* szHeadStr, BYTE* p, WORD wLen)
{
	WaitSemaphore(g_semDebug);

	TMillTime tMillTime;
	char  cStr[32];
	WORD wStrLen;

	memset(cStr, 0, sizeof(cStr));
	GetCurMillTime(&tMillTime);
	MillTimeToStr(tMillTime, cStr);
	memcpy(g_bDbBuf, (BYTE*)cStr, strlen(cStr));
	wStrLen = strlen(cStr);
	g_bDbBuf[wStrLen] = ' ';
	wStrLen += 1;
	memcpy(g_bDbBuf+wStrLen, szHeadStr, strlen(szHeadStr));
	wStrLen += strlen(szHeadStr);


	for (WORD wPrinted=0; wPrinted<wLen; )
	{
		WORD n = PrintBuf(&g_bDbBuf[wStrLen], 1000-wStrLen, p, wLen-wPrinted);
		p += n;
		wPrinted += n;
		wStrLen += n*3;
		if (n==0 || wStrLen>1000)   //�Ѿ���bBuf[]��������,���Ȱѵ�ǰ���������
		{
			g_bDbBuf[wStrLen++] = '\r';
			g_bDbBuf[wStrLen++] = '\n';
			g_bDbBuf[wStrLen++] = 0;
			STRACE(wSwitch, (char*)g_bDbBuf, wStrLen);

			if(g_bDebug2File==2 && IsDebugOn(wSwitch))
			{
				write(g_fHandle, g_bDbBuf, wStrLen);
				g_dwLogLen += wStrLen;
			}

			wStrLen = 0;
		}
	}

	if (wStrLen > 0)
	{
		g_bDbBuf[wStrLen++] = '\r';
		g_bDbBuf[wStrLen++] = '\n';
		g_bDbBuf[wStrLen++] = 0;
		STRACE(wSwitch, (char*)g_bDbBuf, wStrLen);

		if(g_bDebug2File==2 &&IsDebugOn(wSwitch))
		{
			write(g_fHandle, g_bDbBuf, wStrLen);
			g_dwLogLen += wStrLen;
		}

	}

	SignalSemaphore(g_semDebug);
}

void TraceFrm(char* pszHeader, BYTE* pbBuf, WORD wLen)
{
	if (wLen > 0)
	{
		char szBuf[64];
		sprintf(szBuf, "%s %d\n", pszHeader, wLen);
		TraceBuf(DB_FAPROTO, szBuf, pbBuf, wLen);
	}
}

//����:���ڱ���ͨѶ�ں͵���������ص������,ʹ�ñ��ӿ�������
void EnableTrace(bool fEnable)
{
	g_fTraceEnable = fEnable;
}

bool IsTraceEnable() 
{ 
	return g_fTraceEnable;
}
