/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrCtrl.cpp
 * 摘    要：本文件主要实现电表的抄表控制
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2016年8月
 *********************************************************************************************************/
#include "stdafx.h"
#include "FaAPI.h"
#include "MeterAPI.h"
#include "MeterStruct.h"
#include "MtrCtrl.h"
#include "MtrHook.h"
#include "LibDbAPI.h"
#include "DbConst.h"
#include "ComAPI.h"
#include "MtrProAPI.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "DbOIAPI.h"
#include "TaskConst.h"
#include "TaskStruct.h"
#include "DbCctAPI.h"
#include "CctAPI.h"

#ifdef EN_SBJC_V2_CVTEXTPRO
#include "CvtExtPro.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////
//MtrCtrl私有宏定义


////////////////////////////////////////////////////////////////////////////////////////////
//MtrCtrl私有数据定义
TMtrCacheCtrl g_MtrCacheCtrl[MTR_CACHE_NUM];
TSem g_semMtrCacheCtrl;		//抄表控制缓存信号量
TSem g_semMtrExc;			//抄表事件信号量

//TSem g_semMtrCtrl;		//抄表控制线程间的信号量

TSem g_semRdMtr[LOGIC_PORT_NUM];

bool g_fDirRd[LOGIC_PORT_NUM] = {false, false};	//直抄标志
BYTE g_bDirRdStep = 0;	//1类数据抄表状态 1：正在抄，  0：没抄

BYTE g_bMtrRdStatus[PN_MASK_SIZE];
BYTE g_bMtrRdStep[LOGIC_PORT_NUM];	//立即抄表命令状态机 1：收到立即抄表命令， 2：正在抄表， 0：已经抄完
bool g_fStopMtrRd=false;
bool g_f485SchMtr = false;
DWORD g_dwLastStopMtrClick=0;
WORD g_wStopSec = 0;

TMtrPara g_MtrPara[LOGIC_PORT_NUM];
DWORD g_dwLastIntervSec[LOGIC_PORT_NUM];

BYTE g_bPnFailCnt[PN_NUM];
BYTE g_bPnFailFlg[PN_MASK_SIZE];

//判断时间是否跨过当前抄表间隔
DWORD GetCurIntervSec(WORD wPn, TTime* ptmNow)
{
	TTime tmNow = *ptmNow;
	DWORD dwMin = TimeToMinutes(tmNow);
	BYTE bInterv = GetMeterInterv();
    
    if (bInterv == 0)
        return 0;

	return dwMin / bInterv * bInterv * 60;
}

void StopMtrRd(WORD wStopSec)
{
	g_fStopMtrRd = true;
	g_dwLastStopMtrClick = GetClick();
	if (wStopSec == 0)
		wStopSec = 300;

	g_wStopSec = wStopSec;
}

//描述：取得直抄的控制权
void GetDirRdCtrl(BYTE bThrId)
{
	g_fDirRd[bThrId] = true;

	WaitSemaphore(g_semRdMtr[bThrId]);

	DTRACE(DB_METER, ("GetDirRdCtrl: Thread %d---\r\n", bThrId));
}

//描述：释放直抄的控制权
void ReleaseDirRdCtrl(BYTE bThrId)
{
	g_fDirRd[bThrId] = false;
	DTRACE(DB_METER, ("ReleaseDirRdCtrl: Thread %d---\r\n", bThrId));

	SignalSemaphore(g_semRdMtr[bThrId]);
}

//描述：取得测量点所在的线程
BYTE GetPnThread(WORD wPn)
{
	BYTE bPort = GetPnPort(wPn);

	if (bPort<LOGIC_PORT_MIN || bPort>LOGIC_PORT_MAX)
		return 0xff;

	if (bPort == LOGIC_PORT_MIN)
		return 0;
	else if (bPort == LOGIC_PORT_MAX)
		return 1;

	return 0xff;
}

//描述：取得端口所在的线程
BYTE GetPortThread(BYTE bPort)
{
	if (bPort<LOGIC_PORT_MIN || bPort>LOGIC_PORT_MAX)
		return 0xff;

	if (bPort == LOGIC_PORT_MIN)
		return 0;
	else if (bPort == LOGIC_PORT_MAX)
		return 1;

	return 0xff;
}

//描述:取得当前的抄表状态
BYTE GetRdMtrState(BYTE bThrId)
{
	if (g_fDirRd[bThrId])	//直抄标志
		return RD_ERR_DIR;		//正在直抄
	if (g_fStopMtrRd)
		return RD_ERR_STOPRD;	//正在直抄

	return RD_ERR_OK;
}

void UpdateMtrRdStep(BYTE bThrId)
{
	switch (g_bMtrRdStep[bThrId])
	{
	case 1:		//收到立即抄表命令
		DTRACE(DB_METER, ("MtrRdThread: start to direct rd mtr.\r\n"));
		memset(g_bMtrRdStatus, 0, sizeof(g_bMtrRdStatus));	//把完成标志清除
		g_bMtrRdStep[bThrId] = 2;
		break;
	case 2:		//抄表状态
		DTRACE(DB_METER, ("MtrRdThread: finish direct rd mtr.\r\n"));			
		g_bMtrRdStep[bThrId] = 0;
		break;
	default:	//空闲状态
		g_bMtrRdStep[bThrId] = 0;
		break;
	}
}

int DirAskMtrData(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bAddrLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData)
{
	char szTsa[TSA_LEN];
	int iRet = -1;
	BYTE bThrId = 0;
	TOobMtrInfo tMtrInfo;
	//bTsa[0] += 1;
	//bAddrLen -= 1;
	//WORD wPn = MtrAddrToPn(bTsa, bAddrLen);
	WORD wPn = GetMeterPn(bTsa, bAddrLen);

	if (wPn == 0)
	{
		memset(szTsa, 0, sizeof(szTsa));
		DTRACE(DB_FAPROTO, ("DirAskMtrData: Can`t support TSA=%s.\n", HexToStr(bTsa, (bAddrLen>(sizeof(szTsa))? sizeof(szTsa):bAddrLen), szTsa)));
		return -1;
	}

	BYTE bPort = GetPnPort(wPn);

	if (bPort==1 || bPort==2)
	{
		bThrId = GetPnThread(wPn);
		if (bThrId == 0xff)
			return -1;
		GetDirRdCtrl(bThrId);
		GetMeterPara(wPn, &g_MtrPara[bThrId]);
		TMtrPro* pMtrPro = CreateMtrPro(wPn, &g_MtrPara[bThrId], bThrId);
		if (pMtrPro == NULL)
		{
			DTRACE(DB_FAPROTO, ("DirAskMtrData: pMtrPro=NULL, Pro=%d.\n", g_MtrPara[bThrId].bProId));
			ReleaseDirRdCtrl(bThrId);
			return -1;
		}
		iRet = pMtrPro->pfnDirAskItem(pMtrPro, bType, bChoice, pApdu, wApduLen, pbData);
		if (iRet > 2)
		{
			if (g_MtrPara[bThrId].bProId == PRO_TYPE_69845)	//面向对象表抄读减去时间标签+上报标识
				iRet -= 2;	
		}
		ReleaseDirRdCtrl(bThrId);
	}
	else if (bPort == PORT_CCT_PLC)	//PORT_CCT
	{
		iRet = CctProxy(bType, bChoice, bTsa, bAddrLen, pApdu, wApduLen, wTimeOut, pbData);
	}

	return iRet;
}

int MtrDoFwd(TCommPara CommPara, BYTE* pTx, WORD wTxLen, BYTE* pbData, WORD wBufSize, WORD wFrmTimeOut, WORD wByteTimeOut)
{
	BYTE bThrId = 0;
	DWORD dwLen;
	WORD i;
	BYTE bTsa[TSA_LEN];
	BYTE bTsaLen;

	if (CommPara.wPort == PORT_CCT_PLC)
	{
		BYTE bMtrPro = 3;

		//先判断是否是645协议
		for (i=0; i<wTxLen; i++)
		{
			if (pTx[i]==0x68 && pTx[i+7]==0x68)
			{
				bMtrPro = 2;
				break;
			}
		}

		//提取表地址
		if (bMtrPro == 3)	//698.45
		{
			bTsaLen = pTx[4]+1;
			revcpy(bTsa, &pTx[5], bTsaLen);
		}
		else if (bMtrPro == 2)
		{
			bTsaLen = 6;
			revcpy(bTsa, &pTx[i+1], bTsaLen);
		}
		else
		{
			DTRACE(DB_FAPROTO, ("MtrDoFwd() Meter pro unspport!!!\n"));
		}

		return CctTransmit(bTsa, bTsaLen, pTx, wTxLen, wFrmTimeOut, pbData);
	}
	else 
	{
		bThrId = GetPortThread(CommPara.wPort);
		if (bThrId == 0xff)
			return -1;

		CommPara.wPort = MeterPortToPhy(CommPara.wPort); // 抄表的逻辑端口到物理端口的映射`
		GetDirRdCtrl(bThrId);
		if (!MtrProOpenComm(&g_commRs485[bThrId], &CommPara))
		{
			ReleaseDirRdCtrl(bThrId);
			return -1;
		}

		MtrProSend(&g_commRs485[bThrId], pTx, wTxLen);

		dwLen = MtrProRcv(&g_commRs485[bThrId], pbData, wBufSize);

		ReleaseDirRdCtrl(bThrId);

		return dwLen;
	}

	return -1;
}

//描述：取得通道的当前优先级，小于等于当前优先级的任务都可以执行
//参数：@bCn通道号
//返回:通道的当前优先级
//备注：针对载波通道，需要根据关键数据的抄通率，后台每分钟更新通道的当前优先级
//		对于485通道，如果没有优先级管理，可以直接返回优先级的最大值
BYTE GetCurPrio(BYTE bCn)
{
	return MTR_PRIO_FOUR;
}

//描述:针对测量点查询是否有需要抄读的ID
//返回:如果找到一个未读的ID则返回1,如果全部读完则返回0,
//	如果有ID但时间还没到则返回-1,如果暂时停止该测量点的抄读则返回-2,
//	如果其它错误则返回-3
//备注:在一次抄表失败的情况下,是否需要更换另一个ID,由SearchAnUnReadID()的算法决定
int SearchAnUnReadID(BYTE bCn, WORD wPn, TMtrRdCtrl* pMtrRdCtrl, TRdItem* pRdItem, bool fIsCctFlg)
{
	TTime tmNow;
	BYTE bBuf[7];
	int iSchCfgLen;
	TTaskCfg tTaskCfg;
	int iRet = RD_ERR_OK;
	BYTE bMtrMask[PN_MASK_SIZE];
	BYTE bPos, bMask, bPro, bType;
	BYTE *pbSch, *pFmt, *pbFieldFmt;
	BYTE *pbTaskCSD, *pbMs, *pbRSD=NULL, *pbIndex;
	WORD wLen, wFmtLen, wFieldLen, wNum;
	DWORD dwCurSec, dwStartSec, dwEndSec;

	bPro = GetCurPrio(bCn);

	BYTE bTaskIdx = pMtrRdCtrl->schItem.bTaskIdx;
	WORD wItemIdx = pMtrRdCtrl->schItem.wItemIdx;
	while (bTaskIdx < MTR_TASK_NUM)
	{
		if (pMtrRdCtrl->taskSucFlg[bTaskIdx].bValid == 1)
		{
			if (GetTaskCfg(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, &tTaskCfg) 
				&& (tTaskCfg.bState==1))
			{
				if (tTaskCfg.bPrio<=bPro
					&& GetTaskCurExeTime(&tTaskCfg, &dwCurSec, &dwStartSec, &dwEndSec)==0
					&& pMtrRdCtrl->taskSucFlg[bTaskIdx].bRecSaved!=TASK_DATA_FULL)
				{
					pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
					if (pbSch!=NULL)
					{
						switch (tTaskCfg.bSchType)
						{
						case SCH_TYPE_COMM:
							pFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
							memset(bMtrMask, 0, sizeof(bMtrMask));
							pbMs = OoGetField(pbSch, pFmt, wFmtLen, 4, &wLen, &bType, &pbFieldFmt, &wFieldLen);
							if (pbMs != NULL)
							{
								ParserMsParam(pbMs, bMtrMask, sizeof(bMtrMask));
								if (bMtrMask[wPn/8] & (1<<(wPn%8)))	//抄读测量点是否在普通方案中
								{
									if ((pbTaskCSD=OoGetField(pbSch, pFmt, wFmtLen, 3, &wLen, &bType, &pbFieldFmt, &wFieldLen)) != NULL)
									{
										wNum = pbTaskCSD[1];
										for (BYTE j=wItemIdx; j<wNum; j++)
										{
											pbIndex = OoGetField(pbTaskCSD, pbFieldFmt, wFieldLen, j, &wLen, &bType);
											if (pbIndex != NULL)
											{
												bPos = j/8;
												bMask = 1<<(j%8);
												if ((pMtrRdCtrl->taskSucFlg[bTaskIdx].bSucFlg[bPos]&bMask) == 0)
												{
													pRdItem->tiExe = tTaskCfg.tiExe;
													pMtrRdCtrl->schItem.bTaskIdx = bTaskIdx;
													pMtrRdCtrl->schItem.wItemIdx = j+1;
													memcpy(pRdItem->bCSD, pbIndex+1, wLen-1);
													if (*(pbIndex+1) == 0)
													{
														pRdItem->bReqType = 1;
														pRdItem->dwOAD = OoOadToDWord(pbIndex+2);
													}
													else
													{
														pRdItem->bReqType = 3;
														pRdItem->dwOAD = OoOadToDWord(pbIndex+2);
														pbRSD = OoGetField(pbSch, pFmt, wFmtLen, 2, &wLen, &bType);
														GetRSDAndRCSD(pRdItem, pbRSD[3], &pbRSD[5], pbIndex+1, pMtrRdCtrl->taskSucFlg[bTaskIdx].dwTime);
													}
													if (pbRSD!=NULL && pbRSD[3]!=3)	//非曲线数据
													{
														if (j == 0)
														{
															GetCurTime(&tmNow);
															OoTimeToDateTimeS(&tmNow, bBuf);
															UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, TASK_MONIINDEX_STARTIME, bBuf, 7);
															bBuf[0] = 1; //执行中
															UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, TASK_MONIINDEX_STAT, bBuf, 1);
														}
														UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, TASK_MONIINDEX_SENDNUM);
														pbMs = OoGetField(pbSch, pFmt, wFmtLen, 4, &wLen, &bType, &pbFieldFmt, &wFieldLen);
														UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, TASK_MONIINDEX_RDTOTAL, pbMs, PN_MASK_SIZE);
													}
													else
													{
														UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, TASK_MONIINDEX_SENDNUM);
													}

													return RD_ERR_UNFIN;
												}
											}
										}
									}
								}
							}
							break;

						case SCH_TYPE_EVENT:
							pFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
							memset(bMtrMask, 0, sizeof(bMtrMask));
							pbMs = OoGetField(pbSch, pFmt, wFmtLen, 2, &wLen, &bType, &pbFieldFmt, &wFieldLen);
							if (pbMs != NULL)
							{
								ParserMsParam(pbMs, bMtrMask, sizeof(bMtrMask));
								if (bMtrMask[wPn/8] & (1<<(wPn%8)))	//抄读测量点是否在事件方案中
								{
									if ((pbTaskCSD=OoGetField(pbSch, pFmt, wFmtLen, 1, &wLen, &bType, &pbFieldFmt, &wFieldLen)) != NULL)
									{
										pbTaskCSD += 4;
										wNum = pbTaskCSD[1];
										BYTE bEvtAcqFmtDesc[] = {
											1,	//array
											64,	
											DT_ROAD,	//ROAD
										};
										for (BYTE j=wItemIdx; j<wNum; j++)
										{
											pbIndex = OoGetField(pbTaskCSD, bEvtAcqFmtDesc, sizeof(bEvtAcqFmtDesc), j, &wLen, &bType);
											if (pbIndex != NULL)
											{
												bPos = j/8;
												bMask = 1<<(j%8);
												if ((pMtrRdCtrl->taskSucFlg[bTaskIdx].bSucFlg[bPos]&bMask) == 0)
												{
													pMtrRdCtrl->schItem.bTaskIdx = bTaskIdx;
													pMtrRdCtrl->schItem.wItemIdx = j+1;
													memcpy(pRdItem->bCSD, pbIndex+1, wLen-1);

													pRdItem->bReqType = 3;
													pRdItem->dwOAD = OoOadToDWord(pbIndex+1);
													pRdItem->tiExe = tTaskCfg.tiExe;
													pRdItem->dwEvtCnt = GetEvtTaskRecLastSerialNumber(pMtrRdCtrl->bTsa, pMtrRdCtrl->bTsa[0], pbIndex+1, wLen-1);
													pbRSD = OoGetField(pbSch, pFmt, wFmtLen, 2, &wLen, &bType);
													GetRSDAndRCSD(pRdItem, 6, (BYTE *)&tTaskCfg.tiExe, pbIndex);
													if (j == 0)
													{
														GetCurTime(&tmNow);
														OoTimeToDateTimeS(&tmNow, bBuf);
														UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, TASK_MONIINDEX_STARTIME, bBuf, 7);
														bBuf[0] = 1; //执行中
														UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, TASK_MONIINDEX_STAT, bBuf, 1);
													}
													UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, TASK_MONIINDEX_SENDNUM);
													pbMs = OoGetField(pbSch, pFmt, wFmtLen, 2, &wLen, &bType, &pbFieldFmt, &wFieldLen);
													UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, TASK_MONIINDEX_RDTOTAL, pbMs, PN_MASK_SIZE);
													return RD_ERR_UNFIN;
												}
											}
										}
									}
								}
							}
							break;
						}
					}
				}
				else if (pMtrRdCtrl->taskSucFlg[bTaskIdx].bRecSaved != TASK_DATA_FULL)
				{
					iRet = RD_ERR_UNTIME;
				}
			}
		}

		//上一个任务搜寻结束, 开始下一个任务的搜索
		bTaskIdx++;
		wItemIdx = 0;
	}

	//到这里，所有的任务都已经遍历了一遍
	if (fIsCctFlg)
	{
		//重新遍历表地址对应的所有方案，所有方案对应的ID是否都已经执行完
		bTaskIdx = 0;
		if (iRet == RD_ERR_OK)
		{
			while (bTaskIdx < MTR_TASK_NUM)
			{
				if (pMtrRdCtrl->taskSucFlg[bTaskIdx].bValid == 1)
				{
					if (GetTaskCfg(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, &tTaskCfg))
					{
						if (tTaskCfg.bPrio<=bPro
							&& GetTaskCurExeTime(&tTaskCfg, &dwCurSec, &dwStartSec, &dwEndSec)==0
								&& pMtrRdCtrl->taskSucFlg[bTaskIdx].bRecSaved!=TASK_DATA_FULL)
						{
							pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
							if (pbSch!=NULL)
							{
								switch (tTaskCfg.bSchType)
								{
								case SCH_TYPE_COMM:
									pFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
									if ((pbTaskCSD=OoGetField(pbSch, pFmt, wFmtLen, 3, &wLen, &bType, &pbFieldFmt, &wFieldLen)) != NULL)
										wNum = pbTaskCSD[1];
									break;
								case SCH_TYPE_EVENT:
									pFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
									if ((pbTaskCSD=OoGetField(pbSch, pFmt, wFmtLen, 1, &wLen, &bType, &pbFieldFmt, &wFieldLen)) != NULL)
										wNum = pbTaskCSD[1];
									break;
								default:
									continue;
								}

								WORD wSucNum = CalcuBitNum(pMtrRdCtrl->taskSucFlg[bTaskIdx].bSucFlg, TASK_SUC_FLG_LEN);
								if (wNum != wSucNum)
								{
									pMtrRdCtrl->schItem.bTaskIdx = bTaskIdx;
									pMtrRdCtrl->schItem.wItemIdx = 0;
									if (++pMtrRdCtrl->schItem.bLoopCnt >= LOOP_MAX_CNT)
									{
										pMtrRdCtrl->schItem.bLoopCnt = 0;
										return RD_ERR_OK;
									}
									else
										return RD_ERR_CHKTSK;
								}
							}
						}
					}
				}
				bTaskIdx++;
			}
		}
	}
	
	//已搜完一轮，下次重新开始搜索，返回时间未到或者全部抄完
	pMtrRdCtrl->schItem.bTaskIdx = 0;
	pMtrRdCtrl->schItem.wItemIdx = 0;
	pMtrRdCtrl->schItem.bLoopCnt = 0;

	return iRet;
}

//描述：自动把一个测量点要抄的数据项抄一轮
//返回：抄读错误定义
BYTE AutoReadPn(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn, DWORD dwCurIntervSec, BYTE bThrId, bool* pfModified)
{
	int iRet;

	DoFixTask(pMtrRdCtrl, pMtrPro, wPn, pfModified);

	iRet = DoTask(wPn, pMtrRdCtrl, pMtrPro, pfModified);

	DoMtrExc(pMtrRdCtrl, pMtrPro, wPn, pfModified);		//执行抄表事件

	return iRet;
}

void MtrCtrlInit()
{
	BYTE i;

	//g_semMtrCtrl = NewSemaphore(1);
	g_semMtrCacheCtrl = NewSemaphore(1);
	g_semMtrExc = NewSemaphore(1);

	for (i=0; i<LOGIC_PORT_NUM; i++)
	{
		g_semRdMtr[i] = NewSemaphore(1);
	}

	memset(&g_bMtrRdStep, 0, sizeof(g_bMtrRdStep));

	memset(g_bMtrRdStatus, 0, sizeof(g_bMtrRdStatus));
	memset(g_bPnFailCnt, 0, sizeof(g_bPnFailCnt));
	memset(g_bPnFailFlg, 0, sizeof(g_bPnFailFlg));
	memset(g_fMtrFailHapFlg, 0, sizeof(g_fMtrFailHapFlg));

	g_f485FailHapFlg = false;
	g_b485PortStatus = 0;
}

//描述：抄表线程，执行抄表，2类数据冻结、告警判断等任务
TThreadRet MtrRdThread(void* pvPara)
{
	TMtrPro* pMtrPro;
	TMtrRdCtrl* pMtrRdCtrl;
	DWORD dwCurIntervSec;
	WORD i, wPn;
	TTime now;
	BYTE bThrId, bPort, bInit[2];
	BYTE bRdErr, bPos, bMask;
	bool fMtrChg, fModified, fNeedToSave, fInfo;
	bool fHaveRd;		//所有测量点是否发生过抄读
#ifdef EN_SBJC_V2_CVTEXTPRO
	BYTE bOldDay;
	BYTE bOldHour;
#endif

	memset(bInit, 0, sizeof(bInit));
	GetCurTime(&now);
#ifdef EN_SBJC_V2_CVTEXTPRO
	bOldDay = 0;
	bOldHour = now.nHour;
#endif
	bThrId = (int ) pvPara;
	bPort = LOGIC_PORT_MIN + bThrId;

	char pszThrName[32] = {0};
	sprintf(pszThrName, "MtrRdThread-thrd-%d", bThrId);
	int iMonitorID = ReqThreadMonitorID(pszThrName, 4*60*60);	//申请线程监控ID
	//InitThreadMaskId(iMonitorID);

	DTRACE(DB_METER, ("MtrRdThread: bThrId=%d start with bPort=%d\r\n", bThrId, bPort));
	while (1)
	{

		Sleep(10);	//防止CPU空转
		
		UpdThreadRunClick(iMonitorID);

		MtrBroadcast_485( bThrId);//单地址校时
		BroadcastAdjustTime_485(bThrId);//广播校时
		fHaveRd = false;	//所有测量点是否发生过抄读
		const BYTE* pbPnMask = Get485PnMask();	//取得非载波的485屏蔽位.
		if (pbPnMask == NULL)
		{
			Sleep(500);
			continue;
		}

#ifdef EN_SBJC_V2_CVTEXTPRO
		GetCurTime(&now);
		if (bThrId==1 && now.nDay!=bOldDay && (bInit[bThrId]==0 || now.nHour>=23))
		{
			bOldDay = now.nDay;
			SetInfo(INFO_SYNC_T188PARA);
			StopMtrRd(0xffff); //停止抄表
		}
#endif
		wPn = 0;
		while (1)
		{
			UpdThreadRunClick(iMonitorID);

			WaitSemaphore(g_semRdMtr[bThrId]);
			if (!GetThreadRunFlag(bThrId))
			{
				SignalSemaphore(g_semRdMtr[bThrId]);
				Sleep(500);
				break;
			}

			wPn = SearchPnFromMask(pbPnMask, wPn);	//这里搜出的测量点都是485的
			if (wPn >= POINT_NUM)
			{
				Sleep(500);
				SignalSemaphore(g_semRdMtr[bThrId]);
				break;
			}

			if (g_fDirRd[bThrId] || g_fStopMtrRd || g_bMtrRdStep[bThrId]==1 || g_f485SchMtr)	//直抄标志 或 立即抄表命令 或 搜表
			{
				SignalSemaphore(g_semRdMtr[bThrId]);
				break;
			}

			if (GetPnPort(wPn) != bPort) //不是本端口的电能表
			{
				wPn++;
				SignalSemaphore(g_semRdMtr[bThrId]);
				continue;
			}

			bPos = wPn>>3;
			bMask = 1 << (wPn & 7);

			GetCurTime(&now);
			dwCurIntervSec = GetCurIntervSec(wPn, &now);
			if (dwCurIntervSec != g_dwLastIntervSec[bThrId])
			{	//抄表间隔发生切换
				g_dwLastIntervSec[bThrId] = dwCurIntervSec;
				memset(g_bMtrRdStatus, 0, sizeof(g_bMtrRdStatus));	//把完成标志清除
				DTRACE(DB_METER, ("MtrRdThread: clr rd flg for interval or para change, cur pn=%d's\r\n", wPn));
			}

			if ((g_bMtrRdStatus[bPos] & bMask) && !g_bMtrRdStep[bThrId])	//已经抄完
			{
				SignalSemaphore(g_semRdMtr[bThrId]);
				continue;
			}

			//DTRACE(DB_METER, ("MtrRdThread: start read wPn=%d!!!\n", wPn));
			GetMeterPara(wPn, &g_MtrPara[bThrId]);
			pMtrPro = CreateMtrPro(wPn, &g_MtrPara[bThrId], bThrId);
			if (pMtrPro == NULL)
			{
				SignalSemaphore(g_semRdMtr[bThrId]);
				continue;
			}

			pMtrRdCtrl = GetMtrRdCtrl(wPn, g_MtrPara[bThrId].bAddr);
			if (pMtrRdCtrl == NULL)
			{
				DTRACE(DB_METER, ("MtrRdThread: pMtrRdCtrl is NULL, wPn=%d!!!\n", wPn));
				SignalSemaphore(g_semRdMtr[bThrId]);
				continue;
			}

			//4、抄读每个数据项
			fNeedToSave = false;
#ifdef EN_SBJC_V2
			GetCurTime(&now);
			if(bOldHour != now.nHour)
			{
				CleanReadMeterFlag();
				bOldHour = now.nHour;
			}
#endif
			for (i=0; i<2; i++)	//一个测量点最多抄读2轮
			{
				fModified = false;
				bRdErr = AutoReadPn(pMtrRdCtrl, pMtrPro, wPn, dwCurIntervSec, bThrId, &fModified);

				if (fModified)
				{
					fNeedToSave = true;
					fHaveRd = true;		//所有测量点发生过抄读
				}

				if (bRdErr != RD_ERR_UNFIN)		//没抄完
					break;
			}

			if (bRdErr==RD_ERR_DIR && fNeedToSave)		//正在直抄
			{
				PutMtrRdCtrl(wPn, g_MtrPara[bThrId].bAddr, fModified);
				SignalSemaphore(g_semRdMtr[bThrId]);
				break;
			}

			if (bRdErr!=RD_ERR_PARACHG && fNeedToSave)
			{
				//任务数据存储
				SaveTask(pMtrRdCtrl);
			}

			if (bRdErr==RD_ERR_OK || !fNeedToSave)		//无错误，完全抄完
			{	
				//g_bMtrRdStatus[bPos] |= bMask;
				//DTRACE(DB_METER, ("MtrRdThread: mtr=%d rd ok!\r\n", wPn));
			}

			PutMtrRdCtrl(wPn, g_MtrPara[bThrId].bAddr, fModified);
			SignalSemaphore(g_semRdMtr[bThrId]);

			wPn++;
			if (wPn >= POINT_NUM)
			{
				Sleep(500);
			}
            
            Sleep(10);  // 防止其他地方获取不到锁
            
		}//while(1)

		if (g_fStopMtrRd)
		{
			if(bThrId == 1)
			{
#ifdef EN_SBJC_V2_CVTEXTPRO
				if (GetInfo(INFO_SYNC_T188PARA))
				{
					StopMtrRd(0xffff); //停止抄表
					DoSyncDocs();  //一次性同步水气热表档案到所有转换器
				}
#endif
				g_wStopSec = 0;
			}
			if (GetClick()-g_dwLastStopMtrClick > g_wStopSec)  //暂停抄表30秒后 重新开始抄表
				g_fStopMtrRd = false;
		}

		Sleep(100);
	}//while(1)
	
	ReleaseThreadMonitorID(iMonitorID);

	return THREAD_RET_OK;
}


//终端抄表故障事件
void DoPortRdErr(bool fMtrFailHap)
{
    if (fMtrFailHap)   //发生抄表失败
    {
        if (g_b485PortStatus == 0)
            g_b485PortStatus = 1;
    }
    else
    {
        if (g_b485PortStatus == 1)
            g_b485PortStatus = 0;
    }
    
    if (g_b485PortStatus == 1 && !g_f485FailHapFlg)     //IsAllPnRdFail()
	{
		SetInfo(INFO_DEVICE_485_ERR);
		g_f485FailHapFlg = true;
	}
	else if (g_b485PortStatus == 0)
	{
		g_b485PortStatus = 0;
		g_f485FailHapFlg = false;
	}
}
