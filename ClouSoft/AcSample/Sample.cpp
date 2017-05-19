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
int64  g_iBarrelEp;     //�����ۻ���Ͱ A/B/C/��
int64  g_iBarrelEq;  	  //�����ۻ���Ͱ A/B/C/��
WORD   g_wPulseOutP;      //����������й�����������
WORD   g_wPulseOutQ;	  //����������޹�����������
WORD   g_wQuad;		      //����	
DWORD  g_dwPulseClickEp;  //��Ǳ��ʱ��
DWORD  g_dwPulseClickEq;  //��Ǳ��ʱ��

TAvgPower g_AvgPower[AVGP_NUM];  //�����ۼӵ��ܵ�ƽ������
WORD g_wAvgPwrHead = 0;
WORD g_wAvgPwrTail = 0;

#ifdef SYS_VDK
TSem g_semSampleData;
#endif //SYS_VDK


//#pragma align   16
//ADͨ�����Ҽ��
DWORD g_dwAdMaxCycles;    //AD������������CPU������
DWORD g_dwAdMinCycles;	  //AD�����������СCPU������	
DWORD g_dwAdPntCycles;	  //AD��������ĵ�ǰCPU������
DWORD g_dwCycles;			
DWORD g_dwLastCycles;
int   g_iAdCheckDelay;    //��ADͨ�����Ҽ������ӳ�
bool  g_fAdCheckOK;   	  //���Խ���ADͨ�����Ҽ��	
bool  g_fStartAdCheck;    //����ADͨ�����Ҽ��	
WORD  g_wAdCheckPnts;
int   g_iAdErrCnt;
int64 g_iEPerPulse = 0x1C9C3800;  //(10L*1000L*3600L*1000L*10*8/6000)    //ÿ��������ڶ��ٸ� ��/10 * ����/8
WORD g_PulseWidthTop = 20;  //����������Ϊ20mS
WORD g_PulseWidthBottom = 20;

//ֱ������
short g_sDcValue[SCN_NUM];  //ÿ��ͨ����ֱ������
int	  g_iDcSum0[SCN_NUM];
int	  g_iDcSum[SCN_NUM];
WORD  g_wDcPntNum;
bool  g_fNewDcSum;
WORD  g_wDcCnt;


WORD  g_dwAdRstTimes = 0;     //AD������λ����

bool g_fSampleInit = false;
bool g_fFirstInitSample = true;

WORD g_wPulseInterv = 24;	//��Ǳ�����������ʱ��
WORD g_wPulseIntervPhase = (24*3); //�����Ǳ�����������ʱ��

#ifdef SYS_LINUX
fract16 g_fAttInput[SCN_NUM*3][320];//�����ѹͬ������+�������ͬ������+A���ѹ����ͬ������+B���ѹ����ͬ������+C���ѹ����ͬ������
unsigned long ADCdata[240];
#endif //SYS_LINUX

bool InitSample()
{
#ifdef SYS_VDK
	if (g_fFirstInitSample)   //��Щֻ�ܳ�ʼ��һ�εı������������ʼ��
	{
		g_semSampleData = NewSemaphore(1);
		
		g_fFirstInitSample = false;
	}
#endif //SYS_VDK
	   
	WORD wPn = GetAcPn();	//ȡ�ý��ɵĲ������
	InitAcValToDb(wPn);	//��ʼ�������������Ŀ��ƽṹ
	
	g_wSigmaPntNum = NUM_PER_CYC * SIGMA_CYC_NUM;

	g_wSamplePtr = 0;			
#ifdef SYS_VDK
	memset(g_sSampleBuf, 0, sizeof(g_sSampleBuf));  	//�޹���������õ�֮ǰ��û�еĵ�ѹ����,���������
#endif //SYS_VDK

	g_iBarrelEp = 0;    	//�����ۻ���Ͱ
	g_iBarrelEq = 0;    	//�����ۻ���Ͱ
	g_wPulseOutP = 0;   //����������й�����������
	g_wPulseOutQ = 0;	//����������޹�����������
	g_wQuad = 0;		//����	

	DWORD dwClick = GetClick();
	g_dwPulseClickEp = dwClick;  //��Ǳ��ʱ��
	g_dwPulseClickEq = dwClick;  //��Ǳ��ʱ��

	memset(g_AvgPower, 0 , sizeof(g_AvgPower));  //�����ۼӵ��ܵ�ƽ������
	g_wAvgPwrHead = 0;
	g_wAvgPwrTail = 0;
	
	//ADͨ�����Ҽ��
	g_dwAdMaxCycles = 0;     //AD������������CPU������
	g_dwAdMinCycles = 0;	 //AD�����������СCPU������
	g_dwAdPntCycles = 0;     //AD��������ĵ�ǰCPU������

	g_dwCycles = 0;			
	g_dwLastCycles = 0;
	g_iAdCheckDelay = 0;     //��ADͨ�����Ҽ������ӳ�
	
	//ֱ������
	memset(g_sDcValue, 0, sizeof(g_sDcValue));  //ÿ��ͨ����ֱ������
	memset(g_iDcSum0, 0, sizeof(g_iDcSum0));  
	memset(g_iDcSum, 0, sizeof(g_iDcSum));  
	g_wDcPntNum = g_wSigmaPntNum;
	g_fNewDcSum = false;
	g_wDcCnt = 0;
	
	BYTE bBuf[4];
	ReadItemEx(BN10, PN0, 0xa014, bBuf); //0xa014 2 ��Ǳ�����������ʱ�� NN(0~9999��)
	g_wPulseInterv = (WORD )BcdToDWORD(bBuf, 2);
	if (g_wPulseInterv < 3)
		g_wPulseInterv = 24;
	
	g_wPulseIntervPhase = g_wPulseInterv * 3; //�����Ǳ�����������ʱ��
	
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
	
	g_fStartAdCheck = false;    //����ADͨ�����Ҽ��	
	
	return true;
}


//����ֱ������
void CalcuDcValue()	//zqq to modify
{
#ifdef SYS_VDK
	if (g_fNewDcSum == false)
		return;
	
	for (WORD i=0; i<SCN_NUM; i++)
	{
		WORD* pwMaxOver = AcGetMaxOver();
		
		//������	
		if (pwMaxOver[i] > 6)
		{
			g_sDcValue[i] = 0;   //�ڷ���ͨ������������,��Ҫ����ֱ�������ļ���
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
	int iMonitorID = ReqThreadMonitorID("AC-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊ60��

    while (1)
    {
#ifdef SYS_LINUX
		Sleep(50);
#endif //SYS_LINUX

#ifdef EN_AC
			if (GetInfo(INFO_TZ_DC_PARACHG))	//ʱ��ʱ�η��ʲ����ı�
			{
				DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_TZ_DC_PARACHG......\n"));
				AcDateTimeChg();
			}
			if (GetInfo(INFO_AC_PARA))	//���ɲ����ı�
			{
				DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_AC_PARA......\n"));
				AcSaveLog();
				InitSample();//������󲻸�λ��Ҫ���³�ʼ���ڴ����
			}
			AcCalcu();
#endif		
     	
     	UpdThreadRunClick(iMonitorID);
    }
    
    ReleaseThreadMonitorID(iMonitorID);
    
    return THREAD_RET_OK;
}
