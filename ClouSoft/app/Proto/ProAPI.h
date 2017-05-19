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

#define SOCK_TYPE_ETH	0	//socket����Ϊ��̫��
#define SOCK_TYPE_GPRS	1	//socket����ΪGPRS

#define PRO_SVR_MAX		3

//GPRS����ģʽ�� 0:���ģʽ��1���ͻ���ģʽ��2��������ģʽ
#define GPRS_MIX_MODE	0	
#define GPRS_CLI_MODE	1
#define GPRS_SER_MODE	2

//���ӷ�ʽ
#define LK_TCP			0	//TCP
#define LK_UDP			1	//UDP

//����Ӧ�÷�ʽ
#define MAST_SUB_CONN_TYPE	0	//�������ӷ�ʽ
#define MUL_CONN_TYPE		1	//�����ӷ�ʽ

typedef struct{
	char *pszName;

	DWORD 	dwLocalIP;
	WORD 	wLocalPort;

	WORD		wConnectNum;	//֧�ֵ�������
	CProto*		pProto[PRO_SVR_MAX];		//ͨ��Э��ʵ��������
	CSocketIf*	pSocketIf[PRO_SVR_MAX];		//socket�ӿ�ʵ��������
}TSockSvrPara;

//TSockSvrPara��ʹ��˵��:
//1.pProto��pSocketIf�ֱ�ָ��ͨ��Э���socket�ӿ�ʵ������,������ʹ�õ�ʱ��˳��һһ���ʹ��,
//  ����Ϊ����������ǰ�Ѿ���ʼ�����໥�ҽ�,
//2.�����CSocketIf��m_fExitDone��Ϊtrue(�ɹ��˳�)
//3.�����TSocketPara��fSvr��Ϊtrue(������ģʽ)

extern CFaProto g_pEthFaProto; 
extern CProtoIf *g_pEthFaProtoIf;
extern CFaProto g_pGprsFaProto; 
extern CProtoIf *g_pGprsFaProtoIf;

//��Э������ȫ�ֱ���
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
extern bool g_fSvrDiscon;	//��������ʱ�Ͽ�����
extern bool g_fCliIdle;	//�ͻ��˴��ڿ���״̬ 

//extern bool InitFaProto(BYTE bComPort);
bool InitFaProto();
bool InitLocalProto();
bool InitProto();
void* FaProtoThread(void* pvPara);


//����ʾ�Ľӿ�
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

//ȫ�ֶ���

#endif //FAPAPI_H


