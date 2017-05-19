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

#define WM_SYS_NOTIFY 0x1001

extern HWND g_hDgbWnd;

//调试输出接口的定义
extern void debug_printf( const char *fmt, ... );
extern bool IsDebugOn(BYTE bType);

#define DTRACE(debug, x) do { if (IsDebugOn(debug)){TRACE x; debug_printf x;}} while(0)	
#define SAVEDTRACE(debug, x) do { if (IsDebugOn(debug)){TRACE x; debug_printf x;}} while(0)	
#define STRACE(debug, s, len) do { if (IsDebugOn(debug)){TRACE((char* )s); DTRACEOUT(s, len);}} while(0)
void DTRACEOUT(char* s, int len);
//#define DTRACEOUT(s, len)   do { TRACE("%s", s); debug_strace(s, len);} while(0)

#endif //SYSDEBUG_H