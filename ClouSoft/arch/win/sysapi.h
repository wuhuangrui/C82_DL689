 /*********************************************************************************************************
 * Copyright (c) 2006,??????????????
 * All rights reserved.
 *
 * ????:sysapi.h
 * ?    ?:?????????????API???????
 * ????:1.0
 * ?    ?:???
 * ????:2006?9?
*********************************************************************************************************/
#ifndef SYSAPI_H
#define SYSAPI_H
#include "ComStruct.h"

DWORD GetTick();
DWORD GetClick();
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

int CloseConsole(void);
int OpenConsole(void);
bool IsConsoleClosed();
#endif //SYSAPI_H
