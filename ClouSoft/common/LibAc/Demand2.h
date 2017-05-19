/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Demand2.cpp
 * 摘    要：本文件利用脉冲进行需量计算
 * 当前版本：1.1
 * 作    者：岑坚宇
 * 完成日期：2008年5月
*********************************************************************************************************/
#ifndef DEMAND2_H
#define DEMAND2_H

#include "apptypedef.h"
#include "LibDbStruct.h"
#include "LibAcConst.h"
//#include "Sample.h"

#ifdef ACLOG_ENABLE		
	#include "DataLog.h"
#endif //ACLOG_ENABLE

#define DEMAND_TYPE_MAX   64

#define SLIDE_BUF_SIZE    (60+1)

typedef struct {
	WORD wPn0;  	//默认的测量点,一般为测量点0
	WORD wPn1;		//写入的备选测量点,配置为0xffff表示不存在
					//主要针对那种交采测量点可变的情况,数据默认写入wPn0,
					//如果配置的测量点有效,则同时写入wPn1
					
	WORD  wRateNum;//费率数
	WORD wRate;   	//当前的费率,可即时刷新
	DWORD dwConst; 	//脉冲常数
	WORD wFrac;		//需量小数位数,标准格式是645里的NN.NNNN(kw/kvar),配为4
	bool fEnableLog; //支持数据写到铁电
	bool fSingleDemandId; //支持最大需量及发生时间独立存储
	WORD wLogID;	 //日志文件ID
	WORD wMeteringDay;	//抄表日
	WORD wMeteringHour;	//抄表日的小时
	WORD wSlipNum;		//滑差数,通过最大需量周期/滑差时间求得,可即时刷新
	DWORD dwSlipInterv;	//滑差时间,单位分钟
	WORD  wTypeNum; //wID[3][DEMAND_TYPE_MAX]里实际电能类型的个数
	WORD  wLogNum;	//铁电实际保存电能类型个数
	WORD  wInnerID[DEMAND_TYPE_MAX]; //内部计算的电能ID
	WORD  wDemandID[4][DEMAND_TYPE_MAX];  //3个数组分别是本月/上月/上上月ID/当前需量BANK2增加1项用于记录当前实时需量
							//0表示无,比如对应的上上月数据ID为0表示月转存的时候不用转存相应数据
							//ID最低位为f表示带费率块数据,
							//非f表示只是总电能不带费率的单项数据
	WORD  wTimeID[3][DEMAND_TYPE_MAX];
	BYTE  bDemTimeLen;
}TDemandPara;	//需量参数,某些字段修改后需量计算程序程序可即时刷新

typedef struct{
	DWORD 	dwDemand;   //当前费率下的最大需量
	BYTE    bTime[7];   //当前费率下的最大需量发生时间
}TDemandLogItem;

typedef struct{
	BYTE 			bRate;	    //当前费率
	TDemandLogItem	LogItem[DEMAND_TYPE_MAX];
}TDemandLog;		//防止掉电的最大需量数据记录

//在费率发生改变的时候,触发一次测量点数据的保存,所以需量日志不需要保存所有费率的数据,
//只需要保存当前费率的数据就行了,即需量日志记录的都是在同一费率下发生的最大需量

//关于掉电后再次上电时,数据转存到FLASH文件系统的问题:
//由于上电后数据也不是马上转存到FLASH,而只是更新到数据库而已,这里讨论这个保存到铁电存储器的
//的最大需量会不会有由于再次掉电而丢失的危险.
//情况1:如果费率在上电后没发生改变,如果没有更大的最大需量发生,那么铁电里依然保存有以前的
//		本费率最大需量数据,而不会发生新的写日志操作,所以铁电里的需量数据是安全的
//情况2:如果上电后发生了费率更改,因为新的需量数据要到15分钟后才生成,即日志数据要到15分钟后才
//		更新,而在上电后的1分钟内数据库会发生一次整体保存,所以也是不会发生丢失的情况

class CDemand2 {
public:
	CDemand2();
	virtual ~CDemand2();

	void Init(TDemandPara* pDemandPara); //WORD wPoint, WORD wRate
	void ReInit();
	void CalcuDemand(DWORD* pdwPulse, DWORD* pdwTick);
	//void SetRate(WORD wRate) { m_wRate = wRate; };
	void TransferCur();
	void TransferMonth();
	void SetCurMonth(BYTE bCurMonth);
	BYTE GetCurMonth() { return m_bCurMonth; };	
	void ClearDemand();
	void SetSlip(WORD wSlipNum);
	void SaveCurDemand();

#ifdef ACLOG_ENABLE		
	void ResetLog();
	void SaveLog();
	bool ClearLog() { return m_DataLog.ClearLog(); }; //清除日志数据
#endif //ACLOG_ENABLE
	DWORD m_dwPwrSum[DEMAND_TYPE_MAX]; // 改为PUBLIC 以便外部变量入库当前需量 20161019 liuzx

private:
	//电能的顺序是：正、反向有功，正、反向及一、二、三、四象限无功
	DWORD m_dwDemand[DEMAND_TYPE_MAX][RATE_NUM+1]; 
	BYTE  m_bTime[DEMAND_TYPE_MAX][(RATE_NUM+1)*7]; // BIN:0-1YEAR 2 MONTH 3DAY 4HOUR 5MINUTE 6SECOND
	BYTE  m_bDemTimeLen;
	
	DWORD m_dwPwr[DEMAND_TYPE_MAX][SLIDE_BUF_SIZE];
	DWORD m_dwTick[DEMAND_TYPE_MAX][SLIDE_BUF_SIZE];
	WORD  m_wPwrPtr; 
	BYTE  m_bCurMonth;
	bool  m_fPwrFull;
	WORD m_wSlipNum;      //一个需量周期内的滑差时间的个数
	DWORD m_dwSlipTicks;
	
	DWORD m_dwEPerPulse;
	//WORD m_wRate;   //当前的费率
	WORD m_wRateNum;   //费率数
	WORD m_wPn0;
	WORD m_wPn1;
	WORD m_wTypeNum;
	WORD m_wLogNum;
	TDemandPara* m_pDemandPara;
	
	TDataItem m_diDemand0[4][DEMAND_TYPE_MAX]; // 012为最大需量，3为实时需量不记录时间
	TDataItem m_diTime0[3][DEMAND_TYPE_MAX]; 
	
	TDataItem m_diDemand1[4][DEMAND_TYPE_MAX]; 
	TDataItem m_diTime1[3][DEMAND_TYPE_MAX]; 
	
	//TDataItem m_diCurDemand[DEMAND_NUM_MAX];
	//TDataItem m_diCurTime[DEMAND_NUM_MAX];
	//TDataItem m_diLastDemand[DEMAND_NUM_MAX];
	//TDataItem m_diLastTime[DEMAND_NUM_MAX];
	//TDataItem m_diLaLastMonth[DEMAND_NUM_MAX];
	
#ifdef ACLOG_ENABLE	
	CDataLog   		m_DataLog;
	TDemandLog  	m_DemandLog; //防止掉电的电能数据日志记录数据部分
	WORD 			m_wLogSize;	 //日志要写到铁电的实际大小 
	bool 			m_fNewLog;   
#endif //ACLOG_ENABLE
	
	bool			m_fPowerUp;  //上电标志
	bool m_fSingleDemandId; //支持最大需量及发生时间独立存储

	bool IsInTheSameMeteringMonth(WORD nMDay, WORD nMhour, TTime time1, TTime time2);
	void UpdateDemandClrTimes();
	void DoOneSlip(DWORD* pdwPulse, DWORD* pdwTick);
	DWORD AdjEPerPulse(DWORD dwEPerPulse, WORD wFrac);
#ifdef ACLOG_ENABLE
	void SyncToLog();
#endif //ACLOG_ENABLE

};

#endif //DEMAND2_H 
