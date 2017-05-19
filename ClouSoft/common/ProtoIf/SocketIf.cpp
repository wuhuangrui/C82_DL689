
#include "stdafx.h"
#include "FaCfg.h"
#include "FaProto.h"
#include "ProIfAPI.h"
#include "sysarch.h" 
#include "SocketIf.h"
#include "syssock.h" 
#include "Trace.h"
#include "Modem.h"
#include "ProHook.h"
#include "ProIfConst.h"
#include "sysapi.h"
#include "GprsWorker.h"

#ifdef SYS_LINUX
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif 

/////////////////////////////////////////////////////////////////////////////////////////////////////
//CSocketIf

CSocketIf::CSocketIf()
{
	m_wIfType = IF_SOCKET;
	m_Socket = INVALID_SOCKET;
}

CSocketIf::~CSocketIf()
{
}

bool CSocketIf::Init(TSocketPara* pSocketPara)
{
	if (!CProtoIf::Init(&pSocketPara->IfPara)) //基类的初始化
		return false;
		
	m_pSocketPara = pSocketPara;
	
	m_wState = IF_STATE_CONNECT;
	m_bBakIP = 0;
	m_wIPUseCnt = 0;
	m_wDisConnectByPeerCnt = 0;
	return true;	
}

//描述:初始化服务器
void CSocketIf::InitSvr(int socket)
{
	m_Socket = socket;
	m_wState = IF_STATE_TRANS;

	m_fExit = false;
	m_fExitDone = false;
}

void CSocketIf::LoadUnrstPara()
{
	if (m_pfnLoadUnrstPara!=NULL) // && m_fUnrstParaChg
	{
		m_fUnrstParaChg = false;
		if ((*m_pfnLoadUnrstPara)(m_pSocketPara))	//需要重新初始化
		{
			//if (m_wState == IF_STATE_TRANS)
			{
				DisConnect();
			}
		}
	}	
}	

bool CSocketIf::Send(BYTE* pbTxBuf, WORD wLen)
{
	DTRACE(DB_FAPROTO, ("CSocketIf::Send: GetClick()=%d,wLen=%d.\r\n",GetClick(),wLen));
	TraceFrm("<-- CSocketIf::Send:", pbTxBuf, wLen);	
	if (m_Socket != INVALID_SOCKET)
	{
		int iReLen = 0;
		if (m_pSocketPara->fUdp)
		{
			struct  sockaddr_in to;
			to.sin_addr.s_addr = htonl(m_dwRemoteIP);
			to.sin_family = AF_INET;
			to.sin_port = htons(m_wRemotePort);	
			int tolen=sizeof(to);				
			iReLen = sendto(m_Socket, (char* )pbTxBuf, wLen, 0,(struct sockaddr *)&to,tolen);
		}
		else
		{
			iReLen = send(m_Socket, (char* )pbTxBuf, wLen, 0);
		}

		if (m_pSocketPara->fEnableFluxStat)	//是否允许流量统计,只有本socket用的是GPRS通道时才支持
			AddFlux(wLen);

		if (iReLen == wLen)
		{
			m_wGprsTxCnt++;
			//DTRACE(DB_FAPROTO, ("CSocketIf::Send : sock tx a %s frm, tx cnt=%d.\r\n", FapCmdToStr(pbTxBuf[FAP_CMD]), m_wGprsTxCnt));
			//PrintFrmCmd("CSocketIf::Send : sock tx a %s frm\n", pbTxBuf);
			return true;
		}
		else
		{
			DTRACE(DB_FAPROTO, ("CSocketIf::Send : sock tx fail iReLen=%d, wLen=%d.\r\n", iReLen, wLen));
			return false;
		}
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::Send : sock tx fail due to invalid sock.\r\n"));
		return false;
	}
	return false;
}

//描述:接收串口来的数据,如果接收循环缓冲区中还有数据,则返回循环缓冲区中的数据,
//     否则调用串口接收函数,直接等待串口的数据到来
//参数:@pbRxBuf 用来接收返回数据的缓冲区,
//     @wBufSize 接收缓冲区的大小
//返回:返回数据的长度
WORD CSocketIf::Receive(BYTE* pbRxBuf, WORD wBufSize)
{
	int len = 0, i=0; 
	struct  sockaddr_in from; 
	i = sizeof(from);

	if (m_Socket != INVALID_SOCKET)
	{
		Sleep(100);
  		//SockSetLastError(m_Socket, 0);
  		if (m_pSocketPara->fUdp == 0x01)
			len =  recvfrom(m_Socket, (char*)pbRxBuf, wBufSize, 0,(struct sockaddr *)&from, (socklen_t* )&i);
		else
			len =  recv(m_Socket, (char *)pbRxBuf, wBufSize, 0);

	  	if (len == 0)
      	{
  	    	DTRACE(DB_FAPROTO, ("CSocketIf::Receive: close socket due to rx len=0.\r\n"));
	    	DisConnect();
            OnDisConnectByPeer();
	    	return 0;
      	}
      	else if (len == SOCKET_ERROR) 
	  	{
			int iLastErr =  SocketGetLastError(m_Socket);

	  		//DTRACE(DB_FAPROTO, ("CSocketIf::Receive: len==-1 errno=%s\n", strerror(errno)));
	  		//return 0;
			if (iLastErr == EWOULDBLOCK)   //TCP在socket层的超时，应用程序不应执行重传
        	{               //应该坚信只要通信渠道还能传递数据，TCP就会把交付的数据给对方
				SocketSetLastError(m_Socket, 0);
          		return 0;
			}
			else if (iLastErr == 0)
			{
				DTRACE(DB_FAPROTO, ("CSocketIf::Receive: len==-1 errno=%s\n", strerror(iLastErr)));
				return 0;
			}
			else
			{
	  			DTRACE(DB_FAPROTO, ("CSocketIf::Receive: len==-1 errno=%ld, %s\n", iLastErr, strerror(iLastErr)));
	    		DisConnect();
                OnDisConnectByPeer();
				SocketSetLastError(m_Socket, 0);
        		return 0;
			}
      	}
      	else
      	{
			if (m_pSocketPara->fEnableFluxStat)	//是否允许流量统计,只有本socket用的是GPRS通道时才支持
				AddFlux((DWORD )len);

      		DTRACE(DB_FAPROTO, ("CSocketIf::Receive:GetClick()=%d.\r\n",GetClick()));
      		TraceFrm("--> CSocketIf::Receive:", pbRxBuf, len);
      	}
	}
	else
	{
		Sleep(200);
	}

	return len; 
}

bool CSocketIf::SetSocketLed(bool fLight)
{
	if (m_pSocketPara->fUdp || !m_pSocketPara->fEnSocketLed)
		return true;
	
	//return SetSockLed(fLight);
	if (fLight)
		GprsWorkerSetSockLed(SOCK_LED_LIGHT);
	else
		GprsWorkerSetSockLed(SOCK_LED_DARK);

	return true;
}

//描述：建立连接
bool CSocketIf::Connect()
{
	struct  sockaddr_in local_addr;
	
	DisConnect();
	
	if (m_pSocketPara->fUdp == 0x01)
	{
		m_Socket = socket(PF_INET, SOCK_DGRAM, 0);
	}
	else
	{
		m_Socket = socket(PF_INET, SOCK_STREAM, 0);
	}
		
	if (m_Socket == INVALID_SOCKET)
	{
	  	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: fail to create socket.\r\n"));
		m_iLastErr = GPRS_ERR_CON;
	  	return false;
	}

	if (m_pSocketPara->fUdp == 0x01)
	{
		local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(1024);
	
		if (bind(m_Socket, (struct sockaddr *)&local_addr, sizeof(local_addr)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf::Connect: Error: bind error\n"));
		  	Close();
			m_iLastErr = GPRS_ERR_CON;
		  	return false;
		}
	}
	
  	//主备IP切换：
	//1、在没有成功登陆主站的情况下，轮流使用主/备1/备2 IP
	//2、在登陆OK的情况下，清当前IP的使用计数，让它可以继续使用
	if (m_bBakIP > 2)
		m_bBakIP = 0;
	if (m_bBakIP == 0)	//当前要使用主IP
	{
		if (m_wIPUseCnt < m_pIfPara->wConnectNum && m_pSocketPara->dwRemoteIP!=0 && m_pSocketPara->wRemotePort!=0)//主 IP端口有效
		{
	        m_wIPUseCnt++;
			m_dwRemoteIP = m_pSocketPara->dwRemoteIP;
			m_wRemotePort = m_pSocketPara->wRemotePort;
		}
        else
        {
        	m_dwRemoteIP = m_pSocketPara->dwBakIP1;
			m_wRemotePort = m_pSocketPara->wBakPort1;
            m_wIPUseCnt = 0;
           	m_bBakIP = 1; //本切换到备用1 IP
        }
	}
	if (m_bBakIP == 1)	//当前要使用备用1IP
	{
		if (m_wIPUseCnt < m_pIfPara->wConnectNum && m_pSocketPara->dwBakIP1!=0 && m_pSocketPara->wBakPort1!=0)//备用1 IP端口有效
		{
	        m_wIPUseCnt++;
			m_dwRemoteIP = m_pSocketPara->dwBakIP1;
			m_wRemotePort = m_pSocketPara->wBakPort1;
		}
        else
        {
        	m_dwRemoteIP = m_pSocketPara->dwBakIP2;
			m_wRemotePort = m_pSocketPara->wBakPort2;
            m_wIPUseCnt = 0;
           	m_bBakIP = 2; //本切换到备用2 IP
        }
	}
	if (m_bBakIP == 2)	//当前要使用备用2IP
	{
		if (m_wIPUseCnt < m_pIfPara->wConnectNum && m_pSocketPara->dwBakIP2!=0 && m_pSocketPara->wBakPort2!=0)//备用2 IP端口有效
		{
			m_wIPUseCnt++;
			m_dwRemoteIP = m_pSocketPara->dwBakIP2;
			m_wRemotePort = m_pSocketPara->wBakPort2;
		}
		else
        {
			m_dwRemoteIP = m_pSocketPara->dwRemoteIP;
			m_wRemotePort = m_pSocketPara->wRemotePort;        	
            m_wIPUseCnt = 1;   //本次已经使用过一次了
            m_bBakIP = 0;  //本回切换到主IP
        }
	}
	
	struct  sockaddr_in remote_addr;
	remote_addr.sin_addr.s_addr = htonl(m_dwRemoteIP);
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(m_wRemotePort);
	
	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: connecting %d.%d.%d.%d %d.\r\n",
						 (m_dwRemoteIP>>24)&0xff, (m_dwRemoteIP>>16)&0xff, (m_dwRemoteIP>>8)&0xff, m_dwRemoteIP&0xff, 
						 m_wRemotePort));
	
	if (m_pSocketPara->fUdp != 0x01)
	{
		if (connect(m_Socket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) != 0)
    	{
    		DTRACE(DB_FAPROTO, ("CSocketIf::Connect: connect fail.\r\n"));
		  	Close();
	  		//UpdateErrRst(false);
			m_iLastErr = GPRS_ERR_CON;
		  	return false;
    	}
		DTRACE(DB_FAPROTO, ("CSocketIf::Connect: connect ok.\r\n"));	// UDP不打印connect ok
	}
	
	//UpdateErrRst(true);

#ifdef SYS_LINUX
	if (m_pSocketPara->fUdp!=0x01 && m_pSocketPara->fEnTcpKeepAlive) ////开启TCP的keepalive属性	
	{
		int keepAlive = 1; // 开启keepalive属性
		int keepIdle = m_pSocketPara->wKeepIdle; //如该连接在wKeepIdle秒内没有任何数据往来,则进行探测 
		int keepInterval = m_pSocketPara->wKeepInterv; //探测时发包的时间间隔为wKeepInterv秒，如20秒
		int keepCount = m_pSocketPara->wKeepCnt; //探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
	
		if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf: Set keepalive error: %s.\r\n", strerror(errno)));
			return false;
		}
		
		if (setsockopt(m_Socket, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf: Set keepalive idle error: %s.\r\n", strerror(errno)));
			return false;
		}

		if (setsockopt(m_Socket, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf: Set keepalive interv error: %s.\r\n", strerror(errno)));
			return false;
		}
		
		if (setsockopt(m_Socket, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf: Set keepalive count error: %s.\r\n", strerror(errno)));
			return false;
		}
			
		//在程序中表现为,当tcp检测到对端socket不再可用时(不能发出探测包,或探测包没有收到ACK的响应包),select会返回socket可读,并且在recv时返回-1,同时置上errno为ETIMEDOUT。 
	}
#endif //SYS_LINUX
	
	//int timeout = 2000;
	//if (setsockopt(m_Socket, SOL_SOCKET, SO_RCVTIMEO, (char* )&timeout, sizeof(timeout)) != 0)
    //int optval = 1;
	//if (setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) != 0) 
	unsigned int arg = 1;
	if (ioctlsocket(m_Socket, FIONBIO,  (ULONG* )&arg) != 0) //(u_long FAR*)
    {
    	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: ioctl fail.\r\n"));
	  	Close();
		m_iLastErr = GPRS_ERR_CON;
	  	return false;
    }

	m_iLastErr = GPRS_ERR_OK;
	
	SetSocketLed(true);
  	return true;
}

//描述:取连接次数,GPRS或socket连接的时候,如果有备用IP端口的话,连接次数乘2
WORD CSocketIf::GetConnectNum()
{
	WORD wConnectNum = m_pIfPara->wConnectNum;
	if (m_pSocketPara->dwBakIP1!=0 && m_pSocketPara->wBakPort1!=0)	//备用主站IP和端口
		wConnectNum += m_pIfPara->wConnectNum;
	if (m_pSocketPara->dwBakIP2!=0 && m_pSocketPara->wBakPort2!=0)
		wConnectNum += m_pIfPara->wConnectNum;
	return wConnectNum;
}


void CSocketIf::OnConnectFail()
{	
	//连接相关
	m_wConnectFailCnt++;
	
	//DTRACE(DB_FAPROTO, ("CProtoIf::OnConnectFail: m_wConnectFailCnt=%d, ConnectNum=%d\r\n",
	//					m_wConnectFailCnt, GetConnectNum()));	
	
	//登录相关变量
	if (m_wConnectFailCnt >= GetConnectNum())
	{
		m_wConnectFailCnt = 0;
		if (m_pIfPara->dwDormanInterv != 0)
		{
			DTRACE(DB_FAPROTO, ("CProtoIf::OnConnectFail: go to dorman\n"));
			EnterDorman();
		}
		else
		{
			if (m_pSocketPara->bRandLoginFlg & RAND_ON_FAIL) //失败时进行随机延时
			{
				if (m_fRstInConnectFail)	//本接口在连接失败到重试次数后复位接口
				{
					DTRACE(DB_FAPROTO, ("CProtoIf::OnConnectFail: go to dorman!\n"));
					EnterDorman();
				}
				else
					StateToDorman(m_wState);	//从某个状态暂时切换到休眠状态，休眠完还要转回来
			}
			else
			{
				//DTRACE(DB_FAPROTO, ("CProtoIf::OnConnectFail: go to rst\n"));
				Sleep(m_pIfPara->dwConnectInterv * 1000); //接口的连接间隔,单位秒
				if (m_fRstInConnectFail)	//本接口在连接失败到重试次数后复位接口
					m_wState = IF_STATE_RST; 
			}
		}
	}
	else
	{
		if (m_pSocketPara->bRandLoginFlg & RAND_ON_FAIL) //失败时进行随机延时
			StateToDorman(m_wState);	//从某个状态暂时切换到休眠状态，休眠完还要转回来
		else
			Sleep(m_pIfPara->dwConnectInterv * 1000); //接口的连接间隔,单位秒
	}	
}

//描述:在协议登陆成功时调用
void CSocketIf::OnLoginOK()
{ 
	CProtoIf::OnLoginOK();
	
	m_wIPUseCnt = 0; //在登陆OK的情况下，清当前IP的使用计数，让它可以继续使用，
					 //否则让主备IP使用相同的次数后轮流切换
}

//描述:在协议登陆失败时调用,比如多少次失败后断开连接
void CSocketIf::OnLoginFail() 
{	
	m_wLoginFailCnt++;

	m_iLastErr = GPRS_ERR_LOGIN;
	
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
		m_wIPUseCnt = m_pIfPara->wConnectNum;
		m_wLoginFailCnt = 0;
		DTRACE(DB_FAPROTO, ("CSocketIf::OnLoginFail: go to dorman\n"));
		EnterDorman();
	}
	else		//登录失败的次数达到了断开连接的次数的整数倍
	{			//m_wLoginFailCnt%wLoginRstNum==0 && m_wLoginFailCnt<wLoginRstNum*wLoginNum
		m_wIPUseCnt = m_pIfPara->wConnectNum;
		DisConnect();	//只断开连接,不清零登录失败计数,
						//达到wLoginRstNum*wReTryNum次后进入休眠状态
	}		
}

//描述：取得连接、登录失败后，随机避让的休眠时间(心跳周期0.5-1.5倍),单位秒
DWORD CSocketIf::GetRandDormanInterv()
{
	srand((unsigned)time(NULL));
	int iRand = rand() % 100 + 50; 
	return m_pSocketPara->dwBeatSeconds * iRand / 100;
}

//描述：从某个状态暂时切换到休眠状态，休眠完还要转回来
void CSocketIf::StateToDorman(WORD wState)
{
	m_wDormanState = wState; //暂时休眠的状态，休眠完还要转回到该状态
	m_dwDormanInterv = GetRandDormanInterv();

	m_wState = IF_STATE_DORMAN;
	m_dwDormanClick = GetClick();	 //进入休眠的开始时间
	DTRACE(DB_FAPROTO, ("CSocketIf::StateToDorman : random login, old state=%d, m_dwDormanInterv=%ldS \n", 
						wState, m_dwDormanInterv));
}

void CSocketIf::EnterDorman()
{
	if ((m_wState==IF_STATE_CONNECT || m_wState==IF_STATE_LOGIN) && //原来的状态是连接或登陆
		 (m_pSocketPara->bRandLoginFlg & RAND_ON_DORMAN))	//允许随机登陆，主要针对国网：每次登录失败后，经过心跳周期0.5-1.5倍的随机延时（以秒或毫秒计）后重新登录
	{
		m_dwDormanInterv = GetRandDormanInterv();
		DTRACE(DB_FAPROTO, ("CSocketIf::EnterDorman : random login, dwDormanInterv=%ldS \n", m_dwDormanInterv));
	}

	CProtoIf::EnterDorman();
    m_wDisConnectByPeerCnt = 0;
}

void CSocketIf::OnDisConnectByPeer()
{
    m_wDisConnectByPeerCnt++;
    if (m_pSocketPara->wDisConnectByPeerNum!=0 && m_wDisConnectByPeerCnt>=m_pSocketPara->wDisConnectByPeerNum) 
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::OnDisConnectByPeer: go to dorman\n"));
        EnterDorman();
	}
}

//描述:接口保活探测
void CSocketIf::KeepAlive()
{	
	DWORD dwBeatSeconds = GetBeatSeconds();
	if (dwBeatSeconds == 0) //!m_pSocketPara->fBeatOn
		return;
	
	DWORD dwClick = GetClick();
	DWORD dwBrokenTime = dwBeatSeconds + 
					 	 m_pSocketPara->dwBeatTimeouts*m_pSocketPara->wBeatTestTimes;
	if (m_pSocketPara->wBeatTestTimes!=0 && dwClick-m_dwRxClick>dwBrokenTime)
	{	//m_pSocketPara->wBeatTestTimes配置为0,表示不自动掉线,只周期发心跳
		DisConnect(); //重新初始化
		DTRACE(DB_FAPROTO, ("CSocketIf::KeepAlive : DisConnect at click %d\n", dwClick));
	}
	else if (dwClick-m_dwRxClick > dwBeatSeconds)//40 20秒还没收到过一帧，则进行心跳检测
	{	//刚开始时dwBeatClick为0，dwNewClick不为0，能马上发
		if (dwClick-m_dwBeatClick > m_pSocketPara->dwBeatTimeouts)
		{							//心跳超时时间,单位秒
			m_pProto->Beat();
			m_dwBeatClick = dwClick;
			DTRACE(DB_FAPROTO, ("CSocketIf::KeepAlive: heart beat test at click %d\n", dwClick));
		}
	}
}


bool CSocketIf::Close()
{
	if (m_Socket != INVALID_SOCKET)
	{
#ifndef SYS_WIN
		unsigned int arg = 0;
		ioctlsocket(m_Socket, FIONBIO,  (ULONG* )&arg);//设置为非阻塞		
		arg = 1;
		setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(int));
		//要已经处于连接状态的soket在调用closesocket后强制关闭，省掉CLOSE_WAIT的过程：
		struct linger so_linger;
		so_linger.l_onoff = 1;
		so_linger.l_linger = 0;
		setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
#endif
	  	closesocket(m_Socket);
	  	m_Socket = INVALID_SOCKET;
	  	DTRACE(DB_FAPROTO, ("CSocketIf::Close: close socket at click %d\n", GetClick()));
	}
	
	return true;
}


//描述:在接口由连接转为断开的时候调用，不管是主动断开还是被动断开
bool CSocketIf::DisConnect()
{
	if (m_Socket != INVALID_SOCKET) //断开socket连接
	{
		SetSocketLed(false);
		
		m_pProto->OnBroken();
		Close();		
		//closesocket(m_Socket);
	  	m_Socket = INVALID_SOCKET;
	}
	
	CProtoIf::DisConnect();
	
	if (m_pSocketPara->fSvr && m_pSocketPara->bSvrDisconMode==SVR_DISCON_EXIT)
	{				//服务器模式下,socket断开时要求退出线程
					//断开连接后处于空闲模式SVR_DISCON_IDLE不用处理，
					//因为有时候只是先断开连接，还要重新连接上去
		m_fExit = true;
	}
	
	return true;
}


//描述:做一些各个接口相关的非标准的事情,
void CSocketIf::DoIfRelated()
{
	if (m_fSetIdleCmd)	//收到外部的断开连接命令
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::DoIfRelated : %s rx set idle cmd.\r\n", GetName()));

		DisConnect();	//先断开socket方式下的连接,不然切换到短信模式下没法调用

		m_fSetIdleCmd = false;

		m_dwDormanClick = 0; //没有规定休眠的时间,相当于处于无限期的休眠(空闲)状态,
		m_wState = IF_STATE_DORMAN;
	}

	if (m_fDisConnCmd)	//收到外部的断开连接命令
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::DoIfRelated : %s rx disconnect cmd.\r\n", GetName()));

		DisConnect();
    	if (m_pSocketPara->fSvr) //服务器模式下,socket断开时退出线程
        {
		    m_dwDormanClick = 0; //没有规定休眠的时间,相当于处于无限期的休眠(空闲)状态,
			m_wState = IF_STATE_DORMAN;
        }
        
        m_fDisConnCmd = false;
	}
	
	CProtoIf::DoIfRelated();
}
