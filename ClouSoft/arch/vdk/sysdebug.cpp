/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�sysdebug.cpp
 * ժ    Ҫ�����ļ���Ҫ������ϵͳ�µ��ԽӿڵĶ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2008��6��
 * ��    ע��
 *********************************************************************************************************/
#include "stdafx.h"
#include <stdio.h>
#include <ctype.h> 
#include <stdlib.h>
#include <stdarg.h> 
#include "Trace.h"
#include "sysdebug.h"

extern TSem   g_semDebug;
extern BYTE g_bDbBuf[];	//ʹ�ù���������������ÿ���̶߳�������ô��Ķ�ջ

void ppp_trace(int level, const char *fmt,...)
{
	if (IsDebugOn(level))
	{
      	va_list varg;
      	WaitSemaphore(g_semDebug);
      	
      	va_start(varg, fmt );
      	vsprintf((char* )g_bDbBuf, fmt, varg);
      	va_end(varg);
      	DTRACEOUT(g_bDbBuf, strlen((char* )g_bDbBuf));
      	
      	SignalSemaphore(g_semDebug);
	}
}

