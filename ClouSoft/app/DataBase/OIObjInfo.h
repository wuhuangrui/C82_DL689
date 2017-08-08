/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：OIObjInfo.h
 * 摘    要：本文件主要实现所有数据OI数据的格式定义
 * 当前版本：1.0
 * 作    者：孔成波
 * 完成日期：2016年9月
 *********************************************************************************************************/
#ifndef OIOBJINFO_H
#define OIOBJINFO_H
#include "apptypedef.h"

#define DT_NULL		0
#define DT_UNCARE	255

//数据访结果码
//		-1表示本OBIS对象本属性拒绝访问
//		-2表示本OBIS对象不存在
//		-3表示本BOIS对象与类标识不一致
//		-4表示其他原因访问不成功
#define DA_READ_WRITE_DENIED	1		//读写拒绝
#define DA_OBJECT_UNAVAILABLE	2		//对象不存在
#define DA_OBJ_CLASS_INCONSIST	3		//对象类不符
#define DA_OTHER_ERROR			4		//其他错误

//-------简单非定长数据(bit-string octet-string visible-string)的格式定义-----
//变长格式字节字节定义
#define RLF	0	//RL定长
#define RLV	1	//RL变长
#define LRF	2	//LR定长
#define LRV	3	//LR变长

#define BRVS	(0x10 | LRF)	//按位逆序

#define MAP_SYSDB	0
#define MAP_TASKDB	1
#define MAP_VAL		2
#define MAP_BYTE	3


#define OI_RD	1
#define OI_RW	2

#define MAX_MAINIP_NUM		2	//主站IP个数
#define MAX_SMS_MAIN_NUM	5	//短信主站号码
#define MAX_SMS_SEND_NUM	5	//短信通知号码
#define MAX_232_PORT_NUM	2	//232通道个数
#define MAX_485_PORT_NUM	3	//485通道个数
#define MAX_HW_PORT_NUM		1	//红外通道个数
#define MAX_SW_PORT_NUM		8	//开关量通道个数 (门节点在YX8)
#define MAX_DC_PORT_NUM		1	//直流模拟量通道个数
#define MAX_RLY_PORT_NUM	4	//继电器
#define MAX_ALRM_PORT_NUM	1	//告警输出
#define MAX_MUL_PORT_NUM	1	//多功能端子
#define MAX_PLC_PORT_NUM	1	//载波
#define MAX_PLUS_PORT_NUM	2	//脉冲
#define MAX_GPRS_COM_NUM	2	//GPRS通道个数
#define MAX_ETH_COM_NUM		8	//以太网通道个数
#define MAX_TIME_SCH_MTR_NUM 4	//定时搜表参数个数

#define OAD_OI_MASK				0xff001f00		//获取OI, 屏蔽掉属性特征及属性内元素索引
#define OAD_FEAT_MASK			0xffff1fff		//获取OAD, 只屏蔽掉属性特征
#define OAD_FEAT_BIT_OFFSET		13				//获取属性特征，需先dwOAD&~OAD_FEAT_MASK，再右移13位

#define BALANCE_DAY_NUM			3	//结算日个数

extern BYTE g_bOIFmt[];

typedef struct{
	DWORD dwOA;		//object attrib, 高WORD帮OI，低WORD放属性值
	WORD  wClass;	//接口类
	WORD  wMode;	//映射方式，可以映射到系统库或者任务库
	WORD  wID;		//对应系统BN0的ID
	WORD  wPn;		//测量点号
	WORD  wVal;		//属性值，一般用于存储不会改变的属性值
	BYTE* pFmt;		//格式描述串
	WORD  wFmtLen;	//格式描述串长度
	char* pszTableName;	//映射到任务库对应的任务文件名
}ToaMap;

typedef struct{
	DWORD dwOM;		//Object Attribute，高WORD放OI，低WORD放方法标识
	WORD wClass;	//接口类
	BYTE* pFmt;		//参数的格式描述串
	WORD wFmtLen;	//参数的格式描述串的长度
	int (*pfnDoMethod)(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int *piRetLen);
	void* pvAddon;	//附加参数，针对不同的类有不同
}TOmMap;//对象方法映射表

extern ToaMap g_OIConvertClass[];
extern const ToaMap* GetOIMap(DWORD dwOIAtt);
extern TOmMap g_OmMap[];
extern const TOmMap* GetOmMap(DWORD dwOIMethod);

extern BYTE g_bTskUnitFmtDesc[];
extern BYTE g_bCommFmtDesc[];
extern BYTE g_bEvtFmtDesc[];
extern BYTE g_bZJEvtFmtDesc[14];
extern BYTE g_bTranFmtDesc[];
extern BYTE g_bRptFmtDesc[];
extern BYTE g_bRealFmtDesc[];

int DoRightNowStartSchMtr(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClearSchMtrFile(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClearCrossSchMtrFile(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);


int DoClass8BroadcastTimeMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen);

int DoClass8ClockSourceMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen = NULL);
int DoClass8ClockSourceMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen = NULL);


int DoDevInterfaceClass19(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoGprsInterfaceClass25(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoEthInterfaceClass26(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method127_AddMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method128_AddMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method129_UpdataMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method130_UpdataMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method131_DelMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method132_DelMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method133_DelMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int DoClass11Method134_DelAllMtr(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：添加或更新一组任务配置单元
int AddCommonMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：添加透明采集参数
int AddTransMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：删除一组配置单元			
int DelCommonMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：清空配置单元			
int ClrCommonMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：添加或更新一组采集规则
int AddAcqRuleMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：删除一组采集规则
int DelAcqRuleMethod130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：清空采集规则库
int ClrAcqRuleMethod131(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//描述：更新任务状态
int UdpTaskState130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//0x6014 方法：130	重置方案的记录列选择
int ResetSchRecordCSD(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//0x6016 方法：130	更新上报方案标识
int UpdateRptFlgMethod130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);


//描述：获取任务配置单元
extern int GetTaskConfigFromTaskDb(BYTE bTaskId, BYTE *pbRespBuf);

//描述：获取采集方案参数采集方案
extern int GetSchFromTaskDb(BYTE bSchNo, BYTE bSchType, BYTE *pbRespBuf);

//////////////////////////////////////////////////////////////////////////
//总加组方法
int ClrGrpCfgMethod1(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int AddGrpCfgMethod3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int BatAddGrpCfgMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DelGrpCfgMethod5(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//////////////////////////////////////////////////////////////////////////
//遥控方法
int YkCtrlTriAlertMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int YkCtrlDisAlertMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int YkCtrlOpenMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int YkCtrlCloseMethod130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//////////////////////////////////////////////////////////////////////////
//保电方法
extern int InputGuaranteeMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
extern int QuitGuaranteeMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
extern int QuitAutoGuaranteeMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//////////////////////////////////////////////////////////////////////////
//催费告警方法
int InputUrgeFeeMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int QuitUrgeFeeMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//////////////////////////////////////////////////////////////////////////
//中文信息方法
int AddChineseInfoMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DelChineseInfoMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

//////////////////////////////////////////////////////////////////////////
//控制方法
int AddCtrlUnitMethod3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DelCtrlUnitMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int InputCtrlMethod6(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int QuitCtrlMethod7(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int InputCtrlMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

WORD GetOiClass(WORD wOI);

BYTE* GetSchFmt(BYTE bSchType, WORD* pwFmtLen);

char* GetSchTableName(BYTE bSchType);

int GetEvtTaskTableName(BYTE bSchNo, BYTE bCSDIndex, char* pszTableName);

//输入输出方法
int ComPortParaCfgMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int RelayParaCfgMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int MulPortCfgMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int CctTransmitMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);

int ResetStatAll(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen);



#endif
