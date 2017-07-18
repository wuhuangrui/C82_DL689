/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：drivers.h
 * 摘    要：本文件主要实现本系统下设备驱动程序的封装
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年9月
*********************************************************************************************************/
#ifndef DRIVER_H
#define DRIVER_H
#include "ComAPI.h"
#include "FM24CL64.h"
#include "sysstruct.h"
#include "bios.h"

extern WORD g_wHwType;

bool RtcGetTime(TTime& time);
bool RtcSetTime(const TTime& time);

bool InitDrivers(WORD wHwType);
void EnableCtDetect(bool fEnable);
void SelectCtCn(WORD wCn);

//本函数为原DSP工程MENU.cpp中函数，为是VC下调试异常任务方便，在VC工程中定义同名替代
inline void SetAlertNum(unsigned short num)
{
	num;
}

inline WORD GetPhaseStatus(void)
{
	return 0;
}

inline WORD GetHwType()
{
	return g_wHwType;
}

#endif //DRIVER_H
