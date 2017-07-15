 /*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：sysapi.h
 * 摘    要：本文件主要实现对不同系统下API接口函数的封装
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年9月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
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
int IfReadListProc(char *target);//返回值1表示拨号成功;0表示拨号过程中;-1表示拨号失败
DWORD GetLocalAddr(char* interface);
bool InitDir();
unsigned int freespace(char* disk);

int CloseConsole(void);
int OpenConsole(void);
bool IsConsoleClosed();
void CheckConsole();


#endif //SYSAPI_H
