/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FapPara.cpp
 * 摘    要：本文件主要包含FaProto目录下API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：
 *********************************************************************************************************/

#include "stdafx.h"
#include "FaAPI.h"
#include "FaCfg.h"
#include "syssock.h" 
#include "ProAPI.h"
#include "ProPara.h"
#include "sysarch.h" 
//#include "ProMngRpt.h" 
#include "ThreadMonitor.h"
#include "ProAPI.h"
#include "DrvAPI.h"
#include "sysfs.h"
#include "CctAPI.h"


//描述:用电管理终端协议GPRS服务线程
TThreadRet StdProtoThread(void* pvArg)
{
	CProto* pProto = (CProto* )pvArg;
	CProtoIf* pIf = pProto->GetIf();
	WORD wGprsCnMode = CN_MODE_SOCKET;

	char *pstr = pIf->GetName();
	bool fSocketSvrMode = false;

	if (strcmp(pstr, "Gprs-Server")==0 || strcmp(pstr, "Eth-Server")==0)
		fSocketSvrMode = true;
	
	DTRACE(DB_FAPROTO, ("StdProtoThread : if(%s) started!\n", pIf->GetName()));

	if (pIf->GetIfType()==IF_GPRS || pIf->GetIfType()==IF_SOCKET)	//终端上电后，经过0-30秒的随机延时后登录
	{
#ifdef SYS_LINUX
		srand((unsigned)time(NULL));
		int iRand = rand() % 10; 
		DTRACE(DB_FAPROTO, ("StdProtoThread : if(%s) Rand delay for login, please wait %d!\n", pIf->GetName(),iRand));
		Sleep(iRand*1000);
#endif

		BYTE bEmbedProtcocol = 1;
		ReadItemEx(BN1, PN0, 0x2032, &bEmbedProtcocol);
		if (bEmbedProtcocol == 0)
			wGprsCnMode = CN_MODE_EMBED;
		else
			wGprsCnMode = CN_MODE_SOCKET;

	}

	int iMonitorID;
	if (pIf->GetIfType() == IF_GPRS)	//GPRS线程由于需要复位模块,时间长一些
		iMonitorID = ReqThreadMonitorID("GprsPro-thrd", 60*60);	//申请线程监控ID,更新间隔为一个小时
	else if (pIf->GetIfType() == IF_SOCKET)
		iMonitorID = ReqThreadMonitorID("SocketPro-thrd", 60*60);	//申请线程监控ID,更新间隔为一个小时
	else
		iMonitorID = ReqThreadMonitorID("CommPro-thrd", 60*60);	//申请线程监控ID,更新间隔为一个小时

   	pIf->InitRun();

    if(pIf->GetIfType() == IF_GPRS)
    {
        SetGprsOnlineState(false);
    }
    else if(pIf->GetIfType() == IF_SOCKET)
    {
        SetEthOnlineState(false);
    }

	BYTE bBuf[8];
	WORD wState;
	char szTmp[128];
	BYTE bDispCnt = 0;
	pIf->m_fExitDone = false;
	bool fCliConnectFail = false;
	DWORD dwClick;
	while (1)
	{
		UpdThreadRunClick(iMonitorID);

		if (fSocketSvrMode)
		{
			if (!pIf->IsIfValid())
			{	
				Sleep(1000);
				continue;
			}
		}

		pIf->m_wRunCnt++;
		if (fSocketSvrMode)
		{
			if (pIf->m_fExit)
				break;
		}

#ifdef EN_ETHTOGPRS
#ifndef SYS_WIN
		//非服务器接口模式下，检测GPRS、以太网的状态切换
		BYTE bEthLnkState = 0;
		BYTE bModuleType = 0;
		if (!fSocketSvrMode)
		{
			ReadItemEx(BN2, PN0, 0x2050, &bModuleType);	//登录类型 以太网--MODULE_SOCKET，GPRS--MODULE_GC864
			ReadItemEx(BN2, PN0, 0x2052, &bEthLnkState);	//以太网登录状态 1--ok， 0--fail

			//当前线程接口为GPRS &&　当前运行的为MODULE_SOCKET
			if (pIf->GetIfType()==IF_GPRS && bModuleType!=MODULE_ME590)
			{
				//DTRACE(DB_FAPROTO, ("StdProtoThread: GPRS wait module = %d....\n", bModuleType));
				Sleep(5*1000);
				continue;
			}
			else if (pIf->GetIfType()==IF_SOCKET && bModuleType!=MODULE_SOCKET)
			{
				//DTRACE(DB_FAPROTO, ("StdProtoThread: Ethernet wait module=%d....\n", bModuleType));
				Sleep(5*1000);
				continue;
			}
		}
#endif
#endif

		pIf->LoadUnrstPara(); 	//非复位参数发生改变
		pProto->LoadUnrstPara(); 
		pIf->DoIfRelated();	//做一些各个接口相关的非标准的事情
							//比如非连续在线方式下,GPRS和SMS间的切换
		pProto->DoProRelated();	//做一些协议相关的非标准的事情
		
		//接口的状态切换: (休眠)->(复位)->(连接)->(登录)->(传输)
		wState=pIf->GetState();
		switch (wState)  //接口的状态机
		{
			case IF_STATE_DORMAN:  //休眠
				pIf->DoDorman();
				g_fCliIdle = true; //客户端处于空闲状态
				if(pIf->GetIfType() == IF_GPRS)
                {
                    SetGprsOnlineState(false);
                }
                else if(pIf->GetIfType() == IF_SOCKET)
                {
                    SetEthOnlineState(false);
                }
				break;
			
			case IF_STATE_RST:  //复位
				if (pIf->GetIfType()==IF_GPRS && fCliConnectFail && GetNetWorkMode()==GPRS_MIX_MODE)
				{
					pIf->RequestOffline();
					dwClick = GetClick();
					do { Sleep(1000); } while(GetClick()-dwClick < 5);
					if (!g_fSvrDiscon)	//服务器超时断开连接
						SetInfo(INFO_GPRS_OFFLINE);
					fCliConnectFail = false;
					Sleep(1000);  //等待1秒让GprsKeepThread线程GPRSid离线
				}

                if(pIf->GetIfType() == IF_GPRS)
                {
                    SetGprsOnlineState(false);
                }
                else if(pIf->GetIfType() == IF_SOCKET)
                {
                    SetEthOnlineState(false);
                }

				if (pIf->ResetIf() == IF_RST_OK)
				{
					pIf->OnResetOK();
#ifdef SYS_LINUX
					if (pIf->GetIfType() == IF_GPRS)
					{	
						if (((CGprsIf* )pIf)->GetParaCnMode() == CN_MODE_ETHSCK)
							SetDownConnMode(2);
						else
							SetDownConnMode(1);	
					}
#endif
				}
				else
					pIf->OnResetFail();

				g_fSvrDiscon = false;

				if (pIf->GetIfType()==IF_GPRS)
				{
					BYTE bSigStren = GetGprsWorker()->SignStrength();
					WriteItemEx(BN2, PN0, 0x1058, &bSigStren);	
				}
				break;
			case IF_STATE_CONNECT: //连接

                if(pIf->GetIfType() == IF_GPRS)
                {
                    SetGprsOnlineState(false);
                }
                else if(pIf->GetIfType() == IF_SOCKET)
                {
                    SetEthOnlineState(false);
                }
                
				if (pIf->GetIfType()==IF_GPRS || pIf->GetIfType()==IF_SOCKET)
				{
					strcpy(szTmp, "连接");
					WriteItemEx(BN2, PN0, 0x2033, (BYTE *)szTmp);	
				}
				if (pIf->Connect())
				{
					fCliConnectFail = false;
					pIf->OnConnectOK();
					pProto->OnConnectOK();					
				}
				else
				{	
					if (GetNetWorkMode()==GPRS_MIX_MODE)
					{
						fCliConnectFail = true;
					}
					pIf->OnConnectFail();
#ifdef EN_ETHTOGPRS
					ReadItemEx(BN2, PN0, 0x2050, &bModuleType);	//登录类型 以太网--MODULE_SOCKET，GPRS--MODULE_GC864
					if (pIf->GetIfType()==IF_SOCKET && bModuleType==MODULE_SOCKET)
					{
					#ifdef SYS_LINUX
						system("ifconfig eth0 down");
						Sleep(3000);
						system("ifconfig eth0 up");
						LoadSocketLocalPara();
					#endif
					}
#endif
				}

				break;
				
			case IF_STATE_LOGIN:  //登录
    			if(pIf->GetIfType() == IF_GPRS)
                {
                    SetGprsOnlineState(false);
                }
                else if(pIf->GetIfType() == IF_SOCKET)
                {
                    SetEthOnlineState(false);
                }                
                
				if (pProto->Login())
					pIf->OnLoginOK();
				else
					pIf->OnLoginFail();
				break;
				
			case IF_STATE_TRANS:  //传输
    			if(pIf->GetIfType() == IF_GPRS)
                {
                    SetGprsOnlineState(true);
                }
                else if(pIf->GetIfType() == IF_SOCKET)
                {
                    SetEthOnlineState(true);
                }
				pProto->RcvFrm(); //接收到的一帧,并已经对其进行处理
				if (!fSocketSvrMode)
					pIf->KeepAlive();
				pIf->AutoSend();
				break;
				
			default:
				DTRACE(DB_FAPROTO, ("StdProtoThread : enter unkown state!\n"));	
				Sleep(5000);
				break;
		}
	}

	ReleaseThreadMonitorID(iMonitorID);
	pIf->Close();
	pIf->m_fExitDone = true;

	return 0;
}

//短信收发线程，防止收发部分被StdProtoThread延迟太长时间了
TThreadRet GprsSmsThread(void* pvArg)
{
	CProto* pProto = (CProto* )pvArg;
	CProtoIf* pIf = pProto->GetIf();
    int iMonitorID = ReqThreadMonitorID("GprsSms-thrd", 60*60);	//申请线程监控ID,更新间隔为一个小时
	while (1)
	{
        UpdThreadRunClick(iMonitorID);
		if(pIf->GetIfType()==IF_SMS) //GPRS才去检测以太网
		{ 
            
			BYTE bSmsBuf[512];
            BYTE bSmsBufToByte[512];
            
			//CGprsWorker *pGprsWork = GetGprsWorker();
			WORD wLen = 0;//pGprsWork->ReceiveSms(bSmsBuf);
			WORD wLen1 = 0;
			wLen = pIf->Receive(bSmsBuf, wLen1);
            
			if (wLen > 0)
			{
			    DTRACE(DB_FAPROTO, ("GprsSmsThread  IF_SMS Rcv Len=%d\r\n", wLen));
                TraceBuf(DB_FAPROTO, "sms buf:	 ", bSmsBuf, wLen);
                AsciiToByte(bSmsBuf, wLen, bSmsBufToByte);
                TraceBuf(DB_FAPROTO, "sms buftoByte:	 ", bSmsBufToByte, wLen);
				//pIf->m_bGprsDataSrc = DATA_SRC_SMS;
				//pProto->RcvSms(bSmsBuf, wLen);
				pProto->RcvSms(bSmsBufToByte, wLen/2);
				//pIf->m_bGprsDataSrc = 0;
			}
			//pProto->RcvFrm();
		}
		Sleep(2000);
	}

    ReleaseThreadMonitorID(iMonitorID);
	return THREAD_RET_OK;
}


//描述:标准通信线程
TThreadRet SockSvrThread(void* pvArg)
{
	WORD i;
	struct  sockaddr_in local_addr;
	int iMonitorID;

	DTRACE(DB_FAPROTO, ("SockSvrThread : started!\n"));

	TSockSvrPara* pSockSvrPara = (TSockSvrPara* )pvArg;

	if (strcmp(pSockSvrPara->pszName, "Svr-Gprs") == 0)
	{
		DTRACE(DB_FAPROTO, ("SockSvrThread[%s]: Gprs need get local IP addr.\r\n", pSockSvrPara->pszName));
#ifndef SYS_WIN
		pSockSvrPara->dwLocalIP = GetLocalAddr("ppp0");

		//这里需要获取GPRS的本地IP地址
		DTRACE(DB_CRITICAL, ("Get Gprs Ip: 0x%08x\n", pSockSvrPara->dwLocalIP));
#endif
	}

	int sock = socket(PF_INET, SOCK_STREAM, 0);

	if (sock == INVALID_SOCKET)
	{
		DTRACE(DB_FAPROTO, ("SockSvrThread[%s]: fail to create socket, error=%s.\n", pSockSvrPara->pszName,strerror(errno)));
		return THREAD_RET_OK;
	}

	unsigned int arg = 1;
	if (ioctlsocket(sock, FIONBIO,  (ULONG* )&arg) != 0)
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::InitSock[%s]: ioctl fail, error=%s.\r\n", pSockSvrPara->pszName, strerror(errno)));
		close(sock);
		return THREAD_RET_OK;
	}		

	local_addr.sin_addr.s_addr = htonl(pSockSvrPara->dwLocalIP);
	local_addr.sin_family = AF_INET;
	local_addr.sin_port = htons(pSockSvrPara->wLocalPort);

	DTRACE(DB_FAPROTO, ("SockSvrThread[%s]: dwLocalIP=%02d.%02d.%02d.%02d, wPort=%04d.", 
		pSockSvrPara->pszName,
		(pSockSvrPara->dwLocalIP>>24)&0xff, (pSockSvrPara->dwLocalIP>>16)&0xff, 
		(pSockSvrPara->dwLocalIP>>8)&0xff, pSockSvrPara->dwLocalIP&0xff, 
		pSockSvrPara->wLocalPort));

	if (bind(sock, (struct sockaddr* )&local_addr, sizeof(local_addr)) != 0)
	{
		closesocket(sock);
		DTRACE(DB_FAPROTO, ("SockSvrThread[%s]: fail to bind socket, error=%s.\n", pSockSvrPara->pszName, strerror(errno)));
		return THREAD_RET_OK;
	}

	if (listen(sock, 1) != 0)
	{
		closesocket(sock);
		DTRACE(DB_FAPROTO, ("SockSvrThread[%s]: fail to listen socket.\r\n", pSockSvrPara->pszName));
		return THREAD_RET_OK;
	}

	DTRACE(DB_FAPROTO, ("SockSvrThread[%s]: server listen to %d.%d.%d.%d : %d\n",
		pSockSvrPara->pszName,
		(pSockSvrPara->dwLocalIP>>24)&0xff, (pSockSvrPara->dwLocalIP>>16)&0xff, 
		(pSockSvrPara->dwLocalIP>>8)&0xff, pSockSvrPara->dwLocalIP&0xff, 
		pSockSvrPara->wLocalPort));

	iMonitorID = ReqThreadMonitorID("SockSvrThread", 60*60);	//申请线程监控ID,更新间隔为60秒

	while (1)
	{
		struct  sockaddr_in remote_addr;
		int addrlen;
		addrlen = sizeof(remote_addr);
		int newsock = accept(sock, (struct sockaddr* )&remote_addr, (socklen_t* )&addrlen);

		if (newsock != INVALID_SOCKET)
		{
			DWORD dwRemoteIP = ntohl(remote_addr.sin_addr.s_addr);
			DTRACE(DB_FAPROTO, ("SockSvrThread[%s]: accept client %d.%d.%d.%d : %d\n",
				pSockSvrPara->pszName,
				(dwRemoteIP>>24)&0xff, (dwRemoteIP>>16)&0xff, (dwRemoteIP>>8)&0xff, dwRemoteIP&0xff, 
				ntohs(remote_addr.sin_port)));
			ioctlsocket(newsock, FIONBIO,  (ULONG* )&arg);
			for (i=0; i<pSockSvrPara->wConnectNum; i++)
			{
				if (!(CSocketIf *)(pSockSvrPara->pSocketIf[i]->IsIfValid()))
				{
					if (strcmp(pSockSvrPara->pszName, "Svr-Gprs") == 0)
						g_dwGprsSvrAptClick[i] = GetClick();
					else
						g_dwEthSvrAptClick[i] = GetClick();
					
					pSockSvrPara->pSocketIf[i]->InitSvr(newsock);
					break;
				}
			}

			if (i == pSockSvrPara->wConnectNum)
			{
				DTRACE(DB_FAPROTO, ("SockSvrThread[%s]: fail to serve for client's connection due to full\n", pSockSvrPara->pszName));
			}
		}

		Sleep(500);
		UpdThreadRunClick(iMonitorID);
	}

	DTRACE(DB_FAPROTO, ("SockSvrThread: exit\n"));

	ReleaseThreadMonitorID(iMonitorID);
	return THREAD_RET_OK;
}

TThreadRet StdProRptThread(void* pvArg)
{
	CFaProto** ppStart = (CFaProto** )pvArg;
	CProtoIf* pIf;
	bool fRun = false;

	while(1)
	{
/*		CFaProto** ppPro = ppStart;
		while(*ppPro)
		{
			if ((*ppPro)->m_pProMngRpt)
			{				
				pIf = (*ppPro)->GetIf();
				if (pIf)
				{
					if (pIf->GetState() == IF_STATE_TRANS)
					{
						if (pIf->GetIfType() == IF_GPRS) //在线
						{
							 if (pIf->GetCnMode() != CN_MODE_SMS)
								(*ppPro)->m_pProMngRpt->DoMngRptTask();//主动上报检测线程接口
						}
						else
						{
							(*ppPro)->m_pProMngRpt->DoMngRptTask();//主动上报检测线程接口
						}
					}
				}
			}
			ppPro++;
		}
*/
		Sleep(5000);
	}
}





#ifdef EN_ETHTOGPRS
//以太网&GPRS切换线程，本线程只管切换，以太网状态由CheckNetThread线程负责处理
TThreadRet EthernetSwitchThread(void* pvArg)  
{
	int iPing = 0;
	BYTE bModuleTypeOld = MODULE_ME590; 
	//ReadItemEx(BN2, PN0, 0x10d3, &bModuleTypeOld); //保留GPRS模块类型
	BYTE bModuleTypeCurr = MODULE_SOCKET;  //当前连接类型
	BYTE bConnMode = 0; //Socket 连接状态  1:GPRS   2:Socket
	BYTE bNetContTye = 0;

	bNetContTye = GetEthConfigType();	//以太网配置方式 DHCP（0），静态（1），PPPoE（2）

	int iMonitorID = ReqThreadMonitorID("NetSwitch-thrd", 60*60);	//申请线程监控ID,更新间隔为60秒

	while(1)
	{
		if (!IsExecPingCMD())
		{
			Sleep(1000);
			continue;
		}

		if (IsDownSoft())
		{
			if (GetDownConnMode() != 0)
			{
				Sleep(5000);
				continue; //下载状态下确定通讯方式了停止切换
			}
		}
		if(g_fUpdateFirmware)
		{
			Sleep(5000);
			continue; //下载状态下确定通讯方式了停止切换
		}

		iPing = GetEthernetStat(); //

		DTRACE(SOCKETS_DEBUG, ("EthernetSwitchThread: Ping return %d------.\r\n",iPing));
		if(iPing>=0 && bConnMode ==1)  //以太网可用，但还没连接,切换到以太网
		{
			SetInfo(INFO_DISCONNECT);
			DTRACE(SOCKETS_DEBUG, ("EthernetSwitchThread: Socket Start Connect-----.\r\n"));
			bModuleTypeCurr = MODULE_SOCKET;
			WriteItemEx(BN2, PN0, 0x2050, &bModuleTypeCurr); //更改登陆类型为以太网	
			bConnMode = 2;	

			if (bNetContTye != 2)//pppoe不用off
			{
				system("/clou/ppp/script/ppp-off");
				Sleep(1000);
				system("route del default netmask 0.0.0.0 ppp0");
				//system("route add -net 10.98.96.0 netmask 255.255.255.0 eth0");
				//system("route add default gw 10.98.96.7");//
				system(RouteCmd.AddRoute);//添加以默认太网关
			}
		}
		else if(iPing>=0 && bConnMode ==0)  //一开始以太网就OK
		{
			if (bNetContTye != 2)//pppoe不用off
			{
				system("/clou/ppp/script/ppp-off");
				Sleep(1000);
				//system("route add -net 10.98.96.0 netmask 255.255.255.0 eth0");
				//system("route add default gw 10.98.96.7");
				system(RouteCmd.AddRoute);//添加以默认太网关
			}
			
			bConnMode = 2;	

			bModuleTypeCurr = MODULE_SOCKET;
			WriteItemEx(BN2, PN0, 0x2050, &bModuleTypeCurr); //更改登陆类型为以太网	
		}
		else if (iPing<0 && bConnMode==2)  //以太网不可用，需要从以太网切换到GPRS 
		{
			DTRACE(SOCKETS_DEBUG, ("EthernetSwitchThread: Socket Disconnect, del Default GateWay now !!!------.\r\n"));
			SetInfo(INFO_DISCONNECT);
			//以太网不可用时应立即把默认网关删除，虽然下面也会删除但是要等一个心跳间隔
			if (bNetContTye == 2)
			{
				DTRACE(SOCKETS_DEBUG, ("EthernetSwitchThread Close ppp0: exec /clou/ppp/script/pppoe-off\n"));
				system("/clou/ppp/script/pppoe-off");
  				Sleep(5000);
			}

			system("route del default");

			bConnMode = 0 ;

			WriteItemEx(BN2, PN0, 0x2050, &bModuleTypeOld); //更改登陆类型为以太网	
		}
		else if(iPing<0 && bConnMode==1) //以太网不可用，但是当前是GPRS连接类型，不用切换
		{		
			ReadItemEx(BN2, PN0, 0x2050, &bModuleTypeOld);
			DTRACE(SOCKETS_DEBUG, ("EthernetSwitchThread: GPRS  Connected------.\r\n"));
		}
		else if (iPing>=0 && bConnMode==2) //以太网可用，但前是以太网连接类型，不用切换
		{
			DTRACE(SOCKETS_DEBUG, ("EthernetSwitchThread: SOCKET  Connected------.\r\n"));
		}
		else if(iPing<0  && bConnMode==0 )   //以太网不可用，切换到GPRS模式 ,首次登陆
		{
			//SetInfo(INFO_DISCONNECT);
			if (bNetContTye == 2)
			{
				DTRACE(SOCKETS_DEBUG, ("EthernetSwitchThread Close ppp0: exec /clou/ppp/script/pppoe-off\n"));
				system("/clou/ppp/script/pppoe-off");
  				Sleep(5000);
			}

			system("route del default");
			
			WriteItemEx(BN2, PN0, 0x2050, &bModuleTypeOld); 
			DTRACE(SOCKETS_DEBUG, ("EthernetSwitchThread:Ethnet is not alive\r\n"));
			bConnMode = 1 ;

			DTRACE(SOCKETS_DEBUG, ("EthernetSwitchThread: GPRS Start Connect-----.\r\n"));
		}

		Sleep(3*1000); //10秒检测一次
		UpdThreadRunClick(iMonitorID);
	}
	ReleaseThreadMonitorID(iMonitorID);
	return THREAD_RET_OK;
}

//以太网通断检查线程，本线程只管检查，不负责切换
TThreadRet CheckNetThread(void* pvArg)
{
	BYTE bPingRet = 0;
	BYTE bBuf[64];
	char cPingPara[50];
	unsigned long rip=0;
	BYTE bNetContTye = 0;
	static BYTE bGateWayStatus = 0;
	DWORD dwOpenPppoeClick = 0;

	bNetContTye = GetEthConfigType();	//以太网配置方式 DHCP（0），静态（1），PPPoE（2）

	if (bNetContTye == 0)	//DHCP
	{
		LoadSocketLocalPara(); //设置本地以太网参数
		if(GetNetStat()==1) //检查一下以太网物理通道是否已OK
		{
			DoUdhcpc();//system("udhcpc -n &");      //动态获取IP和默认网关
			Sleep(6000); 
			rip = GetDefaultGateWay();
		}
	}
	else if (bNetContTye == 1)	//静态IP
	{
		LoadSocketLocalPara(); //设置本地以太网参数
		GetNetPara(3, bBuf);
		memcpy((BYTE*)&rip, bBuf, 4);
		bGateWayStatus = 1;
	}
	else	//PPPoE
	{
		if(GetNetStat()==1) //检查一下以太网物理通道是否已OK  PPPoE拨号
		{
			bool fOpenPppoeOk = false;
			fOpenPppoeOk = OpenPppoe();
			Sleep(1000); 
			if (fOpenPppoeOk)
			{
				InitPingCMD();      //初始化ping参数
				SetExecPingCMD(true);
				bPingRet = 1;
				WriteItemEx(BN2, PN0, 0x2052, &bPingRet); //以太网测试OK
				DTRACE(SOCKETS_DEBUG, ("CheckNetThread: ---OpenPppoe on---.\r\n"));
			}
			else
			{
				//if (IfReadListProc("ppp0") > 0) //有连接先断开下，防止是GPRS拨号连接
				{
					DTRACE(SOCKETS_DEBUG, ("CheckNetThread Close ppp0: exec /clou/ppp/script/ppp-off\n"));
					SetInfo(INFO_DISCONNECT);
					system("/clou/ppp/script/ppp-off");
					Sleep(5000);
					system("/clou/ppp/script/pppoe-off");
					Sleep(5000);
				}

				if (OpenPppoe() > 0)
				{
					InitPingCMD();      //初始化ping参数
					SetExecPingCMD(true);
					bPingRet = 1;
					WriteItemEx(BN2, PN0, 0x2052, &bPingRet); //以太网测试OK
					DTRACE(SOCKETS_DEBUG, ("CheckNetThread: ---OpenPppoe on---.\r\n"));
				}
				else
				{
					bPingRet = 0;
					WriteItemEx(BN2, PN0, 0x2052, &bPingRet); //以太网测试faile
					DTRACE(SOCKETS_DEBUG, ("CheckNetThread: ---OpenPppoe off---.\r\n"));
				}
			}
		}
	}

	InitPingCMD() ;//初始化一下route和ping参数
	DTRACE(SOCKETS_DEBUG, ("CheckNetThread: ------------CheckNetThread Start--------.\r\n"));

	int iMonitorID = ReqThreadMonitorID("CheckNet-thrd", 60*60);	//申请线程监控ID,更新间隔为60秒
	while(1)
	{
		UpdThreadRunClick(iMonitorID);

		bNetContTye = GetEthConfigType();	//以太网配置方式 DHCP（0），静态（1），PPPoE（2）

		if (IsDownSoft())
		{
			if (GetDownConnMode() != 0)
			{
				goto SLEEP; //下载状态下确定通讯方式了停止切换
			}
		}

		if(g_fUpdateFirmware)
		{
			goto SLEEP;///xzz add.
		}

		if(bNetContTye == 0)  //DHCP自动获取IP模式,检查网关IP是否有效
		{
			if(GetNetStat()==1) //检查一下以太网物理通道是否已OK
			{
				rip = GetDefaultGateWay();
				if (0xffffffff==rip)
					rip = 0;
			}
		}

		if (bNetContTye == 0)	//DHCP
		{
			if (GetClick() - dwOpenPppoeClick > 60)
			{
				if(GetNetStat()==1) //检查一下以太网物理通道是否已OK
				{
					bPingRet = 0;
					ReadItemEx(BN2, PN0, 0x2052, &bPingRet); //以太网测试faile
					if (0 == bPingRet)
					{
						{
							//if (IfReadListProc("ppp0") > 0) //有连接先断开下，防止是GPRS拨号连接
							{
								DTRACE(SOCKETS_DEBUG, ("CheckNetThread Close ppp0: exec /clou/ppp/script/ppp-off\n"));
								SetInfo(INFO_DISCONNECT);
								system("/clou/ppp/script/ppp-off");
								Sleep(5000);
								system("/clou/ppp/script/pppoe-off");
								Sleep(5000);
							}

							if (OpenPppoe() > 0)
							{
								InitPingCMD();      //初始化ping参数
								SetExecPingCMD(true);
								bPingRet = 1;
								WriteItemEx(BN2, PN0, 0x2052, &bPingRet); //以太网测试OK
								DTRACE(SOCKETS_DEBUG, ("CheckNetThread: ---OpenPppoe on---.\r\n"));
							}
							else
							{
								bPingRet = 0;
								WriteItemEx(BN2, PN0, 0x2052, &bPingRet); //以太网测试faile
								DTRACE(SOCKETS_DEBUG, ("CheckNetThread: ---OpenPppoe off---.\r\n"));
							}
						}
					}
				}
				else
				{
					bPingRet = 0;
					WriteItemEx(BN2, PN0, 0x2052, &bPingRet); //以太网测试faile
					DTRACE(SOCKETS_DEBUG, ("GetNetStat: Pppoe NetStat fail  \r\n"));
				}
			}
		}
		else if(bNetContTye==0 && rip<=0) //自动获取IP
		{
			//NOTE:目前在现场可能还是以GPRS模式为主，所以通过应用程序检测以太网不能太频繁
			//后面考虑是否通过系统来检测以太网物理通道是否已经OK
			if ( GetInfo(INFO_COMM_TERMIP))
				LoadSocketLocalPara(); //设置本地以太网参数
			if(rip<=0) //没有配置网关需要自动获取
			{
				if(GetNetStat()==1) //检查一下以太网物理通道是否已OK
				{
					DoUdhcpc();//system("udhcpc -n &");      //动态获取IP和默认网关
					Sleep(10000);
					rip = GetDefaultGateWay();	//取系统默认网关
					if (rip > 0 && 0xffffffff!=rip) 
					{
						InitPingCMD();      //初始化ping参数
						DTRACE(SOCKETS_DEBUG, ("CheckNetThread: System CMD Udhcpc Success  GateWay is %d.%d.%d.%d.\r\n", 
							rip&0xff, (rip>>8)&0xff, (rip>>16)&0xff, (rip>>24)&0xff));
					}
					else
						DTRACE(SOCKETS_DEBUG, ("CheckNetThread: System CMD Udhcpc fail  return 0x%4x------.\r\n",rip));
				}
				else
				{
					DTRACE(SOCKETS_DEBUG, ("CheckNetThread: ---Ethernet is not alive---.\r\n"));
				}
			}
			else
				DTRACE(SOCKETS_DEBUG, ("CheckNetThread: System CMD Udhcpc Success  GateWay is %d.%d.%d.%d.\r\n", 
				DefGWPara.bGateWay[0], DefGWPara.bGateWay[1], DefGWPara.bGateWay[2], DefGWPara.bGateWay[3])); 
		}
		else
		{
			if ( GetInfo(INFO_COMM_TERMIP))
			{
				DelDefaultGateWay();
				LoadSocketLocalPara(); //设置本地以太网参数
				InitPingCMD();      //初始化ping参数
			    bGateWayStatus = 1;
                
			}
			GetNetPara(3, bBuf);
			memcpy((BYTE*)&rip, bBuf, 4);
			if(GetNetStat()==1) //检查一下以太网物理通道是否已OK
			{
				//DTRACE(SOCKETS_DEBUG, ("CheckNetThread: System CMD Set GateWay is %d.%d.%d.%d.\r\n", 
				//		rip&0xff, (rip>>8)&0xff, (rip>>16)&0xff, (rip>>24)&0xff));
						
				if (bGateWayStatus == 0)
				{
					bGateWayStatus = 1;
					AddDefaultGateWay();
				}
				bPingRet = 1;
				WriteItemEx(BN2, PN0, 0x2052, &bPingRet); //以太网测试OK
				DTRACE(SOCKETS_DEBUG, ("CheckNetThread: ---ethernet on---.\r\n"));
			}
			else
			{
				rip = 0;
				if (bGateWayStatus == 1)
				{
					bGateWayStatus = 0;
					DelDefaultGateWay();
				}
				bPingRet = 0;
				WriteItemEx(BN2, PN0, 0x2052, &bPingRet); //以太网断开
				DTRACE(SOCKETS_DEBUG, ("CheckNetThread: ---ethernet off---.\r\n"));
			}
		}

	    
	//	if(Ping()>0)
/*		if(0xffffffff==rip || 0x00==rip)
		{
			DTRACE(SOCKETS_DEBUG, ("CheckNetThread: Ping failed becacuse DefaultGateWay is %d, goto Sleep now!!!.\r\n", rip));	
			rip = 0;
			SetExecPingCMD(true);
			goto SLEEP;
		}

		if(PingByICMP(rip)>0)
	   	{
			if (bGateWayStatus == 0)
			{
				bGateWayStatus = 1;
				AddDefaultGateWay();
			}
			bPingRet = 1;
			WriteItemEx(BN2, PN0, 0x2052, &bPingRet); //以太网测试OK
		}
		else
		{
			if (bGateWayStatus == 1)
			{
				bGateWayStatus = 0;
				DelDefaultGateWay();
			}
			bPingRet = 0;
			rip = 0;
			WriteItemEx(BN2, PN0, 0x2052, &bPingRet); //以太网断开
		}*/

		SetExecPingCMD(true);
SLEEP:
		Sleep(10*1000); //10秒检测一次
		UpdThreadRunClick(iMonitorID);
			
	}
	ReleaseThreadMonitorID(iMonitorID);
}
#endif
