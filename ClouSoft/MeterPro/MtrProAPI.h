/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrProAPI.h
 * 摘    要：本文件主要包含抄表协议的基本API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：潘香玲
 * 完成日期：2011年3月
 * 备    注：
 *********************************************************************************************************/
#ifndef MTRPROAPI_H
#define MTRPROAPI_H

#include "DL645.h"
#include "DL645V07.h"
#include "DL69845.h"
#include "DL645Ext.h"
#include "MeterPro.h"

typedef union
{
	int 	t645Priv ;   
	int	tV07Priv;  
}TMtrPriv;		

typedef struct //各电表协议的需要保存的变量
{
	TMtrPriv tMtrPriv;
}TMtrSaveInf;

extern struct TMtrPro g_MtrPro[3];
bool LoadMtrInfo(WORD wPn, TMtrPara* pMtrPara, TMtrSaveInf* pSaveInf);
bool SaveMtrInfo(WORD wPn, TMtrSaveInf* pSaveInf);
struct TMtrPro* CreateMtrPro(WORD wPn, TMtrPara* pMtrPara, BYTE bThrId);
int AskMtrItem(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD=NULL, BYTE bLenRSD=0, BYTE* pbRCSD=NULL, BYTE bLenRCSD=0);
bool LoadMtrPara(WORD wPn, TMtrPara* pMtrPara);
int OneAddrBroadcast_485(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD);
int Make69845Frm_485(BYTE *pbMtr, BYTE bMtrLen, BYTE bCtrl, BYTE bAFType, BYTE bCA, WORD wFramFmt, BYTE *pbAPDU, WORD wAPDULen, BYTE *pbRespBuf);
int MtrBroadcast_485(BYTE bThrId);
int BroadcastAdjustTime_485(BYTE bThrId);
#endif //MTRPROAPI_H


