
#include "stdafx.h"
#include "FaCfg.h"
#include "FaProto.h"
#include "sysarch.h" 
#include "sysapi.h" 
#include "EmbedGprsIf.h"
#include "syssock.h" 
#include "Trace.h"
#include "ProHook.h"
#include "ProIfConst.h"
#include "GprsWorker.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//CEmbedGprsIf
extern CGprsWorker g_GprsWorker;

CEmbedGprsIf::CEmbedGprsIf()
{
	m_wIfType = IF_SOCKET;
	m_fIfValid = false;
}

CEmbedGprsIf::~CEmbedGprsIf()
{
}

bool CEmbedGprsIf::Init(TGprsPara* pTGprsPara)
{
	if (!CProtoIf::Init(&pTGprsPara->SocketPara.IfPara)) //基类的初始化
		return false;
		
	m_pTEmbedGprsPara = pTGprsPara;
	
	m_wState = IF_STATE_CONNECT;
	m_bBakIP = 0;
	m_wIPUseCnt = 0;
	m_pModem = g_GprsWorker.GetModem();
	m_pProto = NULL;
	
	return true;	
}

//描述:初始化服务器
void CEmbedGprsIf::InitSvr(int socket)
{
	m_wState = IF_STATE_TRANS;

	m_fExit = false;
	m_fExitDone = false;
}

void CEmbedGprsIf::LoadUnrstPara()
{
	if (m_pfnLoadUnrstPara!=NULL) // && m_fUnrstParaChg
	{
		m_fUnrstParaChg = false;
		if ((*m_pfnLoadUnrstPara)(m_pTEmbedGprsPara))	//需要重新初始化
		{
			//if (m_wState == IF_STATE_TRANS)
			{
				DisConnect();
			}
		}
	}	
}	

bool CEmbedGprsIf::Send(BYTE* pbTxBuf, WORD wLen)
{
	int iReLen = 0;
	DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Send: GetClick()=%d,wLen=%d.\r\n", GetClick(),wLen));
	TraceFrm("<-- CEmbedGprsIf::Send:", pbTxBuf, wLen);	
	if (m_pTEmbedGprsPara->SocketPara.fEnableFluxStat)	//是否允许流量统计,只有本socket用的是GPRS通道时才支持
		AddFlux(wLen);
	iReLen = m_pModem->Send(pbTxBuf, wLen);
	if (iReLen == wLen)
	{
		m_wGprsTxCnt++;	
		return true;
	}
	DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Send : sock tx fail iReLen=%d, wLen=%d.\r\n", iReLen, wLen));
	return false;
}

//描述:接收串口来的数据,如果接收循环缓冲区中还有数据,则返回循环缓冲区中的数据,
//     否则调用串口接收函数,直接等待串口的数据到来
//参数:@pbRxBuf 用来接收返回数据的缓冲区,
//     @wBufSize 接收缓冲区的大小
//返回:返回数据的长度
WORD CEmbedGprsIf::Receive(BYTE* pbRxBuf, WORD wBufSize)
{
	int iRecvLen = 0; 
	iRecvLen = m_pModem->Receive(pbRxBuf, wBufSize);
	if (iRecvLen > 0)
	{
		if (m_pTEmbedGprsPara->SocketPara.fEnableFluxStat)	//是否允许流量统计,只有本socket用的是GPRS通道时才支持
			AddFlux((DWORD )iRecvLen);

   		DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Receive:GetClick()=%d.\r\n",GetClick()));
   		TraceFrm("--> CEmbedGprsIf::Receive:", pbRxBuf, iRecvLen);			
	}
	else if (iRecvLen < 0)
	{
		DisConnect();
		return 0;
	}
	return iRecvLen;
}

void CEmbedGprsIf::OnRcvFrm()
{
	CProtoIf::OnRcvFrm();
//	DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Receive:OnRcvFrm()=%d.\r\n", m_dwRxClick));
}

//描述：建立连接
bool CEmbedGprsIf::Connect()
{
	DisConnect();
	
  	//主备IP切换：
	//1、在没有成功登陆主站的情况下，轮流使用主/备1/备2 IP
	//2、在登陆OK的情况下，清当前IP的使用计数，让它可以继续使用
	if (m_bBakIP > 2)
		m_bBakIP = 0;
	if (m_bBakIP == 0)	//当前要使用主IP
	{
		if (m_wIPUseCnt < m_pIfPara->wConnectNum && m_pTEmbedGprsPara->SocketPara.dwRemoteIP!=0 && m_pTEmbedGprsPara->SocketPara.wRemotePort!=0)//主 IP端口有效
		{
	        m_wIPUseCnt++;
			m_dwRemoteIP = m_pTEmbedGprsPara->SocketPara.dwRemoteIP;
			m_wRemotePort = m_pTEmbedGprsPara->SocketPara.wRemotePort;
		}
        else
        {
        	m_dwRemoteIP = m_pTEmbedGprsPara->SocketPara.dwBakIP1;
			m_wRemotePort = m_pTEmbedGprsPara->SocketPara.wBakPort1;
            m_wIPUseCnt = 0;
           	m_bBakIP = 1; //本切换到备用1 IP
        }
	}
	if (m_bBakIP == 1)	//当前要使用备用1IP
	{
		if (m_wIPUseCnt < m_pIfPara->wConnectNum && m_pTEmbedGprsPara->SocketPara.dwBakIP1!=0 && m_pTEmbedGprsPara->SocketPara.wBakPort1!=0)//备用1 IP端口有效
		{
	        m_wIPUseCnt++;
			m_dwRemoteIP = m_pTEmbedGprsPara->SocketPara.dwBakIP1;
			m_wRemotePort = m_pTEmbedGprsPara->SocketPara.wBakPort1;
		}
        else
        {
        	m_dwRemoteIP = m_pTEmbedGprsPara->SocketPara.dwBakIP2;
			m_wRemotePort = m_pTEmbedGprsPara->SocketPara.wBakPort2;
            m_wIPUseCnt = 0;
           	m_bBakIP = 2; //本切换到备用2 IP
        }
	}
	if (m_bBakIP == 2)	//当前要使用备用2IP
	{
		if (m_wIPUseCnt < m_pIfPara->wConnectNum && m_pTEmbedGprsPara->SocketPara.dwBakIP2!=0 && m_pTEmbedGprsPara->SocketPara.wBakPort2!=0)//备用2 IP端口有效
		{
			m_wIPUseCnt++;
			m_dwRemoteIP = m_pTEmbedGprsPara->SocketPara.dwBakIP2;
			m_wRemotePort = m_pTEmbedGprsPara->SocketPara.wBakPort2;
		}
		else
        {
			m_dwRemoteIP = m_pTEmbedGprsPara->SocketPara.dwRemoteIP;
			m_wRemotePort = m_pTEmbedGprsPara->SocketPara.wRemotePort;        	
            m_wIPUseCnt = 1;   //本次已经使用过一次了
            m_bBakIP = 0;  //本回切换到主IP
        }
	}
		
	DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Connect: connecting %d.%d.%d.%d %d.\r\n",
						 (m_dwRemoteIP>>24)&0xff, (m_dwRemoteIP>>16)&0xff, (m_dwRemoteIP>>8)&0xff, m_dwRemoteIP&0xff, 
						 m_wRemotePort));
	
	
	if (!m_pModem->Connect(m_pTEmbedGprsPara->SocketPara.fUdp, m_dwRemoteIP, m_wRemotePort))
	{
		DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Connect: connect failed.\r\n"));	
		return false;
	}
    DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Connect: connect ok.\r\n"));
	m_fIfValid = true;
	
	m_iLastErr = GPRS_ERR_OK;
  	return true;
}

//描述:取连接次数,GPRS或socket连接的时候,如果有备用IP端口的话,连接次数乘2
WORD CEmbedGprsIf::GetConnectNum()
{
	WORD wConnectNum = m_pIfPara->wConnectNum;
	if (m_pTEmbedGprsPara->SocketPara.dwBakIP1!=0 && m_pTEmbedGprsPara->SocketPara.wBakPort1!=0)	//备用主站IP和端口
		wConnectNum += m_pIfPara->wConnectNum;
	if (m_pTEmbedGprsPara->SocketPara.dwBakIP2!=0 && m_pTEmbedGprsPara->SocketPara.wBakPort2!=0)	//备用主站IP和端口
		wConnectNum += m_pIfPara->wConnectNum;
	return wConnectNum;
}

void CEmbedGprsIf::EnterDorman()
{
	CProtoIf::EnterDorman();
}
	
//描述:接口保活探测
void CEmbedGprsIf::KeepAlive()
{	
	DWORD dwBeatSeconds = GetBeatSeconds();
	if (dwBeatSeconds == 0) //!m_pSocketPara->fBeatOn
		return;
	
	DWORD dwClick = GetClick();
	DWORD dwBrokenTime = dwBeatSeconds + 
					 	 m_pTEmbedGprsPara->SocketPara.dwBeatTimeouts*m_pTEmbedGprsPara->SocketPara.wBeatTestTimes;
	if (m_pTEmbedGprsPara->SocketPara.wBeatTestTimes!=0 && dwClick-m_dwRxClick>dwBrokenTime)
	{	//m_pSocketPara->wBeatTestTimes配置为0,表示不自动掉线,只周期发心跳
		DisConnect(); //重新初始化
		DTRACE(DB_FAPROTO, ("CEmbedGprsIf::KeepAlive : DisConnect at click %d\n", dwClick));
	}
	else if (dwClick-m_dwRxClick > dwBeatSeconds)//40 20秒还没收到过一帧，则进行心跳检测
	{	//刚开始时dwBeatClick为0，dwNewClick不为0，能马上发
		if (dwClick-m_dwBeatClick > m_pTEmbedGprsPara->SocketPara.dwBeatTimeouts)
		{							//心跳超时时间,单位秒
			if (m_pProto != NULL)
				m_pProto->Beat();
			m_dwBeatClick = dwClick;
			DTRACE(DB_FAPROTO, ("CEmbedGprsIf::KeepAlive: heart beat test at click %d\n", dwClick));
		}
	}
}

bool CEmbedGprsIf::Close()
{
	m_pModem->Close();
	return true;
}


//描述:在接口由连接转为断开的时候调用，不管是主动断开还是被动断开
bool CEmbedGprsIf::DisConnect()
{
	m_pModem->DisConnect();
	CProtoIf::DisConnect();
	
	if (m_pTEmbedGprsPara->SocketPara.fSvr) //服务器模式下,socket断开时退出线程
		m_fExit = true;
	m_fIfValid = false;
	return true;
}

bool CEmbedGprsIf::IsIfValid()
{
	return m_fIfValid;
}

void CEmbedGprsIf::OnConnectFail()
{
	 CProtoIf::OnConnectFail();
}

//描述:在协议登陆成功时调用
void CEmbedGprsIf::OnLoginOK()
{ 
	CProtoIf::OnLoginOK();
	
	m_wIPUseCnt = 0; //在登陆OK的情况下，清当前IP的使用计数，让它可以继续使用，
					 //否则让主备IP使用相同的次数后轮流切换
}
