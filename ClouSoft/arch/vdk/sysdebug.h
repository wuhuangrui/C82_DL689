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
#include "apptypedef.h"
#include "FaCfg.h"
#include "Comm.h"
#include "ComStruct.h"
#include "ComAPI.h"

//��������Ľӿڶ���
extern CComm& g_commDebug;
extern bool IsDebugOn(BYTE bType);
extern void debug_printf( const char *fmt, ... );

#define DTRACE(debug, x)  do { if (IsDebugOn(debug)) {	debug_printf x;	}} while(0)

#define STRACE(debug, s, len)   do {if (IsDebugOn(debug)){g_commDebug.Write(s, len);}} while(0)
#define DTRACEOUT(s, len)   do { g_commDebug.Write((BYTE* )s, len); } while(0)
//	#define STRACE(debug, s, len)   do {if (IsDebugOn(debug)){g_commDebug.Write(s, len);}} while(0)

#endif //SYSDEBUG_H