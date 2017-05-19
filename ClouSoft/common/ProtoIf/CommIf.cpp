#include "stdafx.h"
#include "FaCfg.h"
#include "sysarch.h" 
#include "stdio.h"
#include "CommIf.h"
#include "FaConst.h"
#include "ComAPI.h"
#include "drivers.h"
#include "Trace.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//CCommIf

CCommIf::CCommIf()
{
	m_pCommIfPara = NULL;
	m_pComm = NULL;
	m_wIfType = IF_COMM;
}

CCommIf::~CCommIf()
{
}

bool CCommIf::Init(TCommIfPara* pCommIfPara)
{
	m_wState = IF_STATE_RST;
	
	if (!CProtoIf::Init(&pCommIfPara->IfPara)) //基类的初始化
		return false;
		
	m_pCommIfPara = pCommIfPara;

	if (m_pCommIfPara->pComm == NULL)
	{	
   		if (!m_Comm.Open(m_pCommIfPara->CommPara)) 
		{
			DTRACE(DB_FAPROTO, ("CCommIf::Init : fail to open COM%d\n", m_pCommIfPara->CommPara.wPort));
			return false;
		}
		
		m_pComm = &m_Comm;
	}
	else
	{
		m_pComm = m_pCommIfPara->pComm;
	}
	
	m_wState = IF_STATE_TRANS; 
	
  	return true;
}

bool CCommIf::Close() 
{ 
	if (m_pCommIfPara->pComm == NULL)
	{	
		if (m_Comm.IsOpen())
			m_Comm.Close();
	}
			
	return true;
}


bool CCommIf::Send(BYTE* pbTxBuf, WORD wLen)
{
    if (IsDebugOn(DB_FAPROTO))
    {
        char szBuf[48];
		TCommPara CommPara;
		m_pComm->GetCommPara(&CommPara);
		sprintf(szBuf, "<-- CCommIf(%d)::Send:", CommPara.wPort);
		TraceFrm(szBuf, pbTxBuf, wLen);
    }
	return m_pComm->Write(pbTxBuf, wLen) == wLen;
}

WORD CCommIf::Receive(BYTE* pbRxBuf, WORD wBufSize)
{
	BYTE bCn;
	WORD wLen = (WORD )(m_pComm->Read(pbRxBuf, wBufSize));

	if (wLen>0 && m_pCommIfPara->fMutual)	//使用共用模式,比如红外和测试232共用一个串口,输出的时候需要进行串口切换)
	{
		TCommPara CommPara;
		if (m_pComm->GetCommPara(&CommPara))
		{
			bCn = GetCommRxCn(CommPara.wPort);
			DTRACE(DB_FAPROTO, ("CCommIf::Receive: rx data from cn=%d.\n", bCn));
			SetCommTxCn(CommPara.wPort, bCn);
		}
	}

	if (wLen>0 && IsDebugOn(DB_FAPROTO))
	{	
	    char szBuf[48];
		TCommPara CommPara;
		if (m_pComm->GetCommPara(&CommPara))
			sprintf(szBuf, "--> CCommIf(%d)::Receive:", CommPara.wPort);
		TraceFrm(szBuf, pbRxBuf, wLen);
	}
		
	return wLen;
}

bool CCommIf::SetUnrstPara(void *pvPara)
{
	if (pvPara == NULL)
		return false;
	if (m_pComm == NULL)
		return false;	
	TCommPara* pPara = (TCommPara *)pvPara;
	return m_pComm->SetBaudRate(pPara->dwBaudRate);	
}

bool CCommIf::GetUnrstPara(void *pvPara)
{
	if (pvPara == NULL)
		return false;
	if (m_pComm == NULL)
		return false;	
		
	return m_pComm->GetCommPara((TCommPara *)pvPara);		
}
