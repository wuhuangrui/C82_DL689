/*********************************************************************************************************
 * Copyright (c) 2008,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�AcFmt.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֽ����������ֵ����ʽ��ת���ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2008��5��
 * ��    ע: ���ļ���Ҫ�������θ��汾������Ĳ�����
 *********************************************************************************************************/
#ifndef ACFMT_H
#define ACFMT_H
#include "apptypedef.h"

bool InitAcValToDb(WORD wPn);
void AcValToDb(int* piVal);
void AcHarmonicToDb(WORD* pwHarPercent, WORD* pwHarVal);
bool InitPulseValToDb(BYTE bPnIndex);
void PulseValToDb(BYTE bPnIndex, int* piVal);

//���ɵ������ݸ�ʽ��ת��
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
					int64 i64MaxE,	//������������ܱ�ʾ�����ֵ
					int iMaxLoss,	//�����������ʧ�ĵ���
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
BYTE PulseFmtToEng(WORD wID, int64 *pi64E, BYTE* pbBuf, WORD wRateNum);		//�����ݿ�����������
BYTE PulseEngToFmt(WORD wID, int64 *pi64E, BYTE* pbBuf, WORD wRateNum);		//����������

#endif //ACFMT_H
