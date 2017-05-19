#ifndef FAAPI_H
#define FAAPI_H

#include "stdio.h"
#include "FaStruct.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "FaProto.h"
#include "TaskManager.h"
#include "FapLink.h"
#include "sysapi.h"
#include "sysdebug.h"
#include "ComAPI.h"
#include "ProAPI.h"
#include "DbAPI.h"
#include "FrzTask.h"
#include "MeterConst.h"
#include "TermEvtTask.h"
#include "Pulse.h"
#include "OoFmt.h"
#include "StatMgr.h"
#ifndef SYS_WIN
#include "AcSample.h"
#endif

extern CQueue g_LinkDownQueue;       //转发给从终端的报文队列
extern CQueue g_LinkUpQueue;           //转发给主站的报文队列
extern bool g_fEnableSlaveReport;      //是否允许主动上报
extern CStatMgr g_StatMgr;

extern WORD  g_wAdjUn;
extern DWORD g_dwTaskMaskClick;
extern DWORD g_dwWrOpClick;
extern bool g_fCurveChange;
extern bool g_fCommTaskChg;
extern TItemDesc  g_TermnParaDesc[];
extern TItemDesc  g_PointParaDesc[];
extern TItemDesc  g_PointDataDesc[];
extern TItemDesc  g_Dlms_TermnParaDesc[];
extern TItemDesc  g_Dlms_TermnExtParaDesc[];
extern TItemDesc  g_Dlms_TermnCtrlDesc[];
extern CDataManager g_DataManager;

extern CComm g_commLocal;
extern CComm g_commTest;
extern CComm g_commRs485[LOGIC_PORT_NUM];
extern DWORD  g_dwClick;
extern TSem   g_semClick;
extern TSem   g_semTask;
extern TSem   g_semExcTask;
extern TSem   g_semMeter485;
extern TSem   g_semFaProto;
extern TSem   g_semMeterLog;
extern TSem   g_semNetStat;
extern TSem   g_semGateWay; //获取网关
extern TSem   g_semGetIP;
extern TSem   g_semClr645Data;
extern TSem   g_semLcd;
extern TSem   g_semRWCtrlModl;
extern TSem   g_semTermEvt;



extern bool g_fTest232;
extern BYTE g_bReadMtrStatus[POINT_NUM];
extern BYTE g_b485PortStatus;
extern bool g_fMtrFailHapFlg[POINT_NUM];
extern bool g_f485FailHapFlg;
extern bool g_fCTValid;
extern TPowerOffTmp g_PowerOffTmp;     //掉电暂存变量
extern unsigned int RemoteDownFlag;
extern BYTE g_bRemoteDownIP[8];  //远程升级IP地址
extern DWORD g_dwExtCmdFlg;
extern DWORD g_dwExtCmdClick;
extern bool g_fFapNeedConnect;
extern BYTE g_bDefaultCfgID;
extern BYTE g_bEnergyClrPnt;
extern bool g_fClearHistory;
extern BYTE g_bClrEnergyStep;	//清电能状态机

extern bool g_fBatPowerOff;		//当前电源状态标志	当主电源断且Vbat<=3.65V置位，当主电源上电或主电源断且Vbat>=3.75清零。
extern bool g_fRxTmpCtrl;
extern	BYTE	g_PulseFlag;
extern bool g_fFrzInit;
extern TTime g_tPowerOn;

extern CQueue g_Queue;     //协议线程的报文消息队列
extern int g_iRstCnt[3]; //8010,8011,8012三个通道登录连续失败的次数
extern int g_iCurChannel; //当前使用的主站地址 0-主通讯通道, 1-备用通道1, 2-备用通道2

extern TSoftVerChg g_SoftVerChg; 
extern bool g_fMasterTerm;
extern WORD g_wLinkInterv;

extern BYTE g_bTermChanelInfo[5];
extern TParaChg g_ParaChg;	
extern void SetParaChg(WORD wClass, BYTE* pbObis);
extern void SaveParaChgEvt();

extern bool IsMountUsb();
extern bool IsInUsbProcess();
extern void SetUsbProcessState(BYTE bState);


extern bool g_fDownSoft;	//是否在下载软件
inline bool IsDownSoft()	//是否在下载软件
{
	return g_fDownSoft;
}

WORD CRCCheck(BYTE* p, WORD wLen);
BYTE NumToStopBits(BYTE n);
BYTE NumToParity(BYTE n);
DWORD NumToBaudrate(BYTE n);
BYTE MeterProtoLocalToInner(BYTE bLocal, BYTE bProto);
bool IsDebugOn(BYTE bType);
void ClearMeterInf();
void SearchMeterInf();
int InitItemDesc(TItemDesc* pItemDesc, WORD num);
TItemDesc* BinarySearchItem(TItemDesc* pItemDesc, WORD num, WORD wID);
int BinarySearchIndex(TItemDesc* pItemDesc, WORD num, WORD wID);
bool SavePoweroffTmp();

void ClearWDG();
char* ErrNumToStr(WORD wNO);
bool AdjTime();
void DoCtrlLoop();
bool GetBatStat();

inline bool IsFapNeedReport()
{
	return g_Queue.GetMsgNum() > 0;
}

//描述:是否是级联的主终端
inline bool IsMasterTerm()
{
	return g_fMasterTerm;
}

bool IsPowerOff();
bool IsAcPowerOff(BYTE* pbIsValid);
CSocketIf* NewProtoSvrIf(int nCnt, BYTE bSockType);
bool InitFaProto(BYTE bComPort);
void FaInitStep1();

void FaClose();
bool FaSave();
bool FaSavePara();
int SearchAlrID(WORD wID, WORD* pAlrID, WORD wNum);

bool PswCheck(BYTE bPerm, BYTE* pbPassword);
int PermCheck(TItemDesc* pItemDesc, BYTE bPerm, BYTE* pbPassword);
bool IsUpdateFirmware();
void GetLogicPortNum(WORD* pwNum, WORD* pwMin, WORD* pwMax);
int GetLogicPortFun(WORD wPortNo);
int SetLogicPortFun(WORD wLogicPort, BYTE bPortFun);
int LogicPortToPhy(WORD wLogicPort);
int MeterPortToPhy(WORD wMeterPort);
int GetLinkPhyPort();
int GetInMtrPhyPort();
int GetAcqPhyPort();
bool IsAcqLogicPort(BYTE bPort);
bool IsMtr485Port(BYTE bPort);
void InitTestMode();
bool IsBBPlcMode();
void SetYxInitFlag(bool fInit);
void UpdateTermPowerOffTime();

//通知消息:
void InitInfo();
void SetInfo(WORD wID, bool fInfo);
bool GetInfo(WORD wID);
void SetDelayInfo(WORD wID);

int ReadItem(WORD wImg, WORD wPn, WORD wID, BYTE* pbBuf, 
			DWORD dwStartTime, DWORD dwEndTime, 
			TBankCtrl* pBankCtrl);
int WriteItem(WORD wImg, WORD wPn, WORD wID, BYTE* pbBuf, 
			  BYTE bPerm, BYTE* pbPassword, DWORD dwTime,
			  TBankCtrl* pBankCtrl);

#ifdef SYS_WIN32
	inline WORD GetCurrenPolarity(void) { return 0; };
#else
	WORD GetCurrenPolarity(void);
#endif //SYS_WIN32

//线程
TThreadRet MainThread(void* pvPara);
TThreadRet WdgThread(void* pvPara);
TThreadRet DriverThread(void* pvPara);
TThreadRet FastSecondThread(void* pvPara);
TThreadRet SlowSecondThread(void* pvPara);
//TThreadRet TaskThread(void* pvPara);
TThreadRet LoadCtrlThread(void* pvPara);
TThreadRet SoundThread(void* pvPara);
TThreadRet LocalDownloadThread(void* pvPara);
TThreadRet SoftDownThread(void* pvPara);
//TThreadRet DataProcThread(void* pvPara);
TThreadRet DataStatThread(void* pvPara);
TThreadRet DownSoftSecondThread(void* pvPara);
TThreadRet Do485MeterSearch(void* pvPara);

extern void FaResetData();
extern void FaResetDataPara();
extern void FaResetAllDataPara();
#ifdef EN_CCT
extern void FaResetCctData();
extern void FaResetCctPara();
#endif
extern void FaClearEnergy();
extern bool IsInProgState();
extern bool IsFkTermn();

//描述：初始化线程ID
void InitThreadMaskId(BYTE bId);

void InitThreadExeFlg();

void ClearThreadExeFlg();

void SetThreadExeFlg();

void ClearRecvThreadMaskId();

//描述：设置有效线程ID
void SetThreadMaskId(BYTE bID);

//描述：接收有效线程ID
void SetRecvThreadMaskId(BYTE bID);

//描述：线程是否可以工作
bool IsThreadExe(BYTE bID);

//描述：所有线程是否都已经接收到信号
bool IsAllThreadRecv();

BYTE BaudrateToGbNum(DWORD dwBaudRate);
int DoMtrFwdFunc(WORD wPort, WORD wTestId, BYTE bMtrPro, BYTE* pbMtrAddr, BYTE* pbRxFrm);
void InitMtrPortSch(BYTE bStartState);
void DoMtrPortSch();

#endif  //FAAPI_H


