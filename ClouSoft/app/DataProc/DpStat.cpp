/****************************************************************************************************
* Copyright (c) 2007,深圳科陆电子科技股份有限公司
* All rights reserved.
* 
* 文件名称：DpStat.cpp
* 摘    要: 本文件实现2类数据统计类
* 当前版本：1.0
* 作    者：杨 凡、李湘平
* 完成日期：2007年8月
* 备    注：
****************************************************************************************************/
#include "stdafx.h"
#include "FaCfg.h"
#include "DpStat.h"
#include "FaStruct.h"
#include "DbConst.h"
#include "DbAPI.h"
#include "Trace.h"
#include <math.h>
#include <stdio.h>
#include "MeterAPI.h"
#include "DbStruct.h"
//#include "DbGbAPI.h"
#include "LibAcConst.h"
#include "FaAPI.h"

#pragma   warning(disable:4309 4805 4018)


//统计的ID
#define STAT_P			0	
#define STAT_DEMAND		1
#define STAT_U			2
#define STAT_UNBALANCE	3
#define STAT_I			4
#define STAT_S			5
#define STAT_COS		6
#define STAT_HARMONIC	7
#define STAT_DC			8
#define STAT_LOAD		9//负载率统计

#define DAY				0
#define MONTH			1
#define MIN_VAL			(-0x7fffffff)
#define MIN_VAL64		(-0x7fffffffffffffff)
#define MAX_VAL			(0x7fffffff)
#define MAX_VAL64		(0x7fffffffffffffff)

//tll 先定义一下，让编译通过
#define MTR_PRIO_POS	5
//抄表需求的优先级，共5档
#define MTR_PRIO_HIGHEST	(0<<MTR_PRIO_POS)	//最高
#define MTR_PRIO_HIGH		(1<<MTR_PRIO_POS)	//高
#define MTR_PRIO_MID		(2<<MTR_PRIO_POS)	//中
#define MTR_PRIO_LOW		(3<<MTR_PRIO_POS)	//低
#define MTR_PRIO_LOWEST		(4<<MTR_PRIO_POS)	//最低
#define MTR_PRIO_GB			(5<<MTR_PRIO_POS)	//垃圾级别
#define MTR_PRIO_DIR		(7<<MTR_PRIO_POS)	//直抄表


//////////////////////////////////////////////////////////////////////////


//谐波的的数据F57 F58的数据个数为109,113，所有操作在同一个线程，因此用全局变量保存数据效率比较高
int g_iDpDataArray[128*2]={0};
//暂存0点抄表有效的极值的起点时间及起点值
int g_iZeroStartVal[128]={0};

//////////////////////////////////////////////////////////////////////////////////////////
//CDpStat

CDpStat::CDpStat()
{
	m_fFisrtDoLoad = true;
	m_wMidTimes = 0;
	m_wUnbITimes = 0;
	m_wUnbUTimes = 0;
	m_dwMonUnbUTimes = 0;
	m_dwMonUnbITimes = 0;
}

CDpStat::~CDpStat()
{

}

//描述:初始化函数
void  CDpStat::Init(BYTE  bPn)
{}


//描述:装载参数
void  CDpStat::LoadLimitPara(BYTE bLimitType)
{}
#if 0
//描述:参数初始化函数
inline void CDpStat::SchedParaInit(TSchedPara * pSchedPara, TStatCtrl *pStatCtrl, BYTE bPn)
{
	//时钟间隔单位
	pSchedPara->bIntervU		=		(BYTE)pStatCtrl->wInterU;
	//间隔
	pSchedPara->wIntervV		=		pStatCtrl->wInterV;			

	//开始抄表的日时分
	pSchedPara->nStartDay		=		0;							
	pSchedPara->nStartHour		=		0;
	pSchedPara->nStartMinute	=		0;
	
	//优先级
	pSchedPara->bPrio			=		pStatCtrl->bPrio;   

	pSchedPara->wReqBn 			=		m_wBn;				//需求ID的BANK号,只用到单测量点提交需求
	pSchedPara->wReqPn			=		bPn;    

	//按照单测量点提交需求
	pSchedPara->pwReqID			=		pStatCtrl->wReqId;  

	//需要ID的个数
	if (g_fTestMode && IsMtrPn(m_bPn) && pStatCtrl->bStatID==STAT_I)	//电科院台体不支持b6a0
		pSchedPara->wReqNum			=		1;
	else
		pSchedPara->wReqNum			=		pStatCtrl->wReqIdNum;

	//按照多测量点提交需求
	pSchedPara->pReqItem		=		NULL; 
	if (m_wBn == BN0)
		pSchedPara->fNoSubmit	=		false;		//485测量点要提交抄表需求
	else
		pSchedPara->fNoSubmit	=		true;		//载波表不用提交抄表需求，只需要查询即可

	pSchedPara->fNoQuery		=		false;				//为true表示不用查询抄表需求是否到达,只需要提交即可,主要针对实时数据的定期更新,减少系统查询消耗
}
#endif
//描述:电表参数更新的时候需要调用的过程，涉及到电表参数
void  CDpStat::DoMtrParaChg()
{}

//描述:抄表间隔发生变化的时候需要调用此过程来更新调度器的抄表间隔，非抄表间隔donothing
//参数:@bMtrIntervV C1F24抄表间隔
//返回:无
//备注:无
void CDpStat::SetMtrInterv(WORD wInterv)
{	
	for (WORD i=0; i < STAT_NUM; i++)
	{		
		if (m_bStatValid[i] & 0x01)
		{
			wInterv = GetStatIntervV(i);
			//m_MeterSched[i].SetInterv(wInterv);
			//抄表间隔发生变化时，重新记录统计的起始时间
			m_dwStatPriM[i] = GetCurMinute();
		}
	}

	return;
}


//描述:数据统计总函数
void CDpStat::DoDataStat()
{
//	DoVoltStat();
	

}

BYTE CDpStat::GetStatIdFromBN11(WORD wBn11ID)
{
	return 0;
}

void CDpStat::AddStatTime(WORD wBn11ID, int* piValBuf)
{
	int iInx = -1;
	const BYTE* pbFmt;
	BYTE bNum=0;
	BYTE bFrzTyp;
	TDataItem di;
	TItemDesc* pItemDesc;

	if (wBn11ID==0x010f || wBn11ID==0x012f || wBn11ID==0x013f || wBn11ID==0x014f || wBn11ID==0x015f || wBn11ID==0x020f)
	{
		bFrzTyp = ADDONS_DAYFRZ;
	}
	else if (wBn11ID==0x017f || wBn11ID==0x019f || wBn11ID==0x01af || wBn11ID==0x01bf || wBn11ID==0x01cf || wBn11ID==0x021f)
	{
		bFrzTyp = ADDONS_MONFRZ;
	}
	else
		return;

	di = GetItemEx(BN11, m_bPn, wBn11ID);

	iInx = BinarySearchIndex(di.pBankCtrl->pItemDesc, di.pBankCtrl->dwItemNum, wBn11ID);

	pItemDesc = di.pBankCtrl->pItemDesc;

	if (iInx > 0)
	{
		if (pItemDesc[iInx].pbFmtStr != NULL)
		{		
			bNum = 0;
			pbFmt = pItemDesc[iInx].pbFmtStr;

			while (*pbFmt != 0xff)
			{
				if (*pbFmt==FMT_BIN && piValBuf[bNum]>0) //累计时间之类的加2分钟
				{
					piValBuf[bNum] += 2;
					if (bFrzTyp == ADDONS_DAYFRZ)
					{
						if (piValBuf[bNum] > 1440)
							piValBuf[bNum] = 1440;
					}
					else if (bFrzTyp == ADDONS_MONFRZ)
					{
						if (piValBuf[bNum] > 1440*31)
							piValBuf[bNum] = 1440*31;
					}
				}

				pbFmt += 2; //下一项格式描述

				bNum ++;
			}		
		}
	}
}

void CDpStat::SetZeroStartTime(WORD wBn11ID, int* piValBuf, DWORD dwSec)
{
	int iInx = 0;
	const BYTE* pbFmt;
	BYTE bNum=0, n=0;
	TDataItem di;
	TItemDesc* pItemDesc;

	di = GetItemEx(BN11, m_bPn, wBn11ID);

	iInx = BinarySearchIndex(di.pBankCtrl->pItemDesc, di.pBankCtrl->dwItemNum, wBn11ID);

	pItemDesc = di.pBankCtrl->pItemDesc;

	if (iInx > 0)
	{
		if (pItemDesc[iInx].pbFmtStr != NULL)
		{		
			bNum = 0;
			pbFmt = pItemDesc[iInx].pbFmtStr;
			
			while (*pbFmt != 0xff)
			{						
				if (*pbFmt==FMT18 || *pbFmt==FMT17) //时刻更新
				{
					MinToFmt(dwSec/60, (BYTE*)&piValBuf[bNum], *pbFmt);
				}				
				else if (*pbFmt == FMT_BIN) //累计时间之类的清零
				{					
					piValBuf[bNum] = 0;
				}				
				else //其他极值数据均保留0点时刻的瞬时值	
				{
					if (g_iZeroStartVal[0]==m_bPn && g_iZeroStartVal[1]==GetStatIdFromBN11(wBn11ID) && g_iZeroStartVal[2]==GetStartM(dwSec/60, DAY))
					{			
					
						piValBuf[bNum] = g_iZeroStartVal[3+n];
						n ++;
						
						if (n > 124) //异常情况
							return;
					}
					else 
						piValBuf[bNum] = 0;
				}
				
				pbFmt += 2; //下一项格式描述

				bNum ++;
			}		
		}
	}
}

//描述:把BANK11里面的数据写到BANK21里面并带上当前的时标，同时清空BANK11对应的ID数据
void CDpStat::SwapBank21(BYTE bPn, WORD wBnOldID, WORD wBnNewID, DWORD dwOldSec)
{
	int iBuf[128]		=	{0};
	int iLen			=	 0;
	DWORD dwSec = GetCurTime();

	if (wBnOldID > 0)
	{
		iLen = ReadItemVal(BN0, bPn, wBnOldID, iBuf);
		iLen = WriteItemVal(BN21,  bPn, wBnNewID,  iBuf, dwOldSec);

		TraceBuf(DB_DP, "###########################SwapBank21-> ", (BYTE*)iBuf, iLen*4);
	}
}

//描述:把BANK11里面的数据写到BANK0里面并带上当前的时标，同时清空BANK11对应的ID数据
void CDpStat::SwapBank32( BYTE bPn, WORD wBn11ID, WORD wBn0ID, DWORD dwOldSec )
{
	int iBuf[128]		=	{0};
	int iZeroData[128]	=	{0};
	int iLen			=	 0;
	DWORD dwSec = GetCurTime();

	if ( wBn0ID > 0 )
	{
		iLen = ReadItemVal(	BN11, bPn, wBn11ID, iBuf);		
		if (!IsPnType(bPn, PN_TYPE_AC))
			AddStatTime(wBn11ID, iBuf);

		//如果是谐波统计未越限，则库里未写过，缺省值含谐波次数均为0
		if ((wBn11ID==0x03bf || wBn11ID==0x03cf || wBn11ID==0x03df) && iBuf[0]==0)
			iBuf[0] = HARMONIC_NUM;

		iLen = WriteItemVal(BN0,  bPn, wBn0ID,  iBuf, dwOldSec);// 

		DTRACE(DB_DP, ("SwapBank32->: bPn=%d, wBn11ID = %02x, wBn0ID = %02x, dwOldSec=%ld\r\n",bPn, wBn11ID, wBn0ID, dwOldSec));
		TraceBuf(DB_DP, "SwapBank32-> ", (BYTE*)iBuf, iLen*4);

	}	

	if ( m_fZeroValid ) //如果数据有效则写有效数据
	{	
		memcpy((BYTE*)iZeroData, (BYTE*)iBuf, sizeof(iZeroData));
		SetZeroStartTime(wBn11ID, iZeroData, dwSec);
	}
	//else 数据无效时间也清零

	//清零是不正确的，比如写最小值的时候，为0的话默认值，比较后，有可能取不到真正的最小值
	WriteItemVal( BN11, bPn, wBn11ID, iZeroData, dwSec);
}


//描述:把BANK11里面的数据写到BANK0里面并带上当前的时标,同时清空BANK11对应的ID数据
void CDpStat::SwapBank64( BYTE bPn, WORD wBn11ID, WORD wBn0ID, DWORD dwOldSec )
{
	int64	iBuf[128]		=	{0};
	int64	iZeroData[128]	=	{0};
	int		iLen			=	0;

	if (wBn0ID)
	{
		iLen = ReadItemVal64( BN11, bPn, wBn11ID, iBuf);		
		iLen = WriteItemVal64(BN0,  bPn, wBn0ID,  iBuf, dwOldSec);


#ifdef SYS_WIN
		TraceBuf(DB_DP, "SwapBank64-> ", (BYTE* )iBuf, iLen*8);
#endif
	}

	WriteItemVal64(BN11, bPn, wBn11ID, iZeroData, GetCurTime());

}

//描述:日变更 把BANK11对应的ID 映射到BANK0
void CDpStat::DayChange(BYTE bStatID, DWORD dwOldDay)
{		
	dwOldDay++;

	//写入当前时刻nSec秒（日切换时刻）的数据，做普通任务的时候，调度器检测 nSec秒之后有没有数据到达，
	//有则马上读取nSec,nSec+120之间的数据并写入任务数据库里面，
	//写入任务数据库库的时候，nSec/(24*60*60) -1 的天数，也就是前一天的天数
	//为了保证数据能够百分之百读取到，加上一个30秒的余量
	DWORD dwSec = dwOldDay*86400;

	switch(bStatID)
	{	
	case STAT_P:
		SwapBank32(m_bPn, 0x010f, 0x308f, dwSec);		//F25:日最大功率及发生时间,有功功率为零时间统计
		SwapBank21(m_bPn, 0x308f, 0x308f, dwSec);		//F25:日最大功率及发生时间,有功功率为零时间统计
		break;
		
	case STAT_DEMAND:
		SwapBank32(m_bPn, 0x011f, 0x309f, dwSec);		//F26:日最大需量及发生时间
		SwapBank21(m_bPn, 0x309f, 0x309f, dwSec);		//F26:日最大需量及发生时间
		break;
		
	case STAT_U:
		SwapBank32(m_bPn, 0x012f, 0x30af, dwSec);		//F27:日电压统计数据
		SwapBank21(m_bPn, 0x30af, 0x30af, dwSec);		//F27:日电压统计数据
//		SwapBank64(m_bPn, 0x00df, 0x0, dwSec);
		break;
		
	case STAT_UNBALANCE:
		SwapBank32(m_bPn, 0x013f, 0x30bf, dwSec);		//F28:日不平衡度统计
		SwapBank21(m_bPn, 0x30bf, 0x30bf, dwSec);		//F28:日不平衡度统计
		if (GetPnProp(m_bPn) == PN_PROP_AC)//只统计交采
			DoUnbalanceTransfer(TIME_UNIT_DAY,dwSec);
		break;
		
	case STAT_I:
		SwapBank32(m_bPn, 0x014f, 0x30cf, dwSec);		//F29:日电流越限统计
		SwapBank21(m_bPn, 0x30cf, 0x30cf, dwSec);		//F29:日电流越限统计
		break;
		
	case STAT_S:
		SwapBank32(m_bPn, 0x015f, 0x30df, dwSec);		//F30:日视在功率越限累计时间
		SwapBank21(m_bPn, 0x30df, 0x30df, dwSec);		//F30:日视在功率越限累计时间
		break;
	
	case STAT_COS:
		SwapBank32(m_bPn, 0x020f, 0x318f, dwSec);		//F43:日功率因数区段累计时间
		SwapBank21(m_bPn, 0x318f, 0x318f, dwSec);		//F43:日功率因数区段累计时间
		break;
	
	case STAT_HARMONIC:
		SwapBank32(m_bPn, 0x035F, 0x32Ef, dwSec);		//F113
		SwapBank32(m_bPn, 0x036F, 0x330f, dwSec);		//F114
		SwapBank32(m_bPn, 0x037F, 0x331f, dwSec);		//F115
		SwapBank32(m_bPn, 0x038F, 0x332f, dwSec);		//F116
		SwapBank32(m_bPn, 0x039F, 0x333f, dwSec);		//F117
		SwapBank32(m_bPn, 0x03aF, 0x334f, dwSec);		//F118
		//0x03bf,0x03cf,0x03df ->0x335f,0x336f,0x337f
		SwapBank32(m_bPn, 0x03bf, 0x335f, dwSec);		//F121
		SwapBank32(m_bPn, 0x03cf, 0x336f, dwSec);		//F122
		SwapBank32(m_bPn, 0x03df, 0x337f, dwSec);		//F123
		if (GetPnProp(m_bPn) == PN_PROP_AC)//只统计交采
			DoHamornicTransfer(dwSec);
		break;

	case STAT_DC:
		SwapBank32(m_bPn, 0x03ef, 0x338F, dwSec);		//F129	
		break;	

	case STAT_LOAD:
		SwapBank32(m_bPn, 0x0a0f, 0x342f, dwSec);		//F31：日负载率统计
		SwapBank21(m_bPn, 0x342f, 0x342f, dwSec);		//F31：日负载率统计
		break;

	default:
		return;
	}	

	//置日数据清除的标志位
	//SetClearFlag(bStatID,true,DAY);	
}


void CDpStat::MonthChange(BYTE bStatID, DWORD dwOldMon)
{	
	TTime tmMon;
	MonthsToTime(dwOldMon, &tmMon);

	if (tmMon.nMonth == 12)
	{
		tmMon.nYear++;
		tmMon.nMonth = 1;
	}
	else
		tmMon.nMonth++;

	DWORD dwSec = TimeToSeconds(tmMon);

	switch(bStatID)
	{	
	case STAT_P:
		SwapBank32(m_bPn, 0x017f, 0x310f, dwSec);		//F33:月最大功率及发生时间,有功功率为零时间统计
		SwapBank21(m_bPn, 0x310f, 0x310f, dwSec);		//F33:月最大功率及发生时间,有功功率为零时间统计
		break;
		
	case STAT_DEMAND:
		SwapBank32(m_bPn, 0x018f, 0x311f, dwSec);		//F34:月最大需量及发生时间
		SwapBank21(m_bPn, 0x311f, 0x311f, dwSec);		//F34:月最大需量及发生时间
		break;
		
	case STAT_U:
		SwapBank32(m_bPn, 0x019f, 0x312f, dwSec);		//F35:月电压统计数据
		SwapBank21(m_bPn, 0x312f, 0x312f, dwSec);		//F35:月电压统计数据
//		SwapBank64(m_bPn, 0x00ef, 0x0, dwSec);
		break;
		
	case STAT_UNBALANCE:
		SwapBank32(m_bPn, 0x01af, 0x313f, dwSec);		//F36:月不平衡度统计
		SwapBank21(m_bPn, 0x313f, 0x313f, dwSec);		//F36:月不平衡度统计
		if (GetPnProp(m_bPn) == PN_PROP_AC)//只统计交采
			DoUnbalanceTransfer(TIME_UNIT_MONTH,dwSec);
		break;
		
	case STAT_I:
		SwapBank32(m_bPn, 0x01bf, 0x314f, dwSec);		//F37:月电流越限统计
		SwapBank21(m_bPn, 0x314f, 0x314f, dwSec);		//F37:月电流越限统计
		break;
		
	case STAT_S:
		SwapBank32(m_bPn, 0x01cf, 0x315f, dwSec);		//F38:月视在功率越限累计时间
		SwapBank21(m_bPn, 0x315f, 0x315f, dwSec);		//F38:月视在功率越限累计时间
		break;
	
	case STAT_COS:
		SwapBank32(m_bPn, 0x021f, 0x319f, dwSec);		//F44:月功率因数区段累计时间
		break;
	
	//case STAT_HARMONIC:		
	//	break;

	case STAT_DC:
		SwapBank32(m_bPn, 0x040f, 0x339F, dwSec);		//F130直流模拟量越限月累计时间，最大/最小值以及发生时间	
		break;	

	case STAT_LOAD:
		SwapBank32(m_bPn, 0x0a1f, 0x343f, dwSec);		//F39：月负载率统计
		SwapBank21(m_bPn, 0x343f, 0x343f, dwSec);		//F39：月负载率统计
		break;

	default:
		return;
	}		
	
	//置月数据清除的标志位
	//SetClearFlag(bStatID,true,MONTH);
	
}

//描述：检查某时刻之后的数据是否有效
bool CDpStat::CheckDataIsVaild(BYTE bStatID, DWORD dwSec)
{		
	bool fRet = false;
	int iVal32[130];

	switch(bStatID)
	{	
	case STAT_P:
		if (ReadItemVal(BN11, m_bPn, 0x010f, iVal32, dwSec) > 0)		//F25:日最大功率及发生时间,有功功率为零时间统计
			fRet = true;
		break;
		
	case STAT_DEMAND:
		if (ReadItemVal(BN11, m_bPn, 0x011f, iVal32, dwSec) > 0)		//F26:日最大需量及发生时间
			fRet = true;
		break;
		
	case STAT_U:
		if (ReadItemVal(BN11, m_bPn, 0x012f, iVal32, dwSec) > 0		//F27:日电压统计数据
			&& ReadItemVal64(BN11, m_bPn, 0x00df, (int64*)iVal32, dwSec) > 0)
			fRet = true;
		break;
		
	case STAT_UNBALANCE:
		if (ReadItemVal(BN11, m_bPn, 0x013f, iVal32, dwSec) > 0)		//F28:日不平衡度统计
			fRet = true;
		break;
		
	case STAT_I:
		if (ReadItemVal(BN11, m_bPn, 0x014f, iVal32, dwSec) > 0)		//F29:日电流越限统计
			fRet = true;
		break;
		
	case STAT_S:
		if (ReadItemVal(BN11, m_bPn, 0x015f, iVal32, dwSec) > 0)		//F30:日视在功率越限累计时间
			fRet = true;
		break;
	
	case STAT_COS:
		if (ReadItemVal(BN11, m_bPn, 0x020f, iVal32, dwSec) > 0)		//F43:日功率因数区段累计时间
			fRet = true;
		break;
	
	case STAT_HARMONIC:
		if (ReadItemVal(BN11, m_bPn, 0x035F, iVal32, dwSec)>0		//F113
		&& ReadItemVal(BN11, m_bPn, 0x036F, iVal32, dwSec)>0		//F114
		&& ReadItemVal(BN11, m_bPn, 0x037F, iVal32, dwSec)>0		//F115
		&& ReadItemVal(BN11, m_bPn, 0x038F, iVal32, dwSec)>0		//F116
		&& ReadItemVal(BN11, m_bPn, 0x039F, iVal32, dwSec)>0		//F117
		&& ReadItemVal(BN11, m_bPn, 0x03aF, iVal32, dwSec)>0		//F118		
		&& ReadItemVal(BN11, m_bPn, 0x03bf, iVal32, dwSec)>0		//F121
		&& ReadItemVal(BN11, m_bPn, 0x03cf, iVal32, dwSec)>0		//F122
		&& ReadItemVal(BN11, m_bPn, 0x03df, iVal32, dwSec)>0)		//F123
			fRet = true;
		break;

	case STAT_DC:
		if (ReadItemVal(BN11, m_bPn, 0x03ef, iVal32, dwSec) > 0)	//F129	
			fRet = true;
		break;	

	case STAT_LOAD:
		if (ReadItemVal(BN11, m_bPn, 0x0a0f, iVal32, dwSec) > 0)		//F31：日负载率统计
			fRet = true;
		break;

	default:
		return fRet;
	}	

	return fRet;
}

DWORD CDpStat::GetStartM(DWORD dwCurTimeM, int iType)
{	
	//if(dwCurTimeM%1440 == 0)
	//	dwCurTimeM--;
	
	if (iType == DAY)
	{		
		return (dwCurTimeM/1440)*1440;
	}
	else if ( iType == MONTH )
	{
		TTime tmStart;
		SecondsToTime(dwCurTimeM*60, &tmStart);	
		tmStart.nDay		= 1;
		tmStart.nHour		= 0;
		tmStart.nMinute		= 0;
		tmStart.nSecond		= 0;		

		return TimeToMinutes(tmStart);
	}

	return dwCurTimeM;
}


DWORD CDpStat::GetEndM(DWORD dwCurTimeM, int iType)
{
	//if(dwCurTimeM%1440 == 0)
	//	dwCurTimeM--;

	if ( iType == DAY )
	{		
		return ((dwCurTimeM+1440)/1440)*1440;
	}
	else if ( iType == MONTH ) 
	{
		TTime tmEnd;
		SecondsToTime(dwCurTimeM*60, &tmEnd);
		if(tmEnd.nMonth == 12)
		{
			tmEnd.nYear++;
			tmEnd.nMonth = 1;
		}
		else
			tmEnd.nMonth++;
		tmEnd.nDay = 1;
		tmEnd.nHour = 0;
		tmEnd.nMinute = 0;
		tmEnd.nSecond = 0;

		return TimeToMinutes(tmEnd);

	}

	return dwCurTimeM;
}


/* //tll 不用了的
void CDpStat::DoSubStat(BYTE bStatID, int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType)
{
	//DTRACE(DB_DP, ("CDpStat::DoSubStat() m_bPn=%d, bStatID=%d! dwCurMin=%d\r\n", m_bPn, bStatID, dwCurMin));
	switch (bStatID)
	{
	case STAT_P:
		DoPowerStat(piData, iLen, dwCurMin, dwIntervM,wNeedToPnType);
		break;
		
	case STAT_DEMAND:
		DoDemandStat(piData, iLen, dwCurMin, dwIntervM,wNeedToPnType);
		break;
		
	case STAT_U:
		DoVoltStat();
		break;
		
	case STAT_UNBALANCE:
		DoUnbalanceStat(piData,iLen, dwCurMin,dwIntervM,wNeedToPnType);
		break;
		
	case STAT_I:
		DoCurrentStat(piData,iLen, dwCurMin,dwIntervM,wNeedToPnType);
		break;
		
	case STAT_S:
		DoAppPowerStat(piData,iLen, dwCurMin,dwIntervM,wNeedToPnType);
		break;
	
	case STAT_COS:
		DoCosStat(piData,iLen, dwCurMin,dwIntervM,wNeedToPnType);
		break;
	
	case STAT_HARMONIC:
		DoHarmonicStat(piData,iLen, dwCurMin,dwIntervM,wNeedToPnType);
		break;

	case STAT_DC:
		DoDcStat(piData,iLen, dwCurMin,dwIntervM,wNeedToPnType);
		break;
		
	case STAT_LOAD:
		DoLoadStat(piData, iLen, dwCurMin,dwIntervM, wNeedToPnType);
		break;

	default:
		DTRACE(DB_DP, ("CDpStat::DoSubStat error bStatID=%d \n", bStatID));
		break;
	}
}
*/

//描述:日/月总及分相最大有功功率、发生时间，有功功率为0时间,C2F25(0x010f)  C2F33(0x017f)
//		对于测量点为脉冲属性，只计算最大功率和为零功率，相位功率填写无效数据
//参数:@piData		指向需求数据项数组的指针,
//	   @iLen		数据项的个数
//	   @dwCurMin	当前时间，单位 分钟
//	   @dwIntervM	间隔
//
//返回:无
//备注:当以值的方式读数据库时，没有对相应的时间字段格式进行值转换
void  CDpStat::DoPowerStat( int *piData, int iLen, DWORD dwCurMin, DWORD dwIntervM,WORD wNeedToPnType)
{
	int		i	=	0;
	if (iLen == 0)
		return;

	int		iCurDayPwr[12]	=		{0};
	int		iCurMonPwr[12]	=		{0};
	DWORD	dwDayStartS		=		60*GetStartM(dwCurMin, DAY );
	DWORD	dwDayEndS		=		60*GetEndM(	dwCurMin, DAY );
	DWORD	dwMonStartS		=		60*GetStartM(dwCurMin, MONTH );
	DWORD	dwMonEndS		=		60*GetEndM(	dwCurMin, MONTH );
	BYTE	bMaskProp		=		(BYTE)((BYTE)((1 << m_bPnProp) & wNeedToPnType));
 	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, DAY)) //日零点，即日切换
	{
		dwDayEndS = dwDayStartS;
		dwDayStartS -= 24*60*60;

		//暂存0点时刻的瞬时数据作为起点数据
		g_iZeroStartVal[0] = m_bPn;
		g_iZeroStartVal[1] = STAT_P;
		g_iZeroStartVal[2] = GetStartM(dwCurMin, DAY);
		for( i=0; i<4; i++ )
		{
			//功率统计最大值最小值 用绝对值统计		
			if (piData[i] == INVALID_VAL)
				continue;	
			piData[i] = abs(piData[i]);
		}
		memcpy((BYTE*)&g_iZeroStartVal[3], (BYTE*)piData, 4*sizeof(int));//0xb63f

	}
	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, MONTH)) //月零点，即月切换
	{
		TTime tm;
		dwMonEndS = dwMonStartS;
		dwMonStartS -= 24*60*60;//上月最后一天的秒数
		SecondsToTime(dwMonStartS, &tm);
		dwMonStartS = GetMonthStart(tm);
		
	}

	bool	fFlag1			=		0;
	bool	fFlag2			=		0;	

	if ( 0 >= ReadItemVal( BN11, m_bPn, 0x010f, iCurDayPwr, dwDayStartS, dwDayEndS)
		|| IsClearData(STAT_P) )
	{
		memset((BYTE*)iCurDayPwr, 0, sizeof(iCurDayPwr));
		//SetArrVal32(iCurDayPwr, INVALID_VAL, 8);//初始值需为无效		
		SetClearFlag(STAT_P,true,DAY);
	}

	if( 0 >= ReadItemVal( BN11, m_bPn, 0x017f, iCurMonPwr, dwMonStartS, dwMonEndS)
		|| IsClearData(STAT_P,MONTH))
	{
		memset((BYTE*)iCurMonPwr, 0, sizeof(iCurMonPwr));
		//SetArrVal32(iCurMonPwr, INVALID_VAL, 8);//初始值需为无效
		SetClearFlag(STAT_P,true,MONTH);
	}

	for( i = 0; i<4; i++ )
	{
		//功率统计最大值最小值 用绝对值统计		
		if (piData[i]==INVALID_VAL || (GetConnectType(m_bPn)==CONNECT_3P3W && i==2))
	    {
	        iCurDayPwr[2*i+1] = 0;
	        iCurMonPwr[1 + 2*i] = 0;
	        continue;
	    }

		piData[i] = abs(piData[i]);

		//日最大功率统计
		if( piData[i]>iCurDayPwr[2*i] || IsClearData(STAT_P,DAY))
		{
			iCurDayPwr[2*i] = piData[i];

			MinToFmt( dwCurMin, (BYTE *)&iCurDayPwr[2*i+1], FMT18 );

			fFlag1 = true;
		}
		if(piData[i] == 0)
		{
			//总及分相有功功率为零时间		
			iCurDayPwr[8 + i] += dwIntervM; 
			fFlag1 = true;
		}
		
		//月最大功率统计
		if( piData[i] > iCurMonPwr[2*i] || IsClearData(STAT_P,MONTH))
		{
			iCurMonPwr[2*i] = piData[i];
			MinToFmt( dwCurMin, (BYTE *)&iCurMonPwr[1 + 2*i], FMT18 );
			fFlag2 = true;
		}
		if( piData[i] == 0)
		{			
			//总及分相有功功率为零时间
			iCurMonPwr[8 + i] += dwIntervM; 			
			fFlag2 = true;
		}

		//对于脉冲表 ，不统计uvw三相的最大功率 全部填无效		
		if( (bMaskProp == PN_TYPE_PULSE && i > 0) || bMaskProp == 0 )			
		{
			//填充无效数据
			iCurDayPwr[2*i] = INVALID_VAL;
			iCurDayPwr[2*i+1] = INVALID_VAL;
			iCurDayPwr[8 + i] = INVALID_VAL;
			
			iCurMonPwr[2*i] = INVALID_VAL;
			iCurMonPwr[2*i+1] = INVALID_VAL;
			iCurMonPwr[8 + i] = INVALID_VAL;

		}		
	}
	
	if( fFlag1 )
		WriteItemVal( BN11, m_bPn, 0x010f, iCurDayPwr, 60*dwCurMin );

	if( fFlag2 )
		WriteItemVal( BN11, m_bPn, 0x017f, iCurMonPwr, 60*dwCurMin );

	//同时清除日月置零标志位
	SetClearFlag(STAT_P,false);

}


//描述:日/月总及分相有功最大需量及发生时间,C2F26(0x011f)  C2F34(0x018f)
//参数:@piData		指向需求数据项数组的指针,
//	   @iLen		数据项的个数
//	   @dwCurMin	当前时间，单位 分钟
//	   @dwIntervM	间隔
//返回:无
//备注:		
void  CDpStat::DoDemandStat( int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType )
{
	if (iLen == 0)
		return;

	int		i					=		0;
	int		iCurDayDemond[8]	=		{0};
	int		iCurMonDemond[8]	=		{0};
	DWORD	dwDayStartS			=		60*GetStartM(dwCurMin, DAY );
	DWORD	dwDayEndS			=		60*GetEndM(	dwCurMin, DAY );
	DWORD	dwMonStartS			=		60*GetStartM(dwCurMin, MONTH );
	DWORD	dwMonEndS			=		60*GetEndM(	dwCurMin, MONTH );
	BYTE	bMaskProp			=		(BYTE)((1 << m_bPnProp) & wNeedToPnType);

	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, DAY)) //日零点，即日切换
	{
		dwDayEndS = dwDayStartS;
		dwDayStartS -= 24*60*60;

		//暂存0点时刻的瞬时数据作为起点数据
		g_iZeroStartVal[0] = m_bPn;
		g_iZeroStartVal[1] = STAT_DEMAND;
		g_iZeroStartVal[2] = GetStartM(dwCurMin, DAY);		
		for( i=0; i<4; i++ )
		{
			//需量统计最大值最小值 用绝对值统计
			if (piData[i] == INVALID_VAL)
				continue;	
			piData[i] = abs(piData[i]);
		}
		memcpy((BYTE*)&g_iZeroStartVal[3], (BYTE*)piData, 4*sizeof(int));//0xb63f
	}
	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, MONTH)) //月零点，即月切换
	{
		TTime tm;
		dwMonEndS = dwMonStartS;
		dwMonStartS -= 24*60*60;//上月最后一天的秒数
		SecondsToTime(dwMonStartS, &tm);
		dwMonStartS = GetMonthStart(tm);
	}

	bool	fFlag1				=		0;
	bool	fFlag2				=		0;

	if ( 0 >= ReadItemVal( BN11, m_bPn, 0x011f, iCurDayDemond, dwDayStartS, dwDayEndS)
		|| IsClearData(STAT_DEMAND))
	{
		memset((BYTE*)iCurDayDemond, 0, sizeof(iCurDayDemond));
		//SetArrVal32(iCurDayDemond, INVALID_VAL, sizeof(iCurDayDemond)/sizeof(int));//初始值需为无效	
		SetClearFlag(STAT_DEMAND,true,DAY);
	}

	if ( 0 >= ReadItemVal( BN11, m_bPn, 0x018f, iCurMonDemond, dwMonStartS, dwMonEndS)
		|| IsClearData(STAT_DEMAND,MONTH))
	{
		memset((BYTE*)iCurMonDemond, 0, sizeof(iCurMonDemond));
		//SetArrVal32(iCurMonDemond, INVALID_VAL, sizeof(iCurMonDemond)/sizeof(int));//初始值需为无效	
		SetClearFlag(STAT_DEMAND,true,MONTH);
	}

	for( i=0; i<4; i++ )
	{
		//需量统计最大值最小值 用绝对值统计
		if (piData[i]==INVALID_VAL || (GetConnectType(m_bPn)==CONNECT_3P3W && i==2))
	    {
	        iCurDayDemond[1 + 2*i] = 0;
	        iCurMonDemond[1 + 2*i] = 0;
	        continue;
	    }

		piData[i] = abs(piData[i]);

		//日最大有功需量统计
		if(piData[i] > iCurDayDemond[2*i] || IsClearData(STAT_DEMAND,DAY))
		{
			iCurDayDemond[2*i] = piData[i];
			MinToFmt(dwCurMin, (BYTE *)&iCurDayDemond[1 + 2*i], FMT18);
			fFlag1 = true ;
		}

		//月最大有功需量统计
		if(piData[i] > iCurMonDemond[2*i] || IsClearData(STAT_DEMAND,MONTH))
		{
			iCurMonDemond[2*i] = piData[i];
			MinToFmt(dwCurMin, (BYTE *)&iCurMonDemond[1 + 2*i], FMT18);
			fFlag2 = true;
		}

		//对于脉冲表 ，不统计uvw三相的最大需量 全部填无效		
		if( (bMaskProp == PN_TYPE_PULSE && i > 0) || bMaskProp == 0 )			
		{
			//填充无效数据
			iCurDayDemond[2*i] = INVALID_VAL;
			iCurDayDemond[2*i+1] = INVALID_VAL;			

			iCurMonDemond[2*i] = INVALID_VAL;
			iCurMonDemond[2*i+1] = INVALID_VAL;	
		}
	}
	
	if( fFlag1 == true )
		WriteItemVal(BN11, m_bPn, 0x011f, iCurDayDemond, 60*dwCurMin);

	if( fFlag2 == true )
		WriteItemVal(BN11, m_bPn, 0x018f, iCurMonDemond, 60*dwCurMin);

	//同时清除日月置零标志位
	SetClearFlag(STAT_DEMAND,false);

}

//描述:日/月电压合格率统计数据 C2F27(0x012f)   C2F35(0x019f)
//参数:@piData		指向需求数据项数组的指针,
//	   @iLen		数据项的个数
//	   @dwCurMin	当前时间，单位 分钟
//	   @dwIntervM	间隔
//返回:无
//备注:		
void  CDpStat::DoVoltStat(int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType)
{}


//描述:日/月不平衡度越限累计时间,C2F28(0x013f)   C2F36(0x01Af)
//参数:@piData		指向需求数据项数组的指针,
//	   @iLen		数据项的个数
//	   @dwCurMin	当前时间，单位 分钟
//	   @dwIntervM	间隔
void  CDpStat::DoUnbalanceStat(int *piData, int iLen, DWORD dwCurMin, DWORD dwIntervM, WORD wNeedToPnType)
{
	int iTmp	= INVALID_VAL;
	if ( iLen == 0 )
	{
		WriteItemVal( BN0, m_bPn, 0x12df, &iTmp, dwCurMin*60 );
		WriteItemVal( BN0, m_bPn, 0x12ef, &iTmp, dwCurMin*60 );
		return;
	}

	//日不平衡度累计
	int  iDayUnbTime[6]		=	{0};    
	//月不平衡度累计
	int  iMonUnbTime[6]		=	{0};    
	BYTE	bMaskProp		=	(BYTE)((BYTE)((1 << m_bPnProp) & wNeedToPnType));
	//此统计不支持pn测试点属性填无效数据
	if( bMaskProp == 0 )
	{
		SetArrVal32(iDayUnbTime,INVALID_VAL,2);
		SetArrVal32(iMonUnbTime,INVALID_VAL,2);

		WriteItemVal(BN11, m_bPn, 0x013f, iDayUnbTime, dwCurMin*60);
		WriteItemVal(BN11, m_bPn, 0x01Af, iMonUnbTime, dwCurMin*60);

		return;
	}


	//当前三相电压
	int    *piABCPhaseVolt	= piData;

	//当前三相电流
	int    *piABCPhaseCur	= piData + 128;	
	
	int		iABCPhaseVolt[2]  = {0};  	
	int     iABCPhaseCur[2]   = {0};
	int		iAvrVolt		=	0;
	int		iAvrCur			=	0;

	int		iUnbVolt		=	0;
	int		iUnbCur			=	0;
	int		iAVal			=	0;
	int		iBVal			=	0;
	int		iCVal			=	0;

//	BYTE bTmpBuf[148];
//	BYTE bMonTmpBuf[2236];
//	BYTE bDataBuf[2];
	WORD wTmpBuf[90];
	WORD wMonTmpBuf[1134];
	WORD wDataBuf;
	BYTE bTime[3];
	TTime time;
//	WORD *pwBuf;
//	WORD *pwNewData;

	DWORD	dwDayStartS		=	60*GetStartM(dwCurMin, DAY );
	DWORD	dwDayEndS		=	60*GetEndM(dwCurMin,	DAY );
	DWORD	dwMonStartS		=	60*GetStartM(dwCurMin, MONTH );
	DWORD	dwMonEndS		=	60*GetEndM(	dwCurMin,	MONTH );

	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, DAY)) //日零点，即日切换
	{
		dwDayEndS = dwDayStartS;
		dwDayStartS -= 24*60*60;
	}
	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, MONTH)) //月零点，即月切换
	{
		TTime tm;
		dwMonEndS = dwMonStartS;
		dwMonStartS -= 24*60*60;//上月最后一天的秒数
		SecondsToTime(dwMonStartS, &tm);
		dwMonStartS = GetMonthStart(tm);
	}

	if ( 0 >= ReadItemVal(BN11, m_bPn, 0x013f, iDayUnbTime,dwDayStartS,dwDayEndS) 
		|| IsClearData(STAT_UNBALANCE))
	{
		memset((BYTE*)iDayUnbTime, 0, sizeof(iDayUnbTime) );
		SetClearFlag(STAT_UNBALANCE,true,DAY);
	}

	if ( 0 >= ReadItemVal(BN11, m_bPn, 0x01Af, iMonUnbTime,dwMonStartS, dwMonEndS) 
		|| IsClearData(STAT_UNBALANCE,MONTH))
	{
		memset((BYTE*)iMonUnbTime, 0, sizeof(iMonUnbTime));
		SetClearFlag(STAT_UNBALANCE,true,MONTH);
	}
	
	bool fIVaild = true, fUVaild = true;

	if (piABCPhaseVolt[0]==INVALID_VAL || piABCPhaseVolt[1]==INVALID_VAL || piABCPhaseVolt[2]==INVALID_VAL)
		fUVaild = false;
	if (piABCPhaseCur[0]==INVALID_VAL || piABCPhaseCur[1]==INVALID_VAL || piABCPhaseCur[2]==INVALID_VAL)
		fIVaild = false;

	if ( fUVaild )
	{
		BYTE bCalcType = 0;
		ReadItemEx(BN24, PN0, 0x5009, &bCalcType);
		if((GetConnectType(m_bPn)==CONNECT_3P3W) && (GetAcPn()==m_bPn || bCalcType==1)) //台体测试判断电表的要算B相电压
		{
			iABCPhaseVolt[0] = piABCPhaseVolt[0];
			iABCPhaseVolt[1] = piABCPhaseVolt[2];
			iAvrVolt	=	GetAverage( iABCPhaseVolt, 2 );
			iAVal		=	GetUnbValue(iAvrVolt, piABCPhaseVolt[0]); 
			iCVal		=	GetUnbValue(iAvrVolt, piABCPhaseVolt[2]);
			iUnbVolt    =   (iAVal>iCVal)?iAVal:iCVal;
		}
		else
		{
			iAvrVolt	=	GetAverage( piABCPhaseVolt, 3 );
			//A相不平衡度(电压)
			iAVal		=	GetUnbValue(iAvrVolt, piABCPhaseVolt[0]); 
			//B相不平衡度
			iBVal		=	GetUnbValue(iAvrVolt, piABCPhaseVolt[1]);  
			//C相不平衡度
			iCVal		=	GetUnbValue(iAvrVolt, piABCPhaseVolt[2]);  

			//取最大值
			iUnbVolt =  (iAVal > iBVal)? ((iAVal>iCVal)?iAVal:iCVal) : ((iBVal > iCVal)? iBVal : iCVal); 	
		}

#ifdef PRO_698
		if (iUnbVolt > iDayUnbTime[4])
		{
			iDayUnbTime[4] = iUnbVolt;
			MinToFmt(dwCurMin, (BYTE *)&iDayUnbTime[5], FMT18);
		}
		if (iUnbVolt > iMonUnbTime[4])
		{
			iMonUnbTime[4] = iUnbVolt;
			MinToFmt(dwCurMin, (BYTE *)&iMonUnbTime[5], FMT17);
		}
#endif
	}
	else	
		iUnbVolt = 0;	

	iTmp = iUnbVolt;

	//写入数据库
	WriteItemVal(BN0, m_bPn, 0x12df, &iTmp, dwCurMin*60);

	piABCPhaseCur[0] = Abs(piABCPhaseCur[0]);
	piABCPhaseCur[1] = Abs(piABCPhaseCur[1]);
	piABCPhaseCur[2] = Abs(piABCPhaseCur[2]);
	if ( fIVaild )
	{
	    if(GetConnectType(m_bPn) == CONNECT_3P3W)
	    {
			iABCPhaseCur[0] = piABCPhaseCur[0];
			iABCPhaseCur[1] = piABCPhaseCur[2];
	        iAvrCur		=	GetAverage( iABCPhaseCur, 2);
	        //A相不平衡度(电流)
			iAVal		=	GetUnbValue(iAvrCur, piABCPhaseCur[0]); 
			//C相不平衡度
			iCVal		=	GetUnbValue(iAvrCur, piABCPhaseCur[2]);
			iUnbCur     =   (iAVal>iCVal)?iAVal:iCVal;
	        
	    }
	    else
	    {
			iAvrCur		=	GetAverage( piABCPhaseCur, 3 );
			//A相不平衡度(电流)
			iAVal		=	GetUnbValue(iAvrCur, piABCPhaseCur[0]); 
			//B相不平衡度
			iBVal		=	GetUnbValue(iAvrCur, piABCPhaseCur[1]);  
			//C相不平衡度
			iCVal		=	GetUnbValue(iAvrCur, piABCPhaseCur[2]);  

			//取最大值
			iUnbCur		=  (iAVal > iBVal)? ((iAVal>iCVal)?iAVal:iCVal) : ((iBVal > iCVal)? iBVal : iCVal);  		
		}

#ifdef PRO_698
		if (iUnbCur > iDayUnbTime[2])
		{
			iDayUnbTime[2] = iUnbCur;
			MinToFmt(dwCurMin, (BYTE *)&iDayUnbTime[3], FMT18);
		}
		if (iUnbCur > iMonUnbTime[2])
		{
			iMonUnbTime[2] = iUnbCur;
			MinToFmt(dwCurMin, (BYTE *)&iMonUnbTime[3], FMT17);
		}
#endif
	}
	else	
		iUnbCur = 0;
	iTmp		=	iUnbCur;

	//电流不平衡最大值写入数据库
	WriteItemVal(BN0, m_bPn, 0x12ef, &iTmp, dwCurMin*60); 

	//电流不平衡度越限
	if( iUnbCur >= m_iUnbalanceCurLimit && m_iUnbalanceCurLimit > 0 )   
	{		
		iDayUnbTime[0] += dwIntervM;
		iMonUnbTime[0] += dwIntervM;
	}

	//电压不平衡度越限发生时间
	if( iUnbVolt >= m_iUnbalanceVoltLimit && m_iUnbalanceVoltLimit > 0 )   
	{
		iDayUnbTime[1] += dwIntervM;
		iMonUnbTime[1] += dwIntervM;
	}

	WriteItemVal(BN11, m_bPn, 0x013f, iDayUnbTime, dwCurMin*60);
	WriteItemVal(BN11, m_bPn, 0x01Af, iMonUnbTime, dwCurMin*60);

	if(fIVaild && (GetPnProp(m_bPn)==PN_PROP_AC))//只统计交采PN_PROP_AC
	{
		GetCurTime(&time);
		TimeToFmt20(time, bTime);
		ReadItemEx(BANK11, PN0, 0x0e20, (BYTE *)wTmpBuf);
		if (memcmp(bTime, (BYTE *)wTmpBuf, 3) != 0)
		{
			//DoUnbalanceTransfer(TIME_UNIT_DAY, 0);// 清除BANK11的数据
			
			if (memcmp(bTime+1, (BYTE *)wTmpBuf+1, 2) != 0)
			{
				//DoUnbalanceTransfer(TIME_UNIT_MONTH, 0);
			}
			
			WriteItemEx(BANK11, PN0, 0x0e20, bTime);
		}
		

		memset(wTmpBuf, 0, sizeof(wTmpBuf));
		ReadItemEx(BANK11, PN0, 0x0e01, (BYTE *)wTmpBuf);//日电流不平衡度95%概率值
		DWORDToBCD(iUnbCur, (BYTE *)&wDataBuf,2);//
		DWORDToBCD(m_wUnbUTimes+1, (BYTE *)&wTmpBuf[0],2);//
		//bTmpBuf[2 + m_wUnbUTimes*2] = bDataBuf[0];
		//bTmpBuf[3 + m_wUnbUTimes*2] = bDataBuf[1];
		//pwBuf = (WORD *)&bTmpBuf[2];
		//pwNewData = (WORD *)&bDataBuf[0];
		MakeSort(wTmpBuf+1, wDataBuf, 72+1);
		WriteItemEx(BANK11, PN0, 0x0e01, (BYTE *)wTmpBuf);

		memset(wTmpBuf, 0, sizeof(wTmpBuf));
		ReadItemEx(BANK11, PN0, 0x0e02, (BYTE *)wTmpBuf);//日电压不平衡度95%概率值
		
		DWORDToBCD(iUnbVolt, (BYTE *)&wDataBuf, 2);//
		DWORDToBCD(m_wUnbUTimes+1, (BYTE *)&wTmpBuf[0],2);//
		//bTmpBuf[2 + m_wUnbUTimes*2] = bDataBuf[0];
		//bTmpBuf[3 + m_wUnbUTimes*2] = bDataBuf[1];
		//pwBuf = (WORD *)&bTmpBuf[2];
		//pwNewData = (WORD *)&bDataBuf[0];
		MakeSort(wTmpBuf+1, wDataBuf, 72+1);
		WriteItemEx(BANK11, PN0, 0x0e02, (BYTE *)wTmpBuf);
		m_wUnbUTimes++;
		if (m_wUnbUTimes > 1440)
		{
			m_wUnbUTimes = 1440;
		}

		if ((m_dwMonUnbUTimes&1)==0 && (m_dwMonUnbUTimes<22320))//只记录偶数次
		{
			memset(wMonTmpBuf, 0, sizeof(wMonTmpBuf));
			ReadItemEx(BANK11, PN0, 0x0e11, (BYTE *)wMonTmpBuf);//月电流不平衡度95%概率值
			DWORDToBCD(iUnbCur, (BYTE *)&wDataBuf, 2);//
			DWORDToBCD(m_dwMonUnbUTimes+1, (BYTE *)&wMonTmpBuf[0],2);//
			//bMonTmpBuf[4 + m_dwMonUnbUTimes*2] = bDataBuf[0];
			//bMonTmpBuf[5 + m_dwMonUnbUTimes*2] = bDataBuf[1];
			//pwBuf = (WORD *)&bMonTmpBuf[2];
			//pwNewData = (WORD *)&bDataBuf[0];
			MakeSort(wMonTmpBuf+1, wDataBuf, 1116+1);
			WriteItemEx(BANK11, PN0, 0x0e11, (BYTE *)wMonTmpBuf);

			memset(wMonTmpBuf, 0, sizeof(wMonTmpBuf));
			ReadItemEx(BANK11, PN0, 0x0e12, (BYTE *)wMonTmpBuf);//月电压不平衡度95%概率值
			DWORDToBCD(iUnbVolt, (BYTE *)&wDataBuf, 2);//
			DWORDToBCD(m_dwMonUnbUTimes+1, (BYTE *)&wMonTmpBuf[0],2);//
			//bMonTmpBuf[4 + m_dwMonUnbUTimes*2] = bDataBuf[0];
			//bMonTmpBuf[5 + m_dwMonUnbUTimes*2] = bDataBuf[1];
			//pwBuf = (WORD *)&bMonTmpBuf[2];
			//pwNewData = (WORD *)&bDataBuf[0];
			MakeSort(wMonTmpBuf+1, wDataBuf, 1116+1);
			WriteItemEx(BANK11, PN0, 0x0e12, (BYTE *)wMonTmpBuf);
			m_dwMonUnbUTimes++;
			if (m_dwMonUnbUTimes > 22320)
			{
				m_dwMonUnbUTimes = 22320;
			}
		}
		
	}


	SetClearFlag(STAT_UNBALANCE,false);
}


//描述:计算平均值
int   CDpStat::GetAverage(int *pBuf, WORD wLen)
{
	if(wLen == 0)
		return 0;
	int		i		=	0;
	int		iSum	=	0;

	for( i = 0; i < wLen; i++)
		iSum += *pBuf++;

	return  (iSum/wLen);
}

//为了计算简单 全部换成整形计算,不使用浮点
int  CDpStat::GetUnbValue(int iAvr, int iVal)
{
	if( iAvr == 0)
		return 0;

	//单位 千份之一
	return  (int)(abs(iVal - iAvr) * 1000/iAvr);    
}

//描述:日/月电流越限累计时间 C2FF29(0x014f)   C2F37(0x01cf)
//参数:@piData		指向需求数据项数组的指针,
//	   @iLen		数据项的个数
//	   @dwCurMin	当前时间，单位 分钟
//	   @dwIntervM	间隔
//返回:无
//备注:		
void  CDpStat::DoCurrentStat(int * piData, int iLen, DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType)
{}

//描述:日/月视在功率越限时间统计 C2F30(0x015f)  C2F38(0x01cf)
//参数:@piData		指向需求数据项数组的指针,
//	   @iLen		数据项的个数
//	   @dwCurMin	当前时间，单位 分钟
//	   @dwIntervM	间隔
//返回:无
//备注:	
void  CDpStat::DoAppPowerStat(int * piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType)
{
	if ( iLen == 0 )
		return;

	int		iDayBuf[4]			=		{0};
	int		iMonBuf[4]			=		{0};
	int		iS					=		0;
	double	dbTmp				=		0;
	bool    fFlag				=		false;

	DWORD	dwDayStartS			=	60*GetStartM(dwCurMin, DAY );
	DWORD	dwDayEndS			=	60*GetEndM(dwCurMin,	DAY );
	DWORD	dwMonStartS			=	60*GetStartM(dwCurMin, MONTH );
	DWORD	dwMonEndS			=	60*GetEndM(	  dwCurMin,	MONTH );
	BYTE	bMaskProp		=		(BYTE)((1 << m_bPnProp) & wNeedToPnType);

	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, DAY)) //日零点，即日切换
	{
		dwDayEndS = dwDayStartS;
		dwDayStartS -= 24*60*60;
	}
	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, MONTH)) //月零点，即月切换
	{
		TTime tm;
		dwMonEndS = dwMonStartS;
		dwMonStartS -= 24*60*60;//上月最后一天的秒数
		SecondsToTime(dwMonStartS, &tm);
		dwMonStartS = GetMonthStart(tm);
	}

	//此统计不支持pn测试点属性填无效数据
	if( bMaskProp == 0 )
	{
		SetArrVal32(iDayBuf,INVALID_VAL,4);
		SetArrVal32(iMonBuf,INVALID_VAL,4);

		WriteItemVal(BN11, m_bPn, 0x015f, iDayBuf, dwCurMin*60);
		WriteItemVal(BN11, m_bPn, 0x01cf, iMonBuf, dwCurMin*60);

		return;
	}

	//当前有功功率读取缓冲
	int		*iP					=		piData;

	//当前无功功率读取缓冲
	int		*iQ					=		piData+128;   	

	if (iP[0]==INVALID_VAL || iQ[0]==INVALID_VAL)
		iS = INVALID_VAL;	
	else if (iP[0] == 0)
		iS = iQ[0];
	else if (iQ[0] == 0)
		iS = iP[0];
	else
	{
		//解决越界问题
		dbTmp = 100*sqrt( (double)((iP[0]/100.0)*(iP[0]/100.0)) + ((iQ[0]/100.0)*(iQ[0]/100.0))); //计算视在功率
		iS	= (int)(dbTmp+0.5);
	}

	//视在功率绝对值
	if (iS != INVALID_VAL)
		iS = abs(iS);

	WriteItemVal(BN0, m_bPn, 0xb670, &iS, dwCurMin*60);	
	

	if ( 0 >= ReadItemVal(BN11, m_bPn, 0x015f, iDayBuf,dwDayStartS,dwDayEndS)
		|| IsClearData(STAT_S))
	{
		memset((BYTE*)iDayBuf, 0, sizeof(iDayBuf));
		SetClearFlag(STAT_S,true,DAY);
	}

	if ( 0 >= ReadItemVal(BN11, m_bPn, 0x01cf, iMonBuf,dwMonStartS,dwMonEndS) 
		|| IsClearData(STAT_S,MONTH) )
	{
		memset((BYTE*)iMonBuf, 0, sizeof(iMonBuf) );
		SetClearFlag(STAT_S,true,MONTH);
	}

	if( ( iS >= m_iUpUpLimitAP) && (m_iUpUpLimitAP > 0) )
	{
		iDayBuf[0] += dwIntervM;
		iMonBuf[0] += dwIntervM;
		fFlag = true;
	}

	if( iS >= m_iUpLimitAP && m_iUpLimitAP>0 )
	{
		iDayBuf[1] += dwIntervM;
		iMonBuf[1] += dwIntervM;
		fFlag = true;
	}

	if ( fFlag )
	{
		WriteItemVal(BN11, m_bPn, 0x015f, iDayBuf, dwCurMin*60);
		WriteItemVal(BN11, m_bPn, 0x01cf, iMonBuf, dwCurMin*60);
	}	

	SetClearFlag(STAT_S,false);
	
}

//描述:日/月功率因素区段累计时间 C2F43(0x020f)   C2F44(0x021f)  
//参数:@piData		指向需求数据项数组的指针,
//	   @iLen		数据项的个数
//	   @dwCurMin	当前时间，单位 分钟
//	   @dwIntervM	间隔
//返回:无
//备注:		
void CDpStat::DoCosStat(int *piData, int iLen, DWORD dwCurMin, DWORD dwIntervM,WORD wNeedToPnType)
{
	if (iLen == 0)
		return;

	DWORD	dwDayStartS			=		60*GetStartM(dwCurMin, DAY );
	DWORD	dwDayEndS			=		60*GetEndM(dwCurMin,	DAY );
	DWORD	dwMonStartS			=		60*GetStartM(dwCurMin, MONTH );
	DWORD	dwMonEndS			=		60*GetEndM(dwCurMin,	MONTH );
	BYTE	bMaskProp			=		(BYTE)((1 << m_bPnProp) & wNeedToPnType);

	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, DAY)) //日零点，即日切换
	{
		dwDayEndS = dwDayStartS;
		dwDayStartS -= 24*60*60;
	}
	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, MONTH)) //月零点，即月切换
	{
		TTime tm;
		dwMonEndS = dwMonStartS;
		dwMonStartS -= 24*60*60;//上月最后一天的秒数
		SecondsToTime(dwMonStartS, &tm);
		dwMonStartS = GetMonthStart(tm);
	}

	int		iCurDayFactor[4]	=		{0};    
	int		iCurMonFactor[4]	=		{0};
	//此统计不支持pn测试点属性填无效数据
	if( bMaskProp == 0 )
	{
		SetArrVal32(iCurDayFactor,INVALID_VAL,4);
		SetArrVal32(iCurMonFactor,INVALID_VAL,4);

		WriteItemVal(BN11, m_bPn, 0x020f, iCurDayFactor, dwCurMin*60);
		WriteItemVal(BN11, m_bPn, 0x021f, iCurMonFactor, dwCurMin*60);

		return;
	}

	//功率因素绝对值
	if (piData[0] != INVALID_VAL)
		piData[0] = abs(piData[0]);	

	if (ReadItemVal(BN11, m_bPn, 0x020f, iCurDayFactor, dwDayStartS, dwDayEndS) <= 0
		|| IsClearData(STAT_COS))
	{
		memset((BYTE*)iCurDayFactor, 0, sizeof(iCurDayFactor));
		SetClearFlag(STAT_COS,true,DAY);
	}

	if (ReadItemVal(BN11, m_bPn, 0x021f, iCurMonFactor, dwMonStartS, dwMonEndS) <= 0
		|| IsClearData(STAT_COS,MONTH))
	{
		memset((BYTE*)iCurMonFactor, 0, sizeof(iCurMonFactor));
		SetClearFlag(STAT_COS,true,MONTH);
	}

	//取总功率因素比较
	if (piData[0]<m_iFactorLimit1)
	{
		iCurDayFactor[0] += dwIntervM;
		iCurMonFactor[0] += dwIntervM;
	}
	else if (piData[0]>=m_iFactorLimit1
			&& piData[0]<m_iFactorLimit2 && m_iFactorLimit2>0)
	{
		iCurDayFactor[1] += dwIntervM;
		iCurMonFactor[1] += dwIntervM;
	}
	else if (piData[0]>=m_iFactorLimit2 && m_iFactorLimit2>0)
	{
		iCurDayFactor[2] += dwIntervM;
		iCurMonFactor[2] += dwIntervM;
	}
	else
	{
		//同时为0的情况，作必要的异常处理
		DTRACE(DB_DP, ("CDpStat:DoCosStat Error: m_iFactorLimit1=%d and m_iFactorLimit2=%d \n",
						m_iFactorLimit1,m_iFactorLimit2));
	}

	WriteItemVal(BN11, m_bPn, 0x020f, iCurDayFactor, dwCurMin*60);
	WriteItemVal(BN11, m_bPn, 0x021f, iCurMonFactor, dwCurMin*60);

	SetClearFlag(STAT_COS,false);


}

//描述:日谐波电流区段累计时间 C2F113 - C2F118 (0x035F - 0x03aF) BN11
//参数:@piData		指向需求数据项数组的指针,
//	   @iLen		数据项的个数
//	   @dwCurMin	当前时间，单位 分钟
//	   @dwIntervM	间隔
//返回:无
//备注:	测试谐波的时候尤其关注N < 19的情况

void   CDpStat::DoHarmonicStat(int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType)
{
	if( iLen < 1 )
		return;
	//F57 piData F58 piData+128
	//DTRACE(DB_DP, ("CDpStat:DoHarmonicStat Start!\n"));
	int		iRet			=	0;
	BYTE	bPhase			=	0;
	int		iDest[38*6]		=	{0};
	int		iTotal[6]		=	{0};
	BYTE	bBuf[32]		=	{0};
	BYTE	bN				=	0;
	BYTE	bN1				=	0;
	BYTE	bN2				=	0;
	int		i				=	0;
	bool	fChange[6]		=	{false};

	DWORD	dwDayStartS		=		60*GetStartM(dwCurMin, DAY );
	DWORD	dwDayEndS		=		60*GetEndM(dwCurMin, DAY );
	DWORD	dwMonStartS		=		60*GetStartM(dwCurMin, MONTH );
	DWORD	dwMonEndS		=		60*GetEndM(	dwCurMin, MONTH );

	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, DAY)) //日零点，即日切换
	{
		dwDayEndS = dwDayStartS;
		dwDayStartS -= 24*60*60;		
	}
	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, MONTH)) //月零点，即月切换
	{
		TTime tm;
		dwMonEndS = dwMonStartS;
		dwMonStartS -= 24*60*60;//上月最后一天的秒数
		SecondsToTime(dwMonStartS, &tm);
		dwMonStartS = GetMonthStart(tm);
	}

	BYTE	bMaskProp		=		(BYTE)((1 << m_bPnProp) & wNeedToPnType);

	//取的F57,F58中的谐波次数
	bN1 = (BYTE)piData[0];
	bN2 = (BYTE)piData[128];
	if( bN1 != bN2 || bN1 < 2)
	{
		DTRACE(DB_DP, ("CDpStat:DoHarmonicStat:bN1=%d,bN2=%d F57 F58 N is Invalid \n",bN1,bN2));
		return;
	}
	//记录谐波次数bN
	bN = bN1;

	//此统计不支持pn测试点属性填无效数据
	if( bMaskProp == 0 )
	{
		DTRACE(DB_DP, ("CDpStat:DoHarmonicStat: bMaskProp=0 not support pn prop m_bPn=%d bProp=%d\n",m_bPn,m_bPnProp));

		int iInvalid[38] = {0};
		SetArrVal32(iInvalid,0,38);

		for (bPhase=0; bPhase<6; bPhase++)
		{
			iRet = WriteItemVal( BN11,m_bPn,0x035f + bPhase*16,iInvalid, dwCurMin*60);
		}

		return;
	}	
	
	//读取出日月冻结三相谐波电流统计结果
	WORD wResId[6] = { 0x035f,0x036f,0x037f,0x038f,0x039f,0x03af};

	for(i = 0; i < 6; i++)
	{
		if(i < 3)
			iRet = ReadItemVal(BN11,m_bPn,wResId[i],iDest+38*i,dwDayStartS,dwDayEndS);
		else
			iRet = ReadItemVal(BN11,m_bPn,wResId[i],iDest+38*i,dwDayStartS,dwDayEndS);
		if( iRet <= 0 || IsClearData(STAT_HARMONIC))
		{
			SetArrVal32(iDest+38*i,0,38);			
			SetClearFlag(STAT_HARMONIC,true,DAY);
			DTRACE(DB_DP, ("CDpStat:DoHarmonicStat: ReadItemVal fail pn,id=%d,%d,iRet=%d\n",m_bPn,wResId[i],iRet));			
		}

		fChange[i] = false;
	}

	//////////////////////////////////////////////////////////////////////////

	//直接读取方式
	//取出电压电流总谐波有效值 ???? 取出的方式有待考虑 todo
	iRet = ReadItemEx(BN2,PN0,0x201f,bBuf);
	if(iRet > 0)
	{
		for( i = 0; i < 6; i++)
		{
			iTotal[i] = abs(FmtToVal(bBuf+2*i,FMT5,2));
		}
	}
	else
	{
		SetArrVal32(iTotal,0,6);
		DTRACE(DB_DP, ("CDpStat:DoHarmonicStat: ReadItemVal fail pn,id=%d,%d,iRet=%d\n",m_bPn,wResId[i],iRet));
	}
	
	/*
	//取出电压电流总谐波有效值 
	for(i = 0; i < 6; i++)
		iTotal[i] = piData[128*2+i];
	*/
	

	//////////////////////////////////////////////////////////////////////////

	//比较UVW电流有效值的最大值
	for(bPhase = 0; bPhase < 3; bPhase++)
	{
		for(i = 0; i < 18; i++)
		{
			if ( INVALID_VAL == piData[i+(bN-1)*(bPhase+3)+1])
			{
				continue;
			}
				
			if( (i+2) <= bN)
			{
				//比较谐波电流大小
				if( piData[i+(bN-1)*(bPhase+3)+1] > iDest[bPhase*38 + i*2] || IsClearData(STAT_HARMONIC))
				{
					iDest[bPhase*38 + i*2] = piData[i+(bN-1)*(bPhase+3)+1];
					MinToFmt(dwCurMin, (BYTE *)&iDest[bPhase*38 + i*2+1], FMT17);		
					fChange[bPhase] = true;
				}
			}
			else
			{
				iDest[bPhase*38 + i*2] = INVALID_VAL;
				iDest[bPhase*38 + i*2 + 1] = INVALID_VAL;				
			}
		}

		//总谐波电流
		if( iTotal[bPhase+3] > iDest[bPhase*38 + i*2] || IsClearData(STAT_HARMONIC))
		{
			iDest[bPhase*38 + i*2] = iTotal[bPhase+3];
			MinToFmt(dwCurMin, (BYTE *)&iDest[bPhase*38 + i*2+1], FMT17);
			fChange[bPhase] = true;
		}
	}

	//比较UVW电压含有率的最大值
	for(bPhase = 0; bPhase < 3; bPhase++)
	{
		for(i = 0; i < 18; i++)
		{
			if (INVALID_VAL == piData[128 + i + 2 + bPhase*bN])
			{
				continue;
			}
			if( (i+2) <= bN)
			{
				//比较谐波电压含有率大小
				if( piData[128 + i + 2 + bPhase*bN] > iDest[38*3+bPhase*38 + i*2] || IsClearData(STAT_HARMONIC))
				{
					iDest[38*3+bPhase*38 + i*2] = piData[128 + i + 2 + bPhase*bN];
					MinToFmt(dwCurMin, (BYTE *)&iDest[38*3+bPhase*38 + i*2 + 1], FMT17);	
					fChange[bPhase+3] = true;
				}
			}
			else
			{
				iDest[38*3 + bPhase*38 + i*2] = INVALID_VAL;
				iDest[38*3 + bPhase*38 + i*2 + 1] = INVALID_VAL;				
			}
		}

		//总谐波电压含有率
		if( piData[128+bPhase*bN+1]> iDest[38*3 + bPhase*38 + i*2] || IsClearData(STAT_HARMONIC))
		{
			iDest[38*3 + bPhase*38 + i*2] = piData[128+bPhase*bN+1];
			MinToFmt(dwCurMin, (BYTE *)&iDest[38*3 + bPhase*38 + i*2+ 1], FMT17);
			fChange[bPhase+3] = true;
		}
	}
//Start----------------------------------------------------95%概率值计算----------------------------------------------------Start
	if (GetPnProp(m_bPn) == PN_PROP_AC)//只统计交采
	{
		int j = 0, k = 0;
//		BYTE bTmpBuf[148];
//		BYTE bDataBuf[2];
		WORD wTmpBuf[74];
		WORD wDataBuf;
		int iOddData ;//奇次畸变电压含有率
		int iEvenData ;//偶次畸变电压含有率
//		WORD *pwBuf ;
//		WORD *pwNewData ;
		BYTE bTime[3];
		TTime time;

		GetCurTime(&time);
		TimeToFmt20(time, bTime);
		ReadItemEx(BANK11, PN0, 0x0d60, (BYTE *)wTmpBuf);
		if (memcmp(bTime, (BYTE *)wTmpBuf, 3) != 0)
		{
			//DoHamornicTransfer(0);//清除BANK11的数据

			WriteItemEx(BANK11, PN0, 0x0d60, bTime);
		}
		for(bPhase = 0; bPhase < 3; bPhase++)
		{
			iOddData = 0;
			iEvenData = 0;
			for(i = 0; i < 18; i++)
			{
				if (INVALID_VAL == piData[128 + i + 2 + bPhase*bN])
				{
					continue;
				}
				if( (i+2) <= bN)
				{
					//电压含有率
						memset(wTmpBuf, 0, sizeof(wTmpBuf));
	//					memset(wDataBuf, 0, sizeof(wDataBuf));
						wDataBuf = 0;
						j = i;
						if(i >= 12)
							j += 1;
						ReadItemEx(BANK11, PN0, 0x0c03+bPhase*0x20+j, (BYTE *)wTmpBuf);
						DWORDToBCD(piData[128 + i + 2 + bPhase*bN], (BYTE *)&wDataBuf, 2);//N次畸变电压含有率
						DWORDToBCD(m_wMidTimes+1, (BYTE *)&wTmpBuf[0], 2);//统计次数
						//bTmpBuf[2 + m_wMidTimes*2] = bDataBuf[0];
						//bTmpBuf[3 + m_wMidTimes*2] = bDataBuf[1];
						//pwBuf = (WORD *)&bTmpBuf[2];
						//pwNewData = (WORD *)&bDataBuf[0];
						MakeSort(wTmpBuf+1, wDataBuf, 72+1);
						WriteItemEx(BANK11, PN0, 0x0c03+bPhase*0x20+j, (BYTE *)wTmpBuf);

					//谐波电流
						memset(wTmpBuf, 0, sizeof(wTmpBuf));
	//					memset(bDataBuf, 0, sizeof(bDataBuf));
						wDataBuf = 0;
						k = i;
						if(i >= 14)
							k += 1;
						ReadItemEx(BANK11, PN0, 0x0d01+bPhase*0x20+k, (BYTE *)wTmpBuf);
						DWORDToBCD(piData[i+(bN-1)*(bPhase+3)+1], (BYTE *)&wDataBuf,2);//N次谐波电流
						DWORDToBCD(m_wMidTimes+1, (BYTE *)&wTmpBuf[0],2);//统计次数
						//bTmpBuf[2 + m_wMidTimes*2] = bDataBuf[0];
						//bTmpBuf[3 + m_wMidTimes*2] = bDataBuf[1];
						//pwBuf = (WORD *)&bTmpBuf[2];
						//pwNewData = (WORD *)&bDataBuf[0];
						MakeSort(wTmpBuf+1, wDataBuf, 72+1);
						WriteItemEx(BANK11, PN0, 0x0d01+bPhase*0x20+k, (BYTE *)wTmpBuf);

					//偶畸变电压含有率
					if((i+2)%2==0)
					{
						iEvenData += piData[128 + i + 2 + bPhase*bN];
					}
					else if((i+2)%2==1)//奇畸变电压含有率
					{
						iOddData += piData[128 + i + 2 + bPhase*bN];
					}
					
				}
				else
				{
					//iDest[38*3 + bPhase*38 + i*2] = INVALID_VAL;
					//iDest[38*3 + bPhase*38 + i*2 + 1] = INVALID_VAL;				
				}


			}
			//总畸变电压含有率
				ReadItemEx(BANK11, PN0, 0x0c00+bPhase*0x20, (BYTE *)wTmpBuf);
				DWORDToBCD(piData[128+bPhase*bN+1], (BYTE *)&wDataBuf,2);//总畸变电压含有率
				DWORDToBCD(m_wMidTimes+1, (BYTE *)&wTmpBuf[0], 2);//统计次数
				//bTmpBuf[2 + m_wMidTimes*2] = bDataBuf[0];
				//bTmpBuf[3 + m_wMidTimes*2] = bDataBuf[1];
				//pwBuf = (WORD *)&bTmpBuf[2];
				//pwNewData = (WORD *)&bDataBuf[0];
				MakeSort(wTmpBuf+1, wDataBuf, 72+1);
				WriteItemEx(BANK11, PN0, 0x0c00+bPhase*0x20, (BYTE *)wTmpBuf);
				
				//奇畸变电压含有率
				ReadItemEx(BANK11, PN0, 0x0c01+bPhase*0x20, (BYTE *)wTmpBuf);
				DWORDToBCD(iOddData, (BYTE *)&wDataBuf,2);
				DWORDToBCD(m_wMidTimes+1, (BYTE *)&wTmpBuf[0],2);//统计次数
				//bTmpBuf[2 + m_wMidTimes*2] = bDataBuf[0];
				//bTmpBuf[3 + m_wMidTimes*2] = bDataBuf[1];
				//pwBuf = (WORD *)&bTmpBuf[2];
				//pwNewData = (WORD *)&bDataBuf[0];
				MakeSort(wTmpBuf+1, wDataBuf, 72+1);
				WriteItemEx(BANK11, PN0, 0x0c01+bPhase*0x20, (BYTE *)wTmpBuf);

			//偶畸变电压含有率
				ReadItemEx(BANK11, PN0, 0x0c02+bPhase*0x20, (BYTE *)wTmpBuf);
				DWORDToBCD(iEvenData, (BYTE *)&wDataBuf,2);
				DWORDToBCD(m_wMidTimes+1, (BYTE *)&wTmpBuf[0],2);//统计次数
				//bTmpBuf[2 + m_wMidTimes*2] = bDataBuf[0];
				//bTmpBuf[3 + m_wMidTimes*2] = bDataBuf[1];
				//pwBuf = (WORD *)&bTmpBuf[2];
				//pwNewData = (WORD *)&bDataBuf[0];
				MakeSort(wTmpBuf+1, wDataBuf, 72+1);
				WriteItemEx(BANK11, PN0, 0x0c02+bPhase*0x20, (BYTE *)wTmpBuf);

				
				//总畸变电流iTotal
				ReadItemEx(BANK11, PN0, 0x0d00+bPhase*0x20, (BYTE *)wTmpBuf);
				DWORDToBCD(iTotal[bPhase+3], (BYTE *)&wDataBuf,2);//总畸变电流
				DWORDToBCD(m_wMidTimes+1, (BYTE *)&wTmpBuf[0],2);//统计次数
				//bTmpBuf[2 + m_wMidTimes*2] = bDataBuf[0];
				//bTmpBuf[3 + m_wMidTimes*2] = bDataBuf[1];
				//pwBuf = (WORD *)&bTmpBuf[2];
				//pwNewData = (WORD *)&bDataBuf[0];
				MakeSort(wTmpBuf+1, wDataBuf, 72+1);
				WriteItemEx(BANK11, PN0, 0x0d00+bPhase*0x20, (BYTE *)wTmpBuf);	

		}

		m_wMidTimes += 1;//谐波统计次数
		if (m_wMidTimes > 1440)
		{
			m_wMidTimes = 1440;
		}
	}
//End----------------------------------------------------95%概率值计算----------------------------------------------------End
	

	for(bPhase = 0; bPhase < 6; bPhase++)
	{
		if(fChange[bPhase])
		{
			iRet = WriteItemVal( BN11,m_bPn,0x035f + bPhase*16,iDest + bPhase*38, dwCurMin*60);
			if( iRet <= 0)
			{
				DTRACE(DB_DP, ("CDpStat:DoHarmonicStat WriteItemVal failed! BN11 m_bPn=%d id=%d\n",m_bPn,0x035f + bPhase*16));
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//U相电压含有率，电流有效值越限时间F121,V相电压含有率越限，电流有效值越限时间F122,W相电压含有率，电流有效值越限时间F123
	//0x03bf,0x03cf,0x03df ->0x335f,0x336f,0x337f
	//F60 0x03cf 谐波越限限值参数
	  
	int iCrashHold[39] = {0};
	for(bPhase = 0; bPhase < 3; bPhase++)
	{
		iRet = ReadItemVal(BN11,m_bPn,0x03bf+16*bPhase,iCrashHold,dwDayStartS,dwDayEndS);
		if( iRet <= 0 || IsClearData(STAT_HARMONIC) )
		{
			SetClearFlag(STAT_HARMONIC,true,DAY);
			SetArrVal32(iCrashHold,0,39);
		}
		//计算U相电压含有率越限时间
		iCrashHold[0] = bN;

		fChange[bPhase] = false;
		int iTmp = 0;
		for(i = 0; i < bN; i++)
		{
			if (INVALID_VAL == piData[128 + i + 1 + bPhase*bN])
			{
				continue;
			}
			iTmp = piData[128 + i + 1 + bPhase*bN];	//电压含有率
			if( i == 0)
			{
				//总谐波电压含有率 > 总谐波电压含有率上限
				if( iTmp >= m_iHarmonicLimit[0] )
				{
					iCrashHold[i+1] += dwIntervM;
					fChange[bPhase] = true;
				}
			}
			else if( (i+1)%2 == 1)
			{
				//奇次谐波含有率
				if( iTmp >= m_iHarmonicLimit[1] )
				{
					iCrashHold[i+1] += dwIntervM;
					fChange[bPhase] = true;
				}

			}
			else if( (i+1)%2 == 0)
			{
				//偶次谐波含有率
				if( iTmp >= m_iHarmonicLimit[2] )
				{
					iCrashHold[i+1] += dwIntervM;
					fChange[bPhase] = true;
				}
			}
		}

		//计算电流有效值越限
		for(i = 0; i < bN; i++)
		{
			if (INVALID_VAL == piData[i+(bN-1)*(bPhase+3)])
			{
				continue;
			}
			iTmp = piData[i+(bN-1)*(bPhase+3)];	//电流有效值
			if( i == 0)
			{
				iTmp = iTotal[bPhase+3];//总谐波电流有效值
				//总谐波电流有效值 > 总谐波电流有效值上限
				if( iTmp >= m_iHarmonicLimit[3] )
				{
					iCrashHold[bN+i+1] += dwIntervM;
					fChange[bPhase] = true;
				}
			}
			else if( (i+1)%2 == 1)
			{
				//奇次电流有效值
				if( iTmp >= m_iHarmonicLimit[13+(i-2)/2] )
				{
					iCrashHold[bN+i+1] += dwIntervM;
					fChange[bPhase] = true;
				}

			}
			else if( (i+1)%2 == 0)
			{
				//偶次电流有效值
				if( iTmp >= m_iHarmonicLimit[4+(i-1)/2] )
				{
					iCrashHold[bN+i+1] += dwIntervM;
					fChange[bPhase] = true;
				}
			}
		}

		//写库
		if(fChange[bPhase])
		{
			iRet = WriteItemVal(BN11,m_bPn,0x03bf+bPhase*16,iCrashHold,dwCurMin*60);
			if( iRet <= 0)
			{
				DTRACE(DB_DP, ("CDpStat:DoHarmonicStat WriteItemVal failed! BN11 m_bPn=%d id=%d\n",m_bPn,0x03bf+bPhase*16));
			}
		}
		
		SetArrVal32(iCrashHold,0,39);
	}

	//同时清除日月置零标志位
	SetClearFlag(STAT_HARMONIC,false);

	return;
}




void  CDpStat::SetDcDataInvalid(int *piDcDay, int *piDcMon,DWORD dwCurMin)
{
	int iRet = 0;
	SetArrVal32(piDcDay,0,6);
	SetArrVal32(piDcMon,0,6);

	iRet = WriteItemVal(BN11,m_bPn,0x03ef,piDcDay,dwCurMin*60);
	iRet = WriteItemVal(BN11,m_bPn,0x040f,piDcMon,dwCurMin*60);

	//同时清除日月置零标志位
	SetClearFlag(STAT_DC,false);

	return;
}

//描述:	日月直流模拟量越限累计时间，最大值最小值以及发生时间 F129 - F130(0x03ef 0x040f 0x338F-0x339F)
//参数:	@piData		指向需求数据项数组的指针,
//		@iLen		数据项的个数
//		@dwCurMin	当前时间，单位 分钟
//		@dwIntervM	间隔
//返回:无
//备注:	


void   CDpStat::DoDcStat(int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType)
{}


//描述:日/月负载率统计,C2F31(0x0a0f)   C2F39(0x0a1f)
//参数:@piData		指向需求数据项数组的指针,
//	   @iLen		数据项的个数
//	   @dwCurMin	当前时间，单位 分钟
//	   @dwIntervM	间隔
void  CDpStat::DoLoadStat(int *piData, int iLen, DWORD  dwCurMin, DWORD dwIntervM, WORD wNeedToPnType)
{
	int	i = 0;
	if (iLen == 0)
		return;

	int	iCurDayLoad[4] = {0};
	int	iCurMonLoad[4] = {0};
	DWORD dwDayStartS = 60*GetStartM(dwCurMin, DAY );
	DWORD dwDayEndS = 60*GetEndM(dwCurMin, DAY );
	DWORD dwMonStartS =	60*GetStartM(dwCurMin, MONTH );
	DWORD dwMonEndS = 60*GetEndM(dwCurMin, MONTH );
	BYTE bMaskProp = (BYTE)((BYTE)((1 << m_bPnProp) & wNeedToPnType));
	int iTmpData[8] = { 0 };	
	int iPwrRating = 0;
	int iCurLoad = 0;
	bool fFlag1 = 0;
	bool fFlag2	= 0;	


	ReadItemVal(BN0, m_bPn, 0x019f, iTmpData);	//F25
	iPwrRating = iTmpData[4];
	if (iPwrRating != 0)
		iCurLoad = piData[0]*1000/iPwrRating;
	else
		iCurLoad = 0;

	DTRACE(DB_DP,("DoDcStat::DoLoadStat PN = %d, power rating %d, power %d###########################.\n", m_bPn, iPwrRating, piData[0]));

 	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, DAY)) //日零点，即日切换
	{
		dwDayEndS = dwDayStartS;
		dwDayStartS -= 24*60*60;

		//暂存0点时刻的瞬时数据作为起点数据
		g_iZeroStartVal[0] = m_bPn;
		g_iZeroStartVal[1] = STAT_LOAD;
		g_iZeroStartVal[2] = GetStartM(dwCurMin, DAY);

		memcpy((BYTE*)&g_iZeroStartVal[3], (BYTE *)&iCurLoad, sizeof(int));//最大值
		memcpy((BYTE*)&g_iZeroStartVal[4], (BYTE *)&iCurLoad, sizeof(int));//最小值

		/*
		//0点时刻的瞬时数据作为起点数据
		iCurDayLoad[2] = iCurLoad;
		MinToFmt(dwCurMin, (BYTE *)&iCurDayLoad[3], FMT18);
		fFlag1 = true;

		iCurMonLoad[2] = iCurLoad;
		MinToFmt(dwCurMin, (BYTE *)&iCurMonLoad[3], FMT17);
		fFlag2 = true;
		*/
	}
	if (dwCurMin!=0 && dwCurMin==GetStartM(dwCurMin, MONTH)) //月零点，即月切换
	{
		TTime tm;
		dwMonEndS = dwMonStartS;
		dwMonStartS -= 24*60*60;//上月最后一天的秒数
		SecondsToTime(dwMonStartS, &tm);
		dwMonStartS = GetMonthStart(tm);		
	}

	if (0 >= ReadItemVal(BN11, m_bPn, 0x0a0f, iCurDayLoad, dwDayStartS, dwDayEndS)
		|| IsClearData(STAT_LOAD))
	{
		memset((BYTE*)iCurDayLoad, 0, sizeof(iCurDayLoad));
		SetClearFlag(STAT_LOAD, true, DAY);
	}

	if( 0 >= ReadItemVal( BN11, m_bPn, 0x0a1f, iCurMonLoad, dwMonStartS, dwMonEndS)
		|| IsClearData(STAT_LOAD, MONTH))
	{
		memset((BYTE*)iCurMonLoad, 0, sizeof(iCurMonLoad));
		SetClearFlag(STAT_LOAD, true, MONTH);
	}

	if (m_fFisrtDoLoad)
	{
		if (iCurDayLoad[2] == 0)
		{
			iCurDayLoad[2] = iCurLoad;
			MinToFmt(dwCurMin, (BYTE *)&iCurDayLoad[3], FMT18);
			fFlag1 = true;
		}
		if (iCurMonLoad[2] == 0)
		{
			iCurMonLoad[2] = iCurLoad;
			MinToFmt(dwCurMin, (BYTE *)&iCurMonLoad[3], FMT17);
			fFlag2 = true;
		}	
		m_fFisrtDoLoad = false;
	}

	//日最大负载率统计
	if (iCurLoad>iCurDayLoad[0] || IsClearData(STAT_LOAD, DAY) || iCurDayLoad[1]==0)//如果以前的时间为零表示原来还没取过值，用当前值作为初值
	{
		iCurDayLoad[0] = iCurLoad;
		MinToFmt(dwCurMin, (BYTE *)&iCurDayLoad[1], FMT18);
		fFlag1 = true;
	}

	//日最小负载率统计
	if (iCurLoad<iCurDayLoad[2] || IsClearData(STAT_LOAD, DAY) || iCurDayLoad[3]==0)//如果以前的时间为零表示原来还没取过值，用当前值作为初值
	{
		iCurDayLoad[2] = iCurLoad;
		MinToFmt(dwCurMin, (BYTE *)&iCurDayLoad[3], FMT18);
		fFlag1 = true;
	}

	//月最大负载率统计
	if (iCurLoad > iCurMonLoad[0] || IsClearData(STAT_LOAD, MONTH) || iCurMonLoad[1]==0)//如果以前的时间为零表示原来还没取过值，用当前值作为初值
	{
		iCurMonLoad[0] = iCurLoad;
		MinToFmt(dwCurMin, (BYTE *)&iCurMonLoad[1], FMT17);
		fFlag2 = true;
	}

	//月最小负载率统计
	if (iCurLoad < iCurMonLoad[2] || IsClearData(STAT_LOAD, MONTH) || iCurMonLoad[3]==0)//如果以前的时间为零表示原来还没取过值，用当前值作为初值
	{
		iCurMonLoad[2] = iCurLoad;
		MinToFmt(dwCurMin, (BYTE *)&iCurMonLoad[3], FMT17);
		fFlag2 = true;
	}

	if (fFlag1)
		WriteItemVal(BN11, m_bPn, 0x0a0f, iCurDayLoad, 60*dwCurMin);

	if (fFlag2)
		WriteItemVal(BN11, m_bPn, 0x0a1f, iCurMonLoad, 60*dwCurMin);

	//同时清除日月置零标志位
	SetClearFlag(STAT_LOAD, false);
}


#ifdef SYS_WIN

//////////////////////////////////////////////////////////////////////////
//调试谐波使用 写入数据f57 f58
void CDpStat::DebugHarmonic(DWORD dwSec)
{
	int iDataArray[256] = {0};
	BYTE bN = HARMONIC_NUM;
	iDataArray[0] = bN;		//谐波次数
	int i = 0;
	BYTE bPhase = 0;
	int iRet = 0;

	for(bPhase = 0; bPhase < 6; bPhase++)
	{
		for(i = 0; i < (HARMONIC_NUM-1); i++)
		{
			iDataArray[bPhase*(bN-1)+1+i] = 1000+bPhase*100+i+2;
		}
	}

	//写入F57
	iRet = WriteItemVal(BN0,m_bPn,0x127f,iDataArray,dwSec);

	memset((BYTE*)iDataArray, 0, sizeof(iDataArray));
	iRet = ReadItemVal(BN0,m_bPn,0x127f,iDataArray,dwSec);

	for(bPhase = 0; bPhase < 3; bPhase++)
	{
		for(i = 0; i < HARMONIC_NUM; i++)
		{
			if( i == 0)
				iDataArray[bPhase*bN+1+i] = 3000+bPhase*100;
			else
				iDataArray[bPhase*bN+1+i] = 2000+bPhase*100+i+1;
		}
	}

	for(bPhase = 3; bPhase < 6; bPhase++)
	{
		for(i = 0; i < (HARMONIC_NUM-1); i++)
		{
			iDataArray[bN*3+(bPhase-3)*(bN-1)+1+i] = 2000+bPhase*100+i+2;
		}
	}

	//写入F58
	iRet = WriteItemVal(BN0,m_bPn,0x128f,iDataArray,dwSec);
	memset((BYTE*)iDataArray, 0, sizeof(iDataArray));
	iRet = ReadItemVal(BN0,m_bPn,0x128f,iDataArray,dwSec);


	BYTE bBuf[16] = {0};
	//写入0x201f
	for( i = 0; i < 6; i++)
	{
		iDataArray[i] = (WORD)100*i;
		ValToFmt(iDataArray[i],bBuf+i*2,FMT5,2);
	}

	//iRet = WriteItemVal(BN2,PN0,0x201f,iDataArray,dwSec);

	iRet = WriteItemEx(BN2,PN0,0x201f,(BYTE*)bBuf,dwSec);

	DWORD	dwDayStartS		=		60*GetStartM(dwSec/60, DAY );
	DWORD	dwDayEndS		=		60*GetEndM(dwSec/60, DAY );

	memset((BYTE*)iDataArray, 0, sizeof(iDataArray));
	iRet = ReadItemEx(BN2,PN0,0x201f,(BYTE*)bBuf,dwDayStartS,dwDayEndS);
	for( i = 0; i < 6; i++)
	{
		iDataArray[i] = FmtToVal(bBuf+i*2,FMT5,2);

	}

	//iRet = ReadItemVal(BN2,PN0,0x201f,iDataArray,dwDayStartS,dwDayEndS);	

	return;

}

void CDpStat::DebugDc(DWORD dwSec)
{
	BYTE bBuf[4] = {0};
	int iDc = 50;
	int iRet = 0;	
	ValToFmt(iDc,bBuf,FMT2,2);
	iRet =WriteItemVal(BN0,m_bPn,0x12Cf,&iDc,dwSec);	
	iDc = 0;
	iRet = ReadItemVal(BN0,m_bPn,0x12cf,&iDc,dwSec);

	return;
}

void CDpStat::DebugPhaseAngle(DWORD dwSec)
{
	BYTE bBuf[12] = {0};
	int iAng = 1780;
	int iRet = 0;	
	for(int i=0; i<6; i++)
		ValToFmt(iAng++, bBuf+2*i, FMT5, 2);
	iRet =WriteItemEx(BN0,m_bPn,0xb66f,bBuf,dwSec);	
	//iAng = 0;
	//iRet = ReadItemVal(BN0,m_bPn,0x126f,&iAng,dwSec);
	return;
}

void CDpStat::DebugData(DWORD wID, DWORD dwSec)
{
	int64 iVal64[5] = {0};	
	int iRet = 0;	

	iRet = ReadItemVal64(BN0, (WORD)m_bPn, wID, iVal64, dwSec);
	for(int i=0; i<5; i++)
	{
		if (i == 0)
			iVal64[i] += 4;
		else
			iVal64[i] = iVal64[0]/4;
	}
	iRet = WriteItemVal64(BN0, (WORD)m_bPn, wID, iVal64, dwSec);	
	

}

void CDpStat::DebugAcEng(DWORD dwSec)
{	
	DebugData(0x901f, dwSec);	
	return;
}

void CDpStat::DebugPulseEng(DWORD dwSec)
{
	BYTE bBuf[50];
	memset(bBuf, 0,	sizeof(bBuf));
	ReadItemEx(BN0, (WORD)m_bPn, 0x8903, bBuf);

	for (BYTE i=0; i<4; i++)
	{
		if (bBuf[4*i] != 0)
		{
			switch (bBuf[4*i+1])
			{
			case 0:
				DebugData(0x901f, dwSec);	break;
			case 1:
				DebugData(0x911f, dwSec);	break;
			case 2:
				DebugData(0x902f, dwSec);	break;
			case 3:
				DebugData(0x912f, dwSec);	break;
			default:
				break;
			}

		}
	}
}

#endif

bool CDpStat::IsClearData(int iStatId, BYTE bDayOrMon)
{
	if(bDayOrMon == DAY )
		return (m_bStatValid[iStatId]&0x02) > 0;
	else if(bDayOrMon == MONTH)
		return (m_bStatValid[iStatId]&0x04) > 0;
	else
		return false;

}

void CDpStat::SetClearFlag(int iStatId,bool fZero,BYTE bDayOrMon)
{
	//bFlag 为1 表示已经清除过数据
	if( fZero )
	{
		if(bDayOrMon == DAY)
			m_bStatValid[iStatId] |= 0x02;
		else if(bDayOrMon == MONTH)
			m_bStatValid[iStatId] |= 0x04;
		else
			m_bStatValid[iStatId] |= 0x06;
	}
	else
	{
		if(bDayOrMon == DAY)
			m_bStatValid[iStatId] &= 0xFD;
		else if(bDayOrMon == MONTH)
			m_bStatValid[iStatId] &= 0xFB;	
		else
			m_bStatValid[iStatId] &= 0xF9;	
	}

	return;

}


//描述：取得每个统计类别的间隔
WORD CDpStat::GetStatIntervV(int iStatId)
{	


	return 0;

}
void CDpStat::DoHamornicTransfer(DWORD dwOldSec)
{
	BYTE bBuf[1+2*3+2*18+2+2*18];
//	BYTE bZeroBuf[310];
	WORD wTmp[75];
	WORD *wBuf,wNum=0,wProMax=0,iRet, wflag = 0;
	memset(bBuf, 0, sizeof(bBuf));
	memset(wTmp, 0, sizeof(wTmp));
	int bPhase,i,j;
	
	for(bPhase=0; bPhase<3; bPhase++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		bBuf[0] = HARMONIC_NUM;//谐波de次数
		//A、B、C相总畸变电压含有率95%概率值
		iRet = ReadItemEx(BN11, PN0, 0x0c00+bPhase*0x20, (BYTE *)wTmp);
		if(iRet > 0)
		{
			wNum = BcdToDWORD((BYTE *)&wTmp[0], 2);
			//wBuf = (WORD*)&bTmp[2];
			//wProMax = GetProbabilityMax(wBuf, wNum);
			//WordToByte(wProMax, bBuf+1);
			wflag = wNum*5/100;
			if (wflag > 72)
			{
				wflag = 72;
			}
			memcpy(bBuf+1, (BYTE *)&wTmp[1+wflag], 2);

			memset(wTmp, 0, sizeof(wTmp));
			WriteItemEx(BN11, PN0, 0x0c00+bPhase*0x20, (BYTE *)wTmp);
		}
		//A、B、C相奇次畸变电压含有率95%概率值
		iRet = ReadItemEx(BN11, PN0, 0x0c01+bPhase*0x20, (BYTE *)wTmp);
		if(iRet > 0)
		{
			wNum =BcdToDWORD((BYTE *)&wTmp[0], 2);
			//wBuf = (WORD*)&bTmp[2];
			//wProMax = GetProbabilityMax(wBuf, wNum);
			//WordToByte(wProMax, bBuf+1+2);
			wflag = wNum*5/100;
			if (wflag > 72)
			{
				wflag = 72;
			}
			memcpy(bBuf+1+2, (BYTE *)&wTmp[1+wflag], 2);

			memset(wTmp, 0, sizeof(wTmp));
			WriteItemEx(BN11, PN0, 0x0c01+bPhase*0x20, (BYTE *)wTmp);
		}
		//A、B、C相偶次畸变电压含有率95%概率值
		iRet = ReadItemEx(BN11, PN0, 0x0c02+bPhase*0x20, (BYTE *)wTmp);
		if(iRet > 0)
		{
			wNum =BcdToDWORD((BYTE *)&wTmp[0], 2);
			//wBuf = (WORD*)&bTmp[2];
			//wProMax = GetProbabilityMax(wBuf, wNum);
			//WordToByte(wProMax, bBuf+1+2+2);
			wflag = wNum*5/100;
			if (wflag > 72)
			{
				wflag = 72;
			}
			memcpy(bBuf+1+2+2, (BYTE *)&wTmp[1+wflag], 2);

			memset(wTmp, 0, sizeof(wTmp));
			WriteItemEx(BN11, PN0, 0x0c02+bPhase*0x20, (BYTE *)wTmp);
		}
		//A、B、C相N次谐波电压含有率95%概率值
		for(i=0; i<HARMONIC_NUM-1; i++)
		{
			j = i;
			if(i >= 12)
				j += 1;
			iRet = ReadItemEx(BN11, PN0, 0x0c03+j+bPhase*0x20, (BYTE *)wTmp);
//			if(iRet < 0)
//				continue;
			wNum = BcdToDWORD((BYTE *)&wTmp[0], 2);
			//wBuf = (WORD*)&bTmp[2];
			//wProMax = GetProbabilityMax(wBuf, wNum);
			//WordToByte(wProMax, bBuf+1+2*3+i*2);
			wflag = wNum*5/100;
			if (wflag > 72)
			{
				wflag = 72;
			}
			memcpy(bBuf+1+2*3+i*2, (BYTE *)&wTmp[1+wflag], 2);

			memset(wTmp, 0, sizeof(wTmp));
			WriteItemEx(BN11, PN0, 0x0c03+j+bPhase*0x20, (BYTE *)wTmp);
		}
		//A、B、C相总畸变电流95%概率值
		iRet = ReadItemEx(BN11, PN0, 0x0d00+bPhase*0x20, (BYTE *)wTmp);
		if(iRet > 0)
		{
			wNum = BcdToDWORD((BYTE *)&wTmp[0], 2);
			//wBuf = (WORD*)&bTmp[2];
			//wProMax = GetProbabilityMax(wBuf, wNum);
			//WordToByte(wProMax, bBuf+1+2*3+2*18);
			wflag = wNum*5/100;
			if (wflag > 72)
			{
				wflag = 72;
			}
			memcpy(bBuf+1+2*3+2*18, (BYTE *)&wTmp[1+wflag], 2);

			memset(wTmp, 0, sizeof(wTmp));
			WriteItemEx(BN11, PN0, 0x0d00+bPhase*0x20, (BYTE *)wTmp);
		}
		//N次谐波电流95%概率值
		for(i=0; i<HARMONIC_NUM-1; i++)
		{
			j = i;
			if(i >= 14)
				j += 1;
			iRet = ReadItemEx(BN11, PN0, 0x0d01+j+bPhase*0x20, (BYTE *)wTmp);
//			if(iRet < 0)
//				continue;
			wNum = BcdToDWORD((BYTE *)&wTmp[0], 2);
			//wBuf = (WORD*)&bTmp[2];
			//wProMax = GetProbabilityMax(wBuf, wNum);
			//WordToByte(wProMax, bBuf+1+2*3+2*18+2+2*i);
			wflag = wNum*5/100;
			if (wflag > 72)
			{
				wflag = 72;
			}
			memcpy(bBuf+1+2*3+2*18+2+2*i, (BYTE *)&wTmp[1+wflag], 2);

			memset(wTmp, 0, sizeof(wTmp));
			WriteItemEx(BN11, PN0, 0x0d01+j+bPhase*0x20, (BYTE *)wTmp);
		}

		WriteItemEx(BN0 ,m_bPn, 0x382f+bPhase*0x10, bBuf, dwOldSec);//转存数据

	}

	m_wMidTimes = 0;
}
void CDpStat::DoUnbalanceTransfer(BYTE bChgFlg, DWORD dwOldSec)
{
	BYTE bBuf[4];
//	BYTE bZeroBuf[148];
	WORD wTmp[1119];
//	BYTE bMonTmp[2236];
//	BYTE bMonZero[2236];
	WORD *wBuf,wNum=0,wProMax=0,iRet,wflag=0;
	DWORD dwMonNum=0;
	memset(bBuf, 0, sizeof(bBuf));
	memset(wTmp, 0, sizeof(wTmp));
//	memset(bMonZero, 0, sizeof(bMonZero));

	if(bChgFlg == TIME_UNIT_DAY)
	{
		//日电流不平衡度95%概率值
		iRet = ReadItemEx(BN11, PN0, 0x0e01, (BYTE *)wTmp);
		if(iRet > 0)
		{
			wNum = BcdToDWORD((BYTE *)&wTmp[0], 2);
			//wBuf = (WORD*)&bTmp[2];
			//wProMax = GetProbabilityMax(wBuf, wNum);
			//WordToByte(wProMax, bBuf);
			wflag = wNum*5/100;
			if (wflag > 72)
			{
				wflag = 72;
			}
			memcpy(bBuf, (BYTE *)&wTmp[1+wflag], 2);

			memset(wTmp, 0, sizeof(wTmp));
			WriteItemEx(BN11, PN0, 0x0e01, (BYTE *)wTmp);
		}
		//日电压不平衡度95%概率值
		iRet = ReadItemEx(BN11, PN0, 0x0e02, (BYTE *)wTmp);
		if(iRet > 0)
		{
			wNum = BcdToDWORD((BYTE *)&wTmp[0], 2);
			//wBuf = (WORD*)&bTmp[2];
			//wProMax = GetProbabilityMax(wBuf, wNum);
			//WordToByte(wProMax, bBuf+2);
			wflag = wNum*5/100;
			if (wflag > 72)
			{
				wflag = 72;
			}
			memcpy(bBuf+2, (BYTE *)&wTmp[1+wflag], 2);

			memset(wTmp, 0, sizeof(wTmp));
			WriteItemEx(BN11, PN0, 0x0e02, (BYTE *)wTmp);
		}
		WriteItemEx(BN0, m_bPn, 0x385f, bBuf, dwOldSec);//入库
		m_wUnbITimes = 0;
		m_wUnbUTimes = 0;
	}
	if(bChgFlg == TIME_UNIT_MONTH)
	{
		//月电流不平衡度95%概率值
		iRet = ReadItemEx(BN11, PN0, 0x0e11, (BYTE *)wTmp);
		if(iRet > 0)
		{
			dwMonNum = BcdToDWORD((BYTE *)wTmp, 2);
			//wBuf = (WORD*)&bMonTmp[4];
			//wProMax = GetProbabilityMax(wBuf, dwMonNum);
			//WordToByte(wProMax, bBuf);
			wflag = wNum*5/100;
			if (wflag > 1116)
			{
				wflag = 1116;
			}
			memcpy(bBuf, (BYTE *)&wTmp[1+wflag], 2);

			memset(wTmp, 0, sizeof(wTmp));
			WriteItemEx(BN11, PN0, 0x0e11, (BYTE *)wTmp);
		}
		//月电压不平衡度95%概率值
		iRet = ReadItemEx(BN11, PN0, 0x0e12, (BYTE *)wTmp);
		if(iRet > 0)
		{
			dwMonNum = BcdToDWORD((BYTE *)&wTmp[0], 2);
			//wBuf = (WORD*)&bMonTmp[4];
			//wProMax = GetProbabilityMax(wBuf, dwMonNum);
			//WordToByte(wProMax, bBuf+2);
			wflag = wNum*5/100;
			if (wflag > 1116)
			{
				wflag = 1116;
			}
			memcpy(bBuf+2, (BYTE *)&wTmp[1+wflag], 2);

			memset(wTmp, 0, sizeof(wTmp));
			WriteItemEx(BN11, PN0, 0x0e12, (BYTE *)wTmp);
		}
		WriteItemEx(BN0, m_bPn, 0x386f, bBuf, dwOldSec);//入库
		m_dwMonUnbITimes = 0;
		m_dwMonUnbUTimes = 0;
	}
	
}


WORD CDpStat::GetProbabilityMax(WORD *wBuf,DWORD dwSize)
{
	DWORD wVal = (dwSize*95)/100;
	
	if (dwSize > 0)
		dwSize -= 1;
	if (dwSize<=0)
		return 0;
	WORD wTmp = 0;

	for(int i=dwSize-1;i>0;i--)
	{
		for(int j=0;j<i;j++)
		{
			if(wBuf[j] > wBuf[j+1])
			{
				wTmp = wBuf[j];
				wBuf[j] = wBuf[j+1];
				wBuf[j+1] = wTmp;
			}
		}
		if(i < wVal)
			break;
	}

	wTmp = wBuf[wVal];
	return wTmp;
}
//
//进行 从大到小的排序
WORD CDpStat::MakeSort(WORD *wBuf, WORD wNewData, WORD wNum)
{
	int i=0, j=0;


	if (wNewData<=wBuf[wNum-1])
	{
		return 0;
	}

	for (i=0; i<wNum; i++)
	{
		if (wNewData >= wBuf[i])
		{
			for (j=wNum-1; j>i; j--)
			{
				wBuf[j] = wBuf[j-1];
			}
			wBuf[i] = wNewData;
			break;
		}

	}

	return 0;
}
