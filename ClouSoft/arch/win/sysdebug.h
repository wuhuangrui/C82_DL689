/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�sysdebug.h
 * ժ    Ҫ�����ļ���Ҫ������ϵͳ�µ��ԽӿڵĶ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע��
 *********************************************************************************************************/
#ifndef SYSDEBUG_H
#define SYSDEBUG_H

#define WM_SYS_NOTIFY 0x1001

extern HWND g_hDgbWnd;

//��������ӿڵĶ���
extern void debug_printf( const char *fmt, ... );
extern bool IsDebugOn(BYTE bType);

#define DTRACE(debug, x) do { if (IsDebugOn(debug)){TRACE x; debug_printf x;}} while(0)	
#define SAVEDTRACE(debug, x) do { if (IsDebugOn(debug)){TRACE x; debug_printf x;}} while(0)	
#define STRACE(debug, s, len) do { if (IsDebugOn(debug)){TRACE((char* )s); DTRACEOUT(s, len);}} while(0)
void DTRACEOUT(char* s, int len);
//#define DTRACEOUT(s, len)   do { TRACE("%s", s); debug_strace(s, len);} while(0)

#endif //SYSDEBUG_H