 /*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AD73360.cpp
 * ժ    Ҫ�����ļ���AD73360�Ĳ�������
 * ��ǰ�汾��1.0
 * ��    �ߣ��ſ�
 * ������ڣ�2006��10��
*********************************************************************************************************/
#ifndef AD73360_H
#define AD73360_H

#include "apptypedef.h"

#define SCN_NUM           6    //������ͨ����
#define NUM_PER_CYC       160  //160 ÿ�����ڲɼ���������
#define CYC_NUM		      50   //16ÿ��ͨ��������ٸ����ڵ�����
#define SBUF_SIZE         (NUM_PER_CYC*CYC_NUM)  //ÿ��ͨ�������������

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
	DWORD dwQuadQ[4];  //�ϵ������ۻ����������޹���������
}TAcPulse;

typedef struct{
	DWORD dwAdRstCnt;
	DWORD dwAdPntCycles;
	DWORD dwAdMaxCycles;
	DWORD dwAdMinCycles;
}TAcStatus;

typedef struct{
	int   iP;		//ƽ���й�����
	int   iQ;		//ƽ���޹�����
	WORD  wTimes;   //�ۼӴ���
	WORD  wQuad;	//����
}TAvgPower;  //�����ۼӵ��ܵ�ƽ������

bool InitAD73360(TAcDriverPara* pAcDriverPara);
void StopAD73360();
int GetSamplePtr();
bool ReadAcStatus(TAcStatus* pAcStatus);

#endif //AD73360_H