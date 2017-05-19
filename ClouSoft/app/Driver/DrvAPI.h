/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DrvAPI.h
 * 摘    要：本文件主要实现本系统下设备驱动程序的封装
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2009年7月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
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
	char AddRoute[50]; //添加默认网关命令
	char DelRoute[50]; //删除默认网关
	char PingCmd[50];  //Ping命令
}RouteCMD; //路由命令

typedef struct{
	BYTE bGateWay[4]; //默认网关
}DefaultGWPara;       //ping命令

extern RouteCMD RouteCmd;
extern DefaultGWPara  DefGWPara; //默认网关

unsigned long GetDefaultGateWay();
string cmd_system(const char* vCmd);
int Ping();
int GetEthernetStat();
int IfReadListProc(char *target, BYTE* pBuf);
void InitRouteCmd();
void InitPingCMD();
bool IsAutoGetIP(); //是否自动获取IP
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

