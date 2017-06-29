/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ProAPI.h
 * ժ    Ҫ�����ļ���Ҫ����FaProtoĿ¼��API������ȫ�ֱ����Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע��
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

#define ETH_IP_CFG_DHCP		0	//��̫����̬��ȡip
#define ETH_IP_CFG_STATIC	1	//��̫����̬IP
#define ETH_IP_CFG_PPPoE	2	//��̫��ΪPPPoE

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




//ȫ�ֶ���

#endif //FAPAPI_H


