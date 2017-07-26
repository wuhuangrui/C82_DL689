/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrCtrl.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֵ��ĳ������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2016��8��
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
void GetDirRdCtrl(BYTE bThrId);	//ȡ��ֱ���Ŀ���Ȩ
void ReleaseDirRdCtrl(BYTE bThrId); //�ͷ�ֱ���Ŀ���Ȩ
BYTE GetRdMtrState(BYTE bThrId);

int DirAskMtrData(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bAddrLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData);
int MtrDoFwd(TCommPara CommPara, BYTE* pTx, WORD wTxLen, BYTE* pbData, WORD wBufSize, WORD wFrmTimeOut, WORD wByteTimeOut);

//////////////////////////////////////////////////////////////////////////////////
//����ID������غ���
BYTE GetCurPrio(BYTE bCn);
int SearchAnUnReadID(BYTE bCn, WORD wPn, TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, bool fIsCctFlg = false);

BYTE AutoReadPn(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn, DWORD dwCurIntervSec, BYTE bThrId, bool* pfModified);

void DoPortRdErr(bool fMtrFailHap);

void MtrCtrlInit();
TThreadRet MtrRdThread(void* pvPara);

#endif //MTRCTRL_H
