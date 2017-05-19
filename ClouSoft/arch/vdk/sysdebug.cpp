/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：sysdebug.cpp
 * 摘    要：本文件主要包含本系统下调试接口的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2008年6月
 * 备    注：
 *********************************************************************************************************/
#include "stdafx.h"
#include <stdio.h>
#include <ctype.h> 
#include <stdlib.h>
#include <stdarg.h> 
#include "Trace.h"
#include "sysdebug.h"

extern TSem   g_semDebug;
extern BYTE g_bDbBuf[];	//使用公共缓冲区，不用每个线程都消耗那么大的堆栈

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

