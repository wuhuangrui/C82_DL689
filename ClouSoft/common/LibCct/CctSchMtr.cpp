/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CctRdCtrl.cpp
 * 摘    要：载波搜表
 * 当前版本：1.0
 * 作    者：CL
 * 完成日期：2016年11月
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

//描述：执行搜表
//返回：搜表是否结束
bool CCctSchMeter::DoSchMtrAddr()
{
	int iRet;

	if (GetInfo(INFO_SCH_MTR) && !m_fRightNowSchMtr)	//获得立即启动搜表
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
	else if(!m_fRightNowSchMtr && IsSchMtrPeriod() && !m_fPeriodSchMtr)	//时段搜表(时段搜表期间未启动立即搜表)
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

	if (m_bSchMtrState == SCH_MTR_EMPTY) //表示未执行搜表
	{
		m_fPeriodSchMtr = false;
		m_fRightNowSchMtr = false;
		return false;
	}

	return true;	//在搜表状态
}

//描述：获取搜表参数
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

//描述：是否在搜表时段参数(60020900)
//返回：>0表示在搜表时间范围内
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


//描述：设置搜表状态
//参数：true-搜表中，false-空闲
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

#define SINGLE_MTR_ADDR		(1<<0x00)	//单表地址，采集器地址为空
#define SINGLE_ACQ_ADDR		(1<<(0x01))	//单采集器地址，表地址为空
#define ACQ_MTR_ADDR		(1<<(0x02))	//表地址+采集器地址

//描述：校验表地址是否在终端档案中
//参数：@pSchMtrRlt 一个搜回的表档案
//备注：路由模块上报的地址有三种
//		1.单表地址
//			判断：1.1 返回的地址是否在当前的“采集器+电表地址”中，存在就删除原先的档案，更新搜回的档案
//		2.采集器+电表地址
//			判断：2.1 采集器地址变更、表地址未变更，需更新当前表地址对应的档案
//				  2.2 采集器地址未变更、表地址变更，终端是否存在相应的地址，否则更新档案
//				  2.3 采集器地址、表地址都是全新的，更新档案
//				  2.4 采集器地址、表地址都相同
//		3.采集器+空表地址
//				  3.1 系统中是否存在该采集器、表地址，不存在，更新档案
//返回：true-表示终端里存在相同的地址，无需更新档案
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
	
	//对路由上报的地址进行判定，属于哪种类型
	if ((IsAllAByte(pSchMtrRlt->bMtr, 0x00, 6) || IsAllAByte(pSchMtrRlt->bMtr, 0xee, 6)) && !IsAllAByte(pSchMtrRlt->bAcqAddr, 0, 6))	//单采集器地址，电表地址为空
		bAddrState = SINGLE_ACQ_ADDR;
	else if (!IsAllAByte(pSchMtrRlt->bMtr, 0x00, 6) && (IsAllAByte(pSchMtrRlt->bAcqAddr, 0x00, 6) || IsAllAByte(pSchMtrRlt->bAcqAddr, 0xee, 6)))	//单电表地址,采集器地址为空
		bAddrState = SINGLE_MTR_ADDR;
	else //采集器+电表地址
		bAddrState = ACQ_MTR_ADDR;

	do 
	{
		memset((BYTE*)&tDbSchMtrRlt, 0, sizeof(tDbSchMtrRlt));
		index = GetSchMtrResult(&iStart, &tDbSchMtrRlt);
		if (index == -1)
			break;

		if (bAddrState == SINGLE_MTR_ADDR)
		{
			if (memcmp(pSchMtrRlt->bMtr, tDbSchMtrRlt.bMtr, 6) == 0)	//终端里存在相同的电表地址
			{
				//if (!IsAllAByte(tDbSchMtrRlt.bAcqAddr, 0x00, 6) && !IsAllAByte(tDbSchMtrRlt.bAcqAddr, 0xee, 6))	//终端里的采集地址不为NULL，替换
					ReplaceOneSchMtrResult(index, pSchMtrRlt);
					LoopSchMtrResult(pSchMtrRlt);
					LoopSchMtrResult(&tDbSchMtrRlt);
				fIsExistSameMtr = true;
				break;
			}
		}
		else if (bAddrState == SINGLE_ACQ_ADDR)
		{
			if (memcmp(pSchMtrRlt->bAcqAddr, tDbSchMtrRlt.bAcqAddr, 6) == 0)	//终端里存在相同采集器地址，就可以直接退出
			{
// 				tDbSchMtrRlt.tSchMtrSucTime = pSchMtrRlt->tSchMtrSucTime;
// 				ReplaceOneSchMtrResult(index, &tDbSchMtrRlt);	//这里替换主要是为了更新搜表时间
				fIsExistSameMtr = true;
				break;
			}
		}
		else	//采集器+电表地址
		{
			if (memcmp(pSchMtrRlt->bAcqAddr, tDbSchMtrRlt.bAcqAddr, 6) == 0)	//采集地址相等
			{
				if ((IsAllAByte(tDbSchMtrRlt.bMtr, 0xee, 6) || IsAllAByte(tDbSchMtrRlt.bMtr, 0x00, 6))	//系统库中电表地址为空，直接代替
					|| (memcmp(pSchMtrRlt->bMtr, tDbSchMtrRlt.bMtr, 6) == 0))	//表地址相等，直接代替
				{
					ReplaceOneSchMtrResult(index, pSchMtrRlt);	//更新搜表时间、更新采集器地址
					LoopSchMtrResult(pSchMtrRlt);
					LoopSchMtrResult(&tDbSchMtrRlt);
					fIsExistSameMtr = true;
					break;
				}
			}

			if (memcmp(pSchMtrRlt->bMtr, tDbSchMtrRlt.bMtr, 6) == 0)	//电表地址相等，无论采集地址是否相等，都直接替换
			{
				ReplaceOneSchMtrResult(index, pSchMtrRlt);	//更新搜表时间、更新采集器地址
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

//描述：写一个搜表结果()
//参数：	@pbBuf 搜表结果
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
		iMod = iFileLen%PER_RLT_LEN;	//无法整除，表示文件出错
		if (iMod != 0)
		{
			DTRACE(DB_CCT_SCH, ("Search meter file error, iFileLen=%d, PerLen=%d, iMod=%d.\n", iFileLen, PER_RLT_LEN, iMod));
			DeleteFile(szTabName);
			return -1;
		}

		wRltNum = iFileLen/PER_RLT_LEN;
		//先检索在wRltNum中是否存在空的位置
		for (wIndex=0; wIndex<wRltNum; wIndex++)
		{
			memset((BYTE*)&tSchMtrRlt, 0, PER_RLT_LEN);
			if (PartReadFile(szTabName, wIndex*PER_RLT_LEN, (BYTE*)&tSchMtrRlt, PER_RLT_LEN))
			{
				if (IsAllAByte(tSchMtrRlt.bMtr, 0, 6) && IsAllAByte(tSchMtrRlt.bAcqAddr, 0x00, 6))	//表地址为全0， 采集器地址为全0就判定为空的位置
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

//描述：替换一个搜表结果
//参数：@wIdx 索引
//		@pSchMtrRlt 要替换的档案
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
		iMod = iFileLen%PER_RLT_LEN;	//无法整除，表示文件出错
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

//描述：检索搜表档案是否缺少
//参数：@pSchMtrRlt 搜表数据
//备注：在替换原有的档案之后会存在两种情况
//	1.在替换表地址后，原采集器档案不存在，需寻找一个空位置保存原采集，格式为ACQ+NULL的采集器
//  2.依据本次传入的参数，遍历终端是否有格式为ACQ+NULL的采集器档案，有就删掉
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
		if (index == -1)	//已结束
			break;

		if (memcmp(tDbSchMtrRlt.bAcqAddr, tSchMtrRlt.bAcqAddr, tSchMtrRlt.bAcqLen) == 0)	//采集地址相等
		{
			fIsSameAcq = true;
			if (IsAllAByte(tDbSchMtrRlt.bMtr, 0, tDbSchMtrRlt.bMtrLen) || IsAllAByte(tDbSchMtrRlt.bMtr, 0xee, tDbSchMtrRlt.bMtrLen))	//终端里的表地址为NULL
			{
				DelSchMtrResult(index);	//终端里pSchMtrRlt的表地址为NULL，pSchMtrRlt与tDbSchMtrRlt的采集器地址相等，直接删除终端里pSchMtrRlt的档案
				break;
			}
		}

// 		//采集器对应的表地址是否为NULL
// 		if (IsAllAByte(tDbSchMtrRlt.bMtr, 0, tDbSchMtrRlt.bMtrLen) || IsAllAByte(tDbSchMtrRlt.bMtr, 0xee, tDbSchMtrRlt.bMtrLen))	//终端里的表地址为NULL
// 		{
// 			if (memcmp(tDbSchMtrRlt.bAcqAddr, tSchMtrRlt.bAcqAddr, tSchMtrRlt.bAcqLen) == 0)	//采集地址相等
// 			{
// 				DelSchMtrResult(index);	//终端里pSchMtrRlt的表地址为NULL，pSchMtrRlt与tDbSchMtrRlt的采集器地址相等，直接删除终端里pSchMtrRlt的档案
// 				fIsSameAcq = true;
// 				break;
// 			}
// 		}
	}while (iStart != -1);

	//到这里fIsSameAcq=false，表明pSchMtrRlt与终端tDbSchMtrRlt没有相同的采集器，需要寻找一个空的位置保存该档案
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

		//采集器对应的表地址是否为NULL
		if (memcmp(tDbMtrInfo.bAcqTsa, tMtrInfo.bAcqTsa, tMtrInfo.bAcqTsaLen) == 0)	//采集地址相等
		{	
			fIsSameAcq = true;
			if (IsAllAByte(tDbMtrInfo.bTsa, 0, tDbMtrInfo.bTsaLen) || IsAllAByte(tDbMtrInfo.bTsa, 0xee, tDbMtrInfo.bTsaLen))	//终端里的表地址为NULL
			{
				DelMeterInfo(wPn);	//终端里pSchMtrRlt的表地址为NULL，pSchMtrRlt与tDbSchMtrRlt的采集器地址相等，直接删除终端里pSchMtrRlt的档案
				InitMtrMask();
				//break;
			}
		}
	}

	//到这里fIsSameAcq=false，表明pSchMtrRlt与终端tDbSchMtrRlt没有相同的采集器，需要寻找一个空的位置保存该档案
#ifndef  VER_ZJ   //浙江搜表不需要保留以前存在的采集器档案  changed by whr 20170811
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

//描述：存储搜表结果()
//参数：	@pbBuf 搜表结果
int CCctSchMeter::SaveSchMtrResult(DWORD dwPortOad, BYTE *pbBuf, WORD wLen, BYTE bMtrAddrLen)
{
	TSchMtrRlt tSchMtrRlt;
	char szCurTime[32];
	BYTE *p = pbBuf;
	BYTE bRptNum;
	BYTE bSlvNum;
	BYTE bDevType;	//00H＝采集器；01H＝电能表；02H～FFH 保留
	BYTE bTotalNodeNum = 0;  //

	memset((BYTE*)&tSchMtrRlt, 0, sizeof(TSchMtrRlt));
	tSchMtrRlt.dwPort = dwPortOad;
	WaitSemaphore(m_tSchMtrSem);
	bRptNum = *p++;	//上报从节点的数量n
	for (BYTE i=0; i<bRptNum; i++)
	{
		bDevType = p[9];
		if (bDevType == 0x00)	//采集器
		{
			revcpy(tSchMtrRlt.bAcqAddr, p, 6);
			tSchMtrRlt.bAcqLen = 6;
			p += 6;
			tSchMtrRlt.bMtrPro = *p++;
			p += 3;	//从节点序号(2) + 从节点设备类型(1)
			bTotalNodeNum = *p++;	//从节点下接从节点数量M
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
		else	//电表
		{
			memset(tSchMtrRlt.bAcqAddr, 0, TSA_LEN);
			tSchMtrRlt.bAcqLen = 0;
			revcpy(tSchMtrRlt.bMtr, p, bMtrAddrLen);
			tSchMtrRlt.bMtrLen = bMtrAddrLen;
			p += bMtrAddrLen;
			tSchMtrRlt.bMtrPro = *p++;
			p += 3;	//从节点序号(2) + 从节点设备类型(1)
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

//描述：获取搜表结果
//参数：@piStart 首次传入为-1，搜表结果未读取完就返回相应的值，读取结束返回0xFFFE
//返回： -1结束，>0本次读取的次数
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

	iMod = iFileLen%PER_RLT_LEN;	//无法整除，表示文件出错
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

//描述：获取搜表结果
//参数：@piStart 首次传入为-1，搜表结果未读取完就返回相应的值，读取结束返回0xFFFE
//返回： -1结束，>0本次读取的次数
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
	p++;	//跳过Arry中的个数

	iMod = iFileLen%PER_RLT_LEN;	//无法整除，表示文件出错
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

			//一个搜表结果
			*p++ = DT_STRUCT;
			*p++ = 0x07;

			//通信地址
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
				//所属采集器地址
				*p++ = DT_TSA;
				*p++ = tSchMtrRlt.bAcqLen + 1;
				*p++ = tSchMtrRlt.bAcqLen - 1;
				memcpy(p, tSchMtrRlt.bAcqAddr, tSchMtrRlt.bAcqLen);
				p += tSchMtrRlt.bAcqLen;
			}
			//规约类型 
			*p++ = DT_ENUM;
			*p++ = tSchMtrRlt.bMtrPro;

			//相位
			*p++ = DT_ENUM;
			*p++ = tSchMtrRlt.bPhase;

			//信号品质
			*p++ = DT_UNSIGN;
			*p++ = tSchMtrRlt.bSQ;

			//搜到的时间
			*p++ = DT_DATE_TIME_S;
			p += OoTimeToDateTimeS(&tSchMtrRlt.tSchMtrSucTime, p);

			//搜到的附加信息
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


//描述：获取搜表结果
//参数：@piStart 首次传入为-1，搜表结果未读取完就返回相应的值，读取结束返回0xFFFE
//返回： -1结束，>0本次读取的次数
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
	p++;	//跳过Arry中的个数

	iMod = iFileLen%PER_CROSS_RLT_LEN;	//无法整除，表示文件出错
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

			//一个跨台区结果
			*p++ = DT_STRUCT;
			*p++ = 0x03;

			//通信地址
			*p++ = DT_TSA;
			*p++ = 6+1;
			*p++ = 6-1;
			memcpy(p, tSchMtrRlt.bMtrAddr, 6);
			p += 6;
			//主节点地址
			*p++ = DT_TSA;
			*p++ = 6+1;
			*p++ = 6-1;
			memcpy(p, tSchMtrRlt.bMainAddr, 6);
			p += 6;

			//变更时间
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

//描述：所有台区搜表结果记录数
int CCctSchMeter::CalSchMtrNum()
{
	char szTabName[64]={0};
	BYTE bBuf[8];
	WORD wRltNum;
	int iFileLen;
	int iMod;


	MK_SCH_MTR_FILE(szTabName);
	iFileLen = GetFileLen(szTabName);

	iMod = iFileLen%PER_RLT_LEN;	//无法整除，表示文件出错
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

//描述：写一个跨台区搜表结果
//参数：	@pbBuf 搜表结果
int CCctSchMeter::SaveCrossSchMtrResult(BYTE *pbBuf, WORD wLen)
{
	BYTE bFrmLen;

	TCrossSchMtrRlt tRlt;	//跨台区搜表结果

	pbBuf++;	//从节点设备类型
	pbBuf++;	//通信协议类型
	bFrmLen = *pbBuf++;	//报文长度L
	if ((pbBuf[0]!=0x68 || pbBuf[7]!=0x68) || (bFrmLen<18))
		return -1;

	//68H	A0	……	A5	68H	9EH	07H	03H	PA0	……	PA5	CS	16H
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
		iMod = iFileLen%PER_CROSS_RLT_LEN;	//无法整除，表示文件出错
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

	//对路由上报的地址进行判定，属于哪种类型
	if ((IsAllAByte(pSchMtrRlt->bMtr, 0x00, 6) || IsAllAByte(pSchMtrRlt->bMtr, 0xee, 6)) && !IsAllAByte(pSchMtrRlt->bAcqAddr, 0, 6))	//单采集器地址，电表地址为空
		bAddrState = SINGLE_ACQ_ADDR;
	else if (!IsAllAByte(pSchMtrRlt->bMtr, 0x00, 6) && (IsAllAByte(pSchMtrRlt->bAcqAddr, 0x00, 6) || IsAllAByte(pSchMtrRlt->bAcqAddr, 0xee, 6)))	//单电表地址,采集器地址为空
		bAddrState = SINGLE_MTR_ADDR;
	else //采集器+电表地址
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
						if (memcmp(pSchMtrRlt->bMtr, tDbMtrInfo.bTsa, 6) == 0)	//终端里存在相同的电表地址
						{
							//if (!IsAllAByte(tDbMtrInfo.bAcqTsa, 0x00, 6) && !IsAllAByte(tDbMtrInfo.bAcqTsa, 0xee, 6) && tDbMtrInfo.bAcqTsa[1]!=0xAA)	//终端里的采集地址不为NULL，替换
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
						if (memcmp(pSchMtrRlt->bAcqAddr, tDbMtrInfo.bAcqTsa, 6) == 0)	//终端里存在相同采集器地址，就可以直接退出
						{
							DTRACE(DB_CCT_SCH, ("[NULL+MTR to 0x6000]  Same acq: %s.\n",HexToStr(pSchMtrRlt->bAcqAddr, 6, szDbAcqBuf)));
							goto Ret_UpdataOneSchMtrToDb;
						}
					}
					else	//采集器+电表地址
					{
						if (memcmp(pSchMtrRlt->bAcqAddr, tDbMtrInfo.bAcqTsa, 6) == 0)	//采集地址相等
						{
							if (IsAllAByte(tDbMtrInfo.bTsa, 0xee, 6) || IsAllAByte(tDbMtrInfo.bTsa, 0x00, 6))	//表地址为空，直接代替
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

						if ((memcmp(pSchMtrRlt->bMtr, tDbMtrInfo.bTsa, 6) == 0))	//电表地址相同
						{
							if (memcmp(pSchMtrRlt->bAcqAddr, tDbMtrInfo.bAcqTsa, 6) != 0)	//采集器地址不相同，直接替换
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

	if (wPnMask >= PN_MASK_SIZE)	//表明系统库中没有电表档案，需入库，寻找一个空位置
	{
		pbMtrMask = GetMtrMask(BANK17, PN0, 0x6001);
		SearchEmptySaveMeter(pSchMtrRlt, (BYTE*)pbMtrMask);
	}
}


//描述：寻找一个空的表序号存储搜回的表档案
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
					if (wPn<=10 && pSchMtrRlt->dwPort==0xf2090201)	//485范围为1~10，载波范围为11~POINT_NUM
						continue;
					 //GetEmptyPn()
					wSn = wPn;
					SetMtrSnToPn(wPn, wPn);
					//配置单元
					*p++ = DT_STRUCT;
					*p++ = 0x04;
					//配置序号
					*p++ = DT_LONG_U;
					*p++ = wPn/256;
					*p++ = wPn%256;
					//基本信息
					*p++ = DT_STRUCT;
					*p++ = 0x0a;
					//通信地址

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

					//波特率
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
					//规约类型 
					*p++ = DT_ENUM;
					*p++ = pSchMtrRlt->bMtrPro;
					//端口
					*p++ = DT_OAD;
					p += OoDWordToOad(pSchMtrRlt->dwPort, p);
					//通信密码
					*p++ = DT_OCT_STR;
					*p++ = 0x00;
					//费率个数
					*p++ = DT_UNSIGN;
					*p++ = 0x04;	//暂时定义4费率
					//用户类型 
					*p++ = DT_UNSIGN;
					*p++ = 0x00;
					//接线方式
					*p++ = DT_ENUM;
					*p++ = OOB_LINE_UNKNOW;
					//额定电压
					*p++ = DT_LONG_U;
					*p++ = 0x00;
					*p++ = 0x00;
					//额定电流
					*p++ = DT_LONG_U;
					*p++ = 0x00;
					*p++ = 0x00;
					//扩展信息---------------------
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
						//采集器地址
						*p++ = DT_TSA;
						*p++ = pSchMtrRlt->bAcqLen + 1;
						*p++ = pSchMtrRlt->bAcqLen - 1;
						memcpy(p, pSchMtrRlt->bAcqAddr, pSchMtrRlt->bAcqLen);
						p += pSchMtrRlt->bAcqLen;
					}
					//资产号
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
					//附加信息域
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

//描述：生成搜表事件
void CCctSchMeter::SaveAlarmEvent(TSchMtrRlt *pSchMtrRlt)
{
	BYTE bAlarmBuf[128] = {0};
	BYTE *p = bAlarmBuf;

	if (m_TSchMtrParm.fIsGenEvt)
	{
		//一个搜表结果
		*p++ = DT_STRUCT;
		*p++ = 0x07;

		//通信地址 
		*p++ = DT_TSA;
		*p++ = pSchMtrRlt->bMtrLen + 1;
		*p++ = pSchMtrRlt->bMtrLen - 1;
		memcpy(p, pSchMtrRlt->bMtr, pSchMtrRlt->bMtrLen);
		p += pSchMtrRlt->bMtrLen;
		
		//采集地址
		*p++ = DT_TSA;
		*p++ = pSchMtrRlt->bAcqLen + 1;
		*p++ = pSchMtrRlt->bAcqLen - 1;
		memcpy(p, pSchMtrRlt->bAcqAddr, pSchMtrRlt->bAcqLen);
		p += pSchMtrRlt->bAcqLen;

		//规约类型
		*p++ = DT_ENUM;
		*p++ = pSchMtrRlt->bMtrPro;

		//相位
		*p++ = DT_ENUM;
		*p++ = pSchMtrRlt->bPhase;

		//信号品质
		*p++ = DT_ENUM;
		*p++ = pSchMtrRlt->bSQ;

		//搜到的时间
		*p++ = DT_DATE_TIME_S;
		p += OoTimeToDateTimeS(&pSchMtrRlt->tSchMtrSucTime, p);

		//搜到的附加信息
		*p++ = DT_ARRAY;
		*p++ = 0x00;

		//强锅提供事件接口。。。。
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

//描述：激活从节点等待时间
int CCctSchMeter::GetNodeActWaitMin()
{
	int iActWaitMin;

	if (m_fRightNowSchMtr)	
	{
		if (m_bActCnt == 0)	//第一次10分钟
			iActWaitMin = 10;
		else if (m_bActCnt == 1)	//第二次20分钟
			iActWaitMin = 20;
		else	// 第三次等待17分钟
			iActWaitMin = 17;

		m_bActCnt++;
	}
	else
	{
		iActWaitMin = 120;	////时间为0，表示主动注册持续时间不限制，填充120min
	}

	return iActWaitMin;
}

//描述：更新电表档案标识
void CCctSchMeter::SetUdpMtrFlg(bool fState)
{
	m_fUdpMtrToDB = fState;
}

//描述：获取电表档案标识
bool CCctSchMeter::GetUdpMtrFlg()
{
	return m_fUdpMtrToDB;
}

//描述：更新告警事件信号屏蔽字
//参数：@wIndex 发现未知电表索引
//		@fState false:清除索引wIndex对应的标识，反之
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


//描述：更新告警事件信号屏蔽字
bool CCctSchMeter::UpdataSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen)
{
	if (wMaskLen < PN_MASK_SIZE)
		return false;

	WaitSemaphore(m_tAlarmSem);
	WriteItemEx(BANK16, PN0, 0x6010, pbMask);
	SignalSemaphore(m_tAlarmSem);

	return true;
}

//描述：获取告警事件信号屏蔽字
bool CCctSchMeter::GetSchMtrEvtMask(BYTE *pbMask, WORD wMaskLen)
{
	if (wMaskLen < PN_MASK_SIZE)
		return false;

	WaitSemaphore(m_tAlarmSem);
	ReadItemEx(BANK16, PN0, 0x6010, pbMask);
	SignalSemaphore(m_tAlarmSem);
	return true;
}

//描述：清除告警事件屏蔽字
void CCctSchMeter::ClearSchMtrEvtMask()
{
	BYTE bMask[PN_MASK_SIZE] = {0};

	WaitSemaphore(m_tAlarmSem);
	WriteItemEx(BANK16, PN0, 0x6010, bMask);
	SignalSemaphore(m_tAlarmSem);
}

//描述：通过索引获取告警数据
//参数：@wIndex 告警索引
//		@pbBuf 返回的告警数据内容
//返回：-1获取数据失败，>0告警数据的长度
int CCctSchMeter::GetSchMtrEvtData(int iIndex, BYTE *pbBuf)
{
	TSchMtrRlt tSchMtrRlt;
	BYTE *pbBuf0 = pbBuf;

	if (GetSchMtrResult(&iIndex, &tSchMtrRlt, false) < 0)
		return -1;

	//一个搜表结果
	*pbBuf++ = DT_STRUCT;
	*pbBuf++ = 0x07;

	//通信地址 
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

	//采集地址
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

	//规约类型
	*pbBuf++ = DT_ENUM;
	*pbBuf++ = tSchMtrRlt.bMtrPro;
	//相位
	*pbBuf++ = DT_ENUM;
	*pbBuf++ = tSchMtrRlt.bPhase;
	//信号品质
	*pbBuf++ = DT_ENUM;
	*pbBuf++ = tSchMtrRlt.bSQ;
	//搜到的时间
	*pbBuf++ = DT_DATE_TIME_S;
	pbBuf += OoTimeToDateTimeS(&tSchMtrRlt.tSchMtrSucTime, pbBuf);
	//搜到的附加信息
	*pbBuf++ = DT_ARRAY;
	*pbBuf++ = 0x00;

	return pbBuf - pbBuf0;
}

//描述：校验上次搜表时间至现在是否超过30天
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
	
	dwDelTimeout = 30*24*60*60;	//30天
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
