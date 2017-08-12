/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CctRdCtrl.cpp
 * ժ    Ҫ���ز��ѱ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�CL
 * ������ڣ�2016��11��
 *********************************************************************************************************/

#include "stdafx.h"
#include "CctSchMtr.h"
#include "ComAPI.h"
#include "CctAPI.h"
#include "ComStruct.h"
#include "DbOIAPI.h"
#include "LibDbAPI.h"
#include "FaCfg.h"

CCctSchMeter::CCctSchMeter(void)
{
	m_bSchMtrState = SCH_MTR_EMPTY;
	m_fRightNowSchMtr = false;
	m_fPeriodSchMtr = false;
	m_fUdpMtrToDB = false;
	m_tSchMtrSem = NewSemaphore(1);	
	m_tAlarmSem = NewSemaphore(1);
}

//������ִ���ѱ�
//���أ��ѱ��Ƿ����
bool CCctSchMeter::DoSchMtrAddr()
{
	int iRet;

	if (GetInfo(INFO_SCH_MTR) && !m_fRightNowSchMtr)	//������������ѱ�
	{
		m_fRightNowSchMtr = true;
		m_bSchMtrState = START_BOARD_SCH_MTR;
		m_fStartBoardCast = false;
		m_bActCnt = 0;
		m_fClrFile = false;
		DTRACE(DB_CCT_SCH, ("DoSchMtrAddr(): Right now search meter start.\n"));
		
		ClearSchMtrEvtMask();
		SetInfo(INFO_START_485_SCH_MTR);
	}
	else if(!m_fRightNowSchMtr && IsSchMtrPeriod() && !m_fPeriodSchMtr)	//ʱ���ѱ�(ʱ���ѱ��ڼ�δ���������ѱ�)
	{
		m_fPeriodSchMtr = true;
		m_bSchMtrState = START_BOARD_SCH_MTR;
		m_fStartBoardCast = false;
		m_fClrFile = false;
		DTRACE(DB_CCT_SCH, ("DoSchMtrAddr(): Period search meter start.\n"));

		ClearSchMtrEvtMask();
		SetInfo(INFO_START_485_SCH_MTR);
	}

	if (m_fRightNowSchMtr || m_fPeriodSchMtr)
	{
		switch(m_bSchMtrState)
		{
		case START_BOARD_SCH_MTR:
			iRet = StartSchMtr();
			if (iRet == 0)
				m_bSchMtrState = START_NODE_ACTIVE;
			else if (iRet == -1)
				m_bSchMtrState = FINISH_SCH_MTR;
			break;
		case START_NODE_ACTIVE:
			if (StartNodeActive())
				m_bSchMtrState = WAIT_MTR_REPORT;
			else
				m_bSchMtrState = FINISH_SCH_MTR;
			break;
		case WAIT_MTR_REPORT:
			if (WaitMtrReport())
				m_bSchMtrState = FINISH_SCH_MTR;
			break;
		case FINISH_SCH_MTR:
			if (FinishSchMtr())
			{
				m_bSchMtrState = SCH_MTR_EMPTY;
				SetInfo(INFO_STOP_485_SCH_MTR);
			}
			break;
		default:
			break;
		}
	}

	if (m_bSchMtrState == SCH_MTR_EMPTY) //��ʾδִ���ѱ�
	{
		m_fPeriodSchMtr = false;
		m_fRightNowSchMtr = false;
		return false;
	}

	return true;	//���ѱ�״̬
}

//��������ȡ�ѱ����
bool CCctSchMeter::GetSchMtrParam(TSchMtrParam *pSchMtrParam)
{
	BYTE bBuf[128];
	BYTE *p = bBuf;
	
	memset((BYTE*)pSchMtrParam, 0, sizeof(TSchMtrParam));
	memset(bBuf, 0, sizeof(bBuf));
	if (ReadItemEx(BANK0, PN0, 0x6006, bBuf) > 0)
	{
		if (!IsAllAByte(bBuf, 0, sizeof(bBuf)))
		{
			if (*p++ == DT_STRUCT)
			{
				if (*p++ == 4)
				{
					if (*p++ == DT_BOOL)
					{
						pSchMtrParam->fAutoSchMtr = *p++;
						if (*p++ == DT_BOOL)
						{
							pSchMtrParam->fAutoUpdMtr = *p++;
							if (*p++ == DT_BOOL)
							{
								pSchMtrParam->fIsGenEvt = *p++;
								if (*p++ == DT_ENUM)
								{
									pSchMtrParam->bClrMtrChoice = *p++;

									return p - bBuf;
								}
							}
						}
					}
				}
			}
		}
		else
		{
			DTRACE(DB_CCT_SCH, ("Get search Mtr param is all zero, OAD=0x%08x.\n", 0x60020800));
		}
	}

	return -1;
}

//�������Ƿ����ѱ�ʱ�β���(60020900)
//���أ�>0��ʾ���ѱ�ʱ�䷶Χ��
bool CCctSchMeter::IsSchMtrPeriod()
{
	TSchMtrTimeParam tTimeParam;
	DWORD dwNowTime, dwStartSchMtrTime, dwEndSchMtrTime;
	BYTE bBuf[256];
	BYTE *p = bBuf;
	static DWORD m_dwCnt = 0;

	dwNowTime = GetCurTime();
	memset((BYTE*)&tTimeParam, 0, sizeof(tTimeParam));
	GetCurTime(&tTimeParam.tStartTime);

	memset(bBuf, 0, sizeof(bBuf));
	if (ReadItemEx(BANK0, PN0, 0x6007, bBuf) > 0)
	{
		if (*p++ == DT_ARRAY)
		{	
			BYTE bNum;
			bNum = *p++;
			for (BYTE k=0; k<bNum; k++)
			{
				if (*p++ == DT_STRUCT)
				{
					if (*p++ == 0x02)
					{
						if (*p++ == DT_TIME)
						{
							tTimeParam.tStartTime.nHour = *p++;
							tTimeParam.tStartTime.nMinute = *p++;
							tTimeParam.tStartTime.nSecond = *p++;
							p++;	//long-unsigned
							tTimeParam.wKeptTime = OoOiToWord(p);
							p += 2;

							dwStartSchMtrTime = TimeToSeconds(tTimeParam.tStartTime);
							dwEndSchMtrTime = dwStartSchMtrTime + tTimeParam.wKeptTime*60;
							if (dwNowTime>dwStartSchMtrTime && dwNowTime<=dwEndSchMtrTime)
							{
								return true;
							}
							else
							{
								char szStartSchTime[32] = {0};
								char szEndSchTime[32] = {0};
								char szNowTime[32] = {0};
								TTime tStartSchTime;
								TTime tEndSchTime;
								TTime tNowTime;
								
								SecondsToTime(dwStartSchMtrTime, &tStartSchTime);
								SecondsToTime(dwEndSchMtrTime, &tEndSchTime);
								GetCurTime(&tNowTime);

								if (m_dwCnt++ > 300)
								{
									m_dwCnt = 0;
									DTRACE(DB_CCT_SCH, ("IsSchMtrPeriod(): StartSchMtrTime=%s, EndSchMtrTime=%s, NowTime=%s, KeepTime=%dS.\n", \
										TimeToStr(tStartSchTime, szStartSchTime), TimeToStr(tEndSchTime, szEndSchTime), TimeToStr(tNowTime, szNowTime),tTimeParam.wKeptTime*60));
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		DTRACE(DB_CCT_SCH, ("Get search Mtr period param fail, OAD=0x%08x.\n", 0x60020900));
	}

	return false;	
}


//�����������ѱ�״̬
//������true-�ѱ��У�false-����
int CCctSchMeter::SetSchMtrState(bool fState)
{
	BYTE bBuf[8];

	memset(bBuf, 0, sizeof(bBuf));
	bBuf[0] = DT_ENUM;
	if (fState)
		bBuf[1] = 1;
	else
		bBuf[1] = 0;
	if (WriteItemEx(BANK0, PN0, 0x6008, bBuf) > 0)
		return 1;

	return -1;
}

#define SINGLE_MTR_ADDR		(1<<0x00)	//�����ַ���ɼ�����ַΪ��
#define SINGLE_ACQ_ADDR		(1<<(0x01))	//���ɼ�����ַ�����ַΪ��
#define ACQ_MTR_ADDR		(1<<(0x02))	//���ַ+�ɼ�����ַ

//������У����ַ�Ƿ����ն˵�����
//������@pSchMtrRlt һ���ѻصı���
//��ע��·��ģ���ϱ��ĵ�ַ������
//		1.�����ַ
//			�жϣ�1.1 ���صĵ�ַ�Ƿ��ڵ�ǰ�ġ��ɼ���+����ַ���У����ھ�ɾ��ԭ�ȵĵ����������ѻصĵ���
//		2.�ɼ���+����ַ
//			�жϣ�2.1 �ɼ�����ַ��������ַδ���������µ�ǰ���ַ��Ӧ�ĵ���
//				  2.2 �ɼ�����ַδ��������ַ������ն��Ƿ������Ӧ�ĵ�ַ��������µ���
//				  2.3 �ɼ�����ַ�����ַ����ȫ�µģ����µ���
//				  2.4 �ɼ�����ַ�����ַ����ͬ
//		3.�ɼ���+�ձ��ַ
//				  3.1 ϵͳ���Ƿ���ڸòɼ��������ַ�������ڣ����µ���
//���أ�true-��ʾ�ն��������ͬ�ĵ�ַ��������µ���
void CCctSchMeter::CheckMtrAddr(TSchMtrRlt *pSchMtrRlt)
{
	char szMtr[32];
	char szAcq[32];
	char szDbMtr[32];
	char szDbAcq[32];
	BYTE bAddrState = 0x00;
	int iStart = -1;
	int index;
	TSchMtrRlt tDbSchMtrRlt;
	bool fIsExistSameMtr = false;
	
	//��·���ϱ��ĵ�ַ�����ж���������������
	if ((IsAllAByte(pSchMtrRlt->bMtr, 0x00, 6) || IsAllAByte(pSchMtrRlt->bMtr, 0xee, 6)) && !IsAllAByte(pSchMtrRlt->bAcqAddr, 0, 6))	//���ɼ�����ַ������ַΪ��
		bAddrState = SINGLE_ACQ_ADDR;
	else if (!IsAllAByte(pSchMtrRlt->bMtr, 0x00, 6) && (IsAllAByte(pSchMtrRlt->bAcqAddr, 0x00, 6) || IsAllAByte(pSchMtrRlt->bAcqAddr, 0xee, 6)))	//������ַ,�ɼ�����ַΪ��
		bAddrState = SINGLE_MTR_ADDR;
	else //�ɼ���+����ַ
		bAddrState = ACQ_MTR_ADDR;

	do 
	{
		memset((BYTE*)&tDbSchMtrRlt, 0, sizeof(tDbSchMtrRlt));
		index = GetSchMtrResult(&iStart, &tDbSchMtrRlt);
		if (index == -1)
			break;

		if (bAddrState == SINGLE_MTR_ADDR)
		{
			if (memcmp(pSchMtrRlt->bMtr, tDbSchMtrRlt.bMtr, 6) == 0)	//�ն��������ͬ�ĵ���ַ
			{
				//if (!IsAllAByte(tDbSchMtrRlt.bAcqAddr, 0x00, 6) && !IsAllAByte(tDbSchMtrRlt.bAcqAddr, 0xee, 6))	//�ն���Ĳɼ���ַ��ΪNULL���滻
					ReplaceOneSchMtrResult(index, pSchMtrRlt);
					LoopSchMtrResult(pSchMtrRlt);
					LoopSchMtrResult(&tDbSchMtrRlt);
				fIsExistSameMtr = true;
				break;
			}
		}
		else if (bAddrState == SINGLE_ACQ_ADDR)
		{
			if (memcmp(pSchMtrRlt->bAcqAddr, tDbSchMtrRlt.bAcqAddr, 6) == 0)	//�ն��������ͬ�ɼ�����ַ���Ϳ���ֱ���˳�
			{
// 				tDbSchMtrRlt.tSchMtrSucTime = pSchMtrRlt->tSchMtrSucTime;
// 				ReplaceOneSchMtrResult(index, &tDbSchMtrRlt);	//�����滻��Ҫ��Ϊ�˸����ѱ�ʱ��
				fIsExistSameMtr = true;
				break;
			}
		}
		else	//�ɼ���+����ַ
		{
			if (memcmp(pSchMtrRlt->bAcqAddr, tDbSchMtrRlt.bAcqAddr, 6) == 0)	//�ɼ���ַ���
			{
				if ((IsAllAByte(tDbSchMtrRlt.bMtr, 0xee, 6) || IsAllAByte(tDbSchMtrRlt.bMtr, 0x00, 6))	//ϵͳ���е���ַΪ�գ�ֱ�Ӵ���
					|| (memcmp(pSchMtrRlt->bMtr, tDbSchMtrRlt.bMtr, 6) == 0))	//���ַ��ȣ�ֱ�Ӵ���
				{
					ReplaceOneSchMtrResult(index, pSchMtrRlt);	//�����ѱ�ʱ�䡢���²ɼ�����ַ
					LoopSchMtrResult(pSchMtrRlt);
					LoopSchMtrResult(&tDbSchMtrRlt);
					fIsExistSameMtr = true;
					break;
				}
			}

			if (memcmp(pSchMtrRlt->bMtr, tDbSchMtrRlt.bMtr, 6) == 0)	//����ַ��ȣ����۲ɼ���ַ�Ƿ���ȣ���ֱ���滻
			{
				ReplaceOneSchMtrResult(index, pSchMtrRlt);	//�����ѱ�ʱ�䡢���²ɼ�����ַ
				LoopSchMtrResult(pSchMtrRlt);
				LoopSchMtrResult(&tDbSchMtrRlt);
				fIsExistSameMtr = true;
				break;
			}
		}
	}while (iStart != -1);

	if (!fIsExistSameMtr)
	{
		SaveOneSchMtrResult(pSchMtrRlt);
		DTRACE(DB_CCT_SCH, ("[Add to 0x6002]  RptMeter:%s, RptAcq:%s, MtrPro:%d.\n", HexToStr(pSchMtrRlt->bMtr, 6, szMtr), HexToStr(pSchMtrRlt->bAcqAddr, 6, szAcq), pSchMtrRlt->bMtrPro));
	}
	else
	{
		DTRACE(DB_CCT_SCH, ("[Update to 0x6002] RptMeter:%s, DbMeter:%s, RptAcq:%s, DbAcq:%s, MtrPro:%d.\n", \
			HexToStr(pSchMtrRlt->bMtr, 6, szMtr), HexToStr(tDbSchMtrRlt.bMtr, 6, szDbMtr), \
			HexToStr(pSchMtrRlt->bAcqAddr, 6, szAcq),  HexToStr(tDbSchMtrRlt.bAcqAddr, 6, szDbAcq), pSchMtrRlt->bMtrPro));
	}

	if (m_TSchMtrParm.fAutoUpdMtr)
		UpdataSchMtrToSysDb(pSchMtrRlt);
}

//������дһ���ѱ���()
//������	@pbBuf �ѱ���
bool CCctSchMeter::SaveOneSchMtrResult(TSchMtrRlt *pSchMtrRlt)
{
	TSchMtrRlt tSchMtrRlt;
	char szTabName[64];
	int iFileLen;
	int iMod;
	WORD wRltNum;
	WORD wIndex;

	memset(szTabName, 0, sizeof(szTabName));
	MK_SCH_MTR_FILE(szTabName);
	iFileLen = GetFileLen(szTabName);
	if (iFileLen < 0)
	{
		iFileLen = 0;
		PartWriteFile(szTabName, iFileLen, (BYTE*)pSchMtrRlt, PER_RLT_LEN);
	}
	else
	{
		iMod = iFileLen%PER_RLT_LEN;	//�޷���������ʾ�ļ�����
		if (iMod != 0)
		{
			DTRACE(DB_CCT_SCH, ("Search meter file error, iFileLen=%d, PerLen=%d, iMod=%d.\n", iFileLen, PER_RLT_LEN, iMod));
			DeleteFile(szTabName);
			return -1;
		}

		wRltNum = iFileLen/PER_RLT_LEN;
		//�ȼ�����wRltNum���Ƿ���ڿյ�λ��
		for (wIndex=0; wIndex<wRltNum; wIndex++)
		{
			memset((BYTE*)&tSchMtrRlt, 0, PER_RLT_LEN);
			if (PartReadFile(szTabName, wIndex*PER_RLT_LEN, (BYTE*)&tSchMtrRlt, PER_RLT_LEN))
			{
				if (IsAllAByte(tSchMtrRlt.bMtr, 0, 6) && IsAllAByte(tSchMtrRlt.bAcqAddr, 0x00, 6))	//���ַΪȫ0�� �ɼ�����ַΪȫ0���ж�Ϊ�յ�λ��
				{
					PartWriteFile(szTabName, wIndex*PER_RLT_LEN, (BYTE*)pSchMtrRlt, PER_RLT_LEN);
					SetSchMtrEvtMask(wIndex, true);
					return true;
				}
			}
		}

		PartWriteFile(szTabName, iFileLen, (BYTE*)pSchMtrRlt, PER_RLT_LEN);
		SetSchMtrEvtMask(wRltNum, true);
	}

	return true;
}

//�������滻һ���ѱ���
//������@wIdx ����
//		@pSchMtrRlt Ҫ�滻�ĵ���
bool CCctSchMeter::ReplaceOneSchMtrResult(WORD wIdx, TSchMtrRlt *pSchMtrRlt)
{
	char szTabName[64];
	int iFileLen;
	int iMod;

	memset(szTabName, 0, sizeof(szTabName));
	MK_SCH_MTR_FILE(szTabName);
	iFileLen = GetFileLen(szTabName);
	if (iFileLen < 0)
	{
		iFileLen = 0;
		PartWriteFile(szTabName, iFileLen, (BYTE*)pSchMtrRlt, PER_RLT_LEN);
		SetSchMtrEvtMask(wIdx, true);
	}
	else
	{
		iMod = iFileLen%PER_RLT_LEN;	//�޷���������ʾ�ļ�����
		if (iMod != 0)
		{
			DTRACE(DB_CCT_SCH, ("ReplaceOneSchMtrResult(): Replace meter file error, iFileLen=%d, PerLen=%d, iMod=%d.\n", iFileLen, PER_RLT_LEN, iMod));
			DeleteFile(szTabName);
			return false;
		}

		iFileLen = wIdx*PER_RLT_LEN;
		PartWriteFile(szTabName, iFileLen, (BYTE*)pSchMtrRlt, PER_RLT_LEN);

		SetSchMtrEvtMask(wIdx, true);
	}

	return true;
}

void CCctSchMeter::DelSchMtrResult(WORD wIdx)
{
	TSchMtrRlt tSchMtrRlt;
	char szTabName[64];
	int iOffset;

	memset((BYTE*)&tSchMtrRlt, 0, sizeof(tSchMtrRlt));
	memset(szTabName, 0, sizeof(szTabName));
	MK_SCH_MTR_FILE(szTabName);
	iOffset = wIdx*PER_RLT_LEN;
	PartWriteFile(szTabName, iOffset, (BYTE*)&tSchMtrRlt, PER_RLT_LEN);
}

//�����������ѱ����Ƿ�ȱ��
//������@pSchMtrRlt �ѱ�����
//��ע�����滻ԭ�еĵ���֮�������������
//	1.���滻���ַ��ԭ�ɼ������������ڣ���Ѱ��һ����λ�ñ���ԭ�ɼ�����ʽΪACQ+NULL�Ĳɼ���
//  2.���ݱ��δ���Ĳ����������ն��Ƿ��и�ʽΪACQ+NULL�Ĳɼ����������о�ɾ��
void  CCctSchMeter::LoopSchMtrResult(TSchMtrRlt *pSchMtrRlt)
{
	TSchMtrRlt tDbSchMtrRlt, tSchMtrRlt;
	int index, iStart;
	bool fIsSameAcq = false;

	if (IsAllAByte(pSchMtrRlt->bAcqAddr, 0, pSchMtrRlt->bAcqLen) || IsAllAByte(pSchMtrRlt->bAcqAddr, 0xee, pSchMtrRlt->bAcqLen))
		return;

	memcpy((BYTE*)&tSchMtrRlt, pSchMtrRlt, sizeof(TSchMtrRlt));
	
	iStart = -1;
	do 
	{
		memset((BYTE*)&tDbSchMtrRlt, 0, sizeof(tDbSchMtrRlt));
		index = GetSchMtrResult(&iStart, &tDbSchMtrRlt);
		if (index == -1)	//�ѽ���
			break;

		if (memcmp(tDbSchMtrRlt.bAcqAddr, tSchMtrRlt.bAcqAddr, tSchMtrRlt.bAcqLen) == 0)	//�ɼ���ַ���
		{
			fIsSameAcq = true;
			if (IsAllAByte(tDbSchMtrRlt.bMtr, 0, tDbSchMtrRlt.bMtrLen) || IsAllAByte(tDbSchMtrRlt.bMtr, 0xee, tDbSchMtrRlt.bMtrLen))	//�ն���ı��ַΪNULL
			{
				DelSchMtrResult(index);	//�ն���pSchMtrRlt�ı��ַΪNULL��pSchMtrRlt��tDbSchMtrRlt�Ĳɼ�����ַ��ȣ�ֱ��ɾ���ն���pSchMtrRlt�ĵ���
				break;
			}
		}

// 		//�ɼ�����Ӧ�ı��ַ�Ƿ�ΪNULL
// 		if (IsAllAByte(tDbSchMtrRlt.bMtr, 0, tDbSchMtrRlt.bMtrLen) || IsAllAByte(tDbSchMtrRlt.bMtr, 0xee, tDbSchMtrRlt.bMtrLen))	//�ն���ı��ַΪNULL
// 		{
// 			if (memcmp(tDbSchMtrRlt.bAcqAddr, tSchMtrRlt.bAcqAddr, tSchMtrRlt.bAcqLen) == 0)	//�ɼ���ַ���
// 			{
// 				DelSchMtrResult(index);	//�ն���pSchMtrRlt�ı��ַΪNULL��pSchMtrRlt��tDbSchMtrRlt�Ĳɼ�����ַ��ȣ�ֱ��ɾ���ն���pSchMtrRlt�ĵ���
// 				fIsSameAcq = true;
// 				break;
// 			}
// 		}
	}while (iStart != -1);

	//������fIsSameAcq=false������pSchMtrRlt���ն�tDbSchMtrRltû����ͬ�Ĳɼ�������ҪѰ��һ���յ�λ�ñ���õ���
	if (!fIsSameAcq)
	{
		memset(tSchMtrRlt.bMtr, 0, 6);
		tSchMtrRlt.bMtrLen = 0;
		GetCurTime(&tSchMtrRlt.tSchMtrSucTime);
		SaveOneSchMtrResult(&tSchMtrRlt);
	}

}

void CCctSchMeter::LoopMtrSysDb(TOobMtrInfo tMtrInfo)
{
	bool fIsSameAcq = false;
	TOobMtrInfo tDbMtrInfo;

	if (IsAllAByte(tMtrInfo.bAcqTsa, 0, tMtrInfo.bAcqTsaLen) || IsAllAByte(tMtrInfo.bAcqTsa, 0xee, tMtrInfo.bAcqTsaLen))
		return;

	for (WORD wPn=0; wPn<POINT_NUM; wPn++)
	{
		memset((BYTE*)&tDbMtrInfo, 0, sizeof(tDbMtrInfo));
		if (!GetMeterInfo(wPn, &tDbMtrInfo))
			continue;

		if ((IsAllAByte(tDbMtrInfo.bTsa, 0, tDbMtrInfo.bTsaLen) || IsAllAByte(tDbMtrInfo.bTsa, 0xee, tDbMtrInfo.bTsaLen))
			&& (IsAllAByte(tDbMtrInfo.bAcqTsa, 0, tDbMtrInfo.bAcqTsaLen) || IsAllAByte(tDbMtrInfo.bAcqTsa, 0xee, tDbMtrInfo.bAcqTsaLen)))
			continue;

		//�ɼ�����Ӧ�ı��ַ�Ƿ�ΪNULL
		if (memcmp(tDbMtrInfo.bAcqTsa, tMtrInfo.bAcqTsa, tMtrInfo.bAcqTsaLen) == 0)	//�ɼ���ַ���
		{	
			fIsSameAcq = true;
			if (IsAllAByte(tDbMtrInfo.bTsa, 0, tDbMtrInfo.bTsaLen) || IsAllAByte(tDbMtrInfo.bTsa, 0xee, tDbMtrInfo.bTsaLen))	//�ն���ı��ַΪNULL
			{
				DelMeterInfo(wPn);	//�ն���pSchMtrRlt�ı��ַΪNULL��pSchMtrRlt��tDbSchMtrRlt�Ĳɼ�����ַ��ȣ�ֱ��ɾ���ն���pSchMtrRlt�ĵ���
				InitMtrMask();
				//break;
			}
		}
	}

	//������fIsSameAcq=false������pSchMtrRlt���ն�tDbSchMtrRltû����ͬ�Ĳɼ�������ҪѰ��һ���յ�λ�ñ���õ���
#ifndef  VER_ZJ   //�㽭�ѱ���Ҫ������ǰ���ڵĲɼ�������  changed by whr 20170811
	if (!fIsSameAcq)
	{
		TSchMtrRlt tSchMtrRlt;

		memset((BYTE*)&tSchMtrRlt, 0, sizeof(tSchMtrRlt));
		MtrInfoConvertSchMtrRlt(&tMtrInfo, &tSchMtrRlt);
		InitMtrMask();

		const BYTE *pbMtrMask = GetMtrMask(BANK17, PN0, 0x6001);
		SearchEmptySaveMeter(&tSchMtrRlt, (BYTE*)pbMtrMask);
	}
#endif
}

//�������洢�ѱ���()
//������	@pbBuf �ѱ���
int CCctSchMeter::SaveSchMtrResult(DWORD dwPortOad, BYTE *pbBuf, WORD wLen, BYTE bMtrAddrLen)
{
	TSchMtrRlt tSchMtrRlt;
	char szCurTime[32];
	BYTE *p = pbBuf;
	BYTE bRptNum;
	BYTE bSlvNum;
	BYTE bDevType;	//00H���ɼ�����01H�����ܱ�02H��FFH ����
	BYTE bTotalNodeNum = 0;  //

	memset((BYTE*)&tSchMtrRlt, 0, sizeof(TSchMtrRlt));
	tSchMtrRlt.dwPort = dwPortOad;
	WaitSemaphore(m_tSchMtrSem);
	bRptNum = *p++;	//�ϱ��ӽڵ������n
	for (BYTE i=0; i<bRptNum; i++)
	{
		bDevType = p[9];
		if (bDevType == 0x00)	//�ɼ���
		{
			revcpy(tSchMtrRlt.bAcqAddr, p, 6);
			tSchMtrRlt.bAcqLen = 6;
			p += 6;
			tSchMtrRlt.bMtrPro = *p++;
			p += 3;	//�ӽڵ����(2) + �ӽڵ��豸����(1)
			bTotalNodeNum = *p++;	//�ӽڵ��½Ӵӽڵ�����M
			bSlvNum = *p++;
			if (bTotalNodeNum == 0)   //add a virtual meter
			{
				tSchMtrRlt.bMtrLen = bMtrAddrLen;	
				memset(tSchMtrRlt.bMtr, 0xee, sizeof(tSchMtrRlt.bMtr));
				tSchMtrRlt.bMtrPro = 0;
				GetCurTime(&tSchMtrRlt.tSchMtrSucTime);
				memset(szCurTime, 0, sizeof(szCurTime));
				DTRACE(DB_CCT_SCH, ("SaveSchMtrResult(): sampler no search meter. Cur time1=%s.\n", TimeToStr(tSchMtrRlt.tSchMtrSucTime, szCurTime)));
				CheckMtrAddr(&tSchMtrRlt);
				memset(szCurTime, 0, sizeof(szCurTime));
				DTRACE(DB_CCT_SCH, ("SaveSchMtrResult(): sampler no search meter. Cur time2=%s.\n", TimeToStr(tSchMtrRlt.tSchMtrSucTime, szCurTime)));
			}
			else {
				for (BYTE k=0; k<bSlvNum; k++)
				{
					tSchMtrRlt.bMtrLen = bMtrAddrLen;	
					revcpy(tSchMtrRlt.bMtr, p, tSchMtrRlt.bMtrLen);
					p += tSchMtrRlt.bMtrLen;
					tSchMtrRlt.bMtrPro = *p++;
					GetCurTime(&tSchMtrRlt.tSchMtrSucTime);
					memset(szCurTime, 0, sizeof(szCurTime));
					DTRACE(DB_CCT_SCH, ("SaveSchMtrResult(): Cur time1=%s.\n", TimeToStr(tSchMtrRlt.tSchMtrSucTime, szCurTime)));
					CheckMtrAddr(&tSchMtrRlt);
					memset(szCurTime, 0, sizeof(szCurTime));
					DTRACE(DB_CCT_SCH, ("SaveSchMtrResult(): Cur time2=%s.\n", TimeToStr(tSchMtrRlt.tSchMtrSucTime, szCurTime)));
				}
			}

		}
		else	//���
		{
			memset(tSchMtrRlt.bAcqAddr, 0, TSA_LEN);
			tSchMtrRlt.bAcqLen = 0;
			revcpy(tSchMtrRlt.bMtr, p, bMtrAddrLen);
			tSchMtrRlt.bMtrLen = bMtrAddrLen;
			p += bMtrAddrLen;
			tSchMtrRlt.bMtrPro = *p++;
			p += 3;	//�ӽڵ����(2) + �ӽڵ��豸����(1)
			GetCurTime(&tSchMtrRlt.tSchMtrSucTime);
			memset(szCurTime, 0, sizeof(szCurTime));
			DTRACE(DB_CCT_SCH, ("SaveSchMtrResult(): Cur time1=%s.\n", TimeToStr(tSchMtrRlt.tSchMtrSucTime, szCurTime)));
			CheckMtrAddr(&tSchMtrRlt);
			memset(szCurTime, 0, sizeof(szCurTime));
			DTRACE(DB_CCT_SCH, ("SaveSchMtrResult(): Cur time2=%s.\n", TimeToStr(tSchMtrRlt.tSchMtrSucTime, szCurTime)));

		}
	}

	CalSchMtrNum();

	SignalSemaphore(m_tSchMtrSem);

	return p - pbBuf;
}

//��������ȡ�ѱ���
//������@piStart �״δ���Ϊ-1���ѱ���δ��ȡ��ͷ�����Ӧ��ֵ����ȡ��������0xFFFE
//���أ� -1������>0���ζ�ȡ�Ĵ���
int CCctSchMeter::GetSchMtrResult(int *piStart, TSchMtrRlt *pSchMtrRlt, bool fGetAll)
{
	char szTabName[64]={0};
	WORD wRltNum;
	int iIndex;
	int iFileLen;
	int iMod;

	if (fGetAll)
	{
		if(*piStart == -1)
			*piStart = 0;
		else
			*piStart = *piStart + 1;
	}

	MK_SCH_MTR_FILE(szTabName);
	iFileLen = GetFileLen(szTabName);
	if (iFileLen == -1)
		return -1;

	iMod = iFileLen%PER_RLT_LEN;	//�޷���������ʾ�ļ�����
	if (iMod != 0)
	{
		DTRACE(DB_CCT_SCH, ("GetSchMtrResult():Search meter file error, iFileLen=%d, PerLen=%d, iMod=%d.\n", iFileLen, PER_RLT_LEN, iMod));
		DeleteFile(szTabName);
		return -1;
	}

	wRltNum = iFileLen/PER_RLT_LEN;
	for (iIndex=*piStart; iIndex<wRltNum; iIndex++)
	{
		memset((BYTE*)pSchMtrRlt, 0, PER_RLT_LEN);
		if (PartReadFile(szTabName, iIndex*PER_RLT_LEN, (BYTE*)pSchMtrRlt, PER_RLT_LEN)
			&& !IsAllAByte((BYTE*)pSchMtrRlt, 0, PER_RLT_LEN))
		{
			break;
		}
	}

	if (iIndex >= wRltNum)
		*piStart = -1;
	else
		*piStart = iIndex;

	return *piStart;
}

//��������ȡ�ѱ���
//������@piStart �״δ���Ϊ-1���ѱ���δ��ȡ��ͷ�����Ӧ��ֵ����ȡ��������0xFFFE
//���أ� -1������>0���ζ�ȡ�Ĵ���
int CCctSchMeter::GetSchMtrResult(int *piStart, BYTE *pbBuf, WORD wMaxLen, DWORD dwStartSec, DWORD dwEndSec)
{
	TSchMtrRlt tSchMtrRlt;
	char szTabName[64]={0};
	WORD wRltNum, wIndex;
	int iFileLen;
	int iMod;
	BYTE *p = pbBuf;
	WORD wRdNum, wCnt;
	DWORD dwSchMtrSec;

	if(*piStart == -1)
		*piStart = 0;

	wCnt = 0;
	wRdNum = wMaxLen/PER_RLT_LEN;
		
	MK_SCH_MTR_FILE(szTabName);
	iFileLen = GetFileLen(szTabName);
	if (iFileLen == -1)
	{
		*p++ = DT_ARRAY;
		*p++ = 0x00;
		*piStart = -1;

		return p - pbBuf;
	}
	
	*p++ = DT_ARRAY;
	p++;	//����Arry�еĸ���

	iMod = iFileLen%PER_RLT_LEN;	//�޷���������ʾ�ļ�����
	if (iMod != 0)
	{
		DTRACE(DB_CCT_SCH, ("GetSchMtrResult():Search meter file error, iFileLen=%d, PerLen=%d, iMod=%d.\n", iFileLen, PER_RLT_LEN, iMod));
		DeleteFile(szTabName);
		return -1;
	}

	wRltNum = iFileLen/PER_RLT_LEN;
	for (wIndex=*piStart; wIndex<wRltNum; wIndex++)
	{
		memset((BYTE*)&tSchMtrRlt, 0, PER_RLT_LEN);
		if (PartReadFile(szTabName, wIndex*PER_RLT_LEN, (BYTE*)&tSchMtrRlt, PER_RLT_LEN)
			&& !IsAllAByte((BYTE*)&tSchMtrRlt, 0, PER_RLT_LEN))
		{
			if (dwStartSec!=0 && dwEndSec!=0)
			{
				dwSchMtrSec = TimeToSeconds(tSchMtrRlt.tSchMtrSucTime);
				if (dwSchMtrSec<dwStartSec && dwSchMtrSec>dwEndSec)
					continue;
			}

			if (++wCnt >= wRdNum)
				break;

			//һ���ѱ���
			*p++ = DT_STRUCT;
			*p++ = 0x07;

			//ͨ�ŵ�ַ
			if (tSchMtrRlt.bMtrLen == 0)
			{
				*p++ = DT_TSA;
				*p++ = 0x07;
				*p++ = 0x05;
				memset(p, 0x00, 6);
				p += 6;
			}
			else
			{
				*p++ = DT_TSA;
				*p++ = tSchMtrRlt.bMtrLen + 1;
				*p++ = tSchMtrRlt.bMtrLen - 1;
				memcpy(p, tSchMtrRlt.bMtr, tSchMtrRlt.bMtrLen);
				p += tSchMtrRlt.bMtrLen;
			}

			if (tSchMtrRlt.bAcqLen == 0)
			{
				*p++ = DT_TSA;
				*p++ = 0x07;
				*p++ = 0x05;
				memset(p, 0x00, 6);
				p += 6;
			}
			else
			{
				//�����ɼ�����ַ
				*p++ = DT_TSA;
				*p++ = tSchMtrRlt.bAcqLen + 1;
				*p++ = tSchMtrRlt.bAcqLen - 1;
				memcpy(p, tSchMtrRlt.bAcqAddr, tSchMtrRlt.bAcqLen);
				p += tSchMtrRlt.bAcqLen;
			}
			//��Լ���� 
			*p++ = DT_ENUM;
			*p++ = tSchMtrRlt.bMtrPro;

			//��λ
			*p++ = DT_ENUM;
			*p++ = tSchMtrRlt.bPhase;

			//�ź�Ʒ��
			*p++ = DT_UNSIGN;
			*p++ = tSchMtrRlt.bSQ;

			//�ѵ���ʱ��
			*p++ = DT_DATE_TIME_S;
			p += OoTimeToDateTimeS(&tSchMtrRlt.tSchMtrSucTime, p);

			//�ѵ��ĸ�����Ϣ
			*p++ = DT_ARRAY;
			*p++ = 0;
		}
	}

	if (wIndex == wRltNum)
		*piStart = -1;
	else
		*piStart = wIndex;

	pbBuf[1] = wCnt;

	return p - pbBuf;
}


//��������ȡ�ѱ���
//������@piStart �״δ���Ϊ-1���ѱ���δ��ȡ��ͷ�����Ӧ��ֵ����ȡ��������0xFFFE
//���أ� -1������>0���ζ�ȡ�Ĵ���
int CCctSchMeter::GetCrossSchMtrResult(int *piStart, BYTE *pbBuf, WORD wMaxLen)
{
	TCrossSchMtrRlt tSchMtrRlt;
	char szTabName[64]={0};
	WORD wRltNum, wIndex;
	int iFileLen;
	int iMod;
	BYTE *p = pbBuf;
	WORD wRdNum, wCnt;

	if(*piStart == -1)
		*piStart = 0;

	wCnt = 0;
	wRdNum = wMaxLen/PER_CROSS_RLT_LEN;

	MK_CROSS_SCH_MTR_FILE(szTabName);
	iFileLen = GetFileLen(szTabName);
	if (iFileLen == -1)
	{
		*p++ = DT_ARRAY;
		*p++ = 0x00;
		*piStart = -1;

		return p - pbBuf;
	}

	*p++ = DT_ARRAY;
	p++;	//����Arry�еĸ���

	iMod = iFileLen%PER_CROSS_RLT_LEN;	//�޷���������ʾ�ļ�����
	if (iMod != 0)
	{
		DTRACE(DB_CCT_SCH, ("GetCrossSchMtrResult():Search meter file error, iFileLen=%d, PerLen=%d, iMod=%d.\n", iFileLen, PER_RLT_LEN, iMod));
		DeleteFile(szTabName);
		return -1;
	}

	wRltNum = iFileLen/PER_CROSS_RLT_LEN;
	for (wIndex=*piStart; wIndex<wRltNum; wIndex++)
	{
		memset((BYTE*)&tSchMtrRlt, 0, PER_CROSS_RLT_LEN);
		if (PartReadFile(szTabName, wIndex*PER_CROSS_RLT_LEN, (BYTE*)&tSchMtrRlt, PER_CROSS_RLT_LEN)
			&& !IsAllAByte((BYTE*)&tSchMtrRlt, 0, PER_CROSS_RLT_LEN))
		{
			if (++wCnt >= wRdNum)
				break;

			//һ����̨�����
			*p++ = DT_STRUCT;
			*p++ = 0x03;

			//ͨ�ŵ�ַ
			*p++ = DT_TSA;
			*p++ = 6+1;
			*p++ = 6-1;
			memcpy(p, tSchMtrRlt.bMtrAddr, 6);
			p += 6;
			//���ڵ��ַ
			*p++ = DT_TSA;
			*p++ = 6+1;
			*p++ = 6-1;
			memcpy(p, tSchMtrRlt.bMainAddr, 6);
			p += 6;

			//���ʱ��
			*p++ = DT_DATE_TIME_S;
			p += OoTimeToDateTimeS(&tSchMtrRlt.tUpdTime, p);
		}
	}

	if (wIndex == wRltNum)
		*piStart = -1;
	else
		*piStart = wIndex;

	pbBuf[1] = wCnt;

	return p - pbBuf;
}

//����������̨���ѱ�����¼��
int CCctSchMeter::CalSchMtrNum()
{
	char szTabName[64]={0};
	BYTE bBuf[8];
	WORD wRltNum;
	int iFileLen;
	int iMod;


	MK_SCH_MTR_FILE(szTabName);
	iFileLen = GetFileLen(szTabName);

	iMod = iFileLen%PER_RLT_LEN;	//�޷���������ʾ�ļ�����
	if (iMod != 0)
	{
		DTRACE(DB_CCT_SCH, ("CalSchMtrNum:Search meter file error, iFileLen=%d, PerLen=%d, iMod=%d.\n", iFileLen, PER_RLT_LEN, iMod));
		DeleteFile(szTabName);
		return -1;
	}

	wRltNum = iFileLen/PER_RLT_LEN;

	memset(bBuf, 0, sizeof(bBuf));
	bBuf[0] = DT_LONG_U;
	bBuf[1] = wRltNum/256;
	bBuf[2] = wRltNum%256;
	WriteItemEx(BANK0, PN0, 0x6004, bBuf);

	DTRACE(DB_CCT_SCH, ("CalSchMtrNum():Search meter Num=%d.\n", wRltNum));

	return wRltNum;
}

//������дһ����̨���ѱ���
//������	@pbBuf �ѱ���
int CCctSchMeter::SaveCrossSchMtrResult(BYTE *pbBuf, WORD wLen)
{
	BYTE bFrmLen;

	TCrossSchMtrRlt tRlt;	//��̨���ѱ���

	pbBuf++;	//�ӽڵ��豸����
	pbBuf++;	//ͨ��Э������
	bFrmLen = *pbBuf++;	//���ĳ���L
	if ((pbBuf[0]!=0x68 || pbBuf[7]!=0x68) || (bFrmLen<18))
		return -1;

	//68H	A0	����	A5	68H	9EH	07H	03H	PA0	����	PA5	CS	16H
	memcpy(tRlt.bMtrAddr, pbBuf+1, 6);
	memcpy(tRlt.bMainAddr, pbBuf+11, 6);
	GetCurTime(&tRlt.tUpdTime);

	return SaveOneCrossSchMtrResult(&tRlt);
}

int CCctSchMeter::SaveOneCrossSchMtrResult(TCrossSchMtrRlt *pSchMtrRlt)
{
	TSchMtrRlt tSchMtrRlt;
	char szTabName[64];
	int iFileLen;
	int iMod;
	WORD wRltNum;
	bool fSameMtrFlg = false;

	memset(szTabName, 0, sizeof(szTabName));
	MK_CROSS_SCH_MTR_FILE(szTabName);
	iFileLen = GetFileLen(szTabName);
	if (iFileLen < 0)
	{
		iFileLen = 0;
		PartWriteFile(szTabName, iFileLen, (BYTE*)pSchMtrRlt, PER_CROSS_RLT_LEN);
	}
	else
	{
		iMod = iFileLen%PER_CROSS_RLT_LEN;	//�޷���������ʾ�ļ�����
		if (iMod != 0)
		{
			DTRACE(DB_CCT_SCH, ("Search cross meter file error, iFileLen=%d, PerLen=%d, iMod=%d.\n", iFileLen, PER_CROSS_RLT_LEN, iMod));
			DeleteFile(szTabName);
			return -1;
		}

		wRltNum = iFileLen/PER_CROSS_RLT_LEN;
		for (WORD i=0; i<wRltNum; i++)
		{
			memset((BYTE*)&tSchMtrRlt, 0, PER_CROSS_RLT_LEN);
			if (PartReadFile(szTabName, i*PER_CROSS_RLT_LEN, (BYTE*)&tSchMtrRlt, PER_CROSS_RLT_LEN))
			{
				if (memcmp((BYTE*)&tSchMtrRlt, (BYTE*)&pSchMtrRlt, PER_CROSS_RLT_LEN) == 0)
				{
					fSameMtrFlg = true;
					break;
				}
			}
		}
		if (!fSameMtrFlg)
		{
			PartWriteFile(szTabName, iFileLen, (BYTE*)pSchMtrRlt, PER_CROSS_RLT_LEN);
		}
	}

	return true;
}

void CCctSchMeter::UpdataSchMtrToSysDb(TSchMtrRlt *pSchMtrRlt)
{
	char szDbAcqBuf[32] = {0};
	char szRptAcqBuf[32] = {0};
	char szDbMtrBuf[32] = {0};
	char szRptMtrBuf[32] = {0};
	const BYTE *pbMtrMask = GetMtrMask(BANK17, PN0, 0x6001);
	WORD wPnMask, wPn;
	BYTE bAddrState = 0x00;

	TOobMtrInfo tRptMtrInfo;

	memset((BYTE*)&tRptMtrInfo, 0, sizeof(tRptMtrInfo));
	SchMtrRltConvertMtrInfo(pSchMtrRlt, &tRptMtrInfo);

	//��·���ϱ��ĵ�ַ�����ж���������������
	if ((IsAllAByte(pSchMtrRlt->bMtr, 0x00, 6) || IsAllAByte(pSchMtrRlt->bMtr, 0xee, 6)) && !IsAllAByte(pSchMtrRlt->bAcqAddr, 0, 6))	//���ɼ�����ַ������ַΪ��
		bAddrState = SINGLE_ACQ_ADDR;
	else if (!IsAllAByte(pSchMtrRlt->bMtr, 0x00, 6) && (IsAllAByte(pSchMtrRlt->bAcqAddr, 0x00, 6) || IsAllAByte(pSchMtrRlt->bAcqAddr, 0xee, 6)))	//������ַ,�ɼ�����ַΪ��
		bAddrState = SINGLE_MTR_ADDR;
	else //�ɼ���+����ַ
		bAddrState = ACQ_MTR_ADDR;

	for (wPnMask=0; wPnMask<PN_MASK_SIZE; wPnMask++)
	{
		if (pbMtrMask[wPnMask] != 0)
		{
			for (BYTE m=0; m<8; m++)
			{
				if (pbMtrMask[wPnMask] & (1<<m))
				{
					TOobMtrInfo tDbMtrInfo, tBakDbMtrInfo;
					wPn = wPnMask*8 + m;
					memset((BYTE*)&tDbMtrInfo, 0, sizeof(tDbMtrInfo));
					GetMeterInfo(wPn, &tDbMtrInfo);

					if (bAddrState == SINGLE_MTR_ADDR)
					{
						if (memcmp(pSchMtrRlt->bMtr, tDbMtrInfo.bTsa, 6) == 0)	//�ն��������ͬ�ĵ���ַ
						{
							//if (!IsAllAByte(tDbMtrInfo.bAcqTsa, 0x00, 6) && !IsAllAByte(tDbMtrInfo.bAcqTsa, 0xee, 6) && tDbMtrInfo.bAcqTsa[1]!=0xAA)	//�ն���Ĳɼ���ַ��ΪNULL���滻
							{
								DTRACE(DB_CCT_SCH, ("[Mtr+NULL to 0x6000] Replace one meter to SysDb: Meter addr: %s, Swap acq addr: %s-->%s.\n",\
									HexToStr(tDbMtrInfo.bTsa, 6, szDbMtrBuf), HexToStr(tDbMtrInfo.bAcqTsa, 6, szDbAcqBuf), HexToStr(pSchMtrRlt->bAcqAddr, 6, szRptAcqBuf)));

								memset((BYTE*)&tBakDbMtrInfo, 0, sizeof(tBakDbMtrInfo));
								tBakDbMtrInfo = tDbMtrInfo;
								tBakDbMtrInfo.bTsaLen = 0;
								memset(tBakDbMtrInfo.bTsa, 0, 6);

								memset(tDbMtrInfo.bAcqTsa, 0x00, 6);
								tDbMtrInfo.bAcqTsaLen = 0;
								SetMeterInfo(wPn, tDbMtrInfo);
								LoopMtrSysDb(tBakDbMtrInfo);
								LoopMtrSysDb(tRptMtrInfo);
								SetUdpMtrFlg(true);
							}
							goto Ret_UpdataOneSchMtrToDb;
						}
					}
					else if (bAddrState == SINGLE_ACQ_ADDR)
					{
						if (memcmp(pSchMtrRlt->bAcqAddr, tDbMtrInfo.bAcqTsa, 6) == 0)	//�ն��������ͬ�ɼ�����ַ���Ϳ���ֱ���˳�
						{
							DTRACE(DB_CCT_SCH, ("[NULL+MTR to 0x6000]  Same acq: %s.\n",HexToStr(pSchMtrRlt->bAcqAddr, 6, szDbAcqBuf)));
							goto Ret_UpdataOneSchMtrToDb;
						}
					}
					else	//�ɼ���+����ַ
					{
						if (memcmp(pSchMtrRlt->bAcqAddr, tDbMtrInfo.bAcqTsa, 6) == 0)	//�ɼ���ַ���
						{
							if (IsAllAByte(tDbMtrInfo.bTsa, 0xee, 6) || IsAllAByte(tDbMtrInfo.bTsa, 0x00, 6))	//���ַΪ�գ�ֱ�Ӵ���
							{
								DTRACE(DB_CCT_SCH, ("[MTR+ACQ to 0x6000] Difference meter:%s-->%s, same acq: %s.\n", HexToStr(pSchMtrRlt->bMtr, 6, szRptMtrBuf), HexToStr(tDbMtrInfo.bTsa, 6, szDbMtrBuf),HexToStr(tDbMtrInfo.bAcqTsa, 6, szDbAcqBuf)));
								
								memset((BYTE*)&tBakDbMtrInfo, 0, sizeof(tBakDbMtrInfo));
								tBakDbMtrInfo = tDbMtrInfo;
								tBakDbMtrInfo.bTsaLen = 0;
								memset(tBakDbMtrInfo.bTsa, 0, 6);

								memcpy(tDbMtrInfo.bTsa, pSchMtrRlt->bMtr, 6);
								tDbMtrInfo.bTsaLen = pSchMtrRlt->bMtrLen;
								tDbMtrInfo.bProType = pSchMtrRlt->bMtrPro;
								SetMeterInfo(wPn, tDbMtrInfo);
								LoopMtrSysDb(tBakDbMtrInfo);
								LoopMtrSysDb(tRptMtrInfo);
								SetUdpMtrFlg(true);
								goto Ret_UpdataOneSchMtrToDb;
							}
						}

						if ((memcmp(pSchMtrRlt->bMtr, tDbMtrInfo.bTsa, 6) == 0))	//����ַ��ͬ
						{
							if (memcmp(pSchMtrRlt->bAcqAddr, tDbMtrInfo.bAcqTsa, 6) != 0)	//�ɼ�����ַ����ͬ��ֱ���滻
							{
								DTRACE(DB_CCT_SCH, ("[MTR+ACQ to 0x6000] Same meter:%s, Difference acq: %s-->%s.\n", HexToStr(pSchMtrRlt->bMtr, 6, szRptMtrBuf), HexToStr(pSchMtrRlt->bAcqAddr, 6, szRptAcqBuf),HexToStr(tDbMtrInfo.bAcqTsa, 6, szDbAcqBuf)));
								
								memset((BYTE*)&tBakDbMtrInfo, 0, sizeof(tBakDbMtrInfo));
								tBakDbMtrInfo = tDbMtrInfo;
								tBakDbMtrInfo.bTsaLen = 0;
								memset(tBakDbMtrInfo.bTsa, 0, 6);
								
								memcpy(tDbMtrInfo.bAcqTsa, pSchMtrRlt->bAcqAddr, 6);
								tDbMtrInfo.bAcqTsaLen = pSchMtrRlt->bAcqLen;
								tDbMtrInfo.bProType = pSchMtrRlt->bMtrPro;
								SetMeterInfo(wPn, tDbMtrInfo);
								LoopMtrSysDb(tBakDbMtrInfo);
								LoopMtrSysDb(tRptMtrInfo);
								SetUdpMtrFlg(true);

								goto Ret_UpdataOneSchMtrToDb;
							}
							else
							{
								DTRACE(DB_CCT_SCH, ("[MTR+ACQ to 0x6000] Same meter:%s, Same acq: %s.\n",HexToStr(tDbMtrInfo.bTsa, 6, szDbMtrBuf), HexToStr(tDbMtrInfo.bAcqTsa, 6, szDbAcqBuf)));
								goto Ret_UpdataOneSchMtrToDb;
							}
						}
					}
				}
			}
		}
	}

Ret_UpdataOneSchMtrToDb:

	if (wPnMask >= PN_MASK_SIZE)	//����ϵͳ����û�е����������⣬Ѱ��һ����λ��
	{
		pbMtrMask = GetMtrMask(BANK17, PN0, 0x6001);
		SearchEmptySaveMeter(pSchMtrRlt, (BYTE*)pbMtrMask);
	}
}


//������Ѱ��һ���յı���Ŵ洢�ѻصı���
void CCctSchMeter::SearchEmptySaveMeter(TSchMtrRlt *pSchMtrRlt, BYTE *pbMtrMask)
{
	for (WORD k=0; k<PN_MASK_SIZE; k++)
	{
		//if (pbMtrMask[k] == 0)
		{
			for (BYTE m=0; m<8; m++)
			{
				if (!(pbMtrMask[k] & (1<<m)))
				{
					WORD wPn = k*8 + m;
					WORD wSn;
					BYTE bMtrBuf[PNPARA_LEN] = {0};
					BYTE *p = bMtrBuf+1; 

					if (wPn == 0) 
						continue;
					if (wPn<=10 && pSchMtrRlt->dwPort==0xf2090201)	//485��ΧΪ1~10���ز���ΧΪ11~POINT_NUM
						continue;
					 //GetEmptyPn()
					wSn = wPn;
					SetMtrSnToPn(wPn, wPn);
					//���õ�Ԫ
					*p++ = DT_STRUCT;
					*p++ = 0x04;
					//�������
					*p++ = DT_LONG_U;
					*p++ = wPn/256;
					*p++ = wPn%256;
					//������Ϣ
					*p++ = DT_STRUCT;
					*p++ = 0x0a;
					//ͨ�ŵ�ַ

					if (pSchMtrRlt->bMtrLen == 0)
					{
						*p++ = DT_TSA;
						*p++ = 0x07;
						*p++ = 0x05;
						memset(p, 0x00, 6);
						p += 6;
					}
					else
					{
						*p++ = DT_TSA;
						*p++ = pSchMtrRlt->bMtrLen + 1;
						*p++ = pSchMtrRlt->bMtrLen - 1;
						memcpy(p, pSchMtrRlt->bMtr, pSchMtrRlt->bMtrLen);
						p += pSchMtrRlt->bMtrLen;
					}

					//������
					*p++ = DT_ENUM;
					switch(pSchMtrRlt->bMtrPro)
					{
					case PRO_TYPE_97:
						*p++ = OOB_BPS_1200;
						break;
					case PRO_TYPE_07:
						*p++ = OOB_BPS_2400;
						break;
					case PRO_TYPE_69845:
						*p++ = OOB_BPS_2400;
						break;
					case PRO_TYPE_T188:
						*p++ = OOB_BPS_1200;
						break;
					default:	//PRO_TYPE_UNKNOW
						*p++ = OOB_BPS_OTHER;
					}
					//��Լ���� 
					*p++ = DT_ENUM;
					*p++ = pSchMtrRlt->bMtrPro;
					//�˿�
					*p++ = DT_OAD;
					p += OoDWordToOad(pSchMtrRlt->dwPort, p);
					//ͨ������
					*p++ = DT_OCT_STR;
					*p++ = 0x00;
					//���ʸ���
					*p++ = DT_UNSIGN;
					*p++ = 0x04;	//��ʱ����4����
					//�û����� 
					*p++ = DT_UNSIGN;
					*p++ = 0x00;
					//���߷�ʽ
					*p++ = DT_ENUM;
					*p++ = OOB_LINE_UNKNOW;
					//���ѹ
					*p++ = DT_LONG_U;
					*p++ = 0x00;
					*p++ = 0x00;
					//�����
					*p++ = DT_LONG_U;
					*p++ = 0x00;
					*p++ = 0x00;
					//��չ��Ϣ---------------------
					*p++ = DT_STRUCT;
					*p++ = 0x04;
					if (pSchMtrRlt->bAcqLen == 0)
					{
						*p++ = DT_TSA;
						*p++ = 0x07;
						*p++ = 0x05;
						memset(p, 0x00, 6);
						p += 6;
					}
					else
					{
						//�ɼ�����ַ
						*p++ = DT_TSA;
						*p++ = pSchMtrRlt->bAcqLen + 1;
						*p++ = pSchMtrRlt->bAcqLen - 1;
						memcpy(p, pSchMtrRlt->bAcqAddr, pSchMtrRlt->bAcqLen);
						p += pSchMtrRlt->bAcqLen;
					}
					//�ʲ���
					*p++ = DT_OCT_STR;
					*p++ = 0x00;
					//PT
					*p++ = DT_LONG_U;
					*p++ = 0;
					*p++ = 0;
					//CT
					*p++ = DT_LONG_U;
					*p++ = 0;
					*p++ = 0;
					//������Ϣ��
					*p++ = DT_ARRAY;
					*p++ = 0;

					bMtrBuf[0] = p - bMtrBuf - 1;

					char szMtr[32];
					char szAcq[32];
					DTRACE(DB_CCT_SCH, ("Add new meter to SysDb, wPn=%d, wSn=%d, Meter:%s, Acq:%s, MtrPro:%d.\n", 
						wPn, wSn,
						HexToStr(pSchMtrRlt->bMtr, 6, szMtr), 
						HexToStr(pSchMtrRlt->bAcqAddr, 6, szAcq), 
						pSchMtrRlt->bMtrPro));

					WriteItemEx(BANK0, wPn, 0x6000, bMtrBuf);
				
					pbMtrMask[k] |= (1<<m);
					WriteItemEx(BANK17, PN0, 0x6001, pbMtrMask);
					SetUdpMtrFlg(true);

					TrigerSaveBank(BN0, SECT_ACQ_MONI, -1);
					return;
				}
			}
		}
	}
}

//�����������ѱ��¼�
void CCctSchMeter::SaveAlarmEvent(TSchMtrRlt *pSchMtrRlt)
{
	BYTE bAlarmBuf[128] = {0};
	BYTE *p = bAlarmBuf;

	if (m_TSchMtrParm.fIsGenEvt)
	{
		//һ���ѱ���
		*p++ = DT_STRUCT;
		*p++ = 0x07;

		//ͨ�ŵ�ַ 
		*p++ = DT_TSA;
		*p++ = pSchMtrRlt->bMtrLen + 1;
		*p++ = pSchMtrRlt->bMtrLen - 1;
		memcpy(p, pSchMtrRlt->bMtr, pSchMtrRlt->bMtrLen);
		p += pSchMtrRlt->bMtrLen;
		
		//�ɼ���ַ
		*p++ = DT_TSA;
		*p++ = pSchMtrRlt->bAcqLen + 1;
		*p++ = pSchMtrRlt->bAcqLen - 1;
		memcpy(p, pSchMtrRlt->bAcqAddr, pSchMtrRlt->bAcqLen);
		p += pSchMtrRlt->bAcqLen;

		//��Լ����
		*p++ = DT_ENUM;
		*p++ = pSchMtrRlt->bMtrPro;

		//��λ
		*p++ = DT_ENUM;
		*p++ = pSchMtrRlt->bPhase;

		//�ź�Ʒ��
		*p++ = DT_ENUM;
		*p++ = pSchMtrRlt->bSQ;

		//�ѵ���ʱ��
		*p++ = DT_DATE_TIME_S;
		p += OoTimeToDateTimeS(&pSchMtrRlt->tSchMtrSucTime, p);

		//�ѵ��ĸ�����Ϣ
		*p++ = DT_ARRAY;
		*p++ = 0x00;

		//ǿ���ṩ�¼��ӿڡ�������
	}
}

void CCctSchMeter::SchMtrRltConvertMtrInfo(TSchMtrRlt *pSchMtrRlt, TOobMtrInfo *pMtrInfo)
{
	pMtrInfo->bProType = pSchMtrRlt->bMtrPro;
	pMtrInfo->bTsaLen = pSchMtrRlt->bMtrLen;
	memcpy(pMtrInfo->bTsa, pSchMtrRlt->bMtr, pMtrInfo->bTsaLen);
	pMtrInfo->bAcqTsaLen = pSchMtrRlt->bAcqLen;
	memcpy(pMtrInfo->bAcqTsa, pSchMtrRlt->bAcqAddr, pMtrInfo->bAcqTsaLen);
	pMtrInfo->dwPortOAD = pSchMtrRlt->dwPort;
}

void CCctSchMeter::MtrInfoConvertSchMtrRlt(TOobMtrInfo *pMtrInfo, TSchMtrRlt *pSchMtrRlt)
{
	pSchMtrRlt->bMtrPro = pMtrInfo->bProType ;
	pSchMtrRlt->bMtrLen = pMtrInfo->bTsaLen;
	memcpy(pSchMtrRlt->bMtr, pMtrInfo->bTsa, pMtrInfo->bTsaLen);
	pSchMtrRlt->bAcqLen = pMtrInfo->bAcqTsaLen;
	memcpy(pSchMtrRlt->bAcqAddr, pMtrInfo->bAcqTsa, pMtrInfo->bAcqTsaLen);
	pSchMtrRlt->dwPort = pMtrInfo->dwPortOAD;
}

void CCctSchMeter::DeleteSearchMtrFile()
{
	char szTabName[64]={0};

	MK_SCH_MTR_FILE(szTabName);
	DeleteFile(szTabName);
	DTRACE(DB_CCT_SCH, ("DeleteSearchMtrFile(): TableName=%s.\n", szTabName));
}

void CCctSchMeter::DeleteCrossSearchMtrFile()
{
	char szTabName[64]={0};

	MK_CROSS_SCH_MTR_FILE(szTabName);
	DeleteFile(szTabName);
	DTRACE(DB_CCT_SCH, ("DeleteCrossSearchMtrFile(): TableName=%s.\n", szTabName));
}

int CCctSchMeter::GetRightNowSchMtrKeepTime()
{
	
	BYTE bBuf[8] = {0};

	ReadItemEx(BANK0, PN0, 0x6009, bBuf);

	if (bBuf[0] != DT_LONG_U)
		return -1;

	return OoOiToWord(bBuf+1);
}

//����������ӽڵ�ȴ�ʱ��
int CCctSchMeter::GetNodeActWaitMin()
{
	int iActWaitMin;

	if (m_fRightNowSchMtr)	
	{
		if (m_bActCnt == 0)	//��һ��10����
			iActWaitMin = 10;
		else if (m_bActCnt == 1)	//�ڶ���20����
			iActWaitMin = 20;
		else	// �����εȴ�17����
			iActWaitMin = 17;

		m_bActCnt++;
	}
	else
	{
		iActWaitMin = 120;	////ʱ��Ϊ0����ʾ����ע�����ʱ�䲻���ƣ����120min
	}

	return iActWaitMin;
}

//���������µ������ʶ
void CCctSchMeter::SetUdpMtrFlg(bool fState)
{
	m_fUdpMtrToDB = fState;
}

//��������ȡ�������ʶ
bool CCctSchMeter::GetUdpMtrFlg()
{
	return m_fUdpMtrToDB;
}

//���������¸澯�¼��ź�������
//������@wIndex ����δ֪�������
//		@fState false:�������wIndex��Ӧ�ı�ʶ����֮
bool CCctSchMeter::SetSchMtrEvtMask(WORD wIndex, bool fState)
{
	BYTE bMask[PN_MASK_SIZE] = {0};

	if (!m_TSchMtrParm.fIsGenEvt)
		return true;

	if (wIndex >= PN_MASK_SIZE)
		return false;

	WaitSemaphore(m_tAlarmSem);
	ReadItemEx(BANK16, PN0, 0x6010, bMask);
	if (fState)
		bMask[wIndex/8] |= (1<<(wIndex%8));
	else
		bMask[wIndex/8] &= ~(1<<(wIndex%8));
	WriteItemEx(BANK16, PN0, 0x6010, bMask);
	SignalSemaphore(m_tAlarmSem);

	return true;
}


//���������¸澯�¼��ź�������
bool CCctSchMeter::UpdataSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen)
{
	if (wMaskLen < PN_MASK_SIZE)
		return false;

	WaitSemaphore(m_tAlarmSem);
	WriteItemEx(BANK16, PN0, 0x6010, pbMask);
	SignalSemaphore(m_tAlarmSem);

	return true;
}

//��������ȡ�澯�¼��ź�������
bool CCctSchMeter::GetSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen)
{
	if (wMaskLen < PN_MASK_SIZE)
		return false;

	WaitSemaphore(m_tAlarmSem);
	ReadItemEx(BANK16, PN0, 0x6010, pbMask);
	SignalSemaphore(m_tAlarmSem);
	return true;
}

//����������澯�¼�������
void CCctSchMeter::ClearSchMtrEvtMask()
{
	BYTE bMask[PN_MASK_SIZE] = {0};

	WaitSemaphore(m_tAlarmSem);
	WriteItemEx(BANK16, PN0, 0x6010, bMask);
	SignalSemaphore(m_tAlarmSem);
}

//������ͨ��������ȡ�澯����
//������@wIndex �澯����
//		@pbBuf ���صĸ澯��������
//���أ�-1��ȡ����ʧ�ܣ�>0�澯���ݵĳ���
int CCctSchMeter::GetSchMtrEvtData(int iIndex, BYTE *pbBuf)
{
	TSchMtrRlt tSchMtrRlt;
	BYTE *pbBuf0 = pbBuf;

	if (GetSchMtrResult(&iIndex, &tSchMtrRlt, false) < 0)
		return -1;

	//һ���ѱ���
	*pbBuf++ = DT_STRUCT;
	*pbBuf++ = 0x07;

	//ͨ�ŵ�ַ 
	if (tSchMtrRlt.bMtrLen == 0)
	{
		*pbBuf++ = DT_TSA;
		*pbBuf++ = 0x07;
		*pbBuf++ = 0x05;
		memset(pbBuf, 0x00, 6);
		pbBuf += 6;
	}
	else
	{
		*pbBuf++ = DT_TSA;
		*pbBuf++ = tSchMtrRlt.bMtrLen + 1;
		*pbBuf++ = tSchMtrRlt.bMtrLen - 1;
		memcpy(pbBuf, tSchMtrRlt.bMtr, tSchMtrRlt.bMtrLen);
		pbBuf += tSchMtrRlt.bMtrLen;
	}

	//�ɼ���ַ
	if (tSchMtrRlt.bAcqLen == 0)
	{
		*pbBuf++ = DT_TSA;
		*pbBuf++ = 0x07;
		*pbBuf++ = 0x05;
		memset(pbBuf, 0x00, 6);
		pbBuf += 6;
	}
	else
	{
		*pbBuf++ = DT_TSA;
		*pbBuf++ = tSchMtrRlt.bAcqLen + 1;
		*pbBuf++ = tSchMtrRlt.bAcqLen - 1;
		memcpy(pbBuf, tSchMtrRlt.bAcqAddr, tSchMtrRlt.bAcqLen);
		pbBuf += tSchMtrRlt.bAcqLen;
	}

	//��Լ����
	*pbBuf++ = DT_ENUM;
	*pbBuf++ = tSchMtrRlt.bMtrPro;
	//��λ
	*pbBuf++ = DT_ENUM;
	*pbBuf++ = tSchMtrRlt.bPhase;
	//�ź�Ʒ��
	*pbBuf++ = DT_ENUM;
	*pbBuf++ = tSchMtrRlt.bSQ;
	//�ѵ���ʱ��
	*pbBuf++ = DT_DATE_TIME_S;
	pbBuf += OoTimeToDateTimeS(&tSchMtrRlt.tSchMtrSucTime, pbBuf);
	//�ѵ��ĸ�����Ϣ
	*pbBuf++ = DT_ARRAY;
	*pbBuf++ = 0x00;

	return pbBuf - pbBuf0;
}

//������У���ϴ��ѱ�ʱ���������Ƿ񳬹�30��
void CCctSchMeter::CheckMeterSearchTime()
{
	TSchMtrRlt tMtrRlt;
	int iStart = -1;
	int index;
	DWORD dwCurTime, dwLastSchTime;
	DWORD dwDelTimeout;
	char szSchTime[64];
	char szCurTime[64];
	char szBuf[32] = {0};
	char szAcqBuf[32] = {0};
	
	dwDelTimeout = 30*24*60*60;	//30��
	dwCurTime = GetCurTime();

	do 
	{
		memset((BYTE*)&tMtrRlt, 0, sizeof(tMtrRlt));
		index = GetSchMtrResult(&iStart, &tMtrRlt);
		if (index == -1)
			break;
		dwLastSchTime = TimeToSeconds(tMtrRlt.tSchMtrSucTime);
		if (abs(dwCurTime-dwLastSchTime) > dwDelTimeout)
		{

			DTRACE(DB_CCT, ("Search meter over 30 days, del meter: %s, acq:%s, MtPro=%d, SearchTime=%s, CurTime=%s, IntervTime=%d.\n", \
				HexToStr(tMtrRlt.bMtr, tMtrRlt.bMtrLen, szBuf), 
				HexToStr(tMtrRlt.bAcqAddr, tMtrRlt.bAcqLen, szAcqBuf), tMtrRlt.bMtrPro, 
				TimeToStr(dwLastSchTime, szSchTime),
				TimeToStr(dwCurTime, szCurTime),
				abs(dwCurTime-dwLastSchTime)));

			int iPn = GetMeterPn(tMtrRlt.bMtr, tMtrRlt.bMtrLen);
			if (iPn > 0)
			{
				SetRdMtrCtrlMask(iPn);
			}

			DelMeterInfo(tMtrRlt.bMtr, tMtrRlt.bMtrLen);

			memset(tMtrRlt.bMtr, 0, sizeof(tMtrRlt.bMtr));
			tMtrRlt.bMtrLen = 0;
			memset(tMtrRlt.bAcqAddr, 0, sizeof(tMtrRlt.bAcqAddr));
			tMtrRlt.bAcqLen = 0;
			ReplaceOneSchMtrResult(index, &tMtrRlt);
			
		}
	}while (iStart != -1);
}
