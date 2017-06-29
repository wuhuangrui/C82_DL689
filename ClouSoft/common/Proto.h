/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Proto.h
 * ժ    Ҫ�����ļ�ʵ��������սͨ�ŵ�Э�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע��
 *********************************************************************************************************/
#ifndef PROTO_H
#define PROTO_H

#include "apptypedef.h"
#include "ProtoIf.h"

#define PRO_FRM_SIZE 	2048
#define LOOP_BUF_SIZE   4096

typedef struct{
	bool fLocal;
	bool fAutoSend;			//�Ƿ�����������͵Ĺ���
	bool fUseLoopBuf;
}TProPara;

class CProtoIf;

class CProto
{
public:
    CProto();
    virtual ~CProto();
    
    //��������
    CProtoIf* m_pIf;
    TProPara* m_pProPara;
	bool m_fConnected;	//Э���Ƿ��Ѿ�������
	
    //��������
    void AttachIf(CProtoIf* pIf) { m_pIf = pIf; }; //�󶨽ӿ�
	CProtoIf* GetIf() { return m_pIf; };
	WORD GetMaxFrmBytes();
	bool Init(TProPara* pProPara);
	bool IsConnected();
	DWORD GetRcvClick(){ return m_dwRcvClick;};	

    //�麯��
	virtual bool RcvFrm(bool fSingleMode=false);
	virtual bool Login();
	virtual bool Logoff() { return true; };
	virtual bool Beat();
	virtual void OnConnectOK();
	virtual void OnBroken() { };
	virtual bool AutoSend() { return true; };
	virtual bool IsNeedAutoSend() { return false; }; //�Ƿ���Ҫ�����ϱ�
	virtual void PrintFrmCmd(char* pszMsg, BYTE* pbFrm) { };
	virtual void LoadUnrstPara() { };	//װ�طǸ�λ����
	virtual void DoProRelated() { };	//��һЩЭ����صķǱ�׼������
	virtual bool Send(BYTE* pbTxBuf, WORD wLen);
	virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize); 
	//virtual void SetCnOAD();
	
protected:
	//��������
	WORD m_nRxStep;
    WORD m_wRxPtr;
	DWORD m_dwRcvClick;
	BYTE m_bRxBuf[PRO_FRM_SIZE]; 
	BYTE m_bTxBuf[PRO_FRM_SIZE]; 
	
	bool m_fUseLoopBuf;
	WORD  m_wRxHead;
	WORD  m_wRxTail;
	BYTE  m_bLoopBuf[LOOP_BUF_SIZE];    //���յ�ѭ��������
	
	void PutToLoopBuf(BYTE* pbBuf, WORD wLen);
	WORD RxFromLoopBuf(BYTE* pbRxBuf, WORD wBufSize);
	void DeleteFromLoopBuf(WORD wLen);
	WORD GetLoopBufLen();

	//�麯��
	virtual int RcvBlock(BYTE* pbBlock, int nLen){ return 0; };
	virtual int SingleRcvBlock(BYTE* pbBlock, int nLen){ return 0; };
	virtual bool HandleFrm(){ return false; };
};
	

#endif //PROTO_H
