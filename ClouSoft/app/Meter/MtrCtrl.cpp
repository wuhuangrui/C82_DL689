/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrCtrl.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�ֵ��ĳ������
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2016��8��
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
//MtrCtrl˽�к궨��


////////////////////////////////////////////////////////////////////////////////////////////
//MtrCtrl˽�����ݶ���
TMtrCacheCtrl g_MtrCacheCtrl[MTR_CACHE_NUM];
TSem g_semMtrCacheCtrl;		//������ƻ����ź���
TSem g_semMtrExc;			//�����¼��ź���

//TSem g_semMtrCtrl;		//��������̼߳���ź���

TSem g_semRdMtr[LOGIC_PORT_NUM];

bool g_fDirRd[LOGIC_PORT_NUM] = {false, false};	//ֱ����־
BYTE g_bDirRdStep = 0;	//1�����ݳ���״̬ 1�����ڳ���  0��û��

BYTE g_bMtrRdStatus[PN_MASK_SIZE];
BYTE g_bMtrRdStep[LOGIC_PORT_NUM];	//������������״̬�� 1���յ������������ 2�����ڳ��� 0���Ѿ�����
bool g_fStopMtrRd=false;
bool g_f485SchMtr = false;
DWORD g_dwLastStopMtrClick=0;
WORD g_wStopSec = 0;

TMtrPara g_MtrPara[LOGIC_PORT_NUM];
DWORD g_dwLastIntervSec[LOGIC_PORT_NUM];

BYTE g_bPnFailCnt[PN_NUM];
BYTE g_bPnFailFlg[PN_MASK_SIZE];

//�ж�ʱ���Ƿ�����ǰ������
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

//������ȡ��ֱ���Ŀ���Ȩ
void GetDirRdCtrl(BYTE bThrId)
{
	g_fDirRd[bThrId] = true;

	WaitSemaphore(g_semRdMtr[bThrId]);

	DTRACE(DB_METER, ("GetDirRdCtrl: Thread %d---\r\n", bThrId));
}

//�������ͷ�ֱ���Ŀ���Ȩ
void ReleaseDirRdCtrl(BYTE bThrId)
{
	g_fDirRd[bThrId] = false;
	DTRACE(DB_METER, ("ReleaseDirRdCtrl: Thread %d---\r\n", bThrId));

	SignalSemaphore(g_semRdMtr[bThrId]);
}

//������ȡ�ò��������ڵ��߳�
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

//������ȡ�ö˿����ڵ��߳�
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

//����:ȡ�õ�ǰ�ĳ���״̬
BYTE GetRdMtrState(BYTE bThrId)
{
	if (g_fDirRd[bThrId])	//ֱ����־
		return RD_ERR_DIR;		//����ֱ��
	if (g_fStopMtrRd)
		return RD_ERR_STOPRD;	//����ֱ��

	return RD_ERR_OK;
}

void UpdateMtrRdStep(BYTE bThrId)
{
	switch (g_bMtrRdStep[bThrId])
	{
	case 1:		//�յ�������������
		DTRACE(DB_METER, ("MtrRdThread: start to direct rd mtr.\r\n"));
		memset(g_bMtrRdStatus, 0, sizeof(g_bMtrRdStatus));	//����ɱ�־���
		g_bMtrRdStep[bThrId] = 2;
		break;
	case 2:		//����״̬
		DTRACE(DB_METER, ("MtrRdThread: finish direct rd mtr.\r\n"));			
		g_bMtrRdStep[bThrId] = 0;
		break;
	default:	//����״̬
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
			if (g_MtrPara[bThrId].bProId == PRO_TYPE_69845)	//������������ȥʱ���ǩ+�ϱ���ʶ
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

		//���ж��Ƿ���645Э��
		for (i=0; i<wTxLen; i++)
		{
			if (pTx[i]==0x68 && pTx[i+7]==0x68)
			{
				bMtrPro = 2;
				break;
			}
		}

		//��ȡ���ַ
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

		CommPara.wPort = MeterPortToPhy(CommPara.wPort); // ������߼��˿ڵ�����˿ڵ�ӳ��`
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

//������ȡ��ͨ���ĵ�ǰ���ȼ���С�ڵ��ڵ�ǰ���ȼ������񶼿���ִ��
//������@bCnͨ����
//����:ͨ���ĵ�ǰ���ȼ�
//��ע������ز�ͨ������Ҫ���ݹؼ����ݵĳ�ͨ�ʣ���̨ÿ���Ӹ���ͨ���ĵ�ǰ���ȼ�
//		����485ͨ�������û�����ȼ���������ֱ�ӷ������ȼ������ֵ
BYTE GetCurPrio(BYTE bCn)
{
	return MTR_PRIO_FOUR;
}

//����:��Բ������ѯ�Ƿ�����Ҫ������ID
//����:����ҵ�һ��δ����ID�򷵻�1,���ȫ�������򷵻�0,
//	�����ID��ʱ�仹û���򷵻�-1,�����ʱֹͣ�ò�����ĳ����򷵻�-2,
//	������������򷵻�-3
//��ע:��һ�γ���ʧ�ܵ������,�Ƿ���Ҫ������һ��ID,��SearchAnUnReadID()���㷨����
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
								if (bMtrMask[wPn/8] & (1<<(wPn%8)))	//�����������Ƿ�����ͨ������
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
													if (pbRSD!=NULL && pbRSD[3]!=3)	//����������
													{
														if (j == 0)
														{
															GetCurTime(&tmNow);
															OoTimeToDateTimeS(&tmNow, bBuf);
															UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIdx].bTaskId, TASK_MONIINDEX_STARTIME, bBuf, 7);
															bBuf[0] = 1; //ִ����
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
								if (bMtrMask[wPn/8] & (1<<(wPn%8)))	//�����������Ƿ����¼�������
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
														bBuf[0] = 1; //ִ����
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

		//��һ��������Ѱ����, ��ʼ��һ�����������
		bTaskIdx++;
		wItemIdx = 0;
	}

	//��������е������Ѿ�������һ��
	if (fIsCctFlg)
	{
		//���±������ַ��Ӧ�����з��������з�����Ӧ��ID�Ƿ��Ѿ�ִ����
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
	
	//������һ�֣��´����¿�ʼ����������ʱ��δ������ȫ������
	pMtrRdCtrl->schItem.bTaskIdx = 0;
	pMtrRdCtrl->schItem.wItemIdx = 0;
	pMtrRdCtrl->schItem.bLoopCnt = 0;

	return iRet;
}

//�������Զ���һ��������Ҫ���������һ��
//���أ�����������
BYTE AutoReadPn(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn, DWORD dwCurIntervSec, BYTE bThrId, bool* pfModified)
{
	int iRet;

	DoFixTask(pMtrRdCtrl, pMtrPro, wPn, pfModified);

	iRet = DoTask(wPn, pMtrRdCtrl, pMtrPro, pfModified);

	DoMtrExc(pMtrRdCtrl, pMtrPro, wPn, pfModified);		//ִ�г����¼�

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

//�����������̣߳�ִ�г���2�����ݶ��ᡢ�澯�жϵ�����
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
	bool fHaveRd;		//���в������Ƿ���������
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
	int iMonitorID = ReqThreadMonitorID(pszThrName, 4*60*60);	//�����̼߳��ID
	//InitThreadMaskId(iMonitorID);

	DTRACE(DB_METER, ("MtrRdThread: bThrId=%d start with bPort=%d\r\n", bThrId, bPort));
	while (1)
	{

		Sleep(10);	//��ֹCPU��ת
		
		UpdThreadRunClick(iMonitorID);

		MtrBroadcast_485( bThrId);//����ַУʱ
		BroadcastAdjustTime_485(bThrId);//�㲥Уʱ
		fHaveRd = false;	//���в������Ƿ���������
		const BYTE* pbPnMask = Get485PnMask();	//ȡ�÷��ز���485����λ.
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
			StopMtrRd(0xffff); //ֹͣ����
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

			wPn = SearchPnFromMask(pbPnMask, wPn);	//�����ѳ��Ĳ����㶼��485��
			if (wPn >= POINT_NUM)
			{
				Sleep(500);
				SignalSemaphore(g_semRdMtr[bThrId]);
				break;
			}

			if (g_fDirRd[bThrId] || g_fStopMtrRd || g_bMtrRdStep[bThrId]==1 || g_f485SchMtr)	//ֱ����־ �� ������������ �� �ѱ�
			{
				SignalSemaphore(g_semRdMtr[bThrId]);
				break;
			}

			if (GetPnPort(wPn) != bPort) //���Ǳ��˿ڵĵ��ܱ�
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
			{	//�����������л�
				g_dwLastIntervSec[bThrId] = dwCurIntervSec;
				memset(g_bMtrRdStatus, 0, sizeof(g_bMtrRdStatus));	//����ɱ�־���
				DTRACE(DB_METER, ("MtrRdThread: clr rd flg for interval or para change, cur pn=%d's\r\n", wPn));
			}

			if ((g_bMtrRdStatus[bPos] & bMask) && !g_bMtrRdStep[bThrId])	//�Ѿ�����
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

			//4������ÿ��������
			fNeedToSave = false;
#ifdef EN_SBJC_V2
			GetCurTime(&now);
			if(bOldHour != now.nHour)
			{
				CleanReadMeterFlag();
				bOldHour = now.nHour;
			}
#endif
			for (i=0; i<2; i++)	//һ����������೭��2��
			{
				fModified = false;
				bRdErr = AutoReadPn(pMtrRdCtrl, pMtrPro, wPn, dwCurIntervSec, bThrId, &fModified);

				if (fModified)
				{
					fNeedToSave = true;
					fHaveRd = true;		//���в����㷢��������
				}

				if (bRdErr != RD_ERR_UNFIN)		//û����
					break;
			}

			if (bRdErr==RD_ERR_DIR && fNeedToSave)		//����ֱ��
			{
				PutMtrRdCtrl(wPn, g_MtrPara[bThrId].bAddr, fModified);
				SignalSemaphore(g_semRdMtr[bThrId]);
				break;
			}

			if (bRdErr!=RD_ERR_PARACHG && fNeedToSave)
			{
				//�������ݴ洢
				SaveTask(pMtrRdCtrl);
			}

			if (bRdErr==RD_ERR_OK || !fNeedToSave)		//�޴�����ȫ����
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
            
            Sleep(10);  // ��ֹ�����ط���ȡ������
            
		}//while(1)

		if (g_fStopMtrRd)
		{
			if(bThrId == 1)
			{
#ifdef EN_SBJC_V2_CVTEXTPRO
				if (GetInfo(INFO_SYNC_T188PARA))
				{
					StopMtrRd(0xffff); //ֹͣ����
					DoSyncDocs();  //һ����ͬ��ˮ���ȱ���������ת����
				}
#endif
				g_wStopSec = 0;
			}
			if (GetClick()-g_dwLastStopMtrClick > g_wStopSec)  //��ͣ����30��� ���¿�ʼ����
				g_fStopMtrRd = false;
		}

		Sleep(100);
	}//while(1)
	
	ReleaseThreadMonitorID(iMonitorID);

	return THREAD_RET_OK;
}


//�ն˳�������¼�
void DoPortRdErr(bool fMtrFailHap)
{
    if (fMtrFailHap)   //��������ʧ��
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
