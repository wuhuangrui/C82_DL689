/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CctRdCtrl.cpp
 * 摘    要：载波抄表控制管理
 * 当前版本：1.0
 * 作    者：CL
 * 完成日期：2016年8月
 *********************************************************************************************************/
#include "stdafx.h"
#include "sysfs.h"
#include <fcntl.h>
#include "CctTaskMangerOob.h"
#include "LibDbConst.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "CctAPI.h"
#include "TaskDB.h"
#include "SchParaCfg.h"
#include "DbOIAPI.h"
#include "TaskManager.h"
#include "OIObjInfo.h"
#include "FrzTask.h"
#include "Trace.h"

TSem	g_semMtrUdp;	//电表档案更新
TSem	g_semTskCfg;	//任务配置单元
TSem	g_semSchCfg;	//采集方案配置

TMemMalloc g_TaskMem[TASK_ID_NUM];
TMemMalloc g_SchMem[TASK_ID_NUM];

//描述：初始化电表屏蔽字
void InitMtrMask()
{
	TOobMtrInfo tTMtrInfo;
	BYTE bMtrMask[PN_MASK_SIZE] = {0};
	BYTE bPlcMtrMask[PN_MASK_SIZE] = {0};
	BYTE bPlcAcqMask[PN_MASK_SIZE] = {0};
	BYTE b485MtrMask[PN_MASK_SIZE] = {0};
	BYTE bMask, bBit;

	for (WORD wPn=0; wPn<POINT_NUM; wPn++)
	{
		memset((BYTE*)&tTMtrInfo, 0, sizeof(tTMtrInfo));
		if (GetMeterInfo(wPn, &tTMtrInfo))
		{	
			bMask = tTMtrInfo.wPn/8;
			bBit = tTMtrInfo.wPn%8;
			bMtrMask[bMask] |= 1<<bBit;

			if (IsAllAByte(tTMtrInfo.bTsa, 0xee, tTMtrInfo.bTsaLen) || IsAllAByte(tTMtrInfo.bTsa, 0x00, tTMtrInfo.bTsaLen))
				continue;

			if ((tTMtrInfo.dwPortOAD&0xffffff00) == 0xf2010200)
			{
				b485MtrMask[bMask] |= 1<<bBit;
			}
			else if ((tTMtrInfo.dwPortOAD&0xffffff00) == 0xf2090200)
			{
				bPlcMtrMask[bMask] |= 1<<bBit;
				if (tTMtrInfo.bAcqTsaLen!=0 && !IsAllAByte(tTMtrInfo.bAcqTsa, 0, tTMtrInfo.bAcqTsaLen))
					bPlcAcqMask[wPn/8] |= 1<<bBit;
			}
		}
	}

	WriteItemEx(BANK17, PN0, 0x6001, bMtrMask);
	WriteItemEx(BANK17, PN0, 0x6002, b485MtrMask);
	WriteItemEx(BANK17, PN0, 0x6003, bPlcMtrMask);
	WriteItemEx(BANK17, PN0, 0x6004, bPlcAcqMask);

	DTRACE(DB_CRITICAL, ("InitMtrMask(): Init meter mask complete.\r\n"));
}

//描述：获取电表屏蔽字
//返回：电表屏蔽字缓冲区指针
const BYTE * GetMtrMask(BYTE bBank, WORD wPn, WORD wID)
{
	return GetItemRdAddr(bBank, wPn, wID);
}

//描述：初始化任务配置单元映射
void InitTaskMap()
{
	int iRetLen;
	BYTE bBuf[1024];

	WaitSemaphore(g_semTskCfg);
	for (WORD i=0; i<TASK_ID_NUM; i++)
	{
		if (g_TaskMem[i].pbCfg != NULL)
		{
			free(g_TaskMem[i].pbCfg);
			g_TaskMem[i].pbCfg = NULL;
		}
	}

	for (WORD i=0; i<TASK_ID_NUM; i++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		if ((iRetLen=GetTaskConfigFromTaskDb(i, bBuf)) <= 0)
			continue;
		g_TaskMem[i].pbCfg = (BYTE*)malloc(iRetLen);
		if (g_TaskMem[i].pbCfg == NULL)
		{
			DTRACE(DB_TASK, ("InitTaskMap: bTaskId=%d malloc() error!\n", i));
			continue;
		}
		else
			DTRACE(DB_TASK, ("InitTaskMap: bTaskId=%d malloc() succ.\n", i));
		g_TaskMem[i].wCfgLen = iRetLen;
		memcpy(g_TaskMem[i].pbCfg, bBuf, iRetLen);
	}
	SignalSemaphore(g_semTskCfg);
}

//描述：获取任务配置表
//参数：@bIndex 任务ID索引
//返回：指向任务ID内存映射表
const BYTE* GetTaskCfgTable(WORD wTaskID)
{
	BYTE *pbPtr = NULL;

	WaitSemaphore(g_semTskCfg);

	if (g_TaskMem[wTaskID].pbCfg != NULL)
		pbPtr = g_TaskMem[wTaskID].pbCfg;

	SignalSemaphore(g_semTskCfg);

	return pbPtr;
}

//描述：初始化采集方案参数映射
void InitSchMap()
{
	TTaskCfg tTaskCfg;
	int iRetLen;
	WORD i;
	WORD wValidSchNum;
	BYTE bBuf[1024];

	WaitSemaphore(g_semSchCfg);

	for (i=0; i<TASK_ID_NUM; i++)
	{
		if (g_SchMem[i].pbCfg != NULL)
		{
			free(g_SchMem[i].pbCfg);
			g_SchMem[i].pbCfg = NULL;
			g_SchMem[i].bSchType = 0;
			g_SchMem[i].bSchNo = 0;
		}
	}

	wValidSchNum = 0;
	for (i=0; i<TASK_ID_NUM; i++)
	{
		if (GetTaskCfg(i, (TTaskCfg*)&tTaskCfg))
		{
			if (tTaskCfg.bSchType==SCH_TYPE_TRANS || tTaskCfg.bTaskId==0)
				continue;
			memset(bBuf, 0, sizeof(bBuf));
			if ((iRetLen=GetSchFromTaskDb(tTaskCfg.bSchNo, tTaskCfg.bSchType, bBuf)) <= 0)
				continue;

			g_SchMem[wValidSchNum].wCfgLen = iRetLen;
			g_SchMem[wValidSchNum].pbCfg = (BYTE*)malloc(iRetLen);
			if (g_SchMem[wValidSchNum].pbCfg == NULL)
			{
				DTRACE(DB_TASK, ("InitSchMap: bSchNum=%d malloc error!\n", tTaskCfg.bSchNo));
				continue;
			}
			else
				DTRACE(DB_TASK, ("InitSchMap: bSchNum=%d malloc succ.\n", tTaskCfg.bSchNo));

			memcpy(g_SchMem[wValidSchNum].pbCfg, bBuf, iRetLen);
			g_SchMem[wValidSchNum].bSchType = tTaskCfg.bSchType;
			g_SchMem[wValidSchNum].bSchNo = tTaskCfg.bSchNo;
			wValidSchNum++;
		}
	}

	SignalSemaphore(g_semSchCfg);
}


BYTE* GetSchCfg(TTaskCfg* pTaskCfg, int *iLen)
{
	int i;
	BYTE *pbSch = NULL;

	WaitSemaphore(g_semSchCfg);
	for (i=0; i<TASK_ID_NUM; i++)
	{
		if (g_SchMem[i].pbCfg)
		{
			if (g_SchMem[i].bSchNo==pTaskCfg->bSchNo &&
				g_SchMem[i].bSchType==pTaskCfg->bSchType)
			{
				*iLen = g_SchMem[i].wCfgLen;
				pbSch = g_SchMem[i].pbCfg;
				break;
			}
		}
	}

	SignalSemaphore(g_semSchCfg);

	return pbSch;
}

BYTE* GetSchCfg(BYTE bIndex, int *iLen)
{
	BYTE *pbSch = NULL;

	WaitSemaphore(g_semSchCfg);
	*iLen = g_SchMem[bIndex].wCfgLen;
	pbSch = g_SchMem[bIndex].pbCfg;
	SignalSemaphore(g_semSchCfg);

	return pbSch;
}

//描述: 取得任务配置的个数
//返回: 任务配置的个数
int GetTaskNum()
{
	WORD wNum = 0;

	WaitSemaphore(g_semSchCfg);
	for (WORD i=0; i<TASK_ID_NUM; i++)
	{
		if (g_TaskMem[i].pbCfg)
			wNum++;
	}
	SignalSemaphore(g_semSchCfg);

	return wNum;
}

//描述: 取得指定索引处的任务配置单元
//参数： @bIndex 任务ID的索引
//	    @pTaskCfg用来返回任务的配置，如果
//		@bIsRdTab是否从任务配置表中获取任务配置单元
//返回:如果读到正确任务配置则返回true，否则返回false
bool GetTaskCfg(BYTE bIndex, TTaskCfg *pTaskCfg, bool bIsRdTab)
{
	BYTE bBuf[1024];
	const BYTE *pbPtr = NULL;

	memset((BYTE*)pTaskCfg, 0, sizeof(TTaskCfg));

	if (bIsRdTab)
	{
		memset(bBuf, 0, sizeof(bBuf));
		if ((GetTaskConfigFromTaskDb(bIndex, bBuf)) <= 0)
			return false;
		pbPtr = bBuf;
	}
	else
	{
		pbPtr = GetTaskCfgTable(bIndex);
	}

	if (pbPtr == NULL)
		return false;

	pbPtr++;	//structure
	pbPtr++;	//structure成员个数
	pbPtr++;	//unsigned 任务ID类型
	pTaskCfg->bTaskId = *pbPtr++;
	if (pTaskCfg->bTaskId == 0)
		return false;
	pbPtr++;	//TI 时间间隔
	pTaskCfg->tiExe.bUnit = *pbPtr++;
	pTaskCfg->tiExe.wVal = OoOiToWord((BYTE*)pbPtr);	
	pbPtr += 2;
	pbPtr++;	//enum
	pTaskCfg->bSchType = *pbPtr++;
	if (pTaskCfg->bSchType > sizeof(g_TSchFieldCfg)/sizeof(TSchFieldCfg))
	{
		DTRACE(DB_TASK, ("GetTaskCfg: bSchType=%d invalid, return!!!\n", pTaskCfg->bSchType));
		return false;
	}

	pbPtr++;	//unsigned 
	pTaskCfg->bSchNo = *pbPtr++;
	pbPtr++;	//DateTimeBCD
	OoDateTimeSToTime((BYTE*)pbPtr, &pTaskCfg->tmStart);
	pbPtr += 7;
	pbPtr++;	//DateTimeBCD
	OoDateTimeSToTime((BYTE*)pbPtr, &pTaskCfg->tmEnd);
	pbPtr += 7;
	pbPtr++;	//TI 时间间隔
	pTaskCfg->tiDelay.bUnit = *pbPtr++;
	pTaskCfg->tiDelay.wVal = OoOiToWord((BYTE*)pbPtr);	
	pbPtr += 2;
	pbPtr++;	//unsigned
	pTaskCfg->bPrio = *pbPtr++;
	pbPtr++;	//enum
	pTaskCfg->bState = *pbPtr++;
	pbPtr++;	//long-unsigned
	pTaskCfg->wPreScript = OoOiToWord((BYTE*)pbPtr);	
	pbPtr += 2;
	pbPtr++;	//long-unsigned
	pTaskCfg->wPostScript = OoOiToWord((BYTE*)pbPtr);	
	pbPtr += 2;	
	pbPtr++;	//struct
	pbPtr++;	//struct成员个数
	pbPtr++;
	pTaskCfg->bPeriodType = *pbPtr++;
	pbPtr++;	//array
	pTaskCfg->bPeriodNum = *pbPtr++;
	for (BYTE i=0; i<pTaskCfg->bPeriodNum; i++)
	{
		pbPtr++;	//struct
		pbPtr++;	//struct 成员个数
		pbPtr++;	//unsigned
		pTaskCfg->period[i].bStarHour = *pbPtr++;
		pbPtr++;	//unsigned
		pTaskCfg->period[i].bStarMin = *pbPtr++;
		pbPtr++;	//unsigned
		pTaskCfg->period[i].bEndHour = *pbPtr++;
		pbPtr++;	//unsigned
		pTaskCfg->period[i].bEndMin = *pbPtr++;
	}

	return true;
}

//描述：取得普通采集方案参数
//参数: @pTaskCfg任务的配置
//	     @pTAcqSchCfg用来返回任务的配置
//返回: 为真获取成功
bool GetCommonSchCfg(TTaskCfg* pTaskCfg, TCommAcqSchCfg* pTCommAcqSchCfg, BYTE *pbArryCSD)
{
	int iSchCfgLen = 0;
	const BYTE *pbPtr = NULL;

	memset((BYTE*)pTCommAcqSchCfg, 0, sizeof(TCommAcqSchCfg));
	
	pbPtr = GetSchCfg(pTaskCfg, &iSchCfgLen);
	if (pbPtr==NULL)
		return false;

	pbPtr++;	//structure
	pbPtr++;	//+structure number
	pbPtr++;	//unsigned
	pTCommAcqSchCfg->bSchNo = *pbPtr;	pbPtr++;
	pbPtr++;	//long-unsigned
	pTCommAcqSchCfg->wStgCnt = OoLongUnsignedToWord((BYTE*)pbPtr);	pbPtr+=2;
	pbPtr++;	//structure
	pbPtr++;	//+structure number
	pbPtr++;	//unsigned 
	pTCommAcqSchCfg->tTAcqType.bAcqType = *pbPtr;	pbPtr++;
	switch(pTCommAcqSchCfg->tTAcqType.bAcqType)
	{
	case 0:	//采集当前数据
		pbPtr++;	//NULL
		memset(pTCommAcqSchCfg->tTAcqType.bAcqData, 0, sizeof(pTCommAcqSchCfg->tTAcqType.bAcqData));
		break;
	case 1:	//采集上第N次
		pbPtr++;	//unsigned 
		pTCommAcqSchCfg->tTAcqType.bAcqData[0] = *pbPtr;	pbPtr++;
		break;
	case 2:	//按冻结时标采集
		pbPtr++;	//NULL
		memset(pTCommAcqSchCfg->tTAcqType.bAcqData, 0, sizeof(pTCommAcqSchCfg->tTAcqType.bAcqData));
		break;
	case 3:	
		pbPtr++;	//TI
		pTCommAcqSchCfg->tTAcqType.bAcqData[0] = *pbPtr;	pbPtr++;	//间隔单位
		pbPtr++;	//long-unsigned
		pTCommAcqSchCfg->tTAcqType.bAcqData[1] = *pbPtr;	pbPtr++;	//间隔值
		pTCommAcqSchCfg->tTAcqType.bAcqData[2] = *pbPtr;	pbPtr++;
		break;
	default:
        break;
	}

	//提取array CSD
	pbPtr++;	//array
	pbArryCSD = (BYTE*)pbPtr;
	pTCommAcqSchCfg->bCSDNum = *pbPtr;	pbPtr++;	//+array number
	if (pTCommAcqSchCfg->bCSDNum > (sizeof(pTCommAcqSchCfg->tTCSD)/sizeof(TCSD)))	//异常处理
	{
		DTRACE(DB_CCT, ("ERROR---InitRdMtrCtrl():: Current CSD number=%d, Max support CSD number=%d.\n", \
			pTCommAcqSchCfg->bCSDNum, (sizeof(pTCommAcqSchCfg->tTCSD)/sizeof(TCSD))));
		pTCommAcqSchCfg->bCSDNum = sizeof(pTCommAcqSchCfg->tTCSD)/sizeof(TCSD);
	}
	for (BYTE i = 0; i < pTCommAcqSchCfg->bCSDNum; i++)
	{
		pbPtr++;	//CSD
		pTCommAcqSchCfg->tTCSD[i].bChoice = *pbPtr;	pbPtr++;	//列选择类型 [0]:OAD, [1]:ROAD
		if (pTCommAcqSchCfg->tTCSD[i].bChoice == 0)	//OAD
		{
			pTCommAcqSchCfg->tTCSD[i].dwOAD = OoOadToDWord((BYTE*)pbPtr);	pbPtr += 4;
		}
		else if (pTCommAcqSchCfg->tTCSD[i].bChoice == 1)	//ROAD
		{
			pTCommAcqSchCfg->tTCSD[i].tTROAD.dwOAD = OoOadToDWord((BYTE*)pbPtr);	pbPtr += 4;	//OAD
			pTCommAcqSchCfg->tTCSD[i].tTROAD.bOADNum = *pbPtr;	pbPtr++;	///关联对象属性OAD个数
			if (pTCommAcqSchCfg->tTCSD[i].tTROAD.bOADNum > (sizeof(pTCommAcqSchCfg->tTCSD[i].tTROAD.dwOADArry)/(sizeof(DWORD))))	//异常处理
			{
				//DTRACE(DB_CCT, ("ERROR---InitRdMtrCtrl():: Current RCD->bOADNum =%d, Max support bOADNum =%d.\n", pTCommAcqSchCfg->tTCSD[i].tTROAD.bOADNum, sizeof(pTCommAcqSchCfg->tTCSD[i].tTROAD.dwOADArry)/(DWORD)));

				pTCommAcqSchCfg->tTCSD[i].tTROAD.bOADNum = sizeof(pTCommAcqSchCfg->tTCSD[i].tTROAD.dwOADArry)/(sizeof(DWORD));
			}
			for (BYTE j = 0; j < pTCommAcqSchCfg->tTCSD[i].tTROAD.bOADNum; j++)
			{
				pTCommAcqSchCfg->tTCSD[i].tTROAD.dwOADArry[j] = OoOadToDWord((BYTE*)pbPtr);	pbPtr += 4;
			}
		}
		else
		{
			DTRACE(DB_CCT, ("ERROR---InitRdMtrCtrl(): Choise is 0 or 1, but pTAcqSchCfg->tTCSD[%d].bChoice is =%d.\n", \
				i, pTCommAcqSchCfg->tTCSD[i].bChoice));
			return false;
		}
	}

	//提取MS集合
	// 		BYTE bMS = *pbPtr++;	//MS
	// 		if (bMS != 92)	//MS数据类型
	// 			return false;
	pTCommAcqSchCfg->bMsChoice = *(pbPtr + 1);
	int iRet = ParserMsParam((BYTE*)pbPtr, pTCommAcqSchCfg->bMtrMask, sizeof(pTCommAcqSchCfg->bMtrMask));
	if (iRet < 0)
		return false;
	pbPtr += iRet;

	pbPtr++;	//enum
	//pbPtr++;	//long-unsigned
	pTCommAcqSchCfg->bStgTimeScale = *pbPtr++;

	return true;
}

//描述：取得事件采集方案参数
//参数: @index事件采集方案号
//	     @pTAcqSchCfg用来返回任务的配置
//返回: 为真获取成功
bool GetEventSchCfg(BYTE bIndex, TEvtAcqSchCfg* pTEvtAcqSchCfg)
{
	int iSchCfgLen = 0;
	int iDbRet = 0;

	if (pTEvtAcqSchCfg != NULL)
	{
		BYTE bBuf[256];
		BYTE *pbPtr = bBuf;

		//pbPtr = GetSchCfg(pTaskCfg, &iSchCfgLen);
		iDbRet=GetSchFromTaskDb(bIndex, g_TSchFieldCfg[1].bSchType, bBuf);
		if (iDbRet<0)
			return false;

		//pbPtr++;	//array
		//pbPtr++;	//array个数
		pbPtr++;	//structure
		pbPtr++;	//+structure number
		pbPtr++;	//unsigned
		pTEvtAcqSchCfg->bSchNo = *pbPtr;	
		pbPtr++;
		
		pbPtr++;	//array
		pbPtr++;
		
		pbPtr++;   //采集类型
		pbPtr++; 

		pbPtr++; //array
		pTEvtAcqSchCfg->bROADNum = *pbPtr;	
		pbPtr++;	//+array 成员个数
		if (pTEvtAcqSchCfg->bROADNum > sizeof(pTEvtAcqSchCfg->tTROAD)/sizeof(TROAD))
		{
			DTRACE(DB_TASK, ("ERROR---GetEventSchCfg():  bSchNo=%d, Max support ROAD num=%d, current ROAD num=%d.\n", \
				 pTEvtAcqSchCfg->bSchNo, sizeof(pTEvtAcqSchCfg->tTROAD)/sizeof(TROAD), pTEvtAcqSchCfg->bROADNum));
			return false;
		}
		for (BYTE i = 0; i < pTEvtAcqSchCfg->bROADNum; i++)
		{
			pbPtr++;	//ROAD类型
			pTEvtAcqSchCfg->tTROAD[i].dwOAD = OoOadToDWord((BYTE*)pbPtr);	
			pbPtr += 4;
			pTEvtAcqSchCfg->tTROAD[i].bOADNum = *pbPtr;	
			pbPtr++;
			if (pTEvtAcqSchCfg->tTROAD[i].bOADNum > sizeof(pTEvtAcqSchCfg->tTROAD[i].dwOADArry)/sizeof(DWORD))
			{
				DTRACE(DB_TASK, ("ERROR---GetEventSchCfg():  bSchNo=%d, i=%d, Max support OAD num=%d, current OAD num=%d.\n", \
					 pTEvtAcqSchCfg->bSchNo, sizeof(pTEvtAcqSchCfg->tTROAD[i].dwOADArry)/sizeof(DWORD), pTEvtAcqSchCfg->tTROAD[i].bOADNum));
				return false;
			}
			for (BYTE j = 0; j < pTEvtAcqSchCfg->tTROAD[i].bOADNum; j++)
			{
				pTEvtAcqSchCfg->tTROAD[i].dwOADArry[j] = OoOadToDWord((BYTE*)pbPtr);	
				pbPtr += 4;
			}
		}
		//提取MS集合
		BYTE bMS = *pbPtr;	//MS
		if (bMS != 92)	//MS数据类型
			return false;
		pTEvtAcqSchCfg->bMsChoice = *(pbPtr + 1);
		int iRet = ParserMsParam((BYTE*)pbPtr, pTEvtAcqSchCfg->bMtrMask, sizeof(pTEvtAcqSchCfg->bMtrMask));
		if (iRet < 0)
			return false;
		pbPtr += iRet;

		//上报标识
		pbPtr++;	//bool
		pTEvtAcqSchCfg->fRptFlg = *pbPtr;	
		pbPtr++;

		//存储深度
		pbPtr++;	//long-unsigned
		pTEvtAcqSchCfg->wStgCnt = OoLongUnsignedToWord((BYTE*)pbPtr);	
		pbPtr++;
	}

	return true;
}

//描述：取得事件采集方案参数
//参数: @pTaskCfg任务的配置
//	     @pTAcqSchCfg用来返回任务的配置
//返回: 为真获取成功
bool GetEventSchCfg(TTaskCfg* pTaskCfg, TEvtAcqSchCfg* pTEvtAcqSchCfg)
{
	int iSchCfgLen = 0;

	if (pTEvtAcqSchCfg != NULL)
	{
		BYTE bBuf[256];
		BYTE *pbPtr = bBuf;

		pbPtr = GetSchCfg(pTaskCfg, &iSchCfgLen);
		if (pbPtr==NULL)
			return false;

		//pbPtr++;	//array
		//pbPtr++;	//array个数
		pbPtr++;	//structure
		pbPtr++;	//+structure number
		pbPtr++;	//unsigned
		pTEvtAcqSchCfg->bSchNo = *pbPtr;	
		pbPtr++;
		pbPtr++;	//array
		pTEvtAcqSchCfg->bROADNum = *pbPtr;	
		pbPtr++;	//+array 成员个数
		if (pTEvtAcqSchCfg->bROADNum > sizeof(pTEvtAcqSchCfg->tTROAD)/sizeof(TROAD))
		{
			DTRACE(DB_TASK, ("ERROR---GetEventSchCfg(): bTaskID=%d, bSchNo=%d, Max support ROAD num=%d, current ROAD num=%d.\n", \
				pTaskCfg->bTaskId, pTEvtAcqSchCfg->bSchNo, sizeof(pTEvtAcqSchCfg->tTROAD)/sizeof(TROAD), pTEvtAcqSchCfg->bROADNum));
			return false;
		}
		for (BYTE i = 0; i < pTEvtAcqSchCfg->bROADNum; i++)
		{
			pbPtr++;	//ROAD类型
			pTEvtAcqSchCfg->tTROAD[i].dwOAD = OoOadToDWord((BYTE*)pbPtr);	
			pbPtr += 4;
			pTEvtAcqSchCfg->tTROAD[i].bOADNum = *pbPtr;	
			pbPtr++;
			if (pTEvtAcqSchCfg->tTROAD[i].bOADNum > sizeof(pTEvtAcqSchCfg->tTROAD[i].dwOADArry)/sizeof(DWORD))
			{
				DTRACE(DB_TASK, ("ERROR---GetEventSchCfg(): bTaskID=%d, bSchNo=%d, i=%d, Max support OAD num=%d, current OAD num=%d.\n", \
					pTaskCfg->bTaskId, pTEvtAcqSchCfg->bSchNo, sizeof(pTEvtAcqSchCfg->tTROAD[i].dwOADArry)/sizeof(DWORD), pTEvtAcqSchCfg->tTROAD[i].bOADNum));
				return false;
			}
			for (BYTE j = 0; j < pTEvtAcqSchCfg->tTROAD[i].bOADNum; j++)
			{
				pTEvtAcqSchCfg->tTROAD[i].dwOADArry[j] = OoOadToDWord((BYTE*)pbPtr);	
				pbPtr += 4;
			}
		}
		//提取MS集合
		BYTE bMS = *pbPtr;	//MS
		if (bMS != 92)	//MS数据类型
			return false;
		pTEvtAcqSchCfg->bMsChoice = *(pbPtr + 1);
		int iRet = ParserMsParam((BYTE*)pbPtr, pTEvtAcqSchCfg->bMtrMask, sizeof(pTEvtAcqSchCfg->bMtrMask));
		if (iRet < 0)
			return false;
		pbPtr += iRet;

		//上报标识
		pbPtr++;	//bool
		pTEvtAcqSchCfg->fRptFlg = *pbPtr;	
		pbPtr++;

		//存储深度
		pbPtr++;	//long-unsigned
		pTEvtAcqSchCfg->wStgCnt = OoLongUnsignedToWord((BYTE*)pbPtr);	
		pbPtr++;
	}

	return true;
}

//描述：获取任务执行周期单位是否小于小时
bool GetTaskCyleUnit(TMtrRdCtrl* pMtrRdCtrl)
{
	TTaskCfg tTaskCfg;

	for (BYTE i=0; i<MTR_TASK_NUM; i++)
	{
		if (pMtrRdCtrl->taskSucFlg[i].bTaskId != 0)
		{
			memset((BYTE*)&tTaskCfg, 0, sizeof(tTaskCfg));
			if (GetTaskCfg(pMtrRdCtrl->taskSucFlg[i].bTaskId, &tTaskCfg))
			{
				if (tTaskCfg.tiExe.bUnit <= TIME_UNIT_HOUR)
					return true;
			}
		}
	}

	return false;
}

//描述: 取得指定任务配置序列号
//参数： @无
//返回:任务序列号
BYTE GetTaskCfgSn()
{
	BYTE bTaskSN = 0;

	//ReadItemEx(BANK16, PN0, 0x6011, &bTaskSN);

	return bTaskSN;
}

//描述：分析MS参数
//参数：@pbBuf 为MS中的数据类型定义
//		@pbMtrMask 对MS解析之后提取的电表地址屏蔽字
//		@wMtrMaskLen 电表屏蔽字长度
//返回：返回MS数据内容的长度
int ParserMsParam(BYTE *pbBuf, BYTE *pbMtrMask, WORD wMtrMaskLen)
{
	int iCmpRet1, iCmpRet2;
	BYTE bMtrMask[PN_MASK_SIZE];
	BYTE *pbPtr = pbBuf;
	BYTE bMsChoice;
	BYTE bGroupMtrRegionCnt = 0;

	if (*pbPtr == DT_MS)	//ms
		pbPtr++;

	memset(bMtrMask, 0, sizeof(bMtrMask));
	memcpy(bMtrMask, GetMtrMask(BANK17, PN0, 0x6001), PN_MASK_SIZE);

	bMsChoice = *pbPtr++;
	switch(bMsChoice)
	{
	case 0:	//无电能表
		memset(pbMtrMask, 0, PN_MASK_SIZE);
		break;
	case 1:	//全部用户地址
		memcpy(pbMtrMask, bMtrMask, PN_MASK_SIZE);
		break;
	case 2:	//一组用户类型
		BYTE bUserTypeArry[MS_ONE_GROUP_USER_TYPE];	//定义8个用户类型
		BYTE bUserTypeCnt;

		memset(bUserTypeArry, 0, sizeof(bUserTypeArry));
		bUserTypeCnt = *pbPtr++;
		if (bUserTypeCnt > sizeof(bUserTypeArry))
			bUserTypeCnt = sizeof(bUserTypeArry);
		for (BYTE bUserIndex=0; bUserIndex<bUserTypeCnt; bUserIndex++)
			bUserTypeArry[bUserIndex] = *pbPtr++;

		for (WORD wMtrMask=0; wMtrMask<PN_MASK_SIZE; wMtrMask++)
		{
			if (bMtrMask[wMtrMask] != 0)
			{
				for (BYTE bBit=0; bBit<8; bBit++)
				{
					if (bMtrMask[wMtrMask] & (1<<(bBit)))
					{
						WORD wMtrSn = wMtrMask*8 + bBit;
						for (BYTE bUserIndex=0; bUserIndex<bUserTypeCnt; bUserIndex++)
						{
							if (GetMeterUserType(wMtrSn) == bUserTypeArry[bUserIndex])
							{
								pbMtrMask[wMtrMask] |= (1<<bBit);
								break;
							}
						}
					}
				}
			}
		}
		break;

	case 3:	//一组用户地址
		BYTE bTsa[10];
		BYTE bTsaLen;
		BYTE bTsaCnt;	

		bTsaCnt = *pbPtr++;
		for (BYTE bTsaCntIndex=0; bTsaCntIndex<bTsaCnt; bTsaCntIndex++)
		{
			*pbPtr++;	//跳过TSA的长度
			bTsaLen = *pbPtr++ + 1;
			memcpy(bTsa, pbPtr, bTsaLen);	
			pbPtr += bTsaLen;
			for (WORD wMtrMask=0; wMtrMask<PN_MASK_SIZE; wMtrMask++)
			{
				if (bMtrMask[wMtrMask] != 0)
				{
					bool fIsFindVlidMtr = false;
					for (BYTE bBit=0; bBit<8; bBit++)
					{
						if (bMtrMask[wMtrMask] & (1<<(bBit)))
						{
							TOobMtrInfo tMtrInfo;
							WORD wPn;

							wPn = wMtrMask*8 + bBit;
							memset((BYTE*)&tMtrInfo, 0, sizeof(tMtrInfo));
							GetMeterInfo(wPn, &tMtrInfo);
							if (bTsaLen==tMtrInfo.bTsaLen && memcmp(bTsa, tMtrInfo.bTsa, bTsaLen)==0)
							{
								pbMtrMask[wMtrMask] |= (1<<bBit);
								fIsFindVlidMtr = true;
							}
						}
					}
					if (fIsFindVlidMtr)
						break;
				}
			}
		}
		break;
	case 4:	//一组配置序号
		WORD wMtrSnCnt;
		WORD wMtrSn;

		wMtrSnCnt = *pbPtr++;
		for (WORD wMtrSnIndex=0; wMtrSnIndex<wMtrSnCnt; wMtrSnIndex++)
		{
			wMtrSn = OoOiToWord(pbPtr);	
			pbPtr += 2;
			for (WORD wMtrMask=0; wMtrMask<PN_MASK_SIZE; wMtrMask++)
			{
				bool fIsVlidFlg = false;

				if (bMtrMask[wMtrMask] != 0)
				{
					BYTE bBit; 
					for (bBit=0; bBit<8; bBit++)
					{
						if (bMtrMask[wMtrMask] & (1<<(bBit)))
						{
							TOobMtrInfo tMtrInfo;
							WORD wPn;

							wPn = wMtrMask*8 + bBit;
							memset((BYTE*)&tMtrInfo, 0, sizeof(tMtrInfo));
							GetMeterInfo(wPn, &tMtrInfo);
							if (tMtrInfo.wMtrSn == wMtrSn)
							{
								fIsVlidFlg = true;
								break;
							}
						}
					}
					if (fIsVlidFlg)
					{
						pbMtrMask[wMtrMask] |= (1<<bBit);
						break;
					}
				}
			}
		}
		break;

	case 5:	//一组用户类型区间
		//BYTE bUserRegion[4][3];	//暂定义4个用户区间
		BYTE bUserRegionCnt;
		TUserRegion tTUserRegion[MS_ONE_GROUP_USER_TYPE_REGION];

		memset((BYTE*)&tTUserRegion, 0, sizeof(tTUserRegion));
		bUserRegionCnt = *pbPtr++;
		if (bUserRegionCnt > sizeof(tTUserRegion))
			bUserRegionCnt = sizeof(tTUserRegion);

		for (BYTE bUserRegionIndex=0; bUserRegionIndex<bUserRegionCnt; bUserRegionIndex++)
		{
			tTUserRegion[bUserRegionIndex].bRegionType = *pbPtr++;	//区间，0-前闭后开，1-前开后闭，2-前闭后闭，3-前开后开
			pbPtr++;	//unsigned
			tTUserRegion[bUserRegionIndex].bUserStart = *pbPtr++;
			pbPtr++;	//unsigned
			tTUserRegion[bUserRegionIndex].bUserEnd = *pbPtr++;
		}

		for (WORD wMtrMask=0; wMtrMask<PN_MASK_SIZE; wMtrMask++)
		{
			if (bMtrMask[wMtrMask] != 0)
			{
				for (BYTE bBit=0; bBit<8; bBit++)
				{
					if (bMtrMask[wMtrMask] & (1<<(bBit)))
					{
						WORD wMtrSn = wMtrMask*8 + bBit;
						BYTE bUserType = GetMeterUserType(wMtrSn);
						bool fIsFindValidUser = false;

						for (BYTE bUserRegionIndex=0; bUserRegionIndex<bUserRegionCnt; bUserRegionIndex++)
						{
							if (tTUserRegion[bUserRegionIndex].bRegionType == 0)	//0-前闭后开
							{
								if ((bUserType>=tTUserRegion[bUserRegionIndex].bUserStart) 
									&& (bUserType<tTUserRegion[bUserRegionIndex].bUserEnd))
								{
									fIsFindValidUser = true;
									break;
								}
								else
									continue;
							}
							else if (tTUserRegion[bUserRegionIndex].bRegionType == 1)	//1-前开后闭
							{
								if ((bUserType>tTUserRegion[bUserRegionIndex].bUserStart) 
									&& (bUserType<=tTUserRegion[bUserRegionIndex].bUserEnd))
								{
									fIsFindValidUser = true;
									break;
								}
								else
									continue;
							}
							else if (tTUserRegion[bUserRegionIndex].bRegionType == 2)	//2-前闭后闭
							{
								if ((bUserType>=tTUserRegion[bUserRegionIndex].bUserStart)
									&& (bUserType<=tTUserRegion[bUserRegionIndex].bUserEnd))
								{
									fIsFindValidUser = true;
									break;
								}
								else
									continue;
							}
							else if (tTUserRegion[bUserRegionIndex].bRegionType == 3)	//3-前开后开
							{
								if ((bUserType>tTUserRegion[bUserRegionIndex].bUserStart)
									&& (bUserType<tTUserRegion[bUserRegionIndex].bUserEnd))
								{
									fIsFindValidUser = true;
									break;
								}
								else
									continue;
							}
						}
						if (fIsFindValidUser)
						{
							pbMtrMask[wMtrMask] |= (1<<bBit);
						}
					}
				}
			}
		}

		break;

	case 6:	//一组用户地址区间
		TTsaRegion tTTsaRegion[4];
		BYTE bTsaRegionCnt;

		memset((BYTE*)&tTTsaRegion, 0, sizeof(TTsaRegion));
		bTsaRegionCnt = *pbPtr++;
		if (bTsaRegionCnt > 4)
			bTsaRegionCnt = 4;

		for (BYTE bTsaRegionIndex=0; bTsaRegionIndex<bTsaRegionCnt; bTsaRegionIndex++)
		{
			tTTsaRegion[bTsaRegionIndex].bRegionType = *pbPtr++;
			pbPtr++;	//tsa
			*pbPtr++;	//跳过TSA的长度
			bTsaLen = *pbPtr++ + 1;
			tTTsaRegion[bTsaRegionIndex].bStartTsaLen = bTsaLen;
			memcpy(tTTsaRegion[bTsaRegionIndex].bTsaStart, pbPtr, bTsaLen);	
			pbPtr += bTsaLen;
			pbPtr++;	//tsa
			*pbPtr++;	//跳过TSA的长度
			bTsaLen = *pbPtr++ + 1;
			tTTsaRegion[bTsaRegionIndex].bEndTsaLen = bTsaLen;
			memcpy(tTTsaRegion[bTsaRegionIndex].bTsaEnd, pbPtr, bTsaLen);	
			pbPtr += bTsaLen;
		}

		for (WORD wMtrMask=0; wMtrMask<PN_MASK_SIZE; wMtrMask++)
		{
			if (bMtrMask[wMtrMask] != 0)
			{
				for (BYTE bBit=0; bBit<8; bBit++)
				{
					if (bMtrMask[wMtrMask] & (1<<(bBit)))
					{
						WORD wMtrSn = wMtrMask*8 + bBit;
						BYTE bDbTsa[TSA_LEN];
						BYTE bDbTsaLen;
						bool fIsFindValidTsa = false;

						bDbTsaLen = GetMeterTsa(wMtrSn, bDbTsa);
						for (BYTE bTsaRegionIndex=0; bTsaRegionIndex<bTsaRegionCnt; bTsaRegionIndex++)
						{
							iCmpRet1 = AarryCompare(bDbTsa, bDbTsaLen, tTTsaRegion[bTsaRegionIndex].bTsaStart, tTTsaRegion[bTsaRegionIndex].bStartTsaLen);
							iCmpRet2 = AarryCompare(bDbTsa, bDbTsaLen, tTTsaRegion[bTsaRegionIndex].bTsaEnd, tTTsaRegion[bTsaRegionIndex].bEndTsaLen);

							if (tTTsaRegion[bTsaRegionIndex].bRegionType == 0)	//0-前闭后开
							{
								if ((iCmpRet1==0 || iCmpRet1==1) && (iCmpRet2==2))
									fIsFindValidTsa = true;
							}
							else if (tTTsaRegion[bTsaRegionIndex].bRegionType == 1)	//1-前开后闭
							{
								if ((iCmpRet1==1) && (iCmpRet2==0 || iCmpRet2==2))
									fIsFindValidTsa = true;
							}
							else if (tTTsaRegion[bTsaRegionIndex].bRegionType == 2)	//2-前闭后闭
							{
								if ((iCmpRet1==0 || iCmpRet1==1) && (iCmpRet2==0 || iCmpRet2==2))
									fIsFindValidTsa = true;
							}
							else if (tTTsaRegion[bTsaRegionIndex].bRegionType == 3)	//3-前开后开
							{
								if ((iCmpRet1==1) && (iCmpRet2==2))
									fIsFindValidTsa = true;
							}
						}

						if (fIsFindValidTsa)
						{
							pbMtrMask[wMtrMask] |= (1<<bBit);
						}
					}
				}
			}
		}

		break;

	case 7:	//一组配置序号区间
		TCfgMtrSnRegion tTCfgMtrSnRegion[4];
		bGroupMtrRegionCnt = *pbPtr++;

		memset((BYTE*)&tTCfgMtrSnRegion, 0, sizeof(tTCfgMtrSnRegion));
		if (bGroupMtrRegionCnt > 4)
			bGroupMtrRegionCnt = 4;
		for (BYTE bGroupMtrIndex=0; bGroupMtrIndex<bGroupMtrRegionCnt; bGroupMtrIndex++)
		{
			tTCfgMtrSnRegion[bGroupMtrIndex].bRegionType = *pbPtr++;
			pbPtr++;	//long-unsigned
			tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnStart = OoOiToWord(pbPtr);	pbPtr += 2;
			pbPtr++;	//long-unsigned
			tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnEnd = OoOiToWord(pbPtr);	pbPtr += 2;
		}

		for (WORD wMtrMask=0; wMtrMask<PN_MASK_SIZE; wMtrMask++)
		{
			if (bMtrMask[wMtrMask] != 0)
			{
				for (BYTE bBit=0; bBit<8; bBit++)
				{
					if (bMtrMask[wMtrMask] & (1<<(bBit)))
					{
						TOobMtrInfo tMtrInfo;
						WORD wPn = wMtrMask*8 + bBit;
						bool fIsVlidMtrSn = false;

						memset((BYTE*)&tMtrInfo, 0, sizeof(tMtrInfo));
						GetMeterInfo(wPn, &tMtrInfo);
						wMtrSn = tMtrInfo.wMtrSn;

						for (BYTE bGroupMtrIndex=0; bGroupMtrIndex<bGroupMtrRegionCnt; bGroupMtrIndex++)
						{
							if (tTCfgMtrSnRegion[bGroupMtrIndex].bRegionType == 0)	//0-前闭后开
							{
								if ((wMtrSn>=tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnStart) && (wMtrSn<tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnEnd))
								{
									fIsVlidMtrSn = true;
									break;
								}
								else
									continue;
							}
							else if (tTCfgMtrSnRegion[bGroupMtrIndex].bRegionType == 1)	//1-前开后闭	
							{
								if ((wMtrSn>tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnStart) && (wMtrSn<=tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnEnd))
								{
									fIsVlidMtrSn = true;
									break;
								}
								else
									continue;
							}
							else if (tTCfgMtrSnRegion[bGroupMtrIndex].bRegionType == 2)	//2-前闭后闭
							{
								if ((wMtrSn>=tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnStart) && (wMtrSn<=tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnEnd))
								{
									fIsVlidMtrSn = true;
									break;
								}
								else
									continue;
							}
							else if (tTCfgMtrSnRegion[bGroupMtrIndex].bRegionType == 3)	//3-前开后开
							{
								if ((wMtrSn>tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnStart) && (wMtrSn<tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnEnd))
								{
									fIsVlidMtrSn = true;
									break;
								}
								else
									continue;
							}
						}

						if (fIsVlidMtrSn)
						{
							pbMtrMask[wMtrMask] |= (1<<bBit);
						}
					}
				}
			}
		}
        break;   //liyan

    default:
        break;
	}

	return pbPtr - pbBuf;
}

//描述：从RSD中获得MS屏蔽字，该函数主要是针对采集方案的
bool GetRSDMS(BYTE *pbRSD, BYTE *pbMtrMask, WORD wMaskSize)
{
	//RSD中的方法
	switch(*pbRSD++)
	{
	case 4:
	case 5:
		pbRSD += 7;	
		break;
	case 6:
	case 7:
	case 8:
		pbRSD += 7;	
		pbRSD += 7;	
		pbRSD += 4;	
		break;
	case 10:
		pbRSD++;
		break;
	default:
		return false;
	}

	pbRSD += ParserMsParam(pbRSD, pbMtrMask, wMaskSize);

	return true;
}

//描述：获得采集方案中的表地址屏蔽字（主要针对普通、事件、实时采集方案）
//参数：@pbBuf 采集方案参数
//		@p 采集方案对应的数据格式
//		@bIndex MS在数据格式中的偏移位置
//		@pbMtrMask 表屏蔽字
bool GetSchMS(BYTE *pbBuf, BYTE *pbFmt, WORD wFmtLen, BYTE bIndex, BYTE *pbMtrMask, WORD wMaskSize)
{
	WORD wLen;
	BYTE bType;
	BYTE *pbRec;
						
	pbRec = OoGetField(pbBuf, pbFmt, wFmtLen, bIndex, &wLen, &bType);
	if (pbRec == NULL)
		return false;
	pbRec += ParserMsParam(pbRec, pbMtrMask, wMaskSize);

	return true;
}

//描述：源地址（RSD）与目的地址（采集方案）比较
bool MtrMaskCompare(BYTE *pbSrcMask, WORD wSrcSize, BYTE *pbDstMask, WORD wDstSize)
{
	WORD i;
	BYTE bBit;

	if (wSrcSize > wDstSize)
		return false;

	if (memcmp(pbSrcMask, pbDstMask, wSrcSize) == 0)
		return true;

	for (i=0; i<wSrcSize; i++)
	{
		if (pbSrcMask[i] != 0)
		{
			for (bBit=0; bBit<8; bBit++)
			{
				if (pbSrcMask[i]&(1<<(bBit)))	//源地址有效
				{
					if (!(pbDstMask[i]&(1<<(bBit))))	//目的地址不存在，直接判定为失败
						return false;
				}
			}
		}
	}

	return true;
}

//描述：把源数据解析成一个个字段的偏移和长度，方便访问
//参数：@ pParser	 字段解析器
//	   @ pFmt	源数据的格式描述串
//	   @ wFmtLen	源数据的格式描述串的长度
//	   @ fParseItem	为false时只解析到配置本身，
//					为true时把字段对应的数据项的长度及偏移也计算出来
//返回:如果正确则返回true,否则返回false
bool OoParseField(TFieldParser* pParser, BYTE* pFmt, WORD wFmtLen, bool fParseItem)
{
	BYTE *pbPtr2 = pParser->pbCfg;
	int iRet;

	if (*pbPtr2!=1 && *pbPtr2!=2 && *pbPtr2!=DT_FRZRELA)	//只对struct and array
		return false;

	pbPtr2++;	//array or struct
	pParser->wNum = *pbPtr2;	pbPtr2++;	//个数
	if (pParser->wNum > 64)
		return false;
	for (BYTE bFieldIndex=0; bFieldIndex<pParser->wNum; bFieldIndex++)
	{
		iRet = OoScanData(pParser->pbCfg, pFmt, wFmtLen, false, bFieldIndex, &pParser->wLen[bFieldIndex], &pParser->bType[bFieldIndex]);		
		if (iRet < 0)
			return false;

		pParser->wPos[bFieldIndex] = iRet;
		//把字段相应的数据解析出来
		if (fParseItem)
		{
			iRet = OoGetDataLen(pParser->bType[bFieldIndex], &pParser->pbCfg[pParser->wPos[bFieldIndex]+1]);	//+1:去除数据类型
#ifdef GW_OOB_DEBUG
			if (pParser->bType[bFieldIndex]==DT_OAD)
			{
				DWORD dwOAD = OoOadToDWord(&pParser->pbCfg[pParser->wPos[bFieldIndex]+1]);
				if (dwOAD == 0xffffff01)
					iRet = 1;
			}
#endif
			if (iRet < 0)
				return false;
			pParser->wItemLen[bFieldIndex] = iRet;
			pParser->wItemOffset[bFieldIndex] = pParser->wTotalLen;
			pParser->wTotalLen += pParser->wItemLen[bFieldIndex];
		}
	}

	return true;
}


//描述：比较两个字段是否相同或者包含于【备注：pbCmpField，pbSrcField是不带数据格式的】
//参数：@ bCmpType 比较字段的类型 （例：bCmpType为RCSD中的类型CSD）
//	   @ pbCmpField比较字段的内容 （例：bCmpType为RCSD中的类型CSD内容）
//	   @ bSrcType源字段的类型 （例：普通采集方案中的CSD类型, bSrcType函数为ReadParserField()中pbType类型）
//	   @ pbSrcField源字段的内容  (例：普通采集方案中的CSD类型, pbSrcField函数为ReadParserField()中pbBuf类型)
//返回:如果两个字段相同则返回0,如果比较字段包含于源字段返回1，否则返回-1
int FieldCmp(BYTE bCmpType, BYTE* pbCmpField, BYTE bSrcType, BYTE* pbSrcField)
{
	int iRet;
	BYTE *pbSrcField0 = pbSrcField;
	BYTE bSrcRelaNum, bCmpRelaNum, bNum, i, j;
	BYTE bArryNum,  bRCSDNum;
	BYTE bCmpFmtType, bSrcFmtType;
	BYTE bInNum=0, bEQNum=0;

	switch (bCmpType)
	{
	case DT_OAD:	//OAD OAD可以与CSD\OAD\FMT_FRZRELA\ARRY CSD比较
		if (bSrcType==DT_OAD || bSrcType==DT_CSD || bSrcType==DT_FRZRELA || bSrcType==DT_ARRAY)
		{
			if (bSrcType == DT_OAD)
			{
				if (memcmp(pbCmpField, pbSrcField, 4) == 0)
					return 0;
			}
			else if (bSrcType == DT_CSD)
			{
				if (*pbSrcField++ == 1)	//只能比较OAD
					return -1;
				if (memcmp(pbCmpField, pbSrcField, 4) == 0)
					return 0;
			}
			else if (bSrcType == DT_FRZRELA)
			{
				pbSrcField += 5;	//传入的pbSrcField起始位置为跳过数据类型的下一个字节位置
				if (memcmp(pbCmpField, pbSrcField, 4) == 0)
					return 0;
			}
			else if (bSrcType == DT_ARRAY)
			{
				bNum = *pbSrcField++;
				for (i=0; i<bNum; i++)
				{
					if (*pbSrcField == DT_CSD)
					{
						if (*(pbSrcField+1) == 0)	//OAD
						{
							if (memcmp(pbCmpField, pbSrcField+2, 4) == 0)
								return 0;
						}
					}
					else
					{
						return -1;
					}
					pbSrcField += OoGetDataTypeLen(pbSrcField);
				}
			}
			else if (bSrcType == DT_PULSE_CFG)
			{
				pbSrcField += 2;	//传入的pbSrcField起始位置为跳过数据类型的下一个字节位置 定位到OAD的起始位置
				if (memcmp(pbCmpField, pbSrcField, 4) == 0)
					return 0;
			}
		}
		break;
	case DT_ROAD:	//ROAD可以与ROAD\CSD\ARRY CSD\ARRAY ROAD比较
		if (bSrcType == DT_ROAD)
		{
			if (memcmp(pbCmpField, pbSrcField, 4) == 0)	//对象属性描述符OAD
			{
				pbCmpField += 4;
				pbSrcField += 4;
				bCmpRelaNum = *pbCmpField++;	//关联OAD
				bSrcRelaNum = *pbSrcField++;
				if (bSrcRelaNum < bCmpRelaNum)
					return -1;
				pbSrcField0 = pbSrcField;
				for (i=0; i<bCmpRelaNum; i++)
				{
					pbSrcField = pbSrcField0;
					for (j=0; j<bSrcRelaNum; j++)
					{
						if (ByteToDWORD(pbCmpField, 4) ==  ByteToDWORD(pbSrcField, 4))
						{
							bEQNum++;
							break;
						}
						pbSrcField += 4;
					}

					pbCmpField += 4;
				}

				if (bCmpRelaNum == bEQNum)	//OAD匹配
				{
					if (bCmpRelaNum < bSrcRelaNum)	//是否包含于源字段中
						return 1;
					return 0;
				}
			}
		}
		else if (bSrcType == DT_CSD)
		{
			if (*pbSrcField++ == 0)	//只能和ROAD比较
				return -1;
			return FieldCmp(bCmpType, pbCmpField, DT_ROAD, pbSrcField);
		}
		else if (bSrcType == DT_ARRAY)	//Array CSD/ARRAY ROAD
		{
			if (*pbSrcField == DT_CSD)
			{
				bNum = *pbSrcField++;
				for (i=0; i<bNum; i++)
				{
					pbSrcField++;
					if (*pbSrcField++ != 0)	//不是ROAD
						return -1;

					iRet = FieldCmp(bCmpType, pbCmpField, DT_ROAD, pbSrcField);
					if (iRet == 0)
					{
						bEQNum++;
						break;
					}
					else if (iRet == 1)
					{
						bInNum++;
						break;
					}
					pbSrcField += ScanCSD(pbSrcField, false);
				}

				if ((bEQNum+bInNum) == bNum)
				{
					if (bInNum == 0)	//不存在被包含的ROAD
						return 0;
					return 1;
				}
			}
			else if (*pbSrcField == DT_ROAD)
			{
				bNum = *pbSrcField++;
				for (i=0; i<bNum; i++)
				{
					if (FieldCmp(bCmpType, pbCmpField, DT_ROAD, pbSrcField) >= 0)
						bEQNum++;
					pbSrcField += ScanCSD(pbSrcField, false);
				}
				if (bNum == bEQNum)
					return 0;
			}
		}
		break;
	case DT_CSD:	//CSD可以与OAD\ROAD\DT_FRZERLA\ARRAY CSD\ARRAY ROAD比较
		if (*pbCmpField++ == 0)	//OAD比较
		{
			if (bSrcType == DT_OAD || bSrcType==DT_FRZRELA)	//OAD
			{
				if (bSrcType==DT_FRZRELA)
					pbSrcField += 5;	//偏移冻结周期
				if (memcmp(pbCmpField, pbSrcField, 4) == 0)
					return 0;
			}
			else if (bSrcType == DT_CSD)	//CSD
			{
				if (*pbSrcField++ == 0)
				{
					if (memcmp(pbCmpField, pbSrcField, 4) == 0)
						return 0;
				}
			}
			else if (bSrcType == DT_ARRAY)	//array里存在ROAD或CSD
			{
				bNum = *pbSrcField++;
				for (i=0; i<bNum; i++)
				{
					bSrcFmtType = *pbSrcField;
					iRet = FieldCmp(DT_OAD, pbCmpField, bSrcFmtType, pbSrcField+1);
					if (iRet == 0)
						return 0;
					pbSrcField += OoGetDataTypeLen(pbSrcField);
				}
			}
		}
		else	//ROAD
		{
			if (bSrcType == DT_ROAD)	
			{
				return FieldCmp(DT_ROAD, pbCmpField, DT_ROAD, pbSrcField);
			}
			else if (bSrcType == DT_CSD)	
			{
				if (*pbSrcField++ == 1)	
					return FieldCmp(DT_ROAD, pbCmpField, DT_ROAD, pbSrcField);
			}
			else if (bSrcType == DT_ARRAY)	//array里存在ROAD或CSD 
			{
				//bSrcType=1有两种意义，一种是DT_ARRY格式，另外一种是CSD内部为1、即ROAD,如果比较字段与源字段的前4个字节相同就认为是ROAD比较
				iRet = FieldCmp(DT_ROAD, pbCmpField, DT_ROAD, pbSrcField);
				if (iRet < 0)
				{
					bNum = *pbSrcField++;
					for (i=0; i<bNum; i++)
					{
						bSrcFmtType = *pbSrcField;
						iRet = FieldCmp(DT_ROAD, pbCmpField, bSrcFmtType, pbSrcField+1);
						if (iRet==0 || iRet==1)
							return 0;
						pbSrcField += OoGetDataTypeLen(pbSrcField);
					}
				}
			}
		}
		break;
	case DT_RCSD:	//RCSD格式可以与ArrayCSD或ArrayROAD或CSD或ROAD或OAD或FMT_FRZRELA比较
		if (bSrcType == DT_ARRAY)
		{
			bInNum = 0;
			bEQNum = 0;
			bRCSDNum = *pbCmpField++;
			bArryNum = *pbSrcField0++; 
			for (i=0; i<bRCSDNum; i++)
			{
				if (*pbCmpField == 0)
					bCmpFmtType = DT_OAD;
				else
					bCmpFmtType = DT_ROAD;
				pbSrcField = pbSrcField0;
				for (j=0; j<bArryNum; j++)
				{
					bSrcFmtType = *pbSrcField;
					if (bSrcFmtType == bCmpFmtType)	//类型匹配
					{
						iRet = FieldCmp(bCmpFmtType, pbCmpField, bSrcFmtType, pbSrcField+1);	//+1去格式类型
						if (iRet == 0)
						{
							bEQNum++;
							break;
						}
						else if (iRet == 1)
						{
							bInNum++;
							break;
						}
					}
					pbSrcField += OoGetDataTypeLen(pbSrcField);
				}

				if (bCmpFmtType == DT_OAD)
					pbCmpField += 5;	//Choice + OAD
				else
					pbCmpField += ScanCSD(pbCmpField, false);
			}
			if ((bEQNum+bInNum) == i)
			{
				if (bInNum == 0)	//不存在被包含的ROAD
					return 0;
				return 1;
			}
		}
		else if (bSrcType==DT_CSD || bSrcType==DT_ROAD 
			|| bSrcType==DT_OAD || bSrcType==DT_FRZRELA)
		{
			bRCSDNum = *pbCmpField++;
			if (bRCSDNum != 1)	//源自段只有一个数据类型，比较字段存在多个数据类型时，这种情况是肯定不匹配的
				break;
			if (*pbCmpField == 0)	//OAD
				bCmpFmtType = DT_OAD;
			else 
				bCmpFmtType = DT_ROAD;
			return FieldCmp(bCmpFmtType, pbCmpField, bSrcType, pbSrcField);
		}
		break;
	case DT_FRZRELA:	//FMT_FRZRELA类型可以与OAD或CSD或FMT_FRZRELA比较
		if (bSrcType == DT_FRZRELA)
		{
			if (memcmp(pbCmpField, pbSrcField, 12) == 0)	//类型不比较
				return 0;	
		}
		else if (bSrcType == DT_OAD)	//OAD
		{
			pbCmpField += 5;
			if (memcmp(pbSrcField, pbCmpField, 4) == 0)
				return 0;
		}
		else if (bSrcType == DT_CSD)	//CSD
		{
			if (*pbSrcField++ == 0)
			{
				pbCmpField += 5;
				if (memcmp(pbSrcField, pbCmpField, 4) == 0)
					return 0;
			}
		}
		break;

	case DT_PULSE_CFG:	//脉冲配置
		if (bSrcType == DT_PULSE_CFG)
		{
			if (memcmp(pbCmpField, pbSrcField, 11) == 0)	//类型不比较
				return 0;	
		}
		else if (bSrcType == DT_OAD)	//OAD
		{			
			pbCmpField += 2;
			if (memcmp(pbSrcField, pbCmpField, 4) == 0)
				return 0;
		}
		break;
	}

	return -1;
}

//描述：从ROAD数据中选择性读取
//参数：@ pbSelROAD 选择性ROAD，一般对应到协议RCSD的CSD里的ROAD
//	   @ pbSrcROAD源ROAD，一般对应到配置里面的一个ROAD字段
//	   @ pbSelData 用来返回读取到的数据
//	   @ pbSrcData 源数据，对应到任务库记录的一个ROAD字段的数据
//返回:如果两个字段相同则返回0,如果比较字段包含于源字段返回1，否则返回-1
int ReadFromROAD_1(BYTE* pbSelROAD, BYTE* pbSrcROAD, BYTE* pbSelData, BYTE* pbSrcData)
{
	int iRet, iDataLen;
	BYTE *pbSrcROAD0;
	BYTE bLkCmpOAD, bLkSrcOAD, bFindNum=0;
	BYTE bSrcType, bSelType;
	BYTE i, j;
	BYTE *pbSelData0 = pbSelData;
	BYTE *pbSrcData0 = pbSrcData;

	if (*pbSelROAD++ == 0)
		bSelType = DT_OAD;
	else
		bSelType = DT_ROAD;

	BYTE bType = *pbSrcROAD;
	if (bType == DT_ROAD)
	{
		bSrcType = DT_ROAD;
	}
	else
	{
		if (*pbSrcROAD++ != DT_CSD)
			return -1;
		if (*pbSrcROAD++ == 0)
			bSrcType = DT_OAD;
		else
			bSrcType = DT_ROAD;
	}

	if (bSelType!=DT_ROAD || bSrcType!=DT_ROAD)
		return -1;

	if (ByteToDWORD(pbSelROAD, 4) == ByteToDWORD(pbSrcROAD, 4))	//OAD
	{
		pbSelROAD += 4;
		pbSrcROAD += 4;
		bLkCmpOAD = *pbSelROAD++;
		bLkSrcOAD = *pbSrcROAD++;
		if (bLkCmpOAD <= bLkSrcOAD)	//OAD关联个数
		{
			iDataLen = 0;
			pbSrcROAD0 = pbSrcROAD;
			for (i=0; i<bLkCmpOAD; i++)
			{
				pbSrcROAD = pbSrcROAD0;
				pbSrcData = pbSrcData0;
				for (j=0; j<bLkSrcOAD; j++)
				{
					if ((iRet=OoGetDataLen(DT_OAD, pbSrcROAD+j*4)) <= 0)
						return -1;	 

					if (ByteToDWORD(pbSelROAD+i*4, 4) == ByteToDWORD(pbSrcROAD+j*4, 4))	
					{
						memcpy(pbSelData, pbSrcData, iRet);
						pbSelData += iRet;
					}
					pbSrcData += iRet;
				}
			}

			if (bFindNum == i)
			{
				iDataLen = pbSelData - pbSelData0;
				pbSelData = pbSelData0;
				return iDataLen;
			}
		}
	}

	return -1;
}

//描述：从ROAD数据中选择性读取
//参数：@ pbSelROAD 选择性ROAD，一般对应到协议RCSD的CSD里的ROAD
//	   @ pbSrcROAD源ROAD，一般对应到配置里面的一个ROAD字段
//	   @ pbSelData 用来返回读取到的数据
//	   @ pbSrcData 源数据，对应到任务库记录的一个ROAD字段的数据
//返回:如果两个字段相同则返回0,如果比较字段包含于源字段返回1，否则返回-1
int ReadFromROAD(BYTE* pbSelROAD, BYTE* pbSrcROAD, BYTE* pbSelData, BYTE* pbSrcData)
{
	int iRet, iDataLen;
	WORD wSelOffset = 0;
	WORD wSrcOffset = 0;
	BYTE *pbSrcROAD0;
	BYTE bLkCmpOAD, bLkSrcOAD, bFindNum=0;
	BYTE bSrcType, bSelType;
	BYTE i, j;

	if (*pbSelROAD++ == 0)
		bSelType = DT_OAD;
	else
		bSelType = DT_ROAD;

	BYTE bType = *pbSrcROAD;
	if (bType == DT_ROAD)
	{
		bSrcType = DT_ROAD;
	}
	else
	{
		if (*pbSrcROAD++ != DT_CSD)
			return -1;
		if (*pbSrcROAD++ == 0)
			bSrcType = DT_OAD;
		else
			bSrcType = DT_ROAD;
	}

	if (bSelType!=DT_ROAD || bSrcType!=DT_ROAD)
		return -1;

	if (ByteToDWORD(pbSelROAD, 4) == ByteToDWORD(pbSrcROAD, 4))	//OAD
	{
		pbSelROAD += 4;
		pbSrcROAD += 4;
		bLkCmpOAD = *pbSelROAD++;
		bLkSrcOAD = *pbSrcROAD++;
		if (bLkCmpOAD <= bLkSrcOAD)	//OAD关联个数
		{
			iDataLen = 0;
			pbSrcROAD0 = pbSrcROAD;
			for (i=0; i<bLkCmpOAD; i++)
			{
				pbSrcROAD = pbSrcROAD0;
				wSrcOffset = 0;
				for (j=0; j<bLkSrcOAD; j++)
				{
					if ((iRet=OoGetDataLen(DT_OAD, pbSrcROAD+j*4)) <= 0)
						return -1;	 

					if (ByteToDWORD(pbSelROAD+i*4, 4) == ByteToDWORD(pbSrcROAD+j*4, 4))	//
					{
						BYTE *pOneSrcData = pbSrcData+wSrcOffset;

						if (IsAllAByte(pOneSrcData, 0x00, iRet) || ((pOneSrcData[0]==0x00) && (pOneSrcData[1]!=0x00))) //全0 || 抄读失败
						{
							*pbSelData++ = 0x00;	//抄读失败，直接填充0
							wSrcOffset += iRet;
							iRet = 1;
							wSelOffset += iRet;
							iDataLen += iRet;
						}
						else
						{
							WORD wRetFmtLen, wRetSrcOffset;
							int iLen;
							iLen = OoCopyOneOadData(pbSrcData+wSrcOffset, pbSrcROAD+j*4, 4, pbSelData+wSelOffset, &wRetFmtLen, &wRetSrcOffset);
							wSrcOffset += iLen;
							//memcpy(pbSelData+wSelOffset, pbSrcData+wSrcOffset, iRet);
							//wSrcOffset += iRet;
							//wSelOffset += iRet;
							//iDataLen += iRet;
							wSelOffset += wRetSrcOffset;
							iDataLen += wRetSrcOffset;
						}
						bFindNum++;
						break;
					}
					wSrcOffset += iRet;
				}
			}
			if (bFindNum == i)
				return iDataLen;
		}
	}

	return -1;
}

//描述: 创建任务库的一个表
//参数：@pszTableName	表名
//	   @ pFixFields	固定字段
//	   @ pDataFields	数据字段，如果没有数据字段传入NULL
//	   @ dwRecNumMax 记录最大保存笔数
//返回:如果正确则返回0,否则返回负数
int CreateTable(char* pszTableName, TFieldParser* pFixFields, TFieldParser* pDataFields, DWORD dwRecNumMax)
{
	TTabCtrl TabCtrl;
	int iFieldLen;
	BYTE bFieldData[512];	
	BYTE bFieldType;
	BYTE bFieldNum = 0;

	memset(&TabCtrl, 0, sizeof(TabCtrl));
	if (pFixFields != NULL)	//固定字段
	{	
		for(WORD wIndex=0; wIndex<pFixFields->wNum; wIndex++)
		{ 
			WORD wItemOffset;
			WORD wItemLen;
			memset(bFieldData, 0, sizeof(bFieldData));
			if ((iFieldLen=ReadParserField(pFixFields, wIndex, bFieldData, &bFieldType, &wItemOffset, &wItemLen)) > 0)	//函数返回的是字段内容长度
			{
				TabCtrl.wField[bFieldNum][0] = TDB_BYTE;
				TabCtrl.wField[bFieldNum][1] = wItemLen;	
				bFieldNum++;
			}
		}
	}

	if ((pDataFields != NULL) && (pDataFields->wCfgLen != 0))	//数据字段
	{
		TabCtrl.wField[bFieldNum][0] = TDB_BYTE;
		TabCtrl.wField[bFieldNum][1] = pDataFields->wTotalLen;	
		bFieldNum++;
	}

	TabCtrl.bPublicNum = 0;							//公用字段的个数
	TabCtrl.bPrivateNum = (bFieldNum!=0)? bFieldNum: 0;			//子字段个数
	TabCtrl.dwMaxRecPublicNum = 0;					//可记录公共个数
	TabCtrl.dwMaxRecPrivateNum = dwRecNumMax;		//最大记录子个数
	TabCtrl.dwCurRecNum = 0;
	TabCtrl.dwCurRecOffset = 0;
	TabCtrl.bVer = 1;

	int iRet = TdbCreateTable(pszTableName, TabCtrl);
	if (iRet==TDB_ERR_OK || iRet==TDB_ERR_TBEXIST)
	{
		DTRACE(DB_TASK, ("CreateTable: create %s ok! rec size=%ld.\n", pszTableName, dwRecNumMax));
		return dwRecNumMax;
	}
	else
	{
		DTRACE(DB_TASK, ("CreateTable: fail to create %s.\n", pszTableName));
	}

	return -1;
}

//描述: 创建到一笔记录到任务库的一个表
//参数：@pszTableName 表名
//	   @ pbRec记录内容
//返回:如果正确则返回true,否则返回false
bool SaveRecord(char* pszTableName, BYTE* pbRec)
{
	int fd = -1;

	fd = TdbOpenTable(pszTableName, O_RDWR|O_BINARY);

	if (fd >= 0)
	{	
		TdbAppendRec(fd, pbRec);
		TdbCloseTable(fd);
		return true;
	}

	return false;
}

//描述: 修改上N笔记录数据
//参数：@pszTableName 表名
//		@index 上N笔		
//	   @ pbRec记录内容
//返回:如果正确则返回true,否则返回false
bool SaveRecordByPhyIdx(char* pszTableName, WORD wPhyIdx, BYTE* pbRec)
{
	int fd = -1;

	//wPhyIdx++;
	
	fd = TdbOpenTable(pszTableName, O_RDWR|O_BINARY);
	if (fd >= 0)
	{	
		TdbAppendRec(fd, wPhyIdx, pbRec);
		TdbCloseTable(fd);
		return true;
	}

	return false;	
}

//描述: 创建到一笔记录到任务库的一个表
//参数：@pszTableName 表名
//	   @ pbRec记录内容
//	   @ piRecPhyIdx返回记录存储的物理位置
//返回:如果正确则返回true,否则返回false
bool SaveRecord(char* pszTableName, BYTE* pbRec, int* piRecPhyIdx)
{
	int fd = -1, iRecPtr = -1, iMaxRecNum = -1;

	fd = TdbOpenTable(pszTableName, O_RDWR|O_BINARY);

	if (fd >= 0)
	{	
		TdbAppendRec(fd, pbRec);
		iRecPtr = TdbGetRecPtr(fd);
		iMaxRecNum = TdbGetMaxRecNum(fd);
		if (iRecPtr < 0)
		{
			TdbCloseTable(fd);
			*piRecPhyIdx = -1;
			return false;
		}

		if (iRecPtr > 0)
		{
			*piRecPhyIdx = iRecPtr;
		}
		else
		{
			*piRecPhyIdx = iMaxRecNum;
		}
		TdbCloseTable(fd);
		return true;
	}

	*piRecPhyIdx = -1;
	return false;
}

//描述: 修改上N笔记录数据
//参数：@pszTableName 表名
//		@index 上N笔		
//	   @ pbRec记录内容
//返回:如果正确则返回true,否则返回false
bool SaveHisRecord(char* pszTableName, int index, BYTE* pbRec)
{
	int fd = -1;
 	int iRet, iIdx;

	if (index < 1)
		return false;	
	//index--;
	
	fd = TdbOpenTable(pszTableName, O_RDWR|O_BINARY);
	if (fd >= 0)
	{	
		iIdx = GetRecIdx(fd, index);
		if (iIdx >= 0)
		{
			iRet = TdbAppendRec(fd, iIdx, pbRec);
			TdbCloseTable(fd);
			return true;
		}
		TdbCloseTable(fd);
	}

	return false;	
}

#define SIZE_1M	(1024*1024)
#define SCH_FILE_MAX_SIZE	(40*SIZE_1M)	//每个方案文件最大空间40M
#define SCH_FILE_TOTAL_MAX_SIZE	(80*SIZE_1M)	//总的方案文件最大空间80M

//描述：初始化采集方案，建任务库表
void InitSchTable()
{
	const char *pszUnitName[] = {"UNIT_SEC", "UNIT_MIN", "UNIT_HOUR", "UNIT_DAY", "UNIT_MONTH", "UNIT_YEAR"};
	TTaskCfg tTaskCfg;
	TFieldParser tFixFields, tDataFields;
	int iRet, iIndex, iArryOff, iMs, iSchCfgLen;
	DWORD dwOMD, dwStgCnt;
	DWORD dwFileSize, dwFileTotalSize, dwFieldDataLen;
	WORD wLen, wPnNum, wFmtLen;
	char pszTableName[32];
	BYTE bType; 
	BYTE *pbFmt, *pbDataFmt, *pbSch;
	WORD wDataFmtLen;
	

	dwFileTotalSize = 0;

	for (WORD wIndex=0; wIndex<TASK_ID_NUM; wIndex++)
	{
		memset((BYTE*)&tTaskCfg, 0, sizeof(tTaskCfg));
		if (GetTaskCfg(wIndex, &tTaskCfg))
		{
			pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
			if (pbSch==NULL)
				continue;

			memset((BYTE*)&tFixFields, 0, sizeof(tFixFields));
			memset((BYTE*)&tDataFields, 0, sizeof(tDataFields));

			//初始化固定字段
			const TFieldCfg *pFixFieldCfg = &g_TSchFieldCfg[tTaskCfg.bSchType-1].tTFixFieldCfg;
			tFixFields.pbCfg = (BYTE*)pFixFieldCfg->pbCfg;
			tFixFields.wCfgLen = pFixFieldCfg->wCfgLen;

			if (!OoParseField(&tFixFields, (BYTE*)pFixFieldCfg->pbFmt, pFixFieldCfg->wFmtLen, true))
				continue;

			//初始化数据字段
			switch (tTaskCfg.bSchType)
			{
			case SCH_TYPE_COMM:
				iIndex = 1;
				iMs = 4;
				iArryOff = 3;
				break;
			case SCH_TYPE_EVENT:
				iIndex = 4;
				iMs = 2;
				iArryOff = 1;
				break;
			case SCH_TYPE_TRANS:	
				iIndex = -1;
				dwStgCnt = 1;
				break;
// 			case SCH_TYPE_REPORT:	//备注：上报采集方案待处理。。。
// 				break;
// 			case SCH_TYPE_SCRIPT:
// 				break;
			default:
				continue;//return;
			}

			pbFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
			if(pbFmt == NULL)
				continue;

			if (iIndex != -1)
			{	
				BYTE *pbMs = OoGetField(pbSch, pbFmt, wFmtLen, iMs, &tDataFields.wCfgLen, &bType);
				if (!pbMs)
				{
					DTRACE(DB_TASK, ("InitSchTable: pbMs is error, bTaskId=%d.\n", tTaskCfg.bTaskId));
					continue;
				}
				//wPnNum = MsToMtrNum(pbMs);
				if ((iRet=OoScanData(pbSch, pbFmt, wFmtLen, false, iIndex, &wLen, &bType)) > 0)	
					dwStgCnt = OoOiToWord(&pbSch[iRet+1]);

				switch (tTaskCfg.tiExe.bUnit)
				{
				case TIME_UNIT_MINUTE:
#if FA_TYPE == FA_TYPE_K32
					dwStgCnt = 96*5;	//密度最小15分钟，保存5天数据
					wPnNum = POINT_NUM;
#else	//C82/D82
					dwStgCnt = 96*5;	//密度最小15分钟，保存5天数据
					wPnNum = 20;
#endif
					break;
				case TIME_UNIT_HOUR:
#if FA_TYPE == FA_TYPE_K32
					dwStgCnt = 24*7;	//密度最小1个小时，保存7天数据
					wPnNum = POINT_NUM;
#else	//C82/D82
					dwStgCnt = 24*7;	//密度最小1个小时，保存7天数据
					wPnNum = 20;
#endif
					break;
				case TIME_UNIT_DAY:
					dwStgCnt = 60;	//密度最小为1天，保存60天数据
					wPnNum = POINT_NUM;
					break;
				case TIME_UNIT_MONTH:
					dwStgCnt = 12;	//密度最小为1个月，保存12个月数据
					wPnNum = POINT_NUM;
					break;
				default:
					dwStgCnt = 100;	//其他定义为100
					wPnNum = POINT_NUM;
				}

				dwStgCnt = wPnNum * dwStgCnt;

				dwFieldDataLen = tFixFields.wTotalLen + tDataFields.wTotalLen;
				dwFileSize = dwStgCnt * dwFieldDataLen;
				if (dwFileSize > SCH_FILE_MAX_SIZE)	//方案存储空间大于SCH_FILE_MAX_SIZE，将对该空间减半
				{
					DTRACE(DB_CRITICAL, ("InitSchTable(): Over file Size: bTaskId=%d, bSchNo=%d, Unit=%s, dwCurFileSize=%.2fM > %dM.\r\n",
						tTaskCfg.bTaskId, tTaskCfg.bSchNo, pszUnitName[tTaskCfg.tiExe.bUnit], (float)dwFileSize/SIZE_1M, SCH_FILE_MAX_SIZE/SIZE_1M));

					dwStgCnt = (SCH_FILE_MAX_SIZE/2)/dwFieldDataLen + 1;
					dwFileTotalSize += dwStgCnt*dwFileTotalSize;
				}
				else
				{
					dwFileTotalSize += dwFileSize;
				}

				if (dwFileTotalSize >= SCH_FILE_TOTAL_MAX_SIZE)	//方案创建的文件空间超过SCH_FILE_TOTAL_MAX_SIZE就停止方案再创建?
				{
					DTRACE(DB_CRITICAL, ("InitSchTable(): Over total file Size: bTaskId=%d, bSchNo=%d, Unit=%s, dwTotalFileSize=%.2fM > %dM.\r\n",
						tTaskCfg.bTaskId, tTaskCfg.bSchNo, pszUnitName[tTaskCfg.tiExe.bUnit], (float)dwFileTotalSize/SIZE_1M, SCH_FILE_TOTAL_MAX_SIZE/SIZE_1M));
					return;
				}
			}

			if (tTaskCfg.bSchType == SCH_TYPE_EVENT)
			{
				int iLen;
				WORD wNum;
				int  iCfgLen;
				BYTE *pbArryROAD = OoGetField(pbSch, pbFmt, wFmtLen, iArryOff, &tDataFields.wCfgLen, &bType);

				pbArryROAD += 4; //跳过采集类型
				if (*pbArryROAD++ == DT_ARRAY)
				{
					wNum = *pbArryROAD++;
					for (WORD i=0; i<wNum; i++)
					{
						iCfgLen = OoGetDataTypeLen(pbArryROAD);
						if (iCfgLen < 0)
						{
							DTRACE(DB_CRITICAL, ("InitSchTable: OoGetDataTypeLen error i=%d.\n", i));
							break;
						}
						iLen = OoGetDataLen(DT_ROAD, pbArryROAD+1);
						if (iLen <= 0)
						{
							DTRACE(DB_CRITICAL, ("InitSchTable: OoGetDataLen error i=%d.\n", i));
							break;
						}
						//数组或结构的具体数据内容
						tDataFields.pbCfg = pbArryROAD;
						tDataFields.wCfgLen = iCfgLen;
						//对配置字段解析的结果
						tDataFields.wNum = 1;
						tDataFields.wPos[0] = 0;
						tDataFields.wLen[0] = iLen;
						tDataFields.bType[0] = DT_ROAD;
						//配置的字段进一步解析成对应数据项的信息
						tDataFields.wTotalLen = iLen;
						tDataFields.wItemOffset[0] = 0;
						tDataFields.wItemLen[0] = 0;

						memset(pszTableName, 0, sizeof(pszTableName));
						sprintf(pszTableName, "%s_%03d_%02d.dat", GetSchTableName(tTaskCfg.bSchType), tTaskCfg.bSchNo, i);
						CreateTable(pszTableName, &tFixFields, &tDataFields, dwStgCnt);	

						pbArryROAD++;
						pbArryROAD += ScanROAD(pbArryROAD, false);
					}
				}
			}
			else
			{
				tDataFields.pbCfg = OoGetField(pbSch, pbFmt, wFmtLen, iArryOff, &tDataFields.wCfgLen, &bType, &pbDataFmt, &wDataFmtLen);
				if (OoParseField(&tDataFields, pbDataFmt, wDataFmtLen, true))
				{
					memset(pszTableName, 0, sizeof(pszTableName));
					sprintf(pszTableName, "%s_%03d.dat", GetSchTableName(tTaskCfg.bSchType), tTaskCfg.bSchNo);
					CreateTable(pszTableName, &tFixFields, &tDataFields, dwStgCnt);
				}
			}

			if (iIndex != -1)
			{
				DTRACE(DB_CRITICAL, ("InitSchTable(): bTaskId=%d, bSchNo=%d, pszTableName=%s, Unit=%s, dwFileSize=%.2fM, dwTotalFileSize=%.2fM.\r\n",
					tTaskCfg.bTaskId, tTaskCfg.bSchNo, pszTableName, pszUnitName[tTaskCfg.tiExe.bUnit], (float)dwFileSize/SIZE_1M, (float)dwFileTotalSize/SIZE_1M));
			}
		}
	}
}

//描述：全局抄表缓存刷新到任务库接口
//参数：@bTaskId 任务ID
//		@bSchType 方案类型
//		@pbRecBuf 数据内容
//		@wIdex 该参数只针对全事件采集采用，其他方案类型可不用考虑
bool WriteCacheDataToTaskDB(BYTE bSchNo, BYTE bSchType, BYTE *pbRecBuf, WORD wRecLen, WORD wIdex, int* piRecPhyIdx)
{
	TSchFieldCfg *p = (TSchFieldCfg*)&g_TSchFieldCfg[bSchType-1];
	char pszTableName[32] = {0};
	char pszOutInfo[64] = {0};

	if (bSchType==SCH_TYPE_EVENT)
	{
		sprintf(pszTableName, "%s_%03d_%02d.dat", p->pszTableName, bSchNo, wIdex);
		sprintf(pszOutInfo, "Save data %s:", pszTableName);
		TraceBuf(DB_TASK, pszOutInfo, pbRecBuf, wRecLen);

		return SaveRecord(pszTableName, pbRecBuf);
	}
	else
	{
		sprintf(pszTableName, "%s_%03d.dat", p->pszTableName, bSchNo);
		sprintf(pszOutInfo, "Save data %s:", pszTableName);
		TraceBuf(DB_TASK, pszOutInfo, pbRecBuf, wRecLen);

		if (piRecPhyIdx != NULL)
		{
			if ((*piRecPhyIdx) > 0)
			{
				return SaveRecordByPhyIdx(pszTableName, (*piRecPhyIdx) - 1, pbRecBuf);
			}
			else
			{
				return SaveRecord(pszTableName, pbRecBuf, piRecPhyIdx);
			}
		}
		else
		{
			return SaveRecord(pszTableName, pbRecBuf);
		}
	}

	return false;
}

//描述：是否是特殊的属性描述符OAD
bool IsSpecOobAttrDescOAD(BYTE* pbOAD)
{
	bool fRet = false;
	DWORD dwOAD;

	dwOAD = OoOadToDWord(pbOAD);
	switch (dwOAD & 0xFFFFFF00)
	{
	case 0x60000200:	//采集档案配置表
	case 0x60020200:	//搜表档案
	case 0x60120200:	//任务配置单元
	case 0x60140200:	//普通采集方案
	case 0x60160200:	//事件采集方案集
	case 0x60180200:	//透明采集方案集
	case 0x601C0200:	//上报采集方案
	case 0x60340200:	//采集任务监控集
		fRet = true;
		break;
	}

	return fRet;
}

static int selector2_60000200(BYTE* pbOAD, BYTE* pbRSD, BYTE* pbRCSD, BYTE* pbBuf, WORD wBufSize, WORD* pwRetNum)
{
	DWORD dwOAD;
	DWORD dwSubOAD;
	BYTE  bSubIdx=0, bType=0, *p=0;
	BYTE bBuf[1024], bDataType=0;
	WORD wDataLen=0;
	int iRet=0, iTotalLen=0, iStartValue=0, iEndValue=0, iUnitValue=0, iDataLen=0, iTmpValue=0;
	BYTE *pbStart = pbBuf;

	dwOAD = OoOadToDWord(pbOAD);
	dwSubOAD = OoOadToDWord(pbRSD);
	pbRSD += 4;
	bSubIdx = dwSubOAD & 0xff;
	const ToaMap* pOI = GetOIMap(dwOAD & 0xFFFFFF00);
	bSubIdx = dwSubOAD & 0xff;
       
	*pwRetNum = 0;

	if (pOI == NULL)
	{
		DTRACE(DB_FAPROTO, ("selector2_60000200: pOI is NULL, dwOAD=0x%08x.\n", dwOAD));
		return -1;
	}

	if (bSubIdx != 1)	//档案信息只能用到子属性1，其它为结构体，无法访问
	{
		DTRACE(DB_FAPROTO, ("selector2_60000200: dwOAD=0x%08x subidx=%d invalid.\n", dwOAD, bSubIdx));
		return -1;
	}

	 //起始值 		 Data，
	 bDataType = *pbRSD;
	 iDataLen = OoGetDataTypeLen(pbRSD);
	 iStartValue = OoStdTypeToInt(pbRSD);
	 if (iDataLen < 0 || iStartValue == -1)
	 	goto error_1;

	 //pbRSD++;
	 pbRSD += iDataLen;
	 //结束值 		 Data，
	 iDataLen = OoGetDataTypeLen(pbRSD);
	 iEndValue = OoStdTypeToInt(pbRSD);
	 if (iDataLen < 0 || iEndValue == -1)
	 	goto error_1;
	 //pbRSD++;
	 pbRSD += iDataLen;
	 //数据间隔		 Data
	 iDataLen = OoGetDataTypeLen(pbRSD);
	 iUnitValue = OoStdTypeToInt(pbRSD);
	 if (iDataLen < 0 || iUnitValue == -1)
	 	goto error_1;
	// pbRSD++;
	 pbRSD += iDataLen;
	
	
	for (WORD i=0; i<POINT_NUM; i++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		if (((iRet=ReadItemEx(BANK0, i, pOI->wID, bBuf))>0) && !IsAllAByte(bBuf, 0, sizeof(bBuf)))
		{
			p = OoGetField(bBuf+1, pOI->pFmt, pOI->wFmtLen, bSubIdx-1, &wDataLen, &bType);	//+1:0x6000档案的第一个自己为整个采集单元的数据长度
			if (p != NULL)
			{
				if (bDataType == *p)  //比较数据类型
				{
					iTmpValue = OoStdTypeToInt(p);
					if (iTotalLen > wBufSize)
					{
						DTRACE(DB_FAPROTO, ("selector2_60000200: Buffer overflow, iRet=%d, wBufSize=%d.\n", iRet, wBufSize));
						goto error_1;
					}
					else if (iStartValue <= iTmpValue && iTmpValue <= iEndValue)
					{
						
						memcpy(pbBuf, bBuf+1, bBuf[0]);
						iTotalLen += bBuf[0];
						pbBuf += bBuf[0];
						*pwRetNum += 1;
					}
				}
			}
			else
			{
				DTRACE(DB_FAPROTO, ("selector2_60000200: i=%d, pointer is Null!!!!\n", i));
			}
		}
	}


	return pbBuf-pbStart;

error_1:
	*pwRetNum = 0;
	return -1;
	
	
}

int SpecReadRecord(BYTE* pbOAD, BYTE* pbRSD, BYTE* pbRCSD, int* piStart, BYTE* pbBuf, WORD wBufSize, WORD* pwRetNum)
{
	int iRet;
	DWORD dwOAD, dwSubOAD;
	WORD  wDataLen;
	BYTE bBuf[1024];
	BYTE *p, bType;
	BYTE bSubIdx, bSchType;
	BYTE *pbBuf0 = pbBuf;
	BYTE bRSD;
	WORD wCopyLen = 0;
	
	bRSD = *pbRSD++;
	if (bRSD == 1)	//方法1
	{
		dwOAD = OoOadToDWord(pbOAD);
		dwSubOAD = OoOadToDWord(pbRSD);
		pbRSD += 4;
		bSubIdx = dwSubOAD & 0xff;
		const ToaMap* pOI = GetOIMap(dwOAD & 0xFFFFFF00);
		if (pOI == NULL)
		{
			DTRACE(DB_FAPROTO, ("SpecReadRecord: pOI is NULL, dwOAD=0x%08x.\n", dwOAD));
			return -1;
		}

		switch (dwOAD & 0xFFFFFF00)
		{
		case 0x60000200:	//采集档案配置表
			if (bSubIdx != 1)	//档案信息只能用到子属性1，其它为结构体，无法访问
			{
				DTRACE(DB_FAPROTO, ("SpecReadRecord: dwOAD=0x%08x subidx=%d invalid.\n", dwOAD, bSubIdx));
				return -1;
			}
			for (WORD i=0; i<POINT_NUM; i++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				if (((iRet=ReadItemEx(BANK0, i, pOI->wID, bBuf))>0) && !IsAllAByte(bBuf, 0, sizeof(bBuf)))
				{
					p = OoGetField(bBuf+1, pOI->pFmt, pOI->wFmtLen, bSubIdx-1, &wDataLen, &bType);	//+1:0x6000档案的第一个自己为整个采集单元的数据长度
					if (p != NULL)
					{
						if (memcmp(pbRSD, p, wDataLen) == 0)	//格式类型比较 && 数据比较
						{
							if (wCopyLen + bBuf[0] > wBufSize)
							{
								DTRACE(DB_FAPROTO, ("SpecReadRecord: Buffer overflow, iRet=%d, wBufSize=%d.\n", iRet, wBufSize));
								goto SpecReadRecord_ret;
							}
							else
							{
								wCopyLen += bBuf[0];
								memcpy(pbBuf, bBuf+1, bBuf[0]);
								pbBuf += bBuf[0];
								*pwRetNum = 1;
							}
						}
					}
					else
					{
						DTRACE(DB_FAPROTO, ("SpecReadRecord: i=%d, pointer is Null!!!!\n", i));
					}
				}
			}
			break;
		case 0x60120200:	//任务配置单元
			for (WORD i=0; i<TASK_ID_NUM; i++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				if ((iRet = GetTaskConfigFromTaskDb(i, bBuf)) > 0)
				{
					p = OoGetField(bBuf, pOI->pFmt, pOI->wFmtLen, bSubIdx-1, &wDataLen, &bType);
					if (memcmp(pbRSD, p, wDataLen) == 0)	//格式类型比较 && 数据比较
					{
						if (wCopyLen + iRet > wBufSize)
						{
							DTRACE(DB_FAPROTO, ("SpecReadRecord: Buffer overflow, iRet=%d, wBufSize=%d.\n", iRet, wBufSize));
							goto SpecReadRecord_ret;
						}
						else
						{
							wCopyLen += iRet;
							memcpy(pbBuf, bBuf, iRet);
							pbBuf += iRet;
							*pwRetNum = 1;
							break;
						}
					}
				}
			}
			break;
		case 0x60140200:	//普通采集方案集
		case 0x60160200:	//事件采集方案集
		case 0x60180200:	//透明采集方案集
		case 0x601C0200:	//上报采集方案集
			if ((dwOAD & 0xFFFFFF00) == 0x60140200)
				bSchType = SCH_TYPE_COMM;
			else if ((dwOAD & 0xFFFFFF00) == 0x60160200)
				bSchType = SCH_TYPE_EVENT;
			else if ((dwOAD & 0xFFFFFF00) == 0x60180200)
				bSchType = SCH_TYPE_TRANS;
			else //0x601C0200
				bSchType = SCH_TYPE_REPORT;

			for (WORD i=0; i<SCH_NO_NUM; i++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				if ((iRet = GetSchFromTaskDb(i, bSchType, bBuf)) > 0)
				{
					p = OoGetField(bBuf, pOI->pFmt, pOI->wFmtLen, bSubIdx-1, &wDataLen, &bType);
					if (p!=NULL && (memcmp(pbRSD, p, wDataLen)==0))	//格式类型比较 && 数据比较
					{
						if (wCopyLen + iRet > wBufSize)
						{
							DTRACE(DB_FAPROTO, ("SpecReadRecord: Buffer overflow, iRet=%d, wBufSize=%d.\n", iRet, wBufSize));
							goto SpecReadRecord_ret;
						}
						else
						{
							wCopyLen += iRet;
							memcpy(pbBuf, bBuf, iRet);
							pbBuf += iRet;
							*pwRetNum = 1;
						}
					}
				}
			}
			break;
		case 0x60340200:	//采集任务监控集
			for (WORD i=0; i<TASK_NUM; i++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				if ((iRet = ReadItemEx(BANK0, i, pOI->wID, bBuf)) > 0)
				{
					if (IsAllAByte(bBuf, 0, sizeof(bBuf)))
						continue;

					p = OoGetField(bBuf, pOI->pFmt, pOI->wFmtLen, bSubIdx-1, &wDataLen, &bType);
					if (p != NULL)
					{
						if (memcmp(pbRSD, p, wDataLen) == 0)	//格式类型比较 && 数据比较
						{
							if (wCopyLen + iRet > wBufSize)
							{
								DTRACE(DB_FAPROTO, ("SpecReadRecord: Buffer overflow, iRet=%d, wBufSize=%d.\n", iRet, wBufSize));
								goto SpecReadRecord_ret;
							}
							else
							{
// 								*pbBuf++ = 0x01;	//记录数据
// 								*pbBuf++ = 0x01;	//M条记录
								wCopyLen += iRet;
								memcpy(pbBuf, bBuf, iRet);
								pbBuf += iRet;
								*pwRetNum = 1;
							}
						}
					}
				}
			}
			break;
		}
	}
	else if (bRSD == 2)
	{
		TTime tStartTime;
		TTime tEndTime;
		DWORD dwStartSec;
		DWORD dwEndSec;
		dwOAD = OoOadToDWord(pbOAD);
		dwSubOAD = OoOadToDWord(pbRSD);
		pbRSD += 4;
		bSubIdx = dwSubOAD & 0xff;


		const ToaMap* pOI = GetOIMap(dwOAD & 0xFFFFFF00);
		if (pOI == NULL)
		{
			DTRACE(DB_FAPROTO, ("SpecReadRecord: pOI is NULL, dwOAD=0x%08x.\n", dwOAD));
			return -1;
		}

		if ((dwOAD & 0xFFFFFF00) == 0x60020200)
		{
			//OAD由外部填充
			//RCSD
			*pbBuf++ = 0x01;	//RCSD个数
			*pbBuf++ = 0x00;	//选择OAD
			pbBuf += OoDWordToOad(dwSubOAD&0xffffff00, pbBuf);
		}
    
		switch (dwOAD & 0xFFFFFF00)
		{
		case 0x60020200:	//所有搜表结果
		
			if (bSubIdx != 6)	//所有搜表信息子属性6，其它为结构体，无法访问
			{
				DTRACE(DB_FAPROTO, ("SpecReadRecord: RSD=2 dwOAD=0x%08x subidx=%d invalid.\n", dwOAD, bSubIdx));
				return -1;
			}

			*pbRSD++;	//DT_DATATIMES
			OoDateTimeSToTime(pbRSD, &tStartTime);
			pbRSD += 7;
			*pbRSD++;	//DT_DATATIMES
			OoDateTimeSToTime(pbRSD, &tEndTime);
			pbRSD += 7;
			dwStartSec = TimeToSeconds(tStartTime);
			dwEndSec = TimeToSeconds(tEndTime);
			iRet = GetSchMtrResult(piStart, pbBuf, wBufSize, dwStartSec, dwEndSec);
			if (iRet < 0)
				goto SpecReadRecord_ret;
			//*pwRetNum = pbBuf[1];
			pbBuf += iRet;
			break;
		case 0x60000200:
			//*pbBuf++ = 01;
			//pbStart=  pbBuf;
			iRet = selector2_60000200(pbOAD, pbRSD-4, pbRCSD, pbBuf, wBufSize, pwRetNum);
			if (iRet < 0)
				goto SpecReadRecord_ret;
			//*pwRetNum = pbBuf[1];
			pbBuf += iRet;
			break;
		default:
			DTRACE(DB_FAPROTO, ("SpecReadRecord: RSD=2 unsupport dwOad=0x%08x.\n", dwOAD));
			goto SpecReadRecord_ret;
		}
	}
	else if (bRSD == 0)//台体协议一致性测试修改
	{
		dwOAD = OoOadToDWord(pbOAD);
		const ToaMap* pOI = GetOIMap(dwOAD & 0xFFFFFF00);
		if (pOI == NULL)
		{
			DTRACE(DB_FAPROTO, ("SpecReadRecord: pOI is NULL, dwOAD=0x%08x.\n", dwOAD));
			return -1;
		}
		switch (dwOAD & 0xFFFFFF00)
		{
		case 0x60000200:	//采集档案配置表
			bSubIdx = 1;
			if (bSubIdx != 1)	//档案信息只能用到子属性1，其它为结构体，无法访问
			{
				DTRACE(DB_FAPROTO, ("SpecReadRecord: dwOAD=0x%08x subidx=%d invalid.\n", dwOAD, bSubIdx));
				return -1;
			}
			for (WORD i=0; i<POINT_NUM; i++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				if (((iRet=ReadItemEx(BANK0, i, pOI->wID, bBuf))>0) && !IsAllAByte(bBuf, 0, sizeof(bBuf)))
				{
					p = OoGetField(bBuf+1, pOI->pFmt, pOI->wFmtLen, bSubIdx-1, &wDataLen, &bType);	//+1:0x6000档案的第一个自己为整个采集单元的数据长度
					if (p != NULL)
					{
						//if (memcmp(pbRSD, p, wDataLen) == 0)	//格式类型比较 && 数据比较
						{
							if (iRet > wBufSize)
							{
								DTRACE(DB_FAPROTO, ("SpecReadRecord: Buffer overflow, iRet=%d, wBufSize=%d.\n", iRet, wBufSize));
								goto SpecReadRecord_ret;
							}
							else
							{
								memcpy(pbBuf, bBuf+1, bBuf[0]);
								pbBuf += bBuf[0];
								*pwRetNum += 1;
							}
						}
					}
					else
					{
						DTRACE(DB_FAPROTO, ("SpecReadRecord: i=%d, pointer is Null!!!!\n", i));
					}
				}
			}
			break;
		case 0x60120200:	//任务采集方案集
			for (WORD i=0; i<SCH_NO_NUM; i++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				if ((iRet = GetTaskConfigFromTaskDb(i, bBuf)) > 0)
				{
					if (wCopyLen + iRet > wBufSize)
					{
						DTRACE(DB_FAPROTO, ("SpecReadRecord: Buffer overflow, dwOAD=%04x, iRet=%d, wBufSize=%d.\n", dwOAD, iRet, wBufSize));
						goto SpecReadRecord_ret;
					}
					else
					{
						wCopyLen += iRet;
						memcpy(pbBuf, bBuf, iRet);
						pbBuf += iRet;
						*pwRetNum += 1;
					}
				}
			}
			break;
		case 0x60140200:	//普通采集方案集
		case 0x60160200:	//事件采集方案集
		case 0x60180200:	//透明采集方案集
		case 0x601C0200:	//上报采集方案集
			if ((dwOAD & 0xFFFFFF00) == 0x60140200)
				bSchType = SCH_TYPE_COMM;
			else if ((dwOAD & 0xFFFFFF00) == 0x60160200)
				bSchType = SCH_TYPE_EVENT;
			else if ((dwOAD & 0xFFFFFF00) == 0x60180200)
				bSchType = SCH_TYPE_TRANS;
			else //0x601C0200
				bSchType = SCH_TYPE_REPORT;
		
			for (WORD i=0; i<SCH_NO_NUM; i++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				if ((iRet = GetSchFromTaskDb(i, bSchType, bBuf)) > 0)
				{
					//p = OoGetField(bBuf, pOI->pFmt, pOI->wFmtLen, bSubIdx-1, &wDataLen, &bType);
					//if (p!=NULL && (memcmp(pbRSD, p, wDataLen)==0)) //格式类型比较 && 数据比较
					{
						if (wCopyLen + iRet > wBufSize)
						{
							DTRACE(DB_FAPROTO, ("SpecReadRecord: Buffer overflow, dwOAD=%04x, iRet=%d, wBufSize=%d.\n", dwOAD, iRet, wBufSize));
							goto SpecReadRecord_ret;
						}
						else
						{
							wCopyLen += iRet;
							memcpy(pbBuf, bBuf, iRet);
							pbBuf += iRet;
							*pwRetNum += 1;
						}
					}
				}
			}
			break;
		default:
			DTRACE(DB_FAPROTO, ("SpecReadRecord: RSD=0 unsupport dwOad=0x%08x.\n", dwOAD));
			goto SpecReadRecord_ret;
		}
		
	}

	iRet = pbBuf - pbBuf0;
	pbBuf = pbBuf0;
	return iRet;

SpecReadRecord_ret:
	*pbBuf++ = DAR;
	*pbBuf++ = DAR_OTHER;

	iRet = pbBuf - pbBuf0;
	pbBuf = pbBuf0;

	return iRet;
}


//描述: 通信协议调用的读记录接口
//参数：@ pbOAD	对象属性描述表
//	   @ pbRSD对应协议中的RSCD
//	   @ pbRCSD 对应协议中的RCSD
//	   @ piTabIdx 对应任务方案的偏移，第一次调用传递-1，后续调用者不能改变
//	   @ piStart 用来传入及返回搜索的起始位置,第一次调用传递-1，后续调用者不能改变
//				搜索结束时，本函数把它设置为-2
//	   @ wSchNum 每次搜索允许返回的记录笔数，搜到这么多笔立刻返回
//	   @ pwRetNum 本次搜索返回的记录笔数，pwRetNum< wSchNum表示搜索结束
//	   @ pbBuf用来返回记录的内容
//返回:如果正确则返回数据的长度,否则返回负数
int ReadRecord(BYTE* pbOAD, BYTE* pbRSD, BYTE* pbRCSD, int *piTabIdx, int* piStart, BYTE* pbBuf, WORD wBufSize, WORD* pwRetNum)
{
	BYTE bTmpBuf[FRZRELA_ID_LEN];	//EVT_ATTRTAB_LEN
	TFieldParser tFixFields, tDataFields;
	int iRet, iRecLen;
	int iRetNum, iRcsdLen;
	WORD wSucRetNum, wScanNum = 0;
	WORD wSchNum, i, wRcsdIdx, wRcsdNum;
	char pszTableName[64];
	BYTE bTmpRcsd[128];
	BYTE *pbBuf0 = pbBuf;

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));
	tDataFields.pbCfg = bTmpBuf;
	tDataFields.wCfgLen = sizeof(bTmpBuf);
	
	if (IsSpecOobAttrDescOAD(pbOAD))
	{
		iRecLen = SpecReadRecord(pbOAD, pbRSD, pbRCSD, piStart, pbBuf, wBufSize, pwRetNum);
	}
	else
	{
		wRcsdIdx = 1;	//协议层中RCSD第一个字节为CSD个数，跳过，即索引初始化为1
		iRetNum = SearchTable(pbOAD, pbRSD, pbRCSD, wRcsdIdx, piTabIdx, pszTableName, &tFixFields, &tDataFields);
		if(iRetNum == -2)
		{
			return iRetNum;
		}

		if (iRetNum <= 0)	//搜表失败
			return -4;

		iRecLen = 0;
		wSucRetNum = 0;
		wRcsdNum = *pbRCSD;
		wSchNum = wBufSize/(tFixFields.wTotalLen + tDataFields.wTotalLen);	//最大支持的内存空间
		DTRACE(DB_TASK, ("###1-wBufSize=%d, wFieldsLen=%d, wSchNum=%d.\n", wBufSize, tFixFields.wTotalLen + tDataFields.wTotalLen, wSchNum));

		if (wRcsdNum == iRetNum)	//完全匹配，只有一张表
		{
			DTRACE(DB_TASK, ("###2-wBufSize=%d, wFieldsLen=%d, wSchNum=%d.\n", wBufSize, tFixFields.wTotalLen + tDataFields.wTotalLen, wSchNum));

NEXT_ONE_TABLE:	//为了解决普通采集方案里跨表MS问题
			if (wSchNum > wSucRetNum)
				iRet = ReadTable(pszTableName, &tFixFields, &tDataFields, pbOAD, pbRSD, pbRCSD, piStart, wSchNum-wSucRetNum, pwRetNum, pbBuf);
			else
				iRet = -1;

			if (iRet > 0)
			{
				wSucRetNum += *pwRetNum;
				pbBuf += iRet;
			}

			if (wSchNum > wSucRetNum)	//缓冲未满，继续检索数据
			{
				if (*piTabIdx != 0)	//所有表还未遍历完
				{
					if (*piStart==-2 || *piStart == -1)	//一张表的数据全部检索完了，需切表
					{
						do 
						{
							*piStart = -1;	//重新初始化
							*piTabIdx = *piTabIdx + 1;	//切下一张表

							wRcsdIdx = 1;	//协议层中RCSD第一个字节为CSD个数，跳过，即索引初始化为1
							iRetNum = SearchTable(pbOAD, pbRSD, pbRCSD, wRcsdIdx, piTabIdx, pszTableName, &tFixFields, &tDataFields);
							if (iRetNum == wRcsdNum)	//搜表成功
							{
								wBufSize = wBufSize - (pbBuf-pbBuf0);
								wSchNum = wBufSize/(tFixFields.wTotalLen + tDataFields.wTotalLen) + wSucRetNum;	//最大支持的内存空间
								DTRACE(DB_TASK, ("###3-wBufSize=%d, wFieldsLen=%d, wSchNum=%d.\n", wBufSize, tFixFields.wTotalLen + tDataFields.wTotalLen, wSchNum));

								goto NEXT_ONE_TABLE;
							}

							if (*piTabIdx == 0)	//表检索结束，直接退出
								break;
						}while (iRetNum != wRcsdNum);
					}
				}
			}

			*pwRetNum = wSucRetNum;
			iRecLen = pbBuf - pbBuf0;
		}
		else	//部分匹配，存在多张表
		{
			DTRACE(DB_TASK, ("###4-wBufSize=%d, wFieldsLen=%d, wSchNum=%d.\n", wBufSize, tFixFields.wTotalLen + tDataFields.wTotalLen, wSchNum));

			i = 0;
			wSucRetNum = 0;
			wScanNum = 0;
			wRcsdIdx = 0;
			iRcsdLen = ScanRCSD(pbRCSD, false);
			while (wScanNum++<wRcsdNum && iRcsdLen>=0)
			{
				memset(bTmpRcsd, 0, sizeof(bTmpRcsd));
				bTmpRcsd[0] = 1;	//处理协议层RCSD中的一个CSD
				memcpy(&bTmpRcsd[1], pbRCSD+1+wRcsdIdx, sizeof(bTmpRcsd)-1);	//RCSD中的参数全部拷贝，实际处理RCSD中的CSD个数由bTmpRcsd[0]决定
				iRetNum = SearchTable(pbOAD, pbRSD, bTmpRcsd/*pbRCSD*/, wRcsdIdx, piTabIdx, pszTableName, &tFixFields, &tDataFields);
				if (iRetNum <= 0)	//搜表失败，直接填充无效数据NULL
				{
					*pbBuf++ = DT_NULL;
					iRetNum = 1;	//强制赋1，方便后面计算wRcsdIdx偏移
				}
				else
				{
					if ((iRet=ReadTable(pszTableName, &tFixFields, &tDataFields, pbOAD, pbRSD, bTmpRcsd, piStart, 1, pwRetNum, pbBuf)) <= 0)
							return 0;// return -1;	//直接退出, 协议一致性 Get_10
					pbBuf += iRet;
					wSucRetNum += *pwRetNum;
				}

				if (i++ >= wSchNum)	//数据是否溢出
				{
					*pwRetNum = wSucRetNum;
					iRecLen = pbBuf - pbBuf0;
					pbBuf = pbBuf0;
					return iRecLen;
				}

				//for (WORD k=0; k<iRetNum; k++)	//根据返回的个数，在wRcsdIdx基础上计算偏移
				wRcsdIdx += ScanCSD(pbRCSD+1+wRcsdIdx, false);

				if (wRcsdIdx >= iRcsdLen)	
				{
					*piStart = -1;
					break;
				}

				*piStart = -1;

			}

			if (*pbOAD != 0x50)	//冻结不需要按表数累加OI_FRZ
				*pwRetNum = wSucRetNum;
			iRecLen = pbBuf - pbBuf0;
		}

		pbBuf = pbBuf0;
	}

	return iRecLen;
}

//描述:任务库的单表读取接口
//参数：@pszTableName	表名
//	   @ pFixFields	固定字段
//	   @ pDataFields	数据字段，如果没有数据字段传入NULL
//	   @ pbRSD对应协议中的RSCD
//	   @ pbRCSD 传递本任务库表支持的字段
//	   @ piStart 用来传入及返回搜索的起始位置,第一次调用传递-1，后续调用者不能改变
//				搜索结束时，本函数把它设置为-2
//	   @ wSchNum 每次搜索允许返回的记录笔数，搜到这么多笔立刻返回
//	   @ pwRetNum 本次搜索返回的记录笔数，pwRetNum< wSchNum表示搜索结束
//	   @ pbBuf用来返回记录的内容
//返回:如果正确则返回数据的长度,否则返回负数
int ReadTable(char* pszTableName, TFieldParser* pFixFields, TFieldParser* pDataFields, 
			  BYTE* pbOAD, BYTE* pbRSD, BYTE* pbRCSD, int* piStart, WORD wSchNum, WORD* pwRetNum, BYTE* pbBuf)
{
	int fd;
	int iRet;
	WORD wSchSubNum, wRetNum, wRecLen;
	//BYTE bRecBuf[APDUSIZE-20];
	//BYTE bRecBuf[1024];
	BYTE bRecBuf[4096];
	BYTE *pbRec;
	BYTE *pbRespRec = pbBuf;
	BYTE *pbRCSD0 = pbRCSD;
	BYTE bCSDNum, bIndex;
	BYTE bRsdMethod, bRecNo;
	bool fFromEnd = false;

	fd = TdbOpenTable(pszTableName, O_RDWR);
	if (fd < 0)
		return -1;

	bRsdMethod = *pbRSD;
	if (bRsdMethod==9 || bRsdMethod==10)	//RSD方法9,10
	{
		bRecNo = pbRSD[1];
		fFromEnd = true;
	}
	else if (bRsdMethod == 0)//RSD方法0特殊处理为方法9，台体协议一致性测试修改
	{
		bRsdMethod = 9;
		bRecNo = 1;
		fFromEnd = true;	
	}
		

	wRetNum = 0;
	wRecLen = pFixFields->wTotalLen + pDataFields->wTotalLen;
	memset(bRecBuf, 0, sizeof(bRecBuf));
	DTRACE(DB_TASK, ("###ReadTable1:%s,  piStart=%d, MaxBuf size=%d.\n", pszTableName, *piStart, wSchNum*wRecLen));
	if (wSchNum*wRecLen > sizeof(bRecBuf))
	{
		DTRACE(DB_TASK, ("###ReadTable1-1: Error %s,  piStart=%d, MaxBuf size=%d.\n", pszTableName, *piStart, wSchNum*wRecLen));
		return -1;
	}
		
	while (TdbReadRec(fd, piStart, bRecBuf, (wSchNum-wRetNum)*wRecLen, fFromEnd, pwRetNum) >= 0)
	{
		DTRACE(DB_TASK, ("###ReadTable2:%s piStart=%d, MaxBuf size=%d, pwRetNum=%d.\n", pszTableName, *piStart, (wSchNum-wRetNum)*wRecLen, *pwRetNum));
		for (WORD i=0; i<*pwRetNum; i++)
		{
			wSchSubNum = 0;
			pbRec = &bRecBuf[i*wRecLen];
			if (RecMatch(pbRec, pFixFields, pbRSD))
			{
				//方法9特殊处理
				if (bRsdMethod == 9)
				{
					if (bRecNo <= *pwRetNum)	//上第N条记录在当前搜索条数的范围内
					{
						if (i != (bRecNo-1))
							continue;
					}
					else
					{
						bRecNo -= *pwRetNum;	//继续从任务库找记录
						break;
					}
				}

				pbRCSD = pbRCSD0;
				bCSDNum = *pbRCSD++;
				for (bIndex=0; bIndex<bCSDNum; bIndex++)
				{
					iRet = ReadRecField(pbRec, 0, pFixFields, pbOAD, pbRCSD, pbRespRec);
					if (iRet < 0)	//固定字段未找到记录,继续数据字段
					{
						iRet = ReadRecField(pbRec, pFixFields->wTotalLen, pDataFields, pbOAD, pbRCSD, pbRespRec);
						if (iRet < 0)	//数据字段未找到记录，直接返回
						{
// 							TdbCloseTable(fd);
// 							goto ReadTable_ret;
							*pbRespRec++ = 0x00;
							wSchSubNum++;
						}
						else
						{
							pbRespRec += iRet;
							wSchSubNum++;
						}
					}
					else
					{
						pbRespRec += iRet;
						wSchSubNum++;
					}
					pbRCSD += ScanCSD(pbRCSD, false);
				}

				if (bCSDNum == wSchSubNum)	//找到一笔记录
					wRetNum++;

				if(wRetNum >= wSchNum)	//笔数达到限制
					goto ReadTable_ret;

				//方法9、10特殊处理。方法10比较特别，涉及到MS的集合，本函数每次只能处理一个电表，所以RSD中的MS由外围函数调用时，必须将MS拆分成一个一个表地址进行数据获取
				if (bRsdMethod == 10)
				{
					if (wRetNum == bRecNo)	//检索到上N条记录就退出
					{
						*piStart = -2;
						goto ReadTable_ret;
					}
				}
				else if (bRsdMethod == 9)	//到这里表明已经找到了上第N笔记录
				{
					*piStart = -2;
					goto ReadTable_ret;
				}
			}
		}

		if (*piStart == -2)
			goto ReadTable_ret;
	}
	
ReadTable_ret:
	DTRACE(DB_TASK, ("###ReadTable3:%s, piStart=%d, MaxBuf size=%d.\n", pszTableName, *piStart, wSchNum*wRecLen));
	
	TdbCloseTable(fd);
	*pwRetNum = wRetNum;

	return pbRespRec - pbBuf;
}

//描述: 取得数据项的信息
//参数：@ bType	数据类型，同Data的数据类型定义
//	   @ pItemDesc 数据项目描述，可以OAD、ROAD、CSD
//	   @ pwLen	用来返回数据长度
//	   @ppFmt	用来返回格式描述串
//	   @pwFmtLen	用来返回格式描述串的长度
//返回：正确则返回true，否则返回false
bool OoGetItemInfo(BYTE bType, BYTE* pItemDesc, WORD* pwLen, BYTE** ppFmt, WORD* pwFmtLen)
{
	return false;
}

//描述：针对Arry数据特殊处理
//参数：@pbDst 目的数据
//		@pbSrc 元数据
//		@wSrcLen 元数据长度
//返回：pbDst数据的长度
int FmtArryData(BYTE *pbDst, BYTE *pbSrc, WORD *pwRetSrcLen /*WORD wSrcLen, BYTE *pFmt, WORD wFmtLen*/)
{
	BYTE bArryNum;
	BYTE bValidArryNum = 0;
	BYTE bFmtLen;
	BYTE *pbSrc0 = pbSrc;
	BYTE *pbDst0 = pbDst;
	int iRet = -1;
	WORD wDstLen;
	WORD wRetSrcLen;

	if (*pbSrc == DT_ARRAY)
	{
		*pbDst++ = *pbSrc++;
		bArryNum = *pbSrc++;
		pbDst++;	//跳过array成员个数，后面通过bValidArryNum填充
		if (bArryNum != 0)
		{
			for (BYTE i=0; i<bArryNum; i++)
			{
				if (*pbSrc == DT_ARRAY)
				{
					pbDst += FmtArryData(pbDst, pbSrc, &wRetSrcLen);
					pbSrc += wRetSrcLen;
				}
				else
				{
					bFmtLen = OoGetDataTypeLen(pbSrc);
					if (bFmtLen==1 || IsAllAByte(pbSrc+1, 0xfe, bFmtLen-1))	//+1跳过格式
					{
						pbSrc += bFmtLen;	//bFmtLen+1： 格式+格式的字节数
					}
					else
					{
						bValidArryNum++;
						memcpy(pbDst, pbSrc, bFmtLen);
						pbSrc += bFmtLen;	//bFmtLen+1： 格式+格式的字节数
						pbDst += bFmtLen;	//bFmtLen+1： 格式+格式的字节数
					}
				}
			}
			pbDst0[1] = bValidArryNum;
		}
		else
		{
			pbDst0[1] = 0x00;	//ARRAY 中有0个成员
		}

		iRet = pbDst - pbDst0;
		pbDst = pbDst0;

		*pwRetSrcLen = pbSrc - pbSrc0;
	}

	return iRet;
}

//描述：ROAD关联OAD数据拷贝
//参数：@pbDst 目的数据
//		@pbSrc 源数据
//		@pwRetSrcLen 源数据返回的偏移
//返回：成功返回目的数据长度，否则返回-1
int OoCopyDataROADLinkOAD(BYTE *pbDst, WORD *pwRetDstLen, BYTE *pbSrc, WORD *pwRetSrcLen, BYTE *pbFmt, WORD wFmtLen)
{
	int iLen;
	WORD wRetSrcLen;
	BYTE *pbDst0 = pbDst;
	BYTE *pbSrc0 = pbSrc;
	BYTE *pROAD = pbFmt;
	BYTE bLkNum;

	pROAD += 4;	//对象属性描述符
	bLkNum = *pROAD++;	//关联对象属性描述符个数
	for (BYTE i=0; i<bLkNum; i++)
	{
		if (*pbSrc == DT_ARRAY)
		{
			wRetSrcLen = 0;
			iLen = FmtArryData(pbDst, pbSrc, &wRetSrcLen);
			if (iLen < 0)	//小于0无法计算pbSrc0的偏移，故不在判断余下的数据，直接退出
			{
				*pbDst++ = 0x00;
				DTRACE(DB_FAPROTO, ("OoCopyDataROADLinkOAD()1 error i=%d.\n", i));
				break;
			}
			else
			{
				pbDst += iLen;
				pbSrc += wRetSrcLen;
			}
		}
		else
		{
			iLen = OoGetDataTypeLen(pbSrc);
			if (iLen < 0)
			{
				*pbDst++ = 0x00;
				DTRACE(DB_FAPROTO, ("OoCopyDataROADLinkOAD()2 error i=%d.\n", i));
				break;
			}
			else
			{
				memcpy(pbDst, pbSrc, iLen);
				pbDst += iLen;
				pbSrc += iLen;
				
			}
		}
	}

	*pwRetDstLen = pbDst - pbDst0;
	pbDst = pbDst0;

	*pwRetSrcLen = pbSrc - pbSrc0;
	pbSrc = pbSrc0;

	if (*pwRetDstLen > 0)
		return *pwRetDstLen;
	else
		return -1;
}

bool OoCopyDataIsValid(BYTE *pbBuf, WORD wLen)
{
	if (IsAllAByte(pbBuf, 0xee, wLen))
		return false;

	if (IsAllAByte(pbBuf, 0xfe, wLen))
		return false;

	return true;
}

int OoCopySrcData(BYTE *pbSrc, BYTE *pFmt, WORD wFmtLen, BYTE *pbDst, WORD *pwRetSrcOffset)
{
	const ToaMap* pOAMap;
	int iTypeLen, iDataLen, iRet, iLen;
	DWORD dwOIAtt;
	WORD wVlidFmtLen;

	BYTE *pbSrc0 = pbSrc;
	BYTE *pbDst0 = pbDst;
	BYTE bArryNum, bIndex;
	BYTE *pStartFmt, *pEndFmt;
	bool fMapSuc = false;
	
	iDataLen = 0;
	dwOIAtt = OoOadToDWord(pFmt);
	dwOIAtt &= OAD_FEAT_MASK;	//获取OAD, 只屏蔽掉属性特征
	bIndex = (BYTE)(dwOIAtt & 0xff);
	pOAMap = GetOIMap(dwOIAtt);
	if (pOAMap != NULL)
	{
		if ((iRet=GetItemLen(BANK0, pOAMap->wID)) < 0)
			return -1;
		iDataLen = iRet;
		pStartFmt = pOAMap->pFmt;
		if ((iLen=OoGetDataTypeFmtValidLen(pOAMap->pFmt, pOAMap->wFmtLen, &wVlidFmtLen)) > 0)
			pEndFmt = pOAMap->pFmt + wVlidFmtLen - 1;
		else
			pEndFmt = pOAMap->pFmt + pOAMap->wFmtLen;

		fMapSuc = true;
	}
	else if (bIndex != 0)	//读子属性
	{
		pOAMap = GetOIMap(dwOIAtt & 0xFFFFFF00);
		if (pOAMap != NULL)
		{
			BYTE bBuf[256];
			BYTE *pbFmt, *pbPtr;
			WORD wFmtLen, wLen;
			BYTE bType;

			memset(bBuf, 0, sizeof(bBuf));
			iDataLen = OoReadAttr(dwOIAtt>>16, (dwOIAtt>>8)&0xff, bBuf, &pbFmt, &wFmtLen);
			if (iDataLen > 0)	//特殊OI，ID本身定义的就是第N个属性
			{
				if (!IsNeedRdSpec(pOAMap))	//非特殊OI
				{
					pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, bIndex-1, &wLen, &bType);	
					if (pbPtr == NULL)
						return -1;
					iRet = OoGetDataTypeLen(pbPtr);
					if (iRet < 0)
						return -1;
					iDataLen = iRet;
					pStartFmt = &bType;
					pEndFmt = pStartFmt + 1;
					fMapSuc = true;
				}
				else
				{
					pStartFmt = pbFmt;

					if ((iLen=OoGetDataTypeFmtValidLen(pOAMap->pFmt, pOAMap->wFmtLen, &wVlidFmtLen)) > 0)
						pEndFmt = pbFmt + wVlidFmtLen - 1;
					else
						pEndFmt = pbFmt + pOAMap->wFmtLen;

					fMapSuc = true;
				}
			}
		}
	}

	if (fMapSuc)
	{
		while (pStartFmt < pEndFmt)
		{
			if (*pStartFmt++==DT_ARRAY && *pbSrc==DT_ARRAY)
			{
				pbSrc++;	
				bArryNum = *pStartFmt++;
				if (bArryNum != *pbSrc)	//如果格式中个数不等于数据中的个数，就用数据中的个数
					bArryNum = *pbSrc;
				pbSrc++;

				*pbDst++ = DT_ARRAY;
				*pbDst++ = bArryNum;

				for (BYTE i=0; i<bArryNum; i++)
				{
					iTypeLen = OoGetDataTypeLen(pbSrc);
					if (iTypeLen < 0)
					{
						DTRACE(DB_FAPROTO, ("OoCopySrcData: OoGetDataTypeLen1 err, OAD=0x%08x.\n", pOAMap->dwOA));
						goto GOTO_RET;
					}
					if (OoCopyDataIsValid(pbSrc+1, iTypeLen-1))
					{
						memcpy(pbDst, pbSrc, iTypeLen);
						pbDst += iTypeLen;
					}
					else
						*pbDst++ = 0;

					pbSrc += iTypeLen;
				}
				pStartFmt++;
			}
			else
			{
				iTypeLen = OoGetDataTypeLen(pbSrc);
				if (iTypeLen < 0)
				{
					DTRACE(DB_FAPROTO, ("OoCopySrcData: OoGetDataTypeLen2 err, OAD=0x%08x.\n", pOAMap->dwOA));
					goto GOTO_RET;
				}
				if (OoCopyDataIsValid(pbSrc+1, iTypeLen-1))
				{
					memcpy(pbDst, pbSrc, iTypeLen);
					pbDst += iTypeLen;
				}
				else
					*pbDst++ = 0;

				pbSrc += iTypeLen;
				pStartFmt++;
			}
		}
	}
	else
	{
		*pbDst++ = 0x00;
		DTRACE(DB_FAPROTO, ("OoCopySrcData: OoGetDataTypeLen3 err, OAD=0x%08x.\n", pOAMap->dwOA));
	}

GOTO_RET:
	*pwRetSrcOffset = pbSrc - pbSrc0;
	iRet = pbDst - pbDst0;
	pbDst = pbDst0;
	return iRet;
}

int OoCopyOneOadData(BYTE *pbSrc, BYTE *pFmt, WORD wFmtLen, BYTE *pbDst, WORD *pwRetFmtLen, WORD *pwRetSrcOffset)
{
	int iRet;
	BYTE *pFmt0 = pFmt;
	BYTE *pbDst0 = pbDst;

	iRet = OoCopySrcData(pbSrc, pFmt, wFmtLen, pbDst, pwRetSrcOffset);
	if (iRet > 0)
		pbDst += iRet;
	else
		*pbDst++ = 0x00;
	pFmt += 4;

	*pwRetFmtLen = pFmt - pFmt0;
	iRet = pbDst - pbDst0;
	pbDst = pbDst0;
	return iRet;
}

int OoFormatSrcData(BYTE *pbSrc, WORD wSrcLen, BYTE *pFmt, WORD wFmtLen, BYTE *pbDst)
{
	int iRet = -1;
	WORD wRetFmtLen=0, wRetSrcOffset=0;
	BYTE *pbSrc0 = pbSrc;
	BYTE *pFmt0 = pFmt;
	BYTE *pbDst0 = pbDst;
	BYTE bLkNum;
	BYTE bRcsdNum;
	BYTE bType = *pFmt++;

	while(pFmt-pFmt0 < wFmtLen)
	{
		if (bType == DT_RCSD)
		{
			bRcsdNum = *pFmt++;
			for (BYTE bRcsdIdx=0; bRcsdIdx<bRcsdNum; bRcsdIdx++)
			{
				if (*pFmt++ == 0)	//OAD
				{
					iRet = OoCopyOneOadData(pbSrc0, pFmt, wFmtLen-(pFmt-pFmt0), pbDst, &wRetFmtLen, &wRetSrcOffset);
					if (iRet > 0)
					{
						pbDst += iRet;
						pbSrc0 += wRetSrcOffset;
					}
					else
					{
						*pbDst++ = 0x00;
						goto GOTO_RET;
					}
					pFmt += wRetFmtLen;
				}
				else	//ROAD
				{
					pFmt += 4;	//跳过主OAD
					bLkNum = *pFmt++;
					for (BYTE i=0; i<bLkNum; i++)
					{
						iRet = OoCopyOneOadData(pbSrc0, pFmt, wFmtLen-(pFmt-pFmt0), pbDst, &wRetFmtLen, &wRetSrcOffset);
						if (iRet > 0)
						{
							pbDst += iRet;
							pbSrc0 += wRetSrcOffset;
						}
						else
						{
							*pbDst++ = 0x00;
							goto GOTO_RET;
						}
						pFmt += wRetFmtLen;
					}
				}
			}
		}
		else if (bType == DT_OAD)
		{
			iRet = OoCopyOneOadData(pbSrc0, pFmt, wFmtLen-(pFmt-pFmt0), pbDst, &wRetFmtLen, &wRetSrcOffset);
			if (iRet > 0)
			{
				pbDst += iRet;
				pbSrc0 += wRetSrcOffset;
			}
			else
			{
				*pbDst++ = 0x00;
				goto GOTO_RET;
			}
			pFmt += wRetFmtLen;
		}
		else if (bType == DT_CSD)
		{
			if (*pFmt++ == 0)	//choice 0:OAD
			{
				iRet = OoCopyOneOadData(pbSrc0, pFmt, wFmtLen-(pFmt-pFmt0), pbDst, &wRetFmtLen, &wRetSrcOffset);
				if (iRet > 0)
				{
					pbDst += iRet;
					pbSrc0 += wRetSrcOffset;
				}
				else
				{
					*pbDst++ = 0x00;
					goto GOTO_RET;
				}
				pFmt += wRetFmtLen;
			}
			else	//ROAD
			{
				pFmt += 4;	//跳过主OAD
				bLkNum = *pFmt++;
				for (BYTE i=0; i<bLkNum; i++)
				{
					iRet = OoCopyOneOadData(pbSrc0, pFmt, wFmtLen-(pFmt-pFmt0), pbDst, &wRetFmtLen, &wRetSrcOffset);
					if (iRet > 0)
					{
						pbDst += iRet;
						pbSrc0 += wRetSrcOffset;
					}
					else
					{
						*pbDst++ = 0x00;
						goto GOTO_RET;
					}
					pFmt += wRetFmtLen;
				}
			}
		}
		else if (bType == DT_ROAD)
		{
			pFmt += 4;	//跳过主OAD
			bLkNum = *pFmt++;
			for (BYTE i=0; i<bLkNum; i++)
			{
				iRet = OoCopyOneOadData(pbSrc0, pFmt, wFmtLen-(pFmt-pFmt0), pbDst, &wRetFmtLen, &wRetSrcOffset);
				if (iRet > 0)
				{
					pbDst += iRet;
					pbSrc0 += wRetSrcOffset;
				}
				else
				{
					*pbDst++ = 0x00;
					goto GOTO_RET;
				}
				pFmt += wRetFmtLen;
			}
		}
		else 
		{
			memcpy(pbDst, pbSrc0, wSrcLen);
			pbDst += wSrcLen;
			pbSrc0 += wRetSrcOffset;
			break;
		}
	}

GOTO_RET:
	iRet = pbDst - pbDst0;
	pbDst = pbDst0;

	return iRet;
}

//描述:面向对象的数据拷贝
//参数：@ pbDst	目标地址
//	   @ pbSrc	源地址
//	   @ wLen	数据长度
//	   @ pFmt 格式描述串
//	   @ wFmtLen格式描述串长度
//返回:如果正确则返回数据的长度,否则返回负数
int OoCopyData(BYTE* pbOAD, BYTE* pbDst, BYTE* pbSrc, WORD wItemOffset, WORD wLen, BYTE* pFmt, WORD wFmtLen)
{
	int iRet;
	WORD wRetSrcLen, wRetDstLen;
	DWORD dwOAD;
	BYTE bTsaLen;
	WORD wSchMtrLen = 0;
	BYTE *pbDst0 = pbDst;
	BYTE *pbSrc0 = pbSrc+wItemOffset;

	dwOAD = OoOadToDWord(pbOAD);

	if ((*pbSrc0==DAR && *(pbSrc0+1)!=0 && IsAllAByte(pbSrc0+2, 0, wLen-2)) || (IsAllAByte(pbSrc0, 0, wLen)))	//DAR=0 || 数据全为0，表示数据抄读失败
	{
// 		memcpy(pbDst, pbSrc0, 2);	//DAR + DAR_FMT 
// 		pbDst += 2;
		*pbDst++ = 0;
	}
	else if (dwOAD>=0x30000000 && dwOAD<0x40000000)	//事件
	{
		if ((*pFmt==DT_OAD) && (OoOadToDWord(pFmt+1)==0x20240200))	//发生源
		{
			switch(dwOAD)
			{
			case 0x30090200:	//电能表正向有功需量超限事件	以下发生源为NULL
			case 0x300A0200:	//电能表反向有功需量超限事件
			case 0x300B0200:	//电能表无功需量超限事件
			case 0x300C0200:	//电能表功率因数超下限事件
			case 0x300D0200:	//电能表全失压事件
			case 0x300E0200:	//电能表辅助电源掉电事件
			case 0x300F0200:	//电能表电压逆相序事件
			case 0x30100200:	//电能表电流逆相序事件
			case 0x30110200:	//电能表掉电事件
			case 0x30130200:	//电能表清零事件
			case 0x30140200:	//电能表需量清零事件
			case 0x30150200:	//电能表事件清零事件
			case 0x30160200:	//电能表校时事件
			case 0x30170200:	//电能表时段表编程事件
			case 0x30180200:	//电能表时区表编程事件
			case 0x30190200:	//电能表周休日编程事件
			case 0x301A0200:	//电能表结算日编程事件
			case 0x301B0200:	//电能表开盖事件
			case 0x301C0200:	//电能表开端钮盒事件
			case 0x301D0200:	//电能表电压不平衡事件
			case 0x301E0200:	//电能表电流不平衡事件
			case 0x301F0200:	//电能表跳闸事件
			case 0x30200200:	//电能表合闸事件
			case 0x30210200:	//电能表节假日编程事件
			case 0x30220200:	//电能表有功组合方式编程事件
			case 0x30240200:	//电能表费率参数表编程事件
			case 0x30250200:	//电能表阶梯表编程事件
			case 0x30260200:	//电能表密钥更新事件
			case 0x30270200:	//电能表异常插卡事件
			case 0x30280200:	//电能表购电记录
			case 0x30290200:	//电能表退费记录
			case 0x302A0200:	//电能表恒定磁场干扰事件
			case 0x302B0200:	//电能表负荷开关误动作事件
			case 0x302C0200:	//电能表电源异常事件
			case 0x302D0200:	//电能表电流严重不平衡事件	
			case 0x302E0200:	//电能表时钟故障事件
			case 0x302F0200:	//电能表计量芯片故障事件
			case 0x31000200:	//终端初始化事件
			case 0x31010200:	//终端版本变更事件
			case 0x31040200:	//终端状态量变位事件
			//case 0x31060200:	//终端停/上电事件
			case 0x31090200:	//终端消息认证错误事件
			case 0x31100200:	//月通信流量超限事件
			case 0x31140200:	//终端对时事件
			case 0x311A0200:	//电能表在网状态切换事件
				*pbDst++ = 0x00;
				break;
			case 0x30230200:	//电能表无功组合方式编程事件
				memcpy(pbDst, pbSrc0, 2);	//DT_ENUM 带数据格式
				pbDst += 2;
				break;
			case 0x31070200:	//终端直流模拟量越上限事件
			case 0x31080200:	//终端直流模拟量越下限事件
				memcpy(pbDst, pbSrc0, 5);	//DT_OAD 带数据格式
				break;
			case 0x310A0200:	//终端故障记录
				memcpy(pbDst, pbSrc0, 2);	//DT_ENUM 带数据格式
				pbDst += 2;
				break;
			case 0x31050200:	//电能表时钟超差事件
			case 0x310B0200:	//电能表示度下降事件
			case 0x310C0200:	//电能量超差事件
			case 0x310D0200:	//电能表飞走事件
			case 0x310E0200:	//电能表停走事件
			case 0x310F0200:	//终端抄表失败事件
				*pbDst++ = *pbSrc0++;	//DT_TSA
				bTsaLen = *pbSrc0++;
				*pbDst++ = bTsaLen;		//length	//zhq modify 170428
				memcpy(pbDst, pbSrc0, bTsaLen);
				pbDst += bTsaLen;
				break;
			case 0x31060200:	//终端停/上电事件
			case 0x31150200:	//遥控跳闸记录
				memcpy(pbDst, pbSrc0, 2);	//DT_UNSIG 带数据格式
				pbDst += 2;
				break;
			case 0x31190200:	//终端电流回路异常事件
				memcpy(pbDst, pbSrc0, 2);	//DT_ENUM 带数据格式
				pbDst += 2;
				break;
			case 0x32020200:	//购电参数设置记录
				memcpy(pbDst, pbSrc0, 3);	//DT_OI 带数据格式
				pbDst += 3;
				break;
			default:
				break;
			}
		}
		else if ((*pFmt==DT_OAD) && (OoOadToDWord(pFmt+1)==0x33020206)) //编程对象列表  array OAD			
		{
			if (*pbSrc0 == DT_NULL)
				*pbDst++ = 0x00;
			else
			{
				bTsaLen = 2+5*pbSrc0[1];
				memcpy(pbDst, pbSrc0, bTsaLen);
				pbDst += bTsaLen;
			}
		}
		else if ((*pFmt==DT_OAD) && ((OoOadToDWord(pFmt+1)==0x330F0206) || (OoOadToDWord(pFmt+1)==0x330F0207) || (OoOadToDWord(pFmt+1)==0x330F0208)))		
		{
			if (*pbSrc0 == DT_NULL)
				*pbDst++ = 0x00;
			else
			{
				memcpy(pbDst, pbSrc0+1, *pbSrc0);	//DT_OI 带数据格式
				pbDst += *pbSrc0;			
			}
		}	
		else if ((*pFmt==DT_OAD) && (OoOadToDWord(pFmt+1)==0x330C0206)) //事件清零列表
		{
			if (*pbSrc0 == DT_NULL)
				*pbDst++ = 0x00;
			else
			{
				bTsaLen = 2+5*pbSrc0[1];
				memcpy(pbDst, pbSrc0, bTsaLen);
				pbDst += bTsaLen;
			}
		}
		else if ((*pFmt==DT_OAD) && (OoOadToDWord(pFmt+1)==0x33000200)) //事件上报状态
		{
			if (*pbSrc0 == DT_NULL)
				*pbDst++ = 0x00;
			else
			{
				bTsaLen = 2+CN_RPT_STATE_LEN*pbSrc0[1];
				memcpy(pbDst, pbSrc0, bTsaLen);
				pbDst += bTsaLen;
			}
		}
		else if ((*pFmt==DT_OAD) && (OoOadToDWord(pFmt+1)==0x20200200)) //事件结束时间
		{
			if (*pbSrc0 == DT_NULL)
				*pbDst++ = 0x00;
			else
			{
				memcpy(pbDst, pbSrc0, 8);
				pbDst += 8;
			}
		}
		else if ((*pFmt==DT_OAD) && (((OoOadToDWord(pFmt+1)) & 0x0000ff00 )==0x00008200))	//事件结束后数据
		{
			if (*pbSrc0 == DT_NULL)
				*pbDst++ = 0x00;
			else
			{
				memcpy(pbDst, pbSrc0, wLen);
				pbDst += wLen;
			}
		}	
		else
		{
			memcpy(pbDst, pbSrc0, wLen);
			pbDst += wLen;
		}	
	}		
	else
	{
		DWORD dwMainOAD = dwOAD;
		dwOAD = OoOadToDWord(pFmt+1);
		if ((*pFmt==DT_OAD) && (dwOAD==0x40010200 || dwOAD==0x202A0200))
		{
#ifdef GW_OOB_DEBUG_0x40010201
			*pbDst++ = DT_TSA;
#else
			*pbDst++ = DT_OCT_STR;
#endif
			bTsaLen = *pbSrc0++;
			*pbDst++ = bTsaLen+1;	//TSA长度
			*pbDst++ = bTsaLen - 1;	//TSA内部octet数据长度
			memcpy(pbDst, pbSrc0, bTsaLen);
			pbDst += bTsaLen;
		}
		else if ((*pFmt==DT_OAD) && (dwOAD==0x60400200 || dwOAD==0x60410200 || dwOAD==0x60420200))	//带上格式
		{
			*pbDst++ = DT_DATE_TIME_S;
			memcpy(pbDst, pbSrc0, wLen);
			pbDst += wLen;
		}
		else if ((*pFmt==DT_OAD) && (OoOadToDWord(pFmt+1)==0x3303206)) //搜表结果
		{
			if (*pbSrc0 == DT_NULL)
			{
				*pbDst++ = 0x00;
			}
			else
			{
				wSchMtrLen = 2+ONE_SCH_MTR_RLT_LEN*pbSrc0[1];
				memcpy(pbDst, pbSrc0, wSchMtrLen);
				pbDst += wSchMtrLen;
			}
		}
		else if ((*pFmt==DT_CSD) && (*(pFmt+1)==1))	//CSD->ROAD 内部存在关联OAD，需单独检索数据
		{
			wRetDstLen = 0;
			wRetSrcLen = 0;
			OoCopyDataROADLinkOAD(pbDst, &wRetSrcLen, pbSrc+wItemOffset, &wRetDstLen, pFmt+2, wFmtLen-2);
			pbDst += wRetSrcLen;
			pbSrc += wRetDstLen;
		}
		else if ((dwMainOAD&0xffffff00)==0x60120300 && (*pFmt==DT_ROAD) && (dwOAD>=0x30000000 && dwOAD<0x33000000))	//事件
		{
			if (dwMainOAD == 0x60120300)
			{
				wRetDstLen = 0;
				wRetSrcLen = 0;
				OoCopyDataROADLinkOAD(pbDst, &wRetSrcLen, pbSrc+7, &wRetDstLen, pFmt+1, wFmtLen); //偏移数据长度字节+发生次数字节 2+5
				pbDst += wRetSrcLen;
				pbSrc += wRetDstLen;
			}
			else
			{
				memcpy(pbDst, pbSrc0, wLen);
				pbDst += wLen;
			}
		}
		else if (*pFmt == DT_ROAD)
		{
			wRetDstLen = 0;
			wRetSrcLen = 0;
			OoCopyDataROADLinkOAD(pbDst, &wRetSrcLen, pbSrc+wItemOffset, &wRetDstLen, pFmt, wFmtLen);
			pbDst += wRetSrcLen;
			pbSrc += wRetDstLen;
		}
		else
		{
#if 1
			iRet = OoFormatSrcData(pbSrc0, wLen, pFmt, wFmtLen, pbDst);
			if (iRet > 0)
				pbDst += iRet;
			else
				*pbDst++ = 0x00;
#else
			int iRet;
			WORD wRetFmtLen;
			BYTE *pFmt0 = pFmt;
			BYTE bLkNum;
			
			while(pFmt-pFmt0 < wFmtLen)
			{
				BYTE bType = *pFmt++;
				if (bType == DT_RCSD)
				{
					if (*pFmt++ == 0)	//OAD
					{
						iRet = OoCopyOneOadData(pbSrc0, pFmt, wFmtLen-(pFmt-pFmt0), pbDst, &wRetFmtLen);
						if (iRet > 0)
						{
							pbDst += iRet;
							pbSrc0 += iRet;
						}
						else
						{
							*pbDst++ = 0x00;
							goto GOTO_RET;
						}
						pFmt += wRetFmtLen;
					}
					else	//ROAD
					{
						pFmt += 4;	//跳过主OAD
						bLkNum = *pFmt++;
						for (BYTE i=0; i<bLkNum; i++)
						{
							iRet = OoCopyOneOadData(pbSrc0, pFmt, wFmtLen-(pFmt-pFmt0), pbDst, &wRetFmtLen);
							if (iRet > 0)
							{
								pbDst += iRet;
								pbSrc0 += iRet;
							}
							else
							{
								*pbDst++ = 0x00;
								goto GOTO_RET;
							}
							pFmt += wRetFmtLen;
						}
					}
				}
				else if (bType == DT_OAD)
				{
					iRet = OoCopyOneOadData(pbSrc0, pFmt, wFmtLen-(pFmt-pFmt0), pbDst, &wRetFmtLen);
					if (iRet > 0)
					{
						pbDst += iRet;
						pbSrc0 += iRet;
					}
					else
					{
						*pbDst++ = 0x00;
						goto GOTO_RET;
					}
					pFmt += wRetFmtLen;
				}
				else if (bType == DT_CSD)
				{
					if (*pFmt++ == 0)	//choice 0:OAD
					{
						iRet = OoCopyOneOadData(pbSrc0, pFmt, wFmtLen-(pFmt-pFmt0), pbDst, &wRetFmtLen);
						if (iRet > 0)
						{
							pbDst += iRet;
							pbSrc0 += iRet;
						}
						else
						{
							*pbDst++ = 0x00;
							goto GOTO_RET;
						}
						pFmt += wRetFmtLen;
					}
					else	//ROAD
					{
						pFmt += 4;	//跳过主OAD
						bLkNum = *pFmt++;
						for (BYTE i=0; i<bLkNum; i++)
						{
							iRet = OoCopyOneOadData(pbSrc0, pFmt, wFmtLen-(pFmt-pFmt0), pbDst, &wRetFmtLen);
							if (iRet > 0)
							{
								pbDst += iRet;
								pbSrc0 += iRet;
							}
							else
							{
								*pbDst++ = 0x00;
								goto GOTO_RET;
							}
							pFmt += wRetFmtLen;
						}
					}
				}
				else if (bType == DT_ROAD)
				{
					pFmt += 4;	//跳过主OAD
					bLkNum = *pFmt++;
					for (BYTE i=0; i<bLkNum; i++)
					{
						iRet = OoCopyOneOadData(pbSrc0, pFmt, wFmtLen-(pFmt-pFmt0), pbDst, &wRetFmtLen);
						if (iRet > 0)
						{
							pbDst += iRet;
							pbSrc0 += iRet;
						}
						else
						{
							*pbDst++ = 0x00;
							goto GOTO_RET;
						}
						pFmt += wRetFmtLen;
					}
				}
				else 
				{
					memcpy(pbDst, pbSrc0, wLen);
					pbDst += wLen;
					break;
				}
			}
#endif
		}
	}
GOTO_RET:	
	wLen = pbDst - pbDst0;
	pbDst = pbDst0;

	return wLen;
}

//描述: 根据搜索条件及访问字段找出相应的任务库表
//参数：@ pbOAD	对象属性描述表
//	   @ pbRSD 对应协议中的RSD
//	   @ pbRCSD第一次调用时传入协议发过来的整个RCSD，
//				如果无法匹配则传递单个RCSD进来
//	   @ pszTableName 用来返回任务库表名，必须传入缓冲区指针
//	   @ pFixFields	如果找到相应的任务库表，则初始化该表的固定字段
//	   @ pDataFields	如果找到相应的任务库表，则初始化该表的数据字段
//返回: 如果正确则返回RCSD中匹配到的个数,否则返回0
int SearchTable(BYTE* pbOAD, BYTE* pbRSD, BYTE* pbRCSD, WORD wRcsdIdx, int *piTabIdx, char* pszTableName, TFieldParser* pFixFields, TFieldParser* pDataFields)
{
	TTaskCfg tTaskCfg;
	TFieldParser tFixFields;
	TFieldParser tDataFields;
	int iSchCfgLen;
	DWORD dwOAD = OoOadToDWord(pbOAD);
	WORD wIdx, wFmtLen,  wMchNum, wMaxMchNum;
	BYTE *pbFmt;
	BYTE bArryIdx, bType;
	BYTE *pbFixFmt, *pbDataFmt, *pbSch;
	WORD wFixFmtLen, wDataFmtLen;
	DWORD dwROAD;
	bool fSchTab = false;

	wMchNum = wMaxMchNum = 0;

	if ((dwOAD&0xffffff00)==0x60120300 || dwOAD==0x60120400)	//普通、事件采集方案、时实监控采集方案
	{
		for (wIdx=*piTabIdx; wIdx<TASK_NUM; wIdx++)
		{
			memset((BYTE*)&tTaskCfg, 0, sizeof(tTaskCfg));
			if (!GetTaskCfg(wIdx, &tTaskCfg))
				continue;

			//固定字段配置
			memset((BYTE*)&tFixFields, 0, sizeof(TFieldParser));
			tFixFields.pbCfg = (BYTE*)g_TSchFieldCfg[tTaskCfg.bSchType-1].tTFixFieldCfg.pbCfg;
			tFixFields.wCfgLen = g_TSchFieldCfg[tTaskCfg.bSchType-1].tTFixFieldCfg.wCfgLen;

			//初始化固定字段
			pbFixFmt = (BYTE*)g_TSchFieldCfg[tTaskCfg.bSchType-1].tTFixFieldCfg.pbFmt;
			wFixFmtLen = g_TSchFieldCfg[tTaskCfg.bSchType-1].tTFixFieldCfg.wFmtLen;
			if (!OoParseField(&tFixFields, pbFixFmt, wFixFmtLen, true))
				return 0;

			//数据字段配置
			pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
			if (pbSch == NULL)
				continue;

			pbFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
			if (pbFmt == NULL)
				continue;
			if (tTaskCfg.bSchType==SCH_TYPE_COMM)
				bArryIdx = 3;
			else if (tTaskCfg.bSchType==SCH_TYPE_EVENT)
				bArryIdx = 1;
			else
				bArryIdx = 1;

			if (tTaskCfg.bSchType == SCH_TYPE_EVENT)
			{
				int iCfgLen, iLen;
				WORD wIdx, wNum;
				BYTE *pbArryROAD;
				bool fIsSchField = false;
				
				memset((BYTE*)&tDataFields, 0, sizeof(TFieldParser));
				pbArryROAD = OoGetField(pbSch, pbFmt, wFmtLen, bArryIdx, &tDataFields.wCfgLen, &bType);

				pbArryROAD += 4; //跳过采集类型
				if (*pbArryROAD++ == DT_ARRAY)
				{
					wNum = *pbArryROAD++;
					for (wIdx=0; wIdx<wNum; wIdx++)
					{
						iCfgLen = OoGetDataTypeLen(pbArryROAD);
						iLen = OoGetDataLen(DT_ROAD, pbArryROAD+1);
						//数组或结构的具体数据内容
						tDataFields.pbCfg = pbArryROAD;
						tDataFields.wCfgLen = iCfgLen;
						//对配置字段解析的结果
						tDataFields.wNum = 1;
						tDataFields.wPos[0] = 0;
						tDataFields.wLen[0] = iLen;
						tDataFields.bType[0] = DT_ROAD;
						//配置的字段进一步解析成对应数据项的信息
						tDataFields.wTotalLen = iLen;
						tDataFields.wItemOffset[0] = 0;
						tDataFields.wItemLen[0] = iLen;

						//RCSD中个数遍历
						wMchNum = TableMatch(pbRCSD, wRcsdIdx, tFixFields.pbCfg, NULL, NULL, tDataFields.pbCfg, NULL, NULL);
						if (wMchNum > wMaxMchNum)
						{
							fSchTab = true;
							wMaxMchNum = wMchNum;

							memcpy((BYTE*)pFixFields, (BYTE*)&tFixFields, sizeof(TFieldParser));
							memcpy((BYTE*)pDataFields, (BYTE*)&tDataFields, sizeof(TFieldParser));

							memset(pszTableName, 0, sizeof(pszTableName));
							sprintf(pszTableName, "%s_%03d_%02d.dat", GetSchTableName(tTaskCfg.bSchType), tTaskCfg.bSchNo, wIdx);
						}
						pbArryROAD++;	
						pbArryROAD += ScanROAD(pbArryROAD, false);
					}
				}
			}
			else if (tTaskCfg.bSchType != SCH_TYPE_REPORT)
			{

				memset((BYTE*)&tDataFields, 0, sizeof(TFieldParser));
				tDataFields.pbCfg = OoGetField(pbSch, pbFmt, wFmtLen, bArryIdx, &tDataFields.wCfgLen, &bType, &pbDataFmt, &wDataFmtLen);
				if (tDataFields.pbCfg == NULL)
					continue;

				//RCSD中个数遍历
				wMchNum = TableMatch(pbRCSD, wRcsdIdx, tFixFields.pbCfg, NULL, NULL, tDataFields.pbCfg, NULL, NULL);
				if (wMchNum > wMaxMchNum)
				{
					wMaxMchNum = wMchNum;
					fSchTab = true;
					*piTabIdx = wIdx;

					//初始化数据字段
					if (!OoParseField(&tDataFields, pbDataFmt, wDataFmtLen, true))
						return 0;

					memcpy((BYTE*)pFixFields, (BYTE*)&tFixFields, sizeof(TFieldParser));
					memcpy((BYTE*)pDataFields, (BYTE*)&tDataFields, sizeof(TFieldParser));

					memset(pszTableName, 0, sizeof(pszTableName));
					sprintf(pszTableName, "%s_%03d.dat", GetSchTableName(tTaskCfg.bSchType), tTaskCfg.bSchNo);
				}
			}
		}

		if (!fSchTab)
			*piTabIdx = 0;
	}	
	else if (dwOAD == 0x601A0200)	//透明采集方案
	{

	}
	else if (dwOAD>=0x50000200 && dwOAD<=0x50110200)	//冻结, 每次根据RCSD偏移值wRcsdIdx 匹配一张表
	{
		BYTE bMethod = *pbRSD;  
		if(bMethod==1)
		{
			if(*(pbRSD+5)==0)
			{
				return -2;
			}
			else if(*(pbRSD+5)==1)
			{
				for(int i=0; i< *(pbRSD+6);i++)
				{
				  BYTE bDt = *(pbRSD+6+1+i*2);				 
				  if(bDt==DT_BOOL)
				  {
					BYTE bVal = *(pbRSD+6+1+i*2+1);
					if (bVal>1)
					{
						return -2;
					}
				  }
				}
			}			
		}
        else if(bMethod==2)
        {
            bool IsDateTimeType =false;
			BYTE *pbRSD2 = pbRSD;
			pbRSD2++; // 跳过方法
			pbRSD2+=4; // 跳过OAD
			if (*pbRSD2==DT_DATE_TIME_S)
            {
                pbRSD2+=8;
                IsDateTimeType = true;
            }
            else if(*pbRSD2==DT_DATE_TIME)
            {
                pbRSD2+=11;
                IsDateTimeType = true;
            }

            if(IsDateTimeType)
            {
                if (*pbRSD2==DT_DATE_TIME_S)
                {
                    pbRSD2+=8;
                }
                else if(*pbRSD2==DT_DATE_TIME)
                {
                    pbRSD2+=11;
                }
    			
    			if (*pbRSD2!=DT_TI && *pbRSD2!=DT_NULL)
    			{
    				return -2;
    			} 
            }            
        }
		
		dwROAD = OoOadToDWord(pbRCSD+2);	//根据RCSD偏移值wRcsdIdx确定表名
		if (!GetFrzTaskFileName((WORD )(dwOAD>>16), dwROAD, pszTableName))
			return -1;

		if (pszTableName == NULL)
			return -1;

		if (GetFrzTaskRecFieldParser(dwROAD, pFixFields, pDataFields, pDataFields->pbCfg, pDataFields->wCfgLen) == false)
			return -1;

		//wMaxMchNum = *pbRCSD;
		wMaxMchNum = 1;	//1次只匹配1张表
	}
	else if (dwOAD>=0x30000000 && dwOAD<=0x32030000)	//事件
	{
		if(GetTermEvtCtrl((WORD )(dwOAD>>16)) != NULL)	//TermEvtTask.h/cpp所支持的所有事件，即不包括抄表事件
		{
			char *p = GetEvtRecFileName(dwOAD);
			if (p == NULL)
				return -1;
			strcpy(pszTableName, p);
			if (pszTableName == NULL)
				return -1;			
			if (GetEvtRecFieldParser(dwOAD, pFixFields, pDataFields, pDataFields->pbCfg, pDataFields->wCfgLen) == false)
				return -1;
			wMaxMchNum = *pbRCSD;
			if (wMaxMchNum == 0)//当无一个OAD时，RCSD=0，即SEQUENCE OF的数据项个数为0，表示“不选择（即全选）”。
			{
				int i, index;
				WORD wLen;
				BYTE *pbtr = pbRCSD;
				wMaxMchNum = pFixFields->wNum + pDataFields->wNum; //事件必有固定字段
				*pbtr = wMaxMchNum;
				pbtr++;
				for (i = 0 ; i < pFixFields->wNum; i++)
				{
					index = pFixFields->wPos[i];
					wLen = pFixFields->wLen[i];
					memcpy(pbtr, &(pFixFields->pbCfg[index]), wLen);
					if (*pbtr == 0x51) //OAD
						*pbtr = 0;
					//else if (*pbtr == 0x52) //ROAD
						//*pbtr = 1;
					pbtr += wLen;
				}
				for (i = 0; i < pDataFields->wNum; i++)
				{
					index = pDataFields->wPos[i];
					wLen = pDataFields->wLen[i];
					memcpy(pbtr, &(pDataFields->pbCfg[index]), wLen);
					if (*pbtr == 0x51) //OAD
						*pbtr = 0;
					//else (*pbtr == 0x52) //ROAD
						//*pbtr = 1;
					pbtr += wLen;
				}
			}
		}
#ifdef TERM_EVENT_DEBUG
		else
		{
			char *p = GetEvtRecFileName(dwOAD);
			if (p != NULL)
				strcpy(pszTableName, p);
			else
				return -1;	

			if (GetMtrExcFieldParser((dwOAD>>16), pFixFields, pDataFields, pDataFields->pbCfg, pDataFields->wCfgLen) == false)
				return -1;
			wMaxMchNum = *pbRCSD;
		}
#endif
	}

	return wMaxMchNum;
}

//返回匹配的RCSD个数
int TableMatch(BYTE* pbRCSD, WORD wRcsdIdx, BYTE* pbFixCfg, BYTE* pbFixFmt, WORD wFixFmtLen, BYTE* pbDataCfg, BYTE* pbDataFmt, WORD wDataFmtLen)
{
	int iRcsdLen;
	WORD wMchNum;
	
	wMchNum = 0;
	iRcsdLen = ScanRCSD(pbRCSD, false);
	while (iRcsdLen>=0 && wRcsdIdx<iRcsdLen)
	{
		if (!RcsdMatch(pbRCSD, wRcsdIdx, pbFixCfg, pbFixFmt, wFixFmtLen)) //固定字段未匹配，到数据字段中匹配
		{
			if (!RcsdMatch(pbRCSD, wRcsdIdx, pbDataCfg, pbDataFmt, wDataFmtLen)) //数据字段未匹配,直接结束当前遍历
				break;
		}
		wMchNum++;
		wRcsdIdx += ScanCSD(pbRCSD+wRcsdIdx, false); 
	}

	return wMchNum;
}

//描述:匹配RCSD，每次只比较RCSD中的一个CSD
//参数:@pbRCSD
//	   @wRcsdIdex 
//返回:如果正确则返回1,否则返回0
int RcsdMatch(BYTE* pbRCSD, WORD wRcsdIdx, BYTE* pbCfg, BYTE* pbFmt, WORD wFmtLen)
{
	if (FieldCmp(DT_CSD, pbRCSD+wRcsdIdx, pbCfg[0], pbCfg+1) >= 0)	//+1:去除array格式
		return 1;
	return 0;
}

typedef struct{
	WORD  wItemOffset;	//数据项在记录中的偏移，
	WORD  wItemLen;	//数据项在记录中的长度，
	BYTE* pbFmt;  		//格式描述串
	WORD wFmtLen; 		//格式描述串长度
}TFieldInfo;



//描述：子段顺序比较（只对CSD类型比较，其它暂时不做处理）
//参数：@pbSrc 源字段数据
//		@pbCmp 比较字段数据
//返回：-1出错，0字段顺序相同，1字段顺序不一致
int FieldOrderCmpare(BYTE* pbSrc, BYTE* pbCmp)
{
	BYTE bSrcNum, bCmpNum;
	if (*pbSrc++ == *pbCmp++)
	{
		if (*pbSrc == 0)	//OAD
		{
			if (memcmp(pbSrc, pbCmp, 5) == 0)
				return 0;
		}
		else	//ROAD
		{
			if (memcmp(pbSrc, pbCmp, 4) == 0)
			{
				pbSrc += 4;
				pbCmp += 4;
				bSrcNum = *pbSrc++;
				bCmpNum = *pbCmp++;
				if (bSrcNum == bCmpNum)
				{
					if (memcmp(pbSrc, pbCmp, bSrcNum*4) == 0)
						return 0;
					else
						return 1;
				}
			}
		}
	}

	return -1;
}


//描述:从一笔记录中读取一个字段
//参数：@ pbRec	记录缓冲区
//	   @ wOffset	pFieldParser所标识的字段相对应pbRec开始的偏移
//	   @ pFieldParser	分别传入固定字段或数据字段
//	   @ pbCSD协议中RCSD的单个CSD，不带CSD格式字节[Choice=0--OAD, Choice=1---OAD--LkOAD]
//	   @ pbBuf用来返回记录的内容
//返回:如果正确则返回数据的长度,否则返回负数
int ReadRecField(BYTE* pbRec, WORD wOffset, TFieldParser* pParser, BYTE* pbOAD, BYTE* pbCSD, BYTE* pbBuf)
{
	int iRet, iNum, iLen;
	WORD wItemOffset, wItemLen;
	BYTE bType, bSrcField[128];
	BYTE *pbBuf0 = pbBuf;
	TFieldInfo tInfo;

	//字段中提取数据
	for (WORD wIndex=0; wIndex<pParser->wNum; wIndex++)
	{
		memset(bSrcField, 0, sizeof(bSrcField));
		if (ReadParserField(pParser, wIndex, bSrcField, &bType, &wItemOffset, &wItemLen) > 0)
		{	
			memset((BYTE*)&tInfo, 0, sizeof(tInfo));
			tInfo.wItemOffset = pParser->wItemOffset[wIndex];
			tInfo.wItemLen = pParser->wItemLen[wIndex];
			tInfo.pbFmt = pParser->pbCfg + pParser->wPos[wIndex];
			tInfo.wFmtLen = pParser->wLen[wIndex];

			iRet = FieldCmp(DT_CSD, pbCSD, bType, bSrcField+1);	//+1:去掉格式
			if (iRet == 0)
			{
				if ((iNum=OoScanRcsdInOadNum(pbCSD)) > 0)	//协议层请求的ROAD数据，在返回时需带上ROAD中关联OAD个数！
				{
					*pbBuf++ = 0x01;
					*pbBuf++ = iNum;
				}
				//到该层if语句表明源字段与抄读字段CSD相同，现在要做的是比较它们CSD内部的关联OAD顺序是否一致
				if (FieldOrderCmpare(pbCSD, bSrcField+1) > 0)	//源字段与比较字段顺序不一致
				{
					BYTE bTmpBuf[1024];
					BYTE bTmpFmt[512];
					BYTE *pbTmpFmt;

					memset(bTmpBuf, 0, sizeof(bTmpBuf));

					ReadFromROAD_1(pbCSD, bSrcField, bTmpBuf, pbRec+(wOffset+tInfo.wItemOffset));	////wOffset+tInfo.wItemOffset 固定字段偏移 + N个CSD数据字段偏移
					
					memset(bTmpFmt, 0, sizeof(bTmpFmt));
					if (tInfo.wFmtLen > sizeof(bTmpFmt)-1)
					{
						DTRACE(DB_CRITICAL, ("ReadRecField(): tInfo.wFmtLen > sizeof(bTmpFmt) error!!!\r\n"));
						return -1;
					}
					pbTmpFmt = bTmpFmt;
					*pbTmpFmt++ = DT_CSD;
					memcpy(pbTmpFmt, pbCSD, tInfo.wFmtLen-1);
					if ((iRet=OoCopyData(pbOAD, pbBuf, bTmpBuf, 0, tInfo.wItemLen, pbTmpFmt, tInfo.wFmtLen)) > 0)
					{
						pbBuf += iRet;
						iLen = pbBuf - pbBuf0;
						pbBuf = pbBuf0;

						return iLen;
					}
				}
				else	//源字段与比较字段顺序一致
				{
					if ((iRet=OoCopyData(pbOAD, pbBuf, pbRec+wOffset, tInfo.wItemOffset, tInfo.wItemLen, tInfo.pbFmt, tInfo.wFmtLen)) > 0)
					{
						pbBuf += iRet;
						iLen = pbBuf - pbBuf0;
						pbBuf = pbBuf0;

						return iLen;
					}
				}
			}
			else if (iRet == 1)	//ROAD，提取部分关联OAD
			{
				if ((iNum=OoScanRcsdInOadNum(pbCSD)) > 0)	//协议层请求的ROAD数据，在返回时需带上ROAD中关联OAD个数！
				{
					*pbBuf++ = 0x01;
					*pbBuf++ = iNum;
				}

				if ((iRet=ReadFromROAD(pbCSD, bSrcField, pbBuf, pbRec+(wOffset+tInfo.wItemOffset))) > 0)	//wOffset+tInfo.wItemOffset 固定字段偏移 + N个CSD数据字段偏移
				{
					pbBuf += iRet;	
					iLen = pbBuf - pbBuf0;
					pbBuf = pbBuf0;

					return iLen;
				}
			}
		}
	}

	return -1;
}

DWORD GetEvtTaskTableFixFieldLen()
{
	TFieldParser tFixFields;
	BYTE *pbFixFmt;
	WORD wFixFmtLen;

	//固定字段配置
	memset((BYTE*)&tFixFields, 0, sizeof(TFieldParser));
	tFixFields.pbCfg = (BYTE*)g_TSchFieldCfg[SCH_TYPE_EVENT-1].tTFixFieldCfg.pbCfg;
	tFixFields.wCfgLen = g_TSchFieldCfg[SCH_TYPE_EVENT-1].tTFixFieldCfg.wCfgLen;

	//初始化固定字段
	pbFixFmt = (BYTE*)g_TSchFieldCfg[SCH_TYPE_EVENT-1].tTFixFieldCfg.pbFmt;
	wFixFmtLen = g_TSchFieldCfg[SCH_TYPE_EVENT-1].tTFixFieldCfg.wFmtLen;
	if (!OoParseField(&tFixFields, pbFixFmt, wFixFmtLen, true))
		return 0;

	return tFixFields.wTotalLen;
}

//描述: 读取全事件采集任务最近一笔记录的记录序号
//参数：@ bSchNo	事件采集方案号
//	   @ bCSDIndex array ROAD索引位置
//返回: 如果正确则返回次数,否则返回0
DWORD GetEvtTaskRecLastSerialNumber(BYTE bSchNo, BYTE bCSDIndex)
{
	char szTableName[32];
	BYTE bRecBuf[MTR_EXC_REC_LEN];
	DWORD dwLastRecIndex=0;
	memset(szTableName, 0, sizeof(szTableName));
	GetEvtTaskTableName(bSchNo, bCSDIndex, szTableName);

	memset(bRecBuf, 0, sizeof(bRecBuf));
	int iLen = ReadLastNRec(szTableName, LAST_REC, bRecBuf, sizeof(bRecBuf));	//读出上一笔记录取最新事件记录序号
	DWORD dwOff = GetEvtTaskTableFixFieldLen();
	if (iLen>0 && dwOff>0)
		dwLastRecIndex = OoDoubleLongUnsignedToDWord(bRecBuf+dwOff+3);	//事件记录序号 高字节在前传输

	return dwLastRecIndex;
}

//描述: 读取全事件采集任务最近一笔记录的记录序号
//参数：@ wPn	电表测量点号
//	   @ dwOAD  事件OAD
//返回: 如果正确则返回次数,否则返回0
DWORD GetEvtTaskRecLastSerialNumber(BYTE* pbTsa, BYTE bTsaLen, BYTE* pbCSD, BYTE bLenCSD)
{
	int iStart = -1;
	int iTabIdx = 0;
	WORD wRetNum = 0;
	BYTE bRSD[30] = {0}; //记录选择描述符
	BYTE bRCSD[128];  //记录列选择描述符                               
	BYTE *ptr = NULL;
	BYTE bOAD[4] = {0x60, 0x12, 0x03, 0x01};  //任务配置表 记录单元
	int nRet;
	BYTE bRecBuf[MTR_EXC_REC_LEN];
	DWORD dwLastRecIndex = 0;

	ptr = bRSD;
	*ptr++ = 10;       //—— RSD，选择方法10
	*ptr++ = 1;        //—— 上一条记录
	*ptr++ = 3;        //—— 电能表集合MS  一组用户地址 [3]
	*ptr++ = 1;        //电表地址个数=1
	*ptr++ = bTsaLen + 1;
	*ptr++ = bTsaLen - 1;
	memcpy(ptr, pbTsa+1, bTsaLen);   //电表地址
	ptr += bTsaLen;

	memset(bRCSD, 0, sizeof(bRCSD));
	ptr = bRCSD;
	*ptr++ = 1;
	*ptr++ = 1;
	memcpy(ptr, pbCSD, bLenCSD);

	memset(bRecBuf, 0, sizeof(bRecBuf));
	nRet = ReadRecord(bOAD, bRSD, bRCSD, &iTabIdx, &iStart, bRecBuf, sizeof(bRecBuf), &wRetNum);
	if (nRet > 0)
	{
		dwLastRecIndex = OoDoubleLongUnsignedToDWord(bRecBuf+5);	//事件记录序号 高字节在前传输
	}

	return dwLastRecIndex;
}

bool RecMatch (BYTE* pbRec, TFieldParser* pFixFields, BYTE* pbRSD)
{
	DWORD dwOAD;
	WORD wIndex;
	WORD wOffset; 
	WORD wFieldLen;
	BYTE bSelect = *pbRSD++;
	BYTE *pbPtr;
	BYTE bNum;

	switch(bSelect)
	{
	case 0:
		return true;
	case 1:
		if (SearchField(pFixFields, pbRSD, &wIndex, &wOffset, &wFieldLen))
		{
			BYTE *pbFieldData = pbRec+wOffset;
			if (OadMatch(TDB_OP_EQ, pbRSD, pbRSD+4, pbFieldData, wFieldLen))
				return true;
		}
		break;
	case 2:	
		if (SearchField(pFixFields, pbRSD, &wIndex, &wOffset, &wFieldLen))
		{
			BYTE *pbFieldData = pbRec+wOffset;
			if (OadMatch(TDB_OP_GE, pbRSD, pbRSD+4, pbFieldData, wFieldLen)
				&& OadMatch(TDB_OP_LT, pbRSD, pbRSD+4+8, pbFieldData, wFieldLen))
			{
				if (pbRSD[4+8+8] == DT_TI)
				{
					if (TiMatch(pbRSD+4+1, pbRSD+4+8+1, pbRSD+4+8+8+1, pbFieldData+1))
						return true;
				}
				else
				{
					DTRACE(DB_TASK, ("RecMatch():The program has not supported the data-type of 'data-interval' in Selector2 which is not 'DT_TI' yet.\n"));
				}
			}
		}
		break;
	case 3:
		bNum = *pbRSD++;
		pbPtr = pbRSD;
		for (BYTE i = 0; i < bNum; i++)
		{
			if (SearchField(pFixFields, pbPtr, &wIndex, &wOffset, &wFieldLen))
			{
				BYTE *pbFieldData = pbRec+wOffset;
				if (OadMatch(TDB_OP_GE, pbPtr, pbPtr+4, pbFieldData, wFieldLen)
					&& OadMatch(TDB_OP_LT, pbPtr, pbPtr+4+7, pbFieldData, wFieldLen))
				{
					if (TiMatch(pbRSD, pbRSD+7, pbRSD+7+7, pbFieldData))					
						return true;
				}
				pbPtr += (4+7+7+1);	
			}
		}
		break;
	case 4:	//采集启动时间
		dwOAD = 0x00024060; //0x60400200
		if (SearchField(pFixFields, (BYTE*)&dwOAD, &wIndex, &wOffset, &wFieldLen))
		{
			BYTE *pbFieldData = pbRec+wOffset;
			if (DateTimeSMatch(TDB_OP_EQ, pbRSD, pbFieldData))
			{
				dwOAD = 0x00020140;	//0x40010200;	
				if (SearchField(pFixFields, (BYTE*)&dwOAD, &wIndex, &wOffset, &wFieldLen))
				{
					pbFieldData = pbRec+wOffset;
					if (MsMatch(pbRSD+7, pbFieldData))
						return true;
				}
			}
		}
		break;
	case 5:	//采集存储时间
		dwOAD = 0x00024260;	//0x60420200; 
		if (SearchField(pFixFields, (BYTE*)&dwOAD, &wIndex, &wOffset, &wFieldLen))
		{
			BYTE *pbFieldData = pbRec+wOffset;
			if (DateTimeSMatch(TDB_OP_EQ, pbRSD, pbFieldData))
			{
				dwOAD = 0x00020140;	//0x40010200;	
				if (SearchField(pFixFields, (BYTE*)&dwOAD, &wIndex, &wOffset, &wFieldLen))
				{
					pbFieldData = pbRec+wOffset;
					if (MsMatch(pbRSD+7, pbFieldData))
						return true;
				}
			}
		}
		break;
	case 6:	//采集启动时间起始、结束值
		dwOAD = 0x00024060;	//0x60400200; 
		if (SearchField(pFixFields, (BYTE*)&dwOAD, &wIndex, &wOffset, &wFieldLen))
		{
			BYTE *pbFieldData = pbRec+wOffset;
			if (TiMatch(pbRSD, pbRSD+7, pbRSD+7+7, pbFieldData))
			{
				dwOAD = 0x00020140;	//0x40010200;	
				if (SearchField(pFixFields, (BYTE*)&dwOAD, &wIndex, &wOffset, &wFieldLen))
				{
					pbFieldData = pbRec+wOffset;
					if (MsMatch(pbRSD+7+7+3, pbFieldData))
						return true;
				}
			}
		}
		break;
	case 7:	//采集存储时间起始、结束值
		dwOAD = 0x00024260;//0x60420200;
		if (SearchField(pFixFields, (BYTE*)&dwOAD, &wIndex, &wOffset, &wFieldLen))
		{
			BYTE *pbFieldData = pbRec+wOffset;
			if (TiMatch(pbRSD, pbRSD+7, pbRSD+7+7, pbFieldData))
			{
				dwOAD = 0x00020140;	//0x40010200;	
				if (SearchField(pFixFields, (BYTE*)&dwOAD, &wIndex, &wOffset, &wFieldLen))
				{
					pbFieldData = pbRec+wOffset;
					if (MsMatch(pbRSD+7+7+3, pbFieldData))
						return true;
				}
			}
		}
		break;
	case 8:	//采集成功时间起始、结束值
		dwOAD = 0x00024160;	//0x60410200;
		if (SearchField(pFixFields, (BYTE*)&dwOAD, &wIndex, &wOffset, &wFieldLen))
		{
			BYTE *pbFieldData = pbRec+wOffset;
			if (TiMatch(pbRSD, pbRSD+7, pbRSD+7+7, pbFieldData))
			{
				dwOAD = 0x00020140;	//0x40010200;	
				if (SearchField(pFixFields, (BYTE*)&dwOAD, &wIndex, &wOffset, &wFieldLen))
				{
					pbFieldData = pbRec+wOffset;
					if (MsMatch(pbRSD+7+7+3, pbFieldData))
						return true;
				}
			}
		}
		break;
	case 9:	//上第N次记录
		return true;
		//break;
	case 10:
		pbRSD++;
		dwOAD = 0x00020140;	//0x40010200;	
		if (SearchField(pFixFields, (BYTE*)&dwOAD, &wIndex, &wOffset, &wFieldLen))
		{
			BYTE *pbFieldData = pbRec+wOffset;
			if (MsMatch(pbRSD, pbFieldData))
				return true;
		}
		break;
	}

	return false;
}

//搜索RSD的字段是记录里面的哪个字段
//固定字段中配置的是OAD 4个字节，或这OAD对应的数据长度
bool SearchField (TFieldParser* pFixFields, BYTE* pbOAD, WORD* pwIndex, WORD* pwOffset, WORD* wFieldLen)
{
	BYTE bIndex;
	BYTE *pbCfg;
	WORD wPos;
	DWORD dwOAD;
	DWORD dwOAD1;

	dwOAD = OoOadToDWord(pbOAD);
	for (bIndex=0; bIndex<pFixFields->wNum; bIndex++)
	{
		if (pFixFields->bType[bIndex] == DT_OAD)	//该函数比较的是OAD，针对其他类型RCSD\CSD全部过滤掉
		{
			wPos = pFixFields->wPos[bIndex];
			pbCfg = pFixFields->pbCfg;
			dwOAD1 = OoOadToDWord(&pbCfg[wPos+1]);	//wPos + 1:固定字段在初始化时，偏移位置包含了数据格式，去掉
			if (dwOAD == dwOAD1)	
			{
				*pwIndex = bIndex;
				*pwOffset = pFixFields->wItemOffset[bIndex];
				*wFieldLen = pFixFields->wItemLen[bIndex];

				return true;
			}
		}
	}

	return false;
}

//RSD的字段匹配方法
bool OadMatch(BYTE bOp, BYTE* pbOAD, BYTE* pbData, BYTE* pbFieldData, WORD wFieldLen)
{
	WORD i;

	switch (bOp)
	{
	case TDB_OP_EQ:
		if (memcmp(pbData, pbFieldData, wFieldLen) == 0)
			return true;
		break;
	case TDB_OP_GE:
		if (memcmp(pbData, pbFieldData, wFieldLen) == 0)
			return true;
		for (i = 0; i<wFieldLen; i++)
		{
			//如时间，低字节为年，所以从低字节开始比较
			if (pbFieldData[i] > pbData[i])
				return true;
			else if (pbFieldData[i] < pbData[i])
				return false;
		}
		break;
	case TDB_OP_GT:
		if (memcmp(pbData, pbFieldData, wFieldLen) == 0)
			return false;
		for (i = 0; i<wFieldLen; i++)
		{
			if (pbFieldData[i] > pbData[i])
				return true;
			else if (pbFieldData[i] < pbData[i])
				return false;
		}
		break;
	case TDB_OP_LT:
		if (memcmp(pbData, pbFieldData, wFieldLen) == 0)
			return false;
		for (i = 0; i<wFieldLen; i++)
		{
			if (pbData[i] > pbFieldData[i])
				return true;
			else if (pbData[i] < pbFieldData[i])
				return false;
		}
		break;
	case TDB_OP_LE:
		if (memcmp(pbData, pbFieldData, wFieldLen) == 0)
			return true;
		for (i = 0; i<wFieldLen; i++)
		{
			if (pbData[i] > pbFieldData[i])
				return true;
			else if (pbData[i] < pbFieldData[i])
				return false;
		}
		break;
	}

	return false;
}

//描述：只比较YYMMDDhhmm
bool DateTimeSMatch(BYTE bOp, BYTE* pbDateTimeS, BYTE* pbFieldData)
{
	BYTE bDateTimeLen = 6;
	BYTE i;

	switch (bOp)
	{
	case TDB_OP_EQ:
		if (memcmp(pbDateTimeS, pbFieldData, bDateTimeLen) == 0)
			return true;
		break;
	case TDB_OP_GE:
		if (memcmp(pbDateTimeS, pbFieldData, bDateTimeLen) == 0)
			return true;
		for (i = 0; i<bDateTimeLen; i++)
		{
			if (pbDateTimeS[i] < pbFieldData[i])
				return true;
			else if(pbDateTimeS[i] > pbFieldData[i])
				return false;
		}
		break;
	case TDB_OP_GT:
		if (memcmp(pbDateTimeS, pbFieldData, bDateTimeLen) == 0)
			return false;
		for (i = 0; i<bDateTimeLen; i++)
		{
			if (pbDateTimeS[i] < pbFieldData[i])
				return true;
			else if(pbDateTimeS[i] > pbFieldData[i])
				return false;
		}
		break;
	case TDB_OP_LT:
		if (memcmp(pbDateTimeS, pbFieldData, bDateTimeLen) == 0)
			return false;
		for (i = 0; i<bDateTimeLen; i++)
		{
			if (pbDateTimeS[i] > pbFieldData[i])
				return true;
			else if(pbDateTimeS[i] < pbFieldData[i])
				return false;
		}
		break;
	case TDB_OP_LE:
		if (memcmp(pbDateTimeS, pbFieldData, bDateTimeLen) == 0)
			return true;
		for (i = 0; i < bDateTimeLen; i++)
		{
			if (pbDateTimeS[i] > pbFieldData[i])
				return true;
			else if(pbDateTimeS[i] < pbFieldData[i])
				return false;
		}
		break;
	}

	return false;
}
bool MsMatch(BYTE* pbMS, BYTE* pbFieldData)
{
	BYTE bMtrMask[PN_MASK_SIZE];
	WORD wPn;

	memset(bMtrMask, 0, sizeof(bMtrMask));
	if (ParserMsParam(pbMS, bMtrMask, sizeof(bMtrMask)) > 0)
	{
		if (((wPn=GetMeterPn(pbFieldData+1, pbFieldData[0]))>0) && //+1：pbFieldData是带表地址长度
			(bMtrMask[wPn/8]&(1<<(wPn%8))))	
			return true;
	}

	return false;
}

bool TiMatch(BYTE* pbStartTime, BYTE* pbEndTime, BYTE* pbTI, BYTE* pbFieldData)
{
	TTime tStartTime, tEndTime, tFieldTime;
	DWORD dwStartSec=0, dwEndSec=0, dwFieldSec;
	DWORD  dwIntervSec;
	WORD wIntervV;
	BYTE bIntervU;
	BYTE *pbPtr = pbTI;
	bool fStartTimeAll0xEE = false;
	bool fEndTimeAll0xEE = false;

	memset((BYTE*)&tStartTime, 0, sizeof(tStartTime));
	if (IsAllAByte(pbStartTime, 0xff, 7))	//全0xff表示时间无效
	{
		fStartTimeAll0xEE = true;
	}
	else
	{
		//DateTimeToTime(pbStartTime, tStartTime);
		OoDateTimeSToTime(pbStartTime, &tStartTime);
		tStartTime.nSecond = 0;
		dwStartSec = TimeToSeconds(tStartTime);
	}

	memset((BYTE*)&tEndTime, 0, sizeof(tEndTime));
	if (IsAllAByte(pbEndTime, 0xff, 7))	//全0xff表示时间无效
	{
		fEndTimeAll0xEE = true;
	}
	else
	{
		//DateTimeToTime(pbEndTime, tEndTime);
		OoDateTimeSToTime(pbEndTime, &tEndTime);
		tEndTime.nSecond = 0;
		dwEndSec = TimeToSeconds(tEndTime);
	}

	if (fStartTimeAll0xEE || fEndTimeAll0xEE)	//时间无效直接返回
		return true;

	memset((BYTE*)&tFieldTime, 0, sizeof(tFieldTime));
	//DateTimeToTime(pbFieldData, tFieldTime);
	OoDateTimeSToTime(pbFieldData, &tFieldTime);
	tFieldTime.nSecond = 0;
	dwFieldSec = TimeToSeconds(tFieldTime);	

	bIntervU = *pbPtr++;
	wIntervV = OoOiToWord(pbPtr);
	if (wIntervV == 0xffff)
	{
		if (dwFieldSec>=dwStartSec && dwFieldSec<dwEndSec)
			return true;
		else
			return false;
	}

	if (bIntervU < TIME_UNIT_DAY)	//读曲线数据的起始时间归整到下一间隔
	{
		//if (wIntervV == 0)	
		//	wIntervV = 1;

		if (wIntervV != 0)	//间隔值为0表示无间隔 add CL 20170712
		{
			//DTRACE(DB_FAFRM, ("TiMatch() wIntervV=%d.\n", wIntervV));

			dwIntervSec = 1;
			if (bIntervU == TIME_UNIT_MINUTE)
				dwIntervSec *= 60;
			else if (bIntervU == TIME_UNIT_HOUR)
				dwIntervSec *= 3600;

			dwIntervSec *= wIntervV;

			if ((dwStartSec%dwIntervSec) != 0)
			{
				dwStartSec = dwStartSec / dwIntervSec * dwIntervSec;
				dwStartSec += dwIntervSec;		//归整到当前间隔的下一个间隔
				//DTRACE(DB_FAFRM, ("TiMatch() start time to next interv, dwIntervSec=%ld.\n", dwIntervSec));
			}
			else
			{
				dwStartSec = dwStartSec / dwIntervSec * dwIntervSec;
				//DTRACE(DB_FAFRM, ("TiMatch() start time to current interv, dwIntervSec=%ld.\n", dwIntervSec));
			}

			SecondsToTime(dwStartSec, &tStartTime);
			//DTRACE(DB_FAFRM, ("TiMatch() tmStart.Hour=%d, tmStart.Minute=%d, tmStart.Second=%d.\n", tStartTime.nHour, tStartTime.nMinute, tStartTime.nSecond));

			if (dwFieldSec>=dwStartSec && dwFieldSec<dwEndSec)
			{
				dwFieldSec = dwFieldSec / dwIntervSec * dwIntervSec;	//归整到当前间隔
				SecondsToTime(dwFieldSec, &tFieldTime);
			}
		}
		else	//wIntervV==0
		{
			if (dwStartSec==dwEndSec && dwStartSec==dwFieldSec)		//起止查询时间相同，浙江主站要求能读回当前时间点的数据
			{
				DTRACE(DB_FAFRM, ("TiMatch() start time equal to end time, dwStartSec=%ld, bIntervU=%d.\n", dwStartSec, bIntervU));
				return true;
			}			
		}
	}

	while (dwStartSec < dwEndSec)
	{
		if (wIntervV == 0)	//时间为0,取间隔周期内的数据
		{
			if (dwFieldSec>=dwStartSec && dwFieldSec<dwEndSec)
				return true;
			else
				break;
		}
		if (dwStartSec == dwFieldSec)
			return true;

		if (!AddIntervs(tStartTime, bIntervU, wIntervV))
			return false;

		dwStartSec = TimeToSeconds(tStartTime);
	}

	return false;
}

//MS计算电表个数
int MsToMtrNum(BYTE *pbMs)
{
	const BYTE bBitCnt[] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4};
	WORD wNum = 0;
	BYTE bMtrMask[PN_MASK_SIZE];

	if (pbMs != NULL)
	{
		memset(bMtrMask, 0, sizeof(bMtrMask));
		ParserMsParam(pbMs, bMtrMask, PN_MASK_SIZE);

		for (WORD i=0; i<PN_MASK_SIZE; i++)
		{
			if (bMtrMask[i] != 0)
			{
				wNum += bBitCnt[bMtrMask[i]&0x0f];
				wNum += bBitCnt[(bMtrMask[i]>>4)&0x0f];
			}
		}

		return wNum;
	}

	return 0;
}




//当日冻结上报时间与电表时间相差1天时，把记录中的数据改为无效数据填充，浙江测试需求
int FillInValueData(BYTE *pSrc, BYTE *pRcsd)
{
	TTime tNowTime, tRdTime;
	DWORD dwFrzOAD;
	BYTE bRcsdNum;
	BYTE bTypeLen;
	BYTE *pSechFrzOad;
	bool fExistFrzOad = false;
	BYTE *pbDataEnd = NULL, *pbDataStart=NULL;   //pSrc数据区


	
	pbDataStart = pbDataEnd = pSrc;

	pSechFrzOad = pRcsd;
	bRcsdNum = *pSechFrzOad++;

			
	for (BYTE bIdx=0; bIdx<bRcsdNum; bIdx++)
	{
		if (*pSechFrzOad++ == 0)	//choice 0 OAD
		{
			dwFrzOAD = OoOadToDWord(pSechFrzOad);
			if (dwFrzOAD==0x202A0200)
			{
				bTypeLen = OoGetDataTypeLen(pSrc);
				pSrc += bTypeLen;
				pbDataEnd +=bTypeLen; 
				pSechFrzOad += 4;
			}
			else
			{
				bTypeLen = OoGetDataTypeLen(pSrc);
				*pSrc = 0;  //填充0 
				pbDataEnd++;
				pSrc += bTypeLen;
				pSechFrzOad += 4;
			}
		}
		else	//choice 1 ROAD
		{
			DWORD bMainOAD;
			BYTE bLnkNum;

			bMainOAD = OoOadToDWord(pSechFrzOad);
			pSechFrzOad += 4;	//主OAD
			bLnkNum = *pSechFrzOad++;
			for (BYTE i=0; i<bLnkNum; i++)
			{
				if (bMainOAD == 0x50040200)
				{
				
				}
				bTypeLen = OoGetDataTypeLen(pSrc);
				if (i==0)
				{
					*pbDataEnd = 0;
					pbDataEnd++;
				}
				pSrc += bTypeLen;
				pSechFrzOad += 4;
			}
		}
	}

	return (int)(pbDataEnd - pbDataStart);
	
}



//描述：判断数据的冻结时标是否正确，这里不能用存储时标0x60420200判断，
//		这个时间由普通采集方案的存储时标决定
//参数：@pSrc 源数据
//		@pRcsd 源数据pSrc对应的RCSD
//返回：大于0表示数据冻结时间合法，等于0表示时间不合法，小于0表示pRcsd内部不存在冻结时标的相应OAD
int DayFrzTimeMatch(BYTE *pSrc, BYTE *pRcsd)
{
	TTime tNowTime, tRdTime;
	DWORD dwFrzOAD;
	BYTE bRcsdNum;
	BYTE bTypeLen;
	BYTE *pSechFrzOad;
	bool fExistFrzOad = false;
	

	pSechFrzOad = pRcsd;
	bRcsdNum = *pSechFrzOad++;
	for (BYTE bIdx=0; bIdx<bRcsdNum; bIdx++)
	{
		if (*pSechFrzOad++ == 0)	//choice 0 OAD
		{
			dwFrzOAD = OoOadToDWord(pSechFrzOad);
			if (dwFrzOAD==0x20210200 || dwFrzOAD==0x60400200 || dwFrzOAD==0x60410200 || dwFrzOAD==0x60420200)
			{
				fExistFrzOad = true;
				GetCurTime(&tNowTime);
				OoDateTimeSToTime(pSrc+1, &tRdTime);
				if (tRdTime.nYear==tNowTime.nYear && tRdTime.nMonth==tNowTime.nMonth && tRdTime.nDay==tNowTime.nDay)
					return 1;
			}
			bTypeLen = OoGetDataTypeLen(pSrc);
			pSrc += bTypeLen;
			pSechFrzOad += 4;
		}
		else	//choice 1 ROAD
		{
			DWORD bMainOAD;
			BYTE bLnkNum;

			bMainOAD = OoOadToDWord(pSechFrzOad);
			pSechFrzOad += 4;	//主OAD
			bLnkNum = *pSechFrzOad++;
			for (BYTE i=0; i<bLnkNum; i++)
			{
				if (bMainOAD == 0x50040200)
				{
					dwFrzOAD = OoOadToDWord(pSechFrzOad);
					
					if (dwFrzOAD==0x20210200 || dwFrzOAD==0x60400200 || dwFrzOAD==0x60410200 || dwFrzOAD==0x60420200)
					{
						fExistFrzOad = true;
						GetCurTime(&tNowTime);
						OoDateTimeSToTime(pSrc+1, &tRdTime);
						if (tRdTime.nYear==tNowTime.nYear && tRdTime.nMonth==tNowTime.nMonth && tRdTime.nDay==tNowTime.nDay)
							return 1;
					}
				}
				bTypeLen = OoGetDataTypeLen(pSrc);
				pSrc += bTypeLen;
				pSechFrzOad += 4;
			}
		}
	}

	if (fExistFrzOad)
		return 0;
	else
		return -1;
}

//描述：搜索698.45与645对应的ID规则
//参数：@pbCSD 面向对象698.45抄读数据项，
//			注意不要带DT_CSD格式，如日冻结
//			正向有功的格式为：01 //choice
//							  50 04 02 00	//主描述OAD 
//							  01	//关联描述OAD个数
//							  00 10 02 00	//关联OAD
//		@bMtrPro 电表协议
//		@pRespID 返回的数据ID，这里用数组返回
//		@pbNum 返回的ID个数
//返回：成功返回true，否则false
bool SearchAcqRule645ID(BYTE *pbCSD, BYTE bMtrPro, T645IdInfo *pT645IdInfo)
{
	TAcqRuleInfo tAcqRuleInfo;
	int iRet;
	BYTE bRuleBuf[512];
	char szTabName[ACQRULE_TABLE_NAME_LEN];

	memset(szTabName, 0, sizeof(szTabName));
	iRet = GetAcqRuleTableName(pbCSD, szTabName, sizeof(szTabName), &tAcqRuleInfo);	
	if (iRet < 0)
		goto ERR_RET;

	memset(bRuleBuf, 0, sizeof(bRuleBuf));
	iRet = GetAcqRuleFromTaskDB(szTabName, pbCSD+1, bRuleBuf);
	if (iRet < 0)
		goto ERR_RET;

	memset(szTabName, 0, sizeof(szTabName));
	iRet = GetOneAcqRuleInfo(bRuleBuf, szTabName, sizeof(szTabName), &tAcqRuleInfo);
	if (iRet < 0)
		goto ERR_RET;

	pT645IdInfo->bMtrPro = bMtrPro;
	if (bMtrPro == 1)	//645-97
	{
		BYTE *pb97 = tAcqRuleInfo.pbDlt97;

		if (*pb97++ == DT_STRUCT)	//AcqCmd_1997
		{
			pb97++;
			if (*pb97++ == DT_ARRAY)	//主用DI
			{
				pT645IdInfo->bMain645Num = *pb97++;
				if (pT645IdInfo->bMain645Num > sizeof(pT645IdInfo->dwMain645Id))
					goto ERR_RET;

				for (BYTE i=0; i<pT645IdInfo->bMain645Num; i++)
				{
					if (*pb97++ == DT_OCT_STR)
					{
						if (*pb97++ != 2)
							goto ERR_RET;
						pT645IdInfo->dwMain645Id[i] = OoOadToDWord(pb97);
						pb97 += 2;
					}
				}
			}

			if (*pb97++ == DT_ARRAY)	//备用DI
			{
				pT645IdInfo->bSlave645Num = *pb97++;
				if (pT645IdInfo->bSlave645Num > sizeof(pT645IdInfo->dwSlave645Id))
					goto ERR_RET;

				for (BYTE i=0; i<pT645IdInfo->bSlave645Num; i++)
				{
					if (*pb97++ == DT_OCT_STR)
					{
						if (*pb97++ != 2)
							goto ERR_RET;
						pT645IdInfo->dwSlave645Id[i] = OoOadToDWord(pb97);
						pb97 += 2;
					}
				}
			}
		}
	}
	else if (bMtrPro == 2)	//645-07
	{
		BYTE *pb07 = tAcqRuleInfo.pbDlt07;

		if (*pb07++ == DT_STRUCT)	//AcqCmd_2007
		{
			pb07++;

			if (*pb07++ == DT_ARRAY)	//主用DI
			{
				BYTE bMain645Num = *pb07++;

				//pT645IdInfo->bMain645Num = *pb07++;
				pT645IdInfo->bMain645Num = bMain645Num;
				if (pT645IdInfo->bMain645Num > (sizeof(pT645IdInfo->dwMain645Id)/sizeof(DWORD)))
					goto ERR_RET;

				for (BYTE i=0; i<pT645IdInfo->bMain645Num; i++)
				{
					if (*pb07++ == DT_OCT_STR)
					{
						if (*pb07++ != 4)
							goto ERR_RET;
						pT645IdInfo->dwMain645Id[i] = OoOadToDWord(pb07);
						pb07 += 4;
					}
				}
			}

			if (*pb07++ == DT_ARRAY)	//备用DI
			{
				pT645IdInfo->bSlave645Num = *pb07++;
				if (pT645IdInfo->bSlave645Num > sizeof(pT645IdInfo->dwSlave645Id))
					goto ERR_RET;

				for (BYTE i=0; i<pT645IdInfo->bSlave645Num; i++)
				{
					if (*pb07++ == DT_OCT_STR)
					{
						if (*pb07++ != 4)
							goto ERR_RET;
						pT645IdInfo->dwSlave645Id[i] = OoOadToDWord(pb07);
						pb07 += 4;
					}
				}
			}
		}
	}
	else
		goto ERR_RET;

	return true;
ERR_RET:
	DTRACE(DB_CRITICAL, ("SearchAcqRule645ID() error.....\n"));
	return false;
}

//描述：获取采集规则表名
//参数：@pbCSD 采集规则
//		@pszTabName 返回采集规则表名
//		@wTabNameLen 采集规则表名长度
//		@pTAcqRuleInfo 采集规则信息
//返回：成功返回pbCSD字节长度，否则-1
int GetAcqRuleTableName(BYTE *pbCSD, char *pszTabName, WORD wTabNameLen, TAcqRuleInfo *pTAcqRuleInfo)
{
	int iRet;
	DWORD dwOAD, dwROAD, dwLnkOAD;
	BYTE bLnkNum;
	BYTE *pbCSD0 = pbCSD;
	char szTmp[16];

	pTAcqRuleInfo->pCSD = pbCSD;
	if (*pbCSD++ == 0)		//OAD
	{
		dwOAD = OoOadToDWord(pbCSD);
		sprintf(pszTabName, "AcqRule_OAD_%08x", dwOAD);
		pbCSD += 4;
	}
	else //ROAD
	{
		dwROAD = OoOadToDWord(pbCSD);
		sprintf(pszTabName, "AcqRule_ROAD_%08x_", dwROAD);
		pbCSD += 4;
		bLnkNum = *pbCSD++;
		memset(szTmp, 0, sizeof(szTmp));
		sprintf(szTmp, "Num_%d_OAD", bLnkNum);
		strcat(pszTabName, szTmp);
		for (BYTE bLnkIdx=0; bLnkIdx<bLnkNum; bLnkIdx++)
		{
			dwLnkOAD = OoOadToDWord(pbCSD);
			pbCSD += 4;
			if (strlen(pszTabName) < (wTabNameLen-5))
			{
				memset(szTmp, 0, sizeof(szTmp));
				sprintf(szTmp, "_%08x", dwLnkOAD);
				strcat(pszTabName, szTmp);
			}
		}
	}

	iRet = pbCSD - pbCSD0;
	pbCSD = pbCSD0;

	return iRet;
}

//描述：获取一个规则库信息
//参数：@pbPara  规则库信息
//		@pszTabName 规则库对应的表名
//		@wTabNameLen 规则库对应的表名长度
//		@pTAcqRuleInfo 采集规则信息
//返回：成功返回一个规则库的长度，否则-1
int GetOneAcqRuleInfo(BYTE *pbPara, char *pszTabName, WORD wTabNameLen, TAcqRuleInfo *pTAcqRuleInfo)
{
	int iRet;
	BYTE bArryNum;
	BYTE *pOneRule;

	if (*pbPara++ == DT_STRUCT)
	{
		pbPara++;
		pOneRule = pbPara;
		if (*pOneRule++ == DT_CSD)
		{
			iRet = GetAcqRuleTableName(pOneRule, pszTabName, wTabNameLen, pTAcqRuleInfo);
			if (iRet < 0)
				goto ERR_RET;
			pOneRule += iRet;
			if (*pOneRule++ == DT_STRUCT)	//规则描述
			{
				BYTE bDiNum;
				pOneRule++;
				pTAcqRuleInfo->pbDlt07 = pOneRule;
				if (*pOneRule++ == DT_STRUCT)	//AcqCmd_2007
				{
					pOneRule++;
					if (*pOneRule++ == DT_ARRAY)	//主用DI
					{
						bDiNum = *pOneRule++;
						for (BYTE bDiIdx=0; bDiIdx<bDiNum; bDiIdx++)
						{
							if (*pOneRule++ == DT_OCT_STR)
							{
								if (*pOneRule++ == 4)
									pOneRule += 4;
								else
									goto ERR_RET;
							}
						}
					}

					if (*pOneRule++ == DT_ARRAY)	//备用DI
					{
						bDiNum = *pOneRule++;
						for (BYTE bDiIdx=0; bDiIdx<bDiNum; bDiIdx++)
						{
							if (*pOneRule++ == DT_OCT_STR)
							{
								if (*pOneRule++ == 4)
									pOneRule += 4;
								else
									goto ERR_RET;
							}
						}
					}
				}

				pTAcqRuleInfo->pbDlt97 = pOneRule;
				if (*pOneRule++ == DT_STRUCT)	//AcqCmd_1997
				{
					pOneRule++;
					if (*pOneRule++ == DT_ARRAY)	//主用DI
					{
						bDiNum = *pOneRule++;
						for (BYTE bDiIdx=0; bDiIdx<bDiNum; bDiIdx++)
						{
							if (*pOneRule++ == DT_OCT_STR)
							{
								if (*pOneRule++ == 2)
									pOneRule += 2;
								else
									goto ERR_RET;
							}
						}
					}

					if (*pOneRule++ == DT_ARRAY)	//备用DI
					{
						bDiNum = *pOneRule++;
						for (BYTE bDiIdx=0; bDiIdx<bDiNum; bDiIdx++)
						{
							if (*pOneRule++ == DT_OCT_STR)
							{
								if (*pOneRule++ == 2)
									pOneRule += 2;
								else
									goto ERR_RET;
							}
						}
					}
				}

				pTAcqRuleInfo->pbTrans = pOneRule;
				if (*pOneRule++ == DT_STRUCT)	//AcqCmd_Trans
				{
					pOneRule++;
					if (*pOneRule++ == DT_OCT_STR)
					{
						iRet = *pOneRule++;
						pOneRule += iRet;
					}
				}
			}
		}
	}

	return pOneRule - pbPara + 2;

ERR_RET:
	return -1;
}

//描述：从任务库里获取采集规则
//参数：@pszTabName 采集规则表名
//		@pbCSD
//		@pbRespRule 返回的采集规则数据
//返回：成功返回采集规则长度，否则-1
int GetAcqRuleFromTaskDB(char *pszTabName, BYTE *pbOAD, BYTE *pbRespBuf)
{
	TTdbReadCtrl TdbReadCtrl;
	TTdbSchRule	SchRule;
	TTdbSchCtrl	SchCtrl; 
	int	schID = -1;
	int fd;
	int iRet = -1;

	if ((fd=TdbOpenTable(pszTabName, O_RDWR|O_BINARY)) >= 0)
	{
		//初始化搜索规则字段
		memset(&SchRule, 0, sizeof(SchRule));
		SchRule.wField = 0;	
		SchRule.wOpNum = 1;	
		SchRule.wOp[0] = TDB_OP_EQ;
		memcpy(&SchRule.bVal[0][0], pbOAD, 4);	//+1：跳过choice

		//排序规则
		memset(&SchCtrl, 0, sizeof(SchCtrl));
		SchCtrl.wSortNum = 1;		//排序规则个数
		SchCtrl.wSortOp[0] = TDB_OP_EQ; 
		SchCtrl.wSortFild[0] = 0;
		SchCtrl.wRecsToSch = 1;
		SchCtrl.iPrivateRecStart = -1;
		SchCtrl.iPublicRecStart = -1;
		SchCtrl.wRecsFound = 0;

		schID =	TdbOpenSch(fd, &SchRule, 1, SchCtrl);
		if (schID<0	|| SchCtrl.wRecsFound==0)
		{
			if (schID >= 0)
				TdbCloseSch(schID);
			TdbCloseTable(fd);
		}
		else
		{
			BYTE bBuf[512] = {0};
			memset(&TdbReadCtrl, 0, sizeof(TdbReadCtrl));
			//TdbReadCtrl.dwFiledNeed	= TDB_ALL_FIELD;
			TdbReadCtrl.dwFiledNeed	= 0x02;
			TdbReadCtrl.iRecStart =	-1;	
			iRet = TdbReadRec(schID, &SchRule, 1, TdbReadCtrl, bBuf);
			if (iRet > 0)
			{
				iRet = ByteToWord(&bBuf[0]);
				memcpy(pbRespBuf, &bBuf[2], iRet);
			}
			TdbCloseSch(schID);
			TdbCloseTable(fd);
		}
	}

	return iRet;
}

//描述：保存采集规则文件表
//参数：@pszSaveTabName 需要保存的文件表
bool SaveAcqRuleTableName(char *pszSaveTabName)
{
	TAcqRuleTable tAcqRuleTable;
	WORD wIdx;
	char szRuleTableName[32];
	BYTE bMskIdx, bBit;
	
	memset((BYTE*)&tAcqRuleTable, 0, sizeof(tAcqRuleTable));
	memset(szRuleTableName, 0, sizeof(szRuleTableName));
	MK_ACQRULE_TABLE_NAME(szRuleTableName);
	//if (PartReadFile(szRuleTableName, 0, (BYTE*)&tAcqRuleTable, ACQRULE_FILE_HEAD_LEN))	//对应的文件不存在
	PartReadFile(szRuleTableName, 0, (BYTE*)&tAcqRuleTable, ACQRULE_FILE_HEAD_LEN);	//对应的文件不存在
	{
		for (bMskIdx=0; bMskIdx<sizeof(tAcqRuleTable.bMsk); bMskIdx++)
		{
			for (bBit=0; bBit<8; bBit++)
			{
				if (!(tAcqRuleTable.bMsk[bMskIdx] & (1<<bBit)))
				{
					wIdx = bMskIdx*8 + bBit;

					tAcqRuleTable.bMsk[bMskIdx] |= (1<<bBit);

					if (PartWriteFile(szRuleTableName, 0, (BYTE*)&tAcqRuleTable, ACQRULE_FILE_HEAD_LEN))//保存表名屏蔽字
					{
						if (PartWriteFile(szRuleTableName, ACQRULE_FILE_MSG_OFFSET(wIdx), (BYTE*)pszSaveTabName,/* strlen(pszSaveTabName)*/ACQRULE_TABLE_NAME_LEN))	//保存报文内容
							goto OK_RET;
					}
					
					goto ERR_RET;
				}
			}
		}
	}
// 	else
// 		goto ERR_RET;
OK_RET:
	DTRACE(DB_CRITICAL, ("SaveAcqRuleTableName() succ...\n"));
	return true;

ERR_RET:
	DTRACE(DB_CRITICAL, ("SaveAcqRuleTableName() fail...\n"));
	return false;
}

//描述：删除采集规则文件表
//参数：@pszSaveTabName 需要保存的文件表
bool DeleteAcqRuleTableName(char *pszDelTabName)
{
	TAcqRuleTable tAcqRuleTable;
	char szRuleTableName[32];

	memset((BYTE*)&tAcqRuleTable, 0, sizeof(tAcqRuleTable));
	memset(szRuleTableName, 0, sizeof(szRuleTableName));
	MK_ACQRULE_TABLE_NAME(szRuleTableName);
	if (PartReadFile(szRuleTableName, 0, (BYTE*)&tAcqRuleTable, ACQRULE_FILE_HEAD_LEN))	//对应的文件不存在
	{
		BYTE *pMsk = tAcqRuleTable.bMsk;
		for (BYTE bMskIdx=0; bMskIdx<sizeof(tAcqRuleTable.bMsk); bMskIdx++)
		{
			if (tAcqRuleTable.bMsk[bMskIdx] != 0)
			{
				for (BYTE bBit=0; bBit<8; bBit++)
				{
					if (tAcqRuleTable.bMsk[bMskIdx] & (1<<bBit))
					{
						WORD wIdx = bMskIdx*8 + bBit;
						BYTE bZeroBuf[ACQRULE_TABLE_NAME_LEN];
						char szDbTabName[ACQRULE_TABLE_NAME_LEN];

						memset(szDbTabName, 0, sizeof(szDbTabName));
						if (!PartReadFile(szRuleTableName, ACQRULE_FILE_MSG_OFFSET(wIdx), (BYTE*)szDbTabName, ACQRULE_TABLE_NAME_LEN))	//读取屏蔽字映射的表名
							goto ERR_RET;

						if (strcmp(pszDelTabName, szDbTabName) != 0)	//表名不相同继续检索下一个
							continue;

						memset(bZeroBuf, 0, sizeof(bZeroBuf));
						tAcqRuleTable.bMsk[bMskIdx] &= ~(1<<bBit);
						if (PartWriteFile(szRuleTableName, 0, (BYTE*)&tAcqRuleTable, ACQRULE_FILE_HEAD_LEN))//保存表名屏蔽字
						{
							if (PartWriteFile(szRuleTableName, ACQRULE_FILE_MSG_OFFSET(wIdx), (BYTE*)bZeroBuf, ACQRULE_TABLE_NAME_LEN))	//清除屏蔽字映射的表名
								goto OK_RET;
						}

						goto ERR_RET;
					}
				}
			}
		}
	}	
// 	else
// 		goto ERR_RET;
OK_RET:
	DTRACE(DB_CRITICAL, ("DeleteAcqRuleTableName() succ...\n"));
	return true;

ERR_RET:
	DTRACE(DB_CRITICAL, ("DeleteAcqRuleTableName() fail...\n"));
	return false;
}

//描述：从文件里获取表名，这里每次返回一个表名，外面判断piStart=-1时，结束
//参数：@piStart 该参数勿修改，初始化为-1
//		@pbRespTab 返回表名
//		@wMaxTabNameLen 表最大长度
//返回：成功true，否则false
bool GetAcqRuleTableName(int *piStart, char *pbRespTab, WORD wMaxTabNameLen)
{
	TAcqRuleTable tAcqRuleTable;
	char szRuleTableName[32];
	BYTE bMskIdx, bBit;
	WORD wIdx;

	if (wMaxTabNameLen < ACQRULE_TABLE_NAME_LEN)
		goto ERR_RET;
	else
		wMaxTabNameLen = ACQRULE_TABLE_NAME_LEN;

	memset(szRuleTableName, 0, sizeof(szRuleTableName));
	memset((BYTE*)&tAcqRuleTable, 0, sizeof(tAcqRuleTable));
	MK_ACQRULE_TABLE_NAME(szRuleTableName);
	if (PartReadFile(szRuleTableName, 0, (BYTE*)&tAcqRuleTable, ACQRULE_FILE_HEAD_LEN))	//对应的文件不存在
	{
		BYTE *pMsk = tAcqRuleTable.bMsk;
		for (bMskIdx=0; bMskIdx<sizeof(tAcqRuleTable.bMsk); bMskIdx++)
		{
			if (tAcqRuleTable.bMsk[bMskIdx] != 0)
			{
				for (bBit=0; bBit<8; bBit++)
				{
					if (!(tAcqRuleTable.bMsk[bMskIdx] & (1<<bBit)))
						continue;

					wIdx = bMskIdx*8 + bBit;
					if (wIdx < *piStart)
						continue;

					if (PartReadFile(szRuleTableName, ACQRULE_FILE_MSG_OFFSET(wIdx), (BYTE*)pbRespTab, wMaxTabNameLen))	//保存报文内容
					{
						*piStart = wIdx+1;
						goto OK_RET;
					}
					else
					{
						goto ERR_RET;
					}
				}
			}
		}

		if (bMskIdx >= sizeof(tAcqRuleTable.bMsk))
			goto ERR_RET;
	}
	else
		goto ERR_RET;

OK_RET:
	//DTRACE(DB_CRITICAL, ("GetAcqRuleTableName() succ...\n"));
	return true;
ERR_RET:
	*piStart = -1;
	//DTRACE(DB_CRITICAL, ("GetAcqRuleTableName() fail...\n"));
	return false;
}

int GetAllAcqRuleInfo(int *piStart, BYTE *pRespBuf, WORD wMaxLen)
{
	int iRet;
	char szTabName[ACQRULE_TABLE_NAME_LEN];
	BYTE *pRespBuf0 = pRespBuf;
	BYTE bNum;
	BYTE bROAD[8];
	char *pszOAD;
	char *pSchData = "AcqRule_ROAD_";
	

	if(*piStart == -1)
		*piStart = 0;
	bNum = 0;
	
	*pRespBuf++ = DT_ARRAY;
	pRespBuf++;	//跳过个数

	do 
	{
		//1.提取采集规则表名
		memset(szTabName, 0, sizeof(szTabName));
		if (!GetAcqRuleTableName(piStart, szTabName, sizeof(szTabName)))	
			goto OK_RET;
		
		if (*piStart < 0)
			goto OK_RET;

		if ((pszOAD=strstr(szTabName, pSchData)) == NULL)
			goto OK_RET;

		pszOAD += strlen(pSchData);

		memset(bROAD, 0, sizeof(bROAD));
		AsciiToByte((BYTE*)pszOAD, 8, bROAD);
		//2. 根据采集规则表名提取对应的内容
		iRet = GetAcqRuleFromTaskDB(szTabName, bROAD, pRespBuf);
		if (iRet < 0)
			continue;
		pRespBuf += iRet;
		bNum++;
	} while ((*piStart != -1) && ((wMaxLen-(pRespBuf-pRespBuf0)) > 200));	//<200作为备用空间，以防段错误


OK_RET:
	iRet = pRespBuf - pRespBuf0;
	pRespBuf = pRespBuf0;
	pRespBuf[1] = bNum;
	return iRet;
}
