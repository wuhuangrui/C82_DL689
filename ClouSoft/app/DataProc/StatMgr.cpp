/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：StatMgr.cpp
 * 摘    要：本文件主要实现终端统计信息及数据统计类的管理
 * 当前版本：1.0
 * 作    者：杨凡、李湘平
 * 完成日期：2008年7月
 *********************************************************************************************************/
#include "stdafx.h"
#include "StatMgr.h"
#include "ComAPI.h"
#include "FaAPI.h"
#include <string.h>
#include <stdio.h>
#include "sysfs.h"
#include "Info.h"
#include "MeterAPI.h"
#ifdef EN_CCT
#include "CctHook.h"
#include "CctStat.h"
#endif
#include "DbOIAPI.h"

#define TERM_STAT_PATHNAME	USER_DATA_PATH"TermStat.dat"
extern BYTE g_bRangeStatFmt[8];



///////////////////////////////////////////////////////////////////////////
//CStatMgr

CStatMgr::CStatMgr()
{
	//memset(&m_pDataStat, 0, sizeof(m_pDataStat));
}

CStatMgr::~CStatMgr()
{
	/*int		i	=	0;

	for (i=1; i<PN_NUM; i++)
	{
		//非简单的上报告警类型
		if ( m_pDataStat[i] != NULL)   
		{
			delete m_pDataStat[i] ;
			m_pDataStat[i]	=	NULL;
		}
	}*/
}

//描述:任务初始化:填充普通任务的控制结构,初始化普通任务的任务表,
//	   在测试点参数修改后，需要重新初始化
bool CStatMgr::Init()
{
	bool fRet = true;
	
	m_semTermLog = NewSemaphore(1);
	m_semStat = NewSemaphore(1);
	
	memset(&m_tmLastRun, 0, sizeof(m_tmLastRun));

	fRet &= InitTermStat();
	fRet &= SaveTermStat();

	LoadStatRela();
	LoadStatRes();
	memset(m_iPower, 0, sizeof(m_iPower));
	m_bPowrIndex = 0;
	m_bPowrIndexCnt = 0;
	return fRet;
}

//描述:读取termInfo.dat 文件数据 更新结构m_termInfo
bool CStatMgr::InitTermStat()
{
#ifdef SYS_WIN
	if (!ReadFile(TERM_STAT_PATHNAME, (BYTE* )&m_TermStatInfo, sizeof(m_TermStatInfo)))
	{
		memset(&m_TermStatInfo, 0, sizeof(m_TermStatInfo));
	}
#else
#ifdef LOG_ENABLE
	m_DataLog.Init(LOG_TERMSTAT, sizeof(m_TermStatInfo));
	if (!m_DataLog.Recover((BYTE* )&m_TermStatInfo))  //日志记录有效
#else
	if (!ReadFile(TERM_STAT_PATHNAME, (BYTE* )&m_TermStatInfo, sizeof(m_TermStatInfo)))
#endif
	{
		DTRACE(DB_DP,("InitTermStat: Recover Fail!\r\n"));
		memset(&m_TermStatInfo, 0, sizeof(m_TermStatInfo));
	}	
	
	if ((!IsTimeEmpty(m_TermStatInfo.tmLastRun)) && (!g_PowerOffTmp.fAlrPowerOff))
	{	
		g_PowerOffTmp.tPoweroff = m_TermStatInfo.tmLastRun;
	}
	else if (!g_PowerOffTmp.fAlrPowerOff)
	{
		GetCurTime(&g_PowerOffTmp.tPoweroff);
	}
#endif //SYS_WIN
	
	m_fTermStatChg = false;
	m_dwStatClick = GetClick();

	BYTE bBuf[12];
	TTime tmNow;
	GetCurTime(&tmNow);
	//memset(&m_TermStatInfo, 0, sizeof(m_TermStatInfo));
	memset(bBuf, 0, sizeof(bBuf));
	//ReadItemEx(BN0, PN0, 0x2204, bBuf);	//0x2204  复位次数,
	bBuf[0] =  0x02;
	bBuf[1] =  0x02;
	bBuf[2] =  0x12;
	bBuf[5] =  0x12;
	m_TermStatInfo.wDayRstStart += 1;
	m_TermStatInfo.wMonRstStart += 1;
	OoWordToLongUnsigned(m_TermStatInfo.wDayRstStart, (&bBuf[3]));
	OoWordToLongUnsigned(m_TermStatInfo.wMonRstStart, (&bBuf[6]));
	WriteItemEx(BN0, PN0, 0x2204, bBuf); 

	memset(bBuf, 0, sizeof(bBuf));
	//ReadItemEx(BN0, PN0, 0x2203, bBuf);	//0x2203  供电时间
	bBuf[0] =  0x02;
	bBuf[1] =  0x02;
	bBuf[2] =  0x06;
	bBuf[7] =  0x06;

	DWORD dwSec1 = TimeToSeconds(m_TermStatInfo.tmLastRun);
	DWORD dwSec2 = TimeToSeconds(tmNow);
	if ((dwSec2>=dwSec1) && (dwSec2<=dwSec1+150))//认为是命令重启终端了
	{
		m_TermStatInfo.wDayPowerTime += dwSec2-dwSec1;//重启前m_TermStatInfo.tmLastRun加了100秒处理,否则停电事件过不了
		m_TermStatInfo.wMonPowerTime += dwSec2-dwSec1;
	}
	OoDWordToDoubleLongUnsigned(m_TermStatInfo.wDayPowerTime/60, (&bBuf[3]));
	OoDWordToDoubleLongUnsigned(m_TermStatInfo.wMonPowerTime/60, (&bBuf[8]));
	WriteItemEx(BN0, PN0, 0x2203, bBuf); 

	memset(bBuf, 0, sizeof(bBuf));
	//ReadItemEx(BN0, PN0, 0x2200, bBuf);	//GPRS流量
	bBuf[0] =  0x02;
	bBuf[1] =  0x02;
	bBuf[2] =  0x06;
	bBuf[7] =  0x06;
	OoDWordToDoubleLongUnsigned(m_TermStatInfo.dwDayFlux, (&bBuf[3]));
	OoDWordToDoubleLongUnsigned(m_TermStatInfo.dwMonFlux, (&bBuf[8]));
	WriteItemEx(BN0, PN0, 0x2200, bBuf); 

	m_TermStatInfo.tmLastRun = tmNow;	//更新最后一次的运行时间
	TrigerSaveBank(BN0, SECT_VARIABLE, -1);//触发保存一次

#ifdef SYS_WIN
	WriteFile(TERM_STAT_PATHNAME, (BYTE* )&m_TermStatInfo, sizeof(m_TermStatInfo));
#else
#ifdef LOG_ENABLE
	m_DataLog.WriteLog((BYTE* )&m_TermStatInfo); 
#else
	WriteFile(TERM_STAT_PATHNAME, (BYTE* )&m_TermStatInfo, sizeof(TTermStatInfo));
	system("sync");
#endif
#endif
	
	return true;
}


//描述:每分钟保存终端统计的临时数据
//备注:由于在实际硬件中需要使用铁电保存终端统计数据,本函数每秒钟在驱动线程被调用,
//	   跟DoTermStat()不在同一个线程被调用,所以要用信号m_semTermLog进行保护
bool CStatMgr::SaveTermStat()
{
	TTermStatInfo TermStatInfo;
	bool fDataInitF = false;
	if (GetInfo(INFO_RST_TERM_STAT))
	{
		WaitSemaphore(m_semTermLog);
		
		memset(&m_TermStatInfo, 0, sizeof(m_TermStatInfo));
		GetCurTime(&m_TermStatInfo.tmLastRun);
		m_fTermStatChg = true;
		
		SignalSemaphore(m_semTermLog);
		memset((BYTE *)&m_PhaseVoltStat[0], 0, sizeof(m_PhaseVoltStat));
		memset((BYTE*)&m_IntvStatRes[0][0], 0, sizeof(m_IntvStatRes));
		memset((BYTE*)&m_AvgStatRes[0][0], 0, sizeof(m_AvgStatRes));
		memset((BYTE*)&m_ExtremStatRes[0][0], 0, sizeof(m_ExtremStatRes));
		fDataInitF = true;
		Sleep(50);
	}
	
	if (GetInfo(INFO_HARDWARE_INIT) || (GetInfo(INFO_PWROFF)))
	{
		WaitSemaphore(m_semTermLog);
		GetCurTime(&m_TermStatInfo.tmLastRun);
		//DWORD dwSec = TimeToSeconds(m_TermStatInfo.tmLastRun) + 100;
		//SecondsToTime(dwSec, &m_TermStatInfo.tmLastRun);
		DTRACE(DB_DP, ("HARDWARE_INIT  or INFO_PWROFF.\r\n"));
		if (fDataInitF == true)//为数据初始化不重启终端作的处理
		{
			BYTE bBuf[12];
			memset(bBuf, 0, sizeof(bBuf));
			//ReadItemEx(BN0, PN0, 0x2204, bBuf);	//0x2204  复位次数,
			bBuf[0] =  0x02;
			bBuf[1] =  0x02;
			bBuf[2] =  0x12;
			bBuf[5] =  0x12;
			m_TermStatInfo.wDayRstStart = 1;
			m_TermStatInfo.wMonRstStart = 1;
			OoWordToLongUnsigned(m_TermStatInfo.wDayRstStart, (&bBuf[3]));
			OoWordToLongUnsigned(m_TermStatInfo.wMonRstStart, (&bBuf[6]));
			WriteItemEx(BN0, PN0, 0x2204, bBuf); 
		}
		m_fTermStatChg = true;
		SignalSemaphore(m_semTermLog);
	}

	if (!m_fTermStatChg)	//数据结构在DoTermStat()中没被更新,没必要保存
		return true;
	
	WaitSemaphore(m_semTermLog);
	
	TermStatInfo = m_TermStatInfo;
	m_fTermStatChg = false;
	SignalSemaphore(m_semTermLog); 
	
#ifdef SYS_WIN
	WriteFile(TERM_STAT_PATHNAME, (BYTE* )&TermStatInfo, sizeof(TermStatInfo));
#else
#ifdef LOG_ENABLE
	m_DataLog.WriteLog((BYTE* )&TermStatInfo); 
#else
	WriteFile(TERM_STAT_PATHNAME, (BYTE* )&TermStatInfo, sizeof(TTermStatInfo));
	system("sync");
	DTRACE(DB_DP, ("SaveTermStat ..... .\r\n"));
	//TraceBuf(DB_CRITICAL, "\r\n####SaveTermStat-> ", (BYTE* )&TermStatInfo, sizeof(TTermStatInfo)); 	
#endif
#endif //SYS_WIN

	return true;
}


//描述:终端统计数据
void CStatMgr::DoTermStat()
{
	TTime tmNow;
	BYTE bBuf[20];
	WORD wDayRstNum;
	WORD wMonRstNum;
	//WORD wRstNum = 0;
	DWORD dwSec;
	//DWORD dwCurSec = GetCurTime();
	DWORD dwCurClick;
	GetCurTime(&tmNow);

	static DWORD dwLastMin = 0;
	DWORD dwCurMin = GetCurMinute();
	
	WaitSemaphore(m_semTermLog);	//统计的整个过程都要进行保护,
								 	//因为数据区复位的时候会直接把整个m_TermStatInfo清除
		//if (IsTimeEmpty(m_TermStatInfo.tmLastRun))	//第一次运行或者统计数据结构被清除过
	//{
	//这部分放到初始化时执行
	//}
		
	if (DaysFrom2000(m_TermStatInfo.tmLastRun) != DaysFrom2000(tmNow))//日切换
	{	
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, PN0, 0x2204, bBuf);	//0x2204  复位次数,
		memset(&bBuf[3], 0, 2);
		WriteItemEx(BN0, PN0, 0x2204, bBuf); 
		m_TermStatInfo.wDayRstStart = 0;

		DTRACE(DB_DP, ("CStatMgr:Day Change:Term supply time reset times:\r\n"));
		
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, PN0, 0x2203, bBuf);	//0x2203  供电时间
		memset(&bBuf[3], 0, 4);
		WriteItemEx(BN0, PN0, 0x2203, bBuf); 
		m_TermStatInfo.wDayPowerTime = 0;
		


#ifdef PRO_698
		m_TermStatInfo.dwDayFlux = 0;//GPRS日流量
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, PN0, 0x2200, bBuf);
		memset(&bBuf[3], 0, 4);
		WriteItemEx(BN0, PN0, 0x2200, bBuf);
#endif

	}

	if (MonthFrom2000(m_TermStatInfo.tmLastRun) != MonthFrom2000(tmNow))//月切换
	{	
		ReadItemEx(BN0, PN0, 0x2204, bBuf);	//0x2204 2 复位次数,HEX 直接从BN2 PN0 0x1021里面读取
		memset(&bBuf[3], 0, 2);
		memset(&bBuf[6], 0, 2);
		WriteItemEx(BN0, PN0, 0x2204, bBuf); 
		m_TermStatInfo.wDayRstStart = 0;
		m_TermStatInfo.wMonRstStart = 0;
		
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, PN0, 0x2203, bBuf);	//0x2203  供电时间
		memset(&bBuf[3], 0, 4);
		memset(&bBuf[8], 0, 4);
		WriteItemEx(BN0, PN0, 0x2203, bBuf); 
		DTRACE(DB_DP, ("CStatMgr:Month Change:Term supply time %d .\r\n", m_TermStatInfo.wDayPowerTime/60));
		m_TermStatInfo.wDayPowerTime = 0;
		m_TermStatInfo.wMonPowerTime = 0;

#ifdef PRO_698
		m_TermStatInfo.dwMonFlux = 0;//GPRS月流量
		m_TermStatInfo.dwDayFlux = 0;
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, PN0, 0x2200, bBuf);
		memset(&bBuf[3], 0, 4);
		memset(&bBuf[8], 0, 4);
		WriteItemEx(BN0, PN0, 0x2200, bBuf);
#endif
	}
	//if (dwCurSec < TimeToSeconds(m_TermStatInfo.tmLastRun))//倒退校时了
	dwCurClick = GetClick();
	if (dwCurClick >=  m_dwStatClick)
		dwSec = dwCurClick - m_dwStatClick;
	else
		dwSec = 0;
	
	//dwCurSec - TimeToSeconds(m_TermStatInfo.tmLastRun);	//台子校过的时间不能计算在内
	m_dwStatClick = dwCurClick;
	m_TermStatInfo.tmLastRun = tmNow;	//更新最后一次的运行时间
	ReadItemEx(BN0, PN0, 0x2203, bBuf);	//0x2203  供电时间
	m_TermStatInfo.wDayPowerTime += dwSec;//更新日供电时间
	if (m_TermStatInfo.wDayPowerTime > 86400)// 1440
		m_TermStatInfo.wDayPowerTime = 86400;// 1440;
	
	m_TermStatInfo.wMonPowerTime += dwSec;//更新月供电时间
	DWORD dwData = m_TermStatInfo.wDayPowerTime;
	if ((dwData % 60) > 30 /*47*/)//第二步时把终端的复位时间都包括在内的2分钟也要按2分钟来算
		dwData = dwData/60 + 1;//过台子
	else
		dwData = dwData/60;
	bBuf[0] = DT_STRUCT;
	bBuf[1] = 2;
	bBuf[2] = DT_DB_LONG_U;
	OoDWordToDoubleLongUnsigned(dwData, &bBuf[3]);
	dwData = m_TermStatInfo.wMonPowerTime;
	if ((dwData % 60) > 47)
		dwData = dwData/60 + 1;
	else
		dwData = dwData/60;
	bBuf[7] = DT_DB_LONG_U;
	OoDWordToDoubleLongUnsigned(dwData, &bBuf[8]);
	int irett=WriteItemEx(BN0, PN0, 0x2203, bBuf); 

	//if (dwLastMin != dwCurMin)
	//if ((dwLastMin>dwCurMin) || (dwCurMin>=(dwLastMin+15)))
	{
		m_fTermStatChg = true;
		//dwLastMin = dwCurMin;
	}

	SignalSemaphore(m_semTermLog); 

	return;
}

void CStatMgr::AddFlux(DWORD dwLen)
{
#ifdef PRO_698
	BYTE bBuf[12];
	//DTRACE(DB_DP, ("old_Flux day=%d  mon=%d,   new add_Flus=%d .\r\n", m_TermStatInfo.dwDayFlux,m_TermStatInfo.dwMonFlux, dwLen));
	WaitSemaphore(m_semTermLog);			//统计的整个过程都要进行保护,
	m_TermStatInfo.dwDayFlux += dwLen;		//终端GPRS日流量
	m_TermStatInfo.dwMonFlux += dwLen;		//终端GPRS月流量

	memset(bBuf, 0, sizeof(bBuf));
	//ReadItemEx(BN0, PN0, 0x2200, bBuf);
	OoReadAttr(0x2200, 0x02, bBuf, NULL, NULL);
	OoDWordToDoubleLongUnsigned(m_TermStatInfo.dwDayFlux, &bBuf[3]);
	OoDWordToDoubleLongUnsigned(m_TermStatInfo.dwMonFlux, &bBuf[8]);
	//WriteItemEx(BN0, PN0, 0x2200, bBuf); //698 C1F10
	OoWriteAttr(0x2200, 0x02, bBuf);
	//m_fTermStatChg = true; //这里不能触发，否则通讯的时候会频繁写文件

	SignalSemaphore(m_semTermLog); 
#endif
}

//运行普通任务:对有效的测量点才运行
bool CStatMgr::DoDataStat()
{
	int	i;
	TTime now;
	GetCurTime(&now);
	
	//测试点参数改变
	if (GetInfo(INFO_STAT_PARA))//记得重新装载一下电压统计的那些参数
	{
	//某一参数更改了，就要初始化掉
	//相应变量，并清数据库里的数据。
		LoadVoltStatPara(&m_VoltPara);
		memset((BYTE *)&m_PhaseVoltStat[0], 0, sizeof(m_PhaseVoltStat));
		for (i=0; i<4; i++)
		{
			SavePhaseVoltStat(0x2130+i, RW_ATTR_RES, &m_PhaseVoltStat[i]);
		}

	//清相应统计变量数据
		TrigerSaveBank(BN11, 0, -1); //即时统计的起点数据
		TrigerSaveBank(BN0, 0, -1); 
	}

	if (GetInfo(INFO_CLASS14_STAT_CHG))//区间参数更新
	{
		TIntvStatRela tmpRela[STAT_OAD_NUM];
		BYTE bBuf[16];
		for (i=0; i<5; i++)
		{
			memcpy(tmpRela, m_IntvStatRela[i], sizeof(TIntvStatRela)*STAT_OAD_NUM);
			memset(m_IntvStatRela[i], 0, sizeof(TIntvStatRela)*STAT_OAD_NUM);
			LoadIntvStatRela(0x2100+i, m_IntvStatRela[i], &m_bIntvStatNum[i]);
			if (memcmp(tmpRela, m_IntvStatRela[i], sizeof(TIntvStatRela)*STAT_OAD_NUM) != 0)
			{
				memset(m_IntvStatRes[i], 0, sizeof(TIntvStatRes)*STAT_OAD_NUM);
				SaveIntvStatRes(0x2108+i, RW_ATTR_RES, m_IntvStatRes[i], m_bIntvStatNum[i]);
				for (BYTE j=0; j<STAT_OAD_NUM; j++)
				{
					memset(bBuf, 0, sizeof(bBuf));
					WriteItemEx(BN11, j, 0x0014+i, bBuf);
				}
			}
		}
		TrigerSaveBank(BN11, 0, -1); //即时统计的起点数据
		TrigerSaveBank(BN0, 0, -1); 
	}

	if (GetInfo(INFO_CLASS15_STAT_CHG))//平均参数更新
	{
		TStatRela tmpRela[STAT_OAD_NUM];
		BYTE bBuf[16];
		for (i=0; i<5; i++)
		{
			memcpy(tmpRela, m_AvgStatRela[i], sizeof(TStatRela)*STAT_OAD_NUM);
			memset(m_AvgStatRela[i], 0, sizeof(TStatRela)*STAT_OAD_NUM);
			LoadAvgStatRela(0x2110+i, m_AvgStatRela[i], &m_bAvgStatNum[i]);
			if (memcmp(tmpRela, m_AvgStatRela[i], sizeof(TStatRela)*STAT_OAD_NUM) != 0)
			{
				memset(m_AvgStatRes[i], 0, sizeof(TAccAvgStatRes)*STAT_OAD_NUM);
				SaveAvgStatRes(0x2118+i, RW_ATTR_RES, m_AvgStatRes[i], m_bAvgStatNum[i]);
				for (BYTE j=0; j<STAT_OAD_NUM; j++)
				{
					memset(bBuf, 0, sizeof(bBuf));
					WriteItemEx(BN11, j, 0x0019+i, bBuf);
				}
			}
		}
		TrigerSaveBank(BN11, 0, -1); //即时统计的起点数据
		TrigerSaveBank(BN0, 0, -1); 
	}

	if (GetInfo(INFO_CLASS16_STAT_CHG))//极值参数更新
	{
		TStatRela tmpRela[STAT_OAD_NUM];
		BYTE bBuf[16];
		for (i=0; i<5; i++)
		{
			memcpy(tmpRela, m_ExtremStatRela[i], sizeof(TStatRela)*STAT_OAD_NUM);
			memset(m_ExtremStatRela[i], 0, sizeof(TStatRela)*STAT_OAD_NUM);
			LoadAvgStatRela(0x2120+i, m_ExtremStatRela[i], &m_bExtremStatNum[i]);
			if (memcmp(tmpRela, m_ExtremStatRela[i], sizeof(TStatRela)*STAT_OAD_NUM) != 0)
			{
				memset(m_ExtremStatRes[i], 0, sizeof(TExtremStatRes)*STAT_OAD_NUM);
				SaveExtremStatRes(0x2128+i, RW_ATTR_RES, m_ExtremStatRes[i], m_bExtremStatNum[i]);
				for (BYTE j=0; j<STAT_OAD_NUM; j++)
				{
					memset(bBuf, 0, sizeof(bBuf));
					WriteItemEx(BN11, j, 0x001e + i, bBuf);
				}
			}
		}
		TrigerSaveBank(BN11, 0, -1); //即时统计的起点数据
		TrigerSaveBank(BN0, 0, -1); 
	}

	CalcuAvgPower();
	if (now.nSecond != m_tmLastRun.nSecond)//每秒刷铁电是否有风险，看能否考虑30秒刷一次
	{
		m_tmLastRun = now;
		DoTermStat();//终端供电时间、复位、流量统计
		WaitSemaphore(m_semStat);
		//区间统计
		IntvStat();
		//累加平均统计
		AccAvgStat();
		//极值统计
		ExtremStat();
		//电压合格率统计
		DoVoltStat();
		//最大最小功率统计
		DoPowerStat();
		SignalSemaphore(m_semStat);
//#ifdef SYS_WIN	
//		SaveTermStat();
//#endif
	}

	return true;
}



//描述：把相电压统计数据（包括协议和中间数据）保存到系统库
//参数：@wOI分相电压统计OI
//		@bAttr 要写入的属性(0表示存当前ID，1-表示存冻结ID)
//		@pPhaseVoltStat分相电压统计数据
void CStatMgr::SavePhaseVoltStat(WORD wOI, BYTE bAttr, TPhaseVoltStat* pPhaseVoltStat)
{
	BYTE bCalData[50] ;
	BYTE * P = bCalData;
	DWORD dwMin = 0;

	memset(bCalData, 0, sizeof(bCalData));
	*P++ = DT_STRUCT;
	*P++ = 0x02;
	//日合格率
	*P++ = DT_STRUCT;
	*P++ = 0x05;
	*P++ = DT_DB_LONG_U;
	dwMin = pPhaseVoltStat->dayStat.dwMoniSecs/60;
	if (pPhaseVoltStat->dayStat.dwMoniSecs%60 >= 55)//做4秒的防误差处理,与上层函数的保持一致
		dwMin += 1;
	OoDWordToDoubleLongUnsigned(dwMin, P);
	P += 4;
	*P++ = DT_LONG_U;
	OoWordToLongUnsigned(pPhaseVoltStat->dayStat.wQualRate, P);
	P += 2;
	*P++ = DT_LONG_U;
	OoWordToLongUnsigned(pPhaseVoltStat->dayStat.wOverRate, P);
	P += 2;
	*P++ = DT_DB_LONG_U;
	dwMin = pPhaseVoltStat->dayStat.dwUpperSecs/60;
	if (pPhaseVoltStat->dayStat.dwUpperSecs%60 >= 55)
		dwMin += 1;
	OoDWordToDoubleLongUnsigned(dwMin, P);
	P += 4;
	*P++ = DT_DB_LONG_U;
	dwMin = pPhaseVoltStat->dayStat.dwLowerSecs/60;
	if (pPhaseVoltStat->dayStat.dwLowerSecs%60 >= 55)
		dwMin += 1;
	OoDWordToDoubleLongUnsigned(dwMin, P);
	P += 4;
	//月合格率
	*P++ = DT_STRUCT;
	*P++ = 0x05;
	*P++ = DT_DB_LONG_U;
	dwMin = pPhaseVoltStat->monStat.dwMoniSecs/60;
	if (pPhaseVoltStat->monStat.dwMoniSecs%60 >= 49)
		dwMin += 1;
	OoDWordToDoubleLongUnsigned(dwMin, P);
	P += 4;
	*P++ = DT_LONG_U;
	OoWordToLongUnsigned(pPhaseVoltStat->monStat.wQualRate, P);
	P += 2;
	*P++ = DT_LONG_U;
	OoWordToLongUnsigned(pPhaseVoltStat->monStat.wOverRate, P);
	P += 2;
	*P++ = DT_DB_LONG_U;
	dwMin = pPhaseVoltStat->monStat.dwUpperSecs/60;
	if (pPhaseVoltStat->monStat.dwUpperSecs%60 >= 49)
		dwMin += 1;
	OoDWordToDoubleLongUnsigned(dwMin, P);
	P += 4;
	*P++ = DT_DB_LONG_U;
	dwMin = pPhaseVoltStat->monStat.dwLowerSecs/60;
	if (pPhaseVoltStat->monStat.dwLowerSecs%60 >= 49)
		dwMin += 1;
	OoDWordToDoubleLongUnsigned(dwMin, P);
	P += 4;

	if (bAttr != RW_ATTR_FRZ)//存当前
	{
		OoWriteAttr(wOI, bAttr, bCalData);//保存合格率数据
		memset(bCalData, 0, sizeof(bCalData));
		OoDWordToDoubleLongUnsigned(pPhaseVoltStat->dayStat.dwQualSecs, bCalData);
		OoDWordToDoubleLongUnsigned(pPhaseVoltStat->monStat.dwQualSecs, bCalData+4);
		OoDWordToDoubleLongUnsigned(pPhaseVoltStat->dayStat.dwOverSecs, bCalData+8);
		OoDWordToDoubleLongUnsigned(pPhaseVoltStat->monStat.dwOverSecs, bCalData+12);
		WriteItemEx(BN11, PN0, 0x0010+(wOI-0x2130), bCalData);//保存合格时间,日在前月在后，不带格式字节符
	}
	else//存冻结ID
	{
		WriteItemEx(BN11, PN0, 0x0032+(wOI-0x2130), bCalData);
	}
}

//描述：从系统库中装入分相电压统计数据
//参数：@wOI分相电压统计OI
//		@pPhaseVoltStat分相电压统计数据
void CStatMgr::LoadPhaseVoltStat(WORD wOI, TPhaseVoltStat* pPhaseVoltStat)
{
	BYTE bCalData[50];
	memset(bCalData, 0, sizeof(bCalData));
	OoReadAttr(wOI, 0x02, bCalData, NULL, NULL);
	BYTE * P = bCalData;
	//当日电压合格率
	P += 5;
	pPhaseVoltStat->dayStat.dwMoniSecs = OoDoubleLongUnsignedToDWord(P)*60;
	P += 5;
	pPhaseVoltStat->dayStat.wQualRate = OoLongUnsignedToWord(P);
	P += 3;
	pPhaseVoltStat->dayStat.wOverRate = OoLongUnsignedToWord(P);
	P += 3;
	pPhaseVoltStat->dayStat.dwUpperSecs = OoDoubleLongUnsignedToDWord(P)*60;
	P += 5;
	pPhaseVoltStat->dayStat.dwLowerSecs = OoDoubleLongUnsignedToDWord(P)*60;
	P += 7;
	//当月电压合格率
	pPhaseVoltStat->monStat.dwMoniSecs = OoDoubleLongUnsignedToDWord(P)*60;
	P += 5;
	pPhaseVoltStat->monStat.wQualRate = OoLongUnsignedToWord(P);
	P += 3;
	pPhaseVoltStat->monStat.wOverRate = OoLongUnsignedToWord(P);
	P += 3;
	pPhaseVoltStat->monStat.dwUpperSecs = OoDoubleLongUnsignedToDWord(P)*60;
	P += 5;
	pPhaseVoltStat->monStat.dwLowerSecs = OoDoubleLongUnsignedToDWord(P)*60;
	//合格时间
	ReadItemEx(BN11, PN0, 0x0010+(wOI-0x2130), bCalData);
	pPhaseVoltStat->dayStat.dwQualSecs = OoDoubleLongUnsignedToDWord(bCalData);
	pPhaseVoltStat->monStat.dwQualSecs = OoDoubleLongUnsignedToDWord(bCalData+4);
	pPhaseVoltStat->dayStat.dwOverSecs = OoDoubleLongUnsignedToDWord(bCalData+8);
	pPhaseVoltStat->monStat.dwOverSecs = OoDoubleLongUnsignedToDWord(bCalData+12);

}

//描述：从系统库中装入电压统计参数
//参数：@pPara电压合格率参数
void CStatMgr::LoadVoltStatPara (TVoltStatPara* pPara)
{
	BYTE bPara[16] = {0};
	OoReadAttr(0x4030,0x02,  bPara, NULL, NULL);
	pPara->wAssesUpLimit = OoLongUnsignedToWord(bPara+3);
	pPara->wAssesLowLimit = OoLongUnsignedToWord(bPara+6);
	pPara->wQualUpLimit = OoLongUnsignedToWord(bPara+9);
	pPara->wQualLowLimit = OoLongUnsignedToWord(bPara+12);
}


//描述:日/月电压合格率统计数据 C2F27(0x012f)   C2F35(0x019f)
//参数:@piData		指向需求数据项数组的指针,
//	   @iLen		数据项的个数
//	   @dwCurMin	当前时间，单位 分钟
//	   @dwIntervM	间隔
//返回:无
//备注:		
//void  CDpStat::DoVoltStat(int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType)
void  CStatMgr::DoVoltStat(void)
{
	int	i =	0;
	int	iRet = 0;

//台体新增功能测试<如往后对时，则对时跳过的时间段 统计到 合格累计时间中>-----------------------------------------------------------
	TTime tmNow;
	static DWORD dwLastSec = 0;
	static TTime tmLast;
	DWORD dwSetSec = 0;//往后对时，跨过的时间段
	DWORD dwMonSetSec = 0;
	bool fMonOver = false;
	bool fDayOver = false;
	DWORD dwCurSec = GetCurTime();
	BYTE bBuf3[8] = {0};

	//memset(bCalData, 0, sizeof(bCalData));
	//if ((dwCurSec/60) == (dwLastSec/60))
	GetCurTime(&tmNow);
	//if (MinutesFrom2000(tmLast) == MinutesFrom2000(tmNow))//每分钟执行一次
	//	return;
	
	if (dwLastSec == 0)
	{
		dwLastSec = dwCurSec;
		tmLast = tmNow;
	}
	DWORD wIntvSec = dwCurSec - dwLastSec;//时间间隔为秒,入库时转换成分
	fDayOver	= false;
	fMonOver= false;
//------------------------------------------------------------------------
	int iVal[3];
	BYTE k;

	memset(iVal, 0, sizeof(iVal));
	int iLen = OoReadVal(0x20000200, iVal, sizeof(iVal)/sizeof(int));//读取原始数据
	if (iLen <= 0)
		return ;

	BYTE bPhaseFlg[4] = {0};
	if (IsAllAByte((BYTE *)&m_VoltPara, 0, sizeof(TVoltStatPara)))
		return;//判断参数无效
	//日/月电压过限统计, 先统计分相再统计总
	for (i=1; i<4; i++)
	{
		//越上限
		if (iVal[i-1]>=m_VoltPara.wQualUpLimit)//wAssesUpLimit
		{			
			m_PhaseVoltStat[i].dayStat.dwUpperSecs+= wIntvSec;//日时间累计
			m_PhaseVoltStat[i].monStat.dwUpperSecs+= wIntvSec;//月时间累计
			bPhaseFlg[i] = 1;
		}
		else if(iVal[i-1]<=m_VoltPara.wQualLowLimit)//越下限wAssesLowLimit
		{
			m_PhaseVoltStat[i].dayStat.dwLowerSecs += wIntvSec;
			m_PhaseVoltStat[i].monStat.dwLowerSecs += wIntvSec;
			bPhaseFlg[i] = 2;
		}
		if ((iVal[i-1]<=m_VoltPara.wQualUpLimit) && (iVal[i-1]>=m_VoltPara.wQualLowLimit))//在合格区间内
		{
			m_PhaseVoltStat[i].dayStat.dwQualSecs += wIntvSec;
			m_PhaseVoltStat[i].monStat.dwQualSecs += wIntvSec;

		}
		if (bPhaseFlg[i] != 0)
		{
			m_PhaseVoltStat[i].dayStat.dwOverSecs += wIntvSec;
			m_PhaseVoltStat[i].monStat.dwOverSecs += wIntvSec;
		}
	}
//统计总电压的
	if ((bPhaseFlg[1]==0) && (bPhaseFlg[2]==0) && (bPhaseFlg[3]==0))//合格
	{//三相都合格时，总的才认为合格
		m_PhaseVoltStat[0].dayStat.dwQualSecs += wIntvSec;
		m_PhaseVoltStat[0].monStat.dwQualSecs += wIntvSec;
	}
	else
	{
		if (bPhaseFlg[1]==1 || bPhaseFlg[2]==1 || bPhaseFlg[3]==1)//超上限
		{
			m_PhaseVoltStat[0].dayStat.dwUpperSecs += wIntvSec;//日时间累计
			m_PhaseVoltStat[0].monStat.dwUpperSecs += wIntvSec; //月时间累计
		}
		//else	//超下限
		if (bPhaseFlg[1]==2 || bPhaseFlg[2]==2 || bPhaseFlg[3]==2)
		{
			m_PhaseVoltStat[0].dayStat.dwLowerSecs += wIntvSec;
			m_PhaseVoltStat[0].monStat.dwLowerSecs += wIntvSec; 
		}
		m_PhaseVoltStat[0].dayStat.dwOverSecs += wIntvSec;
		m_PhaseVoltStat[0].monStat.dwOverSecs += wIntvSec;
	}
	//累计各自的总监测时间并计算合格率与超限率
	DWORD dwQualSecs;
	DWORD dwMoniSecs;
	DWORD dwOverSecs;
	for (i=0; i<4; i++)
	{
		m_PhaseVoltStat[i].dayStat.dwMoniSecs += wIntvSec;
		m_PhaseVoltStat[i].monStat.dwMoniSecs += wIntvSec;//月的
		if (m_PhaseVoltStat[i].dayStat.dwMoniSecs != 0)	
		{
			if ((m_PhaseVoltStat[i].dayStat.dwMoniSecs/60) != 0)	
			{
				dwQualSecs = m_PhaseVoltStat[i].dayStat.dwQualSecs/60;
				dwMoniSecs = m_PhaseVoltStat[i].dayStat.dwMoniSecs/60;
				dwOverSecs = m_PhaseVoltStat[i].dayStat.dwOverSecs/60;
				if (m_PhaseVoltStat[i].dayStat.dwQualSecs%60 >= 55)//做4秒的防误差处理
					dwQualSecs += 1;
				if (m_PhaseVoltStat[i].dayStat.dwMoniSecs%60 >= 55)
					dwMoniSecs += 1;
				if (m_PhaseVoltStat[i].dayStat.dwOverSecs%60 >= 55)
					dwOverSecs += 1;
				
				//m_PhaseVoltStat[i].dayStat.wQualRate = (m_PhaseVoltStat[i].dayStat.dwQualSecs/60)*100*100/(m_PhaseVoltStat[i].dayStat.dwMoniSecs/60);
				m_PhaseVoltStat[i].dayStat.wQualRate = dwQualSecs*100*100/dwMoniSecs;
				if (m_PhaseVoltStat[i].dayStat.wQualRate > 10000)
					m_PhaseVoltStat[i].dayStat.wQualRate = 10000;
				//m_PhaseVoltStat[i].dayStat.wOverRate = (m_PhaseVoltStat[i].dayStat.dwOverSecs/60)*100*100/(m_PhaseVoltStat[i].dayStat.dwMoniSecs/60);
				m_PhaseVoltStat[i].dayStat.wOverRate = dwOverSecs*100*100/dwMoniSecs;
				if (m_PhaseVoltStat[i].dayStat.wOverRate > 10000)
					m_PhaseVoltStat[i].dayStat.wOverRate = 10000;
			}
		}
		else
		{
			memset((BYTE*)&m_PhaseVoltStat[i].dayStat, 0, sizeof(TVoltStat));
		}
		if (m_PhaseVoltStat[i].monStat.dwMoniSecs != 0)	
		{
			if ((m_PhaseVoltStat[i].monStat.dwMoniSecs/60) != 0)	
			{
				dwQualSecs = m_PhaseVoltStat[i].monStat.dwQualSecs/60;
				dwMoniSecs = m_PhaseVoltStat[i].monStat.dwMoniSecs/60;
				dwOverSecs = m_PhaseVoltStat[i].monStat.dwOverSecs/60;
				if (m_PhaseVoltStat[i].monStat.dwQualSecs%60 >= 49)//做4秒的防误差处理
					dwQualSecs += 1;
				if (m_PhaseVoltStat[i].monStat.dwMoniSecs%60 >= 49)
					dwMoniSecs += 1;
				if (m_PhaseVoltStat[i].monStat.dwOverSecs%60 >= 49)
					dwOverSecs += 1;

				 //m_PhaseVoltStat[i].monStat.wQualRate = (m_PhaseVoltStat[i].monStat.dwQualSecs/60)*100*100/(m_PhaseVoltStat[i].monStat.dwMoniSecs/60);
				m_PhaseVoltStat[i].monStat.wQualRate = dwQualSecs*100*100/dwMoniSecs;
				 if ( m_PhaseVoltStat[i].monStat.wQualRate > 10000)
				 	 m_PhaseVoltStat[i].monStat.wQualRate = 10000;
				//m_PhaseVoltStat[i].monStat.wOverRate = (m_PhaseVoltStat[i].monStat.dwOverSecs/60)*100*100/(m_PhaseVoltStat[i].monStat.dwMoniSecs/60);
				m_PhaseVoltStat[i].monStat.wOverRate = dwOverSecs*100*100/dwMoniSecs;
				 if (m_PhaseVoltStat[i].monStat.wOverRate > 10000) 
				 	m_PhaseVoltStat[i].monStat.wOverRate = 10000;
			}
		}
		else
		{
			memset((BYTE*)&m_PhaseVoltStat[i].monStat, 0, sizeof(TVoltStat));
		}
	}

	for (i=0; i<4; i++)
	{
		SavePhaseVoltStat(0x2130+i, RW_ATTR_RES, &m_PhaseVoltStat[i]);
	}

	if (!IsSameMon(tmNow, tmLast))
	{
		fMonOver = true;
		fDayOver = true;
		memset(bBuf3, 0, sizeof(bBuf3));
		for (i=0; i<4; i++)
		{
			//转存冻结，然后清当前数据
			SavePhaseVoltStat(0x2130+i, RW_ATTR_FRZ, &m_PhaseVoltStat[i]);
		}

		memset(m_PhaseVoltStat, 0, sizeof(TPhaseVoltStat)*4);//日\月合格率清零
		wIntvSec = 0;
		DTRACE(DB_FAPROTO, ("DoVoltStat:Mon Change\r\n"));//DB_DP
		for (i=0; i<4; i++)
		{
			SavePhaseVoltStat(0x2130+i, RW_ATTR_RES, &m_PhaseVoltStat[i]);
		}


		//char szBuf[200];
		//sprintf(szBuf, "转存数据为: ");
		//TraceBuf(DB_FAPROTO, szBuf, (BYTE*)&m_PhaseVoltStat[0], 4*sizeof(TPhaseVoltStat));
		
	}
	else if (!IsSameDay(tmNow, tmLast))//跨日了
	{
		fDayOver = true;
		//转存冻结，然后清当前数据
		for (i=0; i<4; i++)
		{
			//转存冻结，然后清当前数据
			SavePhaseVoltStat(0x2130+i, RW_ATTR_FRZ, &m_PhaseVoltStat[i]);
			memset((BYTE*)&m_PhaseVoltStat[i].dayStat, 0, sizeof(TVoltStat));
		}
		wIntvSec = 0;
		DTRACE(DB_FAPROTO, ("DoVoltStat:Day Change\r\n"));//DB_DP

		for (i=0; i<4; i++)
		{
			SavePhaseVoltStat(0x2130+i, RW_ATTR_RES, &m_PhaseVoltStat[i]);
		}

	}
	//先不考虑对时问题
	tmLast = tmNow;  
	dwLastSec = dwCurSec;

}

//区间统计
void CStatMgr::IntvStat()
{
	BYTE i, j, k;
	WORD wLen = 0;
	TTime tmLastCycleTime;
	BYTE bBuf[16];
	TTime tmNow;
	TIntvStatRela * pRela = NULL;//统计参数
	TIntvStatRes * pRes = NULL;//统计结果
	GetCurTime(&tmNow);
	
	for (i=0; i<5; i++)//分、时、日、月、年
	{
		pRela = m_IntvStatRela[i];
		pRes = m_IntvStatRes[i];
		
		if (m_bIntvStatNum[i] == 0)
			continue;
		if (m_bIntvStatNum[i]  > STAT_OAD_NUM)
			m_bIntvStatNum[i] = STAT_OAD_NUM;
		
		for (j=0; j<m_bIntvStatNum[i] ; j++)//按关联对象属性表遍历
		{
			DWORD dwIntVSec;
			BYTE bCycleSwitch[STAT_OAD_NUM];

			if (pRela[j].dwOAD == 0 || pRela[j].wParaNum == 0)
				continue;
			
			memset(bCycleSwitch, 0, STAT_OAD_NUM);
			pRes[j].dwOAD = pRela[j].dwOAD;
			pRes[j].wIntvNum = pRela[j].wParaNum+1;
			
			ReadItemEx(BN11, j, 0x0014+i, bBuf);//读周期与频率的记录时间
			TStatExeCtrl exeCtrl;
			memcpy((BYTE *)&exeCtrl, bBuf, sizeof(TStatExeCtrl));
			BYTE bMoreCycle = 0;
			//判断周期是否到,如到了则进行周期切换，需要处理数据重新统计
			if (IsCycleSwitch(exeCtrl.tmCurCycle, tmNow, i+1, pRela[j].bCycleValue, &dwIntVSec, &bMoreCycle))
			{
				//此时需要进行冻结数据的转存,
				SaveIntvStatRes(0x0023+i, RW_ATTR_FRZ, pRes, m_bIntvStatNum[i]);
				
				//memset(pRes[j].intvVal, 0, sizeof(pRes[j].intvVal));//清累计时间和次数
				//SaveIntvStatRes(0x2108+i, RW_ATTR_RES, pRes, m_bIntvStatNum[i]);

				exeCtrl.tmCurCycle = tmNow;
				//exeCtrl.tmLastSample = tmNow;
				memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
				WriteItemEx(BN11, j, 0x0014+i, bBuf);
				DTRACE(DB_DP, ("IntvStat=%d   Cycle switch\r\n",i));
				bCycleSwitch[j] = 1;
				
			}
			if ((bCycleSwitch[j] == 1) && (bMoreCycle == 1))
			{
				memset(pRes[j].intvVal, 0, sizeof(pRes[j].intvVal));//清累计时间和次数
				SaveIntvStatRes(0x2108+i, RW_ATTR_RES, pRes, m_bIntvStatNum[i]);
				exeCtrl.tmLastSample = tmNow;
				memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
				WriteItemEx(BN11, j, 0x0014+i, bBuf);
				continue;
			}

			//判断统计频率时间是否到
			BYTE bMoreFreq = 0;
			if ( IsCycleSwitch(exeCtrl.tmLastSample, tmNow,pRela[j].tiFreq.bUnit, pRela[j].tiFreq.wVal, &dwIntVSec, &bMoreFreq) == false)
			{
				if (bCycleSwitch[j] == 1)
				{
					memset(pRes[j].intvVal, 0, sizeof(pRes[j].intvVal));
					SaveIntvStatRes(0x2108+i, RW_ATTR_RES, pRes, m_bIntvStatNum[i]);
					exeCtrl.tmLastSample = tmNow;
					memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
					WriteItemEx(BN11, j, 0x0014+i, bBuf);
				}
				continue;
			}

			exeCtrl.tmLastSample = tmNow;
			memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
			WriteItemEx(BN11, j, 0x0014+i, bBuf);
			
			int iVal[5];
			memset(iVal, 0, sizeof(iVal));
			int iLen = OoReadVal(pRela[j].dwOAD, iVal, sizeof(iVal)/sizeof(int));//读取原始数据
			if (iLen <= 0)
			{
				continue;
			}
			DWORD dwbaksec;
			//下边进行数据判断统计
			for (k=0; k<pRela[j].wParaNum; k++)
			{
				if (iVal[0] <= pRela[j].iParaVal[k])
				{
					pRes[j].intvVal[k].dwTotalSecs += dwIntVSec;
					dwbaksec = pRes[j].intvVal[k].dwTotalSecs;
					pRes[j].intvVal[k].dwTotalSecs = dwbaksec/60*60;
					if (dwbaksec%60 > 40)
						pRes[j].intvVal[k].dwTotalSecs += 60;
					pRes[j].intvVal[k].dwTimes += 1;
					break;
				}
			}
			if (k >= pRela[j].wParaNum)//此时认为被统计数据处于最后那个范围区间
			{
				pRes[j].intvVal[k].dwTotalSecs += dwIntVSec;
				dwbaksec = pRes[j].intvVal[k].dwTotalSecs;
				pRes[j].intvVal[k].dwTotalSecs = dwbaksec/60*60;
				if (dwbaksec%60 > 40)
					pRes[j].intvVal[k].dwTotalSecs += 60;
				pRes[j].intvVal[k].dwTimes += 1;
			}
			SaveIntvStatRes(0x2108+i, RW_ATTR_RES, pRes, m_bIntvStatNum[i]);
			if (bCycleSwitch[j] == 1)
			{
				memset(pRes[j].intvVal, 0, sizeof(pRes[j].intvVal));
			}

		}

	}
}

//描述：判断是否已过了一个给定的间隔了，
//参数：@tmLastCycle 上一次的执行时间
//		@ tmNow当前时间
//		@bUnit间隔类型(分、时、日、月、年)
//		@ wValue	间隔值
//		@ dwIntVSec	返回间隔时间的秒数
//		@bMoreCycle 是否已过多个周期时间
//返回:如果两个间隔值不相等则返回true,如果相等则返回false
bool CStatMgr::IsCycleSwitch(TTime tmLastCycle, TTime tmNow, BYTE bUnit, WORD wValue, DWORD * dwIntVSec, BYTE * bMoreCycle)
{
	bool flage = false;
	DWORD dwTime1, dwTime2;
	DWORD dwData = 0;

	if (wValue == 0)
		return false;
	switch (bUnit)
	{
	case 1://MINUTE:
		//dwTime1 = MinutesFrom2000(tmLastCycle);
		//dwTime2 = MinutesFrom2000(tmNow);
		dwTime1 = TimeToSeconds(tmLastCycle);
		dwTime2 = TimeToSeconds(tmNow);
		dwData = 60;
		if (tmLastCycle.nYear==0 || tmLastCycle.nMonth==0 || tmLastCycle.nDay==0 || (dwTime1 > dwTime2))
		{
			flage = true;
			*dwIntVSec = 0;
			* bMoreCycle = 1;
			return flage;
		}
		if ((dwTime2>= dwTime1+wValue*dwData))
			flage = true;
		if (((dwTime2/dwData) >= (dwTime1/dwData +2* wValue)))
			* bMoreCycle = 1;
		break;
		
	case 2://HOUR:
		//dwTime1 = MinutesFrom2000(tmLastCycle);
		//dwTime2 = MinutesFrom2000(tmNow);
		//dwTime1 /= 60*wValue;
		//dwTime2 /= 60*wValue;
		dwTime1 = TimeToSeconds(tmLastCycle);
		dwTime2 = TimeToSeconds(tmNow);
		dwData = 3600;
		if (tmLastCycle.nYear==0 || tmLastCycle.nMonth==0 || tmLastCycle.nDay==0 || (dwTime1 > dwTime2))
		{
			flage = true;
			*dwIntVSec = 0;
			* bMoreCycle = 1;
			return flage;
		}
		if ((dwTime2>= dwTime1+wValue*dwData))
			flage = true;
		if (((dwTime2/dwData) >= (dwTime1/dwData +2* wValue)))
			* bMoreCycle = 1;
		//else if (dwTime1 != dwTime2)
		//	flage = true;
		break;
	case 3://DAY:
		//dwTime1 = DaysFrom2000(tmLastCycle);
		//dwTime2 = DaysFrom2000(tmNow);
		dwTime1 = TimeToSeconds(tmLastCycle);
		dwTime2 = TimeToSeconds(tmNow);
		dwData = 86400;
		if (tmLastCycle.nYear==0 || tmLastCycle.nMonth==0 || tmLastCycle.nDay==0 || (dwTime1 > dwTime2))
		{
			flage = true;
			*dwIntVSec = 0;
			* bMoreCycle = 1;
			return flage;
		}
		if ((dwTime2>= dwTime1+wValue*dwData))
			flage = true;
		if (((dwTime2/dwData) >= (dwTime1/dwData +2* wValue)))
			* bMoreCycle = 1;
		//else if ((dwTime1/wValue) != (dwTime2/wValue))
		//	flage = true;
		break;
	case 4://MONTH:
		dwTime1 = MinutesFrom2000(tmLastCycle);
		dwTime2 = MinutesFrom2000(tmNow);
		if (tmLastCycle.nYear==0 || tmLastCycle.nMonth==0 || tmLastCycle.nDay==0 || (dwTime1 > dwTime2))
		{
			flage = true;
			*dwIntVSec = 0;
			* bMoreCycle = 1;
			return flage;
		}
		else if (wValue<=MonthsPast(tmLastCycle, tmNow))
		{
			flage = true;
			if (2*wValue<=MonthsPast(tmLastCycle, tmNow))
				* bMoreCycle = 1;
		}
		break;
	case 5://YEAR:
		dwTime1 = MinutesFrom2000(tmLastCycle);
		dwTime2 = MinutesFrom2000(tmNow);
		if (tmLastCycle.nYear==0 || tmLastCycle.nMonth==0 || tmLastCycle.nDay==0 || (dwTime1 > dwTime2))
		{
			flage = true;
			*dwIntVSec = 0;
			return flage;
		}
		else if ((wValue*12)<=MonthsPast(tmLastCycle, tmNow))
		{
			flage = true;
			if (2*12*wValue<=MonthsPast(tmLastCycle, tmNow))
				* bMoreCycle = 1;
		}
		break;
	default:
		return false;
	}
	if (flage)
	{
		if (TimeToSeconds(tmNow)>TimeToSeconds(tmLastCycle))//倒退校时
			*dwIntVSec = TimeToSeconds(tmNow)-TimeToSeconds(tmLastCycle);
		else
			*dwIntVSec = 0;
	}
	return flage;
}





//描述：把区间统计结果进行格式转换并保存到系统库
//参数：@wOI 分、时、日、月、年区间统计OI
//		@bAttr 要写入的属性（当前、上一周期）
//		@pRes分时日月年数组的指针
//		@wStatNum 分、时、日、月、年统计的OAD的个数
void CStatMgr::SaveIntvStatRes(WORD wOI, BYTE bAttr, TIntvStatRes * pRes, BYTE wStatNum)
{
	BYTE bData[3000];
	BYTE k;
	BYTE * p = bData;
	int iret;
	
	memset(bData, 0, sizeof(bData));
	*p++ = DT_ARRAY;
	if (wStatNum > STAT_OAD_NUM)
		wStatNum = STAT_OAD_NUM;
	*p++ = wStatNum;
	for (BYTE j=0; j<wStatNum ; j++)
	{
		*p++ = DT_STRUCT;
		*p++ = 0x02;
		*p++ = DT_OAD;
		OoDWordToDoubleLongUnsigned(pRes[j].dwOAD, p);
		p += 4;
		*p++ = DT_ARRAY;
		*p++ = pRes[j].wIntvNum;
			
		for (k=0; k<pRes[j].wIntvNum;k++)
		{
			*p++ = DT_STRUCT;
			*p++ = 0x02;
			*p++ = DT_DB_LONG_U;
			OoDWordToDoubleLongUnsigned((pRes[j].intvVal[k].dwTotalSecs), p);
			p += 4;
			*p++ = DT_DB_LONG_U;
			OoDWordToDoubleLongUnsigned(pRes[j].intvVal[k].dwTimes, p);
			p += 4;
		}
	}
	if (bAttr != RW_ATTR_FRZ)
		iret = OoWriteAttr(wOI, bAttr, bData);
	else
		WriteItemEx(BN11, PN0, wOI, bData);
}

//描述：从系统库中装入区间统计关联对象
//参数：@wOI 分、时、日、月、年区间统计OI
//		@pRela区间统计关联对象
//		@pwStatNum用来返回分、时、日、月、年统计的OAD的个数
bool CStatMgr::LoadIntvStatRela(WORD wOI, TIntvStatRela* pRela, BYTE*pwStatNum)
{
	BYTE bData[3000];
	BYTE * p = bData;
	memset(bData, 0, sizeof(bData));
	if (OoReadAttr(wOI, 0x03, bData, NULL, NULL) < 0)
		return false;
	p ++;//跳过数组类型
	*pwStatNum = *p ++;
	if (*pwStatNum > STAT_OAD_NUM)
	{
		*pwStatNum = 0;
		return false;
	}
	for (BYTE j=0; j<*pwStatNum; j++)
	{
		p += 3;
		pRela[j].dwOAD= OoDoubleLongUnsignedToDWord(p);
		if (pRela[j].dwOAD == 0)
		{
			*pwStatNum = 0;//pRela[j].wParaNum = INTV_STAT_PARA_NUM;
			return false;
		}
		p += 5;
		pRela[j].wParaNum= *p;
		if (pRela[j].wParaNum > INTV_STAT_PARA_NUM)
		{
			*pwStatNum = 0;//pRela[j].wParaNum = INTV_STAT_PARA_NUM;
			return false;
		}
		p += 1;
		int bLen = 0;
		for (BYTE k=0; k<pRela[j].wParaNum; k++)
		{
			bLen = OoGetDataVal(p, &pRela[j].iParaVal[k]);//等陈工那边做好这个接口
			if (bLen < 0)
			{
				*pwStatNum = 0;
				return false;
			}
			p += bLen;
		}
		//p += (bLen+1)*pRela[j].wParaNum;
		p += 1;
		pRela[j].bCycleValue = *p ++;
		p += 1;
		pRela[j].tiFreq.bUnit= *p ++;
		pRela[j].tiFreq.wVal= OoLongUnsignedToWord(p);
		p += 2;
	}
	return true;
}

//描述：从系统库中装入区间统计数据
//参数：@wOI 分、时、日、月、年区间统计OI
//		@pRes区间统计数据
void CStatMgr::LoadIntvStatRes(WORD wOI, TIntvStatRes* pRes, BYTE wStatNum)
{
	BYTE bData[2830];
	BYTE * p = bData;
	BYTE k;
	memset(bData, 0, sizeof(bData));
	if (OoReadAttr(wOI, 0x02, bData, NULL, NULL) < 0)
		return;
	p += 2;////跳过数组
	for (BYTE j=0; j<wStatNum ; j++)
	{
		p += 3;
		pRes[j].dwOAD = OoDoubleLongUnsignedToDWord(p);
		p += 5;
		pRes[j].wIntvNum = *p++;
		for (k=0; k<pRes[j].wIntvNum; k++)
		{
			p += 3;
			pRes[j].intvVal[k].dwTotalSecs = OoDoubleLongUnsignedToDWord(p);
			p += 5;
			pRes[j].intvVal[k].dwTimes = OoDoubleLongUnsignedToDWord(p);
			p += 4;
		}	
	}

}

//累加平均统计
void CStatMgr::AccAvgStat()
{
	BYTE i, j, k;
	WORD bTimes = 0;
	WORD wLen = 0;
	TTime tmLastCycleTime;
	BYTE bBuf[16];
	TTime tmNow;
	TStatRela * pRela = NULL;//统计参数
	TAccAvgStatRes * pRes = NULL;//统计结果
	GetCurTime(&tmNow);
	for (i=0; i<5; i++)//分、时、日、月、年
	{
		pRela = m_AvgStatRela[i];
		pRes = m_AvgStatRes[i];
		
		if (m_bAvgStatNum[i] == 0)
			continue;
		if (m_bIntvStatNum[i]  > STAT_OAD_NUM)
			m_bIntvStatNum[i] = STAT_OAD_NUM;

		for (j=0; j<m_bAvgStatNum[i] ; j++)//按关联对象属性表遍历
		{
			DWORD dwIntVSec;
			if (pRela[j].dwOAD == 0)// || pRela[j].wParaNum == 0)
				continue;
			pRes[j].dwOAD = pRela[j].dwOAD;
			ReadItemEx(BN11, j, 0x003b + i, (BYTE *)&bTimes);//读次数
			ReadItemEx(BN11, j, 0x0019+i, bBuf);//读周期与频率的记录时间
			TStatExeCtrl exeCtrl;
			memcpy((BYTE *)&exeCtrl, bBuf, sizeof(TStatExeCtrl));
			BYTE bMoreCycle = 0;
			//判断周期是否到,如到了则进行周期切换，需要处理数据重新统计
			if (IsCycleSwitch(exeCtrl.tmCurCycle, tmNow, i+1, pRela[j].bCycleValue, &dwIntVSec, &bMoreCycle))
			{
				//此时需要进行冻结数据的转存,
				SaveAvgStatRes(0x0028+i, RW_ATTR_FRZ, pRes, m_bAvgStatNum[i]);
				
				pRes[j].iAcc = 0;//清统计结果
				pRes[j].iAvg = 0;
				SaveAvgStatRes(0x2118+i, RW_ATTR_RES, pRes, m_bAvgStatNum[i]);
				bTimes = 0;
				WriteItemEx(BN11, j, 0x003b + i, (BYTE *)&bTimes);//清累计次数

				exeCtrl.tmCurCycle = tmNow;
				memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
				WriteItemEx(BN11, j, 0x0019+i, bBuf);
				DTRACE(DB_DP, ("AccAvgStat=%d   Cycle switch\r\n",i));
				
			}
			//判断统计频率时间是否到
			if ( IsCycleSwitch(exeCtrl.tmLastSample, tmNow,pRela[j].tiFreq.bUnit, pRela[j].tiFreq.wVal, &dwIntVSec, &bMoreCycle) == false)
			{
				continue;
			}

			exeCtrl.tmLastSample = tmNow;
			memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
			WriteItemEx(BN11, j, 0x0019+i, bBuf);
			
			int iVal[5];
			memset(iVal, 0, sizeof(iVal));
			int iLen = OoReadVal(pRela[j].dwOAD, iVal, sizeof(iVal)/sizeof(int));//读取原始数据
			if (iLen <= 0)
			{
				continue;
			}
			bTimes ++;
			pRes[j].iAcc += iVal[0];
			if (bTimes == 0)
				bTimes = 1;
			pRes[j].iAvg = pRes[j].iAcc/bTimes;
			
			WriteItemEx(BN11, j, 0x003b + i, (BYTE *)&bTimes);
		}
		SaveAvgStatRes(0x2118+i, RW_ATTR_RES, pRes, m_bAvgStatNum[i]);

	}
}


//描述：把平均统计结果进行格式转换并保存到系统库
//参数：@wOI 分、时、日、月、年平均统计OI
//		@bAttr 要写入的属性（当前、上一周期）
//		@pRes分时日月年数组的指针
//		@wStatNum 分、时、日、月、年统计的OAD的个数
//		@Type 数据
void CStatMgr::SaveAvgStatRes(WORD wOI, BYTE bAttr, TAccAvgStatRes * pRes, BYTE wStatNum)
{
	BYTE bData[2830];
	BYTE k;
	BYTE * p = bData;
	BYTE bValType;
	int ValLen=0;
	DWORD dwtOAD;
	WORD wtmpOI;
	
	memset(bData, 0, sizeof(bData));
	*p++ = DT_ARRAY;
	if (wStatNum > STAT_OAD_NUM)
		wStatNum = STAT_OAD_NUM;
	*p++ = wStatNum;
	for (BYTE j=0; j<wStatNum ; j++)
	{
		*p++ = DT_STRUCT;
		*p++ = 0x03;
		*p++ = DT_OAD;
		OoDWordToDoubleLongUnsigned(pRes[j].dwOAD, p);
		p += 4;
		wtmpOI = pRes[j].dwOAD >> 16;
		dwtOAD = pRes[j].dwOAD;
		if ((wtmpOI>= 0x2000 && wtmpOI<=0x200e)||(wtmpOI < 0x11A3))		
		{
			if ((dwtOAD & 0x000000ff) == 0)
				dwtOAD |= 0x0001;//因OoGetValType()对于全属性取不到成员的数据类型
		}
		if (!OoGetValType(dwtOAD, &bValType))
			continue;
		ValLen = OoValToFmt(bValType, pRes[j].iAcc, p);
		if (ValLen < 0)
			return;
		p += ValLen;
		ValLen = OoValToFmt(bValType, pRes[j].iAvg, p);
		if (ValLen < 0)
			return;
		p += ValLen;
	}
	if (bAttr != RW_ATTR_FRZ)
		OoWriteAttr(wOI, bAttr, bData);
	else
		WriteItemEx(BN11, PN0, wOI, bData);
}
//极值和累加都调用这一个参数装载接口
//描述：从系统库中装入平均统计关联对象
//参数：@wOI 分、时、日、月、年平均统计OI
//		@pRela平均统计关联对象
//		@pwStatNum用来返回分、时、日、月、年统计的OAD的个数
bool CStatMgr::LoadAvgStatRela(WORD wOI, TStatRela* pRela, BYTE*pwStatNum)
{
	BYTE bData[3000];
	BYTE * p = bData;
	memset(bData, 0, sizeof(bData));
	if (OoReadAttr(wOI, 0x03, bData, NULL, NULL) < 0)
		return false;
	*p ++;//跳过数组类型
	*pwStatNum = *p ++;
	if (*pwStatNum > STAT_OAD_NUM)
	{
		*pwStatNum = 0;
		return false;
	}
	for (BYTE j=0; j<*pwStatNum; j++)
	{
		p += 3;
		pRela[j].dwOAD= OoDoubleLongUnsignedToDWord(p);
		p += 5;
		pRela[j].bCycleValue = *p ++;
		p += 1;
		pRela[j].tiFreq.bUnit= *p;
		p += 1;
		pRela[j].tiFreq.wVal= OoLongUnsignedToWord(p);
		p += 2;
	}
	return true;
}

//描述：从系统库中装入平均统计数据
//参数：@wOI 分、时、日、月、年平均	统计OI
//		@pRes平均统计数据
void CStatMgr::LoadAvgStatRes(WORD wOI, TAccAvgStatRes* pRes, BYTE wStatNum)
{
	BYTE bData[2830];
	BYTE bValType;
	int ValLen=0;
	BYTE * p = bData;
	BYTE k;
	memset(bData, 0, sizeof(bData));
	if (OoReadAttr(wOI, 0x02, bData, NULL, NULL) < 0)
		return;
	if (p[0] != 0x02 && p[1] != 0x02)//数据无效
		return;
	 p += 2;//跳过数组
	for (BYTE j=0; j<wStatNum ; j++)
	{
		p += 3;
		pRes[j].dwOAD = OoDoubleLongUnsignedToDWord(p);
		p += 5;
		ValLen = OoGetDataVal(p, &pRes[j].iAcc);
		if (ValLen < 0)
			return;
		p += ValLen;
		ValLen = OoGetDataVal(p, &pRes[j].iAcc);
		if (ValLen < 0)
			return;
		p += ValLen;
	}

}

//极值统计
void CStatMgr::ExtremStat()
{
	BYTE i, j, k;
	WORD wLen = 0;
	TTime tmLastCycleTime;
	BYTE bBuf[16];
	TStatRela * pRela = NULL;//统计参数
	TExtremStatRes * pRes = NULL;//统计结果
	TTime tmNow;
	GetCurTime(&tmNow);
	for (i=0; i<5; i++)//分、时、日、月、年
	{
		pRela = m_ExtremStatRela[i];
		pRes = m_ExtremStatRes[i];
		
		if (m_bAvgStatNum[i] == 0)
			continue;
		for (j=0; j<m_bExtremStatNum[i] ; j++)//按关联对象属性表遍历
		{
			DWORD dwIntVSec;
			if (pRela[j].dwOAD == 0)// || pRela[j].wParaNum == 0)
				continue;
			pRes[j].dwOAD = pRela[j].dwOAD;
			ReadItemEx(BN11, j, 0x001e + i, bBuf);//读周期与频率的记录时间
			TStatExeCtrl exeCtrl;
			memcpy((BYTE *)&exeCtrl, bBuf, sizeof(TStatExeCtrl));
			BYTE bMoreCycle = 0;
			//判断周期是否到,如到了则进行周期切换，需要处理数据重新统计
			if (IsCycleSwitch(exeCtrl.tmCurCycle, tmNow, i+1, pRela[j].bCycleValue, &dwIntVSec, &bMoreCycle))
			{
				//此时需要进行冻结数据的转存,
				SaveExtremStatRes(0x002d+i, RW_ATTR_FRZ, pRes, m_bExtremStatNum[i]);
				
				memset((BYTE *)&pRes[j].iMax, 0, sizeof(TExtremStatRes)-sizeof(DWORD));
				SaveExtremStatRes(0x2128+i, RW_ATTR_RES, pRes, m_bExtremStatNum[i]);

				exeCtrl.tmCurCycle = tmNow;
				memset(&exeCtrl.tmLastSample, 0, sizeof(TTime));
				memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
				WriteItemEx(BN11, j, 0x001e + i, bBuf);
				DTRACE(DB_DP, ("ExtremStat=%d   Cycle switch\r\n",i));
				
			}
			//判断统计频率时间是否到
			if ( IsCycleSwitch(exeCtrl.tmLastSample, tmNow,pRela[j].tiFreq.bUnit, pRela[j].tiFreq.wVal, &dwIntVSec, &bMoreCycle) == false)
			{
				continue;
			}

			exeCtrl.tmLastSample = tmNow;
			memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
			WriteItemEx(BN11, j, 0x001e + i, bBuf);
			
			int iVal[5];
			memset(iVal, 0, sizeof(iVal));
			int iLen = OoReadVal(pRela[j].dwOAD, iVal, sizeof(iVal)/sizeof(int));//读取原始数据
			//DTRACE(DB_DP, ("OAD=%8x  iVal0=%d, iVal1=%d, iVal2=%d\r\n",pRela[j].dwOAD,iVal[0],iVal[1],iVal[2]));
			if (iLen <= 0)
			{
				continue;
			}
			if ((iVal[0] > pRes[j].iMax) ||IsAllAByte((BYTE*)&pRes[j].tmMax, 0, sizeof(TTime)))
			{
				pRes[j].iMax = iVal[0];
				pRes[j].tmMax = tmNow;
			}
			if ((iVal[0] < pRes[j].iMin) || IsAllAByte((BYTE*)&pRes[j].tmMin, 0, sizeof(TTime)))
			{
				pRes[j].iMin = iVal[0];
				pRes[j].tmMin= tmNow;
			}
			
		}
		SaveExtremStatRes(0x2128+i, RW_ATTR_RES, pRes, m_bExtremStatNum[i]);

	}
}

//描述：把极值统计结果进行格式转换并保存到系统库
//参数：@wOI 分、时、日、月、年极值统计OI
//		@bAttr 要写入的属性（当前、上一周期）
//		@pRes分时日月年数组的指针
//		@wStatNum 分、时、日、月、年统计的OAD的个数
void CStatMgr::SaveExtremStatRes(WORD wOI, BYTE bAttr, TExtremStatRes * pRes, BYTE wStatNum)
{
	BYTE bData[2830];
	BYTE k;
	BYTE * p = bData;
	BYTE bValType;
	int ValLen=0;
	DWORD dwtOAD;
	WORD wtmpOI;
	
	memset(bData, 0, sizeof(bData));
	*p++ = DT_ARRAY;
	*p++ = wStatNum;
	for (BYTE j=0; j<wStatNum ; j++)
	{
		*p++ = DT_STRUCT;
		*p++ = 0x05;
		*p++ = DT_OAD;
		OoDWordToDoubleLongUnsigned(pRes[j].dwOAD, p);
		p += 4;
		wtmpOI = pRes[j].dwOAD >> 16;
		dwtOAD = pRes[j].dwOAD;
		if ((wtmpOI>= 0x2000 && wtmpOI<=0x200e)||(wtmpOI < 0x11A3))
		{
			if ((dwtOAD & 0x000000ff) == 0)
				dwtOAD |= 0x0001;//因OoGetValType()对于全属性取不到成员的数据类型
		}
		if (!OoGetValType(dwtOAD, &bValType))
			continue;
		ValLen = OoValToFmt(bValType, pRes[j].iMax, p);
		if (ValLen < 0)
			return;
		p += ValLen;
		*p++ = DT_DATE_TIME_S;
		OoWordToLongUnsigned(pRes[j].tmMax.nYear, p);
		p += 2;
		memcpy(p, (BYTE*)&pRes[j].tmMax.nMonth,5);
		p += 5;
		ValLen = OoValToFmt(bValType, pRes[j].iMin, p);
		p += ValLen;
		if (ValLen < 0)
			return;
		*p++ = DT_DATE_TIME_S;
		OoWordToLongUnsigned(pRes[j].tmMin.nYear, p);
		p += 2;
		memcpy(p, (BYTE*)&pRes[j].tmMin.nMonth,5);
		p += 5;
		
	}
	if (bAttr != RW_ATTR_FRZ)
		OoWriteAttr(wOI, bAttr, bData);
	else
		WriteItemEx(BN11, PN0, wOI, bData);
}

//描述：从系统库中装入极值统计数据
//参数：@wOI 分、时、日、月、年极值统计OI
//		@pRes极值统计数据
void CStatMgr::LoadExtremStatRes(WORD wOI, TExtremStatRes* pRes, BYTE wStatNum)
{
	BYTE bData[2830];
	BYTE bValType;
	int ValLen=0;
	BYTE * p = bData;
	BYTE k;
	memset(bData, 0, sizeof(bData));
	if (OoReadAttr(wOI, 0x02, bData, NULL, NULL) < 0)
		return;
	if (p[0] != 0x02 && p[1] != 0x02)//数据无效
		return;
	 p += 2;////跳过数组
	for (BYTE j=0; j<wStatNum ; j++)
	{
		p += 3;
		pRes[j].dwOAD = OoDoubleLongUnsignedToDWord(p);
		p += 5;
		ValLen = OoGetDataVal(p, &pRes[j].iMax);
		if (ValLen < 0)
			return;
		p += ValLen;
		p ++;
		pRes[j].tmMax.nYear=OoLongUnsignedToWord(p);
		p += 2;
		memcpy((BYTE*)&pRes[j].tmMax.nMonth, p, 5);
		p += 5;
		ValLen = OoGetDataVal(p, &pRes[j].iMin);
		if (ValLen < 0)
			return;
		p += ValLen;
		p ++;
		pRes[j].tmMin.nYear=OoLongUnsignedToWord(p);
		p += 2;
		memcpy((BYTE*)&pRes[j].tmMin.nMonth, p, 5);
		p += 5;
	}

}


//装载所有统计类的参数
void CStatMgr::LoadStatRela()
{
	BYTE i;
	LoadVoltStatPara(&m_VoltPara);
	
	memset((BYTE*)&m_IntvStatRela[0][0], 0, sizeof(m_IntvStatRela));
	for (i=0; i<5; i++)
	{
		LoadIntvStatRela(0x2100+i, m_IntvStatRela[i], &m_bIntvStatNum[i]);
	}

	memset((BYTE*)&m_AvgStatRela[0][0], 0, sizeof(m_AvgStatRela));
	for (i=0; i<5; i++)
	{
		LoadAvgStatRela(0x2110+i, m_AvgStatRela[i], &m_bAvgStatNum[i]);
	}

	memset((BYTE*)&m_ExtremStatRela[0][0], 0, sizeof(m_ExtremStatRela));
	for (i=0; i<5; i++)
	{
		LoadAvgStatRela(0x2120+i, m_ExtremStatRela[i], &m_bExtremStatNum[i]);
	}
}

//装载所有统计数据
void CStatMgr::LoadStatRes()
{
	BYTE i;
	memset((BYTE *)&m_PhaseVoltStat[0], 0, sizeof(m_PhaseVoltStat));
	for (i=0; i<4; i++)
	{
		LoadPhaseVoltStat(0x2130+i, &m_PhaseVoltStat[i]);
	}

	memset((BYTE*)&m_IntvStatRes[0][0], 0, sizeof(m_IntvStatRes));
	for (i=0; i<5; i++)
	{
		LoadIntvStatRes(0x2108+i, m_IntvStatRes[i], m_bIntvStatNum[i]);
	}

	memset((BYTE*)&m_AvgStatRes[0][0], 0, sizeof(m_AvgStatRes));
	for (i=0; i<5; i++)
	{
		LoadAvgStatRes(0x2118+i, m_AvgStatRes[i], m_bAvgStatNum[i]);
	}

	memset((BYTE*)&m_ExtremStatRes[0][0], 0, sizeof(m_ExtremStatRes));
	for (i=0; i<5; i++)
	{
		LoadExtremStatRes(0x2128+i, m_ExtremStatRes[i], m_bExtremStatNum[i]);
	}


}

void CStatMgr::CalcuAvgPower(void)
{
	BYTE i, j, bTmp, bCnt = 0;
	int iSumTmp[3*4];//p/q/s
	int iP[4];     //分别是A,B,C和总
	int iQ[4];     //分别是A,B,C和总
	int iS[4];   	 //分别是A,B,C和总
	BYTE bBuf[32];
	
	//记录次数
	m_bPowrIndexCnt++;
	if(m_bPowrIndexCnt>=POWER_AVG_NUM_MAX)
	{
		m_bPowrIndexCnt = POWER_AVG_NUM_MAX;
	}

	//读取实时功率值
	memset(iP, 0, sizeof(iP));
	OoReadVal(0x20040200, iP, 4);
	memset(iQ, 0, sizeof(iQ));
	OoReadVal(0x20050200, iQ, 4);
	memset(iS, 0, sizeof(iS));
	OoReadVal(0x20060200, iS, 4);

	
	//更新实时量
	m_bPowrIndex %= POWER_AVG_NUM_MAX;

	for(i=0;i<4;i++)//total/a/b/c/
	{
		m_iPower[i+0][m_bPowrIndex] = iP[i];
		m_iPower[i+4][m_bPowrIndex] = iQ[i];
		m_iPower[i+8][m_bPowrIndex] = iS[i];
	}		
	m_bPowrIndex++;
	
	
	//计算平均功率
	memset(iSumTmp, 0, sizeof(iSumTmp));
	for(j=0;j<12;j++)
	{
		for(i=0;i<m_bPowrIndexCnt;i++)
		{
			iSumTmp[j] += m_iPower[j][i];
		}
		iSumTmp[j] /= m_bPowrIndexCnt;
	}	
	
#ifndef SYS_WIN
	PowerValToDb(&iSumTmp[0],bBuf,sizeof(bBuf));
	WriteItemEx(BN0, PN0, 0x2007, bBuf);
	PowerValToDb(&iSumTmp[4],bBuf,sizeof(bBuf));
	WriteItemEx(BN0, PN0, 0x2008, bBuf);
	PowerValToDb(&iSumTmp[8],bBuf,sizeof(bBuf));
	WriteItemEx(BN0, PN0, 0x2009, bBuf);
#endif	
	
	return;
}

void CStatMgr::DoPowerStat()
{
	BYTE bBuf[20];
	int iP = 0;
	int iVal;
	TTime tmNow;
	GetCurTime(&tmNow);
	WORD wYear;

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadVal(0x20040201, &iP, 1) < 0)
		return;
	iP = abs(iP);
	
	if (OoReadAttr(0x2140, 0x02, bBuf, NULL, NULL) < 0)
		return;
	wYear = OoLongUnsignedToWord(&bBuf[8]);
	if (wYear != tmNow.nYear||bBuf[10] != tmNow.nMonth||bBuf[11] != tmNow.nDay)//日切换
	{
		memset(bBuf+3, 0, 4);
		OoWordToLongUnsigned(tmNow.nYear, bBuf+8);
		memcpy(bBuf+10, (BYTE*)&tmNow.nMonth,5);
		OoWriteAttr(0x2140, 0x02, bBuf);
	}
	iVal = OoDoubleLongUnsignedToDWord(&bBuf[3]);
	if ((iP > iVal) || IsAllAByte(bBuf+8, 0, 7))
	{
		OoDWordToDoubleLongUnsigned(iP, bBuf+3);
		OoWordToLongUnsigned(tmNow.nYear, bBuf+8);
		memcpy(bBuf+10, (BYTE*)&tmNow.nMonth,5);
		OoWriteAttr(0x2140, 0x02, bBuf);
	}

	if (OoReadAttr(0x2141, 0x02, bBuf, NULL, NULL) < 0)
		return;
	wYear = OoLongUnsignedToWord(&bBuf[8]);
	if (wYear != tmNow.nYear||bBuf[10] != tmNow.nMonth)//月切换
	{
		memset(bBuf+3, 0, 4);
		OoWordToLongUnsigned(tmNow.nYear, bBuf+8);
		memcpy(bBuf+10, (BYTE*)&tmNow.nMonth,5);
		OoWriteAttr(0x2141, 0x02, bBuf);
	}
	iVal = OoDoubleLongUnsignedToDWord(&bBuf[3]);
	if ((iP > iVal) || IsAllAByte(bBuf+8, 0, 7))
	{
		OoDWordToDoubleLongUnsigned(iP, bBuf+3);
		OoWordToLongUnsigned(tmNow.nYear, bBuf+8);
		memcpy(bBuf+10, (BYTE*)&tmNow.nMonth,5);
		OoWriteAttr(0x2141, 0x02, bBuf);
	}
}

void CStatMgr::ResetStat(WORD wOI)
{
	BYTE i = (wOI & 0x000f);
	BYTE bBuf[50];
	BYTE j;

	memset(bBuf, 0, sizeof(bBuf));
	if ((wOI>=0x2200) && (wOI<=0x2204))
	{
		TTermStatInfo TermStatInfo;
		WaitSemaphore(m_semTermLog);
		
		//memset(&m_TermStatInfo, 0, sizeof(m_TermStatInfo));
		//GetCurTime(&m_TermStatInfo.tmLastRun);
		if (wOI==0x2200)
		{
			m_TermStatInfo.dwMonFlux = 0;//GPRS流量
			m_TermStatInfo.dwDayFlux = 0;
			memset(bBuf, 0, sizeof(bBuf));
			ReadItemEx(BN0, PN0, 0x2200, bBuf);
			memset(&bBuf[3], 0, 4);
			memset(&bBuf[8], 0, 4);
			WriteItemEx(BN0, PN0, 0x2200, bBuf);
		
		}
		if (wOI==0x2203)
		{
			memset(bBuf, 0, sizeof(bBuf));
			ReadItemEx(BN0, PN0, 0x2203, bBuf);	//0x2203  供电时间
			memset(&bBuf[3], 0, 4);
			memset(&bBuf[8], 0, 4);
			WriteItemEx(BN0, PN0, 0x2203, bBuf); 
			DTRACE(DB_DP, ("CStatMgr:Month Change:Term supply time %d .\r\n", m_TermStatInfo.wDayPowerTime/60));
			m_TermStatInfo.wDayPowerTime = 0;
			m_TermStatInfo.wMonPowerTime = 0;
		
		}
		if (wOI==0x2204)
		{
			ReadItemEx(BN0, PN0, 0x2204, bBuf);	//0x2204 2 复位次数,
			memset(&bBuf[3], 0, 2);
			memset(&bBuf[6], 0, 2);
			WriteItemEx(BN0, PN0, 0x2204, bBuf); 
			m_TermStatInfo.wDayRstStart = 0;
			m_TermStatInfo.wMonRstStart = 0;
		
		}
		TermStatInfo = m_TermStatInfo;
		
		SignalSemaphore(m_semTermLog);
	
#ifdef SYS_WIN
		WriteFile(TERM_STAT_PATHNAME, (BYTE* )&TermStatInfo, sizeof(TermStatInfo));
#else
#ifdef LOG_ENABLE
		m_DataLog.WriteLog((BYTE* )&TermStatInfo); 
#else
		WriteFile(TERM_STAT_PATHNAME, (BYTE* )&TermStatInfo, sizeof(TermStatInfo));
#endif
#endif //SYS_WIN

		return;
	}
	else
	{
		WaitSemaphore(m_semStat);
		if ((wOI & 0xfff0) == 0x2100)//区间
		{
			memset(m_IntvStatRes[i], 0, sizeof(TIntvStatRes)*STAT_OAD_NUM);
			SaveIntvStatRes(0x2108+i, RW_ATTR_RES, m_IntvStatRes[i], m_bIntvStatNum[i]);
			for (j=0; j<STAT_OAD_NUM; j++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				WriteItemEx(BN11, j, 0x0014+i, bBuf);
			}
		
		}
		else if ((wOI & 0xfff0) == 0x2110)//平均
		{
			memset(m_AvgStatRes[i], 0, sizeof(TAccAvgStatRes)*STAT_OAD_NUM);
			SaveAvgStatRes(0x2118+i, RW_ATTR_RES, m_AvgStatRes[i], m_bAvgStatNum[i]);
			for (j=0; j<STAT_OAD_NUM; j++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				WriteItemEx(BN11, j, 0x0019+i, bBuf);
			}
		}
		else if ((wOI & 0xfff0) == 0x2120)//极值
		{
			memset(m_ExtremStatRes[i], 0, sizeof(TExtremStatRes)*STAT_OAD_NUM);
			SaveExtremStatRes(0x2128+i, RW_ATTR_RES, m_ExtremStatRes[i], m_bExtremStatNum[i]);
			for (j=0; j<STAT_OAD_NUM; j++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				WriteItemEx(BN11, j, 0x001e + i, bBuf);
			}
		}
		else if ((wOI & 0xfff0) == 0x2130)//总电压合格率
		{
			memset((BYTE *)&m_PhaseVoltStat[i], 0, sizeof(TPhaseVoltStat));
			SavePhaseVoltStat(0x2130+i, RW_ATTR_RES, &m_PhaseVoltStat[i]);
		}
		else if ((wOI & 0xfff0) == 0x2140)//日最大有功功率及发生时间
		{
			OoWriteAttr(0x2140+i, 0x02, bBuf);
		}
		SignalSemaphore(m_semStat);
		
		TrigerSaveBank(BN11, 0, -1); //即时统计的起点数据
		TrigerSaveBank(BN0, 0, -1); 
	}
}

