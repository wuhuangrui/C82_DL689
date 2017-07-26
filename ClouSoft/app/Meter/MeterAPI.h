/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MeterAPI.h
 * 摘    要：本文件主要实现抄表的公共接口
 * 当前版本：1.0
 * 作    者：李锦仙
 * 完成日期：2016年9月
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
//电表抄读控制结构接口函数
TMtrRdCtrl* GetMtrRdCtrl(WORD wPn, BYTE*pbTsa);
void PutMtrRdCtrl(WORD wPn, BYTE* pbTsa, bool fModify);

//下面两个函数电表缓存管理内部使用
bool LoadMtrRdCtrl(WORD wPn, BYTE* pbTsa, TMtrRdCtrl* pMtrRdCtrl);
bool SaveMtrRdCtrl(WORD wPn, TMtrRdCtrl* pMtrRdCtrl);
void DeleteMtrRdCtrl();
void DeleteOneMtrRdCtrl(WORD wPn);


///////////////////////////////////////////////////////////////////////////////////////
//电表临时缓存访问接口定义：
bool SaveMtrItemMem(TMtrTmpData* pMtrTmpData, DWORD dwOAD, BYTE* pbData, BYTE bLen);
int GetMtrItemMem(TMtrTmpData* pMtrTmpData, DWORD dwOAD, BYTE* pbData);
DWORD* MtrGetFixedItems(WORD* pwItemNum);
WORD* MtrGetFixedLen();
WORD* MtrGetFixedInItems();
bool SaveMtrInItemMem(WORD wPn, DWORD dwOAD, BYTE* pbData);
void InitMtrTmpData(TMtrTmpData* pMtrTmpData, DWORD* pdwFixOAD, WORD* pwDataLen, WORD wNum);
bool QueryMtrItemMem(TMtrTmpData* pMtrTmpData, DWORD* pdwOAD, WORD wNum);

///////////////////////////////////////////////////////////////////////////////////////////////
//TMtrRdCtrl中临时记录的操作接口
bool AllocTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, TTaskCfg *pTaskCfg, BYTE bCSDNum, WORD wRecLen);
bool FreeTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId);
int ReadTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId, BYTE* pbRec);
int WriteTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId, BYTE* pbRec);
//往临时记录里写某个CSD数据
int WriteTmpRecItem(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId, BYTE* pbRec, BYTE bTaskSucFlgIndex, WORD wOffset, WORD wLen);
int ClrTmpRec(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, BYTE bId);

//////////////////////////////////////////////////////////////////////////////////////////////////////
bool SaveMtrData(TMtrRdCtrl* pMtrRdCtrl, BYTE bRespType, BYTE* pbCSD, BYTE* pbData, WORD wDataLen=0);
//void SaveMtrDataHook(DWORD dwOAD, TMtrExcTmp* pMtrExcTmp);
void SaveMtrDataHook(DWORD dwOAD, TMtrExcTmp* pMtrExcTmp, BYTE bType);

extern void InitMtrExcCtrl(BYTE bPn, TMtrExcTmp* pCtrl);
extern bool AllocateMtrExcMem(BYTE* pbGlobal, TAllocTab* pAllocTab, WORD wTabNum);
extern BYTE DoMtrExc(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn, bool* pfModified);
extern void SaveMangerMtrCacheCtrl(bool fSigLock=false);	//±￡′?3-±í?????á11￡?×￠òa2??áêí·?D?o?á?￡?￡?￡?
extern void ClearMtrCacheCtrl();



#endif //METERAPI_H
