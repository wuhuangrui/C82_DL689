/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FrzTask.cpp
 * 摘    要：本文件主要实现面向对象协议的冻结类
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2016年9月
 *********************************************************************************************************/
#include "stdafx.h"
#include "syscfg.h"
#include "sysfs.h"
#include "FaCfg.h"
#include <fcntl.h>
#include "ComStruct.h"
#include "FrzTask.h"
#include "ComAPI.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "DbOIAPI.h"
#include "CctTaskMangerOob.h"

#define FRZ_DELAY_TIMEOUT		10		//冻结滞后时间

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//全局变量定义
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BYTE g_bFrzFixOAList[] = { 0x01, 0x02, 0x51, 0x20, 0x23, 0x02, 0x00, 0x51, 0x20, 0x21, 0x02, 0x00 };	//固定字段内容描述
BYTE g_bFrzFixFmt[] = { 0x01, 0x02, 0x51 };		//固定字段格式描述串

BYTE g_bFrzSubDataFmt[] = { 0x01, 0x01, 0x51 };

const BYTE g_bFrzOITypeList[] = { FRZ_OIB_INST, FRZ_OIB_SEC, FRZ_OIB_MIN, FRZ_OIB_HOUR, FRZ_OIB_DAY, FRZ_OIB_BALANCEDAY, FRZ_OIB_MONTH, 
										FRZ_OIB_YEAR, FRZ_OIB_TIMEZONE, FRZ_OIB_DAYSTAGE, FRZ_OIB_TARIFF, FRZ_OIB_STAIR, FRZ_OIB_CLRYEAR, };

TFrzCtrl g_FrzCtrl[FRZ_TASK_NUM];


//特殊OAD到ID的映射表
const DWORD g_dwSpecSrcOADList[] = 
{
	0x21000200, 0x21010200,	0x21020200, 0x21030200, 0x21040200,	
	0x21100200, 0x21110200, 0x21120200, 0x21130200, 0x21140200,
	0x21200200, 0x21210200, 0x21220200, 0x21230200, 0x21240200,
	0x21300200, 0x21310200, 0x21320200, 0x21330200, 0x21400200, 
	0x21410200, 0x22000200, 0x22030200, 0x22040200,
};

//#define DEBUG_TEST 1

//任务配置是否有效
bool IsFrzTaskCfgValid(TFrzCtrl* pFrzCtrl)
{
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
	if (pFrzCfg->dwOA!=0 && pFrzCfg->wMaxRecNum!=0)		//任务配置有效
		return true;
	else
		return false;
}


//冻结任务类型序号映射到OI
//描述：获取冻结任务OI
WORD GetFrzOI(BYTE bFrzType)
{	
	if (bFrzType < sizeof(g_bFrzOITypeList))
		return g_bFrzOITypeList[bFrzType] + OI_FRZ;
	else
		return 0;	
}


//OI映射到任务类型序号
//描述：获取冻结任务类型序号
//返回：成功0~FRZ_TYPE_NUM， 失败：0xff
BYTE GetFrzType(WORD wOI)
{
	BYTE bFrzType;

	for (bFrzType=0; bFrzType<sizeof(g_bFrzOITypeList); bFrzType++)
	{
		if (wOI == (g_bFrzOITypeList[bFrzType]+OI_FRZ))
			return bFrzType;
	}

	return 0xff;
}


//0x6a, 0x03, 0x12, 0x00, 0x04, 0x51, 0x00, 0x10, 0x02, 0x00, 0x12, 0x00, 0x08 pbCfgBuf
//初始化子任务冻结控制结构
bool InitSubFrzTask(BYTE bFrzType, BYTE* pbCfgBuf, TFrzCtrl* pFrzCtrl)
{
	char szTableName[TASK_PATH_LEN];
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
	BYTE bRecBuf[FRZ_REC_LEN];

	pFrzCfg->wCycle = OoLongUnsignedToWord(pbCfgBuf+OFFSET_FRZ_CYCLE);
	pFrzCfg->dwOA = OoOadToDWord(pbCfgBuf+OFFSET_FRZ_OAD);
	pFrzCfg->wMaxRecNum = OoLongUnsignedToWord(pbCfgBuf+OFFSET_FRZ_REC_NUM);

	pFrzCtrl->wOI = OI_FRZ + bFrzType;
	sprintf(szTableName, FMT_FRZ_TASK_TABLE, GetFrzOI(bFrzType), pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);

	memset(&pFrzCtrl->tmLastRec, 0, sizeof(TTime));
	if (bFrzType!=FRZ_OIB_INST && ReadLastNRec(szTableName, LAST_REC, bRecBuf, sizeof(bRecBuf)) > 0)	//读取最近一笔记录发生时标
	{
		OoDateTimeSToTime(bRecBuf+REC_TIME_OFFSET, &pFrzCtrl->tmLastRec);
		if (IsInvalidTime(pFrzCtrl->tmLastRec))
			memset(&pFrzCtrl->tmLastRec, 0, sizeof(TTime));
	}

	return true;
}

//BYTE bCmpBuf[] = {0x02, 0x03, 0x12, 0x00, 0x00, 0x51, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00};
//给冻结类型控制结构增加数据类型
void GetFrzFmtBuf(TFrzCtrl* pFrzCtrl, BYTE* p)
{
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
	
	*p++ = DT_STRUCT;	//结构体类型
	*p++ = 0x03;
	*p++ = 0x12;

	OoWordToOi(pFrzCfg->wCycle, p);
	p += 2;

	*p++ = DT_OAD;
	OoDWordToOad(pFrzCfg->dwOA, p);
	p += 4;

	*p++ = 0x12;
	OoWordToOi(pFrzCfg->wMaxRecNum, p);
	p += 2;
}


bool InitFrzTask(BYTE bFrzType, TFrzCtrl* pFrzCtrl)
{
	int iLen;
	bool fExist;
	DWORD dwOA = 0;
	BYTE bIndex, bType = 0;
	WORD i, wOI, wFmtLen = 0, wItemOffset = 0, wItemLen = 0, wCycle = 0;
	BYTE bCfgBuf[20];	//
	BYTE bSubDataBuf[200];	//
	BYTE bBuf[FRZRELA_ID_LEN];		//OAD个数*(sizeof(FRZRELA)+2) + 数组类型1 + 数组元素个数1
	char szTableName[TASK_PATH_LEN];
	BYTE bCmpBuf[DT_FRZRELA_LEN];
	BYTE* pbFmt = NULL;
	TFrzCfg* pFrzCfg = NULL;
	TFieldParser tDataFields = { bBuf };
	TFieldParser tFixFields = { 
		g_bFrzFixOAList,
		sizeof(g_bFrzFixOAList)
	};
	TFieldParser tSubDataFields = { 
		bSubDataBuf,
		SUB_DATD_FIELD_LEN
	};
	
	#ifdef DEBUG_TEST
	BYTE g_bTestData[] = {0x01, 0x02, 0x02, 0x03, 0x12, 0x04, 0x00, 0x51, 0x00, 0x10, 0x02, 0x00, 0x12, 0x08, 0x00, 
									  0x02, 0x03, 0x12, 0x05, 0x00, 0x51, 0x00, 0x20, 0x02, 0x00, 0x12, 0x08, 0x00};
	BYTE bFmtBuf[] = {0x01, 0x02,
							0x02,0x03,
								 0x12,
								 0x51,
								 0x12,
	};
	#endif

	memset(bBuf, 0, sizeof(bBuf));
	#ifdef DEBUG_TEST
		memcpy(bBuf, g_bTestData, sizeof(g_bTestData));
		iLen = tDataFields.wCfgLen =  sizeof(g_bTestData);
		pbFmt = bFmtBuf;
		wFmtLen = sizeof(bFmtBuf);
	#else
		wOI = GetFrzOI(bFrzType);
		iLen = OoReadAttr(wOI, FRZ_ATTRTAB, tDataFields.pbCfg, &pbFmt, &wFmtLen);		//读取关联属性表
	#endif	

	if (iLen>0 && tDataFields.pbCfg[1]>0 && tDataFields.pbCfg[1]<=CAP_OAD_NUM)
	{
		tDataFields.wCfgLen = tDataFields.pbCfg[1]*DT_FRZRELA_LEN + 2;
		if (OoParseField(&tDataFields, pbFmt, wFmtLen, false) == false)
			return false;

		//内存映射表中的任务必须以关联属性表配置为准，以下为同步内存映射表的步骤：
		//Step1, 关联属性表中无，映射表有，则清零该任务映射，同时删表
		for (i=0; i<FRZ_TASK_NUM; i++)	
		{
			if (pFrzCtrl[i].wOI==GetFrzOI(bFrzType) && IsFrzTaskCfgValid(pFrzCtrl+i))		//OI相同
			{
				for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	//在关联属性表中查找是否存在
				{
					memset(bCfgBuf, 0, sizeof(bCfgBuf));
					iLen = ReadParserField(&tDataFields, bIndex, bCfgBuf, &bType, &wItemOffset, &wItemLen);
					if (iLen > 0)
					{
						GetFrzFmtBuf(pFrzCtrl+i, bCmpBuf);
						if (FieldCmp(DT_FRZRELA, bCmpBuf+1, DT_FRZRELA, bCfgBuf+1) == 0)	//完全相同
						{
							memset(bSubDataBuf, 0, sizeof(bSubDataBuf));
							bSubDataBuf[0] = 0x01;	//数组类型
							bSubDataBuf[1] = 0x01;	//数组元素个数
							bSubDataBuf[2] = DT_OAD;	//数据类型
							memcpy(bSubDataBuf+3, bCfgBuf+OFFSET_FRZ_OAD, 4);		//OAD

							memset(&tSubDataFields, 0, sizeof(TFieldParser));
							tSubDataFields.pbCfg = bSubDataBuf;
							tSubDataFields.wCfgLen = SUB_DATD_FIELD_LEN;

							if (OoParseField(&tSubDataFields, g_bFrzSubDataFmt, sizeof(g_bFrzSubDataFmt), true) == false)		//配置字段
								return false;

							if (pFrzCtrl[i].wDataFieldLen != tSubDataFields.wTotalLen)
								bIndex = tDataFields.wNum;		//虽然找到了，但数据字段变化了，需要清除任务映射和删表

							break;
						}
					}
				}

				if (bIndex == tDataFields.wNum)	//在关联属性表中没找到 则删表
				{
					TFrzCfg* pFrzCfg = &pFrzCtrl[i].tCfg;
					sprintf(szTableName, FMT_FRZ_TASK_TABLE, GetFrzOI(bFrzType), pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);
					TdbDeleteTable(szTableName);

					memset(pFrzCtrl+i, 0, sizeof(TFrzCtrl));
					DTRACE(DB_TASK, ("InitFrzTask delete table %s! bFrzType=%d.\r\n", szTableName, bFrzType));
				}
			}
		}

		//Step2, 关联属性表中有，内存映射表无，则添加该任务映射，同时建表
		for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	//关联对象个数
		{
			memset(bCfgBuf, 0, sizeof(bCfgBuf));
			iLen = ReadParserField(&tDataFields, bIndex, bCfgBuf, &bType, &wItemOffset, &wItemLen);		//读出配置的冻结参数
			if (iLen > 0)
			{
				if (OoGetDataLen(DT_OAD, bCfgBuf+OFFSET_FRZ_OAD) <= 0)		//关联对象OAD有效
					continue;	//继续创建其他表
				
				if (bType == 2)
					bCfgBuf[0] = DT_FRZRELA;	//结构体类型处理成内部冻结关联参数类型

				fExist = false;
				for (i=0; i<FRZ_TASK_NUM; i++)	//在内存映射表中查找是否已映射
				{
					if (pFrzCtrl[i].wOI==GetFrzOI(bFrzType) && IsFrzTaskCfgValid(pFrzCtrl+i))		//OI相同
					{
						pFrzCfg = &pFrzCtrl[i].tCfg;
						GetFrzFmtBuf(pFrzCtrl+i, bCmpBuf);
						if (FieldCmp(DT_FRZRELA, bCfgBuf+1, DT_FRZRELA, bCmpBuf+1) == 0)	//完全相同，则表示已存在
						{							
							DTRACE(DB_TASK, ("InitFrzTask OAD already fExist! i=%d, bFrzType=%d, wCycle=%d, dwOA=%04x, wMaxRecNum=%d.\r\n", i, bFrzType, bIndex, pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum));
							fExist = true;
							break;
						}
					}
				}

				if (fExist == false)	//内存映射表中不存在 则增加
				{
					for (i=0; i<FRZ_TASK_NUM; i++)
					{
						if (!IsFrzTaskCfgValid(pFrzCtrl+i))	//找到空位
						{
							DTRACE(DB_TASK, ("InitFrzTask find space i=%d, bFrzType=%d, bIndex=%d.\r\n", i, bFrzType, bIndex));
							InitSubFrzTask(bFrzType, bCfgBuf, pFrzCtrl+i);

							pFrzCfg = &pFrzCtrl[i].tCfg;
							sprintf(szTableName, FMT_FRZ_TASK_TABLE, GetFrzOI(bFrzType), pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);

							memset(&tFixFields, 0, sizeof(TFieldParser));
							tFixFields.pbCfg = g_bFrzFixOAList;
							tFixFields.wCfgLen = sizeof(g_bFrzFixOAList);
							OoParseField(&tFixFields, g_bFrzFixFmt, sizeof(g_bFrzFixFmt), true);	//固定字段

							memset(bSubDataBuf, 0, sizeof(bSubDataBuf));
							bSubDataBuf[0] = 0x01;	//数组类型
							bSubDataBuf[1] = 0x01;	//数组元素个数
							bSubDataBuf[2] = DT_OAD;	//数据类型
							memcpy(bSubDataBuf+3, bCfgBuf+OFFSET_FRZ_OAD, 4);		//OAD

							memset(&tSubDataFields, 0, sizeof(TFieldParser));
							tSubDataFields.pbCfg = bSubDataBuf;
							tSubDataFields.wCfgLen = SUB_DATD_FIELD_LEN;
							if (OoParseField(&tSubDataFields, g_bFrzSubDataFmt, sizeof(g_bFrzSubDataFmt), true) == false)		//配置字段
								return false;

							if (CreateTable(szTableName, &tFixFields, &tSubDataFields, pFrzCfg->wMaxRecNum) > 0)		//每个关联对象建1张表
								pFrzCtrl[i].wDataFieldLen = tSubDataFields.wTotalLen;	//更新数据字段长度

							break;
						}
					}

					if (i == FRZ_TASK_NUM)	//存满了就不做了
					{
						DTRACE(DB_TASK, ("InitFrzTask no space! bFrzType=%d, bIndex=%d.\r\n", bFrzType, bIndex));
						break;
					}
				}
			}
		}
	}

	return true;
}


bool ResetTaskData()
{
	WORD i;
	BYTE bFrzType;
	//bool fTrigSave = false;
	BYTE bBuf[10];
	char szTableName[TASK_PATH_LEN];

	for (bFrzType=0; bFrzType<FRZ_TYPE_NUM; bFrzType++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN11, bFrzType, 0x0b11, bBuf);
		if (bBuf[0]==bFrzType && bBuf[1]==FRZ_CLR_VALID)		//复位命令有效
		{
			for (i=0; i<FRZ_TASK_NUM; i++)
			{
				TFrzCtrl* pFrzCtrl = &g_FrzCtrl[i];
				if (bFrzType==GetFrzType(pFrzCtrl->wOI) && IsFrzTaskCfgValid(pFrzCtrl))
				{
					TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
					sprintf(szTableName, FMT_FRZ_TASK_TABLE, GetFrzOI(bFrzType), pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);
					TdbDeleteTable(szTableName);
				}
			}

			memset(bBuf, 0, sizeof(bBuf));
			WriteItemEx(BN11, bFrzType, 0x0b11, bBuf);
			//fTrigSave = true;
		}
	}

	//if (fTrigSave)
	//	TrigerSaveBank(BN11, 0, -1); //触发保存

	return true;
}


bool InitTask(bool fInit)
{
	int iLen;
	WORD wPn;
	BYTE bFrzType;
	BYTE bBuf[20];
	const WORD wCmdID = 0x0b10;
	
	if (fInit)
	{
		memset(g_FrzCtrl, 0, sizeof(g_FrzCtrl));	//上电或数据区初始化(需要重新检表时)，避免任务存储位置变化

		for (wPn=0; wPn<FRZ_TASK_NUM; wPn++)
		{
			iLen = ReadItemEx(BN11, wPn, wCmdID, bBuf);
			if (iLen > 0)
			{
				if (!IsAllAByte(bBuf, 0, iLen))
				{
					memset(bBuf, 0, iLen);
					WriteItemEx(BN11, wPn, wCmdID, bBuf);	//上电初始化命令参数
				}
			}
		}
	}

	for (bFrzType=0; bFrzType<FRZ_TYPE_NUM; bFrzType++)
		InitFrzTask(bFrzType, g_FrzCtrl);

	g_fFrzInit = true;
	return true;
}

//是否结算日
bool IsBalanceDay(const TTime& rNow)
{
	int iLen;
	WORD i;
	BYTE bBuf[BALANCE_DAY_NUM*6+2];

	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(OI_BALANCEDAY, ATTR2, bBuf, NULL, NULL);	//结算日参数
	if (iLen<=0 || bBuf[0]!=DT_ARRAY)	//默认1日0点
	{
		bBuf[0] = DT_ARRAY;
		bBuf[1] = 1;			//结算日个数
		bBuf[4] = DT_UNSIGN;
		bBuf[5] = 1;			//日
		bBuf[6] = DT_UNSIGN;
		bBuf[7] = 0;			//时
	}

	if (bBuf[1] > BALANCE_DAY_NUM)
		bBuf[1] = BALANCE_DAY_NUM;

	for (i=0; i<bBuf[1]; i++)
	{
		if (bBuf[i*6+4]==DT_UNSIGN && bBuf[i*6+6]==DT_UNSIGN)
		{
			if (rNow.nDay==bBuf[i*6+5] && rNow.nHour==bBuf[i*6+7])		//结算日日和时匹配
				return true;
		}
	}

	return false;
}


//时区是否切换
bool IsTimeZoneChg()
{
	int iLen;
	BYTE bBuf[10];	

	memset(bBuf, 0, sizeof(bBuf));
	iLen = ReadItemEx(BN4, PN0, 0xc900, bBuf);
	if (iLen > 0)
		return (bBuf[0]&0x1);
	else
		return false;
}

//清时区切换标志C900 BIT0
void ClrZoneChgFlag()
{
	BYTE bBuf[10];

	bBuf[0] &= 0xfe;	//清BIT0
	WriteItemEx(BN4, PN0, 0xc900, bBuf);
	TrigerSaveBank(BN4, 0, -1);
}

//时段表是否切换
bool IsDayStageChg()
{
	int iLen;
	BYTE bBuf[10];

	memset(bBuf, 0, sizeof(bBuf));
	iLen = ReadItemEx(BN4, PN0, 0xc900, bBuf);
	if (iLen > 0)
		return (bBuf[0]&0x2);
	else
		return false;
}

//清时段表切换标志C900 BIT1
void ClrDayStageChgFlag()
{
	BYTE bBuf[10];
	
	bBuf[0] &= 0xfd;	//清BIT1
	WriteItemEx(BN4, PN0, 0xc900, bBuf);
	TrigerSaveBank(BN4, 0, -1);
}


//滞后冻结时间是否到达 (配了统计数据的，需等统计完成后再冻结，这里滞后30秒冻结)
bool IsDelayTimeOut(TTime& rNow, bool fFrzStat)
{
	if (fFrzStat == false)
		return true;	//不冻结统计数据，无需延迟冻结

	if ((rNow.nMinute*60+rNow.nSecond) >= FRZ_DELAY_TIMEOUT)
		return true;
	else
		return false;
}


//是否滞后冻结
//统计类OAD数据需要等统计完后再冻结，这里滞后30秒冻结
bool IsDelayFrz(TFrzCtrl* pFrzCtrl)
{
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;

	if ((pFrzCfg->dwOA&OAD_OI_MASK) == 0x21000200 || (pFrzCfg->dwOA&OAD_OI_MASK) == 0x22000200)
		return true;
	else	//需量目前只有当前需量，没有当日当月需量，跨日/跨月时不会清0，故不需要处理
		return false;
}

//bTaskIndex:任务序号
int Timeout(TTime& rNow, WORD wTaskIndex, TFrzCtrl* pFrzCtrl)
{	
	int iLen;
	BYTE bFrzType;
	bool fFrzStat;
	WORD wPn, wIntervV, wDelaySec;	
	DWORD dwCur, dwLast, dwIntervVa;	// 当前时间，上次时间
	DWORD dwClick, dwRxCmdClick;	
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
	BYTE bBuf[20];

	wIntervV = pFrzCfg->wCycle;
	bFrzType = GetFrzType(pFrzCtrl->wOI);
	if (bFrzType >= FRZ_TYPE_NUM)
		return -1;

	fFrzStat = IsDelayFrz(pFrzCtrl);	//是否需要延迟冻结
	if (wIntervV == 0)	//周期为0，规范要求按触发类冻结处理
		return -1;

	// 按冻结类型作判断, 冻结基准时间为2000年1月1日0时0分
	switch (bFrzType)
	{
		/*case FRZ_OIB_SEC:	//防止频繁写flash
			dwCur = TimeToSeconds(rNow);
			dwLast = TimeToSeconds(pFrzCtrl->tmLastRec);

			// 抄冻结默认60秒
			if (wIntervV == 0)
				wIntervV = FRZ_DFTCYC_SEC;

			// 符合间隔时间，且与上笔记录的时间不同，
			if (dwCur%wIntervV==0 && dwCur!=dwLast) 
			{
				//SecondsToTime(dwCur, &rNow);
				return 0;
			}

			return -1;*/

		case FRZ_OIB_MIN:	//对于分钟冻结，在0秒冻结
			dwCur = TimeToMinutes(rNow);			
			dwLast = TimeToMinutes(pFrzCtrl->tmLastRec);	

			// 符合间隔时间，且与上笔记录的时间不同，
			if (dwCur%wIntervV==0 && dwCur!=dwLast && IsDelayTimeOut(rNow, fFrzStat))
				return 0;
			else
				return -1;

		case FRZ_OIB_HOUR:	//对于小时冻结
			dwCur = TimeToMinutes(rNow);			
			dwLast = TimeToMinutes(pFrzCtrl->tmLastRec);			

			dwIntervVa = wIntervV*60;

			// 符合间隔时间，且与上笔记录的时间不同，限制在整分钟冻结
			if (dwCur%dwIntervVa==0 && dwCur!=dwLast && IsDelayTimeOut(rNow, fFrzStat))
				return 0;
			else
				return -1;

		case FRZ_OIB_DAY:	//对于日冻结，在0时冻结
			dwCur = DaysFrom2000(rNow);			
			dwLast = DaysFrom2000(pFrzCtrl->tmLastRec);

			// 符合间隔时间，且与上笔记录的时间不同
			if (dwCur%wIntervV==0 && dwCur!=dwLast && rNow.nHour==0 && IsDelayTimeOut(rNow, fFrzStat))
				return 0;
			else					
				return -1;

		case FRZ_OIB_MONTH:	//对于月冻结，在1日0时冻结
			dwCur = MonthFrom2000(rNow);
			dwLast = MonthFrom2000(pFrzCtrl->tmLastRec);
			
			// 符合间隔时间，且与上笔记录的时间不同
			if (dwCur%wIntervV==0 && dwCur!=dwLast && rNow.nDay==1 && rNow.nHour==0 && IsDelayTimeOut(rNow, fFrzStat))
				return 0;
			else
				return -1;

		case FRZ_OIB_YEAR:	//对于年冻结，在1月1日0时冻结
			dwCur = rNow.nYear-BASETIME;
			dwLast = pFrzCtrl->tmLastRec.nYear-BASETIME; 

			// 符合间隔时间，且与上笔记录的时间不同
			if (dwCur%wIntervV==0 && dwCur!=dwLast && rNow.nMonth==1 && rNow.nDay==1 && rNow.nHour==0 && IsDelayTimeOut(rNow, fFrzStat))
				return 0;
			else
				return -1;

		case FRZ_OIB_BALANCEDAY:	//结算日
			dwCur = DaysFrom2000(rNow);
			dwLast = DaysFrom2000(pFrzCtrl->tmLastRec);

			if (IsBalanceDay(rNow) && dwCur!=dwLast && IsDelayTimeOut(rNow, fFrzStat))
				return 0;
			else
				return -1;

		case FRZ_OIB_TIMEZONE:
			if (!IsTimeZoneChg())
				return -1;

			ClrZoneChgFlag();			
			return 0;

		case FRZ_OIB_DAYSTAGE:
			if (!IsDayStageChg())
				return -1;

			ClrDayStageChgFlag();			
			return 0;

		default: 
			return -1;	//时间没到
	}

	return -1;	//时间没到
}


//是否为统计特殊OAD
bool IsSpecFrzOAD(DWORD dwOAD)
{
	WORD i;

	dwOAD &= OAD_OI_MASK;
	if (dwOAD==0x21000200 || dwOAD==0x22000200)		//统计OAD
	{
		for (i=0; i<sizeof(g_dwSpecSrcOADList)/sizeof(DWORD); i++)
		{
			if (dwOAD == g_dwSpecSrcOADList[i])
				return true;
		}
	}

	return false;
}

//获取统计映射OAD，映射到FRZ_STAT_BASE_OAD为基址+i的扩展OAD上,
//返回值: 0失败, >0成功
DWORD GetLastCycleFrzMapID(DWORD dwOAD)
{
	WORD i;

	//dwOAD &= OAD_OI_MASK;
	dwOAD &= 0xffffff00;
	for (i=0; i<sizeof(g_dwSpecSrcOADList)/sizeof(DWORD); i++)
	{
		if (dwOAD == g_dwSpecSrcOADList[i])
		{
			return FRZ_STAT_BASE_ID + i;
		}
	}

	return 0;
}

//获取统计类型
//BYTE bStatTypeList[] = { TIME_UNIT_MINUTE, TIME_UNIT_HOUR,  TIME_UNIT_DAY, TIME_UNIT_MONTH, TIME_UNIT_YEAR };
//返回：统计类型数组元素的序号, 返回值0xff表示日月统计类型
BYTE GetStatType(DWORD dwOAD)
{
	DWORD dwMaskOAD;
	BYTE bIndex, bType;

	dwMaskOAD = dwOAD & 0xfff01f00;
	bIndex = dwOAD & 0xff;

	switch (dwMaskOAD)
	{
		case 0x21300200:	//总及ABC电压合格率
		case 0x22000200:	//通信流量
		case 0x22030200:	//供电时间
		case 0x22040200:	//复位次数
			if (bIndex == 1)	//当日统计
				bType = 2;	//返回日类型				
			else if (bIndex == 2)	//当月统计
				bType = 3;	//返回月类型
			else
				bType = 0xff;	//返回日、月冻结类型

			break;
		
		case 0x21400200:
			bType = 2;	//返回日类型
			break;

		case 0x21410200:
			bType = 3;	//返回月类型
			break;

		default:
			bType = dwOAD - dwMaskOAD;
			break;
	}

	return bType;
}


//冻结间隔与统计间隔是否一致
bool IsIntervMatch(DWORD dwOAD)
{
	return true;
/*	BYTE bFrzType, bStatType;
	BYTE bFrzTypeList[] = { 0xff, 0xff, TIME_UNIT_MINUTE, TIME_UNIT_HOUR, TIME_UNIT_DAY, 0xff, TIME_UNIT_MONTH, TIME_UNIT_YEAR};
	BYTE bStatTypeList[] = { TIME_UNIT_MINUTE, TIME_UNIT_HOUR,  TIME_UNIT_DAY, TIME_UNIT_MONTH, TIME_UNIT_YEAR };

	bFrzType = GetFrzType((WORD ) dwOAD>>16);	//分时日月年
	if (bFrzType >= FRZ_TYPE_NUM)
		return false;

	bStatType = GetStatType(dwOAD);
	if (bStatType == 0xff)	//日月统计类型
	{
		if (bFrzType == TIME_UNIT_DAY || bFrzType == TIME_UNIT_MONTH)
			return true;
		else
			return false;
	}

	if (bFrzType>=sizeof(bFrzTypeList) || bStatType>=sizeof(bStatTypeList))
		return false;

	return (bFrzTypeList[bFrzType]==bStatTypeList[bStatType]);*/
}


bool FrzData(TTime& tmCurRec, TFrzCtrl* pFrzCtrl)
{
	WORD wID, wBufSize, wRecBufLeft = 0;
	int iLen;
	BYTE bIndex, bType, bFrzType;	
	BYTE bFixBuf[20];	//sizeof(g_bFrzFixOAList);
	BYTE bRecBuf[FRZ_REC_LEN];
	BYTE bOadBuf[4];
	WORD wFmtLen = 0, wItemOffset = 0, wItemLen = 0;
	DWORD dwRecIndex = 0, dwOAD = 0;
	BYTE* pbFmt;
	TTime tmNow;
	char szTableName[TASK_PATH_LEN];
	BYTE* p = bRecBuf;
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;	
	
	TFieldParser tFixFields = { 
		g_bFrzFixOAList, 
		sizeof(g_bFrzFixOAList) 
	};		//冻结类固定字段

	sprintf(szTableName, FMT_FRZ_TASK_TABLE, pFrzCtrl->wOI, pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);
	iLen = ReadLastNRec(szTableName, LAST_REC, bRecBuf, sizeof(bRecBuf));
	if (iLen > 0)
		dwRecIndex = OoDoubleLongUnsignedToDWord(bRecBuf+1);	//冻结记录序号 高字节在前传输		

	dwRecIndex++;
	memset(bRecBuf, 0, sizeof(bRecBuf));
	OoParseField(&tFixFields, g_bFrzFixFmt, sizeof(g_bFrzFixFmt), true);	//固定字段
		
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	//固定字段个数
	{
		memset(bFixBuf, 0, sizeof(bFixBuf));
		if (ReadParserField(&tFixFields, bIndex, bFixBuf, &bType, &wItemOffset, &wItemLen) > 0)		//固定字段OAD
		{			
			const ToaMap* pOaMap = GetOIMap(OoOadToDWord(bFixBuf+1));
			if (pOaMap==NULL || pOaMap->pFmt==NULL)
			{
				DTRACE(DB_TASK, ("FrzData: Read dwOA:%x failed !!\r\n", OoOadToDWord(bFixBuf+1)));
				return false;
			}

			*p++ = *pOaMap->pFmt;	//刷新数据类型

			if (*pOaMap->pFmt == DT_DATE_TIME_S)
			{
				bFrzType = GetFrzType(pFrzCtrl->wOI);
				if (bFrzType >= FRZ_TYPE_NUM)
					return false;

				tmNow = tmCurRec;
				tmNow.nSecond = 0;	//存记录时，将记录时间秒清0,避免同一时间点数据因秒差异搜索不到记录
				if (bFrzType>=FRZ_OIB_DAY && bFrzType<=FRZ_OIB_YEAR)
					tmNow.nMinute = 0;	//存记录时，避免同一时间点数据因分秒差异搜索不到记录

				iLen = OoTimeToDateTimeS(&tmNow, p);		//冻结时间
			}
			else //if (*pOaMap->pFmt == 0x6)	冻结序号
			{
				iLen = OoDWordToDoubleLongUnsigned(dwRecIndex, p);
			}

			p += iLen;
		}
	}

	OoDWordToOad(pFrzCfg->dwOA, bOadBuf);
	iLen = OoGetDataLen(DT_OAD, bOadBuf);	//获取OAD长度
	if (iLen <= 0)
	{
		DTRACE(DB_TASK, ("FrzData: data field len<=0!\r\n"));
		return false;
	}
	
	wRecBufLeft = sizeof(bRecBuf);
	if (wRecBufLeft > (p-bRecBuf))
	{
		wRecBufLeft -= (p-bRecBuf);
	}
	else
	{
		DTRACE(DB_TASK, ("FrzData:  wRecBuf not enough!\r\n"));
		return false;
	}

	if (OoReadOad(pFrzCfg->dwOA, p, iLen, wRecBufLeft))
		p += iLen;

	SaveRecord(szTableName, bRecBuf);
	//pFrzCtrl->tmLastRec = tmCurRec;
	return true;
}

//执行冻结任务
bool DoFrzTask(WORD wTaskIndex, TFrzCtrl* pFrzCtrl)
{
	TTime tmNow;	

	GetCurTime(&tmNow);
	int to = Timeout(tmNow, wTaskIndex, pFrzCtrl);
	if (to == 0)	//时间到
	{
		DTRACE(DB_TASK, ("DoFrzTask: wTaskIndex=%ld, wOI=%02x, dwOAD=%04x time out, do task.\n", pFrzCtrl->wOI, pFrzCtrl->tCfg.dwOA));
		if (FrzData(tmNow, pFrzCtrl))
			pFrzCtrl->tmLastRec = tmNow;	//冻结成功，更新时标
	}

	DoFrzDataCmd(wTaskIndex, pFrzCtrl);		//触发冻结命令
	DoReFrzDataCmd(wTaskIndex, pFrzCtrl);	//触发补冻结命令

	return true;
}

//描述:读取指定任务表、时间、测量点号的一笔记录
//参数:@pbRecBuf 用来返回完整的一笔记录
//返回：如果正确则返回缓存区内记录的长度,小于0表示错误,0表示无数据
int SchFrzRec(char* pszName, const TTime& time)
{
	int fd = TdbOpenTable(pszName, O_RDONLY);
	if (fd < 0)
	{
		DTRACE(DB_TASK, ("SchComTaskRec: fail to open table %s\r\n", pszName));
		return -1;
	}

	TTdbSchRule TdbSchRule[1]; //一条规则
	//第一条规则 
	TdbSchRule[0].wOpNum = 1; //比较方法数量
	TdbSchRule[0].wField = 1; //字段1:冻结时间
	TdbSchRule[0].wOp[0] = TDB_OP_EQ;	

	TdbSchRule[0].bVal[0][0] = (time.nYear>>8) & 0xff;
	TdbSchRule[0].bVal[0][1] = time.nYear & 0xff;
	TdbSchRule[0].bVal[0][2] = time.nMonth;
	TdbSchRule[0].bVal[0][3] = time.nDay;
	TdbSchRule[0].bVal[0][4] = time.nHour;
	TdbSchRule[0].bVal[0][5] = time.nMinute;
	TdbSchRule[0].bVal[0][6] = time.nSecond;

	TTdbSchCtrl TdbSchCtrl; //搜索控制结构,支持二级排序
	//排序规则
	TdbSchCtrl.wSortNum = 1;		//排序规则个数
	TdbSchCtrl.wSortOp[0] = TDB_OP_GT;
	TdbSchCtrl.wSortFild[0] = 0;	

	TdbSchCtrl.wRecsToSch = 1;
	TdbSchCtrl.iPrivateRecStart = -1;
	TdbSchCtrl.iPublicRecStart = -1;
	TdbSchCtrl.wRecsFound = 0;
	int iSchId = TdbOpenSch(fd, TdbSchRule, sizeof(TdbSchRule)/sizeof(TTdbSchRule), TdbSchCtrl);
	if (iSchId<0 || TdbSchCtrl.wRecsFound==0)
	{
		TdbCloseSch(iSchId);
		TdbCloseTable(fd);
		DTRACE(DB_TASK, ("SchComTaskRec: tdb=%s sch fail, iSchId=%d, wRecsFound=%d, time=%04d-%02d-%02d %02d:%02d:%02d.\r\n", 
			pszName, iSchId, TdbSchCtrl.wRecsFound,
			time.nYear, time.nMonth, time.nDay, 
			time.nHour, time.nMinute, time.nSecond
			));
		return 0;
	}	

	//TTdbReadCtrl TdbReadCtrl;
	//TdbReadCtrl.dwFiledNeed = TDB_ALL_FIELD;
	//TdbReadCtrl.iRecStart = -1;

	//日月冻结记录结构:冻结时间(5)+测量点(2)+抄表时间(5)+费率(1)+数据
	//int iRet = TdbReadRec(iSchId, TdbSchRule, sizeof(TdbSchRule)/sizeof(TTdbSchRule), TdbReadCtrl, pbRecBuf);
	TdbCloseSch(iSchId);
	TdbCloseTable(fd);

	//if (IsInvaildData(wTask, pbRecBuf+5+2))//去掉冻结时标+测量点
	//{
	//	iRet = -1;
	//}

	return TdbSchCtrl.wRecsFound;
}

//描述：获取周期性冻结OI的周期单位
//返回：冻结的周期单位，成功：大于0，失败：0
BYTE GetFrzInterU(WORD wOI)
{
	BYTE bFrzType, bFrzInterU = 0;
	bFrzType = GetFrzType(wOI);
	switch(bFrzType)
	{
	//case FRZ_OIB_SEC:
	//	break;
	case FRZ_OIB_MIN:
		bFrzInterU = TIME_UNIT_MINUTE;
		break;
	case FRZ_OIB_HOUR:
		bFrzInterU = TIME_UNIT_HOUR;
		break;
	case FRZ_OIB_DAY:
		bFrzInterU = TIME_UNIT_DAY;
		break;
	case FRZ_OIB_MONTH:
		bFrzInterU = TIME_UNIT_MONTH;
		break;
	case FRZ_OIB_YEAR:
		bFrzInterU = TIME_UNIT_YEAR;
		break;
	default:
		bFrzInterU = 0;
		break;
	}

	return bFrzInterU;
}

//是否收到触发补冻结命令
//返回：需要补冻结的点数
BYTE GetReFrzDotNum(WORD wTaskIndex, BYTE bInterU, BYTE bInterV, TTime* ptmStart)
{
	int iLen, iIntervPast = 0;
	BYTE bDotNum = 0;
	WORD wPn = wTaskIndex;
	const WORD wCmdID = 0x0b16;
	DWORD dwStartSec, dwEndSec, dwCurSec;
	TTime tmStart, tmEnd, tmNow;
	BYTE bBuf[30];

	memset(bBuf, 0, sizeof(bBuf));
	iLen = ReadItemEx(BN11, wPn, wCmdID, bBuf);
	if (iLen > 0)
	{
		if (IsAllAByte(bBuf, 0, iLen) == false)		//收到补冻结命令
		{
			GetCurTime(&tmNow);
			dwCurSec = TimeToSeconds(tmNow);

			memcpy((BYTE* )&dwStartSec, bBuf, sizeof(DWORD));
			if (dwStartSec > dwCurSec)
				dwStartSec = dwCurSec;

			memcpy((BYTE* )&dwEndSec, bBuf+sizeof(DWORD), sizeof(DWORD));
			if (dwEndSec > dwCurSec)
				dwEndSec = dwCurSec;

			SecondsToTime(dwStartSec, &tmStart);
			SecondsToTime(dwEndSec, &tmEnd);

			if (bInterU == TIME_UNIT_YEAR)	//年按12个月算
			{
				bInterV *= 12;
				bInterU = TIME_UNIT_MONTH;
			}

			iIntervPast = IntervsPast(tmStart, tmEnd, bInterU, bInterV);	//从基准时间开始，经历的整数个周期个数iPast	
			if (iIntervPast > 0)
				bDotNum = iIntervPast;

			memcpy(ptmStart, &tmStart, sizeof(TTime));	//返回起始时间

			memset(bBuf, 0, sizeof(bBuf));
			WriteItemEx(BN11, wPn, wCmdID, bBuf);	//清除命令
			//TrigerSaveBank(BN11, 0, -1);
		}
	}

	return bDotNum;
}


//该时间点记录是否存在
bool IsFrzRecExist(WORD wOI, TTime& tmStart, TFrzCtrl* pFrzCtrl)
{
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
	char szTableName[TASK_PATH_LEN];

	sprintf(szTableName, FMT_FRZ_TASK_TABLE, wOI, pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);
	
	return (SchFrzRec(szTableName, tmStart) > 0);
}


//起始时间按间隔归整
bool GetIntervTime(BYTE bInterU, BYTE bInterV, TTime& tmStart)
{
	DWORD dwStart;
	DWORD dwCycle = 1;

	if (bInterV == 0)
		return false;

	switch (bInterU)
	{
		case TIME_UNIT_MINUTE:
			tmStart.nSecond = 0;
			dwStart = TimeToSeconds(tmStart);

			dwCycle = bInterV * 60;

			dwStart = dwStart / dwCycle * dwCycle;
			SecondsToTime(dwStart, &tmStart);
			break;
		
		case TIME_UNIT_HOUR:
			tmStart.nSecond = 0;
			tmStart.nMinute = 0;
			dwStart = TimeToMinutes(tmStart);

			dwCycle = bInterV * 60;
			dwStart = dwStart / dwCycle * dwCycle;
			MinutesToTime(dwStart, &tmStart);
			break;
		
		case TIME_UNIT_DAY:
			tmStart.nSecond = 0;
			tmStart.nMinute = 0;
			tmStart.nHour = 0;
			dwStart = TimeToMinutes(tmStart);
			
			dwCycle = bInterV * 60 * 24;
			dwStart = dwStart / dwCycle * dwCycle;
			MinutesToTime(dwStart, &tmStart);
			break;
		
		case TIME_UNIT_MONTH:
			dwStart = MonthFrom2000(tmStart) / bInterV * bInterV;	//获取距离BASETIME的月份数

			tmStart.nSecond = 0;
			tmStart.nMinute = 0;
			tmStart.nHour = 0;
			tmStart.nDay = 1;
			tmStart.nYear = BASETIME;
			AddIntervs(tmStart, TIME_UNIT_MONTH, dwStart);
			break;

		case TIME_UNIT_YEAR:
			dwStart = tmStart.nYear - BASETIME;
			dwStart = dwStart / bInterV * bInterV;	//获取距离BASETIME的年数

			tmStart.nSecond = 0;
			tmStart.nMinute = 0;
			tmStart.nHour = 0;
			tmStart.nDay = 1;
			tmStart.nMonth = 1;
			tmStart.nYear = BASETIME + dwStart;
			break;

		default: 
			return false;
	}

	return true;
}

//处理勘误增加的方法6 触发数据补冻结（起始时间，截止时间）
void DoReFrzDataCmd(WORD wTaskIndex, TFrzCtrl* pFrzCtrl)
{
	BYTE bInterU, bInterV, bDotNum = 0;
	bool fRxReFrzCmd = false;
	DWORD dwIntervSec = 0;
	TTime tmStart = { 0 };
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;

	bInterU = GetFrzInterU(pFrzCtrl->wOI);
	bInterV = pFrzCfg->wCycle;		//WORD -> BYTE精度可能丢失
	bDotNum = GetReFrzDotNum(wTaskIndex, bInterU, bInterV, &tmStart);
	if (bDotNum>0 && bInterU>0 && bInterV>0)
	{	
		GetIntervTime(bInterU, bInterU, tmStart);
		while (bDotNum-- > 0)
		{
			if (!IsFrzRecExist(pFrzCtrl->wOI, tmStart, pFrzCtrl))		//该点数据不存在
				FrzData(tmStart, pFrzCtrl);			//补冻实时数据

			if (bInterU == TIME_UNIT_YEAR)
				AddIntervs(tmStart, TIME_UNIT_MONTH, pFrzCfg->wCycle*12);	//年按12个月算
			else
				AddIntervs(tmStart, bInterU, pFrzCfg->wCycle);
		}
	}
}


//是否收到方法3触发冻结命令
bool IsRxFrzCmd(WORD wPn)
{
	int iLen;
	WORD wDelaySec;		//每个任务1个冻结命令参数
	DWORD dwClick, dwRxCmdClick;
	const WORD wCmdID = 0x0b10;
	BYTE bBuf[30];
	
	memset(bBuf, 0, sizeof(bBuf));
	iLen = ReadItemEx(BN11, wPn, wCmdID, bBuf);
	if (iLen > 0)
	{
		if (IsAllAByte(bBuf, 0, iLen) == false)		//收到触发冻结命令
		{
			wDelaySec = OoLongUnsignedToWord(bBuf);		//延迟时间
			dwRxCmdClick = ByteToDWORD(bBuf+2, 4);	//接收命令时标

			dwClick = GetClick();
			if (dwClick > dwRxCmdClick)
			{
				if (dwClick-dwRxCmdClick > wDelaySec)
				{
					memset(bBuf, 0, sizeof(bBuf));
					WriteItemEx(BN11, wPn, wCmdID, bBuf);	//清除命令
					//TrigerSaveBank(BN11, 0, -1);
					return true;
				}
			}
		}
	}

	return false;
}


//处理方法3触发命令
void DoFrzDataCmd(WORD wTaskIndex, TFrzCtrl* pFrzCtrl)
{
	TTime tmNow;
	GetCurTime(&tmNow);

	if (IsRxFrzCmd(wTaskIndex))
		FrzData(tmNow, pFrzCtrl);
}


void DoFrzTasks()
{
	WORD i;
	TFrzCtrl* pFrzCtrl;

	if (GetInfo(INFO_FRZPARA_CHG))
	{
		InitTask(false);
	}

	if (GetInfo(INFO_FRZDATA_RESET))	//收到复位命令，先删表再重新初始化
	{
		ResetTaskData();
		InitTask(true);	//重新初始化
	}

	for (i=0; i<FRZ_TASK_NUM; i++)
	{
		pFrzCtrl = &g_FrzCtrl[i];
		if (IsFrzTaskCfgValid(pFrzCtrl))	//任务配置有效
			DoFrzTask(i, pFrzCtrl);
	}
}

/*说明: 通过物理地址直接获得一条记录
 *@szTbName:    需打开的文件名;
 *@bPtr: 一条记录的物理地址
 *@pbBuf:       存冻结数据缓冲区;
 *@iLen:        数据缓冲区大小;
 返回值: <=0错误; >0获得数据的大小
*/
int ReadRecByPhyIdx(char* pszTbName, WORD wPhyIdx, BYTE* pbBuf, int iLen)
{
	int iRet;
	
	if ((iLen<=0) || (wPhyIdx<0))
		return -1;

	int fd = TdbOpenTable(pszTbName, O_RDONLY);
	if (fd < 0)
	{
		DTRACE(DB_TASK, ("ReadRecByPhyIdx: fail to open table:%s.\n", pszTbName));
		return -2;
	}

	iRet = TdbReadRec(fd, wPhyIdx, pbBuf, iLen);
	if (iRet <= 0)
		DTRACE(DB_TASK, ("ReadRecByPhyIdx: TdbReadRec fail! iIdx:%d, iRet:%d.\r\n", wPhyIdx, iRet));

	TdbCloseTable(fd);
	return iRet;
}


/*说明: 读取上N笔记录
 *@szTbName:    需打开的文件名;
 *@bPtr:		上N笔记录
 *@pbBuf:       存冻结数据缓冲区;
 *@iLen:        数据缓冲区大小;
 返回值: <=0错误; >0获得数据的大小
*/
int ReadLastNRec(char* szTbName, BYTE bPtr, BYTE* pbBuf, int iLen)
{
	if (iLen <= 0)
		return -1;
		
	int fd = TdbOpenTable(szTbName, O_RDONLY);
	if (fd < 0)
	{
		DTRACE(DB_TASK, ("ReadLastNRec: fail to open table:%s.\n", szTbName));
		return -2;
	}
	
    int iRet, iIdx;
	iIdx = GetRecIdx(fd, bPtr);
	if (iIdx < 0)
	{
		DTRACE(DB_TASK, ("ReadLastNRec: GetRecIdx fail! bPtr:%d, iIdx:%d! .\r\n", bPtr, iIdx));
		TdbCloseTable(fd);
		return -2;		
	}

	iRet = TdbReadRec(fd, iIdx, pbBuf, iLen);
	if (iRet <= 0)
		DTRACE(DB_TASK, ("ReadLastNRec: TdbReadRec fail! bPtr:%d, iIdx:%d, iRet:%d.\r\n", bPtr, iIdx, iRet));

	TdbCloseTable(fd);
	return iRet;
}


//描述：根据当前表的记录号得到其在表中的记录索引
//参数：@fd 数据库表的句柄; @iRecNo 表的记录号(从1开始)
//返回：当前表记录号的记录索引,小于0表示错误
int GetRecIdx(const int fd, int iRecNo)
{
	int iRecNum = TdbGetRecNum(fd);	//TDB_ERR_DBLOCKED:-11, TDB_ERR_TBNOEXIST:-3
	if (iRecNum <= 0)
		return iRecNum;
		
	if (iRecNo > iRecNum)
		return ERR_OVER_RECNUM;		//ERR_OVER_RECNUM:-20
		
	int iRecPtr = TdbGetRecPtr(fd);	//TDB_ERR_DBLOCKED:-11, TDB_ERR_TBNOEXIST:-3
	if (iRecPtr < 0)
		return iRecPtr;
	
	int iRecIdx, iTmpIdx;
	if (iRecPtr < iRecNum)
	{
		iTmpIdx = (iRecPtr + iRecNum - iRecNo)%iRecNum;
	}
	else
		iTmpIdx = iRecPtr - iRecNo;
	if ( iTmpIdx >= 0 )
		iRecIdx = iTmpIdx;
	else
		iRecIdx = iRecNum + iTmpIdx;
	
	return iRecIdx;
}

/*说明:获取一条记录的物理地址
 *@szTbName:    需打开的文件名;
 *@bPtr: 要获得上次哪个月的冻结数据,从1开始;
 返回值: <=0错误; >0获得的物理地址
*/
int GetRecPhyIdx(char* szTbName, BYTE bPtr)
{	
	int iIdx;
	int fd = TdbOpenTable(szTbName, O_RDONLY);
	
	if (fd < 0)
	{
		DTRACE(DB_TASK, ("ReadFrzRec: fail to open table:%s.\n", szTbName));
		//TdbCloseTable(fd);
		return -1;
	}
	
	iIdx = GetRecIdx(fd, bPtr);
	TdbCloseTable(fd);
	return iIdx;
}

//描述：给交采或外部调用的触发冻结数据接口,比如时段表切换或结算日等冻结
void OnTrigFrzData(WORD wOI)
{
	WORD i;
	TTime tmNow;
	TFrzCtrl* pFrzCtrl;

	GetCurTime(&tmNow);
	for (i=0; i<FRZ_TASK_NUM; i++)
	{
		pFrzCtrl = &g_FrzCtrl[i];
		if (wOI==pFrzCtrl->wOI && IsFrzTaskCfgValid(pFrzCtrl))
			FrzData(tmNow, pFrzCtrl);
	}
}


//方法1：复位
int OnResetFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bPn;
	BYTE bBuf[10];

	if (GetOiClass(wOI)!=IC9 || bMethod!=FRZ_RESET)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}
	
	bPn = bBuf[0] = GetFrzType(wOI);
	if (bPn == 0xff)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	bBuf[1] = FRZ_CLR_VALID;	//有效标志
	WriteItemEx(BN11, bPn, 0x0b11, bBuf);
	//TrigerSaveBank(BN11, 0, -1); //触发保存

	SetInfo(INFO_FRZDATA_RESET);
	*pbRes = 0;	//成功  （0） 返回结果
	return 0;
}

//方法2：执行
//空函数
int OnRunFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	if (GetOiClass(wOI)!=IC9 || bMethod!=FRZ_RUN)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}
	
	//nothing to do

	*pbRes = 0;	//成功  （0） 返回结果
	return 0;
}



//方法3：触发一次记录
//描述：保存触发冻结命令
int OnRxTrigFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i;
	const WORD wCmdID = 0x0b10;
	DWORD dwClick;
	TFrzCtrl* pFrzCtrl;	

	if (bMethod != FRZ_TRIG)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	if (*pbPara != DT_LONG_U)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}
	
	dwClick = GetClick();
	memcpy(pbPara+3, &dwClick, sizeof(DWORD));

	for (i=0; i<FRZ_TASK_NUM; i++)
	{
		pFrzCtrl = &g_FrzCtrl[i];
		if (wOI==pFrzCtrl->wOI && IsFrzTaskCfgValid(pFrzCtrl))
			WriteItemEx(BN11, i, wCmdID, pbPara+1);				// 数据格式如下：2字节延迟时间 + 4字节当前click
	}

	*pbRes = 0;	//成功  （0） 返回结果
	return 0;
}

//方法6：触发数据补冻结（起始时间，截止时间）
//描述：保存触发数据补冻结命令
int OnRxTrigReFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i;	
	const WORD wCmdID = 0x0b16;
	DWORD dwStartSec, dwEndSec;
	TTime tmStart, tmEnd;
	TFrzCtrl* pFrzCtrl;
	BYTE bBuf[20];
	BYTE* pbRxBuf = pbPara;

	if (bMethod != FRZ_REFRZ)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	if (*pbPara++ != DT_DATE_TIME_S)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	OoDateTimeSToTime(pbPara, &tmStart);
	pbPara += 7;
	
	if (*pbPara++ != DT_DATE_TIME_S)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	OoDateTimeSToTime(pbPara, &tmEnd);
	pbPara += 7;

	dwStartSec = TimeToSeconds(tmStart);
	dwEndSec = TimeToSeconds(tmEnd);

	memset(bBuf, 0, sizeof(bBuf));
	memcpy(bBuf, (BYTE* )&dwStartSec, sizeof(DWORD));
	memcpy(bBuf+sizeof(DWORD), (BYTE* )&dwEndSec, sizeof(DWORD));

	for (i=0; i<FRZ_TASK_NUM; i++)
	{
		pFrzCtrl = &g_FrzCtrl[i];
		if (wOI==pFrzCtrl->wOI && IsFrzTaskCfgValid(pFrzCtrl))
			WriteItemEx(BN11, i, wCmdID, bBuf);		// 数据格式如下：起始时间4 + 结束时间4
	}

	*pbRes = 0;	//成功  （0） 返回结果
	return 0;
}




//BYTE bCmpBuf[] = {0x02, 0x03, 0x12, 0x00, 0x00, 0x51, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00};
//方法4：添加一个冻结关联对象属性（参数）
//参数∷=FRZRELA 对象属性描述符
int OnAddFrzAttrCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i;
	int iLen;
	BYTE bCapNum;
	BYTE bBuf[FRZRELA_ID_LEN];		//OAD个数*sizeof(FRZRELA) + 数组类型1 + 数组元素个数1	
		
	if (GetOiClass(wOI)!=IC9 || bMethod!=FRZ_ADDATTR || pbPara[0]!=DT_STRUCT || pbPara[1]!=3)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}	

	// 读取关联属性表
	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, FRZ_ATTRTAB, bBuf, NULL, NULL);
	if (iLen <= 0)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	bCapNum = bBuf[1];
	if (bCapNum >= CAP_OAD_NUM)		//已经存满了 返回失败
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	for (i=0; i<bCapNum; i++)	//遍历是否已经存在
	{
		if (FieldCmp(DT_FRZRELA, &bBuf[i*DT_FRZRELA_LEN + 3], DT_FRZRELA, pbPara+1) == 0)	//完全相同，则认为是无效参数,保证OAD唯一性
		{
			//*pbRes = 3;	//拒绝读写 （3）
			//return -1;
			break;
		}
		else if (FieldCmp(DT_FRZRELA, &bBuf[i*DT_FRZRELA_LEN + 3], DT_OAD, pbPara+OFFSET_FRZ_OAD) == 0)	//OAD相同，但周期和深度不同,修改该关联对象参数
		{
			memcpy(&bBuf[i*DT_FRZRELA_LEN + 2], pbPara, DT_FRZRELA_LEN);	//修改关联对象参数
			break;
		}
	}

	// 新添加一个OAD
	if (i == bCapNum)
	{		
		memcpy(&bBuf[i*DT_FRZRELA_LEN + 2], pbPara, DT_FRZRELA_LEN);	//新增加的放在最后
		bCapNum++;
		bBuf[1] = bCapNum;	//数组元素个数
	}

	// 刷新关联属性表
	if (OoWriteAttr(wOI, FRZ_ATTRTAB, bBuf) <= 0)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	TrigerSaveBank(BN0, SECT5, -1);
	// 返回结果
	*pbRes = 0;	//成功  （0）
	return 0;
}


//方法5：删除一个冻结对象属性（参数）
//参数∷=OAD 对象属性描述符
int OnDelFrzAttrCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i, j;
	int iLen;
	BYTE bCapNum;
	BYTE bBuf[FRZRELA_ID_LEN];		//OAD个数*sizeof(FRZRELA) + 数组类型1 + 数组元素个数1
		
	if (GetOiClass(wOI)!=IC9 || bMethod!=FRZ_DELATTR || pbPara[0]!=DT_OAD)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	// 读取关联属性表
	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, FRZ_ATTRTAB, bBuf, NULL, NULL);
	if (iLen <= 0)	//空的
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}	

	if (bBuf[1] == 0)	//空的
	{
		*pbRes = 0;	//成功
		return 0;
	}

	if (bBuf[1] > CAP_OAD_NUM)
		bCapNum = CAP_OAD_NUM;
	else
		bCapNum = bBuf[1];

	for (i=0; i<bCapNum; i++)	//遍历是否已经存在
	{
		if (FieldCmp(DT_FRZRELA, &bBuf[i*DT_FRZRELA_LEN + 3], DT_OAD, pbPara+1) == 0)	//找到目标
		{
			memset(&bBuf[i*DT_FRZRELA_LEN + 2], 0, DT_FRZRELA_LEN);
			bBuf[1]--;	//数组元素个数
			break;
		}
	}
	
	if (i == bCapNum)	//没找到
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}
	
	for (j=i; j<bCapNum-1; j++)		//后面的参数往前挪
	{
		memcpy(&bBuf[j*DT_FRZRELA_LEN + 2], &bBuf[(j+1)*DT_FRZRELA_LEN + 2], DT_FRZRELA_LEN);
	}
	
	memset(&bBuf[j*DT_FRZRELA_LEN + 2], 0, DT_FRZRELA_LEN);		//j=bCapNum-1清0最后一个关联OAD参数

	// 刷新关联属性表
	if (OoWriteAttr(wOI, FRZ_ATTRTAB, bBuf) <= 0)
	{
		*pbRes = 3;	//写失败 （3）？？？
		return -1;
	}
	else
	{
		TrigerSaveBank(BN0, SECT5, -1);
		*pbRes = 0;	//成功  （0） 返回结果
		return 0;
	}
}


//方法7：批量添加关联属性表（参数）
//参数∷=array 冻结对象(FRZRELA)
int OnBatAddFrzAttrCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i, k;
	int iLen;
	BYTE bCapNum;
	BYTE bBuf[FRZRELA_ID_LEN];		//OAD个数*sizeof(FRZRELA) + 数组类型1 + 数组元素个数1	
		
	if (GetOiClass(wOI)!=IC9 || bMethod!=FRZ_BATADDATTR || pbPara[0]!=DT_ARRAY || pbPara[1]==0 || pbPara[1]>CAP_OAD_NUM)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}	

	// 读取关联属性表
	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, FRZ_ATTRTAB, bBuf, NULL, NULL);
	if (iLen <= 0)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	bCapNum = bBuf[1];
	if (bCapNum+pbPara[1] > CAP_OAD_NUM)		//空间不够 返回失败
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}	

	for (k=0; k<pbPara[1]; k++)	//批量添加pbPara[1]个
	{
		for (i=0; i<bCapNum; i++)	//遍历是否已经存在
		{
			if (FieldCmp(DT_FRZRELA, &bBuf[i*DT_FRZRELA_LEN+3], DT_FRZRELA, &pbPara[k*DT_FRZRELA_LEN+3]) == 0)	//完全相同，则认为是无效参数,保证OAD唯一性
			{
				//*pbRes = 3;	//拒绝读写 （3）
				//return -1;
				break;
			}
			else if (FieldCmp(DT_FRZRELA, &bBuf[i*DT_FRZRELA_LEN + 3], DT_OAD, &pbPara[k*DT_FRZRELA_LEN+8]) == 0)	//OAD相同，但周期和深度不同,修改该关联对象参数
			{
				memcpy(&bBuf[i*DT_FRZRELA_LEN+2], &pbPara[k*DT_FRZRELA_LEN+2], DT_FRZRELA_LEN);	//修改关联对象参数
				break;	//continue;
			}
		}

		// 新添加一个OAD
		if (i == bCapNum)
		{		
			memcpy(&bBuf[i*DT_FRZRELA_LEN + 2], &pbPara[k*DT_FRZRELA_LEN+2], DT_FRZRELA_LEN);	//新增加的放在最后
			bCapNum++;
			bBuf[1] = bCapNum;	//数组元素个数
		}
	}

	// 刷新关联属性表
	if (OoWriteAttr(wOI, FRZ_ATTRTAB, bBuf) <= 0)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	TrigerSaveBank(BN0, SECT5, -1);
	// 返回结果
	*pbRes = 0;	//成功  （0）
	return 0;
}




//方法8：清除关联对象属性表（参数）
//参数∷=NULL
int OnClrAttrTableCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i, j;
	int iLen;
	BYTE bCapNum;
	BYTE bBuf[FRZRELA_ID_LEN];		//OAD个数*sizeof(FRZRELA) + 数组类型1 + 数组元素个数1
		
	if (GetOiClass(wOI)!=IC9 || bMethod!=FRZ_CLRATTR)
	{
		*pbRes = 3;	//拒绝读写 （3）
		return -1;
	}

	// 读取关联属性表
	memset(bBuf, 0, sizeof(bBuf));
	bBuf[0] = DT_ARRAY;

	// 刷新关联属性表
	if (OoWriteAttr(wOI, FRZ_ATTRTAB, bBuf) <= 0)
	{
		*pbRes = 3;	//写失败 （3）？？？
		return -1;
	}
	else
	{
		TrigerSaveBank(BN0, SECT5, -1);
		*pbRes = 0;	//成功  （0） 返回结果
		return 0;
	}
}


bool GetFrzTaskFileName(WORD wOI, DWORD dwROAD, char* pszFileName)
{
	WORD i;		

	for (i=0; i<FRZ_TASK_NUM; i++)
	{
		TFrzCtrl* pFrzCtrl = &g_FrzCtrl[i];
		TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
		if (wOI==pFrzCtrl->wOI && dwROAD==pFrzCfg->dwOA && IsFrzTaskCfgValid(pFrzCtrl))
		{
			sprintf(pszFileName, FMT_FRZ_TASK_TABLE, wOI, pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);
			return true;
		}
	}

	return false;
}



//给协议层GetRequestNormal方式读记录的读接口函数
//返回：>0返回读到冻结数据的长度； <=0:失败
int ReadFrzData(DWORD dwOAD, BYTE* pbBuf, WORD wBufSize, int* piStart)
{
	int iLen, iCnt;
	bool fGetHead = false, fSameCfg = false;
	BYTE bIndex, bRecPtr, bType;
	WORD i, wOI, wDataLen, wFmtLen, wItemOffset, wItemLen;
	DWORD dwSubOAD, dwRecSec;
	BYTE bBuf[FRZRELA_ID_LEN];		//OAD个数*(sizeof(FRZRELA)+2) + 数组类型1 + 数组元素个数1
	char szTableName[TASK_PATH_LEN];
	TFieldParser tDataFields = { bBuf };
	TTime tmRec = { 0 };
	BYTE bCfgBuf[20];
	BYTE bRecBuf[FRZ_REC_LEN];
	BYTE* pbBuf0 = pbBuf;
	BYTE* pbFmt = NULL;

	wOI = (dwOAD>>16) & 0xffff;
	bRecPtr = dwOAD & 0xff;		//最近第N笔记录
	if (bRecPtr == 0)
		return 0;

	iLen = OoReadAttr(wOI, FRZ_ATTRTAB, tDataFields.pbCfg, &pbFmt, &wFmtLen);		//配置字段
	if (iLen>0 && tDataFields.pbCfg[1]>0 && tDataFields.pbCfg[1]<=CAP_OAD_NUM)
	{
		tDataFields.wCfgLen = tDataFields.pbCfg[1]*DT_FRZRELA_LEN + 2;
		if (OoParseField(&tDataFields, pbFmt, wFmtLen, false) == false)
			return 0;

		fGetHead = false;		

		for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	//关联对象个数
		{
			memset(bCfgBuf, 0, sizeof(bCfgBuf));
			iLen = ReadParserField(&tDataFields, bIndex, bCfgBuf, &bType, &wItemOffset, &wItemLen);		//读出配置的冻结参数
			if (iLen > 0)
			{
				wDataLen = OoGetDataLen(DT_OAD, bCfgBuf+OFFSET_FRZ_OAD);
				if (wDataLen <= 0)		//关联对象OAD有效
				{
					*pbBuf++ = DT_NULL;		//无效数据
					continue;
				}

				dwSubOAD = OoOadToDWord(bCfgBuf+OFFSET_FRZ_OAD);
				for (i=0; i<FRZ_TASK_NUM; i++)
				{
					TFrzCtrl* pFrzCtrl = &g_FrzCtrl[i];
					TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
					fSameCfg = (dwSubOAD&~OAD_FEAT_MASK)==(dwOAD&~OAD_FEAT_MASK);	//方案号是否一致
					if (wOI==pFrzCtrl->wOI && dwSubOAD==pFrzCfg->dwOA && IsFrzTaskCfgValid(pFrzCtrl) && fSameCfg)		//OI类型和OAD(方案号)都相同，按照参数中关联属性表OAD顺序读取记录
					{
						sprintf(szTableName, FMT_FRZ_TASK_TABLE, wOI, pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);
						iLen = ReadLastNRec(szTableName, bRecPtr, bRecBuf, sizeof(bRecBuf));
						if (iLen > 0)
						{
							if (fGetHead == false)	//是否已经获取记录序号和存储时间头
							{
								fGetHead = true;
								memcpy(pbBuf, bRecBuf, iLen);
								OoDateTimeSToTime(bRecBuf+REC_TIME_OFFSET, &tmRec);
								dwRecSec = TimeToSeconds(tmRec);

								pbBuf += iLen;
							}
							else	//只取OAD数据
							{
								OoDateTimeSToTime(bRecBuf+REC_TIME_OFFSET, &tmRec);
								if (GetAbsGap(dwRecSec, TimeToSeconds(tmRec)) > MAX_GAP_SEC)	//同一个记录序号，但记录时间相差太大 则置无效数据
								{
									*pbBuf++ = DT_NULL;		//无效数据
								}
								else
								{
									iCnt = pbBuf-pbBuf0;
									if (iCnt+wDataLen >= wBufSize)	//buf空间不够
									{
										DTRACE(DB_TASK, ("ReadFrzData Buf is full, dwOAD=%04x, nLen=%ld + wDataLen=%ld > wBufSize=%ld.\r\n", dwOAD, iCnt, wDataLen, wBufSize));
										return 0;
									}

									memcpy(pbBuf, bRecBuf+REC_FRZ_DATA_OFFSET, wDataLen);
									pbBuf += wDataLen;
								}
							}
						}
						else
						{
							*pbBuf++ = DT_NULL;		//无效数据
						}

						break;	//找到目标，退出for (i=0; i<FRZ_TASK_NUM; i++)循环
					}
				}
			}
		}
	}

	//*piStart++;	//是否需要增加？
	return (int)(pbBuf-pbBuf0);
}


//是否冻结了统计数据
bool IsFrzStatData()
{
	WORD i;
	TFrzCfg* pFrzCfg = NULL;

	if (!g_fFrzInit)	//冻结未初始化时返回true
		return true;

	for (i=0; i<FRZ_TASK_NUM; i++)
	{
		if (IsFrzTaskCfgValid(&g_FrzCtrl[i]))
		{
			pFrzCfg = &g_FrzCtrl[i].tCfg;
			if ((pFrzCfg->dwOA&OAD_OI_MASK) == 0x21000200 || (pFrzCfg->dwOA&OAD_OI_MASK) == 0x22000200)		//是否冻结了统计数据
				return true;
		}
	}

	return false;
}

//上行协议冻结参数变更时触发时调用
void OnStatParaChg()
{
	if (IsFrzStatData())
		SetInfo(INFO_FRZPARA_CHG);
}




//描述：获取事件记录表的固定字段和数据字段，提供给上行接口
bool GetFrzTaskRecFieldParser(DWORD dwROAD, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAttrTabBuf, WORD wBufSize)	
{
	BYTE* pbFmt;
	WORD wOI, wFmtLen = 0;
	int iLen;
	BYTE* pbCfg0 = NULL;

	//GetOIAttrIndex(dwROAD, &wOI, NULL, NULL);		

	// 固定字段
	if (pFixFields != NULL)
	{
		memset((BYTE*)pFixFields, 0, sizeof(TFieldParser));
		pFixFields->pbCfg = g_bFrzFixOAList;
		pFixFields->wCfgLen = sizeof(g_bFrzFixOAList);
		if (OoParseField(pFixFields, g_bFrzFixFmt, sizeof(g_bFrzFixFmt), true) == false)	//必有固定字段
		{	
			DTRACE(DB_TASK, ("GetFrzTaskFieldParser: wOI=%u OoParseField FixFields fail.\r\n", wOI));
			return false;
		}
	}

	//数据字段，关联属性表可以为NULL
	if (pDataFields != NULL)
	{
		pbCfg0 = pDataFields->pbCfg;
		if (pbCfg0 == NULL)
			return false;

		memset((BYTE*)pDataFields, 0, sizeof(TFieldParser));
		pDataFields->pbCfg = pbCfg0;
		pDataFields->pbCfg[0] = 0x01;	//数组类型
		pDataFields->pbCfg[1] = 0x01;	//数组元素个数
		pDataFields->pbCfg[2] = DT_OAD;	//数据类型

		OoDWordToOad(dwROAD, &pDataFields->pbCfg[3]);	//OAD			
					
		pDataFields->wCfgLen = SUB_DATD_FIELD_LEN;

		if (OoParseField(pDataFields, g_bFrzSubDataFmt, sizeof(g_bFrzSubDataFmt), true) == false)		//配置字段
		{
			DTRACE(DB_TASK, ("GetFrzTaskFieldParser: dwROAD=%04x OoParseField DataFields fail.\r\n", dwROAD));
			return false;
		}
	}

	return true;
}


//收到参数/数据/事件记录初始化命令时的处理，外部调用
void FrzTaskOnRxFaResetCmd()
{
	WORD wPn;
	BYTE bFrzType;
	BYTE bBuf[10];

	for (bFrzType=0; bFrzType<FRZ_TYPE_NUM; bFrzType++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		bBuf[0] = bFrzType;
		bBuf[1] = FRZ_CLR_VALID;
		wPn = bFrzType;
		WriteItemEx(BN11, wPn, 0x0b11, bBuf);		//写清零标识
	}

	//TrigerSaveBank(BN11, 0, -1);
	SetInfo(INFO_FRZDATA_RESET);
}
