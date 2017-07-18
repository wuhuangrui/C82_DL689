/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�drivers.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֱ�ϵͳ���豸��������ķ�װ
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��9��
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

//������ΪԭDSP����MENU.cpp�к�����Ϊ��VC�µ����쳣���񷽱㣬��VC�����ж���ͬ�����
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
