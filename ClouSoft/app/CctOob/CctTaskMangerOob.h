#ifndef CCT_TASK_ID_H
#define CCT_TASK_ID_H

#include "TaskDB.h"
#include "ComStruct.h"
#include "DbStruct.h"
#include "MeterStruct.h"
#include "TaskConst.h"
#include "OIObjInfo.h"
#include "DbConst.h"

#define TSA_LEN			17

#define SCH_TYPE_NUM	6	//采集方案类型个数

#define SCH_TYPE_COMM	1	//普通采集方案
#define SCH_TYPE_EVENT	2	//事件采集方案
#define SCH_TYPE_TRANS	3	//透明采集方案
#define SCH_TYPE_REPORT	4	//上报事件方案
#define SCH_TYPE_SCRIPT	5	//脚本事件方案
#define SCH_TYPE_REAL	6	//实时监控方案

#define MS_ONE_GROUP_USER_TYPE		32	//一组用户类型
#define MS_ONE_GROUP_USER_TYPE_REGION		32	//一组用户类型

#define ACQRULE_TABLE_NAME_LEN 128

extern TSem	g_semMtrUdp; //电表档案更新
extern TSem	g_semTskCfg; //任务配置单元
extern TSem	g_semSchCfg;  //采集方案配置

typedef struct {
	BYTE *pbCfg;
	WORD wCfgLen;
	BYTE bSchNo;
	BYTE bSchType;	
}TMemMalloc;

extern TMemMalloc g_TaskMem[TASK_ID_NUM];
extern TMemMalloc g_SchMem[TASK_ID_NUM];


typedef struct {
	DWORD dwOAD;
	BYTE bData[16];
}TAddInfo;	//附属信息

typedef struct {
	WORD wPn;
	WORD wMtrSn;	//电表配置序号
	BYTE bTsaLen;	//表地址长度
	BYTE bTsa[16];	//表地址
	BYTE bBps;	//波特率
	BYTE bProType;	//规约类型
	DWORD dwPortOAD;	//端口
	BYTE bCodeLen;	//通信密码长度
	BYTE bCode[16];	//通信密码
	BYTE bRate;	//费率个数
	BYTE bUserType;	//用户类型
	BYTE bLine;	//接线方式
	WORD wRateVol;	//额定电压
	WORD wRateCurr;	//额定电流
	BYTE bAcqTsaLen;	//采集器地址长度
	BYTE bAcqTsa[16];	//采集器地址
	BYTE bAssetLen;	//资产号长度
	BYTE bAsset[16];	//资产号
	WORD wPT;	//
	WORD wCT;	//
	BYTE bAddInfoCnt;	//附属信息个数，暂定最大8个
	TAddInfo tTAddInfo[8];	//附属信息
}TOobMtrInfo;	//采集档案配置单元 

typedef struct {
	DWORD dwOAD;	//对象属性描述符
	BYTE bOADNum;	//关联对象属性描述符 个数，定义最大为16个
	DWORD dwOADArry[16];	//关联属性OAD
}TROAD;

typedef struct {
	BYTE bChoice;	//0：OAD	1：ROAD	
	DWORD dwOAD;	//对象属性描述符
	TROAD tTROAD;	//记录型对象属性描述符
}TCSD;


typedef struct {
	BYTE bRegionType;	//区间类型 0-前闭后开，1-前开后闭，2-前闭后闭，3-前开后开
	BYTE bUserStart;	//区间起始值
	BYTE bUserEnd;	//区间结束值
}TUserRegion;	//一组用户类型区间

typedef struct {
	BYTE bRegionType;	//区间类型 0-前闭后开，1-前开后闭，2-前闭后闭，3-前开后开
	BYTE bTsaStart[TSA_LEN];	//区间起始值
	BYTE bStartTsaLen;
	BYTE bTsaEnd[TSA_LEN];	//区间结束值
	BYTE bEndTsaLen;
}TTsaRegion;	//一组用户地址区间

typedef struct {
	BYTE bRegionType;	//区间类型 0-前闭后开，1-前开后闭，2-前闭后闭，3-前开后开
	WORD wCfgSnStart;	//区间起始值
	WORD wCfgSnEnd;	//区间结束值
}TCfgMtrSnRegion;	//一组配置序号区间


typedef struct {
	BYTE bBps;	//波特率
	BYTE bCheckBit;	//校验位
	BYTE bDataBit;	//数据位
	BYTE bStopBit;	//停止位
	BYTE bFlowCtrl;	//流控制
}TCOM_PARAM;	//通信参数

typedef struct {
	BYTE bVendorCode[2];	//厂商代码
	BYTE bChipCode[2];		//芯片代码
	BYTE bVerDate[5];		//版本日期 YYMMDDhhmm
	WORD wSfwVer;			//软件版本
}TSWF_VER_INFO;	//版本信息

typedef struct {
	char pszPortDesc[32];			//端口描述符
	TCOM_PARAM tTCOM_PARAM;			//通信参数
	TSWF_VER_INFO tTSWF_VER_INFO;	//版本信息	
}TCCT_MODE_INFO;					//本地模块信息

typedef struct {
	BYTE bPortDescLen;			//端口描述符的有效长度
	char pszPortDesc[32];		//端口描述符
	TCOM_PARAM tTCOM_PARAM;		//端口参数
	BYTE bPortFunc;				//端口功能 上行通信（0），抄表（1），级联（2），停用（3）
}TPORT_PARAM;	//端口参数

#define PERIOD_MAX 24

typedef struct {
	BYTE bStarHour;
	BYTE bStarMin;
	BYTE bEndHour;
	BYTE bEndMin;
}TPeriod; //时段

typedef struct {
	BYTE bTaskId; 	//任务ID
	TTimeInterv tiExe;	 //执行频率
	BYTE bSchType; 	//方案类型
	BYTE bSchNo; 	//方案编号
	TTime tmStart;	//开始时间
	TTime tmEnd;		//结束时间
	TTimeInterv tiDelay;	 //延时
	BYTE bPrio;		//执行优先级 首要（1），必要（2），需要（3），可能（4）
	BYTE bState;		//状态	1正常，2停用
	WORD wPreScript;	//开始前脚本
	WORD wPostScript;	//完成后脚本
	BYTE bPeriodType;	//时段类型
	BYTE bPeriodNum;	//时段表个数
	TPeriod period[PERIOD_MAX];	//时段
	BYTE bFwdTsa[TSA_LEN]; //透明方案中的通信地址
}TTaskCfg;  //任务配置单元

typedef struct {
	BYTE bAcqType;	//采集类型
	BYTE bAcqData[4];	//采集类容
}TAcqType;	//采集方式

typedef struct {
	BYTE bSchNo;	//方案编号
	WORD wStgCnt;	//存储深度
	TAcqType tTAcqType;	//采集方式
	//--------------------------------
	BYTE bCSDNum;	//记录列选择个数，tTCSD[x]的个数
	TCSD tTCSD[20];	//暂时定义20个，这里的数组索引还有另外一个功能，用来统计是否成功
	//--------------------------------

	//BYTE bCsdChoice;	//CSD内列表选择，0：OAD，1：ROAD
	BYTE bMsChoice;	//电能表集合MS
	BYTE bMtrMask[PN_MASK_SIZE];	//电能表集合bMS中的电表屏蔽字
	BYTE bStgTimeScale;	//存储时标
}TCommAcqSchCfg;	//普通采集方案

typedef struct {
	BYTE bSchNo;	//方案编号
	BYTE bROADNum;	//tTROAD中ROAD的有效个数
	TROAD tTROAD[32];	//暂定义位32个ROAD
	BYTE bMsChoice;	//电能表集合MS
	BYTE bMtrMask[PN_MASK_SIZE];	//电能表集合bMS中的电表屏蔽字
	bool fRptFlg;	//上报标识 bool [true:立即上报，false:不上报]
	WORD wStgCnt;	//存储深度
}TEvtAcqSchCfg;	//事件采集方案

typedef struct{
	//698.45规则
	BYTE *pCSD;	//0-OAD, 1-ROAD

	//645-07规则
	BYTE *pbDlt07;	//指向的是AcqCmd_2007

	//645-97规则
	BYTE *pbDlt97;	//指向的是AcqCmd_1997

	//AcqCmd_Trans
	BYTE *pbTrans;	//指向的是AcqCmd_Trans
}TAcqRuleInfo;

typedef struct {
	//698.45规则
	BYTE bChoice;	//0-OAD, 1-ROAD
	BYTE *pCSD;
}TSearchId;

typedef struct {
	BYTE bMtrPro;

	BYTE bMain645Num;	//主ID个数
	DWORD dwMain645Id[8];	//主ID

	BYTE bSlave645Num;	//从ID个数
	DWORD dwSlave645Id[8];	//从ID
}T645IdInfo;

//采集规则表，这里保存所有的表
typedef struct {
	BYTE bMsk[16];	//有效屏蔽字
	char szTableName[ACQRULE_TABLE_NAME_LEN];	//表长度
	/*
	char szTableName1[128];	//表长度
	char szTableName2[128];	//表长度
	......
	*/
}TAcqRuleTable;

#define MK_ACQRULE_TABLE_NAME(pszTableName)	(sprintf(pszTableName, "%sAcqRuleTable", USER_DATA_PATH))	//所有采集规表名都保存到该文件里
#define ACQRULE_FILE_HEAD_LEN		offsetof(TAcqRuleTable, szTableName)
#define ACQRULE_FILE_MSG_LEN		ACQRULE_TABLE_NAME_LEN
#define ACQRULE_FILE_MSG_OFFSET(index)	(ACQRULE_FILE_HEAD_LEN + index*ACQRULE_FILE_MSG_LEN)

void InitMtrMask();

const BYTE * GetMtrMask(BYTE bBank, WORD wPn, WORD wID);

void InitTaskMap();

const BYTE* GetTaskCfgTable(WORD wTaskId);

void InitSchMap();

BYTE* GetSchCfg(TTaskCfg* pTaskCfg, int *iLen);

BYTE* GetSchCfg(BYTE bIndex, int *iLen);

int GetSchParamFromMemory(BYTE bSchNo, BYTE *pbBuf);

int GetTaskNum(); 

bool GetTaskCfg(BYTE bIndex, TTaskCfg *pTaskCfg, bool bIsRdTab = false); 

//描述：取得普通采集方案参数
//参数: @pTaskCfg任务的配置
//	     @pTAcqSchCfg用来返回任务的配置
//返回: 为真获取成功
bool GetCommonSchCfg(TTaskCfg* pTaskCfg, TCommAcqSchCfg* pTCommAcqSchCfg, BYTE *pbArryCSD = NULL);

//描述：取得事件采集方案参数
//参数: @pTaskCfg任务的配置
//	     @pTAcqSchCfg用来返回任务的配置
//返回: 为真获取成功
bool GetEventSchCfg(TTaskCfg* pTaskCfg, TEvtAcqSchCfg* pTEvtAcqSchCfg);

bool GetTaskCyleUnit(TMtrRdCtrl* pMtrRdCtrl);

BYTE GetTaskCfgSn();

int ParserMsParam(BYTE *pbBuf, BYTE *pbMtrMask, WORD wMtrMaskLen);

bool GetRSDMS(BYTE *pbRSD, BYTE *pbMtrMask, WORD wMaskSize);

bool GetSchMS(BYTE *pbBuf, const TOmMap *p, BYTE bIndex, BYTE *pbMtrMask, WORD wMaskSize);

bool OoParseField(TFieldParser* pParser, BYTE* pFmt, WORD wFmtLen, bool fParseItem);

int FieldCmp(BYTE bCmpType, BYTE* pbCmpField, BYTE bSrcType, BYTE* pbSrcField);

int ReadFromROAD_1(BYTE* pbSelROAD, BYTE* pbSrcROAD, BYTE* pbSelData, BYTE* pbSrcData);
int ReadFromROAD(BYTE* pbSelROAD, BYTE* pbSrcROAD, BYTE* pbSelData, BYTE* pbSrcData);

int CreateTable(char* pszTableName, TFieldParser* pFixFields, TFieldParser* pDataFields, DWORD dwRecNumMax);

bool SaveRecord(char* pszTableName, BYTE* pbRec);

bool SaveRecordByPhyIdx(char* pszTableName, WORD wPhyIdx, BYTE* pbRec);

bool SaveHisRecord(char* pszTableName, int index, BYTE* pbRec);

void InitSchTable();

bool WriteCacheDataToTaskDB(BYTE bSchNo, BYTE bSchType, BYTE *pbRecBuf, WORD wRecLen, WORD wIdex = 0, int* piRecPhyIdx = NULL);

int ReadRecord(BYTE* pbOAD, BYTE* pbRSD, BYTE* pbRCSD, int *piTabIdx, int* piStart, BYTE* pbBuf, WORD wBufSize, WORD* pwRetNum);

int ReadTable(char* pszTableName, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbOAD, BYTE* pbRSD, BYTE* pbRCSD, int* piStart, WORD wSchNum, WORD* pwRetNum, BYTE* pbBuf);

int OoCopyOneOadData(BYTE *pbSrc, BYTE *pFmt, WORD wFmtLen, BYTE *pbDst, WORD *pwRetFmtLen, WORD *pwRetSrcOffset);

int OoFormatSrcData(BYTE *pbSrc, WORD wSrcLen, BYTE *pFmt, WORD wFmtLen, BYTE *pbDst);

int OoCopyData(BYTE* pbOAD, BYTE* pbDst, BYTE* pbSrc, WORD wItemOffset, WORD wLen, BYTE* pFmt, WORD wFmtLen);

int SearchTable(BYTE* pbOAD, BYTE* pbRSD, BYTE* pbRCSD, WORD wRcsdIdx, int *piTabIdx, char* pszTableName, TFieldParser* pFixFields, TFieldParser* pDataFields);

int TableMatch(BYTE* pbRCSD, WORD wRcsdIdx, BYTE* pbFixCfg, BYTE* pbFixFmt, WORD wFixFmtLen,BYTE* pbDataCfg, BYTE* pbDataFmt, WORD wDataFmtLen);

int RcsdMatch(BYTE* pbRCSD, WORD wRcsdIdx, BYTE* pbCfg, BYTE* pbFmt, WORD wFmtLen);

int ReadRecField(BYTE* pbRec, WORD wOffset, TFieldParser* pParser, BYTE* pbOAD, BYTE* pbCSD, BYTE* pbBuf);

DWORD GetEvtTaskTableFixFieldLen();

DWORD GetEvtTaskRecLastSerialNumber(BYTE bSchNo, BYTE bCSDIndex);

DWORD GetEvtTaskRecLastSerialNumber(BYTE* pbTsa, BYTE bTsaLen, BYTE* pbCSD, BYTE bLenCSD);

bool RecMatch (BYTE* pbRec, TFieldParser* pFixFields, BYTE* pbRSD);

bool SearchField (TFieldParser* pFixFields, BYTE* pbOAD, WORD* pwIndex, WORD* pwOffset, WORD* wFieldLen);

bool OadMatch(BYTE bOp, BYTE* pbOAD, BYTE* pbData, BYTE* pbFieldData, WORD wFieldLen);

bool DateTimeSMatch(BYTE bOp, BYTE* pbDateTimeS, BYTE* pbFieldData);

bool MsMatch(BYTE* pbMS, BYTE* pbFieldData);

bool TiMatch(BYTE* pbStartTime, BYTE* pbEndTime, BYTE* pbTI, BYTE* pbFieldData);

bool SaveRecord(char* pszTableName, BYTE* pbRec, int* piRecPhyIdx);

int MsToMtrNum(BYTE *pbMs);

int DayFrzTimeMatch(BYTE *pSrc, BYTE *pRcsd);

extern bool SearchAcqRule645ID(BYTE *pbCSD, BYTE bMtrPro, T645IdInfo *pT645IdInfo);
extern int GetAcqRuleTableName(BYTE *pbCSD, char *pszTabName, WORD wTabNameLen, TAcqRuleInfo *pTAcqRuleInfo);
extern int GetOneAcqRuleInfo(BYTE *pbPara, char *pszTabName, WORD wTabNameLen, TAcqRuleInfo *pTAcqRuleInfo);
extern int GetAcqRuleFromTaskDB(char *pszTabName, BYTE *pbOAD, BYTE *pbRespBuf);
extern bool SaveAcqRuleTableName(char *pszSaveTabName);
extern bool DeleteAcqRuleTableName(char *pszDelTabName);
extern bool GetAcqRuleTableName(int *piStart, char *pbRespTab, WORD wMaxTabNameLen);
extern int GetAllAcqRuleInfo(int *piStart, BYTE *pRespBuf, WORD wMaxLen);

#endif