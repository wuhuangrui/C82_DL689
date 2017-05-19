/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbCctAPI.h
 * ժ    Ҫ�����ļ���Ҫʵ��Э���м�����ص����ݿ��׼�ӿ�֮�����չ�ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��4��
 * ��    ע��$Ϊ�˱��⼯������ϵͳ��ӿں�����DbAPI.cpp�еĽӿں���������һ��
 			  ���ⶨ�屾�ļ�
 *********************************************************************************************************/
#ifndef DBCCTAPI_H
#define DBCCTAPI_H
#include "DbConst.h"

//////////////////////////////////////////////////////////////////////////
//�������ӿ�
BYTE CctGetPnProp(WORD wPn);
bool IsCctPn(WORD wPn);
bool IsRJ45Pn(WORD wPn);
bool IsBBCctPn(WORD wPn);
bool CctIsPnValid(WORD wPn);
void CctUpdPnMask();
WORD SearchPnFromMask(const BYTE* pbPnMask, WORD wStartPn);
WORD SchLastPnFromMask(const BYTE* pbPnMask);
bool IsVIPPara(BYTE bCmd, BYTE bSample);

int AddVIP(WORD wPn, BYTE* pbBuf);
void DelVIP(BYTE* pbAddr);
int SearchVIPAddr(WORD wPn);
int ReadCctMeterInf(WORD wPn, BYTE* pbBuf);
void UpDateSelReadInf();

void CctPostDbInit();
void UpdateVipMask();

bool IsSinglePhase(WORD wPn);
bool IsMultiFunc(WORD wPn);
bool IsMultiRate(WORD wPn);

bool IsCctClass1MtrId(WORD wPn, WORD wID);
bool IsCctDataBankId(WORD wBn, WORD wID);
WORD CctGetPnBank(WORD wPn);
WORD CctGetIdBank(WORD wBn, WORD wPn, WORD wID);
void CctClrPnData(WORD wPn);
void CctVipPnData(WORD wPn);
void InitPnProp();
#endif //DBCCTAPI_H
