/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcStruct.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֶԽ����������ݽṹ����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2008��5��
 * ��    ע: 
 *********************************************************************************************************/
#ifndef ACSTRUCT_H
#define ACSTRUCT_H
#include "apptypedef.h"
#include "DbStruct.h"
#include "LibAcStruct.h"

typedef struct{
	DWORD dwStartTime;
	WORD  wRate;
}TRatePeriod;   //����ʱ�η��ʶ���

typedef struct{
	WORD  nYear;
	WORD  nMonth;
	WORD  nDay;
	WORD  nWeek;
	WORD  wDayChart;
}TZone;   //����ʱ�η��ʶ���

typedef struct{
	BYTE 		bTimeZoneSwitchTime[7];//����ʱ���л�ʱ��
	BYTE 		bDayChartSwitchTime[7];//����ʱ���л�ʱ��
	WORD 		wZoneNum;      //��ʱ����
	WORD		wDayChartNum;  //��ʱ�α���
	WORD		wPeriodNum;    //��ʱ����
	WORD		wRateNum;      //������
	WORD		wHolidayNum;   //����������
	WORD		wRestDayChart;  //�����ղ��õ���ʱ�α��
	BYTE		bRestStatus;    //������״̬��
	TZone		zZone[MAX_ZONE_NUM];         //��ʱ����
	TZone	    zHoliday[MAX_HOLIDAY_NUM];   //�������ձ�
	TRatePeriod rpDayChart[MAX_DAY_CHART_NUM][RATE_PERIOD_NUM];  //��ʱ�α�
}TTOU;  //645Э��涨������ʱ�η��ʱ�����ݿ�ת���ڴ����

typedef void (* TPfnAcValToFmt)(int* piVal, BYTE* pbBuf, WORD wLen);

typedef struct{
	bool fDuoPn;  	//���������Ƿ�֧��˫����������,
					//��Ҫ����Թ��꽻�ɵĲ�����ſ�������,�̶��������0,����һ��������������
					//����BANK����չ��������,���ڶ����Լ��õ�������,����һ�㶼��֧��˫������
	WORD wBn;  		//BANK��
	WORD wID;     	//������ID,
					//���Ϊ��ID,��wInnerID��Ϊ��һ��ID������,wSubNumΪ��ID�ĸ���
	WORD wIdx;		//�ڲ����������
	WORD wSubNum;	//��ID�ĸ���
	WORD wLen;		//����������ĳ���
	TPfnAcValToFmt pfnAcValToFmt;	//��ʽת������
	
	//���²����ɳ����Զ���ʼ��
	TDataItem diPn0;
	TDataItem diPn;
}TAcValToDbCtrl;	//��������������

typedef struct{
	WORD wBn;  		//BANK��
	WORD wID;     	//������ID,
					//���Ϊ��ID,��wInnerID��Ϊ��һ��ID������,wSubNumΪ��ID�ĸ���
	WORD wIdx;		//�ڲ����������
	WORD wSubNum;	//��ID�ĸ���
	WORD wLen;		//����������ĳ���
	TPfnAcValToFmt pfnAcValToFmt;	//��ʽת������
	
	//���²����ɳ����Զ���ʼ��	
	TDataItem diPn[MAX_YMNUM];
}TPulseValToDbCtrl;	//��������������



#endif //ACSTRUCT_H
