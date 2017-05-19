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
#include <syslog.h>

//调试输出接口的定义
extern void debug_printf( const char *fmt, ... );
extern bool IsDebugOn(BYTE bType);
/*
#define DTRACE(debug, x)   do { if (IsDebugOn(debug)){printf x;}} while(0)  
#define STRACE(debug, s, len)   do { if (IsDebugOn(debug)){printf((char* )s);}} while(0)
*/
#define DTRACE(debug, x)  do { if (IsDebugOn(debug)) { debug_printf x;/*printf x;*/}} while(0)	
#define STRACE(debug, s, len)   do { if(IsDebugOn(debug)) {syslog(LOG_INFO, s);/*printf((char* )s);*/}} while(0)
#define DTRACEOUT(s, len)   do { syslog(LOG_INFO, s); } while(0)

#endif //SYSDEBUG_H
