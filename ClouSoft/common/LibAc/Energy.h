/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Energy.cpp
 * ժ    Ҫ�����ļ��������������������
 * ��ǰ�汾��1.1
 * ��    �ߣ�᯼���
 * ������ڣ�2008��5��
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
#define ENERGY_BAR_LOG_MAX	42	//���治����С������λ�����֣����浽42�����͵���C��1,2,3,4�����޹��������ľͲ������ˣ�����Ҫ���棬���Խ�IDŲ��ǰ��

typedef struct{
	WORD wDelta;//��ֵ����Сֵ��������
	WORD wCalcPara;//���㷽ʽ
	WORD wSumID;//��ID
	WORD wDivID[8];//��ID
}TEnergyCorrect;

typedef struct {
	WORD wPn0;  	//Ĭ�ϵĲ�����,һ��Ϊ������0
	WORD wPn1;		//д��ı�ѡ������,����Ϊ0xffff��ʾ������
					//��Ҫ������ֽ��ɲ�����ɱ�����,����Ĭ��д��wPn0,
					//������õĲ�������Ч,��ͬʱд��wPn1
					
	WORD wRate;   	//��ǰ�ķ���,�ɼ�ʱˢ��
	DWORD dwConst; 	//���峣��
	WORD wEpFrac;	//�й����ܵ�С��λ��
	WORD wEqFrac;	//�޹����ܵ�С��λ��
	bool fEnableLog; //֧������д������
	WORD wLogID;	 //��־�ļ�ID
	WORD wLogBarID;	 //���ڱ��治��С��������λ���������
	WORD wSignID;	 //������ŵ��������ID,�ŵ�������������,0��ʾ���������
					 //���е������ݿ�ķ���λ���浽һ��ID,ÿ�����ݿ�ռһ���ֽ�,D0~D4�ֱ��ʾ��,1~4���ʵķ���
	int64 i64EpMax;	 //�й����ܵ����ֵ
	int64 i64EqMax;	 //�޹����ܵ����ֵ
	WORD  wTypeNum; //wID[3][ENERGY_TYPE_MAX]��ʵ�ʵ������͵ĸ���
	WORD  wLogNum;	//����ʵ�ʱ���������͸���
	WORD  wLogBarNum;//����ʵ�ʱ�����ܲ�����С������λ���͸���
	WORD  wRateNum;//������
	WORD  wInnerID[ENERGY_TYPE_MAX]; //�ڲ�����ĵ���ID
	TDataItem diInnerBakID[ENERGY_TYPE_MAX]; //�ڲ����ݵĵ���ID
	WORD  wID[4][ENERGY_TYPE_MAX];  //4������ֱ��Ǳ���/����/����/������ID,
									//0��ʾ��,�����Ӧ������������IDΪ0��ʾ��ת���ʱ����ת����Ӧ����
									//ID���λΪf��ʾ�����ʿ�����,
									//��f��ʾֻ���ܵ��ܲ������ʵĵ�������
	WORD wPlusID[ENERGY_TYPE_MAX];	//�������Э�飬��������һ��ID�����ڱ���;��ȵ�����
	bool fEp[ENERGY_TYPE_MAX];		//�ֱ����ÿ���������ݿ��Ƿ����й�/�޹�����
	bool fSign[ENERGY_TYPE_MAX];	//�ֱ����ÿ���������ݿ��Ƿ�֧�ַ���
	WORD wEnergyCorrectNum;			//��ϵ���У��ĸ��������ڷ��ø�λ��ʱ���ܵ��ܴ��ڷ������֮��
	TEnergyCorrect* ptCorrect;//���ڼ����ܵ��������ڷ������
}TEnergyPara;	//���ܲ���,ĳЩ�ֶ��޸ĺ���ܼ���������ɼ�ʱˢ��


typedef struct{
	BYTE 	bRate;	    //��ǰ����
	BYTE 	bEnergy[ENERGY_TYPE_MAX][ENERGY_LOG_LEN];   //���ֵ��ܵ���,��߱��ر�ʾ����,����λ��ʾֵ(�ǲ���)
}TEnergyLog;			//��ֹ����ĵ������ݼ�¼

typedef struct{
//	BYTE 	bRate;	    	//��ǰ����
	BYTE 	bEnergy[ENERGY_BAR_LOG_MAX][4];   //���ֵ��ܵ���,��߱��ر�ʾ����,����λ��ʾֵ(�ǲ���)
}TEngBarLog;			//��ֹ����ĵ������ݼ�¼

//��������ݱ�������15�����ڷ����˵��磬�ҷ��ʷ������л��������ڻָ���ʱ�򶼹鵽�����
//����ʱ��ķ��ʡ�Ϊ�˱������������⣬ֻ�б���ȫ�����ʵ����ݣ���ô���泤��Ϊ���ڵ�5��
//��һ������İ취�ǣ��ڷ��ʷ����л���ʱ�����ϴ���һ����Ӧ���������ݵı���


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
	void EnergyMonitor();//���
	
#ifdef ACLOG_ENABLE	
	void ResetLog();	//���³�ʼ����־
	void SaveLog();
	bool ClearLog() { return (m_DataLog.ClearLog() & m_DataBarLog.ClearLog()); }; //�����־����
#endif //ACLOG_ENABLE

private:
	void CorrectSumEnergy();//������ϵ���,��ֹ�ڸ�λ��ʱ�򶪵�С�����������
	BYTE FindInnerIndex(BYTE bID);//Ѱ�ҵ����ڲ��洢��ID��

	TEnergyPara* m_pEnergyPara;
	int m_iEpUnit;	//�洢�����ݿ��ʽ���й����ܵ�λ:�����λ��1��ʾ�ĵ���
	int m_iEqUnit;	//�洢�����ݿ��ʽ���޹����ܵ�λ:�����λ��1��ʾ�ĵ���
	
	//���ܵ�˳���ǣ����������й�����������һ�����������������޹�
	TDataItem m_diE0[4][ENERGY_TYPE_MAX];
	TDataItem m_diE1[4][ENERGY_TYPE_MAX];
	TDataItem m_diEPlus[ENERGY_TYPE_MAX];	
	TDataItem m_diSign0, m_diSign1;	//������ŵ�������
	int64 m_i64E[ENERGY_TYPE_MAX][RATE_NUM+1]; //�����ݿ��Ӧ�ĵ���
	int m_iBarrelE[ENERGY_TYPE_MAX];       //�����ۻ���Ͱ
	int m_iBakBarrelE[ENERGY_TYPE_MAX];    //�����ۻ���Ͱ
	
	int m_iEPerPulse;   //ÿ��������ڶ��ٸ� 1/10WS
	WORD m_wPn0;	//Ĭ�ϵĲ�����,һ��Ϊ������0
	WORD m_wPn1;	//д��ı�ѡ������,����Ϊ0xffff��ʾ������
	WORD m_wTypeNum;
	WORD m_wLogNum;
	WORD m_wLogBarNum;
	WORD m_wRateNum;//������

	int64 m_i64EpMax;	//����ϵͳ���������ʽ,���ܱ�ʾ���й����ܵ����ֵ
	int64 m_i64EqMax;	//����ϵͳ���������ʽ,���ܱ�ʾ���޹����ܵ����ֵ

#ifdef ACLOG_ENABLE	
	CDataLog   		m_DataLog;
	TEnergyLog  	m_EnergyLog; //��ֹ����ĵ���������־��¼���ݲ���
	bool 			m_fNewLog;   
	WORD 			m_wLogSize;	 //��־Ҫд�������ʵ�ʴ�С 
	
	CDataLog   		m_DataBarLog;
	TEngBarLog		m_EngBarLog;	//��ֹ�������С�����ֶ�ʧ
	WORD 			m_wBarLogSize;
#endif //ACLOG_ENABLE
	
	bool			m_fPowerUp;  //�ϵ��־
	
	DWORD FracToScale(WORD wFrac);
	
#ifdef ACLOG_ENABLE	
	void  SyncToLog();	//���ݿ�����ͬ�����µ�������־
#endif //ACLOG_ENABLE
};

#endif //ENERGY_H
