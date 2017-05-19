/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DrvAPI.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֱ�ϵͳ���豸��������ķ�װ
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2009��7��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
*********************************************************************************************************/
#ifndef DRVAPI_H
#define DRVAPI_H
#include "apptypedef.h"
#include <string>
#ifndef SYS_WIN
#include "Lcd.h"
extern CLcd* g_pLcd;

#endif
using namespace std;
typedef struct{
	char AddRoute[50]; //���Ĭ����������
	char DelRoute[50]; //ɾ��Ĭ������
	char PingCmd[50];  //Ping����
}RouteCMD; //·������

typedef struct{
	BYTE bGateWay[4]; //Ĭ������
}DefaultGWPara;       //ping����

extern RouteCMD RouteCmd;
extern DefaultGWPara  DefGWPara; //Ĭ������

unsigned long GetDefaultGateWay();
string cmd_system(const char* vCmd);
int Ping();
int GetEthernetStat();
int IfReadListProc(char *target, BYTE* pBuf);
void InitRouteCmd();
void InitPingCMD();
bool IsAutoGetIP(); //�Ƿ��Զ���ȡIP
int GetNetStat() ;
int CheckDhcpStat();
int DoUdhcpc();
bool GetNetPara(BYTE bType,  BYTE* pbBuf);
int CheckPppoeStat();
bool OpenPppoe();
void AddDefaultGateWay();
void DelDefaultGateWay();
void SetExecPingCMD( bool fPing);
bool IsExecPingCMD();
void SetDownConnMode(BYTE bConnMode);
BYTE GetDownConnMode();
int GetNetMacbStat();
#endif

