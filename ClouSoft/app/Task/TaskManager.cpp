#include "stdafx.h"
#include "syscfg.h"
#include "FaCfg.h"

#include "TaskManager.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "sysarch.h"
#include "sysfs.h"
#include "TaskDB.h"
#include "MeterAPI.h"
#include "TaskConst.h"
#include "TaskStruct.h"
#include "DbOIAPI.h"
#include "CctAPI.h"
#include "Mem.h"
#include "MtrProAPI.h"
#include "MtrCtrl.h"

DWORD dwTaskLastUpdataTime[TASK_ID_NUM] = { 0 };	//上次任务的间隔执行时间

//判断任务是否需要执行
bool IsNeedExcTask(BYTE bTaskID)
{
	TTime tmNow;
	TTaskCfg tTaskCfg;
	WORD wCurMin, wStartMin, wEndMin;
	DWORD dwStartSecond, dwEndSecond, dwNow;

	memset((BYTE*)&tTaskCfg, 0, sizeof(tTaskCfg));
	if (GetTaskCfg(bTaskID, (TTaskCfg*)&tTaskCfg))	//任务配置单元
	{
		if (tTaskCfg.bState==1)
		{
			AddIntervs(tTaskCfg.tmStart, tTaskCfg.tiDelay.bUnit, tTaskCfg.tiDelay.wVal);
			dwStartSecond = TimeToSeconds(tTaskCfg.tmStart);
			dwEndSecond = TimeToSeconds(tTaskCfg.tmEnd);
			dwNow = GetCurTime();
			if (dwStartSecond<=dwNow && dwNow<dwEndSecond)
			{
				GetCurTime(&tmNow);
				wCurMin = tmNow.nHour*60 + tmNow.nMinute;
				for (BYTE i=0; i<tTaskCfg.bPeriodNum; i++)
				{
					wStartMin = tTaskCfg.period[i].bStarHour*60 + tTaskCfg.period[i].bStarMin;
					wEndMin = tTaskCfg.period[i].bEndHour*60 + tTaskCfg.period[i].bEndMin;
					switch(tTaskCfg.bPeriodType)
					{
					case 0:
						if (wStartMin<=wCurMin && wCurMin<wEndMin)
						{
							return true;
						}
						break;

					case 1:
						if (wStartMin<wCurMin && wCurMin<=wEndMin)
						{
							return true;
						}
						break;

					case 2:
						if (wStartMin<=wCurMin && wCurMin<=wEndMin)
						{
							return true;
						}
						break;

					case 3:
						if (wStartMin<wCurMin && wCurMin<wEndMin)
						{
							return true;
						}
						break;

					default:
						break;
					}
				}
			}
		}
	}

	return false;
}

//描述：组RSD和RCSD内容
//		@pbRSD 接收RSD内存
//		@wRSDLen 返回RSDLen
//		@pbRCSD 接收RCSD内存
//		@wRCSDLen 返回RCSDLen
//		@bType 采集方式
//		@pbData 采集方式内容
//		@pbCSD 组RCSD的内容
void GetRSDAndRCSD(TRdItem* pRdItem, BYTE bMethod, BYTE* pbData, BYTE* pbCSD, DWORD dwCurSec)
{
	TTime tmNow, tmTmp;
	DWORD dwOAD;
	BYTE *pbRSD = pRdItem->bRSD;
	BYTE *pbRCSD = pRdItem->bRCSD;
	TTimeInterv tiInterv;
	DWORD dwStartSecond, dwEndSecond, dwIntev;

	GetCurTime(&tmNow);		

	switch(bMethod)
	{
	case 0: //采集当前数据
		break;

	case 1:	//采集上第N次
		*pbRSD++ = 9;
		*pbRSD++ = *pbData;
		break;

	case 2:	//按冻结时标采集	对应冻结时标：0x2021
		*pbRSD++ = 1; //RSD 方法1
		 OoDWordToOad(0x20210200, pbRSD);	//OAD=0x20210200	记录选择描述符
		pbRSD += 4;
		*pbRSD++ = DT_DATE_TIME_S;

		dwOAD = OoOadToDWord(&pbCSD[1]);
		switch (dwOAD)
		{
		case 0x50000200:	//瞬时冻结
		case 0x50010200:	//秒冻结
			*pbRSD++ = tmNow.nYear/256;
			*pbRSD++ = tmNow.nYear%256;
			*pbRSD++ = tmNow.nMonth;
			*pbRSD++ = tmNow.nDay;
			*pbRSD++ = tmNow.nHour;
			*pbRSD++ = tmNow.nMinute;
			*pbRSD++ = tmNow.nSecond;
			break;

		case 0x50020200:	//分钟冻结
			*pbRSD++ = tmNow.nYear/256;
			*pbRSD++ = tmNow.nYear%256;
			*pbRSD++ = tmNow.nMonth;
			*pbRSD++ = tmNow.nDay;
			*pbRSD++ = tmNow.nHour;
			*pbRSD++ = tmNow.nMinute;
			*pbRSD++ = 0;
			break;

		case 0x50030200:	//小时冻结
			*pbRSD++ = tmNow.nYear/256;
			*pbRSD++ = tmNow.nYear%256;
			*pbRSD++ = tmNow.nMonth;
			*pbRSD++ = tmNow.nDay;
			*pbRSD++ = tmNow.nHour;
			*pbRSD++ = 0;
			*pbRSD++ = 0;
			break;

		case 0x50040200:	//日冻结
		case 0x50050200:	//结算日冻结
			*pbRSD++ = tmNow.nYear/256;
			*pbRSD++ = tmNow.nYear%256;
			*pbRSD++ = tmNow.nMonth;
			*pbRSD++ = tmNow.nDay;
			*pbRSD++ = 0;
			*pbRSD++ = 0;
			*pbRSD++ = 0;
			break;

		case 0x50060200:	//月冻结
			*pbRSD++ = tmNow.nYear/256;
			*pbRSD++ = tmNow.nYear%256;
			*pbRSD++ = tmNow.nMonth;
			*pbRSD++ = 1;
			*pbRSD++ = 0;
			*pbRSD++ = 0;
			*pbRSD++ = 0;
			break;

		case 0x50070200:	//年冻结
			*pbRSD++ = tmNow.nYear/256;
			*pbRSD++ = tmNow.nYear%256;
			*pbRSD++ = 1;
			*pbRSD++ = 1;
			*pbRSD++ = 0;
			*pbRSD++ = 0;
			*pbRSD++ = 0;
			break;
		}
		break;
	case 3:	//按时标间隔采集TI
		*pbRSD++ = 2; //RSD 方法2
		OoDWordToOad(0x20210200, pbRSD);	//OAD=0x20210200	记录选择描述符
		pbRSD += 4;
		tiInterv.bUnit = pbData[0];
		tiInterv.wVal = OoLongUnsignedToWord(pbData+1);
		dwIntev = TiToSecondes(&tiInterv);
		if (dwIntev == 0)
			dwIntev = 300;
		dwStartSecond = dwCurSec;
		dwEndSecond = dwStartSecond + dwIntev;
		SecondsToTime(dwStartSecond, &tmTmp);
		*pbRSD++ = DT_DATE_TIME_S;
		OoTimeToDateTimeS(&tmTmp, pbRSD); //起始值
		pbRSD += 7;
		SecondsToTime(dwEndSecond, &tmTmp);
		*pbRSD++ = DT_DATE_TIME_S;
		OoTimeToDateTimeS(&tmTmp, pbRSD); //结束值
		pbRSD += 7;
		
		*pbRSD++ = DT_TI;
		memcpy(pbRSD, pbData, 3); //TI
		pbRSD += 3;
		break;

	case 5:	//全事件采集方法
		*pbRSD++ = 2; //RSD 方法2
		OoDWordToOad(0x201E0200, pbRSD); //发生时间
		pbRSD += 4;
		memcpy(&tiInterv, pbData, sizeof(tiInterv));
		dwIntev = TiToSecondes(&tiInterv);
		if (dwIntev == 0)
			dwIntev = 300;
		dwStartSecond = (TimeToSeconds(tmNow)/dwIntev - 1)*dwIntev;
		dwEndSecond = dwStartSecond + dwIntev;
		SecondsToTime(dwStartSecond, &tmTmp);
		*pbRSD++ = DT_DATE_TIME_S;
		OoTimeToDateTimeS(&tmTmp, pbRSD); //起始值
		pbRSD += 7;
		*pbRSD++ = DT_DATE_TIME_S;
		SecondsToTime(dwEndSecond, &tmTmp);
		OoTimeToDateTimeS(&tmTmp, pbRSD); //结束值
		pbRSD += 7;

		//*pbRSD++ = NULL; //TI
		*pbRSD++ = 0xFF; //TI
		break;

	case 6:	//全事件采集方法
		*pbRSD++ = 9; //RSD 方法9
		//*pbRSD++ = DT_UNSIGN;
		*pbRSD++ = 1; //上1次事件数据
		break;
	}
	pRdItem->wRsdLen = (WORD)(pbRSD - pRdItem->bRSD);

	if (bMethod == 6) //全事件加上采集次数
	{
		*pbRCSD++ = pbCSD[5]+1;
		*pbRCSD++ = 0;
		OoDWordToOad(0x20220200, pbRCSD);
		pbRCSD += 4;
	}
	else
		*pbRCSD++ = pbCSD[5];

	for (BYTE i=0; i<pbCSD[5]; i++)
	{
		*pbRCSD++ = 0;
		memcpy(pbRCSD, &pbCSD[6+4*i], 4);
		pbRCSD += 4;
	}
	pRdItem->wRcsdLen = (WORD)(pbRCSD - pRdItem->bRCSD);
}

bool SaveTask(TMtrRdCtrl* pMtrRdCtrl)
{
	TTime tmNow;
	int iSchCfgLen;
	WORD wFmtLen, wLen;
	TTimeInterv tiExe;
	DWORD dwIntV, dwCurSec, dwStart, dwEnd, wSucNum;
	BYTE *pbSch, *pbFmt, *pbCollMode, bCollType;
	BYTE bBuf[MEMORY_BLOCK_SIZE], bType;
	TTaskCfg tTaskCfg;

	for (BYTE bTaskIndex=0; bTaskIndex<MTR_TASK_NUM; bTaskIndex++)
	{
		if (pMtrRdCtrl->taskSucFlg[bTaskIndex].bValid)
		{
			wSucNum = CalcuBitNum(pMtrRdCtrl->taskSucFlg[bTaskIndex].bSucFlg, TASK_SUC_FLG_LEN);
			if (wSucNum==0 || pMtrRdCtrl->taskSucFlg[bTaskIndex].bRecSaved==TASK_DATA_FULL) //任务还没有抄读成功 || 任务已经完整入库
				continue;

			if (pMtrRdCtrl->taskSucFlg[bTaskIndex].bRecSaved==TASK_DATA_PART && wSucNum<pMtrRdCtrl->taskSucFlg[bTaskIndex].bCSDItemNum) //部分入库 && 还没有全部抄到
				continue;

			GetTaskCfg(pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, &tTaskCfg);
			switch (tTaskCfg.bSchType)
			{
			case SCH_TYPE_COMM:
				bType = MEM_TYPE_TASK;
				break;

			case SCH_TYPE_EVENT:
				bType = MEM_TYPE_EVT_ACQ;
				break;

			default:
				bType = MEM_TYPE_NONE;
				break;
			}

			if (SaveTaskDataToDB(pMtrRdCtrl, bType, &(pMtrRdCtrl->taskSucFlg[bTaskIndex])))
			{
				if (wSucNum == pMtrRdCtrl->taskSucFlg[bTaskIndex].bCSDItemNum)
				{
					pMtrRdCtrl->taskSucFlg[bTaskIndex].bRecSaved = TASK_DATA_FULL;
					memset(pMtrRdCtrl->taskSucFlg[bTaskIndex].bSucFlg, 0, sizeof(pMtrRdCtrl->taskSucFlg[bTaskIndex].bSucFlg));
				}
				else
				{
					pMtrRdCtrl->taskSucFlg[bTaskIndex].bRecSaved = TASK_DATA_PART;
				}

				memset(bBuf, 0, sizeof(bBuf));
				int iLen = ReadMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, MEM_TYPE_CURVE_FLG, pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, bBuf);
				if (iLen > 0) //曲线任务某个时刻抄读标志特殊处理
				{
					pbFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
					pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
					if (pbSch != NULL)
					{
						pbCollMode = OoGetField(pbSch, pbFmt, wFmtLen, 2, &wLen, &bCollType);	//采集方式
						tiExe.bUnit = pbCollMode[5];
						tiExe.wVal = OoLongUnsignedToWord(pbCollMode+6);
						dwIntV = TiToSecondes(&tiExe);
						if (dwIntV == 0)
							dwIntV = 300;
						GetTaskCurExeTime(&tTaskCfg, &dwCurSec, &dwStart, &dwEnd);
						WORD wPos = (pMtrRdCtrl->taskSucFlg[bTaskIndex].dwTime - (dwCurSec - TiToSecondes(&(tTaskCfg.tiExe)))) / dwIntV;
						bBuf[wPos/8] |= (1<<(wPos%8));
						WriteMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, MEM_TYPE_CURVE_FLG, pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, bBuf);

						wPos = TiToSecondes(&(tTaskCfg.tiExe)) / dwIntV; //计算需要采集多少个曲线点
						if (CalcuBitNum(bBuf, iLen) == wPos) //全部采集成功
						{
							GetCurTime(&tmNow);
							OoTimeToDateTimeS(&tmNow, bBuf);
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, TASK_MONIINDEX_ENDTIME, bBuf, 7);
							bBuf[0] = 2; //已执行
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, TASK_MONIINDEX_STAT, bBuf, 1);
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, TASK_MONIINDEX_SUCNUM);
						}
					}
				}
				else if (iLen < 0) //非曲线任务状态更新
				{
					GetCurTime(&tmNow);
					OoTimeToDateTimeS(&tmNow, bBuf);
					UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, TASK_MONIINDEX_ENDTIME, bBuf, 7);
					bBuf[0] = 2; //已执行
					UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, TASK_MONIINDEX_STAT, bBuf, 1);
					UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, TASK_MONIINDEX_SUCNUM);
				}
			}
		}
	}

	return false;
}

bool SaveTaskDataToDB(TMtrRdCtrl* pMtrRdCtrl, BYTE bType, TTaskSucFlg* ptaskSucFlg, BYTE* pbData, WORD wDataLen, WORD wIdex)
{
	TTaskCfg tTaskCfg;
	TTime tNowTime, tTime;
	WORD wLen, wDataFmtLen;
	BYTE bRecBuf[1024];
	BYTE bFmtType;
	BYTE *pbSch;
	BYTE *pbRecBuf, *pbPtr, *pbDataFmt;
	int iRet, iSchCfgLen;
	bool fIsSaveFlg = false;

	if (!GetTaskCfg(ptaskSucFlg->bTaskId, &tTaskCfg))
		return false;

	pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
	if (pbSch==NULL)
		return false;

	switch (tTaskCfg.bSchType)
	{
	case SCH_TYPE_COMM:
	case SCH_TYPE_EVENT:
	case SCH_TYPE_REAL:
		//采集方案字段分配
		fIsSaveFlg = true;
		memset(bRecBuf, 0, sizeof(bRecBuf));
		pbRecBuf = bRecBuf;

		//目的服务器地址
		memcpy(pbRecBuf, pMtrRdCtrl->bTsa, pMtrRdCtrl->bTsa[0]+1);
		DWORD dwOAD;
		dwOAD = 0x202a0200;
		dwOAD = OoOadToDWord((BYTE*)&dwOAD);
		pbRecBuf += OoGetDataLen(81, (BYTE*)&dwOAD);

		//采集启动时标
		SecondsToTime(ptaskSucFlg->dwTime, &tTime);
		pbRecBuf[0] = tTime.nYear/256;
		pbRecBuf[1] = tTime.nYear%256;
		pbRecBuf[2] = tTime.nMonth;
		pbRecBuf[3] = tTime.nDay;
		pbRecBuf[4] = tTime.nHour;
		pbRecBuf[5] = tTime.nMinute;
		pbRecBuf[6] = tTime.nSecond;
		pbRecBuf += 7;

		if (tTaskCfg.bSchType == SCH_TYPE_COMM)
		{
			BYTE bCollType;
			//曲线数据的采集成功时标应该填充为上一日从零点起开始的间隔时间
			pbDataFmt = GetSchFmt(tTaskCfg.bSchType, &wDataFmtLen);
			pbPtr = OoGetField(pbSch, pbDataFmt, wDataFmtLen, 2, &wLen, &bFmtType);	//采集方式
			if (*pbPtr++ == DT_STRUCT)
			{
				pbPtr++;
				if (*pbPtr++ == DT_UNSIGN)
				{
					bCollType = *pbPtr++;
					if (bCollType == 0x03)	//采集类型为3，按时表间隔采集，直接用规整后的采集启动时标
					{
						//采集成功时标（4）
						pbRecBuf[0] = tTime.nYear/256;
						pbRecBuf[1] = tTime.nYear%256;
						pbRecBuf[2] = tTime.nMonth;
						pbRecBuf[3] = tTime.nDay;
						pbRecBuf[4] = tTime.nHour;
						pbRecBuf[5] = tTime.nMinute;
						pbRecBuf[6] = tTime.nSecond;
						pbRecBuf += 7;
					}
					else
					{
						//采集成功时标（4）
						GetCurTime(&tTime);	
						pbRecBuf[0] = tTime.nYear/256;
						pbRecBuf[1] = tTime.nYear%256;
						pbRecBuf[2] = tTime.nMonth;
						pbRecBuf[3] = tTime.nDay;
						pbRecBuf[4] = tTime.nHour;
						pbRecBuf[5] = tTime.nMinute;
						pbRecBuf[6] = tTime.nSecond;
						pbRecBuf += 7;
					}
				}
			}

			pbPtr = OoGetField(pbSch, pbDataFmt, wDataFmtLen, 5, &wLen, &bFmtType);	
			if (pbPtr == NULL)
				break;
			if (*pbPtr++ == DT_ENUM)
			{
				BYTE bStgType = *pbPtr++;
				switch (bStgType)
				{
				case 0:	//未定义
					GetCurTime(&tTime);
					break;
				case 1:	//任务开始时间
					SecondsToTime(ptaskSucFlg->dwTime, &tTime);
					break;
				case 2:	//相对当日0点0分
					GetCurTime(&tTime);
					tTime.nHour = 0;
					tTime.nMinute = 0;
					tTime.nSecond = 0;
					break;
				case 3:	//相对上日23点59分
					GetCurTime(&tTime);
					AddIntervs(tTime, TIME_UNIT_DAY, -1);
					tTime.nHour = 23;
					tTime.nMinute = 59;
					tTime.nSecond = 0;
					break;
				case 4:	//相对上日0点0分
					GetCurTime(&tTime);
					AddIntervs(tTime, TIME_UNIT_DAY, -1);
					tTime.nHour = 0;
					tTime.nMinute = 0;
					tTime.nSecond = 0;
					break;
				case 5:	//相对当月1日0点0分
					GetCurTime(&tTime);
					tTime.nDay = 1;
					tTime.nHour = 0;
					tTime.nMinute = 0;
					tTime.nSecond = 0;
					break;
				case 6:	//数据冻结时标 
					SecondsToTime(ptaskSucFlg->dwTime, &tTime);
//					int iRet;
//					DWORD dwFrzOAD;	//数据冻结时标
//					DWORD dwOAD;
//					WORD wDataOffset;
//					BYTE bArryNum;
//					BYTE bRsdNum;
//					BYTE *pTime;
//					BYTE bMemRec[512];
//
//					wDataOffset = 0;
//					dwFrzOAD = 0x20210200;
//					pbPtr = OoGetField(pbSch, pbDataFmt, wDataFmtLen, 3, &wLen, &bFmtType);	
//					if (pbPtr != NULL)
//					{
//						if (*pbPtr++ == DT_ARRAY)
//						{
//							bArryNum = *pbPtr++;
//							for (BYTE i=0; i<bArryNum; i++)
//							{				
//								if (*pbPtr++ == DT_CSD)
//								{
//									if (*pbPtr++ == 0)	//OAD
//									{
//										dwOAD = OoOadToDWord(pbPtr);
//										if (dwFrzOAD == dwOAD)
//											goto OK_FRZOAD;	
//										iRet = OoGetDataLen(DT_OAD, pbPtr);
//										if (iRet < 0)
//											goto ERR_FRZOAD;
//										wDataOffset += iRet;
//										pbPtr += 4;
//									}
//									else	//ROAD
//									{
//										dwOAD = OoOadToDWord(pbPtr);
//										if ((dwOAD&0xFF000000) == 0x50000000)	//表示冻结类对象
//										{
//											pbPtr += 4;
//											bRsdNum = *pbPtr++;
//											for (BYTE j=0; j<bRsdNum; j++)
//											{
//												dwOAD = OoOadToDWord(pbPtr);
//												if (dwFrzOAD == dwOAD)
//													goto OK_FRZOAD;
//												iRet = OoGetDataLen(DT_OAD, pbPtr);
//												if (iRet < 0)
//													goto ERR_FRZOAD;
//												wDataOffset += iRet;
//												pbPtr += 4;
//											}
//										}
//									}
//								}
//							}
//						}
//					}
//ERR_FRZOAD:
//					DTRACE(DB_CRITICAL, ("SaveTaskDataToDB(): Can`t find dwFrzOAD=0x%08x.\n", dwFrzOAD));
//					return false;
//OK_FRZOAD:
//					memset(bMemRec, 0, sizeof(bMemRec));
//					iRet = ReadTmpRec(pMtrRdCtrl, bType, ptaskSucFlg->bTaskId, bMemRec);
//					if (iRet < 0)
//					{
//						DTRACE(DB_CRITICAL, ("SaveTaskDataToDB(): iRet=%d.\n", iRet));
//						goto ERR_FRZOAD;
//					}
//					pTime = bMemRec+wDataOffset;
//					if (*pTime == DT_DATE_TIME_S)
//					{
//						OoDateTimeSToTime(pTime+1, &tTime);
//						tTime.nSecond = 0;	//秒强制为0
//						break;
//					}
//					else
//					{
//						TraceBuf(DB_CRITICAL, "SaveTaskDataToDB(): err:", bMemRec, iRet);
//						TraceBuf(DB_CRITICAL, "SaveTaskDataToDB(): err:", pTime, 8);
//						return false;
//					}
					break;
				case 7:	//相对上月月末0点0分
					GetCurTime(&tTime);
					AddIntervs(tTime, TIME_UNIT_MONTH, -1);

					if (tTime.nMonth==1 || tTime.nMonth==3 || tTime.nMonth==5 || tTime.nMonth==7 || tTime.nMonth==8 || tTime.nMonth==10 || tTime.nMonth==12)
					{
						tTime.nDay = 31;
					}
					else if (tTime.nMonth==4 || tTime.nMonth==6 || tTime.nMonth==9 || tTime.nMonth==11)
					{
						tTime.nDay = 30;
					}
					else	//month=2
					{
						if ((tTime.nYear%4==0 && tTime.nYear%100!=0) || tTime.nYear%400==0)   //闰年
							tTime.nDay = 29;
						else
							tTime.nDay = 28;
					}
					tTime.nHour = 0;
					tTime.nMinute = 0;
					tTime.nSecond = 0;
					break;
				default:
					DTRACE(DB_CRITICAL, ("SaveTaskDataToDB(): Nonsupport bStgType=%d.\n", bStgType));
					return false;
				}
			}
			
			//采集存储时标
			pbRecBuf[0] = tTime.nYear/256;
			pbRecBuf[1] = tTime.nYear%256;
			pbRecBuf[2] = tTime.nMonth;
			pbRecBuf[3] = tTime.nDay;
			pbRecBuf[4] = tTime.nHour;
			pbRecBuf[5] = tTime.nMinute;
			pbRecBuf[6] = tTime.nSecond;
			pbRecBuf += 7;
		}
		else
		{
			//采集成功时标（4）
			GetCurTime(&tTime);	
			pbRecBuf[0] = tTime.nYear/256;
			pbRecBuf[1] = tTime.nYear%256;
			pbRecBuf[2] = tTime.nMonth;
			pbRecBuf[3] = tTime.nDay;
			pbRecBuf[4] = tTime.nHour;
			pbRecBuf[5] = tTime.nMinute;
			pbRecBuf[6] = tTime.nSecond;
			pbRecBuf += 7;

			//采集存储时标
			GetCurTime(&tTime);
			pbRecBuf[0] = tTime.nYear/256;
			pbRecBuf[1] = tTime.nYear%256;
			pbRecBuf[2] = tTime.nMonth;
			pbRecBuf[3] = tTime.nDay;
			pbRecBuf[4] = tTime.nHour;
			pbRecBuf[5] = tTime.nMinute;
			pbRecBuf[6] = tTime.nSecond;
			pbRecBuf += 7;
		}


		memcpy(pbRecBuf, pMtrRdCtrl->bTsa, pMtrRdCtrl->bTsa[0]+1);
		dwOAD = 0x40010200;
		dwOAD = OoOadToDWord((BYTE*)&dwOAD);
		pbRecBuf += OoGetDataLen(81, (BYTE*)&dwOAD);

		if (tTaskCfg.bSchType == SCH_TYPE_COMM)
		{
			iRet = ReadTmpRec(pMtrRdCtrl, bType, ptaskSucFlg->bTaskId, pbRecBuf);
		}
		else
		{
			iRet = wDataLen;
			if (pbData != NULL)
				memcpy(pbRecBuf, pbData, wDataLen);
			else
				return true; //不用实际保存
		}
		pbRecBuf += iRet;
		break;
// 	case SCH_TYPE_TRANS:	不在这里执行
// 		break;
	case SCH_TYPE_REPORT:
		break;
	case SCH_TYPE_SCRIPT:
		break;
	}
	if (fIsSaveFlg)
	{
		return WriteCacheDataToTaskDB(tTaskCfg.bSchNo, tTaskCfg.bSchType, bRecBuf, pbRecBuf-bRecBuf, wIdex, &(ptaskSucFlg->iRecPhyIdx));
	}

	return false;
}

bool ClrTaskMoniStat(BYTE bTaskId)
{
	BYTE g_bTaskMoniUnit[] = {DT_STRUCT, 0x08,
		DT_UNSIGN, 0x00,//任务ID
		DT_ENUM, 0x00,	//任务执行状态
		DT_DATE_TIME_S, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	//任务执行开始时间 
		DT_DATE_TIME_S,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	//任务执行结束时间 
		DT_LONG_U, 0x00, 0x00,	//采集总数量
		DT_LONG_U, 0x00, 0x00,	//采集成功数量 
		DT_LONG_U, 0x00, 0x00,	//已发送报文条数
		DT_LONG_U, 0x00, 0x00,	//已接收报文条数 
	};

	g_bTaskMoniUnit[3] = bTaskId;

	return WriteItemEx(BN0, bTaskId, 0x6034, g_bTaskMoniUnit);
}

bool UpdateTaskMoniStat(BYTE bTaskId, BYTE bIndex, void* pbData, WORD wDataLen)
{
	WORD wLen;
	WORD wVal;
	BYTE* pbSrc;
	BYTE bType, bBuf[64];
	const ToaMap* pItem;
	DWORD dwOAD = 0x60340200;

	int iRet = ReadItemEx(BN0, bTaskId, 0x6034, bBuf);
	if (iRet>0 && IsAllAByte(bBuf, 0, iRet))
	{	
		ClrTaskMoniStat(bTaskId);
		ReadItemEx(BN0, bTaskId, 0x6034, bBuf);
	}

	pItem = GetOIMap(dwOAD);
	pbSrc = OoGetField(bBuf, pItem->pFmt, pItem->wFmtLen, bIndex, &wLen, &bType);
	if (*pbSrc == DT_LONG_U)
	{
		if (bIndex == TASK_MONIINDEX_RDTOTAL)
		{
			wVal = MsToMtrNum((BYTE*)pbData);
			OoWordToLongUnsigned(wVal, pbSrc+1);
		}
		else
		{
			wVal = OoLongUnsignedToWord(pbSrc+1);
			if (pbData != NULL)
				wVal += ByteToWord((BYTE* )pbData);
			else
				wVal++;
			OoWordToLongUnsigned(wVal, pbSrc+1);
		}
	}
	else 
	{
		if (pbData != NULL)
			memcpy(pbSrc+1, pbData, wDataLen);
	}

	return WriteItemEx(BN0, bTaskId, 0x6034, bBuf);
}

bool DoTaskSwitch(TMtrRdCtrl* pMtrRdCtrl)
{
	bool fSwitch = false;
	TTime tmNow, tTime;
	int iLen, iSchLen;
	BYTE i, j, k, bType;
	WORD wFmtLen, wLen;
	TTimeInterv tiExe;
	TTaskCfg tTaskCfg;
	BYTE *pbSch=NULL, *pbMs=NULL, *pbFmt=NULL, *pbCollMode=NULL;
	BYTE bBuf[MEMORY_BLOCK_SIZE], bStatBuf[7];
	DWORD dwCurSec, dwStartSec, dwEndSec, dwInterV, dwCurSecbak;

	//任务方案数据采集
	for (i=0; i<MTR_TASK_NUM; i++)
	{
		if (pMtrRdCtrl->taskSucFlg[i].bValid == 1)
		{
			if (GetTaskCfg(pMtrRdCtrl->taskSucFlg[i].bTaskId, &tTaskCfg))
			{
				if (GetTaskCurExeTime(&tTaskCfg, &dwCurSec, &dwStartSec, &dwEndSec) != 0)
				{
					continue;
				}

				memset(bBuf, 0, sizeof(bBuf));
				iLen = ReadMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, MEM_TYPE_CURVE_FLG, pMtrRdCtrl->taskSucFlg[i].bTaskId, bBuf);
				if (iLen > 0) //曲线任务时标切换特殊处理
				{
					dwInterV = 0xffff;
					for (j=0; j<iLen; j++)
					{
						for (k=0; k<8; k++)
						{
							if ((bBuf[j]&(1<<k)) == 0)
							{
								dwInterV = j*8 + k;
								break;
							}
						}
						if (k < 8)
							break;
					}

					pbFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
					pbSch = GetSchCfg(&tTaskCfg, &iSchLen);
					if (pbSch != NULL)
					{
						pbCollMode = OoGetField(pbSch, pbFmt, wFmtLen, 2, &wLen, &bType);	//采集方式
						bType = pbCollMode[3];
						tiExe.bUnit = pbCollMode[5];
						tiExe.wVal = OoLongUnsignedToWord(pbCollMode+6);
						dwCurSecbak = dwCurSec;
						if (dwInterV != 0xffff)
						{
							if (bType == 3 /*&& tiExe.bUnit==1*/) //按TI间隔采集
							{
								dwCurSec -= TiToSecondes(&(tTaskCfg.tiExe));
								dwCurSec += dwInterV*TiToSecondes(&tiExe);
							}
						}
						else
						{
							if (bType == 3 /*&& tiExe.bUnit==1*/) //按TI间隔采集
							{
								dwCurSec -= TiToSecondes(&(tTaskCfg.tiExe));
								dwCurSec += (iLen*8-1)*TiToSecondes(&tiExe); //固定在最后一个采集间隔上
							}
						}
						if (dwCurSec >= dwCurSecbak)	//本间隔已执行完
						{
							if (pMtrRdCtrl->taskSucFlg[i].dwTime>=(dwStartSec-TiToSecondes(&tiExe)) && pMtrRdCtrl->taskSucFlg[i].dwTime<=(dwEndSec-TiToSecondes(&tiExe)))	//本间隔未更新
							{
								dwCurSec = pMtrRdCtrl->taskSucFlg[i].dwTime;	//防止间隔内数据重抄
							}
							dwInterV = 0xffff;	
						}
					}
				}

				if (pMtrRdCtrl->taskSucFlg[i].dwTime == 0)
				{				
					pMtrRdCtrl->taskSucFlg[i].dwTime = dwCurSec;
					pbFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
					pbSch = GetSchCfg(&tTaskCfg, &iSchLen);
					if (pbSch != NULL)
					{
						if (dwTaskLastUpdataTime[pMtrRdCtrl->taskSucFlg[i].bTaskId] != dwCurSec)
						{
							dwTaskLastUpdataTime[pMtrRdCtrl->taskSucFlg[i].bTaskId] = dwCurSec;
							ClrTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId);						
							SecondsToTime(dwStartSec, &tTime);
							OoTimeToDateTimeS(&tTime, bStatBuf);
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_STARTIME, bStatBuf, 7);
							bStatBuf[0] = 1; //执行中
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_STAT, bStatBuf, 1);
							if (tTaskCfg.bSchType == SCH_TYPE_COMM)
								pbMs = OoGetField(pbSch, pbFmt, wFmtLen, 4, &wLen, &bType);
							else
								pbMs = OoGetField(pbSch, pbFmt, wFmtLen, 2, &wLen, &bType);
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_RDTOTAL, pbMs, PN_MASK_SIZE);
						}
					}
				}

				if (dwCurSec != pMtrRdCtrl->taskSucFlg[i].dwTime) //间隔切换
				{
					pbFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
					pbSch = GetSchCfg(&tTaskCfg, &iSchLen);
					if (pbSch != NULL)
					{
						if (iLen<0 && pMtrRdCtrl->taskSucFlg[i].bRecSaved==TASK_DATA_PART && tTaskCfg.bSchType==SCH_TYPE_COMM)
						{
							if (SaveTaskDataToDB(pMtrRdCtrl, MEM_TYPE_TASK, &(pMtrRdCtrl->taskSucFlg[i]))) //强制入库  可以考虑不再强制入库，因为第一回抄到会存一笔
								pMtrRdCtrl->taskSucFlg[i].bRecSaved = TASK_DATA_FULL;
						}
						fSwitch = true;
						pMtrRdCtrl->taskSucFlg[i].bRecSaved = TASK_DATA_NONE;
						pMtrRdCtrl->taskSucFlg[i].iRecPhyIdx = 0;
						pMtrRdCtrl->taskSucFlg[i].dwTime = dwCurSec;
						ClrTmpRec(pMtrRdCtrl, MEM_TYPE_TASK, pMtrRdCtrl->taskSucFlg[i].bTaskId);
						memset(pMtrRdCtrl->taskSucFlg[i].bSucFlg, 0, sizeof(pMtrRdCtrl->taskSucFlg[i].bSucFlg));
						if (iLen>0 && dwInterV==0xffff) //曲线任务整个执行间隔切换
						{
							memset(bBuf, 0, sizeof(bBuf));
							WriteMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, MEM_TYPE_CURVE_FLG, pMtrRdCtrl->taskSucFlg[i].bTaskId, bBuf);
							ClrTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId);
							GetCurTime(&tmNow);
							tmNow.nSecond = 0;
							OoTimeToDateTimeS(&tmNow, bStatBuf);
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_STARTIME, bStatBuf, 7);
							bStatBuf[0] = 1; //执行中
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_STAT, bStatBuf, 1);
							pbMs = OoGetField(pbSch, pbFmt, wFmtLen, 4, &wLen, &bType);
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_RDTOTAL, pbMs, PN_MASK_SIZE);
						}
						else if (iLen < 0) //非曲线任务状态更新
						{
							if (dwTaskLastUpdataTime[pMtrRdCtrl->taskSucFlg[i].bTaskId] != dwCurSec)
							{
								dwTaskLastUpdataTime[pMtrRdCtrl->taskSucFlg[i].bTaskId] = dwCurSec;
								ClrTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId);
								SecondsToTime(dwStartSec, &tTime);
								OoTimeToDateTimeS(&tTime, bStatBuf);
								UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_STARTIME, bStatBuf, 7);
								bStatBuf[0] = 1; //执行中
								UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_STAT, bStatBuf, 1);
								if (tTaskCfg.bSchType == SCH_TYPE_COMM)
									pbMs = OoGetField(pbSch, pbFmt, wFmtLen, 4, &wLen, &bType);
								else
									pbMs = OoGetField(pbSch, pbFmt, wFmtLen, 2, &wLen, &bType);
								UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_RDTOTAL, pbMs, PN_MASK_SIZE);
							}
						}
					}
				}
			}
		}
	}

	return fSwitch;
}

void DoFixTask(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn, bool* pfModified)
{
	int iRet;
	BYTE bBuf[128];
	DWORD *pdwOAD, dwOAD;
	WORD *pwDataLen, *pInID, wCSDLen, wNum;
	WORD wInterv = GetMeterInterv();
	DWORD dwSec = GetCurTime()/(wInterv*60)*wInterv*60;
	pInID = MtrGetFixedInItems();
	pdwOAD = MtrGetFixedItems(&wNum);
	pwDataLen = MtrGetFixedLen();
	BYTE bFailCnt = 0;
	BYTE bTryCnt = 0;

	OoReadAttr(0x310F, ATTR6, bBuf, NULL, NULL);
	bTryCnt = bBuf[3];
	if(bTryCnt == 0)
		bTryCnt =3;

	ReadItemEx(BN2, wPn, 0x6004, &bFailCnt);
	if (GetPnMtrPro(wPn) == PROTOCOLNO_SBJC) //如果是水气热测量点
		return;

	if (dwSec != pMtrRdCtrl->mtrTmpData.dwTime) //间隔切换
	{
		InitMtrTmpData(&pMtrRdCtrl->mtrTmpData, pdwOAD, pwDataLen, wNum);
		pMtrRdCtrl->mtrTmpData.dwTime = dwSec;
		*pfModified = true; //间隔时标变化也要保存
	}

	for (WORD wIndex=0; wIndex<wNum; wIndex++)
	{
		if ((iRet=GetRdMtrState(pMtrPro->bThrId)) != RD_ERR_OK)
		{
			DTRACE(DB_METER, ("DoFixTask: GetRdMtrState iRet = %d\r\n", iRet));
			return ; //直抄状态或停止抄表状态退出
		}

		if (GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, pdwOAD[wIndex], bBuf) < 0)
		{
			iRet = AskMtrItem(pMtrPro, 1, pdwOAD[wIndex], bBuf);
			if (iRet > 0)	//抄表正常
			{
				dwOAD = OoOadToDWord((BYTE *)&pdwOAD[wIndex]);
				wCSDLen = OoGetDataLen(DT_OAD, (BYTE *)&dwOAD);
				if (SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, pdwOAD[wIndex], bBuf, wCSDLen))
				{
					SaveMtrDataHook(dwOAD, &pMtrRdCtrl->mtrExcTmp, 0);
					*pfModified = true; //测量点数据已修改
				}

				WriteItemEx(BN0, wPn, pInID[wIndex], bBuf);
				bFailCnt = 0;
				WriteItemEx(BN2, wPn, 0x6004, &bFailCnt);

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
				if (bFailCnt < bTryCnt/*3*/)
					bFailCnt++;
				WriteItemEx(BN2, wPn, 0x6004, &bFailCnt);
				if (bFailCnt >= bTryCnt/*3*/)
				{
					OnMtrErrEstb(wPn);
					DoPortRdErr(true);
					break;
				}
			}
			else
			{
				dwOAD = OoOadToDWord((BYTE *)&pdwOAD[wIndex]);
				wCSDLen = OoGetDataLen(DT_OAD, (BYTE *)&dwOAD);
				memset(bBuf, INVALID_DATA, sizeof(bBuf));
				if (SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, pdwOAD[wIndex], bBuf, wCSDLen))
				{				
					SaveMtrDataHook(dwOAD, &pMtrRdCtrl->mtrExcTmp, 0);
					*pfModified = true; //测量点数据已修改
				}

				UpdItemErr(BN0, wPn, pInID[wIndex], ERR_ITEM, GetCurTime());
			}
		}
	}
}

int DoTask(WORD wPn, TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, bool* pfModified)
{
	int iRet;
	BYTE bBuf[512];
	TRdItem tRdItem;
	BYTE bTryCnt = 0;
	BYTE bFailCnt = 0;

	OoReadAttr(0x310F, ATTR6, bBuf, NULL, NULL);
	bTryCnt = bBuf[3];
	if(bTryCnt == 0)
		bTryCnt =3;
		
	ReadItemEx(BN2, wPn, 0x6004, &bFailCnt);
	
	*pfModified = DoTaskSwitch(pMtrRdCtrl);
	while ((iRet=SearchAnUnReadID(GetPnCn(wPn), wPn, pMtrRdCtrl, &tRdItem)) > 0)
	{
		if ((iRet=GetRdMtrState(pMtrPro->bThrId)) != RD_ERR_OK)
		{
			DTRACE(DB_METER, ("DoTask: GetRdMtrState iRet = %d\r\n", iRet));
			return iRet; //直抄状态或停止抄表状态退出
		}

		memset(bBuf, 0, sizeof(bBuf));
		if ((tRdItem.dwOAD&0xff000000) == 0x30000000)
		{
			memcpy(bBuf, &tRdItem.dwEvtCnt, sizeof(DWORD));
		}
		DTRACE(DB_METER, ("DoTask: Rd ---> Pn=%d, bTaskId=%d, bTaskIdx=%d wItemIdx=%d, dwOAD=%08x\r\n", \
			wPn, pMtrRdCtrl->taskSucFlg[pMtrRdCtrl->schItem.bTaskIdx].bTaskId, pMtrRdCtrl->schItem.bTaskIdx, pMtrRdCtrl->schItem.wItemIdx, tRdItem.dwOAD));
		iRet = AskMtrItem(pMtrPro, tRdItem.bReqType, tRdItem.dwOAD, bBuf, tRdItem.bRSD, tRdItem.wRsdLen, tRdItem.bRCSD, tRdItem.wRcsdLen);
		if (iRet > 0)	//抄表正常
		{
			UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[pMtrRdCtrl->schItem.bTaskIdx].bTaskId, TASK_MONIINDEX_RCVNUM);
			if (SaveMtrData(pMtrRdCtrl, tRdItem.bReqType, tRdItem.bCSD, bBuf, iRet))	//+1:跳过1字节DAR
				*pfModified = true; //测量点数据已修改

			if (tRdItem.bReqType == 1)
				SaveMtrInItemMem(wPn, tRdItem.dwOAD, bBuf);

			//hyl 20170412 特殊处理3105事件，按任务方案存储
			if (tRdItem.dwOAD==0x40000200)
				SaveMtrDataHook(tRdItem.dwOAD,&pMtrRdCtrl->mtrExcTmp, 1);
			else
				SaveMtrDataHook(tRdItem.dwOAD,&pMtrRdCtrl->mtrExcTmp, 0);

			bFailCnt = 0;
			WriteItemEx(BN2, wPn, 0x6004, &bFailCnt);
			if (IsMtrErr(wPn))
			{
				OnMtrErrRecv(wPn);
				DoPortRdErr(false);
			}
		}
		else if (iRet == 0)
		{
			iRet = RD_ERR_UNFIN;
			if (IsMtrErr(wPn))
				break;
			if (bFailCnt < bTryCnt/*3*/)
				bFailCnt++;
			WriteItemEx(BN2, wPn, 0x6004, &bFailCnt);
#ifdef EN_SBJC_V2          
			if (pMtrPro->pMtrPara->bProId == PROTOCOLNO_SBJC)
			{
				bFailCnt = 3;
				WriteItemEx(BN2, wPn, 0x6004, &bFailCnt);
			}           
#endif

			if (bFailCnt >= bTryCnt/*3*/)
			{
				OnMtrErrEstb(wPn);
				DoPortRdErr(true);
				break;
			}
		}
		else if (iRet == -2)
		{
			OoDWordToOad(tRdItem.dwOAD, bBuf);
			int iLen = OoGetDataLen(DT_OAD, bBuf); //ROAD直接取主OAD的长度
			if (iLen > 0)
			{
				memset(bBuf, INVALID_DATA, sizeof(bBuf));
				if (SaveMtrData(pMtrRdCtrl, tRdItem.bReqType, tRdItem.bCSD, bBuf, iLen))
					*pfModified = true; //测量点数据已修改
			}
		}
	}

	if (tRdItem.bReqType==3 && tRdItem.dwOAD==0x50020200)
		return RD_ERR_UNFIN; //曲线抄读返回没抄完
	else
		return iRet;
}

void DoFwdTask(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, bool* pfModified)
{
	return ;
}

//描述：分配终端事件的临时数据项动态内存，函数会以MEM_TYPE_TERM_EVTITEM类型向pTermMem申请wDataLen大小的内存空间
//参数：@dwOAD 事件OAD
//		@ pTermMem终端动态内存控制结构
//		@ wDataLen 数据长度，不应该超过64字节
//返回:如果正确则返回true,否则返回false
//说明：在事件初始化时扫描关联对象属性表，计算需要缓存数据项个数及长度，
//		如果需要内存总长度在64字节以内，则调用本函数分配临时数据项动态内存，
//		本函数成功返回，则以MEM_TYPE_TERM_EVTITEM更新TEvtBase->bMemType
bool EvtAllocItemMem(DWORD dwOAD, TTermMem* pTermMem, WORD wDataLen)
{
	if (wDataLen <= MEMORY_BLOCK_SIZE)
	{
		return AllocMem(pTermMem->bGlobal, pTermMem->allocTab, TERM_TAB_NUM, MEM_TYPE_TERM_EVTITEM, dwOAD>>8, wDataLen);
	}

	return false;
}

//描述：释放终端事件的临时数据项动态内存
//参数：@dwOAD 事件OAD
//		@ pTermMem终端动态内存控制结构
void EvtFreeItemMem(DWORD dwOAD, TTermMem* pTermMem)
{
	FreeMem(pTermMem->bGlobal, pTermMem->allocTab, TERM_TAB_NUM, MEM_TYPE_TERM_EVTITEM, dwOAD>>8);
}

//描述：读取终端事件的临时数据项动态内存
//参数：@dwOAD 事件OAD
//		@ pTermMem终端动态内存控制结构
//		@pbData 用来返回数据内容
//返回：正确返回数据长度，否则返回-1
//说明：初始化后正常的读出应该为结构【个数+个数*（OAD+LEN）+数据】
int EvtReadItemMem (DWORD dwOAD,TTermMem* pTermMem, BYTE* pbData)
{
	return ReadMem(pTermMem->allocTab, TERM_TAB_NUM, pTermMem->bMem, MEM_TYPE_TERM_EVTITEM, dwOAD>>8, pbData);
}

//描述：写终端事件的临时数据项动态内存
//参数：@dwOAD 事件OAD
//		@ pTermMem终端动态内存控制结构
//		@pbData 用来返回数据内容
//返回：正确返回数据长度，否则返回-1
//说明：应该在初始化时写入初始的结构【个数+个数*（OAD+LEN）+数据】
int EvtWriteItemMem (DWORD dwOAD, TTermMem* pTermMem, BYTE* pbData)
{
	return WriteMem(pTermMem->allocTab, TERM_TAB_NUM, pTermMem->bMem, MEM_TYPE_TERM_EVTITEM, dwOAD>>8, pbData);
}

//描述：从EvtReadItemMem ()返回的动态内存中读取其中一个数据项
//参数：@dwOAD 关联属性中的某个OAD
//		@pbData 从EvtReadItemMem ()中返回的动态内存
//		@pbItem用来返回某个数据项的数据内容
//返回：正确返回数据长度，否则返回-1
int EvtReadOneItem (DWORD dwOAD, BYTE* pbData, BYTE* pbItem)
{
	BYTE bOADNum, bLen = 0, bOffset = 0;
	DWORD dwTmpOAD;
	BYTE i;

	bOADNum = *pbData;
	bOffset = 1+bOADNum*5;
	for(i=0; i<bOADNum; i++)
	{
		dwTmpOAD = OoOadToDWord(pbData+1+5*i);
		bLen = *(pbData+5+i*5);
		if (bOffset > MEMORY_BLOCK_SIZE)
			return -1;
		if (dwTmpOAD == dwOAD)
		{	
			memcpy(pbItem, pbData+bOffset, bLen);
			return bLen;
		}
		bOffset += bLen;
	}

	return -1;
}

//描述：从EvtReadItemMem ()返回的动态内存中写入其中一个数据项
//参数：@dwOAD 关联属性中的某个OAD
//		@pbData 从EvtReadItemMem ()中返回的动态内存
//		@pbItem要写入的某个数据项的数据内容
//返回：正确返回数据长度，否则返回-1
int EvtWriteOneItem (DWORD dwOAD, BYTE* pbData, BYTE* pbItem)
{
	BYTE bOADNum, bLen = 0, bOffset = 0;
	DWORD dwTmpOAD;
	BYTE i;

	bOADNum = *pbData;
	bOffset = 1+bOADNum*5;
	for(i=0; i<bOADNum; i++)
	{
		dwTmpOAD = OoOadToDWord(pbData+1+5*i);
		bLen = *(pbData+5+i*5);
		if (bOffset > MEMORY_BLOCK_SIZE)
			return -1;
		if (dwTmpOAD == dwOAD)
		{	
			memcpy(pbData+bOffset, pbItem, bLen);
			return bLen;
		}
		bOffset += bLen;
	}

	return -1;
}



//描述：分配终端事件的整笔记录动态内存，函数会以MEM_TYPE_TERM_EVTREC类型向pTermMem申请wDataLen大小的内存空间
//参数：@dwOAD 事件OAD
//		@ pTermMem终端动态内存控制结构
//		@ wDataLen 数据长度
//返回:如果正确则返回true,否则返回false
//说明：在事件初始化时扫描关联对象属性表，计算需要整笔记录数据长度，
bool EvtAllocRecMem(DWORD dwOAD, TTermMem* pTermMem, WORD wDataLen)
{
	if (wDataLen <= MEMORY_EVTREC_SIZE)
	{
		return AllocMem(pTermMem->bGlobal, pTermMem->allocTab, TERM_TAB_NUM, MEM_TYPE_TERM_EVTREC, dwOAD>>8, wDataLen);
	}
	return false;
}

//描述：释放终端事件的整笔记录动态内存
//参数：@dwOAD 事件OAD
//		@ pTermMem终端动态内存控制结构
void EvtFreeRecMem(DWORD dwOAD, TTermMem* pTermMem)
{
	FreeMem(pTermMem->bGlobal, pTermMem->allocTab, TERM_TAB_NUM, MEM_TYPE_TERM_EVTREC, dwOAD>>8);
}

//描述：读取终端事件的整笔记录动态内存
//参数：@dwOAD 事件OAD
//		@ pTermMem终端动态内存控制结构
//		@pbData 用来返回数据内容
//返回：正确返回数据长度，否则返回-1
int EvtReadRecMem (DWORD dwOAD,TTermMem* pTermMem, BYTE* pbData)
{
	return ReadMem(pTermMem->allocTab, TERM_TAB_NUM, pTermMem->bMem, MEM_TYPE_TERM_EVTREC, dwOAD>>8, pbData);
}

//描述：读取终端事件的整笔记录动态内存
//参数：@dwOAD 事件OAD
//		@ pTermMem终端动态内存控制结构
//		@pbData 用来返回数据内容
//返回：正确返回数据长度，否则返回-1
int EvtWriteRecMem (DWORD dwOAD, TTermMem* pTermMem, BYTE* pbData)
{
	return WriteMem(pTermMem->allocTab, TERM_TAB_NUM, pTermMem->bMem, MEM_TYPE_TERM_EVTREC, dwOAD>>8, pbData);
}

