/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CctRdCtrl.cpp
 * ժ    Ҫ���ز�������ƹ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�CL
 * ������ڣ�2016��8��
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

TSem	g_semMtrUdp;	//���������
TSem	g_semTskCfg;	//�������õ�Ԫ
TSem	g_semSchCfg;	//�ɼ���������

TMemMalloc g_TaskMem[TASK_ID_NUM];
TMemMalloc g_SchMem[TASK_ID_NUM];

//��������ʼ�����������
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

//��������ȡ���������
//���أ���������ֻ�����ָ��
const BYTE * GetMtrMask(BYTE bBank, WORD wPn, WORD wID)
{
	return GetItemRdAddr(bBank, wPn, wID);
}

//��������ʼ���������õ�Ԫӳ��
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

//��������ȡ�������ñ�
//������@bIndex ����ID����
//���أ�ָ������ID�ڴ�ӳ���
const BYTE* GetTaskCfgTable(WORD wTaskID)
{
	BYTE *pbPtr = NULL;

	WaitSemaphore(g_semTskCfg);

	if (g_TaskMem[wTaskID].pbCfg != NULL)
		pbPtr = g_TaskMem[wTaskID].pbCfg;

	SignalSemaphore(g_semTskCfg);

	return pbPtr;
}

//��������ʼ���ɼ���������ӳ��
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

//����: ȡ���������õĸ���
//����: �������õĸ���
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

//����: ȡ��ָ�����������������õ�Ԫ
//������ @bIndex ����ID������
//	    @pTaskCfg����������������ã����
//		@bIsRdTab�Ƿ���������ñ��л�ȡ�������õ�Ԫ
//����:���������ȷ���������򷵻�true�����򷵻�false
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
	pbPtr++;	//structure��Ա����
	pbPtr++;	//unsigned ����ID����
	pTaskCfg->bTaskId = *pbPtr++;
	if (pTaskCfg->bTaskId == 0)
		return false;
	pbPtr++;	//TI ʱ����
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
	pbPtr++;	//TI ʱ����
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
	pbPtr++;	//struct��Ա����
	pbPtr++;
	pTaskCfg->bPeriodType = *pbPtr++;
	pbPtr++;	//array
	pTaskCfg->bPeriodNum = *pbPtr++;
	for (BYTE i=0; i<pTaskCfg->bPeriodNum; i++)
	{
		pbPtr++;	//struct
		pbPtr++;	//struct ��Ա����
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

//������ȡ����ͨ�ɼ���������
//����: @pTaskCfg���������
//	     @pTAcqSchCfg�����������������
//����: Ϊ���ȡ�ɹ�
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
	case 0:	//�ɼ���ǰ����
		pbPtr++;	//NULL
		memset(pTCommAcqSchCfg->tTAcqType.bAcqData, 0, sizeof(pTCommAcqSchCfg->tTAcqType.bAcqData));
		break;
	case 1:	//�ɼ��ϵ�N��
		pbPtr++;	//unsigned 
		pTCommAcqSchCfg->tTAcqType.bAcqData[0] = *pbPtr;	pbPtr++;
		break;
	case 2:	//������ʱ��ɼ�
		pbPtr++;	//NULL
		memset(pTCommAcqSchCfg->tTAcqType.bAcqData, 0, sizeof(pTCommAcqSchCfg->tTAcqType.bAcqData));
		break;
	case 3:	
		pbPtr++;	//TI
		pTCommAcqSchCfg->tTAcqType.bAcqData[0] = *pbPtr;	pbPtr++;	//�����λ
		pbPtr++;	//long-unsigned
		pTCommAcqSchCfg->tTAcqType.bAcqData[1] = *pbPtr;	pbPtr++;	//���ֵ
		pTCommAcqSchCfg->tTAcqType.bAcqData[2] = *pbPtr;	pbPtr++;
		break;
	default:
        break;
	}

	//��ȡarray CSD
	pbPtr++;	//array
	pbArryCSD = (BYTE*)pbPtr;
	pTCommAcqSchCfg->bCSDNum = *pbPtr;	pbPtr++;	//+array number
	if (pTCommAcqSchCfg->bCSDNum > (sizeof(pTCommAcqSchCfg->tTCSD)/sizeof(TCSD)))	//�쳣����
	{
		DTRACE(DB_CCT, ("ERROR---InitRdMtrCtrl():: Current CSD number=%d, Max support CSD number=%d.\n", \
			pTCommAcqSchCfg->bCSDNum, (sizeof(pTCommAcqSchCfg->tTCSD)/sizeof(TCSD))));
		pTCommAcqSchCfg->bCSDNum = sizeof(pTCommAcqSchCfg->tTCSD)/sizeof(TCSD);
	}
	for (BYTE i = 0; i < pTCommAcqSchCfg->bCSDNum; i++)
	{
		pbPtr++;	//CSD
		pTCommAcqSchCfg->tTCSD[i].bChoice = *pbPtr;	pbPtr++;	//��ѡ������ [0]:OAD, [1]:ROAD
		if (pTCommAcqSchCfg->tTCSD[i].bChoice == 0)	//OAD
		{
			pTCommAcqSchCfg->tTCSD[i].dwOAD = OoOadToDWord((BYTE*)pbPtr);	pbPtr += 4;
		}
		else if (pTCommAcqSchCfg->tTCSD[i].bChoice == 1)	//ROAD
		{
			pTCommAcqSchCfg->tTCSD[i].tTROAD.dwOAD = OoOadToDWord((BYTE*)pbPtr);	pbPtr += 4;	//OAD
			pTCommAcqSchCfg->tTCSD[i].tTROAD.bOADNum = *pbPtr;	pbPtr++;	///������������OAD����
			if (pTCommAcqSchCfg->tTCSD[i].tTROAD.bOADNum > (sizeof(pTCommAcqSchCfg->tTCSD[i].tTROAD.dwOADArry)/(sizeof(DWORD))))	//�쳣����
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

	//��ȡMS����
	// 		BYTE bMS = *pbPtr++;	//MS
	// 		if (bMS != 92)	//MS��������
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

//������ȡ���¼��ɼ���������
//����: @index�¼��ɼ�������
//	     @pTAcqSchCfg�����������������
//����: Ϊ���ȡ�ɹ�
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
		//pbPtr++;	//array����
		pbPtr++;	//structure
		pbPtr++;	//+structure number
		pbPtr++;	//unsigned
		pTEvtAcqSchCfg->bSchNo = *pbPtr;	
		pbPtr++;
		
		pbPtr++;	//array
		pbPtr++;
		
		pbPtr++;   //�ɼ�����
		pbPtr++; 

		pbPtr++; //array
		pTEvtAcqSchCfg->bROADNum = *pbPtr;	
		pbPtr++;	//+array ��Ա����
		if (pTEvtAcqSchCfg->bROADNum > sizeof(pTEvtAcqSchCfg->tTROAD)/sizeof(TROAD))
		{
			DTRACE(DB_TASK, ("ERROR---GetEventSchCfg():  bSchNo=%d, Max support ROAD num=%d, current ROAD num=%d.\n", \
				 pTEvtAcqSchCfg->bSchNo, sizeof(pTEvtAcqSchCfg->tTROAD)/sizeof(TROAD), pTEvtAcqSchCfg->bROADNum));
			return false;
		}
		for (BYTE i = 0; i < pTEvtAcqSchCfg->bROADNum; i++)
		{
			pbPtr++;	//ROAD����
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
		//��ȡMS����
		BYTE bMS = *pbPtr;	//MS
		if (bMS != 92)	//MS��������
			return false;
		pTEvtAcqSchCfg->bMsChoice = *(pbPtr + 1);
		int iRet = ParserMsParam((BYTE*)pbPtr, pTEvtAcqSchCfg->bMtrMask, sizeof(pTEvtAcqSchCfg->bMtrMask));
		if (iRet < 0)
			return false;
		pbPtr += iRet;

		//�ϱ���ʶ
		pbPtr++;	//bool
		pTEvtAcqSchCfg->fRptFlg = *pbPtr;	
		pbPtr++;

		//�洢���
		pbPtr++;	//long-unsigned
		pTEvtAcqSchCfg->wStgCnt = OoLongUnsignedToWord((BYTE*)pbPtr);	
		pbPtr++;
	}

	return true;
}

//������ȡ���¼��ɼ���������
//����: @pTaskCfg���������
//	     @pTAcqSchCfg�����������������
//����: Ϊ���ȡ�ɹ�
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
		//pbPtr++;	//array����
		pbPtr++;	//structure
		pbPtr++;	//+structure number
		pbPtr++;	//unsigned
		pTEvtAcqSchCfg->bSchNo = *pbPtr;	
		pbPtr++;
		pbPtr++;	//array
		pTEvtAcqSchCfg->bROADNum = *pbPtr;	
		pbPtr++;	//+array ��Ա����
		if (pTEvtAcqSchCfg->bROADNum > sizeof(pTEvtAcqSchCfg->tTROAD)/sizeof(TROAD))
		{
			DTRACE(DB_TASK, ("ERROR---GetEventSchCfg(): bTaskID=%d, bSchNo=%d, Max support ROAD num=%d, current ROAD num=%d.\n", \
				pTaskCfg->bTaskId, pTEvtAcqSchCfg->bSchNo, sizeof(pTEvtAcqSchCfg->tTROAD)/sizeof(TROAD), pTEvtAcqSchCfg->bROADNum));
			return false;
		}
		for (BYTE i = 0; i < pTEvtAcqSchCfg->bROADNum; i++)
		{
			pbPtr++;	//ROAD����
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
		//��ȡMS����
		BYTE bMS = *pbPtr;	//MS
		if (bMS != 92)	//MS��������
			return false;
		pTEvtAcqSchCfg->bMsChoice = *(pbPtr + 1);
		int iRet = ParserMsParam((BYTE*)pbPtr, pTEvtAcqSchCfg->bMtrMask, sizeof(pTEvtAcqSchCfg->bMtrMask));
		if (iRet < 0)
			return false;
		pbPtr += iRet;

		//�ϱ���ʶ
		pbPtr++;	//bool
		pTEvtAcqSchCfg->fRptFlg = *pbPtr;	
		pbPtr++;

		//�洢���
		pbPtr++;	//long-unsigned
		pTEvtAcqSchCfg->wStgCnt = OoLongUnsignedToWord((BYTE*)pbPtr);	
		pbPtr++;
	}

	return true;
}

//��������ȡ����ִ�����ڵ�λ�Ƿ�С��Сʱ
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

//����: ȡ��ָ�������������к�
//������ @��
//����:�������к�
BYTE GetTaskCfgSn()
{
	BYTE bTaskSN = 0;

	//ReadItemEx(BANK16, PN0, 0x6011, &bTaskSN);

	return bTaskSN;
}

//����������MS����
//������@pbBuf ΪMS�е��������Ͷ���
//		@pbMtrMask ��MS����֮����ȡ�ĵ���ַ������
//		@wMtrMaskLen ��������ֳ���
//���أ�����MS�������ݵĳ���
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
	case 0:	//�޵��ܱ�
		memset(pbMtrMask, 0, PN_MASK_SIZE);
		break;
	case 1:	//ȫ���û���ַ
		memcpy(pbMtrMask, bMtrMask, PN_MASK_SIZE);
		break;
	case 2:	//һ���û�����
		BYTE bUserTypeArry[MS_ONE_GROUP_USER_TYPE];	//����8���û�����
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

	case 3:	//һ���û���ַ
		BYTE bTsa[10];
		BYTE bTsaLen;
		BYTE bTsaCnt;	

		bTsaCnt = *pbPtr++;
		for (BYTE bTsaCntIndex=0; bTsaCntIndex<bTsaCnt; bTsaCntIndex++)
		{
			*pbPtr++;	//����TSA�ĳ���
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
	case 4:	//һ���������
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

	case 5:	//һ���û���������
		//BYTE bUserRegion[4][3];	//�ݶ���4���û�����
		BYTE bUserRegionCnt;
		TUserRegion tTUserRegion[MS_ONE_GROUP_USER_TYPE_REGION];

		memset((BYTE*)&tTUserRegion, 0, sizeof(tTUserRegion));
		bUserRegionCnt = *pbPtr++;
		if (bUserRegionCnt > sizeof(tTUserRegion))
			bUserRegionCnt = sizeof(tTUserRegion);

		for (BYTE bUserRegionIndex=0; bUserRegionIndex<bUserRegionCnt; bUserRegionIndex++)
		{
			tTUserRegion[bUserRegionIndex].bRegionType = *pbPtr++;	//���䣬0-ǰ�պ󿪣�1-ǰ����գ�2-ǰ�պ�գ�3-ǰ����
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
							if (tTUserRegion[bUserRegionIndex].bRegionType == 0)	//0-ǰ�պ�
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
							else if (tTUserRegion[bUserRegionIndex].bRegionType == 1)	//1-ǰ�����
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
							else if (tTUserRegion[bUserRegionIndex].bRegionType == 2)	//2-ǰ�պ��
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
							else if (tTUserRegion[bUserRegionIndex].bRegionType == 3)	//3-ǰ����
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

	case 6:	//һ���û���ַ����
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
			*pbPtr++;	//����TSA�ĳ���
			bTsaLen = *pbPtr++ + 1;
			tTTsaRegion[bTsaRegionIndex].bStartTsaLen = bTsaLen;
			memcpy(tTTsaRegion[bTsaRegionIndex].bTsaStart, pbPtr, bTsaLen);	
			pbPtr += bTsaLen;
			pbPtr++;	//tsa
			*pbPtr++;	//����TSA�ĳ���
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

							if (tTTsaRegion[bTsaRegionIndex].bRegionType == 0)	//0-ǰ�պ�
							{
								if ((iCmpRet1==0 || iCmpRet1==1) && (iCmpRet2==2))
									fIsFindValidTsa = true;
							}
							else if (tTTsaRegion[bTsaRegionIndex].bRegionType == 1)	//1-ǰ�����
							{
								if ((iCmpRet1==1) && (iCmpRet2==0 || iCmpRet2==2))
									fIsFindValidTsa = true;
							}
							else if (tTTsaRegion[bTsaRegionIndex].bRegionType == 2)	//2-ǰ�պ��
							{
								if ((iCmpRet1==0 || iCmpRet1==1) && (iCmpRet2==0 || iCmpRet2==2))
									fIsFindValidTsa = true;
							}
							else if (tTTsaRegion[bTsaRegionIndex].bRegionType == 3)	//3-ǰ����
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

	case 7:	//һ�������������
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
							if (tTCfgMtrSnRegion[bGroupMtrIndex].bRegionType == 0)	//0-ǰ�պ�
							{
								if ((wMtrSn>=tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnStart) && (wMtrSn<tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnEnd))
								{
									fIsVlidMtrSn = true;
									break;
								}
								else
									continue;
							}
							else if (tTCfgMtrSnRegion[bGroupMtrIndex].bRegionType == 1)	//1-ǰ�����	
							{
								if ((wMtrSn>tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnStart) && (wMtrSn<=tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnEnd))
								{
									fIsVlidMtrSn = true;
									break;
								}
								else
									continue;
							}
							else if (tTCfgMtrSnRegion[bGroupMtrIndex].bRegionType == 2)	//2-ǰ�պ��
							{
								if ((wMtrSn>=tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnStart) && (wMtrSn<=tTCfgMtrSnRegion[bGroupMtrIndex].wCfgSnEnd))
								{
									fIsVlidMtrSn = true;
									break;
								}
								else
									continue;
							}
							else if (tTCfgMtrSnRegion[bGroupMtrIndex].bRegionType == 3)	//3-ǰ����
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

//��������RSD�л��MS�����֣��ú�����Ҫ����Բɼ�������
bool GetRSDMS(BYTE *pbRSD, BYTE *pbMtrMask, WORD wMaskSize)
{
	//RSD�еķ���
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

//��������òɼ������еı��ַ�����֣���Ҫ�����ͨ���¼���ʵʱ�ɼ�������
//������@pbBuf �ɼ���������
//		@p �ɼ�������Ӧ�����ݸ�ʽ
//		@bIndex MS�����ݸ�ʽ�е�ƫ��λ��
//		@pbMtrMask ��������
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

//������Դ��ַ��RSD����Ŀ�ĵ�ַ���ɼ��������Ƚ�
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
				if (pbSrcMask[i]&(1<<(bBit)))	//Դ��ַ��Ч
				{
					if (!(pbDstMask[i]&(1<<(bBit))))	//Ŀ�ĵ�ַ�����ڣ�ֱ���ж�Ϊʧ��
						return false;
				}
			}
		}
	}

	return true;
}

//��������Դ���ݽ�����һ�����ֶε�ƫ�ƺͳ��ȣ��������
//������@ pParser	 �ֶν�����
//	   @ pFmt	Դ���ݵĸ�ʽ������
//	   @ wFmtLen	Դ���ݵĸ�ʽ�������ĳ���
//	   @ fParseItem	Ϊfalseʱֻ���������ñ���
//					Ϊtrueʱ���ֶζ�Ӧ��������ĳ��ȼ�ƫ��Ҳ�������
//����:�����ȷ�򷵻�true,���򷵻�false
bool OoParseField(TFieldParser* pParser, BYTE* pFmt, WORD wFmtLen, bool fParseItem)
{
	BYTE *pbPtr2 = pParser->pbCfg;
	int iRet;

	if (*pbPtr2!=1 && *pbPtr2!=2 && *pbPtr2!=DT_FRZRELA)	//ֻ��struct and array
		return false;

	pbPtr2++;	//array or struct
	pParser->wNum = *pbPtr2;	pbPtr2++;	//����
	if (pParser->wNum > 64)
		return false;
	for (BYTE bFieldIndex=0; bFieldIndex<pParser->wNum; bFieldIndex++)
	{
		iRet = OoScanData(pParser->pbCfg, pFmt, wFmtLen, false, bFieldIndex, &pParser->wLen[bFieldIndex], &pParser->bType[bFieldIndex]);		
		if (iRet < 0)
			return false;

		pParser->wPos[bFieldIndex] = iRet;
		//���ֶ���Ӧ�����ݽ�������
		if (fParseItem)
		{
			iRet = OoGetDataLen(pParser->bType[bFieldIndex], &pParser->pbCfg[pParser->wPos[bFieldIndex]+1]);	//+1:ȥ����������
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


//�������Ƚ������ֶ��Ƿ���ͬ���߰����ڡ���ע��pbCmpField��pbSrcField�ǲ������ݸ�ʽ�ġ�
//������@ bCmpType �Ƚ��ֶε����� ������bCmpTypeΪRCSD�е�����CSD��
//	   @ pbCmpField�Ƚ��ֶε����� ������bCmpTypeΪRCSD�е�����CSD���ݣ�
//	   @ bSrcTypeԴ�ֶε����� ��������ͨ�ɼ������е�CSD����, bSrcType����ΪReadParserField()��pbType���ͣ�
//	   @ pbSrcFieldԴ�ֶε�����  (������ͨ�ɼ������е�CSD����, pbSrcField����ΪReadParserField()��pbBuf����)
//����:��������ֶ���ͬ�򷵻�0,����Ƚ��ֶΰ�����Դ�ֶη���1�����򷵻�-1
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
	case DT_OAD:	//OAD OAD������CSD\OAD\FMT_FRZRELA\ARRY CSD�Ƚ�
		if (bSrcType==DT_OAD || bSrcType==DT_CSD || bSrcType==DT_FRZRELA || bSrcType==DT_ARRAY)
		{
			if (bSrcType == DT_OAD)
			{
				if (memcmp(pbCmpField, pbSrcField, 4) == 0)
					return 0;
			}
			else if (bSrcType == DT_CSD)
			{
				if (*pbSrcField++ == 1)	//ֻ�ܱȽ�OAD
					return -1;
				if (memcmp(pbCmpField, pbSrcField, 4) == 0)
					return 0;
			}
			else if (bSrcType == DT_FRZRELA)
			{
				pbSrcField += 5;	//�����pbSrcField��ʼλ��Ϊ�����������͵���һ���ֽ�λ��
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
				pbSrcField += 2;	//�����pbSrcField��ʼλ��Ϊ�����������͵���һ���ֽ�λ�� ��λ��OAD����ʼλ��
				if (memcmp(pbCmpField, pbSrcField, 4) == 0)
					return 0;
			}
		}
		break;
	case DT_ROAD:	//ROAD������ROAD\CSD\ARRY CSD\ARRAY ROAD�Ƚ�
		if (bSrcType == DT_ROAD)
		{
			if (memcmp(pbCmpField, pbSrcField, 4) == 0)	//��������������OAD
			{
				pbCmpField += 4;
				pbSrcField += 4;
				bCmpRelaNum = *pbCmpField++;	//����OAD
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

				if (bCmpRelaNum == bEQNum)	//OADƥ��
				{
					if (bCmpRelaNum < bSrcRelaNum)	//�Ƿ������Դ�ֶ���
						return 1;
					return 0;
				}
			}
		}
		else if (bSrcType == DT_CSD)
		{
			if (*pbSrcField++ == 0)	//ֻ�ܺ�ROAD�Ƚ�
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
					if (*pbSrcField++ != 0)	//����ROAD
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
					if (bInNum == 0)	//�����ڱ�������ROAD
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
	case DT_CSD:	//CSD������OAD\ROAD\DT_FRZERLA\ARRAY CSD\ARRAY ROAD�Ƚ�
		if (*pbCmpField++ == 0)	//OAD�Ƚ�
		{
			if (bSrcType == DT_OAD || bSrcType==DT_FRZRELA)	//OAD
			{
				if (bSrcType==DT_FRZRELA)
					pbSrcField += 5;	//ƫ�ƶ�������
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
			else if (bSrcType == DT_ARRAY)	//array�����ROAD��CSD
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
			else if (bSrcType == DT_ARRAY)	//array�����ROAD��CSD 
			{
				//bSrcType=1���������壬һ����DT_ARRY��ʽ������һ����CSD�ڲ�Ϊ1����ROAD,����Ƚ��ֶ���Դ�ֶε�ǰ4���ֽ���ͬ����Ϊ��ROAD�Ƚ�
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
	case DT_RCSD:	//RCSD��ʽ������ArrayCSD��ArrayROAD��CSD��ROAD��OAD��FMT_FRZRELA�Ƚ�
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
					if (bSrcFmtType == bCmpFmtType)	//����ƥ��
					{
						iRet = FieldCmp(bCmpFmtType, pbCmpField, bSrcFmtType, pbSrcField+1);	//+1ȥ��ʽ����
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
				if (bInNum == 0)	//�����ڱ�������ROAD
					return 0;
				return 1;
			}
		}
		else if (bSrcType==DT_CSD || bSrcType==DT_ROAD 
			|| bSrcType==DT_OAD || bSrcType==DT_FRZRELA)
		{
			bRCSDNum = *pbCmpField++;
			if (bRCSDNum != 1)	//Դ�Զ�ֻ��һ���������ͣ��Ƚ��ֶδ��ڶ����������ʱ����������ǿ϶���ƥ���
				break;
			if (*pbCmpField == 0)	//OAD
				bCmpFmtType = DT_OAD;
			else 
				bCmpFmtType = DT_ROAD;
			return FieldCmp(bCmpFmtType, pbCmpField, bSrcType, pbSrcField);
		}
		break;
	case DT_FRZRELA:	//FMT_FRZRELA���Ϳ�����OAD��CSD��FMT_FRZRELA�Ƚ�
		if (bSrcType == DT_FRZRELA)
		{
			if (memcmp(pbCmpField, pbSrcField, 12) == 0)	//���Ͳ��Ƚ�
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

	case DT_PULSE_CFG:	//��������
		if (bSrcType == DT_PULSE_CFG)
		{
			if (memcmp(pbCmpField, pbSrcField, 11) == 0)	//���Ͳ��Ƚ�
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

//��������ROAD������ѡ���Զ�ȡ
//������@ pbSelROAD ѡ����ROAD��һ���Ӧ��Э��RCSD��CSD���ROAD
//	   @ pbSrcROADԴROAD��һ���Ӧ�����������һ��ROAD�ֶ�
//	   @ pbSelData �������ض�ȡ��������
//	   @ pbSrcData Դ���ݣ���Ӧ��������¼��һ��ROAD�ֶε�����
//����:��������ֶ���ͬ�򷵻�0,����Ƚ��ֶΰ�����Դ�ֶη���1�����򷵻�-1
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
		if (bLkCmpOAD <= bLkSrcOAD)	//OAD��������
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

//��������ROAD������ѡ���Զ�ȡ
//������@ pbSelROAD ѡ����ROAD��һ���Ӧ��Э��RCSD��CSD���ROAD
//	   @ pbSrcROADԴROAD��һ���Ӧ�����������һ��ROAD�ֶ�
//	   @ pbSelData �������ض�ȡ��������
//	   @ pbSrcData Դ���ݣ���Ӧ��������¼��һ��ROAD�ֶε�����
//����:��������ֶ���ͬ�򷵻�0,����Ƚ��ֶΰ�����Դ�ֶη���1�����򷵻�-1
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
		if (bLkCmpOAD <= bLkSrcOAD)	//OAD��������
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

						if (IsAllAByte(pOneSrcData, 0x00, iRet) || ((pOneSrcData[0]==0x00) && (pOneSrcData[1]!=0x00))) //ȫ0 || ����ʧ��
						{
							*pbSelData++ = 0x00;	//����ʧ�ܣ�ֱ�����0
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

//����: ����������һ����
//������@pszTableName	����
//	   @ pFixFields	�̶��ֶ�
//	   @ pDataFields	�����ֶΣ����û�������ֶδ���NULL
//	   @ dwRecNumMax ��¼��󱣴����
//����:�����ȷ�򷵻�0,���򷵻ظ���
int CreateTable(char* pszTableName, TFieldParser* pFixFields, TFieldParser* pDataFields, DWORD dwRecNumMax)
{
	TTabCtrl TabCtrl;
	int iFieldLen;
	BYTE bFieldData[512];	
	BYTE bFieldType;
	BYTE bFieldNum = 0;

	memset(&TabCtrl, 0, sizeof(TabCtrl));
	if (pFixFields != NULL)	//�̶��ֶ�
	{	
		for(WORD wIndex=0; wIndex<pFixFields->wNum; wIndex++)
		{ 
			WORD wItemOffset;
			WORD wItemLen;
			memset(bFieldData, 0, sizeof(bFieldData));
			if ((iFieldLen=ReadParserField(pFixFields, wIndex, bFieldData, &bFieldType, &wItemOffset, &wItemLen)) > 0)	//�������ص����ֶ����ݳ���
			{
				TabCtrl.wField[bFieldNum][0] = TDB_BYTE;
				TabCtrl.wField[bFieldNum][1] = wItemLen;	
				bFieldNum++;
			}
		}
	}

	if ((pDataFields != NULL) && (pDataFields->wCfgLen != 0))	//�����ֶ�
	{
		TabCtrl.wField[bFieldNum][0] = TDB_BYTE;
		TabCtrl.wField[bFieldNum][1] = pDataFields->wTotalLen;	
		bFieldNum++;
	}

	TabCtrl.bPublicNum = 0;							//�����ֶεĸ���
	TabCtrl.bPrivateNum = (bFieldNum!=0)? bFieldNum: 0;			//���ֶθ���
	TabCtrl.dwMaxRecPublicNum = 0;					//�ɼ�¼��������
	TabCtrl.dwMaxRecPrivateNum = dwRecNumMax;		//����¼�Ӹ���
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

//����: ������һ�ʼ�¼��������һ����
//������@pszTableName ����
//	   @ pbRec��¼����
//����:�����ȷ�򷵻�true,���򷵻�false
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

//����: �޸���N�ʼ�¼����
//������@pszTableName ����
//		@index ��N��		
//	   @ pbRec��¼����
//����:�����ȷ�򷵻�true,���򷵻�false
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

//����: ������һ�ʼ�¼��������һ����
//������@pszTableName ����
//	   @ pbRec��¼����
//	   @ piRecPhyIdx���ؼ�¼�洢������λ��
//����:�����ȷ�򷵻�true,���򷵻�false
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

//����: �޸���N�ʼ�¼����
//������@pszTableName ����
//		@index ��N��		
//	   @ pbRec��¼����
//����:�����ȷ�򷵻�true,���򷵻�false
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
#define SCH_FILE_MAX_SIZE	(40*SIZE_1M)	//ÿ�������ļ����ռ�40M
#define SCH_FILE_TOTAL_MAX_SIZE	(80*SIZE_1M)	//�ܵķ����ļ����ռ�80M

//��������ʼ���ɼ���������������
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

			//��ʼ���̶��ֶ�
			const TFieldCfg *pFixFieldCfg = &g_TSchFieldCfg[tTaskCfg.bSchType-1].tTFixFieldCfg;
			tFixFields.pbCfg = (BYTE*)pFixFieldCfg->pbCfg;
			tFixFields.wCfgLen = pFixFieldCfg->wCfgLen;

			if (!OoParseField(&tFixFields, (BYTE*)pFixFieldCfg->pbFmt, pFixFieldCfg->wFmtLen, true))
				continue;

			//��ʼ�������ֶ�
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
// 			case SCH_TYPE_REPORT:	//��ע���ϱ��ɼ���������������
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
					dwStgCnt = 96*5;	//�ܶ���С15���ӣ�����5������
					wPnNum = POINT_NUM;
#else	//C82/D82
					dwStgCnt = 96*5;	//�ܶ���С15���ӣ�����5������
					wPnNum = 20;
#endif
					break;
				case TIME_UNIT_HOUR:
#if FA_TYPE == FA_TYPE_K32
					dwStgCnt = 24*7;	//�ܶ���С1��Сʱ������7������
					wPnNum = POINT_NUM;
#else	//C82/D82
					dwStgCnt = 24*7;	//�ܶ���С1��Сʱ������7������
					wPnNum = 20;
#endif
					break;
				case TIME_UNIT_DAY:
					dwStgCnt = 60;	//�ܶ���СΪ1�죬����60������
					wPnNum = POINT_NUM;
					break;
				case TIME_UNIT_MONTH:
					dwStgCnt = 12;	//�ܶ���СΪ1���£�����12��������
					wPnNum = POINT_NUM;
					break;
				default:
					dwStgCnt = 100;	//��������Ϊ100
					wPnNum = POINT_NUM;
				}

				dwStgCnt = wPnNum * dwStgCnt;

				dwFieldDataLen = tFixFields.wTotalLen + tDataFields.wTotalLen;
				dwFileSize = dwStgCnt * dwFieldDataLen;
				if (dwFileSize > SCH_FILE_MAX_SIZE)	//�����洢�ռ����SCH_FILE_MAX_SIZE�����Ըÿռ����
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

				if (dwFileTotalSize >= SCH_FILE_TOTAL_MAX_SIZE)	//�����������ļ��ռ䳬��SCH_FILE_TOTAL_MAX_SIZE��ֹͣ�����ٴ���?
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

				pbArryROAD += 4; //�����ɼ�����
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
						//�����ṹ�ľ�����������
						tDataFields.pbCfg = pbArryROAD;
						tDataFields.wCfgLen = iCfgLen;
						//�������ֶν����Ľ��
						tDataFields.wNum = 1;
						tDataFields.wPos[0] = 0;
						tDataFields.wLen[0] = iLen;
						tDataFields.bType[0] = DT_ROAD;
						//���õ��ֶν�һ�������ɶ�Ӧ���������Ϣ
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

//������ȫ�ֳ�����ˢ�µ������ӿ�
//������@bTaskId ����ID
//		@bSchType ��������
//		@pbRecBuf ��������
//		@wIdex �ò���ֻ���ȫ�¼��ɼ����ã������������Ϳɲ��ÿ���
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

//�������Ƿ������������������OAD
bool IsSpecOobAttrDescOAD(BYTE* pbOAD)
{
	bool fRet = false;
	DWORD dwOAD;

	dwOAD = OoOadToDWord(pbOAD);
	switch (dwOAD & 0xFFFFFF00)
	{
	case 0x60000200:	//�ɼ��������ñ�
	case 0x60020200:	//�ѱ���
	case 0x60120200:	//�������õ�Ԫ
	case 0x60140200:	//��ͨ�ɼ�����
	case 0x60160200:	//�¼��ɼ�������
	case 0x60180200:	//͸���ɼ�������
	case 0x601C0200:	//�ϱ��ɼ�����
	case 0x60340200:	//�ɼ������ؼ�
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

	if (bSubIdx != 1)	//������Ϣֻ���õ�������1������Ϊ�ṹ�壬�޷�����
	{
		DTRACE(DB_FAPROTO, ("selector2_60000200: dwOAD=0x%08x subidx=%d invalid.\n", dwOAD, bSubIdx));
		return -1;
	}

	 //��ʼֵ 		 Data��
	 bDataType = *pbRSD;
	 iDataLen = OoGetDataTypeLen(pbRSD);
	 iStartValue = OoStdTypeToInt(pbRSD);
	 if (iDataLen < 0 || iStartValue == -1)
	 	goto error_1;

	 //pbRSD++;
	 pbRSD += iDataLen;
	 //����ֵ 		 Data��
	 iDataLen = OoGetDataTypeLen(pbRSD);
	 iEndValue = OoStdTypeToInt(pbRSD);
	 if (iDataLen < 0 || iEndValue == -1)
	 	goto error_1;
	 //pbRSD++;
	 pbRSD += iDataLen;
	 //���ݼ��		 Data
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
			p = OoGetField(bBuf+1, pOI->pFmt, pOI->wFmtLen, bSubIdx-1, &wDataLen, &bType);	//+1:0x6000�����ĵ�һ���Լ�Ϊ�����ɼ���Ԫ�����ݳ���
			if (p != NULL)
			{
				if (bDataType == *p)  //�Ƚ���������
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
	if (bRSD == 1)	//����1
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
		case 0x60000200:	//�ɼ��������ñ�
			if (bSubIdx != 1)	//������Ϣֻ���õ�������1������Ϊ�ṹ�壬�޷�����
			{
				DTRACE(DB_FAPROTO, ("SpecReadRecord: dwOAD=0x%08x subidx=%d invalid.\n", dwOAD, bSubIdx));
				return -1;
			}
			for (WORD i=0; i<POINT_NUM; i++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				if (((iRet=ReadItemEx(BANK0, i, pOI->wID, bBuf))>0) && !IsAllAByte(bBuf, 0, sizeof(bBuf)))
				{
					p = OoGetField(bBuf+1, pOI->pFmt, pOI->wFmtLen, bSubIdx-1, &wDataLen, &bType);	//+1:0x6000�����ĵ�һ���Լ�Ϊ�����ɼ���Ԫ�����ݳ���
					if (p != NULL)
					{
						if (memcmp(pbRSD, p, wDataLen) == 0)	//��ʽ���ͱȽ� && ���ݱȽ�
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
		case 0x60120200:	//�������õ�Ԫ
			for (WORD i=0; i<TASK_ID_NUM; i++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				if ((iRet = GetTaskConfigFromTaskDb(i, bBuf)) > 0)
				{
					p = OoGetField(bBuf, pOI->pFmt, pOI->wFmtLen, bSubIdx-1, &wDataLen, &bType);
					if (memcmp(pbRSD, p, wDataLen) == 0)	//��ʽ���ͱȽ� && ���ݱȽ�
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
		case 0x60140200:	//��ͨ�ɼ�������
		case 0x60160200:	//�¼��ɼ�������
		case 0x60180200:	//͸���ɼ�������
		case 0x601C0200:	//�ϱ��ɼ�������
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
					if (p!=NULL && (memcmp(pbRSD, p, wDataLen)==0))	//��ʽ���ͱȽ� && ���ݱȽ�
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
		case 0x60340200:	//�ɼ������ؼ�
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
						if (memcmp(pbRSD, p, wDataLen) == 0)	//��ʽ���ͱȽ� && ���ݱȽ�
						{
							if (wCopyLen + iRet > wBufSize)
							{
								DTRACE(DB_FAPROTO, ("SpecReadRecord: Buffer overflow, iRet=%d, wBufSize=%d.\n", iRet, wBufSize));
								goto SpecReadRecord_ret;
							}
							else
							{
// 								*pbBuf++ = 0x01;	//��¼����
// 								*pbBuf++ = 0x01;	//M����¼
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
			//OAD���ⲿ���
			//RCSD
			*pbBuf++ = 0x01;	//RCSD����
			*pbBuf++ = 0x00;	//ѡ��OAD
			pbBuf += OoDWordToOad(dwSubOAD&0xffffff00, pbBuf);
		}
    
		switch (dwOAD & 0xFFFFFF00)
		{
		case 0x60020200:	//�����ѱ���
		
			if (bSubIdx != 6)	//�����ѱ���Ϣ������6������Ϊ�ṹ�壬�޷�����
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
	else if (bRSD == 0)//̨��Э��һ���Բ����޸�
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
		case 0x60000200:	//�ɼ��������ñ�
			bSubIdx = 1;
			if (bSubIdx != 1)	//������Ϣֻ���õ�������1������Ϊ�ṹ�壬�޷�����
			{
				DTRACE(DB_FAPROTO, ("SpecReadRecord: dwOAD=0x%08x subidx=%d invalid.\n", dwOAD, bSubIdx));
				return -1;
			}
			for (WORD i=0; i<POINT_NUM; i++)
			{
				memset(bBuf, 0, sizeof(bBuf));
				if (((iRet=ReadItemEx(BANK0, i, pOI->wID, bBuf))>0) && !IsAllAByte(bBuf, 0, sizeof(bBuf)))
				{
					p = OoGetField(bBuf+1, pOI->pFmt, pOI->wFmtLen, bSubIdx-1, &wDataLen, &bType);	//+1:0x6000�����ĵ�һ���Լ�Ϊ�����ɼ���Ԫ�����ݳ���
					if (p != NULL)
					{
						//if (memcmp(pbRSD, p, wDataLen) == 0)	//��ʽ���ͱȽ� && ���ݱȽ�
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
		case 0x60120200:	//����ɼ�������
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
		case 0x60140200:	//��ͨ�ɼ�������
		case 0x60160200:	//�¼��ɼ�������
		case 0x60180200:	//͸���ɼ�������
		case 0x601C0200:	//�ϱ��ɼ�������
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
					//if (p!=NULL && (memcmp(pbRSD, p, wDataLen)==0)) //��ʽ���ͱȽ� && ���ݱȽ�
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


//����: ͨ��Э����õĶ���¼�ӿ�
//������@ pbOAD	��������������
//	   @ pbRSD��ӦЭ���е�RSCD
//	   @ pbRCSD ��ӦЭ���е�RCSD
//	   @ piTabIdx ��Ӧ���񷽰���ƫ�ƣ���һ�ε��ô���-1�����������߲��ܸı�
//	   @ piStart �������뼰������������ʼλ��,��һ�ε��ô���-1�����������߲��ܸı�
//				��������ʱ����������������Ϊ-2
//	   @ wSchNum ÿ�����������صļ�¼�������ѵ���ô������̷���
//	   @ pwRetNum �����������صļ�¼������pwRetNum< wSchNum��ʾ��������
//	   @ pbBuf�������ؼ�¼������
//����:�����ȷ�򷵻����ݵĳ���,���򷵻ظ���
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
		wRcsdIdx = 1;	//Э�����RCSD��һ���ֽ�ΪCSD��������������������ʼ��Ϊ1
		iRetNum = SearchTable(pbOAD, pbRSD, pbRCSD, wRcsdIdx, piTabIdx, pszTableName, &tFixFields, &tDataFields);
		if(iRetNum == -2)
		{
			return iRetNum;
		}

		if (iRetNum <= 0)	//�ѱ�ʧ��
			return -4;

		iRecLen = 0;
		wSucRetNum = 0;
		wRcsdNum = *pbRCSD;
		wSchNum = wBufSize/(tFixFields.wTotalLen + tDataFields.wTotalLen);	//���֧�ֵ��ڴ�ռ�
		DTRACE(DB_TASK, ("###1-wBufSize=%d, wFieldsLen=%d, wSchNum=%d.\n", wBufSize, tFixFields.wTotalLen + tDataFields.wTotalLen, wSchNum));

		if (wRcsdNum == iRetNum)	//��ȫƥ�䣬ֻ��һ�ű�
		{
			DTRACE(DB_TASK, ("###2-wBufSize=%d, wFieldsLen=%d, wSchNum=%d.\n", wBufSize, tFixFields.wTotalLen + tDataFields.wTotalLen, wSchNum));

NEXT_ONE_TABLE:	//Ϊ�˽����ͨ�ɼ���������MS����
			if (wSchNum > wSucRetNum)
				iRet = ReadTable(pszTableName, &tFixFields, &tDataFields, pbOAD, pbRSD, pbRCSD, piStart, wSchNum-wSucRetNum, pwRetNum, pbBuf);
			else
				iRet = -1;

			if (iRet > 0)
			{
				wSucRetNum += *pwRetNum;
				pbBuf += iRet;
			}

			if (wSchNum > wSucRetNum)	//����δ����������������
			{
				if (*piTabIdx != 0)	//���б�δ������
				{
					if (*piStart==-2 || *piStart == -1)	//һ�ű������ȫ���������ˣ����б�
					{
						do 
						{
							*piStart = -1;	//���³�ʼ��
							*piTabIdx = *piTabIdx + 1;	//����һ�ű�

							wRcsdIdx = 1;	//Э�����RCSD��һ���ֽ�ΪCSD��������������������ʼ��Ϊ1
							iRetNum = SearchTable(pbOAD, pbRSD, pbRCSD, wRcsdIdx, piTabIdx, pszTableName, &tFixFields, &tDataFields);
							if (iRetNum == wRcsdNum)	//�ѱ�ɹ�
							{
								wBufSize = wBufSize - (pbBuf-pbBuf0);
								wSchNum = wBufSize/(tFixFields.wTotalLen + tDataFields.wTotalLen) + wSucRetNum;	//���֧�ֵ��ڴ�ռ�
								DTRACE(DB_TASK, ("###3-wBufSize=%d, wFieldsLen=%d, wSchNum=%d.\n", wBufSize, tFixFields.wTotalLen + tDataFields.wTotalLen, wSchNum));

								goto NEXT_ONE_TABLE;
							}

							if (*piTabIdx == 0)	//�����������ֱ���˳�
								break;
						}while (iRetNum != wRcsdNum);
					}
				}
			}

			*pwRetNum = wSucRetNum;
			iRecLen = pbBuf - pbBuf0;
		}
		else	//����ƥ�䣬���ڶ��ű�
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
				bTmpRcsd[0] = 1;	//����Э���RCSD�е�һ��CSD
				memcpy(&bTmpRcsd[1], pbRCSD+1+wRcsdIdx, sizeof(bTmpRcsd)-1);	//RCSD�еĲ���ȫ��������ʵ�ʴ���RCSD�е�CSD������bTmpRcsd[0]����
				iRetNum = SearchTable(pbOAD, pbRSD, bTmpRcsd/*pbRCSD*/, wRcsdIdx, piTabIdx, pszTableName, &tFixFields, &tDataFields);
				if (iRetNum <= 0)	//�ѱ�ʧ�ܣ�ֱ�������Ч����NULL
				{
					*pbBuf++ = DT_NULL;
					iRetNum = 1;	//ǿ�Ƹ�1������������wRcsdIdxƫ��
				}
				else
				{
					if ((iRet=ReadTable(pszTableName, &tFixFields, &tDataFields, pbOAD, pbRSD, bTmpRcsd, piStart, 1, pwRetNum, pbBuf)) <= 0)
							return 0;// return -1;	//ֱ���˳�, Э��һ���� Get_10
					pbBuf += iRet;
					wSucRetNum += *pwRetNum;
				}

				if (i++ >= wSchNum)	//�����Ƿ����
				{
					*pwRetNum = wSucRetNum;
					iRecLen = pbBuf - pbBuf0;
					pbBuf = pbBuf0;
					return iRecLen;
				}

				//for (WORD k=0; k<iRetNum; k++)	//���ݷ��صĸ�������wRcsdIdx�����ϼ���ƫ��
				wRcsdIdx += ScanCSD(pbRCSD+1+wRcsdIdx, false);

				if (wRcsdIdx >= iRcsdLen)	
				{
					*piStart = -1;
					break;
				}

				*piStart = -1;

			}

			if (*pbOAD != 0x50)	//���᲻��Ҫ�������ۼ�OI_FRZ
				*pwRetNum = wSucRetNum;
			iRecLen = pbBuf - pbBuf0;
		}

		pbBuf = pbBuf0;
	}

	return iRecLen;
}

//����:�����ĵ����ȡ�ӿ�
//������@pszTableName	����
//	   @ pFixFields	�̶��ֶ�
//	   @ pDataFields	�����ֶΣ����û�������ֶδ���NULL
//	   @ pbRSD��ӦЭ���е�RSCD
//	   @ pbRCSD ���ݱ�������֧�ֵ��ֶ�
//	   @ piStart �������뼰������������ʼλ��,��һ�ε��ô���-1�����������߲��ܸı�
//				��������ʱ����������������Ϊ-2
//	   @ wSchNum ÿ�����������صļ�¼�������ѵ���ô������̷���
//	   @ pwRetNum �����������صļ�¼������pwRetNum< wSchNum��ʾ��������
//	   @ pbBuf�������ؼ�¼������
//����:�����ȷ�򷵻����ݵĳ���,���򷵻ظ���
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
	if (bRsdMethod==9 || bRsdMethod==10)	//RSD����9,10
	{
		bRecNo = pbRSD[1];
		fFromEnd = true;
	}
	else if (bRsdMethod == 0)//RSD����0���⴦��Ϊ����9��̨��Э��һ���Բ����޸�
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
				//����9���⴦��
				if (bRsdMethod == 9)
				{
					if (bRecNo <= *pwRetNum)	//�ϵ�N����¼�ڵ�ǰ���������ķ�Χ��
					{
						if (i != (bRecNo-1))
							continue;
					}
					else
					{
						bRecNo -= *pwRetNum;	//������������Ҽ�¼
						break;
					}
				}

				pbRCSD = pbRCSD0;
				bCSDNum = *pbRCSD++;
				for (bIndex=0; bIndex<bCSDNum; bIndex++)
				{
					iRet = ReadRecField(pbRec, 0, pFixFields, pbOAD, pbRCSD, pbRespRec);
					if (iRet < 0)	//�̶��ֶ�δ�ҵ���¼,���������ֶ�
					{
						iRet = ReadRecField(pbRec, pFixFields->wTotalLen, pDataFields, pbOAD, pbRCSD, pbRespRec);
						if (iRet < 0)	//�����ֶ�δ�ҵ���¼��ֱ�ӷ���
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

				if (bCSDNum == wSchSubNum)	//�ҵ�һ�ʼ�¼
					wRetNum++;

				if(wRetNum >= wSchNum)	//�����ﵽ����
					goto ReadTable_ret;

				//����9��10���⴦������10�Ƚ��ر��漰��MS�ļ��ϣ�������ÿ��ֻ�ܴ���һ���������RSD�е�MS����Χ��������ʱ�����뽫MS��ֳ�һ��һ�����ַ�������ݻ�ȡ
				if (bRsdMethod == 10)
				{
					if (wRetNum == bRecNo)	//��������N����¼���˳�
					{
						*piStart = -2;
						goto ReadTable_ret;
					}
				}
				else if (bRsdMethod == 9)	//����������Ѿ��ҵ����ϵ�N�ʼ�¼
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

//����: ȡ�����������Ϣ
//������@ bType	�������ͣ�ͬData���������Ͷ���
//	   @ pItemDesc ������Ŀ����������OAD��ROAD��CSD
//	   @ pwLen	�����������ݳ���
//	   @ppFmt	�������ظ�ʽ������
//	   @pwFmtLen	�������ظ�ʽ�������ĳ���
//���أ���ȷ�򷵻�true�����򷵻�false
bool OoGetItemInfo(BYTE bType, BYTE* pItemDesc, WORD* pwLen, BYTE** ppFmt, WORD* pwFmtLen)
{
	return false;
}

//���������Arry�������⴦��
//������@pbDst Ŀ������
//		@pbSrc Ԫ����
//		@wSrcLen Ԫ���ݳ���
//���أ�pbDst���ݵĳ���
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
		pbDst++;	//����array��Ա����������ͨ��bValidArryNum���
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
					if (bFmtLen==1 || IsAllAByte(pbSrc+1, 0xfe, bFmtLen-1))	//+1������ʽ
					{
						pbSrc += bFmtLen;	//bFmtLen+1�� ��ʽ+��ʽ���ֽ���
					}
					else
					{
						bValidArryNum++;
						memcpy(pbDst, pbSrc, bFmtLen);
						pbSrc += bFmtLen;	//bFmtLen+1�� ��ʽ+��ʽ���ֽ���
						pbDst += bFmtLen;	//bFmtLen+1�� ��ʽ+��ʽ���ֽ���
					}
				}
			}
			pbDst0[1] = bValidArryNum;
		}
		else
		{
			pbDst0[1] = 0x00;	//ARRAY ����0����Ա
		}

		iRet = pbDst - pbDst0;
		pbDst = pbDst0;

		*pwRetSrcLen = pbSrc - pbSrc0;
	}

	return iRet;
}

//������ROAD����OAD���ݿ���
//������@pbDst Ŀ������
//		@pbSrc Դ����
//		@pwRetSrcLen Դ���ݷ��ص�ƫ��
//���أ��ɹ�����Ŀ�����ݳ��ȣ����򷵻�-1
int OoCopyDataROADLinkOAD(BYTE *pbDst, WORD *pwRetDstLen, BYTE *pbSrc, WORD *pwRetSrcLen, BYTE *pbFmt, WORD wFmtLen)
{
	int iLen;
	WORD wRetSrcLen;
	BYTE *pbDst0 = pbDst;
	BYTE *pbSrc0 = pbSrc;
	BYTE *pROAD = pbFmt;
	BYTE bLkNum;

	pROAD += 4;	//��������������
	bLkNum = *pROAD++;	//����������������������
	for (BYTE i=0; i<bLkNum; i++)
	{
		if (*pbSrc == DT_ARRAY)
		{
			wRetSrcLen = 0;
			iLen = FmtArryData(pbDst, pbSrc, &wRetSrcLen);
			if (iLen < 0)	//С��0�޷�����pbSrc0��ƫ�ƣ��ʲ����ж����µ����ݣ�ֱ���˳�
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
	dwOIAtt &= OAD_FEAT_MASK;	//��ȡOAD, ֻ���ε���������
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
	else if (bIndex != 0)	//��������
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
			if (iDataLen > 0)	//����OI��ID������ľ��ǵ�N������
			{
				if (!IsNeedRdSpec(pOAMap))	//������OI
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
				if (bArryNum != *pbSrc)	//�����ʽ�и��������������еĸ��������������еĸ���
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
					pFmt += 4;	//������OAD
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
				pFmt += 4;	//������OAD
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
			pFmt += 4;	//������OAD
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

//����:�����������ݿ���
//������@ pbDst	Ŀ���ַ
//	   @ pbSrc	Դ��ַ
//	   @ wLen	���ݳ���
//	   @ pFmt ��ʽ������
//	   @ wFmtLen��ʽ����������
//����:�����ȷ�򷵻����ݵĳ���,���򷵻ظ���
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

	if ((*pbSrc0==DAR && *(pbSrc0+1)!=0 && IsAllAByte(pbSrc0+2, 0, wLen-2)) || (IsAllAByte(pbSrc0, 0, wLen)))	//DAR=0 || ����ȫΪ0����ʾ���ݳ���ʧ��
	{
// 		memcpy(pbDst, pbSrc0, 2);	//DAR + DAR_FMT 
// 		pbDst += 2;
		*pbDst++ = 0;
	}
	else if (dwOAD>=0x30000000 && dwOAD<0x40000000)	//�¼�
	{
		if ((*pFmt==DT_OAD) && (OoOadToDWord(pFmt+1)==0x20240200))	//����Դ
		{
			switch(dwOAD)
			{
			case 0x30090200:	//���ܱ������й����������¼�	���·���ԴΪNULL
			case 0x300A0200:	//���ܱ����й����������¼�
			case 0x300B0200:	//���ܱ��޹����������¼�
			case 0x300C0200:	//���ܱ��������������¼�
			case 0x300D0200:	//���ܱ�ȫʧѹ�¼�
			case 0x300E0200:	//���ܱ�����Դ�����¼�
			case 0x300F0200:	//���ܱ��ѹ�������¼�
			case 0x30100200:	//���ܱ�����������¼�
			case 0x30110200:	//���ܱ�����¼�
			case 0x30130200:	//���ܱ������¼�
			case 0x30140200:	//���ܱ����������¼�
			case 0x30150200:	//���ܱ��¼������¼�
			case 0x30160200:	//���ܱ�Уʱ�¼�
			case 0x30170200:	//���ܱ�ʱ�α����¼�
			case 0x30180200:	//���ܱ�ʱ�������¼�
			case 0x30190200:	//���ܱ������ձ���¼�
			case 0x301A0200:	//���ܱ�����ձ���¼�
			case 0x301B0200:	//���ܱ����¼�
			case 0x301C0200:	//���ܱ���ť���¼�
			case 0x301D0200:	//���ܱ��ѹ��ƽ���¼�
			case 0x301E0200:	//���ܱ������ƽ���¼�
			case 0x301F0200:	//���ܱ���բ�¼�
			case 0x30200200:	//���ܱ��բ�¼�
			case 0x30210200:	//���ܱ�ڼ��ձ���¼�
			case 0x30220200:	//���ܱ��й���Ϸ�ʽ����¼�
			case 0x30240200:	//���ܱ���ʲ��������¼�
			case 0x30250200:	//���ܱ���ݱ����¼�
			case 0x30260200:	//���ܱ���Կ�����¼�
			case 0x30270200:	//���ܱ��쳣�忨�¼�
			case 0x30280200:	//���ܱ����¼
			case 0x30290200:	//���ܱ��˷Ѽ�¼
			case 0x302A0200:	//���ܱ�㶨�ų������¼�
			case 0x302B0200:	//���ܱ��ɿ��������¼�
			case 0x302C0200:	//���ܱ��Դ�쳣�¼�
			case 0x302D0200:	//���ܱ�������ز�ƽ���¼�	
			case 0x302E0200:	//���ܱ�ʱ�ӹ����¼�
			case 0x302F0200:	//���ܱ����оƬ�����¼�
			case 0x31000200:	//�ն˳�ʼ���¼�
			case 0x31010200:	//�ն˰汾����¼�
			case 0x31040200:	//�ն�״̬����λ�¼�
			//case 0x31060200:	//�ն�ͣ/�ϵ��¼�
			case 0x31090200:	//�ն���Ϣ��֤�����¼�
			case 0x31100200:	//��ͨ�����������¼�
			case 0x31140200:	//�ն˶�ʱ�¼�
			case 0x311A0200:	//���ܱ�����״̬�л��¼�
				*pbDst++ = 0x00;
				break;
			case 0x30230200:	//���ܱ��޹���Ϸ�ʽ����¼�
				memcpy(pbDst, pbSrc0, 2);	//DT_ENUM �����ݸ�ʽ
				pbDst += 2;
				break;
			case 0x31070200:	//�ն�ֱ��ģ����Խ�����¼�
			case 0x31080200:	//�ն�ֱ��ģ����Խ�����¼�
				memcpy(pbDst, pbSrc0, 5);	//DT_OAD �����ݸ�ʽ
				break;
			case 0x310A0200:	//�ն˹��ϼ�¼
				memcpy(pbDst, pbSrc0, 2);	//DT_ENUM �����ݸ�ʽ
				pbDst += 2;
				break;
			case 0x31050200:	//���ܱ�ʱ�ӳ����¼�
			case 0x310B0200:	//���ܱ�ʾ���½��¼�
			case 0x310C0200:	//�����������¼�
			case 0x310D0200:	//���ܱ�����¼�
			case 0x310E0200:	//���ܱ�ͣ���¼�
			case 0x310F0200:	//�ն˳���ʧ���¼�
				*pbDst++ = *pbSrc0++;	//DT_TSA
				bTsaLen = *pbSrc0++;
				*pbDst++ = bTsaLen;		//length	//zhq modify 170428
				memcpy(pbDst, pbSrc0, bTsaLen);
				pbDst += bTsaLen;
				break;
			case 0x31060200:	//�ն�ͣ/�ϵ��¼�
			case 0x31150200:	//ң����բ��¼
				memcpy(pbDst, pbSrc0, 2);	//DT_UNSIG �����ݸ�ʽ
				pbDst += 2;
				break;
			case 0x31190200:	//�ն˵�����·�쳣�¼�
				memcpy(pbDst, pbSrc0, 2);	//DT_ENUM �����ݸ�ʽ
				pbDst += 2;
				break;
			case 0x32020200:	//����������ü�¼
				memcpy(pbDst, pbSrc0, 3);	//DT_OI �����ݸ�ʽ
				pbDst += 3;
				break;
			default:
				break;
			}
		}
		else if ((*pFmt==DT_OAD) && (OoOadToDWord(pFmt+1)==0x33020206)) //��̶����б�  array OAD			
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
				memcpy(pbDst, pbSrc0+1, *pbSrc0);	//DT_OI �����ݸ�ʽ
				pbDst += *pbSrc0;			
			}
		}	
		else if ((*pFmt==DT_OAD) && (OoOadToDWord(pFmt+1)==0x330C0206)) //�¼������б�
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
		else if ((*pFmt==DT_OAD) && (OoOadToDWord(pFmt+1)==0x33000200)) //�¼��ϱ�״̬
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
		else if ((*pFmt==DT_OAD) && (OoOadToDWord(pFmt+1)==0x20200200)) //�¼�����ʱ��
		{
			if (*pbSrc0 == DT_NULL)
				*pbDst++ = 0x00;
			else
			{
				memcpy(pbDst, pbSrc0, 8);
				pbDst += 8;
			}
		}
		else if ((*pFmt==DT_OAD) && (((OoOadToDWord(pFmt+1)) & 0x0000ff00 )==0x00008200))	//�¼�����������
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
			*pbDst++ = bTsaLen+1;	//TSA����
			*pbDst++ = bTsaLen - 1;	//TSA�ڲ�octet���ݳ���
			memcpy(pbDst, pbSrc0, bTsaLen);
			pbDst += bTsaLen;
		}
		else if ((*pFmt==DT_OAD) && (dwOAD==0x60400200 || dwOAD==0x60410200 || dwOAD==0x60420200))	//���ϸ�ʽ
		{
			*pbDst++ = DT_DATE_TIME_S;
			memcpy(pbDst, pbSrc0, wLen);
			pbDst += wLen;
		}
		else if ((*pFmt==DT_OAD) && (OoOadToDWord(pFmt+1)==0x3303206)) //�ѱ���
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
		else if ((*pFmt==DT_CSD) && (*(pFmt+1)==1))	//CSD->ROAD �ڲ����ڹ���OAD���赥����������
		{
			wRetDstLen = 0;
			wRetSrcLen = 0;
			OoCopyDataROADLinkOAD(pbDst, &wRetSrcLen, pbSrc+wItemOffset, &wRetDstLen, pFmt+2, wFmtLen-2);
			pbDst += wRetSrcLen;
			pbSrc += wRetDstLen;
		}
		else if ((dwMainOAD&0xffffff00)==0x60120300 && (*pFmt==DT_ROAD) && (dwOAD>=0x30000000 && dwOAD<0x33000000))	//�¼�
		{
			if (dwMainOAD == 0x60120300)
			{
				wRetDstLen = 0;
				wRetSrcLen = 0;
				OoCopyDataROADLinkOAD(pbDst, &wRetSrcLen, pbSrc+7, &wRetDstLen, pFmt+1, wFmtLen); //ƫ�����ݳ����ֽ�+���������ֽ� 2+5
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
						pFmt += 4;	//������OAD
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
						pFmt += 4;	//������OAD
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
					pFmt += 4;	//������OAD
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

//����: �������������������ֶ��ҳ���Ӧ��������
//������@ pbOAD	��������������
//	   @ pbRSD ��ӦЭ���е�RSD
//	   @ pbRCSD��һ�ε���ʱ����Э�鷢����������RCSD��
//				����޷�ƥ���򴫵ݵ���RCSD����
//	   @ pszTableName ���������������������봫�뻺����ָ��
//	   @ pFixFields	����ҵ���Ӧ�����������ʼ���ñ�Ĺ̶��ֶ�
//	   @ pDataFields	����ҵ���Ӧ�����������ʼ���ñ�������ֶ�
//����: �����ȷ�򷵻�RCSD��ƥ�䵽�ĸ���,���򷵻�0
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

	if ((dwOAD&0xffffff00)==0x60120300 || dwOAD==0x60120400)	//��ͨ���¼��ɼ�������ʱʵ��زɼ�����
	{
		for (wIdx=*piTabIdx; wIdx<TASK_NUM; wIdx++)
		{
			memset((BYTE*)&tTaskCfg, 0, sizeof(tTaskCfg));
			if (!GetTaskCfg(wIdx, &tTaskCfg))
				continue;

			//�̶��ֶ�����
			memset((BYTE*)&tFixFields, 0, sizeof(TFieldParser));
			tFixFields.pbCfg = (BYTE*)g_TSchFieldCfg[tTaskCfg.bSchType-1].tTFixFieldCfg.pbCfg;
			tFixFields.wCfgLen = g_TSchFieldCfg[tTaskCfg.bSchType-1].tTFixFieldCfg.wCfgLen;

			//��ʼ���̶��ֶ�
			pbFixFmt = (BYTE*)g_TSchFieldCfg[tTaskCfg.bSchType-1].tTFixFieldCfg.pbFmt;
			wFixFmtLen = g_TSchFieldCfg[tTaskCfg.bSchType-1].tTFixFieldCfg.wFmtLen;
			if (!OoParseField(&tFixFields, pbFixFmt, wFixFmtLen, true))
				return 0;

			//�����ֶ�����
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

				pbArryROAD += 4; //�����ɼ�����
				if (*pbArryROAD++ == DT_ARRAY)
				{
					wNum = *pbArryROAD++;
					for (wIdx=0; wIdx<wNum; wIdx++)
					{
						iCfgLen = OoGetDataTypeLen(pbArryROAD);
						iLen = OoGetDataLen(DT_ROAD, pbArryROAD+1);
						//�����ṹ�ľ�����������
						tDataFields.pbCfg = pbArryROAD;
						tDataFields.wCfgLen = iCfgLen;
						//�������ֶν����Ľ��
						tDataFields.wNum = 1;
						tDataFields.wPos[0] = 0;
						tDataFields.wLen[0] = iLen;
						tDataFields.bType[0] = DT_ROAD;
						//���õ��ֶν�һ�������ɶ�Ӧ���������Ϣ
						tDataFields.wTotalLen = iLen;
						tDataFields.wItemOffset[0] = 0;
						tDataFields.wItemLen[0] = iLen;

						//RCSD�и�������
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

				//RCSD�и�������
				wMchNum = TableMatch(pbRCSD, wRcsdIdx, tFixFields.pbCfg, NULL, NULL, tDataFields.pbCfg, NULL, NULL);
				if (wMchNum > wMaxMchNum)
				{
					wMaxMchNum = wMchNum;
					fSchTab = true;
					*piTabIdx = wIdx;

					//��ʼ�������ֶ�
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
	else if (dwOAD == 0x601A0200)	//͸���ɼ�����
	{

	}
	else if (dwOAD>=0x50000200 && dwOAD<=0x50110200)	//����, ÿ�θ���RCSDƫ��ֵwRcsdIdx ƥ��һ�ű�
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
			pbRSD2++; // ��������
			pbRSD2+=4; // ����OAD
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
		
		dwROAD = OoOadToDWord(pbRCSD+2);	//����RCSDƫ��ֵwRcsdIdxȷ������
		if (!GetFrzTaskFileName((WORD )(dwOAD>>16), dwROAD, pszTableName))
			return -1;

		if (pszTableName == NULL)
			return -1;

		if (GetFrzTaskRecFieldParser(dwROAD, pFixFields, pDataFields, pDataFields->pbCfg, pDataFields->wCfgLen) == false)
			return -1;

		//wMaxMchNum = *pbRCSD;
		wMaxMchNum = 1;	//1��ֻƥ��1�ű�
	}
	else if (dwOAD>=0x30000000 && dwOAD<=0x32030000)	//�¼�
	{
		if(GetTermEvtCtrl((WORD )(dwOAD>>16)) != NULL)	//TermEvtTask.h/cpp��֧�ֵ������¼����������������¼�
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
			if (wMaxMchNum == 0)//����һ��OADʱ��RCSD=0����SEQUENCE OF�����������Ϊ0����ʾ����ѡ�񣨼�ȫѡ������
			{
				int i, index;
				WORD wLen;
				BYTE *pbtr = pbRCSD;
				wMaxMchNum = pFixFields->wNum + pDataFields->wNum; //�¼����й̶��ֶ�
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

//����ƥ���RCSD����
int TableMatch(BYTE* pbRCSD, WORD wRcsdIdx, BYTE* pbFixCfg, BYTE* pbFixFmt, WORD wFixFmtLen, BYTE* pbDataCfg, BYTE* pbDataFmt, WORD wDataFmtLen)
{
	int iRcsdLen;
	WORD wMchNum;
	
	wMchNum = 0;
	iRcsdLen = ScanRCSD(pbRCSD, false);
	while (iRcsdLen>=0 && wRcsdIdx<iRcsdLen)
	{
		if (!RcsdMatch(pbRCSD, wRcsdIdx, pbFixCfg, pbFixFmt, wFixFmtLen)) //�̶��ֶ�δƥ�䣬�������ֶ���ƥ��
		{
			if (!RcsdMatch(pbRCSD, wRcsdIdx, pbDataCfg, pbDataFmt, wDataFmtLen)) //�����ֶ�δƥ��,ֱ�ӽ�����ǰ����
				break;
		}
		wMchNum++;
		wRcsdIdx += ScanCSD(pbRCSD+wRcsdIdx, false); 
	}

	return wMchNum;
}

//����:ƥ��RCSD��ÿ��ֻ�Ƚ�RCSD�е�һ��CSD
//����:@pbRCSD
//	   @wRcsdIdex 
//����:�����ȷ�򷵻�1,���򷵻�0
int RcsdMatch(BYTE* pbRCSD, WORD wRcsdIdx, BYTE* pbCfg, BYTE* pbFmt, WORD wFmtLen)
{
	if (FieldCmp(DT_CSD, pbRCSD+wRcsdIdx, pbCfg[0], pbCfg+1) >= 0)	//+1:ȥ��array��ʽ
		return 1;
	return 0;
}

typedef struct{
	WORD  wItemOffset;	//�������ڼ�¼�е�ƫ�ƣ�
	WORD  wItemLen;	//�������ڼ�¼�еĳ��ȣ�
	BYTE* pbFmt;  		//��ʽ������
	WORD wFmtLen; 		//��ʽ����������
}TFieldInfo;



//�������Ӷ�˳��Ƚϣ�ֻ��CSD���ͱȽϣ�������ʱ��������
//������@pbSrc Դ�ֶ�����
//		@pbCmp �Ƚ��ֶ�����
//���أ�-1����0�ֶ�˳����ͬ��1�ֶ�˳��һ��
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


//����:��һ�ʼ�¼�ж�ȡһ���ֶ�
//������@ pbRec	��¼������
//	   @ wOffset	pFieldParser����ʶ���ֶ����ӦpbRec��ʼ��ƫ��
//	   @ pFieldParser	�ֱ���̶��ֶλ������ֶ�
//	   @ pbCSDЭ����RCSD�ĵ���CSD������CSD��ʽ�ֽ�[Choice=0--OAD, Choice=1---OAD--LkOAD]
//	   @ pbBuf�������ؼ�¼������
//����:�����ȷ�򷵻����ݵĳ���,���򷵻ظ���
int ReadRecField(BYTE* pbRec, WORD wOffset, TFieldParser* pParser, BYTE* pbOAD, BYTE* pbCSD, BYTE* pbBuf)
{
	int iRet, iNum, iLen;
	WORD wItemOffset, wItemLen;
	BYTE bType, bSrcField[128];
	BYTE *pbBuf0 = pbBuf;
	TFieldInfo tInfo;

	//�ֶ�����ȡ����
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

			iRet = FieldCmp(DT_CSD, pbCSD, bType, bSrcField+1);	//+1:ȥ����ʽ
			if (iRet == 0)
			{
				if ((iNum=OoScanRcsdInOadNum(pbCSD)) > 0)	//Э��������ROAD���ݣ��ڷ���ʱ�����ROAD�й���OAD������
				{
					*pbBuf++ = 0x01;
					*pbBuf++ = iNum;
				}
				//���ò�if������Դ�ֶ��볭���ֶ�CSD��ͬ������Ҫ�����ǱȽ�����CSD�ڲ��Ĺ���OAD˳���Ƿ�һ��
				if (FieldOrderCmpare(pbCSD, bSrcField+1) > 0)	//Դ�ֶ���Ƚ��ֶ�˳��һ��
				{
					BYTE bTmpBuf[1024];
					BYTE bTmpFmt[512];
					BYTE *pbTmpFmt;

					memset(bTmpBuf, 0, sizeof(bTmpBuf));

					ReadFromROAD_1(pbCSD, bSrcField, bTmpBuf, pbRec+(wOffset+tInfo.wItemOffset));	////wOffset+tInfo.wItemOffset �̶��ֶ�ƫ�� + N��CSD�����ֶ�ƫ��
					
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
				else	//Դ�ֶ���Ƚ��ֶ�˳��һ��
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
			else if (iRet == 1)	//ROAD����ȡ���ֹ���OAD
			{
				if ((iNum=OoScanRcsdInOadNum(pbCSD)) > 0)	//Э��������ROAD���ݣ��ڷ���ʱ�����ROAD�й���OAD������
				{
					*pbBuf++ = 0x01;
					*pbBuf++ = iNum;
				}

				if ((iRet=ReadFromROAD(pbCSD, bSrcField, pbBuf, pbRec+(wOffset+tInfo.wItemOffset))) > 0)	//wOffset+tInfo.wItemOffset �̶��ֶ�ƫ�� + N��CSD�����ֶ�ƫ��
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

	//�̶��ֶ�����
	memset((BYTE*)&tFixFields, 0, sizeof(TFieldParser));
	tFixFields.pbCfg = (BYTE*)g_TSchFieldCfg[SCH_TYPE_EVENT-1].tTFixFieldCfg.pbCfg;
	tFixFields.wCfgLen = g_TSchFieldCfg[SCH_TYPE_EVENT-1].tTFixFieldCfg.wCfgLen;

	//��ʼ���̶��ֶ�
	pbFixFmt = (BYTE*)g_TSchFieldCfg[SCH_TYPE_EVENT-1].tTFixFieldCfg.pbFmt;
	wFixFmtLen = g_TSchFieldCfg[SCH_TYPE_EVENT-1].tTFixFieldCfg.wFmtLen;
	if (!OoParseField(&tFixFields, pbFixFmt, wFixFmtLen, true))
		return 0;

	return tFixFields.wTotalLen;
}

//����: ��ȡȫ�¼��ɼ��������һ�ʼ�¼�ļ�¼���
//������@ bSchNo	�¼��ɼ�������
//	   @ bCSDIndex array ROAD����λ��
//����: �����ȷ�򷵻ش���,���򷵻�0
DWORD GetEvtTaskRecLastSerialNumber(BYTE bSchNo, BYTE bCSDIndex)
{
	char szTableName[32];
	BYTE bRecBuf[MTR_EXC_REC_LEN];
	DWORD dwLastRecIndex=0;
	memset(szTableName, 0, sizeof(szTableName));
	GetEvtTaskTableName(bSchNo, bCSDIndex, szTableName);

	memset(bRecBuf, 0, sizeof(bRecBuf));
	int iLen = ReadLastNRec(szTableName, LAST_REC, bRecBuf, sizeof(bRecBuf));	//������һ�ʼ�¼ȡ�����¼���¼���
	DWORD dwOff = GetEvtTaskTableFixFieldLen();
	if (iLen>0 && dwOff>0)
		dwLastRecIndex = OoDoubleLongUnsignedToDWord(bRecBuf+dwOff+3);	//�¼���¼��� ���ֽ���ǰ����

	return dwLastRecIndex;
}

//����: ��ȡȫ�¼��ɼ��������һ�ʼ�¼�ļ�¼���
//������@ wPn	���������
//	   @ dwOAD  �¼�OAD
//����: �����ȷ�򷵻ش���,���򷵻�0
DWORD GetEvtTaskRecLastSerialNumber(BYTE* pbTsa, BYTE bTsaLen, BYTE* pbCSD, BYTE bLenCSD)
{
	int iStart = -1;
	int iTabIdx = 0;
	WORD wRetNum = 0;
	BYTE bRSD[30] = {0}; //��¼ѡ��������
	BYTE bRCSD[128];  //��¼��ѡ��������                               
	BYTE *ptr = NULL;
	BYTE bOAD[4] = {0x60, 0x12, 0x03, 0x01};  //�������ñ� ��¼��Ԫ
	int nRet;
	BYTE bRecBuf[MTR_EXC_REC_LEN];
	DWORD dwLastRecIndex = 0;

	ptr = bRSD;
	*ptr++ = 10;       //���� RSD��ѡ�񷽷�10
	*ptr++ = 1;        //���� ��һ����¼
	*ptr++ = 3;        //���� ���ܱ���MS  һ���û���ַ [3]
	*ptr++ = 1;        //����ַ����=1
	*ptr++ = bTsaLen + 1;
	*ptr++ = bTsaLen - 1;
	memcpy(ptr, pbTsa+1, bTsaLen);   //����ַ
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
		dwLastRecIndex = OoDoubleLongUnsignedToDWord(bRecBuf+5);	//�¼���¼��� ���ֽ���ǰ����
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
	case 4:	//�ɼ�����ʱ��
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
	case 5:	//�ɼ��洢ʱ��
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
	case 6:	//�ɼ�����ʱ����ʼ������ֵ
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
	case 7:	//�ɼ��洢ʱ����ʼ������ֵ
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
	case 8:	//�ɼ��ɹ�ʱ����ʼ������ֵ
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
	case 9:	//�ϵ�N�μ�¼
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

//����RSD���ֶ��Ǽ�¼������ĸ��ֶ�
//�̶��ֶ������õ���OAD 4���ֽڣ�����OAD��Ӧ�����ݳ���
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
		if (pFixFields->bType[bIndex] == DT_OAD)	//�ú����Ƚϵ���OAD�������������RCSD\CSDȫ�����˵�
		{
			wPos = pFixFields->wPos[bIndex];
			pbCfg = pFixFields->pbCfg;
			dwOAD1 = OoOadToDWord(&pbCfg[wPos+1]);	//wPos + 1:�̶��ֶ��ڳ�ʼ��ʱ��ƫ��λ�ð��������ݸ�ʽ��ȥ��
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

//RSD���ֶ�ƥ�䷽��
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
			//��ʱ�䣬���ֽ�Ϊ�꣬���Դӵ��ֽڿ�ʼ�Ƚ�
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

//������ֻ�Ƚ�YYMMDDhhmm
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
		if (((wPn=GetMeterPn(pbFieldData+1, pbFieldData[0]))>0) && //+1��pbFieldData�Ǵ����ַ����
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
	if (IsAllAByte(pbStartTime, 0xff, 7))	//ȫ0xff��ʾʱ����Ч
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
	if (IsAllAByte(pbEndTime, 0xff, 7))	//ȫ0xff��ʾʱ����Ч
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

	if (fStartTimeAll0xEE || fEndTimeAll0xEE)	//ʱ����Чֱ�ӷ���
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

	if (bIntervU < TIME_UNIT_DAY)	//���������ݵ���ʼʱ���������һ���
	{
		//if (wIntervV == 0)	
		//	wIntervV = 1;

		if (wIntervV != 0)	//���ֵΪ0��ʾ�޼�� add CL 20170712
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
				dwStartSec += dwIntervSec;		//��������ǰ�������һ�����
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
				dwFieldSec = dwFieldSec / dwIntervSec * dwIntervSec;	//��������ǰ���
				SecondsToTime(dwFieldSec, &tFieldTime);
			}
		}
		else	//wIntervV==0
		{
			if (dwStartSec==dwEndSec && dwStartSec==dwFieldSec)		//��ֹ��ѯʱ����ͬ���㽭��վҪ���ܶ��ص�ǰʱ��������
			{
				DTRACE(DB_FAFRM, ("TiMatch() start time equal to end time, dwStartSec=%ld, bIntervU=%d.\n", dwStartSec, bIntervU));
				return true;
			}			
		}
	}

	while (dwStartSec < dwEndSec)
	{
		if (wIntervV == 0)	//ʱ��Ϊ0,ȡ��������ڵ�����
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

//MS���������
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




//���ն����ϱ�ʱ������ʱ�����1��ʱ���Ѽ�¼�е����ݸ�Ϊ��Ч������䣬�㽭��������
int FillInValueData(BYTE *pSrc, BYTE *pRcsd)
{
	TTime tNowTime, tRdTime;
	DWORD dwFrzOAD;
	BYTE bRcsdNum;
	BYTE bTypeLen;
	BYTE *pSechFrzOad;
	bool fExistFrzOad = false;
	BYTE *pbDataEnd = NULL, *pbDataStart=NULL;   //pSrc������


	
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
				*pSrc = 0;  //���0 
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
			pSechFrzOad += 4;	//��OAD
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



//�������ж����ݵĶ���ʱ���Ƿ���ȷ�����ﲻ���ô洢ʱ��0x60420200�жϣ�
//		���ʱ������ͨ�ɼ������Ĵ洢ʱ�����
//������@pSrc Դ����
//		@pRcsd Դ����pSrc��Ӧ��RCSD
//���أ�����0��ʾ���ݶ���ʱ��Ϸ�������0��ʾʱ�䲻�Ϸ���С��0��ʾpRcsd�ڲ������ڶ���ʱ�����ӦOAD
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
			pSechFrzOad += 4;	//��OAD
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

//����������698.45��645��Ӧ��ID����
//������@pbCSD �������698.45���������
//			ע�ⲻҪ��DT_CSD��ʽ�����ն���
//			�����й��ĸ�ʽΪ��01 //choice
//							  50 04 02 00	//������OAD 
//							  01	//��������OAD����
//							  00 10 02 00	//����OAD
//		@bMtrPro ���Э��
//		@pRespID ���ص�����ID�����������鷵��
//		@pbNum ���ص�ID����
//���أ��ɹ�����true������false
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
			if (*pb97++ == DT_ARRAY)	//����DI
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

			if (*pb97++ == DT_ARRAY)	//����DI
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

			if (*pb07++ == DT_ARRAY)	//����DI
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

			if (*pb07++ == DT_ARRAY)	//����DI
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

//��������ȡ�ɼ��������
//������@pbCSD �ɼ�����
//		@pszTabName ���زɼ��������
//		@wTabNameLen �ɼ������������
//		@pTAcqRuleInfo �ɼ�������Ϣ
//���أ��ɹ�����pbCSD�ֽڳ��ȣ�����-1
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

//��������ȡһ���������Ϣ
//������@pbPara  �������Ϣ
//		@pszTabName ������Ӧ�ı���
//		@wTabNameLen ������Ӧ�ı�������
//		@pTAcqRuleInfo �ɼ�������Ϣ
//���أ��ɹ�����һ�������ĳ��ȣ�����-1
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
			if (*pOneRule++ == DT_STRUCT)	//��������
			{
				BYTE bDiNum;
				pOneRule++;
				pTAcqRuleInfo->pbDlt07 = pOneRule;
				if (*pOneRule++ == DT_STRUCT)	//AcqCmd_2007
				{
					pOneRule++;
					if (*pOneRule++ == DT_ARRAY)	//����DI
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

					if (*pOneRule++ == DT_ARRAY)	//����DI
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
					if (*pOneRule++ == DT_ARRAY)	//����DI
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

					if (*pOneRule++ == DT_ARRAY)	//����DI
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

//����������������ȡ�ɼ�����
//������@pszTabName �ɼ��������
//		@pbCSD
//		@pbRespRule ���صĲɼ���������
//���أ��ɹ����زɼ����򳤶ȣ�����-1
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
		//��ʼ�����������ֶ�
		memset(&SchRule, 0, sizeof(SchRule));
		SchRule.wField = 0;	
		SchRule.wOpNum = 1;	
		SchRule.wOp[0] = TDB_OP_EQ;
		memcpy(&SchRule.bVal[0][0], pbOAD, 4);	//+1������choice

		//�������
		memset(&SchCtrl, 0, sizeof(SchCtrl));
		SchCtrl.wSortNum = 1;		//����������
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

//����������ɼ������ļ���
//������@pszSaveTabName ��Ҫ������ļ���
bool SaveAcqRuleTableName(char *pszSaveTabName)
{
	TAcqRuleTable tAcqRuleTable;
	WORD wIdx;
	char szRuleTableName[32];
	BYTE bMskIdx, bBit;
	
	memset((BYTE*)&tAcqRuleTable, 0, sizeof(tAcqRuleTable));
	memset(szRuleTableName, 0, sizeof(szRuleTableName));
	MK_ACQRULE_TABLE_NAME(szRuleTableName);
	//if (PartReadFile(szRuleTableName, 0, (BYTE*)&tAcqRuleTable, ACQRULE_FILE_HEAD_LEN))	//��Ӧ���ļ�������
	PartReadFile(szRuleTableName, 0, (BYTE*)&tAcqRuleTable, ACQRULE_FILE_HEAD_LEN);	//��Ӧ���ļ�������
	{
		for (bMskIdx=0; bMskIdx<sizeof(tAcqRuleTable.bMsk); bMskIdx++)
		{
			for (bBit=0; bBit<8; bBit++)
			{
				if (!(tAcqRuleTable.bMsk[bMskIdx] & (1<<bBit)))
				{
					wIdx = bMskIdx*8 + bBit;

					tAcqRuleTable.bMsk[bMskIdx] |= (1<<bBit);

					if (PartWriteFile(szRuleTableName, 0, (BYTE*)&tAcqRuleTable, ACQRULE_FILE_HEAD_LEN))//�������������
					{
						if (PartWriteFile(szRuleTableName, ACQRULE_FILE_MSG_OFFSET(wIdx), (BYTE*)pszSaveTabName,/* strlen(pszSaveTabName)*/ACQRULE_TABLE_NAME_LEN))	//���汨������
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

//������ɾ���ɼ������ļ���
//������@pszSaveTabName ��Ҫ������ļ���
bool DeleteAcqRuleTableName(char *pszDelTabName)
{
	TAcqRuleTable tAcqRuleTable;
	char szRuleTableName[32];

	memset((BYTE*)&tAcqRuleTable, 0, sizeof(tAcqRuleTable));
	memset(szRuleTableName, 0, sizeof(szRuleTableName));
	MK_ACQRULE_TABLE_NAME(szRuleTableName);
	if (PartReadFile(szRuleTableName, 0, (BYTE*)&tAcqRuleTable, ACQRULE_FILE_HEAD_LEN))	//��Ӧ���ļ�������
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
						if (!PartReadFile(szRuleTableName, ACQRULE_FILE_MSG_OFFSET(wIdx), (BYTE*)szDbTabName, ACQRULE_TABLE_NAME_LEN))	//��ȡ������ӳ��ı���
							goto ERR_RET;

						if (strcmp(pszDelTabName, szDbTabName) != 0)	//��������ͬ����������һ��
							continue;

						memset(bZeroBuf, 0, sizeof(bZeroBuf));
						tAcqRuleTable.bMsk[bMskIdx] &= ~(1<<bBit);
						if (PartWriteFile(szRuleTableName, 0, (BYTE*)&tAcqRuleTable, ACQRULE_FILE_HEAD_LEN))//�������������
						{
							if (PartWriteFile(szRuleTableName, ACQRULE_FILE_MSG_OFFSET(wIdx), (BYTE*)bZeroBuf, ACQRULE_TABLE_NAME_LEN))	//���������ӳ��ı���
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

//���������ļ����ȡ����������ÿ�η���һ�������������ж�piStart=-1ʱ������
//������@piStart �ò������޸ģ���ʼ��Ϊ-1
//		@pbRespTab ���ر���
//		@wMaxTabNameLen ����󳤶�
//���أ��ɹ�true������false
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
	if (PartReadFile(szRuleTableName, 0, (BYTE*)&tAcqRuleTable, ACQRULE_FILE_HEAD_LEN))	//��Ӧ���ļ�������
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

					if (PartReadFile(szRuleTableName, ACQRULE_FILE_MSG_OFFSET(wIdx), (BYTE*)pbRespTab, wMaxTabNameLen))	//���汨������
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
	pRespBuf++;	//��������

	do 
	{
		//1.��ȡ�ɼ��������
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
		//2. ���ݲɼ����������ȡ��Ӧ������
		iRet = GetAcqRuleFromTaskDB(szTabName, bROAD, pRespBuf);
		if (iRet < 0)
			continue;
		pRespBuf += iRet;
		bNum++;
	} while ((*piStart != -1) && ((wMaxLen-(pRespBuf-pRespBuf0)) > 200));	//<200��Ϊ���ÿռ䣬�Է��δ���


OK_RET:
	iRet = pRespBuf - pRespBuf0;
	pRespBuf = pRespBuf0;
	pRespBuf[1] = bNum;
	return iRet;
}
