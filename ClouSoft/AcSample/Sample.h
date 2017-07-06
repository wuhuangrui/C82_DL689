/*********************************************************************************************************
 * Copyright (c) 2005,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcSample.cpp
 * ժ    Ҫ�����ļ��Խ��������Ĺ����������������������ж���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2005��11��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
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

extern int64  g_iBarrelEp;  	  //�����ۻ���Ͱ A/B/C/��
extern int64  g_iBarrelEq;	   	  //�����ۻ���Ͱ A/B/C/��
extern WORD  g_wPulseOutP;        //����������й�����������
extern WORD  g_wPulseOutQ;	       //����������޹�����������
extern WORD g_wQuad;		       //����	
extern TAvgPower g_AvgPower[AVGP_NUM];  //�����ۼӵ��ܵ�ƽ������
extern WORD g_wAvgPwrHead;
extern WORD g_wAvgPwrTail;
extern bool g_fSampleInit;
extern DWORD  g_dwPulseClickEp;  //��Ǳ��ʱ��
extern DWORD  g_dwPulseClickEq;  //��Ǳ��ʱ��

//ADͨ�����Ҽ��
extern DWORD g_dwAdMaxCycles;
extern DWORD g_dwAdMinCycles;
extern DWORD g_dwAdPntCycles;
extern WORD  g_dwAdRstTimes;
extern DWORD g_dwCycles;			
extern DWORD g_dwLastCycles;
extern int   g_iAdCheckDelay;    //��ADͨ�����Ҽ������ӳ�
extern bool  g_fAdCheckOK;   	 //���Խ���ADͨ�����Ҽ��	
extern bool  g_fStartAdCheck;    //����ADͨ�����Ҽ��	
extern WORD  g_wAdCheckPnts;
extern int   g_iAdErrCnt;
extern int64 g_iEPerPulse;      //ÿ��������ڶ��ٸ� ��/100 * ����/8
extern WORD g_PulseWidthTop;     //����������Ϊ20mS
extern WORD g_PulseWidthBottom;
extern WORD g_wPulseInterv;	//��Ǳ�����������ʱ��
extern WORD g_wPulseIntervPhase; //�����Ǳ�����������ʱ��

#ifdef SYS_LINUX
extern fract16 g_fAttInput[SCN_NUM*3][320];//�����ѹͬ������+�������ͬ������+A���ѹ����ͬ������+B���ѹ����ͬ������+C���ѹ����ͬ������
extern unsigned long ADCdata[240];
extern bool g_fInputReady;
#endif //SYS_LINUX

//ֱ������
extern short g_sDcValue[SCN_NUM];  //ÿ��ͨ����ֱ������
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
