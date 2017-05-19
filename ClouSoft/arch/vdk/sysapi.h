 /*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�sysapi.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֶԲ�ͬϵͳ��API�ӿں����ķ�װ
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��9��
 *********************************************************************************************************/
#ifndef SYSAPI_H
#define SYSAPI_H
#include "ComAPI.h"

extern DWORD g_dwClick;
inline DWORD GetClick() { return g_dwClick; }; //ȡ��ʱ�ӵδ�
extern unsigned long TickMs;
inline DWORD GetTick() { return TickMs; };

bool GetSysTime(TTime* pTime);
bool SetSysTime(const TTime& t);
void SyncTimer();
void SysInit();
void SysCleanup();

bool IsAppExit();
bool IsAppExitDone();
void ResetApp();
int IfReadListProc(char *target);
bool InitDir();

#endif //SYSAPI_H
