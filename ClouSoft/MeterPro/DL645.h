/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Dl645.h
 * ժ    Ҫ�����ļ�����97��645����Э�����ض���
 * ��ǰ�汾��1.0
 * ��    �ߣ������
 * ������ڣ�2016��11��
 * ��    ע��
 *********************************************************************************************************/
#ifndef DL645_H
#define DL645_H
#include "stdafx.h"
#include "MeterPro.h"

//97��645Э������ṹ����
typedef struct 
{
	//BYTE bSubPro; //��Э�����ú�		
	bool fRd901f;
	bool fRd9010;
	bool fRdSID;
	BYTE bAddrByte; //��ַ����6�ֽ�(12λ)������ֽ�	
}T645Priv;

typedef struct 
{	
	WORD nRxStep;
	WORD wRxPtr;
	WORD wRxCnt;    
	WORD wRxDataLen;		
}T645Tmp;

//97��645Э�����ӿں�������
bool Mtr645Init(struct TMtrPro* pMtrPro, BYTE bThrId);
int DL645AskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD=NULL, BYTE bLenRSD=0, BYTE* pbRCSD=NULL, BYTE bLenRCSD=0); //�����ݽӿ�
int DL645DirAskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, BYTE bChoice, BYTE* pbTx, WORD wTxLen, BYTE* pbData);
bool DL645RcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize); //�������պ���
void DL645GetProPrintType(BYTE* pbPrintPro, char* pszProName);//��ȡ��ӡЭ������
int DL645pfnWriteItem(struct TMtrPro* pMtrPro, DWORD dwOAD, DWORD dwId, WORD wLen);


#endif //DL645_H


