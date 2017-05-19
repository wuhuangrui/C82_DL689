/***********************************************************************************************
* Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
* 
* �ļ����ƣ�DpStat.h
* ժ    Ҫ: ���ļ��ṩ2������ͳ�ƹ��ܽӿ�
* ��ǰ�汾��1.0
* ��    �ߣ��� ��
* ������ڣ�2007��8��
* ��    ע��
***********************************************************************************************/
#ifndef DPSTAT_H
#define DPSTAT_H
#include  "ComStruct.h"
//#include  "MeterSched.h"

#define STAT_NUM 10


typedef struct 
{
	WORD	wInterV;			//���ֵ
	WORD	wInterU;			//�����λ
	BYTE	bPrio;				//���ȼ�
	WORD	wReqIdNum;
	WORD	wReqId[4];
//	WORD	wSwapNum;			//ͳ��id����
//	WORD	wSwapBN11Id[4];		//��BN0��Ӧ����ʱid����
//	WORD	wSwapBN0Id[4];		//��BN11��Ӧ����ʱid����

	BYTE	bStatID;			//ͳ�����͵�ID
	char	*pszName;			//ͳ�Ƶ�����
	WORD	wPnType;			//��Ҫִ�б�ͳ�ƵĲ���������
	
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
	const BYTE* pbFmtStr;		//��ʽ������
}TIdFmtStr;


class CDpStat
{
public:
	CDpStat(void);
	virtual ~CDpStat();

	//������Ե��� ����������
	void  Init(BYTE  bPn);
	void  DoDataStat();

	void  DoMtrParaChg();
	void  SetMtrInterv(WORD wInterv);

	BYTE  GetOldPnProp() {	return m_bPnProp; };	

	//-------------------------------------------------------------------
	WORD			m_wMidTimes; //г��ͳ�ƴ���
	WORD			m_wUnbITimes;
	WORD			m_wUnbUTimes;
	DWORD			m_dwMonUnbITimes;
	DWORD			m_dwMonUnbUTimes;
	void			DoUnbalanceTransfer(BYTE bChgFlg, DWORD dwSize);
	void            DoHamornicTransfer(DWORD dwOldSec);//г��ͳ�����
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

	 //ȱʡ�ж�ʱ��Ϊ5����
	void  PhaseBreakStat( int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM, int DefJudgeTime = 5);  

	void SwapBank21(BYTE bPn, WORD wBnOldID, WORD wBnNewID, DWORD dwOldSec);

	void SwapBank32( BYTE bPn, WORD wBn11ID, WORD wBn0ID, DWORD dwOldSec );

	void SwapBank64( BYTE bPn, WORD wBn11ID, WORD wBn0ID, DWORD dwOldSec );

	//�޹���������
	void  DoCosStat(int *piData,	int iLen,	DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType);
	void  DoHarmonicStat(int *piData,	int iLen,	DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType);	
	
	//ֱ��ģ����
	void  DoDcStat(int *piData,	int iLen,	DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType);	
	void  SetDcDataInvalid(int *piDcDay, int *piDcMon,DWORD dwCurMin);

	//������ͳ��
	void DoLoadStat(int *piData, int iLen,DWORD  dwCurMin, DWORD dwIntervM,WORD wNeedToPnType);

	DWORD GetStartM(DWORD dwCurTimeM, int iType);
	DWORD GetEndM(	DWORD dwCurTimeM, int iType);


	//�ն�����
//	void  TermPowerSupplyRstStat();
//	void  TermDayRstTimesStat();
//	void  TermDayMonCtrlStat();

	//�ܼ�������
//	void  GTActPowerDayMonStat();
//	void  GTActEnergyDayMonStat();
//	void  GTReactEnergyDayMonStat();
	
	//�ܼ���Խ��ͳ��
//	void  GTPowerViolationStat();

	//г���������
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

	int		  		m_iUpUpLimitAP;				//���ڹ���������
	int         	m_iUpLimitAP;				//���ڹ�������

	int         	m_iUpUpLimitVolt;			//��ѹ������
	int         	m_iUpLimitVolt;				//��ѹ�ϸ�����

	int         	m_iLowLowLimitVolt;			//��ѹ������
	int         	m_iLowLimitVolt;			//��ѹ�ϸ�����

	int         	m_iUpUpLimitCur;			//����������
	int         	m_iUpLimitCur;				//��������

	int         	m_iUpLimitZeroCur;			//�����������

	int         	m_iUnbalanceVoltLimit;		//�����ѹ��ƽ����ֵ
	int         	m_iUnbalanceCurLimit;		//���������ƽ����ֵ

	int         	m_iPhaseBreakLimit;			//��ѹ��������

	int         	m_iFactorLimit1;			//������ֵ����1
	int         	m_iFactorLimit2;			//������ֵ����2

	int				m_iDcUpHold;				//ֱ��ģ����������
	int				m_iDcDownHold;				//ֱ��ģ����������

	int         	m_iRatedVolt;				//���ѹ
	int         	m_iRatedCur;				//�����

	int				m_iHarmonicLimit[40+2];		//г��Խ�޲���

	TTime         	m_tmNow;					//��ǰʱ��
	TTime         	m_tmLast[STAT_NUM];			//��һ��ִ��ʱ�䣬���������л��ж���

	WORD			m_wBn;
	BYTE			m_bCount;					//������
	BYTE			m_bPn;						//������ţ����ܼ���ţ�	
	BYTE			m_bPnProp;					//������Ե������
	DWORD			m_dwLastMin;				//��һ����ͳ�Ƶ�ʱ��
	//CMeterSched  	m_MeterSched[STAT_NUM];

	//λD0����״̬�Ƿ�ͳ�� D1��¼��������	 D2 ��¼�������� 	
	BYTE  			m_bStatValid[STAT_NUM];	
	DWORD			m_dwStatPriM[STAT_NUM];		//��¼��һ��ͳ�Ƶ�ʱ�䣬��λ����
	DWORD			m_dwLastTotalIntvM[STAT_NUM];	//��¼��һ��ͳ�Ƶ��ۼ�ʱ�䣬��λ����

	bool			m_fZeroValid;				//0��������Ч���ݻ�����Ч���ݵı�־ 

	bool			m_fDayChgFlg[STAT_NUM];		//��ʱ�����л��ı�־

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

