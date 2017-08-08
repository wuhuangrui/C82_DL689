/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�StatMgr.cpp
 * ժ    Ҫ�����ļ���Ҫʵ���ն�ͳ����Ϣ������ͳ����Ĺ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��������ƽ
 * ������ڣ�2008��7��
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
		//�Ǽ򵥵��ϱ��澯����
		if ( m_pDataStat[i] != NULL)   
		{
			delete m_pDataStat[i] ;
			m_pDataStat[i]	=	NULL;
		}
	}*/
}

//����:�����ʼ��:�����ͨ����Ŀ��ƽṹ,��ʼ����ͨ����������,
//	   �ڲ��Ե�����޸ĺ���Ҫ���³�ʼ��
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

//����:��ȡtermInfo.dat �ļ����� ���½ṹm_termInfo
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
	if (!m_DataLog.Recover((BYTE* )&m_TermStatInfo))  //��־��¼��Ч
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
	//ReadItemEx(BN0, PN0, 0x2204, bBuf);	//0x2204  ��λ����,
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
	//ReadItemEx(BN0, PN0, 0x2203, bBuf);	//0x2203  ����ʱ��
	bBuf[0] =  0x02;
	bBuf[1] =  0x02;
	bBuf[2] =  0x06;
	bBuf[7] =  0x06;

	DWORD dwSec1 = TimeToSeconds(m_TermStatInfo.tmLastRun);
	DWORD dwSec2 = TimeToSeconds(tmNow);
	if ((dwSec2>=dwSec1) && (dwSec2<=dwSec1+150))//��Ϊ�����������ն���
	{
		m_TermStatInfo.wDayPowerTime += dwSec2-dwSec1;//����ǰm_TermStatInfo.tmLastRun����100�봦��,����ͣ���¼�������
		m_TermStatInfo.wMonPowerTime += dwSec2-dwSec1;
	}
	OoDWordToDoubleLongUnsigned(m_TermStatInfo.wDayPowerTime/60, (&bBuf[3]));
	OoDWordToDoubleLongUnsigned(m_TermStatInfo.wMonPowerTime/60, (&bBuf[8]));
	WriteItemEx(BN0, PN0, 0x2203, bBuf); 

	memset(bBuf, 0, sizeof(bBuf));
	//ReadItemEx(BN0, PN0, 0x2200, bBuf);	//GPRS����
	bBuf[0] =  0x02;
	bBuf[1] =  0x02;
	bBuf[2] =  0x06;
	bBuf[7] =  0x06;
	OoDWordToDoubleLongUnsigned(m_TermStatInfo.dwDayFlux, (&bBuf[3]));
	OoDWordToDoubleLongUnsigned(m_TermStatInfo.dwMonFlux, (&bBuf[8]));
	WriteItemEx(BN0, PN0, 0x2200, bBuf); 

	m_TermStatInfo.tmLastRun = tmNow;	//�������һ�ε�����ʱ��
	TrigerSaveBank(BN0, SECT_VARIABLE, -1);//��������һ��

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


//����:ÿ���ӱ����ն�ͳ�Ƶ���ʱ����
//��ע:������ʵ��Ӳ������Ҫʹ�����籣���ն�ͳ������,������ÿ�����������̱߳�����,
//	   ��DoTermStat()����ͬһ���̱߳�����,����Ҫ���ź�m_semTermLog���б���
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
		if (fDataInitF == true)//Ϊ���ݳ�ʼ���������ն����Ĵ���
		{
			BYTE bBuf[12];
			memset(bBuf, 0, sizeof(bBuf));
			//ReadItemEx(BN0, PN0, 0x2204, bBuf);	//0x2204  ��λ����,
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

	if (!m_fTermStatChg)	//���ݽṹ��DoTermStat()��û������,û��Ҫ����
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


//����:�ն�ͳ������
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
	
	WaitSemaphore(m_semTermLog);	//ͳ�Ƶ��������̶�Ҫ���б���,
								 	//��Ϊ��������λ��ʱ���ֱ�Ӱ�����m_TermStatInfo���
		//if (IsTimeEmpty(m_TermStatInfo.tmLastRun))	//��һ�����л���ͳ�����ݽṹ�������
	//{
	//�ⲿ�ַŵ���ʼ��ʱִ��
	//}
		
	if (DaysFrom2000(m_TermStatInfo.tmLastRun) != DaysFrom2000(tmNow))//���л�
	{	
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, PN0, 0x2204, bBuf);	//0x2204  ��λ����,
		memset(&bBuf[3], 0, 2);
		WriteItemEx(BN0, PN0, 0x2204, bBuf); 
		m_TermStatInfo.wDayRstStart = 0;

		DTRACE(DB_DP, ("CStatMgr:Day Change:Term supply time reset times:\r\n"));
		
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, PN0, 0x2203, bBuf);	//0x2203  ����ʱ��
		memset(&bBuf[3], 0, 4);
		WriteItemEx(BN0, PN0, 0x2203, bBuf); 
		m_TermStatInfo.wDayPowerTime = 0;
		


#ifdef PRO_698
		m_TermStatInfo.dwDayFlux = 0;//GPRS������
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, PN0, 0x2200, bBuf);
		memset(&bBuf[3], 0, 4);
		WriteItemEx(BN0, PN0, 0x2200, bBuf);
#endif

	}

	if (MonthFrom2000(m_TermStatInfo.tmLastRun) != MonthFrom2000(tmNow))//���л�
	{	
		ReadItemEx(BN0, PN0, 0x2204, bBuf);	//0x2204 2 ��λ����,HEX ֱ�Ӵ�BN2 PN0 0x1021�����ȡ
		memset(&bBuf[3], 0, 2);
		memset(&bBuf[6], 0, 2);
		WriteItemEx(BN0, PN0, 0x2204, bBuf); 
		m_TermStatInfo.wDayRstStart = 0;
		m_TermStatInfo.wMonRstStart = 0;
		
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, PN0, 0x2203, bBuf);	//0x2203  ����ʱ��
		memset(&bBuf[3], 0, 4);
		memset(&bBuf[8], 0, 4);
		WriteItemEx(BN0, PN0, 0x2203, bBuf); 
		DTRACE(DB_DP, ("CStatMgr:Month Change:Term supply time %d .\r\n", m_TermStatInfo.wDayPowerTime/60));
		m_TermStatInfo.wDayPowerTime = 0;
		m_TermStatInfo.wMonPowerTime = 0;

#ifdef PRO_698
		m_TermStatInfo.dwMonFlux = 0;//GPRS������
		m_TermStatInfo.dwDayFlux = 0;
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, PN0, 0x2200, bBuf);
		memset(&bBuf[3], 0, 4);
		memset(&bBuf[8], 0, 4);
		WriteItemEx(BN0, PN0, 0x2200, bBuf);
#endif
	}
	//if (dwCurSec < TimeToSeconds(m_TermStatInfo.tmLastRun))//����Уʱ��
	dwCurClick = GetClick();
	if (dwCurClick >=  m_dwStatClick)
		dwSec = dwCurClick - m_dwStatClick;
	else
		dwSec = 0;
	
	//dwCurSec - TimeToSeconds(m_TermStatInfo.tmLastRun);	//̨��У����ʱ�䲻�ܼ�������
	m_dwStatClick = dwCurClick;
	m_TermStatInfo.tmLastRun = tmNow;	//�������һ�ε�����ʱ��
	ReadItemEx(BN0, PN0, 0x2203, bBuf);	//0x2203  ����ʱ��
	m_TermStatInfo.wDayPowerTime += dwSec;//�����չ���ʱ��
	if (m_TermStatInfo.wDayPowerTime > 86400)// 1440
		m_TermStatInfo.wDayPowerTime = 86400;// 1440;
	
	m_TermStatInfo.wMonPowerTime += dwSec;//�����¹���ʱ��
	DWORD dwData = m_TermStatInfo.wDayPowerTime;
	if ((dwData % 60) > 30 /*47*/)//�ڶ���ʱ���ն˵ĸ�λʱ�䶼�������ڵ�2����ҲҪ��2��������
		dwData = dwData/60 + 1;//��̨��
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
	WaitSemaphore(m_semTermLog);			//ͳ�Ƶ��������̶�Ҫ���б���,
	m_TermStatInfo.dwDayFlux += dwLen;		//�ն�GPRS������
	m_TermStatInfo.dwMonFlux += dwLen;		//�ն�GPRS������

	memset(bBuf, 0, sizeof(bBuf));
	//ReadItemEx(BN0, PN0, 0x2200, bBuf);
	OoReadAttr(0x2200, 0x02, bBuf, NULL, NULL);
	OoDWordToDoubleLongUnsigned(m_TermStatInfo.dwDayFlux, &bBuf[3]);
	OoDWordToDoubleLongUnsigned(m_TermStatInfo.dwMonFlux, &bBuf[8]);
	//WriteItemEx(BN0, PN0, 0x2200, bBuf); //698 C1F10
	OoWriteAttr(0x2200, 0x02, bBuf);
	//m_fTermStatChg = true; //���ﲻ�ܴ���������ͨѶ��ʱ���Ƶ��д�ļ�

	SignalSemaphore(m_semTermLog); 
#endif
}

//������ͨ����:����Ч�Ĳ����������
bool CStatMgr::DoDataStat()
{
	int	i;
	TTime now;
	GetCurTime(&now);
	
	//���Ե�����ı�
	if (GetInfo(INFO_STAT_PARA))//�ǵ�����װ��һ�µ�ѹͳ�Ƶ���Щ����
	{
	//ĳһ���������ˣ���Ҫ��ʼ����
	//��Ӧ�������������ݿ�������ݡ�
		LoadVoltStatPara(&m_VoltPara);
		memset((BYTE *)&m_PhaseVoltStat[0], 0, sizeof(m_PhaseVoltStat));
		for (i=0; i<4; i++)
		{
			SavePhaseVoltStat(0x2130+i, RW_ATTR_RES, &m_PhaseVoltStat[i]);
		}

	//����Ӧͳ�Ʊ�������
		TrigerSaveBank(BN11, 0, -1); //��ʱͳ�Ƶ��������
		TrigerSaveBank(BN0, 0, -1); 
	}

	if (GetInfo(INFO_CLASS14_STAT_CHG))//�����������
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
		TrigerSaveBank(BN11, 0, -1); //��ʱͳ�Ƶ��������
		TrigerSaveBank(BN0, 0, -1); 
	}

	if (GetInfo(INFO_CLASS15_STAT_CHG))//ƽ����������
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
		TrigerSaveBank(BN11, 0, -1); //��ʱͳ�Ƶ��������
		TrigerSaveBank(BN0, 0, -1); 
	}

	if (GetInfo(INFO_CLASS16_STAT_CHG))//��ֵ��������
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
		TrigerSaveBank(BN11, 0, -1); //��ʱͳ�Ƶ��������
		TrigerSaveBank(BN0, 0, -1); 
	}

	CalcuAvgPower();
	if (now.nSecond != m_tmLastRun.nSecond)//ÿ��ˢ�����Ƿ��з��գ����ܷ���30��ˢһ��
	{
		m_tmLastRun = now;
		DoTermStat();//�ն˹���ʱ�䡢��λ������ͳ��
		WaitSemaphore(m_semStat);
		//����ͳ��
		IntvStat();
		//�ۼ�ƽ��ͳ��
		AccAvgStat();
		//��ֵͳ��
		ExtremStat();
		//��ѹ�ϸ���ͳ��
		DoVoltStat();
		//�����С����ͳ��
		DoPowerStat();
		SignalSemaphore(m_semStat);
//#ifdef SYS_WIN	
//		SaveTermStat();
//#endif
	}

	return true;
}



//�����������ѹͳ�����ݣ�����Э����м����ݣ����浽ϵͳ��
//������@wOI�����ѹͳ��OI
//		@bAttr Ҫд�������(0��ʾ�浱ǰID��1-��ʾ�涳��ID)
//		@pPhaseVoltStat�����ѹͳ������
void CStatMgr::SavePhaseVoltStat(WORD wOI, BYTE bAttr, TPhaseVoltStat* pPhaseVoltStat)
{
	BYTE bCalData[50] ;
	BYTE * P = bCalData;
	DWORD dwMin = 0;

	memset(bCalData, 0, sizeof(bCalData));
	*P++ = DT_STRUCT;
	*P++ = 0x02;
	//�պϸ���
	*P++ = DT_STRUCT;
	*P++ = 0x05;
	*P++ = DT_DB_LONG_U;
	dwMin = pPhaseVoltStat->dayStat.dwMoniSecs/60;
	if (pPhaseVoltStat->dayStat.dwMoniSecs%60 >= 55)//��4��ķ�����,���ϲ㺯���ı���һ��
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
	//�ºϸ���
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

	if (bAttr != RW_ATTR_FRZ)//�浱ǰ
	{
		OoWriteAttr(wOI, bAttr, bCalData);//����ϸ�������
		memset(bCalData, 0, sizeof(bCalData));
		OoDWordToDoubleLongUnsigned(pPhaseVoltStat->dayStat.dwQualSecs, bCalData);
		OoDWordToDoubleLongUnsigned(pPhaseVoltStat->monStat.dwQualSecs, bCalData+4);
		OoDWordToDoubleLongUnsigned(pPhaseVoltStat->dayStat.dwOverSecs, bCalData+8);
		OoDWordToDoubleLongUnsigned(pPhaseVoltStat->monStat.dwOverSecs, bCalData+12);
		WriteItemEx(BN11, PN0, 0x0010+(wOI-0x2130), bCalData);//����ϸ�ʱ��,����ǰ���ں󣬲�����ʽ�ֽڷ�
	}
	else//�涳��ID
	{
		WriteItemEx(BN11, PN0, 0x0032+(wOI-0x2130), bCalData);
	}
}

//��������ϵͳ����װ������ѹͳ������
//������@wOI�����ѹͳ��OI
//		@pPhaseVoltStat�����ѹͳ������
void CStatMgr::LoadPhaseVoltStat(WORD wOI, TPhaseVoltStat* pPhaseVoltStat)
{
	BYTE bCalData[50];
	memset(bCalData, 0, sizeof(bCalData));
	OoReadAttr(wOI, 0x02, bCalData, NULL, NULL);
	BYTE * P = bCalData;
	//���յ�ѹ�ϸ���
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
	//���µ�ѹ�ϸ���
	pPhaseVoltStat->monStat.dwMoniSecs = OoDoubleLongUnsignedToDWord(P)*60;
	P += 5;
	pPhaseVoltStat->monStat.wQualRate = OoLongUnsignedToWord(P);
	P += 3;
	pPhaseVoltStat->monStat.wOverRate = OoLongUnsignedToWord(P);
	P += 3;
	pPhaseVoltStat->monStat.dwUpperSecs = OoDoubleLongUnsignedToDWord(P)*60;
	P += 5;
	pPhaseVoltStat->monStat.dwLowerSecs = OoDoubleLongUnsignedToDWord(P)*60;
	//�ϸ�ʱ��
	ReadItemEx(BN11, PN0, 0x0010+(wOI-0x2130), bCalData);
	pPhaseVoltStat->dayStat.dwQualSecs = OoDoubleLongUnsignedToDWord(bCalData);
	pPhaseVoltStat->monStat.dwQualSecs = OoDoubleLongUnsignedToDWord(bCalData+4);
	pPhaseVoltStat->dayStat.dwOverSecs = OoDoubleLongUnsignedToDWord(bCalData+8);
	pPhaseVoltStat->monStat.dwOverSecs = OoDoubleLongUnsignedToDWord(bCalData+12);

}

//��������ϵͳ����װ���ѹͳ�Ʋ���
//������@pPara��ѹ�ϸ��ʲ���
void CStatMgr::LoadVoltStatPara (TVoltStatPara* pPara)
{
	BYTE bPara[16] = {0};
	OoReadAttr(0x4030,0x02,  bPara, NULL, NULL);
	pPara->wAssesUpLimit = OoLongUnsignedToWord(bPara+3);
	pPara->wAssesLowLimit = OoLongUnsignedToWord(bPara+6);
	pPara->wQualUpLimit = OoLongUnsignedToWord(bPara+9);
	pPara->wQualLowLimit = OoLongUnsignedToWord(bPara+12);
}


//����:��/�µ�ѹ�ϸ���ͳ������ C2F27(0x012f)   C2F35(0x019f)
//����:@piData		ָ�����������������ָ��,
//	   @iLen		������ĸ���
//	   @dwCurMin	��ǰʱ�䣬��λ ����
//	   @dwIntervM	���
//����:��
//��ע:		
//void  CDpStat::DoVoltStat(int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType)
void  CStatMgr::DoVoltStat(void)
{
	int	i =	0;
	int	iRet = 0;

//̨���������ܲ���<�������ʱ�����ʱ������ʱ��� ͳ�Ƶ� �ϸ��ۼ�ʱ����>-----------------------------------------------------------
	TTime tmNow;
	static DWORD dwLastSec = 0;
	static TTime tmLast;
	DWORD dwSetSec = 0;//�����ʱ�������ʱ���
	DWORD dwMonSetSec = 0;
	bool fMonOver = false;
	bool fDayOver = false;
	DWORD dwCurSec = GetCurTime();
	BYTE bBuf3[8] = {0};

	//memset(bCalData, 0, sizeof(bCalData));
	//if ((dwCurSec/60) == (dwLastSec/60))
	GetCurTime(&tmNow);
	//if (MinutesFrom2000(tmLast) == MinutesFrom2000(tmNow))//ÿ����ִ��һ��
	//	return;
	
	if (dwLastSec == 0)
	{
		dwLastSec = dwCurSec;
		tmLast = tmNow;
	}
	DWORD wIntvSec = dwCurSec - dwLastSec;//ʱ����Ϊ��,���ʱת���ɷ�
	fDayOver	= false;
	fMonOver= false;
//------------------------------------------------------------------------
	int iVal[3];
	BYTE k;

	memset(iVal, 0, sizeof(iVal));
	int iLen = OoReadVal(0x20000200, iVal, sizeof(iVal)/sizeof(int));//��ȡԭʼ����
	if (iLen <= 0)
		return ;

	BYTE bPhaseFlg[4] = {0};
	if (IsAllAByte((BYTE *)&m_VoltPara, 0, sizeof(TVoltStatPara)))
		return;//�жϲ�����Ч
	//��/�µ�ѹ����ͳ��, ��ͳ�Ʒ�����ͳ����
	for (i=1; i<4; i++)
	{
		//Խ����
		if (iVal[i-1]>=m_VoltPara.wQualUpLimit)//wAssesUpLimit
		{			
			m_PhaseVoltStat[i].dayStat.dwUpperSecs+= wIntvSec;//��ʱ���ۼ�
			m_PhaseVoltStat[i].monStat.dwUpperSecs+= wIntvSec;//��ʱ���ۼ�
			bPhaseFlg[i] = 1;
		}
		else if(iVal[i-1]<=m_VoltPara.wQualLowLimit)//Խ����wAssesLowLimit
		{
			m_PhaseVoltStat[i].dayStat.dwLowerSecs += wIntvSec;
			m_PhaseVoltStat[i].monStat.dwLowerSecs += wIntvSec;
			bPhaseFlg[i] = 2;
		}
		if ((iVal[i-1]<=m_VoltPara.wQualUpLimit) && (iVal[i-1]>=m_VoltPara.wQualLowLimit))//�ںϸ�������
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
//ͳ���ܵ�ѹ��
	if ((bPhaseFlg[1]==0) && (bPhaseFlg[2]==0) && (bPhaseFlg[3]==0))//�ϸ�
	{//���඼�ϸ�ʱ���ܵĲ���Ϊ�ϸ�
		m_PhaseVoltStat[0].dayStat.dwQualSecs += wIntvSec;
		m_PhaseVoltStat[0].monStat.dwQualSecs += wIntvSec;
	}
	else
	{
		if (bPhaseFlg[1]==1 || bPhaseFlg[2]==1 || bPhaseFlg[3]==1)//������
		{
			m_PhaseVoltStat[0].dayStat.dwUpperSecs += wIntvSec;//��ʱ���ۼ�
			m_PhaseVoltStat[0].monStat.dwUpperSecs += wIntvSec; //��ʱ���ۼ�
		}
		//else	//������
		if (bPhaseFlg[1]==2 || bPhaseFlg[2]==2 || bPhaseFlg[3]==2)
		{
			m_PhaseVoltStat[0].dayStat.dwLowerSecs += wIntvSec;
			m_PhaseVoltStat[0].monStat.dwLowerSecs += wIntvSec; 
		}
		m_PhaseVoltStat[0].dayStat.dwOverSecs += wIntvSec;
		m_PhaseVoltStat[0].monStat.dwOverSecs += wIntvSec;
	}
	//�ۼƸ��Ե��ܼ��ʱ�䲢����ϸ����볬����
	DWORD dwQualSecs;
	DWORD dwMoniSecs;
	DWORD dwOverSecs;
	for (i=0; i<4; i++)
	{
		m_PhaseVoltStat[i].dayStat.dwMoniSecs += wIntvSec;
		m_PhaseVoltStat[i].monStat.dwMoniSecs += wIntvSec;//�µ�
		if (m_PhaseVoltStat[i].dayStat.dwMoniSecs != 0)	
		{
			if ((m_PhaseVoltStat[i].dayStat.dwMoniSecs/60) != 0)	
			{
				dwQualSecs = m_PhaseVoltStat[i].dayStat.dwQualSecs/60;
				dwMoniSecs = m_PhaseVoltStat[i].dayStat.dwMoniSecs/60;
				dwOverSecs = m_PhaseVoltStat[i].dayStat.dwOverSecs/60;
				if (m_PhaseVoltStat[i].dayStat.dwQualSecs%60 >= 55)//��4��ķ�����
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
				if (m_PhaseVoltStat[i].monStat.dwQualSecs%60 >= 49)//��4��ķ�����
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
			//ת�涳�ᣬȻ���嵱ǰ����
			SavePhaseVoltStat(0x2130+i, RW_ATTR_FRZ, &m_PhaseVoltStat[i]);
		}

		memset(m_PhaseVoltStat, 0, sizeof(TPhaseVoltStat)*4);//��\�ºϸ�������
		wIntvSec = 0;
		DTRACE(DB_FAPROTO, ("DoVoltStat:Mon Change\r\n"));//DB_DP
		for (i=0; i<4; i++)
		{
			SavePhaseVoltStat(0x2130+i, RW_ATTR_RES, &m_PhaseVoltStat[i]);
		}


		//char szBuf[200];
		//sprintf(szBuf, "ת������Ϊ: ");
		//TraceBuf(DB_FAPROTO, szBuf, (BYTE*)&m_PhaseVoltStat[0], 4*sizeof(TPhaseVoltStat));
		
	}
	else if (!IsSameDay(tmNow, tmLast))//������
	{
		fDayOver = true;
		//ת�涳�ᣬȻ���嵱ǰ����
		for (i=0; i<4; i++)
		{
			//ת�涳�ᣬȻ���嵱ǰ����
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
	//�Ȳ����Ƕ�ʱ����
	tmLast = tmNow;  
	dwLastSec = dwCurSec;

}

//����ͳ��
void CStatMgr::IntvStat()
{
	BYTE i, j, k;
	WORD wLen = 0;
	TTime tmLastCycleTime;
	BYTE bBuf[16];
	TTime tmNow;
	TIntvStatRela * pRela = NULL;//ͳ�Ʋ���
	TIntvStatRes * pRes = NULL;//ͳ�ƽ��
	GetCurTime(&tmNow);
	
	for (i=0; i<5; i++)//�֡�ʱ���ա��¡���
	{
		pRela = m_IntvStatRela[i];
		pRes = m_IntvStatRes[i];
		
		if (m_bIntvStatNum[i] == 0)
			continue;
		if (m_bIntvStatNum[i]  > STAT_OAD_NUM)
			m_bIntvStatNum[i] = STAT_OAD_NUM;
		
		for (j=0; j<m_bIntvStatNum[i] ; j++)//�������������Ա����
		{
			DWORD dwIntVSec;
			BYTE bCycleSwitch[STAT_OAD_NUM];

			if (pRela[j].dwOAD == 0 || pRela[j].wParaNum == 0)
				continue;
			
			memset(bCycleSwitch, 0, STAT_OAD_NUM);
			pRes[j].dwOAD = pRela[j].dwOAD;
			pRes[j].wIntvNum = pRela[j].wParaNum+1;
			
			ReadItemEx(BN11, j, 0x0014+i, bBuf);//��������Ƶ�ʵļ�¼ʱ��
			TStatExeCtrl exeCtrl;
			memcpy((BYTE *)&exeCtrl, bBuf, sizeof(TStatExeCtrl));
			BYTE bMoreCycle = 0;
			//�ж������Ƿ�,�絽������������л�����Ҫ������������ͳ��
			if (IsCycleSwitch(exeCtrl.tmCurCycle, tmNow, i+1, pRela[j].bCycleValue, &dwIntVSec, &bMoreCycle))
			{
				//��ʱ��Ҫ���ж������ݵ�ת��,
				SaveIntvStatRes(0x0023+i, RW_ATTR_FRZ, pRes, m_bIntvStatNum[i]);
				
				//memset(pRes[j].intvVal, 0, sizeof(pRes[j].intvVal));//���ۼ�ʱ��ʹ���
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
				memset(pRes[j].intvVal, 0, sizeof(pRes[j].intvVal));//���ۼ�ʱ��ʹ���
				SaveIntvStatRes(0x2108+i, RW_ATTR_RES, pRes, m_bIntvStatNum[i]);
				exeCtrl.tmLastSample = tmNow;
				memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
				WriteItemEx(BN11, j, 0x0014+i, bBuf);
				continue;
			}

			//�ж�ͳ��Ƶ��ʱ���Ƿ�
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
			int iLen = OoReadVal(pRela[j].dwOAD, iVal, sizeof(iVal)/sizeof(int));//��ȡԭʼ����
			if (iLen <= 0)
			{
				continue;
			}
			DWORD dwbaksec;
			//�±߽��������ж�ͳ��
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
			if (k >= pRela[j].wParaNum)//��ʱ��Ϊ��ͳ�����ݴ�������Ǹ���Χ����
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

//�������ж��Ƿ��ѹ���һ�������ļ���ˣ�
//������@tmLastCycle ��һ�ε�ִ��ʱ��
//		@ tmNow��ǰʱ��
//		@bUnit�������(�֡�ʱ���ա��¡���)
//		@ wValue	���ֵ
//		@ dwIntVSec	���ؼ��ʱ�������
//		@bMoreCycle �Ƿ��ѹ��������ʱ��
//����:����������ֵ������򷵻�true,�������򷵻�false
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
		if (TimeToSeconds(tmNow)>TimeToSeconds(tmLastCycle))//����Уʱ
			*dwIntVSec = TimeToSeconds(tmNow)-TimeToSeconds(tmLastCycle);
		else
			*dwIntVSec = 0;
	}
	return flage;
}





//������������ͳ�ƽ�����и�ʽת�������浽ϵͳ��
//������@wOI �֡�ʱ���ա��¡�������ͳ��OI
//		@bAttr Ҫд������ԣ���ǰ����һ���ڣ�
//		@pRes��ʱ�����������ָ��
//		@wStatNum �֡�ʱ���ա��¡���ͳ�Ƶ�OAD�ĸ���
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

//��������ϵͳ����װ������ͳ�ƹ�������
//������@wOI �֡�ʱ���ա��¡�������ͳ��OI
//		@pRela����ͳ�ƹ�������
//		@pwStatNum�������ط֡�ʱ���ա��¡���ͳ�Ƶ�OAD�ĸ���
bool CStatMgr::LoadIntvStatRela(WORD wOI, TIntvStatRela* pRela, BYTE*pwStatNum)
{
	BYTE bData[3000];
	BYTE * p = bData;
	memset(bData, 0, sizeof(bData));
	if (OoReadAttr(wOI, 0x03, bData, NULL, NULL) < 0)
		return false;
	p ++;//������������
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
			bLen = OoGetDataVal(p, &pRela[j].iParaVal[k]);//�ȳ¹��Ǳ���������ӿ�
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

//��������ϵͳ����װ������ͳ������
//������@wOI �֡�ʱ���ա��¡�������ͳ��OI
//		@pRes����ͳ������
void CStatMgr::LoadIntvStatRes(WORD wOI, TIntvStatRes* pRes, BYTE wStatNum)
{
	BYTE bData[2830];
	BYTE * p = bData;
	BYTE k;
	memset(bData, 0, sizeof(bData));
	if (OoReadAttr(wOI, 0x02, bData, NULL, NULL) < 0)
		return;
	p += 2;////��������
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

//�ۼ�ƽ��ͳ��
void CStatMgr::AccAvgStat()
{
	BYTE i, j, k;
	WORD bTimes = 0;
	WORD wLen = 0;
	TTime tmLastCycleTime;
	BYTE bBuf[16];
	TTime tmNow;
	TStatRela * pRela = NULL;//ͳ�Ʋ���
	TAccAvgStatRes * pRes = NULL;//ͳ�ƽ��
	GetCurTime(&tmNow);
	for (i=0; i<5; i++)//�֡�ʱ���ա��¡���
	{
		pRela = m_AvgStatRela[i];
		pRes = m_AvgStatRes[i];
		
		if (m_bAvgStatNum[i] == 0)
			continue;
		if (m_bIntvStatNum[i]  > STAT_OAD_NUM)
			m_bIntvStatNum[i] = STAT_OAD_NUM;

		for (j=0; j<m_bAvgStatNum[i] ; j++)//�������������Ա����
		{
			DWORD dwIntVSec;
			if (pRela[j].dwOAD == 0)// || pRela[j].wParaNum == 0)
				continue;
			pRes[j].dwOAD = pRela[j].dwOAD;
			ReadItemEx(BN11, j, 0x003b + i, (BYTE *)&bTimes);//������
			ReadItemEx(BN11, j, 0x0019+i, bBuf);//��������Ƶ�ʵļ�¼ʱ��
			TStatExeCtrl exeCtrl;
			memcpy((BYTE *)&exeCtrl, bBuf, sizeof(TStatExeCtrl));
			BYTE bMoreCycle = 0;
			//�ж������Ƿ�,�絽������������л�����Ҫ������������ͳ��
			if (IsCycleSwitch(exeCtrl.tmCurCycle, tmNow, i+1, pRela[j].bCycleValue, &dwIntVSec, &bMoreCycle))
			{
				//��ʱ��Ҫ���ж������ݵ�ת��,
				SaveAvgStatRes(0x0028+i, RW_ATTR_FRZ, pRes, m_bAvgStatNum[i]);
				
				pRes[j].iAcc = 0;//��ͳ�ƽ��
				pRes[j].iAvg = 0;
				SaveAvgStatRes(0x2118+i, RW_ATTR_RES, pRes, m_bAvgStatNum[i]);
				bTimes = 0;
				WriteItemEx(BN11, j, 0x003b + i, (BYTE *)&bTimes);//���ۼƴ���

				exeCtrl.tmCurCycle = tmNow;
				memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
				WriteItemEx(BN11, j, 0x0019+i, bBuf);
				DTRACE(DB_DP, ("AccAvgStat=%d   Cycle switch\r\n",i));
				
			}
			//�ж�ͳ��Ƶ��ʱ���Ƿ�
			if ( IsCycleSwitch(exeCtrl.tmLastSample, tmNow,pRela[j].tiFreq.bUnit, pRela[j].tiFreq.wVal, &dwIntVSec, &bMoreCycle) == false)
			{
				continue;
			}

			exeCtrl.tmLastSample = tmNow;
			memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
			WriteItemEx(BN11, j, 0x0019+i, bBuf);
			
			int iVal[5];
			memset(iVal, 0, sizeof(iVal));
			int iLen = OoReadVal(pRela[j].dwOAD, iVal, sizeof(iVal)/sizeof(int));//��ȡԭʼ����
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


//��������ƽ��ͳ�ƽ�����и�ʽת�������浽ϵͳ��
//������@wOI �֡�ʱ���ա��¡���ƽ��ͳ��OI
//		@bAttr Ҫд������ԣ���ǰ����һ���ڣ�
//		@pRes��ʱ�����������ָ��
//		@wStatNum �֡�ʱ���ա��¡���ͳ�Ƶ�OAD�ĸ���
//		@Type ����
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
				dwtOAD |= 0x0001;//��OoGetValType()����ȫ����ȡ������Ա����������
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
//��ֵ���ۼӶ�������һ������װ�ؽӿ�
//��������ϵͳ����װ��ƽ��ͳ�ƹ�������
//������@wOI �֡�ʱ���ա��¡���ƽ��ͳ��OI
//		@pRelaƽ��ͳ�ƹ�������
//		@pwStatNum�������ط֡�ʱ���ա��¡���ͳ�Ƶ�OAD�ĸ���
bool CStatMgr::LoadAvgStatRela(WORD wOI, TStatRela* pRela, BYTE*pwStatNum)
{
	BYTE bData[3000];
	BYTE * p = bData;
	memset(bData, 0, sizeof(bData));
	if (OoReadAttr(wOI, 0x03, bData, NULL, NULL) < 0)
		return false;
	*p ++;//������������
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

//��������ϵͳ����װ��ƽ��ͳ������
//������@wOI �֡�ʱ���ա��¡���ƽ��	ͳ��OI
//		@pResƽ��ͳ������
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
	if (p[0] != 0x02 && p[1] != 0x02)//������Ч
		return;
	 p += 2;//��������
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

//��ֵͳ��
void CStatMgr::ExtremStat()
{
	BYTE i, j, k;
	WORD wLen = 0;
	TTime tmLastCycleTime;
	BYTE bBuf[16];
	TStatRela * pRela = NULL;//ͳ�Ʋ���
	TExtremStatRes * pRes = NULL;//ͳ�ƽ��
	TTime tmNow;
	GetCurTime(&tmNow);
	for (i=0; i<5; i++)//�֡�ʱ���ա��¡���
	{
		pRela = m_ExtremStatRela[i];
		pRes = m_ExtremStatRes[i];
		
		if (m_bAvgStatNum[i] == 0)
			continue;
		for (j=0; j<m_bExtremStatNum[i] ; j++)//�������������Ա����
		{
			DWORD dwIntVSec;
			if (pRela[j].dwOAD == 0)// || pRela[j].wParaNum == 0)
				continue;
			pRes[j].dwOAD = pRela[j].dwOAD;
			ReadItemEx(BN11, j, 0x001e + i, bBuf);//��������Ƶ�ʵļ�¼ʱ��
			TStatExeCtrl exeCtrl;
			memcpy((BYTE *)&exeCtrl, bBuf, sizeof(TStatExeCtrl));
			BYTE bMoreCycle = 0;
			//�ж������Ƿ�,�絽������������л�����Ҫ������������ͳ��
			if (IsCycleSwitch(exeCtrl.tmCurCycle, tmNow, i+1, pRela[j].bCycleValue, &dwIntVSec, &bMoreCycle))
			{
				//��ʱ��Ҫ���ж������ݵ�ת��,
				SaveExtremStatRes(0x002d+i, RW_ATTR_FRZ, pRes, m_bExtremStatNum[i]);
				
				memset((BYTE *)&pRes[j].iMax, 0, sizeof(TExtremStatRes)-sizeof(DWORD));
				SaveExtremStatRes(0x2128+i, RW_ATTR_RES, pRes, m_bExtremStatNum[i]);

				exeCtrl.tmCurCycle = tmNow;
				memset(&exeCtrl.tmLastSample, 0, sizeof(TTime));
				memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
				WriteItemEx(BN11, j, 0x001e + i, bBuf);
				DTRACE(DB_DP, ("ExtremStat=%d   Cycle switch\r\n",i));
				
			}
			//�ж�ͳ��Ƶ��ʱ���Ƿ�
			if ( IsCycleSwitch(exeCtrl.tmLastSample, tmNow,pRela[j].tiFreq.bUnit, pRela[j].tiFreq.wVal, &dwIntVSec, &bMoreCycle) == false)
			{
				continue;
			}

			exeCtrl.tmLastSample = tmNow;
			memcpy(bBuf, (BYTE*)&exeCtrl, sizeof(exeCtrl));
			WriteItemEx(BN11, j, 0x001e + i, bBuf);
			
			int iVal[5];
			memset(iVal, 0, sizeof(iVal));
			int iLen = OoReadVal(pRela[j].dwOAD, iVal, sizeof(iVal)/sizeof(int));//��ȡԭʼ����
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

//�������Ѽ�ֵͳ�ƽ�����и�ʽת�������浽ϵͳ��
//������@wOI �֡�ʱ���ա��¡��꼫ֵͳ��OI
//		@bAttr Ҫд������ԣ���ǰ����һ���ڣ�
//		@pRes��ʱ�����������ָ��
//		@wStatNum �֡�ʱ���ա��¡���ͳ�Ƶ�OAD�ĸ���
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
				dwtOAD |= 0x0001;//��OoGetValType()����ȫ����ȡ������Ա����������
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

//��������ϵͳ����װ�뼫ֵͳ������
//������@wOI �֡�ʱ���ա��¡��꼫ֵͳ��OI
//		@pRes��ֵͳ������
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
	if (p[0] != 0x02 && p[1] != 0x02)//������Ч
		return;
	 p += 2;////��������
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


//װ������ͳ����Ĳ���
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

//װ������ͳ������
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
	int iP[4];     //�ֱ���A,B,C����
	int iQ[4];     //�ֱ���A,B,C����
	int iS[4];   	 //�ֱ���A,B,C����
	BYTE bBuf[32];
	
	//��¼����
	m_bPowrIndexCnt++;
	if(m_bPowrIndexCnt>=POWER_AVG_NUM_MAX)
	{
		m_bPowrIndexCnt = POWER_AVG_NUM_MAX;
	}

	//��ȡʵʱ����ֵ
	memset(iP, 0, sizeof(iP));
	OoReadVal(0x20040200, iP, 4);
	memset(iQ, 0, sizeof(iQ));
	OoReadVal(0x20050200, iQ, 4);
	memset(iS, 0, sizeof(iS));
	OoReadVal(0x20060200, iS, 4);

	
	//����ʵʱ��
	m_bPowrIndex %= POWER_AVG_NUM_MAX;

	for(i=0;i<4;i++)//total/a/b/c/
	{
		m_iPower[i+0][m_bPowrIndex] = iP[i];
		m_iPower[i+4][m_bPowrIndex] = iQ[i];
		m_iPower[i+8][m_bPowrIndex] = iS[i];
	}		
	m_bPowrIndex++;
	
	
	//����ƽ������
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
	if (wYear != tmNow.nYear||bBuf[10] != tmNow.nMonth||bBuf[11] != tmNow.nDay)//���л�
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
	if (wYear != tmNow.nYear||bBuf[10] != tmNow.nMonth)//���л�
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
			m_TermStatInfo.dwMonFlux = 0;//GPRS����
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
			ReadItemEx(BN0, PN0, 0x2203, bBuf);	//0x2203  ����ʱ��
			memset(&bBuf[3], 0, 4);
			memset(&bBuf[8], 0, 4);
			WriteItemEx(BN0, PN0, 0x2203, bBuf); 
			DTRACE(DB_DP, ("CStatMgr:Month Change:Term supply time %d .\r\n", m_TermStatInfo.wDayPowerTime/60));
			m_TermStatInfo.wDayPowerTime = 0;
			m_TermStatInfo.wMonPowerTime = 0;
		
		}
		if (wOI==0x2204)
		{
			ReadItemEx(BN0, PN0, 0x2204, bBuf);	//0x2204 2 ��λ����,
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
		if ((wOI & 0xfff0) == 0x2100)//����
		{
			memset(m_IntvStatRes[i], 0, sizeof(TIntvStatRes)*STAT_OAD_NUM);
			SaveIntvStatRes(0x2108+i, RW_ATTR_RES, m_IntvStatRes[i], m_bIntvStatNum[i]);
			for (j=0; j<STAT_OAD_NUM; j++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				WriteItemEx(BN11, j, 0x0014+i, bBuf);
			}
		
		}
		else if ((wOI & 0xfff0) == 0x2110)//ƽ��
		{
			memset(m_AvgStatRes[i], 0, sizeof(TAccAvgStatRes)*STAT_OAD_NUM);
			SaveAvgStatRes(0x2118+i, RW_ATTR_RES, m_AvgStatRes[i], m_bAvgStatNum[i]);
			for (j=0; j<STAT_OAD_NUM; j++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				WriteItemEx(BN11, j, 0x0019+i, bBuf);
			}
		}
		else if ((wOI & 0xfff0) == 0x2120)//��ֵ
		{
			memset(m_ExtremStatRes[i], 0, sizeof(TExtremStatRes)*STAT_OAD_NUM);
			SaveExtremStatRes(0x2128+i, RW_ATTR_RES, m_ExtremStatRes[i], m_bExtremStatNum[i]);
			for (j=0; j<STAT_OAD_NUM; j++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				WriteItemEx(BN11, j, 0x001e + i, bBuf);
			}
		}
		else if ((wOI & 0xfff0) == 0x2130)//�ܵ�ѹ�ϸ���
		{
			memset((BYTE *)&m_PhaseVoltStat[i], 0, sizeof(TPhaseVoltStat));
			SavePhaseVoltStat(0x2130+i, RW_ATTR_RES, &m_PhaseVoltStat[i]);
		}
		else if ((wOI & 0xfff0) == 0x2140)//������й����ʼ�����ʱ��
		{
			OoWriteAttr(0x2140+i, 0x02, bBuf);
		}
		SignalSemaphore(m_semStat);
		
		TrigerSaveBank(BN11, 0, -1); //��ʱͳ�Ƶ��������
		TrigerSaveBank(BN0, 0, -1); 
	}
}

