/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FrzTask.cpp
 * ժ    Ҫ�����ļ���Ҫʵ���������Э��Ķ�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2016��9��
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

#define FRZ_DELAY_TIMEOUT		10		//�����ͺ�ʱ��

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ȫ�ֱ�������
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

BYTE g_bFrzFixOAList[] = { 0x01, 0x02, 0x51, 0x20, 0x23, 0x02, 0x00, 0x51, 0x20, 0x21, 0x02, 0x00 };	//�̶��ֶ���������
BYTE g_bFrzFixFmt[] = { 0x01, 0x02, 0x51 };		//�̶��ֶθ�ʽ������

BYTE g_bFrzSubDataFmt[] = { 0x01, 0x01, 0x51 };

const BYTE g_bFrzOITypeList[] = { FRZ_OIB_INST, FRZ_OIB_SEC, FRZ_OIB_MIN, FRZ_OIB_HOUR, FRZ_OIB_DAY, FRZ_OIB_BALANCEDAY, FRZ_OIB_MONTH, 
										FRZ_OIB_YEAR, FRZ_OIB_TIMEZONE, FRZ_OIB_DAYSTAGE, FRZ_OIB_TARIFF, FRZ_OIB_STAIR, FRZ_OIB_CLRYEAR, };

TFrzCtrl g_FrzCtrl[FRZ_TASK_NUM];


//����OAD��ID��ӳ���
const DWORD g_dwSpecSrcOADList[] = 
{
	0x21000200, 0x21010200,	0x21020200, 0x21030200, 0x21040200,	
	0x21100200, 0x21110200, 0x21120200, 0x21130200, 0x21140200,
	0x21200200, 0x21210200, 0x21220200, 0x21230200, 0x21240200,
	0x21300200, 0x21310200, 0x21320200, 0x21330200, 0x21400200, 
	0x21410200, 0x22000200, 0x22030200, 0x22040200,
};

//#define DEBUG_TEST 1

//���������Ƿ���Ч
bool IsFrzTaskCfgValid(TFrzCtrl* pFrzCtrl)
{
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
	if (pFrzCfg->dwOA!=0 && pFrzCfg->wMaxRecNum!=0)		//����������Ч
		return true;
	else
		return false;
}


//���������������ӳ�䵽OI
//��������ȡ��������OI
WORD GetFrzOI(BYTE bFrzType)
{	
	if (bFrzType < sizeof(g_bFrzOITypeList))
		return g_bFrzOITypeList[bFrzType] + OI_FRZ;
	else
		return 0;	
}


//OIӳ�䵽�����������
//��������ȡ���������������
//���أ��ɹ�0~FRZ_TYPE_NUM�� ʧ�ܣ�0xff
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
//��ʼ�������񶳽���ƽṹ
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
	if (bFrzType!=FRZ_OIB_INST && ReadLastNRec(szTableName, LAST_REC, bRecBuf, sizeof(bRecBuf)) > 0)	//��ȡ���һ�ʼ�¼����ʱ��
	{
		OoDateTimeSToTime(bRecBuf+REC_TIME_OFFSET, &pFrzCtrl->tmLastRec);
		if (IsInvalidTime(pFrzCtrl->tmLastRec))
			memset(&pFrzCtrl->tmLastRec, 0, sizeof(TTime));
	}

	return true;
}

//BYTE bCmpBuf[] = {0x02, 0x03, 0x12, 0x00, 0x00, 0x51, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00};
//���������Ϳ��ƽṹ������������
void GetFrzFmtBuf(TFrzCtrl* pFrzCtrl, BYTE* p)
{
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
	
	*p++ = DT_STRUCT;	//�ṹ������
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
	BYTE bBuf[FRZRELA_ID_LEN];		//OAD����*(sizeof(FRZRELA)+2) + ��������1 + ����Ԫ�ظ���1
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
		iLen = OoReadAttr(wOI, FRZ_ATTRTAB, tDataFields.pbCfg, &pbFmt, &wFmtLen);		//��ȡ�������Ա�
	#endif	

	if (iLen>0 && tDataFields.pbCfg[1]>0 && tDataFields.pbCfg[1]<=CAP_OAD_NUM)
	{
		tDataFields.wCfgLen = tDataFields.pbCfg[1]*DT_FRZRELA_LEN + 2;
		if (OoParseField(&tDataFields, pbFmt, wFmtLen, false) == false)
			return false;

		//�ڴ�ӳ����е���������Թ������Ա�����Ϊ׼������Ϊͬ���ڴ�ӳ���Ĳ��裺
		//Step1, �������Ա����ޣ�ӳ����У������������ӳ�䣬ͬʱɾ��
		for (i=0; i<FRZ_TASK_NUM; i++)	
		{
			if (pFrzCtrl[i].wOI==GetFrzOI(bFrzType) && IsFrzTaskCfgValid(pFrzCtrl+i))		//OI��ͬ
			{
				for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	//�ڹ������Ա��в����Ƿ����
				{
					memset(bCfgBuf, 0, sizeof(bCfgBuf));
					iLen = ReadParserField(&tDataFields, bIndex, bCfgBuf, &bType, &wItemOffset, &wItemLen);
					if (iLen > 0)
					{
						GetFrzFmtBuf(pFrzCtrl+i, bCmpBuf);
						if (FieldCmp(DT_FRZRELA, bCmpBuf+1, DT_FRZRELA, bCfgBuf+1) == 0)	//��ȫ��ͬ
						{
							memset(bSubDataBuf, 0, sizeof(bSubDataBuf));
							bSubDataBuf[0] = 0x01;	//��������
							bSubDataBuf[1] = 0x01;	//����Ԫ�ظ���
							bSubDataBuf[2] = DT_OAD;	//��������
							memcpy(bSubDataBuf+3, bCfgBuf+OFFSET_FRZ_OAD, 4);		//OAD

							memset(&tSubDataFields, 0, sizeof(TFieldParser));
							tSubDataFields.pbCfg = bSubDataBuf;
							tSubDataFields.wCfgLen = SUB_DATD_FIELD_LEN;

							if (OoParseField(&tSubDataFields, g_bFrzSubDataFmt, sizeof(g_bFrzSubDataFmt), true) == false)		//�����ֶ�
								return false;

							if (pFrzCtrl[i].wDataFieldLen != tSubDataFields.wTotalLen)
								bIndex = tDataFields.wNum;		//��Ȼ�ҵ��ˣ��������ֶα仯�ˣ���Ҫ�������ӳ���ɾ��

							break;
						}
					}
				}

				if (bIndex == tDataFields.wNum)	//�ڹ������Ա���û�ҵ� ��ɾ��
				{
					TFrzCfg* pFrzCfg = &pFrzCtrl[i].tCfg;
					sprintf(szTableName, FMT_FRZ_TASK_TABLE, GetFrzOI(bFrzType), pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);
					TdbDeleteTable(szTableName);

					memset(pFrzCtrl+i, 0, sizeof(TFrzCtrl));
					DTRACE(DB_TASK, ("InitFrzTask delete table %s! bFrzType=%d.\r\n", szTableName, bFrzType));
				}
			}
		}

		//Step2, �������Ա����У��ڴ�ӳ����ޣ�����Ӹ�����ӳ�䣬ͬʱ����
		for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	//�����������
		{
			memset(bCfgBuf, 0, sizeof(bCfgBuf));
			iLen = ReadParserField(&tDataFields, bIndex, bCfgBuf, &bType, &wItemOffset, &wItemLen);		//�������õĶ������
			if (iLen > 0)
			{
				if (OoGetDataLen(DT_OAD, bCfgBuf+OFFSET_FRZ_OAD) <= 0)		//��������OAD��Ч
					continue;	//��������������
				
				if (bType == 2)
					bCfgBuf[0] = DT_FRZRELA;	//�ṹ�����ʹ�����ڲ����������������

				fExist = false;
				for (i=0; i<FRZ_TASK_NUM; i++)	//���ڴ�ӳ����в����Ƿ���ӳ��
				{
					if (pFrzCtrl[i].wOI==GetFrzOI(bFrzType) && IsFrzTaskCfgValid(pFrzCtrl+i))		//OI��ͬ
					{
						pFrzCfg = &pFrzCtrl[i].tCfg;
						GetFrzFmtBuf(pFrzCtrl+i, bCmpBuf);
						if (FieldCmp(DT_FRZRELA, bCfgBuf+1, DT_FRZRELA, bCmpBuf+1) == 0)	//��ȫ��ͬ�����ʾ�Ѵ���
						{							
							DTRACE(DB_TASK, ("InitFrzTask OAD already fExist! i=%d, bFrzType=%d, wCycle=%d, dwOA=%04x, wMaxRecNum=%d.\r\n", i, bFrzType, bIndex, pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum));
							fExist = true;
							break;
						}
					}
				}

				if (fExist == false)	//�ڴ�ӳ����в����� ������
				{
					for (i=0; i<FRZ_TASK_NUM; i++)
					{
						if (!IsFrzTaskCfgValid(pFrzCtrl+i))	//�ҵ���λ
						{
							DTRACE(DB_TASK, ("InitFrzTask find space i=%d, bFrzType=%d, bIndex=%d.\r\n", i, bFrzType, bIndex));
							InitSubFrzTask(bFrzType, bCfgBuf, pFrzCtrl+i);

							pFrzCfg = &pFrzCtrl[i].tCfg;
							sprintf(szTableName, FMT_FRZ_TASK_TABLE, GetFrzOI(bFrzType), pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);

							memset(&tFixFields, 0, sizeof(TFieldParser));
							tFixFields.pbCfg = g_bFrzFixOAList;
							tFixFields.wCfgLen = sizeof(g_bFrzFixOAList);
							OoParseField(&tFixFields, g_bFrzFixFmt, sizeof(g_bFrzFixFmt), true);	//�̶��ֶ�

							memset(bSubDataBuf, 0, sizeof(bSubDataBuf));
							bSubDataBuf[0] = 0x01;	//��������
							bSubDataBuf[1] = 0x01;	//����Ԫ�ظ���
							bSubDataBuf[2] = DT_OAD;	//��������
							memcpy(bSubDataBuf+3, bCfgBuf+OFFSET_FRZ_OAD, 4);		//OAD

							memset(&tSubDataFields, 0, sizeof(TFieldParser));
							tSubDataFields.pbCfg = bSubDataBuf;
							tSubDataFields.wCfgLen = SUB_DATD_FIELD_LEN;
							if (OoParseField(&tSubDataFields, g_bFrzSubDataFmt, sizeof(g_bFrzSubDataFmt), true) == false)		//�����ֶ�
								return false;

							if (CreateTable(szTableName, &tFixFields, &tSubDataFields, pFrzCfg->wMaxRecNum) > 0)		//ÿ����������1�ű�
								pFrzCtrl[i].wDataFieldLen = tSubDataFields.wTotalLen;	//���������ֶγ���

							break;
						}
					}

					if (i == FRZ_TASK_NUM)	//�����˾Ͳ�����
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
		if (bBuf[0]==bFrzType && bBuf[1]==FRZ_CLR_VALID)		//��λ������Ч
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
	//	TrigerSaveBank(BN11, 0, -1); //��������

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
		memset(g_FrzCtrl, 0, sizeof(g_FrzCtrl));	//�ϵ����������ʼ��(��Ҫ���¼��ʱ)����������洢λ�ñ仯

		for (wPn=0; wPn<FRZ_TASK_NUM; wPn++)
		{
			iLen = ReadItemEx(BN11, wPn, wCmdID, bBuf);
			if (iLen > 0)
			{
				if (!IsAllAByte(bBuf, 0, iLen))
				{
					memset(bBuf, 0, iLen);
					WriteItemEx(BN11, wPn, wCmdID, bBuf);	//�ϵ��ʼ���������
				}
			}
		}
	}

	for (bFrzType=0; bFrzType<FRZ_TYPE_NUM; bFrzType++)
		InitFrzTask(bFrzType, g_FrzCtrl);

	g_fFrzInit = true;
	return true;
}

//�Ƿ������
bool IsBalanceDay(const TTime& rNow)
{
	int iLen;
	WORD i;
	BYTE bBuf[BALANCE_DAY_NUM*6+2];

	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(OI_BALANCEDAY, ATTR2, bBuf, NULL, NULL);	//�����ղ���
	if (iLen<=0 || bBuf[0]!=DT_ARRAY)	//Ĭ��1��0��
	{
		bBuf[0] = DT_ARRAY;
		bBuf[1] = 1;			//�����ո���
		bBuf[4] = DT_UNSIGN;
		bBuf[5] = 1;			//��
		bBuf[6] = DT_UNSIGN;
		bBuf[7] = 0;			//ʱ
	}

	if (bBuf[1] > BALANCE_DAY_NUM)
		bBuf[1] = BALANCE_DAY_NUM;

	for (i=0; i<bBuf[1]; i++)
	{
		if (bBuf[i*6+4]==DT_UNSIGN && bBuf[i*6+6]==DT_UNSIGN)
		{
			if (rNow.nDay==bBuf[i*6+5] && rNow.nHour==bBuf[i*6+7])		//�������պ�ʱƥ��
				return true;
		}
	}

	return false;
}


//ʱ���Ƿ��л�
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

//��ʱ���л���־C900 BIT0
void ClrZoneChgFlag()
{
	BYTE bBuf[10];

	bBuf[0] &= 0xfe;	//��BIT0
	WriteItemEx(BN4, PN0, 0xc900, bBuf);
	TrigerSaveBank(BN4, 0, -1);
}

//ʱ�α��Ƿ��л�
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

//��ʱ�α��л���־C900 BIT1
void ClrDayStageChgFlag()
{
	BYTE bBuf[10];
	
	bBuf[0] &= 0xfd;	//��BIT1
	WriteItemEx(BN4, PN0, 0xc900, bBuf);
	TrigerSaveBank(BN4, 0, -1);
}


//�ͺ󶳽�ʱ���Ƿ񵽴� (����ͳ�����ݵģ����ͳ����ɺ��ٶ��ᣬ�����ͺ�30�붳��)
bool IsDelayTimeOut(TTime& rNow, bool fFrzStat)
{
	if (fFrzStat == false)
		return true;	//������ͳ�����ݣ������ӳٶ���

	if ((rNow.nMinute*60+rNow.nSecond) >= FRZ_DELAY_TIMEOUT)
		return true;
	else
		return false;
}


//�Ƿ��ͺ󶳽�
//ͳ����OAD������Ҫ��ͳ������ٶ��ᣬ�����ͺ�30�붳��
bool IsDelayFrz(TFrzCtrl* pFrzCtrl)
{
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;

	if ((pFrzCfg->dwOA&OAD_OI_MASK) == 0x21000200 || (pFrzCfg->dwOA&OAD_OI_MASK) == 0x22000200)
		return true;
	else	//����Ŀǰֻ�е�ǰ������û�е��յ�������������/����ʱ������0���ʲ���Ҫ����
		return false;
}

//bTaskIndex:�������
int Timeout(TTime& rNow, WORD wTaskIndex, TFrzCtrl* pFrzCtrl)
{	
	int iLen;
	BYTE bFrzType;
	bool fFrzStat;
	WORD wPn, wIntervV, wDelaySec;	
	DWORD dwCur, dwLast, dwIntervVa;	// ��ǰʱ�䣬�ϴ�ʱ��
	DWORD dwClick, dwRxCmdClick;	
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
	BYTE bBuf[20];

	wIntervV = pFrzCfg->wCycle;
	bFrzType = GetFrzType(pFrzCtrl->wOI);
	if (bFrzType >= FRZ_TYPE_NUM)
		return -1;

	fFrzStat = IsDelayFrz(pFrzCtrl);	//�Ƿ���Ҫ�ӳٶ���
	if (wIntervV == 0)	//����Ϊ0���淶Ҫ�󰴴����ඳ�ᴦ��
		return -1;

	// �������������ж�, �����׼ʱ��Ϊ2000��1��1��0ʱ0��
	switch (bFrzType)
	{
		/*case FRZ_OIB_SEC:	//��ֹƵ��дflash
			dwCur = TimeToSeconds(rNow);
			dwLast = TimeToSeconds(pFrzCtrl->tmLastRec);

			// ������Ĭ��60��
			if (wIntervV == 0)
				wIntervV = FRZ_DFTCYC_SEC;

			// ���ϼ��ʱ�䣬�����ϱʼ�¼��ʱ�䲻ͬ��
			if (dwCur%wIntervV==0 && dwCur!=dwLast) 
			{
				//SecondsToTime(dwCur, &rNow);
				return 0;
			}

			return -1;*/

		case FRZ_OIB_MIN:	//���ڷ��Ӷ��ᣬ��0�붳��
			dwCur = TimeToMinutes(rNow);			
			dwLast = TimeToMinutes(pFrzCtrl->tmLastRec);	

			// ���ϼ��ʱ�䣬�����ϱʼ�¼��ʱ�䲻ͬ��
			if (dwCur%wIntervV==0 && dwCur!=dwLast && IsDelayTimeOut(rNow, fFrzStat))
				return 0;
			else
				return -1;

		case FRZ_OIB_HOUR:	//����Сʱ����
			dwCur = TimeToMinutes(rNow);			
			dwLast = TimeToMinutes(pFrzCtrl->tmLastRec);			

			dwIntervVa = wIntervV*60;

			// ���ϼ��ʱ�䣬�����ϱʼ�¼��ʱ�䲻ͬ�������������Ӷ���
			if (dwCur%dwIntervVa==0 && dwCur!=dwLast && IsDelayTimeOut(rNow, fFrzStat))
				return 0;
			else
				return -1;

		case FRZ_OIB_DAY:	//�����ն��ᣬ��0ʱ����
			dwCur = DaysFrom2000(rNow);			
			dwLast = DaysFrom2000(pFrzCtrl->tmLastRec);

			// ���ϼ��ʱ�䣬�����ϱʼ�¼��ʱ�䲻ͬ
			if (dwCur%wIntervV==0 && dwCur!=dwLast && rNow.nHour==0 && IsDelayTimeOut(rNow, fFrzStat))
				return 0;
			else					
				return -1;

		case FRZ_OIB_MONTH:	//�����¶��ᣬ��1��0ʱ����
			dwCur = MonthFrom2000(rNow);
			dwLast = MonthFrom2000(pFrzCtrl->tmLastRec);
			
			// ���ϼ��ʱ�䣬�����ϱʼ�¼��ʱ�䲻ͬ
			if (dwCur%wIntervV==0 && dwCur!=dwLast && rNow.nDay==1 && rNow.nHour==0 && IsDelayTimeOut(rNow, fFrzStat))
				return 0;
			else
				return -1;

		case FRZ_OIB_YEAR:	//�����궳�ᣬ��1��1��0ʱ����
			dwCur = rNow.nYear-BASETIME;
			dwLast = pFrzCtrl->tmLastRec.nYear-BASETIME; 

			// ���ϼ��ʱ�䣬�����ϱʼ�¼��ʱ�䲻ͬ
			if (dwCur%wIntervV==0 && dwCur!=dwLast && rNow.nMonth==1 && rNow.nDay==1 && rNow.nHour==0 && IsDelayTimeOut(rNow, fFrzStat))
				return 0;
			else
				return -1;

		case FRZ_OIB_BALANCEDAY:	//������
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
			return -1;	//ʱ��û��
	}

	return -1;	//ʱ��û��
}


//�Ƿ�Ϊͳ������OAD
bool IsSpecFrzOAD(DWORD dwOAD)
{
	WORD i;

	dwOAD &= OAD_OI_MASK;
	if (dwOAD==0x21000200 || dwOAD==0x22000200)		//ͳ��OAD
	{
		for (i=0; i<sizeof(g_dwSpecSrcOADList)/sizeof(DWORD); i++)
		{
			if (dwOAD == g_dwSpecSrcOADList[i])
				return true;
		}
	}

	return false;
}

//��ȡͳ��ӳ��OAD��ӳ�䵽FRZ_STAT_BASE_OADΪ��ַ+i����չOAD��,
//����ֵ: 0ʧ��, >0�ɹ�
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

//��ȡͳ������
//BYTE bStatTypeList[] = { TIME_UNIT_MINUTE, TIME_UNIT_HOUR,  TIME_UNIT_DAY, TIME_UNIT_MONTH, TIME_UNIT_YEAR };
//���أ�ͳ����������Ԫ�ص����, ����ֵ0xff��ʾ����ͳ������
BYTE GetStatType(DWORD dwOAD)
{
	DWORD dwMaskOAD;
	BYTE bIndex, bType;

	dwMaskOAD = dwOAD & 0xfff01f00;
	bIndex = dwOAD & 0xff;

	switch (dwMaskOAD)
	{
		case 0x21300200:	//�ܼ�ABC��ѹ�ϸ���
		case 0x22000200:	//ͨ������
		case 0x22030200:	//����ʱ��
		case 0x22040200:	//��λ����
			if (bIndex == 1)	//����ͳ��
				bType = 2;	//����������				
			else if (bIndex == 2)	//����ͳ��
				bType = 3;	//����������
			else
				bType = 0xff;	//�����ա��¶�������

			break;
		
		case 0x21400200:
			bType = 2;	//����������
			break;

		case 0x21410200:
			bType = 3;	//����������
			break;

		default:
			bType = dwOAD - dwMaskOAD;
			break;
	}

	return bType;
}


//��������ͳ�Ƽ���Ƿ�һ��
bool IsIntervMatch(DWORD dwOAD)
{
	return true;
/*	BYTE bFrzType, bStatType;
	BYTE bFrzTypeList[] = { 0xff, 0xff, TIME_UNIT_MINUTE, TIME_UNIT_HOUR, TIME_UNIT_DAY, 0xff, TIME_UNIT_MONTH, TIME_UNIT_YEAR};
	BYTE bStatTypeList[] = { TIME_UNIT_MINUTE, TIME_UNIT_HOUR,  TIME_UNIT_DAY, TIME_UNIT_MONTH, TIME_UNIT_YEAR };

	bFrzType = GetFrzType((WORD ) dwOAD>>16);	//��ʱ������
	if (bFrzType >= FRZ_TYPE_NUM)
		return false;

	bStatType = GetStatType(dwOAD);
	if (bStatType == 0xff)	//����ͳ������
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
	};		//������̶��ֶ�

	sprintf(szTableName, FMT_FRZ_TASK_TABLE, pFrzCtrl->wOI, pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);
	iLen = ReadLastNRec(szTableName, LAST_REC, bRecBuf, sizeof(bRecBuf));
	if (iLen > 0)
		dwRecIndex = OoDoubleLongUnsignedToDWord(bRecBuf+1);	//�����¼��� ���ֽ���ǰ����		

	dwRecIndex++;
	memset(bRecBuf, 0, sizeof(bRecBuf));
	OoParseField(&tFixFields, g_bFrzFixFmt, sizeof(g_bFrzFixFmt), true);	//�̶��ֶ�
		
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	//�̶��ֶθ���
	{
		memset(bFixBuf, 0, sizeof(bFixBuf));
		if (ReadParserField(&tFixFields, bIndex, bFixBuf, &bType, &wItemOffset, &wItemLen) > 0)		//�̶��ֶ�OAD
		{			
			const ToaMap* pOaMap = GetOIMap(OoOadToDWord(bFixBuf+1));
			if (pOaMap==NULL || pOaMap->pFmt==NULL)
			{
				DTRACE(DB_TASK, ("FrzData: Read dwOA:%x failed !!\r\n", OoOadToDWord(bFixBuf+1)));
				return false;
			}

			*p++ = *pOaMap->pFmt;	//ˢ����������

			if (*pOaMap->pFmt == DT_DATE_TIME_S)
			{
				bFrzType = GetFrzType(pFrzCtrl->wOI);
				if (bFrzType >= FRZ_TYPE_NUM)
					return false;

				tmNow = tmCurRec;
				tmNow.nSecond = 0;	//���¼ʱ������¼ʱ������0,����ͬһʱ������������������������¼
				if (bFrzType>=FRZ_OIB_DAY && bFrzType<=FRZ_OIB_YEAR)
					tmNow.nMinute = 0;	//���¼ʱ������ͬһʱ�������������������������¼

				iLen = OoTimeToDateTimeS(&tmNow, p);		//����ʱ��
			}
			else //if (*pOaMap->pFmt == 0x6)	�������
			{
				iLen = OoDWordToDoubleLongUnsigned(dwRecIndex, p);
			}

			p += iLen;
		}
	}

	OoDWordToOad(pFrzCfg->dwOA, bOadBuf);
	iLen = OoGetDataLen(DT_OAD, bOadBuf);	//��ȡOAD����
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

//ִ�ж�������
bool DoFrzTask(WORD wTaskIndex, TFrzCtrl* pFrzCtrl)
{
	TTime tmNow;	

	GetCurTime(&tmNow);
	int to = Timeout(tmNow, wTaskIndex, pFrzCtrl);
	if (to == 0)	//ʱ�䵽
	{
		DTRACE(DB_TASK, ("DoFrzTask: wTaskIndex=%ld, wOI=%02x, dwOAD=%04x time out, do task.\n", pFrzCtrl->wOI, pFrzCtrl->tCfg.dwOA));
		if (FrzData(tmNow, pFrzCtrl))
			pFrzCtrl->tmLastRec = tmNow;	//����ɹ�������ʱ��
	}

	DoFrzDataCmd(wTaskIndex, pFrzCtrl);		//������������
	DoReFrzDataCmd(wTaskIndex, pFrzCtrl);	//��������������

	return true;
}

//����:��ȡָ�������ʱ�䡢������ŵ�һ�ʼ�¼
//����:@pbRecBuf ��������������һ�ʼ�¼
//���أ������ȷ�򷵻ػ������ڼ�¼�ĳ���,С��0��ʾ����,0��ʾ������
int SchFrzRec(char* pszName, const TTime& time)
{
	int fd = TdbOpenTable(pszName, O_RDONLY);
	if (fd < 0)
	{
		DTRACE(DB_TASK, ("SchComTaskRec: fail to open table %s\r\n", pszName));
		return -1;
	}

	TTdbSchRule TdbSchRule[1]; //һ������
	//��һ������ 
	TdbSchRule[0].wOpNum = 1; //�ȽϷ�������
	TdbSchRule[0].wField = 1; //�ֶ�1:����ʱ��
	TdbSchRule[0].wOp[0] = TDB_OP_EQ;	

	TdbSchRule[0].bVal[0][0] = (time.nYear>>8) & 0xff;
	TdbSchRule[0].bVal[0][1] = time.nYear & 0xff;
	TdbSchRule[0].bVal[0][2] = time.nMonth;
	TdbSchRule[0].bVal[0][3] = time.nDay;
	TdbSchRule[0].bVal[0][4] = time.nHour;
	TdbSchRule[0].bVal[0][5] = time.nMinute;
	TdbSchRule[0].bVal[0][6] = time.nSecond;

	TTdbSchCtrl TdbSchCtrl; //�������ƽṹ,֧�ֶ�������
	//�������
	TdbSchCtrl.wSortNum = 1;		//����������
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

	//���¶����¼�ṹ:����ʱ��(5)+������(2)+����ʱ��(5)+����(1)+����
	//int iRet = TdbReadRec(iSchId, TdbSchRule, sizeof(TdbSchRule)/sizeof(TTdbSchRule), TdbReadCtrl, pbRecBuf);
	TdbCloseSch(iSchId);
	TdbCloseTable(fd);

	//if (IsInvaildData(wTask, pbRecBuf+5+2))//ȥ������ʱ��+������
	//{
	//	iRet = -1;
	//}

	return TdbSchCtrl.wRecsFound;
}

//��������ȡ�����Զ���OI�����ڵ�λ
//���أ���������ڵ�λ���ɹ�������0��ʧ�ܣ�0
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

//�Ƿ��յ���������������
//���أ���Ҫ������ĵ���
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
		if (IsAllAByte(bBuf, 0, iLen) == false)		//�յ�����������
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

			if (bInterU == TIME_UNIT_YEAR)	//�갴12������
			{
				bInterV *= 12;
				bInterU = TIME_UNIT_MONTH;
			}

			iIntervPast = IntervsPast(tmStart, tmEnd, bInterU, bInterV);	//�ӻ�׼ʱ�俪ʼ�����������������ڸ���iPast	
			if (iIntervPast > 0)
				bDotNum = iIntervPast;

			memcpy(ptmStart, &tmStart, sizeof(TTime));	//������ʼʱ��

			memset(bBuf, 0, sizeof(bBuf));
			WriteItemEx(BN11, wPn, wCmdID, bBuf);	//�������
			//TrigerSaveBank(BN11, 0, -1);
		}
	}

	return bDotNum;
}


//��ʱ����¼�Ƿ����
bool IsFrzRecExist(WORD wOI, TTime& tmStart, TFrzCtrl* pFrzCtrl)
{
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
	char szTableName[TASK_PATH_LEN];

	sprintf(szTableName, FMT_FRZ_TASK_TABLE, wOI, pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);
	
	return (SchFrzRec(szTableName, tmStart) > 0);
}


//��ʼʱ�䰴�������
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
			dwStart = MonthFrom2000(tmStart) / bInterV * bInterV;	//��ȡ����BASETIME���·���

			tmStart.nSecond = 0;
			tmStart.nMinute = 0;
			tmStart.nHour = 0;
			tmStart.nDay = 1;
			tmStart.nYear = BASETIME;
			AddIntervs(tmStart, TIME_UNIT_MONTH, dwStart);
			break;

		case TIME_UNIT_YEAR:
			dwStart = tmStart.nYear - BASETIME;
			dwStart = dwStart / bInterV * bInterV;	//��ȡ����BASETIME������

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

//���������ӵķ���6 �������ݲ����ᣨ��ʼʱ�䣬��ֹʱ�䣩
void DoReFrzDataCmd(WORD wTaskIndex, TFrzCtrl* pFrzCtrl)
{
	BYTE bInterU, bInterV, bDotNum = 0;
	bool fRxReFrzCmd = false;
	DWORD dwIntervSec = 0;
	TTime tmStart = { 0 };
	TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;

	bInterU = GetFrzInterU(pFrzCtrl->wOI);
	bInterV = pFrzCfg->wCycle;		//WORD -> BYTE���ȿ��ܶ�ʧ
	bDotNum = GetReFrzDotNum(wTaskIndex, bInterU, bInterV, &tmStart);
	if (bDotNum>0 && bInterU>0 && bInterV>0)
	{	
		GetIntervTime(bInterU, bInterU, tmStart);
		while (bDotNum-- > 0)
		{
			if (!IsFrzRecExist(pFrzCtrl->wOI, tmStart, pFrzCtrl))		//�õ����ݲ�����
				FrzData(tmStart, pFrzCtrl);			//����ʵʱ����

			if (bInterU == TIME_UNIT_YEAR)
				AddIntervs(tmStart, TIME_UNIT_MONTH, pFrzCfg->wCycle*12);	//�갴12������
			else
				AddIntervs(tmStart, bInterU, pFrzCfg->wCycle);
		}
	}
}


//�Ƿ��յ�����3������������
bool IsRxFrzCmd(WORD wPn)
{
	int iLen;
	WORD wDelaySec;		//ÿ������1�������������
	DWORD dwClick, dwRxCmdClick;
	const WORD wCmdID = 0x0b10;
	BYTE bBuf[30];
	
	memset(bBuf, 0, sizeof(bBuf));
	iLen = ReadItemEx(BN11, wPn, wCmdID, bBuf);
	if (iLen > 0)
	{
		if (IsAllAByte(bBuf, 0, iLen) == false)		//�յ�������������
		{
			wDelaySec = OoLongUnsignedToWord(bBuf);		//�ӳ�ʱ��
			dwRxCmdClick = ByteToDWORD(bBuf+2, 4);	//��������ʱ��

			dwClick = GetClick();
			if (dwClick > dwRxCmdClick)
			{
				if (dwClick-dwRxCmdClick > wDelaySec)
				{
					memset(bBuf, 0, sizeof(bBuf));
					WriteItemEx(BN11, wPn, wCmdID, bBuf);	//�������
					//TrigerSaveBank(BN11, 0, -1);
					return true;
				}
			}
		}
	}

	return false;
}


//������3��������
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

	if (GetInfo(INFO_FRZDATA_RESET))	//�յ���λ�����ɾ�������³�ʼ��
	{
		ResetTaskData();
		InitTask(true);	//���³�ʼ��
	}

	for (i=0; i<FRZ_TASK_NUM; i++)
	{
		pFrzCtrl = &g_FrzCtrl[i];
		if (IsFrzTaskCfgValid(pFrzCtrl))	//����������Ч
			DoFrzTask(i, pFrzCtrl);
	}
}

/*˵��: ͨ�������ֱַ�ӻ��һ����¼
 *@szTbName:    ��򿪵��ļ���;
 *@bPtr: һ����¼�������ַ
 *@pbBuf:       �涳�����ݻ�����;
 *@iLen:        ���ݻ�������С;
 ����ֵ: <=0����; >0������ݵĴ�С
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


/*˵��: ��ȡ��N�ʼ�¼
 *@szTbName:    ��򿪵��ļ���;
 *@bPtr:		��N�ʼ�¼
 *@pbBuf:       �涳�����ݻ�����;
 *@iLen:        ���ݻ�������С;
 ����ֵ: <=0����; >0������ݵĴ�С
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


//���������ݵ�ǰ��ļ�¼�ŵõ����ڱ��еļ�¼����
//������@fd ���ݿ��ľ��; @iRecNo ��ļ�¼��(��1��ʼ)
//���أ���ǰ���¼�ŵļ�¼����,С��0��ʾ����
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

/*˵��:��ȡһ����¼�������ַ
 *@szTbName:    ��򿪵��ļ���;
 *@bPtr: Ҫ����ϴ��ĸ��µĶ�������,��1��ʼ;
 ����ֵ: <=0����; >0��õ������ַ
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

//�����������ɻ��ⲿ���õĴ����������ݽӿ�,����ʱ�α��л�������յȶ���
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


//����1����λ
int OnResetFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bPn;
	BYTE bBuf[10];

	if (GetOiClass(wOI)!=IC9 || bMethod!=FRZ_RESET)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}
	
	bPn = bBuf[0] = GetFrzType(wOI);
	if (bPn == 0xff)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	bBuf[1] = FRZ_CLR_VALID;	//��Ч��־
	WriteItemEx(BN11, bPn, 0x0b11, bBuf);
	//TrigerSaveBank(BN11, 0, -1); //��������

	SetInfo(INFO_FRZDATA_RESET);
	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;
}

//����2��ִ��
//�պ���
int OnRunFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	if (GetOiClass(wOI)!=IC9 || bMethod!=FRZ_RUN)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}
	
	//nothing to do

	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;
}



//����3������һ�μ�¼
//���������津����������
int OnRxTrigFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i;
	const WORD wCmdID = 0x0b10;
	DWORD dwClick;
	TFrzCtrl* pFrzCtrl;	

	if (bMethod != FRZ_TRIG)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	if (*pbPara != DT_LONG_U)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}
	
	dwClick = GetClick();
	memcpy(pbPara+3, &dwClick, sizeof(DWORD));

	for (i=0; i<FRZ_TASK_NUM; i++)
	{
		pFrzCtrl = &g_FrzCtrl[i];
		if (wOI==pFrzCtrl->wOI && IsFrzTaskCfgValid(pFrzCtrl))
			WriteItemEx(BN11, i, wCmdID, pbPara+1);				// ���ݸ�ʽ���£�2�ֽ��ӳ�ʱ�� + 4�ֽڵ�ǰclick
	}

	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;
}

//����6���������ݲ����ᣨ��ʼʱ�䣬��ֹʱ�䣩
//���������津�����ݲ���������
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
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	if (*pbPara++ != DT_DATE_TIME_S)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	OoDateTimeSToTime(pbPara, &tmStart);
	pbPara += 7;
	
	if (*pbPara++ != DT_DATE_TIME_S)
	{
		*pbRes = 3;	//�ܾ���д ��3��
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
			WriteItemEx(BN11, i, wCmdID, bBuf);		// ���ݸ�ʽ���£���ʼʱ��4 + ����ʱ��4
	}

	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;
}




//BYTE bCmpBuf[] = {0x02, 0x03, 0x12, 0x00, 0x00, 0x51, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00};
//����4�����һ����������������ԣ�������
//������=FRZRELA ��������������
int OnAddFrzAttrCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i;
	int iLen;
	BYTE bCapNum;
	BYTE bBuf[FRZRELA_ID_LEN];		//OAD����*sizeof(FRZRELA) + ��������1 + ����Ԫ�ظ���1	
		
	if (GetOiClass(wOI)!=IC9 || bMethod!=FRZ_ADDATTR || pbPara[0]!=DT_STRUCT || pbPara[1]!=3)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}	

	// ��ȡ�������Ա�
	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, FRZ_ATTRTAB, bBuf, NULL, NULL);
	if (iLen <= 0)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	bCapNum = bBuf[1];
	if (bCapNum >= CAP_OAD_NUM)		//�Ѿ������� ����ʧ��
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	for (i=0; i<bCapNum; i++)	//�����Ƿ��Ѿ�����
	{
		if (FieldCmp(DT_FRZRELA, &bBuf[i*DT_FRZRELA_LEN + 3], DT_FRZRELA, pbPara+1) == 0)	//��ȫ��ͬ������Ϊ����Ч����,��֤OADΨһ��
		{
			//*pbRes = 3;	//�ܾ���д ��3��
			//return -1;
			break;
		}
		else if (FieldCmp(DT_FRZRELA, &bBuf[i*DT_FRZRELA_LEN + 3], DT_OAD, pbPara+OFFSET_FRZ_OAD) == 0)	//OAD��ͬ�������ں���Ȳ�ͬ,�޸ĸù����������
		{
			memcpy(&bBuf[i*DT_FRZRELA_LEN + 2], pbPara, DT_FRZRELA_LEN);	//�޸Ĺ����������
			break;
		}
	}

	// �����һ��OAD
	if (i == bCapNum)
	{		
		memcpy(&bBuf[i*DT_FRZRELA_LEN + 2], pbPara, DT_FRZRELA_LEN);	//�����ӵķ������
		bCapNum++;
		bBuf[1] = bCapNum;	//����Ԫ�ظ���
	}

	// ˢ�¹������Ա�
	if (OoWriteAttr(wOI, FRZ_ATTRTAB, bBuf) <= 0)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	TrigerSaveBank(BN0, SECT5, -1);
	// ���ؽ��
	*pbRes = 0;	//�ɹ�  ��0��
	return 0;
}


//����5��ɾ��һ������������ԣ�������
//������=OAD ��������������
int OnDelFrzAttrCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i, j;
	int iLen;
	BYTE bCapNum;
	BYTE bBuf[FRZRELA_ID_LEN];		//OAD����*sizeof(FRZRELA) + ��������1 + ����Ԫ�ظ���1
		
	if (GetOiClass(wOI)!=IC9 || bMethod!=FRZ_DELATTR || pbPara[0]!=DT_OAD)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	// ��ȡ�������Ա�
	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, FRZ_ATTRTAB, bBuf, NULL, NULL);
	if (iLen <= 0)	//�յ�
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}	

	if (bBuf[1] == 0)	//�յ�
	{
		*pbRes = 0;	//�ɹ�
		return 0;
	}

	if (bBuf[1] > CAP_OAD_NUM)
		bCapNum = CAP_OAD_NUM;
	else
		bCapNum = bBuf[1];

	for (i=0; i<bCapNum; i++)	//�����Ƿ��Ѿ�����
	{
		if (FieldCmp(DT_FRZRELA, &bBuf[i*DT_FRZRELA_LEN + 3], DT_OAD, pbPara+1) == 0)	//�ҵ�Ŀ��
		{
			memset(&bBuf[i*DT_FRZRELA_LEN + 2], 0, DT_FRZRELA_LEN);
			bBuf[1]--;	//����Ԫ�ظ���
			break;
		}
	}
	
	if (i == bCapNum)	//û�ҵ�
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}
	
	for (j=i; j<bCapNum-1; j++)		//����Ĳ�����ǰŲ
	{
		memcpy(&bBuf[j*DT_FRZRELA_LEN + 2], &bBuf[(j+1)*DT_FRZRELA_LEN + 2], DT_FRZRELA_LEN);
	}
	
	memset(&bBuf[j*DT_FRZRELA_LEN + 2], 0, DT_FRZRELA_LEN);		//j=bCapNum-1��0���һ������OAD����

	// ˢ�¹������Ա�
	if (OoWriteAttr(wOI, FRZ_ATTRTAB, bBuf) <= 0)
	{
		*pbRes = 3;	//дʧ�� ��3��������
		return -1;
	}
	else
	{
		TrigerSaveBank(BN0, SECT5, -1);
		*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
		return 0;
	}
}


//����7��������ӹ������Ա�������
//������=array �������(FRZRELA)
int OnBatAddFrzAttrCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i, k;
	int iLen;
	BYTE bCapNum;
	BYTE bBuf[FRZRELA_ID_LEN];		//OAD����*sizeof(FRZRELA) + ��������1 + ����Ԫ�ظ���1	
		
	if (GetOiClass(wOI)!=IC9 || bMethod!=FRZ_BATADDATTR || pbPara[0]!=DT_ARRAY || pbPara[1]==0 || pbPara[1]>CAP_OAD_NUM)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}	

	// ��ȡ�������Ա�
	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, FRZ_ATTRTAB, bBuf, NULL, NULL);
	if (iLen <= 0)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	bCapNum = bBuf[1];
	if (bCapNum+pbPara[1] > CAP_OAD_NUM)		//�ռ䲻�� ����ʧ��
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}	

	for (k=0; k<pbPara[1]; k++)	//�������pbPara[1]��
	{
		for (i=0; i<bCapNum; i++)	//�����Ƿ��Ѿ�����
		{
			if (FieldCmp(DT_FRZRELA, &bBuf[i*DT_FRZRELA_LEN+3], DT_FRZRELA, &pbPara[k*DT_FRZRELA_LEN+3]) == 0)	//��ȫ��ͬ������Ϊ����Ч����,��֤OADΨһ��
			{
				//*pbRes = 3;	//�ܾ���д ��3��
				//return -1;
				break;
			}
			else if (FieldCmp(DT_FRZRELA, &bBuf[i*DT_FRZRELA_LEN + 3], DT_OAD, &pbPara[k*DT_FRZRELA_LEN+8]) == 0)	//OAD��ͬ�������ں���Ȳ�ͬ,�޸ĸù����������
			{
				memcpy(&bBuf[i*DT_FRZRELA_LEN+2], &pbPara[k*DT_FRZRELA_LEN+2], DT_FRZRELA_LEN);	//�޸Ĺ����������
				break;	//continue;
			}
		}

		// �����һ��OAD
		if (i == bCapNum)
		{		
			memcpy(&bBuf[i*DT_FRZRELA_LEN + 2], &pbPara[k*DT_FRZRELA_LEN+2], DT_FRZRELA_LEN);	//�����ӵķ������
			bCapNum++;
			bBuf[1] = bCapNum;	//����Ԫ�ظ���
		}
	}

	// ˢ�¹������Ա�
	if (OoWriteAttr(wOI, FRZ_ATTRTAB, bBuf) <= 0)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	TrigerSaveBank(BN0, SECT5, -1);
	// ���ؽ��
	*pbRes = 0;	//�ɹ�  ��0��
	return 0;
}




//����8����������������Ա�������
//������=NULL
int OnClrAttrTableCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD i, j;
	int iLen;
	BYTE bCapNum;
	BYTE bBuf[FRZRELA_ID_LEN];		//OAD����*sizeof(FRZRELA) + ��������1 + ����Ԫ�ظ���1
		
	if (GetOiClass(wOI)!=IC9 || bMethod!=FRZ_CLRATTR)
	{
		*pbRes = 3;	//�ܾ���д ��3��
		return -1;
	}

	// ��ȡ�������Ա�
	memset(bBuf, 0, sizeof(bBuf));
	bBuf[0] = DT_ARRAY;

	// ˢ�¹������Ա�
	if (OoWriteAttr(wOI, FRZ_ATTRTAB, bBuf) <= 0)
	{
		*pbRes = 3;	//дʧ�� ��3��������
		return -1;
	}
	else
	{
		TrigerSaveBank(BN0, SECT5, -1);
		*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
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



//��Э���GetRequestNormal��ʽ����¼�Ķ��ӿں���
//���أ�>0���ض����������ݵĳ��ȣ� <=0:ʧ��
int ReadFrzData(DWORD dwOAD, BYTE* pbBuf, WORD wBufSize, int* piStart)
{
	int iLen, iCnt;
	bool fGetHead = false, fSameCfg = false;
	BYTE bIndex, bRecPtr, bType;
	WORD i, wOI, wDataLen, wFmtLen, wItemOffset, wItemLen;
	DWORD dwSubOAD, dwRecSec;
	BYTE bBuf[FRZRELA_ID_LEN];		//OAD����*(sizeof(FRZRELA)+2) + ��������1 + ����Ԫ�ظ���1
	char szTableName[TASK_PATH_LEN];
	TFieldParser tDataFields = { bBuf };
	TTime tmRec = { 0 };
	BYTE bCfgBuf[20];
	BYTE bRecBuf[FRZ_REC_LEN];
	BYTE* pbBuf0 = pbBuf;
	BYTE* pbFmt = NULL;

	wOI = (dwOAD>>16) & 0xffff;
	bRecPtr = dwOAD & 0xff;		//�����N�ʼ�¼
	if (bRecPtr == 0)
		return 0;

	iLen = OoReadAttr(wOI, FRZ_ATTRTAB, tDataFields.pbCfg, &pbFmt, &wFmtLen);		//�����ֶ�
	if (iLen>0 && tDataFields.pbCfg[1]>0 && tDataFields.pbCfg[1]<=CAP_OAD_NUM)
	{
		tDataFields.wCfgLen = tDataFields.pbCfg[1]*DT_FRZRELA_LEN + 2;
		if (OoParseField(&tDataFields, pbFmt, wFmtLen, false) == false)
			return 0;

		fGetHead = false;		

		for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	//�����������
		{
			memset(bCfgBuf, 0, sizeof(bCfgBuf));
			iLen = ReadParserField(&tDataFields, bIndex, bCfgBuf, &bType, &wItemOffset, &wItemLen);		//�������õĶ������
			if (iLen > 0)
			{
				wDataLen = OoGetDataLen(DT_OAD, bCfgBuf+OFFSET_FRZ_OAD);
				if (wDataLen <= 0)		//��������OAD��Ч
				{
					*pbBuf++ = DT_NULL;		//��Ч����
					continue;
				}

				dwSubOAD = OoOadToDWord(bCfgBuf+OFFSET_FRZ_OAD);
				for (i=0; i<FRZ_TASK_NUM; i++)
				{
					TFrzCtrl* pFrzCtrl = &g_FrzCtrl[i];
					TFrzCfg* pFrzCfg = &pFrzCtrl->tCfg;
					fSameCfg = (dwSubOAD&~OAD_FEAT_MASK)==(dwOAD&~OAD_FEAT_MASK);	//�������Ƿ�һ��
					if (wOI==pFrzCtrl->wOI && dwSubOAD==pFrzCfg->dwOA && IsFrzTaskCfgValid(pFrzCtrl) && fSameCfg)		//OI���ͺ�OAD(������)����ͬ�����ղ����й������Ա�OAD˳���ȡ��¼
					{
						sprintf(szTableName, FMT_FRZ_TASK_TABLE, wOI, pFrzCfg->wCycle, pFrzCfg->dwOA, pFrzCfg->wMaxRecNum);
						iLen = ReadLastNRec(szTableName, bRecPtr, bRecBuf, sizeof(bRecBuf));
						if (iLen > 0)
						{
							if (fGetHead == false)	//�Ƿ��Ѿ���ȡ��¼��źʹ洢ʱ��ͷ
							{
								fGetHead = true;
								memcpy(pbBuf, bRecBuf, iLen);
								OoDateTimeSToTime(bRecBuf+REC_TIME_OFFSET, &tmRec);
								dwRecSec = TimeToSeconds(tmRec);

								pbBuf += iLen;
							}
							else	//ֻȡOAD����
							{
								OoDateTimeSToTime(bRecBuf+REC_TIME_OFFSET, &tmRec);
								if (GetAbsGap(dwRecSec, TimeToSeconds(tmRec)) > MAX_GAP_SEC)	//ͬһ����¼��ţ�����¼ʱ�����̫�� ������Ч����
								{
									*pbBuf++ = DT_NULL;		//��Ч����
								}
								else
								{
									iCnt = pbBuf-pbBuf0;
									if (iCnt+wDataLen >= wBufSize)	//buf�ռ䲻��
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
							*pbBuf++ = DT_NULL;		//��Ч����
						}

						break;	//�ҵ�Ŀ�꣬�˳�for (i=0; i<FRZ_TASK_NUM; i++)ѭ��
					}
				}
			}
		}
	}

	//*piStart++;	//�Ƿ���Ҫ���ӣ�
	return (int)(pbBuf-pbBuf0);
}


//�Ƿ񶳽���ͳ������
bool IsFrzStatData()
{
	WORD i;
	TFrzCfg* pFrzCfg = NULL;

	if (!g_fFrzInit)	//����δ��ʼ��ʱ����true
		return true;

	for (i=0; i<FRZ_TASK_NUM; i++)
	{
		if (IsFrzTaskCfgValid(&g_FrzCtrl[i]))
		{
			pFrzCfg = &g_FrzCtrl[i].tCfg;
			if ((pFrzCfg->dwOA&OAD_OI_MASK) == 0x21000200 || (pFrzCfg->dwOA&OAD_OI_MASK) == 0x22000200)		//�Ƿ񶳽���ͳ������
				return true;
		}
	}

	return false;
}

//����Э�鶳��������ʱ����ʱ����
void OnStatParaChg()
{
	if (IsFrzStatData())
		SetInfo(INFO_FRZPARA_CHG);
}




//��������ȡ�¼���¼��Ĺ̶��ֶκ������ֶΣ��ṩ�����нӿ�
bool GetFrzTaskRecFieldParser(DWORD dwROAD, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAttrTabBuf, WORD wBufSize)	
{
	BYTE* pbFmt;
	WORD wOI, wFmtLen = 0;
	int iLen;
	BYTE* pbCfg0 = NULL;

	//GetOIAttrIndex(dwROAD, &wOI, NULL, NULL);		

	// �̶��ֶ�
	if (pFixFields != NULL)
	{
		memset((BYTE*)pFixFields, 0, sizeof(TFieldParser));
		pFixFields->pbCfg = g_bFrzFixOAList;
		pFixFields->wCfgLen = sizeof(g_bFrzFixOAList);
		if (OoParseField(pFixFields, g_bFrzFixFmt, sizeof(g_bFrzFixFmt), true) == false)	//���й̶��ֶ�
		{	
			DTRACE(DB_TASK, ("GetFrzTaskFieldParser: wOI=%u OoParseField FixFields fail.\r\n", wOI));
			return false;
		}
	}

	//�����ֶΣ��������Ա����ΪNULL
	if (pDataFields != NULL)
	{
		pbCfg0 = pDataFields->pbCfg;
		if (pbCfg0 == NULL)
			return false;

		memset((BYTE*)pDataFields, 0, sizeof(TFieldParser));
		pDataFields->pbCfg = pbCfg0;
		pDataFields->pbCfg[0] = 0x01;	//��������
		pDataFields->pbCfg[1] = 0x01;	//����Ԫ�ظ���
		pDataFields->pbCfg[2] = DT_OAD;	//��������

		OoDWordToOad(dwROAD, &pDataFields->pbCfg[3]);	//OAD			
					
		pDataFields->wCfgLen = SUB_DATD_FIELD_LEN;

		if (OoParseField(pDataFields, g_bFrzSubDataFmt, sizeof(g_bFrzSubDataFmt), true) == false)		//�����ֶ�
		{
			DTRACE(DB_TASK, ("GetFrzTaskFieldParser: dwROAD=%04x OoParseField DataFields fail.\r\n", dwROAD));
			return false;
		}
	}

	return true;
}


//�յ�����/����/�¼���¼��ʼ������ʱ�Ĵ����ⲿ����
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
		WriteItemEx(BN11, wPn, 0x0b11, bBuf);		//д�����ʶ
	}

	//TrigerSaveBank(BN11, 0, -1);
	SetInfo(INFO_FRZDATA_RESET);
}
