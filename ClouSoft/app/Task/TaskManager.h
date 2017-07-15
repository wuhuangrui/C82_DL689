#ifndef TASKMANAGER_H
#define TASKMANAGER_H
#include "FaCfg.h"
#include "DbFmt.h"
#include "TaskStruct.h"
#include "ComStruct.h"
#include "DbStruct.h"
#include "CctTaskMangerOob.h"
#include "MeterPro.h"

extern DWORD dwTaskLastUpdataTime[TASK_ID_NUM];

bool IsNeedExcTask(BYTE bTaskID);
void GetRSDAndRCSD(TRdItem* pRdItem, BYTE bMethod, BYTE* pbData, BYTE* pbCSD, DWORD dwCurSec=0);
void DoFixTask(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn, bool* pfModified);
int DoTask(WORD wPn, TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, bool* pfModified);
bool SaveTask(TMtrRdCtrl* pMtrRdCtrl);
bool SaveTaskDataToDB(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, TTaskSucFlg* taskSucFlg, BYTE* pbData=NULL, WORD wDataLen=0, WORD wIdex=0);

bool ClrTaskMoniStat(BYTE bTaskId);
bool UpdateTaskMoniStat(BYTE bTaskId, BYTE bIndex, void* pbData=NULL, WORD wDataLen=0);
bool DoTaskSwitch(TMtrRdCtrl* pMtrRdCtrl);
///////////////////////////////////////////////////////////////////////////////////////////////
//终端事件临时数据项访问接口函数
bool EvtAllocItemMem(DWORD dwOAD, TTermMem* pTermMem, WORD wDataLen);
void EvtFreeItemMem(DWORD dwOAD, TTermMem* pTermMem);
int EvtReadItemMem (DWORD dwOAD,TTermMem* pTermMem, BYTE* pbData);
int EvtWriteItemMem (DWORD dwOAD, TTermMem* pTermMem, BYTE* pbData);
int EvtReadOneItem (DWORD dwOAD, BYTE* pbData, BYTE* pbItem);
int EvtWriteOneItem (DWORD dwOAD, BYTE* pbData, BYTE* pbItem);
bool EvtAllocRecMem(DWORD dwOAD, TTermMem* pTermMem, WORD wDataLen);
void EvtFreeRecMem(DWORD dwOAD, TTermMem* pTermMem);
int EvtReadRecMem (DWORD dwOAD,TTermMem* pTermMem, BYTE* pbData);
int EvtWriteRecMem (DWORD dwOAD, TTermMem* pTermMem, BYTE* pbData);

#endif  //TASKMANAGER_H

