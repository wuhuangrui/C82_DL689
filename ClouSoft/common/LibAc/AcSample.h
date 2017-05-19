/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcSample.cpp
 * ժ    Ҫ�����ļ���73360�ɼ�����������м���,�����Чֵ,����,��������,Ƶ��,����,��ǵ�
 * ��ǰ�汾��1.1
 * ��    �ߣ�᯼���
 * ������ڣ�2008��5��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
*********************************************************************************************************/

#ifndef ACSAMPLE_H
#define ACSAMPLE_H
#include "syscfg.h"
#include "apptypedef.h"
#include "stdlib.h"
#include "math.h"
#include "sysarch.h"
//#include "Sample.h"
#include "Energy.h"
#include "Demand2.h"
#include "AcFmt.h"

#ifdef SYS_LINUX
	#include "filter2.h"
	#include "Att.h"
#else	//SYS_VDK
	#include <filter.h>
#endif

#define CT_NUM      64
#define CT_AVGNUM   15


typedef struct {
	WORD wPoint;  	//������
	WORD wRate;   	//��ǰ�ķ���,�ɼ�ʱˢ��
	DWORD dwConst; 	//���峣��
	BYTE bConnectType; //�ն˽��߷�ʽ 1	1:����;3:��������;4:��������
	DWORD dwUn;		   //���ѹ,��ʽNNNNN.N
	DWORD dwIn;		   //�����,��ʽNNN.NNN
	BYTE bEpMode;	   //�й������ۼ�ģʽ
					   //		D2			|		D1			|			D0			|
					   // ����������� 		| 	���򲻼���		| 		�����ۼ�ģʽ	|
					   // 0:������;1����	| 0:����;1������	| 0:������;1����ֵ��	|
					   // D2D1=00:��->��,��->��;������ֱ����
					   // D2D1=01:��->��;		ֻ������,���򲻼�
					   // D2D1=10:����->��,��->��;
					   // D2D1=11:����->��		���򲻼�
	BYTE bCombEpMode;	//����й��ļ�����ʽ
						//		D3			|		D2			|			D1			|			D0
						//		�����		|	�����			|			�����		|		�����
	BYTE bAEqMode;	   //�����޹������ۼӱ�־,D7D6,D5D4,D3D2,D1D0,�ֱ��Ӧ4~1�����޹��ۼӱ�־,
					   //��λ:1-��; 0-��;	
					   //��λ:1-����; 0-������
	BYTE bREqMode;	   //�����޹������ۼӱ�־,D7D6,D5D4,D3D2,D1D0,�ֱ��Ӧ4~1�����޹��ۼӱ�־,
					   //��λ:1-��; 0-��;	
					   //��λ:1-����; 0-������
	WORD wAutoDate[AUTO_DATE_NUM];    //�Զ�������,���ֽڱ�ʾ������,���ֽڱ�ʾ�����յ�Сʱ,�ɼ�ʱˢ��
	WORD wDayFrzTime;  //�ս���ʱ��,Сʱ,BIN,0xffff��ʾ������,
	BYTE bAngleClockwise;	//�Ƕȷ���,0��ʾ�ǶȰ�����ʱ�뷽���ʾ,Ua,Ub,Uc�ֱ�Ϊ0,240,120
							//		   1��ʾ�ǶȰ���˳ʱ�뷽���ʾ,Ua,Ub,Uc�ֱ�Ϊ0,120,240
	bool fCalcuHarmonic;	//�Ƿ����г��
	WORD wHarmNum;			//г���������
	DWORD dwHarmonicIstart;//г��������������
	WORD wEnergyID[];
	WORD wDemandID[];
	TEnergyPara EnergyPara; //���ܲ���
	TDemandPara	DemandPara; //��������
}TAcPara;	//���ɲ���,ĳЩ�ֶ��޸ĺ���ܼ���������ɼ�ʱˢ��


bool AcLoadPara(WORD wPn, TAcPara* pAcPara);

class CAcSample{
	friend void CalcuDcValue();
	
public:
	CAcSample();
	virtual ~CAcSample();

    int m_iValue[SCN_NUM+1];  //��Чֵ +6�������liuzx 20161202
    int m_iBaseVal[SCN_NUM];	//������ѹ������
    int m_iP[4];     //�ֱ���A,B,C����
    int m_iQ[4];     //�ֱ���A,B,C����
    int m_iCos[4];   //�ֱ���A,B,C����
    int m_iFreq;
	int m_iAngle[SCN_NUM+1];//����1·�������
	int m_iVal[AC_VAL_NUM];
	bool m_fDateTimeChg;//Уʱ��־

	void Calcu();     		//ÿ�ܲ����ü���һ��,�����Чֵ,����,��������,Ƶ��,����
	bool Init(WORD wPn);    //��ʼ���ڲ���Ա
	void ReInit();
    void Transfer();  //����ת�������ݿ���

	bool TrigerAdj(BYTE* bBuf);
	bool IsPowerOff();   //����������⵽������
    WORD GetPhaseStatus();
	void SaveLog();
	bool ClearLog();
	void ClearDemand(); //��������־,����߳�����,�����߳�ȥִ��
	void DateTimeChg(void);//ʱ��仯����
	bool GetChipFaultStatus();
	bool SetPulseRatio(BYTE* pbRx);
	BYTE GetPulseRatio(void);
	
private:
	WORD m_wPn;
	WORD m_wFreq;	//�Ŵ�1000��
	bool m_fParaChg;	
	TAcPara m_AcPara;	
	
	//��������
	DWORD m_dwLastAutoDate[AUTO_DATE_NUM];//�ϴν�����(�ϴ���ת��ʱ��)
	DWORD m_dwLastDayFrzTime;//�ϴζ���ʱ��(�ϴ���ת��ʱ��)
	DWORD m_dwCTRatio;
	DWORD m_dwPTRatio;
	DWORD m_dwPowerRatio;
	DWORD m_dwUn;
	DWORD m_dwIn;

	
	//Ƶ�ʸ���
	DWORD m_dwFreqPnts;  //Ƶ�ʸ���FREQ_CYC_NUM������(��׼ÿ����NUM_PER_CYC��)���� * FREQ_UNIT
	
	//���ܼ���������
	int64 m_iBarrelEp[4];	//�й��ۼƵ�A,B,C,���й������ۼƵ�Ͱ
	int64 m_iBarrelEq[4]; //�޹��ۼƵ�A,B,C,���޹������ۼƵ�Ͱ
	int64  m_iSigmaEP[4];  	//�����й����ܺ͹����õ����ۼ�ֵ,Ϊ�˱����������Чλ,�Ҳ����ڴ������ѹ��ʱ�����,������64λlong long
	int64  m_iSigmaEQ[4];     //�����޹����ܺ͹����õ����ۼ�ֵ

#ifdef SYS_LINUX
	bool m_fATT7022;
	CAtt m_Att;
	WORD   m_wQuad[4];			//���� ������δת��˳��ʹ��0123
#else //SYS_VDK
	//Ƶ�ʸ���
	WORD m_wFreqPtr;     //Ƶ�ʸ��ٵļ���ָ��
	WORD m_wZeroCnt;     //Ƶ�ʸ����Ѿ����ٵ��Ĺ������
	WORD m_wFreqPntCnt;  //Ƶ�ʸ��ٵĲ������Ƶ������������ĵ���
	WORD m_wZeroPntCnt;	 //Ƶ�ʸ��ٵ���������������ĵ���
	WORD m_wFreqCn;      //��������Ƶ�ʸ��ٵ�ͨ��ֻ����0,1,2��Ua,Ub,Uc
	bool m_fPrePos;      //Ƶ�ʸ��ٵ���һ������������
	short m_sZero1[3];   //�����1,�ֱ���:��������λ������
	short m_sZero2[3];   //�����2,�ֱ���:��������λ������
	WORD  m_wFreqRstCnt;

	int64  m_dwSigmaValue[SCN_NUM];	//������Чֵ�õ����ۼ�ֵ
	int  m_iDcValue[SCN_NUM];		//ÿ��ͨ����ֱ������
	int  m_iDcSum[SCN_NUM];
	WORD m_wSigmaPtr;				//ʹ��SIGMA�㷨��ָ��
	WORD m_wShiftPtrQ;			    //�����޹�ʱ��ѹ����ڵ�����ǰ����90�ȵ�ָ��
	int  m_iShiftFracP[3];			//�й���λУ����С������
	int  m_iShiftFracQ[3];			//�޹�90�����༰��λУ����С������
	WORD m_wSigmaPntCnt;			//SIGMA�ۼƵĵ���
	int  m_iFracP[3];				//��λУ���Ƕ�	
	int  m_iAdj[SCN_NUM];         	//У������	
	//bool m_fPhaseAdj;				//��λУ����־
	WORD m_wAdjStep;
	//WORD m_wAdjUn;
	DWORD m_dwAdjU[3];
	DWORD m_dwAdjI[3];
	DWORD m_dwAdjP[3];
	DWORD m_dwAdjS[3];
	WORD m_wShfitI[3];
	
	DWORD m_dwPosSum;
	WORD m_wCalcuPtr;  //��ǰ����ָ��
	WORD m_wCycPntNum;
	WORD m_wFftInterv;   //�ڼ���FFT��ȡ����ʱ�õ��ĵ���(����FREQ_UNIT��)
	WORD m_wFftCycCnt;	 //��������ÿ�����һ��FFT�����ڼ���
	
	//������	
	short m_sLastSample[SCN_NUM];
	WORD  m_wOverCnt[SCN_NUM];
	WORD  m_wMaxOver[SCN_NUM];
	
	WORD  m_wQuad;			//����	
	
#endif //SYS_VDK
	
	//г�����	
	WORD m_wHarPercent[HARM_NUM_MAX*SCN_NUM];	//г��������
	WORD m_wHarVal[HARM_NUM_MAX*SCN_NUM];		//г����Чֵ

	TDataItem m_diAngle;
	//CT���
	unsigned short m_wCtValue[3][CT_NUM];   
	WORD           m_wCtPtr;
	unsigned short m_wCtAvg[3][CT_AVGNUM];  
	WORD           m_wCtAvgPtr;

	//���ܱ�
	CEnergy 	m_Energy;    //����ʵ���
	CDemand2    m_Demand;	 //����
	//TRatePeriod m_RatePeriod[RATE_PERIOD_NUM];  //����ʱ�η��ʶ���
	WORD   m_wRate;
	int    m_iPulse[ENERGY_NUM_MAX];  	//���ĵ���������
	int    m_iLastPulse[ENERGY_NUM_MAX];  //�ϴμ���ʱ�ĵ���������
	BYTE   m_bEnergyMinute;
	int    m_iPe;    //����������ܵ��������й�
	int    m_iQe;	 //����������ܵ��������޹�	
	int64  m_iEPerPulse;
	int    m_iS[4];   //�ֱ���A,B,C����
	
	WORD m_wSlideNum;      //һ�����������ڵĻ���ʱ��ĸ���
	WORD m_wDemandPeriod;  //��������
	WORD m_wSlideInterv;   //����ʱ��
	TDataItem m_diDemandPeriod; //��������
	TDataItem m_diSlideInterv;  //����ʱ��
	DWORD m_dwLastDateMin[AUTO_DATE_NUM];		//�ϴγ�����ִ�еķ���
	bool m_fDateAdjBackward[AUTO_DATE_NUM];	//������ִ��ʱ����ǰ����
	DWORD m_dwDemandPulse[DEMAND_NUM_MAX];
	DWORD m_dwDemandTick[DEMAND_NUM_MAX];
	DWORD m_dwLastTick;
	bool m_fPowerOff;   //����������⵽������

	//������״̬�ּ��������йص��쳣
	bool m_fDisOrderU; 
	bool m_fDisOrderI;
	BYTE m_bCurPolar;
	BYTE m_bPhaseStatus;
    WORD m_wPnStatus;
	
	bool m_fTrigerSave;	//�ڷ��ʡ��·ݻ򳭱��շ����л���ʱ�򣬴������ݿ�ȥ���汾�����������
	bool m_fStopSaveLog;
	bool m_fClrDemand;	//��������־,����߳�����,�����߳�ȥִ��



#ifdef SYS_LINUX
	void CopyToFftBuf();
	void AddPulse();
	void UpdateAcValue();
	
#else //SYS_VDK
	//Ƶ�ʸ���
	void GetZero(short* psZero, WORD wFreqPtr);
	int CalcuFreq(short* psZero1, short* psZero2, WORD wFreqPntCnt);
	void ResetFreqSync();
	void FreqSync(WORD wEndPtr);
	
	void UpdateFreq();
	void CopyToFftBuf();
	unsigned short CalcuValue(complex_fract16* cplx);
	int CalcuP(complex_fract16* cplxI, complex_fract16* cplxV);
	int CalcuQ(complex_fract16* cplxI, complex_fract16* cplxV);
	void SaveAdj();
	void InitAdj();
	void PhaseAdjust();
	void AdErrCheck(bool fCaluTimeout);
	
	void ChannelOverflow(WORD wCn, short sSample);
#endif

	
	void CalcuS();
	long CalcuCos(long P,long Q);
	void CalcuAngle();
	int CalcuZero();
	void RunMeter();
	void PwrToPulse();
	int PwrToEnergy(int iPwr, WORD wPnt, int64& riBarrelE);
	
	void LoadPara();
	
	void CaluPhaseStatus();
	bool CaluDisOrder34(int* piAngle);
	bool CaluDisOrderU33(int* piAngle);
	bool CaluDisOrderI33(int* piAngle);
	BYTE CalCurPolar(int* piAngle, BYTE bConnectType);
	
	//г����������
	void DoHarmonic();
	unsigned short CalcuTotalHarmonic(complex_fract16* cplx, int iBaseHarVal, int* piBasePercent);
	unsigned short CalcuHarmonic(complex_fract16* cplx, int total, double dGainFactor);
	void HarmonicAntiJitter(void);
};

void AcCalcu();     		//ÿ�ܲ����ü���һ��,�����Чֵ,����,��������,Ƶ��,����
bool AcInit(WORD wPn);    //��ʼ���ڲ���Ա
bool AcTrigerAdj(BYTE* bBuf);
bool AcIsPowerOff();   //����������⵽������
void AcSaveLog();
bool AcClearLog();
void AcClearDemand();
#ifdef SYS_VDK
WORD* AcGetMaxOver();
#endif
void AcDateTimeChg();//Уʱorʱ�䷢���仯ʱ���ã�ǿ���ж��Ƿ���Ҫ�����л����������
bool AcSetPulseRatio(BYTE* pbRx);
BYTE AcGetPulseRatio(void);

bool AcGetFaultStatus();//��ȡоƬ����״̬

#endif //ACSAMPLE_H
