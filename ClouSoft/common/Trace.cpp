/*********************************************************************************************************
* Copyright (c) 2006,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：Trace.cpp
* 摘    要：本文件主要包含各系统下调试输出的公共接口
* 当前版本：1.0
* 作    者：岑坚宇
* 完成日期：2007年8月
* 备    注：
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
BYTE g_bDbBuf[DB_BUF_LEN];	//使用公共缓冲区，不用每个线程都消耗那么大的堆栈

//描述:初始化不同系统下调试最低层的部分
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
	//TDataItem di = GetItemEx(1, 0, 0x1000);  //0x1000 1 调试信息输出总开关,1打开
	//g_pbDbAllOn = di.pbAddr;

	TDataItem di = GetItemEx(BN1, PN0, 0x1001);  //0x1001 2 调试输出自动关断时间,单位分钟,0不自动关断
	g_pbDbOffDelay = di.pbAddr;

	di = GetItemEx(BN1, PN0, 0x1002);  //0x1002 16 各项的调试输出开关
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

		if(g_dwLogLen>4*1000*1000)  //文件太大，超过4M了
		{
			close(g_fHandle);
			g_fHandle = open(pszPathName, O_CREAT|O_RDWR|O_BINARY|O_TRUNC, S_IREAD|S_IWRITE);
			g_dwLogLen = 0;
		}

		if (g_fHandle < 0)
			return  false;
		
		sprintf((char*)bBuf, "\r\n####################### CL818C7-LOG Start at %d-%d-%d %d:%d:%d #######################\r\n", tm.nYear,tm.nMonth,tm.nDay, tm.nHour,tm.nMinute,tm.nSecond);
		write(g_fHandle, bBuf, strlen((char*)bBuf));

        g_bDebug2File = 2; //初始化完成，这里写文件没有信号量保护，所以只有当初始化完成后下面才允许写文件
	}

	return true;
}

bool IsDebugOn(BYTE bType)
{
	if (!g_fTraceEnable) //对于本地通讯口和调试输出口重叠的情况,使用本接口来开关
		return false;

	if (g_fDbInited == false)
		return true;

	if (g_pbDbItem == NULL)//有些在g_pbDbItem没有初始化的时候打印会死机
		return false;

	if ((*g_pbDbItem & 0x01) == 0)     //总开关关闭
		return false;

	BYTE flag = g_pbDbItem[bType/8];
	if ((flag & (1<<bType%8)) == 0)    //该项不输出调试信息
		return false;

	/*WORD wDelayMinutes = *g_pbDbOffDelay + (WORD )*(g_pbDbOffDelay + 1) * 0x100;
	if (wDelayMinutes != 0)
	{
		DWORD dwClick = GetClick();
		
	}*/

	//if (g_bDebug2File && 
	//	( (((GetClick()-g_dwDebugClick)/60)>3*24*60) || (g_dwLogLen>4*1000*1000))) //最大输出3天、最大4M调试信息到文件/mnt/data/log中，以先到着为准

	if (g_bDebug2File>0 && 
		(g_dwLogLen>4*1000*1000 || GetClick()>3*24*60*60)) //最大输出3天、最大4M调试信息到文件/mnt/data/log中，以先到者为准
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

	va_start(varg, fmt ); //include <stdarg.h> va_start,va_arg,va_end是在stdarg.h中被定义成宏
	vsprintf((char* )g_bDbBuf, fmt, varg);//送格式化输出到串中,int vsprintf(char *string, char *format, va_list param); 
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
		if (n==0 || wStrLen>1000)   //已经往bBuf[]里塞满了,则先把当前的数据输出
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

//描述:对于本地通讯口和调试输出口重叠的情况,使用本接口来开关
void EnableTrace(bool fEnable)
{
	g_fTraceEnable = fEnable;
}

bool IsTraceEnable() 
{ 
	return g_fTraceEnable;
}
