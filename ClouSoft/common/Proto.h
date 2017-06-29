/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Proto.h
 * 摘    要：本文件实现了与主战通信的协议基类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：
 *********************************************************************************************************/
#ifndef PROTO_H
#define PROTO_H

#include "apptypedef.h"
#include "ProtoIf.h"

#define PRO_FRM_SIZE 	2048
#define LOOP_BUF_SIZE   4096

typedef struct{
	bool fLocal;
	bool fAutoSend;			//是否具有主动上送的功能
	bool fUseLoopBuf;
}TProPara;

class CProtoIf;

class CProto
{
public:
    CProto();
    virtual ~CProto();
    
    //变量定义
    CProtoIf* m_pIf;
    TProPara* m_pProPara;
	bool m_fConnected;	//协议是否已经连接上
	
    //函数定义
    void AttachIf(CProtoIf* pIf) { m_pIf = pIf; }; //绑定接口
	CProtoIf* GetIf() { return m_pIf; };
	WORD GetMaxFrmBytes();
	bool Init(TProPara* pProPara);
	bool IsConnected();
	DWORD GetRcvClick(){ return m_dwRcvClick;};	

    //虚函数
	virtual bool RcvFrm(bool fSingleMode=false);
	virtual bool Login();
	virtual bool Logoff() { return true; };
	virtual bool Beat();
	virtual void OnConnectOK();
	virtual void OnBroken() { };
	virtual bool AutoSend() { return true; };
	virtual bool IsNeedAutoSend() { return false; }; //是否需要主动上报
	virtual void PrintFrmCmd(char* pszMsg, BYTE* pbFrm) { };
	virtual void LoadUnrstPara() { };	//装载非复位参数
	virtual void DoProRelated() { };	//做一些协议相关的非标准的事情
	virtual bool Send(BYTE* pbTxBuf, WORD wLen);
	virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize); 
	//virtual void SetCnOAD();
	
protected:
	//变量定义
	WORD m_nRxStep;
    WORD m_wRxPtr;
	DWORD m_dwRcvClick;
	BYTE m_bRxBuf[PRO_FRM_SIZE]; 
	BYTE m_bTxBuf[PRO_FRM_SIZE]; 
	
	bool m_fUseLoopBuf;
	WORD  m_wRxHead;
	WORD  m_wRxTail;
	BYTE  m_bLoopBuf[LOOP_BUF_SIZE];    //接收的循环缓冲区
	
	void PutToLoopBuf(BYTE* pbBuf, WORD wLen);
	WORD RxFromLoopBuf(BYTE* pbRxBuf, WORD wBufSize);
	void DeleteFromLoopBuf(WORD wLen);
	WORD GetLoopBufLen();

	//虚函数
	virtual int RcvBlock(BYTE* pbBlock, int nLen){ return 0; };
	virtual int SingleRcvBlock(BYTE* pbBlock, int nLen){ return 0; };
	virtual bool HandleFrm(){ return false; };
};
	

#endif //PROTO_H
