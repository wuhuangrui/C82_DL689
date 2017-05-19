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
#include <syslog.h>

//��������ӿڵĶ���
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
