/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Energy.cpp
 * 摘    要：本文件利用脉冲进行需量计算
 * 当前版本：1.1
 * 作    者：岑坚宇
 * 完成日期：2008年5月
*********************************************************************************************************/
#ifndef ENERGY_H
#define ENERGY_H

#include "apptypedef.h"
#include "LibDbStruct.h"
#include "LibAcConst.h"

#ifdef ACLOG_ENABLE
	#include "DataLog.h"
#endif //ACLOG_ENABLE

#define ENERGY_TYPE_MAX		64
#define ENERGY_LOG_LEN		5
#define ENERGY_BAR_LOG_MAX	42	//保存不足最小计量单位的数字，保存到42个，就到了C相1,2,3,4象限无功，后续的就不保存了，如需要保存，可以将ID挪到前面

typedef struct{
	WORD wDelta;//差值，最小值的整数倍
	WORD wCalcPara;//计算方式
	WORD wSumID;//合ID
	WORD wDivID[8];//分ID
}TEnergyCorrect;

typedef struct {
	WORD wPn0;  	//默认的测量点,一般为测量点0
	WORD wPn1;		//写入的备选测量点,配置为0xffff表示不存在
					//主要针对那种交采测量点可变的情况,数据默认写入wPn0,
					//如果配置的测量点有效,则同时写入wPn1
					
	WORD wRate;   	//当前的费率,可即时刷新
	DWORD dwConst; 	//脉冲常数
	WORD wEpFrac;	//有功电能的小数位数
	WORD wEqFrac;	//无功电能的小数位数
	bool fEnableLog; //支持数据写到铁电
	WORD wLogID;	 //日志文件ID
	WORD wLogBarID;	 //用于保存不足小数计量单位的脉冲电量
	WORD wSignID;	 //保存符号的数据项的ID,放到测量点数据里,0表示不保存符号
					 //所有电能数据块的符号位保存到一个ID,每个数据块占一个字节,D0~D4分别表示总,1~4费率的符号
	int64 i64EpMax;	 //有功电能的最大值
	int64 i64EqMax;	 //无功电能的最大值
	WORD  wTypeNum; //wID[3][ENERGY_TYPE_MAX]里实际电能类型的个数
	WORD  wLogNum;	//铁电实际保存电能类型个数
	WORD  wLogBarNum;//铁电实际保存电能不足最小计量单位类型个数
	WORD  wRateNum;//费率数
	WORD  wInnerID[ENERGY_TYPE_MAX]; //内部计算的电能ID
	TDataItem diInnerBakID[ENERGY_TYPE_MAX]; //内部备份的电能ID
	WORD  wID[4][ENERGY_TYPE_MAX];  //4个数组分别是本月/上日/上月/上上月ID,
									//0表示无,比如对应的上上月数据ID为0表示月转存的时候不用转存相应数据
									//ID最低位为f表示带费率块数据,
									//非f表示只是总电能不带费率的单项数据
	WORD wPlusID[ENERGY_TYPE_MAX];	//面向对象协议，电量增加一个ID，用于保存低精度的数据
	bool fEp[ENERGY_TYPE_MAX];		//分别标明每个电能数据块是否是有功/无功电能
	bool fSign[ENERGY_TYPE_MAX];	//分别标明每个电能数据块是否支持符号
	WORD wEnergyCorrectNum;			//组合电能校验的个数，用于放置复位的时候总电能大于分相电能之和
	TEnergyCorrect* ptCorrect;//用于计算总电量不等于分相电量
}TEnergyPara;	//电能参数,某些字段修改后电能计算程序程序可即时刷新


typedef struct{
	BYTE 	bRate;	    //当前费率
	BYTE 	bEnergy[ENERGY_TYPE_MAX][ENERGY_LOG_LEN];   //各种电能的总,最高比特表示符号,其余位表示值(非补码)
}TEnergyLog;			//防止掉电的电能数据记录

typedef struct{
//	BYTE 	bRate;	    	//当前费率
	BYTE 	bEnergy[ENERGY_BAR_LOG_MAX][4];   //各种电能的总,最高比特表示符号,其余位表示值(非补码)
}TEngBarLog;			//防止掉电的电能数据记录

//如果在数据保存间隔的15分钟内发生了掉电，且费率发生了切换，电量在恢复的时候都归到了最后
//保存时候的费率。为了避免这样的问题，只有保存全部费率的数据，那么保存长度为现在的5倍
//另一个避免的办法是，在费率发生切换的时候，马上触发一次相应测量点数据的保存


class CEnergy {
public:
	CEnergy();
	virtual ~CEnergy();

	void Init(TEnergyPara* pEnergyPara); //WORD wPoint, WORD wRate, DWORD dwConst
	void ReInit();
	void AddPulse(int* piPulse);
	//void SetRate(WORD wRate) { m_wRate = wRate; };
	void TransferCur();
	void TransferDay();
	void TransferMonth();
	void EnergyMonitor();//检测
	
#ifdef ACLOG_ENABLE	
	void ResetLog();	//重新初始化日志
	void SaveLog();
	bool ClearLog() { return (m_DataLog.ClearLog() & m_DataBarLog.ClearLog()); }; //清除日志数据
#endif //ACLOG_ENABLE

private:
	void CorrectSumEnergy();//计算组合电能,防止在复位的时候丢掉小数点后面数据
	BYTE FindInnerIndex(BYTE bID);//寻找电能内部存储的ID号

	TEnergyPara* m_pEnergyPara;
	int m_iEpUnit;	//存储到数据库格式的有功电能单位:即最低位的1表示的电能
	int m_iEqUnit;	//存储到数据库格式的无功电能单位:即最低位的1表示的电能
	
	//电能的顺序是：正、反向有功，正、反向及一、二、三、四象限无功
	TDataItem m_diE0[4][ENERGY_TYPE_MAX];
	TDataItem m_diE1[4][ENERGY_TYPE_MAX];
	TDataItem m_diEPlus[ENERGY_TYPE_MAX];	
	TDataItem m_diSign0, m_diSign1;	//保存符号的数据项
	int64 m_i64E[ENERGY_TYPE_MAX][RATE_NUM+1]; //与数据库对应的电能
	int m_iBarrelE[ENERGY_TYPE_MAX];       //电能累积的桶
	int m_iBakBarrelE[ENERGY_TYPE_MAX];    //电能累积的桶
	
	int m_iEPerPulse;   //每个脉冲等于多少个 1/10WS
	WORD m_wPn0;	//默认的测量点,一般为测量点0
	WORD m_wPn1;	//写入的备选测量点,配置为0xffff表示不存在
	WORD m_wTypeNum;
	WORD m_wLogNum;
	WORD m_wLogBarNum;
	WORD m_wRateNum;//费率数

	int64 m_i64EpMax;	//根据系统库数据项格式,所能表示的有功电能的最大值
	int64 m_i64EqMax;	//根据系统库数据项格式,所能表示的无功电能的最大值

#ifdef ACLOG_ENABLE	
	CDataLog   		m_DataLog;
	TEnergyLog  	m_EnergyLog; //防止掉电的电能数据日志记录数据部分
	bool 			m_fNewLog;   
	WORD 			m_wLogSize;	 //日志要写到铁电的实际大小 
	
	CDataLog   		m_DataBarLog;
	TEngBarLog		m_EngBarLog;	//防止掉电电能小数部分丢失
	WORD 			m_wBarLogSize;
#endif //ACLOG_ENABLE
	
	bool			m_fPowerUp;  //上电标志
	
	DWORD FracToScale(WORD wFrac);
	
#ifdef ACLOG_ENABLE	
	void  SyncToLog();	//数据库数据同步更新到铁电日志
#endif //ACLOG_ENABLE
};

#endif //ENERGY_H
