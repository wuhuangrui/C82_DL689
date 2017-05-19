#include "stdafx.h"
#include "syscfg.h"
#include "sysarch.h" 
#include "stdio.h"
#include "P2PIf.h"
#include "FaConst.h"
#include "ComAPI.h"
#include "drivers.h"
#include "Trace.h"
#include "sysapi.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

CP2PIf::CP2PIf()
{
	m_wIfType = IF_P2P;
}

CP2PIf::~CP2PIf()
{
}

bool CP2PIf::Init(TP2PIfPara* pP2PIfPara)
{
	if (!CProtoIf::Init(&pP2PIfPara->IfPara)) //基类的初始化
		return false;
		
	m_pP2PIfPara = pP2PIfPara;

   	if (!m_Comm.Open(m_pP2PIfPara->CommPara)) 
	{
		DTRACE(DB_FAPROTO, ("CP2PIf::Init : fail to open COM%d\n", pP2PIfPara->CommPara.wPort));
		return false;
	}
	
	m_wState = IF_STATE_RST; 
	
	m_Comm.Config(m_pP2PIfPara->CommPara);

	if (!m_pModem->Init(&m_pP2PIfPara->ModemPara))
		return false;
	
	m_pModem->SetComm(&m_Comm); //串口共用
								
  	return true;
}

bool CP2PIf::Close() 
{ 
	if (m_Comm.IsOpen())
		m_Comm.Close();
			
	return true;
}


bool CP2PIf::Send(BYTE* pbTxBuf, WORD wLen)
{
	TraceFrm("<-- CP2PIf::Send:", pbTxBuf, wLen);
//	if(m_pCommIfPara->CommPara.wPort==2)//COMM_LOCAL)
//	if(m_pCommIfPara->CommPara.wPort==(BYTE )m_pCommIfPara.IfPara.pvArg)//COMM_LOCAL)
	{
		if (m_Comm.Write(pbTxBuf, wLen) == wLen)
			return true;
		else
			return false;
	}
	return false;
}

WORD CP2PIf::Receive(BYTE* pbRxBuf, WORD wBufSize)
{
	WORD wLen = (WORD )(m_Comm.Read(pbRxBuf, wBufSize));
	if(wLen > 0)
	    TraceFrm("--> CP2PIf::Receive:", pbRxBuf, wLen);

	return wLen;
}

//描述：复位操作
//返回: IF_RST_OK复位成功,IF_RST_HARDFAIL硬复位失败,
int CP2PIf::ResetIf()
{
	WORD wTryCnt = 0;
	
	if (!m_Comm.IsOpen())
	{
		DTRACE(DB_FAPROTO, ("CP2PIf::ResetIf: re open comm for at\n"));
		m_Comm.Open();	
	}
	
	if (m_pModem->ResetModem() != MODEM_NO_ERROR)
	{
		return IF_RST_HARDFAIL;
	}
	
	DTRACE(DB_FAPROTO, ("CP2PIf::ResetIf: ResetIf OK.\n"));
	
	return IF_RST_OK;
}

//描述：测试modem状态是否正常
//返回: true-正常, false-异常
void CP2PIf::KeepAlive()
{
    DWORD dwClick = GetClick();
	if (dwClick-m_dwRxClick>5*60 && dwClick-m_dwBeatClick>5*60)//每5分钟检测一次modem状态
	{	//刚开始时dwBeatClick为0，dwNewClick不为0，能马上发
	    if (!m_pModem->TestModem())
	        m_wState = IF_STATE_RST;
    	m_dwBeatClick = dwClick;
		DTRACE(DB_FAPROTO, ("CP2PIf::KeepAlive: test at click %d\n", dwClick));
	}    
}

void CP2PIf::OnResetFail()
{
    //m_wState = IF_STATE_RST;
}

void CP2PIf::OnResetOK()
{
    m_wState = IF_STATE_TRANS;
}