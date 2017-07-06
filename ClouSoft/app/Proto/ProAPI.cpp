/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FapAPI.cpp
 * 摘    要：本文件主要包含FaProto目录下API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：
 *********************************************************************************************************/
#include "stdafx.h"
#include "syscfg.h"
#include "FaProto.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "GprsIf.h"
#include "GR47.h"
#include "Wavecom.h"
#include "MC39.h"
#include "MG815.h"
#include "CM180.h"
#include "GC864.h"
#include "MC2106.h"
#include "SocketIf.h"
#include "CommIf.h"
#include "R230mIf.h"
#include "Proto.h"
#include "ProtoIf.h"
#include "ProAPI.h"
#include "ProPara.h"
#include "drivers.h"
#include "LC6311.h"
#include "CX06833.h"
#include "P2PIf.h"
#include "ProIfAPI.h"
#ifdef EN_INMTR
#include "FaProtoDL645.h"
#endif
#include "DrvConst.h"
#include "CM180.h"
#include "CSTDMODEM.h"
#include "FaCfg.h"
#include "GprsWorker.h"

CFaProto g_pEthFaProto; 
CProtoIf *g_pEthFaProtoIf = NULL;
CFaProto g_pGprsFaProto; 
CProtoIf *g_pGprsFaProtoIf = NULL;

//主协议的相关全局变量
//CFaProto g_FaProto[2];
//TFaProPara g_GbProPara[2];
CFaProto g_FaProto;
TFaProPara g_GbProPara;

CGprsIf g_GprsIf;
TGprsPara g_GprsPara;
//GPRS工作线程类
TGprsWorkerPara g_WorkerPara;
CR230mIf g_R230mIf;		//230通信接口
TR230mIfPara g_R230mIfPara;

CP2PIf g_P2PIf;		//p2p通信接口
TP2PIfPara g_P2PIfPara;



//本地维护口
CFaProto g_FapLocal;
CFaProto g_FapTest;

CCommIf g_CommIf;
CCommIf g_CommTestIf;
CCommIf g_CommLinkIf;
TSocketPara g_SocketPara;
CSocketIf g_SocketIf;
TCommIfPara g_CommIfPara;
TCommIfPara g_CommTestIfPara;
TCommIfPara g_CommLinkIfPara;
TFaProPara g_GbLocalPara;
TFaProPara g_GbTestPara;
TFaProPara g_GbLinkPara;


#define TO_TIMES	10
bool g_fGPRSSvrOnline  = false;

DWORD g_dwGprsSvrAptClick[PRO_SVR_MAX] = {0};
DWORD g_dwEthSvrAptClick[PRO_SVR_MAX] = {0};

BYTE g_bGPRSMode = GPRS_MIX_MODE;
int g_iGprsSvrId = -1;
bool g_fSvrDiscon = false;	//服务器超时断开连接
bool g_fCliIdle = false;	//客户端处于空闲状态 
TSockSvrPara	g_FaSvrParaGprs;
TSockSvrPara	g_FaSvrParaEth;
//TGbProPara 		g_GbSvrPara;
TFaProPara		g_GbSvrPara;
CFaProto 		g_FaSvrProtoGprs[PRO_SVR_MAX];
CFaProto 		g_FaSvrProtoEth[PRO_SVR_MAX];
DWORD	g_dwSvrModeBeatTestTimes;
static bool m_fSocketInit = false;
TSocketPara g_SocketSvrPara[PRO_SVR_MAX];
TSocketPara g_SocketSvrParaEth[PRO_SVR_MAX];
TSocketPara g_SocketSvrParaGprs[PRO_SVR_MAX];

CSocketIf g_SocketSvrIf[PRO_SVR_MAX];
CSocketIf g_SocketSvrIfEth[PRO_SVR_MAX];
CSocketIf g_SocketSvrIfGprs[PRO_SVR_MAX];


CModem* NewModem(BYTE bModuleType, BYTE bModuleArea)
{
	BYTE  bConnType = 1;
	CModem *pModem;
	switch (bModuleType)
	{
		case MODULE_GR47:
			pModem = new CGR47(MODULE_GR47);
			break;
		case MODULE_WAVECOM:
			pModem= new CWavecom();
			break;
		case MODULE_GR47_15:
			pModem= new CGR47(MODULE_GR47_15);
			break;
		case MODULE_MC39:
			pModem= new CMC39(MODULE_MC39);
			break;
		case MODULE_GC864:
		    pModem= new CGC864(MODULE_GC864, bModuleArea);
		    break;
		case MODULE_ME3000:
		    pModem= new CGC864(MODULE_ME3000, bModuleArea);
		    break;
		case MODULE_EF0306:
		    pModem= new CGC864(MODULE_EF0306, bModuleArea);
		    break;
		case MODULE_MG815:
		    pModem= new CMG815(MODULE_MG815);
		    break;
		case MODULE_MC8331A:
		    pModem= new CMG815(MODULE_MC8331A);
		    break;
		case MODULE_MC39_NEW:
			pModem= new CMC39(MODULE_MC39_NEW);
			break;
		case MODULE_M580I:
			pModem= new CMC39(MODULE_M580I);
			break;
		case MODULE_LC6311:
			pModem= new CL6311(MODULE_LC6311);
			break;
		case MODULE_LC6311_2G:
			pModem= new CL6311(MODULE_LC6311_2G);
			break;
		case MODULE_MC37I:
			pModem= new CMC39(MODULE_MC39_NEW);
			break;			
		case MODULE_M580Z:
			pModem= new CGC864(MODULE_M580Z, bModuleArea);
			break;
		case MODULE_ME590:
			{
				ReadItemEx(BN1, PN0, 0x2032, &bConnType);
				if(1 == bConnType)
					pModem= new CGC864(MODULE_ME590, bModuleArea);
				else
#ifndef SYS_WIN
				pModem= new CSTDMODEM(MODULE_ME590, bModuleArea, 0x2055);
#else
				pModem= NULL;
#endif
				break;	
			}
		case MODULE_CM180:
		    pModem= new CCM180();
		    break;
		case MODULE_GL868:
			pModem = new CGC864(MODULE_GL868, bModuleArea);
			break;
		case MODULE_G600:
			pModem = new CMC39(MODULE_G600);
			break;
		case MODULE_MC323:
			pModem= new CMC39(MODULE_MC323);
			break;
		case MODULE_MC39_TEMPOFF:
			pModem= new CMC39(MODULE_MC39_TEMPOFF);
			break;
		case MODULE_GC864_1:
			pModem= new CGC864(MODULE_GC864_1, bModuleArea);
			break;		
		case MODULE_MC2106:
#ifndef SYS_WIN
			pModem= new CMC2106();
#endif
			break;		
		case MODULE_GC864_2:
			pModem= new CGC864(MODULE_GC864_2, bModuleArea);
			break;		
		default:
			pModem= new CMC39(MODULE_MC39_NEW);
			break;
	}
	return pModem;
}


CProtoIf* NewProtoIf(BYTE bSockType)
{
	BYTE bModuleType;
	ReadItemEx(BN1, PN0, 0x2012, &bModuleType); //0x2012 1 GPRS模块型号,0表示GR47 1.3版,1表示SIM,2表示WAVECOM,3表示华为,4表示GR47 1.5版

#ifdef SYS_LINUX	
	if (bModuleType == MODULE_UNKNOWN)//没有配置模块型号
		bModuleType = GetModemType();
	if (bModuleType == MODULE_UNKNOWN)
		bModuleType = MODULE_SOCKET;
#endif

#ifdef SYS_WIN	
	bModuleType = MODULE_SOCKET;
#endif

	if (bSockType == SOCK_TYPE_ETH)
	{
		g_SocketPara.IfPara.pszName = "Socket";

		LoadSocketPara(&g_SocketPara, bSockType);
		LoadSocketUnrstPara(&g_SocketPara, bSockType);
		g_SocketIf.Init(&g_SocketPara);
		g_SocketIf.SetUnrstParaFunc(LoadSocketChangePara);
		LoadSocketLocalPara();
		return (CProtoIf* )&g_SocketIf;
	}
	else	//GPRS/CDMA
	{
		CModem* pModem = NewModem(bModuleType, MODULE_FOR_GW);
		if (pModem == NULL)
		{
			DTRACE(DB_FAPROTO, ("NewProtoIf : fail to new GPRS/CDMA module, type=%d\n", bModuleType));
			return NULL;
		}

		GetGprsWorker()->AttachModem(pModem);
		LoadGprsWorkerPara(&g_WorkerPara);
		GetGprsWorker()->Init(&g_WorkerPara);
		GetGprsWorker()->SetUnrstParaFunc(LoadGprsWorkerUnrstPara);	

		LoadGprsPara(&g_GprsPara, bSockType);
		g_GprsIf.Init(&g_GprsPara);
		g_GprsIf.SetUnrstParaFunc(LoadGprsUnrstPara);		
		return (CProtoIf* )&g_GprsIf;
	}
}

CSocketIf* NewProtoSvrIf(int nCnt, BYTE bSockType)
{
	if (!m_fSocketInit)
	{
		LoadSocketLocalPara();
		m_fSocketInit = true;
	}

	if (bSockType ==  SOCK_TYPE_ETH)
	{
		LoadSocketPara(&g_SocketSvrParaEth[nCnt], bSockType);
		LoadSocketUnrstPara(&g_SocketSvrParaEth[nCnt], bSockType);
		g_SocketSvrParaEth[nCnt].IfPara.pszName="Eth-Server";
		g_SocketSvrParaEth[nCnt].fSvr = true;
		g_SocketSvrParaEth[nCnt].bSvrDisconMode = SVR_DISCON_IDLE;	//断开连接后处于空闲模式:服务器断开连接模式：
		g_SocketSvrIfEth[nCnt].Init(&g_SocketSvrParaEth[nCnt]);
		g_SocketSvrIfEth[nCnt].SetUnrstParaFunc(LoadSocketChangePara);
		return (CSocketIf* )&g_SocketSvrIfEth[nCnt];
	}
	else 
	{
		LoadSocketPara(&g_SocketSvrParaGprs[nCnt], bSockType);
		LoadSocketUnrstPara(&g_SocketSvrParaGprs[nCnt], bSockType);
		g_SocketSvrParaGprs[nCnt].IfPara.pszName ="Gprs-Server";
		g_SocketSvrParaGprs[nCnt].fSvr = true;
		g_SocketSvrParaGprs[nCnt].bSvrDisconMode = SVR_DISCON_IDLE;	//断开连接后处于空闲模式:服务器断开连接模式：
		g_SocketSvrIfGprs[nCnt].Init(&g_SocketSvrParaGprs[nCnt]);
		g_SocketSvrIfGprs[nCnt].SetUnrstParaFunc(LoadSocketChangePara);
		return (CSocketIf* )&g_SocketSvrIfGprs[nCnt];
	}

	return NULL;
}


bool InitFaProto()
{
	//协议参数的初始化
	memset(&g_GbProPara, 0, sizeof(g_GbProPara));
	LoadFaProPara(&g_GbProPara);
	g_GbProPara.ProPara.fLocal = false;
	g_GbProPara.ProPara.fAutoSend = true;
	g_GbProPara.wConnectType = CONNECTTYPE_ETH;

	//-------------------------------Ethernet--------------------------------
	g_pEthFaProto.Init(&g_GbProPara);
	//新建并初始化接口
	g_pEthFaProtoIf = NewProtoIf(SOCK_TYPE_ETH);
	if (g_pEthFaProtoIf == NULL)
	{
		DTRACE(DB_FAPROTO, ("InitFaProto : fail to new g_pEthFaProtoIf\n"));
		return false;
	}
	//协议和接口的绑定
	g_pEthFaProto.AttachIf(g_pEthFaProtoIf);
	g_pEthFaProtoIf->AttachProto(&g_pEthFaProto);
	g_pEthFaProtoIf->m_fExit=false;

	//--------------------------------GPRS-------------------------------------
	g_GbProPara.wConnectType = CONNECTTYPE_GPRS;
	g_pGprsFaProto.Init(&g_GbProPara);
	//新建并初始化接口
	g_pGprsFaProtoIf = NewProtoIf(SOCK_TYPE_GPRS);
	if (g_pGprsFaProtoIf == NULL)
	{
		DTRACE(DB_FAPROTO, ("InitFaProto : fail to new g_pGprsFaProtoIf\n"));
		return false;
	}
	//协议和接口的绑定
	g_pGprsFaProto.AttachIf(g_pGprsFaProtoIf);
	g_pGprsFaProtoIf->AttachProto(&g_pGprsFaProto);
	g_pGprsFaProtoIf->m_fExit=false;

	return true;
}

bool InitLocalProto()
{
	//维护口的初始化
	LoadFaProPara(&g_GbLocalPara);
	LoadLocalPara(&g_CommIfPara);
	g_CommIf.Init(&g_CommIfPara);
	g_FapLocal.AttachIf(&g_CommIf);
	g_CommIf.AttachProto(&g_FapLocal);
	g_GbLocalPara.ProPara.fLocal = true;
	g_GbLocalPara.ProPara.fAutoSend = false;
	g_GbLocalPara.ProPara.fUseLoopBuf = true;
	g_GbLocalPara.wConnectType = CONNECTTYPE_LOCAL;
	g_FapLocal.Init(&g_GbLocalPara);	
	g_CommIf.m_fExit=false;
	return true;
}


#ifdef EN_INMTR
//本地红外645协议维护口
bool InitLocalDL645Proto()
{
	if (g_CommTestIf.m_pComm == NULL)	//红外通道与232通道公用
		return false;

	//内部电表口的初始化
	memset(&g_Local645ProPara, 0, sizeof(g_Local645ProPara));
	LoadLocalDL645ProPara(&g_Local645ProPara);
	LoadLocalDL645CommProPara(&g_Local645ProPara);

	g_LocalDL645.AttachIf(&g_CommTestIf);
	//g_CommIf.AttachProto(&g_LocalDL645);	//不用绑定645协议了
	g_LocalDL645.Init(&g_Local645ProPara);
}
#endif

bool InitTestProto()
{
	//维护口的初始化
	LoadFaProPara(&g_GbTestPara);
	LoadTestPara(&g_CommTestIfPara);
	g_CommTestIf.Init(&g_CommTestIfPara);
	g_FapTest.AttachIf(&g_CommTestIf);
	g_CommTestIf.AttachProto(&g_FapTest);
	g_GbTestPara.ProPara.fLocal = true;
	g_GbTestPara.ProPara.fAutoSend = false;
	g_GbTestPara.ProPara.fUseLoopBuf = true;
	g_GbTestPara.wConnectType = CONNECTTYPE_LOCAL;
	g_FapTest.Init(&g_GbTestPara);	
	g_CommTestIf.m_fExit = false;
	return true;
}


bool InitSvrProto()
{
	//LoadGbProPara(&g_GbSvrPara);
	LoadFaProPara(&g_GbSvrPara);
	g_GbSvrPara.ProPara.fAutoSend = false;
	g_GbSvrPara.ProPara.fLocal = false;
	g_GbSvrPara.wConnectType = CONNECTTYPE_ETH;


	//初始化以太网服务器相关参数
#ifndef SYS_WIN
	BYTE bIpAddr[8] = {0};
	GetEthLocalIp(bIpAddr);	
	g_FaSvrParaEth.dwLocalIP = OoOadToDWord(bIpAddr);
	g_FaSvrParaEth.wLocalPort = GetEthListenPort();
#else
	g_FaSvrParaEth.dwLocalIP = 0x7f000001;		
	g_FaSvrParaEth.wLocalPort = 9300;
#endif

	g_FaSvrParaEth.pszName = "Svr-Eth";
	g_FaSvrParaEth.wConnectNum = PRO_SVR_MAX;
	for (WORD i=0; i<PRO_SVR_MAX; i++)
	{
		g_FaSvrParaEth.pProto[i] = (CProto* )&g_FaSvrProtoEth[i];
		((CFaProto *) g_FaSvrParaEth.pProto[i])->Init(&g_GbSvrPara);

		g_FaSvrParaEth.pSocketIf[i] = (CSocketIf* )NewProtoSvrIf(i, SOCK_TYPE_ETH);
		g_FaSvrParaEth.pSocketIf[i]->AttachProto(g_FaSvrParaEth.pProto[i]);
		g_FaSvrParaEth.pProto[i]->AttachIf(g_FaSvrParaEth.pSocketIf[i]);
		g_FaSvrParaEth.pSocketIf[i]->m_fExitDone = true;
	}

#ifndef  SYS_WIN
	//初始化Gprs服务器相关参数
	g_GbSvrPara.wConnectType = CONNECTTYPE_GPRS;
	WORD wLocalPort;
	g_FaSvrParaGprs.dwLocalIP = 0;		//到这里Gprs无法获取到IP，需要pppd拨号之后才可以
	wLocalPort = GetGprsListenPort();
	g_FaSvrParaGprs.wLocalPort = OoOiToWord((BYTE*)&wLocalPort);

	g_FaSvrParaGprs.pszName = "Svr-Gprs";
	g_FaSvrParaGprs.wConnectNum = PRO_SVR_MAX;
	for (WORD i=0; i<PRO_SVR_MAX; i++)
	{
		g_FaSvrParaGprs.pProto[i] = (CProto* )&g_FaSvrProtoGprs[i];
		((CFaProto *) g_FaSvrParaGprs.pProto[i])->Init(&g_GbSvrPara);

		g_FaSvrParaGprs.pSocketIf[i] = (CSocketIf* )NewProtoSvrIf(i, SOCK_TYPE_GPRS);
		g_FaSvrParaGprs.pSocketIf[i]->AttachProto(g_FaSvrParaGprs.pProto[i]);
		g_FaSvrParaGprs.pProto[i]->AttachIf(g_FaSvrParaGprs.pSocketIf[i]);
		g_FaSvrParaGprs.pSocketIf[i]->m_fExitDone = true;
	}
#endif

	return true;
}


bool InitProto()
{
	InitProIf();
	InitFaProto();
	InitTestProto();

	InitSvrProto();
#ifdef SYS_LINUX
	InitLocalProto();
#endif
	return true;
}


//取得GPRS工作模式
BYTE GetNetWorkMode()
{
	BYTE bModule;
	BYTE bWorkMode;

	ReadItemEx(BANK2, PN0, 0x10d3, &bModule);
	if (bModule == MODULE_SOCKET)
		bWorkMode = GetEthWorkMode();
	else
		bWorkMode = GetGprsWorkMode();

	return bWorkMode;
}

void GprsDisConnect()
{
	DWORD dwClick;
	BYTE bMode = GetNetWorkMode();
	switch(bMode)
	{
	case GPRS_MIX_MODE:
		for (int i=0; i<g_FaSvrParaGprs.wConnectNum; i++)
		{
			if (g_FaSvrParaGprs.pSocketIf[i]->IsIfValid())
			{
				g_FaSvrParaGprs.pSocketIf[i]->SetDisConnect();
				dwClick = GetClick();
				do 
				{
					if (!g_FaSvrParaGprs.pSocketIf[i]->IsIfValid())
						break;
					Sleep(500);
				} while (GetClick()-dwClick < 10);
			}
		}
		
		g_fCliIdle = false;
		g_pGprsFaProtoIf->SetIdle();
		dwClick = GetClick();
		do{ 
			Sleep(1000); 
		} 
		while ((GetClick()-dwClick<120) && (g_fCliIdle==false));
		
		ReqOffline(g_iGprsSvrId);
		Sleep(4000);
		ReqOnline(g_iGprsSvrId);
		
		g_fSvrDiscon = true;
        SetInfo(INFO_ACTIVE);
		break;

	case GPRS_SER_MODE:
		for (int i=0; i<g_FaSvrParaGprs.wConnectNum; i++)
		{
			if (g_FaSvrParaGprs.pSocketIf[i]->IsIfValid())
			{
				g_FaSvrParaGprs.pSocketIf[i]->SetDisConnect();
			}
			DWORD dwClick = GetClick();
			do 
			{
				if (!g_FaSvrParaGprs.pSocketIf[i]->IsIfValid())
					break;
				Sleep(500);
			} while (GetClick()-dwClick < 10);
		}
		ReqOffline(g_iGprsSvrId);
		Sleep(2000);
		ReqOnline(g_iGprsSvrId);
		break;

	case GPRS_CLI_MODE:
		g_pGprsFaProtoIf->SetDisConnect();
		break;
	}
}

//描述：此函数的主要目的是保持GPRS在线，主要用在GPRS服务器模式下，终端有稳定的链路连接
TThreadRet GprsKeepThread(void* pvPara)
{
	TSockSvrPara* pSockSvrPara = (TSockSvrPara* )pvPara;
	DWORD dwBeatSeconds;
	BYTE bBuf[12];
    int i;
	
	DTRACE(DB_FAPROTO, ("GprsKeepThread : [%s] started!, g_iGprsSvrId=%d.\n", pSockSvrPara->pszName, g_iGprsSvrId));
	if (g_iGprsSvrId < 0)
		return THREAD_RET_OK;
	ReqOnline(g_iGprsSvrId);
	dwBeatSeconds = GetGprsBeat();

	DWORD dwSvrRxClick = 0;
	while (1)
	{
		Sleep(1000);
		if (GetInfo(INFO_GPRS_OFFLINE))
		{
			DTRACE(DB_FAPROTO, ("GprsKeepThread: GetInfo INFO_GPRS_OFFLINE, ReqOffline !!\r\n"));
			ReqOffline(g_iGprsSvrId);
			Sleep(6000);		//延时3秒，让模块重新拨号
			ReqOnline(g_iGprsSvrId);
		}
		if (GetNetWorkMode() == GPRS_CLI_MODE)//运行中修改了通信模式，改为客户端长在线模式
		{
			ReqOffline(g_iGprsSvrId);
			Sleep(2000);
			continue;
		}
		 /* 
		else if (GetNetWorkMode() == GPRS_MIX_MODE && g_GprsIf.GetCnMode() == CN_MODE_SMS)
		{//混合模式，且终端客户端connect成功，但login不成功16次之后会进入dorman
			ReqOffline(g_iGprsSvrId); //用户离线，让终端模块复位
			SetInfo(INFO_ACTIVE);
		}*/

		
		bool fSvrOnline = false;
		DWORD dwCommClick = 0;
		//维护线程心跳是否过期，过期掉线
		for (i=0; i<pSockSvrPara->wConnectNum; i++)
		{
			if (pSockSvrPara->pSocketIf[i]->IsIfValid())
			{
                dwCommClick = pSockSvrPara->pSocketIf[i]->GetRxClick();

                if (g_dwGprsSvrAptClick[i] > pSockSvrPara->pSocketIf[i]->GetRxClick())
                  dwCommClick = g_dwGprsSvrAptClick[i];

				if (GetClick()-dwCommClick >= 2*60)	//终端接收主站连接后，如果主站连续两分钟没有通信，终端主动断开链接
				{
					DTRACE(DB_FAPROTO, ("GprsKeepThread: %s svr=%d exit!!\r\n", pSockSvrPara->pszName, i));
					pSockSvrPara->pSocketIf[i]->SetDisConnect();
					DWORD dwClick = GetClick();
					do 
					{
						if (!pSockSvrPara->pSocketIf[i]->IsIfValid())
							break;
						Sleep(500);
					} while (GetClick()-dwClick < 10);
				}
			}

			if (pSockSvrPara->pSocketIf[i]->IsIfValid())
			{
				fSvrOnline = true;
			}

			if (dwSvrRxClick < dwCommClick)
			{
				dwSvrRxClick = dwCommClick;
			}
		}

		g_fGPRSSvrOnline = fSvrOnline;
		if (!fSvrOnline && (GetClick()-dwSvrRxClick) >= g_dwSvrModeBeatTestTimes)
		//if (!fSvrOnline && (GetClick()-dwSvrRxClick) >= (dwBeatMinutes*60*TO_TIMES))
		{//没有一个客户端连接到服务器，且当前时间距离上次客户端连接的时间超过了10个心跳周期就断开重连
		 //防止启动后终端客户端连到服务器，但主站不连接终端服务器fSvrOnline为false，导致终端不停重连
          DTRACE(DB_FAPROTO, ("GprsKeepThread:  let client connect master when svr off, now Click:%d, SvrRxClick:%d, beat:%d!!\r\n",GetClick(), dwSvrRxClick, dwBeatSeconds));
          
          GprsDisConnect();
          dwSvrRxClick = GetClick();
		}

        if (GetNetWorkMode() == GPRS_SER_MODE)
        {
          if (GetGprsWorker()->IsOnline())
		  {
			continue;
		  }
		  else
		  {
			DTRACE(DB_FAPROTO, ("GprsKeepThread:  ReqOnline when GprsWorker is offline!!\r\n"));
			
			ReqOnline(g_iGprsSvrId);

			Sleep(5000);
		  }
        }
	}

	return THREAD_RET_OK;
}


//描述：此函数的主要目的是保持以太网在线，主要用在以太网服务器模式下，终端有稳定的链路连接
TThreadRet EthKeepThread(void* pvPara)
{
	TSockSvrPara* pSockSvrPara = (TSockSvrPara* )pvPara;
	DWORD dwBeatSeconds;
	BYTE bBuf[12];
    int i;

	DTRACE(DB_FAPROTO, ("EthKeepThread : [%s] started!\n", pSockSvrPara->pszName));
	dwBeatSeconds = GetEthBeat();

	DWORD dwSvrRxClick = 0;
	while (1)
	{
		Sleep(1000);
		
		bool fSvrOnline = false;
		DWORD dwCommClick = 0;
		//维护线程心跳是否过期，过期掉线
		for (i=0; i<pSockSvrPara->wConnectNum; i++)
		{
			if (pSockSvrPara->pSocketIf[i]->IsIfValid())
			{
                dwCommClick = pSockSvrPara->pSocketIf[i]->GetRxClick();

                if (g_dwEthSvrAptClick[i] > pSockSvrPara->pSocketIf[i]->GetRxClick())
                  dwCommClick = g_dwEthSvrAptClick[i];

				if (GetClick()-dwCommClick >= 2*60)	//终端接收主站连接后，如果主站连续两分钟没有通信，终端主动断开链接
				{
					DTRACE(DB_FAPROTO, ("EthKeepThread: %s svr=%d exit!!\r\n", pSockSvrPara->pszName, i));
					pSockSvrPara->pSocketIf[i]->SetDisConnect();
					DWORD dwClick = GetClick();
					do 
					{
						if (!pSockSvrPara->pSocketIf[i]->IsIfValid())
							break;
						Sleep(500);
					} while (GetClick()-dwClick < 10);
				}
			}

			if (pSockSvrPara->pSocketIf[i]->IsIfValid())
			{
				fSvrOnline = true;
			}

			if (dwSvrRxClick < dwCommClick)
			{
				dwSvrRxClick = dwCommClick;
			}
		}

		g_fGPRSSvrOnline = fSvrOnline;
		if (!fSvrOnline && (GetClick()-dwSvrRxClick) >= g_dwSvrModeBeatTestTimes)
		//if (!fSvrOnline && (GetClick()-dwSvrRxClick) >= (dwBeatMinutes*60*TO_TIMES))
		{//没有一个客户端连接到服务器，且当前时间距离上次客户端连接的时间超过了10个心跳周期就断开重连
		 //防止启动后终端客户端连到服务器，但主站不连接终端服务器fSvrOnline为false，导致终端不停重连
          DTRACE(DB_FAPROTO, ("GprsKeepThread:  let client connect master when svr off, now Click:%d, SvrRxClick:%d, beat:%d!!\r\n",GetClick(), dwSvrRxClick, dwBeatSeconds));
          
          GprsDisConnect();
          dwSvrRxClick = GetClick();
		}

        if (GetNetWorkMode() == GPRS_SER_MODE)
        {
          if (GetGprsWorker()->IsOnline())
		  {
			continue;
		  }
		  else
		  {
			DTRACE(DB_FAPROTO, ("GprsKeepThread:  ReqOnline when GprsWorker is offline!!\r\n"));
			Sleep(5000);
		  }
        }
	}

	return THREAD_RET_OK;
}

void NewFaProtoThread()
{
#ifndef SYS_WIN
	if (g_pGprsFaProtoIf->GetIfType() == IF_GPRS || (g_pGprsFaProtoIf != NULL && g_pGprsFaProtoIf->GetIfType() == IF_GPRS))
		NewThread(GprsWorkerThread, NULL, 8192, THREAD_PRIORITY_NORMAL);

	NewThread(StdProtoThread, &g_pGprsFaProto, 16384, THREAD_PRIORITY_ABOVE_NORMAL);
	if (GetGprsWorkMode() != GPRS_CLI_MODE)	//如果不是简单客户端模式，启动GPRS服务器线程和保持在线线程
	{
		g_iGprsSvrId = GetGprsWorker()->ReqUserID();		
		DTRACE(DB_FAPROTO, ("GprsKeepThread: g_iGprsSvrId=%d \r\n", g_iGprsSvrId));

		NewThread(SockSvrThread, &g_FaSvrParaGprs, 12288, THREAD_PRIORITY_NORMAL);
		NewThread(GprsKeepThread, &g_FaSvrParaGprs, 12288, THREAD_PRIORITY_NORMAL);	//此线程比StdProtoThread先创建，可以获得GPRSid为0，只有为0的客户有权利进去Dorman
		for (int i=0; i<g_FaSvrParaGprs.wConnectNum; i++)
		{
			NewThread(StdProtoThread, g_FaSvrParaGprs.pProto[i], 16384, THREAD_PRIORITY_NORMAL);
		}
	}
#endif

	NewThread(StdProtoThread, &g_pEthFaProto, 16384, THREAD_PRIORITY_ABOVE_NORMAL);
	if (GetEthWorkMode() != GPRS_CLI_MODE)
	{
		NewThread(SockSvrThread, &g_FaSvrParaEth, 12288, THREAD_PRIORITY_NORMAL);
		NewThread(EthKeepThread, &g_FaSvrParaEth, 12288, THREAD_PRIORITY_NORMAL);	//此线程比StdProtoThread先创建，可以获得GPRSid为0，只有为0的客户有权利进去Dorman
		for (int i=0; i<g_FaSvrParaEth.wConnectNum; i++)
		{
			NewThread(StdProtoThread, g_FaSvrParaEth.pProto[i], 16384, THREAD_PRIORITY_NORMAL);
		}
	}

	//备注：以下线程只能二选1，因为他们都绑定了相同的物理端口！！！CL 20161030
	NewThread(StdProtoThread, &g_FapTest, 16384, THREAD_PRIORITY_NORMAL);

#ifdef SYS_LINUX
	NewThread(StdProtoThread, &g_FapLocal, 16384, THREAD_PRIORITY_NORMAL);
#endif
} 

bool InitUpdateProto(CFaProto  *pUpdateFapLocal, 
					TFaProPara *pUpdateGbLocalPara, 
					CCommIf    *pUpdateCommIf,
					TCommIfPara *pUpdateCommIfPara,
					CComm	*pComm
					)
{
	//维护口的初始化
	//LoadFaProPara(pUpdateGbLocalPara);
	LoadLocalPara(pUpdateCommIfPara);
	pUpdateCommIfPara->pComm = pComm;//修正为实际的串口
	pUpdateCommIfPara->IfPara.pszName = "Local_Update";	//接口名称,Local为红外
	pUpdateCommIf->Init(pUpdateCommIfPara);
	pUpdateFapLocal->AttachIf(pUpdateCommIf);
	pUpdateCommIf->AttachProto(pUpdateFapLocal);
	pUpdateGbLocalPara->ProPara.fLocal = true;
	pUpdateGbLocalPara->ProPara.fAutoSend = false;
	pUpdateGbLocalPara->ProPara.fUseLoopBuf = true;
	pUpdateGbLocalPara->wConnectType = CONNECTTYPE_LOCAL;
	pUpdateFapLocal->Init(pUpdateGbLocalPara);	
	pUpdateCommIf->m_fExit=false;
	return true;
}


void NewFaUpdateThread()
{
    CFaProto *pUpdateFapLocal = new CFaProto;		
	TFaProPara* pUpdateGbLocalPara = new TFaProPara;
	CCommIf* pUpdateCommIf = new CCommIf;   
	TCommIfPara* pUpdateCommIfPara = new TCommIfPara;
	CComm* pUpdateComm = new CComm;
	DWORD dwRade = CBR_115200;	
	
	if (g_bRemoteDownIP[3] == 0x01)
		dwRade = CBR_9600;	
	else if(g_bRemoteDownIP[3] == 0x02)
		dwRade = CBR_19200;	
	else if(g_bRemoteDownIP[3] == 0x04)
		dwRade = CBR_38400;	
	else if(g_bRemoteDownIP[3] == 0x06)
		dwRade = CBR_57600;
	while (!pUpdateComm->Open(COMM_METER, dwRade, 8, ONESTOPBIT, NOPARITY))
	{
	    DTRACE(DB_FAPROTO, ("NewFaRemoteDownThread : Open port failed, retry 2000ms later.\r\n"));
	    Sleep(2000);
	}
	DTRACE(DB_FAPROTO, ("NewFaRemoteDownThread : Open port%d OK.\r\n", COMM_METER));
	InitUpdateProto(pUpdateFapLocal, pUpdateGbLocalPara, pUpdateCommIf, pUpdateCommIfPara, pUpdateComm);
    		
	NewThread(StdProtoThread, pUpdateFapLocal, 16384,THREAD_PRIORITY_NORMAL);

	NewFaProtoThread();
}



void WHTraceSecsTime(char *pStr,DWORD dwSecs)
{
	TTime t;
	SecondsToTime(dwSecs, &t);
	DTRACE(DB_FAPROTO, ("%s",pStr));
	DTRACE(DB_FAPROTO, (" %d-%02d-%02d %02d:%02d:%02d\r\n",t.nYear,t.nMonth,t.nDay,t.nHour,t.nMinute,t.nSecond));
}
