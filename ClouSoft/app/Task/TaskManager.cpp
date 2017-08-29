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

DWORD dwTaskLastUpdataTime[TASK_ID_NUM] = { 0 };	//�ϴ�����ļ��ִ��ʱ��

//�ж������Ƿ���Ҫִ��
bool IsNeedExcTask(BYTE bTaskID)
{
	TTime tmNow;
	TTaskCfg tTaskCfg;
	WORD wCurMin, wStartMin, wEndMin;
	DWORD dwStartSecond, dwEndSecond, dwNow;

	memset((BYTE*)&tTaskCfg, 0, sizeof(tTaskCfg));
	if (GetTaskCfg(bTaskID, (TTaskCfg*)&tTaskCfg))	//�������õ�Ԫ
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

//��������RSD��RCSD����
//		@pbRSD ����RSD�ڴ�
//		@wRSDLen ����RSDLen
//		@pbRCSD ����RCSD�ڴ�
//		@wRCSDLen ����RCSDLen
//		@bType �ɼ���ʽ
//		@pbData �ɼ���ʽ����
//		@pbCSD ��RCSD������
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
	case 0: //�ɼ���ǰ����
		break;

	case 1:	//�ɼ��ϵ�N��
		*pbRSD++ = 9;
		*pbRSD++ = *pbData;
		break;

	case 2:	//������ʱ��ɼ�	��Ӧ����ʱ�꣺0x2021
		*pbRSD++ = 1; //RSD ����1
		 OoDWordToOad(0x20210200, pbRSD);	//OAD=0x20210200	��¼ѡ��������
		pbRSD += 4;
		*pbRSD++ = DT_DATE_TIME_S;

		dwOAD = OoOadToDWord(&pbCSD[1]);
		switch (dwOAD)
		{
		case 0x50000200:	//˲ʱ����
		case 0x50010200:	//�붳��
			*pbRSD++ = tmNow.nYear/256;
			*pbRSD++ = tmNow.nYear%256;
			*pbRSD++ = tmNow.nMonth;
			*pbRSD++ = tmNow.nDay;
			*pbRSD++ = tmNow.nHour;
			*pbRSD++ = tmNow.nMinute;
			*pbRSD++ = tmNow.nSecond;
			break;

		case 0x50020200:	//���Ӷ���
			*pbRSD++ = tmNow.nYear/256;
			*pbRSD++ = tmNow.nYear%256;
			*pbRSD++ = tmNow.nMonth;
			*pbRSD++ = tmNow.nDay;
			*pbRSD++ = tmNow.nHour;
			*pbRSD++ = tmNow.nMinute;
			*pbRSD++ = 0;
			break;

		case 0x50030200:	//Сʱ����
			*pbRSD++ = tmNow.nYear/256;
			*pbRSD++ = tmNow.nYear%256;
			*pbRSD++ = tmNow.nMonth;
			*pbRSD++ = tmNow.nDay;
			*pbRSD++ = tmNow.nHour;
			*pbRSD++ = 0;
			*pbRSD++ = 0;
			break;

		case 0x50040200:	//�ն���
		case 0x50050200:	//�����ն���
			*pbRSD++ = tmNow.nYear/256;
			*pbRSD++ = tmNow.nYear%256;
			*pbRSD++ = tmNow.nMonth;
			*pbRSD++ = tmNow.nDay;
			*pbRSD++ = 0;
			*pbRSD++ = 0;
			*pbRSD++ = 0;
			break;

		case 0x50060200:	//�¶���
			*pbRSD++ = tmNow.nYear/256;
			*pbRSD++ = tmNow.nYear%256;
			*pbRSD++ = tmNow.nMonth;
			*pbRSD++ = 1;
			*pbRSD++ = 0;
			*pbRSD++ = 0;
			*pbRSD++ = 0;
			break;

		case 0x50070200:	//�궳��
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
	case 3:	//��ʱ�����ɼ�TI
		*pbRSD++ = 2; //RSD ����2
		OoDWordToOad(0x20210200, pbRSD);	//OAD=0x20210200	��¼ѡ��������
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
		OoTimeToDateTimeS(&tmTmp, pbRSD); //��ʼֵ
		pbRSD += 7;
		SecondsToTime(dwEndSecond, &tmTmp);
		*pbRSD++ = DT_DATE_TIME_S;
		OoTimeToDateTimeS(&tmTmp, pbRSD); //����ֵ
		pbRSD += 7;
		
		*pbRSD++ = DT_TI;
		memcpy(pbRSD, pbData, 3); //TI
		pbRSD += 3;
		break;

	case 5:	//ȫ�¼��ɼ�����
		*pbRSD++ = 2; //RSD ����2
		OoDWordToOad(0x201E0200, pbRSD); //����ʱ��
		pbRSD += 4;
		memcpy(&tiInterv, pbData, sizeof(tiInterv));
		dwIntev = TiToSecondes(&tiInterv);
		if (dwIntev == 0)
			dwIntev = 300;
		dwStartSecond = (TimeToSeconds(tmNow)/dwIntev - 1)*dwIntev;
		dwEndSecond = dwStartSecond + dwIntev;
		SecondsToTime(dwStartSecond, &tmTmp);
		*pbRSD++ = DT_DATE_TIME_S;
		OoTimeToDateTimeS(&tmTmp, pbRSD); //��ʼֵ
		pbRSD += 7;
		*pbRSD++ = DT_DATE_TIME_S;
		SecondsToTime(dwEndSecond, &tmTmp);
		OoTimeToDateTimeS(&tmTmp, pbRSD); //����ֵ
		pbRSD += 7;

		//*pbRSD++ = NULL; //TI
		*pbRSD++ = 0xFF; //TI
		break;

	case 6:	//ȫ�¼��ɼ�����
		*pbRSD++ = 9; //RSD ����9
		//*pbRSD++ = DT_UNSIGN;
		*pbRSD++ = 1; //��1���¼�����
		break;
	}
	pRdItem->wRsdLen = (WORD)(pbRSD - pRdItem->bRSD);

	if (bMethod == 6) //ȫ�¼����ϲɼ�����
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
			if (wSucNum==0 || pMtrRdCtrl->taskSucFlg[bTaskIndex].bRecSaved==TASK_DATA_FULL) //����û�г����ɹ� || �����Ѿ��������
				continue;

			if (pMtrRdCtrl->taskSucFlg[bTaskIndex].bRecSaved==TASK_DATA_PART && wSucNum<pMtrRdCtrl->taskSucFlg[bTaskIndex].bCSDItemNum) //������� && ��û��ȫ������
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
				if (iLen > 0) //��������ĳ��ʱ�̳�����־���⴦��
				{
					pbFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
					pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
					if (pbSch != NULL)
					{
						pbCollMode = OoGetField(pbSch, pbFmt, wFmtLen, 2, &wLen, &bCollType);	//�ɼ���ʽ
						tiExe.bUnit = pbCollMode[5];
						tiExe.wVal = OoLongUnsignedToWord(pbCollMode+6);
						dwIntV = TiToSecondes(&tiExe);
						if (dwIntV == 0)
							dwIntV = 300;
						GetTaskCurExeTime(&tTaskCfg, &dwCurSec, &dwStart, &dwEnd);
						WORD wPos = (pMtrRdCtrl->taskSucFlg[bTaskIndex].dwTime - (dwCurSec - TiToSecondes(&(tTaskCfg.tiExe)))) / dwIntV;
						bBuf[wPos/8] |= (1<<(wPos%8));
						WriteMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, MEM_TYPE_CURVE_FLG, pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, bBuf);

						wPos = TiToSecondes(&(tTaskCfg.tiExe)) / dwIntV; //������Ҫ�ɼ����ٸ����ߵ�
						if (CalcuBitNum(bBuf, iLen) == wPos) //ȫ���ɼ��ɹ�
						{
							GetCurTime(&tmNow);
							OoTimeToDateTimeS(&tmNow, bBuf);
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, TASK_MONIINDEX_ENDTIME, bBuf, 7);
							bBuf[0] = 2; //��ִ��
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, TASK_MONIINDEX_STAT, bBuf, 1);
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, TASK_MONIINDEX_SUCNUM);
						}
					}
				}
				else if (iLen < 0) //����������״̬����
				{
					GetCurTime(&tmNow);
					OoTimeToDateTimeS(&tmNow, bBuf);
					UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[bTaskIndex].bTaskId, TASK_MONIINDEX_ENDTIME, bBuf, 7);
					bBuf[0] = 2; //��ִ��
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
		//�ɼ������ֶη���
		fIsSaveFlg = true;
		memset(bRecBuf, 0, sizeof(bRecBuf));
		pbRecBuf = bRecBuf;

		//Ŀ�ķ�������ַ
		memcpy(pbRecBuf, pMtrRdCtrl->bTsa, pMtrRdCtrl->bTsa[0]+1);
		DWORD dwOAD;
		dwOAD = 0x202a0200;
		dwOAD = OoOadToDWord((BYTE*)&dwOAD);
		pbRecBuf += OoGetDataLen(81, (BYTE*)&dwOAD);

		//�ɼ�����ʱ��
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
			//�������ݵĲɼ��ɹ�ʱ��Ӧ�����Ϊ��һ�մ������ʼ�ļ��ʱ��
			pbDataFmt = GetSchFmt(tTaskCfg.bSchType, &wDataFmtLen);
			pbPtr = OoGetField(pbSch, pbDataFmt, wDataFmtLen, 2, &wLen, &bFmtType);	//�ɼ���ʽ
			if (*pbPtr++ == DT_STRUCT)
			{
				pbPtr++;
				if (*pbPtr++ == DT_UNSIGN)
				{
					bCollType = *pbPtr++;
					if (bCollType == 0x03)	//�ɼ�����Ϊ3����ʱ�����ɼ���ֱ���ù�����Ĳɼ�����ʱ��
					{
						//�ɼ��ɹ�ʱ�꣨4��
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
						//�ɼ��ɹ�ʱ�꣨4��
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
				case 0:	//δ����
					GetCurTime(&tTime);
					break;
				case 1:	//����ʼʱ��
					SecondsToTime(ptaskSucFlg->dwTime, &tTime);
					break;
				case 2:	//��Ե���0��0��
					GetCurTime(&tTime);
					tTime.nHour = 0;
					tTime.nMinute = 0;
					tTime.nSecond = 0;
					break;
				case 3:	//�������23��59��
					GetCurTime(&tTime);
					AddIntervs(tTime, TIME_UNIT_DAY, -1);
					tTime.nHour = 23;
					tTime.nMinute = 59;
					tTime.nSecond = 0;
					break;
				case 4:	//�������0��0��
					GetCurTime(&tTime);
					AddIntervs(tTime, TIME_UNIT_DAY, -1);
					tTime.nHour = 0;
					tTime.nMinute = 0;
					tTime.nSecond = 0;
					break;
				case 5:	//��Ե���1��0��0��
					GetCurTime(&tTime);
					tTime.nDay = 1;
					tTime.nHour = 0;
					tTime.nMinute = 0;
					tTime.nSecond = 0;
					break;
				case 6:	//���ݶ���ʱ�� 
					SecondsToTime(ptaskSucFlg->dwTime, &tTime);
//					int iRet;
//					DWORD dwFrzOAD;	//���ݶ���ʱ��
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
//										if ((dwOAD&0xFF000000) == 0x50000000)	//��ʾ���������
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
//						tTime.nSecond = 0;	//��ǿ��Ϊ0
//						break;
//					}
//					else
//					{
//						TraceBuf(DB_CRITICAL, "SaveTaskDataToDB(): err:", bMemRec, iRet);
//						TraceBuf(DB_CRITICAL, "SaveTaskDataToDB(): err:", pTime, 8);
//						return false;
//					}
					break;
				case 7:	//���������ĩ0��0��
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
						if ((tTime.nYear%4==0 && tTime.nYear%100!=0) || tTime.nYear%400==0)   //����
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
			
			//�ɼ��洢ʱ��
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
			//�ɼ��ɹ�ʱ�꣨4��
			GetCurTime(&tTime);	
			pbRecBuf[0] = tTime.nYear/256;
			pbRecBuf[1] = tTime.nYear%256;
			pbRecBuf[2] = tTime.nMonth;
			pbRecBuf[3] = tTime.nDay;
			pbRecBuf[4] = tTime.nHour;
			pbRecBuf[5] = tTime.nMinute;
			pbRecBuf[6] = tTime.nSecond;
			pbRecBuf += 7;

			//�ɼ��洢ʱ��
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
				return true; //����ʵ�ʱ���
		}
		pbRecBuf += iRet;
		break;
// 	case SCH_TYPE_TRANS:	��������ִ��
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
		DT_UNSIGN, 0x00,//����ID
		DT_ENUM, 0x00,	//����ִ��״̬
		DT_DATE_TIME_S, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	//����ִ�п�ʼʱ�� 
		DT_DATE_TIME_S,	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	//����ִ�н���ʱ�� 
		DT_LONG_U, 0x00, 0x00,	//�ɼ�������
		DT_LONG_U, 0x00, 0x00,	//�ɼ��ɹ����� 
		DT_LONG_U, 0x00, 0x00,	//�ѷ��ͱ�������
		DT_LONG_U, 0x00, 0x00,	//�ѽ��ձ������� 
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

	//���񷽰����ݲɼ�
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
				if (iLen > 0) //��������ʱ���л����⴦��
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
						pbCollMode = OoGetField(pbSch, pbFmt, wFmtLen, 2, &wLen, &bType);	//�ɼ���ʽ
						bType = pbCollMode[3];
						tiExe.bUnit = pbCollMode[5];
						tiExe.wVal = OoLongUnsignedToWord(pbCollMode+6);
						dwCurSecbak = dwCurSec;
						if (dwInterV != 0xffff)
						{
							if (bType == 3 /*&& tiExe.bUnit==1*/) //��TI����ɼ�
							{
								dwCurSec -= TiToSecondes(&(tTaskCfg.tiExe));
								dwCurSec += dwInterV*TiToSecondes(&tiExe);
							}
						}
						else
						{
							if (bType == 3 /*&& tiExe.bUnit==1*/) //��TI����ɼ�
							{
								dwCurSec -= TiToSecondes(&(tTaskCfg.tiExe));
								dwCurSec += (iLen*8-1)*TiToSecondes(&tiExe); //�̶������һ���ɼ������
							}
						}
						if (dwCurSec >= dwCurSecbak)	//�������ִ����
						{
							if (pMtrRdCtrl->taskSucFlg[i].dwTime>=(dwStartSec-TiToSecondes(&tiExe)) && pMtrRdCtrl->taskSucFlg[i].dwTime<=(dwEndSec-TiToSecondes(&tiExe)))	//�����δ����
							{
								dwCurSec = pMtrRdCtrl->taskSucFlg[i].dwTime;	//��ֹ����������س�
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
							bStatBuf[0] = 1; //ִ����
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_STAT, bStatBuf, 1);
							if (tTaskCfg.bSchType == SCH_TYPE_COMM)
								pbMs = OoGetField(pbSch, pbFmt, wFmtLen, 4, &wLen, &bType);
							else
								pbMs = OoGetField(pbSch, pbFmt, wFmtLen, 2, &wLen, &bType);
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_RDTOTAL, pbMs, PN_MASK_SIZE);
						}
					}
				}

				if (dwCurSec != pMtrRdCtrl->taskSucFlg[i].dwTime) //����л�
				{
					pbFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
					pbSch = GetSchCfg(&tTaskCfg, &iSchLen);
					if (pbSch != NULL)
					{
						if (iLen<0 && pMtrRdCtrl->taskSucFlg[i].bRecSaved==TASK_DATA_PART && tTaskCfg.bSchType==SCH_TYPE_COMM)
						{
							if (SaveTaskDataToDB(pMtrRdCtrl, MEM_TYPE_TASK, &(pMtrRdCtrl->taskSucFlg[i]))) //ǿ�����  ���Կ��ǲ���ǿ����⣬��Ϊ��һ�س������һ��
								pMtrRdCtrl->taskSucFlg[i].bRecSaved = TASK_DATA_FULL;
						}
						fSwitch = true;
						pMtrRdCtrl->taskSucFlg[i].bRecSaved = TASK_DATA_NONE;
						pMtrRdCtrl->taskSucFlg[i].iRecPhyIdx = 0;
						pMtrRdCtrl->taskSucFlg[i].dwTime = dwCurSec;
						ClrTmpRec(pMtrRdCtrl, MEM_TYPE_TASK, pMtrRdCtrl->taskSucFlg[i].bTaskId);
						memset(pMtrRdCtrl->taskSucFlg[i].bSucFlg, 0, sizeof(pMtrRdCtrl->taskSucFlg[i].bSucFlg));
						if (iLen>0 && dwInterV==0xffff) //������������ִ�м���л�
						{
							memset(bBuf, 0, sizeof(bBuf));
							WriteMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, MEM_TYPE_CURVE_FLG, pMtrRdCtrl->taskSucFlg[i].bTaskId, bBuf);
							ClrTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId);
							GetCurTime(&tmNow);
							tmNow.nSecond = 0;
							OoTimeToDateTimeS(&tmNow, bStatBuf);
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_STARTIME, bStatBuf, 7);
							bStatBuf[0] = 1; //ִ����
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_STAT, bStatBuf, 1);
							pbMs = OoGetField(pbSch, pbFmt, wFmtLen, 4, &wLen, &bType);
							UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_RDTOTAL, pbMs, PN_MASK_SIZE);
						}
						else if (iLen < 0) //����������״̬����
						{
							if (dwTaskLastUpdataTime[pMtrRdCtrl->taskSucFlg[i].bTaskId] != dwCurSec)
							{
								dwTaskLastUpdataTime[pMtrRdCtrl->taskSucFlg[i].bTaskId] = dwCurSec;
								ClrTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId);
								SecondsToTime(dwStartSec, &tTime);
								OoTimeToDateTimeS(&tTime, bStatBuf);
								UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[i].bTaskId, TASK_MONIINDEX_STARTIME, bStatBuf, 7);
								bStatBuf[0] = 1; //ִ����
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
	if (GetPnMtrPro(wPn) == PROTOCOLNO_SBJC) //�����ˮ���Ȳ�����
		return;

	if (dwSec != pMtrRdCtrl->mtrTmpData.dwTime) //����л�
	{
		InitMtrTmpData(&pMtrRdCtrl->mtrTmpData, pdwOAD, pwDataLen, wNum);
		pMtrRdCtrl->mtrTmpData.dwTime = dwSec;
		*pfModified = true; //���ʱ��仯ҲҪ����
	}

	for (WORD wIndex=0; wIndex<wNum; wIndex++)
	{
		if ((iRet=GetRdMtrState(pMtrPro->bThrId)) != RD_ERR_OK)
		{
			DTRACE(DB_METER, ("DoFixTask: GetRdMtrState iRet = %d\r\n", iRet));
			return ; //ֱ��״̬��ֹͣ����״̬�˳�
		}

		if (GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, pdwOAD[wIndex], bBuf) < 0)
		{
			iRet = AskMtrItem(pMtrPro, 1, pdwOAD[wIndex], bBuf);
			if (iRet > 0)	//��������
			{
				dwOAD = OoOadToDWord((BYTE *)&pdwOAD[wIndex]);
				wCSDLen = OoGetDataLen(DT_OAD, (BYTE *)&dwOAD);
				if (SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, pdwOAD[wIndex], bBuf, wCSDLen))
				{
					SaveMtrDataHook(dwOAD, &pMtrRdCtrl->mtrExcTmp, 0);
					*pfModified = true; //�������������޸�
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
					*pfModified = true; //�������������޸�
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
			return iRet; //ֱ��״̬��ֹͣ����״̬�˳�
		}

		memset(bBuf, 0, sizeof(bBuf));
		if ((tRdItem.dwOAD&0xff000000) == 0x30000000)
		{
			memcpy(bBuf, &tRdItem.dwEvtCnt, sizeof(DWORD));
		}
		DTRACE(DB_METER, ("DoTask: Rd ---> Pn=%d, bTaskId=%d, bTaskIdx=%d wItemIdx=%d, dwOAD=%08x\r\n", \
			wPn, pMtrRdCtrl->taskSucFlg[pMtrRdCtrl->schItem.bTaskIdx].bTaskId, pMtrRdCtrl->schItem.bTaskIdx, pMtrRdCtrl->schItem.wItemIdx, tRdItem.dwOAD));
		iRet = AskMtrItem(pMtrPro, tRdItem.bReqType, tRdItem.dwOAD, bBuf, tRdItem.bRSD, tRdItem.wRsdLen, tRdItem.bRCSD, tRdItem.wRcsdLen);
		if (iRet > 0)	//��������
		{
			UpdateTaskMoniStat(pMtrRdCtrl->taskSucFlg[pMtrRdCtrl->schItem.bTaskIdx].bTaskId, TASK_MONIINDEX_RCVNUM);
			if (SaveMtrData(pMtrRdCtrl, tRdItem.bReqType, tRdItem.bCSD, bBuf, iRet))	//+1:����1�ֽ�DAR
				*pfModified = true; //�������������޸�

			if (tRdItem.bReqType == 1)
				SaveMtrInItemMem(wPn, tRdItem.dwOAD, bBuf);

			//hyl 20170412 ���⴦��3105�¼��������񷽰��洢
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
			int iLen = OoGetDataLen(DT_OAD, bBuf); //ROADֱ��ȡ��OAD�ĳ���
			if (iLen > 0)
			{
				memset(bBuf, INVALID_DATA, sizeof(bBuf));
				if (SaveMtrData(pMtrRdCtrl, tRdItem.bReqType, tRdItem.bCSD, bBuf, iLen))
					*pfModified = true; //�������������޸�
			}
		}
	}

	if (tRdItem.bReqType==3 && tRdItem.dwOAD==0x50020200)
		return RD_ERR_UNFIN; //���߳�������û����
	else
		return iRet;
}

void DoFwdTask(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, bool* pfModified)
{
	return ;
}

//�����������ն��¼�����ʱ�����̬�ڴ棬��������MEM_TYPE_TERM_EVTITEM������pTermMem����wDataLen��С���ڴ�ռ�
//������@dwOAD �¼�OAD
//		@ pTermMem�ն˶�̬�ڴ���ƽṹ
//		@ wDataLen ���ݳ��ȣ���Ӧ�ó���64�ֽ�
//����:�����ȷ�򷵻�true,���򷵻�false
//˵�������¼���ʼ��ʱɨ������������Ա�������Ҫ������������������ȣ�
//		�����Ҫ�ڴ��ܳ�����64�ֽ����ڣ�����ñ�����������ʱ�����̬�ڴ棬
//		�������ɹ����أ�����MEM_TYPE_TERM_EVTITEM����TEvtBase->bMemType
bool EvtAllocItemMem(DWORD dwOAD, TTermMem* pTermMem, WORD wDataLen)
{
	if (wDataLen <= MEMORY_BLOCK_SIZE)
	{
		return AllocMem(pTermMem->bGlobal, pTermMem->allocTab, TERM_TAB_NUM, MEM_TYPE_TERM_EVTITEM, dwOAD>>8, wDataLen);
	}

	return false;
}

//�������ͷ��ն��¼�����ʱ�����̬�ڴ�
//������@dwOAD �¼�OAD
//		@ pTermMem�ն˶�̬�ڴ���ƽṹ
void EvtFreeItemMem(DWORD dwOAD, TTermMem* pTermMem)
{
	FreeMem(pTermMem->bGlobal, pTermMem->allocTab, TERM_TAB_NUM, MEM_TYPE_TERM_EVTITEM, dwOAD>>8);
}

//��������ȡ�ն��¼�����ʱ�����̬�ڴ�
//������@dwOAD �¼�OAD
//		@ pTermMem�ն˶�̬�ڴ���ƽṹ
//		@pbData ����������������
//���أ���ȷ�������ݳ��ȣ����򷵻�-1
//˵������ʼ���������Ķ���Ӧ��Ϊ�ṹ������+����*��OAD+LEN��+���ݡ�
int EvtReadItemMem (DWORD dwOAD,TTermMem* pTermMem, BYTE* pbData)
{
	return ReadMem(pTermMem->allocTab, TERM_TAB_NUM, pTermMem->bMem, MEM_TYPE_TERM_EVTITEM, dwOAD>>8, pbData);
}

//������д�ն��¼�����ʱ�����̬�ڴ�
//������@dwOAD �¼�OAD
//		@ pTermMem�ն˶�̬�ڴ���ƽṹ
//		@pbData ����������������
//���أ���ȷ�������ݳ��ȣ����򷵻�-1
//˵����Ӧ���ڳ�ʼ��ʱд���ʼ�Ľṹ������+����*��OAD+LEN��+���ݡ�
int EvtWriteItemMem (DWORD dwOAD, TTermMem* pTermMem, BYTE* pbData)
{
	return WriteMem(pTermMem->allocTab, TERM_TAB_NUM, pTermMem->bMem, MEM_TYPE_TERM_EVTITEM, dwOAD>>8, pbData);
}

//��������EvtReadItemMem ()���صĶ�̬�ڴ��ж�ȡ����һ��������
//������@dwOAD ���������е�ĳ��OAD
//		@pbData ��EvtReadItemMem ()�з��صĶ�̬�ڴ�
//		@pbItem��������ĳ�����������������
//���أ���ȷ�������ݳ��ȣ����򷵻�-1
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

//��������EvtReadItemMem ()���صĶ�̬�ڴ���д������һ��������
//������@dwOAD ���������е�ĳ��OAD
//		@pbData ��EvtReadItemMem ()�з��صĶ�̬�ڴ�
//		@pbItemҪд���ĳ�����������������
//���أ���ȷ�������ݳ��ȣ����򷵻�-1
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



//�����������ն��¼������ʼ�¼��̬�ڴ棬��������MEM_TYPE_TERM_EVTREC������pTermMem����wDataLen��С���ڴ�ռ�
//������@dwOAD �¼�OAD
//		@ pTermMem�ն˶�̬�ڴ���ƽṹ
//		@ wDataLen ���ݳ���
//����:�����ȷ�򷵻�true,���򷵻�false
//˵�������¼���ʼ��ʱɨ������������Ա�������Ҫ���ʼ�¼���ݳ��ȣ�
bool EvtAllocRecMem(DWORD dwOAD, TTermMem* pTermMem, WORD wDataLen)
{
	if (wDataLen <= MEMORY_EVTREC_SIZE)
	{
		return AllocMem(pTermMem->bGlobal, pTermMem->allocTab, TERM_TAB_NUM, MEM_TYPE_TERM_EVTREC, dwOAD>>8, wDataLen);
	}
	return false;
}

//�������ͷ��ն��¼������ʼ�¼��̬�ڴ�
//������@dwOAD �¼�OAD
//		@ pTermMem�ն˶�̬�ڴ���ƽṹ
void EvtFreeRecMem(DWORD dwOAD, TTermMem* pTermMem)
{
	FreeMem(pTermMem->bGlobal, pTermMem->allocTab, TERM_TAB_NUM, MEM_TYPE_TERM_EVTREC, dwOAD>>8);
}

//��������ȡ�ն��¼������ʼ�¼��̬�ڴ�
//������@dwOAD �¼�OAD
//		@ pTermMem�ն˶�̬�ڴ���ƽṹ
//		@pbData ����������������
//���أ���ȷ�������ݳ��ȣ����򷵻�-1
int EvtReadRecMem (DWORD dwOAD,TTermMem* pTermMem, BYTE* pbData)
{
	return ReadMem(pTermMem->allocTab, TERM_TAB_NUM, pTermMem->bMem, MEM_TYPE_TERM_EVTREC, dwOAD>>8, pbData);
}

//��������ȡ�ն��¼������ʼ�¼��̬�ڴ�
//������@dwOAD �¼�OAD
//		@ pTermMem�ն˶�̬�ڴ���ƽṹ
//		@pbData ����������������
//���أ���ȷ�������ݳ��ȣ����򷵻�-1
int EvtWriteRecMem (DWORD dwOAD, TTermMem* pTermMem, BYTE* pbData)
{
	return WriteMem(pTermMem->allocTab, TERM_TAB_NUM, pTermMem->bMem, MEM_TYPE_TERM_EVTREC, dwOAD>>8, pbData);
}

