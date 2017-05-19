/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Demand2.cpp
 * ժ    Ҫ�����ļ��������������������
 * ��ǰ�汾��1.1
 * ��    �ߣ�᯼���
 * ������ڣ�2008��5��
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
	WORD wPn0;  	//Ĭ�ϵĲ�����,һ��Ϊ������0
	WORD wPn1;		//д��ı�ѡ������,����Ϊ0xffff��ʾ������
					//��Ҫ������ֽ��ɲ�����ɱ�����,����Ĭ��д��wPn0,
					//������õĲ�������Ч,��ͬʱд��wPn1
					
	WORD  wRateNum;//������
	WORD wRate;   	//��ǰ�ķ���,�ɼ�ʱˢ��
	DWORD dwConst; 	//���峣��
	WORD wFrac;		//����С��λ��,��׼��ʽ��645���NN.NNNN(kw/kvar),��Ϊ4
	bool fEnableLog; //֧������д������
	bool fSingleDemandId; //֧���������������ʱ������洢
	WORD wLogID;	 //��־�ļ�ID
	WORD wMeteringDay;	//������
	WORD wMeteringHour;	//�����յ�Сʱ
	WORD wSlipNum;		//������,ͨ�������������/����ʱ�����,�ɼ�ʱˢ��
	DWORD dwSlipInterv;	//����ʱ��,��λ����
	WORD  wTypeNum; //wID[3][DEMAND_TYPE_MAX]��ʵ�ʵ������͵ĸ���
	WORD  wLogNum;	//����ʵ�ʱ���������͸���
	WORD  wInnerID[DEMAND_TYPE_MAX]; //�ڲ�����ĵ���ID
	WORD  wDemandID[4][DEMAND_TYPE_MAX];  //3������ֱ��Ǳ���/����/������ID/��ǰ����BANK2����1�����ڼ�¼��ǰʵʱ����
							//0��ʾ��,�����Ӧ������������IDΪ0��ʾ��ת���ʱ����ת����Ӧ����
							//ID���λΪf��ʾ�����ʿ�����,
							//��f��ʾֻ���ܵ��ܲ������ʵĵ�������
	WORD  wTimeID[3][DEMAND_TYPE_MAX];
	BYTE  bDemTimeLen;
}TDemandPara;	//��������,ĳЩ�ֶ��޸ĺ���������������ɼ�ʱˢ��

typedef struct{
	DWORD 	dwDemand;   //��ǰ�����µ��������
	BYTE    bTime[7];   //��ǰ�����µ������������ʱ��
}TDemandLogItem;

typedef struct{
	BYTE 			bRate;	    //��ǰ����
	TDemandLogItem	LogItem[DEMAND_TYPE_MAX];
}TDemandLog;		//��ֹ���������������ݼ�¼

//�ڷ��ʷ����ı��ʱ��,����һ�β��������ݵı���,����������־����Ҫ�������з��ʵ�����,
//ֻ��Ҫ���浱ǰ���ʵ����ݾ�����,��������־��¼�Ķ�����ͬһ�����·������������

//���ڵ�����ٴ��ϵ�ʱ,����ת�浽FLASH�ļ�ϵͳ������:
//�����ϵ������Ҳ��������ת�浽FLASH,��ֻ�Ǹ��µ����ݿ����,��������������浽����洢����
//����������᲻���������ٴε������ʧ��Σ��.
//���1:����������ϵ��û�����ı�,���û�и���������������,��ô��������Ȼ��������ǰ��
//		�����������������,�����ᷢ���µ�д��־����,��������������������ǰ�ȫ��
//���2:����ϵ�����˷��ʸ���,��Ϊ�µ���������Ҫ��15���Ӻ������,����־����Ҫ��15���Ӻ��
//		����,�����ϵ���1���������ݿ�ᷢ��һ�����屣��,����Ҳ�ǲ��ᷢ����ʧ�����

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
	bool ClearLog() { return m_DataLog.ClearLog(); }; //�����־����
#endif //ACLOG_ENABLE
	DWORD m_dwPwrSum[DEMAND_TYPE_MAX]; // ��ΪPUBLIC �Ա��ⲿ������⵱ǰ���� 20161019 liuzx

private:
	//���ܵ�˳���ǣ����������й�����������һ�����������������޹�
	DWORD m_dwDemand[DEMAND_TYPE_MAX][RATE_NUM+1]; 
	BYTE  m_bTime[DEMAND_TYPE_MAX][(RATE_NUM+1)*7]; // BIN:0-1YEAR 2 MONTH 3DAY 4HOUR 5MINUTE 6SECOND
	BYTE  m_bDemTimeLen;
	
	DWORD m_dwPwr[DEMAND_TYPE_MAX][SLIDE_BUF_SIZE];
	DWORD m_dwTick[DEMAND_TYPE_MAX][SLIDE_BUF_SIZE];
	WORD  m_wPwrPtr; 
	BYTE  m_bCurMonth;
	bool  m_fPwrFull;
	WORD m_wSlipNum;      //һ�����������ڵĻ���ʱ��ĸ���
	DWORD m_dwSlipTicks;
	
	DWORD m_dwEPerPulse;
	//WORD m_wRate;   //��ǰ�ķ���
	WORD m_wRateNum;   //������
	WORD m_wPn0;
	WORD m_wPn1;
	WORD m_wTypeNum;
	WORD m_wLogNum;
	TDemandPara* m_pDemandPara;
	
	TDataItem m_diDemand0[4][DEMAND_TYPE_MAX]; // 012Ϊ���������3Ϊʵʱ��������¼ʱ��
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
	TDemandLog  	m_DemandLog; //��ֹ����ĵ���������־��¼���ݲ���
	WORD 			m_wLogSize;	 //��־Ҫд�������ʵ�ʴ�С 
	bool 			m_fNewLog;   
#endif //ACLOG_ENABLE
	
	bool			m_fPowerUp;  //�ϵ��־
	bool m_fSingleDemandId; //֧���������������ʱ������洢

	bool IsInTheSameMeteringMonth(WORD nMDay, WORD nMhour, TTime time1, TTime time2);
	void UpdateDemandClrTimes();
	void DoOneSlip(DWORD* pdwPulse, DWORD* pdwTick);
	DWORD AdjEPerPulse(DWORD dwEPerPulse, WORD wFrac);
#ifdef ACLOG_ENABLE
	void SyncToLog();
#endif //ACLOG_ENABLE

};

#endif //DEMAND2_H 
