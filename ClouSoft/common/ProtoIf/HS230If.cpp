#include "stdafx.h"
#include "syscfg.h"
#include "sysarch.h" 
#include "stdio.h"
#include "HS230If.h"
#include "FaConst.h"
#include "ComAPI.h"
#include "drivers.h"
#include "Trace.h"
#include "sysapi.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////

CHS230If::CHS230If()
{
	m_cCSQ = 0;
	m_wIfType = IF_P2P;
}

CHS230If::~CHS230If()
{
}

bool CHS230If::Init(THS230IfPara* pHS230IfPara)
{
	if (!CProtoIf::Init(&pHS230IfPara->IfPara)) //基类的初始化
		return false;
		
	m_pHS230IfPara = pHS230IfPara;

   	if (!m_Comm.Open(pHS230IfPara->CommPara)) 
	{
		DTRACE(DB_FAPROTO, ("CHS230If::Init : fail to open COM%d\n", pHS230IfPara->CommPara.wPort));
		return false;
	}
	
	m_wState = IF_STATE_RST; 
	
	m_Comm.Config(pHS230IfPara->CommPara);
									
  	return true;
}

bool CHS230If::Close() 
{ 
	if (m_Comm.IsOpen())
		m_Comm.Close();
			
	return true;
}


bool CHS230If::Send(BYTE* pbTxBuf, WORD wLen)
{
	if (wLen > 0)
	{
		TraceFrm("<-- CHS230If::Send:", pbTxBuf, wLen);
	
		if (m_Comm.Write(pbTxBuf, wLen) == wLen)
			return true;
		else
			return false;
	}
	return false;
}

WORD CHS230If::Receive(BYTE* pbRxBuf, WORD wBufSize)
{
	WORD wLen = (WORD )(m_Comm.Read(pbRxBuf, wBufSize));
	if(wLen > 0)
	{
		BYTE bBuf[32];
		
	    TraceFrm("--> CHS230If::Receive:", pbRxBuf, wLen);
   		if (m_Comm.Write((void *)"AT+DB=?\r", 8) != 8) 
        	return 0;
	
		DWORD dwRead = m_Comm.Read(bBuf, 120);
		if (dwRead > 0)
		{
			bBuf[dwRead] = 0;
			TraceFrm("<-- CHS230If::Receive:", bBuf, dwRead);		
			
			if (dwRead==7 && bBuf[6]==0x0d)
				m_cCSQ = bBuf[5];
			else
				m_cCSQ = bBuf[6];
		}	    
	}

	return wLen;
}

int CHS230If::ATCommand(BYTE* pbCmd, WORD wCmdLen, BYTE* pbAnsOK, WORD wAnsLen, WORD nWaitSeconds)
{
	BYTE bBuf[1024];
	WORD i = 0;
	WORD wRead = 0;
	
	TraceFrm("<-- CHS230If::ATCommand:", pbCmd, wCmdLen);
    if (m_Comm.Write(pbCmd, wCmdLen) != wCmdLen)
		return 1;
		
	do
	{
		if (i!=0 && i!=1)   //对于nWaitSeconds==0的情况,Read()两回,不Sleep()
			Sleep(1000);

		WORD wLen = m_Comm.Read(&bBuf[wRead], 1000-wRead);
		wRead += wLen;
		if (wRead >= 1000)
		{
			wRead = 0;   
			    //相当于本次和以前接收到的数据都丢弃,一般情况下不会发生这样的情况,只是一种预防措施
				//所以丢弃就丢弃了
		}
		
		bBuf[wRead] = 0;
		if (memcmp(bBuf+2, pbAnsOK, wAnsLen) >= 0)   //接收到正确回答
		{
			TraceFrm("CHS230If::ATCommand : rx OK ", bBuf, wRead);
			return 0;
		}
		
		i++;
		if (i == 1)    //对于nWaitSeconds==0的情况,至少还得再来一次
		{
			continue;
		}
		else if (i >= nWaitSeconds)
		{
			break;
		}
	} while(1);

	if (wRead != 0)
	{
		bBuf[wRead] = 0;
		TraceFrm("CHS230If::ATCommand : rx ", bBuf, wRead);
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CHS230If::ATCommand : rx no answer.\r\n", bBuf));
	}

	return 2;
}

//描述：复位操作
//返回: IF_RST_OK复位成功,IF_RST_HARDFAIL硬复位失败,
int CHS230If::ResetIf()
{
	WORD wTryCnt = 0;
	BYTE bBuf[64];
	const char* szAddr = "AT+ADDR=";
	const char* szChn = "AT+CHN=";
	const char* szPow = "AT+POW=";
	BYTE bOK[] = "OK";
	
	if (!m_Comm.IsOpen())
	{
		DTRACE(DB_FAPROTO, ("CHS230If::ResetIf: re open comm for at\n"));
		m_Comm.Open();	
	}
	
	memcpy(bBuf, szAddr, strlen(szAddr));
	bBuf[strlen(szAddr)] = (m_pHS230IfPara->dwAddr>>0)&0xff;
	bBuf[strlen(szAddr)+1] = (m_pHS230IfPara->dwAddr>>8)&0xff;
	bBuf[strlen(szAddr)+2] = ',';
	bBuf[strlen(szAddr)+3] = (m_pHS230IfPara->dwAddr>>16)&0xff;
	bBuf[strlen(szAddr)+4] = (m_pHS230IfPara->dwAddr>>24)&0xff;
	bBuf[strlen(szAddr)+5] = 0x0d;
	if (ATCommand(bBuf, strlen(szAddr)+6, bOK, 2) != 0)
		return IF_RST_HARDFAIL;
	
	memcpy(bBuf, szChn, strlen(szChn));
	bBuf[strlen(szChn)] = m_pHS230IfPara->bChn;
	bBuf[strlen(szChn)+1] = 0x0d;
	if (ATCommand(bBuf, strlen(szChn)+2, bOK, 2) != 0)
		return IF_RST_HARDFAIL;

	memcpy(bBuf, szPow, strlen(szPow));
	bBuf[strlen(szPow)] = (m_pHS230IfPara->wPow>>8)&0xff;
	bBuf[strlen(szPow)+1] = m_pHS230IfPara->wPow&0xff;
	bBuf[strlen(szPow)+2] = 0x0d;
	if (ATCommand(bBuf, strlen(szPow)+3, bOK, 2) != 0)
		return IF_RST_HARDFAIL;
	
	DTRACE(DB_FAPROTO, ("CHS230If::ResetIf: ResetIf OK.\n"));
	
	return IF_RST_OK;
}

//描述：测试modem状态是否正常
//返回: true-正常, false-异常
void CHS230If::KeepAlive()
{
    DWORD dwClick = GetClick();
	if (dwClick-m_dwRxClick>5*60 && dwClick-m_dwBeatClick>5*60)//每5分钟检测一次modem状态
	{	//刚开始时dwBeatClick为0，dwNewClick不为0，能马上发
		BYTE bBuf[64];
		const char* szAddr = "ADDR=";
		const char* szChn = "CHN=";
		const char* szPow = "POW=";

		memcpy(bBuf, szAddr, strlen(szAddr));
		bBuf[strlen(szAddr)] = (m_pHS230IfPara->dwAddr>>0)&0xff;
		bBuf[strlen(szAddr)+1] = (m_pHS230IfPara->dwAddr>>8)&0xff;
		bBuf[strlen(szAddr)+2] = ',';
		bBuf[strlen(szAddr)+3] = (m_pHS230IfPara->dwAddr>>16)&0xff;
		bBuf[strlen(szAddr)+4] = (m_pHS230IfPara->dwAddr>>14)&0xff;
		if (ATCommand((BYTE *)"AT+ADDR=?\r", 10, bBuf, strlen(szAddr)+5) != 0)
	        m_wState = IF_STATE_RST;

		memcpy(bBuf, szChn, strlen(szChn));
		bBuf[strlen(szChn)] = m_pHS230IfPara->bChn;
		if (ATCommand((BYTE *)"AT+CHN=?\r", 9, bBuf, strlen(szChn)+1) != 0)
	        m_wState = IF_STATE_RST;

		memcpy(bBuf, szPow, strlen(szPow));
		bBuf[strlen(szPow)] = (m_pHS230IfPara->wPow>>8)&0xff;
		bBuf[strlen(szPow)+1] = m_pHS230IfPara->wPow&0xff;
		if (ATCommand((BYTE *)"AT+POW=?\r", 9, bBuf, strlen(szPow)+2) != 0)
	        m_wState = IF_STATE_RST;
	        
    	m_dwBeatClick = dwClick;
		DTRACE(DB_FAPROTO, ("CHS230If::KeepAlive: test at click %d\n", dwClick));
	}    
}

void CHS230If::OnResetFail()
{
    //m_wState = IF_STATE_RST;
}

void CHS230If::OnResetOK()
{
    m_wState = IF_STATE_TRANS;
}
