/****************************************************************************************************
* Copyright (c) 2008,深圳科陆电子科技股份有限公司
* All rights reserved.
* 
* 文件名称：DataProcess.h
* 摘    要: 本文件提供一类数据及中间数据的实现
* 当前版本：1.0
* 作    者：潘香玲
* 完成日期：2008年3月
* 备    注：
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

//差值运算：需考虑满度复零的问题
//返回值：1 运算OK； 0 运算因子有无效数据； -1 运算为小值－大值，不正确
int CDataProc::DataDelta(int64* iResVal, int64 iSrcVal1, int64 iSrcVal2)
{	
	int64 iabsVal1, iabsVal2;

	//考虑到费率不齐的时候
	if (iSrcVal1==INVALID_VAL64 || iSrcVal2==INVALID_VAL64)
	{
		*iResVal = INVALID_VAL64;
		return 0;
	}

	if (iSrcVal1<0 && iSrcVal2<0) //总加合成走反向的时候取绝对值
	{
		iabsVal1 = -iSrcVal1;
		iabsVal2 = -iSrcVal2;
	}
	else //总加合成走正向的时候（原则上不会出现总加合成由正走到负或由负走到正的情况）
	{
		iabsVal1 = iSrcVal1;
		iabsVal2 = iSrcVal2;
	}

	//考虑数据复零
	if (iabsVal2 > iabsVal1)
	{		
		//*iResVal = iSrcVal2; //目前还不知道iSrcVal1的最大值能到多少
		//DTRACE(DB_DP, ("DataDelta********* sub is error: pn = %d iSrcVal2=%lld,iSrcVal1=%lld,iResVal=%lld\r\n", m_bPn, iSrcVal2, iSrcVal1, *iResVal));
		//0xFFFFFFFFFFFFFFFF-iSrcVal2+1+iSrcVal1;
		return -1;
	}
	else //减运算
	{
		*iResVal = iSrcVal1 - iSrcVal2;
	}
	
	return 1;
}

//描述：代数和运算，相加或相减
bool CDataProc::DataSum(int64* iResVal, int64 iSrcVal1, int64 iSrcVal2, BYTE bFlag)
{	
	//考虑到费率不齐的时候
	if (iSrcVal1==INVALID_VAL64 || iSrcVal2==INVALID_VAL64 )							
	{
		*iResVal = INVALID_VAL64;
		return false;
	}

	if (bFlag == 0) //加运算
	{
		*iResVal = iSrcVal1 + iSrcVal2;
	}
	else	//减运算
	{
		*iResVal = iSrcVal1 - iSrcVal2;
	}

	return true;
}

//描述：某时刻对应的间隔的开始时间及间隔结束时间的秒数
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

//描述：清零电能块数据
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
		WriteItemEx(wBank, m_wPn, wID, bBuf, dwSec); //清零
	}
	else
	{
		WriteItemVal64(wBank, m_wPn, wID, piValBuf, dwSec); //清零	
	}
}
