/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：sysdebug.h
 * 摘    要：本文件主要包含本系统下调试接口的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：
 *********************************************************************************************************/
#ifndef SYSDEBUG_H
#define SYSDEBUG_H
#include "apptypedef.h"
#include "FaCfg.h"
#include "Comm.h"
#include "ComStruct.h"
#include "ComAPI.h"

//调试输出的接口定义
extern CComm& g_commDebug;
extern bool IsDebugOn(BYTE bType);
extern void debug_printf( const char *fmt, ... );

#define DTRACE(debug, x)  do { if (IsDebugOn(debug)) {	debug_printf x;	}} while(0)

#define STRACE(debug, s, len)   do {if (IsDebugOn(debug)){g_commDebug.Write(s, len);}} while(0)
#define DTRACEOUT(s, len)   do { g_commDebug.Write((BYTE* )s, len); } while(0)
//	#define STRACE(debug, s, len)   do {if (IsDebugOn(debug)){g_commDebug.Write(s, len);}} while(0)

#endif //SYSDEBUG_H