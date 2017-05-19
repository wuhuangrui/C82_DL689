/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DL69845.h
 * ժ    Ҫ�����ļ�����698.45����Э�����ض���
 * ��ǰ�汾��1.0
 * ��    �ߣ������
 * ������ڣ�2016��8��
 * ��    ע��
 *********************************************************************************************************/
#ifndef DL69845_H
#define DL69845_H
#include "MeterPro.h"

#define DL698_CTL_PRM_CLI						0x40
#define DL698_CTL_DIR_CLI						0x00

#define DL698_CTL_AFN_USERDATA						0x03

#define	DL69845_LEN_POS	      1
#define	DL69845_CTL_POS	      3
#define	DL69845_TSA_POS	      4

#define DL69845_APPSVR_GETREQUEST		0x05
#define DL69845_APPSVR_GETREQUEST_NORMAL		0x01
#define DL69845_APPSVR_GETREQUEST_NORMAL_LIST	0x02
#define DL69845_APPSVR_GETREQUEST_RECORD		0x03
#define DL69845_APPSVR_GETRESPONSE		0x85

typedef struct 
{	
	WORD nRxStep;
	WORD wRxPtr;
	WORD wRxCnt;    
	WORD wRxDataLen;
	WORD wRxAPDUPos;
	WORD wRxAPDULen;
}T698Tmp;

WORD GetRequestNormal(DWORD dwOAD, BYTE* pbTxBuf);
WORD GetRequestRecord(DWORD dwOAD, BYTE* pbTxBuf, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD);
int GetResponseNormal(DWORD dwOAD, BYTE* pbSrcBuf, WORD wSrcLen, BYTE* pbDstBuf);
int GetResponseRecord(DWORD dwOAD, BYTE* pbSrcBuf, WORD wSrcLen, BYTE* pbRCSD, BYTE bLenRCSD, BYTE* pbDstBuf);

//698.45��Э�����ӿں�������
bool Mtr69845Init(struct TMtrPro* pMtrPro, BYTE bThrId);
int DL69845AskItem(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD); //�����ݽӿ�
int DL69845DirAskItem(struct TMtrPro* pMtrPro, BYTE bRespType, BYTE bChoice, BYTE* pbTx, WORD wTxLen, BYTE* pbData);

WORD DL69845MakeFrm(WORD wPn, BYTE* pbAddr, BYTE* pbTxBuf, BYTE* pbAPDU, WORD wAPDULen);
int DL69845WriteItem(struct TMtrPro* pMtrPro, DWORD dwOAD, DWORD dwId, WORD wLen);

#endif //DL69845_H


