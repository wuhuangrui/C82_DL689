/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：driver.h
 * 摘    要：本文件主要实现本系统下设备驱动程序的封装
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年9月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/
#ifndef DRIVER_H
#define DRIVER_H

#include "apptypedef.h"
#include "ComStruct.h"
#include "bios.h"
#include "YX.h"

extern WORD g_wHwType;

void PortSwitch(BYTE bChl);
bool RtcGetTime(TTime& time);
bool RtcSetTime(const TTime& time);
void AdjustRTC();
bool RtcSetAlarm(const TTime& time);
bool RtcCancelAlarm();

bool InitDrivers(WORD wHwType);
void EnableCtDetect(bool fEnable);
void SelectCtCn(WORD wCn);
bool GetRtcChipStatus();

inline WORD GetHwType()
{
	return g_wHwType;
}

#endif //DRIVER_H
