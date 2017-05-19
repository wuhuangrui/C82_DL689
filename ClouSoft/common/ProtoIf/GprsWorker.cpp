/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：GprsWorker.cpp
 * 摘    要：本文件主要实现GPRS工作线程
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年8月
 * 备    注：
 *********************************************************************************************************/
#include "stdafx.h"
#include <stdio.h>
#include <sys/stat.h>
#include "FaCfg.h"
#include "FaConst.h"
#include "sysarch.h" 
#include "GprsWorker.h"
#include "ProIfConst.h"
#include "sysapi.h"
#include "GprsIf.h"
#include "ComAPI.h"
#include "ThreadMonitor.h"
#include "ProIfAPI.h"
#include "ProHook.h"

#if defined(SYS_VDK) || defined(SYS_UCOSII)
#include "lwip/ctrl.h"
#include "ppp.h"
#endif //SYS_VDK

#define GPRS_DEBUG_INTERV	(2*60)   //调试输出的间隔,单位秒

#define GPRS_MODE_IDLE     	0	//空闲模式
#define GPRS_MODE_SOCKET    1	//基于TCP/IP的通信模式
#define GPRS_MODE_SMS      	2	//短信
#define GPRS_MODE_COMM     	3	//串口通信模式
#define GPRS_MODE_CMD     	4	//命令模式
#define GPRS_MODE_PWROFF	5	//掉电模式

#define SMS_MAX_FRM 		10
#define SMS_FRM_SIZE 		256

CGprsWorker g_GprsWorker;

/////////////////////////////////////////////////////////////////////////////////////////////////////
//CGprsWorker

CGprsWorker::CGprsWorker()
{
	m_wSignStrength = 0;
	m_wState = GPRS_STATE_IDLE; //GPRS_STATE_IDLE; GPRS_STATE_SMS

	m_dwUser = 0;
	m_dwOnlineReqFlg = 0;	//对应位为1表示该用户要上线
	m_dwRstOnSmsFlag = 0;	//对应位为1表示该用户下线后要处于短信状态
	
	m_iPd = -1;	//ppp的设备号,只在lwip中用到
	
	m_dwDebugClick = 0;
	m_dwSmsOverflowClick = 0;
	m_dwSignClick = 0;
	m_dwUpdTxPwrClick = 0;
	m_dwUpdSysInfoClick = 0;
	m_bLedAct = 0;

	m_pfnLoadUnrstPara = NULL;
	m_fModemPwrOn = true;		//false
	m_wFailCnt = 0;		//上网失败计数
	m_wDormanCnt = 0;	//主用户请求进入休眠状态计数
	m_iLastErr = GPRS_ERR_OK;
	m_wGprsWorkStep = GPRS_STEP_IDLE;
	m_dwMuxClick = 0;
}

CGprsWorker::~CGprsWorker()
{
	
}

void CGprsWorker::LoadUnrstPara()
{
	if (m_pfnLoadUnrstPara != NULL)
	{
		(*m_pfnLoadUnrstPara)(m_pWorkerPara);
	}	
}

//描述:申请GPRS的用户号
//返回:正确0~31,错误-1
int CGprsWorker::ReqUserID()
{
	WORD i;
	DWORD dwMask = 1;

	WaitSemaphore(m_semWorker);

	for (i=0; i<32; i++)
	{
		if ((m_dwUser&dwMask) == 0)
		{
			m_dwUser |= dwMask;
			break;
		}

		dwMask <<= 1;
	}

	SignalSemaphore(m_semWorker);

	if (i < 32)
		return i;
	else
		return -1;
}

//描述:释放GPRS的用户号
void CGprsWorker::ReleaseUserID(int iUser)
{
	DWORD dwMask = 1 << iUser;

	WaitSemaphore(m_semWorker);

	m_dwUser &= ~dwMask;

	SignalSemaphore(m_semWorker);
}

//描述:向工作者线程提出上线请求
void CGprsWorker::ReqOnline(int iUser)
{
	if (iUser < 0)
		return;

	DWORD dwMask = 1 << iUser;

	WaitSemaphore(m_semWorker);

	m_dwOnlineReqFlg |= dwMask;

	SignalSemaphore(m_semWorker);
}

void CGprsWorker::ReqOffline(int iUser)
{
	if (iUser < 0)
		return;

	DWORD dwMask = 1 << iUser;

	WaitSemaphore(m_semWorker);

	m_dwOnlineReqFlg &= ~dwMask;

	SignalSemaphore(m_semWorker);
}

void CGprsWorker::ReqDorman(int iUser)
{
	if (iUser != 0)	//我们把用户0当成是主用户,
	{
		DTRACE(DB_FAPROTO, ("CGprsWorker::ReqDorman : iUser=%ld not suported\n", iUser));
		return;		//只有主用户请求休眠才有效
	}
	
	m_wDormanCnt++;
	DTRACE(DB_FAPROTO, ("CGprsWorker::ReqDorman : iUser=0, m_wDormanCnt=%d, m_wFailCnt=%d\n", 
						m_wDormanCnt, m_wFailCnt));
}

void CGprsWorker::SetWorkMode(int iUser, BYTE bRstOnSms)
{
	if (iUser < 0)
		return;

	DWORD dwMask = 1 << iUser;

	WaitSemaphore(m_semWorker);
	
	if (bRstOnSms)
		m_dwRstOnSmsFlag |= dwMask;
	else
		m_dwRstOnSmsFlag &= ~dwMask;

	SignalSemaphore(m_semWorker);
}

bool CGprsWorker::Init(TGprsWorkerPara* pWorkerPara)
{	
	m_dwUser = 0;
	m_semWorker = NewSemaphore(1);
	m_semWorkerLocker = NewSemaphore(1);
	
	m_pWorkerPara = pWorkerPara;		
	m_Comm.Config(m_pWorkerPara->CommPara);

	if (!m_pModem->Init(&m_pWorkerPara->ModemPara))
		return false;
	
	m_pModem->SetComm(&m_Comm); //串口共用
			//串口可能还要被别的接口所使用,所以本接口线程虽然起来了,但先不要打开串口
			
	if (m_pWorkerPara->fEmbedProtocol)//模块协议栈不用跟别的地方公用串口
	{
		m_Comm.Open();
	}
	
	m_SmsRxFrmQue.Init(SMS_MAX_FRM, SMS_FRM_SIZE);	//短信接收的帧队列
	m_SmsTxFrmQue.Init(SMS_MAX_FRM, SMS_FRM_SIZE);	//短信发送的帧队列
			
  	return true;
}

bool CGprsWorker::OpenPpp()
{
#ifdef SYS_LINUX
	char command[256];
	int nCnt = 0;
	WORD wConnectWait = m_pWorkerPara->wConnectWait;
	
	if (m_pWorkerPara->fEmbedProtocol)
		return m_pModem->PPPOpen(m_pModem->GetATDT(), m_pWorkerPara->szPppUser, m_pWorkerPara->szPppPsw);

	
  	memset(command, 0, sizeof(command));
  	if (m_pModem->GetModuleType()==MODULE_LC6311 || 
  		m_pModem->GetModuleType()==MODULE_LC6311_2G)
  	{
  		sprintf(command, "/clou/ppp/script/ppp-on %s %s %s %d /dev/ttyUSB1",
  						  m_pWorkerPara->szPppUser, m_pWorkerPara->szPppPsw, 
  						  m_pModem->GetATDT(), wConnectWait);  		
  	}
  	else
  	{
		if(m_pWorkerPara->fEnMux)
  			sprintf(command, "/clou/ppp/script/ppp-on %s %s %s %d", 
  						  m_pWorkerPara->szPppUser, m_pWorkerPara->szPppPsw, 
  						  m_pModem->GetATDT(), wConnectWait);
		else
			sprintf(command, "/clou/ppp/script/ppp-on %s %s %s %d /dev/ttyAT3", 
			m_pWorkerPara->szPppUser, m_pWorkerPara->szPppPsw, 
			m_pModem->GetATDT(), wConnectWait);

	}
  	DTRACE(DB_FAPROTO, ("CGprsWorker::OpenPpp : command : %s.\r\n", command));
  	
	//if (!m_pWorkerPara->fDetailDiagnose)
	{	  	
  		m_wGprsWorkStep = GPRS_STEP_PPP;		
  		wConnectWait += 10;
  		for (WORD i=0; i<3; i++)
  		{
  			ClosePpp();
  			DTRACE(DB_FAPROTO, ("CGprsWorker::OpenPpp: connect try %d\n", i));
  			system(command);
  			for (WORD j=0; j<wConnectWait; j++)
  			{
  				Sleep(1000);
  				if (IfReadListProc("ppp0") > 0)//拨号成功
  				{
  					if (m_pWorkerPara->fEnSocketLed && m_pWorkerPara->fEnMux) //是否控制Socket Led,只针对国网标准模块
					{
						//if(m_pWorkerPara->fEnMux)
  							SetSockLed(false);
					}  					
  					return true;
  				}
  					
  				if (IfReadListProc("ppp0") == -1)
  					return true;

  			/*
  				if (IfReadListProc("ppp0") == -1)//本地/远程IP相同，这种情况对于M580I/Z可以联网，但是对于其它模块则不行
  				{
  					if (m_pModem->GetModuleType()==MODULE_M580Z 
  				 	|| m_pModem->GetModuleType()==MODULE_M580I
  				 	|| m_pModem->GetModuleType()==MODULE_ME3000
  				 	|| m_pModem->GetModuleType()==MODULE_MC39
  				 	|| m_pModem->GetModuleType()==MODULE_MC39_NEW)
	  					return true;
  					break;
  				}
  			*/	
  			}
  		}
		
		unsigned int step, error;
    	int f;
    	//char* steps[] = { "Serial connection established", "authentication succeeded", "local  IP address" };
    	//char* errors[] = { "no error", "authentication failed", "get ip failed" };
		//由于没有改动chat程序，因此只要调用的ppp但是没有成功，并且ppp中的各种错误也都没有就认为是拨号失败。
    	f = open("/var/log/ppp_stats", O_RDONLY, S_IREAD|S_IWRITE);  //|O_BINARY ,|O_BINARY S_IREAD|S_IWRITE
		if (f >= 0)
		{
			lseek(f, 0, SEEK_SET);
			read(f, &step, sizeof(step));
			lseek(f, 4, SEEK_SET);
			read(f, &error, sizeof(error));
			close(f);
			//if (step>2 || error>2)
			//	printf("step %d, error %d.\r\n", step, error);
			//else
			//	printf("%s, %s\n", steps[step], errors[error]);
			return false;
			/*if (error == 1)
				return GPRS_ERR_AUTH;
			else if (error == 2)
				return GPRS_ERR_IP;
			else
				return GPRS_ERR_PPP;*/
		}  		
  	}
	
#endif //SYS_LINUX

#if defined(SYS_VDK) || defined(SYS_UCOSII)
    
	if (m_pWorkerPara->fEmbedProtocol)
		return m_pModem->PPPOpen(m_pModem->GetATDT(), m_pWorkerPara->szPppUser, m_pWorkerPara->szPppPsw);    
    
	//SetAuth();
	pppSetAuth(PPPAUTHTYPE_ANY, m_pWorkerPara->szPppUser, m_pWorkerPara->szPppPsw); 
	
	for (WORD i=0; i<2; i++)
	{
		if (m_pModem->ATCommand(m_pModem->GetATDT(), "CONNECT", 
								NULL, NULL, m_pWorkerPara->wConnectWait) > 0)
		{
			m_iPd = pppOpen(&m_Comm, NULL, NULL);
			if (m_iPd >= 0)
			{
				DTRACE(DB_FAPROTO, ("CGprsWorker::ResetForSock : pppOpen ok, fd=%d .\r\n", m_iPd));
				return true;
			}
			else
			{
				DTRACE(DB_FAPROTO, ("CGprsWorker::ResetForSock : pppOpen fail, fd=%d .\r\n", m_iPd));
			}
		}
	}
	
	//UpdateErrRst(false);   //在这里只更新失败,只有在socket成功了才更新成功
#endif //SYS_VDK

	m_iLastErr = GPRS_ERR_PPP;
	return false;	
}

bool CGprsWorker::ClosePpp(void)
{	
	if (m_pWorkerPara->fEmbedProtocol)
	{
		return m_pModem->PPPClose();
	}
	
#ifdef SYS_LINUX	
//	if (IfReadListProc("ppp0") > 0)
//	{
		DTRACE(DB_FAPROTO, ("CGprsWorker::ClosePPP: exec /clou/ppp/script/ppp-off\n"));
		system("/clou/ppp/script/ppp-off");
		unlink("/var/log/ppp_stats");
		Sleep(1000);
//	}
#endif //SYS_LINUX
	
#if defined(SYS_VDK) || defined(SYS_UCOSII)
	if (m_iPd >= 0)
	{
		int iRet = pppClose(m_iPd);
		DTRACE(DB_FAPROTO, ("CFapGPRS::ResetGPRS : close ppp fd=%d, ret=%d .\r\n", m_iPd, iRet));
		m_iPd = -1;
	}
#endif //SYS_VDK
	
	return true;
}

bool CGprsWorker::UpdateSignStrength()
{
	m_wSignStrength = m_pModem->UpdateSignStrength();
	return m_pModem->IsSignValid(m_wSignStrength);
}

int CGprsWorker::ResetModem(bool fNeedPwrOn, WORD wCnMode)
{
	m_dwMuxClick = 0;//不允许使用串口复用
	if (fNeedPwrOn)
	{
		m_pModem->PowerOn();
		m_fModemPwrOn = true;
		//Sleep(5000);
	}
	
	if (m_pWorkerPara->fEnMux)		//是否允许串口复用
	{
		//每次还是关闭一下串口	
		if (m_Comm.IsOpen())
		{	
			DTRACE(DB_FAPROTO, ("CGprsWorker::ResetModem: close comm for mux rst\r\n"));
			m_Comm.Close();
		}
		
		m_Comm.Config(m_pWorkerPara->CommPara);		//这里打开的是最原始的物理串口（#define COMM_GPRS 3）
			//因为在ppp拨号前，调用下面函数重新打开复用的硬件串口m_Comm.Open(17, CBR_115200, 8, ONESTOPBIT, NOPARITY);	
			//它会改变m_Comm中的端口号配置，所以这里要恢复一下串口的配置
			//17号串口是不能直接用来发数据的
	}
	
	//从这个地方直到PPP拨号前，AT命令用的都是实际串口，
	//ppp拨号后，AT命令再要用串口，比如socket连接上后设置LED、接收短信等，
	//必须要用GSMTTY串口，并且用完后马上关闭,GSMTTY串口不宜一直打开
	//避免出现系统退出时，串口关闭顺序的问题
	if (!m_Comm.IsOpen())
	{
		DTRACE(DB_FAPROTO, ("CGprsWorker::ResetModem: re open comm for at\n"));
		m_Comm.Open();
	}
	m_wGprsWorkStep = GPRS_STEP_RST;
	if (m_pModem->ResetModem() != MODEM_NO_ERROR)
	{
		m_iLastErr = GPRS_ERR_RST;
		DTRACE(DB_FAPROTO, ("CGprsWorker::ResetModem: ResetModem Fail!\n"));
		return IF_RST_HARDFAIL;
	}	

	
	m_wGprsWorkStep = GPRS_STEP_SIGN;
	for (WORD i=0; i<5; i++)
	{
		if (UpdateSignStrength())
		{
			break;
		}
		Sleep(2000);
	}

	m_wGprsWorkStep = GPRS_STEP_APN;
	char cAPN[64] = {0};
	int iModemRet = m_pModem->InitAPN(cAPN);
	if (iModemRet != MODEM_NO_ERROR)
	{
		if (iModemRet == MODEM_SIM_FAIL)
			m_iLastErr = GPRS_ERR_SIM;
		else if (iModemRet == MODEM_REG_FAIL)
			m_iLastErr = GPRS_ERR_REG;
		return IF_RST_SOFTFAIL;
	}
	else
	{
		memset(m_pWorkerPara->ModemPara.szAPN, 0, sizeof(m_pWorkerPara->ModemPara.szAPN));
		strcpy(m_pWorkerPara->ModemPara.szAPN, cAPN);
		DTRACE(DB_FAPROTO, ("CGprsWorker::ResetModem: Init APN:<%s> success!\r\n", m_pWorkerPara->ModemPara.szAPN));
	}

	m_wGprsWorkStep = GPRS_STEP_SIGN;
	//if (m_wSignStrength<=0 || m_wSignStrength>31)
	{
		for (WORD i=0; i<5; i++)
		{
			if (UpdateSignStrength())
			{
				break;
			}
			Sleep(2000);
		}
	}
	if(wCnMode==GPRS_MODE_IDLE || wCnMode==GPRS_MODE_PWROFF || wCnMode==GPRS_MODE_SMS)
	{
		DTRACE(DB_FAPROTO, ("CGprsWorker::ResetModem: wCnMode = %d\r\n", wCnMode));
		return IF_RST_OK;
	}
	
    if (m_pModem->GetGPS(m_pWorkerPara->diGPS))
	{	
	}
	
	if (m_pWorkerPara->ptNetInfo != NULL)
	{
		m_pModem->GetNetInfo(m_pWorkerPara->ptNetInfo);
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CGprsWorker::ResetGprs: m_pWorkerPara->ptNetInfo == NULL.\r\n"));
	}
	
	if (!m_pModem->GetSYSINFO())
		if (!m_pModem->GetSYSINFO())
			m_pModem->GetSYSINFO();		//提前获取制式，用于选择用户名后缀

	if (!m_pWorkerPara->fEmbedProtocol)//模块协议栈的话就不跟别的地方共用串口了
	{		
		if (m_pWorkerPara->fEnMux)	//允许串口复用，在这个时候打开串口复用功能
		{
			if (!m_pModem->EnMux())
				return false;

			if (m_Comm.IsOpen())	//关闭原来的真实串口
			{
				DTRACE(DB_FAPROTO, ("CGprsWorker::ResetModem : close real comm for ppp\n"));
				m_Comm.Close();	
				Sleep(100);
			}
			
			m_Comm.Open(17, CBR_115200, 8, ONESTOPBIT, NOPARITY);	
			m_dwMuxClick = GetClick();
						//重新打开17号串口，其实它还是对应原来真实的串口，
						//但是在CComm中对17号串口的打开做特殊处理，设置允许串口复用
			//m_pModem->GetSYSINFO();
		}
		else	//不允许串口复用，串口必须关闭给PPP用
		{
			if (m_Comm.IsOpen())
			{
				DTRACE(DB_FAPROTO, ("CGprsWorker::ResetModem : close comm for ppp\n"));
				m_Comm.Close();	
				Sleep(100);
			}	
		}
	}		
	return IF_RST_OK;
}

//描述：复位操作
//返回: IF_RST_OK复位成功,IF_RST_HARDFAIL硬复位失败
//		IF_RST_SOFTFAIL软复位失败(协议层)
int CGprsWorker::ResetGprs(WORD wCnMode)
{
#ifdef SYS_WIN
	return IF_RST_OK;
#endif //SYS_WIN

	WORD wTryCnt = 0;
	int iModemRet = MODEM_NO_ERROR;
	
again:
	ClosePpp();	//如果PPP打开的,先关闭PPP
	
	if (wCnMode==GPRS_MODE_IDLE || wCnMode==GPRS_MODE_PWROFF)	//可能别的接口要使用串口,这里先关闭,让出串口
	{
		
		if (m_pWorkerPara->fEmbedProtocol)//模块协议栈的话就不跟别的地方共用串口了
		{
			if (wCnMode == GPRS_MODE_PWROFF)
			{
				m_pModem->PowerOff();
				m_fModemPwrOn = false;
				//Sleep(5000);
			}
			return IF_RST_OK;	
		}
			
		if (m_pWorkerPara->ModemPara.fEnSms)
		{
			DTRACE(DB_FAPROTO,("ResetGprs::wCnMode=%d, ModerPara.fEnSms=true!!\r\n", wCnMode));
			ResetModem(m_fModemPwrOn!=true, wCnMode);
		}
		else if (m_Comm.IsOpen())
		{	
			DTRACE(DB_FAPROTO, ("CGprsWorker::ResetGPRS: close comm in idle\n"));
			m_Comm.Close();
		}

		if (wCnMode == GPRS_MODE_PWROFF)
		{
			m_pModem->PowerOff();
			m_fModemPwrOn = false;
			//Sleep(5000);
		}
		
		return IF_RST_OK;
	}

	int nRet = ResetModem(m_fModemPwrOn!=true, wCnMode);
	if (nRet != IF_RST_OK)
		return IF_RST_HARDFAIL;
	
	m_dwSignClick = m_dwSmsOverflowClick = GetClick();

	if (wCnMode == GPRS_MODE_SOCKET) 	//基于TCP/IP的通信模式
	{		
		if(OpenPpp())
		{
			return IF_RST_OK;
		}
		else
		{
			return IF_RST_SOFTFAIL;
		}
	}
	else if (wCnMode == GPRS_MODE_SMS) 	 //短信
	{
		///////////////////////////////////
		//return IF_RST_OK;//xzz：个人感觉直接返回也可以完成，之前的resetmodem已经复位过短信了，不需要再复位了
		///////////////////////////////////
		if (m_pWorkerPara->fEnMux)
			return IF_RST_OK;
		
		if (m_pModem->ResetSMS() == true)
			return IF_RST_OK;
		else
			return IF_RST_SOFTFAIL;
	}
	else if (wCnMode == GPRS_MODE_CMD)
	{
		return IF_RST_OK;
	}
	else
	{
		return IF_RST_SOFTFAIL;
	}
	
	wTryCnt++;
	if (wTryCnt >= 2)
		return IF_RST_SOFTFAIL;
		 
	goto again;
}

void CGprsWorker::ResetToOfflineState()
{
	//DTRACE(DB_FAPROTO, ("CGprsWorker::ResetToOfflineState: @@@@@ m_wDormanCnt=%d, wDormanToPwroffCnt=%d,m_wFailCnt=%d\n",
	//				m_wDormanCnt, m_pWorkerPara->wDormanToPwroffCnt, m_wFailCnt));
	
	WORD i;
	
	if (m_pWorkerPara->wRstNum == 0)
		m_pWorkerPara->wRstNum = 2;
	
	if (m_pWorkerPara->wDormanToPwroffCnt!=0 && //允许给模块掉电
		m_wDormanCnt>=m_pWorkerPara->wDormanToPwroffCnt && //用户连续多次失败导致进入休眠状态
		m_wFailCnt>=m_pWorkerPara->wRstNum)	//连续两次上网上不去
	{	//即由于上网失败多次导致进入休眠状态,需要关模块电源
		DTRACE(DB_FAPROTO, ("CGprsWorker::ResetToOfflineState: @@@@@ powrer off to idle state\n"));
		ResetGprs(GPRS_MODE_PWROFF); //关模块电源
				//ResetModem()中也可能会对模块关电源
				//但ResetModem()中只要AT命令测试OK就不会关模块电源
				//这里关模块电源的标准是PPP拨号不成功
		m_wState = GPRS_STATE_IDLE;
		m_wGprsWorkStep = GPRS_STEP_IDLE;
		
		m_wFailCnt = 0;		//上网失败计数
		m_wDormanCnt = 0;	//主用户请求进入休眠状态计数
		
		//如果m_dwRstOnSmsFlag不等于0的话,在DoIdleState()中还是会
		//再次调用ResetToOfflineState()切换到短信状态
	}
	else if (m_dwRstOnSmsFlag)
	{
		DTRACE(DB_FAPROTO, ("CGprsWorker::ResetToOfflineState: to sms state \n"));

		for (i=0; i<m_pWorkerPara->wRstNum; i++)
		{
			if (ResetGprs(GPRS_MODE_SMS) == IF_RST_OK)
				break;
		}

		m_wState = GPRS_STATE_SMS;
		m_wGprsWorkStep = GPRS_STEP_SMS;
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CGprsWorker::ResetToOfflineState: to idle state \n"));
		ResetGprs(GPRS_MODE_IDLE);

		m_wState = GPRS_STATE_IDLE;
		m_wGprsWorkStep = GPRS_STEP_IDLE;
	}
}

void CGprsWorker::ResetToOnlineState()
{
	DTRACE(DB_FAPROTO, ("CGprsWorker::ResetToOnlineState: open ppp \n"));

	WORD i;
	if (m_pWorkerPara->wRstNum == 0)
		m_pWorkerPara->wRstNum = 2;
		
	for (i=0; i<m_pWorkerPara->wRstNum; i++)
	{
		if (ResetGprs(GPRS_MODE_SOCKET) == IF_RST_OK)
			break;

		//Sleep(10*1000);
	}

	if (i < m_pWorkerPara->wRstNum)
	{
		m_wState = GPRS_STATE_OL;
		m_wGprsWorkStep = GPRS_STEP_ONLINE;
		
		m_wFailCnt = 0;		//上网失败计数
		m_wDormanCnt = 0;	//主用户请求进入休眠状态计数
	}
	else
	{
		m_wFailCnt++;
		WaitSemaphore(m_semWorker);	 
		m_dwOnlineReqFlg = 0;	//在连续尝试2次后,还是拨不上网
								//清除所有上网请求,如果应用还要
								//继续尝试,由应用本身控制
		SignalSemaphore(m_semWorker);
		
		ResetToOfflineState();
	}
}

void CGprsWorker::DoIdleState()
{
	if (m_dwOnlineReqFlg)	//对应位为1表示该用户要上线
	{
		ResetToOnlineState();
	}
	else if (m_dwRstOnSmsFlag)	
	{
		ResetToOfflineState();	//切换到短信状态
	}	
	else
	{
		
		DWORD dwClick = GetClick();
		if (dwClick-m_dwDebugClick > GPRS_DEBUG_INTERV)
		{
			m_dwDebugClick = dwClick;
			DTRACE(DB_FAPROTO, ("CGprsWorker::DoIdleState in idle state\n"));
		}
	}
}

void CGprsWorker::DoOnlineState()
{
	//进到这里就表示当前已经处于在线状态,网络是连接的

	if (m_dwOnlineReqFlg)	//对应位为1表示该用户要上线
	{

		DWORD dwClick = GetClick();
		if (dwClick-m_dwDebugClick > GPRS_DEBUG_INTERV)
		{
			m_dwDebugClick = dwClick;
			DTRACE(DB_FAPROTO, ("CGprsWorker::DoOnlineState: in online state\n"));
		}
	}
	else
	{
		ResetToOfflineState();
	}
}

CComm m_CommCmd;
void CGprsWorker::DoSmsState()
{
	if (m_dwOnlineReqFlg)	//对应位为1表示该用户要上线
	{
		ResetToOnlineState();
	}
	else if (m_dwRstOnSmsFlag == 0)
	{
		ResetToOfflineState();	//切换到空闲状态
	}	
	else
	{
		DWORD dwClick = GetClick();

#ifndef SYS_WIN
		BYTE bBuf[512];
		
		/*if (m_pModem->CheckActive())
		{
			SetInfo(m_pWorkerPara->wInfoActive); //INFO_ACTIVE
		}
		
		if(m_dwMuxClick!=0)
		{
			if((GetClick() - m_dwMuxClick) < 30)
				return;
			
			m_dwMuxClick = GetClick();
				
			if ((m_CommCmd.Open(19, CBR_115200, 8, ONESTOPBIT, NOPARITY) == false)) //!m_fModemComOpen || 
			{
				DTRACE(DB_FAPROTO, ("CGprsWorker::DoOnlineSms: fail to Open Comm\n"));
				return;	
			}
			
			m_pModem->SetComm(&m_CommCmd);
		}*/
		
		
		//接收短信
		WORD wLen = m_pModem->ReceiveSms(bBuf, 512, true);
		if (wLen > 0)
		{
			TraceFrm("--> CGprsWorker::DoSmsState: rx sms ", bBuf, wLen);	
			if (!m_SmsRxFrmQue.Append(bBuf, wLen, 2000))
			{
				DTRACE(DB_FAPROTO, ("CGprsWorker::DoSmsState: fail to append sms\n"));
			}
		}
		
		//发送短信
		wLen = m_SmsTxFrmQue.Remove(bBuf, 100);
		if (wLen > 0)
		{
			TraceFrm("<-- CGprsWorker::DoSmsState: tx sms ", bBuf, wLen);
			m_pModem->SendSms(bBuf, wLen);
		}
		
		//场强检测
		if (dwClick-m_dwSignClick > 1*60)
		{
			m_dwSignClick = dwClick;
			if (!UpdateSignStrength())
			{	//没有场强直接复位模块
				ResetGprs(GPRS_MODE_SMS);
			}
		}
		
		//短信溢出检测		
		if (dwClick-m_dwSmsOverflowClick > 2*60)
		{
			m_dwSmsOverflowClick = dwClick;
			m_pModem->AvoidSmsOverflow();	//查看是否需要删除短信，避免短信溢出
		}
#endif //SYS_WIN

		if (dwClick-m_dwDebugClick > GPRS_DEBUG_INTERV)
		{
			m_dwDebugClick = dwClick;
			DTRACE(DB_FAPROTO, ("CGprsWorker::DoSmsState in sms state\n"));
		}
		
		//if(m_dwMuxClick!=0)
		//{
		//	m_CommCmd.Close();
		//	m_pModem->SetComm(&m_Comm);
		//}
	
	}
}


bool CGprsWorker::DoOnlineSms()
{
	//static bool fFirst = true;
   if(!m_pWorkerPara->ModemPara.fEnSms)
   		return false;//不需要短信功能时，不要开启这个函数
#ifndef SYS_WIN
	BYTE bBuf[512];
	//接收短信
	if(m_dwMuxClick!=0)
	{
		if((GetClick() - m_dwMuxClick) < 30)
			return false;
		
		m_dwMuxClick = GetClick();
			
		if ((m_CommCmd.Open(19, CBR_115200, 8, ONESTOPBIT, NOPARITY) == false)) //!m_fModemComOpen || 
		{
			DTRACE(DB_FAPROTO, ("CGprsWorker::DoOnlineSms: fail to Open Comm\n"));
			return false;	
		}
		
		m_pModem->SetComm(&m_CommCmd);
	}
	else//使用真实串口读取短信
	{
		if(!m_Comm.IsOpen())
		{
			//真实串口没打开，则不去读取
			return false;
		}
		DTRACE(DB_FAPROTO, ("CGprsWorker::DoOnlineSms: GetSms by Really Comm.\r\n"));
	}
	
	
	WORD wLen = m_pModem->ReceiveSms(bBuf, 512, true);
	if (wLen > 0)
	{
		TraceFrm("--> CGprsWorker::DoSmsState: rx sms ", bBuf, wLen);	
		if (!m_SmsRxFrmQue.Append(bBuf, wLen, 2000))
		{
			DTRACE(DB_FAPROTO, ("CGprsWorker::DoSmsState: fail to append sms\n"));
		}
		else
			DTRACE(DB_FAPROTO, ("CGprsWorker::m_SmsRxFrmQue Append a sms OK\n"));
	}

	//发送短信
	wLen = m_SmsTxFrmQue.Remove(bBuf, 100);
	if (wLen > 0)
	{
		TraceFrm("<-- CGprsWorker::DoSmsState: tx sms ", bBuf, wLen);
		m_pModem->SendSms(bBuf, wLen);
	}
	if(m_dwMuxClick!=0)
	{
		m_CommCmd.Close();
	}
	m_pModem->SetComm(&m_Comm);
	
#endif
	return true;
}

bool CGprsWorker::SendTextSms(BYTE* pbSmsAddr, BYTE bSmsAddrLen, BYTE* pbTxBuf, WORD wLen)
{
	bool fRet = false;
	
	WaitSemaphore(m_semWorkerLocker);
	
	fRet = m_pModem->SendTextSms(pbSmsAddr, bSmsAddrLen, pbTxBuf, wLen);
	
	SignalSemaphore(m_semWorkerLocker);
	
	return fRet;
}

//const BYTE b1800Pwr[16] = {30, 28, 26, 24, 22, 20, 18, 16, 14, 12, 10, 8, 6, 4, 2, 0};
//const BYTE b900Pwr[20] = {0, 0, 0, 0, 0, 33, 31, 29, 27, 25, 23, 21, 19, 17, 15, 13, 11, 9, 7, 5};
bool CGprsWorker::ReadTxPwrAndSign(BYTE* pbTxPwr, int16* pbSign)
{	
	CComm comGSM;
	char* p;
	WORD i,j;
	DWORD dwArfcn = 0;
	bool fRet = false;
	BYTE bBuf[128];
	
	if (comGSM.Open(19, CBR_115200, 8, ONESTOPBIT, NOPARITY) == false)
	{
		DTRACE(DB_FAPROTO, ("ReadTxPwrAndSign::ReadTxPwrAndSign : fail to open GSM mux comm\r\n"));
		return false;
	}	
	char* pszPwrCmd = "AT$MYCGED\r\n";
	
	p = pszPwrCmd;

	
	for (i=0; i<2; i++)
	{
		DTRACE(DB_FAPROTO, ("ReadTxPwrAndSign::ReadTxPwrAndSign : tx %s\r\n", p));
		comGSM.Write((BYTE* )p, strlen(p));
		Sleep(500);
		int len = comGSM.Read(bBuf, sizeof(bBuf));
		if (len > 0)
		{
			bBuf[len] = 0;	
			DTRACE(DB_FAPROTO, ("ReadTxPwrAndSign::ReadTxPwrAndSign : rx %s\r\n", bBuf));
			
			if (strstr((char* )bBuf, "OK") != NULL)   //接收到正确回答
			{
				char* exp = strstr((char*)bBuf, ",");
				if(exp != NULL)
				{
					if(*(exp+1) == '-')
					{
						*pbSign = 0 - SearchStrVal(exp+2, (char* )&bBuf[len]);
						//m_wSignStrength = ((113 - *pbSign) + 1) / 2;
						//*pbSign = ~(*pbSign - 1);
					}
					else
					{
						//m_wSignStrength = 31;
						*pbSign = SearchStrVal(exp+1, (char* )&bBuf[len]);
					}
					/*if (m_wSignStrength > 31)//首先不是负数
					{
						if(m_wSignStrength == 99)
							m_wSignStrength = 0;
						else
							m_wSignStrength = 31;
					}*/
				}
				exp++;
				exp = strstr(exp, ",");
				if(exp != NULL)
				{
					if(*(exp+1) == '-')
					{
						*pbTxPwr = ~(SearchStrVal(exp+2, (char* )&bBuf[len])-1);
					}
					else
					{
						*pbTxPwr = SearchStrVal(exp+1, (char* )&bBuf[len]);
					}
				}

				fRet = true;
				break;
			}
			else
			{
				fRet = false;
			}
		}
		else
		{
			DTRACE(DB_FAPROTO, ("ReadTxPwrAndSign::ReadTxPwrAndSign : rx no ans\r\n", bBuf));
		}
		
		Sleep(500);
	}
	
	char* pszCSQ = "AT+CSQ\r\n";
    comGSM.Write(pszCSQ, strlen(pszCSQ));
	Sleep(500);  //避免Read接收到以前的通信数据而立即返回
	int len = comGSM.Read(bBuf, 120);
	bBuf[len] = 0;
	DTRACE(DB_FAPROTO, ("CFaProto::UpdateSignStrength : AT+CSQ rx %s.\r\n", bBuf));

	//+CSQ:
	p = strstr((char* )bBuf, "+CSQ:");
	if (len<9 || p==NULL)
	{
		;
	}
	else
	{
		m_wSignStrength = SearchStrVal(p+5, (char* )&bBuf[len]);
	}

	comGSM.Close();
	
	return fRet;
}

//描述：更新发射功率到系统库
void CGprsWorker::UpdTxPwr()
{
	/*BYTE bBuf[48];
	ReadBankId(BN0, PN0, 0x230f, bBuf);
	char *p = strstr((char* )bBuf, "M590");
	char *p1 = strstr((char* )bBuf, "N710");*/
	if (m_pWorkerPara->wUpdTxPwrInterv!=0 &&	//更新发射功率的间隔，单位秒，为0表示不更新
		m_pWorkerPara->fEnMux)// && (p!=NULL || p1!=NULL))		//允许串口复用
	{
		if (m_dwUpdTxPwrClick==0 || 
			GetClick()-m_dwUpdTxPwrClick >= m_pWorkerPara->wUpdTxPwrInterv)
		{
			//友方模块做此功能
			BYTE bTxPwr = 0;
			int16 bSign  = 0;
			//DTRACE(DB_FAPROTO, ("UpdTxPwr:: Modem is NEOWAY\r\n"));
			if (ReadTxPwrAndSign(&bTxPwr, &bSign)) //读发射功率成功
			{
				UpdateTxPwr(bTxPwr, bSign);	//调用外部的挂钩函数
			}
			m_dwUpdTxPwrClick = GetClick();
		}
	}
}

void CGprsWorker::UpdSysInfo()
{
	if (!m_pWorkerPara->fEnMux)
		return;
	DWORD dwClick = GetClick();
	if (GetClick()-m_dwUpdSysInfoClick >= 3*60)
	{
		m_dwUpdSysInfoClick = GetClick();
		if ((m_CommCmd.Open(19, CBR_115200, 8, ONESTOPBIT, NOPARITY) == false)) //!m_fModemComOpen || 
		{
			DTRACE(DB_FAPROTO, ("CGprsWorker::UpdSysInfo: fail to Open Comm!\r\n"));
			return;	
		}
		m_pModem->SetComm(&m_CommCmd);
		m_pModem->GetSYSINFO();
		m_CommCmd.Close();
		m_pModem->SetComm(&m_Comm);
		//m_pModem->UpdateMyTime();
	}
}

/*bool CGprsWorker::UpdSysInfo()
{
#ifndef SYS_WIN
	BYTE bBuf[128];

	//接收AT
	if(m_dwMuxClick!=0)
	{
		if((GetClick() - m_dwMuxClick) < 30)
			return false;
		m_dwMuxClick = GetClick();
			
		if ((m_CommCmd.Open(19, CBR_115200, 8, ONESTOPBIT, NOPARITY) == false)) //!m_fModemComOpen || 
		{
			DTRACE(DB_FAPROTO, ("CGprsWorker::UpdSysInfo: fail to Open Comm\n"));
			return false;	
		}
		m_pModem->SetComm(&m_CommCmd);
	}
	else//使用真实串口读取短信
	{
		if(!m_Comm.IsOpen())
		{
			//真实串口没打开，则不去读取
			return false;
		}
		DTRACE(DB_FAPROTO, ("CGprsWorker::UpdSysInfo: GetSysInfo by Really Comm.\r\n"));
	}
	
	WORD wLen = m_pModem->RcvSYSINFO();
	
	if(m_dwMuxClick!=0)
	{
		m_CommCmd.Close();
	}
	m_pModem->SetComm(&m_Comm);
#endif
	return true;
}*/

/*bool CGprsWorker::SetNetInfo(BYTE bNetType)
{
	bool fRtn = false;

#ifndef SYS_WIN
	if (m_dwMuxClick == 0)
		return false;
	if ((m_CommCmd.Open(19, CBR_115200, 8, ONESTOPBIT, NOPARITY) == false)) //!m_fModemComOpen || 
	{
		DTRACE(DB_FAPROTO, ("CGprsWorker::SetNetInfo: fail to Open Comm.\r\n"));
		return false;	
	}
	m_pModem->SetComm(&m_CommCmd);

	fRtn = m_pModem->SetMYNETINFO(bNetType);

	m_CommCmd.Close();
	m_pModem->SetComm(&m_Comm);
#endif
	return fRtn;
}*/


void CGprsWorker::DoLed()
{
#ifdef SYS_LINUX
	if (m_bLedAct == SOCK_LED_LIGHT)	//0初值不动作，1表示点亮，2表示熄灭
	{
		SetSockLed(true);
	}
	else if (m_bLedAct == SOCK_LED_DARK)
	{
		SetSockLed(false);
	}
#endif //SYS_LINUX

	m_bLedAct = 0;
}
WORD CGprsWorker::GetErrBytes()
{ 
	if(m_pModem!=NULL)
		return m_pModem->GetErrBytes(); 
	else
		return 0;
}


//GPRS上线/下线的规则:
//1.使用者每次有新的上网需要,都要发起上网请求,使用完了后发起下线请求
//2.worker发现有新的上网请求,则进行拨号上网的尝试,连续重拨次数,如果都失败,则清除所有上网请求,
//  退回到短信或者空闲状态
//3.一般上线的连续重拨次数都比较小,2~3次,所以都外界来说都当成了一次上网;如果应用要实现重拨间隔
//  等功能的话,则在CGprsIf类里实现,因为工作者线程要为所有的使用者复位,不能为某个应用停滞一个长
//	时间间隔
void CGprsWorker::RunThread()
{
	DTRACE(DB_FAPROTO, ("CGprsWorker::RunThread: started!\n"));
	int iMonitorID = ReqThreadMonitorID("GprsWkr-thrd", 30*60);	//申请线程监控ID,更新间隔为2小时
	
	while (1)
	{
		WaitSemaphore(m_semWorkerLocker);
		UpdThreadRunClick(iMonitorID);
		
		LoadUnrstPara();
		if (m_dwMuxClick)
		{
			DoLed();
			UpdSysInfo();
		}
		switch (m_wState)
		{
			case GPRS_STATE_IDLE:
				DoIdleState();
				DoOnlineSms();
				break;
			case GPRS_STATE_OL:		//在线状态
				DoOnlineState();
				DoOnlineSms();
				UpdTxPwr(); //更新发射功率到系统库
				break;
			case GPRS_STATE_SMS:	//短信状态,包括短信激活和振铃激活
				DoSmsState();
				break;
		}
		
		SignalSemaphore(m_semWorkerLocker);
		Sleep(1000);
	}
	
	ReleaseThreadMonitorID(iMonitorID);
}

int CGprsWorker::GetGprsWorkLastErr()
{
	return m_iLastErr;
}

WORD CGprsWorker::GetGprsWorkStep()
{
	if (m_wGprsWorkStep == GPRS_STEP_APN)
	{
		if (m_pModem->GetModemStep() == MODEM_STEP_SIM)
			return GPRS_STEP_SIM;
		if (m_pModem->GetModemStep() == MODEM_STEP_REG)
			return GPRS_STEP_REG;
	}
	
#ifdef SYS_LINUX
	if (m_wGprsWorkStep == GPRS_STEP_PPP)
	{
		//if (!m_pWorkerPara->fDetailDiagnose)
		//	return m_wGprsWorkStep;
		//unsigned int step, error;
    	int f;
    	//char* steps[] = { "Serial connection established", "authentication succeeded", "local  IP address" };
    	//char* errors[] = { "no error", "authentication failed", "get ip failed" };
		
		unsigned int step, error;
    	f = open("/var/log/ppp_stats", O_RDONLY, S_IREAD|S_IWRITE);  //|O_BINARY ,|O_BINARY S_IREAD|S_IWRITE
		if (f >= 0)
		{
			lseek(f, 0, SEEK_SET);
			read(f, &step, sizeof(step));
			lseek(f, 4, SEEK_SET);
			read(f, &error, sizeof(error));
			close(f);
			//if (step>2 || error>2)
			//	printf("step %d, error %d.\r\n", step, error);
			//else
			//	printf("%s, %s\n", steps[step], errors[error]);
			if (step == 0)
				return GPRS_STEP_DIAL;
			else if (step == 1)
				return GPRS_STEP_AUTH;
			else if (step == 2)
				return GPRS_STEP_IP;
		}		
	}
#endif
	
	return m_wGprsWorkStep;
}

//描述:向工作者线程提出上线请求
int ReqOnline(int iGprsUser)
{
	DTRACE(DB_FAPROTO, ("ReqOnline : user=%d, flg=0x%04x......\r\n", iGprsUser, g_GprsWorker.GetOnlineReqFlg()));
	g_GprsWorker.ReqOnline(iGprsUser);
	
    DWORD dwClick = GetClick();
    do //for (WORD i=0; i<3*60; i++)
	{
		if (g_GprsWorker.IsOnline())
		{	
			DTRACE(DB_FAPROTO, ("ReqOnline : ok, user=%d\n", iGprsUser));
			return GPRS_ERR_OK;
		}
		
		if (!g_GprsWorker.IsKeepOnline())	//可能由于拨号失败,工作线程已经把所有的上网请求给
		{									//清除了,不用再等了	
			DTRACE(DB_FAPROTO, ("CGprsIf::ReqOnline : fail due to req flg clear by worker, user=%d\n", iGprsUser, g_GprsWorker.GetOnlineReqFlg()));
			return g_GprsWorker.GetLastErr();
		}
			
		Sleep(1000);
	} while(GetClick()-dwClick < 3*60); 

	DTRACE(DB_FAPROTO, ("CGprsIf::ReqOnline : fail, user=%d, flg=0x%04x\r\n", iGprsUser, g_GprsWorker.GetOnlineReqFlg()));
	return g_GprsWorker.GetLastErr();
}

bool ReqOffline(int iGprsUser)
{
	DTRACE(DB_FAPROTO, ("ReqOffline : user=%d, flg=0x%04x......\n", iGprsUser, g_GprsWorker.GetOnlineReqFlg()));
	g_GprsWorker.ReqOffline(iGprsUser);
	
	DWORD dwClick = GetClick();
    do  //for (WORD i=0; i<2*60; i++)
    {
		Sleep(1000);

		if (g_GprsWorker.IsKeepOnline()) //在ReqOffline()时,已经把本身m_dwOnlineReqFlg中的对应位给
		{								 //清除了,但别的用户还要求保持在线,不用再等了
			DTRACE(DB_FAPROTO, ("ReqOffline : fail due to others keep online, user=%d, flg=0x%04x \r\n", iGprsUser, g_GprsWorker.GetOnlineReqFlg()));
			return false;
		}

		if (g_GprsWorker.IsOnline() == false)
		{
			DTRACE(DB_FAPROTO, ("ReqOffline : ok, user=%d\r\n", iGprsUser));
			return true;
		}
	} while(GetClick()-dwClick < 2*60); 

	DTRACE(DB_FAPROTO, ("ReqOffline : fail, user=%d, flg=0x%04x \r\n", iGprsUser, g_GprsWorker.GetOnlineReqFlg()));
	return false;
}

bool ReqDorman(int iUser)
{
	g_GprsWorker.ReqDorman(iUser);
	return ReqOffline(iUser);
}

WORD GetSignStrength()
{
	return g_GprsWorker.SignStrength();
}

//描述:GPRS工作者线程
TThreadRet GprsWorkerThread(void* pvArg)
{
	g_GprsWorker.RunThread();
	return THREAD_RET_OK;
}

int GetGprsWorkLastErr()
{
	return g_GprsWorker.GetGprsWorkLastErr();	
}

WORD GetGprsWorkStep()
{
	return g_GprsWorker.GetGprsWorkStep();	
}

CGprsWorker* GetGprsWorker()
{
	return &g_GprsWorker;
}

void GprsWorkerSetSockLed(BYTE bAct)
{
	g_GprsWorker.SetSockLedAct(bAct);
}