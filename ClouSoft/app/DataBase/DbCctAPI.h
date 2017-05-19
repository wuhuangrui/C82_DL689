/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbCctAPI.h
 * 摘    要：本文件主要实现协议中集抄相关的数据库标准接口之外的扩展接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年4月
 * 备    注：$为了避免集中器的系统库接口函数跟DbAPI.cpp中的接口函数混杂在一起
 			  特意定义本文件
 *********************************************************************************************************/
#ifndef DBCCTAPI_H
#define DBCCTAPI_H
#include "DbConst.h"

//////////////////////////////////////////////////////////////////////////
//集中器接口
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
