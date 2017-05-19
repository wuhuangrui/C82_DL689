/*********************************************************************************************************
 * Copyright (c) 2011,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DL645Ext.h
 * 摘    要：本文件给出07版645抄表协议的相关定义
 * 当前版本：1.0
 * 作    者：郭树海
 * 完成日期：2015年10月
 * 备    注：
 *********************************************************************************************************/
#ifndef DL645EXT_H
#define DL645EXT_H
#include "stdafx.h"
#include "MeterPro.h"


#define	DL645EXT_CMD	      8	
#define	DL645EXT_LEN	      9	
#define	DL645EXT_EXTCMD	      10	
#define	DL645Ext_DATA	      11


#define	DL645EXT_CMD_T188 	0x08
#define	DL645EXT_CMD_OTHER 	0x0B
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
#define	DL645V07_CMD_CTRL		0x1C	//跳合闸、报警、保电

//#define	DL645V07CMD_MAX			DL645V07CMD_EVENT_RESET
#define	DL645V07_CMD_GET	0x1f

typedef struct
{
	WORD wDL645Id;
	DWORD dwProId;
	WORD wDL645Len;
	WORD wProLen;	
}TItemListExt;

typedef struct
{
	TTime tmStart;
	BYTE bNum;	
}TExtRdLoadInfo;

typedef struct 
{	
	WORD nRxStep;
	WORD wRxPtr;
	WORD wRxCnt;    
	WORD wRxDataLen;
	BYTE bRdNextSeq;
	bool fRdNext;
	TExtRdLoadInfo tRdLoadInfo;
}T645ExtTmp;


//07版645协议对外接口函数定义
bool Mtr645ExtInit(struct TMtrPro* pMtrPro, BYTE bThrId);
int DL645ExtAskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD); //读数据接口
int DL645ExtDirAskItemEx(struct TMtrPro* pMtrPro, BYTE bRespType, BYTE bChoice, BYTE* pbTx, WORD wTxLen, BYTE* pbData);
bool DL645ExtRcvBlock(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize); //解析接收函数
void DL645ExtGetProPrintType(BYTE* pbPrintPro, char* pszProName);//获取打印协议名称

bool IsReadDataItem(WORD wPn); // 抄失败是否超过3次
void ReadDataFailCnt(WORD wPn);// 抄失败计数
void UpdateReadDataState();// 更新总抄读状态
void CleanReadMeterFlag();// 清除补抄抄读标志 
void InitReadMeterFlg();//初始化补抄抄读标志
BYTE Make188AskItemFrm(BYTE bMtrType, BYTE* pbAddr, DWORD dwID, BYTE bSER, BYTE* pbFrm);
int MakeExt645Frm(BYTE* pbAddr, BYTE bExtCode, const BYTE* pbDataArea, BYTE bDataAreaLen, BYTE* pbFrm);
BYTE MakeJsJdAskItemFrm(BYTE bMtrType, BYTE* pbAddr, DWORD dwID, BYTE bSER, BYTE* pbFrm);
BYTE MakeJsLxAskItemFrm(BYTE bMtrType, BYTE* pbAddr, DWORD dwID, BYTE bSER, BYTE* pbFrm);
int DL645ExtpfnWriteItem(struct TMtrPro* pMtrPro, DWORD dwOAD, DWORD dwId, WORD wLen);

#endif //DL645EXT_H


