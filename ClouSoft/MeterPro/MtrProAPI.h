/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrProAPI.h
 * ժ    Ҫ�����ļ���Ҫ��������Э��Ļ���API������ȫ�ֱ����Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2011��3��
 * ��    ע��
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

typedef struct //�����Э�����Ҫ����ı���
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


