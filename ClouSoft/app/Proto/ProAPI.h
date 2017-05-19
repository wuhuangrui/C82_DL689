/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProAPI.h
 * 摘    要：本文件主要包含FaProto目录下API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：
 *********************************************************************************************************/
#ifndef FAPAPI_H
#define FAPAPI_H

#include "FaProto.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "GprsIf.h"
#include "GR47.h"
#include "Wavecom.h"
#include "MC39.h"
#include "SocketIf.h"
#include "CommIf.h"
#include "Proto.h"
#include "ProtoIf.h"
#include "P2PIf.h"

#define SOCK_TYPE_ETH	0	//socket类型为以太网
#define SOCK_TYPE_GPRS	1	//socket类型为GPRS

#define PRO_SVR_MAX		3

//GPRS工作模式： 0:混合模式，1：客户端模式，2：服务器模式
#define GPRS_MIX_MODE	0	
#define GPRS_CLI_MODE	1
#define GPRS_SER_MODE	2

//连接方式
#define LK_TCP			0	//TCP
#define LK_UDP			1	//UDP

//连接应用方式
#define MAST_SUB_CONN_TYPE	0	//主备链接方式
#define MUL_CONN_TYPE		1	//多连接方式

typedef struct{
	char *pszName;

	DWORD 	dwLocalIP;
	WORD 	wLocalPort;

	WORD		wConnectNum;	//支持的连接数
	CProto*		pProto[PRO_SVR_MAX];		//通信协议实例的数组
	CSocketIf*	pSocketIf[PRO_SVR_MAX];		//socket接口实例的数组
}TSockSvrPara;

//TSockSvrPara的使用说明:
//1.pProto和pSocketIf分别指向通信协议和socket接口实例数组,它们在使用的时候按顺序一一配对使用,
//  在作为参数传进来前已经初始化且相互挂接,
//2.必须把CSocketIf的m_fExitDone置为true(成功退出)
//3.必须把TSocketPara的fSvr置为true(服务器模式)

extern CFaProto g_pEthFaProto; 
extern CProtoIf *g_pEthFaProtoIf;
extern CFaProto g_pGprsFaProto; 
extern CProtoIf *g_pGprsFaProtoIf;

//主协议的相关全局变量
//extern CFaProto g_FaProto[2];
//extern TFaProPara g_GbProPara[2];
extern CFaProto g_FaProto;
extern CFaProto g_FapTest;
extern TFaProPara g_GbProPara;
extern CFaProto g_FapLinkProto;
extern CFaProto g_FapLocal;
extern CCommIf g_CommIf;
extern TSocketPara g_SocketPara;
extern TSockSvrPara g_SockSvrPara;
extern DWORD g_dwSvrModeBeatTestTimes;

extern TGprsPara g_GprsPara;
extern bool g_fGPRSSvrOnline;
extern BYTE g_bGPRSMode;
extern DWORD g_dwGprsSvrAptClick[PRO_SVR_MAX];
extern DWORD g_dwEthSvrAptClick[PRO_SVR_MAX];
extern bool g_fSvrDiscon;	//服务器超时断开连接
extern bool g_fCliIdle;	//客户端处于空闲状态 

//extern bool InitFaProto(BYTE bComPort);
bool InitFaProto();
bool InitLocalProto();
bool InitProto();
void* FaProtoThread(void* pvPara);


//与显示的接口
inline bool IsGprsConnected()
{
	bool fRet = false;

	if (g_pEthFaProto.IsConnected())
		fRet = true;

	if (g_pGprsFaProto.IsConnected() && GetGprsWorker()->IsOnline())
		fRet = true;

	return fRet;
}
inline bool IsGPRSSvrLink()
{
	return g_fGPRSSvrOnline;
}

BYTE GetNetWorkMode();
void GprsDisConnect();
extern WORD readSignal(void);
TThreadRet LinkThread(void* pvPara);
TThreadRet StdProtoThread(void* pvArg);
TThreadRet StdProRptThread(void* pvArg);
TThreadRet SockSvrThread(void* pvArg);
TThreadRet DoMasterThread(void* pvArg);
TThreadRet CheckNetThread(void* pvArg);
TThreadRet EthernetSwitchThread(void* pvArg);
void NewFaProtoThread();
void NewFaUpdateThread();

//全局定义

#endif //FAPAPI_H


