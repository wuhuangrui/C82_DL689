/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�StatMgr.h
 * ժ    Ҫ�����ļ���Ҫʵ���ն�ͳ����Ϣ������ͳ����Ĺ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��������ƽ
 * ������ڣ�2008��7��
 *********************************************************************************************************/
#ifndef STATMGR_H
#define STATMGR_H
#include "DpStat.h"
#include "DataLog.h"
#include  "DbConst.h"
#include "CctTaskMangerOob.h"

#define POWER_AVG_NUM_MAX 	60	//ƽ����������������

#define STAT_OAD_NUM						20	//һ��ͳ�Ʒ��������OAD����
#define RW_ATTR_RES						2	//����ͳ����ID�Ľ������
#define RW_ATTR_RELA						3	//����ͳ����ID�Ĳ�������
#define RW_ATTR_FRZ						8	//����ͳ����ID�Ķ�������
typedef struct{
	TTime tmLastRun;		//���һ�����е�ʱ��,���������ϵ��ʱ���ж������Ƿ����л�
	WORD wDayRstStart;		//�ն˵��ո�λ������ʼֵ,���л���ʱ�������㵱�ո�λ����
	WORD wMonRstStart;		//�ն˵��¸�λ������ʼֵ,���л���ʱ�������㵱�¸�λ����
	DWORD wDayPowerTime;		//�ն˵��չ���ʱ��,����ʱ���룬�洢ϵͳ��ʱת�ɷ�
	DWORD wMonPowerTime;		//�ն˵��¹���ʱ��,����ʱ���룬�洢ϵͳ��ʱת�ɷ�
	DWORD dwDayFlux;		//�ն�GPRS������
	DWORD dwMonFlux;		//�ն�GPRS������
}TTermStatInfo;				//�ն�ͳ����Ϣ,ÿ����д�뵽�ļ�ϵͳ��

/*typedef struct {
	DWORD	dwMoniSecs;    //���ʱ��
	WORD   wQualifiedRate;  //�ϸ���
	WORD   wOverRate;  	//������
	DWORD  dwUpperSecs;  //������ʱ��
	DWORD  dwLowerSecs;  //������ʱ��
} TVoltStat; //��ѹ�ϸ���
*/
typedef struct {
	//Э��Ҫ������
	DWORD	dwMoniSecs;    //���ʱ�䣬��λ��
	WORD	wQualRate;  	//�ϸ���
	WORD   wOverRate;  	//������
	DWORD  dwUpperSecs;  //������ʱ�䣬��λ��
	DWORD  dwLowerSecs;  //������ʱ�䣬��λ��

	//�м����ݣ���Ҫ���浽�м�����ID
	DWORD dwQualSecs;	//�ϸ�ʱ�䣬��λ��
	DWORD dwOverSecs;	//����ʱ�䣬����Խ����Խ���ޣ�ͬʱ����ֻ��һ�ε�λ��
	//DWORD dwCurCycle;  	//��ǰ���ڵ�ʱ��ֵ
	DWORD dwLastSample;	//�ϴβ���ʱ���
} TVoltStat; //��ѹ�ϸ���




typedef struct {
	WORD	wAssesUpLimit;  //��ѹ��������
	WORD	wAssesLowLimit; //��ѹ��������
	WORD   wQualUpLimit;	//��ѹ�ϸ�����
	WORD   wQualLowLimit;//��ѹ�ϸ�����
} TVoltStatPara;  //��ѹ�ϸ��ʲ���




typedef struct {
	TVoltStat dayStat;  //���յ�ѹ�ϸ���
	TVoltStat monStat;  //���µ�ѹ�ϸ���
}TPhaseVoltStat;  //�ܼ������ѹ�ϸ���

typedef struct {
	TTime 	tmCurCycle;  	//��ǰ���ڵ�ʱ��ֵ
	TTime	tmLastSample;	//�ϴβ���ʱ���
} TStatExeCtrl;  //ͳ�Ƶ�ִ�п���

//����ͳ��
#define INTV_STAT_PARA_NUM	10	//����ͳ�Ƶ�Խ���жϲ����ĸ���
#define INTV_STAT_RES_NUM (INTV_STAT_PARA_NUM+1)	//����ͳ�Ƶ�Խ���жϲ����ĸ���
typedef struct {
	DWORD 	dwOAD;  //������������������OAD
	WORD	wParaNum;	//Խ���жϲ����ĸ���
	int  		iParaVal[INTV_STAT_PARA_NUM];   //Խ���жϲ���  array Data
	BYTE 	bCycleValue;		//ͳ������  unsigned
	TTimeInterv tiFreq;			//ͳ��Ƶ��  TI
} TIntvStatRela;  //һ������ͳ�ƹ�������

typedef struct {
	DWORD 	dwTotalSecs;   //�ۼ�ʱ��
	DWORD 	dwTimes;  	//�ۼƴ���
} TIntvVal;  //һ������ͳ��ֵ

typedef struct {
	DWORD 	dwOAD;  	//������������������OAD
	WORD	wIntvNum;	//����ĸ���
	TIntvVal	intvVal[INTV_STAT_RES_NUM];   //����ͳ��ֵ array һ��ͳ������
} TIntvStatRes;  //һ������ͳ�ƽ��

//�ۼ�ƽ��ͳ��
typedef struct {
	DWORD 		dwOAD;  	//������������������OAD
	BYTE 		bCycleValue;	//ͳ������  unsigned
	TTimeInterv 	tiFreq;		//ͳ��Ƶ��  TI
} TStatRela;  //һ��ͨ�õ�ͳ�ƹ�������

typedef struct {
	DWORD 	dwOAD;  	//������������������OAD
	int		iAcc;		//�ۼӺ�  instance-specific
	int		iAvg;   	//ƽ��ֵ  instance-specific
} TAccAvgStatRes;  //һ���ۼ�ƽ��ͳ�ƽ��
//��ֵͳ��
typedef struct {
	DWORD 	dwOAD;  	//��������������OAD
	int		iMax;		//���ֵ  instance-specific
	TTime	tmMax;		//���ֵ����ʱ��  date_time_s
	int		iMin;   		//��Сֵ  instance-specific
	TTime	tmMin;		//���ֵ����ʱ��  date_time_s
} TExtremStatRes;  //һ����ֵͳ�ƽ��


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
	//CDpStat*		m_pDataStat[PN_NUM+1];	//ָ������
	TTime 			m_tmLastRun;			//��¼��һ���е�ʱ��
	DWORD 			m_dwStatClick;
	//BYTE			m_bMtrInterv[PN_NUM+1];			//�����ϵĳ�����
	TTermStatInfo	m_TermStatInfo;
	bool			m_fTermStatChg;
	TPhaseVoltStat m_PhaseVoltStat[4];	//��ѹ�ϸ���,��\A\B\C
	TVoltStatPara m_VoltPara;

	BYTE			 m_bIntvStatNum[5];//��¼��������������Ը���,�����ٸ�OAD
	TIntvStatRela m_IntvStatRela[5][STAT_OAD_NUM];///�������,�֡�ʱ���ա��¡���
	TIntvStatRes m_IntvStatRes[5][STAT_OAD_NUM];///������

	BYTE			 m_bAvgStatNum[5];//��¼��ƽ�����������Ը���,�����ٸ�OAD
	TStatRela m_AvgStatRela[5][STAT_OAD_NUM];///ƽ������,�֡�ʱ���ա��¡���
	TAccAvgStatRes m_AvgStatRes[5][STAT_OAD_NUM];//ƽ�����

	BYTE			 m_bExtremStatNum[5];//��¼����ֵ���������Ը���,�����ٸ�OAD
	TStatRela m_ExtremStatRela[5][STAT_OAD_NUM];///��ֵ����,�֡�ʱ���ա��¡���
	TExtremStatRes m_ExtremStatRes[5][STAT_OAD_NUM];///��ֵ���
	//һ����ƽ������
	int m_iPower[3*4][POWER_AVG_NUM_MAX];	 //�ֱ�����,A,B,C60sƽ����¼
	BYTE m_bPowrIndex;	 //д��¼60sƽ������
	BYTE m_bPowrIndexCnt;	 //60sƽ�����ʼ�¼����
	//1����ƽ������
	void CalcuAvgPower();

	void DoTermStat();
	bool InitTermStat();			//��ȡTermInfo.dat�ļ���Ϣ
};


#endif
