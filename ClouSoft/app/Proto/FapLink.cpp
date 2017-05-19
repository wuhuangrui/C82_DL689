 /*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FapLink.cpp
 * 摘    要：本文件主要实现级联命令的主终端的控制流程
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年8月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 备注:1.g_FapLink与g_FaProto的交互方式:通过CFapLink的成员m_Queue与全局变量
 *		  g_Queue,把需要对方发送的报文加入到队列中去
 *		2.主终端与从终端的切换:当主站地址的改变导致主/从终端发生切换的时候,
 *		  级联的主终端线程void CFapLink::DoMasterThread()不用重启,因为程序中
 *		  根据IsMasterTerm()来决定什么事情改做,什么事情不该做
 *		3.级联参数的更新:由ReInit()动态刷新参数
*********************************************************************************************************/

#include "stdafx.h"
#include "FaCfg.h"
#include "FaAPI.h"
#include "Queue.h"
#include "FaProto.h"
#include "ProPara.h"

CQueue g_LinkDownQueue;       //转发给从终端的报文队列
CQueue g_LinkUpQueue;           //转发给主站的报文队列
bool g_fEnableSlaveReport = false;      //是否允许主动上报
DWORD g_dwSlaveAddr[FAP_LINK_SLAVE_NUM];  //从终端地址
/////////////////////////////////////////////////////////////////////////////////////////////////////
//CFapLink

//描述:按照级联通信流程,为有需要的从终端转发主动上报的报文
void LinkOneSlave(BYTE index,CFaProto* pProto)
{
	//根据应答帧，判断是否转发主站
	//循环主站和终端之间的报文发送，
	//直到超时或者终端应答无主动上报
	TFapMsg* pMsgMaster;
	//主配电终端，发送允许主动上报控制命令
	//pProto->MakeLinkFrm(0,g_dwSlaveAddr[index]);
	//接收应答帧
	int i=0;
	g_fEnableSlaveReport=true;
	while ( (i< 10)&&g_fEnableSlaveReport)
	{
		Sleep(1000);
		pProto->RcvFrm(); //接收到的一帧,并已经对其进行处理
		pMsgMaster = (TFapMsg* )(g_LinkDownQueue.Remove(1));   //看看是否有主站的回帧
		if (pMsgMaster != NULL)
		{
			DTRACE(DB_FAPROTO, ("CFapLink::LinkOneSlave : forward a frm\n"));
			pProto->Send(pMsgMaster->bTxBuf, pMsgMaster->wTxLen);  //把主站的回帧转发给从终端
			delete pMsgMaster;
		}
		i++;
	}
	DTRACE(DB_FAPROTO, ("CFapLink::LinkOneSlave : forward a frm,g_fEnableSlaveReport=%d\n",g_fEnableSlaveReport));
 	return;
}

//描述:中继那些主站到从终端的通信
void DoForward(CFaProto* pProto)
{
	TFapMsg* pMsgMaster;
	//循环3次从接收队列，接收主站的报文
	//将主站的报文转发给终端
	//接收终端的报文
	//将终端的报文放到发送队列，转发给主站
	for (int i=0;i<3;i++)
	{
		pMsgMaster = (TFapMsg* )(g_LinkDownQueue.Remove(1));   //看看是否有主站的回帧
		if (pMsgMaster != NULL)
		{
			DTRACE(DB_FAPROTO, ("CFapLink::DoForward : forward a frm\n"));
			pProto->Send(pMsgMaster->bTxBuf, pMsgMaster->wTxLen);  //把主站的回帧转发给从终端
			delete pMsgMaster;
		}
		pProto->RcvFrm(); //接收到的一帧,并已经对其进行处理
	}
	return ;
}

//描述:运行级联的主终端线程
TThreadRet DoMasterThread(void* pvArg)
{
	DWORD dwLinkClick[FAP_LINK_SLAVE_NUM];
	CFaProto* pProto = (CFaProto* )pvArg;
	CProtoIf* pIf = pProto->GetIf();
	memset((BYTE*) dwLinkClick, 0, sizeof(dwLinkClick));
	DTRACE(DB_FAPROTO, ("DoMasterThread : started!\n"));
	while (1)
	{		
		if (pIf->m_fExit) break;

		Sleep(1000);				
		if (GetInfo(INFO_LINK))	
		{
			LoadLinkTermPara();
		}

		if (IsMasterTerm() == false)	//从终端级联口 处理主终端的查询和上传数据给主终端
		{
			pProto->RcvFrm(); //接收到的一帧,并已经对其进行处理
			pIf->AutoSend();
			continue;
		}

		//主终端级联口
		DoForward(pProto); //中继那些主站到从终端的通信
		for (BYTE i=0; i<FAP_LINK_SLAVE_NUM; i++) //看从终端有无上报需求
		{
			if (g_dwSlaveAddr[i] == 0)	//无效地址
				continue;	//break;
			if (GetClick()-dwLinkClick[i] > g_wLinkInterv)
			{
				dwLinkClick[i] = GetClick();
				DTRACE(DB_FAPROTO, ("DoMasterThread : LinkOneSlave!\n"));
				LinkOneSlave(i,pProto); //按照级联通信流程,为有需要的从终端转发主动上报的报文
			}
		}
	}
	return 0;
}
