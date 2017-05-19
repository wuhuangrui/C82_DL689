/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DL645V07.h
 * ժ    Ҫ�����ļ�����07��645����Э�����ض���
 * ��ǰ�汾��1.0
 * ��    �ߣ������
 * ������ڣ�2016��11��
 * ��    ע��
 *********************************************************************************************************/
#ifndef DL645V07_H
#define DL645V07_H
#include "stdafx.h"
#include "MeterPro.h"

#define	DL645V07_CMD	      8	
#define	DL645V07_LEN	      9	
#define	DL645V07_DATA	      10

//#define	DL645V07_CMD_RESERVE	0x00	
#define	DL645V07_CMD_BC_TIME	0x08
#define	DL645V07_CMD_ASK_DATA	0x11	
#define	DL645V07_CMD_ASK_NEXT	0x12	
//#define	DL645V07_CMD_ASK_ADDR	0x13	
#define	DL645V07_CMD_WRITE_DATA	0x14	
//#define	DL645V07_CMD_WRITE_ADDR	0x15	
//#define	DL645V07_CMD_FRZ		0x16	
//#define	DL645V07_CMD_CHG_BR		0x17	
//#define	DL645V07_CMD_CHG_PSW	0x18	
//#define	DL645V07_CMD_DMD_RESET	0x19	
//#define	DL645V07_CMD_ENG_RESET	0x1A
//#define	DL645V07_CMD_EVENT_RESET 0x1B
#define	DL645V07_CMD_CTRL		0x1C	//����բ������������

//#define	DL645V07CMD_MAX			DL645V07CMD_EVENT_RESET
#define	DL645V07_CMD_GET	0x1f

typedef struct
{
	WORD wDL645Id;
	DWORD dwProId;
	WORD wDL645Len;
	WORD wProLen;	
}TItemList;

typedef struct
{
	TTime tmStart;
	BYTE bNum;	
}TRdLoadInfo;

typedef struct 
{
	BYTE bSubPro; //��Э�����ú�			
}TV07Priv;

typedef struct 
{	
	WORD nRxStep;
	WORD wRxPtr;
	WORD wRxCnt;    
	WORD wRxDataLen;
	BYTE bRdNextSeq;
	bool fRdNext;
	TRdLoadInfo tRdLoadInfo;
}TV07Tmp;


//07��645Э�����ӿں�������
bool Mtr645V07Init(struct TMtrPro* pMtrPro, BYTE bThrId);
int DL645V07AskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD=NULL, BYTE bLenRSD=0, BYTE* pbRCSD=NULL, BYTE bLenRCSD=0); //�����ݽӿ�
int DL645V07DirAskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, BYTE bChoice, BYTE* pbTx, WORD wTxLen, BYTE* pbData);
bool DL645V07RcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize); //�������պ���
void DL645V07GetProPrintType(BYTE* pbPrintPro, char* pszProName);//��ȡ��ӡЭ������
bool DL645toDL645V07(WORD wPn, WORD wDL645ID, TItemList* ptDL645V07);
WORD DL645V07MakeFrm(TV07Tmp* pTmpV07, BYTE* pbTxBuf, BYTE* pbAddr, BYTE bCmd, BYTE bLen);
int DL645V07pfnWriteItem(struct TMtrPro* pMtrPro, DWORD dwOAD, DWORD dwId, WORD wLen);

bool IsOnlyHapTime(DWORD dwEvtID);
bool IsOneIDHapEndTime(DWORD dwEvtID);
bool IsNotHapEndEng(DWORD dwEvtID);
bool IsHapEndEngSameID(DWORD dwEvtID);
bool IsHapEngOAD(DWORD dwOAD);

bool IsEndEngOAD(DWORD dwOAD);

#endif //DL645V07_H


