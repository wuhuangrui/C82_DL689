 /*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�sysapi.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ֶԲ�ͬϵͳ��API�ӿں����ķ�װ
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��9��
 *********************************************************************************************************/
#include "stdafx.h"
#include "fs.h"
#include "sysapi.h"
#include "apptypedef.h"
#include "FaCfg.h"
#include "rtcbios.h"
#include "ComAPI.h"
#include "RTCBIOS.H"

bool GetSysTime(TTime* pTime)
{
	tm_t time;
	rtc.readTimer(time);
	pTime->nYear = time.year + 2000;
	pTime->nMonth = time.month;
	pTime->nDay = time.date;
	pTime->nHour = time.hour;
	pTime->nMinute = time.minute;
	pTime->nSecond = time.second;
	pTime->nWeek = time.wday;
	
	if (IsInvalidTime(*pTime))
		return false;
	else
		return true;
}

bool SetSysTime(const TTime& t)
{
	tm_t time;
	time.second = t.nSecond;
	time.minute = t.nMinute;
	time.hour = t.nHour;
	time.date = t.nDay;
	time.month = t.nMonth;
	time.year = t.nYear-2000;
	
	time.wday = t.nWeek;
		 	
	/*DTRACE(DB_DATAMANAGER, ("SetSysTime : adj time to %02d/%02d/%02d %02d:%02d:%02d, week %d.\r\n", 
			  				time.year, time.month, time.date, 
			  				time.hour, time.minute, time.second, time.wday)); */
			  				
	return rtc.setTimer(time);
}

void SyncTimer()
{
}

void SysInit()
{
}

bool g_fAppExit = false;

//����:��ǰӦ�ó����Ƿ������˳�
bool IsAppExit()
{
	return g_fAppExit;
}

//����:��ǰӦ�ó����Ƿ��˳����
bool IsAppExitDone()
{
	return true;
}

//����:��λӦ�ó���
void ResetApp()
{
	ResetCPU();
	g_fAppExit = true;	 
}


static DWORD g_dwRunCnt = 0;


//������ȷ��Ӧ����������Ҫ��Ŀ¼��������
bool InitDir()
{
	DIR* dir = opendir("/root/user");
	if (dir == NULL)
	{
		if (mkdir("/root/user", 0) == -1)
			return false;
	}
	else
	{
		closedir(dir);
	}

	dir = opendir(USER_PATH"para");
	if (dir == NULL)
	{
		if (mkdir(USER_PATH"para", 0) == -1)
			return false;
	}
	else
	{
		closedir(dir);
	}

	dir = opendir(USER_PATH"data");
	if (dir == NULL)
	{
		if (mkdir(USER_PATH"data", 0) == -1)
			return false;
	}
	else 
	{
		closedir(dir);
	}

	dir = opendir(USER_PATH"cfg");
	if (dir == NULL)
	{
		if (mkdir(USER_PATH"cfg", 0) == -1)
			return false;
	}
	else 
	{
		closedir(dir);
	}

	return true;
}
