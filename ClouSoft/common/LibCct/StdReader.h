#ifndef STDREADER_H
#define STDREADER_H

#include "CctAPI.h"
#include "LoopBuf.h"
#include "Comm.h"
#include "sysarch.h"
#include "sysapi.h"
#include "CctSchMtr.h"
#include "TransmitApi.h"

//载波相位信息
#define PLC_PHASE_UNK				0x00	
#define PLC_PHASE_A					0x01
#define PLC_PHASE_B					0x02
#define PLC_PHASE_C					0x03


//路由主动请求抄读标识
#define RT_RD_FAIL					0x00	//抄读失败
#define RT_RD_SUCC					0x01	//抄读成功
#define RT_RD_GOTO					0x02	//可以抄读

#define PRO_TYPE_TRANS				0x00
#define PRO_TYPE_645_97				0x01
#define PRO_TYPE_645_07				0x02

//服务器地址的地址类型
#define SER_ADDR_TYPE_SIG			0x00	//单地址
#define SER_ADDR_TYPE_COMM			0x01	//通配地址
#define SER_ADDR_TYPE_GRP			0x02	//组地址地址 
#define SER_ADDR_TYPE_BC			0x03	//广播地址

#define PRO_69845_LEN_OFFSET		0x01	//长度域L偏移
#define PRO_69845_DATA_LEN			0x02	//长度域L字节数
#define PRO_68945_CTRL_OFFSET		0x03	//控制域C
#define PRO_69845_HEAD_CS_OFFSET	0x04	//帧头校验
#define PRO_69845_HEAD_CS_LEN		0x02	//帧头校验长度
#define PRO_69845_FRAM_CS_LEN		0x02	//帧校验长度

#define PRO_3762_LEN_OFFSET			0x01	//长度偏移
#define PRO_3762_DATA_LEN			0x02	//长度

#define FN(x)						(x)


//AFN
#define AFN_CON		            	0x00	//确认∕否认
#define AFN_INIT			        0x01	//初始化
#define AFN_TRAN	    	    	0x02	//数据转发
#define AFN_QRYDATA			    	0x03	//查询数据
#define FRM_LNKTEST			    	0x04	//链路接口检测
#define AFN_CTRL			    	0x05	//控制命令
#define AFN_REP			    		0x06	//主动上报
#define AFN_QRYRT					0x10	//路由查询
#define AFN_SETRT		    		0x11	//路由设置
#define AFN_CTRLRT					0x12	//路由控制
#define AFN_RTFWD 		    		0x13	//路由数据转发
#define AFN_RTRD 		    		0x14	//路由数据抄读
#define AFN_TRSFILE   				0x15    //文件传输


//载波集中器的底层通信链路定义
#define AR_LNK_UNKNOW				0xff    //未知类型
#define AR_LNK_ES					0x01	//东软载波
#define AR_LNK_TC					0x02	//青岛鼎信
#define AR_LNK_RSC					0x03	//瑞斯康
#define AR_LNK_XL					0x04	//芯珑
#define AR_LNK_CL					0x05	//科陆无线
#define AR_LNK_LS					0x06	//芯珑
#define AR_LNK_SGD					0x06	//深国电
#define AR_LNK_GY					0x07	//光一


//周期抄表模式    
#define CCT_RD_BY_TERM				0x01	//集中器主导抄表
#define CCT_RD_BY_ROUTE				0x02	//路由主导抄表
#define CCT_RD_TERM_ROUTE			0x03	//两种抄表模式都支持

//从节点同步模式
#define CCT_NO_SYNC					0x00    //不需要下发从节点信息
#define CCT_NEED_SYNC				0x01    //需要下发从节点信息

//路由模块通信方式
#define CCT_RD_TYPE_PLC				0x01    //窄带电力线载波
#define CCT_RD_TYPE_HSPLC			0x02	//宽带电力线载波
#define CCT_RD_TYPE_RADIO			0x03	//微功率无线通信



typedef struct{
	BYTE  bModule;		//模块类型
	BYTE  bModType;     //通信方式
	BYTE  bRtType;      //路由管理方式
	BYTE  bNodeType;    //从节点信息模式
	BYTE  bRdType;      //周期抄表模式
	BYTE  bTansTmOut;   //传输延时参数支持
	BYTE  bRdFailType;  //失败节点切换发起方式
	BYTE  bBdCfmType;   //广播命令确认方式
	BYTE  bBdChnlType;  //广播命令信道执行方式
	BYTE  bChnlNum;     //信道数量
	BYTE  bSpeedNum;    //速率数量
	BYTE  bVolMis;      //低压电网掉电信息
	BYTE  bUpdateTmOut; //升级操作等待超时时间
	BYTE  bNodeTmOut;   //从节点监控最大超时时间
	BYTE  bTrySendCnt;	//扩展：协议没有定义，针对不同的路由模块发送不同的次数

	WORD  wBdTmOut;     //广播命令最大超时时间
	WORD  wFrmMaxLen;   //最大支持的报文长度
	WORD  wTansMaxLen;  //文件传输支持的最大单个数据包长度
	WORD  wMaxNodeNum;  //支持的最大从节点数量
	WORD  wNodeNum;     //当前从节点数量

	BYTE  bMainAdd[6];      //主节点地址
	BYTE  bProRelsDate[3];  //通信模块使用的协议发布日期
	BYTE  bProRecdDate[3];  //通信模块使用的协议最后备案日期
	BYTE  bFacCode[9];      //厂商代码及版本信息
	WORD  wSpeedArr[5];       // 通信速率
}RtRunModeOob; //本地通信模块运行模式信息

typedef enum {
	CCT_RUN_OTHER,	//运行其它功能
	CCT_SCH_MTR,	//搜表状态
	CCT_RD_MTR,		//抄表（自动、直抄）
// 	CCT_AUTO_RD,	//自动抄表		AFN=14-F1
// 	CCT_DIR_RD,		//直抄电表数据	AFN=13-F1
}TCCT_RUN_STATE;

typedef struct {
	BYTE bPhase;	//载波通信相位：0未知相，1～3指第1、2、3相
	BYTE bTsaLen;	//表地址有效长度
	BYTE bTsa[TSA_LEN];	//表地址
	BYTE bRevTsa[TSA_LEN];	//表地址逆序
	WORD wPn;
	WORD wNodeSn;	//从节点序号
}TRtReqInfo;

typedef struct {
	WORD wProType;//=PROTOCOL_TYPE_698_45;	
	BYTE bHead;	//起始字符 0x68
	WORD wDataLen;	//除起始字符和结束字符之外的帧字节数
	BYTE bCtrl;	//控制域
	BYTE bSAFlg;	//地址标识，bit0~3表示服务器地址字节数，bit4~5逻辑地址，bit6~7为服务器地址的
					//类别标识, 0表示单地址，1表示通配地址，2表示组地址，3表示广播地址
	BYTE bSALen;	//bSA的有效长度
	BYTE bSA[TSA_LEN];	//服务器地址，数组按最大空间
	BYTE bCA;	//客户机地址CA	取值范围0…255，值为0表示不关注客户机地址
	WORD wHCS;	//是对帧头部分除起始字符和HCS本身之外的所有字节的校验，校验算法见附录D
	WORD wAPDULen;	//APDU应用层协议数据单元长度
	BYTE bAPDUData[256];	//开辟256空间存储APDU单元数据
	WORD wFCS;	//是对整帧除起始字符、结束字符和FCS本身之外的所有字节的校验，校验算法见附录D
	BYTE bEnd;	//0x16
}TFrm69845;	//698.45帧格式

typedef struct {
	WORD bProType;//=PROTOCOL_TYPE_1376_2;
	BYTE bHead;	//0x16
	WORD wDataLen;	//数据长度 帧头到帧尾
	BYTE bCtrl;	//Bit7传输方向位，Bit6启动标识位，Bit5~0通信方式
	BYTE bR[6];	//信息域R
	BYTE bSrcAddr[6];	//源地址
	BYTE bRelayAddr[6*6];	//中继地址
	BYTE bDesAddr[6];	//目的地址
	BYTE bAfn;	//应用功能码
	BYTE bDt[2];	//数据单元标识
	WORD wDtLen;	//Dt数据单元有效长度
	BYTE bDtBuf[512];	//Dt数据单元开辟最大空间
	BYTE bCs;	//校验和
	BYTE bEnd;	//0x16
}TFrm13762;	//1376.2帧格式

typedef struct {
	bool fGetPlcInfo;	//获取模块信息
	bool fPlcInit;		//载波模块是否初始化
	bool fSetMainNode;	//是否设置主节点地址
	bool fSyncAddr;		//是否同步档案
	bool fIsNeedRtReq;	//是否需要路由主动请求，瑞斯康不需要，鼎信、东软需要
	BYTE bTermLen;	//终端地址长度
	BYTE bTermAddr[TSA_LEN];	//终端逻辑地址
	DWORD dwGetSyncAddrInfoClick;  //收到档案同步信号的时刻
}TRtStat;

typedef struct {
	int iStart;	//透明方案建表是根据“0x6019的方案内容”，即一个表地址对应一张表，
				//所有的表名都在一个文件中，该参数iStart用来控制在该文件中偏移位置
	int iMsgSn;		//透明传输方案报文序号
	bool fStart;
	bool fFinsh;	//任务透明任务方案是否执行结束
}TransSchSate;	//透明方案参数

typedef struct {
	bool fDirRdFlg;			//直抄请求，如：代理请求
	bool fRtPause;				//true路由已经暂停
	BYTE bRdFailTsa[TSA_LEN];	//上次抄读失败的表地址
	TTime tLastUdpTime;	
	DWORD dwLstDirClk;			//上次任务直抄的时间
	DWORD dwBcWaitClick;  // 广播命令后需要等待的时间

	TTaskCfg tCurExeTaskCfg;	//当前正在执行的任务配置单元
	TransSchSate tTransSchSate;
}TCctRunStateInfo;	//载波线程运行信息


class CStdReader : public CCctSchMeter{
private:
	CLoopBuf m_LoopBuf;

	CComm  m_Comm;

	TFrm13762 m_TRcv13762;

	TCctRunStateInfo m_TRunStateInfo;

	RtRunModeOob m_RtRunMdInfo;

	TRtStat m_TRtStat;

	BYTE m_bCctExeState;

	char *m_pszName;

	bool m_fRxComlpete;

	TSem m_semReader;

	int m_iPn;
	DWORD m_dwLastWaitSec;//上一次路由回复等待时间,下发广播帧时会用该变量
	bool m_fIsLastLocalModuleState; //  记录变动前上一次载波模块状态
public:
	
	CStdReader(void);
	
	virtual ~CStdReader(void){};	

	void Init();

	void InitRcv();

	void InitPhyPort();

	void ClearRcv();

	void  LockReader();

	void  UnLockReader();

	bool ModuleInfoCheck();
private:
	void LockDirRd();

	void UnLockDirRd();

private:
	bool InitRouter(BYTE bFn);


	bool GetInitState();

	bool Is3762Afn13Fn01(BYTE *pbData, int iLen);
	bool IsValid3762Frame(BYTE *pbData, int iLen, TFrm13762 &tmpInfo3762);
	bool Copy376ToBuf(BYTE *pbData, int iMaxLen, TFrm13762 *pRcv13762);

public:
	bool Afn01Fn01_HardwareInit();

	bool Afn01Fn02_ParmInit();

	bool Afn01Fn03_DataInit();

	bool Afn03Fn04_ReadMainNodeAddr(BYTE* pbBuf);

	void Afn03Fn10_RptRtRunInfo(BYTE* pbBuf);

	bool Afn05Fn01_SetMainNodeAddr();

	bool Afn05Fn3_StartBoardCast(BYTE *pbReqBuf, WORD wLen);

	void Afn06Fn01_RptNodeInfo();

	bool Afn06Fn02_RptData();

	void Afn06Fn03_RptRtInfo();

	void Afn06Fn04_RptMtrInfo();

	void Afn06Fn5_RptNodeEvt();

	WORD Afn10Fn01_RdRtNodeNum();

	int Afn10Fn02_RdNodeInfo(BYTE bRdMtrNum, WORD wStartSn, BYTE *pbOutBuf);

	int Afn10Fn04_QueryRtRunInfo(BYTE *pbRespBuf);

	bool Afn11Fn01_AddNode(BYTE *pbInBuf, BYTE bInLen);

	int Afn11Fn02_DelNode(BYTE *pbInBuf, BYTE bInLen); 

	bool Afn11Fn05_ActSlaveNodeRpt(BYTE *pbBuf, BYTE bLen);

	bool Afn11Fn06_StopSlaveNodeRpt();

	int DirAskProxy(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData);
private:

	int Set_OAD_to_645_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pInApdu, WORD wInApduLen, WORD wTimeOut, BYTE* pbData);
	int Act_OAD_to_645_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pInApdu, WORD wInApduLen, WORD wTimeOut, BYTE* pbData);
	int Do_uplink_request_to_698_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pInApdu, WORD wInApduLen, WORD wTimeOut, BYTE* pbData);
	int Read_OneOAD_from_645_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pInApdu, WORD wInApduLen, WORD wTimeOut, BYTE* pbData);
	int Read_RecordData_from_645_meter(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pInApdu, WORD wInApduLen, WORD wTimeOut, BYTE* pbData);


	bool RtCtrl(BYTE bFn);

public:
	bool Afn12Fn01_RtRestart();

	bool Afn12Fn02_RtPause();

	bool Afn12Fn03_RtResume();

	int Afn13Fn01_RtFwd(BYTE *pbTsa, BYTE bTsaLen, const BYTE *pbInBuf, WORD wInLen, TMtrRdCtrl *pMtrRdCtrl, TRdItem *pRdItem, BYTE *pbOutBuf, BYTE bProId, bool fAnaly645data=false, bool fIs645Proxy=false, BYTE *pbRcvFrm=NULL, WORD *pwRcvLen=0);

	int Afn14Fn1_RtReqRd();

	bool Afn14Fn2_RtReqClk();

	//---------------------------------------------------------------
	void RouterPause();
	void RouterResume();
	//---------------------------------------------------------------

	bool ReadPlcModuleInfo();

	int DirectReadMeterData(BYTE *pbTsa, BYTE bTsaLen, BYTE bProId, DWORD dwID, BYTE *pbInBuf, WORD wInLen, BYTE *pbOutBuf, BYTE *pbRcvFrm, WORD *pwRcvLen);


//////////////////////////////ProtoProc//////////////////////////////////////
	WORD Make645Frm(BYTE* pbFrm, const BYTE* pbAddr, BYTE bCmd, BYTE bDataLen);

	int Make698_45Frm(BYTE *pbMtr, BYTE bMtrLen, BYTE bCtrl, BYTE bAFType, BYTE bCA, WORD wFramFmt, BYTE *pbAPDU, WORD wAPDULen, BYTE *pbRespBuf);

	bool DecodeFram69845(BYTE *pbInBuf, WORD wInLen, TFrm69845 *ptTFrmFmt);

	int DecodeReportApdu(BYTE *pApdu, WORD wApduLen, TRdItem *pRptItem, BYTE *pbBuf, bool fIsRptFlg=false);

	int Make1376_2Frm(BYTE *pbTsa, BYTE bTsaLen, BYTE bCtrl, BYTE *pbR,  BYTE bAfn, BYTE bFn, const void * pbInbuf, WORD wInLen, BYTE *pbRespBuf);

	int Pro1376_2ToBuf(TFrm13762 *ptTFrmFmt, BYTE *pbOutBuf);

	int MakeTransmitAcqSchFram();

	//WORD GetRequestNormal(DWORD dwOAD, BYTE* pbTxBuf);

	//WORD GetRequestRecord(DWORD dwOAD, BYTE* pbTxBuf, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD);

	//int GetResponseNormal(DWORD dwOAD, BYTE* pbSrcBuf, WORD wSrcLen, BYTE* pbDstBuf);

	//int GetResponseRecord(DWORD dwOAD, BYTE* pbSrcBuf, WORD wSrcLen, BYTE* pbRCSD, BYTE bLenRCSD, BYTE* pbDstBuf);

	void GetRSDAndRCSD(DWORD *pdwOAD, BYTE* pbRSD, WORD* wRSDLen, BYTE* pbRCSD, WORD* wRCSDLen, BYTE bMethod, BYTE* pbData, BYTE* pbCSD);

/////////////////////////////////End ProtoProc///////////////////////////////////

	int DirectTransSchMsg(BYTE bSchNo, TransFilePara *pTransFilePara, TTransMsg *pTTransMsg);

	int SaveTransSchMsg(BYTE bSchNo, BYTE *pbMsgBuf, WORD wMsgLen);

	int DoFwdData(BYTE *pbTsa, BYTE bTsaLen, const BYTE *pbReqBuf, WORD wReqLen, WORD wTimeOut, BYTE *pbRespBuf, bool fGetApdu = false);
	int DoFwdData(BYTE * pbReqBuf, WORD wReqLen, WORD wTimeOut, BYTE * pbRespBuf, int iMaxResLen);
#if 0
	
#if 0
	int DirectReadMeterCommonData(TTaskCfg *pTTaskCfg, TCommAcqSchCfg *pTCommAcqSchCfg, WORD wMtrSn);
#endif

	int DirectReadMeterTransmitData(WORD wMtrSn, const BYTE *pbReqBuf, BYTE bReqLen, BYTE *pbRespBuf);
#endif

	void InitTaskIdSched();

	bool DoTaskIdSched();

	bool IsDirReadTi(TTimeInterv *pTi);

	bool QueryFrzTaskReadState();

	bool DoAllAcqSch();

	bool DoCommSch();

	bool DoTransSch();

	bool SyncMeterAddr();

	void DoOtherTask();

	bool DoReadRptErc();
	//------------------------------------------------------------
	bool StartBoardCast(int iMin);
	
	int StartSchMtr();

	bool StartNodeActive();

	bool WaitMtrReport();

	int FinishSchMtr();

	//------------------------------------------------------------

	void CctRunStateMonitor();

	DWORD Send(BYTE *pSendBuf, DWORD dwLen);

	DWORD Receive(BYTE *pRecvBuf, DWORD wLen);

	BYTE GetCtrl(BYTE bRcvCtrl);

	WORD DtToFn(BYTE* pbDt);

	void FnToDt(WORD wFn, BYTE* pbDt);

	int RcvFrame(BYTE *pbBlock, int nLen);

	bool DefHanleFrm();

	bool RxHandleFrm(DWORD dwSeconds, bool fIsDefHanleFrm=true);

	void DoAutoRead();

	void CctRunStateCheck();

	void RunThread();
	int DL645_9707MakeFrm(BYTE *pbMtr, BYTE bMtrLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *bpBuf);
	int DL645_EXTMakeFrm(BYTE *pbMtr, BYTE bMtrLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *bpBuf);
	int GetDL645_9707DataVal(BYTE *psData, BYTE bsLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *pbData, TRdItem *pRdItem=NULL, WORD wPn=0);
	int GetDL645_EXTDataVal(BYTE *psData, BYTE bsLen, BYTE bProId, BYTE bSubProId, DWORD dwOAD, BYTE *pbData);
	int OneAddrBroadcast(BYTE *pbTsa, BYTE *pbInBuf, WORD wInLen, TMtrPara tTMtrPara, TRdItem *pRdItem, BYTE *pbData, BYTE bProId);
	int Afn13Fn01_Broadcast(BYTE *pbTsa, BYTE bTsaLen, const BYTE *pbInBuf, WORD wInLen, TRdItem *pRdItem, BYTE *pbOutBuf, BYTE bProId, BYTE bWaitTm);
	int ReadDL645_9707Time(BYTE * pDbTsa, TMtrPara tTMtrPara, TRdItem *pRdItem, BYTE *pbData, BYTE bProId);
	int MtrBroadcast();
	int BroadcastAdjustTime();
	int Broadcast(BYTE *pbTsa, BYTE bTsaLen, BYTE *pbReqBuf, WORD wReqLen, WORD wTimeOut, BYTE *pbRespBuf, BYTE bPro = 0);

	//全事件接口
	int DL645V07AskItemErc(TRdItem* pRdItem, BYTE* pbAddr, BYTE bAddrLen, BYTE bProId, DWORD dwOAD, BYTE* pbRxBuf);
	int DL645V07ProIdTxRx(TRdItem* pRdItem, BYTE* pbAddr, BYTE bAddrLen, BYTE bProId, DWORD dwProId, BYTE* pbRxBuf, WORD wMaxRxLen);
	int DL645V07AskItemErcHapEndEng(TRdItem* pRdItem, BYTE* pbAddr, BYTE bAddrLen, BYTE bProId,  DWORD dwOAD, TErcRdCtrl* pErcRdCtrl, BYTE* pbDstBuf);
	bool SaveRepErcData(BYTE *pbBuf, WORD wLen);

	//不同协议类型表抄读接口
	int ReadDLT_645(TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, TMtrPara* pMtrPara, BYTE* pbRxBuf, WORD wMaxRxLen);
	int ReadDLT_69845(TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, TMtrPara* pMtrPara, BYTE* pbRxBuf, WORD wMaxRxLen);
	int ReadDLT_SBJC(TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, TMtrPara* pMtrPara, BYTE* pbRxBuf, WORD wMaxRxLen);
	
	int SetRouterRequestInfo(BYTE bState, BYTE *pBuf);

};


void NewCctThread();

int GetSchMtrResult(int *piStart, BYTE *pbBuf, WORD wMaxLen, DWORD dwStartSec, DWORD dwEndSec);

int GetCrossSchMtrResult(int *piStart, BYTE *pbBuf, WORD wMaxLen);

void SaveSearchPnToDb(BYTE* pbMtrAddr, BYTE bAddrLen, BYTE bPro, TTime tmNow, BYTE bPort);

bool SetSchMtrEvtMask(WORD wIndex, bool fState);

bool UpdataSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen);

bool GetSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen);

void ClearSchMtrEvtMask();

int GetSchMtrEvtData(int iIndex, BYTE *pbBuf);

extern CStdReader *g_CStdReader;
#endif
