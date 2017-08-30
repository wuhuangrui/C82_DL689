/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrExc.h
 * 摘    要：本文件主要实现面向对象协议的抄表事件类
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2016年10月
 *********************************************************************************************************/

#ifndef MTREXC_H
#define MTREXC_H
#include "DbStruct.h"
#include "MeterStruct.h"
#include "TermEvtTask.h"

//#define TSA_OCTSTR_TYPE				1		//TSA类型定义：TSA(0x55) + OCT_STR_LEN(N+2) + TSA_LEN(N) + ADDR[0] ~ ADDR[N]

#define TERM_EXC_NUM				33		//终端事件个数
#define MTR_EXC_NUM					7		//sizeof(g_wMtrExcOI)/sizeof(WORD) 抄表事件个数
#define ITEM_RD_TIME_NUM			3		//时标个数

#define MAX_JUDGE_OAD				5		//判断抄表事件需抄读OAD最大个数


#define MTR_EXC_REC_LEN				1024	//暂定？？

#define SAMPLE_MTR_NUM				4		//停电最大抄读的电能表个数
#define SAMPLE_CFG_ID_LEN			(SAMPLE_MTR_NUM*19 + 33)	//停电数据采集配置参数长度(13+19*SAMPLE_MTR_NUM+20) = 109

#define DIFF_COMP_CFG_NUM			10
#define DIFF_COMP_CFG_ID_LEN		(DIFF_COMP_CFG_NUM*28 + 2)	//有功总电能差动组配置参数长度

#define RESPONSE_TYPE_NORAML		1
#define RESPONSE_TYPE_RECORD		2


//当前事件状态--（瞬时状态）
#define ERC_STATE_MIDDLE			0	//中间态
#define ERC_STATE_HAPPEN			1	//发生
#define ERC_STATE_RECOVER			2	//恢复


#define OI_MTR_CLOCK_ERR		0x3105	//电能表时钟超差
#define OI_MTR_ENERGY_DEC		0x310B	//电能表示度下降
#define OI_MTR_ENERGY_ERR		0x310C	//电能量超差
#define OI_MTR_FLEW				0x310D	//电能表飞走
#define OI_MTR_STOP				0x310E	//电能表停走
#define OI_MTR_RD_FAIL			0x310F	//抄表失败事件
#define OI_MTR_DATA_CHG			0x311C	//电能表数据变更监控记录

#define OI_UN					0x4104	//额定电压

#define OAD_EVT_RPT_STATE		0x33000200	//标准事件上报状态OAD

#define	MTREXC_CLR_ID			0x0b15
#define	MTREXC_CLR_VALID		0x5A

//在dwItemRdTime[ITEM_RD_TIME_NUM]中的序号
#define EP_POS_ITEM_INDEX		0	//正有
#define EP_NEG_ITEM_INDEX		1	//反有
#define CLOCK_ITEM_INDEX		2	//时钟

#define TERM_EVENT_DEBUG

typedef struct
{
	WORD wOI;					//OI
	TFieldParser* pFixField;	//字段解析器
	BYTE* pbFmt;				//固定字段格式串
	WORD wFmtLen;				//固定字段格式串长度
}TMtrExcFixUnitDes;	//事件记录单元固定字段描述

int DoMtrExcMethod1(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoMtrExcMethod2(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoMtrExcMethod3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoMtrExcMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int DoMtrExcMethod5(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
//bool AllocateMtrExcMem(BYTE* pbGlobal, TAllocTab* pAllocTab, WORD wTabNum);

void InitMtrExc();
void ClrMtrExc();
void ReInitEvtPara(DWORD dwOAD);
void MtrExcOnRxFaResetCmd();
int GetMtrExcIndex(WORD wOI);

void SetMtrExcTableFlag(WORD wOI);
void ClrMtrExcTableFlag(WORD wOI);
bool IsMtrExcTableCreate(WORD wOI);
int GetMtrExcEvtRecord(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbRecBuf, WORD wBufSize);
bool GetMtrExcFieldParser(WORD wOI, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAtrrTabBuf, WORD wBufSize);
bool UpdateMtrExcRptState(DWORD dwCnOAD, TEvtMsg* pEvtMsg, BYTE bRptState);
int OoProRptMtrExcRecord(WORD wOI, BYTE bAttr, BYTE* pbRecBuf, WORD wRecLen, WORD wBufSize);

void SetMtrExcOadDefCfg(WORD wOI);

//========================电能表数据变更监控记录==========================
//描述：电能表数据变更监控记录，获取任务OAD
int GetMtrDataChgCSD(BYTE* bCfg);
extern bool RefreshMtrExcMem();

#endif //MTREXC_H
