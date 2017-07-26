/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrCtrl.h
 * 摘    要：本文件主要实现电表的抄表控制
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2016年8月
 *********************************************************************************************************/
#ifndef MTRCTRL_H
#define MTRCTRL_H
#include "MeterStruct.h"
#include "LibDbConst.h"
#include "MeterPro.h"
#include "MtrHook.h"

extern bool g_fDirRd[LOGIC_PORT_NUM];
extern BYTE g_bDirRdStep;
extern BYTE g_bMtrRdStep[LOGIC_PORT_NUM];
extern BYTE g_bMtrRdStatus[PN_MASK_SIZE];
//extern TSem g_semMtrCtrl;
extern TSem g_semMtrCacheCtrl;
extern TSem g_semMtrExc;
extern TSem g_semRdMtr[LOGIC_PORT_NUM];
extern bool g_f485SchMtr;

void StopMtrRd(WORD wStopSec);
void GetDirRdCtrl(BYTE bThrId);	//取得直抄的控制权
void ReleaseDirRdCtrl(BYTE bThrId); //释放直抄的控制权
BYTE GetRdMtrState(BYTE bThrId);

int DirAskMtrData(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bAddrLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData);
int MtrDoFwd(TCommPara CommPara, BYTE* pTx, WORD wTxLen, BYTE* pbData, WORD wBufSize, WORD wFrmTimeOut, WORD wByteTimeOut);

//////////////////////////////////////////////////////////////////////////////////
//抄表ID搜索相关函数
BYTE GetCurPrio(BYTE bCn);
int SearchAnUnReadID(BYTE bCn, WORD wPn, TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, bool fIsCctFlg = false);

BYTE AutoReadPn(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn, DWORD dwCurIntervSec, BYTE bThrId, bool* pfModified);

void DoPortRdErr(bool fMtrFailHap);

void MtrCtrlInit();
TThreadRet MtrRdThread(void* pvPara);

#endif //MTRCTRL_H
