/*********************************************************************************************************
 * Copyright (c) 2005,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Pulse.cpp
 * 摘    要：
 * 当前版本：1.0
 * 作    者：张 强
 * 完成日期：2008年7月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/
#include "Pulse.h"
#include "Energy.h"
#include "Demand2.h"		
#include "AcFmt.h"
#include "ComAPI.h"
#include "math.h"
#include "Sample.h"
#include "FaAPI.h"
#include "AcHook.h"
#include "LibAcConst.h"
#include "DbOIAPI.h"
#include "OoFmt.h"

//BYTE	g_bPulseFlag = 0xff;    	//浙江版可设为0x00
//static TTime g_tmPwrCalcu;
TPulseInData  g_PulseInData[MAX_YMNUM];

CPulseManager g_PulseManager;	//脉冲管理类

//描述:在交采执行月冻结时的回调函数,只提供交采测量点,默认测量点由本函数决定
//	   需不需要也同时执行
//参数:@wPn 交采的测量点,如果支持双测量点,则wPn指的应该是非默认测量点,
//			但在外部没有配置交采测量点的时候,wPn也可能被赋为默认测量点(如PN0)
//			在支持双测量点的情况下,如果函数判断到wPn为非默认测量点,在执行
//			wPn的相应操作的同时,也应该为默认测量点执行相应操作
//	   @time 日冻结的时标,只使用年、月、日、时几个字段	
/*void PulseOnMonthFrz(WORD wIndex, const TTime& time, BYTE bFrzIdx)
{
	if ((wIndex >= 4) || (bFrzIdx >= AUTO_DATE_NUM))
		return;

	DWORD dwTmp = MonthFrom2000(time);
	WriteItemEx(BN18, wIndex, 0x0614+bFrzIdx, (BYTE *)&dwTmp); 	//最后一次月冻结时间
	TrigerSaveBank(BN18, 0, -1); 
}*/

//描述:交采库用来判断某个月冻结是否已经冻结的回调函数,只提供交采测量点,默认测量点由本函数决定
//	   需不需要也同时执行
//参数:@wPn 交采的测量点,如果支持双测量点,则wPn指的应该是非默认测量点,
//			但在外部没有配置交采测量点的时候,wPn也可能被赋为默认测量点(如PN0)
//			在支持双测量点的情况下,如果函数判断到wPn为非默认测量点,在执行
//			wPn的相应操作的同时,也应该为默认测量点执行相应操作
//	   @time 日冻结的时标,只使用年、月、日、时几个字段	
/*bool PulseIsMonthFrozen(WORD wIndex, const TTime& time, BYTE bFrzIdx)
{
	if ((wIndex >= 4) || (bFrzIdx >= AUTO_DATE_NUM))
		return false;
	
	DWORD dwTmp = 0;
	ReadItemEx(BN18, wIndex, 0x0614+bFrzIdx, (BYTE *)&dwTmp);	//最后一次月冻结时间
	return (dwTmp == MonthFrom2000(time));
}*/


CPulse::CPulse()
{
	m_fValid = false;
	m_fStopSaveLog = false;
	//m_bPortIdx = 0;
	memset(&m_PulseCfg, 0, sizeof(TPulseCfg));
}

CPulse::~CPulse()
{
	
}


bool CPulse::Init(TPulseCfg* pPulseCfg)
{    
    m_wPn = pPulseCfg->wPn;
	if (!PulseLoadPara(pPulseCfg, &m_PulsePara))
		return false;

	ReadItemEx(BN1, PN0, 0x2040+pPulseCfg->bPortNo, (BYTE* )&m_PulseCfg);	//读取该路脉冲旧的参数配置

	if (memcmp(&m_PulseCfg, pPulseCfg, sizeof(TPulseCfg)) != 0)	//第1次上电初始化且该路脉冲配置改变
	{
	    DTRACE(DB_FA, ("CPulse::Init Cfg Change: Old Cfg wPn=%d,bPortNo=%d,bType=%d,iConst=%u.\r\n", m_PulseCfg.wPn, m_PulseCfg.bPortNo, m_PulseCfg.bType, m_PulseCfg.i64Const));
	    DTRACE(DB_FA, ("CPulse::Init Cfg Change: New Cfg wPn=%d,bPortNo=%d,bType=%d,iConst=%u.\r\nClearPulseLog.\r\n", pPulseCfg->wPn, pPulseCfg->bPortNo, pPulseCfg->bType, pPulseCfg->i64Const));	    

	    m_PulseCfg = *pPulseCfg;
	    WriteItemEx(BN1, PN0, 0x2040+pPulseCfg->bPortNo, (BYTE* )pPulseCfg);
	    TrigerSaveBank(BN1, 0, -1);

    	ClearLogBlock(pPulseCfg->bPortNo);//清除该路所在块的日志数据
	}


	m_wRate = GetRate(m_wPn);
	m_PulsePara.EnergyPara.wRate = m_wRate;
	//m_PulsePara.DemandPara.wRate = m_wRate;
	
	//memset(m_dwDayEnergy, 0, sizeof(m_dwDayEnergy));
	//memset(m_dwMonthEnergy, 0, sizeof(m_dwMonthEnergy));

	m_Energy.Init(&m_PulsePara.EnergyPara);   //多费率电能
	//m_Demand.Init(&m_PulsePara.DemandPara);   //需量

#ifdef ACLOG_ENABLE	
	m_Energy.ResetLog();	//重新初始化日志
	//m_Demand.ResetLog();	//重新初始化日志
#endif
	
	memset(m_dwPulse, 0, sizeof(m_dwPulse));
	memset(m_dwLastPulse, 0 , sizeof(m_dwLastPulse));  //上次计算时的电能脉冲数
	m_fTrigerSave = false;//在费率、月份或抄表日发生切换的时候，触发数据库去保存本测量点的数据
	//m_fClrDemand = false; //清需量标志
	m_bEnergyMinute = 0;	
	m_fValid = true;
	m_wPowerPtr = 0;
	m_fStopSaveLog = false;	//调用完清日志函数后注意清该标志，否则铁电不保存数据	

	for (WORD i = 0; i < AUTO_DATE_NUM; i++)
		m_dwLastDateMin[i] = GetCurMinute();	//上次抄表日执行的分钟

	memset(m_fDateAdjBackward, 0, sizeof(m_fDateAdjBackward));//抄表日执行时间往前调整	

	return true;
}


bool CPulse::LoadPara(void)		//参数刷新
{
	PulseLoadPara(&m_PulseCfg, &m_PulsePara);
	m_Energy.ReInit();   //多费率电能
	//m_Demand.ReInit();   //需量
	m_wRate = GetRate(m_wPn);
	m_PulsePara.EnergyPara.wRate = m_wRate;
	//m_PulsePara.DemandPara.wRate = m_wRate;
	
	return true;
}


//描述:计算脉冲功率,每分钟调用本函数一次
void CPulse::CalPower(void)
{
    WORD i;
	BYTE bBuf[8];

    //if (!m_fValid)
    //	continue;

    BYTE bPortIndex = m_PulseCfg.bPortNo - 1;
	WORD wWritePtr = g_PulseInData[bPortIndex].WritePtr;
	if (wWritePtr != m_wPowerPtr)
	{
		DWORD dwPulseNum;
		if (wWritePtr >= m_wPowerPtr)
			dwPulseNum = wWritePtr - m_wPowerPtr;
		else
			dwPulseNum = wWritePtr + TICK_BUF_LENGTH - m_wPowerPtr;
	
		dwPulseNum--;
		if (dwPulseNum == 0)	//比如3个脉冲，周期只能测出2个周期
		{
			m_i64Power = 0;
			return;
		}

		WORD wHeadPtr;
		if (wWritePtr == 0)
			wHeadPtr = TICK_BUF_LENGTH - 1;
		else
			wHeadPtr = wWritePtr - 1;  
			
		int64 tmp = m_PulseCfg.i64Const * (g_PulseInData[bPortIndex].TickBuf[wHeadPtr] - g_PulseInData[bPortIndex].TickBuf[m_wPowerPtr]);
		m_i64Power = (int64 ) 3600 * dwPulseNum * 1000000 / tmp * 10;		//FMT9为4位小数位，故放大10倍
		if (m_PulseCfg.bType > EQ_POS)	//反向功率
			m_i64Power = -m_i64Power;

		m_wPowerPtr = wWritePtr;	//消费
	}
	else
	{
		//清该路功率为0
		m_i64Power = 0;
	}
}


void CPulse::Run(bool fCalcuPwr)
{
	BYTE bBuf[4];
	if (!m_fValid)
	{
		//清该路功率为0
		m_i64Power = 0;
		return;
	}
		
	if (fCalcuPwr)
	{							
		CalPower();
	}
	
	RunMeter();
}


//描述:计算电表的电能和需量,每秒调用一次
void CPulse::RunMeter()
{
    BYTE bPortIndex, bType;
	WORD i, wTypeNum, wInnerID;
	int iDiffPulse[ENERGY_NUM_MAX];  //前后两次电能脉冲数的差
	DWORD dwDemandPulse[AC_DEMAND_NUM];
	//DWORD dwDemandTick[ENERGY_NUM_MAX];
	
	memset(iDiffPulse, 0, sizeof(iDiffPulse));
	memset(dwDemandPulse, 0, sizeof(dwDemandPulse));
	
	bType = m_PulseCfg.bType;
	bPortIndex = m_PulseCfg.bPortNo - 1;
	wTypeNum = m_PulsePara.EnergyPara.wTypeNum;
	wInnerID = m_PulsePara.EnergyPara.wInnerID[0]; //内部计算的电能ID

	m_dwPulse[wInnerID] = g_PulseInData[bPortIndex].Pulse;
	iDiffPulse[0] = m_dwPulse[wInnerID] - m_dwLastPulse[wInnerID];

	m_Energy.AddPulse(iDiffPulse);   //多费率电能
	
	/*if (m_fClrDemand)	//清需量标志,别的线程设置,交采线程去执行
	{
		m_Energy.TransferMonth();
		//m_Demand.TransferMonth();
		m_fClrDemand = false;
		m_fTrigerSave = true; //触发数据库去保存本测量点的数据
	}*/

	TTime now;
	GetCurTime(&now);
	DWORD dwTick = GetTick();
	
	if (now.nMinute != m_bEnergyMinute)   //每分钟执行一次
	{  //要在分钟的开头执行

		DWORD dwCurMin = TimeToMinutes(now);	//上次抄表日执行的分钟
		DWORD dwDate = (DWORD )now.nMonth*0x10000 + (DWORD )now.nDay*0x100 + now.nHour;

		for (i=0; i<AUTO_DATE_NUM; i++)
		{
			if (dwCurMin < m_dwLastDateMin[i])
				m_fDateAdjBackward[i] = true;			//抄表日执行时间往前调整
			m_dwLastDateMin[i] = dwCurMin;

			/*if ((dwDate&0xffff)==m_PulsePara.wAutoDate[i]) 	//当前时间处于抄表日
			{	
				if ((dwDate!=m_dwLastAutoDate[i] && !PulseIsMonthFrozen(bPortIndex, now, i)) || m_fDateAdjBackward[i])
				{		//抄表日且与上次月日时不同 && 该日没冻结过 || 抄表日执行时间往前调整
					DWORD dwTmp = 0;
					ReadItemEx(BN18, bPortIndex, 0x0614, (BYTE *)&dwTmp);	//最后一次月冻结时间
					DTRACE(DB_FA, ("CPulse::RunMeter: transfer month, bPortIndex=%d, dwTmp=%ld\r\n", 
								   bPortIndex, dwTmp));
					
					PulseOnMonthFrz(bPortIndex, now, i);
					m_Energy.TransferMonth();
					//m_Demand.TransferMonth();
					
					m_fDateAdjBackward[i] = false;
					m_dwLastAutoDate[i] = dwDate; //在复位后如果日和小时没变,会重新再转存一遍
					m_fTrigerSave = true; //在费率、月份或抄表日发生切换的时候，触发数据库去保存本测量点的数据
				}
			}*/
		}

		m_wRate = GetRate(m_wPn);
		m_PulsePara.EnergyPara.wRate = m_wRate;
		//m_PulsePara.DemandPara.wRate = m_wRate;
		m_bEnergyMinute = now.nMinute;
	}

	/*WORD wWritePtr = g_PulseInData[bPortIndex].WritePtr;
	WORD wHeadPtr;
	if (wWritePtr == 0)
		wHeadPtr = TICK_BUF_LENGTH - 1;
	else
		wHeadPtr = wWritePtr - 1;

	DWORD dwDemandTick = 0;
	wTypeNum = m_PulsePara.DemandPara.wTypeNum;
	DWORD dwDemandTick0 = g_PulseInData[bPortIndex].TickBuf[wHeadPtr];
	
	for (i=0; i<wTypeNum; i++)
	{
		wInnerID = m_PulsePara.DemandPara.wInnerID[i]; //内部计算的电能ID
		dwDemandPulse[i] = m_dwPulse[wInnerID];		
		dwDemandTick = GetTick();
		if (dwDemandTick - dwDemandTick0 > 60*1000)
			dwDemandTick = GetTick();
		else
			dwDemandTick = dwDemandTick0;			
	}		
	
	//m_Demand.CalcuDemand(dwDemandPulse, &dwDemandTick);*/
	
	/*if (m_fTrigerSave) //在费率、月份或抄表日发生切换的时候，触发数据库去保存本测量点的数据
	{	  				
		TrigerSaveBank(BN0, SECT_PN_DATA, m_wPn);	//触发保存 //TrigerSavePoint(m_wPn);
		m_fTrigerSave = false;
	}*/
	
	memcpy(m_dwLastPulse, m_dwPulse, sizeof(m_dwPulse));
}


void CPulse::SaveLog()
{
#ifdef ACLOG_ENABLE
		
	if (m_fStopSaveLog)
		return;	

	m_Energy.SaveLog();
	//m_Demand.SaveLog();
	
#endif //ACLOG_ENABLE
	
}

//清除日志数据
bool CPulse::ClearLog()
{
#ifdef ACLOG_ENABLE

	m_fStopSaveLog = true;
	
	m_Energy.ClearLog();
	//m_Demand.ClearLog();
	
#endif //ACLOG_ENABLE
	
	return true;
}


//清除日志数据
bool CPulse::ClearLogBlock(BYTE bPortNo)
{
#ifdef ACLOG_ENABLE

	m_fStopSaveLog = true;
	
	CDataLog PulseLog;
	WORD wEnergyID = LOG_PULSE_ENERGY1 + bPortNo - 1;
	//WORD wDemandID = LOG_PULSE_DEMAND1 + bPortNo - 1;

	PulseLog.ClearBlock(wEnergyID);
	//PulseLog.ClearBlock(wDemandID);
	
#endif //ACLOG_ENABLE
	
	return true;
}



//当日、当月电量统计数据及当前电能示值清0
bool CPulse::ResetData()
{
	int iLen;
	WORD i, wOI;
	BYTE bAttr;
	BYTE bBuf[80];

	wOI = OI_PULSE_BASE + m_wPn;
	for (bAttr=ATTR7; bAttr<=ATTR18; bAttr++)	//脉冲属性7~18 数据清0
	{		
		memset(bBuf, 0, sizeof(bBuf));
		iLen = OoReadAttr(wOI, bAttr, bBuf, NULL, NULL);
		if (iLen > 0)
		{
			for (i=0; i<TOTAL_RATE_NUM; i++)
			{
				memset(bBuf+9*i+3, 0, 8);	//8 高精度电能量∷=long64-unsigned 
			}

			OoWriteAttr(wOI, bAttr, bBuf);
		}
	}

	TrigerSaveBank(BN0, SECT2, -1);

	return true;
}



CPulseManager::CPulseManager()
{
    m_bPulseNum = 0;
    m_bPulsePnNum = 0;
    m_dwLastTick = 0;
    m_bYMFlag = 0xff;	//脉冲占用遥信标志位
}

CPulseManager::~CPulseManager()
{

}

bool CPulseManager::Init()
{
    int iLen = 0;
	DWORD dwOA;
    bool fPulsePnValid;
    int64 i64Const;
	WORD i, j, wPn, wID, wOI, wFmtLen=0;
	BYTE bIndex, bProp, bPulsePnIndex, bYMFlag, bCfgNum=0, bType = 0;	
	TPulseCfg PulseCfg;
    TPulseCfg* pPulseCfg = &PulseCfg;
    BYTE bBuf[PULSE_CFG_ID_LEN];
	BYTE bNum = 1, bPortNo = 1;
	BYTE* pbFmt = NULL;
	const WORD wRateNum = RATE_NUM+1;
	
	m_bPulseNum = 0;
    m_bPulsePnNum = 0;	//先清除
    m_dwLastTick = 0;
	m_dwLastStatClick = 0;
    
    memset(&g_PulseInData, 0, sizeof(g_PulseInData));
    memset(m_PulsePnDesc, 0, sizeof(m_PulsePnDesc));
	
	for (bType=0; bType<MAX_PULSE_TYPE; bType++)
	{
		wID = PULSE_HI_POSE_ID + bType;
		ReadItemEx(BN0, wPn, wID, bBuf);
		AcFmtToEng(wID, m_i64LastE[bType], bBuf, false, false, 0, wRateNum);	//当前电能示值
	}

	ReadItemEx(BN10, PN0, 0xa1bd, &bNum);
	if(bNum==0 || bNum>8)
		bNum = 1;

    for (i=0; i<MAX_YMNUM; i++)
    {
    	for (j=0; j<MAX_PULSE_TYPE; j++)
			m_PulsePnDesc[i].bIndex[j] = m_PulsePnDesc[i].bPortNo[j] = 0xff;	//设置为无效0xff

		m_Pulse[i].SetValid(false);
    }
    
    bIndex = 0;	//脉冲索引
    bPulsePnIndex = 0;	//脉冲测量点索引
    bYMFlag = 0;
    m_bYMFlag = 0xff;	//初始化为全部占用
    
	//DTRACE(DB_FA, ("CPulseManager::Init: bNum=%d\r\n", bNum));
    for (wPn=PN0; wPn<PULSE_PN_NUM; wPn++)		//脉冲计量点号<-->测量点号 映射
    {
        fPulsePnValid = false;
		wOI = OI_PULSE_BASE + wPn;
		DTRACE(DB_FA, ("CPulseManager::Init: wOI=0x%04x\r\n", wOI));
		iLen = OoReadAttr(wOI, ATTR4, bBuf, &pbFmt, &wFmtLen);	//脉冲配置参数

		#if 0
		BYTE g_bPulseCfgBuf[] = {0x01, 0x02, 0x02, 0x03, 0x51, 0x00, 0x10, 0x02, 0x00, 0x16, 0x00, 0x12, 0x00, 0x19, 
											 0x02, 0x03, 0x51, 0x00, 0x10, 0x02, 0x00, 0x16, 0x00, 0x12, 0x00, 0x19,
		};
		#endif

		bCfgNum = bBuf[1];	//脉冲实际配置路数
		if (iLen>0 && bCfgNum>0 && bCfgNum<=MAX_PULSE_TYPE)		//参数设置时需保证每个测量点最多4个类型的脉冲输入
        {
			DTRACE(DB_FA, ("CPulseManager::Init: bCfgNum=%d.\r\n", bCfgNum));
            memset(&PulseCfg, 0, sizeof(TPulseCfg));
    		for (j=0; j<bCfgNum; j++)
    		{
				dwOA = OoOadToDWord(bBuf+j*PULSE_CFG_LEN+5);	//OAD
				DTRACE(DB_FA, ("CPulseManager::Init: dwOA=0x%08x\r\n", dwOA));
				bPortNo = dwOA & 0xff;		//获取输入端口号
				if ((dwOA>>16) != OI_PULSE_INPUT)	//判断参数是否有效
					continue;

    		    pPulseCfg->wPn = wPn;
    			pPulseCfg->bPortNo = bPortNo+bNum-1;	//脉冲端口号
    			pPulseCfg->bType = bBuf[j*PULSE_CFG_LEN+10];		//脉冲属性
    			pPulseCfg->i64Const = OoLongUnsignedToWord(bBuf+j*PULSE_CFG_LEN+12);		//电表常数
				DTRACE(DB_FA, ("CPulseManager::Init: wPn=%d, bPortNo=%d, bType=%d, i64Const=%ld.\r\n", wPn, pPulseCfg->bPortNo, pPulseCfg->bType, pPulseCfg->i64Const));
    			if (pPulseCfg->bPortNo>0 && pPulseCfg->bPortNo<=MAX_YMNUM && pPulseCfg->i64Const>0 && pPulseCfg->bType<MAX_PULSE_TYPE)
    			{
					DTRACE(DB_FA, ("CPulseManager::Init2: wPn=%d, bPortNo=%d, bType=%d, i64Const=%ld.\r\n", wPn, pPulseCfg->bPortNo, pPulseCfg->bType, pPulseCfg->i64Const));
					if (m_Pulse[bIndex].Init(pPulseCfg))
					{
					    m_PulsePnDesc[bPulsePnIndex].wPn = wPn;
					    m_PulsePnDesc[bPulsePnIndex].bPortNo[pPulseCfg->bType] = pPulseCfg->bPortNo;
					    m_PulsePnDesc[bPulsePnIndex].bIndex[pPulseCfg->bType] = bIndex;
						DTRACE(DB_FA, ("CPulseManager::Init3: wPn=%d, bPortNo=%d, bType=%d, i64Const=%ld.\r\n", wPn, pPulseCfg->bPortNo, pPulseCfg->bType, pPulseCfg->i64Const));

					    fPulsePnValid = true;
					    bYMFlag |= (1<<(pPulseCfg->bPortNo-1));	//脉冲占用标志
						bIndex++;
						if (bIndex >= MAX_YMNUM)
				    		break;
					}
    			}
    		}

			if (fPulsePnValid)
			{
	    		InitPulseValToDb(bPulsePnIndex);
				bPulsePnIndex++;
				if (bPulsePnIndex >= PULSE_PN_NUM)
    				break;
			}
        }
    }

    m_bYMFlag = bYMFlag;	//脉冲占用遥信标志位
	m_bPulseNum = bIndex;	//参数有效的脉冲路数
	m_bPulsePnNum = bPulsePnIndex;	//脉冲测量点个数
	if (m_bPulseNum == 0)	//F9清脉冲配置后，需要把各路脉冲的铁电数据也清除
	{
		iLen = ReadItemEx(BN1, PN0, 0x2041, bBuf);
	    if (iLen>0 && bBuf[0]!=0)
	    {
			memset(bBuf, 0, sizeof(bBuf));
			for (i=0; i<MAX_YMNUM; i++)
				WriteItemEx(BN1, PN0, 0x2041+i, bBuf);

			TrigerSaveBank(BN1, 0, -1);
	    }
	}

	DTRACE(DB_FA, ("CPulseManager::Init: m_bPulseNum=%d\r\n", m_bPulseNum));

	return true;
}

//判断本脉冲测量点是否无效（输入端口全为0则无效）
bool CPulseManager::IsPulsePnInvalid(TPulsePnDesc* pPnDesc)
{
	BYTE bType;
	for (bType=0; bType<MAX_PULSE_TYPE; bType++)
	{
		if (pPnDesc->bPortNo[bType] != 0)
			return false;
	}

	return true;
}

void CPulseManager::CalcPwr()
{
	WORD wPnIndex;
	int64 i64PosPower, i64NegPower, i64PosNPower, i64NegNPower;
    BYTE bPosEpIndex, bNegEpIndex, bPosEqIndex, bNegEqIndex;
	int iVal[PULSE_VAL_NUM];
	
	for (wPnIndex=0; wPnIndex<m_bPulsePnNum; wPnIndex++)	//脉冲测量点序号
	{
	    TPulsePnDesc* pPnDesc = &m_PulsePnDesc[wPnIndex];
		if (IsPulsePnInvalid(pPnDesc))
			continue;

		i64PosPower = 0;
		i64NegPower = 0;

   		bPosEpIndex = pPnDesc->bIndex[EP_POS];
   		bNegEpIndex = pPnDesc->bIndex[EP_NEG];
   		
   		if (bPosEpIndex < MAX_YMNUM)
   			i64PosPower = m_Pulse[bPosEpIndex].CurPower();	//该路总有功功率
   		if (bNegEpIndex < MAX_YMNUM)
   			i64NegPower = m_Pulse[bNegEpIndex].CurPower();	//该路总有功功率

   		if (i64PosPower != 0)
   			iVal[0] = i64PosPower;
   		else if (i64NegPower != 0)
   			iVal[0] = i64NegPower;
   		else
   			iVal[0] = 0;
   			
   		bPosEqIndex = pPnDesc->bIndex[EQ_POS];
   		bNegEqIndex = pPnDesc->bIndex[EQ_NEG];

   		i64PosNPower = 0;
   		i64NegNPower = 0;

   		if (bPosEqIndex < MAX_YMNUM)
   			i64PosNPower = m_Pulse[bPosEqIndex].CurPower();	//该路总有功功率
   		if (bNegEqIndex < MAX_YMNUM)
   			i64NegNPower = m_Pulse[bNegEqIndex].CurPower();	//该路总有功功率

   		if (i64PosNPower != 0)
   			iVal[1] = i64PosNPower;
   		else if (i64NegNPower != 0)
   			iVal[1] = i64NegNPower;
	    else
	    	iVal[1] = 0;

	    PulseValToDb(wPnIndex, iVal);
	}
}

void CPulseManager::CalcPnStatEnergy(WORD wPnIndex, bool fClrDayEnergy, bool fClrMonthEnergy)
{	
	BYTE bType = 0;
	WORD i, wPn, wID;
	int64 i64E[MAX_PULSE_TYPE][RATE_NUM+1]; //与数据库对应的电能
	int64 i64DeltaE[MAX_PULSE_TYPE][RATE_NUM+1];
	WORD wEDayID[] = { 0x2406, 0x2410, 0x2408, 0x2412 };
	WORD wEMonthID[] = { 0x2407, 0x2411, 0x2409, 0x2413 };
	WORD wStartDayID[] = {0x0d01, 0x0d05, 0x0d03, 0x0d07 };
	WORD wStartMonthID[] = {0x0d02, 0x0d06, 0x0d04, 0x0d08 };

	BYTE bBuf[80];
	int64 i64LastDayE[MAX_PULSE_TYPE][RATE_NUM+1];		//当日起点电量
	int64 i64LastMonthE[MAX_PULSE_TYPE][RATE_NUM+1];	//当月起点电量
	const WORD wRateNum = RATE_NUM+1;
	TPulsePnDesc* pPnDesc = NULL;
	TTime tmNow, tmLastStat;

	GetCurTime(&tmNow);
	pPnDesc = &m_PulsePnDesc[wPnIndex];
	if (IsPulsePnInvalid(pPnDesc))
	{
		DTRACE(DB_FA, ("CPulseManager::CalcPnStatEnergy: wPnIndex=%d invalid, return\r\n", wPnIndex));
		return;
	}

	wPn = pPnDesc->wPn;
	ReadItemEx(BN11, wPn, 0x0d00, bBuf);
	Fmt15ToTime(bBuf, tmLastStat);
	if (IsDiffDay(tmLastStat, tmNow))
	{
		DTRACE(DB_FA, ("CPulseManager::CalcPnStatEnergy: wPn=%d day chg.\r\n", wPn));
		for (bType=0; bType<MAX_PULSE_TYPE; bType++)
		{
			wID = PULSE_HI_POSE_ID + bType;
			ReadItemEx(BN0, wPn, wID, bBuf);

			WriteItemEx(BN11, wPn, wStartDayID[bType], bBuf);		//更新日起点值
		}

		fClrDayEnergy = true;

		if (!IsSameMon(tmLastStat, tmNow))
		{
			for (bType=0; bType<MAX_PULSE_TYPE; bType++)
			{
				wID = PULSE_HI_POSE_ID + bType;
				ReadItemEx(BN0, wPn, wID, bBuf);
				WriteItemEx(BN11, wPn, wStartMonthID[bType], bBuf);		//更新月起点值
			}

			fClrMonthEnergy = true;
		}

		tmLastStat = tmNow;
		TimeToFmt15(tmLastStat, bBuf);
		WriteItemEx(BN11, wPn, 0x0d00, bBuf);	//更新最近一次统计时间

		TrigerSaveBank(BN11, 0, -1);
	}

	//读取当日/当月起点值
	for (bType=0; bType<MAX_PULSE_TYPE; bType++)
	{	
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN11, wPn, wStartDayID[bType], bBuf);
		AcFmtToEng(wStartDayID[bType], i64LastDayE[bType], bBuf, false, false, 0, wRateNum);	//当日起点电能示值
		if (bType == 0)
			DTRACE(DB_FA, ("wStartDayID[bType]=%d, i64LastDayE[bType][0]=%lld, i64LastDayE[bType][1]=%lld, i64LastDayE[bType][2]=%lld, , i64LastDayE[bType][3]=%lld.\r\n",  wStartDayID[bType], i64LastDayE[bType][0], i64LastDayE[bType][1], i64LastDayE[bType][2], i64LastDayE[bType][3]));

		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN11, wPn, wStartMonthID[bType], bBuf);
		AcFmtToEng(wStartMonthID[bType], i64LastMonthE[bType], bBuf, false, false, 0, wRateNum);	//当月起点电能示值		
	}

	//读取当前值，并用当前值-起点值，计算出差值
	for (bType=0; bType<MAX_PULSE_TYPE; bType++)
	{
		wID = PULSE_HI_POSE_ID + bType;
		ReadItemEx(BN0, wPn, wID, bBuf);
		AcFmtToEng(wID, i64E[bType], bBuf, false, false, 0, wRateNum);	//当前电能示值
		if (bType == 0)
			DTRACE(DB_FA, ("wCurID = %04x , i64E[bType][0]=%lld, i64E[bType][1]=%lld, i64E[bType][2]=%lld, i64E[bType][3]=%lld.\r\n",  wID, i64E[bType][0], i64E[bType][1], i64E[bType][2], i64E[bType][3]));

		if (memcmp(i64E[bType], m_i64LastE[bType], wRateNum*sizeof(int64))!=0 || fClrDayEnergy || fClrMonthEnergy)		//当日/当月有变化
		{
			DTRACE(DB_FA, ("step000, bType=%d, fClrDayEnergy=%d, fClrMonthEnergy=%d.\r\n",  bType, fClrDayEnergy, fClrMonthEnergy));
			//当日电量计算
			if (fClrDayEnergy)
			{
				memset(i64DeltaE[bType], 0, wRateNum*sizeof(int64));
			}
			else
			{
				for (i=0; i<wRateNum; i++)
				{					
					if (i64E[bType][i] > i64LastDayE[bType][i])
						i64DeltaE[bType][i] = i64E[bType][i]-i64LastDayE[bType][i];
					else
						i64DeltaE[bType][i] = 0;

					DTRACE(DB_FA, ("bType=%d i64E[%d][i]=%lld, i64LastDayE=%lld, i64DeltaE=%lld.\r\n",  bType, bType, i, i64E[bType][i], i64LastDayE[bType][i], i64DeltaE[bType][i]));
				}
			}

			memset(bBuf, 0, sizeof(bBuf));
			AcEngToFmt(wEDayID[bType], i64DeltaE[bType], bBuf, false, false, wRateNum);
			DTRACE(DB_FA, ("step111 wPn=%d, wEDayID[%d]=%04x, i64DeltaE[0]=%lld, i64DeltaE[1]=%lld, i64DeltaE[2]=%lld, i64DeltaE[3]=%lld.\r\n", wPn, bType, wEDayID[bType], i64DeltaE[bType][0], i64DeltaE[bType][1], i64DeltaE[bType][2], i64DeltaE[bType][3]));
			//DTRACE(DB_FA, ("step222 wPn=%d, bBuf[0]=%lld, i64DeltaE[1]=%lld, i64DeltaE[2]=%lld, i64DeltaE[3]=%lld.\r\n", wPn, i64DeltaE[bType][0], i64DeltaE[bType][1], i64DeltaE[bType][2], i64DeltaE[bType][3]));
			//TraceBuf(DB_FA, "step333-> ", (BYTE*)bBuf, 27);
			WriteItemEx(BN0, wPn, wEDayID[bType], bBuf);

			//当月电量计算
			if (fClrMonthEnergy)
			{
				memset(i64DeltaE[bType], 0, wRateNum*sizeof(int64));
			}
			else
			{
				for (i=0; i<wRateNum; i++)
				{
					if (i64E[bType][i] > i64LastMonthE[bType][i])
						i64DeltaE[bType][i] = i64E[bType][i]-i64LastMonthE[bType][i];
					else
						i64DeltaE[bType][i] = 0;
				}
			}

			memset(bBuf, 0, sizeof(bBuf));
			AcEngToFmt(wEMonthID[bType], i64DeltaE[bType], bBuf, false, false, wRateNum);
			WriteItemEx(BN0, wPn, wEMonthID[bType], bBuf);

			memcpy(m_i64LastE[bType], i64E[bType], wRateNum*sizeof(int64));
		}
	}
}

//当日&当月电量统计
void CPulseManager::CalcStatEnergy()
{	
	WORD wPnIndex = 0;	
	bool fClrDayEnergy = false, fClrMonthEnergy = false;

	for (wPnIndex=0; wPnIndex<m_bPulsePnNum; wPnIndex++)	//脉冲测量点序号
		CalcPnStatEnergy(wPnIndex, fClrDayEnergy, fClrMonthEnergy);
}



void CPulseManager::Run()
{
	WORD i;	

	if (GetInfo(INFO_PULSE))	//脉冲配置参数8903更改
	{
	    Init();
	    for (i=0; i<m_bPulseNum; i++)
			m_Pulse[i].LoadPara();

		SetInfo(INFO_YX_PARA);
	}

	if (GetInfo(INFO_PULSEDATA_RESET))	//收到复位数据命令
	{
		ResetPulseData();
		SetInfo(INFO_PULSE);	//重新初始化
		return;
	}

	TTime now;
	GetCurTime(&now);
	bool fCalcuPwr = false;		//让所有功率的更新都同步
	if (GetTick()-m_dwLastTick >= 60*1000)
	{ //上电超过30秒才能算
		m_dwLastTick = GetTick();
		fCalcuPwr = true;
	}

	for (i=0; i<m_bPulseNum; i++)
	{
		m_Pulse[i].Run(fCalcuPwr);
	}
	
	if (fCalcuPwr)
		CalcPwr();	//功率入库

	if (GetClick()-m_dwLastStatClick >= 3)
	{
		CalcStatEnergy();
		m_dwLastStatClick = GetClick();
	}
}


//铁电存储脉冲电量和需量
void CPulseManager::SaveLog()
{
#ifdef ACLOG_ENABLE
	WORD i;

    for (i=0; i<m_bPulseNum; i++)
	{
		m_Pulse[i].SaveLog();
	}
#endif //ACLOG_ENABLE
}


void CPulseManager::ClearLog()
{
#ifdef ACLOG_ENABLE
	WORD i;

    for (i=0; i<m_bPulseNum; i++)
	{
		m_Pulse[i].ClearLog();
	}
#endif //ACLOG_ENABLE
}


bool  CPulseManager::ResetPulseData()
{
	WORD i, wPn, wPnIndex;
	bool fTrigSave = false;
	BYTE bBuf[10];

	for (wPn=PN0; wPn<PULSE_PN_NUM; wPn++)		//脉冲计量点号<-->测量点号 映射
	{
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN11, wPn, 0x0b14, bBuf);
		if (bBuf[0]==wPn && bBuf[1]==0xA5)		//复位命令有效
		{
			for (i=0; i<m_bPulseNum; i++)
			{
				if (m_Pulse[i].IsValid() && m_Pulse[i].GetPn()==wPn)	//找出该测量点下的有效脉冲对象
				{
					m_Pulse[i].ClearLog();		//清铁电日志
					m_Pulse[i].ResetData();		//清统计中间数据

					m_Pulse[i].SetValid(false);		//防止清0后的数据被覆盖
				}
			}

			for (wPnIndex=0; wPnIndex<m_bPulsePnNum; wPnIndex++)	//脉冲测量点序号
			{
				TPulsePnDesc* pPnDesc = &m_PulsePnDesc[wPnIndex];
				if (pPnDesc->wPn == wPn)
				{
					DTRACE(DB_FA, ("CPulseManager::ResetPulseData: clr wPn=%d stat data.\r\n", wPn));
					CalcPnStatEnergy(wPnIndex, true, true);
				}
			}

			memset(bBuf, 0, sizeof(bBuf));
			WriteItemEx(BN11, wPn, 0x0b14, bBuf);	//清除命令
			fTrigSave = true;
		}
	}

	if (fTrigSave)
		TrigerSaveBank(BN11, 0, -1); //触发保存

	return true;
}


//方法1：复位
int OnResePulseCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bPn;
	BYTE bBuf[10];

	if (GetOiClass(wOI)!=IC12 || bMethod!=OMD_PULSE_RESET)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}
	
	bPn = bBuf[0] = (wOI - OI_PULSE_BASE);
	if (bPn >= PULSE_PN_NUM)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	bBuf[1] = 0xA5;	//有效标志
	WriteItemEx(BN11, bPn, 0x0b14, bBuf);
	TrigerSaveBank(BN11, 0, -1); //触发保存

	SetInfo(INFO_PULSEDATA_RESET);
	*pbRes = 0;	//成功  （0） 返回结果
	return 0;
}

//方法2：执行
//空函数
int OnRunPulseCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	if (GetOiClass(wOI)!=IC12 || bMethod!=OMD_PULSE_RUN)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}
	
	//nothing to do

	*pbRes = 0;	//成功  （0） 返回结果
	return 0;
}


//BYTE bCmpBuf[] = {0x02, 0x03, 0x51, 0x00, 0x10, 0x02, 0x00, 0x16, 0x00, 0x12, 0x00, 0x19, 
//方法3：添加脉冲输入单元
//参数∷=脉冲输入单元
int OnAddPulseCfgCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i, j;
	int iLen;
	bool fTypeAlreadyExist = false;
	BYTE bCfgNum;
	BYTE bBuf[PULSE_CFG_ID_LEN];
		
	if (GetOiClass(wOI)!=IC12 || bMethod!=OMD_PULSE_ADDCFG || pbPara[0]!=DT_STRUCT || pbPara[1]!=3)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}	

	// 读取关联属性表
	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, ATTR4, bBuf, NULL, NULL);	//读取脉冲配置
	if (iLen <= 0)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}
	
	bCfgNum = bBuf[1];	
	for (i=0; i<bCfgNum; i++)	//遍历是否已经存在 bBuf+j*PULSE_CFG_LEN+5
	{
		if (FieldCmp(DT_PULSE_CFG, &bBuf[i*PULSE_CFG_LEN + 3], DT_PULSE_CFG, pbPara+1) == 0)	//完全相同，则认为是无效参数,保证OAD唯一性
		{
			*pbRes = 3;	//拒绝读写 （3）
			return -1;
		}
		else if (FieldCmp(DT_PULSE_CFG, &bBuf[i*PULSE_CFG_LEN + 3], DT_OAD, pbPara+OFFSET_PULSE_PORT) == 0)	//OAD相同，但脉冲属性或脉冲常数不同,修改
		{
			fTypeAlreadyExist = false;
			for (j=0; j<bCfgNum; j++)	//遍历是否已经存在 bBuf+j*PULSE_CFG_LEN+5
			{
				if (j == i)
					continue;

				if (bBuf[j*PULSE_CFG_LEN + 10] == pbPara[OFFSET_PULSE_TYPE])	//该属性已经存在 +10为属性的偏移
				{
					fTypeAlreadyExist = true;
					*pbRes = 3;	//拒绝读写 （3）
					return -1;
				}
			}

			if (!fTypeAlreadyExist)	//该脉冲属性没设置过
			{
				memcpy(&bBuf[i*PULSE_CFG_LEN + 2], pbPara, PULSE_CFG_LEN);	//修改关联对象参数
				break;
			}
		}
	}

	// 新添加一个脉冲输入
	if (i == bCfgNum)
	{
		if (bCfgNum >= MAX_PULSE_TYPE)		//已经存满了 返回失败
		{
			*pbRes = 3;	//拒绝读写 （3）
			return -1;
		}

		fTypeAlreadyExist = false;
		for (j=0; j<bCfgNum; j++)	//遍历是否已经存在 bBuf+j*PULSE_CFG_LEN+5
		{
			if (bBuf[j*PULSE_CFG_LEN + 10] == pbPara[OFFSET_PULSE_TYPE])
			{
				fTypeAlreadyExist = true;	//该属性已经存在
				*pbRes = 3;	//拒绝读写 （3）
				return -1;
			}
		}

		if (!fTypeAlreadyExist)	//该脉冲属性没设置过
		{
			memcpy(&bBuf[i*PULSE_CFG_LEN + 2], pbPara, PULSE_CFG_LEN);
			bCfgNum++;
			bBuf[1] = bCfgNum;	//数组元素个数
		}		
	}

	if (OoWriteAttr(wOI, ATTR4, bBuf) <= 0)		//设置脉冲配置
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}
	else
	{		
		*pbRes = 0;	//成功  （0）// 返回结果
		return 1;
	}
}
int BatAddPulseCfgCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD wLen;
	const ToaMap *pOadMap;
	BYTE *pbFeildFmt;
	WORD wFeildLen;
	BYTE bPnNum, bType;
	int iTmpLen = iParaLen;
	BYTE* pbTmp = pbPara;
	DWORD dwOAD;
	pbTmp++; //数组类型
	bPnNum = *pbTmp++; //元素个数
	iTmpLen -= 2;
	dwOAD = wOI;
	dwOAD = (dwOAD<<16) + 0x0400;
	pOadMap = GetOIMap(dwOAD);
	for (BYTE i=0; i<bPnNum; i++)
	{
		pbTmp = OoGetField(pbPara, pOadMap->pFmt, pOadMap->wFmtLen, i, &wLen, &bType, &pbFeildFmt, &wFeildLen);
		if (OnAddPulseCfgCmd(wOI, bMethod, bOpMode, pbTmp, wLen, pvAddon, pbFeildFmt, wFeildLen, pbRes) == 1)
		{
			iTmpLen -= wLen;
		}
		else
		{
			return -1;
		}
	}
	if (iTmpLen == 0)
		return 0;
	else
		return -1;
}

//方法4：删除脉冲输入单元
//参数∷=脉冲输入端口号OAD
int OnDelPulseCfgCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i, j;
	int iLen;
	BYTE bCfgNum;
	BYTE bBuf[PULSE_CFG_ID_LEN];
		
	if (GetOiClass(wOI)!=IC12 || bMethod!=OMD_PULSE_DELCFG || pbPara[0]!=DT_OAD)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, ATTR4, bBuf, NULL, NULL);	//读取脉冲配置
	if (iLen<=0 || bBuf[1]==0)	//空的
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	if (bBuf[1] > MAX_PULSE_TYPE)
		bCfgNum = MAX_PULSE_TYPE;
	else
		bCfgNum = bBuf[1];

	for (i=0; i<bCfgNum; i++)	//遍历是否已经存在
	{
		if (FieldCmp(DT_PULSE_CFG, &bBuf[i*DT_FRZRELA_LEN + 3], DT_OAD, pbPara+1) == 0)	//找到目标
		{
			memset(&bBuf[i*PULSE_CFG_LEN + 2], 0, PULSE_CFG_LEN);
			bBuf[1]--;	//数组元素个数
			break;
		}
	}
	
	if (i == bCfgNum)	//没找到
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	for (j=i; j<bCfgNum-1; j++)		//后面的参数往前挪
	{
		memcpy(&bBuf[j*PULSE_CFG_LEN + 2], &bBuf[(j+1)*PULSE_CFG_LEN + 2], PULSE_CFG_LEN);
	}

	// 刷新关联属性表
	if (OoWriteAttr(wOI, ATTR4, bBuf) <= 0)		//设置脉冲配置
	{
		*pbRes = 3;			//写失败 （3）
		return -1;
	}
	else
	{
		*pbRes = 0;		//成功  （0） 返回结果
		return 1;
	}
}

