/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：PotoIf.cpp
 * 摘    要：本文件实现了通信接口基类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：接口的状态切换: (休眠)->(复位)->(连接)->(登录)->(传输)
 *          各状态下错误计数的清零规则:各状态的错误计数由各状态自己控制,某状态不清零别的状态的计数
 *********************************************************************************************************/
#include "stdafx.h"
#include "ProtoIf.h"
#include "FaCfg.h"
#include "FaConst.h"
#include "Info.h"
#include "Trace.h"
#include "sysapi.h"
#include "ProIfConst.h"

////////////////////////////////////////////////////////////////////////////////////////////
//CProtoIf

CProtoIf::CProtoIf()
{
	m_fRstInConnectFail = false;	//在连接失败到重试次数后不复位接口
	m_wState = IF_STATE_RST;
	m_wIfType = IF_UNKNOWN;
	m_dwDebugClick = 0;
	m_iLastErr = 0;
	m_dwRstIfClick = 0;		 //记录接口上次复位时刻或者接收到报文的时刻
}

CProtoIf::~CProtoIf()
{

}


bool CProtoIf::Init(TIfPara* pIfPara)
{
	m_pIfPara = pIfPara;
	
	m_fExit = false;
	m_fExitDone = false;
	m_fUnrstParaChg = false;  	//非复位参数发生改变
	m_fNeedActive = false;		//需要激活	
	m_wRunCnt = 0;
	m_dwDormanClick = 0;	    //进入休眠的开始时间
	
	//心跳相关
	m_dwRxClick = 0;
	m_dwBeatClick = 0;
	
	//失败计数数
	m_wResetFailCnt = 0;		 
	m_wConnectFailCnt = 0; 
	m_wLoginFailCnt = 0;

	m_fDisConnCmd = false;			//收到外部的断开连接命令
	m_fSetIdleCmd = false;			//收到外部的处于空闲状态的命令
	m_dwDormanInterv = 0;		//临时设定的休眠间隔，单位秒
	m_wDormanState = IF_STATE_DORMAN; //暂时休眠的状态，休眠完还要转回到该状态

	DTRACE(DB_FAPROTO, ("CProtoIf::Init: if(%s) init to fNeedLogin=%d, wMaxFrmBytes=%d, dwRstInterv=%ld, dwConnectInterv=%ld, wLoginRstNum=%ld, dwLoginInterv=%ld, wRstNum=%d, wReSendNum=%d, wReTryNum=%d, dwDormanInterv=%ld\n", 
						GetName(),
						m_pIfPara->fNeedLogin,
						m_pIfPara->wMaxFrmBytes, 		
						m_pIfPara->dwRstInterv,		
						m_pIfPara->dwConnectInterv,	
						m_pIfPara->wLoginRstNum, 	
						m_pIfPara->dwLoginInterv, 	
						m_pIfPara->wRstNum,           
						m_pIfPara->wReSendNum,		
						m_pIfPara->wReTryNum,
						m_pIfPara->dwDormanInterv));

	return true;
}

char* CProtoIf::GetName()
{
	char* pszName = "unknown";
	
	if (m_pIfPara->pszName != NULL)
		pszName = m_pIfPara->pszName;
	
	return pszName;
}
	
void CProtoIf::OnResetOK()
{
	//复位相关
	m_wState = IF_STATE_CONNECT;
	m_wResetFailCnt = 0;
	m_dwRstIfClick = GetClick(); //记录接口上次复位时刻或者接收到报文的时刻
}

void CProtoIf::OnResetFail()
{
	m_wResetFailCnt++;
	m_dwRstIfClick = GetClick(); //记录接口上次复位时刻或者接收到报文的时刻

	if (m_pIfPara->dwDormanInterv!=0 && m_wResetFailCnt>=m_pIfPara->wRstNum)
	{
		DTRACE(DB_FAPROTO, ("CProtoIf::OnResetFail: go to dorman\n"));
		m_wResetFailCnt = 0;
		EnterDorman();
	}
	else
	{
		Sleep(m_pIfPara->dwRstInterv * 1000);
	}	
}

//描述:在接口由断开转为连接的时候调用
void CProtoIf::OnConnectOK()
{
	if (IsNeedLogin())
		m_wState = IF_STATE_LOGIN;
	else	
		m_wState = IF_STATE_TRANS;

	//心跳相关
	m_dwRxClick = GetClick();
	m_dwBeatClick = 0;
	
	//连接相关
	m_wConnectFailCnt = 0;
}

void CProtoIf::OnConnectFail()
{	
	//连接相关
	m_wConnectFailCnt++;
	
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
			Sleep(m_pIfPara->dwConnectInterv * 1000); //接口的连接间隔,单位秒
			if (m_fRstInConnectFail)	//本接口在连接失败到重试次数后复位接口
				m_wState = IF_STATE_RST; 
		}
	}
	else
	{
		Sleep(m_pIfPara->dwConnectInterv * 1000); //接口的连接间隔,单位秒
	}	
}


//描述:在接口由连接转为断开的时候调用，不管是主动断开还是被动断开
bool CProtoIf::DisConnect()
{
	if (m_wState > IF_STATE_CONNECT)
		m_wState = IF_STATE_CONNECT;
	
	//心跳相关
	//m_dwRxClick = 0;	//不能清m_dwRxClick,不然正常运行m_pIfPara->dwNoRxRstAppInterv秒后,
						//只要有一次没有联网成功就会引起终端复位
	m_dwBeatClick = 0;
	
	//连接相关
	//m_wConnectFailCnt = 0; //断开连接不需要复位失败计数,因为有在socket
							 //方式下,每次连接前都主动地尝试断开之前的
							 //连接,清零会使连接失败计数失效,本计数只有
							 //在接口复位成功或者累计到重试次数后才清零
	return true;
}

//描述:在收到外部的断开连接命令时,调用本函数通知接口
//参数:@dwDormanInterv 动态设定的休眠间隔，单位秒
void CProtoIf::SetDisConnect(DWORD dwDormanInterv)
{
	m_fDisConnCmd = true;	//收到外部的断开连接命令
	m_dwDormanInterv = dwDormanInterv;	//临时设定的休眠间隔，单位秒
}

//描述:在收到外部的处于空闲状态的命令时,调用本函数通知接口
void CProtoIf::SetIdle()
{
	m_fSetIdleCmd = true;			//收到外部的处于空闲状态的命令
}

//使接口进入休眠方式
void CProtoIf::EnterDorman()
{
	DTRACE(DB_FAPROTO, ("CProtoIf::EnterDorman : enter dorman mode for %ldS \n", 
						m_dwDormanInterv!=0 ? m_dwDormanInterv : m_pIfPara->dwDormanInterv));

	DisConnect();
	m_wState = IF_STATE_DORMAN;
	m_dwDormanClick = GetClick();	 //进入休眠的开始时间
}    		

//描述:进入休眠模式,每次睡5秒,并判断休眠时间到了没有
void CProtoIf::DoDorman()
{
	Sleep(5000);
	DWORD dwClick = GetClick();
	DWORD dwDormanInterv;

	if (m_dwDormanClick != 0)
	{
		if (m_dwDormanInterv != 0)	//临时设定的休眠间隔，单位秒
			dwDormanInterv = m_dwDormanInterv;
		else
			dwDormanInterv = m_pIfPara->dwDormanInterv;

		if (dwClick-m_dwDormanClick < dwDormanInterv)
		{
			DTRACE(DB_FAPROTO, ("CProtoIf::DoDorman: remain %ld\n", dwDormanInterv + m_dwDormanClick - dwClick));
		}
		else
		{
			if (m_wDormanState != IF_STATE_DORMAN) //暂时休眠的状态，休眠完还要转回到该状态
			{
				m_wState = m_wDormanState;
				DTRACE(DB_FAPROTO, ("CProtoIf::DoDorman: wake up to %d\n", m_wState));
			}
			else
			{
				m_wState = IF_STATE_RST;
				DTRACE(DB_FAPROTO, ("CProtoIf::DoDorman: wake up to rst\n"));
			}

			m_dwDormanClick = 0;
			m_dwDormanInterv = 0;	//临时设定的休眠间隔，单位秒
			m_wDormanState = IF_STATE_DORMAN;
		}		
	}
	else //m_dwDormanClick==0 
	{	 //没有规定休眠的时间,相当于处于无限期的休眠(空闲)状态,
		 //要退出这种状态,就要靠在DoIfRelated()中根据接口相关的
		 //情况把m_wState状态改变,比如从不在线时段切换到在线时段
		 //接口的插入等情况
		if (dwClick-m_dwDebugClick > IF_DEBUG_INTERV)
		{
			m_dwDebugClick = dwClick;
			DTRACE(DB_FAPROTO, ("CProtoIf::DoDorman: if(%s)in idle mode\n", GetName()));
		}
	}
}	

DWORD CProtoIf::GetWakeUpTime()
{
	return m_pIfPara->dwDormanInterv + m_dwDormanClick - GetClick();
}

//描述:在协议登陆成功时调用
void CProtoIf::OnLoginOK()
{ 
	m_wState = IF_STATE_TRANS;
	m_wLoginFailCnt = 0; 
	m_iLastErr = GPRS_ERR_OK;
}

//描述:在协议登陆失败时调用,比如多少次失败后断开连接
void CProtoIf::OnLoginFail() 
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
		Sleep(m_pIfPara->dwLoginInterv*1000); //登录间隔
	}
	else if (m_wLoginFailCnt >= wLoginRstNum*wLoginNum)
	{					
		m_wLoginFailCnt = 0;
		DTRACE(DB_FAPROTO, ("CProtoIf::OnLoginFail: go to dorman\n"));
		EnterDorman();
	}
	else		//登录失败的次数达到了断开连接的次数的整数倍
	{			//m_wLoginFailCnt%wLoginRstNum==0 && m_wLoginFailCnt<wLoginRstNum*wLoginNum
		DisConnect();	//只断开连接,不清零登录失败计数,
						//达到wLoginRstNum*wReTryNum次后进入休眠状态
	}		
}

void CProtoIf::OnRcvFrm()
{
	m_dwRstIfClick = m_dwRxClick = GetClick();
}
  
void CProtoIf::AutoSend()
{
	if (m_pProto->m_pProPara->fAutoSend) //是否具有主动上送的功能
	{	
		m_pProto->AutoSend();
	}
}

//初始创建接口时初始化
void CProtoIf::InitRun()
{
	m_dwRstIfClick = GetClick();
}


//描述:接口相关操作函数,目前实现的操作有:
// 		1.超过多久没有通讯复位终端/模块
void CProtoIf::DoIfRelated()
{
	DWORD dwClick = GetClick();
	if (m_pIfPara->dwNoRxRstAppInterv != 0) //无接收复位终端间隔,单位秒,0表示不复位
	{
		if (dwClick-m_dwRxClick > m_pIfPara->dwNoRxRstAppInterv)
		{
			DTRACE(DB_FAPROTO, ("CProtoIf::DoIfRelated: if(%s) no rx reset app!\n", GetName()));
			SetInfo(m_pIfPara->wInfoAppRst);	//CPU复位 INFO_APP_RST
			return;
		}
	}

	if (m_pIfPara->dwNoRxRstIfInterv != 0) //无接收复位接口间隔,单位秒,0表示不复位
	{
		if (dwClick-m_dwRstIfClick > m_pIfPara->dwNoRxRstIfInterv)
		{
			DTRACE(DB_FAPROTO, ("CProtoIf::DoIfRelated: if(%s) no rx reset!\n", GetName()));
			DisConnect();
			m_wState = IF_STATE_RST; 
			m_dwRstIfClick = dwClick; //记录接口上次复位时刻或者接收到报文的时刻
			return;
		}
	}
}

//接口是否还处于传输状态
bool CProtoIf::CanTrans() 
{ 
	WORD wState = GetState();
	if (m_bGprsDataSrc == DATA_SRC_SMS)
		return true;
	else if (wState>IF_STATE_CONNECT && wState<=IF_STATE_TRANS)
		return true;

	return false;
}; 
