 /*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：sysapi.h
 * 摘    要：本文件主要实现对不同系统下API接口函数的封装
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年9月
 *********************************************************************************************************/
#ifndef SYSAPI_H
#define SYSAPI_H
#include "ComAPI.h"

extern DWORD g_dwClick;
inline DWORD GetClick() { return g_dwClick; }; //取得时钟滴答
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
