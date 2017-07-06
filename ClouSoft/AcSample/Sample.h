/*********************************************************************************************************
 * Copyright (c) 2005,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcSample.cpp
 * 摘    要：本文件对交流采样的公共变量、常量、函数进行定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2005年11月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/
#ifndef SAMPLE_H
#define SAMPLE_H
#include "syscfg.h"
#include "bios.h"
#include "sysarch.h"
#include "AcConst.h"
#include "AcStruct.h"
#include "AcSample.h"

#include "filter2.h"


extern WORD g_wSamplePtr;
extern WORD g_wSigmaPntNum;
extern WORD g_wSigmaPntCnt;

extern int64  g_iBarrelEp;  	  //电能累积的桶 A/B/C/总
extern int64  g_iBarrelEq;	   	  //电能累积的桶 A/B/C/总
extern WORD  g_wPulseOutP;        //用来输出的有功电能脉冲数
extern WORD  g_wPulseOutQ;	       //用来输出的无功电能脉冲数
extern WORD g_wQuad;		       //象限	
extern TAvgPower g_AvgPower[AVGP_NUM];  //用来累加电能的平均功率
extern WORD g_wAvgPwrHead;
extern WORD g_wAvgPwrTail;
extern bool g_fSampleInit;
extern DWORD  g_dwPulseClickEp;  //防潜动时标
extern DWORD  g_dwPulseClickEq;  //防潜动时标

//AD通道错乱检测
extern DWORD g_dwAdMaxCycles;
extern DWORD g_dwAdMinCycles;
extern DWORD g_dwAdPntCycles;
extern WORD  g_dwAdRstTimes;
extern DWORD g_dwCycles;			
extern DWORD g_dwLastCycles;
extern int   g_iAdCheckDelay;    //对AD通道错乱检测进行延迟
extern bool  g_fAdCheckOK;   	 //可以进行AD通道错乱检测	
extern bool  g_fStartAdCheck;    //启动AD通道错乱检测	
extern WORD  g_wAdCheckPnts;
extern int   g_iAdErrCnt;
extern int64 g_iEPerPulse;      //每个脉冲等于多少个 瓦/100 * 毫秒/8
extern WORD g_PulseWidthTop;     //脉冲宽度至少为20mS
extern WORD g_PulseWidthBottom;
extern WORD g_wPulseInterv;	//防潜动脉冲最大间隔时间
extern WORD g_wPulseIntervPhase; //单相防潜动脉冲最大间隔时间

#ifdef SYS_LINUX
extern fract16 g_fAttInput[SCN_NUM*3][320];//三相电压同步样点+三相电流同步样点+A相电压电流同步样点+B相电压电流同步样点+C相电压电流同步样点
extern unsigned long ADCdata[240];
extern bool g_fInputReady;
#endif //SYS_LINUX

//直流分量
extern short g_sDcValue[SCN_NUM];  //每个通道的直流分量
extern int	  g_iDcSum0[SCN_NUM];
extern int	  g_iDcSum[SCN_NUM];
extern WORD  g_wDcPntNum;
extern bool  g_fNewDcSum;
extern WORD  g_wDcCnt;

bool InitSample();
bool InitRatePeriod(WORD wPn);
WORD GetRate(WORD wPn);

WORD CaluPhaseStatus();
void CalcuDcValue();


TThreadRet AcThread(void* pvPara);

#include "AcSample.h"


#define GET_CYCLES(cyc)	asm ("%0 = CYCLES;" : "=d" (cyc))

#endif //SAMPLE_H
