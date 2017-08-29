/****************************************************************************************************
* Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
* 
* �ļ����ƣ�DataProcess.h
* ժ    Ҫ: ���ļ��ṩһ�����ݼ��м����ݵ�ʵ��
* ��ǰ�汾��1.0
* ��    �ߣ�������
* ������ڣ�2008��3��
* ��    ע��
****************************************************************************************************/
#include "stdafx.h"
#include "DataProc.h"
#include "FaStruct.h"
#include "DbConst.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include <math.h>

CDataProc::CDataProc()
{
	m_wPn = 0;         	
	m_bPnProp = 0;    	
	m_bMtrIntv = 0;
	m_bRateNum = RATE_NUM;

}

CDataProc::~CDataProc(void)
{

}

//��ֵ���㣺�迼�����ȸ��������
//����ֵ��1 ����OK�� 0 ������������Ч���ݣ� -1 ����ΪСֵ����ֵ������ȷ
int CDataProc::DataDelta(int64* iResVal, int64 iSrcVal1, int64 iSrcVal2)
{	
	int64 iabsVal1, iabsVal2;

	//���ǵ����ʲ����ʱ��
	if (iSrcVal1==INVALID_VAL64 || iSrcVal2==INVALID_VAL64)
	{
		*iResVal = INVALID_VAL64;
		return 0;
	}

	if (iSrcVal1<0 && iSrcVal2<0) //�ܼӺϳ��߷����ʱ��ȡ����ֵ
	{
		iabsVal1 = -iSrcVal1;
		iabsVal2 = -iSrcVal2;
	}
	else //�ܼӺϳ��������ʱ��ԭ���ϲ�������ܼӺϳ������ߵ������ɸ��ߵ����������
	{
		iabsVal1 = iSrcVal1;
		iabsVal2 = iSrcVal2;
	}

	//�������ݸ���
	if (iabsVal2 > iabsVal1)
	{		
		//*iResVal = iSrcVal2; //Ŀǰ����֪��iSrcVal1�����ֵ�ܵ�����
		//DTRACE(DB_DP, ("DataDelta********* sub is error: pn = %d iSrcVal2=%lld,iSrcVal1=%lld,iResVal=%lld\r\n", m_bPn, iSrcVal2, iSrcVal1, *iResVal));
		//0xFFFFFFFFFFFFFFFF-iSrcVal2+1+iSrcVal1;
		return -1;
	}
	else //������
	{
		*iResVal = iSrcVal1 - iSrcVal2;
	}
	
	return 1;
}

//���������������㣬��ӻ����
bool CDataProc::DataSum(int64* iResVal, int64 iSrcVal1, int64 iSrcVal2, BYTE bFlag)
{	
	//���ǵ����ʲ����ʱ��
	if (iSrcVal1==INVALID_VAL64 || iSrcVal2==INVALID_VAL64 )							
	{
		*iResVal = INVALID_VAL64;
		return false;
	}

	if (bFlag == 0) //������
	{
		*iResVal = iSrcVal1 + iSrcVal2;
	}
	else	//������
	{
		*iResVal = iSrcVal1 - iSrcVal2;
	}

	return true;
}

//������ĳʱ�̶�Ӧ�ļ���Ŀ�ʼʱ�估�������ʱ�������
void CDataProc::TimeToIntvS(TTime tmVal, BYTE bInterv, TIntvSec* pIntervS)
{ 
	BYTE bIntvType	= 0;
	BYTE bIntvT		= 0;
	switch (bInterv)
	{
		case INTVCAL:
			bIntvType = TIME_UNIT_MINUTE;
			bIntvT = m_bMtrIntv;
			break;
		case INTV15M:
			bIntvType = TIME_UNIT_MINUTE;
			bIntvT = 15;
			break;
		case INTVDAY:
			bIntvType = TIME_UNIT_DAY;
			bIntvT = m_bMtrIntv;
			break;
		case INTVMON:
			bIntvType = TIME_UNIT_MONTH;
			bIntvT = m_bMtrIntv;
			break;
		default:
			return;
			break;
	}
	TimeToIntervS(tmVal, bIntvType, pIntervS->dwS, pIntervS->dwEndS, bIntvT);
}

//������������ܿ�����
void CDataProc::ClsBlockE(WORD wBank, WORD wID, int64* piValBuf, int64 iDstVal, DWORD dwSec)
{
	SetArrVal64(piValBuf, iDstVal, TOTAL_RATE_NUM);

	if (wID>=0x2306 && wID<=0x2309)
	{
		BYTE bBuf[64];
		bBuf[0] = DT_ARRAY;
		bBuf[1] = TOTAL_RATE_NUM;
		for (int j=0; j<BLOCK_ITEMNUM;j++)
		{
			bBuf[2+9*j] = DT_LONG64;
			OoInt64ToLong64(piValBuf[j], &bBuf[3+9*j]);
		}
		WriteItemEx(wBank, m_wPn, wID, bBuf, dwSec); //����
	}
	else
	{
		WriteItemVal64(wBank, m_wPn, wID, piValBuf, dwSec); //����	
	}
}
