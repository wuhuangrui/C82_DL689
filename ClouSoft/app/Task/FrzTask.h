/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FrzTask.h
 * ժ    Ҫ�����ļ���Ҫʵ���������Э��Ķ�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2016��9��
 *********************************************************************************************************/

#ifndef FRZTASK_H
#define FRZTASK_H

#include "DbStruct.h"

#define FRZ_TASK_NUM					200		//����������
#define CAP_OAD_NUM						64


#define IC9								9					//������
#define OI_FRZ							0x5000

#define FRZ_TYPE_NUM					13

#define DT_FRZRELA_LEN					13
#define FRZRELA_ID_LEN					(DT_FRZRELA_LEN*CAP_OAD_NUM + 2)	//������������Ա�ID����

#define SUB_DATD_FIELD_LEN				7	//ARRAY 1 + Ԫ�ظ��� 1 + DT_OAD 1 + OAD 4 = 7


#define TASK_PATH_LEN					64
#define REC_TIME_OFFSET					6
#define REC_FRZ_DATA_OFFSET				13		//DATA�������͵���ʼƫ��
#define FRZ_REC_LEN						256		//����Ϊ����IDһ�ʼ�¼

#define MAX_GAP_SEC						120		//��¼ʱ��������

#define FMT_FRZ_TASK_TABLE				"frz%04x_%04x_%08x_%04x.dat"

// ����ӿ������Ժ궨��
//#define FRZ_LOGIC						1					//�߼��� 			static  	ctet-string
//#define FRZ_RECORDTAB					2					//�������ݱ� 		dyn  		array
#define FRZ_ATTRTAB						3					//�����������Ա� 	static  	array

// ����ӿ��෽��
#define FRZ_RESET						1					//��λ
#define FRZ_RUN							2					//ִ��
#define FRZ_TRIG						3					//����һ�μ�¼
#define FRZ_ADDATTR						4					//���һ����������������� 
#define FRZ_DELATTR						5					//ɾ��һ�����������������
#define FRZ_REFRZ						6					//����������
#define FRZ_BATADDATTR					7					//������ӹ����������� 
#define FRZ_CLRATTR						8					//��������������Ա�


//�������Ͷ���
#define FRZ_OIB_INST					0x00				//00H��˲ʱ����
#define FRZ_OIB_SEC						0x01				//01H���붳��
#define FRZ_OIB_MIN						0x02				//02H�����Ӷ���
#define FRZ_OIB_HOUR					0x03				//03H��Сʱ����
#define FRZ_OIB_DAY						0x04				//04H���ն���
#define FRZ_OIB_BALANCEDAY				0x05				//05H�������ն���
#define FRZ_OIB_MONTH					0x06				//06H���¶���
#define FRZ_OIB_YEAR					0x07				//07H���궳��
#define FRZ_OIB_TIMEZONE				0x08				//08H��ʱ�����л�����
#define FRZ_OIB_DAYSTAGE				0x09				//09H����ʱ�α��л�����
#define FRZ_OIB_TARIFF					0x0A				//0AH�����ʵ���л�����
#define FRZ_OIB_STAIR					0x0B				//0BH�������л�����
//#define FRZ_OIB_VRATE					0x10				//10H����ѹ�ϸ����¶���
#define FRZ_OIB_CLRYEAR					0x11				//11H�����ݽ��㶳��


//Ĭ�� ����ͬ��.�洢���ڽ����Զ�����������Ч,��������洢��������
#define FRZ_DFTCYC_SEC					60					//01H���붳��,Ĭ��60��
#define FRZ_DFTCYC_MIN					60					//02H�����Ӷ��ᣬĬ��60����
#define FRZ_DFTCYC_HOUR					1					//03H��Сʱ���ᣬĬ��1Сʱ
#define FRZ_DFTCYC_DAY					1					//04H���ն��ᣬĬ��1��
#define FRZ_DFTCYC_MON					1					//06H���¶��ᣬĬ��1��
#define FRZ_DFTCYC_YEAR					1					//07H���궳�ᣬĬ��1��

#define FRZ_CLR_VALID					0xA5

//�����1���������������е�ƫ��
#define OFFSET_FRZ_CYCLE				3	
#define OFFSET_FRZ_OAD					6
#define OFFSET_FRZ_REC_NUM				11

#define LAST_REC						1	//��1�ʼ�¼

#define FRZ_STAT_BASE_ID				0x0023		//BN11����һ���ڽ����ʼID

typedef struct{
	WORD	wCycle;					//��������  long-unsigned
	DWORD	dwOA;					//������������������  OAD
	WORD	wMaxRecNum;				//�洢���  long-unsigned
}TFrzCfg;		//�����������

typedef struct{
	WORD		wOI;				//�����ʶ
	TFrzCfg		tCfg;				//�����������
	TTime		tmLastRec;			//��1�ʼ�¼ʱ��	
	WORD		wDataFieldLen;		//�����ֶγ���
}TFrzCtrl;		//�������


bool InitTask(bool fInit);
void DoFrzTasks();
void OnTrigFrzData(WORD wOI);
int ReadRecByPhyIdx(char* pszTbName, WORD wPhyIdx, BYTE* pbBuf, int iLen);
int ReadLastNRec(char* szTbName, BYTE bPtr, BYTE* pbBuf, int iLen);
int GetRecIdx(const int fd, int iRecNo);
int GetRecPhyIdx(char* szTbName, BYTE bPtr);
bool GetFrzTaskFileName(WORD wOI, DWORD dwROAD, char* pszFileName);
bool InitFrzTaskFieldParser(BYTE bTableIndex, BYTE* pbRCSD, TFieldParser* pFixFields, TFieldParser* pDataFields);
int ReadFrzData(DWORD dwOAD, BYTE* pbBuf, WORD wBufSize, int* piStart);
void FrzTaskOnRxFaResetCmd();

//�����ӿں���
int OnResetFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnRunFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnRxTrigFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnAddFrzAttrCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnDelFrzAttrCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnRxTrigReFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnBatAddFrzAttrCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen);
int OnClrAttrTableCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen);

void OnStatParaChg();
bool GetFrzTaskRecFieldParser(DWORD dwROAD, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAttrTabBuf, WORD wBufSize);

bool IsIntervMatch(DWORD dwOAD);
bool IsSpecFrzOAD(DWORD dwOAD);
DWORD GetLastCycleFrzMapID(DWORD dwOAD);
void DoFrzDataCmd(WORD wTaskIndex, TFrzCtrl* pFrzCtrl);
void DoReFrzDataCmd(WORD wTaskIndex, TFrzCtrl* pFrzCtrl);

#endif //FRZTASK_H
