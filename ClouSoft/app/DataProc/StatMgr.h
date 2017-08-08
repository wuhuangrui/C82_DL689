/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：StatMgr.h
 * 摘    要：本文件主要实现终端统计信息及数据统计类的管理
 * 当前版本：1.0
 * 作    者：杨凡、李湘平
 * 完成日期：2008年7月
 *********************************************************************************************************/
#ifndef STATMGR_H
#define STATMGR_H
#include "DpStat.h"
#include "DataLog.h"
#include  "DbConst.h"
#include "CctTaskMangerOob.h"

#define POWER_AVG_NUM_MAX 	60	//平均功率最大允许个数

#define STAT_OAD_NUM						20	//一个统计方案的最大OAD个数
#define RW_ATTR_RES						2	//操作统计类ID的结果属性
#define RW_ATTR_RELA						3	//操作统计类ID的参数属性
#define RW_ATTR_FRZ						8	//操作统计类ID的冻结属性
typedef struct{
	TTime tmLastRun;		//最后一次运行的时间,用来重新上电的时候判断日月是否发生切换
	WORD wDayRstStart;		//终端当日复位次数起始值,日切换的时候用来算当日复位次数
	WORD wMonRstStart;		//终端当月复位次数起始值,月切换的时候用来算当月复位次数
	DWORD wDayPowerTime;		//终端当日供电时间,计算时按秒，存储系统库时转成分
	DWORD wMonPowerTime;		//终端当月供电时间,计算时按秒，存储系统库时转成分
	DWORD dwDayFlux;		//终端GPRS日流量
	DWORD dwMonFlux;		//终端GPRS月流量
}TTermStatInfo;				//终端统计消息,每分钟写入到文件系统中

/*typedef struct {
	DWORD	dwMoniSecs;    //监测时间
	WORD   wQualifiedRate;  //合格率
	WORD   wOverRate;  	//超限率
	DWORD  dwUpperSecs;  //超上限时间
	DWORD  dwLowerSecs;  //超下限时间
} TVoltStat; //电压合格率
*/
typedef struct {
	//协议要求数据
	DWORD	dwMoniSecs;    //监测时间，单位秒
	WORD	wQualRate;  	//合格率
	WORD   wOverRate;  	//超限率
	DWORD  dwUpperSecs;  //超上限时间，单位秒
	DWORD  dwLowerSecs;  //超下限时间，单位秒

	//中间数据，需要保存到中间数据ID
	DWORD dwQualSecs;	//合格时间，单位秒
	DWORD dwOverSecs;	//超限时间，包括越上限越下限，同时发生只计一次单位秒
	//DWORD dwCurCycle;  	//当前周期的时间值
	DWORD dwLastSample;	//上次采样时间点
} TVoltStat; //电压合格率




typedef struct {
	WORD	wAssesUpLimit;  //电压考核上限
	WORD	wAssesLowLimit; //电压考核下限
	WORD   wQualUpLimit;	//电压合格上限
	WORD   wQualLowLimit;//电压合格下限
} TVoltStatPara;  //电压合格率参数




typedef struct {
	TVoltStat dayStat;  //当日电压合格率
	TVoltStat monStat;  //当月电压合格率
}TPhaseVoltStat;  //总及分相电压合格率

typedef struct {
	TTime 	tmCurCycle;  	//当前周期的时间值
	TTime	tmLastSample;	//上次采样时间点
} TStatExeCtrl;  //统计的执行控制

//区间统计
#define INTV_STAT_PARA_NUM	10	//区间统计的越限判断参数的个数
#define INTV_STAT_RES_NUM (INTV_STAT_PARA_NUM+1)	//区间统计的越限判断参数的个数
typedef struct {
	DWORD 	dwOAD;  //关联对象属性描述符OAD
	WORD	wParaNum;	//越限判断参数的个数
	int  		iParaVal[INTV_STAT_PARA_NUM];   //越限判断参数  array Data
	BYTE 	bCycleValue;		//统计周期  unsigned
	TTimeInterv tiFreq;			//统计频率  TI
} TIntvStatRela;  //一个区间统计关联对象

typedef struct {
	DWORD 	dwTotalSecs;   //累计时间
	DWORD 	dwTimes;  	//累计次数
} TIntvVal;  //一个区间统计值

typedef struct {
	DWORD 	dwOAD;  	//关联对象属性描述符OAD
	WORD	wIntvNum;	//区间的个数
	TIntvVal	intvVal[INTV_STAT_RES_NUM];   //区间统计值 array 一个统计区间
} TIntvStatRes;  //一个区间统计结果

//累加平均统计
typedef struct {
	DWORD 		dwOAD;  	//关联对象属性描述符OAD
	BYTE 		bCycleValue;	//统计周期  unsigned
	TTimeInterv 	tiFreq;		//统计频率  TI
} TStatRela;  //一个通用的统计关联对象

typedef struct {
	DWORD 	dwOAD;  	//关联对象属性描述符OAD
	int		iAcc;		//累加和  instance-specific
	int		iAvg;   	//平均值  instance-specific
} TAccAvgStatRes;  //一个累加平均统计结果
//极值统计
typedef struct {
	DWORD 	dwOAD;  	//对象属性描述符OAD
	int		iMax;		//最大值  instance-specific
	TTime	tmMax;		//最大值发生时间  date_time_s
	int		iMin;   		//最小值  instance-specific
	TTime	tmMin;		//最大值发生时间  date_time_s
} TExtremStatRes;  //一个极值统计结果


class CStatMgr
{
public:
	CStatMgr();
	~CStatMgr();

	bool Init();
	bool DoDataStat();
	bool SaveTermStat();

	void AddFlux(DWORD dwLen);
	void DoVoltStat(void);
	void IntvStat();
	bool IsCycleSwitch(TTime tmLastCycle, TTime tmNow, BYTE bUnit, WORD wValue, DWORD * dwIntVSec, BYTE * bMoreCycle);
	DWORD OoOadToDWordLen(BYTE* pbBuf, BYTE bLen);
	void AvreStat();

	void LoadStatRela();
	void SavePhaseVoltStat(WORD wOI, BYTE bAttr, TPhaseVoltStat* pPhaseVoltStat);
	void LoadPhaseVoltStat(WORD wOI, TPhaseVoltStat* pPhaseVoltStat);
	void SaveIntvStatRes(WORD wOI, BYTE bAttr, TIntvStatRes * pRes, BYTE wStatNum);
	bool LoadIntvStatRela(WORD wOI, TIntvStatRela* pRela, BYTE*pwStatNum);
	void LoadIntvStatRes(WORD wOI, TIntvStatRes* pRes, BYTE wStatNum);
	void AccAvgStat();
	void SaveAvgStatRes(WORD wOI, BYTE bAttr, TAccAvgStatRes * pRes, BYTE wStatNum);
	bool LoadAvgStatRela(WORD wOI, TStatRela* pRela, BYTE*pwStatNum);
	void LoadAvgStatRes(WORD wOI, TAccAvgStatRes* pRes, BYTE wStatNum);
	void ExtremStat();
	void SaveExtremStatRes(WORD wOI, BYTE bAttr, TExtremStatRes * pRes, BYTE wStatNum);
	void LoadExtremStatRes(WORD wOI, TExtremStatRes* pRes, BYTE wStatNum);
	void LoadVoltStatPara (TVoltStatPara* pPara);
	void LoadStatRes();
	void DoPowerStat();
	void ResetStat(WORD wOI);
private:
	TSem			m_semTermLog;
	TSem			m_semStat;
#ifndef SYS_WIN
	CDataLog 		m_DataLog;
#endif
	//CDpStat*		m_pDataStat[PN_NUM+1];	//指针数组
	TTime 			m_tmLastRun;			//记录上一运行的时间
	DWORD 			m_dwStatClick;
	//BYTE			m_bMtrInterv[PN_NUM+1];			//保存老的抄表间隔
	TTermStatInfo	m_TermStatInfo;
	bool			m_fTermStatChg;
	TPhaseVoltStat m_PhaseVoltStat[4];	//电压合格率,总\A\B\C
	TVoltStatPara m_VoltPara;

	BYTE			 m_bIntvStatNum[5];//记录各区间参数的属性个数,即多少个OAD
	TIntvStatRela m_IntvStatRela[5][STAT_OAD_NUM];///区间参数,分、时、日、月、年
	TIntvStatRes m_IntvStatRes[5][STAT_OAD_NUM];///区间结果

	BYTE			 m_bAvgStatNum[5];//记录各平均参数的属性个数,即多少个OAD
	TStatRela m_AvgStatRela[5][STAT_OAD_NUM];///平均参数,分、时、日、月、年
	TAccAvgStatRes m_AvgStatRes[5][STAT_OAD_NUM];//平均结果

	BYTE			 m_bExtremStatNum[5];//记录各极值参数的属性个数,即多少个OAD
	TStatRela m_ExtremStatRela[5][STAT_OAD_NUM];///极值参数,分、时、日、月、年
	TExtremStatRes m_ExtremStatRes[5][STAT_OAD_NUM];///极值结果
	//一分钟平均功率
	int m_iPower[3*4][POWER_AVG_NUM_MAX];	 //分别是总,A,B,C60s平均记录
	BYTE m_bPowrIndex;	 //写记录60s平均功率
	BYTE m_bPowrIndexCnt;	 //60s平均功率记录次数
	//1分钟平均功率
	void CalcuAvgPower();

	void DoTermStat();
	bool InitTermStat();			//读取TermInfo.dat文件信息
};


#endif
