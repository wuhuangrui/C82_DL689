/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Dl645.h
 * 摘    要：本文件给出97版645抄表协议的相关定义
 * 当前版本：1.0
 * 作    者：李锦仙
 * 完成日期：2016年11月
 * 备    注：
 *********************************************************************************************************/
#ifndef DL645_H
#define DL645_H
#include "stdafx.h"
#include "MeterPro.h"

//97版645协议变量结构定义
typedef struct 
{
	//BYTE bSubPro; //子协议配置号		
	bool fRd901f;
	bool fRd9010;
	bool fRdSID;
	BYTE bAddrByte; //地址不足6字节(12位)的填充字节	
}T645Priv;

typedef struct 
{	
	WORD nRxStep;
	WORD wRxPtr;
	WORD wRxCnt;    
	WORD wRxDataLen;		
}T645Tmp;

//97版645协议对外接口函数定义
bool Mtr645Init(struct TMtrPro* pMtrPro, BYTE bThrId);
int DL645AskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD=NULL, BYTE bLenRSD=0, BYTE* pbRCSD=NULL, BYTE bLenRCSD=0); //读数据接口
int DL645DirAskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, BYTE bChoice, BYTE* pbTx, WORD wTxLen, BYTE* pbData);
bool DL645RcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize); //解析接收函数
void DL645GetProPrintType(BYTE* pbPrintPro, char* pszProName);//获取打印协议名称
int DL645pfnWriteItem(struct TMtrPro* pMtrPro, DWORD dwOAD, DWORD dwId, WORD wLen);


#endif //DL645_H


