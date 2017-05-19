 /*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AD73360.cpp
 * 摘    要：本文件对AD73360的采样控制
 * 当前版本：1.0
 * 作    者：张凯
 * 完成日期：2006年10月
*********************************************************************************************************/
#ifndef AD73360_H
#define AD73360_H

#include "apptypedef.h"

#define SCN_NUM           6    //采样的通道数
#define NUM_PER_CYC       160  //160 每个周期采集的样点数
#define CYC_NUM		      50   //16每个通道缓存多少个周期的样点
#define SBUF_SIZE         (NUM_PER_CYC*CYC_NUM)  //每个通道缓存的样点数

typedef struct{
	int	 iEPerPulse;
	WORD wPulseWidthTop;
	WORD wPulseWidthBottom;
}TAcDriverPara;

typedef struct{
	DWORD dwPosP;
	DWORD dwNegP;
	DWORD dwPosQ;
	DWORD dwNegQ;
	DWORD dwQuadQ[4];  //上电以来累积的四象限无功电能脉冲
}TAcPulse;

typedef struct{
	DWORD dwAdRstCnt;
	DWORD dwAdPntCycles;
	DWORD dwAdMaxCycles;
	DWORD dwAdMinCycles;
}TAcStatus;

typedef struct{
	int   iP;		//平均有功功率
	int   iQ;		//平均无功功率
	WORD  wTimes;   //累加次数
	WORD  wQuad;	//象限
}TAvgPower;  //用来累加电能的平均功率

bool InitAD73360(TAcDriverPara* pAcDriverPara);
void StopAD73360();
int GetSamplePtr();
bool ReadAcStatus(TAcStatus* pAcStatus);

#endif //AD73360_H