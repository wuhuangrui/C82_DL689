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


#ifdef EN_CCT	//����������
#include "CctStat.h"
#include "CctCtrl.h"
#endif

#include "StdReader.h"
#include "StatMgr.h"
#include "DbOIAPI.h"
#include "DL645Ext.h"
#include "SearchMeter.h"
#include "ComAPI.h"

#define YX_OFF_STATE	0		//��
#define YX_ON_STATE		1		//��

//#define OOP_TAITI_TEST	1		//�ϵĲ���ESAM̨����Բſ���

CStatMgr g_StatMgr;
//CQueue g_Queue;     //Э���̵߳ı�����Ϣ����
int g_iRstCnt[] = {0, 0, 0}; //8010,8011,8012����ͨ����¼����ʧ�ܵĴ���
int g_iCurChannel = 0; //��ǰʹ�õ���վ��ַ 0-��ͨѶͨ��, 1-����ͨ��1, 2-����ͨ��2
DWORD g_dwTaskMaskClick = 0;
DWORD  g_dwWrOpClick = 0;
bool g_fCurveChange = false;
bool g_fCommTaskChg = false;
bool g_fAddressChg = false;
bool g_fMtrParaChg = false;
bool g_fGrpParaChg = false;
bool g_fCtrlGrpParaChg = false;	//�����ܼ����������
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
BYTE g_bReadMtrStatus[POINT_NUM];//485����״̬:0����,1����ʧ��,2����ָ�
BYTE g_b485PortStatus;//485�����״̬:0����,1���Ϸ���,2���ϻָ�
bool g_fMtrFailHapFlg[POINT_NUM];
bool g_f485FailHapFlg;

bool g_fInAdjSysClock = false;	//�Ƿ��ڶ�ʱ״̬

WORD   g_wAdjUn = 2200;

TSem   g_semTask;
TSem   g_semExcTask;
TSem   g_semMeter485;
TSem   g_semFaProto;
TSem   g_semMeterLog;
TSem   g_semNetStat; //�������״̬
TSem   g_semGateWay; //��ȡ����
TSem   g_semGetIP;
TSem   g_semClr645Data;		//645����������0����
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

bool g_fBatPowerOff = false;	//����Դ�ϵ��ҵ�ص�ѹ<3.65V
bool g_fCTValid = false;
WORD g_wTcpipRunCnt = 0;
bool g_fFapNeedConnect = false;
bool g_fMasterTerm = false;
WORD g_wLinkInterv = 120;
BYTE g_bClrEnergyStep = 0;	//�����״̬��

DWORD g_dwExtCmdFlg = 0;
DWORD g_dwExtCmdClick = 0;
BYTE g_bDefaultCfgID = 0;
BYTE g_bEnergyClrPnt = 0;

bool g_fSearchMeter = false;
bool g_fClearMeterInf = false;
bool g_fClearHistory = false;
static DWORD g_dwAcOffClick = 0;
static DWORD g_dwAcUpClick = 0;
static bool  g_fAcUp = false;   	//��������û������,��Ҫ�����ж��Ƿ��ǰ�������
static bool   g_fFaShutDown = true;  //�ն��Ƿ����ڽ���ͣ���ĵ�عضϲ���
bool g_fPowerOffAcked = false;	//ͣ��ȷ�ϱ�־

bool g_fDownSoft = false;	//�Ƿ����������
DWORD g_dwFileTransCurSec = 0;


TDataItem g_diTimeRule;  //��ʱ����
TTime g_tmMinuteTask;	 //1���������ʱ��
TTime g_tmDayTask;       //1������ʱ��

TPowerOffTmp g_PowerOffTmp;     //�����ݴ����
TPowerOffTmp g_DefaultPowerOffTmp = {PWROFF_VER, //�汾 
									 true,  //�����ݴ������Ч��־
									 false,   //bool fAlrPowerOff	����ǰ�ϱ���ͣ��澯
									 0,				//WORD wRstNum;	��λ����
									 0,				//�̼߳�ظ�λ����
									 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
									  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
									  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	
									  0, 0},		//�̼߳�ظ�λ���һ�θ�λ���߳�����

									 {0, 0, 0, 0, 0, 0, 0},//tPoweroff �ϴ�ͣ��ʱ��
									 {0, 0, 0, 0, 0, 0, 0},//tPoweroff �ϴ��ϵ�ʱ��

									}; 
BYTE g_bTermChanelInfo[5] = {	2,	//��������·�� 8
								4, //����������·�� 4
								3, //���������·��(�ִ�) 4
								0xF4,0x01  //�ɼ����/����װ�õ����� 500
}; 
BYTE g_bEvtCfgInfo[9] = { 0x40, 0x00, 0x00, 0x00, 0x01,0xff, 0xe3, 0x3b, 0xff};	//֧���¼�1~33������11,15,16,18,20,22��֧��
TTime g_tPowerOn;
BYTE g_bRemoteDownIP[8];
//unsigned long TickMs;


bool g_fFrzInit = false;	//�����ʼ���Ƿ����

void CheckDownSoft(void);

//U���Ƿ����
//���أ�true �Ѳ��룬 false δ����
bool IsMountUsb(void);

void SetMountUsb(BYTE bState);

//����USB��������Ƿ����״̬ 0��δ���룬1���ѽ���
void SetUsbProcessState(BYTE bState);



//CL818C7��3·485�ں�debug�ڹ��ã�0xa200����0�ص��ԣ�
void InitConsole()
{
	BYTE bPortFun = PORT_FUN_ACQ;
	
	ReadItemEx(BN10, PN0, 0xa180, (BYTE*)&bPortFun);

#ifndef EN_SPECIAL_TRANSFORMER
	if (bPortFun != PORT_FUN_DEBUG)
	{//��485�������ʱ���ùر�console
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
	const BYTE bParityTab[] = {NOPARITY, EVENPARITY, ODDPARITY}; 
	if (n >= 3)
	{
		return NOPARITY;
	}
	
	return bParityTab[n];
}



BYTE NumToStopBits(BYTE n)
{
	const BYTE bStopBitsTab[] = {ONESTOPBIT, TWOSTOPBITS, TWOSTOPBITS};
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
//����:У׼ʱ��
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

		SetInfo(INFO_RST_TERM_STAT); //��λ�ն�ͳ������
#ifdef EN_CCT
		SetInfo(INFO_PLC_CLRRT);	 //���������·��
#endif
		LockDB();
		TdbLock();

		char command[64] = {0};
		sprintf(command, "rm -rf %s*", USER_PARA_PATH);
		system(command);
		memset(command, 0, sizeof(command));
		sprintf(command, "rm -rf %s*", USER_DATA_PATH);
		system(command);

		if (iAdjLen > 0)	//ֻ������ǰ������ļ���ʱ��ű���
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
	else if (g_dwExtCmdFlg == FLG_HARD_RST)
	{
		SavePoweroffTmp();	//�յ������������󲻱���ϵͳ���ļ�,�Ᵽ��������,����������Ӧʱ�����
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

//����:���ļ��г�ʼ�������ݴ����,�����Ч,��ʹ��Ĭ��
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



	//��λ����
	g_PowerOffTmp.wRstNum++;	
	g_DefaultPowerOffTmp.wRstNum = g_PowerOffTmp.wRstNum;
	WriteItemEx(BANK2, PN0, 0x1021, (BYTE *)&g_PowerOffTmp.wRstNum);	//0x1021 2 ��λ����,HEX

	//�̼߳�ظ�λ����
	g_DefaultPowerOffTmp.wMonitorRstNum = g_PowerOffTmp.wMonitorRstNum;	
	memcpy(g_DefaultPowerOffTmp.szMonitorRstThrd, g_PowerOffTmp.szMonitorRstThrd, sizeof(g_PowerOffTmp.szMonitorRstThrd));
	WriteItemEx(BANK2, PN0, 0x1029, (BYTE *)&g_PowerOffTmp.wMonitorRstNum); //0x1029 2 �̼߳�ظ�λ����,HEX
	WriteItemEx(BANK2, PN0, 0x102a, (BYTE* )g_PowerOffTmp.szMonitorRstThrd); 	//0x102a 1 �̼߳�ظ�λID,HEX
	
	//g_DefaultPowerOffTmp.fAlrPowerOff = g_PowerOffTmp.fAlrPowerOff;		//��ͣ��澯��־������ͨ��Э���и澯��ȷ�ϲ���
	GetCurTime(&(g_PowerOffTmp.tPowerOn));
	WriteFile(USER_DATA_PATH"PoweroffTemp.cfg", (BYTE* )&g_DefaultPowerOffTmp, sizeof(g_DefaultPowerOffTmp));


	//д��ȱʡ�ն�����/������˿���������
	//WriteItemEx(BN0, PN0, 0x5510, g_bTermChanelInfo);	
	//WriteItemEx(BN0, PN0, 0x5706, g_bEvtCfgInfo);	
	//BYTE bBuf[12] = {64, 0,0,0,0,0,0,0,0,16,0,0};
	//WriteItemEx(BN0, PN0, 0x5511, bBuf);
}

//����:��������ݴ����
bool SavePoweroffTmp()
{
	return WriteFile(USER_DATA_PATH"PoweroffTemp.cfg", (BYTE* )&g_PowerOffTmp, sizeof(g_PowerOffTmp));
}

void UdpMeterPara()
{
		if (GetInfo(INFO_METER_PARA) == false)
			return; 
		
		DTRACE(DB_METER, ("UdpGrpPara: para chg!\r\n"));
	
	//	SetCtrlGrpParaChg(true);//���������߳�ֹͣ����
	
	//	SetMtrParaChg(true);
	//	SetGrpParaChg(true);

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


//CT/PT���������Ϊ��Ӱ�������㣬��������
void UdpCtPtPara()
{
	if (GetInfo(INFO_CTPT_PARA) == false)
	 	return;	
	
	DTRACE(DB_METER, ("UdpCtPtPara: para chg!\r\n"));	

//	SetGrpParaChg(true);
//	SetCtrlGrpParaChg(true); //���������߳�ֹͣ����

	SetInfo(INFO_AC_PARA);	 //���ɲ������
	//SetInfo(INFO_COMTASK_PARA); //��ͨ����������
	//SetInfo(INFO_EXC_PARA);		//�쳣�������������ܼ��鳬�
	//SetInfo(INFO_STAT_PARA);	//ͳ�Ʋ���������ܼ���ͳ�ƣ�

	TrigerSaveBank(BN11, 0, -1); //��ʱͳ�Ƶ��������
}

//������ȷ��Ӧ����������Ҫ��Ŀ¼��������
//��ע������֮ǰ��ȥ�İ汾û��������չ���������²����ݣ����Ա������Ͳ��ŵ������棬�ɳ�������Ƿ��ʼ����Ŀ¼
bool InitDir2()
{
#ifdef SYS_LINUX	

	DIR* dir = opendir(USER_EXT_PATH);
	if (dir == NULL)
	{
		if (mkdir(USER_EXT_PATH, 0) == -1)
			return false;
	}
	else
	{
		closedir(dir);
	}
	dir = opendir(USER_BAK_PATH);
	if (dir == NULL)
	{
		if (mkdir(USER_BAK_PATH, 0) == -1)
			return false;
	}
	else
	{
		closedir(dir);
	}
#endif //SYS_LINUX

	return true;
}

bool InitTCPIP();

TTdbPeriTime g_tmPeriTime[2];
/*
#ifdef SYS_LINUX
void sigCtrC(int n)
{
//	printf("\r\n sigCtrC !!! \r\n\r\n");	
	system("/clou/ppp/script/ppp-off");	//�ȹر�PPP
	Sleep(1000);
	int pid = getpid();
	kill(pid, SIGABRT);
	exit(0);	//�߳��˳�
}
#endif
*/

void FaInitPortFrom698(void)
{
	TCommPara tCommParaTmp, tCommPara;
	BYTE bFunc[LOGIC_PORT_NUM], bFlag = 0, bFuncFlag = 0;
	int i;
	bool fRet;
	g_commLocal.Open(COMM_DEBUG, CBR_9600, 8, ONESTOPBIT, EVENPARITY);	//��������� ������9600��ż
	g_commLocal.GetCommPara(&tCommParaTmp);
	for(i=LOGIC_PORT_NUM; i>0; i--)
	{
		tCommPara.wPort = g_iInSnToPhyPort[i-1];
		fRet = OIRead_PortPara(0xF201, i-1, &tCommPara, &bFunc[i-1]);
		TraceBuf(DB_CRITICAL, ("\r\n FaInitStep1 :tCommParaTmp->"), (BYTE*)&tCommParaTmp, sizeof(tCommParaTmp));
		TraceBuf(DB_CRITICAL, ("\r\n FaInitStep1 :tCommPara->"), (BYTE*)&tCommPara, sizeof(tCommPara));
		if (true == fRet)
		{
			bFuncFlag |= 1<<(i-1);//��¼��Ч���ܿ�
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
					bFlag = 1;//�ر�
				}

				if (!g_commLocal.Open(tCommPara))
				{	
					DTRACE(DB_CRITICAL, ("FaInitStep1: g_commLocal open fail.i=%d\r\n",i-1));
				}
				else
				{
					DTRACE(DB_CRITICAL, ("FaInitStep1: g_commLocal open succ.i=%d\r\n",i-1));
					bFlag = 0;//�ɹ���
					break;
				}
			}
		
		}
	}
	if(bFlag == 1)
	{//���ر�
		DTRACE(DB_CRITICAL, ("FaInitStep1: g_commLocal recover\r\n"));
		g_commLocal.Open(tCommParaTmp);//�ָ�
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
	char bUserPath[128];
	sprintf(bUserPath, USER_PATH);

	if (InitDirNew(bUserPath) == false)
	{
		DTRACE(DB_FA, ("FaInit: error:  fail to create sys dir.\r\n"));
		return;
	}
	if (InitDir2() == false)
	{
		DTRACE(DB_FA, ("FaInit: error:  fail to create sys dir2.\r\n"));
		return;
	}
	InitInfo();

	TdbSetMaxTabNum(1024);	//Ŀ
	TdbSetMaxRecLen(720);
	TdbSetMaxFieldLen(900);
	TdbSetPeriCacheNum(PN_VALID_NUM);
	TdbInit(USER_TASKDATA_PATH);
    
	InitDB(); //�汾����¼��õ������
	
	//ApllyCfgAuto();//�����Զ�Ӧ�������ļ�
	
	InitPoweroffTmp();	//��ʼ���������
	GetCurTime(&g_tPowerOn);
	//��ʼ�����Կ�
#ifdef SYS_LINUX
	if (IsDownSoft())	//�յ�����������,�Ѳ��ԿڵĲ���������Ϊ115200,Ҳ��������
		g_commTest.Open(COMM_TEST, CBR_115200, 8, ONESTOPBIT, NOPARITY);
	else
		g_commTest.Open(COMM_TEST, CBR_9600, 8, ONESTOPBIT, NOPARITY);

	//��ʼ������ά����
	g_commLocal.Open(COMM_LOCAL, CBR_2400, 8, ONESTOPBIT, NOPARITY);
#else
	g_commTest.Open(COMM_TEST, CBR_9600, 8, ONESTOPBIT, NOPARITY);
#endif //SYS_LINUX

// 	if (g_commYK.Open(COMM_PLC, CBR_19200, 8, ONESTOPBIT, EVENPARITY))
// 		DTRACE(DB_CTRL, ("*****FaInit: open COMM_PLC success!!!\r\n"));
// 	else
// 		DTRACE(DB_CTRL, ("*****FaInit: open COMM_PLC failed!!!\r\n"));
	//InitTestMode();
	
	InitDebug();   //������Ϣ��������ݿ��ʼ����Ž���,��Ϊ�õ������ݿ�
//	InitConsole();

#ifdef SYS_LINUX
    //������ݿ���������698Э��Ķ˿���Ϣ������698Э���ʼ���˿�   whr 20170704
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
	strcpy(szTmp, "����");
	WriteItemEx(BN2, PN0, 0x2030, (BYTE *)szTmp);	
	strcpy(szTmp, "����");
	WriteItemEx(BN2, PN0, 0x2032, (BYTE *)szTmp);
	strcpy(szTmp, "����");
	WriteItemEx(BN2, PN0, 0x2033, (BYTE *)szTmp);
#endif

	//������Ҫʹ���ڲ��汾��ʱ���ڲ��汾���еİ汾��������Ϣ��0���ɳ����Զ�ȡ�����汾�İ汾�ż����ڣ���ʾ���ڲ��汾�ϡ�
//     if(IsAllAByte(&g_bInnerSoftVer[16], 0x00, 7))
// 		memcpy(&g_bInnerSoftVer[16], &g_bSoftVer[12], 7);
// 
 	WriteItemEx(BN2, PN0, 0x2107, g_bInnerSoftVer);

	WriteItemEx(BN2, PN0, 0x210e, (BYTE* )&g_PowerOffTmp.fAlrPowerOff); //�ϵ��ʼ��ͣ�ϵ�״̬

	memset(szTmp, 0, sizeof(szTmp));
	szTmp[0] = DT_LONG;
	WriteItemEx(BN2, PN0, 0x6003, (BYTE* )szTmp);	//�ϵ������ź�ǿ��ֵ
}

//����:ϵͳ��ʼ���ڼ�������ĳ�ʼ���ҽ�
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

//������@pbIsValid 1��ʾ��Ч������ֵ��ʾ��Ч
//���أ������Ƿ�ͣ��
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

	nRead = ReadItemEx(BN2, PN0, 0x210e, (BYTE* )&fPowerOff); //��������������ʱ�������ϴ��жϵ�״̬
	if (nRead <= 0)
		return false;

	nRead = ReadItemEx(BN25, PN0, 0x5001, bBuf);	//�����ѹ
	if (nRead <= 0)
		return false;

	wUn = (DWORD )BcdToDWORD(bBuf, 3) / 10; //���ѹֵ2200,һλС��
	if (wUn == 0)
		wUn = 2200;	//���ѹ,��ʽNNNNN.N

	memset(bBuf, 0, sizeof(bBuf));
	nRead = OoReadAttr(TERM_POWOFF, ATTR6, bBuf, NULL, NULL); //��ȡ����6���ò���-ͣ�����ݲɼ����ò���
	if (nRead<=0 || bBuf[0]!=DT_STRUCT)
		return false;

	wBaseOffset = 0;
	pbBuf = &bBuf[12];	//ָ��TSA����
	for (i=0; i<bBuf[12]; i++)
	{
		wBaseOffset += 2;	//TSA��������1 + TSA����1
		wBaseOffset += pbBuf[wBaseOffset];
	}
	
	wBaseOffset += 12;	//��bBuf��ʼ�������ǰ���12���ֽ�
	wBaseOffset += 16;	//ͣ���¼������ֵ-ͣ�緢����ѹ��ֵƫ��

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

	bErcData[0] = 8; //bitλ��
	bErcData[1] = 1;
 	ReadItemEx(BN0, PN0, 0x100f, bSoftVer);
	memcpy(bErcData+2, bSoftVer+16, 4 );
	memcpy(bErcData+6, bSoftVer+16, 4 );

}

bool g_fYxInit = false;
static BYTE g_bYxVal = 0;

void SetYxInitFlag(bool fInit)
{
	g_fYxInit = fInit;
}

//����������һ������F9ң�ŵ�״̬��
void DoYX()
{
	int i, iLen;
	BYTE bYxVal = 0; //ң��״̬��־
    BYTE bChgFlg;	 //״̬��λ��־
	BYTE bBuf[20] = {0};
	BYTE bValid, bYxFlag = 0;
	const WORD wOI = OI_YX;

	if (IsAcPowerOff(&bValid) || bValid==1)
		return ;
    
#ifdef SYS_LINUX
	 BYTE bDoorStat =  0x10;
	 bDoorStat = GetDoorStatus(); //�Žڵ��״̬Ϊ�ߵ�ƽ
#endif
    int nRead = ReadItemEx(BN2, PN0, 0x1100, &bYxVal);
    if (nRead <= 0 || GetClick() < 5)
    	return;

#if FA_TYPE == FA_TYPE_K32
	bYxVal <<= 7;
#endif


/*
	if(bDoorStat > 0)
		bDoorStat = 0x00;
	else
		bDoorStat = 0x10;

	 bYxVal = (bYxVal&0xef) | bDoorStat;
*/


    if (!g_fYxInit)
    {//��һ�ν�����ȡң��״̬λ�����жϣ�
        g_bYxVal = bYxVal;
        g_fYxInit = true;

		 memset(bBuf, 0, sizeof(bBuf));
		iLen = OoReadAttr(wOI, ATTR4, bBuf, NULL, NULL);	//ȡ��������������Բ���
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
			if (bYxFlag & (1<<i))	//YX����
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

			WriteItemEx(BN0, i, 0xF203, bBuf);  //�洢ң��״̬��
		}

        return;
    }
 
	bChgFlg = g_bYxVal ^ bYxVal;
    g_bYxVal = bYxVal;

	DTRACE(DB_CRITICAL, ("DoYX: yx chg, bYxVal=%x, bChgFlg=%d.\r\n", bYxVal, bChgFlg));

    if (bChgFlg != 0)
    {
		for (i=0; i<MAX_SW_PORT_NUM; i++)
		{
			if (bChgFlg & (1<<i))	//ĳ·YX�б仯�Ÿ��¸�·����Ӱ������YX��λ��־
			{
				DTRACE(DB_CRITICAL, ("DoYX: yx chg, i=%d, bYxVal=%x.\r\n", bYxVal));
				bBuf[0] = DT_STRUCT;
				bBuf[1] = 2;
				bBuf[2] = DT_UNSIGN;

				#ifdef OOP_TAITI_TEST
				if (bYxVal & (1<<i))	//YX״̬
					bBuf[3] = YX_OFF_STATE;
				else
					bBuf[3] = YX_ON_STATE;
				#else
				if (bYxVal & (1<<i))	//YX״̬
					bBuf[3] = YX_ON_STATE;
				else
					bBuf[3] = YX_OFF_STATE;
				#endif

				bBuf[4] = DT_UNSIGN;
				bBuf[5] = 1;	//��λ��־
				WriteItemEx(BN0, i, 0xF203, bBuf);  //�洢ң��״̬��
			}
		}
	}
}

//���𣺿��ƻ�·������
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
	//��һ�η���  ��ʼ�������ն����Ƕ�������Ƭ���������� ��ReadCtrlLoop<0 ,g_fCtrlStateInitһֱ����Ϊtrue
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
			bCtrlStateVal = bRetFrm;//���³�ʼֵ
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
	bool fForbitWDG = false;
    while (1)
    {
        Sleep(100);
		if (!fForbitWDG)
			ClearWDG();

		if (GetInfo(INFO_STOP_FEED_WDG))
			fForbitWDG = true;

		i++;
		if (i == 4)
		{
			if (j & 1)
			{
				iMonitorID = DoThreadMonitor();	//�̼߳��,ÿ��ִ��һ��
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

//������ֻ��һ���������ֶΣ��Ǽ�⵽��ص�ѹ�ܵ͵�ʱ��Ҫ�����
//�����µ�ض�·�˺ĵ����
//��ƽʱ����ʱ���ݵ�ص�ѹ�����ƣ����ϵ�ʱ�͵��������û��ϵ
static bool g_fIsBatOk = true;//20140320
void DoBatMgr()
{
#ifdef SYS_LINUX
	WORD fVal;
	BYTE bBuf[8];
	
	BYTE bIsValid = 0;

	if (IsAcPowerOff(&bIsValid))  //����ͣ��
	{
		if (bIsValid == 1)
			return;
		return;
	}
	
	ReadItemEx(BANK2, POINT0, 0x1028, bBuf); //0x1028 2 ��ص�ѹ NN.NN V
	fVal = BcdToDWORD(bBuf, 2);
	if (fVal > 645 || fVal==0) //û�ӵ��//20131212-3
	{
		g_fIsBatOk = false;
		GprsBatCharge(false);//�رճ��
		return ;
	}
		
	if (fVal < 430)
	{
		GprsBatCharge(true);//�򿪳��
		//BackBatOnOff(true);//ͣ��󣬵���޷��ض�
	}
	else if (fVal > 530)
	{
		GprsBatCharge(false);//�رճ��
		//BackBatOnOff(false);
	}
#endif
}

//ȡ���״̬��true��ʾ����Ѳ��
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
	//if (IsAcPowerOff() == 0)  //����ͣ��
		//return;
	ReadItemEx(BANK2, POINT0, 0x1028, bBuf); //0x1028 2 ��ص�ѹ NN.NN V
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
bool InitVBreakStat()	//��ʼ������ͳ��
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
	//ʱ��оƬ����
	ReadItemEx(BN2, PN0, 0x1122, bBuf);
	bBuf[1] = GetRtcChipStatus();
	if(bBuf[1]!=bBuf[0])
	{
		WriteItemEx(BN2, PN0, 0x1122, bBuf);
	}	

	
	//����оƬ����
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
	// 1.�ȶ�ȡ����������
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
				bUpPortFlag = 1;//�����п�
			}
			bFuncFlag |= 1<<(i-1);//��¼��Ч���ܿ�				
		}
	}
	// 2.�ݴ�ά�������ô���
	/*if(bUpPortFlag == 0)
	{//û��ά������,�ָ���3·Ϊά����
		bFunc[2] = 0;
		bFuncFlag |= 1<<2;
		DTRACE(DB_CRITICAL, ("DriverThread: g_commLocal reset to 2\r\n"));
	}*/
	DTRACE(DB_CRITICAL, ("DriverThread: bFuncFlag=%x\r\n",bFuncFlag));
	// 3.�����ڲ�ͨ�ſ�����
	for(i=LOGIC_PORT_NUM; i>0; i--)
	{
		DTRACE(DB_CRITICAL, ("DriverThread: bFunc[%d]=%d\r\n",i-1,bFunc[i-1]));
		if(bFuncFlag&(1<<(i-1)))
		{//��Ч������
			//����ͨ�ţ�0��������1����������2����ͣ�ã�3��
			switch(bFunc[i-1])
			{
				case 0:
					DTRACE(DB_CRITICAL, ("DriverThread: %d not support setting to PORT_FUN_LOCAL485\r\n",i));
					//һ�ͼ�����ֻ����·485�ڣ���֧�ֵ���·485���ù���
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
					break;//����������
			}
			WORD wID;
			BYTE b;
			if (i == 1) //485-1��
				wID = 0xa131;
			else if (i == 2)	//485-2��
				wID = 0xa132;
			//һ�ͼ�����ֻ����·485�ڣ���֧�ֵ���·485���ù���
			//else if (i == 3)	//485-3��
				//wID = 0xa180;
			ReadItemEx(BN10, PN0, wID, (BYTE*)&b);
			DTRACE(DB_CRITICAL, ("DriverThread: %d wID=%x,b=%d\r\n",i,wID,b));
		}
	}			
	// 4.�л��˿�
	bFlag = 0;//�ɹ���
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
				bFlag = 1;//�ر�
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
				bFlag = 0;//�ɹ���
				break;
			}
		}
	}
	
	if(bFlag == 1)
	{//���ر�
		DTRACE(DB_CRITICAL, ("DriverThread: g_commLocal recover\r\n"));
		g_commLocal.Open(tCommParaTmp);//�ָ���ǰ��
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

//����:���̷߳�������صĺ���
TThreadRet DriverThread(void* pvPara)
{
	
	int iMonitorID = ReqThreadMonitorID("driver-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊһ��Сʱ
	DTRACE(DB_CRITICAL, ("DriverThread : started!\n"));
	
#ifdef SYS_LINUX
	WORD  wCnt=0, wCheck=0;
	CDCSample dcSample;
//		InitDrvPara();
	CDCProc dcProc;
	CCalcPulse calcPulse;
	DCInit();
	dcProc.Init();
	if (!calcPulse.Init(4))
		DTRACE(DB_CRITICAL, ("DriverThread : CCalcPulse init failed.\r\n"));

	GprsBatOnOff(false); //���������
	BackBatOnOff(true);
	GprsBatCharge(true);  //�ȴ򿪳�翪�أ�û�ӵ��ʱ��ؿڵĵ�ѹΪ7v����//20131212-3
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
      //  DoBatMgr();  //D82û�г�����
		BatStas();//xzz
#ifndef SYS_WIN
		wCnt++;
		wCheck++;
		if(wCheck >= 50)  //5s��һ�³�翪�أ�����µ���Ƿ�Ӻ�//20131212-3
		{
			//5s��ȡһ�»ỰʱЧʣ��ʱ�� 
			Esam_ReflashSessionRemainTime();
		
			//5s��һ�³�翪�أ�����µ���Ƿ�Ӻ�//20131212-3
			WaitSemaphore(g_semBat);
			g_fIsBatOk = true;
			GprsBatCharge(true);
			Sleep(200);
			dcSample.Run();
			DoBatMgr();
			SignalSemaphore(g_semBat);
			wCheck = 0;
		}	
		if (wCnt >= 10)	//ÿ1��ִ��һ��
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
		if (GetInfo(INFO_RS232_PARACHG))	//�յ�232�����޸���Ϣ
		{
			DTRACE(DB_CRITICAL, ("DriverThread : rx INFO_RS232_PARACHG......\r\n"));
		}
		if (GetInfo(INFO_RS485_PARACHG))	//�յ�485�����޸���Ϣ
		{
			DealInfoRS485ParaChg();
		}
		if (GetInfo(INFO_INFRA_PARACHG))	//�յ���������޸���Ϣ
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
//�����߳�  ����ͨ�����쳣����
TThreadRet TaskThread(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("task-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊ120��

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
		Sleep(1000); //����ʵʱ�Բ���̫�� 10s
#ifdef EN_CCT
		UpdCctRdStatus();
		g_CctTaskManager.DoCommonTask();
#endif
		Sleep(1000); //����ʵʱ�Բ���̫�� 5s
		g_TaskManager.DoTask();
//		g_TRJTaskManager.DoTask();
		UpdThreadRunClick(iMonitorID);
	}

	ReleaseThreadMonitorID(iMonitorID);

	return THREAD_RET_OK;
}

//���ݴ����߳�
TThreadRet DataProcThread(void* pvPara)
{	
	int iMonitorID = ReqThreadMonitorID("DataProc-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊ120��

	UpdMtrDataProcess(); //������
	UpdGrpDataProcess(true); //�ܼ�����

	while (1)
	{		
		if (IsMtrParaChg())
		{
			UpdMtrDataProcess();
			SetMtrParaChg(false);
		}

		if (!IsMtrParaChg() )//&& IsGrpParaChg()) //һ��Ҫ��������¹����ٸ����ܼ���
		{
			UpdGrpDataProcess(false);	
			SetGrpParaChg(false);	

			//�����м���һ�飬�ٷ��������߳����¿�ʼ����
			RunMtrDataProcess();
			if (IsFkTermn())	//�����ն˲���Ҫ���ܼ������ݵĴ���
			{
				RunGrpDataProcess();
			}
			TrigerSaveBank(BN18, 0, -1); //���ʾֵ	��������.
			
			SetCtrlGrpParaChg(false);	
		}

		RunMtrDataProcess();
		if (IsFkTermn())	//�����ն˲���Ҫ���ܼ������ݵĴ���
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
//���ݴ����߳�
TThreadRet DataStatThread(void* pvPara)
{	
	int iMonitorID = ReqThreadMonitorID("DataStat-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊ120��


	while (1)
	{		
		g_StatMgr.DoDataStat();

		UpdThreadRunClick(iMonitorID);

		Sleep(1000);
	}	
	
	ReleaseThreadMonitorID(iMonitorID);

	return THREAD_RET_OK;
}

//�����߳�
TThreadRet LoadCtrlThread(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("LoadCtrl-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊ120��

	Sleep(5000);	//������ʱ�����ӳ�5������.
	g_LoadCtrl.Init();
	SetCtrlThreadStart(true);
	
	static DWORD dwClick = 0;
	while (1)
	{
		if (dwClick==0 || (GetClick()-dwClick>20) || GetInfo(INFO_CTRL))//�յ���Ϣ���߼��50������ִ��һ��
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
#ifdef EN_VARCPS //�Ƿ������޹�����(VAR compensator)����
//�޹������߳�
TThreadRet LoadWGCtrlThread(void* pvPara)
{
	DTRACE(DB_CRITICAL, ("LoadWGCtrlThread : started!\n"));
	int iMonitorID = ReqThreadMonitorID("WGCtrl-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊ120��
	InitWGYK();
	Sleep(5000);	//������ʱ�����ӳ�5������.
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
//����:�����߳�
TThreadRet FastSecondThread(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("FastSecond-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊ60��
	DTRACE(DB_CRITICAL, ("FastSecondThread : started!\n"));

	BYTE bPlcStusOld = GetPlcStatus(); 
	BYTE bPlcStusCur = bPlcStusOld;
	bool fFirstStus  = (bPlcStusOld==PLC_MODULE_EXIST) ? true : false;
	WORD wCnt = 0;

	while(1)
	{
		wCnt++;
		if (wCnt >= 10)	//ÿ1��ִ��һ��
		{	
			wCnt = 0;
			UdpMeterPara();
			UdpCtPtPara();

//#ifdef EN_AC
//			DoVBreakStat();		//645����ͳ��
//#endif

#ifndef SYS_WIN
		    g_PulseManager.Run();
#endif	//SYS_WIN

			bPlcStusCur = GetPlcStatus();
			if(bPlcStusCur != bPlcStusOld)
			{
				bPlcStusOld = bPlcStusCur; 
				if(!fFirstStus) //�ն�����ʱû��ģ�飬�����Ų��ģ�飬���ܷ�ģ�������Ϣ
				{
					fFirstStus = true;
					continue;
				}

				SetInfo(INFO_PLC_MOD_CHANGED);
				DTRACE(DB_CRITICAL, ("FastSecondThread : SetInfo INFO_PLC_MOD_CHANGED!\n"));	
			}

			UpdThreadRunClick(iMonitorID);
		}

		SyncTimer();//ÿ��Сʱͬ��һ��Ӳ��ʱ��
		Sleep(100);
	}

	ReleaseThreadMonitorID(iMonitorID);
}

/*
TThreadRet DownSoftSecondThread(void* pvPara)
{
	DTRACE(DB_CRITICAL, ("DownSoftSecondThread : started!\n"));

	while(1)
	{
	    if (IsUpdateFirmware())
			break;

		if (GetInfo(INFO_APP_RST))	//CPU��λ
		{
			DTRACE(DB_CRITICAL, ("DownSoftSecondThread : rx INFO_APP_RST......\n"));
			Sleep(1000);
			ResetCPU();
		}

		Sleep(1000);
	}
	
	return 0;
}
*/

TThreadRet  Do485MeterSearch(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("Do485MeterSearch-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊ60��
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

bool g_fPowerOffSave = false;		//������Ƿ���Ҫ��������
//��Դ����
void DoPowerManagement()
{
	BYTE bIsValid;
/*
	static bool fPowoffInit = false;
	if (!fPowoffInit && g_pPowerOff != NULL && GetClick()>10)
	{
		fPowoffInit = g_pPowerOff->InitRun(0, 14);
	}

	if (g_TaskManager.m_fInited && fPowoffInit && g_pPowerOff)	//ͣ�ϵ�澯��Ҫ������ȡ��������ֹ�������߳��У���Ϊ�����������¸澯����ʧ��
		g_pPowerOff->RunTask();
*/
#ifndef SYS_WIN	
	BYTE bBuf[64];
	DWORD dwClick = GetClick();
	CheckPower();
	if (!IsAcPowerOff(&bIsValid) && bIsValid==0)  //�����е�
	{	
		g_dwAcOffClick = 0;
		g_fPowerOffSave = false;		//������Ƿ���Ҫ��������
		
		if (g_dwAcUpClick == 0)
		{
			g_dwAcUpClick = dwClick;
			return;
		}
		
		if (dwClick-g_dwAcUpClick >= 3)
		{
			g_dwAcUpClick = 0;
			g_fAcUp = true;  //��������û������,��Ҫ�����ж��Ƿ��ǰ�������
			if (g_fFaShutDown)  //��ǰ�ع����
			{
			    GprsBatOnOff(true); //���´��������
				BackBatOnOff(true); //�򿪺󱸵��
				g_fFaShutDown = false;  //ֻҪ������������,�Ͳ�ȥ�ػ�
			}
		}
	}
	else		//������ͣ��,��ǰ��Դ�ɵ�ع���		
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
		if (wAcPn != PN0)	//���ɲ�����
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
			GprsBatOnOff(true);//�򿪺󱸵��
			BackBatOnOff(true); //�򿪺󱸵��
			g_dwAcOffClick = GetClick();
			g_fPowerOffSave = true;
			#ifdef VER_LIAONING
			InitLoopCnt();
			#endif
			return;
		}
		
		if (dwClick-g_dwAcOffClick >= 6)
		{
			if (g_fPowerOffSave && g_fAcUp && (dwClick-g_dwAcOffClick >= 30))	//�����30�봥����������
			{
				TrigerSave();
				TrigerSavePara();
				//TdbTrigerSave();
				g_fPowerOffSave = false;
			}
				
			if (dwClick-g_dwAcOffClick >= 5*60)	//5*60 zqq add
			{	//��ǰ���е��,����ͣ����,Ҫ�ȵ�ͣ��澯��ȷ���˲Źضϵ��
				//���� ͣ�糬��5�����������Ҳ�ùص��
				//TdbDoSave();
				FaSave();	//��Դ�ܳŵ�����,Ӧ��Ҳ���㹻�ĵ�������������
				g_fFaShutDown = true;  //֪ͨ����߳�ȥ�ػ�
				Sleep(60*1000);        //��GPRS�ػ�
				while (GetClick()-dwClick < 5)
				{
					Sleep(1000); //ȷ��������������ʾ���
				}	
                DTRACE(DB_CRITICAL, ("DoPowerManagement : BackBatOff!\r\n"));
				Sleep(10*1000);        //��GPRS�ػ�
				GprsBatOnOff(false); //��GPRS��Դ
				BackBatOnOff(false); //�رպ󱸵��
				DTRACE(DB_CRITICAL, ("DoPowerManagement : g_dwAcOffClick := 0!\n"));
				g_dwAcOffClick = 0;
				Sleep(3*60*1000); //��Ҫ������ʹ����ѭ�����п�����;�������ˣ�����������
			}

			#ifdef VER_LIAONING
			if (g_fAcUp==false && GetLoopCnt() >= 1 && dwClick-g_dwAcOffClick >= 30)
			#else
			if (g_fAcUp==false && dwClick-g_dwAcOffClick >= 2*60)
			#endif
			//if (g_fAcUp==false && dwClick-g_dwAcOffClick>=1*30)	//TO DO:����2�ܹص�� //zqq add
			{	//���������û��������,���ǰ�������,���������ܺ�ضϵ��
			
				//FaSave();	//�������ѾͲ�����������,����������
				g_fFaShutDown = true;  //֪ͨ����߳�ȥ�ػ�
				while (GetClick()-dwClick < 5)
				{
					Sleep(500); //ȷ��������������ʾ���
				}	

				GprsBatOnOff(false); //��GPRS��Դ
				BackBatOnOff(false); //�رպ󱸵��
				g_dwAcOffClick = 0;
				Sleep(10000); //��Ҫ������ʹ����ѭ�����п�����;�������ˣ�����������
			}
		}
	}
#endif
}



#define RESTART_SYSTEM	1	//����ϵͳ
#define EXEC_INSTANCE	2	//ͨ�÷�����ִ�о��������
#define DATA_INIT		3	//���ݳ�ʼ��
#define PARAM_INIT		4	//������ʼ��
#define EVENT_INIT		5	//�¼���ʼ��
#define DEMAND_INIT		6	//������ʼ��


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
				sprintf(pszTabName, "rm -rf %s%s_%03d_*", USER_DATA_PATH, GetSchTableName(tTaskCfg.bSchType), tTaskCfg.bSchNo);
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
	char command[64] = {0};

	sprintf(command, "rm -rf %sCommSch*", USER_DATA_PATH);
	system(command);
	memset(command, 0, sizeof(command));
	sprintf(command, "rm -rf %sEvtSch*", USER_DATA_PATH);
	system(command);
	memset(command, 0, sizeof(command));
	sprintf(command, "rm -rf %sRptSch*", USER_DATA_PATH);
	system(command);
	memset(command, 0, sizeof(command));
	sprintf(command, "rm -rf %sAcqRule*", USER_DATA_PATH);
	system(command);
	memset(command, 0, sizeof(command));
	sprintf(command, "rm -rf %sRptSch*", USER_DATA_PATH);
	system(command);
	memset(command, 0, sizeof(command));
	sprintf(command, "rm -rf %sTaskCfgUnit_*", USER_DATA_PATH);
    system(command);    
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
			bSect==SECT_FROZEN_DATA || bSect==SECT_VARIABLE)	//20170102 hyl ����Ҫ����¼�DATA���¼����ֻᵥ������
			ClearBankData(BN0, bSect);
	}
	ClearBankData(BN16, -1);
	DelSchData();
#ifndef SYS_WIN
	AcClearDemand();
	AcClearLog();
	InitSample();//������󲻸�λ��Ҫ���³�ʼ���ڴ����
#endif
	UnLockDB();

	for (WORD wId=0; wId<TASK_ID_NUM; wId++)
		ClrTaskMoniStat(wId);

	SaveMangerMtrCacheCtrl(true);
	DeleteMtrRdCtrl();
	ClearMtrCacheCtrl();	//CL add 170718
	InitMtrCacheCtrl();
	//RefreshMtrCacheCtrl();   //changed by whr 20170711
}

void FaResetPara()
{
	LockDB();

	for (BYTE bSect=0; bSect<SECT_NUM; bSect++)
	{
		if (bSect==SECT_EVENT_DATA)//20170102 hyl ����Ҫ����¼�DATA���¼����ֻᵥ������
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


//���»�ȡͣ��ʱ��,������������ͣ���¼�, ��ͣ���¼���Ч��־��Ϊ��Чʱ�����ݳ�ʼ�����¼���ʼ��ʱ����
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
		SaveMangerMtrCacheCtrl();	//��λǰ�ȱ��泭����ṹ

#ifdef SYS_LINUX
		system("/clou/ppp/script/ppp-off");
		SetInfo(INFO_STOP_FEED_WDG);	//ֹͣι��	
#endif
		Sleep(10*1000);
		ResetCPU();
		break;
	case EXEC_INSTANCE:
		break;
	case DATA_INIT:
		FaResetData_1();
		MtrExcOnRxFaResetCmd();		//�����¼���������
		FrzTaskOnRxFaResetCmd();	//�������ݳ�ʼ��
		SetInfo(INFO_RST_TERM_STAT); 
		SetInfo(INFO_HARDWARE_INIT);
		UpdateTermPowerOffTime();	//��Ҫ����DealSpecTrigerEvt(MTR_MTRCLEAR)ǰ��

		WaitSemaphore(g_semTermEvt);
		DealSpecTrigerEvt(MTR_MTRCLEAR);
		DealSpecTrigerEvt(TERM_INIT);//��Ҫ����DealSpecTrigerEvt(MTR_MTRCLEAR)����
		SignalSemaphore(g_semTermEvt);
		
		//Sleep(10*1000);
		//ResetCPU();	//ESAM���ݳ�ʼ��ʱ��Ӧ�������ն�
		break;
	case PARAM_INIT:
		FaResetPara();
		InitTermEvt();
		MtrExcOnRxFaResetCmd();		//�����¼���������
		FrzTaskOnRxFaResetCmd();	//�������ݳ�ʼ��
		SetInfo(INFO_RST_TERM_STAT); 
		UpdateTermPowerOffTime();
		
		WaitSemaphore(g_semTermEvt);		
		DealSpecTrigerEvt(MTR_MTRCLEAR);
		DealSpecTrigerEvt(TERM_INIT);
		SignalSemaphore(g_semTermEvt);
		
		Sleep(10*1000);
		ResetCPU();	
		break;
	case EVENT_INIT:
		FaResetEvent();
		MtrExcOnRxFaResetCmd();		//�����¼���������
		UpdateTermPowerOffTime();

		WaitSemaphore(g_semTermEvt);		
		DealSpecTrigerEvt(MTR_EVTCLEAR);
		DealSpecTrigerEvt(TERM_INIT);
		SignalSemaphore(g_semTermEvt);

		//ResetCPU();
		Sleep(10*1000);
		break;
	case DEMAND_INIT:
		FaResetDemand();

		WaitSemaphore(g_semTermEvt);		
		DealSpecTrigerEvt(MTR_DMDCLEAR);
		DealSpecTrigerEvt(TERM_INIT);
		SignalSemaphore(g_semTermEvt);

		Sleep(10*1000);
		break;
	default:
		DTRACE(DB_TASK, ("FaDataInit(): Method=%d unspport\n", bMethod));
		break;
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
		if (bSignStren >= 100)//TD��ģ���ź�ǿ�ȷ�ΧΪ100-199
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
		if (bSignStren == 0)  //0ȫ�������Ϊ�ε�ģ�飬������̫��ʱʹ��
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


/* ����������ű�����ʼִ�д��ʱ������/tmp/update.logд0�� �����ɡ�������������/tmp/update.logд1��
*  �����ֳ���ͣ�Ķ�ȡ/tmp/update.log�ļ���������Ϊ1ʱ����λ������
*/
bool IsTermUptateEnd()
{
    BYTE bBuf[8] = {0};
    if(!PartReadFile("/tmp/update.log", 0, bBuf, 1))
    {
        return false;
    }

    if(bBuf[0] == '1')
    {        
        return true;
    }
    else
    {   
        return false;
    }
}

void WriteCfgPathName(void)
{
	char szCfgBuf[50];

	memset(szCfgBuf, 0, sizeof(szCfgBuf));
	sprintf(szCfgBuf, "clou.dft");
	WriteFile(USER_CFG_PATH"cfgpermit.cfg", (BYTE*)szCfgBuf, strlen(szCfgBuf));
	DTRACE(DB_FA, ("WriteCfgPathName: write cfgpermit.cfg!\r\n"));
}



//extern bool IsMountedOK(char *str);
void DoCheckUsb(void)
{
	char str[64] = {0};
	strcpy(str, "/mnt/usb");

	
	if (!IsInUsbProcess())
	{
		if (IsMountedOK(str))
		{
			SetMountUsb(1);
			//DTRACE(0, ("USB IN\r\n"));
		}
		else
		{
			SetMountUsb(0);
			//DTRACE(0, ("USB WAIT\r\n"));
		}
	}
	else
	{
		if (!IsMountedOK(str))
		{
			SetUsbProcessState(0);
			SetMountUsb(0);
			//DTRACE(0, ("USB OUT\r\n"));
		}
	}
	
}




//����:���߳�
TThreadRet SlowSecondThread(void* pvPara)
{
		int iMonitorID = ReqThreadMonitorID("SlowSecond-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊ5����

	DTRACE(DB_CRITICAL, ("SlowSecondThread : started!\n"));
	//SaveSoftVerChg();

	while (1)
	{
		if (GetInfo(INFO_UPD_MTR_CTRL))
		{
			RequestThreadsSem();
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_UPD_MTR_CTRL start ###\r\n"));
			SetThreadDelayFlg();
			SaveMangerMtrCacheCtrl(true);
			DeleteMtrRdCtrl();
			ClearMtrCacheCtrl();
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_UPD_MTR_CTRL end ###\r\n"));
			ReleaseThreadsSem();
		}

		if (GetInfo(INFO_MTR_UPDATE))
		{
			RequestThreadsSem();
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_MTR_UPDATE start ###\r\n"));
			SetThreadDelayFlg();
			InitMtrMask();
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_MTR_UPDATE end ###\r\n"));
			ReleaseThreadsSem();
		}

		if (GetInfo(INFO_MTR_ALL_CLEAR))
		{
			RequestThreadsSem();
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_MTR_ALL_CLEAR start ###\r\n"));
			SetThreadDelayFlg();
			DelSchData();
			InitMtrMask();
			SaveMangerMtrCacheCtrl(true);
			DeleteMtrRdCtrl();
			SetInfo(INFO_SYNC_MTR);
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_MTR_ALL_CLEAR end ###\r\n"));
			ReleaseThreadsSem();
		}

		if (GetInfo(INFO_TASK_CFG_UPDATE))
		{
			RequestThreadsSem();
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_TASK_CFG_UPDATE start ###\r\n"));
			SetThreadDelayFlg();

			ClearSchData();
			InitTaskMap();
 			InitSchMap();
 			InitSchTable();
//			DeleteMtrRdCtrl();
			SetInfo(INFO_SYNC_MTR);
//			InitMtrCacheCtrl();

			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_TASK_CFG_UPDATE end ###\r\n"));
			ReleaseThreadsSem();
		}

		if (GetInfo(INFO_ACQ_SCH_UPDATE))
		{
			RequestThreadsSem();
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_ACQ_SCH_UPDATE start ###\r\n"));
			SetThreadDelayFlg();
			//DelSchData();	//��ClearSchDataɾ��
			ClearSchData();
			InitTaskMap();
			InitSchMap();
			InitSchTable();
			//DeleteMtrRdCtrl();	//���ﲻ��ɾ��������ƽṹ��ͳһ��᯹�����Ľӿڴ���
			//InitMtrCacheCtrl();
			SetInfo(INFO_SYNC_MTR);
			
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_ACQ_SCH_UPDATE end ###\r\n"));
			ReleaseThreadsSem();
		}
		if (GetInfo(INFO_RP_SCH_UPDATE))
		{
			RequestThreadsSem();
			SetThreadDelayFlg();
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_RP_SCH_UPDATE start ###\r\n"));
			
			ClearSchData();
			InitSchMap();
			InitSchTable();
			//ClearBankData(BN16, 0, -1);	���������ܲ������ᵼ��bank16���������������������
			//ClearReportParam();

			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_RP_SCH_UPDATE end ###\r\n"));
			ReleaseThreadsSem();
		}

		if (GetInfo(INFO_CLASS19_METHOD_RST) || GetInfo(INFO_APP_RST))	//Ӳ����ʼ��
		{
			RequestThreadsSem();
			SetThreadDelayFlg();
			DTRACE(DB_CRITICAL, ("Dev interface class=19 method=1. Reset system...\n"));
			FaInit(RESTART_SYSTEM);
			//ReleaseThreadsSem(); Ӳ����ʼ����Ҫ��λ�նˣ����ﲻ�ͷ�������ֹ�����߳�����
		}

		if (GetInfo(INFO_CLASS19_METHOD_EXE))	
		{
			DTRACE(DB_CRITICAL, ("Dev interface class=19 method=2. Exe...\n"));
			FaInit(EXEC_INSTANCE);
		}

		if (GetInfo(INFO_CLASS19_METHOD_DATA_INIT))	
		{
			RequestThreadsSem();
			SetThreadDelayFlg();
			DTRACE(DB_CRITICAL, ("Dev interface class=19 method=3. Data init...\n"));
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_CLASS19_METHOD_DATA_INIT start ###\r\n"));
			FaInit(DATA_INIT);
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_CLASS19_METHOD_DATA_INIT end ###\r\n"));
			ReleaseThreadsSem();
		}

		if (GetInfo(INFO_CLASS19_METHOD_RST_FACT_PARA))	
		{
			DTRACE(DB_FA, ("Dev interface class=19 method=4. Restore factory para...\n"));
			WriteCfgPathName(); //�ָ������������Զ�Ӧ�������ļ�
			RequestThreadsSem();
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_CLASS19_METHOD_RST_FACT_PARA start ###\r\n"));
			SetThreadDelayFlg();
			FaInit(PARAM_INIT);
			DTRACE(DB_CRITICAL, ("### SlowSecondThread(): INFO_CLASS19_METHOD_RST_FACT_PARA end ###\r\n"));
			ReleaseThreadsSem();
		}

		if (GetInfo(INFO_CLASS19_METHOD_EVT_INIT))	
		{
			DTRACE(DB_CRITICAL, ("Dev interface class=19 method=5. Event init...\n"));
			FaInit(EVENT_INIT);
		}

		if (GetInfo(INFO_CLASS19_METHOD_DEM_INIT))	
		{
			DTRACE(DB_CRITICAL, ("Dev interface class=19 method=6. Demand init...\n"));
			FaInit(DEMAND_INIT);
		}		

		if (GetInfo(INFO_DISCONNECT)) //���ݸ�λ
		{
			DTRACE(DB_CRITICAL, ("SlowSecondThread : rx INFO_DISCONNECT......\n"));
			g_pGprsFaProtoIf->SetDisConnect(); //���յ��ⲿ�ĶϿ���������ʱ,���ñ�����֪ͨ�ӿ�
			g_pEthFaProtoIf->SetDisConnect();
		}
        
        if (IsTermUptateEnd())  
        {// Ϊ���� class=19 method=1����ļ�������λ�� �������︴λ���ӵ�����Ϣ��������
            DTRACE(DB_CRITICAL, ("term update success, Reset system...\n"));
            FaInit(RESTART_SYSTEM);
        }

		DoMangerMtrCacheCtrl();
		
		DoFaSave();
		DoFapCmd();
		DoPowerManagement();
		CheckSignStrength();
		DoCheckUsb();
		
		UpdThreadRunClick(iMonitorID);
		Sleep(1000);
	}	

	ReleaseThreadMonitorID(iMonitorID);
	return THREAD_RET_OK;
}

//����ģ���߳�--����ģ���LED�ƣ��̵��������߼��
//ֻ��ԡ���Ƭ�����͡��Ŀ���ģ�� ��Ч
TThreadRet CtrlMoudleThread(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("CtrlMoudleThread-thrd", 60*60*60);	//�����̼߳��ID,���¼��Ϊ60����

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
				DoCtrlLoop();//���߼��
				bCnt = 0;
			}

			DoAlrLedCtrl();//�澯�ƿ���
			DoLedByPulse();//�̵���--�����������
			bCnt ++;
		}
		else
		{
			bCnt = 0;
		}
		Sleep(50);
		
		UpdThreadRunClick(iMonitorID);
	}

	ReleaseThreadMonitorID(iMonitorID);
	return THREAD_RET_OK;
}
//Ŀǰ�˿ںŵ�ת����Ҫ�����������������:
//�߼��˿ں�wLogicPort-->�ڲ��̶����wInSn-->����˿ں�wPhyPort



void GetLogicPortNum(WORD* pwNum, WORD* pwMin, WORD* pwMax)
{
	*pwNum = LOGIC_PORT_NUM;
	*pwMin = LOGIC_PORT_MIN;
	*pwMax = LOGIC_PORT_MAX;
}

//����:�߼��˿ںŵ��ڲ��̶���ŵ�ת��
//����:@wLogicPort �߼��˿ں�
//����:�����ȷ�򷵻��ڲ��̶����,���򷵻�-1
int LogicPortToInSn(WORD wLogicPort)
{
	if (wLogicPort<LOGIC_PORT_MIN || wLogicPort>LOGIC_PORT_MAX)
		return -1;

	//1.�߼��˿ں�-->�ڲ��̶���ŵ�ת��
	WORD wInSn = wLogicPort + 1 - LOGIC_PORT_MIN;	//����485�� 2 3 4
		
	BYTE bOrder = 0;
	ReadItemEx(BN10, PN0, 0xa166, (BYTE*)&bOrder);	//0xa166 1 ���������˳��,0:��->��ֱ��ǿ�1,��2...; 1:��->�ҷֱ��ǿ�1,��2...

	//����Ǵ���->���˳��ʼ���1,2...,���߼���==wMeterPort,����Ҫ����
	//����Ǵ���->�ҵ�˳��ʼ���1,2...,���߼���!=wMeterPort,��Ҫ����

	if (bOrder == 1)	//����->�ҵ�˳��ʼ���1,2...
		wInSn = LOGIC_PORT_NUM - wInSn + 1;

	return wInSn;
}

//����:�߼��˿ںŵ�����˿ںŵ�ӳ��
//����:@wLogicPort �߼��˿ں�
//����:�����ȷ�򷵻�����˿ں�,���򷵻�-1
int InSnToPhyPort(WORD wInSn)
{
	if (wInSn==0 || wInSn>LOGIC_PORT_NUM)	//�Ƿ����߼��˿ں�
		return -1;
	else
		return g_iInSnToPhyPort[wInSn-1];
}

//����:�߼��˿ںŵ�����˿ںŵ�ӳ��
//����:@wLogicPort �߼��˿ں�
//����:�����ȷ�򷵻�����˿ں�,���򷵻�-1
int LogicPortToPhy(WORD wLogicPort)
{
	//1.�߼��˿ں�-->�ڲ��̶���ŵ�ת��
	int iInSn = LogicPortToInSn(wLogicPort);
	if (iInSn < 0)
		return iInSn;

	return InSnToPhyPort((WORD )iInSn);
}

//����:ȡ���ڲ��̶���Ŷ˿ڹ�������
//��ע:0xa131��0xa132���õ����߼��˿�,�̶�����->���˳��ʼ���
int GetInSnPortFun(WORD wInSn)
{
	WORD wID;
	BYTE b;
	int iPortFun = -1;

	if (IsDownSoft())
		return -1;

	if (wInSn == 1)	//485-1��
		wID = 0xa131;
	else if (wInSn == 2)	//485-2��
		wID = 0xa132;
	else if (wInSn == 3)	//485-3��
		wID = 0xa180;
	else
		return -1;
	
	int iLen = ReadItemEx(BN10, PN0, wID, (BYTE*)&b);
	if (iLen <= 0)
		return -1;
	
	iPortFun = b;
		//liuzhixing20170526
	//		//3�����ڵ������
	//		if ((iPortFun==PORT_FUN_DEBUG||iPortFun==PORT_FUN_LOCAL485) && wInSn==3)	//485-3�����ڵ��Կڻ�ά���ڣ�������
	//			return -1;

		
	if (iPortFun<0 || iPortFun>PORT_FUN_JC485) //���óɷǷ����ݶ�Ĭ�ϳɳ����
		iPortFun = PORT_FUN_RDMTR;
		
#ifdef EN_SPECIAL_TRANSFORMER
	if (iPortFun > PORT_FUN_LINK)
		iPortFun = PORT_FUN_RDMTR;
#endif

	return iPortFun;
}

//����:ȡ���߼��˿ںŵĹ�������
int GetLogicPortFun(WORD wLogicPort)
{
	//1.�߼��˿ں�-->�ڲ��̶���ŵ�ת��
	int iInSn = LogicPortToInSn(wLogicPort);
	if (iInSn < 0)
		return iInSn;

	return GetInSnPortFun((WORD )iInSn);
}

//����:�����ڲ��̶���Ŷ˿ڹ�������
int SetInSnPortFun(WORD wInSn, BYTE bPortFun)
{
	WORD wID;

	if (IsDownSoft())
		return -1;

	if (wInSn == 1)	//485-1��
		wID = 0xa131;
	else if (wInSn == 2)	//485-2��
		wID = 0xa132;
	else if (wInSn == 3)	//485-3��
		wID = 0xa180;
	else
		return -1;
	
	WriteItemEx(BN10, PN0, wID, (BYTE*)&bPortFun);
	return 0;
}

//����:�����߼��˿ڵĹ�������
int SetLogicPortFun(WORD wLogicPort, BYTE bPortFun)
{
	//1.�߼��˿ں�-->�ڲ��̶���ŵ�ת��
	int iInSn = LogicPortToInSn(wLogicPort);
	if (iInSn < 0)
		return iInSn;

	return SetInSnPortFun((WORD )iInSn, bPortFun);
}

//����:����ڵ��߼��˿ڵ�����˿ڵ�ӳ��
//����:�����ȷ����˿ں�,���򷵻ظ���
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

//����:�����ڵ��߼��˿ڵ�����˿ڵ�ӳ��
//����:�����ȷ����˿ں�,���򷵻ظ���
int GetLinkPhyPort()
{	
	for (WORD i=LOGIC_PORT_MIN; i<=LOGIC_PORT_MAX; i++)	//����Ӳ���ϵ�3���߼��˿ڽ���ѭ��
	{
		if (GetLogicPortFun(i) == PORT_FUN_LINK)
			return LogicPortToPhy(i);
	}

	return -1;
}


//����:������ڵ��߼��˿ڵ�����˿ڵ�ӳ��
//����:�����ȷ����˿ں�,���򷵻ظ���
int GetInMtrPhyPort()
{
	for (WORD i=LOGIC_PORT_MIN; i<=LOGIC_PORT_MAX; i++)	//����Ӳ���ϵ�3���߼��˿ڽ���ѭ��
	{
		if (GetLogicPortFun(i) == PORT_FUN_INMTR)
			return LogicPortToPhy(i);
	}

	return -1;
}

//����:�ɼ��ڵ��߼��˿ڵ�����˿ڵ�ӳ��
//����:�����ȷ����˿ں�,���򷵻ظ���
int GetAcqPhyPort()
{
	#ifdef SYS_WIN
		return COMM_DEBUG;
	#endif
	for (WORD i=LOGIC_PORT_MIN; i<=LOGIC_PORT_MAX; i++)	//����Ӳ���ϵ�3���߼��˿ڽ���ѭ��
	{
		int iPortFun = GetLogicPortFun(i);
		if (iPortFun==PORT_FUN_ACQ ||  iPortFun==PORT_FUN_JC485)
			return LogicPortToPhy(i);
	}

	return -1;
}

//����:�ж��߼����Ƿ�Ϊ485�ɼ�����
//����:��485�ɼ����ڷ���true
bool IsAcqLogicPort(BYTE bPort)
{
	int iPortFun = GetLogicPortFun(bPort);
	if (iPortFun==PORT_FUN_ACQ || iPortFun==PORT_FUN_JC485)
		return true;
	
	return false;
}

//��������ʼ��
void FaResetData()
{
#if 0
	int i;
	DTRACE(DB_FAPROTO, ("FaResetData: started...\n"));
	
	FaSave();
	SetInfo(INFO_RST_TERM_STAT); //��λ�ն�ͳ������
	
	LockDB();
	for (i=0; i<SECT_NUM; i++)
	{
		if (i==SECT_KEEP_PARA || i==SECT_TERMN_PARA || i==SECT_PN_PARA || i==SECT_PN_DATA || i==SECT_EXT_PN_DATA)  //�����ļ��ͽ��ɼ������ݳ���
			continue;
			
		ClearBankData(BN0, i);
	}

	for (i=0; i<POINT_NUM; i++)	//�����������
	{
		if (i == PN0 || i == GetAcPn()) //������0�ͽ��ɲ�����������ݳ���
			continue;

		ClearBankData(BN0, SECT_PN_DATA, i);
		ClearBankData(BN0, SECT_EXT_PN_DATA, i);
	}

	ClearBankData(BN11, 0);	//������м�����
	ClearBankData(BN18, 0);	//������м�����
	
	DTRACE(DB_FAPROTO, ("FaResetData: clr tdb files\n"));
	
#ifdef EN_CCT	//����������
	FaResetCctData();
#endif

#ifndef SYS_WIN	
	AcClearLog();
	g_PulseManager.ClearLog();
#endif
	
	//ResetCPU(); //����Э���ﲻ�������︴λ,��Ϊ������Ҫ�����¼�
#endif
}

#ifdef EN_CCT	//����������
//����:�������������
void FaResetCctData()
{
    ClearBankData(BN16, 0); //��������״̬��
	//ClearBankData(BN20, 0); //������ͨ������.
    ClearBankData(BN21, 0); //�����๦�ܱ������.
}

//����:������������ʼ��
void FaResetCctPara()
{
	SetInfo(INFO_PLC_CLRRT); //��·��
}
#endif

//��������ʼ��
void FaResetDataPara()
{
#if 0
	int i;
	DTRACE(DB_FAPROTO, ("FaResetDataPara: started...\n"));
	
	SetInfo(INFO_RST_TERM_STAT); //��λ�ն�ͳ������
#ifdef EN_CCT	//����������
	SetInfo(INFO_PLC_CLRRT);	 //���������·��
#endif
	LockDB();
	ClearWDG();
	for (i=0; i<SECT_NUM; i++)
	{
		if (i==SECT_KEEP_PARA || i==SECT_PN_DATA || i==SECT_EXT_PN_DATA) //���������ļ��ͽ��ɼ������ݳ���
			continue;

		ClearBankData(BN0, i);
	}

	for (i=0; i<POINT_NUM; i++)	//�����������
	{
		if (i == PN0 || i == GetAcPn()) //������0�ͽ��ɲ�����������ݳ���
			continue;

		ClearBankData(BN0, SECT_PN_DATA, i);
		ClearBankData(BN0, SECT_EXT_PN_DATA, i);
	}

	ClearBankData(BN11, 0);	//������м�����
	ClearBankData(BN18, 0);	//������м�����
	//������չ����,���������ÿ���ط��Ĳ�ͬ����������,���Բ������
	
	DTRACE(DB_FAPROTO, ("FaResetDataPara: clr tdb files\n"));
	
#ifdef EN_CCT	//����������
	FaResetCctData();
	FaResetCctPara();
#endif
#ifndef SYS_WIN	
//	g_AcSample.ClearLog();
	g_PulseManager.ClearLog();
#endif
	
//	ResetCPU(); //����Э���ﲻ�������︴λ,��Ϊ������Ҫ�����¼�
#endif
}

//ȫ����������ʼ��
void FaResetAllDataPara()
{
#if 0
	int i;
	DTRACE(DB_FAPROTO, ("FaResetDataPara: started...\n"));
	
	SetInfo(INFO_RST_TERM_STAT); //��λ�ն�ͳ������
	
	LockDB();
	ClearWDG();
	for (i=0; i<SECT_NUM; i++)//���������������
	{	
		if (i == SECT_KEEP_PARA) //���������ļ�����
			continue;
		ClearBankData(BN0, i);
	}
		
	ClearBankData(BN11, 0);	//������м�����
	ClearBankData(BN18, 0);	//������м�����
	//������չ����,���������ÿ���ط��Ĳ�ͬ����������,���Բ������
	
	DTRACE(DB_FAPROTO, ("FaResetDataPara: clr tdb files\n"));
	
#ifndef SYS_WIN	
	AcClearLog();
#endif
	
//	ResetCPU(); //����Э���ﲻ�������︴λ,��Ϊ������Ҫ�����¼�
#endif
}
//���������
void FaClearEnergy()
{
#if 0
	int i;
	DTRACE(DB_FAPROTO, ("FaClearEnergy: started...\n"));

	SetInfo(INFO_RST_TERM_STAT); //��λ�ն�ͳ������

	LockDB();
	ClearBankData(BN0, SECT_PN_DATA, POINT0);
	ClearBankData(BN0, SECT_PN_DATA, GetAcPn());

	ClearBankData(BN0, SECT_EXT_PN_DATA, PN0);
	ClearBankData(BN0, SECT_EXT_PN_DATA, GetAcPn());
	
#ifdef EN_AC	
	AcClearLog();
#endif

	//ResetCPU(); //����Э���ﲻ�������︴λ,��Ϊ������Ҫ�����¼�
#endif
}

//����:��ʼ��Ӧ�ò���,��ЩӦ�ÿ��ܱ�����̵߳���,��ʼ��Ӧ����ŵ�ĳ���߳���,
//	   ��Ϊ�̵߳�ִ���Ⱥ��޷�ȷ��,���ܻᵼ���쳣,Ӧ�ŵ�������,��Ϊ�����������߳��е���
// 	   ��ʼ������һ��ͬ�������첽�ķ�ʽ�½���
void InitApp()
{
	g_StatMgr.Init();	 //ͳ��
	ApllyCfgAuto();//�����Զ�Ӧ�������ļ�
	g_pmParaMgr.ReSetParamKeepReadFile();
}
#ifdef EN_INMTR
void DoAmpHour();
#endif
//���߳�
extern TThreadRet RJ45ReadMtrThread(void* pvPara);


//��ʼ����������
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
	if (b<31)//������/������
	{
		bInValidDemTime[8] = b+1;
	}
	memcpy(&bInValidDemTime[3], &bBuf[5], 4);
	WriteItemEx(BN0, wPn, 0xc117, bInValidDemTime);
	if (wPn != PN0)
		WriteItemEx(BN0, PN0, 0xc117, bInValidDemTime);
}

extern TThreadRet DisplayThread(void* pvArg);

TThreadRet MainThread(void* pvPara)
{
	SysInitDebug();	//��ʼ����ͬϵͳ�µ�����Ͳ�Ĳ���
#ifdef SYS_LINUX
	sys_signal_setup();
#endif
	InitDrivers(HW_CL790D82);
	InitThreadMonitor(); //��ʼ���̼߳��
	int iMonitorID = ReqThreadMonitorID("main-thrd", 60*60);	//�����̼߳��ID,���¼��Ϊ10����

	DTRACE(DB_CRITICAL, ("MainThread : started V1.58!\n"));

	FaInitStep1();
	FaInitDrivers(); //�ⲿ��������صĳ�ʼ��,�õ����ݿ�,���Ա���ŵ�FaInitStep1()��
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
	SaveSoftVerChg();	//���ն˰汾����¼�����Ҫ����InitTermEvt()֮��

#ifdef EN_CTRL	//�Ƿ�������ƹ���
//	if (IsFkTermn())
//		g_LoadCtrl.Init();
#endif

	InitAcData();
#ifndef SYS_WIN
	NewThread(WdgThread, NULL, 8192, THREAD_PRIORITY_ABOVE_NORMAL);
#ifdef EN_AC	//�Ƿ������ɹ���
	InitSample();
#endif
#endif
//	InitVBreakStat();

#ifdef EN_ETHTOGPRS //������̫����GPRS�໥�л�,�ȼ��������ɣ���Ϊ�����ͨ���߳�Ҫ�õ����
	NewThread(CheckNetThread,  NULL, 8192, THREAD_PRIORITY_NORMAL);
#endif

	if (IsDownSoft())
	{
		NewFaUpdateThread();

		NewThread(DriverThread, NULL, 8192, THREAD_PRIORITY_NORMAL);

	#ifndef SYS_WIN
		EnableTrace(false);
		if (g_pLcd != NULL)
		{
			g_pLcd->Clear();			
			if( g_bRemoteDownIP[0] == 0xff )
				g_pLcd->Print("\r����sftp����", 0, 3,false,true);
			else
				g_pLcd->Print("\rԶ��sftp����", 0, 3,false,true);
			g_pLcd->Print("\r�������س���,���Ժ�...", 0, 4,false,true);
			g_pLcd->Refresh();
		}
	#endif

		DWORD dwUpdateTime = 0;
		while(1)
		{
			UpdThreadRunClick(iMonitorID);

			Sleep(1000);
			dwUpdateTime++;
			if(dwUpdateTime > 3*60*60) //�����3��Сʱ
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

#ifdef EN_CTRL	//�Ƿ�������ƹ���
	if (IsFkTermn())	//�����ն˲���Ҫ���ܼ������ݵĴ���
	{
		NewThread(CtrlMoudleThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
		NewThread(LoadCtrlThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
	}
#endif

#ifdef EN_VARCPS //�Ƿ������޹�����(VAR compensator)����
//	NewThread(LoadWGCtrlThread, NULL, 8192, THREAD_PRIORITY_NORMAL);			
#endif

	NewMeterThread();
	NewFaProtoThread();
	//NewThread(TaskDBThread, NULL, 8192, THREAD_PRIORITY_NORMAL);
	
	NewCctThread();

#ifdef EN_INMTR
//	NewInMtrThread();
#endif

#ifdef EN_ETHTOGPRS //������̫����GPRS�໥�л�������̷߳�����������Ǻܿ���һ��ʼ��̫���Ѿ����ߣ����һ�����ֻ��һ����������
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
	
	NewThread(DoTermEvt, NULL, 8192, THREAD_PRIORITY_NORMAL);

	while (1)
	{
		UpdThreadRunClick(iMonitorID);
		//DoTermEvt();
		
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


bool IsFkTermn()
{
	BYTE bRunMode;
	ReadItemEx(BN1, PN0, 0x2100, &bRunMode);
	if (bRunMode == 2)
		return true;
	else
		return false;
}



//��ʾ�Ƿ���U����������
//���أ�true �ѽ��룬 false δ����
bool IsInUsbProcess()
{
	BYTE bState = 0;

	if (ReadItemEx(BN2, PN0, 0x2112, &bState) > 0)
		return (bState!=0);
	else
		return false;
}

//U���Ƿ����
//���أ�true �Ѳ��룬 false δ����
bool IsMountUsb()
{
	BYTE bState = 0;

	if (ReadItemEx(BN2, PN0, 0x2111, &bState) > 0)
		return (bState!=0);
	else
		return false;
}

void SetMountUsb(BYTE bState)
{
	WriteItemEx(BN2, PN0, 0x2111, &bState);
}

//����USB��������Ƿ����״̬ 0��δ���룬1���ѽ���
void SetUsbProcessState(BYTE bState)
{
	WriteItemEx(BN2, PN0, 0x2112, &bState);
}


//�����������̵߳��ź���
void RequestThreadsSem()
{
	StopMtrRd(0xffff); //ֹͣ����
	//485-1,485-2,485-3
	for (BYTE i=LOGIC_PORT_MIN; i<LOGIC_PORT_MAX; i++)
		WaitSemaphore(g_semRdMtr[i-LOGIC_PORT_MIN]);

	//PLC
#if (FA_TYPE == FA_TYPE_C82) 
		g_CStdReader->LockReader();	
#endif
}

//�������ͷ��̵߳��ź���
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



#define THREAD_MASK_ID	(THRD_MNTR_NUM/8+1)
#define  THREAD_DELAY_TIME		15	//�߳��ӳ�15Sִ��
BYTE g_bThreadDelayFlag[THREAD_MASK_ID] = {0};
DWORD g_dwThreadClick[THRD_MNTR_NUM] = {0};

//���������������߳���ʱ��ʶ
void SetThreadDelayFlg()
{
	memset(g_bThreadDelayFlag, 0xff, sizeof(g_bThreadDelayFlag));
}

//��������ȡ�߳��Ƿ��������
bool GetThreadRunFlag(BYTE bThrId)
{
	if (g_bThreadDelayFlag[bThrId/8] & (1<<(bThrId%8)))
	{
		g_bThreadDelayFlag[bThrId/8] &= ~(1<<(bThrId%8));
		g_dwThreadClick[bThrId] = GetClick();
	}

	if (GetClick()-g_dwThreadClick[bThrId] > THREAD_DELAY_TIME)
		return true;

	return false;
}





BYTE BaudrateToGbNum(DWORD dwBaudRate)
{
	const WORD GBBaudTab[8] = {CBR_300, CBR_600, CBR_1200, CBR_2400, CBR_4800, 0, CBR_9600, CBR_19200};

		for (BYTE i=0; i<sizeof(GBBaudTab)/sizeof(WORD); i++)
		{
			if (GBBaudTab[i] == dwBaudRate)
				return i;
		}	
		return 0;
};


//485�˿�ת������
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
	/*iPort = MeterPortToPhy(wPort); // ������߼��˿ڵ�����˿ڵ�ӳ��
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
			if (g_dwFileTransCurSec > dwCurSec ||dwCurSec > g_dwFileTransCurSec+60*10)//10���Ӷ�û���·���������������������־
			{
				g_fDownSoft = false;
				g_dwFileTransCurSec = 0;
			}
		}
	}

}

