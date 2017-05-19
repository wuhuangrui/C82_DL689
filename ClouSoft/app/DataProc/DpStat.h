/***********************************************************************************************
* Copyright (c) 2007,深圳科陆电子科技股份有限公司
* All rights reserved.
* 
* 文件名称：DpStat.h
* 摘    要: 本文件提供2类数据统计功能接口
* 当前版本：1.0
* 作    者：杨 凡
* 完成日期：2007年8月
* 备    注：
***********************************************************************************************/
#ifndef DPSTAT_H
#define DPSTAT_H
#include  "ComStruct.h"
//#include  "MeterSched.h"

#define STAT_NUM 10


typedef struct 
{
	WORD	wInterV;			//间隔值
	WORD	wInterU;			//间隔单位
	BYTE	bPrio;				//优先级
	WORD	wReqIdNum;
	WORD	wReqId[4];
//	WORD	wSwapNum;			//统计id个数
//	WORD	wSwapBN11Id[4];		//与BN0对应的临时id个数
//	WORD	wSwapBN0Id[4];		//与BN11对应的临时id个数

	BYTE	bStatID;			//统计类型的ID
	char	*pszName;			//统计的名称
	WORD	wPnType;			//需要执行本统计的测量点类型
	
}TStatCtrl;

/*
typedef struct 
{
	WORD wDayNum;
	WORD wDayBN11Id[4];
	WORD wDayBN0Id[4];	
	WORD wMonNum;
	WORD wMonBn11Id[4];
	WORD wMonBn0Id[4];

}TSwapInfo;*/

typedef struct 
{
	WORD wID;
	const BYTE* pbFmtStr;		//格式描述串
}TIdFmtStr;


class CDpStat
{
public:
	CDpStat(void);
	virtual ~CDpStat();

	//传入测试点编号 测量点数据
	void  Init(BYTE  bPn);
	void  DoDataStat();

	void  DoMtrParaChg();
	void  SetMtrInterv(WORD wInterv);

	BYTE  GetOldPnProp() {	return m_bPnProp; };	

	//-------------------------------------------------------------------
	WORD			m_wMidTimes; //谐波统计次数
	WORD			m_wUnbITimes;
	WORD			m_wUnbUTimes;
	DWORD			m_dwMonUnbITimes;
	DWORD			m_dwMonUnbUTimes;
	void			DoUnbalanceTransfer(BYTE bChgFlg, DWORD dwSize);
	void            DoHamornicTransfer(DWORD dwOldSec);//谐波统计入库
	WORD            GetProbabilityMax(WORD *wBuf,DWORD dwSize);
	WORD			MakeSort(WORD *wBuf, WORD wNewData, WORD wNum);
	//---------------------------------------------------------------------

private:
		
	void  LoadLimitPara(BYTE bLimitType = 0);


	void  DoPowerStat( int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType );
	
	void  DoDemandStat( int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType );

	void  DoVoltStat(int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType);

	void  DoUnbalanceStat( int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType );

	void  DoCurrentStat( int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType );

	void  DoAppPowerStat( int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType );

	 //缺省判断时间为5分钟
	void  PhaseBreakStat( int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM, int DefJudgeTime = 5);  

	void SwapBank21(BYTE bPn, WORD wBnOldID, WORD wBnNewID, DWORD dwOldSec);

	void SwapBank32( BYTE bPn, WORD wBn11ID, WORD wBn0ID, DWORD dwOldSec );

	void SwapBank64( BYTE bPn, WORD wBn11ID, WORD wBn0ID, DWORD dwOldSec );

	//无功补偿数据
	void  DoCosStat(int *piData,	int iLen,	DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType);
	void  DoHarmonicStat(int *piData,	int iLen,	DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType);	
	
	//直流模拟量
	void  DoDcStat(int *piData,	int iLen,	DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType);	
	void  SetDcDataInvalid(int *piDcDay, int *piDcMon,DWORD dwCurMin);

	//负载率统计
	void DoLoadStat(int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType);

	DWORD GetStartM(DWORD dwCurTimeM, int iType);
	DWORD GetEndM(	DWORD dwCurTimeM, int iType);


	//终端数据
//	void  TermPowerSupplyRstStat();
//	void  TermDayRstTimesStat();
//	void  TermDayMonCtrlStat();

	//总加组数据
//	void  GTActPowerDayMonStat();
//	void  GTActEnergyDayMonStat();
//	void  GTReactEnergyDayMonStat();
	
	//总加组越限统计
//	void  GTPowerViolationStat();

	//谐波监测数据
//	void  UVWPhaseHarmoVoltStat();
//	void  UVWPhaseHarmoCurrentStat();
//	void  UVWHarmoWaveViolationStat();

	
	//void			SchedParaInit(TSchedPara *pSchedpara,TStatCtrl *pStatCtrl, BYTE bPn);
	int				GetAverage(int *pBuf, WORD wLen);
	int				GetUnbValue(int iAvr, int iVal);
	void			DayChange(BYTE bStatID, DWORD dwOldDay);
	void			MonthChange(BYTE bStatID, DWORD dwOldMon);
	bool			IsClearData(int iStatId,BYTE bDayOrMon = 0);
	void			SetClearFlag(int iStatId,bool fZero,BYTE bDayOrMon = 0xff);
	WORD			GetStatIntervV(int iStatId);
	void			AddStatTime(WORD wBn11ID, int* piValBuf);
	void			SetZeroStartTime(WORD wBn11ID, int* piValBuf, DWORD dwSec);
	bool			CheckDataIsVaild(BYTE bStatID, DWORD dwSec);
	BYTE			GetStatIdFromBN11(WORD wBn11ID);

	int		  		m_iUpUpLimitAP;				//视在功率上上限
	int         	m_iUpLimitAP;				//视在功率上限

	int         	m_iUpUpLimitVolt;			//电压上上限
	int         	m_iUpLimitVolt;				//电压合格上限

	int         	m_iLowLowLimitVolt;			//电压下下限
	int         	m_iLowLimitVolt;			//电压合格下限

	int         	m_iUpUpLimitCur;			//电流上上限
	int         	m_iUpLimitCur;				//电流上限

	int         	m_iUpLimitZeroCur;			//零序电流上限

	int         	m_iUnbalanceVoltLimit;		//三相电压不平衡限值
	int         	m_iUnbalanceCurLimit;		//三相电流不平衡限值

	int         	m_iPhaseBreakLimit;			//电压断相门限

	int         	m_iFactorLimit1;			//功率限值参数1
	int         	m_iFactorLimit2;			//功率限值参数2

	int				m_iDcUpHold;				//直流模拟量的上限
	int				m_iDcDownHold;				//直流模拟量的下限

	int         	m_iRatedVolt;				//额定电压
	int         	m_iRatedCur;				//额定电流

	int				m_iHarmonicLimit[40+2];		//谐波越限参数

	TTime         	m_tmNow;					//当前时间
	TTime         	m_tmLast[STAT_NUM];			//上一个执行时间，用于日月切换判断用

	WORD			m_wBn;
	BYTE			m_bCount;					//计数器
	BYTE			m_bPn;						//测量点号（或总加组号）	
	BYTE			m_bPnProp;					//保存测试点的属性
	DWORD			m_dwLastMin;				//上一次做统计的时间
	//CMeterSched  	m_MeterSched[STAT_NUM];

	//位D0控制状态是否统计 D1记录过日清零	 D2 记录过月清零 	
	BYTE  			m_bStatValid[STAT_NUM];	
	DWORD			m_dwStatPriM[STAT_NUM];		//记录上一次统计的时间，单位分钟
	DWORD			m_dwLastTotalIntvM[STAT_NUM];	//记录上一次统计的累计时间，单位分钟

	bool			m_fZeroValid;				//0点是填无效数据还是有效数据的标志 

	bool			m_fDayChgFlg[STAT_NUM];		//对时跨日切换的标志

	bool			m_fFisrtDoLoad;

#ifdef SYS_WIN
	void			DebugHarmonic(DWORD dwSec);
	void			DebugDc(DWORD dwSec);
	void			DebugPhaseAngle(DWORD dwSec);
	void			DebugAcEng(DWORD dwSec);	
	void			DebugPulseEng(DWORD dwSec);
	void			DebugData(DWORD wID, DWORD dwSec);
#endif
	
};


#endif //DPSTAT_H

