#include "stdafx.h"
#include <time.h>
#include "FaCfg.h"
#include "FaAPI.h"
#include "sysfs.h"
#include "sysarch.h"
#include <stdio.h>
#include <ctype.h> 
#include <stdlib.h>
#include <stdarg.h> 
#include <time.h> 
#include "ComStruct.h"
#include "drivers.h"
#include "YK.h"
#include "YX.h"
#include "DbAPI.h"
#include "ProAPI.h"
#include "MeterAPI.h"
#include "Trace.h"
#include "TaskDB.h"
#include "LoadCtrl.h"
#include "ThreadMonitor.h"
#include "ParaMgr.h"
#ifdef EN_CCT
#include "CctAPI.h"
#include "CctTaskManager.h"
#endif
#include "AcConst.h"
#include "DrvPara.h"
#include "DrvAPI.h"
#include "DrvCtrlAPI.h"
#include "MtrHook.h"
#include "MtrCtrl.h"
#include "DrvConst.h"
#ifndef SYS_WIN
#include "Sample.h"
#include "Pulse.h"
#include "ProPara.h"
#include "ctcheck.h"
#include "Esam.h"
#endif //SYS_WIN

#ifdef SYS_LINUX
#include "DCProc.h"
#include "DCSample.h"
#include "CalcPulse.h"
#include <signal.h>
#include "segment_debug.h"
#endif


#ifdef EN_CCT	//允许集抄功能
#include "CctStat.h"
#include "CctCtrl.h"
#endif

#include "StdReader.h"
#include "StatMgr.h"
#include "DbOIAPI.h"
#include "DL645Ext.h"
#include "SearchMeter.h"

#define YX_OFF_STATE	0		//分
#define YX_ON_STATE		1		//合

//#define OOP_TAITI_TEST	1		//老的不带ESAM台体测试才开放

CStatMgr g_StatMgr;
//CQueue g_Queue;     //协议线程的报文消息队列
int g_iRstCnt[] = {0, 0, 0}; //8010,8011,8012三个通道登录连续失败的次数
int g_iCurChannel = 0; //当前使用的主站地址 0-主通讯通道, 1-备用通道1, 2-备用通道2
DWORD g_dwTaskMaskClick = 0;
DWORD  g_dwWrOpClick = 0;
bool g_fCurveChange = false;
bool g_fCommTaskChg = false;
bool g_fAddressChg = false;
bool g_fMtrParaChg = false;
bool g_fGrpParaChg = false;
bool g_fCtrlGrpParaChg = false;	//控制总加组参数更改
CComm g_commYK;
CComm g_commLocal;
CComm g_commTest;
CComm g_commRs485[LOGIC_PORT_NUM];
CComm& g_commDebug = g_commTest;
//CTaskManager g_TaskManager;
#ifdef EN_CCT
CCctTaskManager g_CctTaskManager;
#endif

bool g_fTest232 = false;
BYTE g_bReadMtrStatus[POINT_NUM];//485表抄表状态:0正常,1抄表失败,2抄表恢复
BYTE g_b485PortStatus;//485抄表口状态:0正常,1故障发生,2故障恢复
bool g_fMtrFailHapFlg[POINT_NUM];
bool g_f485FailHapFlg;
bool g_fTestMode = false;
bool g_fInAdjSysClock = false;	//是否处于对时状态

WORD   g_wAdjUn = 2200;

TSem   g_semTask;
TSem   g_semExcTask;
TSem   g_semMeter485;
TSem   g_semFaProto;
TSem   g_semMeterLog;
TSem   g_semNetStat; //检查网线状态
TSem   g_semGateWay; //获取网关
TSem   g_semGetIP;
TSem   g_semClr645Data;		//645计量数据清0命令
TSem   g_semLcd;
TSem   g_semRWCtrlModl;
TSem   g_semBat;//20131212-3
TSem   g_semTermEvt;

#ifdef SYS_WIN32
	TSem   g_semClickPeriod;
	TSem   g_semClick;   
#endif //SYS_WIN32



WORD g_wAccessDeniedCnt = 0;
bool g_fRxTmpCtrl = false;

bool g_fBatPowerOff = false;	//主电源断电且电池电压<3.65V
bool g_fCTValid = false;
WORD g_wTcpipRunCnt = 0;
bool g_fFapNeedConnect = false;
bool g_fMasterTerm = false;
WORD g_wLinkInterv = 120;
BYTE g_bClrEnergyStep = 0;	//清电能状态机

DWORD g_dwExtCmdFlg = 0;
DWORD g_dwExtCmdClick = 0;
BYTE g_bDefaultCfgID = 0;
BYTE g_bEnergyClrPnt = 0;

bool g_fSearchMeter = false;
bool g_fClearMeterInf = false;
bool g_fClearHistory = false;
static DWORD g_dwAcOffClick = 0;
static DWORD g_dwAcUpClick = 0;
static bool  g_fAcUp = false;   	//交流电有没起来过,主要用来判断是否是按键唤醒
static bool   g_fFaShutDown = true;  //终端是否正在进行停电后的电池关断操作
bool g_fPowerOffAcked = false;	//停电确认标志

bool g_fDownSoft = false;	//是否在下载软件
DWORD g_dwFileTransCurSec = 0;


TDataItem g_diTimeRule;  //对时规则
TTime g_tmMinuteTask;	 //1分钟任务的时标
TTime g_tmDayTask;       //1天任务时标

TPowerOffTmp g_PowerOffTmp;     //掉电暂存变量
TPowerOffTmp g_DefaultPowerOffTmp = {PWROFF_VER, //版本 
									 true,  //掉电暂存变量有效标志
									 false,   //GPRS已连接
									 {0, 0, 0, 0},  //远程下载软件的服务器IP地址
									 false,			//bool fAlrPowerOff	掉电前上报了停电告警
									 0,				//WORD wRstNum;	复位次数
									 0,				//线程监控复位次数
									 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
									  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
									  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
									  0, 0},		//线程监控复位最后一次复位的线程名称
									 0,				//iRemoteDownPort
									 {0, 0, 0, 0, 0, 0, 0},//tPoweroff 上次停电时间
									 {0, 0, 0, 0, 0, 0, 0},//tPoweroff 上次上电时间
									 0,				//记录参数初始化事件的有效无效
									}; 
BYTE g_bTermChanelInfo[5] = {	2,	//脉冲输入路数 8
								4, //开关量输入路数 4
								3, //开关量输出路数(轮次) 4
								0xF4,0x01  //采集电表/交采装置的数量 500
}; 
BYTE g_bEvtCfgInfo[9] = { 0x40, 0x00, 0x00, 0x00, 0x01,0xff, 0xe3, 0x3b, 0xff};	//支持事件1~33，其中11,15,16,18,20,22不支持
TTime g_tPowerOn;
BYTE g_bRemoteDownIP[8];
//unsigned long TickMs;

TSoftVerChg g_SoftVerChg;	 //缓存的版本变更事件
TParaChg g_ParaChg;	//缓存的参数变更事件,一次应用服务最多同时处理50个OBIS对象
bool g_fFrzInit = false;	//冻结初始化是否完成

void CheckDownSoft(void);


//写入参数变更事件缓存数据
void SetParaChg(WORD wClass, BYTE* pbObis)
{
	bool fFind = false;
	for(WORD i=0; i<g_ParaChg.wNum; i++)
	{
		if (wClass==g_ParaChg.wClass[i] && memcmp(pbObis, &g_ParaChg.bObis[i][0], 6)==0)
		{
			fFind = true;
			break;
		}
	}
	if ( !fFind )
	{
		g_ParaChg.wClass[g_ParaChg.wNum] = wClass;
		memcpy(&g_ParaChg.bObis[g_ParaChg.wNum][0], pbObis, 6);
		g_ParaChg.wNum++;
	}
}

//写入参数变更事件
void SaveParaChgEvt()
{
	bool fFind = false;
	BYTE bBuf[MAXNUM_ONEERC3*6+3];
	TTime time;
	WORD wAddr;

	if (g_ParaChg.wNum>0 && g_ParaChg.wNum<MAXNUM_ONEERC3)
	{
		GetCurTime(&time);
		//终端地址
		 ReadItemEx( BN0, 0, 0x4037, (BYTE*)&wAddr);//倒序		
	
		bBuf[0] = (BYTE)wAddr;
		bBuf[1] = g_ParaChg.wNum;
		for(WORD i=0; i<g_ParaChg.wNum; i++)
		{
			memcpy(bBuf+2+i*6, &g_ParaChg.bObis[i][0], 6);

		}
//		if ( !SaveAlrData(ERC_PARACHG, time, bBuf, g_ParaChg.wNum*6+2) )
		{
			DTRACE(DB_METER_EXC, ("SaveParaChgEvt is failed"));
		}
	}
}

//CL818C7第3路485口和debug口共用，0xa200大于0关调试，
void InitConsole()
{
	BYTE bPortFun = PORT_FUN_ACQ;
	
	ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);

#ifndef EN_SPECIAL_TRANSFORMER
	if (bPortFun != PORT_FUN_DEBUG)
	{//做485抄表口用时，用关闭console
#ifdef SYS_LINUX
		signal(SIGINT, SIG_IGN);
		signal(SIGQUIT, SIG_IGN);
		signal(SIGTSTP, SIG_IGN);
		CloseConsole();
#endif
	}
#endif
}


//////////////////////////////JX//////////////////////////
WORD CRCCheck(BYTE* p, WORD wLen)
{
	WORD wMSBInfo;
	WORD wCrcData = 0xffff;
	WORD wIndexI;
	WORD wIndexJ;
	
	for (wIndexI=0; wIndexI < wLen; wIndexI++)
	{
		wCrcData = wCrcData ^ p[wIndexI];
		for (wIndexJ=0; wIndexJ<8; wIndexJ++)
		{
			wMSBInfo = wCrcData&0x0001;
			wCrcData = wCrcData >> 1;
			
			if (wMSBInfo!=0)
			{
				wCrcData = wCrcData ^ 0xa001;
			}
		}
	}
	return wCrcData;
}


////////////////////////////////////////////////////////////

BYTE NumToParity(BYTE n)
{
	static BYTE bParityTab[] = {NOPARITY, EVENPARITY, ODDPARITY}; 
	if (n >= 3)
	{
		return NOPARITY;
	}
	
	return bParityTab[n];
}



BYTE NumToStopBits(BYTE n)
{
	static BYTE bStopBitsTab[] = {ONESTOPBIT, TWOSTOPBITS, TWOSTOPBITS};
	if (n >= 3)
	{
		return ONESTOPBIT;
	}
	
	return bStopBitsTab[n];
}


DWORD NumToBaudrate(BYTE n)
{
	switch (n)
	{
	case 0:
		return CBR_300;
	case 1:
		return CBR_600;
	case 2:   
		return CBR_1200;
	case 3:   
		return CBR_2400;
	case 4:
		return CBR_4800;
	case 5:
		return 7200;
	case 6:
		return CBR_9600;
	case 7:
		return CBR_19200;
	default:
		return CBR_9600;
	}
	
	return CBR_9600;
}

/////////////////////////////////////////////////////////////////////////////////
//描述:校准时间
bool AdjTime()
{
	return false;
}

void DoFapCmd()
{
	if (g_dwExtCmdFlg==0 || GetClick()-g_dwExtCmdClick<2)
		return;
		
	if (g_dwExtCmdFlg == FLG_FORMAT_DISK)
	{
#ifndef SYS_WIN
		BYTE bBuf[256];
		int iAdjLen = readfile(USER_PARA_PATH"bank25.cfg", bBuf, 256);

		SetInfo(INFO_RST_TERM_STAT); //复位终端统计数据
#ifdef EN_CCT
		SetInfo(INFO_PLC_CLRRT);	 //集中器清除路由
#endif
		LockDB();
		system("rm -rf /mnt/data/para/*");
		system("rm -rf /mnt/data/data/*");

		if (iAdjLen > 0)	//只有在以前有这个文件的时候才保存
			WriteFile(USER_PARA_PATH"bank25.cfg", bBuf, iAdjLen);

#ifdef EN_AC
		AcClearLog();
#endif
		
		g_PulseManager.ClearLog();
#endif //SYS_WIN

		Sleep(2000);
        ResetApp();
	}
	else if (g_dwExtCmdFlg == FLG_DEFAULT_CFG)
	{
	}
	else if (g_dwExtCmdFlg==FLG_HARD_RST || g_dwExtCmdFlg==FLG_REMOTE_DOWN)
	{
		SavePoweroffTmp();	//收到软件下载命令后不保存系统库文件,光保存掉电变量,避免命令响应时间过长
		ResetApp();
	}
	else if (g_dwExtCmdFlg == FLG_DISABLE_METEREXC)
	{
		//g_TaskManager.DisableMeterExc();
	}
	else if (g_dwExtCmdFlg == FLG_ENERGY_CLR)
	{
		FaClearEnergy();
		ResetApp();
	}
	g_dwExtCmdFlg = 0;
}


WORD g_wOldTcpipRunCnt = 0; 
WORD g_wOldFaProtoRunCnt = 0;
DWORD g_dwProtoCheckClick = 0;


void SemInit()
{
	g_semTask = NewSemaphore(1);
	g_semExcTask = NewSemaphore(1);
	g_semMeter485 = NewSemaphore(1);
	g_semFaProto = NewSemaphore(1);
	g_semMeterLog = NewSemaphore(1);
	g_semNetStat = NewSemaphore(1);
	g_semGateWay = NewSemaphore(1);
	g_semGetIP = NewSemaphore(1);
	g_semClr645Data = NewSemaphore(0, 1);
	g_semLcd = NewSemaphore(1);
	g_semRWCtrlModl = NewSemaphore(1);
	g_semBat = NewSemaphore(1);//20131212-3

	g_semMtrUdp = NewSemaphore(1);
	g_semTskCfg = NewSemaphore(1);
	g_semSchCfg = NewSemaphore(1);
	g_semTermEvt = NewSemaphore(1);

#ifdef SYS_WIN32
	g_semClick = NewSemaphore(1);
	g_semClickPeriod = NewPeriodicSemaphore(1000);
#endif //SYS_WIN32	
}

//描述:从文件中初始化掉电暂存变量,如果无效,则使用默认
void InitPoweroffTmp()
{
	g_PowerOffTmp.fTmpValid = false;
	memset(g_bRemoteDownIP, 0x00, sizeof(g_bRemoteDownIP));

	bool fResult = ReadFile(USER_DATA_PATH"PoweroffTemp.cfg", (BYTE* )&g_PowerOffTmp, sizeof(g_PowerOffTmp));

	if (!fResult || g_PowerOffTmp.wVer!=PWROFF_VER || !g_PowerOffTmp.fTmpValid)
	{
		DTRACE(DB_GLOBAL, ("InitPoweroffTmp : use default power off tmp.\r\n"));
		memcpy(&g_PowerOffTmp, &g_DefaultPowerOffTmp, sizeof(g_PowerOffTmp));
	}

	if(g_PowerOffTmp.bRemoteDownIP[0] != 0 || g_PowerOffTmp.bRemoteDownIP[1] != 0 || 
	   g_PowerOffTmp.bRemoteDownIP[2] != 0 || g_PowerOffTmp.bRemoteDownIP[3] != 0)
	{
		memcpy(g_bRemoteDownIP,  &(g_PowerOffTmp.bRemoteDownIP[0]), sizeof(g_bRemoteDownIP));
		memset(&(g_PowerOffTmp.bRemoteDownIP[0]), 0x00, 8);
		g_fDownSoft = true;
	}

	//复位次数
	g_PowerOffTmp.wRstNum++;	
	g_DefaultPowerOffTmp.wRstNum = g_PowerOffTmp.wRstNum;
	WriteItemEx(BANK2, PN0, 0x1021, (BYTE *)&g_PowerOffTmp.wRstNum);	//0x1021 2 复位次数,HEX

	//线程监控复位次数
	g_DefaultPowerOffTmp.wMonitorRstNum = g_PowerOffTmp.wMonitorRstNum;	
	memcpy(g_DefaultPowerOffTmp.szMonitorRstThrd, g_PowerOffTmp.szMonitorRstThrd, sizeof(g_PowerOffTmp.szMonitorRstThrd));
	WriteItemEx(BANK2, PN0, 0x1029, (BYTE *)&g_PowerOffTmp.wMonitorRstNum); //0x1029 2 线程监控复位次数,HEX
	WriteItemEx(BANK2, PN0, 0x102a, (BYTE* )g_PowerOffTmp.szMonitorRstThrd); 	//0x102a 1 线程监控复位ID,HEX
	
	//g_DefaultPowerOffTmp.fAlrPowerOff = g_PowerOffTmp.fAlrPowerOff;		//清停电告警标志，留到通信协议中告警被确认才清
	GetCurTime(&(g_PowerOffTmp.tPowerOn));
	WriteFile(USER_DATA_PATH"PoweroffTemp.cfg", (BYTE* )&g_DefaultPowerOffTmp, sizeof(g_DefaultPowerOffTmp));


	//写入缺省终端输入/输出及端口配置数据
	//WriteItemEx(BN0, PN0, 0x5510, g_bTermChanelInfo);	
	//WriteItemEx(BN0, PN0, 0x5706, g_bEvtCfgInfo);	
	//BYTE bBuf[12] = {64, 0,0,0,0,0,0,0,0,16,0,0};
	//WriteItemEx(BN0, PN0, 0x5511, bBuf);
}

//描述:保存掉电暂存变量
bool SavePoweroffTmp()
{
	return WriteFile(USER_DATA_PATH"PoweroffTemp.cfg", (BYTE* )&g_PowerOffTmp, sizeof(g_PowerOffTmp));
}

void UdpMeterPara()
{
	if (GetInfo(INFO_METER_PARA) == false)
	 	return;	
	
	DTRACE(DB_METER, ("UdpMeterPara: para chg!\r\n"));

//	SetCtrlGrpParaChg(true);//发给控制线程停止运算

	UpdMeterPnMask();
//	UpdPnMask();
	SetInfo(INFO_AC_PARA);		//交采参数变更
	SetInfo(INFO_COMTASK_PARA); //普通任务参数变更
//	SetInfo(INFO_EXC_PARA);		//异常任务参数变更
//	SetInfo(INFO_PULSE);		//脉冲参数变更
	SetInfo(INFO_STAT_PARA);	//统计参数变更
	SetInfo(INFO_DC_SAMPLE);	//直流模拟量
	SetInfo(INFO_CTRL);			//控制参数
	SetInfo(INFO_RJMTR_PARACHG);//RJ45抄表参数
//	SetInfo(INFO_PRIMTR_PARACHG);//Prime抄表参数

//	SetMtrParaChg(true);
//	SetGrpParaChg(true);
	
	ClrPnChgMask();

	//CCT
#ifdef EN_CCT
	CctUpdPnMask();
	CctInitStat(); //初始化集抄抄读统计信息
	
	//UpdateVipMask(); //CctUpdPnMask里有处理
	
	SetInfo(INFO_PLC_PARA); //本函数放到CctUpdPnMask()后,
							//载波模块更新参数时可能用到测量点屏蔽位
	SetInfo(INFO_PLC_STATUPDATA);
#endif

	TrigerSaveBank(BN11, 0, -1); //即时统计的起点数据
}

bool g_fProgState = false;

bool IsInProgState()
{
#ifdef SYS_WIN
	return false;
#else
	//return g_MeterAuth.IsInProgState();
	return g_fProgState;
#endif
}


//CT/PT参数变更，为不影响电表运算，单独处理
void UdpCtPtPara()
{
	if (GetInfo(INFO_CTPT_PARA) == false)
	 	return;	
	
	DTRACE(DB_METER, ("UdpCtPtPara: para chg!\r\n"));	

//	SetGrpParaChg(true);
//	SetCtrlGrpParaChg(true); //发给控制线程停止运算

	SetInfo(INFO_AC_PARA);	 //交采参数变更
	//SetInfo(INFO_COMTASK_PARA); //普通任务参数变更
	//SetInfo(INFO_EXC_PARA);		//异常任务参数变更（总加组超差）
	//SetInfo(INFO_STAT_PARA);	//统计参数变更（总加组统计）

	TrigerSaveBank(BN11, 0, -1); //即时统计的起点数据
}


bool InitTCPIP();

TTdbPeriTime g_tmPeriTime[2];
/*
#ifdef SYS_LINUX
void sigCtrC(int n)
{
//	printf("\r\n sigCtrC !!! \r\n\r\n");	
	system("/clou/ppp/script/ppp-off");	//先关闭PPP
	Sleep(1000);
	int pid = getpid();
	kill(pid, SIGABRT);
	exit(0);	//线程退出
}
#endif
*/

void FaInitPortFrom698(void)
{
	TCommPara tCommParaTmp, tCommPara;
	BYTE bFunc[LOGIC_PORT_NUM], bFlag = 0, bFuncFlag = 0;
	int i;
	bool fRet;
	g_commLocal.Open(COMM_DEBUG, CBR_9600, 8, ONESTOPBIT, EVENPARITY);	//调试输出口 波特率9600，偶
	g_commLocal.GetCommPara(&tCommParaTmp);
	for(i=LOGIC_PORT_NUM; i>0; i--)
	{
		tCommPara.wPort = g_iInSnToPhyPort[i-1];
		fRet = OIRead_PortPara(0xF201, i-1, &tCommPara, &bFunc[i-1]);
		TraceBuf(DB_CRITICAL, ("\r\n FaInitStep1 :tCommParaTmp->"), (BYTE*)&tCommParaTmp, sizeof(tCommParaTmp));
		TraceBuf(DB_CRITICAL, ("\r\n FaInitStep1 :tCommPara->"), (BYTE*)&tCommPara, sizeof(tCommPara));
		if (true == fRet)
		{
			bFuncFlag |= 1<<(i-1);//记录有效功能口
			if(bFunc[i-1] == 0 && 
				(tCommParaTmp.wPort != tCommPara.wPort 
				|| tCommParaTmp.dwBaudRate != tCommPara.dwBaudRate
				|| tCommParaTmp.bByteSize != tCommPara.bByteSize
				|| tCommParaTmp.bParity != tCommPara.bParity
				|| tCommParaTmp.bStopBits != tCommPara.bStopBits
				))
			{
				DTRACE(DB_CRITICAL, ("FaInitStep1: g_commLocal need to config.i=%d\r\n",i-1));
				if(g_commLocal.IsOpen())
				{
					DTRACE(DB_CRITICAL, ("FaInitStep1: g_commLocal IsOpen.i=%d\r\n",i-1));
					g_commLocal.Close();
					bFlag = 1;//关闭
				}

				if (!g_commLocal.Open(tCommPara))
				{	
					DTRACE(DB_CRITICAL, ("FaInitStep1: g_commLocal open fail.i=%d\r\n",i-1));
				}
				else
				{
					DTRACE(DB_CRITICAL, ("FaInitStep1: g_commLocal open succ.i=%d\r\n",i-1));
					bFlag = 0;//成功打开
					break;
				}
			}
		
		}
	}
	if(bFlag == 1)
	{//被关闭
		DTRACE(DB_CRITICAL, ("FaInitStep1: g_commLocal recover\r\n"));
		g_commLocal.Open(tCommParaTmp);//恢复
	}
	DTRACE(DB_CRITICAL, ("FaInitStep1: g_commLocal init finish\r\n"));

}


void FaInitStep1()
{
	DTRACE(DB_FA, ("\r\n\r\n/**********************************************/\r\n"));
			DTRACE(DB_FA, ("/***           "FA_NAME" www.szclou.com     ***/\r\n")); 
    		DTRACE(DB_FA, ("/**********************************************/\r\n\r\n"));

	SysInit();
	SemInit();

#ifdef SYS_LINUX
//	signal(SIGHUP, sigCtrC);	
/*	signal(SIGINT, sigCtrC);	*/
//	signal(SIGQUIT, sigCtrC);
//	signal(SIGTERM, sigCtrC);	
#endif

	if (InitDir() == false)
	{
		DTRACE(DB_FA, ("FaInit: error:  fail to create sys dir.\r\n"));
		return;
	}
	InitInfo();

	TdbSetMaxRecLen(720);
	TdbSetMaxFieldLen(900);
	TdbSetPeriCacheNum(PN_VALID_NUM);
	TdbInit(USER_TASKDATA_PATH);
    
	InitDB(); //版本变更事件用到任务库
	
	ApllyCfgAuto();//开机自动应用配置文件
	
	InitPoweroffTmp();	//初始化掉电变量
	GetCurTime(&g_tPowerOn);
	//初始化测试口
#ifdef SYS_LINUX
	if (IsDownSoft())	//收到了下载命令,把测试口的波特率设置为115200,也用来下载
		g_commTest.Open(COMM_TEST, CBR_115200, 8, ONESTOPBIT, NOPARITY);
	else
		g_commTest.Open(COMM_TEST, CBR_9600, 8, ONESTOPBIT, NOPARITY);

	//初始化本地维护口
	g_commLocal.Open(COMM_LOCAL, CBR_2400, 8, ONESTOPBIT, NOPARITY);
#else
	g_commTest.Open(COMM_TEST, CBR_9600, 8, ONESTOPBIT, NOPARITY);
#endif //SYS_LINUX

// 	if (g_commYK.Open(COMM_PLC, CBR_19200, 8, ONESTOPBIT, EVENPARITY))
// 		DTRACE(DB_CTRL, ("*****FaInit: open COMM_PLC success!!!\r\n"));
// 	else
// 		DTRACE(DB_CTRL, ("*****FaInit: open COMM_PLC failed!!!\r\n"));
	InitTestMode();
	
	InitDebug();   //调试信息输出在数据库初始化后才进行,因为用到了数据库
//	InitConsole();

#ifdef SYS_LINUX
    //如果数据库中配置了698协议的端口信息，根据698协议初始化端口   whr 20170704
	FaInitPortFrom698();
#endif

#ifdef EN_SBJC_V2_CVTEXTPRO
	InitReadMeterFlg();
#endif
	char szTmp[21];
	memset(szTmp, 0, sizeof(szTmp));

#ifdef ENGLISH_DISP
	strcpy(szTmp, "NOR");
	WriteItemEx(BN2, PN0, 0x2030, (BYTE *)szTmp);	
	strcpy(szTmp, "Rd-M");
	WriteItemEx(BN2, PN0, 0x2032, (BYTE *)szTmp);
	strcpy(szTmp, "OffL");
	WriteItemEx(BN2, PN0, 0x2033, (BYTE *)szTmp);
#else
	strcpy(szTmp, "正常");
	WriteItemEx(BN2, PN0, 0x2030, (BYTE *)szTmp);	
	strcpy(szTmp, "抄表");
	WriteItemEx(BN2, PN0, 0x2032, (BYTE *)szTmp);
	strcpy(szTmp, "掉线");
	WriteItemEx(BN2, PN0, 0x2033, (BYTE *)szTmp);
#endif

	//当不需要使用内部版本号时，内部版本号中的版本及日期信息填0，由程序自动取发布版本的版本号及日期，显示到内部版本上。
//     if(IsAllAByte(&g_bInnerSoftVer[16], 0x00, 7))
// 		memcpy(&g_bInnerSoftVer[16], &g_bSoftVer[12], 7);
// 
 	WriteItemEx(BN2, PN0, 0x2107, g_bInnerSoftVer);

	WriteItemEx(BN2, PN0, 0x210e, (BYTE* )&g_PowerOffTmp.fAlrPowerOff); //上电初始化停上电状态
}

//描述:系统初始化期间对驱动的初始化挂接
void FaInitDrivers()
{
	InitLcd();
#ifndef SYS_WIN
	g_PulseManager.Init();
   
   	BYTE  bHardVer = GetHardVer();
   	WriteItemEx(BN2, PN0, 0x1057, &bHardVer);
   	DTRACE(DB_CRITICAL, ("FaIniDrivers: GetHardVer = %d.\r\n", bHardVer));

	BYTE bFmType = bHardVer & 0x04;
	if(bFmType==0x04)
		DTRACE(DB_CRITICAL, ("FaIniDrivers: FM24CL Size = 2K.\r\n"));
	else
		DTRACE(DB_CRITICAL, ("FaIniDrivers: FM24CL Size = 8K.\r\n"));
#endif
}

bool IsUpdateFirmware()
{
	return false;
}

bool FaSave()
{
	g_DataManager.Save();
	return true;
}

bool FaSavePara()
{
	g_DataManager.SavePara();
	return true;
}


void FaClose()
{
	FaSave();
	SavePoweroffTmp();
	ResetApp();
}

void DoFaSave()
{
	DbDoSave();
}

//参数：@pbIsValid 1表示无效，其他值表示有效
//返回：交采是否停电
bool IsAcPowerOff(BYTE* pbIsValid)
{
	int nRead;
	WORD i = 0, wBaseOffset = 0;
	bool fPowerOff = false;
	WORD wUn = 0, wMaxU = 1000, wMinU = 200;

	WORD wU[3];
	BYTE bBuf[SAMPLE_CFG_ID_LEN];
	BYTE* pbBuf = bBuf;

	*pbIsValid = 0;
	if (GetClick() < 9)
	{
		*pbIsValid = 1;
		return false;
	}

	nRead = ReadItemEx(BN2, PN0, 0x210e, (BYTE* )&fPowerOff); //当条件都不满足时，返回上次判断的状态
	if (nRead <= 0)
		return false;

	nRead = ReadItemEx(BN25, PN0, 0x5001, bBuf);	//读额定电压
	if (nRead <= 0)
		return false;

	wUn = (DWORD )BcdToDWORD(bBuf, 3) / 10; //额定电压值2200,一位小数
	if (wUn == 0)
		wUn = 2200;	//额定电压,格式NNNNN.N

	memset(bBuf, 0, sizeof(bBuf));
	nRead = OoReadAttr(TERM_POWOFF, ATTR6, bBuf, NULL, NULL); //读取属性6配置参数-停电数据采集配置参数
	if (nRead<=0 || bBuf[0]!=DT_STRUCT)
		return false;

	wBaseOffset = 0;
	pbBuf = &bBuf[12];	//指向TSA个数
	for (i=0; i<bBuf[12]; i++)
	{
		wBaseOffset += 2;	//TSA数据类型1 + TSA长度1
		wBaseOffset += pbBuf[wBaseOffset];
	}
	
	wBaseOffset += 12;	//从bBuf开始，需加上前面的12个字节
	wBaseOffset += 16;	//停电事件甄别限值-停电发生电压限值偏移

	wMinU = OoLongUnsignedToWord(bBuf+wBaseOffset);
	if (wMinU == 0)
		wMinU = wUn * 6 / 10;

	wMaxU = OoLongUnsignedToWord(bBuf+wBaseOffset+3);
	if (wMaxU == 0)
		wMaxU = wUn * 8 / 10;

	memset(bBuf, 0, sizeof(bBuf));
	nRead = OoReadAttr(0x2000, ATTR2, bBuf, NULL, NULL);
	if (nRead <= 0 || bBuf[0]!=DT_ARRAY || bBuf[1]!=3 || bBuf[2]!=DT_LONG_U)
		return false;

	for (i=0; i<3; i++)
	{
		wU[i] = OoLongUnsignedToWord(bBuf+i*3+3);
	}

	if (wU[0]<wMinU && wU[1]<wMinU && wU[2]<wMinU)
	{
		if (!fPowerOff)
		{
			fPowerOff = true;
			WriteItemEx(BN2, PN0, 0x210e, (BYTE* )&fPowerOff);
		}
	}
	else if(wU[0]>=wMaxU || wU[1]>=wMaxU || wU[2]>=wMaxU)
	{
		if (fPowerOff)
		{
			fPowerOff = false;
			WriteItemEx(BN2, PN0, 0x210e, (BYTE* )&fPowerOff);
		}
	}

    return fPowerOff;
}

bool IsPowerOff()
{
#ifdef SYS_LINUX
	return (IsPoweredByAC()==false);
#endif //SYS_LINUX

#ifdef SYS_WIN
	return false;
#endif //SYS_WIN
}


char* g_pszFapErrStr[] = {"ERR_OK",
						  "ERR_FORWARD",
						  "ERR_INVALID",
						  "ERR_PERM",
						  "ERR_ITEM",
						  "ERR_TIME",
						  "ERR_ADDR",
						  "ERR_SEND",
						  "ERR_SMS"};

char* ErrNumToStr(WORD wNO)
{
	if (wNO < sizeof(g_pszFapErrStr)/sizeof(char *))
		return g_pszFapErrStr[wNO];
	else
		return "UNKNOW ERR";
}

int SearchAlrID(WORD wID, WORD* pAlrID, WORD wNum)
{
	for (WORD i=0; i<wNum; i++)
	{
		if (wID == *pAlrID++)
			return i;  		
	}	
	return -1;
}	

void PushEvt_ParaInit()
{
	BYTE bErcData[10];
 	BYTE bSoftVer[SOFT_VER_LEN]; 

	bErcData[0] = 8; //bit位数
	bErcData[1] = 1;
 	ReadItemEx(BN0, PN0, 0x100f, bSoftVer);
	memcpy(bErcData+2, bSoftVer+16, 4 );
	memcpy(bErcData+6, bSoftVer+16, 4 );

	//先保存到掉电变量，上电后再写入任务库
	g_PowerOffTmp.bParaEvtType = 0; //GetErcType(ERC_INIT_VER);
	if (g_PowerOffTmp.bParaEvtType > 0)
	{
		g_PowerOffTmp.ParaInit.time = GetCurTime();	
		memcpy(&g_PowerOffTmp.ParaInit.bVerInfo, bErcData, 10); 
		//SaveAlrData(ERC_INIT_VER, t, bErcData);	
	}
}

bool g_fYxInit = false;
static BYTE g_bYxVal = 0;

void SetYxInitFlag(bool fInit)
{
	g_fYxInit = fInit;
}

//描述：设置一类数据F9遥信的状态；
void DoYX()
{
	int i, iLen;
	BYTE bYxVal = 0; //遥信状态标志
    BYTE bChgFlg;	 //状态变位标志
	BYTE bBuf[20] = {0};
	BYTE bValid, bYxFlag = 0;
    BYTE bDoorStat = 0x10;
	const WORD wOI = OI_YX;

	if (IsAcPowerOff(&bValid) || bValid==1)
		return ;
    
#ifdef SYS_LINUX
	 bDoorStat = GetDoorStatus(); //门节点分状态为高电平
#endif
    int nRead = ReadItemEx(BN2, PN0, 0x1100, &bYxVal);
    if (nRead <= 0 || GetClick() < 5)
    	return;

	if(bDoorStat > 0)
		bDoorStat = 0x00;
	else
		bDoorStat = 0x10;

	 bYxVal = (bYxVal&0xef) | bDoorStat;	

    if (!g_fYxInit)
    {//第一次仅仅读取遥信状态位，不判断；
        g_bYxVal = bYxVal;
        g_fYxInit = true;

		 memset(bBuf, 0, sizeof(bBuf));
		iLen = OoReadAttr(wOI, ATTR4, bBuf, NULL, NULL);	//取开关量接入和属性参数
		if (iLen>0 && bBuf[2]==DT_BIT_STR && bBuf[5]==DT_BIT_STR)
		{
			bBuf[4] = BitReverse(bBuf[4]);
			bYxFlag = bBuf[4] & 0x000f;
		}
		else
		{
			bYxFlag = 0;
		}

		for (i=0; i<MAX_SW_PORT_NUM; i++)
		{
			bBuf[0] = DT_STRUCT;
			bBuf[1] = 2;
			bBuf[2] = DT_UNSIGN;

			DTRACE(DB_CRITICAL, ("DoYX: bYxFlag = %x.\r\n", bYxFlag));
			if (bYxFlag & (1<<i))	//YX接入
			{
				#ifdef OOP_TAITI_TEST
				if (bYxVal & (1<<i))
					bBuf[3] = YX_OFF_STATE;
				else
					bBuf[3] = YX_ON_STATE;
				#else
				if (bYxVal & (1<<i))
					bBuf[3] = YX_ON_STATE;
				else
					bBuf[3] = YX_OFF_STATE;
				#endif
			}
			else
			{
				DTRACE(DB_CRITICAL, ("DoYX: bBuf[3] = %x.\r\n", bBuf[3]));
				bBuf[3] = YX_OFF_STATE;
			}

			bBuf[4] = DT_UNSIGN;
			bBuf[5] = 0;

			WriteItemEx(BN0, i, 0xF203, bBuf);  //存储遥信状态量
		}

        return;
    }
 
	bChgFlg = g_bYxVal ^ bYxVal;
    g_bYxVal = bYxVal;

    if (bChgFlg != 0)
    {
		for (i=0; i<MAX_SW_PORT_NUM; i++)
		{
			if (bChgFlg & (1<<i))	//某路YX有变化才更新该路，不影响其他YX变位标志
			{
				DTRACE(DB_CRITICAL, ("DoYX: yx chg, i=%d, bYxVal=%x.\r\n", bYxVal));
				bBuf[0] = DT_STRUCT;
				bBuf[1] = 2;
				bBuf[2] = DT_UNSIGN;

				#ifdef OOP_TAITI_TEST
				if (bYxVal & (1<<i))	//YX状态
					bBuf[3] = YX_OFF_STATE;
				else
					bBuf[3] = YX_ON_STATE;
				#else
				if (bYxVal & (1<<i))	//YX状态
					bBuf[3] = YX_ON_STATE;
				else
					bBuf[3] = YX_OFF_STATE;
				#endif

				bBuf[4] = DT_UNSIGN;
				bBuf[5] = 1;	//变位标志
				WriteItemEx(BN0, i, 0xF203, bBuf);  //存储遥信状态量
			}
		}
	}
}

//描叙：控制回路检测断线
void DoCtrlLoop()
{
	TTime time;
	memset(&time, 0, sizeof(time)); 
	BYTE bAlrBuf[2] = {0};
	BYTE bRetFrm = 0;
	BYTE bChgFlg;
	BYTE bLen = 0;
	BYTE bLoop = 0;
    static BYTE bLastLoop = 0;
	static bool fCtrlStateInit = false;
	static BYTE bCtrlStateVal = 0;

	bLen = ReadCtrlLoop(bLoop);
	//有一次发现  初始化起来终端总是读不到单片机数据数据 则ReadCtrlLoop<0 ,g_fCtrlStateInit一直不能为true
	if((bLen > 0) || (!fCtrlStateInit))
	{
		if (IsDebugOn(115))
			bLoop = 0;
		if (IsDebugOn(116))
			bLoop = 1;
		if (IsDebugOn(117))
			bLoop = 2;
		bRetFrm = bLoop;

		if(bLen > 0)
			bRetFrm = ((~bRetFrm) & 0x03) | (bRetFrm & 0xfc);

		if(!fCtrlStateInit)
		{
			fCtrlStateInit = true;
			bAlrBuf[0] = bRetFrm;
			bCtrlStateVal = bRetFrm;//更新初始值
			WriteItemEx(BN0, PN0, 0x152f, bAlrBuf);
			return;
		}

		bChgFlg = bRetFrm^bCtrlStateVal;
		bCtrlStateVal = bRetFrm;
	
		bLastLoop = bLoop;
		DoLoopLed(bLastLoop);

		if(bChgFlg)
		{
			ReadItemEx(BN0, PN0, 0x152f, bAlrBuf);
			bAlrBuf[0] = bRetFrm;
			bAlrBuf[1] |= bChgFlg;
			WriteItemEx(BN0, PN0, 0x152f, bAlrBuf);
			bAlrBuf[0] = bChgFlg;
			bAlrBuf[1] = bRetFrm;
			GetCurTime(&time);	
//			SaveAlrData(ERC_CTRLSTATE, time, bAlrBuf);
			DTRACE(DB_METER_EXC, ("FaAPI__DoCtrlLoop: ############# CtrlState happen!!!\r\n"));

		}
	}

	if (bLen == 0)
		DoLoopLed(bLastLoop);
}

TThreadRet WdgThread(void* pvPara)
{
	int iMonitorID;
    WORD i=0, j=0;
    while (1)
    {
        Sleep(100);
		ClearWDG();

		i++;
		if (i == 4)
		{
			if (j & 1)
			{
				iMonitorID = DoThreadMonitor();	//线程监控,每秒执行一次
				if (iMonitorID < 0)
				{
					iMonitorID = -iMonitorID-1;
					GetMonitorThreadName(iMonitorID, g_PowerOffTmp.szMonitorRstThrd); 
					g_PowerOffTmp.wMonitorRstNum++;
					DTRACE(DB_CRITICAL, ("WdgThread:reset %s.\r\n", g_PowerOffTmp.szMonitorRstThrd));
					SavePoweroffTmp();
					ResetApp();
					while(1);
				}
			}

			j++;
			
			if(IsGprsConnected())
				ToggleRunLed();
			else
#ifdef SYS_WIN
				;
#else
				SetRunLed(true);
#endif
	
			i = 0;
		}
    }
    
    return THREAD_RET_OK;
}

//充电控制只是一个辅助的手段，是检测到电池电压很低的时候要慢充电
//就是怕电池短路了耗电大了
//就平时运行时根据电池电压来控制，刚上电时和掉电后打开与否都没关系
static bool g_fIsBatOk = true;//20140320
void DoBatMgr()
{
#ifdef SYS_LINUX
	WORD fVal;
	BYTE bBuf[8];
	
	BYTE bIsValid = 0;

	if (IsAcPowerOff(&bIsValid))  //交采停电
	{
		if (bIsValid == 1)
			return;
		return;
	}
	
	ReadItemEx(BANK2, POINT0, 0x1028, bBuf); //0x1028 2 电池电压 NN.NN V
	fVal = BcdToDWORD(bBuf, 2);
	if (fVal > 645 || fVal==0) //没接电池//20131212-3
	{
		g_fIsBatOk = false;
		GprsBatCharge(false);//关闭充电
		return ;
	}
		
	if (fVal < 430)
	{
		GprsBatCharge(true);//打开充电
		//BackBatOnOff(true);//停电后，电池无法关断
	}
	else if (fVal > 530)
	{
		GprsBatCharge(false);//关闭充电
		//BackBatOnOff(false);
	}
#endif
}

//取电池状态，true表示电池已插好
bool GetBatStat()//20140320
{
	bool fIsBatOk = true;
	WaitSemaphore(g_semBat);
	fIsBatOk = g_fIsBatOk;
	SignalSemaphore(g_semBat);
	return fIsBatOk;
}

void BatStas()
{
#ifdef SYS_LINUX
	float fVal;
	BYTE bBuf[8];
	BYTE bBatStat = 0x00;
	//if (IsAcPowerOff() == 0)  //交采停电
		//return;
	ReadItemEx(BANK2, POINT0, 0x1028, bBuf); //0x1028 2 电池电压 NN.NN V
	fVal = (float)BcdToDWORD(bBuf, 2)/100.0;

	if(fVal>6.5 || fVal<4.4)
	{
		bBatStat = 0x00;
		WriteItemEx(BANK2, POINT0, 0x2053, &bBatStat);
	}
	else
	{
		bBatStat = 0x01;
		WriteItemEx(BANK2, POINT0, 0x2053, &bBatStat);
	}

	WriteItemEx(BANK2, POINT0, 0x2053, &bBatStat);
#endif
}
/*
//#ifdef EN_AC
bool InitVBreakStat()	//初始化断相统计
{
	return g_VBreakStat.InitRun(); 
}

bool DoVBreakStat()
{
	return g_VBreakStat.DoTask();
}
//#endif
*/
void UpdateDeviceFaultStatus(void)
{
#ifndef SYS_WIN
	BYTE bBuf[10];
	//时钟芯片故障
	ReadItemEx(BN2, PN0, 0x1122, bBuf);
	bBuf[1] = GetRtcChipStatus();
	if(bBuf[1]!=bBuf[0])
	{
		WriteItemEx(BN2, PN0, 0x1122, bBuf);
	}	

	
	//计量芯片故障
	ReadItemEx(BN2, PN0, 0x1123, bBuf);
	bBuf[1] = AcGetFaultStatus();
	if(bBuf[1]!=bBuf[0])
	{
		WriteItemEx(BN2, PN0, 0x1123, bBuf);
	}	
#endif
}

void DealInfoRS485ParaChg(void)
{
	TCommPara tCommPara[LOGIC_PORT_NUM],tCommParaTmp;
	BYTE bFunc[LOGIC_PORT_NUM], bFlag = 0, bFuncFlag = 0, bUpPortFlag = 0;
	int i;
	bool fRet;
	memset(bFunc, 0x00, LOGIC_PORT_NUM);
	DTRACE(DB_CRITICAL, ("DriverThread : rx INFO_RS485_PARACHG......\r\n"));
	g_commLocal.GetCommPara(&tCommParaTmp);
	// 1.先读取参数及功能
	for(i=LOGIC_PORT_NUM; i>0; i--)
	{
		tCommPara[i-1].wPort = g_iInSnToPhyPort[i-1];
		fRet = OIRead_PortPara(0xF201, i-1, &tCommPara[i-1], &bFunc[i-1]);
		TraceBuf(DB_CRITICAL, ("\r\n DriverThread :tCommParaTmp->"), (BYTE*)&tCommParaTmp, sizeof(tCommParaTmp));
		TraceBuf(DB_CRITICAL, ("\r\n DriverThread :tCommPara->"), (BYTE*)&tCommPara[i-1], sizeof(tCommPara[i-1]));
		if (true == fRet)
		{
			if(bFunc[i-1]==0)
			{
				bUpPortFlag = 1;//有上行口
			}
			bFuncFlag |= 1<<(i-1);//记录有效功能口				
		}
	}
	// 2.容错维护口配置错误
	if(bUpPortFlag == 0)
	{//没有维护口了,恢复第3路为维护口
		bFunc[2] = 0;
		bFuncFlag |= 1<<2;
		DTRACE(DB_CRITICAL, ("DriverThread: g_commLocal reset to 2\r\n"));
	}
	DTRACE(DB_CRITICAL, ("DriverThread: bFuncFlag=%x\r\n",bFuncFlag));
	// 3.更新内部通信口配置
	for(i=LOGIC_PORT_NUM; i>0; i--)
	{
		DTRACE(DB_CRITICAL, ("DriverThread: bFunc[%d]=%d\r\n",i-1,bFunc[i-1]));
		if(bFuncFlag&(1<<(i-1)))
		{//有效才设置
			//上行通信（0），抄表（1），级联（2），停用（3）
			switch(bFunc[i-1])
			{
				case 0:
					DTRACE(DB_CRITICAL, ("DriverThread: %d not support setting to PORT_FUN_LOCAL485\r\n",i));
					//一型集中器只有两路485口，不支持第三路485复用功能
					//SetLogicPortFun(i,PORT_FUN_LOCAL485);
					break;
				case 1:
					DTRACE(DB_CRITICAL, ("DriverThread: %d set to PORT_FUN_RDMTR\r\n",i));
					SetLogicPortFun(i,PORT_FUN_RDMTR);
					break;
				case 2:
					DTRACE(DB_CRITICAL, ("DriverThread: %d set to PORT_FUN_LINK\r\n",i));
					SetLogicPortFun(i,PORT_FUN_LINK);
					break;
				default:
					break;//其他不处理
			}
			WORD wID;
			BYTE b;
			if (i == 1) //485-1口
				wID = 0xa131;
			else if (i == 2)	//485-2口
				wID = 0xa132;
			//一型集中器只有两路485口，不支持第三路485复用功能
			//else if (i == 3)	//485-3口
				//wID = 0xa180;
			ReadItemEx(BN10, PN0, wID, (BYTE*)&b);
			DTRACE(DB_CRITICAL, ("DriverThread: %d wID=%x,b=%d\r\n",i,wID,b));
		}
	}			
	// 4.切换端口
	bFlag = 0;//成功打开
	for(i=LOGIC_PORT_NUM; i>0; i--)
	{
		tCommPara[i-1].wPort = g_iInSnToPhyPort[i-1];
		TraceBuf(DB_CRITICAL, ("\r\n DriverThread :tCommParaTmp->"), (BYTE*)&tCommParaTmp, sizeof(tCommParaTmp));
		TraceBuf(DB_CRITICAL, ("\r\n DriverThread :tCommPara->"), (BYTE*)&tCommPara[i-1], sizeof(tCommPara[i-1]));
		if((bFuncFlag&(1<<(i-1))) &&
			 (bFunc[i-1] == 0) && 
			(tCommParaTmp.wPort != tCommPara[i-1].wPort 
			|| tCommParaTmp.dwBaudRate != tCommPara[i-1].dwBaudRate
			|| tCommParaTmp.bByteSize != tCommPara[i-1].bByteSize
			|| tCommParaTmp.bParity != tCommPara[i-1].bParity
			|| tCommParaTmp.bStopBits != tCommPara[i-1].bStopBits
			))
		{
			DTRACE(DB_CRITICAL, ("DriverThread: g_commLocal need to config.i=%d\r\n",i-1));
			if(g_commLocal.IsOpen())
			{
				DTRACE(DB_CRITICAL, ("DriverThread: g_commLocal IsOpen.i=%d\r\n",i-1));
				g_commLocal.Close();
				bFlag = 1;//关闭
			}
	
			if(g_commRs485[i-1].IsOpen())
			{
				g_commRs485[i-1].Close();
			}
			
			if (!g_commLocal.Open(tCommPara[i-1]))
			{	
				DTRACE(DB_CRITICAL, ("DriverThread: g_commLocal open fail.i=%d\r\n",i-1));
			}
			else
			{
				DTRACE(DB_CRITICAL, ("DriverThread: g_commLocal open succ.i=%d\r\n",i-1));
				bFlag = 0;//成功打开
				break;
			}
		}
	}
	
	if(bFlag == 1)
	{//被关闭
		DTRACE(DB_CRITICAL, ("DriverThread: g_commLocal recover\r\n"));
		g_commLocal.Open(tCommParaTmp);//恢复当前口
	}

}

void DealInfoINFRAParaChg(void)
{
	DTRACE(DB_CRITICAL, ("DriverThread : rx INFO_INFRA_PARACHG......\r\n"));
	TCommPara tCommPara;
	bool fRet;
	g_commTest.GetCommPara(&tCommPara);
	fRet = OIRead_PortPara(0xF202, 0, &tCommPara, NULL);
	if(fRet == true)
	{
		if(g_commTest.IsOpen())
		{
			g_commTest.Close();
		}
		g_commTest.Open(tCommPara);	
	}
}

//描述:本线程放驱动相关的函数
TThreadRet DriverThread(void* pvPara)
{
	WORD wCnt = 0, wCheck=0;//20131212-3
	int iMonitorID = ReqThreadMonitorID("driver-thrd", 60*60);	//申请线程监控ID,更新间隔为一个小时
	DTRACE(DB_CRITICAL, ("DriverThread : started!\n"));
	
#ifdef SYS_LINUX
	CDCSample dcSample;
//		InitDrvPara();
	CDCProc dcProc;
	CCalcPulse calcPulse;
	DCInit();
	dcProc.Init();
	if (!calcPulse.Init(4))
		DTRACE(DB_CRITICAL, ("DriverThread : CCalcPulse init failed.\r\n"));

	GprsBatOnOff(false); //打开两个电池
	BackBatOnOff(true);
	GprsBatCharge(true);  //先打开充电开关，没接电池时电池口的电压为7v左右//20131212-3
#endif

#ifndef SYS_WIN
//	g_CTcheck.Init();
	dcSample.Init();
	EsamInit();
#endif

	InitYX();
#ifdef EN_CTRL
	InitYK();
#endif
	YXRun();

	Sleep(200);
#ifndef SYS_WIN
	dcSample.Run();
#endif
	DoBatMgr();//20131212-3
	while (1)
	{
		//DoAlrLedCtrl();
      //  DoBatMgr();  //D82没有充电管理
		BatStas();//xzz
#ifndef SYS_WIN
		wCnt++;
		wCheck++;
		if(wCheck >= 50)  //5s打开一下充电开关，检测下电池是否接好//20131212-3
		{
			//5s获取一下会话时效剩余时间 
			Esam_ReflashSessionRemainTime();
		
			//5s打开一下充电开关，检测下电池是否接好//20131212-3
			WaitSemaphore(g_semBat);
			g_fIsBatOk = true;
			GprsBatCharge(true);
			Sleep(200);
			dcSample.Run();
			DoBatMgr();
			SignalSemaphore(g_semBat);
			wCheck = 0;
		}	
		if (wCnt >= 10)	//每1秒执行一次
		{	
#ifdef EN_AC				
			AcSaveLog();
#endif
			g_PulseManager.SaveLog();
			g_StatMgr.SaveTermStat();
			UpdateDeviceFaultStatus();

		#ifdef SYS_LINUX
			DCRun();
		#endif //SYS_LINUX

			wCnt = 0;
			
	    }
		//dcProc.DoDCProc();
		YXRun();
        DoYX();
		
	    YKRun();
		BeepPulse();
#endif //SYS_WIN

#ifdef SYS_LINUX
		calcPulse.Run();
#endif

#ifndef SYS_WIN
//		g_CTcheck.Run();
#endif
		if (GetInfo(INFO_RS232_PARACHG))	//收到232参数修改消息
		{
			DTRACE(DB_CRITICAL, ("DriverThread : rx INFO_RS232_PARACHG......\r\n"));
		}
		if (GetInfo(INFO_RS485_PARACHG))	//收到485参数修改消息
		{
			DealInfoRS485ParaChg();
		}
		if (GetInfo(INFO_INFRA_PARACHG))	//收到红外参数修改消息
		{
			DealInfoINFRAParaChg();
		}
		TransferLidStatus();
		UpdThreadRunClick(iMonitorID);
		Sleep(100);
	}	

#ifdef SYS_LINUX
	calcPulse.Close();
	DCClose();
#endif

	ReleaseThreadMonitorID(iMonitorID);

	return THREAD_RET_OK;
}

/*
//任务线程  含普通任务及异常任务
TThreadRet TaskThread(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("task-thrd", 60*60);	//申请线程监控ID,更新间隔为120秒

	SaveSoftVerChg();

	DTRACE(DB_CRITICAL, ("TaskThread : started!\n"));
	g_TaskManager.Init();	
//	g_TRJTaskManager.Init();
#ifdef EN_CCT
	g_CctTaskManager.Init();
#endif

	DWORD dwOldClick = GetClick();

	while (1)
	{
//		DoMtrAnd485ErrErc();
		Sleep(1000); //任务实时性不是太高 10s
#ifdef EN_CCT
		UpdCctRdStatus();
		g_CctTaskManager.DoCommonTask();
#endif
		Sleep(1000); //任务实时性不是太高 5s
		g_TaskManager.DoTask();
//		g_TRJTaskManager.DoTask();
		UpdThreadRunClick(iMonitorID);
	}

	ReleaseThreadMonitorID(iMonitorID);

	return THREAD_RET_OK;
}

//数据处理线程
TThreadRet DataProcThread(void* pvPara)
{	
	int iMonitorID = ReqThreadMonitorID("DataProc-thrd", 60*60);	//申请线程监控ID,更新间隔为120秒

	UpdMtrDataProcess(); //测量点
	UpdGrpDataProcess(true); //总加数据

	while (1)
	{		
		if (IsMtrParaChg())
		{
			UpdMtrDataProcess();
			SetMtrParaChg(false);
		}

		if (!IsMtrParaChg() )//&& IsGrpParaChg()) //一定要测量点更新过了再更新总加组
		{
			UpdGrpDataProcess(false);	
			SetGrpParaChg(false);	

			//先运行计算一遍，再发给控制线程重新开始运算
			RunMtrDataProcess();
			if (IsFkTermn())	//负控终端才需要做总加组数据的处理
			{
				RunGrpDataProcess();
			}
			TrigerSaveBank(BN18, 0, -1); //起点示值	触发保存.
			
			SetCtrlGrpParaChg(false);	
		}

		RunMtrDataProcess();
		if (IsFkTermn())	//负控终端才需要做总加组数据的处理
		{
			RunGrpDataProcess();
		}
		ClearWDG();

		UpdThreadRunClick(iMonitorID);
		Sleep(1000);
	}	

	ReleaseThreadMonitorID(iMonitorID);

	return THREAD_RET_OK;
}

*/
//数据处理线程
TThreadRet DataStatThread(void* pvPara)
{	
	int iMonitorID = ReqThreadMonitorID("DataStat-thrd", 60*60);	//申请线程监控ID,更新间隔为120秒


	while (1)
	{		
		g_StatMgr.DoDataStat();

		UpdThreadRunClick(iMonitorID);

		Sleep(1000);
	}	
	
	ReleaseThreadMonitorID(iMonitorID);

	return THREAD_RET_OK;
}

//负控线程
TThreadRet LoadCtrlThread(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("LoadCtrl-thrd", 60*60);	//申请线程监控ID,更新间隔为120秒

	Sleep(5000);	//在启动时负控延迟5秒启动.
	g_LoadCtrl.Init();
	SetCtrlThreadStart(true);
	
	static DWORD dwClick = 0;
	while (1)
	{
		if (dwClick==0 || (GetClick()-dwClick>20) || GetInfo(INFO_CTRL))//收到信息或者间隔50秒以上执行一次
		{
			g_LoadCtrl.DoCtrl();
			UpdThreadRunClick(iMonitorID);
			dwClick = GetClick();
		}
		Sleep(500);
	}

	ReleaseThreadMonitorID(iMonitorID);

	return THREAD_RET_OK;
}

/*
#ifdef EN_VARCPS //是否允许无功补偿(VAR compensator)功能
//无功补偿线程
TThreadRet LoadWGCtrlThread(void* pvPara)
{
	DTRACE(DB_CRITICAL, ("LoadWGCtrlThread : started!\n"));
	int iMonitorID = ReqThreadMonitorID("WGCtrl-thrd", 60*60);	//申请线程监控ID,更新间隔为120秒
	InitWGYK();
	Sleep(5000);	//在启动时负控延迟5秒启动.
	g_Compensate.Init();
	while (1)
	{
		g_Compensate.CalcCurrQStatus();
		UpdThreadRunClick(iMonitorID);
		Sleep(1000);
	}

	ReleaseThreadMonitorID(iMonitorID);

	return THREAD_RET_OK;
}
#endif
*/
//描述:快秒线程
TThreadRet FastSecondThread(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("FastSecond-thrd", 60*60);	//申请线程监控ID,更新间隔为60秒
	DTRACE(DB_CRITICAL, ("FastSecondThread : started!\n"));

	BYTE bPlcStusOld = GetPlcStatus(); 
	BYTE bPlcStusCur = bPlcStusOld;
	bool fFirstStus  = (bPlcStusOld==PLC_MODULE_EXIST) ? true : false;
	WORD wCnt = 0;

	while(1)
	{
		wCnt++;
		if (wCnt >= 10)	//每1秒执行一次
		{	
			wCnt = 0;
			UdpMeterPara();
			UdpCtPtPara();

//#ifdef EN_AC
//			DoVBreakStat();		//645断相统计
//#endif

#ifndef SYS_WIN
		    g_PulseManager.Run();
#endif	//SYS_WIN

			bPlcStusCur = GetPlcStatus();
			if(bPlcStusCur != bPlcStusOld)
			{
				bPlcStusOld = bPlcStusCur; 
				if(!fFirstStus) //终端启动时没插模块，后来才插的模块，不能发模块更换消息
				{
					fFirstStus = true;
					continue;
				}

				SetInfo(INFO_PLC_MOD_CHANGED);
				DTRACE(DB_CRITICAL, ("FastSecondThread : SetInfo INFO_PLC_MOD_CHANGED!\n"));	
			}

			UpdThreadRunClick(iMonitorID);
		}

		SyncTimer();//每个小时同步一下硬件时钟
		Sleep(100);
	}

	ReleaseThreadMonitorID(iMonitorID);
}

TThreadRet DownSoftSecondThread(void* pvPara)
{
	DTRACE(DB_CRITICAL, ("DownSoftSecondThread : started!\n"));

	while(1)
	{
	    if (IsUpdateFirmware())
			break;

		if (GetInfo(INFO_APP_RST))	//CPU复位
		{
			DTRACE(DB_CRITICAL, ("DownSoftSecondThread : rx INFO_APP_RST......\n"));
			Sleep(1000);
			ResetCPU();
		}

		Sleep(1000);
	}
	
	return 0;
}

TThreadRet  Do485MeterSearch(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("Do485MeterSearch-thrd", 60*60);	//申请线程监控ID,更新间隔为60秒
	DTRACE(DB_CRITICAL, ("Do485MeterSearch : started!\n"));

	InitMtrPortSch(0);
	while(1)
	{
		if(GetInfo(INFO_START_485_SCH_MTR))
		{
			g_f485SchMtr = true;
			InitMtrPortSch(1);
			DTRACE(DB_CRITICAL, ("Do485MeterSearch : Start 485 search meter, dwClick=%ld!\n", GetClick()));
		}

		if (GetInfo(INFO_STOP_485_SCH_MTR))
		{
			InitMtrPortSch(0);
			DTRACE(DB_CRITICAL, ("Do485MeterSearch : Stop 485 search meter, dwClick=%ld!\n", GetClick()));
			g_f485SchMtr = false;
		}

		DoMtrPortSch();

		Sleep(100);
		UpdThreadRunClick(iMonitorID);
	}

	ReleaseThreadMonitorID(iMonitorID);
}

bool g_fPowerOffSave = false;		//掉电后是否需要触发保存
//电源管理
void DoPowerManagement()
{
	BYTE bBuf[64];
	BYTE bIsValid;
	DWORD dwClick = GetClick();
/*
	static bool fPowoffInit = false;
	if (!fPowoffInit && g_pPowerOff != NULL && GetClick()>10)
	{
		fPowoffInit = g_pPowerOff->InitRun(0, 14);
	}

	if (g_TaskManager.m_fInited && fPowoffInit && g_pPowerOff)	//停上电告警需要单独提取出来，防止再任务线程中，因为抄表阻塞导致告警运行失败
		g_pPowerOff->RunTask();
*/
#ifndef SYS_WIN	
	CheckPower();
	if (!IsAcPowerOff(&bIsValid) && bIsValid==0)  //交采有电
	{	
		g_dwAcOffClick = 0;
		g_fPowerOffSave = false;		//掉电后是否需要触发保存
		
		if (g_dwAcUpClick == 0)
		{
			g_dwAcUpClick = dwClick;
			return;
		}
		
		if (dwClick-g_dwAcUpClick >= 3)
		{
			g_dwAcUpClick = 0;
			g_fAcUp = true;  //交流电有没起来过,主要用来判断是否是按键唤醒
			if (g_fFaShutDown)  //以前关过电池
			{
			    GprsBatOnOff(true); //重新打开两个电池
				BackBatOnOff(true); //打开后备电池
				g_fFaShutDown = false;  //只要交流电起来了,就不去关机
			}
		}
	}
	else		//发生了停电,当前电源由电池供电		
	{
#ifdef SYS_LINUX	
		memset(bBuf, 0, sizeof(bBuf));
		WriteItemEx(BN0, PN0, 0xb61f, bBuf);
		WriteItemEx(BN0, PN0, 0xb62f, bBuf);
		WriteItemEx(BN0, PN0, 0xb63f, bBuf);
		WriteItemEx(BN0, PN0, 0xb64f, bBuf);
		WriteItemEx(BN0, PN0, 0xb65f, bBuf);
		WriteItemEx(BN0, PN0, 0xb66f, bBuf);
		WriteItemEx(BN0, PN0, 0xb6a0, bBuf);
		WORD wAcPn = GetAcPn();
		if (wAcPn != PN0)	//交采测量点
		{
			WriteItemEx(BN0, wAcPn, 0xb61f, bBuf);
			WriteItemEx(BN0, wAcPn, 0xb62f, bBuf);
			WriteItemEx(BN0, wAcPn, 0xb63f, bBuf);
			WriteItemEx(BN0, wAcPn, 0xb64f, bBuf);
			WriteItemEx(BN0, wAcPn, 0xb65f, bBuf);
			WriteItemEx(BN0, wAcPn, 0xb66f, bBuf);
			WriteItemEx(BN0, wAcPn, 0xb6a0, bBuf);
		}
#endif 

		g_dwAcUpClick = 0;
		
		if (g_dwAcOffClick == 0)
		{
			DTRACE(DB_CRITICAL, ("DoPowerManagement : BackBatOn!\n"));
			GprsBatOnOff(true);//打开后备电池
			BackBatOnOff(true); //打开后备电池
			g_dwAcOffClick = GetClick();
			g_fPowerOffSave = true;
			#ifdef VER_LIAONING
			InitLoopCnt();
			#endif
			return;
		}
		
		if (dwClick-g_dwAcOffClick >= 6)
		{
			if (g_fPowerOffSave && g_fAcUp && (dwClick-g_dwAcOffClick >= 30))	//掉电后30秒触发保存数据
			{
				TrigerSave();
				TrigerSavePara();
				//TdbTrigerSave();
				g_fPowerOffSave = false;
			}
				
			if (dwClick-g_dwAcOffClick >= 5*60)	//5*60 zqq add
			{	//以前是有电的,现在停电了,要等到停电告警被确认了才关断电池
				//或者 停电超过5分钟无论如何也得关电池
				//TdbDoSave();
				FaSave();	//电源能撑到现在,应该也有足够的电量来保存数据
				g_fFaShutDown = true;  //通知别的线程去关机
				Sleep(60*1000);        //等GPRS关机
				while (GetClick()-dwClick < 5)
				{
					Sleep(1000); //确保最后的数据项显示完毕
				}	
                DTRACE(DB_CRITICAL, ("DoPowerManagement : BackBatOff!\r\n"));
				Sleep(10*1000);        //等GPRS关机
				GprsBatOnOff(false); //关GPRS电源
				BackBatOnOff(false); //关闭后备电池
				DTRACE(DB_CRITICAL, ("DoPowerManagement : g_dwAcOffClick := 0!\n"));
				g_dwAcOffClick = 0;
				Sleep(3*60*1000); //不要在这里使用死循环，有可能中途又来电了，让它继续跑
			}

			#ifdef VER_LIAONING
			if (g_fAcUp==false && GetLoopCnt() >= 1 && dwClick-g_dwAcOffClick >= 30)
			#else
			if (g_fAcUp==false && dwClick-g_dwAcOffClick >= 2*60)
			#endif
			//if (g_fAcUp==false && dwClick-g_dwAcOffClick>=1*30)	//TO DO:轮显2周关电池 //zqq add
			{	//交流电从来没有起来过,则是按键唤醒,在轮显两周后关断电池
			
				//FaSave();	//按键唤醒就不保存数据了,避免损坏数据
				g_fFaShutDown = true;  //通知别的线程去关机
				while (GetClick()-dwClick < 5)
				{
					Sleep(500); //确保最后的数据项显示完毕
				}	

				GprsBatOnOff(false); //关GPRS电源
				BackBatOnOff(false); //关闭后备电池
				g_dwAcOffClick = 0;
				Sleep(10000); //不要在这里使用死循环，有可能中途又来电了，让它继续跑
			}
		}
	}
#endif
}



#define RESTART_SYSTEM	1	//重启系统
#define EXEC_INSTANCE	2	//通用方法，执行具体的事例
#define DATA_INIT		3	//数据初始化
#define PARAM_INIT		4	//参数初始化
#define EVENT_INIT		5	//事件初始化
#define DEMAND_INIT		6	//需量初始化


void DelSchData()
{
	char pszTabName[64];
	int iRet;
	TTaskCfg tTaskCfg;

	for (WORD wID=0; wID<TASK_ID_NUM; wID++)
	{
		memset(&tTaskCfg, 0, sizeof(tTaskCfg));
		if (GetTaskCfg(wID, &tTaskCfg))
		{
			if (tTaskCfg.bSchType == SCH_TYPE_EVENT)
			{
#ifndef SYS_WIN
				memset(pszTabName, 0, sizeof(pszTabName));
				sprintf(pszTabName, "rm -rf /mnt/data/data/%s_%03d_*", GetSchTableName(tTaskCfg.bSchType), tTaskCfg.bSchNo);
				system(pszTabName);
				DTRACE(DB_TASK, ("DelSchData: %s.\n", pszTabName));
#endif
			}
			else
			{
				memset(pszTabName, 0, sizeof(pszTabName));
				sprintf(pszTabName, "%s_%03d.dat", GetSchTableName(tTaskCfg.bSchType), tTaskCfg.bSchNo);
				iRet = TdbClearRec(pszTabName); 
				DTRACE(DB_TASK, ("DelSchData: %s, iRet=%d.\n", pszTabName, iRet));
			}
		}
	}
}

void DelSchPara()
{
	char pszTabName[64];
	TTaskCfg tTaskCfg;

	for (WORD wID=0; wID<TASK_ID_NUM; wID++)
	{
		memset(&tTaskCfg, 0, sizeof(tTaskCfg));
		if (GetTaskCfg(wID, &tTaskCfg))
		{
			memset(pszTabName, 0, sizeof(pszTabName));
			sprintf(pszTabName, "%s_%03d.para", GetSchTableName(tTaskCfg.bSchType), tTaskCfg.bSchNo);
			TdbClearRec(pszTabName); 
			memset(pszTabName, 0, sizeof(pszTabName));
			sprintf(pszTabName, "%s_%03d.dat", GetSchTableName(tTaskCfg.bSchType), tTaskCfg.bSchNo);
			TdbClearRec(pszTabName); 
		}
	}

#ifndef SYS_WIN
	system("rm -rf /mnt/data/data/CommSch*");
	system("rm -rf /mnt/data/data/EvtSch*");
	system("rm -rf /mnt/data/data/RptSch*");
	system("rm -rf /mnt/data/data/AcqRule*");
	system("rm -rf /mnt/data/data/RptSch*");
    system("rm -rf /mnt/data/data/TaskCfgUnit_*");    
#endif
}

void DelTaskPara()
{
	char pszTabName[64];
	TTaskCfg tTaskCfg;

	for (WORD wID=0; wID<TASK_ID_NUM; wID++)
	{
		memset(&tTaskCfg, 0, sizeof(tTaskCfg));
		if (GetTaskCfg(wID, &tTaskCfg))
		{
			memset(pszTabName, 0, sizeof(pszTabName));
			sprintf(pszTabName, "%TaskCfgUnit_%03d.para", tTaskCfg.bSchNo);
			TdbClearRec(pszTabName); 
		}
	}
}

void FaResetData_1()
{
	FaSavePara();
	LockDB();

	for (BYTE bSect=0; bSect<SECT_NUM; bSect++)
	{
		//if (bSect==SECT_ENERGY || bSect==SECT_EVENT_DATA || 
		//	bSect==SECT_FROZEN_DATA || bSect==SECT_VARIABLE)
		if (bSect==SECT_ENERGY || bSect==SECT_DEMAND || 
			bSect==SECT_FROZEN_DATA || bSect==SECT_VARIABLE)	//20170102 hyl 不需要清除事件DATA，事件部分会单独处理。
			ClearBankData(BN0, bSect);
	}
	ClearBankData(BN16, -1);
	DelSchData();
#ifndef SYS_WIN
	AcClearDemand();
	AcClearLog();
	InitSample();//面向对象不复位需要重新初始化内存变量
#endif
	UnLockDB();

	for (WORD wId=0; wId<TASK_ID_NUM; wId++)
		ClrTaskMoniStat(wId);

	DeleteMtrRdCtrl();
}

void FaResetPara()
{
	LockDB();

	for (BYTE bSect=0; bSect<SECT_NUM; bSect++)
	{
		if (bSect==SECT_EVENT_DATA)//20170102 hyl 不需要清除事件DATA，事件部分会单独处理。
			continue;
		ClearBankData(BN0, bSect);
	}
	DelSchPara();

	FaResetData_1();
}

void FaResetEvent()
{
	BYTE bSect;

	FaSavePara();
#if 0
	LockDB();
	for (bSect=0; bSect<SECT_NUM; bSect++)
	{
		if (bSect==SECT_EVENT || bSect==SECT_EVENT_DATA)
			ClearBankData(BN0, bSect);
	}
	UnLockDB();
#endif
}

void FaResetDemand()
{
	BYTE bSect;

	FaSavePara();
	LockDB();
	for (bSect=0; bSect<SECT_NUM; bSect++)
	{
		if (bSect == SECT_DEMAND)
			ClearBankData(BN0, bSect);
	}
#ifndef SYS_WIN
	AcClearDemand();
#endif
	UnLockDB();
}


//重新获取停电时间,避免立即产生停电事件, 在停电事件有效标志变为有效时、数据初始化和事件初始化时调用
void UpdateTermPowerOffTime()
{
	GetCurTime(&g_PowerOffTmp.tPoweroff);
	GetCurTime(&g_tPowerOn);
	g_PowerOffTmp.fAlrPowerOff = false;
	SavePoweroffTmp();
}

void FaInit(BYTE bMethod)
{
	switch (bMethod)
	{
	case RESTART_SYSTEM:
		FaSave();
		//TDoSave();
		SetInfo(INFO_HARDWARE_INIT);
		Sleep(10*1000);
		ResetCPU();
		break;
	case EXEC_INSTANCE:
		break;
	case DATA_INIT:
		FaResetData_1();
		MtrExcOnRxFaResetCmd();		//抄表事件数据清零
		FrzTaskOnRxFaResetCmd();	//冻结数据初始化
		SetInfo(INFO_RST_TERM_STAT); 
		SetInfo(INFO_HARDWARE_INIT);
		UpdateTermPowerOffTime();	//需要放在DealSpecTrigerEvt(MTR_MTRCLEAR)前面
		DealSpecTrigerEvt(MTR_MTRCLEAR);
		DealSpecTrigerEvt(TERM_INIT);//需要放在DealSpecTrigerEvt(MTR_MTRCLEAR)后面
		
		//Sleep(10*1000);
		//ResetCPU();	//ESAM数据初始化时不应该重启终端
		break;
	case PARAM_INIT:
		FaResetPara();
		InitTermEvt();
		MtrExcOnRxFaResetCmd();		//抄表事件数据清零
		FrzTaskOnRxFaResetCmd();	//冻结数据初始化
		SetInfo(INFO_RST_TERM_STAT); 
		UpdateTermPowerOffTime();
		DealSpecTrigerEvt(MTR_MTRCLEAR);
		DealSpecTrigerEvt(TERM_INIT);
		Sleep(10*1000);
		ResetCPU();	
		break;
	case EVENT_INIT:
		FaResetEvent();
		MtrExcOnRxFaResetCmd();		//抄表事件数据清零
		UpdateTermPowerOffTime();
		DealSpecTrigerEvt(MTR_EVTCLEAR);
		DealSpecTrigerEvt(TERM_INIT);
		//ResetCPU();
		Sleep(10*1000);
		break;
	case DEMAND_INIT:
		FaResetDemand();
		DealSpecTrigerEvt(MTR_DMDCLEAR);
		DealSpecTrigerEvt(TERM_INIT);
		Sleep(10*1000);
		break;
	default:
		DTRACE(DB_TASK, ("FaDataInit(): Method=%d unspport\n", bMethod));
	}
}

#define LED_ON	0
#define LED_OFF 1
void CheckSignStrength()
{
	static BYTE bLastSign = 0;
	BYTE bSignStren = GetSignStrength();

	if (bSignStren != bLastSign)
	{
		bLastSign = bSignStren;
		if (bSignStren >= 100)//TD的模块信号强度范围为100-199
		{
			bSignStren -= 100;		
			if (bSignStren != 99)
			{
				bSignStren = (bSignStren)*31/98 + 10;
				if (bSignStren > 31)
					bSignStren = 31;
			}
		}	
		if (bSignStren > 31)
			bSignStren = 0;	

		bSignStren = (BYTE)(bSignStren & 0x00FF);
		WriteItemEx(BN2, PN0, 0x1058, &bSignStren);

#ifndef SYS_WIN
#if 0
		if (bSignStren == 0)  //0全灭掉，作为拔掉模块，或者以太网时使用
		{
			SetGprsNetSignalLed1(LED_OFF);
			SetGprsNetSignalLed2(LED_OFF);
		}
		else if (bSignStren < 8)
		{
			SetGprsNetSignalLed1(LED_OFF);
			SetGprsNetSignalLed2(LED_ON);

		}
		else if ((bSignStren >= 8) && (bSignStren < 16))
		{
			SetGprsNetSignalLed1(LED_ON);
			SetGprsNetSignalLed2(LED_ON);
		}
		else
		{
			SetGprsNetSignalLed1(LED_ON);
			SetGprsNetSignalLed2(LED_OFF);
		}
#endif
#endif
	}
}



//描述:慢线程
TThreadRet SlowSecondThread(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("SlowSecond-thrd", 4*60*60);	//申请线程监控ID,更新间隔为5分钟

	DTRACE(DB_CRITICAL, ("SlowSecondThread : started!\n"));
	//SaveSoftVerChg();

	while (1)
	{
		if (GetInfo(INFO_MTR_UPDATE))
		{
			RequestThreadsSem();
			InitMtrMask();
			InitMtrCacheCtrl();
			DeleteMtrRdCtrl();
			SetInfo(INFO_SYNC_MTR);
			ReleaseThreadsSem();
		}

		if (GetInfo(INFO_TASK_CFG_UPDATE))
		{
			RequestThreadsSem();
			InitTaskMap();
			InitSchMap();
			InitSchTable();
			DeleteMtrRdCtrl();
			SetInfo(INFO_SYNC_MTR);
			InitMtrCacheCtrl();
			ReleaseThreadsSem();
		}

		if (GetInfo(INFO_ACQ_SCH_UPDATE))
		{
			RequestThreadsSem();
			InitSchMap();
			InitSchTable();
			DeleteMtrRdCtrl();
			SetInfo(INFO_SYNC_MTR);
			InitMtrCacheCtrl();
			ReleaseThreadsSem();
		}
		if (GetInfo(INFO_RP_SCH_UPDATE))
		{
			RequestThreadsSem();
			InitSchMap();
			InitSchTable();
			ClearBankData(BN16, 0, -1);
			ReleaseThreadsSem();
		}

		if (GetInfo(INFO_CLASS19_METHOD_RST))	
		{
			DTRACE(DB_FA, ("Dev interface class=19 method=1. Reset system...\n"));
			FaInit(RESTART_SYSTEM);
		}

		if (GetInfo(INFO_CLASS19_METHOD_EXE))	
		{
			DTRACE(DB_FA, ("Dev interface class=19 method=2. Exe...\n"));
			FaInit(EXEC_INSTANCE);
		}

		if (GetInfo(INFO_CLASS19_METHOD_DATA_INIT))	
		{
			DTRACE(DB_FA, ("Dev interface class=19 method=3. Data init...\n"));
			RequestThreadsSem();
			FaInit(DATA_INIT);
			ReleaseThreadsSem();
		}

		if (GetInfo(INFO_CLASS19_METHOD_RST_FACT_PARA))	
		{
			DTRACE(DB_FA, ("Dev interface class=19 method=4. Restore factory para...\n"));
			RequestThreadsSem();
			FaInit(PARAM_INIT);
			ReleaseThreadsSem();
		}

		if (GetInfo(INFO_CLASS19_METHOD_EVT_INIT))	
		{
			DTRACE(DB_FA, ("Dev interface class=19 method=5. Event init...\n"));
			FaInit(EVENT_INIT);
		}

		if (GetInfo(INFO_CLASS19_METHOD_DEM_INIT))	
		{
			DTRACE(DB_FA, ("Dev interface class=19 method=6. Demand init...\n"));
			FaInit(DEMAND_INIT);
		}		

		if (GetInfo(INFO_DISCONNECT)) //数据复位
		{
			DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_DISCONNECT......\n"));
			g_pGprsFaProtoIf->SetDisConnect(); //在收到外部的断开连接命令时,调用本函数通知接口
			g_pEthFaProtoIf->SetDisConnect();
		}

		DoMangerMtrCacheCtrl();
		
		DoFaSave();
		DoFapCmd();
		DoPowerManagement();
		CheckSignStrength();

		
		UpdThreadRunClick(iMonitorID);
		Sleep(1000);
	}	

	ReleaseThreadMonitorID(iMonitorID);
	return THREAD_RET_OK;
}

//控制模块线程--控制模块的LED灯，继电器，断线检测
//只针对“单片机类型”的控制模块 有效
TThreadRet CtrlMoudleThread(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("CtrlMoudleThread-thrd", 60*60*60);	//申请线程监控ID,更新间隔为60分钟

	DTRACE(DB_CRITICAL, ("CtrlMoudleThread : started.......\n"));
	BYTE bCnt = 0;
	GetCtrlVer();
	InitCtrlModule();

	while (1)
	{
		if (GetCtrlThreadState())
		{
			if (bCnt == 15)
			{
				DoCtrlLoop();//断线检测
				bCnt = 0;
			}

			DoAlrLedCtrl();//告警灯控制
			DoLedByPulse();//继电器--脉冲输出控制
			bCnt ++;
		}
		else
		{
			bCnt = 0;
		}
		Sleep(50);
		
		UpdThreadRunClick(iMonitorID);
	}

	return THREAD_RET_OK;
}
//目前端口号的转换都要经过下面的两个步骤:
//逻辑端口号wLogicPort-->内部固定序号wInSn-->物理端口号wPhyPort



void GetLogicPortNum(WORD* pwNum, WORD* pwMin, WORD* pwMax)
{
	*pwNum = LOGIC_PORT_NUM;
	*pwMin = LOGIC_PORT_MIN;
	*pwMax = LOGIC_PORT_MAX;
}

//描述:逻辑端口号到内部固定序号的转换
//参数:@wLogicPort 逻辑端口号
//返回:如果正确则返回内部固定序号,否则返回-1
int LogicPortToInSn(WORD wLogicPort)
{
	if (wLogicPort<LOGIC_PORT_MIN || wLogicPort>LOGIC_PORT_MAX)
		return -1;

	//1.逻辑端口号-->内部固定序号的转换
	WORD wInSn = wLogicPort + 1 - LOGIC_PORT_MIN;	//国网485口 2 3 4
		
	BYTE bOrder = 0;
	ReadItemEx(BN10, PN0, 0xa166, (BYTE*)&bOrder);	//0xa166 1 抄表口排列顺序,0:右->左分别是口1,口2...; 1:左->右分别是口1,口2...

	//如果是从右->左的顺序开始编号1,2...,则逻辑号==wMeterPort,不需要调整
	//如果是从左->右的顺序开始编号1,2...,则逻辑号!=wMeterPort,需要调整

	if (bOrder == 1)	//从左->右的顺序开始编号1,2...
		wInSn = LOGIC_PORT_NUM - wInSn + 1;

	return wInSn;
}

//描述:逻辑端口号到物理端口号的映射
//参数:@wLogicPort 逻辑端口号
//返回:如果正确则返回物理端口号,否则返回-1
int InSnToPhyPort(WORD wInSn)
{
	if (wInSn==0 || wInSn>LOGIC_PORT_NUM)	//非法的逻辑端口号
		return -1;
	else
		return g_iInSnToPhyPort[wInSn-1];
}

//描述:逻辑端口号到物理端口号的映射
//参数:@wLogicPort 逻辑端口号
//返回:如果正确则返回物理端口号,否则返回-1
int LogicPortToPhy(WORD wLogicPort)
{
	//1.逻辑端口号-->内部固定序号的转换
	int iInSn = LogicPortToInSn(wLogicPort);
	if (iInSn < 0)
		return iInSn;

	return InSnToPhyPort((WORD )iInSn);
}

//描述:取得内部固定序号端口功能配置
//备注:0xa131和0xa132配置的是逻辑端口,固定从右->左的顺序开始编号
int GetInSnPortFun(WORD wInSn)
{
	WORD wID;
	BYTE b;
	int iPortFun = -1;

	if (IsDownSoft())
		return -1;

	if (wInSn == 1)	//485-1口
		wID = 0xa131;
	else if (wInSn == 2)	//485-2口
		wID = 0xa132;
	else if (wInSn == 3)	//485-3口
		wID = 0xa180;
	else
		return -1;
	
	int iLen = ReadItemEx(BN10, PN0, wID, (BYTE*)&b);
	if (iLen <= 0)
		return -1;
	
	iPortFun = b;
		//liuzhixing20170526
	//		//3口用于调试输出
	//		if ((iPortFun==PORT_FUN_DEBUG||iPortFun==PORT_FUN_LOCAL485) && wInSn==3)	//485-3口用于调试口或维护口，不抄表
	//			return -1;

		
	if (iPortFun<0 || iPortFun>PORT_FUN_JC485) //配置成非法数据都默认成抄表口
		iPortFun = PORT_FUN_RDMTR;
		
#ifdef EN_SPECIAL_TRANSFORMER
	if (iPortFun > PORT_FUN_LINK)
		iPortFun = PORT_FUN_RDMTR;
#endif

	return iPortFun;
}

//描述:取得逻辑端口号的功能配置
int GetLogicPortFun(WORD wLogicPort)
{
	//1.逻辑端口号-->内部固定序号的转换
	int iInSn = LogicPortToInSn(wLogicPort);
	if (iInSn < 0)
		return iInSn;

	return GetInSnPortFun((WORD )iInSn);
}

//描述:设置内部固定序号端口功能配置
int SetInSnPortFun(WORD wInSn, BYTE bPortFun)
{
	WORD wID;

	if (IsDownSoft())
		return -1;

	if (wInSn == 1)	//485-1口
		wID = 0xa131;
	else if (wInSn == 2)	//485-2口
		wID = 0xa132;
	else if (wInSn == 3)	//485-3口
		wID = 0xa180;
	else
		return -1;
	
	WriteItemEx(BN10, PN0, wID, (BYTE*)&bPortFun);
	return 0;
}

//描述:设置逻辑端口的功能配置
int SetLogicPortFun(WORD wLogicPort, BYTE bPortFun)
{
	//1.逻辑端口号-->内部固定序号的转换
	int iInSn = LogicPortToInSn(wLogicPort);
	if (iInSn < 0)
		return iInSn;

	return SetInSnPortFun((WORD )iInSn, bPortFun);
}

//描述:抄表口的逻辑端口到物理端口的映射
//返回:如果正确物理端口号,否则返回负数
int MeterPortToPhy(WORD wMeterPort)
{
	if (GetLogicPortFun(wMeterPort) == PORT_FUN_RDMTR)
		return LogicPortToPhy(wMeterPort);

	return -1;
}


bool IsMtr485Port(BYTE bPort)
{
	if (GetLogicPortFun(bPort) == PORT_FUN_RDMTR)
		return true;
	
	return false;
}

//描述:级联口的逻辑端口到物理端口的映射
//返回:如果正确物理端口号,否则返回负数
int GetLinkPhyPort()
{	
	for (WORD i=LOGIC_PORT_MIN; i<=LOGIC_PORT_MAX; i++)	//按照硬件上的3个逻辑端口进行循环
	{
		if (GetLogicPortFun(i) == PORT_FUN_LINK)
			return LogicPortToPhy(i);
	}

	return -1;
}


//描述:被抄表口的逻辑端口到物理端口的映射
//返回:如果正确物理端口号,否则返回负数
int GetInMtrPhyPort()
{
	for (WORD i=LOGIC_PORT_MIN; i<=LOGIC_PORT_MAX; i++)	//按照硬件上的3个逻辑端口进行循环
	{
		if (GetLogicPortFun(i) == PORT_FUN_INMTR)
			return LogicPortToPhy(i);
	}

	return -1;
}

//描述:采集口的逻辑端口到物理端口的映射
//返回:如果正确物理端口号,否则返回负数
int GetAcqPhyPort()
{
	#ifdef SYS_WIN
		return COMM_DEBUG;
	#endif
	for (WORD i=LOGIC_PORT_MIN; i<=LOGIC_PORT_MAX; i++)	//按照硬件上的3个逻辑端口进行循环
	{
		int iPortFun = GetLogicPortFun(i);
		if (iPortFun==PORT_FUN_ACQ ||  iPortFun==PORT_FUN_JC485)
			return LogicPortToPhy(i);
	}

	return -1;
}

//描述:判断逻辑口是否为485采集器口
//返回:是485采集器口返回true
bool IsAcqLogicPort(BYTE bPort)
{
	int iPortFun = GetLogicPortFun(bPort);
	if (iPortFun==PORT_FUN_ACQ || iPortFun==PORT_FUN_JC485)
		return true;
	
	return false;
}

//数据区初始化
void FaResetData()
{
#if 0
	int i;
	DTRACE(DB_FAPROTO, ("FaResetData: started...\n"));
	
	FaSave();
	SetInfo(INFO_RST_TERM_STAT); //复位终端统计数据
	
	LockDB();
	for (i=0; i<SECT_NUM; i++)
	{
		if (i==SECT_KEEP_PARA || i==SECT_TERMN_PARA || i==SECT_PN_PARA || i==SECT_PN_DATA || i==SECT_EXT_PN_DATA)  //参数文件和交采计量数据除外
			continue;
			
		ClearBankData(BN0, i);
	}

	for (i=0; i<POINT_NUM; i++)	//清测量点数据
	{
		if (i == PN0 || i == GetAcPn()) //测量点0和交采测量点计量数据除外
			continue;

		ClearBankData(BN0, SECT_PN_DATA, i);
		ClearBankData(BN0, SECT_EXT_PN_DATA, i);
	}

	ClearBankData(BN11, 0);	//国标版中间数据
	ClearBankData(BN18, 0);	//国标版中间数据
	
	DTRACE(DB_FAPROTO, ("FaResetData: clr tdb files\n"));
	
#ifdef EN_CCT	//允许集抄功能
	FaResetCctData();
#endif

#ifndef SYS_WIN	
	AcClearLog();
	g_PulseManager.ClearLog();
#endif
	
	//ResetCPU(); //国标协议里不能在这里复位,因为接着需要生成事件
#endif
}

#ifdef EN_CCT	//允许集抄功能
//描述:清除集中器数据
void FaResetCctData()
{
    ClearBankData(BN16, 0); //集抄冻结状态字
	//ClearBankData(BN20, 0); //集抄普通表清零.
    ClearBankData(BN21, 0); //集抄多功能表表清零.
}

//描述:集中器参数初始化
void FaResetCctPara()
{
	SetInfo(INFO_PLC_CLRRT); //清路由
}
#endif

//参数区初始化
void FaResetDataPara()
{
#if 0
	int i;
	DTRACE(DB_FAPROTO, ("FaResetDataPara: started...\n"));
	
	SetInfo(INFO_RST_TERM_STAT); //复位终端统计数据
#ifdef EN_CCT	//允许集抄功能
	SetInfo(INFO_PLC_CLRRT);	 //集中器清除路由
#endif
	LockDB();
	ClearWDG();
	for (i=0; i<SECT_NUM; i++)
	{
		if (i==SECT_KEEP_PARA || i==SECT_PN_DATA || i==SECT_EXT_PN_DATA) //保留参数文件和交采计量数据除外
			continue;

		ClearBankData(BN0, i);
	}

	for (i=0; i<POINT_NUM; i++)	//清测量点数据
	{
		if (i == PN0 || i == GetAcPn()) //测量点0和交采测量点计量数据除外
			continue;

		ClearBankData(BN0, SECT_PN_DATA, i);
		ClearBankData(BN0, SECT_EXT_PN_DATA, i);
	}

	ClearBankData(BN11, 0);	//国标版中间数据
	ClearBankData(BN18, 0);	//国标版中间数据
	//国标扩展参数,是我们针对每个地方的不同而做的设置,所以不用清除
	
	DTRACE(DB_FAPROTO, ("FaResetDataPara: clr tdb files\n"));
	
#ifdef EN_CCT	//允许集抄功能
	FaResetCctData();
	FaResetCctPara();
#endif
#ifndef SYS_WIN	
//	g_AcSample.ClearLog();
	g_PulseManager.ClearLog();
#endif
	
//	ResetCPU(); //国标协议里不能在这里复位,因为接着需要生成事件
#endif
}

//全部参数区初始化
void FaResetAllDataPara()
{
#if 0
	int i;
	DTRACE(DB_FAPROTO, ("FaResetDataPara: started...\n"));
	
	SetInfo(INFO_RST_TERM_STAT); //复位终端统计数据
	
	LockDB();
	ClearWDG();
	for (i=0; i<SECT_NUM; i++)//含保留参数均清除
	{	
		if (i == SECT_KEEP_PARA) //保留参数文件除外
			continue;
		ClearBankData(BN0, i);
	}
		
	ClearBankData(BN11, 0);	//国标版中间数据
	ClearBankData(BN18, 0);	//国标版中间数据
	//国标扩展参数,是我们针对每个地方的不同而做的设置,所以不用清除
	
	DTRACE(DB_FAPROTO, ("FaResetDataPara: clr tdb files\n"));
	
#ifndef SYS_WIN	
	AcClearLog();
#endif
	
//	ResetCPU(); //国标协议里不能在这里复位,因为接着需要生成事件
#endif
}
//清电能命令
void FaClearEnergy()
{
#if 0
	int i;
	DTRACE(DB_FAPROTO, ("FaClearEnergy: started...\n"));

	SetInfo(INFO_RST_TERM_STAT); //复位终端统计数据

	LockDB();
	ClearBankData(BN0, SECT_PN_DATA, POINT0);
	ClearBankData(BN0, SECT_PN_DATA, GetAcPn());

	ClearBankData(BN0, SECT_EXT_PN_DATA, PN0);
	ClearBankData(BN0, SECT_EXT_PN_DATA, GetAcPn());
	
#ifdef EN_AC	
	AcClearLog();
#endif

	//ResetCPU(); //国标协议里不能在这里复位,因为接着需要生成事件
#endif
}

//描述:初始化应用部分,有些应用可能被多个线程调用,初始化应避免放到某个线程中,
//	   因为线程的执行先后无法确定,可能会导致异常,应放到本函数,因为本函数在主线程中调用
// 	   初始化会在一个同步而非异步的方式下进行
void InitApp()
{
	g_StatMgr.Init();	 //统计
	ReSetParamKeepReadFile();
}
#ifdef EN_INMTR
void DoAmpHour();
#endif
//主线程
extern TThreadRet RJ45ReadMtrThread(void* pvPara);
//是否使用基于窄频带的宽带PLC抄表模式
bool IsBBPlcMode()
{
	BYTE bPlcMode;
	ReadItemEx(BN15, PN0, 0x5006, &bPlcMode);	//载波模块类型
	//if (bPlcMode == AR_LNK_PRIME || bPlcMode == AR_LNK_G3)
	{
		return true;
	}

	return false;
}

//初始化交采数据
void InitAcData()
{
	WORD wPn = GetAcPn();
	BYTE bInValidDemTime[DATETIMELEN] = {0xff, 0x00, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	BYTE bBuf[100];
	for (int i=0; i<5; i++)
	{
		memcpy(bBuf+DATETIMELEN*i, bInValidDemTime, DATETIMELEN);
	}
	WriteItemEx(BN0, PN0, 0xb01f, bBuf);
	WriteItemEx(BN0, PN0, 0xb02f, bBuf);
	WriteItemEx(BN0, PN0, 0xb11f, bBuf);
	WriteItemEx(BN0, PN0, 0xb12f, bBuf);
	WriteItemEx(BN0, PN0, 0xb13f, bBuf);
	WriteItemEx(BN0, PN0, 0xb14f, bBuf);
	WriteItemEx(BN0, PN0, 0xb15f, bBuf);
	WriteItemEx(BN0, PN0, 0xb16f, bBuf);
	if (wPn != PN0)
	{
		WriteItemEx(BN0, wPn, 0xb01f, bBuf);
		WriteItemEx(BN0, wPn, 0xb02f, bBuf);
		WriteItemEx(BN0, wPn, 0xb11f, bBuf);
		WriteItemEx(BN0, wPn, 0xb12f, bBuf);
		WriteItemEx(BN0, wPn, 0xb13f, bBuf);
		WriteItemEx(BN0, wPn, 0xb14f, bBuf);
		WriteItemEx(BN0, wPn, 0xb15f, bBuf);
		WriteItemEx(BN0, wPn, 0xb16f, bBuf);
	}

	ReadItemEx(BN0, PN0, 0x40C0, bBuf);
	DWORD dwDay = bBuf[1]+(bBuf[2]<<8)+(bBuf[3]<<16);
	BYTE b;
	for (b=0; b<31; b++)
	{
		if ((dwDay&(0x01<<b))!=0)
		{
			break;
		}
	}
	if (b<31)//抄表日/结算日
	{
		bInValidDemTime[8] = b+1;
	}
	memcpy(&bInValidDemTime[3], &bBuf[5], 4);
	WriteItemEx(BN0, wPn, 0xc117, bInValidDemTime);
	if (wPn != PN0)
		WriteItemEx(BN0, PN0, 0xc117, bInValidDemTime);
}

extern TThreadRet DisplayThread(void* pvArg);
extern void SimuAcData();
TThreadRet MainThread(void* pvPara)
{
	SysInitDebug();	//初始化不同系统下调试最低层的部分
#ifdef SYS_LINUX
	sys_signal_setup();
#endif
	InitDrivers(HW_CL790D82);
	InitThreadMonitor(); //初始化线程监控
	int iMonitorID = ReqThreadMonitorID("main-thrd", 60*60);	//申请线程监控ID,更新间隔为10分钟

	DTRACE(DB_CRITICAL, ("MainThread : started V1.58!\n"));

	FaInitStep1();
	FaInitDrivers(); //这部分驱动相关的初始化,用到数据库,所以必须放到FaInitStep1()后
	InitDrvPara();
	InitApp();

	InitMtrMask();
	InitTaskMap();
	InitSchMap();
	InitSchTable();

	InitMeter();
	InitProto();

	InitTask(true);
	InitMtrExc();
	
	InitTermEvt();
	SaveSoftVerChg();	//做终端版本变更事件，需要放在InitTermEvt()之后

#ifdef EN_CTRL	//是否允许控制功能
//	if (IsFkTermn())
//		g_LoadCtrl.Init();
#endif

	InitAcData();
#ifndef SYS_WIN
	NewThread(WdgThread, NULL, 8192, THREAD_PRIORITY_ABOVE_NORMAL);
#ifdef EN_AC	//是否允许交采功能
	InitSample();
#endif
#endif
//	InitVBreakStat();

#ifdef EN_ETHTOGPRS //允许以太网和GPRS相互切换,先检查下网络吧，因为下面的通信线程要用到结果
	NewThread(CheckNetThread,  NULL, 8192, THREAD_PRIORITY_NORMAL);
#endif

	if (IsDownSoft())
	{
		NewFaUpdateThread();
		NewThread(DownSoftSecondThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
		NewThread(DriverThread, NULL, 8192, THREAD_PRIORITY_NORMAL);

	#ifndef SYS_WIN
		EnableTrace(false);
		if (g_pLcd != NULL)
		{
			g_pLcd->Clear();			
			if( g_bRemoteDownIP[0] == 0xff )
				g_pLcd->Print("\r本地sftp下载", 0, 3,false,true);
			else
				g_pLcd->Print("\r远程sftp下载", 0, 3,false,true);
			g_pLcd->Print("\r正在下载程序,请稍候...", 0, 4,false,true);
			g_pLcd->Refresh();
		}
	#endif

		DWORD dwUpdateTime = 0;
		while(1)
		{
			UpdThreadRunClick(iMonitorID);

			Sleep(1000);
			dwUpdateTime++;
			if(dwUpdateTime > 3*60*60) //最长下载3个小时
				ResetCPU();
		}	
	}
#ifndef SYS_WIN

#ifdef  EN_AC
	NewThread(AcThread, NULL, 8192, THREAD_PRIORITY_ABOVE_NORMAL);
#endif

	NewThread(DriverThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
	
#endif
	NewThread(DisplayThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
	NewThread(FastSecondThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
	NewThread(SlowSecondThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
//	NewThread(TaskThread, NULL, 8192*2, THREAD_PRIORITY_NORMAL);
//	NewThread(DataProcThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
	NewThread(DataStatThread, NULL, 8192, THREAD_PRIORITY_NORMAL);

#ifdef EN_CTRL	//是否允许控制功能
	if (IsFkTermn())	//负控终端才需要做总加组数据的处理
	{
		NewThread(CtrlMoudleThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
		NewThread(LoadCtrlThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
	}
#endif

#ifdef EN_VARCPS //是否允许无功补偿(VAR compensator)功能
//	NewThread(LoadWGCtrlThread, NULL, 8192, THREAD_PRIORITY_NORMAL);			
#endif

	NewMeterThread();
	NewFaProtoThread();
	//NewThread(TaskDBThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
	
	NewCctThread();

#ifdef EN_INMTR
//	NewInMtrThread();
#endif

#ifdef EN_ETHTOGPRS //允许以太网和GPRS相互切换，这个线程放最后的问题就是很可能一开始以太网已经上线，这个一起来又会掉一次再重新连
	NewThread(EthernetSwitchThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
#endif

#if 0
	T645IdInfo t645IdInfo;
	BYTE bRCSD[32] = {0};
	BYTE *pbRCSD = bRCSD;

	*pbRCSD++ = 0x01;
	*pbRCSD++ = 0x50;
	*pbRCSD++ = 0x04;
	*pbRCSD++ = 0x02;
	*pbRCSD++ = 0x00;
	*pbRCSD++ = 0x01;
	*pbRCSD++ = 0x00;
	*pbRCSD++ = 0x10;
	*pbRCSD++ = 0x02;
	*pbRCSD++ = 0x00;
	memset((BYTE*)&t645IdInfo, 0, sizeof(t645IdInfo));
	SearchAcqRule645ID(bRCSD, 0x02, &t645IdInfo);
#endif

	NewThread(Do485MeterSearch, NULL, 8192, THREAD_PRIORITY_NORMAL);
	int cnt=0;
	while (1)
	{
		UpdThreadRunClick(iMonitorID);
		DoTermEvt();
		
		DoFrzTasks();

		CheckDownSoft();

#ifdef SYS_WIN
		//SimuAcData();
#endif	
		Sleep(500);
	}	
	
	SysCleanup();

	return THREAD_RET_OK;
}
void InitTestMode()
{
  	BYTE bVal;
	g_fTestMode = false;
	ReadItemEx(BN10, PN0, 0xa1a8, &bVal);
	if (bVal == 1 )
	{//测试模式
		g_fTestMode = true;
	}
	return;
}

bool IsFkTermn()
{
	BYTE bRunMode;
	ReadItemEx(BN1, PN0, 0x2100, &bRunMode);
	if (bRunMode == 2)
		return true;
	else
		return false;
}
#ifdef SYS_WIN
#define ENERGY_TYPE_MAX		64
#define AC_ENERGY_NUM   	39//8  //交采的电能类型数量
#define DEMAND_TYPE_MAX   64
#define AC_DEMAND_NUM   	34	//交采的需量类型数量	
bool IsAcEngSign(WORD wOI)
{
	if ((wOI>=0x0030 && wOI<=0x0043) || (wOI>=0x0630 && wOI<=0x0643) || (wOI==0x0001))
		return true;
	else
		return false;
}

WORD AcEpToFmt(int64 val, BYTE* pbBuf, bool fHigPre, bool fSign)
{
	val = ABS(val);
	if (fHigPre)
	{
		if (fSign)	//带符号的
			pbBuf[0] = DT_LONG64;
		else
			pbBuf[0] = DT_LONG64_U;
			
//			memcpy(&pbBuf[1], (BYTE*)&val, 8);
		OoInt64ToLong64(val,&pbBuf[1]);
		return 9;//4;				
	}
	else
	{
		val /= 100;	//降低精度，保留两位小数即可
		if (fSign)
			pbBuf[0] = DT_DB_LONG;
		else
			pbBuf[0] = DT_DB_LONG_U;
			
//			memcpy(&pbBuf[1], (BYTE*)&val, 4);
		OoDWordToDoubleLongUnsigned(val,&pbBuf[1]);
		return 5;//4;		
	}
}



bool IsDemFmt5(WORD wOI)
{
	if ((wOI>=0x1030 && wOI<=0x1043) || (wOI>=0x1130 && wOI<=0x1143))
		return true;
	else
		return false;		
}

WORD AcDemandToFmt(DWORD dwDemand, BYTE* pbTime, BYTE* pbBuf, bool fFmt5)
{
	*pbBuf++ = 0x02;
	*pbBuf++ = 0x02;
	
	if (fFmt5)
		*pbBuf++ = DT_DB_LONG;
	else
		*pbBuf++ = DT_DB_LONG_U;
		
//		memcpy(pbBuf, (BYTE *)&dwDemand, 4);
	OoIntToDoubleLong(dwDemand,pbBuf);
	pbBuf += 4;
	
	*pbBuf++ = DT_DATE_TIME_S;//发生时间DateTimeBCD_S
	memcpy(pbBuf, pbTime, 7);//数据入库前已经转换过，所以直接入库	
//		OoIntToDoubleLong(dwDemand,pbBuf);
	
	return 15;
}



void SimuAcData()
{
	int i, j;
	static DWORD dwClick = 0;
	BYTE bPhase;
	BYTE bBuf[300];

	BYTE bVoltBuf[] = {01,03,18,0x98,0x08,18,0x98,0x08,18,0x98,0x08};//电压220.0
	WriteItemEx(BN0, PN0, 0x2000, bVoltBuf);

	BYTE bCurrBuf[] = {01,04,
			5,0x88,0x13,0x00,0x00,
			5,0x88,0x13,0x00,0x00,
			5,0x88,0x13,0x00,0x00,
			5,0x00,0x00,0x00,0x00};//电流:5.000A
	WriteItemEx(BN0, PN0, 0x2001, bCurrBuf);

	BYTE bVoltAng[] = {01,03,18,0x00,0x00,18,0xb0,0x04,18,0x60,0x09};//电压相角：0.0，120.0，240.0
	WriteItemEx(BN0, PN0, 0x2002, bVoltAng);

	BYTE bCurAng[] = {01,03,18,0x58,0x02,18,0x58,0x02,18,0x58,0x02};//电流相角：60.0，60.0，60.0
	WriteItemEx(BN0, PN0, 0x2003, bCurAng);

	BYTE bPDataBuf[] = {01,04,5,0xe8,0x80,0x00,0x00,5,0xf8,0x2a,0x00,0x00,5,0xf8,0x2a,0x00,0x00,5,0xf8,0x2a,0x00,0x00};	//功率3.3000，1.1000，1.1000，1.1000，单位-1W
	WriteItemEx(BN0, PN0, 0x2004, bPDataBuf);
	WriteItemEx(BN0, PN0, 0x2007, bPDataBuf);

	BYTE bQDataBuf[] = {01,04,5,0x28,0x23,0x00,0x00,5,0xb8,0x0b,0x00,0x00,5,0xb8,0x0b,0x00,0x00,5,0xb8,0x0b,0x00,0x00};	//功率0.9000，0.3000，0.3000，0.3000，单位-1Var
	WriteItemEx(BN0, PN0, 0x2005, bQDataBuf);
	WriteItemEx(BN0, PN0, 0x2008, bQDataBuf);

	BYTE bSDataBuf[] = {01,04,5,0xa0,0x8c,0x00,0x00,5,0xe0,0x2e,0x00,0x00,5,0xe0,0x2e,0x00,0x00,5,0xe0,0x2e,0x00,0x00};	//功率3.6000，1.2000，1.2000，1.2000，单位-1VA
	WriteItemEx(BN0, PN0, 0x2006, bSDataBuf);
	WriteItemEx(BN0, PN0, 0x2009, bQDataBuf);

	BYTE bCosDataBuf[] = {01,04,16,0xe7,0x03,16,0xe7,0x03,16,0xe7,0x03,16,0xe7,0x03};	//功率因数，单位-3 0.999
	WriteItemEx(BN0, PN0, 0x200a, bCosDataBuf);
	
	BYTE bFreqDataBuf[] = {01,01,18,0x88,0x13};	//电网频率，单位-2 50.00
	WriteItemEx(BN0, PN0, 0x200f, bFreqDataBuf);


	BYTE bPDemandDataBuf[] = {0x01,0x01,0x5,0xa0,0x8c,0x00,0x00,};	//功率3.6000，单位-4kWh
	WriteItemEx(BN0, PN0, 0x2017, bPDemandDataBuf);
	BYTE bQDemandDataBuf[] = {0x01,0x01,0x5,0xa0,0x8c,0x00,0x00,};	//功率3.6000，单位-4kvar
	WriteItemEx(BN0, PN0, 0x2018, bQDemandDataBuf);

	

	int64 m_i64E[ENERGY_TYPE_MAX][RATE_NUM+1]; //与数据库对应的电能
	static WORD g_wAcCurEnergyID[3][AC_ENERGY_NUM] = 
	{
		//入库的数据项ID
		{
			0x0610, 0x0620, 						//正、反向有功
			0x0630, 0x0640, 						//组合无功1、2
			0x0650, 0x0660, 0x0670, 0x0680, 		//一二三四象限无功
			
			0x0611, 0x0612, 0x0613, 				//A/B/C分相正向有功电能
			0x0621, 0x0622, 0x0623, 				//A/B/C分相反向有功电能
			0x0631, 0x0632, 0x0633, 				//A/B/C组合无功1电能
			0x0641, 0x0642, 0x0643, 				//A/B/C组合无功2电能
			
			0x0601, 								//组合有功	
	
			0x0631, 0x0632, 0x0633, 				//A/B/C分相组合无功1
			0x0641, 0x0642, 0x0643, 				//A/B/C分相组合无功2
	
			0x0651, 0x0652, 0x0653, 				//A/B/C分相一象限无功
			0x0661, 0x0662, 0x0663, 				//A/B/C分相二象限无功
			0x0671, 0x0672, 0x0673, 				//A/B/C分相三象限无功
			0x0681, 0x0682, 0x0683, 				//A/B/C分相四象限无功
	//暂时不做视在
	//			0x0690, 0x06a0, 						//正/反向视在电能
	//			0x0691, 0x0692, 0x0693, 				//A/B/C分相正向视在电能
	//			0x06a1, 0x06a2, 0x06a3, 				//A/B/C分相反向视在电能
			
		}, 
		
		//对应的内部计算ID
		{
			EP_POS_ABC, EP_NEG_ABC, 				//正、方向有功
			EQ_COM_ABC1, EQ_COM_ABC2,				//总组合无功1、2
			EQ_Q1, EQ_Q2, EQ_Q3, EQ_Q4, 			//一二三四象限无功
	
			EP_POS_A,EP_POS_B,EP_POS_C, 			//A/B/C分相正向有功电能
			EP_NEG_A,EP_NEG_B,EP_NEG_C, 			//A/B/C分相反向有功电能
	
			EQ_POS_A,EQ_POS_B,EQ_POS_C, 			//A/B/C分相感性无功电能
			EQ_NEG_A,EQ_NEG_B,EQ_NEG_C, 			//A/B/C分相容性无功电能 	
			EP_COM_ABC, 							//组合有功总
			EQ_COM_A1,EQ_COM_B1,EQ_COM_C1,			//A/B/C分相组合无功1
			EQ_COM_A2,EQ_COM_B2,EQ_COM_C2,			//A/B/C分相组合无功2
	
			EQ_Q1_A, EQ_Q1_B, EQ_Q1_C,				//A/B/C分相一象限无功
			EQ_Q2_A, EQ_Q2_B, EQ_Q2_C,				//A/B/C分相二象限无功
			EQ_Q3_A, EQ_Q3_B, EQ_Q3_C,				//A/B/C分相三象限无功
			EQ_Q4_A, EQ_Q4_B, EQ_Q4_C,				//A/B/C分相四象限无功
	
	//			ES_POS_ABC, ES_NEG_ABC, 				//正、方向视在
	//			ES_POS_A,ES_POS_B,ES_POS_C, 			//A/B/C分相正向视在电能
	//			ES_NEG_A,ES_NEG_B,ES_NEG_C, 			//A/B/C分相反向视在电能
	
			
		},
		{
			0x0010, 0x0020, 								//正、反向有功
			0x0030, 0x0040, 								//组合无功1、2
			0x0050, 0x0060, 0x0070, 0x0080, //一二三四象限无功
			
			0x0011, 0x0012, 0x0013, 				//A/B/C分相正向有功电能
			0x0021, 0x0022, 0x0023, 				//A/B/C分相正向有功电能
			0x0031, 0x0032, 0x0033, 				//A/B/C分相正向有功电能
			0x0041, 0x0042, 0x0043, 				//A/B/C分相正向有功电能
			
			0x0001,//组合有功	
			0x0031, 0x0032, 0x0033, 		//A/B/C分相组合无功1
			0x0041, 0x0042, 0x0043, 		//A/B/C分相组合无功2
	
			0x0051, 0x0052, 0x0053, 				//A/B/C分相一象限无功
			0x0061, 0x0062, 0x0063, 				//A/B/C分相二象限无功
			0x0071, 0x0072, 0x0073, 				//A/B/C分相三象限无功
			0x0081, 0x0082, 0x0083, 				//A/B/C分相四象限无功
	
	//			0x0090, 0x00a0, 						//正/反向视在电能
	//			0x0091, 0x0092, 0x0093, 				//A/B/C分相正向视在电能
	//			0x00a1, 0x00a2, 0x00a3, 				//A/B/C分相反向视在电能
		}
	};
	//准备电量数据
	for(i=0;i<AC_ENERGY_NUM;i++)
	{
		m_i64E[g_wAcCurEnergyID[1][i]][0] = 0x0000+((DWORD)g_wAcCurEnergyID[1][i]<<16);
		m_i64E[g_wAcCurEnergyID[1][i]][1] = 0x0101+((DWORD)g_wAcCurEnergyID[1][i]<<16);
		m_i64E[g_wAcCurEnergyID[1][i]][2] = 0x0202+((DWORD)g_wAcCurEnergyID[1][i]<<16);
		m_i64E[g_wAcCurEnergyID[1][i]][3] = 0x0303+((DWORD)g_wAcCurEnergyID[1][i]<<16);
		m_i64E[g_wAcCurEnergyID[1][i]][4] = 0x0404+((DWORD)g_wAcCurEnergyID[1][i]<<16);
	}
	BYTE *p = bBuf;
	WORD wLen;
	for(i=0;i<AC_ENERGY_NUM;i++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = 0x01;
		bBuf[1] = (RATE_NUM+1);
		p = bBuf+2;
		for(j=0;j<(RATE_NUM+1);j++)
		{
			if (IsAcEngSign(g_wAcCurEnergyID[2][i]))
				wLen = AcEpToFmt(m_i64E[g_wAcCurEnergyID[1][i]][j], p, false, true);
			else
				wLen = AcEpToFmt(m_i64E[g_wAcCurEnergyID[1][i]][j], p, false, false);
			p += wLen;
		}	
		WriteItemEx(BN0, PN0, g_wAcCurEnergyID[2][i], bBuf);
	}

	p = bBuf;
	for(i=0;i<AC_ENERGY_NUM;i++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = 0x01;
		bBuf[1] = (RATE_NUM+1);
		p = bBuf+2;
		for(j=0;j<(RATE_NUM+1);j++)
		{
			if (IsAcEngSign(g_wAcCurEnergyID[2][i]))
				wLen = AcEpToFmt(m_i64E[g_wAcCurEnergyID[1][i]][j], p, true, true);
			else
				wLen = AcEpToFmt(m_i64E[g_wAcCurEnergyID[1][i]][j], p, true, false);
			p += wLen;
		}	
		WriteItemEx(BN0, PN0, g_wAcCurEnergyID[0][i], bBuf);
	}

	
	static WORD g_wAcCurDemandID[2][AC_DEMAND_NUM] =
	{
		//入库的数据项ID
		{
			0x1010, 0x1020, 						//正、反向有功最大需量
			0x1030, 0x1040, 						//组合无功1、2最大需量
			0x1050, 0x1060, 0x1070, 0x1080, 		//一二三四象限无功最大需量
			
			0x1011, 0x1012, 0x1013, 				//A/B/C分相正向有功电能
			0x1021, 0x1022, 0x1023, 				//A/B/C分相反向有功电能
			0x1031, 0x1032, 0x1033, 				//A/B/C组合无功1电能
			0x1041, 0x1042, 0x1043, 				//A/B/C组合无功2电能	
			0x1051, 0x1052, 0x1053, 				//A/B/C一象限无功	
			0x1061, 0x1062, 0x1063, 				//A/B/C二象限无功	
			0x1071, 0x1072, 0x1073, 				//A/B/C三象限无功	
			0x1081, 0x1082, 0x1083, 				//A/B/C四象限无功	
	
			0x2017, 0x2018, 						//绝对值有功无功需量	
	//			0x1090, 0x10a0, 						//正/反向视在电能
	//			0x1091, 0x1092, 0x1093, 				//A/B/C分相正向视在电能
	//			0x10a1, 0x10a2, 0x10a3, 				//A/B/C分相反向视在电能
	
		},
		
		//对应的内部计算ID
		{
			EP_POS_ABC, EP_NEG_ABC, 		//正、方向有功
			EQ_COM_ABC1, EQ_COM_ABC2,		//组合无功1、2最大需量
			EQ_Q1, EQ_Q2, EQ_Q3, EQ_Q4, 	//一二三四象限无功最大需量
			
			EP_POS_A,EP_POS_B,EP_POS_C, 	//A/B/C分相正向有功电能
			EP_NEG_A,EP_NEG_B,EP_NEG_C, 	//A/B/C分相反向有功电能
	
			EQ_COM_A1,EQ_COM_B1,EQ_COM_C1,		//A/B/C分相组合无功1电能
			EQ_COM_A2,EQ_COM_B2,EQ_COM_C2,		//A/B/C分相组合无功2电能	
			
			EQ_Q1_A, EQ_Q1_B, EQ_Q1_C,				//A/B/C分相一象限无功
			EQ_Q2_A, EQ_Q2_B, EQ_Q2_C,				//A/B/C分相二象限无功
			EQ_Q3_A, EQ_Q3_B, EQ_Q3_C,				//A/B/C分相三象限无功
			EQ_Q4_A, EQ_Q4_B, EQ_Q4_C,				//A/B/C分相四象限无功
			EP_ABS_ABC,EQ_ABS_ABC,					//绝对值有功无功需量	
	//			ES_POS_ABC, ES_NEG_ABC, 				//正、方向视在
	//			ES_POS_A,ES_POS_B,ES_POS_C, 			//A/B/C分相正向视在电能
	//			ES_NEG_A,ES_NEG_B,ES_NEG_C, 			//A/B/C分相反向视在电能
		}
	};
	DWORD m_dwDemand[DEMAND_TYPE_MAX][RATE_NUM+1]; 
	BYTE  m_bTime[DEMAND_TYPE_MAX][(RATE_NUM+1)*7]; 
	TTime now;
	BYTE bTime[32];
	bool fFmt5;
	GetSysTime(&now);
	//准备需量数据
	for(i=0;i<AC_DEMAND_NUM;i++)
	{
		m_dwDemand[g_wAcCurDemandID[1][i]][0] = 0x1010+((DWORD)g_wAcCurDemandID[1][i]<<16);
		m_dwDemand[g_wAcCurDemandID[1][i]][1] = 0x1111+((DWORD)g_wAcCurDemandID[1][i]<<16);
		m_dwDemand[g_wAcCurDemandID[1][i]][2] = 0x1212+((DWORD)g_wAcCurDemandID[1][i]<<16);
		m_dwDemand[g_wAcCurDemandID[1][i]][3] = 0x1313+((DWORD)g_wAcCurDemandID[1][i]<<16);
		m_dwDemand[g_wAcCurDemandID[1][i]][4] = 0x1414+((DWORD)g_wAcCurDemandID[1][i]<<16);
		OoTimeToDateTimeS(&now,bTime);
		//memset(bTime, 0, sizeof(bTime));
		memcpy(&m_bTime[g_wAcCurDemandID[1][i]][0], bTime, 7);
		memcpy(&m_bTime[g_wAcCurDemandID[1][i]][7], bTime, 7);
		memcpy(&m_bTime[g_wAcCurDemandID[1][i]][14], bTime, 7);
		memcpy(&m_bTime[g_wAcCurDemandID[1][i]][21], bTime, 7);
		memcpy(&m_bTime[g_wAcCurDemandID[1][i]][28], bTime, 7);
	}

	p = bBuf;
	for(i=0;i<AC_ENERGY_NUM;i++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = 0x01;
		bBuf[1] = (RATE_NUM+1);
		p = bBuf+2;
		for(j=0;j<(RATE_NUM+1);j++)
		{
			fFmt5 = IsDemFmt5(g_wAcCurDemandID[0][i]);
			wLen = AcDemandToFmt(m_dwDemand[g_wAcCurDemandID[1][i]][j], &m_bTime[g_wAcCurDemandID[1][i]][j*7], p, fFmt5);
			p += wLen;
		}	
		WriteItemEx(BN0, PN0, g_wAcCurDemandID[0][i], bBuf);
	}






	//谐波次数(19)+三相电压电流总谐波含量及2~19次谐波含量
//		BYTE *p = bBuf;
//		*p++ = HARMONIC_NUM;

	int nHarPecent[6*(HARMONIC_NUM-1)] ={    
									550,	2,		350,	2,		250,	3,		2,  1,  3,  5,	1,	3,	5,	2,	4,	1,	2,	4,	1,	2,		//Ua
									750,	450,	3,		350,	2,		150,	2,	2,	4,	1,	2,	1,	3,	5,	1,	3,	5,	1,	3,	2,//Ub
									650,	4,		400,	2,		200,	3,		200,7,	5,	3,	1,	2,	4,	6,	5,	2,	4,	6,	5,	2,//Uc
									350,	2,		250,	2,		250,	3,		2,	8,	6,	4,	2,	1,	7,	5,	3,	1,	3,	2,	2,	3,//Ia
									820,	350,	4,		250,	3,		250,	2,	2,	4,	6,	8,	1,	3,	5,	7,	2,	4,	6,	8,	2,//Ib
									850,	3,		550,	3,		250,	3,		150,3,	3,	3,	2,	2,	2,	4,	4,	4,	3,	3,	3,	2};//Ic

//			WriteItemVal(BN0, PN0, 0x128f, nHarPecent);
//			if (bPn != PN0)
//				WriteItemVal(BN0, PN0, 0x128f, nHarPecent);

		TDataItem g_diVoltDistortion;	//电压波形失真度
		TDataItem g_diCurDistortion;	//电流波形失真度
		TDataItem g_diVoltHarPercent[3];	//电压谐波含量 总、2-19次
		TDataItem g_diCurHarPercent[3]; //电流谐波含量 总、2-19次
		TDataItem g_diHarmonicNum;	//谐波次数

		g_diVoltDistortion = GetItemEx(BN0, PN0, 0x200b);
		g_diCurDistortion = GetItemEx(BN0, PN0, 0x200c);
		
		for(i=0;i<3;i++)
		{
			g_diVoltHarPercent[i] = GetItemEx(BN0, PN0, 0x2600+i);
			g_diCurHarPercent[i] = GetItemEx(BN0, PN0, 0x2603+i);
		}
		g_diHarmonicNum = GetItemEx(BN0, PN0, 0x2606);


	
		//电压波形失真度/总谐波畸变率
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = 0x01;
		bBuf[1] = 0x03;
		for(i=0;i<3;i++)
		{
			bBuf[2+i*3] = DT_LONG;
			OoInt16ToLong(nHarPecent[(HARMONIC_NUM-1)*i],&bBuf[3+i*3]);
		}	
//			WriteItem(g_diVoltDistortion, bBuf);
		WriteItemEx(BN0, PN0, 0x200b, bBuf);
		
		//电流波形失真度/总谐波畸变率
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = 0x01;
		bBuf[1] = 0x03;
		for(i=0;i<3;i++)
		{
			bBuf[2+i*3] = DT_LONG;
			OoInt16ToLong(nHarPecent[(HARMONIC_NUM-1)*i+3*(HARMONIC_NUM-1)],&bBuf[3+i*3]);
		}
//			WriteItem(g_diCurDistortion, bBuf);
		WriteItemEx(BN0, PN0, 0x200c, bBuf);
	
	
		//电压总/2-N次谐波含量
		for(i=0;i<3;i++)
		{
			memset(bBuf, 0x00, sizeof(bBuf));
			bBuf[0] = 0x01;
			bBuf[1] = (HARMONIC_NUM-1);
			for(j=0;j<(HARMONIC_NUM-1);j++)
			{
				bBuf[2+j*3] = DT_LONG;
	//				memcpy(&bBuf[3+j*3], (BYTE *)&pwHarPercent[HARMONIC_NUM*i+j], 2);
				OoInt16ToLong(nHarPecent[(HARMONIC_NUM-1)*i+j],&bBuf[3+j*3]);
			}
//				WriteItem(g_diVoltHarPercent[i], bBuf);
			WriteItemEx(BN0, PN0, 0x2600+i, bBuf);
		}	
		
		//电流总/2-N次谐波含量
		for(i=0;i<3;i++)
		{
			memset(bBuf, 0x00, sizeof(bBuf));
			bBuf[0] = 0x01;
			bBuf[1] = (HARMONIC_NUM-1);
			for(j=0;j<(HARMONIC_NUM-1);j++)
			{
				bBuf[2+j*3] = DT_LONG;
	//				memcpy(&bBuf[3+j*3], (BYTE *)&pwHarPercent[HARMONIC_NUM*i+j+3*HARMONIC_NUM], 2);
				OoInt16ToLong(nHarPecent[(HARMONIC_NUM-1)*i+j+3*(HARMONIC_NUM-1)],&bBuf[3+j*3]);
			}
//				WriteItem(g_diCurHarPercent[i], bBuf);
			WriteItemEx(BN0, PN0, 0x2603+i, bBuf);
		}	


		//谐波次数
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = 0x01;
		bBuf[1] = 0x01;
		bBuf[2] = DT_UNSIGN;
		bBuf[3] = HARMONIC_NUM;
//			WriteItem(g_diHarmonicNum, bBuf);
		WriteItemEx(BN0, PN0, 0x2606, bBuf);




//		p = bBuf;
//		*p++ = 19;
//		int nHarVal[6*19+1] ={	19,		1100,	20,		800,	2,		955,	3,		2,  1,  3,  5,	1,	3,	5,	2,	4,	1,	2,	4,	1,	//Ua
//										1300,	800,	30,		1100,	2,		250,	2,	2,	4,	1,	2,	1,	3,	5,	1,	3,	5,	1,	3,	//Ub
//										1800,	40,		850,	2,		118,	3,		200,7,	5,	3,	1,	2,	4,	6,	5,	2,	4,	6,	5,	//Uc
//										1200,	20,		860,	2,		258,	3,		2,	8,	6,	4,	2,	1,	7,	5,	3,	1,	3,	2,	2,	//Ia
//										1150,	1000,	40,		865,	3,		250,	2,	2,	4,	6,	8,	1,	3,	5,	7,	2,	4,	6,	8,	//Ib
//										1250,	30,		550,	3,		651,	3,		150,3,	3,	3,	2,	2,	2,	4,	4,	4,	3,	3,	3};//Ic
//	
//			WriteItemVal(BN0, PN0, 0x127f, nHarVal);
//			if (PN0 != PN0)
//				WriteItemVal(BN0, PN0, 0x127f, nHarVal);
//		}
}
#endif

//显示是否处于U盘升级界面
//返回：true 已进入， false 未进入
bool IsInUsbProcess()
{
	BYTE bState = 0;

	if (ReadItemEx(BN2, PN0, 0x2112, &bState) > 0)
		return (bState!=0);
	else
		return false;
}

//U盘是否插入
//返回：true 已插入， false 未插入
bool IsMountUsb()
{
	BYTE bState = 0;

	if (ReadItemEx(BN2, PN0, 0x2111, &bState) > 0)
		return (bState!=0);
	else
		return false;
}

//设置USB处理界面是否进入状态 0：未进入，1：已进入
void SetUsbProcessState(BYTE bState)
{
	WriteItemEx(BN2, PN0, 0x2112, &bState);
}


//描述：申请线程的信号量
void RequestThreadsSem()
{
	//485-1,485-2,485-3
	for (BYTE i=LOGIC_PORT_MIN; i<LOGIC_PORT_MAX; i++)
		WaitSemaphore(g_semRdMtr[i-LOGIC_PORT_MIN]);

	//PLC
#if (FA_TYPE == FA_TYPE_C82) 
		g_CStdReader->LockReader();	
#endif
}

//描述：释放线程的信号量
void ReleaseThreadsSem()
{
	//485-1,485-2,485-3
	for (BYTE i=LOGIC_PORT_MIN; i<LOGIC_PORT_MAX; i++)
		SignalSemaphore(g_semRdMtr[i-LOGIC_PORT_MIN]);

	//PLC
#if (FA_TYPE == FA_TYPE_C82) 
	g_CStdReader->UnLockReader();	
#endif
}


BYTE BaudrateToGbNum(DWORD dwBaudRate)
{
	WORD GBBaudTab[8] = {
		CBR_300,  CBR_600, CBR_1200, CBR_2400, 
		CBR_4800, 0, CBR_9600, CBR_19200};

		for (BYTE i=0; i<sizeof(GBBaudTab)/sizeof(WORD); i++)
		{
			if (GBBaudTab[i] == dwBaudRate)
				return i;
		}	
		return 0;
};


//485端口转发函数
int DoMtrFwdFunc(WORD wPort, WORD wTestId, BYTE bMtrPro, BYTE* pbMtrAddr, BYTE* pbRxFrm)
{
	DWORD dwID;	
	BYTE bFrmLen;
	int iLen = 0, iPort;
	BYTE bCmdFrm[256];
	BYTE bBuf[128];
	TCommPara tCommPara;

	if (GetLogicPortFun(wPort) != PORT_FUN_RDMTR) //-1
	{
		DTRACE(DB_METER, ("DoMtrFwdFunc: wPort=%d is not PORT_FUN_RDMTR!\r\n", wPort));
		return -1;	
	}

	iPort = wPort;
	/*iPort = MeterPortToPhy(wPort); // 抄表的逻辑端口到物理端口的映射
	if (iPort < 0)
	{
		DTRACE(DB_METER, ("GetMeterPara : fail to map port %d to physic\n", wPort));
		return false;
	}*/

	if (bMtrPro == CCT_MTRPRO_07)
	{
	     if (wTestId == 0x9010)
			dwID = 0x00010000;
	     else 
	     	dwID = 0x0001ff00;

		 tCommPara.dwBaudRate = CBR_2400;
	}
	else
	{
		dwID = wTestId;

		tCommPara.dwBaudRate = CBR_1200;
	}	
	
	tCommPara.wPort = (WORD )iPort;
	tCommPara.bByteSize = 8;
	tCommPara.bStopBits = ONESTOPBIT;
	tCommPara.bParity = EVENPARITY;

	bFrmLen = Make645AskItemFrm(bMtrPro, pbMtrAddr, dwID, bCmdFrm);	
	iLen = MtrDoFwd(tCommPara, bCmdFrm, bFrmLen, bBuf, sizeof(bBuf), 9000, 10);
	if (iLen <= 0)
		return -13;

	memcpy(pbRxFrm, bBuf, iLen);
	return iLen;
}


void InitMtrPortSch(BYTE bStartState)
{
	WORD wPortNum, wPortMin, wPortMax;
	GetLogicPortNum(&wPortNum, &wPortMin, &wPortMax);
	for (BYTE i=0; i<wPortNum; i++)
	{
		if ( IsMtr485Port(wPortMin+i) )
			InitSearch(i,bStartState);
	}
}

void DoMtrPortSch()
{
	WORD wPortNum, wPortMin, wPortMax;
	GetLogicPortNum(&wPortNum, &wPortMin, &wPortMax);
	for (BYTE i=0; i<wPortNum; i++)
	{
		if ( IsMtr485Port(wPortMin+i) )
			DoSearch(i);
		else
			InitSearch(i,0);
	}
}

void CheckDownSoft(void)
{
	if (g_dwFileTransCurSec != 0)
	{
		if (g_fDownSoft)
		{
			DWORD dwCurSec = GetCurTime();;
			if (g_dwFileTransCurSec > dwCurSec ||dwCurSec > g_dwFileTransCurSec+60*10)//10分钟都没有下发升级包就清掉这个升级标志
				g_fDownSoft = false;
		}
	}
}

