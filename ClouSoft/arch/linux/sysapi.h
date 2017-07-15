 /*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�sysapi.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֶԲ�ͬϵͳ��API�ӿں����ķ�װ
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��9��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
*********************************************************************************************************/
#ifndef SYSAPI_H
#define SYSAPI_H
#include "ComStruct.h"

extern bool g_fAppExit;

bool GetSysTime(TTime* pTime);
bool SetSysTime(const TTime& t);
void SyncTimer();
DWORD GetClick();
DWORD GetTick();
void SysInit();
void SysCleanup();

bool IsAppExit();
bool IsAppExitDone();

void ResetApp();
void ResetCPU(void);
int IfReadListProc(char *target);//����ֵ1��ʾ���ųɹ�;0��ʾ���Ź�����;-1��ʾ����ʧ��
DWORD GetLocalAddr(char* interface);
bool InitDir();
unsigned int freespace(char* disk);

int CloseConsole(void);
int OpenConsole(void);
bool IsConsoleClosed();
void CheckConsole();


#endif //SYSAPI_H
