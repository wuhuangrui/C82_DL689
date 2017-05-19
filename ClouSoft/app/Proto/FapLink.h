/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FapLink.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ּ�����������ն˵Ŀ�������
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��8��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
*********************************************************************************************************/
#ifndef FAPLINK_H
#define FAPLINK_H

#include "FaProto.h"

#define FAP_LINK_SLAVE_NUM   8

extern DWORD g_dwSlaveAddr[];  //���ն˵�ַ

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
	CQueue m_Queue;     //����Ҫ�������ն˵ı�����Ϣ����
	
	bool m_fSlaveNeedReport;  //���ն����ϱ�����
	DWORD m_dwSlaveAddr[FAP_LINK_SLAVE_NUM];
	DWORD m_dwLinkClick[FAP_LINK_SLAVE_NUM];
	BYTE m_bCommCfg[5];  //��������
	bool m_fCommOpen;  //���ڴ򿪱�־
	
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
