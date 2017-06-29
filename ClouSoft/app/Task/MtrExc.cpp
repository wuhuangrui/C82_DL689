/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MtrExc.cpp
 * ժ    Ҫ�����ļ���Ҫʵ���������Э��ĳ����¼�
 * ��ǰ�汾��1.0
 * ��    �ߣ�
 * ������ڣ�2016��10��
 *********************************************************************************************************/
#include "stdafx.h"
#include "syscfg.h"
#include "sysfs.h"
#include "FaCfg.h"
#include <fcntl.h>
#include "ComStruct.h"
#include "ComAPI.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "DbOIAPI.h"
#include "CctTaskMangerOob.h"
#include "MtrExc.h"
#include "TermEvtTask.h"
#include "OoFmt.h"
#include "MeterAPI.h"
#include "MtrProAPI.h"
#include "MeterStruct.h"
#include "CctAPI.h"
#include "Mem.h"
#include "MtrHook.h"
#include "MtrCtrl.h"

extern bool UpdateMtrExcStatData(WORD wOI, BYTE bState, TMtrExcTmp* pExcTmp, BYTE* pbTsa);
extern bool DoMtrClockErr(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);
extern bool DoMtrEnergyDec(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);
extern bool DoMtrEnergyErr(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);
extern bool DoMtrFlew(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);
extern bool DoMtrStop(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);
extern bool DoMtrRdFail(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);
extern bool DoMtrDataChg(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn);


#define TRY_READ_NUM		3	//�����س�����
//#define MTR_EXC_NUM		(sizeof(g_wMtrExcOI)/sizeof(WORD))

const WORD g_wMtrExcOI[MTR_EXC_NUM] = { OI_MTR_CLOCK_ERR, OI_MTR_ENERGY_DEC, OI_MTR_ENERGY_ERR, OI_MTR_FLEW, OI_MTR_STOP, OI_MTR_RD_FAIL, OI_MTR_DATA_CHG};


//�¼��ж��õ���OAD�б�
static DWORD g_dwJudgeOADList[][MAX_JUDGE_OAD] = 
{
	{ OI_MTR_CLOCK_ERR,		0x40000200,		0 },
	{ OI_MTR_ENERGY_DEC,	0x00100201,		0x00200201,		0 },
	{ OI_MTR_ENERGY_ERR,	0x00100200,		0x00200200,		0 },
	{ OI_MTR_FLEW,			0x00100200,		0x00200200,		0 },
	{ OI_MTR_STOP,			0x00100201,		0x00200201,		0x20040201,		0 },
	{ OI_MTR_RD_FAIL,		0 },	//����ʧ���¼��жϲ���������ID
	{ OI_MTR_DATA_CHG,		0 },	//���ܱ����ݱ����ؼ�¼����ID���������õ�������Ҫ���������˴�����������
};

//��׼�¼���¼��Ԫ
BYTE g_bMtrExcFixOAList[] = {
	DT_ARRAY,
	5,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	//��¼���
	DT_OAD, 0x20, 0x1E, 0x02, 0x00, //����ʱ��
	DT_OAD, 0x20, 0x20, 0x02, 0x00, //����ʱ��
	DT_OAD, 0x20, 0x24, 0x02, 0x00, //����Դ
	DT_OAD, 0x33, 0x00, 0x02, 0x00, //ͨ���ϱ�״̬
};	//�̶��ֶ���������


//���ܱ�ʱ�ӳ����¼��Ԫ
BYTE g_bMtrClockExcFixOAList[] = {
	DT_ARRAY,
	7,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	//��¼���
	DT_OAD, 0x20, 0x1E, 0x02, 0x00, //����ʱ��
	DT_OAD, 0x20, 0x20, 0x02, 0x00, //����ʱ��
	DT_OAD, 0x20, 0x24, 0x02, 0x00, //����Դ
	DT_OAD, 0x33, 0x00, 0x02, 0x00, //ͨ���ϱ�״̬
	DT_OAD, 0x40, 0x00, 0x02, 0x00, //���ܱ�ʱ��
	DT_OAD, 0x40, 0x00, 0x02, 0x00, //�ն�ʱ��
};	//�̶��ֶ���������

//���ܱ����ݱ����ؼ�¼��Ԫ
BYTE g_bMtrDataChgExcFixOAList[] = {
	DT_ARRAY,
	8, //5,//  7,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,//�¼���¼���  double-long-unsigned��
	DT_OAD, 0x20, 0x1E, 0x02, 0x00, //�¼�����ʱ��  date_time_s��
	DT_OAD, 0x20, 0x20, 0x02, 0x00, //�¼�����ʱ��  date_time_s��
	DT_OAD, 0x20, 0x24, 0x02, 0x00, //�¼�����Դ    TSA��
	DT_OAD, 0x33, 0x00, 0x02, 0x00, //�¼��ϱ�״̬  array ͨ���ϱ�״̬��
	DT_OAD, 0x33, 0x0F, 0x02, 0x06, //������ݶ���  CSD��(5B)
	DT_OAD, 0x33, 0x0F, 0x02, 0x07, //�仯ǰ����    Data
	DT_OAD, 0x33, 0x0F, 0x02, 0x08, // �仯������    Data
};	//�̶��ֶ���������

BYTE g_bStdMtrExcFixFmt[] = {
	DT_ARRAY,
	5,
	DT_OAD,
};	//�̶��ֶθ�ʽ������

BYTE g_bMtrClockExcFixFmt[] = {
	DT_ARRAY,
	7,
	DT_OAD,
};	//�̶��ֶθ�ʽ������

BYTE g_bMtrDataChgMtrExcFixFmt[] = {
	DT_ARRAY,
	8,
	DT_OAD,
};	//�̶��ֶθ�ʽ������

TFieldParser g_tStdFixFields = { g_bMtrExcFixOAList, sizeof(g_bMtrExcFixOAList) };	//�̶��ֶ�
TFieldParser g_tMtrClockFixFields = { g_bMtrClockExcFixOAList, sizeof(g_bMtrClockExcFixOAList) };	//�̶��ֶ�
TFieldParser g_tMtrDataChgFixFields = { g_bMtrDataChgExcFixOAList, sizeof(g_bMtrDataChgExcFixOAList) };	//�̶��ֶ�


//�����¼���¼��Ԫ�̶��ֶ�����
TMtrExcFixUnitDes g_MtrExcFixUnitDesc[] = {
	{ OI_MTR_CLOCK_ERR,		&g_tMtrClockFixFields,	g_bMtrClockExcFixFmt,	sizeof(g_bMtrClockExcFixFmt) },	//ʱ�ӳ����¼���Ԫ
	{ OI_MTR_ENERGY_DEC,	&g_tStdFixFields,		g_bStdMtrExcFixFmt,		sizeof(g_bStdMtrExcFixFmt) },	//��׼�¼���Ԫ
	{ OI_MTR_ENERGY_ERR,	&g_tStdFixFields,		g_bStdMtrExcFixFmt,		sizeof(g_bStdMtrExcFixFmt) },	//��׼�¼���Ԫ
	{ OI_MTR_FLEW,			&g_tStdFixFields,		g_bStdMtrExcFixFmt,		sizeof(g_bStdMtrExcFixFmt) },	//��׼�¼���Ԫ
	{ OI_MTR_STOP,			&g_tStdFixFields,		g_bStdMtrExcFixFmt,		sizeof(g_bStdMtrExcFixFmt) },	//��׼�¼���Ԫ
	{ OI_MTR_RD_FAIL,		&g_tStdFixFields,		g_bStdMtrExcFixFmt,		sizeof(g_bStdMtrExcFixFmt) },	//��׼�¼���Ԫ
	{ OI_MTR_DATA_CHG,		&g_tMtrDataChgFixFields,		g_bMtrDataChgMtrExcFixFmt,		sizeof(g_bMtrDataChgMtrExcFixFmt) },	//���ܱ����ݱ����ؼ�¼��Ԫ
};

//����:ȡ���ID����ID��ӳ������
DWORD* GetJudgeOADList(WORD wOI)
{	
	//����BANK��BANK0��ͬ�������ID����
	WORD wNum = sizeof(g_dwJudgeOADList) / (sizeof(DWORD)*MAX_JUDGE_OAD);
	for (WORD i=0; i<wNum; i++)
	{
		if (wOI == g_dwJudgeOADList[i][0])
		{
			if (wOI == OI_MTR_DATA_CHG)		//���ܱ����ݱ����ؼ�¼��CSD�����ݣ�������OAD����ʹ�ô˺�����Ҫ����NULL.����MtrExc.cpp�������ݣ�������
				return NULL;
			
			return &g_dwJudgeOADList[i][1];
		}
	}
	
	return NULL;
}


//����ֵ���¼���ţ�>=0��ʾ�ɹ��� <0ʧ��
int GetMtrExcIndex(WORD wOI)
{	
	for (WORD i=0; i<MTR_EXC_NUM; i++)
	{
		if (wOI == g_wMtrExcOI[i])
			return i;
	}

	return -1;
}

//��ȡ�¼�OI
WORD GetMtrExcOI(BYTE bExcIndex)
{
	if (bExcIndex >= MTR_EXC_NUM)
		return 0;

	return g_wMtrExcOI[bExcIndex];
}

//��ѯ�Ƿ��յ�����������յ�������ʱ�䵽�������������ٷ���true,����false
//���أ��Ƿ񴥷�����
bool IsTrigerSaveRec(WORD wPn, WORD wOI, BYTE bLastState)
{
	int iOffset, iLen;
	BYTE bTrigerState = 0;
	WORD wID, wCmdID, wDelayHapSec, wDelayRecvSec;
	DWORD dwCurSec = 0, dwRxCmdSec = 0;
	const WORD wBaseID = 0x0b20;
	BYTE bBuf[30];

	iOffset = GetMtrExcIndex(wOI);
	if (iOffset < 0)
		return false;

	wID = wBaseID + iOffset;
	memset(bBuf, 0, sizeof(bBuf));
	iLen = ReadItemEx(BN11, wPn, wID, bBuf);
	if (iLen > 0)
	{
		if (IsAllAByte(bBuf, 0, iLen) == false)		//�յ�������������
		{
			bTrigerState = bBuf[8];
			if (bTrigerState == 0)
			{
				bBuf[8] = bTrigerState = 1;	//����״̬�����յ����� 
				WriteItemEx(BN11, wPn, wID, bBuf);
			}

			wDelayHapSec = ByteToWord(bBuf, 2);		//�ӳٷ���ʱ��
			wDelayRecvSec = ByteToWord(bBuf+2, 2);	//�ӳٻָ�ʱ��
			dwRxCmdSec = ByteToDWORD(bBuf+4, 4);	//��������ʱ��			

			dwCurSec = GetCurTime();
			switch (bTrigerState)	//״̬��  ����0 -> �յ�����1 -> ��������2 -> �����ָ�3 -> ����0(�������)
			{
			case 1:
				if (bLastState==EVT_S_AFT_HP || bLastState==EVT_S_BF_END)	//�Ѿ�����
				{
					return true;	//ǿ�ƻָ��¼�
				}
				else
				{
					if (dwCurSec-dwRxCmdSec>wDelayHapSec && dwCurSec>dwRxCmdSec)
					{
						bBuf[8] = bTrigerState = 2;		//����״̬���� ���������¼�
						WriteItemEx(BN11, wPn, wCmdID, bBuf);
						return true;
					}
				}
				break;
			case 2:
				if (dwCurSec-dwRxCmdSec>(wDelayRecvSec+wDelayHapSec) && dwCurSec>dwRxCmdSec)
				{
					bBuf[8] = bTrigerState = 3;		//����״̬���� �����ָ��¼�
					WriteItemEx(BN11, wPn, wCmdID, bBuf);
					return true;
				}
				break;
				
			case 3:
				memset(bBuf, 0, sizeof(bBuf));	//����״̬���� �������
				WriteItemEx(BN11, wPn, wCmdID, bBuf);
				TrigerSaveBank(BN11, 0, -1);
				break;
			default:
				memset(bBuf, 0, sizeof(bBuf));	//�������
				WriteItemEx(BN11, wPn, wCmdID, bBuf);
				break;
			}
		}
	}

	return false;
}


//�Ƿ��յ�������������
//����:@wOI:
//	   @bLastState:�ϴ�״̬��
//	   @pbState:���ص�ǰ״̬���������¼ʹ��
bool IsRxTrigerSaveCmd(WORD wPn, WORD wOI, BYTE bLastState, BYTE* pbState)
{
	bool fTrigerSave;

	fTrigerSave = IsTrigerSaveRec(wPn, wOI, bLastState);
	if (fTrigerSave)
	{
		if (bLastState==EVT_S_AFT_HP || bLastState==EVT_S_BF_END)
			*pbState = EVT_S_AFT_END;	//�ѷ����������ָ��¼�
		else
			*pbState = EVT_S_AFT_HP;	//�ѻָ������������¼�
	}

	return fTrigerSave;
}

//���ݸ�ʽ��ȡ�й�����ֵ
//������@bFmt:DT_LONG64_U�߾��ȵ���ֵ�� ����:�;��ȵ���ֵ

uint64 GetEnergyValByFmt(BYTE bFmt, BYTE* pbBuf, BYTE* pbLen)
{
	uint64 ui64Val = 0;

	if (pbBuf[0] == DT_LONG64_U)	//�߾���
	{
		*pbLen = 9;
		ui64Val = OoLong64UnsignedTouUint64(pbBuf+1);
	}
	else	//�;���
	{
		*pbLen = 5;
		ui64Val = OoDoubleLongUnsignedToDWord(pbBuf+1);
	}

	return ui64Val;
}

TMtrExcFixUnitDes* GetMtrExcFixUnit(WORD wOI)
{
	WORD i;
	TFieldParser tFixParser;
	for (i=0; i<sizeof(g_MtrExcFixUnitDesc) / (sizeof(TMtrExcFixUnitDes)); i++)
	{
		if (wOI == g_MtrExcFixUnitDesc[i].wOI)
		{
			memset((BYTE* )&tFixParser, 0, sizeof(TFieldParser));	//������ֶν�������ʼ��

			tFixParser.pbCfg = g_MtrExcFixUnitDesc[i].pFixField->pbCfg;
			tFixParser.wCfgLen = g_MtrExcFixUnitDesc[i].pFixField->wCfgLen;

			memcpy((BYTE* )g_MtrExcFixUnitDesc[i].pFixField, (BYTE* )&tFixParser, sizeof(TFieldParser));	//�ṹ�������ֵ
			return &g_MtrExcFixUnitDesc[i];
		}
	}

	return NULL;
}

//��������쳣�ĳ�ʼ��
bool InitSubMtrExc(BYTE bIndex)
{
	int iRet;
	DWORD dwOAD;
	WORD wOI, wFmtLen = 0, wMaxNum = 0;
	TFieldParser tDataFields;

	BYTE bBuf[EVT_ATTRTAB_LEN] = {0};	
	BYTE* pbFmt = NULL;
	const ToaMap* pOaMap = NULL;

	memset((BYTE*)&tDataFields, 0, sizeof(TFieldParser));

	wOI = GetMtrExcOI(bIndex);
	if (wOI == 0)
		return false;	

	SetMtrExcOadDefCfg(wOI);

	const TMtrExcFixUnitDes* pUnitDesc = GetMtrExcFixUnit(wOI);
	if (pUnitDesc == NULL)
		return false;

	if (OoParseField(pUnitDesc->pFixField, pUnitDesc->pbFmt, pUnitDesc->wFmtLen, true) == false)		// �̶��ֶ�
		return false;	

	memset(bBuf, 0, sizeof(bBuf));
	tDataFields.pbCfg = bBuf;
	tDataFields.wCfgLen = OoReadAttr(wOI, ATTR3, tDataFields.pbCfg, &pbFmt, &wFmtLen);	//�������Ա�
	if (tDataFields.wCfgLen <= 0)
		return false;

	if (OoParseField(&tDataFields, pbFmt, wFmtLen, true) == false)
		return false;

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(wOI, ATTR5, bBuf, NULL, NULL) <= 0)	//����5 ����¼��
	{
		DTRACE(DB_METER_EXC, ("InitEvt: wOI=%u Init fail because Read wMaxNum fail.\r\n", wOI));
		return false;
	}

	wMaxNum = OoLongUnsignedToWord(bBuf+1);
	if (wMaxNum == 0)	// ����¼��Ϊ0��������
		return false;

	dwOAD = GetOAD(wOI, ATTR2, 0);
	pOaMap = GetOIMap(dwOAD);
	if (pOaMap == NULL)
		return false;

	iRet = CreateTable(pOaMap->pszTableName, pUnitDesc->pFixField, &tDataFields, wMaxNum);
	if (iRet <= 0)
		return false;

	SetMtrExcTableFlag(wOI);		//��λ������ɱ�־
	DTRACE(DB_METER_EXC, ("InitMtrExc: bIndex=%d, Init OK.\r\n", bIndex));
	return true;
}


//�ⲿ�ӿں��������߳��ϵ��ʼ������
//��ʼ������
void InitMtrExc()
{
	BYTE bIndex;
	for (bIndex=0; bIndex<MTR_EXC_NUM; bIndex++)
	{
		InitSubMtrExc(bIndex);
	}
}



//��������ȡ����쳣�ĳ���������
//������@wOI�¼���OI
//		@pwJudgeOAD���������ж���Ҫ������OAD����
//		@pwJudgeOadNum���������ж���Ҫ������OAD����ĸ���
//		@pbRelaOAD	�������ع����������Ա�
//����:����¼���Ҫ�ж���������ȷ�򷵻�true,���򷵻�false
bool GetMtrExcRdItem(WORD wOI, DWORD* pdwJudgeOAD, WORD* pwJudgeOadNum, BYTE* pbRelaOAD)
{	
	int iLen;
	BYTE bBuf[40];
	DWORD* pdwSubOAD;

	iLen = OoReadAttr(wOI, ATTR9, bBuf, NULL, NULL);		//����Ч��ʶ
	if (iLen <= 0 || bBuf[0] != DT_BOOL)
		return false;

	if (bBuf[1] == 0)	//��Ч
		return false;

	pdwSubOAD = GetJudgeOADList(wOI);
	if (pdwSubOAD == NULL)
		return false;

	*pwJudgeOadNum = 0;
	while(*pdwSubOAD != 0)
	{
		*pdwJudgeOAD++ = *pdwSubOAD++;
		(*pwJudgeOadNum)++;
		if (*pwJudgeOadNum >= MAX_JUDGE_OAD-1)
			break;
	}

	iLen = OoReadAttr(wOI, ATTR3, pbRelaOAD, NULL, NULL);		//���������Ա�
	if (iLen <= 0)
		return false;
	else
		return true;
}


//**************************�¼���¼��ȡ�ӿ�*********************************************
int GetMtrExcEvtSpecField(DWORD dwFieldOAD, BYTE* pbField, WORD wFieldLen, BYTE* pbStart)
{
	BYTE bArrayNum;
	int iRet = -1;

	*pbStart = 0;	//�������ݵ���ʼλ��

	switch(dwFieldOAD)
	{
		case 0x20200200:		//�¼�����ʱ��
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_DATE_TIME_S)
				iRet = 8;		//ʱ�䳤�ȣ��Ѽ�����
			break;	
		case 0x20240200:		//�¼�����Դ
			iRet = pbField[1] + 2;	
			break;
		/*case 0x330C0206:		//�¼������б�
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_ARRAY)
			{
				bArrayNum = pbField[1];
				iRet = 2+5*bArrayNum;
			}			
			break;	*/
		case 0x33000200:		//�¼��ϱ�״̬
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_ARRAY)
			{
				bArrayNum = pbField[1];
				iRet = 2+CN_RPT_STATE_LEN*bArrayNum;
			}
			break;	
		case 0x330F0206:	
		case 0x330F0207:		
		case 0x330F0208:		
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else
			{
				iRet = pbField[0];
				*pbStart = 1;
			}			
			break;			
		default:
			if (pbField[0] == DT_NULL)	//��Ч���ݷ���NULL
				iRet = 1;
			else
				return wFieldLen;
	}

	if ((iRet>0) && (iRet<=wFieldLen))
		return iRet;
	else
		return 1;
}

//��������ȡ�¼��̶��ֶ�/�����ֶ�
//������ @wOI �����ʶ
//		@pFixFields ���صĹ̶��ֶ�
//		@pDataFields ���ص������ֶ�
//		@pbAtrrTabBuf �������Ա�����
//		@wBufSize pbDataCfg�������Ĵ�С
//���أ���ȷ��ȡ���̶��ֶ�/�����ֶη���true�����򷵻�false
bool GetMtrExcFieldParser(WORD wOI, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAtrrTabBuf, WORD wBufSize)
{
	int iLen;
	WORD wFmtLen = 0;
	TFieldParser tDataFields = { 0 };
	DWORD dwROAD;

	BYTE* pbFmt = NULL;
	TFieldParser* pTmpDataFields = &tDataFields;

	// �̶��ֶ�
	if (pFixFields != NULL)
	{
		const TMtrExcFixUnitDes* pUnitDesc = GetMtrExcFixUnit(wOI);
		if (pUnitDesc == NULL)
			return false;
		
		if (OoParseField(pUnitDesc->pFixField, pUnitDesc->pbFmt, pUnitDesc->wFmtLen, true) == false) 	//�̶��ֶ�
			return false;

		memcpy((BYTE* )pFixFields, (BYTE* )pUnitDesc->pFixField, sizeof(TFieldParser));	//�ṹ�������ֵ
	}

	//�����ֶΣ��������Ա����ΪNULL
	if (pDataFields != NULL)
	{
		/*if (wBufSize < EVT_ATTRTAB_LEN)
			return false;*/

		memset(pbAtrrTabBuf, 0, wBufSize);
		pTmpDataFields->pbCfg = pbAtrrTabBuf;
		iLen= OoReadAttr(wOI, ATTR3, pTmpDataFields->pbCfg, &pbFmt, &wFmtLen);
		if (iLen > 0)
		{
			pTmpDataFields->wCfgLen = iLen;
			if (OoParseField(pTmpDataFields, pbFmt, wFmtLen, true) == false)
			{	
				//DTRACE(DB_INMTR, ("GetEvtFieldParser: wOI=%u OoParseField() pDataFields fail.\r\n", wOI));
				return false;
			}
		}
		else
		{
			memset(pbAtrrTabBuf, 0, wBufSize);
			pDataFields->pbCfg = NULL;
			pDataFields->wNum= 0;
		}

		memcpy((BYTE* )pDataFields, (BYTE* )pTmpDataFields, sizeof(TFieldParser));	//�ṹ�������ֵ
	}

	dwROAD = GetOAD(wOI, ATTR2, 0);	//�¼���ӦOAD
	DelEvtOad(dwROAD, 0);
	return true;
}

int OoProReadMtrExcEvtRecord(WORD wOI, BYTE bAttr, BYTE* pbRecBuf, WORD wRecLen, WORD wBufSize)
{
	BYTE bType, bOadBuf[10], bAttrBuf[EVT_ATTRTAB_LEN] = {0};
	BYTE bTmpRecBuf[MTR_EXC_REC_LEN];	//һ����¼������
	BYTE* pbTmpRec = bTmpRecBuf;
	BYTE* pbRec = pbRecBuf;
	WORD wItemOffset, wItemLen, wTotalLen;
	DWORD dwOAD;
	int iLen;
	TFieldParser tFixFields;	//�̶��ֶ�
	TFieldParser tDataFields;
	const ToaMap* pOaMap = NULL;
	BYTE bIndex;
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	WORD wFmtLen = 0;
	BYTE bStart = 0;

	memset((BYTE* )&tFixFields, 0, sizeof(TFieldParser));
	memset((BYTE* )&tDataFields, 0, sizeof(TFieldParser));

	if (GetMtrExcFieldParser(wOI, &tFixFields, &tDataFields, bAttrBuf, sizeof(bAttrBuf)) == false)
		return -1;

	OoReadAttr(wOI, ATTR3, bRelaOAD, NULL, NULL);		//���������Ա�

	//�����������ͺ�Ԫ�ظ���
	wTotalLen = 0;
	*pbTmpRec++ = DT_STRUCT;
	wTotalLen++;
	*pbTmpRec++ = tFixFields.wNum + bRelaOAD[1];
	wTotalLen++;

	//����̶��ֶ��ϱ���Ϣ,�¼�����Դ����������
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)
			return -1;	//ֱ�ӷ��أ��̶��ֶβ�Ӧ��������

		if (bType!=DT_OAD || wItemLen==0)
			return -1;

		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);

		//��������
		iLen = GetMtrExcEvtSpecField(dwOAD, pbRec, wItemLen, &bStart);
		if (iLen>0 && iLen<=wItemLen)
			memcpy(pbTmpRec, pbRec+bStart, iLen);
		else
			return -1;

		pbRec += wItemLen;	 
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}

	memcpy(pbTmpRec, pbRec, tDataFields.wTotalLen);
	wTotalLen += tDataFields.wTotalLen;
	if (wTotalLen <= wBufSize)
	{
		memcpy(pbRecBuf, bTmpRecBuf, wTotalLen);
		return wTotalLen;
	}
	else
	{
		return -1;
	}
}

//����������һ���¼���¼���ṩ�����нӿ�
//������@wOI 	�����ʶ
//		@bAtrr	���Ա�ʶ�������� bit-string��SIZE��8����
//		@bIndex������Ԫ������
//		@pbRecBuf��¼���ջ�����
//		@wBufSize��¼���ջ������Ĵ�С
//���أ���ȷ���ؼ�¼�ĳ��ȣ����򷵻ظ���
int GetMtrExcEvtRecord(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbRecBuf, WORD wBufSize)
{
	DWORD dwROAD;
	int iLen;
	char* pszFileName;

	dwROAD= GetOAD(wOI, bAttr, bIndex);
	DelEvtOad(dwROAD, 0);

	pszFileName = GetEvtRecFileName(dwROAD&0xffff1f00);
	if (pszFileName == NULL)
		return -1;

	// ��ȡ��¼
	iLen = ReadLastNRec(pszFileName, bIndex, pbRecBuf, wBufSize);
	if (iLen <= 0)
		return iLen;

	return OoProReadMtrExcEvtRecord(wOI, bAttr, pbRecBuf, iLen, wBufSize);
}


BYTE DoMtrExc(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn, bool* pfModified)
{
	//��������Ҫ�����������ͳһ������
	int iRet, nOADLen;
	DWORD dwOAD;
	WORD i, wIndex, wJudgeOadNum = 0;
	WORD wRSDLen = 0, wRCSDLen = 0;
	bool fTrigerSave;
	WORD wOI, wFailCnt = 0;
	BYTE bRSDBuf[32], bRCSDBuf[256];
	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	BYTE bBuf[60];
	BYTE bOADBuf[4];

	if (GetInfo(INFO_MTR_EXC_RESET))		
	{
		DTRACE(DB_FA, ("DoMtrExc rx INFO_MTR_EXC_RESET...\n"));
		ClrMtrExc();	//�¼�����
		InitMtrExc();	//��ʼ������ ���¼��ж��м������ڳ����߳��У���Ч������ų�ʼ����
	}

	for (i=0; i<MTR_EXC_NUM; i++)
	{
		memset(bRelaOAD, 0, sizeof(bRelaOAD));
		memset(dwJudgeOAD, 0, sizeof(dwJudgeOAD));

		wOI = GetMtrExcOI(i);

		if (IsMtrExcTableCreate(wOI) == false)	//��δ����, �¼������ж���
			continue;

		wJudgeOadNum = 0;
		if (GetMtrExcRdItem(wOI, dwJudgeOAD, &wJudgeOadNum, bRelaOAD))
		{
			for (wIndex=0; wIndex<wJudgeOadNum; wIndex++)	//wJudgeOADȡ����ID��Ҫ��Ϊ�˷�ֹ��������û�г�������Ҫ��������ʱ�������ٳ�һ��
			{
				dwOAD = dwJudgeOAD[wIndex];
				if (dwOAD == 0x40000200)	//hyl 3105�������ﳭ��
					continue;

				if (GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bBuf) <= 0)	//û����
				{
					iRet = AskMtrItem(pMtrPro, RESPONSE_TYPE_NORAML, dwOAD, bBuf, bRSDBuf, wRSDLen, bRCSDBuf, wRCSDLen);
					if (iRet > 0)	//��������
					{
						OoDWordToOad(dwOAD, bOADBuf);
						nOADLen = OoGetDataLen(DT_OAD, bOADBuf);
						if (nOADLen > 0)
						{
#ifdef TERM_EVENT_DEBUG
							SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bBuf, nOADLen);
#else
							SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bBuf+1, nOADLen);	//+1 ����DAR
#endif
							SaveMtrDataHook(dwOAD, &pMtrRdCtrl->mtrExcTmp, 0);
							*pfModified = true; //�������������޸�
						}
						wFailCnt = 0;
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
						wFailCnt++;
						if (wFailCnt >= 3)
						{
							OnMtrErrEstb(wPn);
							DoPortRdErr(true);
							break;
						}
					}
					else
					{
						OoDWordToOad(dwOAD, bOADBuf);
						nOADLen = OoGetDataLen(DT_OAD, bOADBuf);
						memset(bBuf, INVALID_DATA, sizeof(bBuf));
						if (nOADLen > 0)
						{
#ifdef TERM_EVENT_DEBUG
							SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bBuf, nOADLen);
#else
							SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bBuf+1, nOADLen);	//+1 ����DAR
#endif
							SaveMtrDataHook(dwOAD, &pMtrRdCtrl->mtrExcTmp, 0);
							*pfModified = true; //�������������޸�
						}
					}
				}
			}
		}

		//����Ӧ���¼������н��з����������жϣ�����״̬��
		//�ٵ���ReadAndSaveMtrExcRec()�����ݼ������¼
		switch(wOI)
		{
		case OI_MTR_CLOCK_ERR:
			DoMtrClockErr(pMtrRdCtrl, pMtrPro, wPn);
			break;

		case OI_MTR_ENERGY_DEC:
			DoMtrEnergyDec(pMtrRdCtrl, pMtrPro, wPn);
			break;

		case OI_MTR_ENERGY_ERR:
			DoMtrEnergyErr(pMtrRdCtrl, pMtrPro, wPn);
			break;
			
		case OI_MTR_FLEW:
			DoMtrFlew(pMtrRdCtrl, pMtrPro, wPn);
			break;

		case OI_MTR_STOP:
			DoMtrStop(pMtrRdCtrl, pMtrPro, wPn);
			break;

		case OI_MTR_RD_FAIL:
			DoMtrRdFail(pMtrRdCtrl, pMtrPro, wPn);	//����ֻ�����貶��Ĺ���OAD���ݺ�ͳ������
			break;
		case OI_MTR_DATA_CHG:
			DoMtrDataChg(pMtrRdCtrl, pMtrPro, wPn);	
			break;
		default:
			break;
		}
	}

	return 1;
}

//��ȡ���һ���¼�����洢λ��
//���أ�>=0�ɹ���<0ʧ��
int GetLastRecPhyIdx(WORD wOI, TMtrExcTmp* pMtrTmp)
{
	int nIndex;

	nIndex = GetMtrExcIndex(wOI);
	if (nIndex < 0)
		return -1;

	return pMtrTmp->wLastRecPhyIdx[nIndex];
}

void UpdateLastRecPhyIdx(WORD wOI, TMtrExcTmp* pMtrTmp, int nRecPhyIdx)
{
	int nIndex;

	nIndex = GetMtrExcIndex(wOI);
	if (nIndex<0 || nRecPhyIdx<0)
		return;
	else
		pMtrTmp->wLastRecPhyIdx[nIndex] = nRecPhyIdx;
}


//������������쳣�Ĺ���������,���浽��ʱ��¼�����ڷ�����������󱣴��¼�������
//������@pMtrRdCtrl���������
//		@wOI�¼���OI
//		@bState�¼�״̬������ǰ���¼��������¼�����ǰ���¼�������
//				������������������������Ƿ����
//		@pbRelaOAD	�������ع����������Ա�
//����:����¼���Ҫ�ж���������ȷ�򷵻�true,���򷵻�false
bool ReadAndSaveMtrExcRec(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wOI, BYTE bState, BYTE* pbRelaOAD, bool* pfIsSaveRec)
{
	#if 0
	BYTE g_bTestData[] = {0x01, 0x02, 0x51, 0x00, 0x10, 0x02, 0x00, 
									  0x51, 0x00, 0x20, 0x02, 0x00,};
	#endif

	int iLen, iRecLen = 0, nMtrExcIdx = 0;
	WORD i, wItemOffset = 0, wItemLen = 0;
	bool fReadSuccess = true, fOnMtrExcHap, fOnMtrExcEnd;
	BYTE bCapNum, bIndex, bType = 0, bChnNum = 0, bTsaLen = 0;
	int nOADLen, nLastRecPhyIdx;
	DWORD dwOAD, dwEvtOAD, dwRecIndex = 0;
	BYTE bBuf[MTR_EXC_REC_LEN], bTmpBuf[100];
	BYTE bRecBuf[MTR_EXC_REC_LEN];
	BYTE bFixBuf[20], bOADBuf[4];
	TTime tmCurRec;	
	//TFieldParser tFixFields = { g_bMtrExcFixOAList, sizeof(g_bMtrExcFixOAList) };	//�̶��ֶ�
	BYTE* pbBuf = bBuf;
	BYTE* pbRec = bRecBuf;
	const ToaMap* pOaMap = NULL;
	const ToaMap* pEvtOaMap = NULL;
	WORD wRSDLen, wRCSDLen = 0;
	BYTE bRSDBuf[32], bRCSDBuf[256];
	TMtrExcTmp* pExcTmp = &pMtrRdCtrl->mtrExcTmp;

	nMtrExcIdx = GetMtrExcIndex(wOI);
	if (nMtrExcIdx < 0)
		return false;

	fOnMtrExcHap = (bState==EVT_S_AFT_HP && pExcTmp->dwLastStatClick[nMtrExcIdx]==0);		//�¼��շ�����־
	fOnMtrExcEnd = (bState==EVT_S_AFT_END && pExcTmp->dwLastStatClick[nMtrExcIdx]!=0);		//�¼��ս�����־

	//�������Ա������ֶδ�����¼�ÿ��״̬����Ҫ����
	memset(bBuf, 0, sizeof(bBuf));
	bCapNum = (pbRelaOAD[1]>CAP_OAD_NUM) ? CAP_OAD_NUM : pbRelaOAD[1];
	if (bCapNum > 0)
	{
		iLen = ReadMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, MEM_TYPE_MTREXC, wOI, bBuf);
		if (iLen <= 0)
		{
			DTRACE(DB_METER_EXC, ("ReadAndSaveMtrExcRec: wOI = %04x read mem fail!\r\n", wOI));
			return false;
		}

		for (i=0; i<bCapNum; i++)		//�����¼�״̬��������������
		{
			dwOAD = OoOadToDWord(&pbRelaOAD[5*i+3]);
			nOADLen = OoGetDataLen(DT_OAD, &pbRelaOAD[5*i+3]);
			if (nOADLen <= 0)
				return false;

			if (bState!=(BYTE ) ((dwOAD&~OAD_FEAT_MASK)>>OAD_FEAT_BIT_OFFSET))	//����������һ��
			{
				pbBuf += nOADLen;
				continue;
			}
			else if (wOI==OI_MTR_RD_FAIL && (bState==EVT_S_AFT_HP||bState==EVT_S_BF_END))	//����ʧ���¼�����ʱ������ȥ������
			{
				memset(pbBuf, 0, nOADLen);	//ȫ0Ϊ��Ч����?
				pbBuf += nOADLen;
				continue;
			}

			memset(bTmpBuf, 0, sizeof(bTmpBuf));
			dwOAD &= OAD_FEAT_MASK;		//ȥ�����������ٳ���
			if (dwOAD == 0x40000200)	//hyl 3105�������ﳭ��
				continue;
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bTmpBuf);
			if (iLen<=0 || IsAllAByte(bTmpBuf, 0, nOADLen))	//û����
			{
				if (AskMtrItem(pMtrPro, RESPONSE_TYPE_NORAML, dwOAD, bTmpBuf, bRSDBuf, nOADLen, bRCSDBuf, wRCSDLen) > 0)	//��������
				{
					SaveMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwOAD, bTmpBuf, nOADLen);	//��������DAR zhq modify 17-02-17
					memcpy(pbBuf, bTmpBuf+1, nOADLen);	//+1 ����DAR

					SaveMtrDataHook(dwOAD, &pMtrRdCtrl->mtrExcTmp, 0);
				}
				else
				{
					memset(pbBuf, 0, nOADLen);	//ȫ0Ϊ��Ч����
					fReadSuccess = false;
				}
			}
			else
			{
				memcpy(pbBuf, bTmpBuf, nOADLen);
			}

			pbBuf += nOADLen;
		}

		iLen = WriteMem(pMtrRdCtrl->allocTab, MTR_TAB_NUM, pMtrRdCtrl->bMem, MEM_TYPE_MTREXC, wOI, bBuf);		//������ʱ������
	}

	//���ʼ�¼������ȴ���̶��ֶ����ݣ��ٰѴ���õ������ֶ������룬ƴ��һ��������¼
	if (bState==EVT_S_AFT_HP || bState==EVT_S_AFT_END)
	{
		dwEvtOAD = GetOAD(wOI, ATTR2, 0);	//�¼���ӦOAD
		pEvtOaMap = GetOIMap(dwEvtOAD);
		if (pEvtOaMap==NULL || pEvtOaMap->pszTableName==NULL)
		{
			DTRACE(DB_METER_EXC, ("ReadAndSaveMtrExcRec: dwOAD %08x not found, or table name is null!\r\n", dwEvtOAD));
			return false;
		}

		DTRACE(DB_METER_EXC, ("ReadAndSaveMtrExcRec: dwEvt OAD = %08x, bState = %d.\r\n", dwEvtOAD, bState));
		memset(bRecBuf, 0, sizeof(bRecBuf));
		iLen = ReadLastNRec(pEvtOaMap->pszTableName, LAST_REC, bRecBuf, sizeof(bRecBuf));	//������һ�ʼ�¼ȡ�����¼���¼���
		if (iLen > 0)
			dwRecIndex = OoDoubleLongUnsignedToDWord(bRecBuf+1);	//�¼���¼��� ���ֽ���ǰ����

		if (fOnMtrExcHap)	//�շ����¼�
		{
			dwRecIndex++;	//����ʱ����
			memset(bRecBuf, 0, sizeof(bRecBuf));
		}
		else
		{
			nLastRecPhyIdx = GetLastRecPhyIdx(wOI, &pMtrRdCtrl->mtrExcTmp);
			if (nLastRecPhyIdx >= 0)
#ifdef TERM_EVENT_DEBUG
				iLen = ReadRecByPhyIdx(pEvtOaMap->pszTableName, nLastRecPhyIdx-1, bRecBuf, sizeof(bRecBuf));	//�Ѿ��������¼�����ȡ��֮ǰ����ļ�¼���ٸ�����Ӧ����
#else
				iLen = ReadRecByPhyIdx(pEvtOaMap->pszTableName, nLastRecPhyIdx, bRecBuf, sizeof(bRecBuf));	//�Ѿ��������¼�����ȡ��֮ǰ����ļ�¼���ٸ�����Ӧ����
#endif
		}

		pbRec = bRecBuf;

		TMtrExcFixUnitDes* pUnitDesc = GetMtrExcFixUnit(wOI);
		if (pUnitDesc == NULL)
			return false;
			
		if (OoParseField(pUnitDesc->pFixField, pUnitDesc->pbFmt, pUnitDesc->wFmtLen, false) == false)		// �̶��ֶ�
			return false;

		for (bIndex=0; bIndex<pUnitDesc->pFixField->wNum; bIndex++)	//�̶��ֶθ���----��ʼ�̶��ֶδ��
		{
			memset(bFixBuf, 0, sizeof(bFixBuf));
			if (ReadParserField(pUnitDesc->pFixField, bIndex, bFixBuf, &bType, &wItemOffset, &wItemLen) > 0)		//�̶��ֶ�OAD
			{
				dwOAD = OoOadToDWord(bFixBuf+1);	//�̶��ֶ�OAD
				pOaMap = GetOIMap(dwOAD);
				if (pOaMap == NULL)
				{
					DTRACE(DB_METER_EXC, ("ReadAndSaveMtrExcRec: Read dwOAD:%x failed !!\r\n", dwOAD));
					return false;
				}

				nOADLen = OoGetDataLen(DT_OAD, bFixBuf+1);
				nOADLen--;		//ȥ��1�ֽ���������
				if (nOADLen <= 0)
				{
					DTRACE(DB_METER_EXC, ("ReadAndSaveMtrExcRec: nOADLen = %d!!\r\n", nOADLen));
					return false;
				}
				
				if (pOaMap->dwOA == 0x20220200)		//�¼���¼��� ���ֽ���ǰ����
				{
					*pbRec++ = *pOaMap->pFmt;	//��������

					OoDWordToDoubleLongUnsigned(dwRecIndex, pbRec);
					pbRec += nOADLen;
				}
				else if (pOaMap->dwOA == 0x20240200)	//�¼�����Դ -- ���ͨ�ŵ�ַ(oct-string)
				{
#ifdef MTREXC_ADDR_TPYE_TSA
					*pbRec++ = DT_TSA;			//��������
#else
					*pbRec++ = DT_OCT_STR;		//��������
#endif

					memset(pbRec, 0, nOADLen);

#ifdef MTREXC_ADDR_TPYE_TSA
					bTsaLen = pMtrRdCtrl->bTsa[0] & 0x0f;
					*pbRec++ = (bTsaLen+1);			//OCT_STR����
					if (bTsaLen > 0)
						*pbRec++ = (bTsaLen-1);		//TSA��Ч����
					else
						*pbRec++ = 0;

					if (bTsaLen <= (sizeof(pMtrRdCtrl->bTsa)-1))
						memcpy(pbRec, pMtrRdCtrl->bTsa+1, bTsaLen);	//�������ʵ�ʵ�ַ����
					else
						return false;
#else
					*pbRec++ = pMtrRdCtrl->bTsa[0];	//��Ч����
					if (pMtrRdCtrl->bTsa[0] < sizeof(pMtrRdCtrl->bTsa))
						memcpy(pbRec, pMtrRdCtrl->bTsa+1, pMtrRdCtrl->bTsa[0]);
					else
						return false;
#endif
					
#ifdef TERM_EVENT_DEBUG
					pbRec += (nOADLen-2); //��������
#else
					pbRec += nOADLen;
#endif
				}
				else if (pOaMap->dwOA == OAD_EVT_RPT_STATE)	//�¼��ϱ�״̬  array ͨ���ϱ�״̬ 
				{										//DT_ARRAY 02 DT_STRUCT 02 DT_OAD 45 00 02 00 DT_UNSIGN 00	DT_STRUCT 02 DT_OAD 45 10 02 00 DT_UNSIGN 00
					iLen = OoReadAttr(0x4300, ATTR10, bTmpBuf, NULL, NULL);	//��ȡ���ò���
					if (iLen<=0 || bTmpBuf[0]!=DT_ARRAY)
					{
						*pbRec++ = DT_NULL;		//��������
						pbRec += CN_RPT_TOTAL_LEN-1;	//����
					}
					else
					{
						*pbRec++ = DT_ARRAY;	//��������

						bChnNum = (bTmpBuf[1]>CN_RPT_NUM) ? CN_RPT_NUM : bTmpBuf[1];
						*pbRec++ = bChnNum;	//OAD����

						for (i=0; i<bChnNum; i++)
						{
							*pbRec++ = DT_STRUCT;
							*pbRec++ = 0x02;
							memcpy(pbRec, &bTmpBuf[5*i+2], 5); //ͨ��OAD
							pbRec += 5;

							*pbRec++ = DT_UNSIGN;
							if (fOnMtrExcHap)
								*pbRec = 0x00;
							else
								*pbRec &= 0x03;	//��������ǰ���ϱ���־

							pbRec++;
						}
						pbRec += CN_RPT_STATE_LEN*(CN_RPT_NUM - bChnNum);	//����
					}

					
				}
				else if (pOaMap->dwOA == 0x40000200)	//�����ն�ʱ��
				{
					if  (bIndex == pUnitDesc->pFixField->wNum-1)	 //�ն�ʱ��
					{
						*pbRec++ = *pOaMap->pFmt;	//��������

						GetCurTime(&tmCurRec);
						OoTimeToDateTimeS(&tmCurRec, pbRec);												
					}
					else	//���ʱ��
					{
						if (GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, pOaMap->dwOA, pbRec) <= 0)	//û����
						{
							*pbRec++ = DT_NULL;	//��������
							memset(pbRec, 0, 7);
						}
						else
						{
							pbRec++;	//��������
						}
					}

					pbRec += 7;
				}
				else if (pOaMap->dwOA == 0x330f0206)		//�¼���¼��� ���ֽ���ǰ����
				{
					memcpy(pbRec, pExcTmp->mtrDataChg.bCSD, MTEDATACHG_CSD_LEN);
					pbRec += MTEDATACHG_CSD_LEN;
				}
				else if (pOaMap->dwOA == 0x330f0207)		//�¼���¼��� ���ֽ���ǰ����
				{
					memcpy(pbRec, pExcTmp->mtrDataChg.bOldData, MTEDATACHG_DATA_LEN);
					pbRec += MTEDATACHG_DATA_LEN;
				}
				else if (pOaMap->dwOA == 0x330f0208)		//�¼���¼��� ���ֽ���ǰ����
				{
					memcpy(pbRec, pExcTmp->mtrDataChg.bNewData, MTEDATACHG_DATA_LEN);
					pbRec += MTEDATACHG_DATA_LEN;
				}
				else
				{
					if (pOaMap->dwOA != 0x20200200)	//�¼�����ʱ��
						*pbRec++ = *pOaMap->pFmt;	//��������

					GetCurTime(&tmCurRec);
					if (fOnMtrExcHap)	//�¼��շ���
					{
						if (pOaMap->dwOA == 0x201E0200)	//�¼�����ʱ��
						{
							OoTimeToDateTimeS(&tmCurRec, pbRec);
						}
						else if (pOaMap->dwOA == 0x20200200)	//����ʱ���¼�����ʱ����0
						{
							*pbRec++ = DT_NULL;	//��������
							memset(pbRec, 0, nOADLen);
						}
					}
					else if (fOnMtrExcEnd)	//�¼��ս���
					{
						if (pOaMap->dwOA == 0x20200200)	//�¼�����ʱ��
						{
							*pbRec++ = *pOaMap->pFmt;	//��������
							OoTimeToDateTimeS(&tmCurRec, pbRec);
						}
					}
					else if (bState==EVT_S_AFT_HP && pOaMap->dwOA==0x20200200)	//�����ڼ䣬����ʱ��Ϊȫ0
					{
						*pbRec++ = DT_NULL;	//��������
						memset(pbRec, 0, nOADLen);
					}
					else
					{
						pbRec++;	//��������
					}

					pbRec += nOADLen;
				}
			}
		}		//---�̶��ֶ����ݴ������

		iRecLen = (pbRec-bRecBuf);	//+�̶��ֶγ���
		iRecLen += (pbBuf-bBuf);	//+�����ֶγ���
		if (iRecLen <= sizeof(bRecBuf))
		{
			iLen = pbBuf - bBuf;	//�����ֶγ���
			if (iLen > 0)
				memcpy(pbRec, bBuf, iLen);	//���������Ա������ֶ����ݿ���,ƴ��һ�ʼ�¼

			if (fOnMtrExcHap)	//������һ���¼���¼
			{
				if (SaveRecord(pEvtOaMap->pszTableName, bRecBuf, &nLastRecPhyIdx))
					UpdateLastRecPhyIdx(wOI, &pMtrRdCtrl->mtrExcTmp, nLastRecPhyIdx);	//��¼�·�����¼�Ĵ洢λ��

				*pfIsSaveRec = true;
			}
			else
			{
				SaveRecordByPhyIdx(pEvtOaMap->pszTableName, nLastRecPhyIdx, bRecBuf);	//�Ȼ�ȡ������¼�Ĵ洢λ�ã��ٸ��¼�¼
				*pfIsSaveRec = true;
			}
		}
	}
	
	UpdateMtrExcStatData(wOI, bState, pExcTmp, pMtrRdCtrl->bTsa);		//��¼����󣬸����¼�ͳ������

	return fReadSuccess;
}

//�����¼��ϱ�
void DoMtrExcRpt(WORD wOI, BYTE bSendRptFlag)
{
	BYTE bRptFlag = 0, bChnNum = 0;
	int i, iLen, nRecIdx;
	bool fRpt = false;
	DWORD dwOAD;
	BYTE bTmpBuf[60];
	DWORD dwChnOAD;
	const ToaMap* pOaMap = NULL;

	memset(bTmpBuf, 0, sizeof(bTmpBuf));
	if (OoReadAttr(wOI,  ATTR8, bTmpBuf, NULL, NULL) > 0)	//�ϱ���ʶ�����ϱ���0�����¼������ϱ���1�����¼��ָ��ϱ���2�����¼������ָ����ϱ���3��
		bRptFlag = bTmpBuf[1];
	
	if ((bSendRptFlag==EVT_STAGE_HP && (bRptFlag&0x01)==0x01) || (bSendRptFlag==EVT_STAGE_END && (bRptFlag&0x02)==0x02))		//��Ҫ�ϱ�
	{
		dwOAD = GetOAD(wOI, ATTR2, 0);
		pOaMap = GetOIMap(dwOAD);
		if (pOaMap==NULL || pOaMap->pszTableName==NULL)
			return;

		nRecIdx = GetRecPhyIdx(pOaMap->pszTableName, 1);
		if (nRecIdx >= 0)
		{
			memset(bTmpBuf, 0, sizeof(bTmpBuf));
			iLen = OoReadAttr(0x4300, ATTR10, bTmpBuf, NULL, NULL);	//��ȡ���ò���
			if (iLen<=0 || bTmpBuf[0]!=DT_ARRAY)
				return;
			
			bChnNum = (bTmpBuf[1]>CN_RPT_NUM) ? CN_RPT_NUM : bTmpBuf[1];
			for (i=0; i<bChnNum; i++)
			{
				dwChnOAD = OoOadToDWord(&bTmpBuf[5*i+3]);	//ͨ��OAD
				SendEvtMsg(dwChnOAD, dwOAD, nRecIdx, bSendRptFlag);
			}
		}
	}
}


//�����������¼�״̬�������������ݱ����¼
//����:@pbState���������¼�״̬��״̬
//     @bCurErcFlag:��ǰ�¼�״̬ 0���м�̬��1��������2���ָ�
void UpdateMtrExcStateAndSaveRec(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wOI, BYTE* pbRelaOAD, BYTE bCurErcFlag, BYTE* pbState)
{
	bool fRet = false;	//����������ĳ�����
	BYTE bSendRptFlag = EVT_STAGE_UNCARE;
	int nMtrExcIdx = -1;
	TMtrExcTmp* pExcTmp = &pMtrRdCtrl->mtrExcTmp;
	bool fIsSaveRec = false;
	DWORD dwROAD;

	nMtrExcIdx = GetMtrExcIndex(wOI);
	if (nMtrExcIdx < 0)
		return;
	
	switch (*pbState)	//�¼�״̬��
	{
	case EVT_S_BF_HP:
		if (bCurErcFlag == ERC_STATE_HAPPEN)	//����
			*pbState = EVT_S_AFT_HP;

		fRet = ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, *pbState, pbRelaOAD, &fIsSaveRec);	//���س������
		if (*pbState==EVT_S_AFT_HP && fRet)
		{
			*pbState = EVT_S_BF_END;
			bSendRptFlag = EVT_STAGE_HP;
		}
		else if (!fRet)
		{
			pExcTmp->bTryReadCnt[nMtrExcIdx]++;
		}

		break;

	case EVT_S_AFT_HP:
		fRet = ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, *pbState, pbRelaOAD, &fIsSaveRec);
		if (fRet || pExcTmp->bTryReadCnt[nMtrExcIdx]>TRY_READ_NUM)	//����ɻ�ﵽ���Գ������ʱ�л�����һ״̬
		{
			*pbState = EVT_S_BF_END;
			pExcTmp->bTryReadCnt[nMtrExcIdx] = 0;
			bSendRptFlag = EVT_STAGE_HP;
		}
		else
		{
			pExcTmp->bTryReadCnt[nMtrExcIdx]++;
		}

		break;

	case EVT_S_BF_END:
		if (bCurErcFlag == ERC_STATE_RECOVER)	 //�ָ�
			*pbState = EVT_S_AFT_END;

		fRet = ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, *pbState, pbRelaOAD, &fIsSaveRec);
		if (*pbState==EVT_S_AFT_END && fRet)
		{
			*pbState = EVT_S_BF_HP;
			bSendRptFlag = EVT_STAGE_END;
		}
		else if (!fRet)
		{
			pExcTmp->bTryReadCnt[nMtrExcIdx]++;
		}

		break;

	case EVT_S_AFT_END:
		fRet = ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, *pbState, pbRelaOAD, &fIsSaveRec);
		if (fRet || pExcTmp->bTryReadCnt[nMtrExcIdx]>TRY_READ_NUM)
		{
			*pbState = EVT_S_BF_HP;
			pExcTmp->bTryReadCnt[nMtrExcIdx] = 0;
			bSendRptFlag = EVT_STAGE_END;
		}
		else
		{
			pExcTmp->bTryReadCnt[nMtrExcIdx]++;
		}

		break;
	}

	if (fIsSaveRec)
	{	
		dwROAD = GetOAD(wOI, 0x02, 0);
		AddEvtOad(dwROAD, bSendRptFlag);
	}

	DoMtrExcRpt(wOI, bSendRptFlag);	//�ϱ��¼�
}


////��λ�����־
void SetMtrExcTableFlag(WORD wOI)
{
	WORD wPn;
	BYTE bFlag = 1;

	wPn = GetMtrExcIndex(wOI);
	WriteItemEx(BN2, wPn, 0x2300, &bFlag);
}


//��������־
void ClrMtrExcTableFlag(WORD wOI)
{
	WORD wPn;
	BYTE bFlag = 0;

	wPn = GetMtrExcIndex(wOI);
	WriteItemEx(BN2, wPn, 0x2300, &bFlag);
}

//��ѯ��־
bool IsMtrExcTableCreate(WORD wOI)
{
	WORD wPn;
	int iLen;
	BYTE bFlag = 0;

	wPn = GetMtrExcIndex(wOI);
	iLen = ReadItemEx(BN2, wPn, 0x2300, &bFlag);
	if (iLen > 0)
		return (bFlag==1);
	else
		return false;
}


//========================���ܱ�ʱ�䳬��==========================
//���������ܱ�ʱ�䳬���¼������ĳ�ʼ����
void InitMtrClockErr(WORD wPn, TMtrClockErr* pCtrl)
{
	pCtrl->bState = EVT_S_BF_HP;
}


//���������ܱ�ʱ�䳬���¼����жϣ�����ն˺͵��ʱ�䳬�����Уʱ��ֵ��Ĭ��Ϊ5���ӣ�������Ϊ���ʱ�䳬�
bool DoMtrClockErr(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{	
	bool fRet;
	DWORD dwMtrSecs, dwCurSecs, dwDiff;
	int iLen;	
	TTime time;
	TTime tmMtrTime;
	BYTE bCurState, bLen;
	WORD wChecktmHold, wRecoverHold, wJudgeOadNum = 0;
	BYTE bAlrBuf = 0, bCurErcFlag = 0;
	BYTE bBuf[40];
	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE* pbBuf = bBuf;
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	const WORD wOI = OI_MTR_CLOCK_ERR;

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrClockErr* pCtrl = &pMtrTmp->mtrClockErr;

	memset(&time, 0, sizeof(time));
	memset(&tmMtrTime, 0, sizeof(tmMtrTime));
	if (!GetMtrExcRdItem(wOI, &dwJudgeOAD[0], &wJudgeOadNum, bRelaOAD))
		return false;

	//if (IsRxTrigerSaveCmd(wPn, wOI, pCtrl->bState, &bCurState))
	//	return ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, bCurState, bRelaOAD);

	if (QueryMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD, wJudgeOadNum))		//���ݵ���
	{
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD wIndex=0; wIndex<wJudgeOadNum; wIndex++)
		{
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD[wIndex], pbBuf);
			if (iLen <= 0)	//û����
				return false;

			pbBuf += iLen;
		}

		OoDateTimeSToTime(bBuf+1, &tmMtrTime);
#ifdef TERM_EVENT_DEBUG
		tmMtrTime.nSecond = 0; //Ŀǰ�����ص�����ʱ��������ΪFF����ش������ݣ���ʱ���㴦�� --QLS
#endif

		if (IsInvalidTime(tmMtrTime))
			return false;

		memset(bBuf, 0, sizeof(bBuf));
		iLen = OoReadAttr(wOI, ATTR6, bBuf, NULL, NULL);	// ��ȡ���ò���
		if (iLen <= 0 || bBuf[2] != DT_LONG_U)
			return false;

		wChecktmHold = OoLongUnsignedToWord(&bBuf[3]);	//�쳣�б���ֵ  long-unsigned����λ���룩
		if (wChecktmHold == 0)
			wChecktmHold = 300;

		wRecoverHold = wChecktmHold;
		dwMtrSecs = TimeToSeconds(tmMtrTime);		//���ǰʱ�任�����
		dwCurSecs = pMtrTmp->dwItemRdTime[CLOCK_ITEM_INDEX];	//�ն˳ɹ��������ʱ�ӵ�ʱ��
		SecondsToTime(dwCurSecs, &time);
		if (dwCurSecs==0 || IsInvalidTime(time))
		{
			DTRACE(DB_METER_EXC, ("CMtrTimeErr::RunTask: fail to get term time, when c011 read, pn=%d, dwCurSecs=%ld.\r\n", wPn, dwCurSecs));
			DTRACE(DB_METER_EXC, ("CMtrTimeErr::RunTask: curTime = %02d-%02d-%02d %02d:%02d:%02d, pn=%d\r\n", time.nYear, time.nMonth, time.nDay,
                                  time.nHour, time.nMinute, time.nSecond, wPn));
			return false;
		}

		dwDiff = GetAbsGap(dwCurSecs, dwMtrSecs);	//�����ն�ʱ������ֵ
		
		if (dwDiff >= (DWORD)wChecktmHold)	//����״̬
		{
			bCurErcFlag = ERC_STATE_HAPPEN;
			//DTRACE(DB_METER_EXC, ("CMtrTimeErr::###### CMtrTimeErr event happened!!! ######\r\n"));
			//DTRACE(DB_METER_EXC, ("CMtrTimeErr::RunTask: dwDiff=%ds  pCtrl->bState=%d\r\n", dwDiff, pCtrl->bState));
		}
		else if (dwDiff < (DWORD)wRecoverHold)	//����״̬
			bCurErcFlag = ERC_STATE_RECOVER;
		else
			bCurErcFlag = ERC_STATE_MIDDLE;

		UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bCurErcFlag, &pCtrl->bState);

		return true;
	}

	return false;
}




void InitEnergyDec(BYTE wPn, TMtrEnergyDec* pCtrl)
{
	BYTE i;

	pCtrl->ui64PosE = 0;		//�����������й�
	pCtrl->ui64NegE = 0;	//�����������й�
	pCtrl->bState = EVT_S_BF_HP;

	memset(pCtrl->bAddr, 0, sizeof(pCtrl->bAddr));
	for (i=0; i<2; i++)
		pCtrl->fInvalid[i] = true;
}


//���ܱ�ʾ���½�
bool DoMtrEnergyDec(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{
  	BYTE i = 0;
	WORD wValidNum = 0;
	BYTE bBuf[100];
	BYTE bAlrBuf[20];
	BYTE bAddrBuf[MTR_ADDR_LEN];
	TTime time;	
	BYTE bMtrInterv;
	WORD wIdNum;	
	bool fInvalid[2];//���򡢷����й�����ֵ�Ƿ���Ч 
	int iLen = 0;
	BYTE bCurState, bTotalErcFlag, bLen;
	BYTE bCurErcFlag[2];
	WORD wJudgeOadNum = 0;

	uint64 ui64PosE, ui64NegE, ui64Energy = 0, ui64PreEnergy = 0;

	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE* pbBuf = bBuf;
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	const WORD wOI = OI_MTR_ENERGY_DEC;

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrEnergyDec* pCtrl = &pMtrTmp->mtrEnergyDec;

	memset(bBuf, 0, sizeof(bBuf));
	memset(bAlrBuf, 0, sizeof(bAlrBuf));
	memset(bAddrBuf, 0, sizeof(bAddrBuf));
	memset(fInvalid, 0, sizeof(fInvalid));
	memset(&time, 0, sizeof(time));

	if (!GetMtrExcRdItem(wOI, &dwJudgeOAD[0], &wJudgeOadNum, bRelaOAD))
		return false;

	//if (IsRxTrigerSaveCmd(wPn, wOI, pCtrl->bState, &bCurState))
	//	return ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, bCurState, bRelaOAD);

	if (pMtrRdCtrl->bTsa[0] >= sizeof(pMtrRdCtrl->bTsa))
		return false;

	//��ȡ���ֵַ���ж��Ƿ���Ч��������ַΪ0��˵��δ���ñ��ַ�����أ�
	if(IsAllAByte(&pMtrRdCtrl->bTsa[1], 0, pMtrRdCtrl->bTsa[0]))	//��ַȫ0
	{
		pCtrl->fInvalid[0] = pCtrl->fInvalid[1] = true;
		return false;
	}
	else
	{		
		memcpy(bAddrBuf, &pMtrRdCtrl->bTsa[0], sizeof(bAddrBuf));
	}	

	if (QueryMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD, wJudgeOadNum))		//���ݵ���
	{
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD wIndex=0; wIndex<wJudgeOadNum; wIndex++)
		{
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD[wIndex], pbBuf);
			if (iLen <= 0)	//û����
				return false;
			
			pbBuf += iLen;
		}		

		fInvalid[0] = fInvalid[1] = false;
		
#ifdef TERM_EVENT_DEBUG
		ui64PosE = GetEnergyValByFmt(bBuf[0], bBuf, &bLen);	//�����й���
		ui64NegE = GetEnergyValByFmt(bBuf[bLen], bBuf+bLen, &bLen);	//�����й���
#else
		ui64PosE = GetEnergyValByFmt(bBuf[0], bBuf+1, &bLen);	//�����й���
		ui64NegE = GetEnergyValByFmt(bBuf[bLen], bBuf+bLen+1, &bLen);	//�����й���
#endif

		if (memcmp(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1) != 0)	//���ַ�ı�
		{
			memcpy(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1);
		}
		else
		{
			memset(bCurErcFlag, ERC_STATE_MIDDLE, sizeof(bCurErcFlag));
			for (i=0; i<2; i++)
			{
				if (pCtrl->fInvalid[i] || fInvalid[i])	//������ֵ�����ڵ���ֵ��Ч���������жϣ�
					continue;

				if (i == 0)
				{
					ui64Energy = ui64PosE;	//�ȱȽ������й�����ֵ��
					ui64PreEnergy = pCtrl->ui64PosE;
				}
				else
				{
					ui64Energy = ui64NegE;
					ui64PreEnergy = pCtrl->ui64NegE;
				}
								
				if (ui64Energy<ui64PreEnergy && (ui64PreEnergy-ui64Energy)<9999990000LL)	//�й�����ֵ�½��ҷ�����־��0������ֹ����ߵ����̶��������֣�
				{
					bCurErcFlag[i] = ERC_STATE_HAPPEN;	//����
					//DTRACE(DB_METER_EXC, ("EnergyDec::###### EnergyDec event happened!!! ######\r\n"));
				}
				else if (ui64Energy >= ui64PreEnergy)	//�й�����ֵû���½��ҷ�����־��1��
					bCurErcFlag[i] = ERC_STATE_RECOVER;	//�ָ�
				else
					bCurErcFlag[i] = ERC_STATE_MIDDLE;
			}

			if (bCurErcFlag[0]==ERC_STATE_HAPPEN || bCurErcFlag[1]==ERC_STATE_HAPPEN)	//���� �� ���з���
				bTotalErcFlag = ERC_STATE_HAPPEN;
			else if (bCurErcFlag[0]==ERC_STATE_RECOVER && bCurErcFlag[1]==ERC_STATE_RECOVER)	//���кͷ��ж��ָ�
				bTotalErcFlag = ERC_STATE_RECOVER;
			else
				bTotalErcFlag = ERC_STATE_MIDDLE;

			UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bTotalErcFlag, &pCtrl->bState);
		}

		//�������ڵ�ֵ������Ϊ��һ���ڱȽ����ݣ�
		if (!fInvalid[0])
			pCtrl->ui64PosE = ui64PosE;

		if (!fInvalid[1])
			pCtrl->ui64NegE = ui64NegE;

		pCtrl->fInvalid[0] = fInvalid[0];
		pCtrl->fInvalid[1] = fInvalid[1];
	}

	return true;
}


//����������
void InitEnergyErr(BYTE wPn, TMtrEnergyErr* pCtrl)
{
	BYTE i;
	pCtrl->ui64PosE = 0;	//�����������й�
	pCtrl->ui64NegE = 0;	//�����ڷ����й�
	pCtrl->bState = EVT_S_BF_HP;

	memset(pCtrl->bAddr, 0, sizeof(pCtrl->bAddr));
	for(i=0; i<2; i++)
	{
		pCtrl->fInvalid[i] = true;
		pCtrl->dwSeconds[i] = 0;
	}
}


bool DoMtrEnergyErr(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{
	DWORD dwFlewHold = 0;// ���ֵ��
	BYTE i = 0;
	BYTE bBuf[100], bAddr[6];
	
	DWORD dwCurMin;
	WORD wIdNum, wValidNum;
	BYTE bMtrInterv = 2;	//GetMeterInterv();		//����������GetMeterInterv()���ã�������
	int iLen = 0;
	TTime time;
	
	bool fInit[2];

	WORD wUn = 0 ;//���ѹֵ
	WORD wIn = 0;//������ֵ(1���ֽڣ�1��С��λ��
	BYTE bConnType = 0, bCurState, bLen;

	uint64 ui64DeltEnergy = 0;//�����ʾֵ�߹��ĵ���(4λС��) 
	uint64 ui64PastEnergy = 0;//�����������߹��ĵ���

	bool fInvalid[2];//���������й�ֵ�Ƿ���Ч
	uint64 ui64PosE = 0;//��ǰ�����й�
	uint64 ui64NegE = 0;//��ǰ�����й�

	TOobMtrInfo tMtrInfo;
	BYTE btemp = 0, bTotalErcFlag;
	WORD wE0 = 0, wJudgeOadNum = 0;
	DWORD dwTimePast = 0;
	BYTE bCurErcFlag[2];
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	BYTE bAddrBuf[MTR_ADDR_LEN];
	const WORD wOI = OI_MTR_ENERGY_ERR;

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrEnergyErr* pCtrl = &pMtrTmp->mtrEnergyErr;
	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE* pbBuf = bBuf;

	//memset(bAlrBuf, 0, sizeof(bAlrBuf));
	memset(bAddr, 0, sizeof(bAddr));
	memset(&time, 0, sizeof(time));
	memset(fInit, 0, sizeof(fInit));

	if (!GetMtrExcRdItem(wOI, &dwJudgeOAD[0], &wJudgeOadNum, bRelaOAD))
		return false;

	//if (IsRxTrigerSaveCmd(wPn, wOI, pCtrl->bState, &bCurState))
	//	return ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, bCurState, bRelaOAD);

	if (pMtrRdCtrl->bTsa[0] >= sizeof(pMtrRdCtrl->bTsa))
		return false;

	//��ȡ���ֵַ���ж��Ƿ���Ч��������ַΪ0��˵��δ���ñ��ַ�����أ�
	if(IsAllAByte(&pMtrRdCtrl->bTsa[1], 0, pMtrRdCtrl->bTsa[0]))	//��ַȫ0
	{
		pCtrl->fInvalid[0] = pCtrl->fInvalid[1] = true;
		return false;
	}
	else
	{		
		memcpy(bAddrBuf, &pMtrRdCtrl->bTsa[0], sizeof(bAddrBuf));
	}	

	if (QueryMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD, wJudgeOadNum))		//���ݵ���
	{
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD wIndex=0; wIndex<wJudgeOadNum; wIndex++)
		{
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD[wIndex], pbBuf);
			if (iLen <= 0)	//û����
				return false;
			
			if (pbBuf[2] == DT_DB_LONG_U)
				iLen = 5;	//ȡ���ܵ���
			else
				iLen = 9;	//ȡ���ܵ���

			memmove(pbBuf, pbBuf+2, iLen);
			pbBuf += iLen;
		}		

		fInvalid[0] = fInvalid[1] = false;

#ifdef TERM_EVENT_DEBUG
		ui64PosE = GetEnergyValByFmt(bBuf[0], bBuf, &bLen);	//�����й���
		ui64NegE = GetEnergyValByFmt(bBuf[bLen], bBuf+bLen, &bLen);	//�����й���
#else
		ui64PosE = GetEnergyValByFmt(bBuf[0], bBuf+1, &bLen);	//�����й���
		ui64NegE = GetEnergyValByFmt(bBuf[bLen], bBuf+bLen+1, &bLen);	//�����й���
#endif

		if (memcmp(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1) != 0)		//����һ�γ������ַ���������¿�ʼ�ж�
		{
			memcpy(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1);
			fInit[0] = true;
			fInit[1] = true;
		}
		else
		{
			if (!GetMeterInfo(wPn, &tMtrInfo))	//��ȡ�����ѹ������������߷�ʽ�����ò���F25����
				return false;
	
			wUn = tMtrInfo.wRateVol; //���ѹֵ
			wIn = tMtrInfo.wRateCurr; //�����ֵ
#ifdef TERM_EVENT_DEBUG
			bConnType = tMtrInfo.bLine;
#else
			bConnType = GetConnectType(wPn);
#endif
			if (wUn == 0)
				wUn = 2200;

			if (wIn == 0)
				wIn = 50;
#ifdef TERM_EVENT_DEBUG
			if (bConnType == 2)
#else
			if (bConnType == CONNECT_3P3W)//��������
#endif
			{
				wE0 = 500;//0.05*10000
				btemp = 2;//��������b���޵������������๦�ʼ��ɣ�
			}
			else
			{
				wE0 = 1000;//0.1*10000
				btemp = 3;
			}

			memset(bBuf, 0, sizeof(bBuf));
			iLen = OoReadAttr(wOI, ATTR6, bBuf, NULL, NULL); // ��ȡ���ò���
			if (iLen<=0 || bBuf[2]!=DT_DB_LONG_U)
				dwFlewHold = 500;	//5.0����
			else
				dwFlewHold = OoDoubleLongUnsignedToDWord(&bBuf[3]);	//�쳣�б���ֵ double-long-unsigned����λ��%���޻��㣩
#ifdef TERM_EVENT_DEBUG
			if (dwFlewHold == 0)   
				dwFlewHold = 500;
#endif

			memset(bCurErcFlag, ERC_STATE_MIDDLE, sizeof(bCurErcFlag));

			for (i=0; i<2; i++)		//�жϣ�
			{
				//�����ڻ��������й�����ֵ��Ч�����жϣ�
				if (fInvalid[i] || pCtrl->fInvalid[i])
				{
					fInit[i] = true;
					continue;
				}

				if (i==0 && ui64PosE>pCtrl->ui64PosE)	//���������ںͱ����ڵ��ܲ�ֵ�����ʾ���½���ͣ�ߣ����жϣ�
				{
					ui64DeltEnergy = ui64PosE - pCtrl->ui64PosE;
				}
				else if (i==1 && ui64NegE>pCtrl->ui64NegE)
				{
					ui64DeltEnergy = ui64NegE - pCtrl->ui64NegE;
				}
				else
				{
					fInit[i] = true;
					continue;
				}

				//DTRACE(DB_METER_EXC, ("EnergyErr::******RunTask:pn=%d, TimeDec=%d\r\n", wPn, GetClick()-pCtrl->dwSeconds[i]));
				//����������߹��ĵ���
				if((GetClick()-pCtrl->dwSeconds[i]) < ((DWORD)bMtrInterv*60/2))
					continue;
				dwTimePast = (GetClick()-pCtrl->dwSeconds[i]) / ((DWORD) bMtrInterv*60) * ((DWORD)bMtrInterv*60);
				if (dwTimePast == 0)	//����һ����������ʱ��
					dwTimePast = GetClick() - pCtrl->dwSeconds[i];

				ui64PastEnergy = ((uint64) wUn) * wIn * btemp * dwTimePast;	//w����λС����

				//������ʾ���߹��ĵ�������wE0,���¿�ʼ�жϣ�
					//��������λ��w��
				ui64PastEnergy *= dwFlewHold;			//�����������߹��ĵ���
				ui64DeltEnergy *= (3600 * 1000 * 100);	//���ʾ���߹��ĵ���

				if (ui64DeltEnergy >= ui64PastEnergy)	//�й�����ֵ�½��ҷ�����־��0������ֹ����ߵ����̶��������֣�
				{
					bCurErcFlag[i] = ERC_STATE_HAPPEN;	//����
					DTRACE(DB_METER_EXC, ("EnergyErr::###### EnergyErr event happened! ui64DeltEnergy=%u,  ui64PastEnergy=%u.######\r\n", ui64DeltEnergy, ui64PastEnergy));					
				}
				else if (ui64DeltEnergy < ui64PastEnergy)	//�й�����ֵû���½��ҷ�����־��1��
				{
					bCurErcFlag[i] = ERC_STATE_RECOVER;	//�ָ�
					DTRACE(DB_METER_EXC, ("EnergyErr::###### EnergyErr event recover! ui64DeltEnergy=%u,  ui64PastEnergy=%u.######\r\n", ui64DeltEnergy, ui64PastEnergy));					
				}
				else
				{
					bCurErcFlag[i] = ERC_STATE_MIDDLE;
				}

				fInit[i] = true;
			}

			if (bCurErcFlag[0]==ERC_STATE_HAPPEN || bCurErcFlag[1]==ERC_STATE_HAPPEN)	//���� �� ���з���
				bTotalErcFlag = ERC_STATE_HAPPEN;
			else if (bCurErcFlag[0]==ERC_STATE_RECOVER && bCurErcFlag[1]==ERC_STATE_RECOVER)	//���кͷ��ж��ָ�
				bTotalErcFlag = ERC_STATE_RECOVER;
			else
				bTotalErcFlag = ERC_STATE_MIDDLE;

			UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bTotalErcFlag, &pCtrl->bState);
		}

		//��ʼ��
		for (i=0; i<2; i++)
		{
			pCtrl->fInvalid[i] = fInvalid[i];
			if (!fInvalid[i] && fInit[i])
			{
				pCtrl->dwSeconds[i] = GetClick();
				if (i == 0)
					pCtrl->ui64PosE = ui64PosE;
				else
					pCtrl->ui64NegE = ui64NegE;
			}
		}
	}

	return true;
}


void InitMtrFlew(BYTE wPn, TMtrFlew* pCtrl)
{
	BYTE i;
	pCtrl->ui64PosE = 0;	//�����������й�
	pCtrl->ui64NegE = 0;	//�����ڷ����й�
	pCtrl->bState = EVT_S_BF_HP;

	memset(pCtrl->bAddr, 0, sizeof(pCtrl->bAddr));
	for(i=0; i<2; i++)
	{
		pCtrl->fInvalid[i] = true;
		pCtrl->dwSeconds[i] = 0;
	}
}


bool DoMtrFlew(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{
	DWORD dwFlewHold = 0;// ���ֵ��
	BYTE i = 0;
	BYTE bBuf[100], bAddr[6];
	
	DWORD dwCurMin;
	WORD wIdNum, wValidNum;
	BYTE bMtrInterv = 2;	//GetMeterInterv();
	int iLen = 0;
	TTime time;
		
	bool fInit[2];

	WORD wUn = 0 ;//���ѹֵ
	WORD wIn = 0;//������ֵ(1���ֽڣ�1��С��λ��
	BYTE bConnType = 0, bCurState, bLen;

	uint64 ui64DeltEnergy = 0;//�����ʾֵ�߹��ĵ���(4λС��) 
	uint64 ui64PastEnergy = 0;//�����������߹��ĵ���

	bool fInvalid[2];//���������й�ֵ�Ƿ���Ч
	uint64 ui64PosE = 0;//��ǰ�����й�
	uint64 ui64NegE = 0;//��ǰ�����й�

	TOobMtrInfo tMtrInfo;
	BYTE btemp = 0, bTotalErcFlag;
	WORD wE0 = 0, wJudgeOadNum = 0;
	DWORD dwTimePast = 0;
	BYTE bCurErcFlag[2];
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	BYTE bAddrBuf[MTR_ADDR_LEN];
#ifdef TERM_EVENT_DEBUG
	const WORD wOI = OI_MTR_FLEW;
#else
	const WORD wOI = OI_MTR_ENERGY_ERR;
#endif

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrFlew* pCtrl = &pMtrTmp->mtrFlew;
	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE* pbBuf = bBuf;

	//memset(bAlrBuf, 0, sizeof(bAlrBuf));
	memset(bAddr, 0, sizeof(bAddr));
	memset(&time, 0, sizeof(time));
	memset(fInit, 0, sizeof(fInit));

	if (!GetMtrExcRdItem(wOI, &dwJudgeOAD[0], &wJudgeOadNum, bRelaOAD))
		return false;

	//if (IsRxTrigerSaveCmd(wPn, wOI, pCtrl->bState, &bCurState))
	//	return ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, bCurState, bRelaOAD);

	if (pMtrRdCtrl->bTsa[0] >= sizeof(pMtrRdCtrl->bTsa))
		return false;

	//��ȡ���ֵַ���ж��Ƿ���Ч��������ַΪ0��˵��δ���ñ��ַ�����أ�
	if(IsAllAByte(&pMtrRdCtrl->bTsa[1], 0, pMtrRdCtrl->bTsa[0]))	//��ַȫ0
	{
		pCtrl->fInvalid[0] = pCtrl->fInvalid[1] = true;
		return false;
	}
	else
	{		
		memcpy(bAddrBuf, &pMtrRdCtrl->bTsa[0], sizeof(bAddrBuf));
	}	

	if (QueryMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD, wJudgeOadNum))		//���ݵ���
	{
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD wIndex=0; wIndex<wJudgeOadNum; wIndex++)
		{
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD[wIndex], pbBuf);
			if (iLen <= 0)	//û����
				return false;
			
			if (pbBuf[2] == DT_DB_LONG_U)
				iLen = 5;	//ȡ���ܵ���
			else
				iLen = 9;	//ȡ���ܵ���

			memmove(pbBuf, pbBuf+2, iLen);
			//DTRACE(DB_METER_EXC, ("EnergyErr::stepDDD :pn=%d, iLen=%d, wIndex=%d bBuf[0]=0x%02x, bBuf[1]=0x%02x, bBuf[2]=0x%02x, bBuf[3]=0x%02x, bBuf[4]=0x%02x.\r\n", wPn, iLen, wIndex, pbBuf[0], pbBuf[1], pbBuf[2], pbBuf[3], pbBuf[4]));
			pbBuf += iLen;
		}

		fInvalid[0] = fInvalid[1] = false;

#ifdef TERM_EVENT_DEBUG
		ui64PosE = GetEnergyValByFmt(bBuf[0], bBuf, &bLen);	//�����й���
		ui64NegE = GetEnergyValByFmt(bBuf[bLen], bBuf+bLen, &bLen);	//�����й���
#else
		ui64PosE = GetEnergyValByFmt(bBuf[0], bBuf+1, &bLen);	//�����й���
		ui64NegE = GetEnergyValByFmt(bBuf[bLen], bBuf+bLen+1, &bLen);	//�����й���
#endif		
		
		if (memcmp(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1) != 0)		//����һ�γ������ַ���������¿�ʼ�ж�
		{
			memcpy(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1);
			fInit[0] = true;
			fInit[1] = true;
		}
		else
		{
			if (!GetMeterInfo(wPn, &tMtrInfo))	//��ȡ�����ѹ������������߷�ʽ�����ò���F25����
				return false;
	
			wUn = tMtrInfo.wRateVol; //���ѹֵ
			wIn = tMtrInfo.wRateCurr; //�����ֵ
#ifdef TERM_EVENT_DEBUG
			bConnType = tMtrInfo.bLine;
#else
			bConnType = GetConnectType(wPn);
#endif
			if (wUn == 0)
				wUn = 2200;

			if (wIn == 0)
				wIn = 50;

#ifdef TERM_EVENT_DEBUG
			if (bConnType == 2)
#else
			if (bConnType == CONNECT_3P3W)//��������
#endif
			{
				wE0 = 500;//0.05*10000
				btemp = 2;//��������b���޵������������๦�ʼ��ɣ�
			}
			else
			{
				wE0 = 1000;//0.1*10000
				btemp = 3;
			}

			memset(bBuf, 0, sizeof(bBuf));
			iLen = OoReadAttr(wOI, ATTR6, bBuf, NULL, NULL); // ��ȡ���ò���
			if (iLen<=0 || bBuf[2]!=DT_DB_LONG_U)
				dwFlewHold = 800;	//8.0����
			else
				dwFlewHold = OoDoubleLongUnsignedToDWord(&bBuf[3]);	//�쳣�б���ֵ double-long-unsigned����λ��%���޻��㣩
#ifdef TERM_EVENT_DEBUG
			if (dwFlewHold == 0)
				dwFlewHold = 800;	//8.0����
#endif


			memset(bCurErcFlag, ERC_STATE_MIDDLE, sizeof(bCurErcFlag));

			for (i=0; i<2; i++)		//�жϣ�
			{
				//�����ڻ��������й�����ֵ��Ч�����жϣ�
				if (fInvalid[i] || pCtrl->fInvalid[i])
				{
					fInit[i] = true;
					continue;
				}

				if (i==0 && ui64PosE>pCtrl->ui64PosE)	//���������ںͱ����ڵ��ܲ�ֵ�����ʾ���½���ͣ�ߣ����жϣ�
				{
					ui64DeltEnergy = ui64PosE - pCtrl->ui64PosE;
				}
				else if (i==1 && ui64NegE>pCtrl->ui64NegE)
				{
					ui64DeltEnergy = ui64NegE - pCtrl->ui64NegE;
				}
				else
				{
					fInit[i] = true;
					continue;
				}

				//DTRACE(DB_METER_EXC, ("EnergyErr::******RunTask:pn=%d, TimeDec=%d\r\n", wPn, GetClick()-pCtrl->dwSeconds[i]));
				//����������߹��ĵ���
				if((GetClick()-pCtrl->dwSeconds[i]) < ((DWORD)bMtrInterv*60/2))
					continue;

				dwTimePast = (GetClick()-pCtrl->dwSeconds[i]) / ((DWORD) bMtrInterv*60) * ((DWORD)bMtrInterv*60);
				if (dwTimePast == 0)	//����һ����������ʱ��
					dwTimePast = GetClick() - pCtrl->dwSeconds[i];

				ui64PastEnergy = ((uint64) wUn) * wIn * btemp * dwTimePast;	//w����λС����

				//������ʾ���߹��ĵ�������wE0,���¿�ʼ�жϣ�
					//��������λ��w��
				ui64PastEnergy *= dwFlewHold;			//�����������߹��ĵ���
				ui64DeltEnergy *= (1000 * 3600 * 100);	//���ʾ���߹��ĵ���

				if (ui64DeltEnergy >= ui64PastEnergy)	//�й�����ֵ�½��ҷ�����־��0������ֹ����ߵ����̶��������֣�
				{
					bCurErcFlag[i] = ERC_STATE_HAPPEN;	//����
					DTRACE(DB_METER_EXC, ("EnergyErr::###### EnergyFlew event happened! ui64DeltEnergy=%u,  ui64PastEnergy=%u.######\r\n", ui64DeltEnergy, ui64PastEnergy));
				}
				else if (ui64DeltEnergy < ui64PastEnergy)	//�й�����ֵû���½��ҷ�����־��1��
				{
					bCurErcFlag[i] = ERC_STATE_RECOVER;	//�ָ�
					DTRACE(DB_METER_EXC, ("EnergyErr::###### EnergyFlew event recover! ui64DeltEnergy=%u,  ui64PastEnergy=%u.######\r\n", ui64DeltEnergy, ui64PastEnergy));
				}
				else
				{
					bCurErcFlag[i] = ERC_STATE_MIDDLE;
				}

				fInit[i] = true;
			}

			if (bCurErcFlag[0]==ERC_STATE_HAPPEN || bCurErcFlag[1]==ERC_STATE_HAPPEN)	//���� �� ���з���
				bTotalErcFlag = ERC_STATE_HAPPEN;
			else if (bCurErcFlag[0]==ERC_STATE_RECOVER && bCurErcFlag[1]==ERC_STATE_RECOVER)	//���кͷ��ж��ָ�
				bTotalErcFlag = ERC_STATE_RECOVER;
			else
				bTotalErcFlag = ERC_STATE_MIDDLE;

			UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bTotalErcFlag, &pCtrl->bState);
		}

		//��ʼ��
		for (i=0; i<2; i++)
		{
			pCtrl->fInvalid[i] = fInvalid[i];
			if (!fInvalid[i] && fInit[i])
			{
				pCtrl->dwSeconds[i] = GetClick();
				if (i == 0)
					pCtrl->ui64PosE = ui64PosE;
				else
					pCtrl->ui64NegE = ui64NegE;
			}
		}
	}

	return true;
}


void InitMtrStop(BYTE bPn, TMtrStop* pCtrl)
{
	pCtrl->ui64PosE = 0;
	pCtrl->ui64NegE = 0;
	pCtrl->bState = EVT_S_BF_HP;
	pCtrl->dwSeconds = 0;
	pCtrl->fInvalid = true;
	memset(pCtrl->bAddr, 0, sizeof(pCtrl->bAddr));
}


//���������ͣ���¼����жϣ��õ�ǰ����ʼ�����������������������趨ֵ��Ĭ��ֵΪ��.1kWh���������������Բ������仯������Ϊ���ͣ�ߡ������������������仯������Ϊ���ͣ���¼��ָ���ͬʱ�������㡣���������й��ͷ����й���һ���жϡ�
bool DoMtrStop(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{
	BYTE bStopHold = 0;//ͣ�߷�ֵ��
	BYTE bBuf[100];
	BYTE bAlrBuf[20];
	BYTE bAddr[6];
	WORD wValidNum = 0, wIdNum = 0, wJudgeOadNum = 0;
	DWORD dwCurMin;
	BYTE bMtrInterv, bLen, bOffset, bCurErcFlag, bCurState;

	TTime time;	
	bool fErc;
	bool fInit;
	int iLen;
	uint64 ui64PosE;
	uint64 ui64NegE;
	DWORD dwPower;
	DWORD dwDeltSeconds, dwStopHold ;
	uint64 ui64PastEnergy;
	TTimeInterv tiExe;
	BYTE bAddrBuf[MTR_ADDR_LEN];
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE* pbBuf = bBuf;
	const WORD wOI = OI_MTR_STOP;
	const DWORD dwE0 = 3600 * 100;//����ʵ�ʹ�������ʱ�����������ķ�ֵ��0.1kwh)��

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrStop* pCtrl = &pMtrTmp->mtrStop;

	fErc = false;
	fInit = false;
	memset(bAlrBuf, 0, sizeof(bAlrBuf));
	memset(bAddr, 0, sizeof(bAddr));
	memset(&time, 0, sizeof(time));

	if (!GetMtrExcRdItem(wOI, &dwJudgeOAD[0], &wJudgeOadNum, bRelaOAD))
		return false;

	//if (IsRxTrigerSaveCmd(wPn, wOI, pCtrl->bState, &bCurState))
	//	return ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, bCurState, bRelaOAD);

	if (pMtrRdCtrl->bTsa[0] >= sizeof(pMtrRdCtrl->bTsa))
		return false;

	//��ȡ���ֵַ���ж��Ƿ���Ч��������ַΪ0��˵��δ���ñ��ַ�����أ�
	if(IsAllAByte(&pMtrRdCtrl->bTsa[1], 0, pMtrRdCtrl->bTsa[0]))	//��ַȫ0
	{
		pCtrl->fInvalid = true;
		return false;
	}
	else
	{		
		memcpy(bAddrBuf, &pMtrRdCtrl->bTsa[0], sizeof(bAddrBuf));
	}	

	if (QueryMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD, wJudgeOadNum))		//���ݵ���
	{
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD wIndex=0; wIndex<wJudgeOadNum; wIndex++)
		{
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD[wIndex], pbBuf);
			if (iLen <= 0)	//û����
				return false;
			
			pbBuf += iLen;
		}
		
		bOffset = 0;
		ui64PosE = GetEnergyValByFmt(bBuf[bOffset], bBuf+bOffset+1, &bLen);	//�����й���
		bOffset += bLen;

		ui64NegE = GetEnergyValByFmt(bBuf[bOffset], bBuf+bOffset+1, &bLen);	//�����й���
		bOffset += bLen;

		dwPower = ABS(OoDoubleLongToInt(bBuf+bOffset+1));	//ʵ���й�����

		if (memcmp(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1) != 0)		//����һ�γ������ַ���������¿�ʼ�ж�
		{
			memcpy(pCtrl->bAddr, bAddrBuf, bAddrBuf[0]+1);
			fInit = true;
		}
		else if (pCtrl->fInvalid || ui64NegE<pCtrl->ui64NegE || ui64PosE<pCtrl->ui64PosE || dwPower==0)
		{//������ֵ��Ч�����������򣨷����й�����ֵС�����������򣨷����й�����ֵ������δ���ع��ʣ������ڲ��жϣ�
			fInit = true;
		}
		else
		{			
			//��ȡ���ͣ�߷�ֵ��
			memset(bBuf, 0, sizeof(bBuf));
			iLen = OoReadAttr(wOI, ATTR6, bBuf, NULL, NULL); // ��ȡ���ò���
			if (iLen<=0 || bBuf[2]!=DT_TI)
			{
				tiExe.bUnit = 1;
				tiExe.wVal = 15;
			}
			else
			{
				tiExe.bUnit = bBuf[3];
				tiExe.wVal = OoLongUnsignedToWord(&bBuf[4]);	//�쳣�б���ֵ
			}

			dwStopHold = TiToSecondes(&tiExe);	//ͣ�߷�ֵ
			if (dwStopHold == 0)
				dwStopHold = 15 * 60;

			bCurErcFlag = ERC_STATE_MIDDLE;
			if (ui64PosE==pCtrl->ui64PosE && ui64NegE==pCtrl->ui64NegE)	//�жϵ��ͣ�ߣ�
			{
				if (pCtrl->dwSeconds == 0)
				{
#ifdef TERM_EVENT_DEBUG
					pCtrl->dwSeconds =  GetCurTime();
#else
					pCtrl->dwSeconds = GetClick();//ͣ�߷����Ŀ�ʼ��ʱʱ�䣻
#endif
				}
				else
				{
#ifdef TERM_EVENT_DEBUG
					if (GetCurTime() < pCtrl->dwSeconds)
					{
						fInit = true;
						dwDeltSeconds = 0;
					}
					else
					{
						dwDeltSeconds = GetCurTime() - pCtrl->dwSeconds;
					}
#else
					dwDeltSeconds = GetClick() - pCtrl->dwSeconds;
#endif
					ui64PastEnergy = ((uint64)dwPower) * dwDeltSeconds; //(��λw,��λС����

#ifdef TERM_EVENT_DEBUG
					if (dwDeltSeconds>=dwStopHold)
					{
						DTRACE(DB_METER_EXC, ("CMtrStop::RunTask: ******pn=%d, dwDeltSeconds=%d, ui64PastEnergy=%lld\r\n", wPn, dwDeltSeconds, ui64PastEnergy));
						if (ui64PastEnergy >= dwE0/10)	//OOP����Ϊ2λС��
						{
							bCurErcFlag = ERC_STATE_HAPPEN;	//����
							DTRACE(DB_METER_EXC, ("CMtrStop::###### CMtrStop event happened!!! ######\r\n"));
						}
						
						fInit = true;
					}
#else
					DTRACE(DB_METER_EXC, ("CMtrStop::RunTask: ******pn=%d, dwDeltSeconds=%d, ui64PastEnergy=%lld\r\n", wPn, dwDeltSeconds, ui64PastEnergy));
					if (dwDeltSeconds>=dwStopHold && ui64PastEnergy>=dwE0/10)
						bCurErcFlag = ERC_STATE_HAPPEN;	//����

					fInit = true;
#endif
				}
			}
			else	//�����ڵ���ֵ���������ڵ���ֵ�����쳣��־Ϊ1������ͣ�߻ָ���
			{
				bCurErcFlag = ERC_STATE_RECOVER; //�ָ�
				fInit = true;
			}
			
			UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bCurErcFlag, &pCtrl->bState);
		}

		pCtrl->ui64PosE = ui64PosE;
		pCtrl->ui64NegE = ui64NegE;
		pCtrl->fInvalid = false;
		if (fInit)
			pCtrl->dwSeconds = 0;
	}

	return fErc;
}

//��ʼ������ʧ���¼�
void InitMtrRdFail(WORD wPn, TMtrRdFail* pCtrl)
{
	pCtrl->bState = EVT_S_BF_HP;
}


//����ʧ���¼�
bool DoMtrRdFail(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{	
	bool fRet;
	DWORD dwMtrSecs, dwCurSecs, dwDiff;
	int iLen;	
	TTime time;
	TTime tmMtrTime;
	BYTE bCurState, bLen;
	WORD wChecktmHold, wRecoverHold, wJudgeOadNum = 0;
	BYTE bAlrBuf = 0, bCurErcFlag = 0;
	BYTE bBuf[40];
	DWORD dwJudgeOAD[MAX_JUDGE_OAD];
	BYTE* pbBuf = bBuf;
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
#ifdef TERM_EVENT_DEBUG
	const WORD wOI = OI_MTR_RD_FAIL;
#else
	const WORD wOI = OI_MTR_CLOCK_ERR;
#endif

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrRdFail* pCtrl = &pMtrTmp->mtrRdFail;

	//DTRACE(DB_METER_EXC, ("DoMtrRdFail::###### DoMtrRdFail step 0!!! ######\r\n"));
	memset(&time, 0, sizeof(time));
	memset(&tmMtrTime, 0, sizeof(tmMtrTime));
	if (!GetMtrExcRdItem(wOI, &dwJudgeOAD[0], &wJudgeOadNum, bRelaOAD))
		return false;

	//if (IsRxTrigerSaveCmd(wPn, wOI, pCtrl->bState, &bCurState))
	//	return ReadAndSaveMtrExcRec(pMtrRdCtrl, pMtrPro, wOI, bCurState, bRelaOAD);
					
	//DTRACE(DB_METER_EXC, ("DoMtrRdFail::RunTask:  pCtrl->bState=%d at click = %ld.\r\n", pCtrl->bState, GetClick()));	

	if (IsMtrErr(wPn))	//����״̬
	{
		bCurErcFlag = ERC_STATE_HAPPEN;
		DTRACE(DB_METER_EXC, ("DoMtrRdFail::###### DoMtrRdFail event happened!!! ######\r\n"));
	}
	else	//����״̬
		bCurErcFlag = ERC_STATE_RECOVER;

	UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bCurErcFlag, &pCtrl->bState);		//�����������Ա����ݣ����浽��ʱ��¼��

	return true;
}



//========================���ܱ����ݱ����ؼ�¼==========================
//���������ܱ����ݱ����ؼ�¼����ȡ����OAD
/*int GetMtrDataChgCSD(BYTE* bCfg)
{
	int iLen;
	TTaskCfg tTaskCfg;
	TCommAcqSchCfg tCommAcqSchCfg;
	BYTE bIndex, bBuf[MTEDATACHG_CSD_LEN];
	BYTE* pbCSDCfg = bBuf;

	if (bCfg == NULL)
		return -1;

	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(OI_MTR_DATA_CHG, ATTR6, bBuf, NULL, NULL);	// ��ȡ���ò���
	if (iLen > 0 && bBuf[0] == DT_STRUCT && bBuf[1] == 0x01  && bBuf[2] == DT_UNSIGN)
		bIndex = bBuf[3];
	else
		return -1;

	if (!GetTaskCfg(bIndex, (TTaskCfg*)&tTaskCfg))
		return -1;
	
	if (!GetCommonSchCfg((TTaskCfg*)&tTaskCfg, (TCommAcqSchCfg*)&tCommAcqSchCfg))
		return -1;

	iLen = 0;
	memset(bBuf, 0, sizeof(bBuf));
	
	//��ȡ��������ĵ�һ��CSD
	if ((tCommAcqSchCfg.bCSDNum > 0))
	{	
		pbCSDCfg++;		//��һ���ֽ�ΪCSD�ĳ���
		*pbCSDCfg = DT_CSD;pbCSDCfg++;iLen++;
		*pbCSDCfg = tCommAcqSchCfg.tTCSD[0].bChoice;pbCSDCfg++;iLen++;
		if (tCommAcqSchCfg.tTCSD[0].bChoice == 0)	
		{	
			OoDWordToOad(tCommAcqSchCfg.tTCSD[0].dwOAD, pbCSDCfg);
			iLen +=4;
			bBuf[0] = iLen;
			memcpy(bCfg, bBuf, iLen+1);
		}
		//else
		//{	
		//	memcpy(&bBuf[2], tCommAcqSchCfg.tTCSD.tTROAD, 4);		//������һ�³¹���ô��ȡROAD��ʽ����ʱֻ֧��OAD������������
		//	return true;
		//}
	}

	return iLen;
}*/

//���������ܱ����ݱ����ؼ�¼����ȡ����OAD
int GetMtrDataChgCSD(BYTE* bCfg)
{
	TTaskCfg tTaskCfg;
	int iLen, iSchCfgLen, iArryIdx;
	WORD wFmtLen, wCfgLen, wDataFmtLen;
	BYTE bIndex, bType, bBuf[10] = {0};
	BYTE *pbSch, *pbFmt, *pbArryCsd, *pbDataFmt;
	BYTE *pbCfg0 = bCfg;

	if (bCfg == NULL)
		return -1;

	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(OI_MTR_DATA_CHG, ATTR6, bBuf, NULL, NULL);	// ��ȡ���ò���
	if (iLen > 0 && bBuf[0] == DT_STRUCT && bBuf[1] == 0x01  && bBuf[2] == DT_UNSIGN)
		bIndex = bBuf[3];
	else
		return -1;

	if (!GetTaskCfg(bIndex, (TTaskCfg*)&tTaskCfg))
		return -1;

	if (tTaskCfg.bSchType != SCH_TYPE_COMM)	//��ʱֻ������ͨ�ɼ�����
		return -1;

	pbSch = GetSchCfg(&tTaskCfg, &iSchCfgLen);
	if (pbSch==NULL)
		return -1;

	pbFmt = GetSchFmt(tTaskCfg.bSchType, &wFmtLen);
	if(pbFmt == NULL)
		return -1;

	iArryIdx = 3;
	pbArryCsd = OoGetField(pbSch, pbFmt, wFmtLen, iArryIdx, &wCfgLen, &bType, &pbDataFmt, &wDataFmtLen);
	if (pbArryCsd == NULL)
		return -1;

	if (pbArryCsd[0]==DT_ARRAY && pbArryCsd[1]!=0)
	{
		iLen = ScanCSD(&pbArryCsd[3], false);	//����DT_CSD��ʽ
		if (iLen > 0)
		{
			//memcpy(pbSch, &pbArryCsd[2], iLen+1);	//+1������DT_CSD��ʽ
			iLen++;		//+1������DT_CSD��ʽ
			*bCfg++ = iLen;
			memcpy(bCfg, &pbArryCsd[2], iLen);
			bCfg += iLen;
			bCfg = pbCfg0;
			return iLen;

		}
	}

	return -1;
}

//���������ܱ����ݱ����ؼ�¼�ĳ�ʼ����
void InitMtrDataChg(WORD wPn, TMtrDataChg* pCtrl)
{
	pCtrl->bState = EVT_S_BF_HP;
	memset(pCtrl->bAddr, 0, sizeof(pCtrl->bAddr));
	memset(pCtrl->bOldCSD, 0, sizeof(pCtrl->bCSD));
	memset(pCtrl->bCSD, 0, sizeof(pCtrl->bCSD));
	memset(pCtrl->bOldData, 0, sizeof(pCtrl->bOldData));
	memset(pCtrl->bNewData, 0, sizeof(pCtrl->bNewData));
}


int GetCsdRec(TMtrRdCtrl* pMtrRdCtrl, BYTE *pbCsd, WORD wRcsdLen, BYTE *pbRec, WORD wMaxRecLen)
{
	TOobMtrInfo tMtrInfo;
	int iRet, iTabIdx, iStart;
	WORD wPn, wRetNum;
	BYTE bOAD[4];
	BYTE bRsd[32];
	BYTE bRcsd[256];
	BYTE *pRsd, *pRcsd;
	
	//OAD
	bOAD[0] = 0x60;
	bOAD[1] = 0x12;
	bOAD[2] = 0x03;
	bOAD[3] = 0x00;

	//RSD
	pRsd = bRsd;
	*pRsd++ = 0x0A;	//����10
	*pRsd++ = 0x01;	//��һ�ʼ�¼
	GetMeterInfo(&pMtrRdCtrl->bTsa[1], pMtrRdCtrl->bTsa[0], &tMtrInfo);
	*pRsd++ = 0x04;	//MSһ���������
	*pRsd++ = 0x01;	//1�������
	pRsd += OoWordToOi(tMtrInfo.wMtrSn, pRsd);

	//RCSD
	pRcsd = bRcsd;
	*pRcsd++ = 0x01;	//RCSD����
	memcpy(pRcsd, pbCsd+1, wRcsdLen-1);	//+1��ȥ��DT_CSD
	pRcsd += (wRcsdLen-1);

	iTabIdx = 0;
	iStart = -1;
	iRet = ReadRecord(bOAD, bRsd, bRcsd, &iTabIdx, &iStart, pbRec, wMaxRecLen, &wRetNum);

	return iRet;
}

//���������ܱ����ݱ����ؼ�¼���жϣ�����б䣬����Ϊ����һ����¼
bool DoMtrDataChg(TMtrRdCtrl* pMtrRdCtrl, TMtrPro* pMtrPro, WORD wPn)
{	
	static DWORD dwClick = 0;
	BYTE bCurErcFlag;
	bool fIsCSDChg = false;
	int iLen = 0;	
	WORD wJudgeOadNum = 0;
	BYTE bBuf[512];
	DWORD dwJudgeOAD[MAX_JUDGE_OAD], dwOAD;
	BYTE* pbBuf = bBuf;
	BYTE bRelaOAD[EVT_ATTRTAB_LEN] = {0};
	const WORD wOI = OI_MTR_DATA_CHG;

	TMtrExcTmp* pMtrTmp = &pMtrRdCtrl->mtrExcTmp;
	TMtrClockErr* pCtrl = &pMtrTmp->mtrClockErr;

	BYTE bCSDCfg[MTEDATACHG_CSD_LEN];

	iLen = OoReadAttr(wOI, ATTR3, bRelaOAD, NULL, NULL);		//���������Ա�
	if (iLen <= 0)
		return false;

	memset(bCSDCfg, 0, sizeof(bCSDCfg));	//bCSDCfg�ĵ�һ���ֽ�Ϊ����
	iLen = GetMtrDataChgCSD(bCSDCfg);
	if (iLen <= 0)
		return false;

	if (memcmp(pMtrTmp->mtrDataChg.bCSD, bCSDCfg, iLen) != 0)	//���ò����б�
	{
		memcpy(pMtrTmp->mtrDataChg.bOldCSD, pMtrTmp->mtrDataChg.bCSD, MTEDATACHG_CSD_LEN);
		memset(pMtrTmp->mtrDataChg.bCSD, 0, sizeof(MTEDATACHG_CSD_LEN));
		memcpy(pMtrTmp->mtrDataChg.bCSD, bCSDCfg, iLen);	
		fIsCSDChg = true;
	}

	//��ȡ���ݺ�����Ҫ�¹��ṩ�ӿڣ���ʱ��֧��OAD��̨�ӡ�����������������
	/*if (bCSDCfg[1] != DT_CSD)
		return false;
	
	if (bCSDCfg[2] != 0)
		return false;
	
	wJudgeOadNum = 1;
	memcpy((BYTE*)&dwOAD, &bCSDCfg[3], 4);
	OoDWordToOad(dwOAD, (BYTE*)&dwJudgeOAD);

	if (QueryMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD, wJudgeOadNum))		//���ݵ���
	{
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD wIndex=0; wIndex<wJudgeOadNum; wIndex++)
		{
			iLen = GetMtrItemMem(&pMtrRdCtrl->mtrTmpData, dwJudgeOAD[wIndex], pbBuf);
			if (iLen <= 0)	//û����
				return false;

			pbBuf += iLen;
			fIsNeedCmp = true;	//��ȡ��iLen���ȵ�����
		}
	}
	//���ϲ����ɳ¹��ṩ����ȡ���ݵ�pbBuf����������ݵĳ���iLen��
	

	if (fIsNeedCmp)		//���ݵ���*/
	
	if (bCSDCfg[1] != DT_CSD)
		return false;
	
	//if (bCSDCfg[1] != 0)
	//	return false;

	if  (GetClick()-dwClick<60) 	// 1���ӳ�һ������
		return false;

	dwClick = GetClick();
	
	memset(bBuf, 0, sizeof(bBuf));
	iLen = GetCsdRec(pMtrRdCtrl, bCSDCfg+1, iLen, bBuf, sizeof(bBuf));	//��ע������Ҫ���Ʒ��������Ĵ�������10s����һ��!!! add CL
	if (iLen > 0)		//���ݵ���
	{
		if ((IsAllAByte(pMtrTmp->mtrDataChg.bOldData, 0, sizeof(pMtrTmp->mtrDataChg.bOldData))) || fIsCSDChg)	
		{
			pMtrTmp->mtrDataChg.bOldData[0] = iLen;
			memcpy(pMtrTmp->mtrDataChg.bOldData+1, bBuf, iLen);
		}

		if ((IsAllAByte(pMtrTmp->mtrDataChg.bNewData, 0, sizeof(pMtrTmp->mtrDataChg.bNewData))) || fIsCSDChg)	
		{	
			pMtrTmp->mtrDataChg.bNewData[0] = iLen;
			memcpy(pMtrTmp->mtrDataChg.bNewData+1, bBuf, iLen);
		}

		if (memcmp(pMtrTmp->mtrDataChg.bNewData+1, bBuf, iLen) != 0)
		{
			memcpy(pMtrTmp->mtrDataChg.bOldData, pMtrTmp->mtrDataChg.bNewData, pMtrTmp->mtrDataChg.bNewData[0]+1);
			pMtrTmp->mtrDataChg.bNewData[0] = iLen;
			memcpy(pMtrTmp->mtrDataChg.bNewData+1, bBuf, iLen);
			bCurErcFlag = ERC_STATE_HAPPEN;
		}
		else
			bCurErcFlag = ERC_STATE_RECOVER;

		UpdateMtrExcStateAndSaveRec(pMtrRdCtrl, pMtrPro, wOI, bRelaOAD, bCurErcFlag, &pCtrl->bState);

		return true;
	}

	return false;
}

//**************************�¼���������*********************************************
//����1����λ
int DoMtrExcMethod1(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bClrFlag;
	int iPn;

	if (bMethod!=EVT_RESET || iParaLen!=0x02)
		goto END_ERR;

	iPn = GetMtrExcIndex(wOI);
	if (iPn < 0)
		goto END_ERR;

	if (pbPara[0]==DT_INT && pbPara[1]==0x00)
	{
		bClrFlag = EVT_CLR_VALID;
		WriteItemEx(BN11, iPn, MTREXC_CLR_ID, &bClrFlag);
		TrigerSaveBank(BN11, 0, -1); //��������
		SetInfo(INFO_MTR_EXC_RESET);

		*pbRes = 0;	//�ɹ�  ��0��
		return 0;
	}

END_ERR:
	*pbRes = 3;	
	return -1;
}

//����2��ִ��
//�պ���
int DoMtrExcMethod2(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	int iPn;	

	if (bMethod != EVT_RUN)	
		goto END_ERR;
	
	iPn = GetMtrExcIndex(wOI);
	if (iPn < 0)
		goto END_ERR;

	//nothing to do

	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;

END_ERR:
	*pbRes = 3;	
	return -1;
}



//����3������һ�μ�¼
//���������津����������
//�������¼�����Դ�� �ӳٴ���ʱ�䡢 �ӳٻָ�ʱ��
int DoMtrExcMethod3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	int iOffset;
	BYTE bTsaLen;
	WORD wPn, wID;
	DWORD dwCurSec = 0;
	BYTE bBuf[30];
	BYTE* p = bBuf;
	const WORD wBaseID = 0x0b20;

	if (bMethod != EVT_TRIG)	
		goto END_ERR;

	iOffset = GetMtrExcIndex(wOI);
	if (iOffset < 0)
		goto END_ERR;

	wID = wBaseID + iOffset;
#ifdef MTREXC_ADDR_TPYE_TSA
	if (*pbPara != DT_TSA)
		goto END_ERR;
#else
	if (*pbPara != DT_OCT_STR)
		goto END_ERR;
#endif

	bTsaLen = pbPara[2] & 0x0f;		//TSA_OCTSTR_TYPE
	bTsaLen++;

	wPn = MtrAddrToPn(&pbPara[3], bTsaLen);
	if (wPn == PN0)
		goto END_ERR;

	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN11, wPn, wID, bBuf);
	if (bBuf[8] != 0)	//״̬����Ϊ0����ʾ���ڴ���������,�ݲ�����������
		goto END_ERR;

	p = bBuf;
	memset(bBuf, 0, sizeof(bBuf));
	memcpy(p, pbPara+bTsaLen+4, 2);		//�ӳٷ���ʱ��
	p += 2;

	memcpy(p, pbPara+bTsaLen+7, 2);		//�ӳٻָ�ʱ��
	p += 2;

	dwCurSec = GetCurTime();
	memcpy(p, &dwCurSec, sizeof(DWORD));
	p += 4;
	
	WriteItemEx(BN11, wPn, wID, bBuf);
 
	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;

END_ERR:
	*pbRes = 3;	
	return -1;	
}

//����4�����һ�������������ԣ�������
//������=OAD ��������������
int DoMtrExcMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	int iLen, iPn;
	BYTE bClrFlag;
	BYTE bOADNum, bIndex;
	BYTE bBuf[EVT_ATTRTAB_LEN];	//�������Ա�����

	if ((bMethod!=EVT_ADDATTR)	|| (iParaLen!=5) || (pbPara[0]!=DT_OAD))
		goto END_ERR;

	iPn = GetMtrExcIndex(wOI);
	if (iPn < 0)
		goto END_ERR;

	//�������OAD
	iLen = OoReadAttr(wOI, ATTR3, bBuf, NULL, NULL);	//����3 �����������Ա�
	if (iLen<=0 || bBuf[0]!=DT_ARRAY || bBuf[1]==0x00)	//����������������Ϊ�գ���������OAD
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = DT_ARRAY;
		bBuf[1] = 1;
		memcpy(bBuf, pbPara, 5);
		goto END_OK;
	}

	bOADNum = bBuf[1];
	if (bOADNum >= CAP_OAD_NUM)	//�Ѵ���
		goto END_ERR;

	for (bIndex=0; bIndex<bOADNum; bIndex++)
	{
		if (memcmp(&bBuf[2+bIndex*5], pbPara, 5) == 0)	//�Ѵ���OAD
			goto END_ERR;
	}

	if (bIndex == bOADNum)	//û����ͬ�ģ�������һ��OAD
	{
		bBuf[1]++;
		memcpy(&bBuf[2+bIndex*5], pbPara, 5);	//�����ӵķ������
		goto END_OK;
	}

END_ERR:
	*pbRes = 3;	
	return -1;
	
END_OK:
	if (OoWriteAttr(wOI, ATTR3, bBuf) <= 0)		//����3 �����������Ա�
		goto END_ERR;

	bClrFlag = EVT_CLR_VALID;
	WriteItemEx(BN11, iPn, MTREXC_CLR_ID, &bClrFlag);		//д�����ʶ
	TrigerSaveBank(BN11, 0, -1);
	SetInfo(INFO_MTR_EXC_RESET);

	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;
}

//����5��ɾ��һ���������ԣ�������
//������=OAD ��������������
int DoMtrExcMethod5(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bBuf[EVT_ATTRTAB_LEN];	//�������Ա�����
	BYTE bOADNum, bIndex;
	BYTE bClrFlag;
	int iLen, iPn, j = 0;

	if (bMethod!=EVT_DELATTR || iParaLen!=5 || pbPara[0]!=DT_OAD)
		goto END_ERR;
	
	iPn = GetMtrExcIndex(wOI);
	if (iPn < 0)
		goto END_ERR;
	
	iLen = OoReadAttr(wOI, ATTR3, bBuf, NULL, NULL);	//����3 �����������Ա�
	if ((iLen<=0) || (bBuf[1]!=DT_ARRAY) || (bBuf[1]==0x00))	//����������������Ϊ�գ��޷�ɾ��
		goto END_ERR;

	bOADNum = (bBuf[1]>CAP_OAD_NUM) ? CAP_OAD_NUM : bBuf[1];
	for (bIndex=0; bIndex<bOADNum; bIndex++)	//�����Ƿ��Ѿ�����
	{
		if (memcmp(&bBuf[2+bIndex*5], pbPara, 5) == 0)	//�Ѵ���OAD
		{
			memset(&bBuf[bIndex*5 + 2], 0, 5);
			bBuf[1]--;	//����Ԫ�ظ���
			break;
		}
	}
	
	if (bIndex == bOADNum)	//û�ҵ�
		goto END_ERR;
	
	for (j=bIndex; j<bOADNum-1; j++)		//����Ĳ�����ǰŲ
	{
		memcpy(&bBuf[j*5 + 2], &bBuf[(j+1)*5 + 2], 5);
	}
	
	// ˢ�¹������Ա�
	if (OoWriteAttr(wOI, ATTR3, bBuf) <= 0)
		goto END_ERR;

	bClrFlag = EVT_CLR_VALID;
	WriteItemEx(BN11, iPn, MTREXC_CLR_ID, &bClrFlag);		//д�����ʶ
	TrigerSaveBank(BN11, 0, -1);
	SetInfo(INFO_MTR_EXC_RESET);

	*pbRes = 0;	//�ɹ�  ��0�� ���ؽ��
	return 0;

END_ERR:
	*pbRes = 3;
	return -1;
}


//���������澯�¼����ƽṹ�ĳ�ʼ����
//		���ϵ����߲����������ú���ñ�����������ÿ�ζ�����
void InitMtrExcCtrl(BYTE bPn, TMtrExcTmp* pCtrl)
{
	InitMtrClockErr(bPn, &pCtrl->mtrClockErr);	//ERC_MTRTIME:

	InitEnergyErr(bPn, &pCtrl->mtrEnergyErr);	//ERC_MTRERR:

	InitEnergyDec(bPn, &pCtrl->mtrEnergyDec); //���ܱ�ʾ���½�
	  
	InitMtrStop(bPn, &pCtrl->mtrStop); //���ܱ�ͣ��

	InitMtrFlew(bPn, &pCtrl->mtrFlew);//�����¼�

	InitMtrRdFail(bPn, &pCtrl->mtrRdFail);	//���ܱ���ʧ���¼�

	InitMtrDataChg(bPn, &pCtrl->mtrDataChg);	//���ܱ����ݱ����ؼ�¼

	memset(pCtrl->wLastRecPhyIdx, 0, sizeof(pCtrl->wLastRecPhyIdx));
	memset(pCtrl->dwLastStatClick, 0, sizeof(pCtrl->dwLastStatClick));
	memset(pCtrl->bTryReadCnt, 0, sizeof(pCtrl->bTryReadCnt));
}

//���䳭���¼���ʱ�ڴ�ռ�
bool AllocateMtrExcMem(BYTE* pbGlobal, TAllocTab* pAllocTab, WORD wTabNum)
{
	DWORD dwOAD = 0;
	int iLen, nOADLen = 0;
	WORD i, wOI, wRecLen;
	BYTE bIndex, bCapNum = 0;
	bool fAllocateSuccess = true;	//�Ƿ����ɹ�
	BYTE bRelaOAD[EVT_ATTRTAB_LEN];
	BYTE* pbRelaOAD = bRelaOAD;

	for (bIndex=0; bIndex<MTR_EXC_NUM; bIndex++)	//�¼��������
	{
		wRecLen = 0;
		wOI = GetMtrExcOI(bIndex);	//��ȡ�¼�OI
		if (wOI == 0)
		{
			fAllocateSuccess = false;
			continue;
		}

		memset(bRelaOAD, 0, sizeof(bRelaOAD));
		iLen = OoReadAttr(wOI, ATTR3, pbRelaOAD, NULL, NULL);	//����3 �������Ա�
		bCapNum = (pbRelaOAD[1]>CAP_OAD_NUM) ? CAP_OAD_NUM : pbRelaOAD[1];		
		if (iLen>0 && pbRelaOAD[0]==DT_ARRAY && bCapNum>0)
		{
			for (i=0; i<bCapNum; i++)	//�������Ա�Ԫ�ظ���
			{
				dwOAD = OoOadToDWord(&pbRelaOAD[5*i+3]);
				nOADLen = OoGetDataLen(DT_OAD, &pbRelaOAD[5*i+3]);
				if (nOADLen <= 0)
				{
					wRecLen = 0;
					break;
				}

				wRecLen += nOADLen;
			}

			if (wRecLen > 0)
			{
				if (!AllocMem(pbGlobal, pAllocTab, wTabNum, MEM_TYPE_MTREXC, wOI, wRecLen))
					fAllocateSuccess = false;
			}
		}
	}

	if (!fAllocateSuccess)
		DTRACE(DB_METER_EXC, ("AllocateMtrExcMem::wOI = %04x, allocate fail! \r\n", wOI));

	return fAllocateSuccess;
}




//��������������ʱ���������㴦��
//������@pEvtCtrl�¼�����
//���أ���
//ע���������������һ���¼���¼������
// 1. �������Ա���---���ò���/���ɾ��OAD����
// 2. ����¼���ݱ��
// 3. �������---��λ����/�������/�¼�������/��������
void ClrMtrExc(int nIndex)
{
	int iLen;
	WORD wPn, wOI;
	WORD wCurRecNum;
	BYTE bClrFlag = 0;
	const ToaMap* pOaMap;
	DWORD dwOAD, dwTimes, dwSec;
	BYTE bBuf[EVT_ATRR_MAXLEN];
	
	if (nIndex < 0)
		return;

	wOI = GetMtrExcOI(nIndex);
	if (wOI == 0)
		return;

	wPn = nIndex;
	if (ReadItemEx(BN11, wPn, MTREXC_CLR_ID, &bClrFlag) <= 0)
		return;

	if (bClrFlag != MTREXC_CLR_VALID)
	{
		bClrFlag = 0;
		WriteItemEx(BN11, wPn, MTREXC_CLR_ID, &bClrFlag);	//��������е������ٽ���ʶ����
		return;	//����Ҫ����
	}	

	dwOAD = GetOAD(wOI, ATTR2, 0);
	pOaMap = GetOIMap(dwOAD);
	if (pOaMap != NULL)
	{
		TdbDeleteTable(pOaMap->pszTableName);	//ɾ����¼��
		ClrMtrExcTableFlag(wOI);	//��������־
	}

	//��0ͳ������
	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, ATTR4, bBuf, NULL, NULL);	//��ǰ��¼���ۼ�
	if (iLen > 0)
	{
		wCurRecNum = 0;
		OoWordToLongUnsigned(wCurRecNum, bBuf+1);
		OoWriteAttr(wOI, ATTR4, bBuf);
	}

	iLen =OoReadAttr(wOI, ATTR7, bBuf, NULL, NULL);		//��ǰֵ��¼���ۼƴ����ۼ�
	if (iLen > 0)
	{
		dwTimes = 0;
		OoDWordToDoubleLongUnsigned(dwTimes, bBuf+25);
		OoWriteAttr(wOI, ATTR7, bBuf);

		dwSec = 0;
		OoDWordToDoubleLongUnsigned(dwSec, bBuf+30);
		OoWriteAttr(wOI, ATTR7, bBuf);
	}

	//�������ʶ���
	bClrFlag = 0;
	WriteItemEx(BN11, wPn, MTREXC_CLR_ID, &bClrFlag);	//��������е������ٽ���ʶ����
	TrigerSaveBank(BN11, 0, -1);
}


//�����¼��������
void ClrMtrExc()
{
	WORD i;

	for (i=0; i<MTR_EXC_NUM; i++)
	{	
		ClrMtrExc(i);	//������������
	}
}



//����:	�����¼�ϵͳ����DYN�������ݣ�������¼�����ԡ���ǰֵ��¼������
bool UpdateMtrExcStatData(WORD wOI, BYTE bState, TMtrExcTmp* pExcTmp, BYTE* pbTsa)
{
	BYTE bOffset  = 0;
	int iLen, nIndex;
	TTime tmCurRec;
	bool fOnMtrExcHap, fOnMtrExcEnd, fTrigerSave = false;
	WORD wMaxNum = 0, wCurRecNum = 0;
	DWORD dwTimes, dwClick, dwSec;
	BYTE bBuf[EVT_ATRR_MAXLEN];

	nIndex = GetMtrExcIndex(wOI);
	if (nIndex < 0)
		return false;

	if (OoReadAttr(wOI, ATTR5, bBuf, NULL, NULL) > 0)	//����¼��
		wMaxNum = OoLongUnsignedToWord(bBuf+1);

	if (wMaxNum == 0)
		return false;

	iLen = OoReadAttr(wOI, ATTR4, bBuf, NULL, NULL);	//��ǰ��¼���ۼӣ���ȡ��ֵ
	if (iLen <= 0)
		return false;

	fOnMtrExcHap = (bState==EVT_S_AFT_HP && pExcTmp->dwLastStatClick[nIndex]==0);	//�¼��շ���
	fOnMtrExcEnd = (bState==EVT_S_AFT_END && pExcTmp->dwLastStatClick[nIndex]!=0);	//�¼��ս���

	dwClick = GetClick();
	wCurRecNum = OoLongUnsignedToWord(bBuf+1);	//�¼���������
	if (wCurRecNum<wMaxNum && fOnMtrExcHap)	//��ǰ��¼����1��ֱ����¼����������¼��
	{
		wCurRecNum++;
		OoWordToLongUnsigned(wCurRecNum, bBuf+1);
		OoWriteAttr(wOI, ATTR4, bBuf);
	}

	iLen =OoReadAttr(wOI, ATTR7, bBuf, NULL, NULL);		//��ǰֵ��¼���ۼƴ����ۼӣ���ȡ��ֵ
	if (iLen <= 0)
		return false;

	bOffset = 0;
	bOffset += 4;

	bBuf[bOffset] = DT_TSA;
	bOffset++;	

	if (pbTsa[0]<=16 && pbTsa[0]>0)	//���ʵ�ʳ���
	{
		bBuf[bOffset] = pbTsa[0] + 1;	//length
		bOffset++;

		bBuf[bOffset] = pbTsa[0] - 1;	//length
		bOffset++;

		memcpy(bBuf+bOffset, &pbTsa[1], pbTsa[0]);	//�����ʵ�ʳ���
		bOffset += pbTsa[0];
	}
	else
	{
		bBuf[bOffset] = DT_NULL;
		bOffset++;
	}

	bBuf[bOffset] = DT_STRUCT;
	bOffset++;

	bBuf[bOffset] = 0x02;
	bOffset++;

	if (fOnMtrExcHap)
	{
		
		bBuf[bOffset] = DT_DB_LONG_U;
		bOffset++;
		dwTimes = OoDoubleLongUnsignedToDWord(bBuf+bOffset);
		dwTimes++;
		OoDWordToDoubleLongUnsigned(dwTimes, bBuf+bOffset);
		bOffset += 4;

		bBuf[bOffset] = DT_DB_LONG_U;
		bOffset++;

		memset(&bBuf[bOffset], 0, 4);
		bOffset += 4;

		OoWriteAttr(wOI, ATTR7, bBuf);

		pExcTmp->dwLastStatClick[nIndex] = dwClick;		//��������ܸ����¼�������ͳ��ʱ�� ---�Ƿ���Ҫ��������ͳ�����ݣ�
		fTrigerSave = true;
	}
	else if (bState==EVT_S_AFT_HP || bState==EVT_S_BF_END)		//�¼������ڼ���ۼ�ʱ��
	{
		bOffset += 5;

		bBuf[bOffset] = DT_DB_LONG_U;
		bOffset++;

		dwSec = OoDoubleLongUnsignedToDWord(bBuf+bOffset);
		if (pExcTmp->dwLastStatClick[nIndex] && dwClick > pExcTmp->dwLastStatClick[nIndex])
		{
			dwSec += (dwClick-pExcTmp->dwLastStatClick[nIndex]);
			OoDWordToDoubleLongUnsigned(dwSec, bBuf+bOffset);

			OoWriteAttr(wOI, ATTR7, bBuf);
			pExcTmp->dwLastStatClick[nIndex] = dwClick;
		}
		//fTrigerSave = true;	//�����һֱдϵͳ��
	}
	else if (fOnMtrExcEnd)	//�¼��ս���
	{
		pExcTmp->dwLastStatClick[nIndex] = 0;	//����ͳ��ʱ�ֹ꣬ͣͳ��(�¼������ڼ��ͳ������) ---�Ƿ���Ҫ��������ͳ�����ݣ�
	}

	if (fTrigerSave)
		TrigerSaveBank(BN0, SECT16, -1);

	return true; 
}


//�����������¼����������Ҫ���³�ʼ���¼���
//������@dwOAD���ݱ�ʶ
//���أ���
void ReInitMtrExcPara(DWORD dwOAD)
{
	int nIndex;
	BYTE bClrFlag, bAttr = 0;
	WORD wOI, wPn;

	GetOIAttrIndex(dwOAD, &wOI, &bAttr, NULL);
	nIndex = GetMtrExcIndex(wOI);
	if (nIndex < 0)
		return;

	if (bAttr==ATTR3 || bAttr==ATTR5)	//�������Ա������¼�����ʱ��Ҫ���¼������³�ʼ��
	{
		wPn = nIndex;
		bClrFlag = MTREXC_CLR_VALID;
		WriteItemEx(BN11, wPn, MTREXC_CLR_ID, &bClrFlag);		//д�����ʶ
		TrigerSaveBank(BN11, 0, -1);

		SetInfo(INFO_MTR_EXC_RESET);
	}
}


//�����������¼����������Ҫ���³�ʼ���¼����ṩ�����ж���ӿ�
//������@dwOAD���ݱ�ʶ
//���أ���
void ReInitEvtPara(DWORD dwOAD)
{
	ReInitMrtEvtPara(dwOAD);
	ReInitMtrExcPara(dwOAD);
}



//�յ�����/����/�¼���¼��ʼ������ʱ�Ĵ����ⲿ����
void MtrExcOnRxFaResetCmd()
{
	int nIndex;
	BYTE bClrFlag;
	WORD wPn;

	for (nIndex=0; nIndex<MTR_EXC_NUM; nIndex++)
	{
		wPn = nIndex;
		bClrFlag = MTREXC_CLR_VALID;
		WriteItemEx(BN11, wPn, MTREXC_CLR_ID, &bClrFlag);		//д�����ʶ
	}

	TrigerSaveBank(BN11, 0, -1);
	SetInfo(INFO_MTR_EXC_RESET);
}

//0x3105 ���ܱ�ʱ�ӳ���
const BYTE g_bMClkCfg[] = {
	0x01,0x00,
};
//0x310B ���ܱ�ʾ���½�
const BYTE g_bMDecCfg[] = {
	0x01,0x02,
	0x51,0x00,0x10,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,
};
//0x310C ����������
const BYTE g_bMErrCfg[] = {
	0x01,0x02,
	0x51,0x00,0x10,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,
};
//0x310D ���ܱ����
const BYTE g_bMFCfg[] = {
	0x01,0x02,
	0x51,0x00,0x10,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,
};
//0x310E ���ܱ�ͣ��
const BYTE g_bMSCfg[] = {
	0x01,0x01,
	0x51,0x00,0x10,0x22,0x01,
};
//0x310F ����ʧ���¼�
const BYTE g_bMRFCfg[] = {
	0x01,0x02,
	//0x51,0x60,0x41,0x22,0x00,
	0x51,0x00,0x10,0x22,0x01,
	0x51,0x00,0x30,0x22,0x01,
};
//0x311C ���ܱ����ݱ����ؼ�¼
const BYTE g_bDaCgCfg[] = {
	0x01,0x00,
};

//���������ù������Ա�Ĭ�ϲ���
void SetMtrExcOadDefCfg(WORD wOI)
{
	BYTE bBuf[EVT_ATTRTAB_LEN];
	DWORD dwOAD;
	int iLen;
	const ToaMap* pOI;
	
	dwOAD = GetOAD(wOI, ATTR3, 0);
	pOI = GetOIMap(dwOAD);
	if (pOI == NULL)
		return;

	memset(bBuf, 0, sizeof(bBuf));	
	iLen = ReadItemEx(BN0, pOI->wPn, pOI->wID, bBuf);
	if (iLen>0 && IsAllAByte(bBuf, 0, sizeof(bBuf)))
	{
		if (wOI == OI_MTR_CLOCK_ERR)
			memcpy(bBuf, g_bMClkCfg, sizeof(g_bMClkCfg));
		else if (wOI == OI_MTR_ENERGY_DEC)
			memcpy(bBuf, g_bMDecCfg, sizeof(g_bMDecCfg));
		else if (wOI == OI_MTR_ENERGY_ERR)
			memcpy(bBuf, g_bMErrCfg, sizeof(g_bMErrCfg));
		else if (wOI == OI_MTR_FLEW)
			memcpy(bBuf, g_bMFCfg, sizeof(g_bMFCfg));
		else if (wOI == OI_MTR_STOP)
			memcpy(bBuf, g_bMSCfg, sizeof(g_bMSCfg));
		else if (wOI == OI_MTR_RD_FAIL)
			memcpy(bBuf, g_bMRFCfg, sizeof(g_bMRFCfg));
		else if (wOI == OI_MTR_DATA_CHG)
			memcpy(bBuf, g_bDaCgCfg, sizeof(g_bDaCgCfg));
		else
			return;
		
		WriteItemEx(BN0, pOI->wPn, pOI->wID, bBuf);
		TrigerSaveBank(BN0, SECT3, -1);
	}
	
	return;
}



//���������³����¼���¼���ϱ�״̬
//������@dwCnOAD ͨ��OAD
//		@pEvtMsg�¼��ϱ���Ϣ
//		@bRptState Ҫ�õı�־λ����¼�е�ͨ���ϱ�״̬��ԭ��ֵ��������ֵ
//���أ������ȷ�򷵻�true,���򷵻�false
bool UpdateMtrExcRptState(DWORD dwCnOAD, TEvtMsg* pEvtMsg, BYTE bRptState)
{
	BYTE bAttr, bItem, bIndex, bType, bCnNum, i = 0;
	BYTE bOadBuf[10];
	BYTE bRecBuf[MTR_EXC_REC_LEN];
	BYTE* pbRec = bRecBuf;
	WORD wOI, wItemOffset, wItemLen;
	DWORD dwOAD, dwRecCnOAD;
	int iLen, nIndex = 0;
	TFieldParser tFixFields;	
	char* pszFileName = NULL;
	
	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	GetOIAttrIndex(pEvtMsg->dwOAD, &wOI, &bAttr, NULL);
	
	//��ȡ�¼����ƽṹ
	nIndex = GetMtrExcIndex(wOI);
	if (nIndex < 0)
		return false;	

	//ȡ���ϱ��¼��ļ�¼
	memset(bRecBuf, 0, sizeof(bRecBuf));
	iLen = GetEvtRec(pEvtMsg, bRecBuf, sizeof(bRecBuf), 1);
	if (iLen <= 0)
		return false;
	
	//��ȡ�̶��ֶ�
	if (GetMtrExcFieldParser(wOI, &tFixFields, NULL, NULL, 0) == false)
		return false;

	if (tFixFields.wNum == 0)
		return false;

	//����ˢ��ͨ���ϱ�״̬
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	//�̶��ֶθ���
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
			return false;

		if (bType != DT_OAD) 
			return false;

		if (wItemLen == 0) 
			return false;

		dwOAD = OoOadToDWord(bOadBuf+1);
		if (dwOAD == 0x33000200)	//ͨ���ϱ�״̬ˢ��
		{
			if (pbRec != DT_NULL)
			{
				bCnNum = *(pbRec+1);
				if (bCnNum >= CN_RPT_NUM)
					bCnNum = CN_RPT_NUM;

				for(i=0; i<bCnNum; i++)
				{	
					dwRecCnOAD = OoDoubleLongUnsignedToDWord(pbRec+5+i*9);
					if ((dwCnOAD&0xff000000) == 0x45000000)	//Ҫ�뺯��SendEvtMsg()��ƥ��
					{	dwRecCnOAD &=0xff000000;
						dwCnOAD &=0xff000000;
					}
					else
					{	
						dwRecCnOAD &=0xffff0000;
						dwCnOAD &=0xffff0000;
					}
					if (dwCnOAD == dwRecCnOAD)
						*(pbRec+10+i*9) |= bRptState;
				}
			}
		}

		pbRec += wItemLen;
	}

	pszFileName = GetEvtRecFileName(pEvtMsg->dwOAD&0xffff1f00);
	if (pszFileName == NULL)
		return false;

	//if (bRptState&0x0a)
	//	AddEvtOad(pEvtMsg->dwOAD, 1);	//���ϱ�

	//������¼
	SaveRecordByPhyIdx(pszFileName, pEvtMsg->wRecIdx, bRecBuf);
	return true;
}


//
int OoProRptMtrExcRecord(WORD wOI, BYTE bAttr, BYTE* pbRecBuf, WORD wRecLen, WORD wBufSize)
{
	BYTE bType, bOadBuf[10], bBuf[EVT_ATTRTAB_LEN];
	BYTE bTmpRecBuf[MTR_EXC_REC_LEN];	//һ����¼������
	BYTE* pbTmpRec = bTmpRecBuf;
	BYTE* pbRec = pbRecBuf;
	WORD wItemOffset, wItemLen, wTotalLen;
	DWORD dwOAD;
	int iLen;
	TFieldParser tFixFields;
	TFieldParser tDataFields;
	const ToaMap* pOaMap = NULL;
	BYTE bIndex, bStart = 0;

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));

	//��ȡ�̶��ֶκ������ֶ�
	if (GetMtrExcFieldParser(wOI, &tFixFields, &tDataFields, bBuf, sizeof(bBuf)) == false)
	{
		DTRACE(DB_INMTR, ("OoProRptMtrExcRecord: wOI=%u GetEvtFieldParser() fail.\r\n", wOI));
		return -1;
	}

	wTotalLen = 0;

	// 4���ֽڵ��¼���¼OAD
	OoDWordToOad(GetOAD(wOI, bAttr, 0), pbTmpRec);
	pbTmpRec += 4;
	wTotalLen+= 4;

	// 1�ֽ�Ԫ�ظ���
	*pbTmpRec++ = tFixFields.wNum+tDataFields.wNum;
	wTotalLen++;

	// 5�ֽ�ÿ��Ԫ������OAD*Ԫ�ظ���
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)
	{
		//ѡ��OAD����
		*pbTmpRec++ = 0;wTotalLen++;
		//OAD
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;	//ֱ�ӷ��أ��̶��ֶβ�Ӧ��������
		}

		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;
		}

		memcpy(pbTmpRec, bOadBuf+1, 4);
		pbTmpRec += 4;
		wTotalLen += 4;
	}

	for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	
	{
		//ѡ��OAD����
		*pbTmpRec++ = 0;wTotalLen++;
		//OAD
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;	//ֱ�ӷ��أ��̶��ֶβ�Ӧ��������
		}

		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;
		}

		memcpy(pbTmpRec, bOadBuf+1, 4);
		pbTmpRec += 4;
		wTotalLen += 4;
	}

	// 1�ֽڽ��
	*pbTmpRec++ = 1;
	wTotalLen++;
	// 1�ֽڽ��������1��
	*pbTmpRec++ = 1;
	wTotalLen++;

	//��������	
	//����̶��ֶε��¼�����Դ���ϱ���Ϣ���¼������б����������
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;	//ֱ�ӷ��أ��̶��ֶβ�Ӧ��������
		}

		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;
		}

		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);
		
		//��������
		iLen = GetMtrExcEvtSpecField(dwOAD, pbRec, wItemLen, &bStart);
		if (iLen>0 && iLen<=wItemLen)
			memcpy(pbTmpRec, pbRec+bStart, iLen);
		else
			return -1;

		pbRec += wItemLen;	 
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}

	//���������ֶ�
	for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;	//ֱ�ӷ��أ��̶��ֶβ�Ӧ��������
		}

		if (bType!=DT_OAD || wItemLen==0)
		{	
			//pEvtCtrl->pEvtBase[0].fInitOk = false;	//��Ҫ���³�ʼ��
			return -1;
		}		
		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);

		//��������
		iLen = GetMtrExcEvtSpecField(dwOAD, pbRec, wItemLen, &bStart);
		if (iLen>0 && iLen<=wItemLen)
			memcpy(pbTmpRec, pbRec+bStart, iLen);
		else
			return -1;
		
		pbRec += wItemLen;	
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}

	//memcpy(pbTmpRec, pbRec, tDataFields.wTotalLen);
	//wTotalLen +=  tDataFields.wTotalLen;
	if (wTotalLen <= wBufSize)
	{	
		memcpy(pbRecBuf, bTmpRecBuf, wTotalLen);
		//TrigerSaveBank(BN0, SECT3, -1);
		return wTotalLen;
	}

	return -1;
}

