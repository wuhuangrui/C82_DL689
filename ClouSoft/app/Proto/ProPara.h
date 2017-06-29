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
#ifndef PROPARA_H
#define PROPARA_H

//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <netdb.h>
#include "CommIf.h"
#include "GprsIf.h"
#include "ProtoIf.h"
#include "SocketIf.h"
#include "R230mIf.h"
#include "P2PIf.h"
#include "FaAPI.h"
#include "FaProto.h"

#define ETH_IP_CFG_DHCP		0	//以太网动态获取ip
#define ETH_IP_CFG_STATIC	1	//以太网静态IP
#define ETH_IP_CFG_PPPoE	2	//以太网为PPPoE

extern bool g_fUpdateFirmware;

BYTE GetEthWorkMode();
BYTE GetEthLnkType();
BYTE GetEthLnkAppType();
WORD GetEthListenPort();
BYTE GetEthProxySerAddr(char *pAddr);
WORD GetEthProxySerPort();
BYTE GetEthTimeOut();
BYTE GetEthTryCnt();
DWORD GetEthBeat();
BYTE GetEthConfigType();
BYTE GetEthLocalIp(BYTE *pAddr);
BYTE GetEthNetMask(BYTE *pNetMsk);
BYTE GetEthGataway(BYTE *pbAddr);
BYTE GetEthPPPoEUserName(char *pszUserName);
BYTE GetEthPPPoEUserPwd(char *pszUserPwd);
BYTE GetEthMacAddr(BYTE *pbMac);

BYTE GetGprsWorkMode();
BYTE GetGprsInlineType();
BYTE GetGprsLnkType();
BYTE GetGprsAppLnkType();
WORD GetGprsListenPort();
BYTE GetGprsAPN(char *pszAPN);
BYTE GetGprsUserName(char *pszUserName);
BYTE GetGprsUserPwd(char *pszUserPwd);
BYTE GetGprsProxyAddr(BYTE *pbAddr);
WORD GetGprsProxyPort();
BYTE GetGprsTimeOut();
BYTE GetGprsTryCnt();
DWORD GetGprsBeat();
BYTE GetGprsSmsCenterNo(BYTE *pbSmsNo);
BYTE GetGprsSmsMastNo(BYTE *pMastNo);
BYTE GetGprsSmsDstNo(BYTE *pbDstNo);


void LoadSocketUnrstPara(TSocketPara* pPara, BYTE bSockType);
void LoadSocketPara(TSocketPara* pPara, BYTE bSockType);
void LoadSocketLocalPara();
bool LoadSocketChangePara(TSocketPara* pPara);
void LoadGprsPara(TGprsPara* pGprsPara, BYTE bSockType);
void LoadReportGprsPara(TGprsPara* pGprsPara);
void LoadLocalPara(TCommIfPara* pCommIfPara);
void LoadTestPara(TCommIfPara* pCommIfPara);
bool Load230mUnrstPara(TR230mIfPara* pR230mIfPara);
void Load230MPara(TR230mIfPara* pR230mIfPara);
void LoadGprsUserNamePara(TGprsPara* pGprsPara);
void LoadP2PPara(TP2PIfPara* pP2PIfPara);
void LoadGprsModePara(TGprsPara* pGprsPara);

void LoadSmsPara(TModemPara *pModemPara);
void LoadGprsChangePara(TGprsPara* pGprsPara);
void LoadFaProPara(TFaProPara* pGbPara);

bool LoadGprsUnrstPara(TGprsPara* pGprsPara);
bool LoadReportGprsUnrstPara(TGprsPara* pGprsPara);
bool LoadGprsWorkerUnrstPara(TGprsWorkerPara* pWorkerPara);
void LoadGprsWorkerPara(TGprsWorkerPara* pWorkerPara);

WORD PhoneToStr(BYTE* pbPhone, WORD wPhoneLen, char* pszStr);




//全局定义

#endif //FAPAPI_H


