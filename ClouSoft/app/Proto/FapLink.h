/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FapLink.cpp
 * 摘    要：本文件主要实现级联命令的主终端的控制流程
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年8月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/
#ifndef FAPLINK_H
#define FAPLINK_H

#include "FaProto.h"

#define FAP_LINK_SLAVE_NUM   8

extern DWORD g_dwSlaveAddr[];  //从终端地址

class CFapLink
{
public:
    CFapLink();
    virtual ~CFapLink();
	bool Init();
	bool ForwardSlaveFrm(TFapMsg* pMsg);
	void DoMasterThread();

protected:
	WORD  m_wRxHead;
	WORD  m_wRxTail;
	WORD m_nRxStep;
    WORD m_wRxPtr;
    WORD m_nRxCnt;
    WORD m_wRxDataLen;
	BYTE m_bRxBuf[FAP_FRM_SIZE]; 
	BYTE m_bTxBuf[FAP_FRM_SIZE]; 

	CComm m_Comm;
	CQueue m_Queue;     //级联要求发往从终端的报文消息队列
	
	bool m_fSlaveNeedReport;  //从终端有上报需求
	DWORD m_dwSlaveAddr[FAP_LINK_SLAVE_NUM];
	DWORD m_dwLinkClick[FAP_LINK_SLAVE_NUM];
	BYTE m_bCommCfg[5];  //串口配置
	bool m_fCommOpen;  //串口打开标志
	
	bool Send(BYTE* pbTxBuf, WORD wLen);
	WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
	
	bool ReInit();
	void InitRcv();
	int RcvBlock(BYTE* pbBlock, int nLen);
	WORD MakeFrm(BYTE* pbTxBuf, BYTE bCmd, BYTE* pbRTUA, BYTE bMSTA, BYTE bFSEQ, BYTE bISEQ, bool fErr, WORD wDataLen);
	bool HandleFrm();
	bool RcvFrm(WORD wDelay);
	void LinkOneSlave(BYTE* pbSlaveAddr);
	void DoForward();
	bool IsRxLinkFrm();
};
 
 
#endif  //FAPLINK_H
