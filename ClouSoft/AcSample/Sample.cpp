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
#include "FaCfg.h"
#include "Sample.h"
#include "fm24cl64.h"
#include "AcFmt.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include "FaAPI.h"

#ifdef SYS_LINUX
#include "Att.h"
#endif //SYS_LINUX

WORD g_wSamplePtr = 0;
WORD g_wSigmaPntNum = NUM_PER_CYC * SIGMA_CYC_NUM;
int64  g_iBarrelEp;     //电能累积的桶 A/B/C/总
int64  g_iBarrelEq;  	  //电能累积的桶 A/B/C/总
WORD   g_wPulseOutP;      //用来输出的有功电能脉冲数
WORD   g_wPulseOutQ;	  //用来输出的无功电能脉冲数
WORD   g_wQuad;		      //象限	
DWORD  g_dwPulseClickEp;  //防潜动时标
DWORD  g_dwPulseClickEq;  //防潜动时标

TAvgPower g_AvgPower[AVGP_NUM];  //用来累加电能的平均功率
WORD g_wAvgPwrHead = 0;
WORD g_wAvgPwrTail = 0;

#ifdef SYS_VDK
TSem g_semSampleData;
#endif //SYS_VDK


//#pragma align   16
//AD通道错乱检测
DWORD g_dwAdMaxCycles;    //AD采样间隔的最大CPU周期数
DWORD g_dwAdMinCycles;	  //AD采样间隔的最小CPU周期数	
DWORD g_dwAdPntCycles;	  //AD采样间隔的当前CPU周期数
DWORD g_dwCycles;			
DWORD g_dwLastCycles;
int   g_iAdCheckDelay;    //对AD通道错乱检测进行延迟
bool  g_fAdCheckOK;   	  //可以进行AD通道错乱检测	
bool  g_fStartAdCheck;    //启动AD通道错乱检测	
WORD  g_wAdCheckPnts;
int   g_iAdErrCnt;
int64 g_iEPerPulse = 0x1C9C3800;  //(10L*1000L*3600L*1000L*10*8/6000)    //每个脉冲等于多少个 瓦/10 * 毫秒/8
WORD g_PulseWidthTop = 20;  //脉冲宽度至少为20mS
WORD g_PulseWidthBottom = 20;

//直流分量
short g_sDcValue[SCN_NUM];  //每个通道的直流分量
int	  g_iDcSum0[SCN_NUM];
int	  g_iDcSum[SCN_NUM];
WORD  g_wDcPntNum;
bool  g_fNewDcSum;
WORD  g_wDcCnt;


WORD  g_dwAdRstTimes = 0;     //AD采样复位次数

bool g_fSampleInit = false;
bool g_fFirstInitSample = true;

WORD g_wPulseInterv = 24;	//防潜动脉冲最大间隔时间
WORD g_wPulseIntervPhase = (24*3); //单相防潜动脉冲最大间隔时间

#ifdef SYS_LINUX
fract16 g_fAttInput[SCN_NUM*3][320];//三相电压同步样点+三相电流同步样点+A相电压电流同步样点+B相电压电流同步样点+C相电压电流同步样点
unsigned long ADCdata[240];
#endif //SYS_LINUX

bool InitSample()
{
#ifdef SYS_VDK
	if (g_fFirstInitSample)   //那些只能初始化一次的变量放在这里初始化
	{
		g_semSampleData = NewSemaphore(1);
		
		g_fFirstInitSample = false;
	}
#endif //SYS_VDK
	   
	WORD wPn = GetAcPn();	//取得交采的测量点号
	InitAcValToDb(wPn);	//初始化交采数据入库的控制结构
	
	g_wSigmaPntNum = NUM_PER_CYC * SIGMA_CYC_NUM;

	g_wSamplePtr = 0;			
#ifdef SYS_VDK
	memset(g_sSampleBuf, 0, sizeof(g_sSampleBuf));  	//无功计算可能用到之前还没有的电压样点,所以先清除
#endif //SYS_VDK

	g_iBarrelEp = 0;    	//电能累积的桶
	g_iBarrelEq = 0;    	//电能累积的桶
	g_wPulseOutP = 0;   //用来输出的有功电能脉冲数
	g_wPulseOutQ = 0;	//用来输出的无功电能脉冲数
	g_wQuad = 0;		//象限	

	DWORD dwClick = GetClick();
	g_dwPulseClickEp = dwClick;  //防潜动时标
	g_dwPulseClickEq = dwClick;  //防潜动时标

	memset(g_AvgPower, 0 , sizeof(g_AvgPower));  //用来累加电能的平均功率
	g_wAvgPwrHead = 0;
	g_wAvgPwrTail = 0;
	
	//AD通道错乱检测
	g_dwAdMaxCycles = 0;     //AD采样间隔的最大CPU周期数
	g_dwAdMinCycles = 0;	 //AD采样间隔的最小CPU周期数
	g_dwAdPntCycles = 0;     //AD采样间隔的当前CPU周期数

	g_dwCycles = 0;			
	g_dwLastCycles = 0;
	g_iAdCheckDelay = 0;     //对AD通道错乱检测进行延迟
	
	//直流分量
	memset(g_sDcValue, 0, sizeof(g_sDcValue));  //每个通道的直流分量
	memset(g_iDcSum0, 0, sizeof(g_iDcSum0));  
	memset(g_iDcSum, 0, sizeof(g_iDcSum));  
	g_wDcPntNum = g_wSigmaPntNum;
	g_fNewDcSum = false;
	g_wDcCnt = 0;
	
	BYTE bBuf[4];
	ReadItemEx(BN10, PN0, 0xa014, bBuf); //0xa014 2 防潜动脉冲最大间隔时间 NN(0~9999秒)
	g_wPulseInterv = (WORD )BcdToDWORD(bBuf, 2);
	if (g_wPulseInterv < 3)
		g_wPulseInterv = 24;
	
	g_wPulseIntervPhase = g_wPulseInterv * 3; //单相防潜动脉冲最大间隔时间
	
	InitRatePeriod(wPn);
#ifdef EN_AC	
	AcInit(wPn);
#endif	
	
	g_wAdCheckPnts = 0;
	g_iAdErrCnt = 0;
	g_fAdCheckOK = false;

#ifdef SYS_VDK
	InitAD73360();
#endif //SYS_VDK

	g_fSampleInit = true;
	
	g_fStartAdCheck = false;    //启动AD通道错乱检测	
	
	return true;
}


//计算直流分量
void CalcuDcValue()	//zqq to modify
{
#ifdef SYS_VDK
	if (g_fNewDcSum == false)
		return;
	
	for (WORD i=0; i<SCN_NUM; i++)
	{
		WORD* pwMaxOver = AcGetMaxOver();
		
		//溢出检测	
		if (pwMaxOver[i] > 6)
		{
			g_sDcValue[i] = 0;   //在发生通道溢出的情况下,不要进行直流分量的计算
			pwMaxOver[i] = 0;
		}
		else
		{
			g_sDcValue[i] = g_iDcSum[i] / g_wDcPntNum;
		}
	}
	
	g_fNewDcSum = false;
#endif //SYS_VDK
}

TThreadRet AcThread(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("AC-thrd", 60*60);	//申请线程监控ID,更新间隔为60秒

    while (1)
    {
#ifdef SYS_LINUX
		Sleep(50);
#endif //SYS_LINUX

#ifdef EN_AC
			if (GetInfo(INFO_TZ_DC_PARACHG))	//时区时段费率参数改变
			{
				DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_TZ_DC_PARACHG......\n"));
				AcDateTimeChg();
			}
			if (GetInfo(INFO_AC_PARA))	//交采参数改变
			{
				DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_AC_PARA......\n"));
				AcSaveLog();
				InitSample();//面向对象不复位需要重新初始化内存变量
			}
			AcCalcu();
#endif		
     	
     	UpdThreadRunClick(iMonitorID);
    }
    
    ReleaseThreadMonitorID(iMonitorID);
    
    return THREAD_RET_OK;
}
