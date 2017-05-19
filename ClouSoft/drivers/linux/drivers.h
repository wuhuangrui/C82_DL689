/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�driver.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֱ�ϵͳ���豸��������ķ�װ
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��9��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
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
