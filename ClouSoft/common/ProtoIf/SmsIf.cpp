#include "stdafx.h"
#include <stdio.h>
#include "FaCfg.h"
#include "FaConst.h"
#include "sysarch.h" 
#include "sysapi.h"
#include "SmsIf.h"
#include "ProHook.h"
#include "ProIfConst.h"
#include "Info.h"

#ifdef SYS_VDK
#include "lwip/ctrl.h"
#include "ppp.h"
#endif //SYS_VDK

	
/////////////////////////////////////////////////////////////////////////////////////////////////////
//CGprsIf

extern CGprsWorker g_GprsWorker;

CSmsIf::CSmsIf()
{
	//m_wCnMode = CN_MODE_SOCKET;
	//m_bRstMode = GPRS_RST_ON_IDLE;	  //GPRS模块的复位模式,复位后处于空闲状态还是短信状态
	m_wIfType = IF_SMS;
}


CSmsIf::~CSmsIf()
{
	
}


bool CSmsIf::Init(TSmsIfPara* pSmsIfPara)
{
	if (!CProtoIf::Init(&pSmsIfPara->IfPara)) //基类的初始化
		return false;
	
    m_pSmsIfPara = pSmsIfPara;
	//m_iGprsUser = g_GprsWorker.ReqUserID();
	m_wState = IF_STATE_TRANS; 
	return true;//ReInit(m_pGprsPara);
}

//描述:接收串口来的数据,如果接收循环缓冲区中还有数据,则返回循环缓冲区中的数据,
//     否则调用串口接收函数,直接等待串口的数据到来
//参数:@pbRxBuf 用来接收返回数据的缓冲区,
//     @wBufSize 接收缓冲区的大小
//返回:返回数据的长度
WORD CSmsIf::Receive(BYTE* pbRxBuf, WORD wBufSize)
{
	WORD wLen = 0;
	
	wLen = g_GprsWorker.ReceiveSms(pbRxBuf);
	if (wLen > 0)
		DTRACE(DB_FAPROTO, ("CSmsIf::Receive : rx len=%d in sms mode\n", wLen));
			
	return wLen;
}

bool CSmsIf::Send(BYTE* pbTxBuf, WORD wLen)
{
	DTRACE(DB_FAPROTO, ("CSmsIf::Send : tx len=%d in sms mode\n", wLen));
	return g_GprsWorker.SendSms(pbTxBuf, wLen);
}

