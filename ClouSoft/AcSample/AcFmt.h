/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcFmt.h
 * 摘    要：本文件主要实现交采数据项的值到格式的转换接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2008年5月
 * 备    注: 本文件主要用来屏蔽各版本间参数的差异性
 *********************************************************************************************************/
#ifndef ACFMT_H
#define ACFMT_H
#include "apptypedef.h"

bool InitAcValToDb(WORD wPn);
void AcValToDb(int* piVal);
void AcHarmonicToDb(WORD* pwHarPercent, WORD* pwHarVal);
bool InitPulseValToDb(BYTE bPnIndex);
void PulseValToDb(BYTE bPnIndex, int* piVal);

//交采电能数据格式的转换
void VoltValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
void CurrValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
void PowerValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
void CosValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
void AngValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
void FValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
/*
void AcAngToFmt(int val, BYTE* pbBuf, WORD wLen);
WORD AcEpToFmt(int64 val, BYTE* pbBuf);
WORD AcFmtToEp(BYTE* pbBuf, int64* piVal);
WORD AcEqToFmt(int64 val, BYTE* pbBuf);
WORD AcFmtToEq(BYTE* pbBuf, int64* piVal);
*/
WORD AcEpToFmt(int64 val, BYTE* pbBuf, bool fHigPre, bool fSign);
WORD AcFmtToEp(BYTE* pbBuf, int64* piVal, bool fHigPre, bool fSign);
BYTE AcEngToFmt(WORD wID, int64 *pi64E, BYTE* pbBuf, bool fHigPre, bool fSign, WORD wRateNum);
BYTE AcFmtToEng(WORD wID, int64 *pi64E, BYTE* pbBuf, bool fHigPre, bool fSign, BYTE bSign, WORD wRateNum);
void AcEngMonitor(BYTE* pSrc,WORD wRateNum, WORD wID, 
					int64 *pi64E, int64 *pm_i64E, BYTE *pbTemp,
					int64 i64MaxE,	//电能数据入库能表示的最大值
					int iMaxLoss,	//保存间隔内最大丢失的电能
					bool fSign, BYTE bSign);

//WORD AcFmtToDemand(BYTE* pbBuf, DWORD* pdwDemand);
//WORD AcDemandAndTimeToFmt(BYTE* pSrcBuf, BYTE* pDstBuf);
bool IsAcEngSign(WORD wOI);
bool IsDemFmt5(WORD wOI);
WORD AcFmtToDemand(BYTE* pbBuf, DWORD* pdwDemand, BYTE* pbTime);
WORD AcFmtToDemandAndTime(BYTE* pbBuf, DWORD* pdwDemand, BYTE* pbTime,WORD wRateNum);
WORD AcDemandToFmt(DWORD *pdwDemand, BYTE* pbTime, BYTE* pbBuf, WORD wDemandID, WORD wRateNum);
WORD AcFmtToDemandTime(BYTE* pbBuf, BYTE* pbTime,WORD wRateNum,BYTE bDemTimeLen);
bool AcFmtToDemandTime(BYTE* pbTime, TTime *pTime );
WORD AcCurDemandToFmt(DWORD dwDemand, BYTE* pbBuf, WORD wDemandID);
WORD AcIDToRatenum(WORD wID, WORD wRateNum);
WORD AcDemandTimeToFmt(BYTE* pSrcBuf, BYTE* pDstBuf, WORD wRateNum, BYTE bDemTimeLen);
void AcTTimeToDemandTime(BYTE* pbTime, TTime *pTime );
BYTE AcDemandTimeGetMonth(BYTE* pbTime);

BYTE PulseHiFmtToLoEng(WORD wID, int64 *pi64E, BYTE* pbBuf, WORD wRateNum);
BYTE PulseFmtToEng(WORD wID, int64 *pi64E, BYTE* pbBuf, WORD wRateNum);		//从数据库读出脉冲电量
BYTE PulseEngToFmt(WORD wID, int64 *pi64E, BYTE* pbBuf, WORD wRateNum);		//脉冲电量入库

#endif //ACFMT_H
