/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrExc.cpp
 * 摘    要：本文件主要实现面向对象协议的抄表事件
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2016年10月
 *********************************************************************************************************/
#include "stdafx.h"
#include "syscfg.h"
#include "sysfs.h"
#include "FaCfg.h"
#include <fcntl.h>
#include "ComStruct.h"
#include "ComAPI.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "DbOIAPI.h"
#include "CctTaskMangerOob.h"
#include "MtrExc.h"
#include "TermEvtTask.h"
#include "OoFmt.h"
#include "MeterAPI.h"
#include "MtrProAPI.h"
#include "MeterStruct.h"
#include "CctAPI.h"
#include "Mem.h"
#include "MtrHook.h"
#include "MtrCtrl.h"

extern bool UpdateMtrExcStatData(WORD wOI, BYTE bState, TMtrExcTmp* pExcTmp, BYTE* pbTsa);
extern bool DoMtrClockErr(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);
extern bool DoMtrEnergyDec(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);
extern bool DoMtrEnergyErr(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);
extern bool DoMtrFlew(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);
extern bool DoMtrStop(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);
extern bool DoMtrRdFail(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);
extern bool DoMtrDataChg(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);


#define TRY_READ_NUM		3	//尝试重抄次数
//#define MTR_EXC_NUM		(sizeof(g_wMtrExcOI)/sizeof(WORD))

const WORD g_wMtrExcOI[MTR_EXC_NUM] = { OI_MTR_CLOCK_ERR, OI_MTR_ENERGY_DEC, OI_MTR_ENERGY_ERR, OI_MTR_FLEW, OI_MTR_STOP, OI_MTR_RD_FAIL, OI_MTR_DATA_CHG};


//事件判断用到的OAD列表
static DWORD g_dwJudgeOADList[][MAX_JUDGE_OAD] = 
{
	{ OI_MTR_CLOCK_ERR,		0x40000200,		0 },
	{ OI_MTR_ENERGY_DEC,	0x00100201,		0x00200201,		0 },
	{ OI_MTR_ENERGY_ERR,	0x00100200,		0x00200200,		0 },
	{ OI_MTR_FLEW,			0x00100200,		0x00200200,		0 },
	{ OI_MTR_STOP,			0x00100201,		0x00200201,		0x20040201,		0 },
	{ OI_MTR_RD_FAIL,		0 },	//抄表失败事件判断不依赖抄表ID
	{ OI_MTR_DATA_CHG,		0 },	//电能表数据变更监控记录抄表ID依赖于配置的任务，需要单独处理，此处不进行配置
};

//标准事件记录单元
BYTE g_bMtrExcFixOAList[] = {
	DT_ARRAY,
	5,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	//记录序号
	DT_OAD, 0x20, 0x1E, 0x02, 0x00, //发生时间
	DT_OAD, 0x20, 0x20, 0x02, 0x00, //结束时间
	DT_OAD, 0x20, 0x24, 0x02, 0x00, //发生源
	DT_OAD, 0x33, 0x00, 0x02, 0x00, //通道上报状态
};	//固定字段内容描述


//电能表时钟超差记录单元
BYTE g_bMtrClockExcFixOAList[] = {
	DT_ARRAY,
	7,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	//记录序号
	DT_OAD, 0x20, 0x1E, 0x02, 0x00, //发生时间
	DT_OAD, 0x20, 0x20, 0x02, 0x00, //结束时间
	DT_OAD, 0x20, 0x24, 0x02, 0x00, //发生源
	DT_OAD, 0x33, 0x00, 0x02, 0x00, //通道上报状态
	DT_OAD, 0x40, 0x00, 0x02, 0x00, //电能表时钟
	DT_OAD, 0x40, 0x00, 0x02, 0x00, //终端时钟
};	//固定字段内容描述

//电能表数据变更监控记录单元
BYTE g_bMtrDataChgExcFixOAList[] = {
	DT_ARRAY,
	8, //5,//  7,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,//事件记录序号  double-long-unsigned，
	DT_OAD, 0x20, 0x1E, 0x02, 0x00, //事件发生时间  date_time_s，
	DT_OAD, 0x20, 0x20, 0x02, 0x00, //事件结束时间  date_time_s，
	DT_OAD, 0x20, 0x24, 0x02, 0x00, //事件发生源    TSA，
	DT_OAD, 0x33, 0x00, 0x02, 0x00, //事件上报状态  array 通道上报状态，
	DT_OAD, 0x33, 0x0F, 0x02, 0x06, //监控数据对象  CSD，(5B)
	DT_OAD, 0x33, 0x0F, 0x02, 0x07, //变化前数据    Data
	DT_OAD, 0x33, 0x0F, 0x02, 0x08, // 变化后数据    Data
};	//固定字段内容描述

BYTE g_bStdMtrExcFixFmt[] = {
	DT_ARRAY,
	5,
	DT_OAD,
};	//固定字段格式描述串

BYTE g_bMtrClockExcFixFmt[] = {
	DT_ARRAY,
	7,
	DT_OAD,
};	//固定字段格式描述串

BYTE g_bMtrDataChgMtrExcFixFmt[] = {
	DT_ARRAY,
	8,
	DT_OAD,
};	//固定字段格式描述串

TFieldParser g_tStdFixFields = { g_bMtrExcFixOAList, sizeof(g_bMtrExcFixOAList) };	//固定字段
TFieldParser g_tMtrClockFixFields = { g_bMtrClockExcFixOAList, sizeof(g_bMtrClockExcFixOAList) };	//固定字段
TFieldParser g_tMtrDataChgFixFields = { g_bMtrDataChgExcFixOAList, sizeof(g_bMtrDataChgExcFixOAList) };	//固定字段


//抄表事件记录单元固定字段描述
TMtrExcFixUnitDes g_MtrExcFixUnitDesc[] = {
	{ OI_MTR_CLOCK_ERR,		&g_tMtrClockFixFields,	g_bMtrClockExcFixFmt,	sizeof(g_bMtrClockExcFixFmt) },	//时钟超差事件单元
	{ OI_MTR_ENERGY_DEC,	&g_tStdFixFields,		g_bStdMtrExcFixFmt,		sizeof(g_bStdMtrExcFixFmt) },	//标准事件单元
	{ OI_MTR_ENERGY_ERR,	&g_tStdFixFields,		g_bStdMtrExcFixFmt,		sizeof(g_bStdMtrExcFixFmt) },	//标准事件单元
	{ OI_MTR_FLEW,			&g_tStdFixFields,		g_bStdMtrExcFixFmt,		sizeof(g_bStdMtrExcFixFmt) },	//标准事件单元
	{ OI_MTR_STOP,			&g_tStdFixFields,		g_bStdMtrExcFixFmt,		sizeof(g_bStdMtrExcFixFmt) },	//标准事件单元
	{ OI_MTR_RD_FAIL,		&g_tStdFixFields,		g_bStdMtrExcFixFmt,		sizeof(g_bStdMtrExcFixFmt) },	//标准事件单元
	{ OI_MTR_DATA_CHG,		&g_tMtrDataChgFixFields,		g_bMtrDataChgMtrExcFixFmt,		sizeof(g_bMtrDataChgMtrExcFixFmt) },	//电能表数据变更监控记录单元
};

//描述:取组合ID到子ID的映射数组
DWORD* GetJudgeOADList(WORD wOI)
{	
	//集抄BANK和BANK0用同样的组合ID规则
	WORD wNum = sizeof(g_dwJudgeOADList) / (sizeof(DWORD)*MAX_JUDGE_OAD);
	for (WORD i=0; i<wNum; i++)
	{
		if (wOI == g_dwJudgeOADList[i][0])
		{
			if (wOI == OI_MTR_DATA_CHG)		//电能表数据变更监控记录按CSD读数据，而不是OAD，不使用此函数。要返回NULL.不让MtrExc.cpp抄表数据，由任务抄
				return NULL;
			
			return &g_dwJudgeOADList[i][1];
		}
	}
	
	return NULL;
}


//返回值：事件序号，>=0表示成功， <0失败
int GetMtrExcIndex(WORD wOI)
{	
	for (WORD i=0; i<MTR_EXC_NUM; i++)
	{
		if (wOI == g_wMtrExcOI[i])
			return i;
	}

	return -1;
}

//获取事件OI
WORD GetMtrExcOI(BYTE bExcIndex)
{
	if (bExcIndex >= MTR_EXC_NUM)
		return 0;

	return g_wMtrExcOI[bExcIndex];
}

//查询是否收到触发命令，如收到命令且时间到达，则先清除命令再返回true,否则false
//返回：是否触发冻结
bool IsTrigerSaveRec(WORD wPn, WORD wOI, BYTE bLastState)
{
	int iOffset, iLen;
	BYTE bTrigerState = 0;
	WORD wID, wCmdID, wDelayHapSec, wDelayRecvSec;
	DWORD dwCurSec = 0, dwRxCmdSec = 0;
	const WORD wBaseID = 0x0b20;
	BYTE bBuf[30];

	iOffset = GetMtrExcIndex(wOI);
	if (iOffset < 0)
		return false;

	wID = wBaseID + iOffset;
	memset(bBuf, 0, sizeof(bBuf));
	iLen = ReadItemEx(BN11, wPn, wID, bBuf);
	if (iLen > 0)
	{
		if (IsAllAByte(bBuf, 0, iLen) == false)		//收到触发冻结命令
		{
			bTrigerState = bBuf[8];
			if (bTrigerState == 0)
			{
				bBuf[8] = bTrigerState = 1;	//更新状态机，收到命令 
				WriteItemEx(BN11, wPn, wID, bBuf);
			}

			wDelayHapSec = ByteToWord(bBuf, 2);		//延迟发生时间
			wDelayRecvSec = ByteToWord(bBuf+2, 2);	//延迟恢复时间
			dwRxCmdSec = ByteToDWORD(bBuf+4, 4);	//接收命令时标			

			dwCurSec = GetCurTime();
			switch (bTrigerState)	//状态机  空闲0 -> 收到命令1 -> 触发发生2 -> 触发恢复3 -> 空闲0(清除命令)
			{
			case 1:
				if (bLastState==EVT_S_AFT_HP || bLastState==EVT_S_BF_END)	//已经发生
				{
					return true;	//强制恢复事件
				}
				else
				{
					if (dwCurSec-dwRxCmdSec>wDelayHapSec && dwCurSec>dwRxCmdSec)
					{
						bBuf[8] = bTrigerState = 2;		//更新状态机， 触发发生事件
						WriteItemEx(BN11, wPn, wCmdID, bBuf);
						return true;
					}
				}
				break;
			case 2:
				if (dwCurSec-dwRxCmdSec>(wDelayRecvSec+wDelayHapSec) && dwCurSec>dwRxCmdSec)
				{
					bBuf[8] = bTrigerState = 3;		//更新状态机， 触发恢复事件
					WriteItemEx(BN11, wPn, wCmdID, bBuf);
					return true;
				}
				break;
				
			case 3:
				memset(bBuf, 0, sizeof(bBuf));	//更新状态机， 清除命令
				WriteItemEx(BN11, wPn, wCmdID, bBuf);
				TrigerSaveBank(BN11, 0, -1);
				break;
			default:
				memset(bBuf, 0, sizeof(bBuf));	//清除命令
				WriteItemEx(BN11, wPn, wCmdID, bBuf);
				break;
			}
		}
	}

	return false;
}


//是否收到触发冻结命令
//参数:@wOI:
//	   @bLastState:上次状态机
//	   @pbState:返回当前状态机给保存记录使用
bool IsRxTrigerSaveCmd(WORD wPn, WORD wOI, BYTE bLastState, BYTE* pbState)
{
	bool fTrigerSave;

	fTrigerSave = IsTrigerSaveRec(wPn, wOI, bLastState);
	if (fTrigerSave)
	{
		if (bLastState==EVT_S_AFT_HP || bLastState==EVT_S_BF_END)
			*pbState = EVT_S_AFT_END;	//已发生，触发恢复事件
		else
			*pbState = EVT_S_AFT_HP;	//已恢复，触发发生事件
	}

	return fTrigerSave;
}

//根据格式获取有功电能值
//参数：@bFmt:DT_LONG64_U高精度电能值， 其他:低精度电能值

uint64 GetEnergyValByFmt(BYTE bFmt, BYTE* pbBuf, BYTE* pbLen)
{
	uint64 ui64Val = 0;

	if (pbBuf[0] == DT_LONG64_U)	//高精度
	{
		*pbLen = 9;
		ui64Val = OoLong64UnsignedTouUint64(pbBuf+1);
	}
	else	//低精度
	{
		*pbLen = 5;
		ui64Val = OoDoubleLongUnsignedToDWord(pbBuf+1);
	}

	return ui64Val;
}

TMtrExcFixUnitDes* GetMtrExcFixUnit(WORD wOI)
{
	WORD i;
	TFieldParser tFixParser;
	for (i=0; i<sizeof(g_MtrExcFixUnitDesc) / (sizeof(TMtrExcFixUnitDes)); i++)
	{
		if (wOI == g_MtrExcFixUnitDesc[i].wOI)
		{
			memset((BYTE* )&tFixParser, 0, sizeof(TFieldParser));	//必须给字段解析器初始化

			tFixParser.pbCfg = g_MtrExcFixUnitDesc[i].pFixField->pbCfg;
			tFixParser.wCfgLen = g_MtrExcFixUnitDesc[i].pFixField->wCfgLen;

			memcpy((BYTE* )g_MtrExcFixUnitDesc[i].pFixField, (BYTE* )&tFixParser, sizeof(TFieldParser));	//结构体变量赋值
			return &g_MtrExcFixUnitDesc[i];
		}
	}

	return NULL;
}

//单个电表异常的初始化
bool InitSubMtrExc(BYTE bIndex)
{
	int iRet;
	DWORD dwOAD;
	WORD wOI, wFmtLen = 0, wMaxNum = 0;
	TFieldParser tDataFields;

	BYTE bBuf[EVT_ATTRTAB_LEN] = {0};	
	BYTE* pbFmt = NULL;
	const ToaMap* pOaMap = NULL;

	memset((BYTE*)&tDataFields, 0, sizeof(TFieldParser));

	wOI = GetMtrExcOI(bIndex);
	if (wOI == 0)
		return false;	

	SetMtrExcOadDefCfg(wOI);

	const TMtrExcFixUnitDes* pUnitDesc = GetMtrExcFixUnit(wOI);
	if (pUnitDesc == NULL)
		return false;

	if (OoParseField(pUnitDesc->pFixField, pUnitDesc->pbFmt, pUnitDesc->wFmtLen, true) == false)		// 固定字段
		return false;	

	memset(bBuf, 0, sizeof(bBuf));
	tDataFields.pbCfg = bBuf;
	tDataFields.wCfgLen = OoReadAttr(wOI, ATTR3, tDataFields.pbCfg, &pbFmt, &wFmtLen);	//关联属性表
	if (tDataFields.wCfgLen <= 0)
		return false;

	if (OoParseField(&tDataFields, pbFmt, wFmtLen, true) == false)
		return false;

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(wOI, ATTR5, bBuf, NULL, NULL) <= 0)	//属性5 最大记录数
	{
		DTRACE(DB_METER_EXC, ("InitEvt: wOI=%u Init fail because Read wMaxNum fail.\r\n", wOI));
		return false;
	}

	wMaxNum = OoLongUnsignedToWord(bBuf+1);
	if (wMaxNum == 0)	// 最大记录数为0，不建表
		return false;

	dwOAD = GetOAD(wOI, ATTR2, 0);
	pOaMap = GetOIMap(dwOAD);
	if (pOaMap == NULL)
		return false;

	iRet = CreateTable(pOaMap->pszTableName, pUnitDesc->pFixField, &tDataFields, wMaxNum);
	if (iRet <= 0)
		return false;

	SetMtrExcTableFlag(wOI);		//置位建表完成标志
	DTRACE(DB_METER_EXC, ("InitMtrExc: bIndex=%d, Init OK.\r\n", bIndex));
	return true;
}


//外部接口函数，主线程上电初始化调用
//初始化建表
void InitMtrExc()
{
	BYTE bIndex;
	for (bIndex=0; bIndex<MTR_EXC_NUM; bIndex++)
	{
		InitSubMtrExc(bIndex);
	}
}



//描述：获取电表异常的抄读数据项
//参数：@wOI事件的OI
//		@pwJudgeOAD用来返回判断需要抄读的OAD数组
//		@pwJudgeOadNum用来返回判断需要抄读的OAD数组的个数
//		@pbRelaOAD	用来返回关联对象属性表
//返回:如果事件需要判断且配置正确则返回true,否则返回false
bool GetMtrExcRdItem(WORD wOI, DWORD* pdwJudgeOAD, WORD* pwJudgeOadNum, BYTE* pbRelaOAD)
{	
	int iLen;
	BYTE bBuf[40];
	DWORD* pdwSubOAD;

	iLen = OoReadAttr(wOI, ATTR9, bBuf, NULL, NULL);		//读有效标识
	if (iLen <= 0 || bBuf[0] != DT_BOOL)
		return false;

	if (bBuf[1] == 0)	//无效
		return false;

	pdwSubOAD = GetJudgeOADList(wOI);
	if (pdwSubOAD == NULL)
		return false;

	*pwJudgeOadNum = 0;
	while(*pdwSubOAD != 0)
	{
		*pdwJudgeOAD++ = *pdwSubOAD++;
		(*pwJudgeOadNum)++;
		if (*pwJudgeOadNum >= MAX_JUDGE_OAD-1)
			break;
	}

	iLen = OoReadAttr(wOI, ATTR3, pbRelaOAD, NULL, NULL);		//读关联属性表
	if (iLen <= 0)
		return false;
	else
		return true;
}


//**************************事件记录读取接口*********************************************
int GetMtrExcEvtSpecField(DWORD dwFieldOAD, BYTE* pbField, WORD wFieldLen, BYTE* pbStart)
{
	BYTE bArrayNum;
	int iRet = -1;

	*pbStart = 0;	//拷贝数据的起始位置

	switch(dwFieldOAD)
	{
		case 0x20200200:		//事件结束时间
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_DATE_TIME_S)
				iRet = 8;		//时间长度，已加类型
			break;	
		case 0x20240200:		//事件发生源
			iRet = pbField[1] + 2;	
			break;
		/*case 0x330C0206:		//事件清零列表
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_ARRAY)
			{
				bArrayNum = pbField[1];
				iRet = 2+5*bArrayNum;
			}			
			break;	*/
		case 0x33000200:		//事件上报状态
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_ARRAY)
			{
				bArrayNum = pbField[1];
				iRet = 2+CN_RPT_STATE_LEN*bArrayNum;
			}
			break;	
		case 0x330F0206:	
		case 0x330F0207:		
		case 0x330F0208:		
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else
			{
				iRet = pbField[0];
				*pbStart = 1;
			}			
			break;			
		default:
			if (pbField[0] == DT_NULL)	//无效数据返回NULL
				iRet = 1;
			else
				return wFieldLen;
	}

	if ((iRet>0) && (iRet<=wFieldLen))
		return iRet;
	else
		return 1;
}

//描述：获取事件固定字段/数据字段
//参数： @wOI 对象标识
//		@pFixFields 返回的固定字段
//		@pDataFields 返回的数据字段
//		@pbAtrrTabBuf 关联属性表缓冲区
//		@wBufSize pbDataCfg缓冲区的大小
//返回：正确获取到固定字段/数据字段返回true，否则返回false
bool GetMtrExcFieldParser(WORD wOI, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAtrrTabBuf, WORD wBufSize)
{
	int iLen;
	WORD wFmtLen = 0;
	TFieldParser tDataFields = { 0 };
	DWORD dwROAD;

	BYTE* pbFmt = NULL;
	TFieldParser* pTmpDataFields = &tDataFields;

	// 固定字段
	if (pFixFields != NULL)
	{
		const TMtrExcFixUnitDes* pUnitDesc = GetMtrExcFixUnit(wOI);
		if (pUnitDesc == NULL)
			return false;
		
		if (OoParseField(pUnitDesc->pFixField, pUnitDesc->pbFmt, pUnitDesc->wFmtLen, true) == false) 	//固定字段
			return false;

		memcpy((BYTE* )pFixFields, (BYTE* )pUnitDesc->pFixField, sizeof(TFieldParser));	//结构体变量赋值
	}

	//数据字段，关联属性表可以为NULL
	if (pDataFields != NULL)
	{
		/*if (wBufSize < EVT_ATTRTAB_LEN)
			return false;*/

		memset(pbAtrrTabBuf, 0, wBufSize);
		pTmpDataFields->pbCfg = pbAtrrTabBuf;
		iLen= OoReadAttr(wOI, ATTR3, pTmpDataFields->pbCfg, &pbFmt, &wFmtLen);
		if (iLen > 0)
		{
			pTmpDataFields->wCfgLen = iLen;
			if (OoParseField(pTmpDataFields, pbFmt, wFmtLen, true) == false)
			{	
				//DTRACE(DB_INMTR, ("GetEvtFieldParser: wOI=%u OoParseField() pDataFields fail.\r\n", wOI));
				return false;
			}
		}
		else
		{
			memset(pbAtrrTabBuf, 0, wBufSize);
			pDataFields->pbCfg = NULL;
			pDataFields->wNum= 0;
		}

		memcpy((BYTE* )pDataFields, (BYTE* )pTmpDataFields, sizeof(TFieldParser));	//结构体变量赋值
	}

	dwROAD = GetOAD(wOI, ATTR2, 0);	//事件对应OAD
	DelEvtOad(dwROAD, 0);
	return true;
}

int OoProReadMtrExcEvtRecord(WORD wOI, BYTE bAttr, BYTE* pbRecBuf, WORD wRecLen, WORD wBufSize)
{
	BYTE bType, bOadBuf[10], bAttrBuf[EVT_ATTRTAB_LEN] = {0};
	BYTE bTmpRecBuf[MTR_EXC_REC_LEN];	//一条记录缓冲区
	BYTE* pbTmpRec = bTmpRecBuf;
	BYTE* pbRec = pbRecBuf;
	WORD wItemOffset, wItemLen, wTotalLen;
	DWORD dwOAD;
	int iLen;
	TFieldParser tFixFields;	//固定字段
	TFieldParser tDataFields;
	const ToaMap* pOaMap = NULL;
	BYTE bIndex;
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	WORD wFmtLen = 0;
	BYTE bStart = 0;

	memset((BYTE* )&tFixFields, 0, sizeof(TFieldParser));
	memset((BYTE* )&tDataFields, 0, sizeof(TFieldParser));

	if (GetMtrExcFieldParser(wOI, &tFixFields, &tDataFields, bAttrBuf, sizeof(bAttrBuf)) == false)
		return -1;

	OoReadAttr(wOI, ATTR3, bRelaOAD, NULL, NULL);		//读关联属性表

	//跳过数据类型和元素个数
	wTotalLen = 0;
	*pbTmpRec++ = DT_STRUCT;
	wTotalLen++;
	*pbTmpRec++ = tFixFields.wNum + bRelaOAD[1];
	wTotalLen++;

	//处理固定字段上报信息,事件发生源等特殊数据
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)
			return -1;	//直接返回，固定字段不应答有问题

		if (bType!=DT_OAD || wItemLen==0)
			return -1;

		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);

		//调整数据
		iLen = GetMtrExcEvtSpecField(dwOAD, pbRec, wItemLen, &bStart);
		if (iLen>0 && iLen<=wItemLen)
			memcpy(pbTmpRec, pbRec+bStart, iLen);
		else
			return -1;

		pbRec += wItemLen;	 
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}

	memcpy(pbTmpRec, pbRec, tDataFields.wTotalLen);
	wTotalLen += tDataFields.wTotalLen;
	if (wTotalLen <= wBufSize)
	{
		memcpy(pbRecBuf, bTmpRecBuf, wTotalLen);
		return wTotalLen;
	}
	else
	{
		return -1;
	}
}

//描述：读出一条事件记录，提供给上行接口
//参数：@wOI 	对象标识
//		@bAtrr	属性标识及其特征 bit-string（SIZE（8））
//		@bIndex属性内元素索引
//		@pbRecBuf记录接收缓冲区
//		@wBufSize记录接收缓冲区的大小
//返回：正确返回记录的长度，否则返回负数
int GetMtrExcEvtRecord(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbRecBuf, WORD wBufSize)
{
	DWORD dwROAD;
	int iLen;
	char* pszFileName;

	dwROAD= GetOAD(wOI, bAttr, bIndex);
	DelEvtOad(dwROAD, 0);

	pszFileName = GetEvtRecFileName(dwROAD&0xffff1f00);
	if (pszFileName == NULL)
		return -1;

	// 读取记录
	iLen = ReadLastNRec(pszFileName, bIndex, pbRecBuf, wBufSize);
	if (iLen <= 0)
		return iLen;

	return OoProReadMtrExcEvtRecord(wOI, bAttr, pbRecBuf, iLen, wBufSize);
}


BYTE DoMtrExc(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn, bool* pfModified)
{
	//把所有需要抄表的数据项统一抄上来
	int iRet, nOADLen;
	DWORD dwOAD;
	WORD i, wIndex, wJudgeOadNum = 0;
	WORD wRSDLen = 0, wRCSDLen = 0;
	bool fTrigerSave;
	WORD wOI, wFailCnt = 0;
	BYTE bRSDBuf[32], bRCSDBuf[256];
	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	BYTE bBuf[60];
	BYTE bOADBuf[4];

	if (GetInfo(INFO_MTR_EXC_RESET))		
	{
		DTRACE(DB_FA, ("DoMtrExc rx INFO_MTR_EXC_RESET...\n"));
		ClrMtrExc();	//事件清零
		InitMtrExc();	//初始化建表 （事件判断中间数据在抄表线程中，有效测量点才初始化）
	}

	for (i=0; i<MTR_EXC_NUM; i++)
	{
		memset(bRelaOAD, 0, sizeof(bRelaOAD));
		memset(dwJudgeOAD, 0, sizeof(dwJudgeOAD));

		wOI = GetMtrExcOI(i);

		if (IsMtrExcTableCreate(wOI) == false)	//表未创建, 事件不用判断了
			continue;

		wJudgeOadNum = 0;
		if (GetMtrExcRdItem(wOI, dwJudgeOAD, &wJudgeOadNum, bRelaOAD))
		{
			for (wIndex=0; wIndex<wJudgeOadNum; wIndex++)	//wJudgeOAD取出的ID主要是为了防止其它任务没有抄这里需要的数据项时，这里再抄一下
			{
				dwOAD = dwJudgeOAD[wIndex];
				if (dwOAD == 0x40000200)	//hyl 3105不在这里抄表
					continue;

				if (GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bBuf) <= 0)	//没抄到
				{
					iRet = AskMtrItem(pMtrPro, RESPONSE_TYPE_NORAML, dwOAD, bBuf, bRSDBuf, wRSDLen, bRCSDBuf, wRCSDLen);
					if (iRet > 0)	//抄表正常
					{
						OoDWordToOad(dwOAD, bOADBuf);
						nOADLen = OoGetDataLen(DT_OAD, bOADBuf);
						if (nOADLen > 0)
						{
#ifdef TERM_EVENT_DEBUG
							SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bBuf, nOADLen);
#else
							SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bBuf+1, nOADLen);	//+1 跳过DAR
#endif
							SaveMtrDataHook(dwOAD, &pMtrRdCtrl->mtrExcTmp, 0);
							*pfModified = true; //测量点数据已修改
						}
						wFailCnt = 0;
						if (IsMtrErr(wPn))
						{
							OnMtrErrRecv(wPn);
							DoPortRdErr(false);
						}
					}
					else if (iRet == 0)
					{
						if (IsMtrErr(wPn))
							break;
						wFailCnt++;
						if (wFailCnt >= 3)
						{
							OnMtrErrEstb(wPn);
							DoPortRdErr(true);
							break;
						}
					}
					else
					{
						OoDWordToOad(dwOAD, bOADBuf);
						nOADLen = OoGetDataLen(DT_OAD, bOADBuf);
						memset(bBuf, INVALID_DATA, sizeof(bBuf));
						if (nOADLen > 0)
						{
#ifdef TERM_EVENT_DEBUG
							SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bBuf, nOADLen);
#else
							SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bBuf+1, nOADLen);	//+1 跳过DAR
#endif
							SaveMtrDataHook(dwOAD, &pMtrRdCtrl->mtrExcTmp, 0);
							*pfModified = true; //测量点数据已修改
						}
					}
				}
			}
		}

		//在相应的事件函数中进行发生、结束判断，更新状态机
		//再调用ReadAndSaveMtrExcRec()抄数据及保存记录
		switch(wOI)
		{
		case OI_MTR_CLOCK_ERR:
			DoMtrClockErr(pMtrRdCtrl, pMtrPro, wPn);
			break;

		case OI_MTR_ENERGY_DEC:
			DoMtrEnergyDec(pMtrRdCtrl, pMtrPro, wPn);
			break;

		case OI_MTR_ENERGY_ERR:
			DoMtrEnergyErr(pMtrRdCtrl, pMtrPro, wPn);
			break;
			
		case OI_MTR_FLEW:
			DoMtrFlew(pMtrRdCtrl, pMtrPro, wPn);
			break;

		case OI_MTR_STOP:
			DoMtrStop(pMtrRdCtrl, pMtrPro, wPn);
			break;

		case OI_MTR_RD_FAIL:
			DoMtrRdFail(pMtrRdCtrl, pMtrPro, wPn);	//这里只更新需捕获的关联OAD数据和统计数据
			break;
		case OI_MTR_DATA_CHG:
			DoMtrDataChg(pMtrRdCtrl, pMtrPro, wPn);	
			break;
		default:
			break;
		}
	}

	return 1;
}

//获取最近一次事件物理存储位置
//返回：>=0成功，<0失败
int GetLastRecPhyIdx(WORD wOI, TMtrExcTmp* pMtrTmp)
{
	int nIndex;

	nIndex = GetMtrExcIndex(wOI);
	if (nIndex < 0)
		return -1;

	return pMtrTmp->wLastRecPhyIdx[nIndex];
}

void UpdateLastRecPhyIdx(WORD wOI, TMtrExcTmp* pMtrTmp, int nRecPhyIdx)
{
	int nIndex;

	nIndex = GetMtrExcIndex(wOI);
	if (nIndex<0 || nRecPhyIdx<0)
		return;
	else
		pMtrTmp->wLastRecPhyIdx[nIndex] = nRecPhyIdx;
}


//描述：抄电表异常的关联数据项,保存到临时记录区，在发生后与结束后保存记录到任务库
//参数：@pMtrRdCtrl电表抄读控制
//		@wOI事件的OI
//		@bState事件状态：发生前、事件发生后、事件结束前、事件结束后，
//				用来决定关联数据项的属性是否符合
//		@pbRelaOAD	用来返回关联对象属性表
//返回:如果事件需要判断且配置正确则返回true,否则返回false
bool ReadAndSaveMtrExcRec(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wOI, BYTE bState, BYTE* pbRelaOAD, bool* pfIsSaveRec)
{
	#if 0
	BYTE g_bTestData[] = {0x01, 0x02, 0x51, 0x00, 0x10, 0x02, 0x00, 
									  0x51, 0x00, 0x20, 0x02, 0x00,};
	#endif

	int iLen, iRecLen = 0, nMtrExcIdx = 0;
	WORD i, wItemOffset = 0, wItemLen = 0;
	bool fReadSuccess = true, fOnMtrExcHap, fOnMtrExcEnd;
	BYTE bCapNum, bIndex, bType = 0, bChnNum = 0, bTsaLen = 0;
	int nOADLen, nLastRecPhyIdx;
	DWORD dwOAD, dwEvtOAD, dwRecIndex = 0;
	BYTE bBuf[MTR_EXC_REC_LEN], bTmpBuf[100];
	BYTE bRecBuf[MTR_EXC_REC_LEN];
	BYTE bFixBuf[20], bOADBuf[4];
	TTime tmCurRec;	
	//TFieldParser tFixFields = { g_bMtrExcFixOAList, sizeof(g_bMtrExcFixOAList) };	//固定字段
	BYTE* pbBuf = bBuf;
	BYTE* pbRec = bRecBuf;
	const ToaMap* pOaMap = NULL;
	const ToaMap* pEvtOaMap = NULL;
	WORD wRSDLen, wRCSDLen = 0;
	BYTE bRSDBuf[32], bRCSDBuf[256];
	TMtrExcTmp* pExcTmp = &pMtrRdCtrl->mtrExcTmp;

	nMtrExcIdx = GetMtrExcIndex(wOI);
	if (nMtrExcIdx < 0)
		return false;

	fOnMtrExcHap = (bState==EVT_S_AFT_HP && pExcTmp->dwLastStatClick[nMtrExcIdx]==0);		//事件刚发生标志
	fOnMtrExcEnd = (bState==EVT_S_AFT_END && pExcTmp->dwLastStatClick[nMtrExcIdx]!=0);		//事件刚结束标志

	//关联属性表数据字段打包，事件每个状态机都要更新
	memset(bBuf, 0, sizeof(bBuf));
	bCapNum = (pbRelaOAD[1]>CAP_OAD_NUM) ? CAP_OAD_NUM : pbRelaOAD[1];
	if (bCapNum > 0)
	{
		iLen = ReadMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, MEM_TYPE_MTREXC, wOI, bBuf);
		if (iLen <= 0)
		{
			DTRACE(DB_METER_EXC, ("ReadAndSaveMtrExcRec: wOI = %04x read mem fail!\r\n", wOI));
			return false;
		}

		for (i=0; i<bCapNum; i++)		//根据事件状态抄读关联数据项
		{
			dwOAD = OoOadToDWord(&pbRelaOAD[5*i+3]);
			nOADLen = OoGetDataLen(DT_OAD, &pbRelaOAD[5*i+3]);
			if (nOADLen <= 0)
				return false;

			if (bState!=(BYTE ) ((dwOAD&~OAD_FEAT_MASK)>>OAD_FEAT_BIT_OFFSET))	//属性特征不一致
			{
				pbBuf += nOADLen;
				continue;
			}
			else if (wOI==OI_MTR_RD_FAIL && (bState==EVT_S_AFT_HP||bState==EVT_S_BF_END))	//抄表失败事件发生时，不用去抄表了
			{
				memset(pbBuf, 0, nOADLen);	//全0为无效数据?
				pbBuf += nOADLen;
				continue;
			}

			memset(bTmpBuf, 0, sizeof(bTmpBuf));
			dwOAD &= OAD_FEAT_MASK;		//去除属性特征再抄表
			if (dwOAD == 0x40000200)	//hyl 3105不在这里抄表
				continue;
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bTmpBuf);
			if (iLen<=0 || IsAllAByte(bTmpBuf, 0, nOADLen))	//没抄到
			{
				if (AskMtrItem(pMtrPro, RESPONSE_TYPE_NORAML, dwOAD, bTmpBuf, bRSDBuf, nOADLen, bRCSDBuf, wRCSDLen) > 0)	//抄表正常
				{
					SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bTmpBuf, nOADLen);	//不用跳过DAR zhq modify 17-02-17
					memcpy(pbBuf, bTmpBuf+1, nOADLen);	//+1 跳过DAR

					SaveMtrDataHook(dwOAD, &pMtrRdCtrl->mtrExcTmp, 0);
				}
				else
				{
					memset(pbBuf, 0, nOADLen);	//全0为无效数据
					fReadSuccess = false;
				}
			}
			else
			{
				memcpy(pbBuf, bTmpBuf, nOADLen);
			}

			pbBuf += nOADLen;
		}

		iLen = WriteMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, MEM_TYPE_MTREXC, wOI, bBuf);		//更新临时数据区
	}

	//整笔记录打包：先打包固定字段数据，再把打包好的数据字段数拷入，拼成一笔完整记录
	if (bState==EVT_S_AFT_HP || bState==EVT_S_AFT_END)
	{
		dwEvtOAD = GetOAD(wOI, ATTR2, 0);	//事件对应OAD
		pEvtOaMap = GetOIMap(dwEvtOAD);
		if (pEvtOaMap==NULL || pEvtOaMap->pszTableName==NULL)
		{
			DTRACE(DB_METER_EXC, ("ReadAndSaveMtrExcRec: dwOAD %08x not found, or table name is null!\r\n", dwEvtOAD));
			return false;
		}

		DTRACE(DB_METER_EXC, ("ReadAndSaveMtrExcRec: dwEvt OAD = %08x, bState = %d.\r\n", dwEvtOAD, bState));
		memset(bRecBuf, 0, sizeof(bRecBuf));
		iLen = ReadLastNRec(pEvtOaMap->pszTableName, LAST_REC, bRecBuf, sizeof(bRecBuf));	//读出上一笔记录取最新事件记录序号
		if (iLen > 0)
			dwRecIndex = OoDoubleLongUnsignedToDWord(bRecBuf+1);	//事件记录序号 高字节在前传输

		if (fOnMtrExcHap)	//刚发生事件
		{
			dwRecIndex++;	//发生时递增
			memset(bRecBuf, 0, sizeof(bRecBuf));
		}
		else
		{
			nLastRecPhyIdx = GetLastRecPhyIdx(wOI, &pMtrRdCtrl->mtrExcTmp);
			if (nLastRecPhyIdx >= 0)
#ifdef TERM_EVENT_DEBUG
				iLen = ReadRecByPhyIdx(pEvtOaMap->pszTableName, nLastRecPhyIdx-1, bRecBuf, sizeof(bRecBuf));	//已经发生的事件，先取出之前保存的记录，再更新相应部分
#else
				iLen = ReadRecByPhyIdx(pEvtOaMap->pszTableName, nLastRecPhyIdx, bRecBuf, sizeof(bRecBuf));	//已经发生的事件，先取出之前保存的记录，再更新相应部分
#endif
		}

		pbRec = bRecBuf;

		TMtrExcFixUnitDes* pUnitDesc = GetMtrExcFixUnit(wOI);
		if (pUnitDesc == NULL)
			return false;
			
		if (OoParseField(pUnitDesc->pFixField, pUnitDesc->pbFmt, pUnitDesc->wFmtLen, false) == false)		// 固定字段
			return false;

		for (bIndex=0; bIndex<pUnitDesc->pFixField->wNum; bIndex++)	//固定字段个数----开始固定字段打包
		{
			memset(bFixBuf, 0, sizeof(bFixBuf));
			if (ReadParserField(pUnitDesc->pFixField, bIndex, bFixBuf, &bType, &wItemOffset, &wItemLen) > 0)		//固定字段OAD
			{
				dwOAD = OoOadToDWord(bFixBuf+1);	//固定字段OAD
				pOaMap = GetOIMap(dwOAD);
				if (pOaMap == NULL)
				{
					DTRACE(DB_METER_EXC, ("ReadAndSaveMtrExcRec: Read dwOAD:%x failed !!\r\n", dwOAD));
					return false;
				}

				nOADLen = OoGetDataLen(DT_OAD, bFixBuf+1);
				nOADLen--;		//去掉1字节数据类型
				if (nOADLen <= 0)
				{
					DTRACE(DB_METER_EXC, ("ReadAndSaveMtrExcRec: nOADLen = %d!!\r\n", nOADLen));
					return false;
				}
				
				if (pOaMap->dwOA == 0x20220200)		//事件记录序号 高字节在前传输
				{
					*pbRec++ = *pOaMap->pFmt;	//数据类型

					OoDWordToDoubleLongUnsigned(dwRecIndex, pbRec);
					pbRec += nOADLen;
				}
				else if (pOaMap->dwOA == 0x20240200)	//事件发生源 -- 电表通信地址(oct-string)
				{
#ifdef MTREXC_ADDR_TPYE_TSA
					*pbRec++ = DT_TSA;			//数据类型
#else
					*pbRec++ = DT_OCT_STR;		//数据类型
#endif

					memset(pbRec, 0, nOADLen);

#ifdef MTREXC_ADDR_TPYE_TSA
					bTsaLen = pMtrRdCtrl->bTsa[0] & 0x0f;
					*pbRec++ = (bTsaLen+1);			//OCT_STR长度
					if (bTsaLen > 0)
						*pbRec++ = (bTsaLen-1);		//TSA有效长度
					else
						*pbRec++ = 0;

					if (bTsaLen <= (sizeof(pMtrRdCtrl->bTsa)-1))
						memcpy(pbRec, pMtrRdCtrl->bTsa+1, bTsaLen);	//拷贝电表实际地址长度
					else
						return false;
#else
					*pbRec++ = pMtrRdCtrl->bTsa[0];	//有效长度
					if (pMtrRdCtrl->bTsa[0] < sizeof(pMtrRdCtrl->bTsa))
						memcpy(pbRec, pMtrRdCtrl->bTsa+1, pMtrRdCtrl->bTsa[0]);
					else
						return false;
#endif
					
#ifdef TERM_EVENT_DEBUG
					pbRec += (nOADLen-2); //跳过长度
#else
					pbRec += nOADLen;
#endif
				}
				else if (pOaMap->dwOA == OAD_EVT_RPT_STATE)	//事件上报状态  array 通道上报状态 
				{										//DT_ARRAY 02 DT_STRUCT 02 DT_OAD 45 00 02 00 DT_UNSIGN 00	DT_STRUCT 02 DT_OAD 45 10 02 00 DT_UNSIGN 00
					iLen = OoReadAttr(0x4300, ATTR10, bTmpBuf, NULL, NULL);	//读取配置参数
					if (iLen<=0 || bTmpBuf[0]!=DT_ARRAY)
					{
						*pbRec++ = DT_NULL;		//数据类型
						pbRec += CN_RPT_TOTAL_LEN-1;	//补足
					}
					else
					{
						*pbRec++ = DT_ARRAY;	//数据类型

						bChnNum = (bTmpBuf[1]>CN_RPT_NUM) ? CN_RPT_NUM : bTmpBuf[1];
						*pbRec++ = bChnNum;	//OAD个数

						for (i=0; i<bChnNum; i++)
						{
							*pbRec++ = DT_STRUCT;
							*pbRec++ = 0x02;
							memcpy(pbRec, &bTmpBuf[5*i+2], 5); //通道OAD
							pbRec += 5;

							*pbRec++ = DT_UNSIGN;
							if (fOnMtrExcHap)
								*pbRec = 0x00;
							else
								*pbRec &= 0x03;	//保留发生前的上报标志

							pbRec++;
						}
						pbRec += CN_RPT_STATE_LEN*(CN_RPT_NUM - bChnNum);	//补足
					}

					
				}
				else if (pOaMap->dwOA == 0x40000200)	//电表或终端时钟
				{
					if  (bIndex == pUnitDesc->pFixField->wNum-1)	 //终端时钟
					{
						*pbRec++ = *pOaMap->pFmt;	//数据类型

						GetCurTime(&tmCurRec);
						OoTimeToDateTimeS(&tmCurRec, pbRec);												
					}
					else	//电表时钟
					{
						if (GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, pOaMap->dwOA, pbRec) <= 0)	//没抄到
						{
							*pbRec++ = DT_NULL;	//数据类型
							memset(pbRec, 0, 7);
						}
						else
						{
							pbRec++;	//数据类型
						}
					}

					pbRec += 7;
				}
				else if (pOaMap->dwOA == 0x330f0206)		//事件记录序号 高字节在前传输
				{
					memcpy(pbRec, pExcTmp->mtrDataChg.bCSD, MTEDATACHG_CSD_LEN);
					pbRec += MTEDATACHG_CSD_LEN;
				}
				else if (pOaMap->dwOA == 0x330f0207)		//事件记录序号 高字节在前传输
				{
					memcpy(pbRec, pExcTmp->mtrDataChg.bOldData, MTEDATACHG_DATA_LEN);
					pbRec += MTEDATACHG_DATA_LEN;
				}
				else if (pOaMap->dwOA == 0x330f0208)		//事件记录序号 高字节在前传输
				{
					memcpy(pbRec, pExcTmp->mtrDataChg.bNewData, MTEDATACHG_DATA_LEN);
					pbRec += MTEDATACHG_DATA_LEN;
				}
				else
				{
					if (pOaMap->dwOA != 0x20200200)	//事件结束时间
						*pbRec++ = *pOaMap->pFmt;	//数据类型

					GetCurTime(&tmCurRec);
					if (fOnMtrExcHap)	//事件刚发生
					{
						if (pOaMap->dwOA == 0x201E0200)	//事件发生时间
						{
							OoTimeToDateTimeS(&tmCurRec, pbRec);
						}
						else if (pOaMap->dwOA == 0x20200200)	//发生时，事件结束时间清0
						{
							*pbRec++ = DT_NULL;	//数据类型
							memset(pbRec, 0, nOADLen);
						}
					}
					else if (fOnMtrExcEnd)	//事件刚结束
					{
						if (pOaMap->dwOA == 0x20200200)	//事件结束时间
						{
							*pbRec++ = *pOaMap->pFmt;	//数据类型
							OoTimeToDateTimeS(&tmCurRec, pbRec);
						}
					}
					else if (bState==EVT_S_AFT_HP && pOaMap->dwOA==0x20200200)	//发生期间，结束时间为全0
					{
						*pbRec++ = DT_NULL;	//数据类型
						memset(pbRec, 0, nOADLen);
					}
					else
					{
						pbRec++;	//数据类型
					}

					pbRec += nOADLen;
				}
			}
		}		//---固定字段数据打包结束

		iRecLen = (pbRec-bRecBuf);	//+固定字段长度
		iRecLen += (pbBuf-bBuf);	//+数据字段长度
		if (iRecLen <= sizeof(bRecBuf))
		{
			iLen = pbBuf - bBuf;	//数据字段长度
			if (iLen > 0)
				memcpy(pbRec, bBuf, iLen);	//将关联属性表数据字段内容拷入,拼成一笔记录

			if (fOnMtrExcHap)	//新生成一笔事件记录
			{
				if (SaveRecord(pEvtOaMap->pszTableName, bRecBuf, &nLastRecPhyIdx))
					UpdateLastRecPhyIdx(wOI, &pMtrRdCtrl->mtrExcTmp, nLastRecPhyIdx);	//记录下发生记录的存储位置

				*pfIsSaveRec = true;
			}
			else
			{
				SaveRecordByPhyIdx(pEvtOaMap->pszTableName, nLastRecPhyIdx, bRecBuf);	//先获取发生记录的存储位置，再更新记录
				*pfIsSaveRec = true;
			}
		}
	}
	
	UpdateMtrExcStatData(wOI, bState, pExcTmp, pMtrRdCtrl->bTsa);		//记录保存后，更新事件统计数据

	return fReadSuccess;
}

//抄表事件上报
void DoMtrExcRpt(WORD wOI, BYTE bSendRptFlag)
{
	BYTE bRptFlag = 0, bChnNum = 0;
	int i, iLen, nRecIdx;
	bool fRpt = false;
	DWORD dwOAD;
	BYTE bTmpBuf[60];
	DWORD dwChnOAD;
	const ToaMap* pOaMap = NULL;

	memset(bTmpBuf, 0, sizeof(bTmpBuf));
	if (OoReadAttr(wOI,  ATTR8, bTmpBuf, NULL, NULL) > 0)	//上报标识，不上报（0），事件发生上报（1），事件恢复上报（2），事件发生恢复均上报（3）
		bRptFlag = bTmpBuf[1];
	
	if ((bSendRptFlag==EVT_STAGE_HP && (bRptFlag&0x01)==0x01) || (bSendRptFlag==EVT_STAGE_END && (bRptFlag&0x02)==0x02))		//需要上报
	{
		dwOAD = GetOAD(wOI, ATTR2, 0);
		pOaMap = GetOIMap(dwOAD);
		if (pOaMap==NULL || pOaMap->pszTableName==NULL)
			return;

		nRecIdx = GetRecPhyIdx(pOaMap->pszTableName, 1);
		if (nRecIdx >= 0)
		{
			memset(bTmpBuf, 0, sizeof(bTmpBuf));
			iLen = OoReadAttr(0x4300, ATTR10, bTmpBuf, NULL, NULL);	//读取配置参数
			if (iLen<=0 || bTmpBuf[0]!=DT_ARRAY)
				return;
			
			bChnNum = (bTmpBuf[1]>CN_RPT_NUM) ? CN_RPT_NUM : bTmpBuf[1];
			for (i=0; i<bChnNum; i++)
			{
				dwChnOAD = OoOadToDWord(&bTmpBuf[5*i+3]);	//通道OAD
				SendEvtMsg(dwChnOAD, dwOAD, nRecIdx, bSendRptFlag);
			}
		}
	}
}


//描述：更新事件状态机，并抄读数据保存记录
//参数:@pbState用来返回事件状态机状态
//     @bCurErcFlag:当前事件状态 0：中间态，1：发生，2：恢复
void UpdateMtrExcStateAndSaveRec(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wOI, BYTE* pbRelaOAD, BYTE bCurErcFlag, BYTE* pbState)
{
	bool fRet = false;	//关联数据项的抄表结果
	BYTE bSendRptFlag = EVT_STAGE_UNCARE;
	int nMtrExcIdx = -1;
	TMtrExcTmp* pExcTmp = &pMtrRdCtrl->mtrExcTmp;
	bool fIsSaveRec = false;
	DWORD dwROAD;

	nMtrExcIdx = GetMtrExcIndex(wOI);
	if (nMtrExcIdx < 0)
		return;
	
	switch (*pbState)	//事件状态机
	{
	case EVT_S_BF_HP:
		if (bCurErcFlag == ERC_STATE_HAPPEN)	//发生
			*pbState = EVT_S_AFT_HP;

		fRet = ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, *pbState, pbRelaOAD, &fIsSaveRec);	//返回抄读结果
		if (*pbState==EVT_S_AFT_HP && fRet)
		{
			*pbState = EVT_S_BF_END;
			bSendRptFlag = EVT_STAGE_HP;
		}
		else if (!fRet)
		{
			pExcTmp->bTryReadCnt[nMtrExcIdx]++;
		}

		break;

	case EVT_S_AFT_HP:
		fRet = ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, *pbState, pbRelaOAD, &fIsSaveRec);
		if (fRet || pExcTmp->bTryReadCnt[nMtrExcIdx]>TRY_READ_NUM)	//抄表成或达到尝试抄表次数时切换到下一状态
		{
			*pbState = EVT_S_BF_END;
			pExcTmp->bTryReadCnt[nMtrExcIdx] = 0;
			bSendRptFlag = EVT_STAGE_HP;
		}
		else
		{
			pExcTmp->bTryReadCnt[nMtrExcIdx]++;
		}

		break;

	case EVT_S_BF_END:
		if (bCurErcFlag == ERC_STATE_RECOVER)	 //恢复
			*pbState = EVT_S_AFT_END;

		fRet = ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, *pbState, pbRelaOAD, &fIsSaveRec);
		if (*pbState==EVT_S_AFT_END && fRet)
		{
			*pbState = EVT_S_BF_HP;
			bSendRptFlag = EVT_STAGE_END;
		}
		else if (!fRet)
		{
			pExcTmp->bTryReadCnt[nMtrExcIdx]++;
		}

		break;

	case EVT_S_AFT_END:
		fRet = ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, *pbState, pbRelaOAD, &fIsSaveRec);
		if (fRet || pExcTmp->bTryReadCnt[nMtrExcIdx]>TRY_READ_NUM)
		{
			*pbState = EVT_S_BF_HP;
			pExcTmp->bTryReadCnt[nMtrExcIdx] = 0;
			bSendRptFlag = EVT_STAGE_END;
		}
		else
		{
			pExcTmp->bTryReadCnt[nMtrExcIdx]++;
		}

		break;
	}

	if (fIsSaveRec)
	{	
		dwROAD = GetOAD(wOI, 0x02, 0);
		AddEvtOad(dwROAD, bSendRptFlag);
	}

	DoMtrExcRpt(wOI, bSendRptFlag);	//上报事件
}


////置位建表标志
void SetMtrExcTableFlag(WORD wOI)
{
	WORD wPn;
	BYTE bFlag = 1;

	wPn = GetMtrExcIndex(wOI);
	WriteItemEx(BN2, wPn, 0x2300, &bFlag);
}


//清除建表标志
void ClrMtrExcTableFlag(WORD wOI)
{
	WORD wPn;
	BYTE bFlag = 0;

	wPn = GetMtrExcIndex(wOI);
	WriteItemEx(BN2, wPn, 0x2300, &bFlag);
}

//查询标志
bool IsMtrExcTableCreate(WORD wOI)
{
	WORD wPn;
	int iLen;
	BYTE bFlag = 0;

	wPn = GetMtrExcIndex(wOI);
	iLen = ReadItemEx(BN2, wPn, 0x2300, &bFlag);
	if (iLen > 0)
		return (bFlag==1);
	else
		return false;
}


//========================电能表时间超差==========================
//描述：电能表时间超差事件参数的初始化。
void InitMtrClockErr(WORD wPn, TMtrClockErr* pCtrl)
{
	pCtrl->bState = EVT_S_BF_HP;
}


//描述：电能表时间超差事件的判断：如果终端和电表时间超过电表校时阀值（默认为5分钟），则认为电表时间超差。
bool DoMtrClockErr(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{	
	bool fRet;
	DWORD dwMtrSecs, dwCurSecs, dwDiff;
	int iLen;	
	TTime time;
	TTime tmMtrTime;
	BYTE bCurState, bLen;
	WORD wChecktmHold, wRecoverHold, wJudgeOadNum = 0;
	BYTE bAlrBuf = 0, bCurErcFlag = 0;
	BYTE bBuf[40];
	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE* pbBuf = bBuf;
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	const WORD wOI = OI_MTR_CLOCK_ERR;

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrClockErr* pCtrl = &pMtrTmp->mtrClockErr;

	memset(&time, 0, sizeof(time));
	memset(&tmMtrTime, 0, sizeof(tmMtrTime));
	if (!GetMtrExcRdItem(wOI, &dwJudgeOAD[0], &wJudgeOadNum, bRelaOAD))
		return false;

	//if (IsRxTrigerSaveCmd(wPn, wOI, pCtrl->bState, &bCurState))
	//	return ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, bCurState, bRelaOAD);

	if (QueryMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD, wJudgeOadNum))		//数据到达
	{
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD wIndex=0; wIndex<wJudgeOadNum; wIndex++)
		{
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD[wIndex], pbBuf);
			if (iLen <= 0)	//没抄到
				return false;

			pbBuf += iLen;
		}

		OoDateTimeSToTime(bBuf+1, &tmMtrTime);
#ifdef TERM_EVENT_DEBUG
		tmMtrTime.nSecond = 0; //目前三相表回的日期时间秒数据为FF，表回错了数据，暂时清零处理 --QLS
#endif

		if (IsInvalidTime(tmMtrTime))
			return false;

		memset(bBuf, 0, sizeof(bBuf));
		iLen = OoReadAttr(wOI, ATTR6, bBuf, NULL, NULL);	// 读取配置参数
		if (iLen <= 0 || bBuf[2] != DT_LONG_U)
			return false;

		wChecktmHold = OoLongUnsignedToWord(&bBuf[3]);	//异常判别阈值  long-unsigned（单位：秒）
		if (wChecktmHold == 0)
			wChecktmHold = 300;

		wRecoverHold = wChecktmHold;
		dwMtrSecs = TimeToSeconds(tmMtrTime);		//电表当前时间换算成秒
		dwCurSecs = pMtrTmp->dwItemRdTime[CLOCK_ITEM_INDEX];	//终端成功抄读电表时钟的时标
		SecondsToTime(dwCurSecs, &time);
		if (dwCurSecs==0 || IsInvalidTime(time))
		{
			DTRACE(DB_METER_EXC, ("CMtrTimeErr::RunTask: fail to get term time, when c011 read, pn=%d, dwCurSecs=%ld.\r\n", wPn, dwCurSecs));
			DTRACE(DB_METER_EXC, ("CMtrTimeErr::RunTask: curTime = %02d-%02d-%02d %02d:%02d:%02d, pn=%d\r\n", time.nYear, time.nMonth, time.nDay,
                                  time.nHour, time.nMinute, time.nSecond, wPn));
			return false;
		}

		dwDiff = GetAbsGap(dwCurSecs, dwMtrSecs);	//电表和终端时间差绝对值
		
		if (dwDiff >= (DWORD)wChecktmHold)	//发生状态
		{
			bCurErcFlag = ERC_STATE_HAPPEN;
			//DTRACE(DB_METER_EXC, ("CMtrTimeErr::###### CMtrTimeErr event happened!!! ######\r\n"));
			//DTRACE(DB_METER_EXC, ("CMtrTimeErr::RunTask: dwDiff=%ds  pCtrl->bState=%d\r\n", dwDiff, pCtrl->bState));
		}
		else if (dwDiff < (DWORD)wRecoverHold)	//结束状态
			bCurErcFlag = ERC_STATE_RECOVER;
		else
			bCurErcFlag = ERC_STATE_MIDDLE;

		UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bCurErcFlag, &pCtrl->bState);

		return true;
	}

	return false;
}




void InitEnergyDec(BYTE wPn, TMtrEnergyDec* pCtrl)
{
	BYTE i;

	pCtrl->ui64PosE = 0;		//上周期正向有功
	pCtrl->ui64NegE = 0;	//上周期正向有功
	pCtrl->bState = EVT_S_BF_HP;

	memset(pCtrl->bAddr, 0, sizeof(pCtrl->bAddr));
	for (i=0; i<2; i++)
		pCtrl->fInvalid[i] = true;
}


//电能表示度下降
bool DoMtrEnergyDec(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{
  	BYTE i = 0;
	WORD wValidNum = 0;
	BYTE bBuf[100];
	BYTE bAlrBuf[20];
	BYTE bAddrBuf[MTR_ADDR_LEN];
	TTime time;	
	BYTE bMtrInterv;
	WORD wIdNum;	
	bool fInvalid[2];//正向、反向有功电能值是否有效 
	int iLen = 0;
	BYTE bCurState, bTotalErcFlag, bLen;
	BYTE bCurErcFlag[2];
	WORD wJudgeOadNum = 0;

	uint64 ui64PosE, ui64NegE, ui64Energy = 0, ui64PreEnergy = 0;

	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE* pbBuf = bBuf;
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	const WORD wOI = OI_MTR_ENERGY_DEC;

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrEnergyDec* pCtrl = &pMtrTmp->mtrEnergyDec;

	memset(bBuf, 0, sizeof(bBuf));
	memset(bAlrBuf, 0, sizeof(bAlrBuf));
	memset(bAddrBuf, 0, sizeof(bAddrBuf));
	memset(fInvalid, 0, sizeof(fInvalid));
	memset(&time, 0, sizeof(time));

	if (!GetMtrExcRdItem(wOI, &dwJudgeOAD[0], &wJudgeOadNum, bRelaOAD))
		return false;

	//if (IsRxTrigerSaveCmd(wPn, wOI, pCtrl->bState, &bCurState))
	//	return ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, bCurState, bRelaOAD);

	if (pMtrRdCtrl->bTsa[0] >= sizeof(pMtrRdCtrl->bTsa))
		return false;

	//获取表地址值并判断是否有效；如果表地址为0，说明未配置表地址，返回；
	if(IsAllAByte(&pMtrRdCtrl->bTsa[1], 0, pMtrRdCtrl->bTsa[0]))	//地址全0
	{
		pCtrl->fInvalid[0] = pCtrl->fInvalid[1] = true;
		return false;
	}
	else
	{		
		memcpy(bAddrBuf, &pMtrRdCtrl->bTsa[0], sizeof(bAddrBuf));
	}	

	if (QueryMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD, wJudgeOadNum))		//数据到达
	{
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD wIndex=0; wIndex<wJudgeOadNum; wIndex++)
		{
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD[wIndex], pbBuf);
			if (iLen <= 0)	//没抄到
				return false;
			
			pbBuf += iLen;
		}		

		fInvalid[0] = fInvalid[1] = false;
		
#ifdef TERM_EVENT_DEBUG
		ui64PosE = GetEnergyValByFmt(bBuf[0], bBuf, &bLen);	//正向有功总
		ui64NegE = GetEnergyValByFmt(bBuf[bLen], bBuf+bLen, &bLen);	//反向有功总
#else
		ui64PosE = GetEnergyValByFmt(bBuf[0], bBuf+1, &bLen);	//正向有功总
		ui64NegE = GetEnergyValByFmt(bBuf[bLen], bBuf+bLen+1, &bLen);	//反向有功总
#endif

		if (memcmp(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1) != 0)	//表地址改变
		{
			memcpy(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1);
		}
		else
		{
			memset(bCurErcFlag, ERC_STATE_MIDDLE, sizeof(bCurErcFlag));
			for (i=0; i<2; i++)
			{
				if (pCtrl->fInvalid[i] || fInvalid[i])	//上周期值或本周期电能值无效，不进行判断；
					continue;

				if (i == 0)
				{
					ui64Energy = ui64PosE;	//先比较正向有功电能值；
					ui64PreEnergy = pCtrl->ui64PosE;
				}
				else
				{
					ui64Energy = ui64NegE;
					ui64PreEnergy = pCtrl->ui64NegE;
				}
								
				if (ui64Energy<ui64PreEnergy && (ui64PreEnergy-ui64Energy)<9999990000LL)	//有功电能值下降且发生标志置0；（防止电表走到满刻度重新走字）
				{
					bCurErcFlag[i] = ERC_STATE_HAPPEN;	//发生
					//DTRACE(DB_METER_EXC, ("EnergyDec::###### EnergyDec event happened!!! ######\r\n"));
				}
				else if (ui64Energy >= ui64PreEnergy)	//有功电能值没有下降且发生标志置1；
					bCurErcFlag[i] = ERC_STATE_RECOVER;	//恢复
				else
					bCurErcFlag[i] = ERC_STATE_MIDDLE;
			}

			if (bCurErcFlag[0]==ERC_STATE_HAPPEN || bCurErcFlag[1]==ERC_STATE_HAPPEN)	//正有 或 反有发生
				bTotalErcFlag = ERC_STATE_HAPPEN;
			else if (bCurErcFlag[0]==ERC_STATE_RECOVER && bCurErcFlag[1]==ERC_STATE_RECOVER)	//正有和反有都恢复
				bTotalErcFlag = ERC_STATE_RECOVER;
			else
				bTotalErcFlag = ERC_STATE_MIDDLE;

			UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bTotalErcFlag, &pCtrl->bState);
		}

		//将本周期的值存入作为下一周期比较依据；
		if (!fInvalid[0])
			pCtrl->ui64PosE = ui64PosE;

		if (!fInvalid[1])
			pCtrl->ui64NegE = ui64NegE;

		pCtrl->fInvalid[0] = fInvalid[0];
		pCtrl->fInvalid[1] = fInvalid[1];
	}

	return true;
}


//电能量超差
void InitEnergyErr(BYTE wPn, TMtrEnergyErr* pCtrl)
{
	BYTE i;
	pCtrl->ui64PosE = 0;	//上周期正向有功
	pCtrl->ui64NegE = 0;	//上周期反向有功
	pCtrl->bState = EVT_S_BF_HP;

	memset(pCtrl->bAddr, 0, sizeof(pCtrl->bAddr));
	for(i=0; i<2; i++)
	{
		pCtrl->fInvalid[i] = true;
		pCtrl->dwSeconds[i] = 0;
	}
}


bool DoMtrEnergyErr(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{
	DWORD dwFlewHold = 0;// 超差阀值；
	BYTE i = 0;
	BYTE bBuf[100], bAddr[6];
	
	DWORD dwCurMin;
	WORD wIdNum, wValidNum;
	BYTE bMtrInterv = 2;	//GetMeterInterv();		//抄表间隔函数GetMeterInterv()能用？？？？
	int iLen = 0;
	TTime time;
	
	bool fInit[2];

	WORD wUn = 0 ;//额定电压值
	WORD wIn = 0;//最大电流值(1个字节，1个小数位）
	BYTE bConnType = 0, bCurState, bLen;

	uint64 ui64DeltEnergy = 0;//电表按照示值走过的电量(4位小数) 
	uint64 ui64PastEnergy = 0;//电表按照最大功率走过的电量

	bool fInvalid[2];//正、反向有功值是否有效
	uint64 ui64PosE = 0;//当前正向有功
	uint64 ui64NegE = 0;//当前反向有功

	TOobMtrInfo tMtrInfo;
	BYTE btemp = 0, bTotalErcFlag;
	WORD wE0 = 0, wJudgeOadNum = 0;
	DWORD dwTimePast = 0;
	BYTE bCurErcFlag[2];
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	BYTE bAddrBuf[MTR_ADDR_LEN];
	const WORD wOI = OI_MTR_ENERGY_ERR;

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrEnergyErr* pCtrl = &pMtrTmp->mtrEnergyErr;
	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE* pbBuf = bBuf;

	//memset(bAlrBuf, 0, sizeof(bAlrBuf));
	memset(bAddr, 0, sizeof(bAddr));
	memset(&time, 0, sizeof(time));
	memset(fInit, 0, sizeof(fInit));

	if (!GetMtrExcRdItem(wOI, &dwJudgeOAD[0], &wJudgeOadNum, bRelaOAD))
		return false;

	//if (IsRxTrigerSaveCmd(wPn, wOI, pCtrl->bState, &bCurState))
	//	return ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, bCurState, bRelaOAD);

	if (pMtrRdCtrl->bTsa[0] >= sizeof(pMtrRdCtrl->bTsa))
		return false;

	//获取表地址值并判断是否有效；如果表地址为0，说明未配置表地址，返回；
	if(IsAllAByte(&pMtrRdCtrl->bTsa[1], 0, pMtrRdCtrl->bTsa[0]))	//地址全0
	{
		pCtrl->fInvalid[0] = pCtrl->fInvalid[1] = true;
		return false;
	}
	else
	{		
		memcpy(bAddrBuf, &pMtrRdCtrl->bTsa[0], sizeof(bAddrBuf));
	}	

	if (QueryMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD, wJudgeOadNum))		//数据到达
	{
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD wIndex=0; wIndex<wJudgeOadNum; wIndex++)
		{
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD[wIndex], pbBuf);
			if (iLen <= 0)	//没抄到
				return false;
			
			if (pbBuf[2] == DT_DB_LONG_U)
				iLen = 5;	//取出总电能
			else
				iLen = 9;	//取出总电能

			memmove(pbBuf, pbBuf+2, iLen);
			pbBuf += iLen;
		}		

		fInvalid[0] = fInvalid[1] = false;

#ifdef TERM_EVENT_DEBUG
		ui64PosE = GetEnergyValByFmt(bBuf[0], bBuf, &bLen);	//正向有功总
		ui64NegE = GetEnergyValByFmt(bBuf[bLen], bBuf+bLen, &bLen);	//反向有功总
#else
		ui64PosE = GetEnergyValByFmt(bBuf[0], bBuf+1, &bLen);	//正向有功总
		ui64NegE = GetEnergyValByFmt(bBuf[bLen], bBuf+bLen+1, &bLen);	//反向有功总
#endif

		if (memcmp(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1) != 0)		//当第一次抄表或表地址更换后，重新开始判断
		{
			memcpy(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1);
			fInit[0] = true;
			fInit[1] = true;
		}
		else
		{
			if (!GetMeterInfo(wPn, &tMtrInfo))	//读取电表额定电压，额定电流，接线方式（设置参数F25）；
				return false;
	
			wUn = tMtrInfo.wRateVol; //额定电压值
			wIn = tMtrInfo.wRateCurr; //额定电流值
#ifdef TERM_EVENT_DEBUG
			bConnType = tMtrInfo.bLine;
#else
			bConnType = GetConnectType(wPn);
#endif
			if (wUn == 0)
				wUn = 2200;

			if (wIn == 0)
				wIn = 50;
#ifdef TERM_EVENT_DEBUG
			if (bConnType == 2)
#else
			if (bConnType == CONNECT_3P3W)//三相三线
#endif
			{
				wE0 = 500;//0.05*10000
				btemp = 2;//三相三线b相无电流，故算两相功率即可；
			}
			else
			{
				wE0 = 1000;//0.1*10000
				btemp = 3;
			}

			memset(bBuf, 0, sizeof(bBuf));
			iLen = OoReadAttr(wOI, ATTR6, bBuf, NULL, NULL); // 读取配置参数
			if (iLen<=0 || bBuf[2]!=DT_DB_LONG_U)
				dwFlewHold = 500;	//5.0倍数
			else
				dwFlewHold = OoDoubleLongUnsignedToDWord(&bBuf[3]);	//异常判别阈值 double-long-unsigned（单位：%，无换算）
#ifdef TERM_EVENT_DEBUG
			if (dwFlewHold == 0)   
				dwFlewHold = 500;
#endif

			memset(bCurErcFlag, ERC_STATE_MIDDLE, sizeof(bCurErcFlag));

			for (i=0; i<2; i++)		//判断；
			{
				//此周期或上周期有功电能值无效，则不判断；
				if (fInvalid[i] || pCtrl->fInvalid[i])
				{
					fInit[i] = true;
					continue;
				}

				if (i==0 && ui64PosE>pCtrl->ui64PosE)	//计算上周期和本周期电能差值，如果示度下降或停走，不判断；
				{
					ui64DeltEnergy = ui64PosE - pCtrl->ui64PosE;
				}
				else if (i==1 && ui64NegE>pCtrl->ui64NegE)
				{
					ui64DeltEnergy = ui64NegE - pCtrl->ui64NegE;
				}
				else
				{
					fInit[i] = true;
					continue;
				}

				//DTRACE(DB_METER_EXC, ("EnergyErr::******RunTask:pn=%d, TimeDec=%d\r\n", wPn, GetClick()-pCtrl->dwSeconds[i]));
				//按照最大功率走过的电量
				if((GetClick()-pCtrl->dwSeconds[i]) < ((DWORD)bMtrInterv*60/2))
					continue;
				dwTimePast = (GetClick()-pCtrl->dwSeconds[i]) / ((DWORD) bMtrInterv*60) * ((DWORD)bMtrInterv*60);
				if (dwTimePast == 0)	//不到一个抄表间隔的时间
					dwTimePast = GetClick() - pCtrl->dwSeconds[i];

				ui64PastEnergy = ((uint64) wUn) * wIn * btemp * dwTimePast;	//w（２位小数）

				//当电表的示度走过的电量超过wE0,重新开始判断；
					//（电量单位：w）
				ui64PastEnergy *= dwFlewHold;			//电表按照最大功率走过的电量
				ui64DeltEnergy *= (3600 * 1000 * 100);	//电表示度走过的电量

				if (ui64DeltEnergy >= ui64PastEnergy)	//有功电能值下降且发生标志置0；（防止电表走到满刻度重新走字）
				{
					bCurErcFlag[i] = ERC_STATE_HAPPEN;	//发生
					DTRACE(DB_METER_EXC, ("EnergyErr::###### EnergyErr event happened! ui64DeltEnergy=%u,  ui64PastEnergy=%u.######\r\n", ui64DeltEnergy, ui64PastEnergy));					
				}
				else if (ui64DeltEnergy < ui64PastEnergy)	//有功电能值没有下降且发生标志置1；
				{
					bCurErcFlag[i] = ERC_STATE_RECOVER;	//恢复
					DTRACE(DB_METER_EXC, ("EnergyErr::###### EnergyErr event recover! ui64DeltEnergy=%u,  ui64PastEnergy=%u.######\r\n", ui64DeltEnergy, ui64PastEnergy));					
				}
				else
				{
					bCurErcFlag[i] = ERC_STATE_MIDDLE;
				}

				fInit[i] = true;
			}

			if (bCurErcFlag[0]==ERC_STATE_HAPPEN || bCurErcFlag[1]==ERC_STATE_HAPPEN)	//正有 或 反有发生
				bTotalErcFlag = ERC_STATE_HAPPEN;
			else if (bCurErcFlag[0]==ERC_STATE_RECOVER && bCurErcFlag[1]==ERC_STATE_RECOVER)	//正有和反有都恢复
				bTotalErcFlag = ERC_STATE_RECOVER;
			else
				bTotalErcFlag = ERC_STATE_MIDDLE;

			UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bTotalErcFlag, &pCtrl->bState);
		}

		//初始化
		for (i=0; i<2; i++)
		{
			pCtrl->fInvalid[i] = fInvalid[i];
			if (!fInvalid[i] && fInit[i])
			{
				pCtrl->dwSeconds[i] = GetClick();
				if (i == 0)
					pCtrl->ui64PosE = ui64PosE;
				else
					pCtrl->ui64NegE = ui64NegE;
			}
		}
	}

	return true;
}


void InitMtrFlew(BYTE wPn, TMtrFlew* pCtrl)
{
	BYTE i;
	pCtrl->ui64PosE = 0;	//上周期正向有功
	pCtrl->ui64NegE = 0;	//上周期反向有功
	pCtrl->bState = EVT_S_BF_HP;

	memset(pCtrl->bAddr, 0, sizeof(pCtrl->bAddr));
	for(i=0; i<2; i++)
	{
		pCtrl->fInvalid[i] = true;
		pCtrl->dwSeconds[i] = 0;
	}
}


bool DoMtrFlew(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{
	DWORD dwFlewHold = 0;// 超差阀值；
	BYTE i = 0;
	BYTE bBuf[100], bAddr[6];
	
	DWORD dwCurMin;
	WORD wIdNum, wValidNum;
	BYTE bMtrInterv = 2;	//GetMeterInterv();
	int iLen = 0;
	TTime time;
		
	bool fInit[2];

	WORD wUn = 0 ;//额定电压值
	WORD wIn = 0;//最大电流值(1个字节，1个小数位）
	BYTE bConnType = 0, bCurState, bLen;

	uint64 ui64DeltEnergy = 0;//电表按照示值走过的电量(4位小数) 
	uint64 ui64PastEnergy = 0;//电表按照最大功率走过的电量

	bool fInvalid[2];//正、反向有功值是否有效
	uint64 ui64PosE = 0;//当前正向有功
	uint64 ui64NegE = 0;//当前反向有功

	TOobMtrInfo tMtrInfo;
	BYTE btemp = 0, bTotalErcFlag;
	WORD wE0 = 0, wJudgeOadNum = 0;
	DWORD dwTimePast = 0;
	BYTE bCurErcFlag[2];
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	BYTE bAddrBuf[MTR_ADDR_LEN];
#ifdef TERM_EVENT_DEBUG
	const WORD wOI = OI_MTR_FLEW;
#else
	const WORD wOI = OI_MTR_ENERGY_ERR;
#endif

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrFlew* pCtrl = &pMtrTmp->mtrFlew;
	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE* pbBuf = bBuf;

	//memset(bAlrBuf, 0, sizeof(bAlrBuf));
	memset(bAddr, 0, sizeof(bAddr));
	memset(&time, 0, sizeof(time));
	memset(fInit, 0, sizeof(fInit));

	if (!GetMtrExcRdItem(wOI, &dwJudgeOAD[0], &wJudgeOadNum, bRelaOAD))
		return false;

	//if (IsRxTrigerSaveCmd(wPn, wOI, pCtrl->bState, &bCurState))
	//	return ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, bCurState, bRelaOAD);

	if (pMtrRdCtrl->bTsa[0] >= sizeof(pMtrRdCtrl->bTsa))
		return false;

	//获取表地址值并判断是否有效；如果表地址为0，说明未配置表地址，返回；
	if(IsAllAByte(&pMtrRdCtrl->bTsa[1], 0, pMtrRdCtrl->bTsa[0]))	//地址全0
	{
		pCtrl->fInvalid[0] = pCtrl->fInvalid[1] = true;
		return false;
	}
	else
	{		
		memcpy(bAddrBuf, &pMtrRdCtrl->bTsa[0], sizeof(bAddrBuf));
	}	

	if (QueryMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD, wJudgeOadNum))		//数据到达
	{
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD wIndex=0; wIndex<wJudgeOadNum; wIndex++)
		{
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD[wIndex], pbBuf);
			if (iLen <= 0)	//没抄到
				return false;
			
			if (pbBuf[2] == DT_DB_LONG_U)
				iLen = 5;	//取出总电能
			else
				iLen = 9;	//取出总电能

			memmove(pbBuf, pbBuf+2, iLen);
			//DTRACE(DB_METER_EXC, ("EnergyErr::stepDDD :pn=%d, iLen=%d, wIndex=%d bBuf[0]=0x%02x, bBuf[1]=0x%02x, bBuf[2]=0x%02x, bBuf[3]=0x%02x, bBuf[4]=0x%02x.\r\n", wPn, iLen, wIndex, pbBuf[0], pbBuf[1], pbBuf[2], pbBuf[3], pbBuf[4]));
			pbBuf += iLen;
		}

		fInvalid[0] = fInvalid[1] = false;

#ifdef TERM_EVENT_DEBUG
		ui64PosE = GetEnergyValByFmt(bBuf[0], bBuf, &bLen);	//正向有功总
		ui64NegE = GetEnergyValByFmt(bBuf[bLen], bBuf+bLen, &bLen);	//反向有功总
#else
		ui64PosE = GetEnergyValByFmt(bBuf[0], bBuf+1, &bLen);	//正向有功总
		ui64NegE = GetEnergyValByFmt(bBuf[bLen], bBuf+bLen+1, &bLen);	//反向有功总
#endif		
		
		if (memcmp(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1) != 0)		//当第一次抄表或表地址更换后，重新开始判断
		{
			memcpy(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1);
			fInit[0] = true;
			fInit[1] = true;
		}
		else
		{
			if (!GetMeterInfo(wPn, &tMtrInfo))	//读取电表额定电压，额定电流，接线方式（设置参数F25）；
				return false;
	
			wUn = tMtrInfo.wRateVol; //额定电压值
			wIn = tMtrInfo.wRateCurr; //额定电流值
#ifdef TERM_EVENT_DEBUG
			bConnType = tMtrInfo.bLine;
#else
			bConnType = GetConnectType(wPn);
#endif
			if (wUn == 0)
				wUn = 2200;

			if (wIn == 0)
				wIn = 50;

#ifdef TERM_EVENT_DEBUG
			if (bConnType == 2)
#else
			if (bConnType == CONNECT_3P3W)//三相三线
#endif
			{
				wE0 = 500;//0.05*10000
				btemp = 2;//三相三线b相无电流，故算两相功率即可；
			}
			else
			{
				wE0 = 1000;//0.1*10000
				btemp = 3;
			}

			memset(bBuf, 0, sizeof(bBuf));
			iLen = OoReadAttr(wOI, ATTR6, bBuf, NULL, NULL); // 读取配置参数
			if (iLen<=0 || bBuf[2]!=DT_DB_LONG_U)
				dwFlewHold = 800;	//8.0倍数
			else
				dwFlewHold = OoDoubleLongUnsignedToDWord(&bBuf[3]);	//异常判别阈值 double-long-unsigned（单位：%，无换算）
#ifdef TERM_EVENT_DEBUG
			if (dwFlewHold == 0)
				dwFlewHold = 800;	//8.0倍数
#endif


			memset(bCurErcFlag, ERC_STATE_MIDDLE, sizeof(bCurErcFlag));

			for (i=0; i<2; i++)		//判断；
			{
				//此周期或上周期有功电能值无效，则不判断；
				if (fInvalid[i] || pCtrl->fInvalid[i])
				{
					fInit[i] = true;
					continue;
				}

				if (i==0 && ui64PosE>pCtrl->ui64PosE)	//计算上周期和本周期电能差值，如果示度下降或停走，不判断；
				{
					ui64DeltEnergy = ui64PosE - pCtrl->ui64PosE;
				}
				else if (i==1 && ui64NegE>pCtrl->ui64NegE)
				{
					ui64DeltEnergy = ui64NegE - pCtrl->ui64NegE;
				}
				else
				{
					fInit[i] = true;
					continue;
				}

				//DTRACE(DB_METER_EXC, ("EnergyErr::******RunTask:pn=%d, TimeDec=%d\r\n", wPn, GetClick()-pCtrl->dwSeconds[i]));
				//按照最大功率走过的电量
				if((GetClick()-pCtrl->dwSeconds[i]) < ((DWORD)bMtrInterv*60/2))
					continue;

				dwTimePast = (GetClick()-pCtrl->dwSeconds[i]) / ((DWORD) bMtrInterv*60) * ((DWORD)bMtrInterv*60);
				if (dwTimePast == 0)	//不到一个抄表间隔的时间
					dwTimePast = GetClick() - pCtrl->dwSeconds[i];

				ui64PastEnergy = ((uint64) wUn) * wIn * btemp * dwTimePast;	//w（２位小数）

				//当电表的示度走过的电量超过wE0,重新开始判断；
					//（电量单位：w）
				ui64PastEnergy *= dwFlewHold;			//电表按照最大功率走过的电量
				ui64DeltEnergy *= (1000 * 3600 * 100);	//电表示度走过的电量

				if (ui64DeltEnergy >= ui64PastEnergy)	//有功电能值下降且发生标志置0；（防止电表走到满刻度重新走字）
				{
					bCurErcFlag[i] = ERC_STATE_HAPPEN;	//发生
					DTRACE(DB_METER_EXC, ("EnergyErr::###### EnergyFlew event happened! ui64DeltEnergy=%u,  ui64PastEnergy=%u.######\r\n", ui64DeltEnergy, ui64PastEnergy));
				}
				else if (ui64DeltEnergy < ui64PastEnergy)	//有功电能值没有下降且发生标志置1；
				{
					bCurErcFlag[i] = ERC_STATE_RECOVER;	//恢复
					DTRACE(DB_METER_EXC, ("EnergyErr::###### EnergyFlew event recover! ui64DeltEnergy=%u,  ui64PastEnergy=%u.######\r\n", ui64DeltEnergy, ui64PastEnergy));
				}
				else
				{
					bCurErcFlag[i] = ERC_STATE_MIDDLE;
				}

				fInit[i] = true;
			}

			if (bCurErcFlag[0]==ERC_STATE_HAPPEN || bCurErcFlag[1]==ERC_STATE_HAPPEN)	//正有 或 反有发生
				bTotalErcFlag = ERC_STATE_HAPPEN;
			else if (bCurErcFlag[0]==ERC_STATE_RECOVER && bCurErcFlag[1]==ERC_STATE_RECOVER)	//正有和反有都恢复
				bTotalErcFlag = ERC_STATE_RECOVER;
			else
				bTotalErcFlag = ERC_STATE_MIDDLE;

			UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bTotalErcFlag, &pCtrl->bState);
		}

		//初始化
		for (i=0; i<2; i++)
		{
			pCtrl->fInvalid[i] = fInvalid[i];
			if (!fInvalid[i] && fInit[i])
			{
				pCtrl->dwSeconds[i] = GetClick();
				if (i == 0)
					pCtrl->ui64PosE = ui64PosE;
				else
					pCtrl->ui64NegE = ui64NegE;
			}
		}
	}

	return true;
}


void InitMtrStop(BYTE bPn, TMtrStop* pCtrl)
{
	pCtrl->ui64PosE = 0;
	pCtrl->ui64NegE = 0;
	pCtrl->bState = EVT_S_BF_HP;
	pCtrl->dwSeconds = 0;
	pCtrl->fInvalid = true;
	memset(pCtrl->bAddr, 0, sizeof(pCtrl->bAddr));
}


//描述：电表停走事件的判断：用当前电表功率计算电量增量△，当△大于设定值（默认值为０.1kWh）而电表电量读数仍不发生变化，则认为电表停走。当电表电量读数发生变化，则认为电表停走事件恢复，同时将△清零。对于正向有功和反向有功，一起判断。
bool DoMtrStop(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{
	BYTE bStopHold = 0;//停走阀值；
	BYTE bBuf[100];
	BYTE bAlrBuf[20];
	BYTE bAddr[6];
	WORD wValidNum = 0, wIdNum = 0, wJudgeOadNum = 0;
	DWORD dwCurMin;
	BYTE bMtrInterv, bLen, bOffset, bCurErcFlag, bCurState;

	TTime time;	
	bool fErc;
	bool fInit;
	int iLen;
	uint64 ui64PosE;
	uint64 ui64NegE;
	DWORD dwPower;
	DWORD dwDeltSeconds, dwStopHold ;
	uint64 ui64PastEnergy;
	TTimeInterv tiExe;
	BYTE bAddrBuf[MTR_ADDR_LEN];
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE* pbBuf = bBuf;
	const WORD wOI = OI_MTR_STOP;
	const DWORD dwE0 = 3600 * 100;//按照实际功率运行时电能量超过的阀值（0.1kwh)；

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrStop* pCtrl = &pMtrTmp->mtrStop;

	fErc = false;
	fInit = false;
	memset(bAlrBuf, 0, sizeof(bAlrBuf));
	memset(bAddr, 0, sizeof(bAddr));
	memset(&time, 0, sizeof(time));

	if (!GetMtrExcRdItem(wOI, &dwJudgeOAD[0], &wJudgeOadNum, bRelaOAD))
		return false;

	//if (IsRxTrigerSaveCmd(wPn, wOI, pCtrl->bState, &bCurState))
	//	return ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, bCurState, bRelaOAD);

	if (pMtrRdCtrl->bTsa[0] >= sizeof(pMtrRdCtrl->bTsa))
		return false;

	//获取表地址值并判断是否有效；如果表地址为0，说明未配置表地址，返回；
	if(IsAllAByte(&pMtrRdCtrl->bTsa[1], 0, pMtrRdCtrl->bTsa[0]))	//地址全0
	{
		pCtrl->fInvalid = true;
		return false;
	}
	else
	{		
		memcpy(bAddrBuf, &pMtrRdCtrl->bTsa[0], sizeof(bAddrBuf));
	}	

	if (QueryMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD, wJudgeOadNum))		//数据到达
	{
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD wIndex=0; wIndex<wJudgeOadNum; wIndex++)
		{
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD[wIndex], pbBuf);
			if (iLen <= 0)	//没抄到
				return false;
			
			pbBuf += iLen;
		}
		
		bOffset = 0;
		ui64PosE = GetEnergyValByFmt(bBuf[bOffset], bBuf+bOffset+1, &bLen);	//正向有功总
		bOffset += bLen;

		ui64NegE = GetEnergyValByFmt(bBuf[bOffset], bBuf+bOffset+1, &bLen);	//反向有功总
		bOffset += bLen;

		dwPower = ABS(OoDoubleLongToInt(bBuf+bOffset+1));	//实际有功功率

		if (memcmp(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1) != 0)		//当第一次抄表或表地址更换后，重新开始判断
		{
			memcpy(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1);
			fInit = true;
		}
		else if (pCtrl->fInvalid || ui64NegE<pCtrl->ui64NegE || ui64PosE<pCtrl->ui64PosE || dwPower==0)
		{//上周期值无效，或本周期正向（反向）有功电能值小于上周期正向（反向）有功电能值，或电表未加载功率，则本周期不判断；
			fInit = true;
		}
		else
		{			
			//获取电表停走阀值；
			memset(bBuf, 0, sizeof(bBuf));
			iLen = OoReadAttr(wOI, ATTR6, bBuf, NULL, NULL); // 读取配置参数
			if (iLen<=0 || bBuf[2]!=DT_TI)
			{
				tiExe.bUnit = 1;
				tiExe.wVal = 15;
			}
			else
			{
				tiExe.bUnit = bBuf[3];
				tiExe.wVal = OoLongUnsignedToWord(&bBuf[4]);	//异常判别阈值
			}

			dwStopHold = TiToSecondes(&tiExe);	//停走阀值
			if (dwStopHold == 0)
				dwStopHold = 15 * 60;

			bCurErcFlag = ERC_STATE_MIDDLE;
			if (ui64PosE==pCtrl->ui64PosE && ui64NegE==pCtrl->ui64NegE)	//判断电表停走；
			{
				if (pCtrl->dwSeconds == 0)
				{
#ifdef TERM_EVENT_DEBUG
					pCtrl->dwSeconds =  GetCurTime();
#else
					pCtrl->dwSeconds = GetClick();//停走发生的开始计时时间；
#endif
				}
				else
				{
#ifdef TERM_EVENT_DEBUG
					if (GetCurTime() < pCtrl->dwSeconds)
					{
						fInit = true;
						dwDeltSeconds = 0;
					}
					else
					{
						dwDeltSeconds = GetCurTime() - pCtrl->dwSeconds;
					}
#else
					dwDeltSeconds = GetClick() - pCtrl->dwSeconds;
#endif
					ui64PastEnergy = ((uint64)dwPower) * dwDeltSeconds; //(单位w,４位小数）

#ifdef TERM_EVENT_DEBUG
					if (dwDeltSeconds>=dwStopHold)
					{
						DTRACE(DB_METER_EXC, ("CMtrStop::RunTask: ******pn=%d, dwDeltSeconds=%d, ui64PastEnergy=%lld\r\n", wPn, dwDeltSeconds, ui64PastEnergy));
						if (ui64PastEnergy >= dwE0/10)	//OOP电能为2位小数
						{
							bCurErcFlag = ERC_STATE_HAPPEN;	//发生
							DTRACE(DB_METER_EXC, ("CMtrStop::###### CMtrStop event happened!!! ######\r\n"));
						}
						
						fInit = true;
					}
#else
					DTRACE(DB_METER_EXC, ("CMtrStop::RunTask: ******pn=%d, dwDeltSeconds=%d, ui64PastEnergy=%lld\r\n", wPn, dwDeltSeconds, ui64PastEnergy));
					if (dwDeltSeconds>=dwStopHold && ui64PastEnergy>=dwE0/10)
						bCurErcFlag = ERC_STATE_HAPPEN;	//发生

					fInit = true;
#endif
				}
			}
			else	//本周期电能值大于上周期电能值，且异常标志为1，则电表停走恢复。
			{
				bCurErcFlag = ERC_STATE_RECOVER; //恢复
				fInit = true;
			}
			
			UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bCurErcFlag, &pCtrl->bState);
		}

		pCtrl->ui64PosE = ui64PosE;
		pCtrl->ui64NegE = ui64NegE;
		pCtrl->fInvalid = false;
		if (fInit)
			pCtrl->dwSeconds = 0;
	}

	return fErc;
}

//初始化抄表失败事件
void InitMtrRdFail(WORD wPn, TMtrRdFail* pCtrl)
{
	pCtrl->bState = EVT_S_BF_HP;
}


//抄表失败事件
bool DoMtrRdFail(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{	
	bool fRet;
	DWORD dwMtrSecs, dwCurSecs, dwDiff;
	int iLen;	
	TTime time;
	TTime tmMtrTime;
	BYTE bCurState, bLen;
	WORD wChecktmHold, wRecoverHold, wJudgeOadNum = 0;
	BYTE bAlrBuf = 0, bCurErcFlag = 0;
	BYTE bBuf[40];
	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE* pbBuf = bBuf;
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
#ifdef TERM_EVENT_DEBUG
	const WORD wOI = OI_MTR_RD_FAIL;
#else
	const WORD wOI = OI_MTR_CLOCK_ERR;
#endif

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrRdFail* pCtrl = &pMtrTmp->mtrRdFail;

	//DTRACE(DB_METER_EXC, ("DoMtrRdFail::###### DoMtrRdFail step 0!!! ######\r\n"));
	memset(&time, 0, sizeof(time));
	memset(&tmMtrTime, 0, sizeof(tmMtrTime));
	if (!GetMtrExcRdItem(wOI, &dwJudgeOAD[0], &wJudgeOadNum, bRelaOAD))
		return false;

	//if (IsRxTrigerSaveCmd(wPn, wOI, pCtrl->bState, &bCurState))
	//	return ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, bCurState, bRelaOAD);
					
	//DTRACE(DB_METER_EXC, ("DoMtrRdFail::RunTask:  pCtrl->bState=%d at click = %ld.\r\n", pCtrl->bState, GetClick()));	

	if (IsMtrErr(wPn))	//发生状态
	{
		bCurErcFlag = ERC_STATE_HAPPEN;
		DTRACE(DB_METER_EXC, ("DoMtrRdFail::###### DoMtrRdFail event happened!!! ######\r\n"));
	}
	else	//结束状态
		bCurErcFlag = ERC_STATE_RECOVER;

	UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bCurErcFlag, &pCtrl->bState);		//抄读关联属性表数据，保存到临时记录区

	return true;
}



//========================电能表数据变更监控记录==========================
//描述：电能表数据变更监控记录，获取任务OAD
/*int GetMtrDataChgCSD(BYTE* bCfg)
{
	int iLen;
	TTaskCfg tTaskCfg;
	TCommAcqSchCfg tCommAcqSchCfg;
	BYTE bIndex, bBuf[MTEDATACHG_CSD_LEN];
	BYTE* pbCSDCfg = bBuf;

	if (bCfg == NULL)
		return -1;

	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(OI_MTR_DATA_CHG, ATTR6, bBuf, NULL, NULL);	// 读取配置参数
	if (iLen > 0 && bBuf[0] == DT_STRUCT && bBuf[1] == 0x01  && bBuf[2] == DT_UNSIGN)
		bIndex = bBuf[3];
	else
		return -1;

	if (!GetTaskCfg(bIndex, (TTaskCfg*)&tTaskCfg))
		return -1;
	
	if (!GetCommonSchCfg((TTaskCfg*)&tTaskCfg, (TCommAcqSchCfg*)&tCommAcqSchCfg))
		return -1;

	iLen = 0;
	memset(bBuf, 0, sizeof(bBuf));
	
	//获取的是任务的第一个CSD
	if ((tCommAcqSchCfg.bCSDNum > 0))
	{	
		pbCSDCfg++;		//第一个字节为CSD的长度
		*pbCSDCfg = DT_CSD;pbCSDCfg++;iLen++;
		*pbCSDCfg = tCommAcqSchCfg.tTCSD[0].bChoice;pbCSDCfg++;iLen++;
		if (tCommAcqSchCfg.tTCSD[0].bChoice == 0)	
		{	
			OoDWordToOad(tCommAcqSchCfg.tTCSD[0].dwOAD, pbCSDCfg);
			iLen +=4;
			bBuf[0] = iLen;
			memcpy(bCfg, bBuf, iLen+1);
		}
		//else
		//{	
		//	memcpy(&bBuf[2], tCommAcqSchCfg.tTCSD.tTROAD, 4);		//后续问一下陈工怎么获取ROAD格式，暂时只支持OAD。。。。。。
		//	return true;
		//}
	}

	return iLen;
}*/

//描述：电能表数据变更监控记录，获取任务OAD
int GetMtrDataChgCSD(BYTE* bCfg)
{
	TTaskCfg tTaskCfg;
	int iLen, iSchCfgLen, iArryIdx;
	WORD wFmtLen, wCfgLen, wDataFmtLen;
	BYTE bIndex, bType, bBuf[10] = {0};
	BYTE *pbSch, *pbFmt, *pbArryCsd, *pbDataFmt;
	BYTE *pbCfg0 = bCfg;

	if (bCfg == NULL)
		return -1;

	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(OI_MTR_DATA_CHG, ATTR6, bBuf, NULL, NULL);	// 读取配置参数
	if (iLen > 0 && bBuf[0] == DT_STRUCT && bBuf[1] == 0x01  && bBuf[2] == DT_UNSIGN)
		bIndex = bBuf[3];
	else
		return -1;

	if (!GetTaskCfg(bIndex, (TTaskCfg*)&tTaskCfg))
		return -1;

	if (tTaskCfg.bSchType != SCH_TYPE_COMM)	//暂时只考虑普通采集方案
		return -1;

	pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
	if (pbSch==NULL)
		return -1;

	pbFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
	if(pbFmt == NULL)
		return -1;

	iArryIdx = 3;
	pbArryCsd = OoGetField(pbSch, pbFmt, wFmtLen, iArryIdx, &wCfgLen, &bType, &pbDataFmt, &wDataFmtLen);
	if (pbArryCsd == NULL)
		return -1;

	if (pbArryCsd[0]==DT_ARRAY && pbArryCsd[1]!=0)
	{
		iLen = ScanCSD(&pbArryCsd[3], false);	//跳过DT_CSD格式
		if (iLen > 0)
		{
			//memcpy(pbSch, &pbArryCsd[2], iLen+1);	//+1：加上DT_CSD格式
			iLen++;		//+1：加上DT_CSD格式
			*bCfg++ = iLen;
			memcpy(bCfg, &pbArryCsd[2], iLen);
			bCfg += iLen;
			bCfg = pbCfg0;
			return iLen;

		}
	}

	return -1;
}

//描述：电能表数据变更监控记录的初始化。
void InitMtrDataChg(WORD wPn, TMtrDataChg* pCtrl)
{
	pCtrl->bState = EVT_S_BF_HP;
	memset(pCtrl->bAddr, 0, sizeof(pCtrl->bAddr));
	memset(pCtrl->bOldCSD, 0, sizeof(pCtrl->bCSD));
	memset(pCtrl->bCSD, 0, sizeof(pCtrl->bCSD));
	memset(pCtrl->bOldData, 0, sizeof(pCtrl->bOldData));
	memset(pCtrl->bNewData, 0, sizeof(pCtrl->bNewData));
}


int GetCsdRec(TMtrRdCtrl* pMtrRdCtrl, BYTE *pbCsd, WORD wRcsdLen, BYTE *pbRec, WORD wMaxRecLen)
{
	TOobMtrInfo tMtrInfo;
	int iRet, iTabIdx, iStart;
	WORD wPn, wRetNum;
	BYTE bOAD[4];
	BYTE bRsd[32];
	BYTE bRcsd[256];
	BYTE *pRsd, *pRcsd;
	
	//OAD
	bOAD[0] = 0x60;
	bOAD[1] = 0x12;
	bOAD[2] = 0x03;
	bOAD[3] = 0x00;

	//RSD
	pRsd = bRsd;
	*pRsd++ = 0x0A;	//方法10
	*pRsd++ = 0x01;	//上一笔记录
	GetMeterInfo(&pMtrRdCtrl->bTsa[1], pMtrRdCtrl->bTsa[0], &tMtrInfo);
	*pRsd++ = 0x04;	//MS一组配置序号
	*pRsd++ = 0x01;	//1个表序号
	pRsd += OoWordToOi(tMtrInfo.wMtrSn, pRsd);

	//RCSD
	pRcsd = bRcsd;
	*pRcsd++ = 0x01;	//RCSD个数
	memcpy(pRcsd, pbCsd+1, wRcsdLen-1);	//+1：去掉DT_CSD
	pRcsd += (wRcsdLen-1);

	iTabIdx = 0;
	iStart = -1;
	iRet = ReadRecord(bOAD, bRsd, bRcsd, &iTabIdx, &iStart, pbRec, wMaxRecLen, &wRetNum);

	return iRet;
}

//描述：电能表数据变更监控记录的判断：如果有变，则认为产生一条记录
bool DoMtrDataChg(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{	
	static DWORD dwClick = 0;
	BYTE bCurErcFlag;
	bool fIsCSDChg = false;
	int iLen = 0;	
	WORD wJudgeOadNum = 0;
	BYTE bBuf[512];
	DWORD dwJudgeOAD[MAX_JUDGE_OAD], dwOAD;
	BYTE* pbBuf = bBuf;
	BYTE bRelaOAD[EVT_ATTRTAB_LEN] = {0};
	const WORD wOI = OI_MTR_DATA_CHG;

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrClockErr* pCtrl = &pMtrTmp->mtrClockErr;

	BYTE bCSDCfg[MTEDATACHG_CSD_LEN];

	iLen = OoReadAttr(wOI, ATTR3, bRelaOAD, NULL, NULL);		//读关联属性表
	if (iLen <= 0)
		return false;

	memset(bCSDCfg, 0, sizeof(bCSDCfg));	//bCSDCfg的第一个字节为长度
	iLen = GetMtrDataChgCSD(bCSDCfg);
	if (iLen <= 0)
		return false;

	if (memcmp(pMtrTmp->mtrDataChg.bCSD, bCSDCfg, iLen) != 0)	//配置参数有变
	{
		memcpy(pMtrTmp->mtrDataChg.bOldCSD, pMtrTmp->mtrDataChg.bCSD, MTEDATACHG_CSD_LEN);
		memset(pMtrTmp->mtrDataChg.bCSD, 0, sizeof(MTEDATACHG_CSD_LEN));
		memcpy(pMtrTmp->mtrDataChg.bCSD, bCSDCfg, iLen);	
		fIsCSDChg = true;
	}

	//获取数据后续需要陈工提供接口，暂时先支持OAD过台子。。。。。。。。。
	/*if (bCSDCfg[1] != DT_CSD)
		return false;
	
	if (bCSDCfg[2] != 0)
		return false;
	
	wJudgeOadNum = 1;
	memcpy((BYTE*)&dwOAD, &bCSDCfg[3], 4);
	OoDWordToOad(dwOAD, (BYTE*)&dwJudgeOAD);

	if (QueryMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD, wJudgeOadNum))		//数据到达
	{
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD wIndex=0; wIndex<wJudgeOadNum; wIndex++)
		{
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD[wIndex], pbBuf);
			if (iLen <= 0)	//没抄到
				return false;

			pbBuf += iLen;
			fIsNeedCmp = true;	//获取长iLen长度的数据
		}
	}
	//从上部分由陈工提供，获取数据到pbBuf，计算好数据的长度iLen。
	

	if (fIsNeedCmp)		//数据到达*/
	
	if (bCSDCfg[1] != DT_CSD)
		return false;
	
	//if (bCSDCfg[1] != 0)
	//	return false;

	if  (GetClick()-dwClick<60) 	// 1分钟抄一次数据
		return false;

	dwClick = GetClick();
	
	memset(bBuf, 0, sizeof(bBuf));
	iLen = GetCsdRec(pMtrRdCtrl, bCSDCfg+1, iLen, bBuf, sizeof(bBuf));	//备注：这里要控制访问任务库的次数，如10s访问一次!!! add CL
	if (iLen > 0)		//数据到达
	{
		if ((IsAllAByte(pMtrTmp->mtrDataChg.bOldData, 0, sizeof(pMtrTmp->mtrDataChg.bOldData))) || fIsCSDChg)	
		{
			pMtrTmp->mtrDataChg.bOldData[0] = iLen;
			memcpy(pMtrTmp->mtrDataChg.bOldData+1, bBuf, iLen);
		}

		if ((IsAllAByte(pMtrTmp->mtrDataChg.bNewData, 0, sizeof(pMtrTmp->mtrDataChg.bNewData))) || fIsCSDChg)	
		{	
			pMtrTmp->mtrDataChg.bNewData[0] = iLen;
			memcpy(pMtrTmp->mtrDataChg.bNewData+1, bBuf, iLen);
		}

		if (memcmp(pMtrTmp->mtrDataChg.bNewData+1, bBuf, iLen) != 0)
		{
			memcpy(pMtrTmp->mtrDataChg.bOldData, pMtrTmp->mtrDataChg.bNewData, pMtrTmp->mtrDataChg.bNewData[0]+1);
			pMtrTmp->mtrDataChg.bNewData[0] = iLen;
			memcpy(pMtrTmp->mtrDataChg.bNewData+1, bBuf, iLen);
			bCurErcFlag = ERC_STATE_HAPPEN;
		}
		else
			bCurErcFlag = ERC_STATE_RECOVER;

		UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bCurErcFlag, &pCtrl->bState);

		return true;
	}

	return false;
}

//**************************事件方法操作*********************************************
//方法1：复位
int DoMtrExcMethod1(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bClrFlag;
	int iPn;

	if (bMethod!=EVT_RESET || iParaLen!=0x02)
		goto END_ERR;

	iPn = GetMtrExcIndex(wOI);
	if (iPn < 0)
		goto END_ERR;

	if (pbPara[0]==DT_INT && pbPara[1]==0x00)
	{
		bClrFlag = EVT_CLR_VALID;
		WriteItemEx(BN11, iPn, MTREXC_CLR_ID, &bClrFlag);
		TrigerSaveBank(BN11, 0, -1); //触发保存
		SetInfo(INFO_MTR_EXC_RESET);

		*pbRes = 0;	//成功  （0）
		return 0;
	}

END_ERR:
	*pbRes = 3;	
	return -1;
}

//方法2：执行
//空函数
int DoMtrExcMethod2(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	int iPn;	

	if (bMethod != EVT_RUN)	
		goto END_ERR;
	
	iPn = GetMtrExcIndex(wOI);
	if (iPn < 0)
		goto END_ERR;

	//nothing to do

	*pbRes = 0;	//成功  （0） 返回结果
	return 0;

END_ERR:
	*pbRes = 3;	
	return -1;
}



//方法3：触发一次记录
//描述：保存触发冻结命令
//参数：事件发生源、 延迟触发时间、 延迟恢复时间
int DoMtrExcMethod3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	int iOffset;
	BYTE bTsaLen;
	WORD wPn, wID;
	DWORD dwCurSec = 0;
	BYTE bBuf[30];
	BYTE* p = bBuf;
	const WORD wBaseID = 0x0b20;

	if (bMethod != EVT_TRIG)	
		goto END_ERR;

	iOffset = GetMtrExcIndex(wOI);
	if (iOffset < 0)
		goto END_ERR;

	wID = wBaseID + iOffset;
#ifdef MTREXC_ADDR_TPYE_TSA
	if (*pbPara != DT_TSA)
		goto END_ERR;
#else
	if (*pbPara != DT_OCT_STR)
		goto END_ERR;
#endif

	bTsaLen = pbPara[2] & 0x0f;		//TSA_OCTSTR_TYPE
	bTsaLen++;

	wPn = MtrAddrToPn(&pbPara[3], bTsaLen);
	if (wPn == PN0)
		goto END_ERR;

	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN11, wPn, wID, bBuf);
	if (bBuf[8] != 0)	//状态机不为0，表示正在触发冻结中,暂不接收新命令
		goto END_ERR;

	p = bBuf;
	memset(bBuf, 0, sizeof(bBuf));
	memcpy(p, pbPara+bTsaLen+4, 2);		//延迟发生时间
	p += 2;

	memcpy(p, pbPara+bTsaLen+7, 2);		//延迟恢复时间
	p += 2;

	dwCurSec = GetCurTime();
	memcpy(p, &dwCurSec, sizeof(DWORD));
	p += 4;
	
	WriteItemEx(BN11, wPn, wID, bBuf);
 
	*pbRes = 0;	//成功  （0） 返回结果
	return 0;

END_ERR:
	*pbRes = 3;	
	return -1;	
}

//方法4：添加一个关联对象属性（参数）
//参数∷=OAD 对象属性描述符
int DoMtrExcMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	int iLen, iPn;
	BYTE bClrFlag;
	BYTE bOADNum, bIndex;
	BYTE bBuf[EVT_ATTRTAB_LEN];	//关联属性表缓冲区

	if ((bMethod!=EVT_ADDATTR)	|| (iParaLen!=5) || (pbPara[0]!=DT_OAD))
		goto END_ERR;

	iPn = GetMtrExcIndex(wOI);
	if (iPn < 0)
		goto END_ERR;

	//处理添加OAD
	iLen = OoReadAttr(wOI, ATTR3, bBuf, NULL, NULL);	//属性3 关联对象属性表
	if (iLen<=0 || bBuf[0]!=DT_ARRAY || bBuf[1]==0x00)	//读数据有问题或读出为空，清零后添加OAD
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = DT_ARRAY;
		bBuf[1] = 1;
		memcpy(bBuf, pbPara, 5);
		goto END_OK;
	}

	bOADNum = bBuf[1];
	if (bOADNum >= CAP_OAD_NUM)	//已存满
		goto END_ERR;

	for (bIndex=0; bIndex<bOADNum; bIndex++)
	{
		if (memcmp(&bBuf[2+bIndex*5], pbPara, 5) == 0)	//已存在OAD
			goto END_ERR;
	}

	if (bIndex == bOADNum)	//没有相同的，新增加一个OAD
	{
		bBuf[1]++;
		memcpy(&bBuf[2+bIndex*5], pbPara, 5);	//新增加的放在最后
		goto END_OK;
	}

END_ERR:
	*pbRes = 3;	
	return -1;
	
END_OK:
	if (OoWriteAttr(wOI, ATTR3, bBuf) <= 0)		//属性3 关联对象属性表
		goto END_ERR;

	bClrFlag = EVT_CLR_VALID;
	WriteItemEx(BN11, iPn, MTREXC_CLR_ID, &bClrFlag);		//写清零标识
	TrigerSaveBank(BN11, 0, -1);
	SetInfo(INFO_MTR_EXC_RESET);

	*pbRes = 0;	//成功  （0） 返回结果
	return 0;
}

//方法5：删除一个对象属性（参数）
//参数∷=OAD 对象属性描述符
int DoMtrExcMethod5(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bBuf[EVT_ATTRTAB_LEN];	//关联属性表缓冲区
	BYTE bOADNum, bIndex;
	BYTE bClrFlag;
	int iLen, iPn, j = 0;

	if (bMethod!=EVT_DELATTR || iParaLen!=5 || pbPara[0]!=DT_OAD)
		goto END_ERR;
	
	iPn = GetMtrExcIndex(wOI);
	if (iPn < 0)
		goto END_ERR;
	
	iLen = OoReadAttr(wOI, ATTR3, bBuf, NULL, NULL);	//属性3 关联对象属性表
	if ((iLen<=0) || (bBuf[1]!=DT_ARRAY) || (bBuf[1]==0x00))	//读数据有问题或读出为空，无法删除
		goto END_ERR;

	bOADNum = (bBuf[1]>CAP_OAD_NUM) ? CAP_OAD_NUM : bBuf[1];
	for (bIndex=0; bIndex<bOADNum; bIndex++)	//遍历是否已经存在
	{
		if (memcmp(&bBuf[2+bIndex*5], pbPara, 5) == 0)	//已存在OAD
		{
			memset(&bBuf[bIndex*5 + 2], 0, 5);
			bBuf[1]--;	//数组元素个数
			break;
		}
	}
	
	if (bIndex == bOADNum)	//没找到
		goto END_ERR;
	
	for (j=bIndex; j<bOADNum-1; j++)		//后面的参数往前挪
	{
		memcpy(&bBuf[j*5 + 2], &bBuf[(j+1)*5 + 2], 5);
	}
	
	// 刷新关联属性表
	if (OoWriteAttr(wOI, ATTR3, bBuf) <= 0)
		goto END_ERR;

	bClrFlag = EVT_CLR_VALID;
	WriteItemEx(BN11, iPn, MTREXC_CLR_ID, &bClrFlag);		//写清零标识
	TrigerSaveBank(BN11, 0, -1);
	SetInfo(INFO_MTR_EXC_RESET);

	*pbRes = 0;	//成功  （0） 返回结果
	return 0;

END_ERR:
	*pbRes = 3;
	return -1;
}


//描述：各告警事件控制结构的初始化，
//		在上电后或者参数重新配置后调用本函数，不用每次都调用
void InitMtrExcCtrl(BYTE bPn, TMtrExcTmp* pCtrl)
{
	InitMtrClockErr(bPn, &pCtrl->mtrClockErr);	//ERC_MTRTIME:

	InitEnergyErr(bPn, &pCtrl->mtrEnergyErr);	//ERC_MTRERR:

	InitEnergyDec(bPn, &pCtrl->mtrEnergyDec); //电能表示度下降
	  
	InitMtrStop(bPn, &pCtrl->mtrStop); //电能表停走

	InitMtrFlew(bPn, &pCtrl->mtrFlew);//飞走事件

	InitMtrRdFail(bPn, &pCtrl->mtrRdFail);	//电能表抄表失败事件

	InitMtrDataChg(bPn, &pCtrl->mtrDataChg);	//电能表数据变更监控记录

	memset(pCtrl->wLastRecPhyIdx, 0, sizeof(pCtrl->wLastRecPhyIdx));
	memset(pCtrl->dwLastStatClick, 0, sizeof(pCtrl->dwLastStatClick));
	memset(pCtrl->bTryReadCnt, 0, sizeof(pCtrl->bTryReadCnt));
}

//分配抄表事件临时内存空间
bool AllocateMtrExcMem(BYTE* pbGlobal, TAllocTab* pAllocTab, WORD wTabNum)
{
	DWORD dwOAD = 0;
	int iLen, nOADLen = 0;
	WORD i, wOI, wRecLen;
	BYTE bIndex, bCapNum = 0;
	bool fAllocateSuccess = true;	//是否分配成功
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	BYTE* pbRelaOAD = bRelaOAD;

	for (bIndex=0; bIndex<MTR_EXC_NUM; bIndex++)	//事件类型序号
	{
		wRecLen = 0;
		wOI = GetMtrExcOI(bIndex);	//获取事件OI
		if (wOI == 0)
		{
			fAllocateSuccess = false;
			continue;
		}

		memset(bRelaOAD, 0, sizeof(bRelaOAD));
		iLen = OoReadAttr(wOI, ATTR3, pbRelaOAD, NULL, NULL);	//属性3 关联属性表
		bCapNum = (pbRelaOAD[1]>CAP_OAD_NUM) ? CAP_OAD_NUM : pbRelaOAD[1];		
		if (iLen>0 && pbRelaOAD[0]==DT_ARRAY && bCapNum>0)
		{
			for (i=0; i<bCapNum; i++)	//关联属性表元素个数
			{
				dwOAD = OoOadToDWord(&pbRelaOAD[5*i+3]);
				nOADLen = OoGetDataLen(DT_OAD, &pbRelaOAD[5*i+3]);
				if (nOADLen <= 0)
				{
					wRecLen = 0;
					break;
				}

				wRecLen += nOADLen;
			}

			if (wRecLen > 0)
			{
				if (!AllocMem(pbGlobal, pAllocTab, wTabNum, MEM_TYPE_MTREXC, wOI, wRecLen))
					fAllocateSuccess = false;
			}
		}
	}

	if (!fAllocateSuccess)
		DTRACE(DB_METER_EXC, ("AllocateMtrExcMem::wOI = %04x, allocate fail! \r\n", wOI));

	return fAllocateSuccess;
}




//描述：正常运行时的数据清零处理
//参数：@pEvtCtrl事件控制
//返回：无
//注：以下情况会引起一条事件记录的清零
// 1. 关联属性表变更---设置参数/添加删除OAD方法
// 2. 最大记录数据变更
// 3. 清零操作---复位方法/电表清零/事件总清零/需量清零
void ClrMtrExc(int nIndex)
{
	int iLen;
	WORD wPn, wOI;
	WORD wCurRecNum;
	BYTE bClrFlag = 0;
	const ToaMap* pOaMap;
	DWORD dwOAD, dwTimes, dwSec;
	BYTE bBuf[EVT_ATRR_MAXLEN];
	
	if (nIndex < 0)
		return;

	wOI = GetMtrExcOI(nIndex);
	if (wOI == 0)
		return;

	wPn = nIndex;
	if (ReadItemEx(BN11, wPn, MTREXC_CLR_ID, &bClrFlag) <= 0)
		return;

	if (bClrFlag != MTREXC_CLR_VALID)
	{
		bClrFlag = 0;
		WriteItemEx(BN11, wPn, MTREXC_CLR_ID, &bClrFlag);	//清除完所有的数据再将标识清零
		return;	//不需要清零
	}	

	dwOAD = GetOAD(wOI, ATTR2, 0);
	pOaMap = GetOIMap(dwOAD);
	if (pOaMap != NULL)
	{
		TdbDeleteTable(pOaMap->pszTableName);	//删除记录表
		ClrMtrExcTableFlag(wOI);	//清除建表标志
	}

	//清0统计数据
	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, ATTR4, bBuf, NULL, NULL);	//当前记录数累加
	if (iLen > 0)
	{
		wCurRecNum = 0;
		OoWordToLongUnsigned(wCurRecNum, bBuf+1);
		OoWriteAttr(wOI, ATTR4, bBuf);
	}

	iLen =OoReadAttr(wOI, ATTR7, bBuf, NULL, NULL);		//当前值记录表累计次数累加
	if (iLen > 0)
	{
		dwTimes = 0;
		OoDWordToDoubleLongUnsigned(dwTimes, bBuf+25);
		OoWriteAttr(wOI, ATTR7, bBuf);

		dwSec = 0;
		OoDWordToDoubleLongUnsigned(dwSec, bBuf+30);
		OoWriteAttr(wOI, ATTR7, bBuf);
	}

	//将清零标识清除
	bClrFlag = 0;
	WriteItemEx(BN11, wPn, MTREXC_CLR_ID, &bClrFlag);	//清除完所有的数据再将标识清零
	TrigerSaveBank(BN11, 0, -1);
}


//清零事件相关数据
void ClrMtrExc()
{
	WORD i;

	for (i=0; i<MTR_EXC_NUM; i++)
	{	
		ClrMtrExc(i);	//处理清零命令
	}
}



//描述:	处理事件系统库中DYN属性数据，包括记录数属性、当前值记录表属性
bool UpdateMtrExcStatData(WORD wOI, BYTE bState, TMtrExcTmp* pExcTmp, BYTE* pbTsa)
{
	BYTE bOffset  = 0;
	int iLen, nIndex;
	TTime tmCurRec;
	bool fOnMtrExcHap, fOnMtrExcEnd, fTrigerSave = false;
	WORD wMaxNum = 0, wCurRecNum = 0;
	DWORD dwTimes, dwClick, dwSec;
	BYTE bBuf[EVT_ATRR_MAXLEN];

	nIndex = GetMtrExcIndex(wOI);
	if (nIndex < 0)
		return false;

	if (OoReadAttr(wOI, ATTR5, bBuf, NULL, NULL) > 0)	//最大记录数
		wMaxNum = OoLongUnsignedToWord(bBuf+1);

	if (wMaxNum == 0)
		return false;

	iLen = OoReadAttr(wOI, ATTR4, bBuf, NULL, NULL);	//当前记录数累加，先取初值
	if (iLen <= 0)
		return false;

	fOnMtrExcHap = (bState==EVT_S_AFT_HP && pExcTmp->dwLastStatClick[nIndex]==0);	//事件刚发生
	fOnMtrExcEnd = (bState==EVT_S_AFT_END && pExcTmp->dwLastStatClick[nIndex]!=0);	//事件刚结束

	dwClick = GetClick();
	wCurRecNum = OoLongUnsignedToWord(bBuf+1);	//事件发生次数
	if (wCurRecNum<wMaxNum && fOnMtrExcHap)	//当前记录数加1，直到记录数等于最大记录数
	{
		wCurRecNum++;
		OoWordToLongUnsigned(wCurRecNum, bBuf+1);
		OoWriteAttr(wOI, ATTR4, bBuf);
	}

	iLen =OoReadAttr(wOI, ATTR7, bBuf, NULL, NULL);		//当前值记录表累计次数累加，先取初值
	if (iLen <= 0)
		return false;

	bOffset = 0;
	bOffset += 4;

	bBuf[bOffset] = DT_TSA;
	bOffset++;	

	if (pbTsa[0]<=16 && pbTsa[0]>0)	//电表实际长度
	{
		bBuf[bOffset] = pbTsa[0] + 1;	//length
		bOffset++;

		bBuf[bOffset] = pbTsa[0] - 1;	//length
		bOffset++;

		memcpy(bBuf+bOffset, &pbTsa[1], pbTsa[0]);	//拷电表实际长度
		bOffset += pbTsa[0];
	}
	else
	{
		bBuf[bOffset] = DT_NULL;
		bOffset++;
	}

	bBuf[bOffset] = DT_STRUCT;
	bOffset++;

	bBuf[bOffset] = 0x02;
	bOffset++;

	if (fOnMtrExcHap)
	{
		
		bBuf[bOffset] = DT_DB_LONG_U;
		bOffset++;
		dwTimes = OoDoubleLongUnsignedToDWord(bBuf+bOffset);
		dwTimes++;
		OoDWordToDoubleLongUnsigned(dwTimes, bBuf+bOffset);
		bOffset += 4;

		bBuf[bOffset] = DT_DB_LONG_U;
		bOffset++;

		memset(&bBuf[bOffset], 0, 4);
		bOffset += 4;

		OoWriteAttr(wOI, ATTR7, bBuf);

		pExcTmp->dwLastStatClick[nIndex] = dwClick;		//到这里才能更新事件发生的统计时标 ---是否需要触发保存统计数据？
		fTrigerSave = true;
	}
	else if (bState==EVT_S_AFT_HP || bState==EVT_S_BF_END)		//事件发生期间的累计时间
	{
		bOffset += 5;

		bBuf[bOffset] = DT_DB_LONG_U;
		bOffset++;

		dwSec = OoDoubleLongUnsignedToDWord(bBuf+bOffset);
		if (pExcTmp->dwLastStatClick[nIndex] && dwClick > pExcTmp->dwLastStatClick[nIndex])
		{
			dwSec += (dwClick-pExcTmp->dwLastStatClick[nIndex]);
			OoDWordToDoubleLongUnsigned(dwSec, bBuf+bOffset);

			OoWriteAttr(wOI, ATTR7, bBuf);
			pExcTmp->dwLastStatClick[nIndex] = dwClick;
		}
		//fTrigerSave = true;	//这里会一直写系统库
	}
	else if (fOnMtrExcEnd)	//事件刚结束
	{
		pExcTmp->dwLastStatClick[nIndex] = 0;	//清零统计时标，停止统计(事件发生期间的统计数据) ---是否需要触发保存统计数据？
	}

	if (fTrigerSave)
		TrigerSaveBank(BN0, SECT16, -1);

	return true; 
}


//描述：抄表事件参数变更需要重新初始化事件。
//参数：@dwOAD数据标识
//返回：无
void ReInitMtrExcPara(DWORD dwOAD)
{
	int nIndex;
	BYTE bClrFlag, bAttr = 0;
	WORD wOI, wPn;

	GetOIAttrIndex(dwOAD, &wOI, &bAttr, NULL);
	nIndex = GetMtrExcIndex(wOI);
	if (nIndex < 0)
		return;

	if (bAttr==ATTR3 || bAttr==ATTR5)	//关联属性表或最大记录数变更时需要清事件后重新初始化
	{
		wPn = nIndex;
		bClrFlag = MTREXC_CLR_VALID;
		WriteItemEx(BN11, wPn, MTREXC_CLR_ID, &bClrFlag);		//写清零标识
		TrigerSaveBank(BN11, 0, -1);

		SetInfo(INFO_MTR_EXC_RESET);
	}
}


//描述：抄表事件参数变更需要重新初始化事件。提供给上行对外接口
//参数：@dwOAD数据标识
//返回：无
void ReInitEvtPara(DWORD dwOAD)
{
	ReInitMrtEvtPara(dwOAD);
	ReInitMtrExcPara(dwOAD);
}



//收到参数/数据/事件记录初始化命令时的处理，外部调用
void MtrExcOnRxFaResetCmd()
{
	int nIndex;
	BYTE bClrFlag;
	WORD wPn;

	for (nIndex=0; nIndex<MTR_EXC_NUM; nIndex++)
	{
		wPn = nIndex;
		bClrFlag = MTREXC_CLR_VALID;
		WriteItemEx(BN11, wPn, MTREXC_CLR_ID, &bClrFlag);		//写清零标识
	}

	TrigerSaveBank(BN11, 0, -1);
	SetInfo(INFO_MTR_EXC_RESET);
}

//0x3105 电能表时钟超差
const BYTE g_bMClkCfg[] = {
	0x01,0x00,
};
//0x310B 电能表示度下降
const BYTE g_bMDecCfg[] = {
	0x01,0x02,
	0x51,0x00,0x10,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,
};
//0x310C 电能量超差
const BYTE g_bMErrCfg[] = {
	0x01,0x02,
	0x51,0x00,0x10,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,
};
//0x310D 电能表飞走
const BYTE g_bMFCfg[] = {
	0x01,0x02,
	0x51,0x00,0x10,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,
};
//0x310E 电能表停走
const BYTE g_bMSCfg[] = {
	0x01,0x01,
	0x51,0x00,0x10,0x22,0x01,
};
//0x310F 抄表失败事件
const BYTE g_bMRFCfg[] = {
	0x01,0x02,
	//0x51,0x60,0x41,0x22,0x00,
	0x51,0x00,0x10,0x22,0x01,
	0x51,0x00,0x30,0x22,0x01,
};
//0x311C 电能表数据变更监控记录
const BYTE g_bDaCgCfg[] = {
	0x01,0x00,
};

//描述：设置关联属性表默认参数
void SetMtrExcOadDefCfg(WORD wOI)
{
	BYTE bBuf[EVT_ATTRTAB_LEN];
	DWORD dwOAD;
	int iLen;
	const ToaMap* pOI;
	
	dwOAD = GetOAD(wOI, ATTR3, 0);
	pOI = GetOIMap(dwOAD);
	if (pOI == NULL)
		return;

	memset(bBuf, 0, sizeof(bBuf));	
	iLen = ReadItemEx(BN0, pOI->wPn, pOI->wID, bBuf);
	if (iLen>0 && IsAllAByte(bBuf, 0, sizeof(bBuf)))
	{
		if (wOI == OI_MTR_CLOCK_ERR)
			memcpy(bBuf, g_bMClkCfg, sizeof(g_bMClkCfg));
		else if (wOI == OI_MTR_ENERGY_DEC)
			memcpy(bBuf, g_bMDecCfg, sizeof(g_bMDecCfg));
		else if (wOI == OI_MTR_ENERGY_ERR)
			memcpy(bBuf, g_bMErrCfg, sizeof(g_bMErrCfg));
		else if (wOI == OI_MTR_FLEW)
			memcpy(bBuf, g_bMFCfg, sizeof(g_bMFCfg));
		else if (wOI == OI_MTR_STOP)
			memcpy(bBuf, g_bMSCfg, sizeof(g_bMSCfg));
		else if (wOI == OI_MTR_RD_FAIL)
			memcpy(bBuf, g_bMRFCfg, sizeof(g_bMRFCfg));
		else if (wOI == OI_MTR_DATA_CHG)
			memcpy(bBuf, g_bDaCgCfg, sizeof(g_bDaCgCfg));
		else
			return;
		
		WriteItemEx(BN0, pOI->wPn, pOI->wID, bBuf);
		TrigerSaveBank(BN0, SECT3, -1);
	}
	
	return;
}



//描述：更新抄表事件记录的上报状态
//参数：@dwCnOAD 通道OAD
//		@pEvtMsg事件上报消息
//		@bRptState 要置的标志位，记录中的通道上报状态的原有值会或上这个值
//返回：如果正确则返回true,否则返回false
bool UpdateMtrExcRptState(DWORD dwCnOAD, TEvtMsg* pEvtMsg, BYTE bRptState)
{
	BYTE bAttr, bItem, bIndex, bType, bCnNum, i = 0;
	BYTE bOadBuf[10];
	BYTE bRecBuf[MTR_EXC_REC_LEN];
	BYTE* pbRec = bRecBuf;
	WORD wOI, wItemOffset, wItemLen;
	DWORD dwOAD, dwRecCnOAD;
	int iLen, nIndex = 0;
	TFieldParser tFixFields;	
	char* pszFileName = NULL;
	
	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	GetOIAttrIndex(pEvtMsg->dwOAD, &wOI, &bAttr, NULL);
	
	//获取事件控制结构
	nIndex = GetMtrExcIndex(wOI);
	if (nIndex < 0)
		return false;	

	//取出上报事件的记录
	memset(bRecBuf, 0, sizeof(bRecBuf));
	iLen = GetEvtRec(pEvtMsg, bRecBuf, sizeof(bRecBuf), 1);
	if (iLen <= 0)
		return false;
	
	//获取固定字段
	if (GetMtrExcFieldParser(wOI, &tFixFields, NULL, NULL, 0) == false)
		return false;

	if (tFixFields.wNum == 0)
		return false;

	//重新刷新通道上报状态
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	//固定字段个数
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
			return false;

		if (bType != DT_OAD) 
			return false;

		if (wItemLen == 0) 
			return false;

		dwOAD = OoOadToDWord(bOadBuf+1);
		if (dwOAD == 0x33000200)	//通道上报状态刷新
		{
			if (pbRec != DT_NULL)
			{
				bCnNum = *(pbRec+1);
				if (bCnNum >= CN_RPT_NUM)
					bCnNum = CN_RPT_NUM;

				for(i=0; i<bCnNum; i++)
				{	
					dwRecCnOAD = OoDoubleLongUnsignedToDWord(pbRec+5+i*9);
					if ((dwCnOAD&0xff000000) == 0x45000000)	//要与函数SendEvtMsg()相匹配
					{	dwRecCnOAD &=0xff000000;
						dwCnOAD &=0xff000000;
					}
					else
					{	
						dwRecCnOAD &=0xffff0000;
						dwCnOAD &=0xffff0000;
					}
					if (dwCnOAD == dwRecCnOAD)
						*(pbRec+10+i*9) |= bRptState;
				}
			}
		}

		pbRec += wItemLen;
	}

	pszFileName = GetEvtRecFileName(pEvtMsg->dwOAD&0xffff1f00);
	if (pszFileName == NULL)
		return false;

	//if (bRptState&0x0a)
	//	AddEvtOad(pEvtMsg->dwOAD, 1);	//已上报

	//修正记录
	SaveRecordByPhyIdx(pszFileName, pEvtMsg->wRecIdx, bRecBuf);
	return true;
}


//
int OoProRptMtrExcRecord(WORD wOI, BYTE bAttr, BYTE* pbRecBuf, WORD wRecLen, WORD wBufSize)
{
	BYTE bType, bOadBuf[10], bBuf[EVT_ATTRTAB_LEN];
	BYTE bTmpRecBuf[MTR_EXC_REC_LEN];	//一条记录缓冲区
	BYTE* pbTmpRec = bTmpRecBuf;
	BYTE* pbRec = pbRecBuf;
	WORD wItemOffset, wItemLen, wTotalLen;
	DWORD dwOAD;
	int iLen;
	TFieldParser tFixFields;
	TFieldParser tDataFields;
	const ToaMap* pOaMap = NULL;
	BYTE bIndex, bStart = 0;

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));

	//获取固定字段和数据字段
	if (GetMtrExcFieldParser(wOI, &tFixFields, &tDataFields, bBuf, sizeof(bBuf)) == false)
	{
		DTRACE(DB_INMTR, ("OoProRptMtrExcRecord: wOI=%u GetEvtFieldParser() fail.\r\n", wOI));
		return -1;
	}

	wTotalLen = 0;

	// 4个字节的事件记录OAD
	OoDWordToOad(GetOAD(wOI, bAttr, 0), pbTmpRec);
	pbTmpRec += 4;
	wTotalLen+= 4;

	// 1字节元素个数
	*pbTmpRec++ = tFixFields.wNum+tDataFields.wNum;
	wTotalLen++;

	// 5字节每个元素类型OAD*元素个数
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)
	{
		//选择OAD类型
		*pbTmpRec++ = 0;wTotalLen++;
		//OAD
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;	//直接返回，固定字段不应答有问题
		}

		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;
		}

		memcpy(pbTmpRec, bOadBuf+1, 4);
		pbTmpRec += 4;
		wTotalLen += 4;
	}

	for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	
	{
		//选择OAD类型
		*pbTmpRec++ = 0;wTotalLen++;
		//OAD
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;	//直接返回，固定字段不应答有问题
		}

		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;
		}

		memcpy(pbTmpRec, bOadBuf+1, 4);
		pbTmpRec += 4;
		wTotalLen += 4;
	}

	// 1字节结果
	*pbTmpRec++ = 1;
	wTotalLen++;
	// 1字节结果条数，1条
	*pbTmpRec++ = 1;
	wTotalLen++;

	//具体数据	
	//处理固定字段的事件发生源、上报信息、事件清零列表等特殊数据
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;	//直接返回，固定字段不应答有问题
		}

		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;
		}

		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);
		
		//调整数据
		iLen = GetMtrExcEvtSpecField(dwOAD, pbRec, wItemLen, &bStart);
		if (iLen>0 && iLen<=wItemLen)
			memcpy(pbTmpRec, pbRec+bStart, iLen);
		else
			return -1;

		pbRec += wItemLen;	 
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}

	//处理数据字段
	for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;	//直接返回，固定字段不应答有问题
		}

		if (bType!=DT_OAD || wItemLen==0)
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;
		}		
		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);

		//调整数据
		iLen = GetMtrExcEvtSpecField(dwOAD, pbRec, wItemLen, &bStart);
		if (iLen>0 && iLen<=wItemLen)
			memcpy(pbTmpRec, pbRec+bStart, iLen);
		else
			return -1;
		
		pbRec += wItemLen;	
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}

	//memcpy(pbTmpRec, pbRec, tDataFields.wTotalLen);
	//wTotalLen +=  tDataFields.wTotalLen;
	if (wTotalLen <= wBufSize)
	{	
		memcpy(pbRecBuf, bTmpRecBuf, wTotalLen);
		//TrigerSaveBank(BN0, SECT3, -1);
		return wTotalLen;
	}

	return -1;
}

