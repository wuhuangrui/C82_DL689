/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MeterAPI.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֳ���Ĺ����ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ������
 * ������ڣ�2016��9��
 *********************************************************************************************************/
#ifndef METERAPI_H
#define METERAPI_H
#include "sysarch.h"
#include "MeterPro.h"
#include "ThreadMonitor.h"
#include "CctTaskMangerOob.h"

DWORD GbValToBaudrate(BYTE val);
BYTE GbValToParity(BYTE val);
BYTE GbValToStopBits(BYTE val);
BYTE GbValToByteSize(BYTE val);

bool GetMeterPara(WORD wPn, TMtrPara* pMtrPara);
BYTE GetMeterAddr(WORD wPn, BYTE* pbAddr);
BYTE GetPnCn(WORD wPn);

WORD GetMeterInterv();

///////////////////////////////////////////////////////////////////////////////////////
bool InitMeter();
void NewMeterThread();

///////////////////////////////////////////////////////////////////////////////////////
void InitMtrCacheCtrl();
void RefreshMtrCacheCtrl();
void SchRefreshMtrRdCtrl(BYTE bSchNo);
void TaskRefreshMtrRdCtrl(BYTE bTaskNo);
void InitMtrRdCtrl(WORD wPn, BYTE* pbTsa, TMtrRdCtrl* pMtrRdCtrl);
void DoMangerMtrCacheCtrl();

//////////////////////////////////////////////////////////////////////////////////////
//��������ƽṹ�ӿں���
TMtrRdCtrl* GetMtrRdCtrl(WORD wPn, BYTE*pbTsa);
void PutMtrRdCtrl(WORD wPn, BYTE* pbTsa, bool fModify);

//���������������������ڲ�ʹ��
bool LoadMtrRdCtrl(WORD wPn, BYTE* pbTsa, TMtrRdCtrl* pMtrRdCtrl);
bool SaveMtrRdCtrl(WORD wPn, TMtrRdCtrl* pMtrRdCtrl);
void DeleteMtrRdCtrl();
void DeleteOneMtrRdCtrl(WORD wPn);


///////////////////////////////////////////////////////////////////////////////////////
//�����ʱ������ʽӿڶ��壺
bool SaveMtrItemMem(TMtrTmpData* pMtrTmpData, DWORD dwOAD, BYTE* pbData, BYTE bLen);
int GetMtrItemMem(TMtrTmpData* pMtrTmpData, DWORD dwOAD, BYTE* pbData);
DWORD* MtrGetFixedItems(WORD* pwItemNum);
WORD* MtrGetFixedLen();
WORD* MtrGetFixedInItems();
bool SaveMtrInItemMem(WORD wPn, DWORD dwOAD, BYTE* pbData);
void InitMtrTmpData(TMtrTmpData* pMtrTmpData, DWORD* pdwFixOAD, WORD* pwDataLen, WORD wNum);
bool QueryMtrItemMem(TMtrTmpData* pMtrTmpData, DWORD* pdwOAD, WORD wNum);

///////////////////////////////////////////////////////////////////////////////////////////////
//TMtrRdCtrl����ʱ��¼�Ĳ����ӿ�
bool AllocTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, TTaskCfg *pTaskCfg, BYTE bCSDNum, WORD wRecLen);
bool FreeTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId);
int ReadTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId, BYTE* pbRec);
int WriteTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId, BYTE* pbRec);
//����ʱ��¼��дĳ��CSD����
int WriteTmpRecItem(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId, BYTE* pbRec, BYTE bTaskSucFlgIndex, WORD wOffset, WORD wLen);
int ClrTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId);

//////////////////////////////////////////////////////////////////////////////////////////////////////
bool SaveMtrData(TMtrRdCtrl* pMtrRdCtrl, BYTE bRespType, BYTE* pbCSD, BYTE* pbData, WORD wDataLen=0);
//void SaveMtrDataHook(DWORD dwOAD, TMtrExcTmp* pMtrExcTmp);
void SaveMtrDataHook(DWORD dwOAD, TMtrExcTmp* pMtrExcTmp, BYTE bType);

extern void InitMtrExcCtrl(BYTE bPn, TMtrExcTmp* pCtrl);
extern bool AllocateMtrExcMem(BYTE* pbGlobal, TAllocTab* pAllocTab, WORD wTabNum);
extern BYTE DoMtrExc(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn, bool* pfModified);
extern void SaveMangerMtrCacheCtrl(bool fSigLock=false);	//�����?3-����?????��11��?���騰a2??��������?D?o?��?��?��?��?
extern void ClearMtrCacheCtrl();



#endif //METERAPI_H
