#include "stdafx.h"
#include <stdio.h>
#include "FaCfg.h"
#include "FaConst.h"
#include "sysarch.h" 
#include "sysapi.h"
#include "GprsIf.h"
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

CGprsIf::CGprsIf()
{
	m_wSignStrength = 0;
	m_wCnMode = CN_MODE_SOCKET;
	m_bRstMode = GPRS_RST_ON_IDLE;	  //GPRS模块的复位模式,复位后处于空闲状态还是短信状态

	//统计数据
	m_wGprsTxCnt = 0;
	m_wGprsRxCnt = 0;
	m_wSmsTxCnt = 0;
	m_wSmsRxCnt = 0;
	
	m_dwErrRstClick = 0;
	m_dwSignClick = 0;
	m_dwSmsOverflowClick = 0;
	m_dwPeriodDropInterv = 0; //时段在线模式的激活方式自动掉线时间,单位分钟
	
	m_fRstInConnectFail = true;	//在连接失败到重试次数后复位接口
	//m_iPd = -1;	//ppp的设备号,只在lwip中用到
	m_iGprsUser = -1;
	
	m_wIfType = IF_GPRS;
}


CGprsIf::~CGprsIf()
{
	
}


bool CGprsIf::ReInit(TGprsPara* pGprsPara)
{	
	m_pGprsPara = pGprsPara;
	m_wCnMode = m_pGprsPara->wCnMode; 
	
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)
	{
		if (!CSocketIf::Init(&pGprsPara->SocketPara)) //基类的初始化
			return false;
	}
	else if (m_wCnMode == CN_MODE_EMBED)	
	{		
		m_pSocketPara = &pGprsPara->SocketPara;
		if (!CProtoIf::Init(&pGprsPara->SocketPara.IfPara)) //基类的初始化
			return false;
		
		if (!m_embdGprsIf.Init(pGprsPara)) //基类的初始化
			return false;		
	}
	else
	{
		if (!CProtoIf::Init(&pGprsPara->SocketPara.IfPara)) //基类的初始化
			return false;		
	}
				
	//在非连续在线的情况下,wCnMode设置为GPRS的通道模式,
	//所以非连续在线上电后会首先以GPRS登录一次
	if (m_wCnMode == CN_MODE_ETHSCK)	//以太网只能永久在线
	{
		m_wState = IF_STATE_CONNECT;
	}
	else if (m_pGprsPara->bOnlineMode == ONLINE_M_JIT)	//即时在线,有需要才上线
	{
		m_wState = IF_STATE_DORMAN;
	}
	else if (m_pGprsPara->bOnlineMode == ONLINE_M_PERSIST) //永久在线
	{
	    m_wState = IF_STATE_RST;
	}
	else //激活和时段在线
	{
		m_wState = IF_STATE_RST;
		
        m_wCnMode = m_pGprsPara->wCnMode;
        if (m_pGprsPara->fRstOnSms)
        {    
            if (m_pGprsPara->dwPowerupDropInterv == 0) //上电激活的自动掉线时间,设为0自动取消上电激活
                m_wCnMode = CN_MODE_SMS;
            else if (GetClick() >= m_pGprsPara->dwPowerupDropInterv*60)	//m_pGprsPara->dwPowerupDropInterv==0 参数重新配置引起接口重新初始化,超过了上电激活时间也不上线
                m_wCnMode = CN_MODE_SMS;
            //else							//其它的情况都要上电激活
            //	m_wCnMode = m_pGprsPara->wCnMode;
        }		
			//时段在线模式先把模块置于空闲状态,
			//在DoIfRelated()等到上线的时段到了,才复位模块
	}	
	
	m_bRstMode = GetGprsRstMode();	  //GPRS模块的复位模式,复位后处于空闲状态还是短信状态
	g_GprsWorker.SetWorkMode(m_iGprsUser, m_bRstMode);

	DTRACE(DB_FAPROTO, ("CGprsIf::Init: if(%s) init to CN mode = %s, online mode = %s, BeatSeconds=%ld, m_iGprsUser=%d\n", 
						GetName(),
						CnModeToStr(m_wCnMode), 
						OnlineModeToStr(m_pGprsPara->bOnlineMode),
						GetBeatSeconds(),
						m_iGprsUser));
	m_dwFluxOverClick = 0;	//流量超标的起始时标,一旦掉线重新判断
	return true;
}

//描述:取得GPRS模块的复位模式,复位后处于空闲状态还是短信状态
//返回:GPRS_RST_ON_IDLE复位后处于空闲状态,GPRS_RST_ON_SMS复位后处于短信状态
BYTE CGprsIf::GetGprsRstMode()
{
	BYTE bRstMode = GPRS_RST_ON_IDLE;
	if (m_wCnMode == CN_MODE_ETHSCK)	//以太网只能永久在线
		return bRstMode;
		
	if ((m_pGprsPara->bOnlineMode==ONLINE_M_PERIOD || m_pGprsPara->bOnlineMode==ONLINE_M_ACTIVE) &&
		!m_pGprsPara->fRstOnSms)			//是否复位到短信模式，主要针对激活模式和时段在线模式
	{
		return bRstMode;
	}

	if (m_pGprsPara->bOnlineMode==ONLINE_M_PERIOD 
		|| m_pGprsPara->bOnlineMode==ONLINE_M_ACTIVE 
		|| m_pGprsPara->bOnlineMode==ONLINE_M_SMS 
		|| (m_pGprsPara->bOnlineMode==ONLINE_M_PERSIST && m_pGprsPara->fEnableFluxCtrl)
		|| m_pGprsPara->bOnlineMode==ONLINE_M_DMINSMS) 
	{	//						在线模式			&&		允许流量控制
	    bRstMode = GPRS_RST_ON_SMS;
	}

	return bRstMode;
}

bool CGprsIf::Init(TGprsPara* pGprsPara)
{
    m_pGprsPara = pGprsPara;
	m_iGprsUser = g_GprsWorker.ReqUserID();
	return ReInit(m_pGprsPara);
}	

//描述：在运行过程中可以重新设置通道模式，方便在GPRS socket/模块协议栈和以太网间切换
bool CGprsIf::ResetCnMode(WORD wCnMode)
{
	m_pGprsPara->wCnMode = wCnMode;
	return ReInit(m_pGprsPara); 
}

char* CGprsIf::CnModeToStr(WORD wMode)
{
	static char* pszCnModeToStr[] = {"socket",			//0
					  	  			"sms",				//1
						    		"embed tcp/ip",		//2
						    		"comm",				//3
						    		"at cmd",			//4
						    		"eth socket",		//5
						    	  };
	
	if (wMode < CN_MODE_NUM)
		return pszCnModeToStr[wMode];
	else
		return "unknown";
}	


char* CGprsIf::OnlineModeToStr(WORD wMode)
{
	static char* pszOnlineModeToStr[] = {"unknown",
										 "persist",
					  	  				 "active(nononline)",
						    			 "period",
						    			 "sms"};
	
	if (wMode <= 3)
		return pszOnlineModeToStr[wMode];
	else
		return "unknown";
}

WORD CGprsIf::GetMaxFrmBytes()
{ 
	return m_wCnMode==CN_MODE_SMS ? m_pGprsPara->wSmsMaxFrmBytes :
					  m_pGprsPara->SocketPara.IfPara.wMaxFrmBytes; 
}

bool CGprsIf::Close()
{
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)
	{
		return CSocketIf::Close();
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{
		return m_embdGprsIf.Close();
	}
	return true;
}

//描述:接收串口来的数据,如果接收循环缓冲区中还有数据,则返回循环缓冲区中的数据,
//     否则调用串口接收函数,直接等待串口的数据到来
//参数:@pbRxBuf 用来接收返回数据的缓冲区,
//     @wBufSize 接收缓冲区的大小
//返回:返回数据的长度
WORD CGprsIf::Receive(BYTE* pbRxBuf, WORD wBufSize)
{
	WORD wLen = 0;
	//if (g_GprsWorker.IsOnline() && m_wCnMode!=CN_MODE_SMS)
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)
	{
		return CSocketIf::Receive(pbRxBuf, wBufSize);
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{
		wLen = m_embdGprsIf.Receive(pbRxBuf, wBufSize);
		if (!m_embdGprsIf.IsIfValid())
			CProtoIf::DisConnect();
		return wLen;
	}
	else if (m_wCnMode == CN_MODE_SMS)
	{
		wLen = g_GprsWorker.ReceiveSms(pbRxBuf);
		if (wLen > 0)
			DTRACE(DB_FAPROTO, ("CGprsIf::Receive : rx len=%d in sms mode\n", wLen));
			
		return wLen;
	}
	else
	{
		Sleep(1000);
		return 0;
	}
	return 0;
}

void CGprsIf::OnRcvFrm()
{
	m_dwRstIfClick = m_dwRxClick = GetClick();
	m_embdGprsIf.OnRcvFrm();
}

bool CGprsIf::Send(BYTE* pbTxBuf, WORD wLen)
{
	//if (g_GprsWorker.IsOnline())
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)
	{
		if (m_bGprsDataSrc == DATA_SRC_SMS)//数据来源
			return g_GprsWorker.SendSms(pbTxBuf, wLen);
		else
			return CSocketIf::Send(pbTxBuf, wLen);
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{	
		return m_embdGprsIf.Send(pbTxBuf, wLen);
	}
	else if (m_wCnMode == CN_MODE_SMS)
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::Send : tx len=%d in sms mode\n", wLen));
		return g_GprsWorker.SendSms(pbTxBuf, wLen);
	}
	else
	{
		return false;
	}
	return false;
}

//描述：复位操作
//返回: IF_RST_OK复位成功,IF_RST_HARDFAIL硬复位失败,
//		IF_RST_SOFTFAIL软复位失败(协议层)
int CGprsIf::ResetIf()
{
	WORD wTryCnt = 0;
	
again:
	if (m_wState == IF_STATE_DORMAN)	//已经进入休眠模式，就此返回
	{
		DisConnect();
		ReqDorman(m_iGprsUser); //ReqOffline(m_iGprsUser);

		return IF_RST_OK;
	}
	
	if (m_wCnMode == CN_MODE_ETHSCK) //以太网SOCKET通信模式
	{
#ifndef SYS_WIN		
		//ModemPowerOff();	//关模块电源至少要2秒以上
#endif
		
		return IF_RST_OK;
	}
	
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_EMBED) 	//基于TCP/IP的通信模式
	{
		DisConnect();
		ReqOffline(m_iGprsUser);

		m_iLastErr = ReqOnline(m_iGprsUser);
		if(m_iLastErr == GPRS_ERR_OK)
			return IF_RST_OK;
		else
			return IF_RST_SOFTFAIL;
	}
	else
	{
		DisConnect();
		ReqOffline(m_iGprsUser);

		return IF_RST_OK;
	}
	
	wTryCnt++;
	if (wTryCnt >= 2)
		return IF_RST_SOFTFAIL;
		 
	goto again;
}

//描述:给外部调用，让接口掉线
bool CGprsIf::RequestOffline()
{	
	DisConnect();
 	return ReqOffline(m_iGprsUser);
}

bool CGprsIf::Connect()
{
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)  //基于TCP/IP的通信模式
	{
		return CSocketIf::Connect();
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{
		return m_embdGprsIf.Connect();
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::Connect : nothing to do in %s mode\n", CnModeToStr(m_wCnMode)));
	}	

	return true; 
}

bool CGprsIf::DisConnect()
{	
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)  //基于TCP/IP的通信模式
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::DisConnect .\r\n"));
		if (CSocketIf::DisConnect() == false)
			return false;
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::DisConnect .\r\n"));
		CProtoIf::DisConnect();
		if (m_embdGprsIf.DisConnect() == false)
			return false;
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::DisConnect : nothing to do in %s mode\n", CnModeToStr(m_wCnMode)));
	}	

	return 	CProtoIf::DisConnect();
}

void CGprsIf::KeepAlive()
{
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)  //基于TCP/IP的通信模式
	{
		if ((m_pGprsPara->bOnlineMode == ONLINE_M_PERSIST) && 
		(m_pGprsPara->fEnableFluxCtrl) && 
		(m_wCnMode==CN_MODE_SOCKET) && //当前已经处于在线状态
		IsFluxOver())	//流量超标
		{
			return;  //不再发心跳，也不再检测
		}

		return CSocketIf::KeepAlive();
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{
		//m_embdGprsIf这个现在只是作为CGprsIf的成员，
		//CGprsIf绑定协议的时候并没有给其成员也绑定协议，
		//这里做这个工作
		if (m_embdGprsIf.GetProto() == NULL)
			m_embdGprsIf.AttachProto(m_pProto);	

		if (!((m_pGprsPara->bOnlineMode == ONLINE_M_PERSIST) && 
		(m_pGprsPara->fEnableFluxCtrl) && 
		IsFluxOver()))	//流量超标	
		{
			m_embdGprsIf.KeepAlive();
		}

		if (!m_embdGprsIf.IsIfValid())
			CProtoIf::DisConnect();
	}	
}
	
void CGprsIf::OnConnectFail() 
{	
	if ((m_pGprsPara->bOnlineMode==ONLINE_M_PERIOD
		 || m_pGprsPara->bOnlineMode==ONLINE_M_ACTIVE
		 || m_pGprsPara->bOnlineMode==ONLINE_M_DMINSMS) && //激活模式/非连续在线模式
		m_pGprsPara->fRstOnSms && m_wCnMode!=CN_MODE_SMS &&	m_wCnMode!=CN_MODE_ETHSCK && //处于GPRS模式
		m_wConnectFailCnt+1 >= GetConnectNum())
	{			
		//GPRS连接不成功,自动切换到短信模式,不进入休眠状态
		
		DTRACE(DB_FAPROTO, ("CGprsIf::OnConnectFail : %s switch to sms mode\n", GetName()));
		
		DisConnect();	//先断开socket方式下的连接,不然切换到短信模式下没法调用
		m_wState = IF_STATE_RST; 
		m_wCnMode = CN_MODE_SMS; 
		m_wConnectFailCnt = 0; 
			//避免GPRS连接不成功, 在调用CProtoIf::OnConnectFail()时
			//进入休眠状态,m_wConnectFailCnt重新开始计数,
			//如果在短信方式下还是失败超过/重试次数,
			//则要进入休眠模式,避免损坏模块
		if (m_pGprsPara->bOnlineMode == ONLINE_M_DMINSMS)
			m_dwDormanClick = GetClick();
	}
	
	if(m_wCnMode == CN_MODE_EMBED)
	{	
		CProtoIf::OnConnectFail();
		m_embdGprsIf.OnConnectFail();
	}
	else
		CSocketIf::OnConnectFail();
}


void CGprsIf::OnResetFail() 
{	
	if ((m_pGprsPara->bOnlineMode==ONLINE_M_PERIOD 
		|| m_pGprsPara->bOnlineMode==ONLINE_M_ACTIVE
		|| m_pGprsPara->bOnlineMode==ONLINE_M_DMINSMS) && //激活模式/非连续在线模式
		m_pGprsPara->fRstOnSms && m_wCnMode!=CN_MODE_SMS &&	m_wCnMode!=CN_MODE_ETHSCK && //处于GPRS模式
		m_wResetFailCnt+1 >= m_pIfPara->wRstNum)
	{			
		//GPRS连接不成功,自动切换到短信模式
		m_wCnMode = CN_MODE_SMS; 
		m_wResetFailCnt = 0; 
			//避免GPRS连接不成功, 在调用CProtoIf::OnConnectFail()时
			//进入休眠状态,m_wConnectFailCnt重新开始计数,
			//如果在短信方式下还是失败超过/重试次数,
			//则要进入休眠模式,避免损坏模块
		if (m_pGprsPara->bOnlineMode == ONLINE_M_DMINSMS)
			m_dwDormanClick = GetClick();			
	}
	
	CProtoIf::OnResetFail();
}

//描述:在协议登陆成功时调用
void CGprsIf::OnLoginOK()
{ 
	if (m_wCnMode == CN_MODE_EMBED)
	{
		CProtoIf::OnLoginOK();
		m_embdGprsIf.ResetIPUseCnt();
	}
	else
		CSocketIf::OnLoginOK();
}


//描述:在协议登陆失败时调用,比如多少次失败后断开连接
void CGprsIf::OnLoginFail() 
{	
	m_wLoginFailCnt++;
	
	WORD wLoginRstNum = m_pIfPara->wLoginRstNum;
	if (wLoginRstNum == 0)
		wLoginRstNum = 1;
		
	WORD wLoginNum = m_pIfPara->wLoginNum;
	if (wLoginNum == 0)
		wLoginNum = 1;
	
	if (m_wLoginFailCnt%wLoginRstNum != 0)
	{					//登录失败的次数还没到断开连接的次数
		if (m_pSocketPara->bRandLoginFlg & RAND_ON_FAIL) //失败时进行随机延时
			StateToDorman(m_wState);	//从某个状态暂时切换到休眠状态，休眠完还要转回来
		else
			Sleep(m_pIfPara->dwLoginInterv*1000); //登录间隔
	}
	else if (m_wLoginFailCnt >= wLoginRstNum*wLoginNum)
	{					
		m_wLoginFailCnt = 0;
		
		if ((m_pGprsPara->bOnlineMode==ONLINE_M_PERIOD 			 
			|| m_pGprsPara->bOnlineMode==ONLINE_M_ACTIVE
			|| m_pGprsPara->bOnlineMode==ONLINE_M_DMINSMS) && //激活模式/非连续在线模式
			m_pGprsPara->fRstOnSms && m_wCnMode!=CN_MODE_SMS && m_wCnMode!=CN_MODE_ETHSCK)	//处于GPRS模式
		{
			//GPRS连接不成功,自动切换到短信模式
			DTRACE(DB_FAPROTO, ("CGprsIf::OnLoginFail : %s switch to sms mode\n", GetName()));
		
			DisConnect();	//先断开socket方式下的连接,不然切换到短信模式下没法调用
			m_wState = IF_STATE_RST; 
			m_wCnMode = CN_MODE_SMS; 
			if (m_pGprsPara->bOnlineMode == ONLINE_M_DMINSMS)
				m_dwDormanClick = GetClick();
		}
		else
		{
			if (m_wCnMode == CN_MODE_EMBED)
				m_embdGprsIf.SetMaxIpUseCnt();
			else
				SetMaxIpUseCnt();
				
			DTRACE(DB_FAPROTO, ("CGprsIf::OnLoginFail: go to dorman\n"));
			EnterDorman();
		}
	}
	else		//登录失败的次数达到了断开连接的次数的整数倍
	{			//m_wLoginFailCnt%wLoginRstNum==0 && m_wLoginFailCnt<wLoginRstNum*wLoginNum
		if (m_wCnMode == CN_MODE_EMBED)
			m_embdGprsIf.SetMaxIpUseCnt();
		else
			SetMaxIpUseCnt();
		
		DisConnect();	//只断开连接,不清零登录失败计数,
						//达到wLoginRstNum*wReTryNum次后进入休眠状态
	}		
}

//描述:检查是否需要激活,
//返回:如果需要激活则返回true,否则返回false
//备注:在下列情况下需要激活:1.接收激活消息(短信/振铃);2.主动上报
bool CGprsIf::CheckActivation()
{
	//return GetInfo(INFO_ACTIVE);
	
	if (GetInfo(m_pGprsPara->SocketPara.IfPara.wInfoActive)) //收到了短信激活帧 INFO_ACTIVE
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::CheckActivation : switch to gprs mode due to rx activate info\n"));
		return true;
	}	
	
	/*if (m_pGprsPara->fEnableRingActive) //允许振铃激活  用INFO_ACTIVE实现
	{
		if (m_pModem->HaveRing()) //是否收到过振铃信号
		{
			DTRACE(DB_FAPROTO, ("CGprsIf::CheckActivation : switch to gprs mode due to ring\n"));
			return true;
		}
	}*/
	
	if (m_pGprsPara->fEnableAutoSendActive) //允许主动上报激活
	{
		if (m_pProto->IsNeedAutoSend()) //需要主动上报
		{
			DTRACE(DB_FAPROTO, ("CGprsIf::CheckActivation : switch to gprs mode due to auto send\n"));
			return true;
		}
	}	

	return false;
}

//描述:做一些各个接口相关的非标准的事情,
//		本接口要做的事情有:
//		1.模式切换,比如非连续在线方式下,GPRS和SMS间的切换
void CGprsIf::DoIfRelated()
{
	//非连续在线模式下,要实现的切换有:
	//GPRS一段时间没有通信,切换回SMS方式
	//SMS收到短信唤醒或电话唤醒,切换到GPRS方式
	
	DWORD dwClick = GetClick();

	if (m_fSetIdleCmd)	//收到外部的断开连接命令
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s rx set idle cmd.\r\n", GetName()));

		if ((m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK || m_wCnMode==CN_MODE_EMBED) && 
			m_wState>=IF_STATE_CONNECT) //当前已经处于在线状态IF_STATE_TRANS
		{
			DisConnect();	//先断开socket方式下的连接,不然切换到短信模式下没法调用
		}

		m_fSetIdleCmd = false;

		m_dwDormanClick = 0; //没有规定休眠的时间,相当于处于无限期的休眠(空闲)状态,
		m_wState = IF_STATE_DORMAN;
		ResetIf();
		m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式
	}

	if (m_fDisConnCmd)	//收到外部的断开连接命令
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s rx disconnect cmd, m_wCnMode : %d, m_wState : %d.\r\n", 
							GetName(), m_wCnMode, m_wState));

		if ((m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK || m_wCnMode==CN_MODE_EMBED) && 
			m_wState>=IF_STATE_CONNECT) //当前已经处于在线状态IF_STATE_TRANS
		{
			DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s disconnect from sockect mode, m_dwDormanInterv=%ld...\r\n", 
								GetName(), m_dwDormanInterv));
			CProto* pProto = GetProto();
			if (pProto!=NULL && m_wState==IF_STATE_TRANS) 
				pProto->Logoff();

			DisConnect();	//先断开socket方式下的连接,不然切换到短信模式下没法调用
			if (m_dwDormanInterv != 0)	//临时设定的休眠间隔，单位秒
			{
				m_dwDormanClick = GetClick();	 //进入休眠的开始时间
				m_wState = IF_STATE_DORMAN;
			}
			else
			{
				m_wState = IF_STATE_RST; 
			}

			if (m_bRstMode == GPRS_RST_ON_SMS)
				m_wCnMode = CN_MODE_SMS; //切换到SMS模式
			else
				m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式

			//一些模式下用到的变量也统一清理一下
			m_dwSignClick = 0;
			m_dwSmsOverflowClick = 0;
			m_dwPeriodDropInterv = 0;
			m_dwFluxOverClick = 0;	//流量超标的起始时标,一旦掉线重新判断
		}
		else
		{
			if (m_pGprsPara->bOnlineMode != ONLINE_M_DMINSMS)//休眠时进入短信模式，得休眠，避免一直在短信模式不出来
				m_dwDormanInterv = 0;	//临时设定的休眠间隔，单位秒
		}

		m_fDisConnCmd = false;
	}

	if (m_wCnMode == CN_MODE_ETHSCK)	//如果是以太网，下面的在线模式跟它无关，就此返回
	{
		CProtoIf::DoIfRelated();
		return;
	}

	if (m_pGprsPara->bOnlineMode == ONLINE_M_ACTIVE)
	{						//激活模式/非连续在线模式
		if (m_wCnMode == CN_MODE_SMS)	//当前处于短信模式下
		{
			if (CheckActivation()) //需要激活
			{	
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to gprs mode\n", GetName()));
				DisConnect();
				m_wState = IF_STATE_RST; 
				m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式
												   //模块复位由线程函数来处理
			}
		}
		else	//当前处于GPRS模式下
		{
			//在dwActiveDropInterv分钟内没有接收到主站的报文,则主动掉线
			if (m_wState==IF_STATE_TRANS && dwClick-m_dwRxClick>m_pGprsPara->dwActiveDropInterv*60)
			{	
				if (dwClick > m_pGprsPara->dwPowerupDropInterv*60) 
				{			//上电激活的自动掉线时间,设为0自动取消上电激活
					DisConnect();
					
					if (m_pGprsPara->fRstOnSms)
					{
						DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to sms mode due to active timeouts\r\n", GetName()));
						m_wState = IF_STATE_RST; 
						m_wCnMode = CN_MODE_SMS; //切换到SMS模式
										 	 //模块复位由线程函数来处理
						m_dwSignClick = 0;
						m_dwSmsOverflowClick = 0;
					}
					else
					{
						DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to idle mode due to active timeouts\r\n", GetName()));
						m_dwDormanClick = 0; //没有规定休眠的时间,相当于处于无限期的休眠(空闲)状态,
						m_wState = IF_STATE_DORMAN;
						ResetIf();
						m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式
					}
				}
			}
			else if (m_wState==IF_STATE_DORMAN && !m_pGprsPara->fRstOnSms)
			{
				if (CheckActivation()) //需要激活
				{	
					DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s active to gprs mode from dorman\n", GetName()));
					m_dwDormanClick = 0;
					DisConnect();
					m_wState = IF_STATE_RST; 
					m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式
													   //模块复位由线程函数来处理
				}
			}
		}		
	}
	else if (m_pGprsPara->bOnlineMode == ONLINE_M_PERIOD)
	{									//时段在线模式
		//TTime time;
		//GetCurTime(&time);
		bool fInPeriod = GprsIsInPeriod();

		if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_EMBED) //当前已经处于在线状态
		{
			if (m_wState==IF_STATE_TRANS && //只有在处于传输状态才做切换,其它状态如果失败自动会切换到短信状态
				!fInPeriod && //当前时段要掉线
				dwClick>m_pGprsPara->dwPowerupDropInterv*60 &&
				dwClick-m_dwRxClick>m_dwPeriodDropInterv*60)
			{	//上电激活的自动掉线时间,设为0自动取消上电激活
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to period offline\n", GetName()));
				
				DisConnect();	//先断开socket方式下的连接,不然切换到短信模式下没法调用
				m_wState = IF_STATE_RST; 
				m_wCnMode = CN_MODE_SMS; //切换到SMS模式
										 //模块复位由线程函数来处理
				m_dwPeriodDropInterv = 0;
				m_dwSignClick = 0;
				m_dwSmsOverflowClick = 0;
			}
		}
		else //当前处于短信模式下 或者 当前处于掉线状态
		{
			if (fInPeriod) //当前要时段在线
			{ 	
				m_dwPeriodDropInterv = 0;
						
				m_wState = IF_STATE_RST; 
				m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式
												   //模块复位由线程函数来处理
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to period online\n", GetName()));
			}
			else if (CheckActivation())
			{
				m_dwPeriodDropInterv = m_pGprsPara->dwActiveDropInterv;
				
				m_wState = IF_STATE_RST; 
				m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式
												   //模块复位由线程函数来处理
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s active to online in period mode\n", GetName()));
			}	
		}
	}
	else if (m_pGprsPara->bOnlineMode == ONLINE_M_JIT)
	{		//JUST IN TIME 按需要即时上线,如单独的上报端口
		bool fNeedSend = m_pProto->IsNeedAutoSend();
		if (m_dwDormanClick==0 && m_wState==IF_STATE_DORMAN)
		{	//模块处于空闲状态 而不是休眠状态
			if (fNeedSend)
			{
				m_wState = IF_STATE_RST;	//退出IF_STATE_DORMAN状态
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to JIT mode\n", GetName()));
			}	
		}
		else if (m_wState == IF_STATE_TRANS)
		{	//当前已经处于在线状态 m_wCnMode == CN_MODE_SOCKET
			if (!fNeedSend)
			{
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to idle mode\n", GetName()));
				m_dwDormanClick = 0; //没有规定休眠的时间,相当于处于无限期的休眠(空闲)状态,
				m_wState = IF_STATE_DORMAN;
				ResetIf();
				m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式
												   //模块复位由线程函数来处理
			}	
		}	
	}
	else if (m_pGprsPara->bOnlineMode==ONLINE_M_PERSIST) //持续在线模式的流量控制
	{
		if (m_pGprsPara->fEnableFluxCtrl) //允许流量控制
		{
			if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_EMBED) //当前已经处于在线状态
			{
				if (m_wState==IF_STATE_TRANS && //只有在处于传输状态才做切换,其它状态如果失败自动会切换到短信状态
					IsFluxOver())	//流量超标
				{	
					if (m_dwFluxOverClick == 0)
					{
						m_dwFluxOverClick = GetClick();	//流量超标的起始时标
						GprsOnFluxOver();		//回调函数,用于生成告警记录等用途
					}

					//上电激活的自动掉线时间,设为0自动取消上电激活
					if (GprsIsTxComplete(m_dwFluxOverClick) && //告警和主动上送数据都送完
						dwClick>m_pGprsPara->dwPowerupDropInterv*60 && dwClick-m_dwRxClick>m_pGprsPara->dwActiveDropInterv*60) 
					{	//上电激活的自动掉线时间,设为0自动取消上电激活
						DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to sms mode due to flux over\n", GetName()));
						CProto* pProto = GetProto();
						if (pProto != NULL) 
							pProto->Logoff();

						DisConnect();
						m_wState = IF_STATE_RST; 
						m_wCnMode = CN_MODE_SMS; //切换到SMS模式
												 //模块复位由线程函数来处理
						m_dwSignClick = 0;
						m_dwSmsOverflowClick = 0;
						m_dwFluxOverClick = 0;	//流量超标的起始时标,一旦掉线重新判断
						//GprsOnFluxOver();		//回调函数,用于生成告警记录等用途
					}
				}
			}
			else //当前处于短信模式下 或者 当前处于掉线状态
			{
				m_dwFluxOverClick = 0;	//流量超标的起始时标,一旦掉线重新判断
										
				if (CheckActivation())
				{
					DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to gprs mode due to active in flux ctrl\n", GetName()));
					DisConnect();
					m_wState = IF_STATE_RST; 
					m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式
				}
				else if (!IsFluxOver())	//由于月切换或者流量设置为0或者流量设得更大,责切换回在线模式
				{
					DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to gprs mode to flux not over\n", GetName()));
					DisConnect();
					m_wState = IF_STATE_RST; 
					m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式
				}
			}
		}
		else if (m_wCnMode == CN_MODE_SMS) //防止流量控制中途切换到不允许,一致处于短信状态
		{
			m_dwFluxOverClick = 0;	//流量超标的起始时标
									
			DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to gprs mode due to exit flux ctrl\n", GetName()));
			DisConnect();
			m_wState = IF_STATE_RST; 
			m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式
		}		
	}
	else if (m_pGprsPara->bOnlineMode == ONLINE_M_DMINSMS) //休眠时进入GPRS
	{
		if (m_wCnMode == CN_MODE_SMS)
		{
			if (GetClick()-m_dwDormanClick >= m_pIfPara->dwDormanInterv)
			{
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to gprs mode, %d seconds exit dormant.\n", GetName(), m_pIfPara->dwDormanInterv));
				DisConnect();
				m_wState = IF_STATE_RST; 
				m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式
			}
			else
			{
				Sleep(1000);
				if ((GetClick()%5) == 0)
					DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : do dorman remain %ld\n", m_pIfPara->dwDormanInterv + m_dwDormanClick - dwClick));
			}
		}
	}	
	
	CProtoIf::DoIfRelated();
}


//使接口进入休眠方式
void CGprsIf::EnterDorman()
{
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)
	{
		CSocketIf::EnterDorman();
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{
		CProtoIf::EnterDorman();
		m_embdGprsIf.EnterDorman();
	}			
	ResetIf();
    m_wDisConnectByPeerCnt = 0;	
	//m_fRstOk = false;  //休眠完了后接口需要复位
}    		

void CGprsIf::OnDisConnectByPeer()
{
    m_wDisConnectByPeerCnt++;
    if (m_pSocketPara->wDisConnectByPeerNum!=0 && m_wDisConnectByPeerCnt>=m_pSocketPara->wDisConnectByPeerNum) 
    {
        m_wDisConnectByPeerCnt = 0;	
        
      	if (m_pGprsPara->bOnlineMode==ONLINE_M_ACTIVE || 
            m_pGprsPara->bOnlineMode==ONLINE_M_JIT || 
            m_pGprsPara->bOnlineMode == ONLINE_M_PERIOD ||
            m_pGprsPara->bOnlineMode == ONLINE_M_DMINSMS)
        {
            if (m_pGprsPara->fRstOnSms && m_wCnMode!=CN_MODE_ETHSCK)
            {
                DTRACE(DB_FAPROTO, ("CGprsIf::OnDisConnectByPeer : %s switch to sms mode \r\n", GetName()));
                m_wState = IF_STATE_RST; 
                m_wCnMode = CN_MODE_SMS; //切换到SMS模式
                                     //模块复位由线程函数来处理
                m_dwPeriodDropInterv = 0;
                m_dwSignClick = 0;
                m_dwSmsOverflowClick = 0;
           		if (m_pGprsPara->bOnlineMode == ONLINE_M_DMINSMS)
					m_dwDormanClick = GetClick();
            }
            else
            {
                DTRACE(DB_FAPROTO, ("CGprsIf::OnDisConnectByPeer : %s switch to idle mode \r\n", GetName()));
                m_dwDormanClick = 0; //没有规定休眠的时间,相当于处于无限期的休眠(空闲)状态,
                m_wState = IF_STATE_DORMAN;
                ResetIf();
                m_wCnMode = m_pGprsPara->wCnMode;  //切换到GPRS的通道模式
            }
        }
        else
        {
			DTRACE(DB_FAPROTO, ("CGprsIf::OnDisConnectByPeer: go to dorman\n"));
            EnterDorman();
        }
    }
}

void CGprsIf::LoadUnrstPara()
{
	if (m_pfnLoadUnrstPara != NULL)  //&& m_fUnrstParaChg //m_fUnrstParaChg = false;
	{
		if ((*m_pfnLoadUnrstPara)(m_pGprsPara))	//需要重新初始化
		{
			//if (m_wState == IF_STATE_TRANS)
			{
				DisConnect();
				ReqOffline(m_iGprsUser);
				ReInit(m_pGprsPara);
			}				
		}
		else	//不需要重新初始化
		{
			BYTE bRstMode = GetGprsRstMode();	  //GPRS模块的复位模式,复位后处于空闲状态还是短信状态
			if (bRstMode != m_bRstMode)
			{
				m_bRstMode = bRstMode;
				g_GprsWorker.SetWorkMode(m_iGprsUser, m_bRstMode);
			}
		}
	}	
}

//本接口的通信协议是否需要登录
bool CGprsIf::IsNeedLogin() 
{
	if (m_wCnMode == CN_MODE_SMS)	//短信通信方式或者非连续在线切换到短信方式
		return false;				//不需要登录
	else	
		return m_pIfPara->fNeedLogin;
}


//描述:取心跳间隔,返回0表示不用心跳
DWORD CGprsIf::GetBeatSeconds() 
{
	DWORD dwClick = GetClick();
	if (m_pGprsPara->bOnlineMode == ONLINE_M_ACTIVE) //激活模式/非连续在线模式
	{									
		if (dwClick < m_pGprsPara->dwPowerupDropInterv*60)
		{			//上电激活的自动掉线时间,设为0自动取消上电激活
			return m_pGprsPara->dwPowerupBeatMinutes;	 //上电激活的心跳间隔
		}
		else
		{
			return 0;
		}	
	}
	else if (m_pGprsPara->bOnlineMode == ONLINE_M_PERIOD) //时段在线模式
	{
		if (dwClick < m_pGprsPara->dwPowerupDropInterv*60)
		{			//上电激活的自动掉线时间,设为0自动取消上电激活
			return m_pGprsPara->dwPowerupBeatMinutes;	 //上电激活的心跳间隔
		}
		else
		{
			return m_pGprsPara->SocketPara.dwBeatSeconds;
		}	
	}
	else
	{
		return m_pGprsPara->SocketPara.dwBeatSeconds;
	}
}

CEmbedGprsIf* CGprsIf::GetEmbedGprsIf()
{
    return &m_embdGprsIf;
}
