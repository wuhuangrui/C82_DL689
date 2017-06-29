/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbOIAPI.h
 * 摘    要：本文件主要实现系统数据库和面向对象ID之间的转换
 * 当前版本：1.0
 * 作    者：孔成波
 * 完成日期：2016年8月
 *********************************************************************************************************/
#ifndef DBOIAPI_H
#define DBOIAPI_H
#include "DbStruct.h"
#include "OIObjInfo.h"
#include "Comm.h"

BYTE OIGetStrLen(BYTE* pbStr, BYTE bLen, BYTE bFill);
int OoReadAttr(WORD wOI, BYTE bAttr, BYTE* pbBuf, BYTE** ppFmt, WORD* pwFmtLen);
int OoProReadAttr(WORD wOI, BYTE bAtrr, BYTE bIndex, BYTE* pbBuf, WORD wBufSize, int* piStart);
int OoProWriteAttr(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbBuf, WORD wLen, bool fIsSecurityLayer);
int OoGetDataTypeFmtValidLen(BYTE *pFmt, WORD wFmtLen, WORD *pwRetFmtLen);
int OoGetDataTypeLen(BYTE* pbSrc);
int ScanArray(BYTE* pbSrc, bool fRevOrder);
int ScanMS(BYTE* pbSrc, bool fRevOrder);
int ScanRSD(BYTE* pbSrc, bool fRevOrder);
int ScanCSD(BYTE* pbSrc, bool fRevOrder);
int ScanROAD(BYTE* pbSrc, bool fRevOrder);
int ScanRCSD(BYTE* pbSrc, bool fRevOrder);
int OIFmtData(BYTE* pbSrc, BYTE* pbDst, BYTE* pbFmt, WORD wFmtLen);
int OIUnFmtData(BYTE* pbSrc, BYTE* pbDst, BYTE* pbFmt, WORD wFmtLen);
int OoScanData(BYTE* pbSrc, BYTE* pbFmt, WORD wFmtLen, bool fRevOrder, int iIndex, WORD* pwLen, BYTE* pbType, BYTE** ppFieldFmt=NULL, WORD* pwFieldFmtLen=NULL);

int OoDataFieldScan(BYTE* pbSrc, BYTE* pbFmt, WORD wFmtLen);
BYTE* OoGetField(BYTE* pbData, BYTE* pbFmt, WORD wFmtLen, WORD wIndex, WORD* pwLen, BYTE* pbType, BYTE** ppFieldFmt=NULL, WORD* pwFieldFmtLen=NULL);
int OoWriteField(BYTE* pbSrc, WORD wSrcLen, BYTE* pFmt, WORD wFmtLen, WORD wIndex, BYTE* pbField, WORD wFieldLen);
int OoParseVal(BYTE* pbSrc, BYTE* pFmt, WORD wFmtLen, int* piVal);
int OoReadVal(DWORD dwOAD, int* piVals, WORD wValMax);
int OoGetDataVal(BYTE* pbData, int* piVal);
int OoValToFmt(BYTE bValType, int iVal, BYTE* pbData);
bool OoGetValType (DWORD dwOAD, BYTE* pbValType);
bool OoParseField2(TFieldParser* pParser, BYTE* pFmt, WORD wFmtLen, bool fParseItem);
int ReadParserField(TFieldParser* pParser, WORD wIndex, BYTE* pbBuf, BYTE* pbType, WORD*  pwItemOffset, WORD* pwItemLen);
int OoReadField(BYTE* pbSrc, BYTE* pbFmt, WORD wFmtLen, WORD wIndex, BYTE* pbField, BYTE* pbType);
int OoGetDataLen(BYTE bType, BYTE* pItemDesc);
int OoScanFmt(BYTE *pbBuf, BYTE *pbFmtBuf, WORD &wFmtLen);
int OoReadAttr2(WORD wOI, BYTE bAtrr, BYTE* pbBuf, BYTE** ppFmt, WORD* pwFmtLen);
int OoScanRcsdInOadNum(BYTE *pbRcsd);
int OoWriteAttr(WORD wOI, BYTE bAtrr, BYTE* pbBuf);

int DoObjMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int* piParaLen, BYTE* pvAddon, BYTE* pbRes);
DWORD GetOAD(WORD wOI, BYTE bAtrr, BYTE bIndex);
int OoGetDataVal(BYTE* pbData, int* piVal);
int OoReadOAD(DWORD dwOAD, BYTE* pbBuf, BYTE** ppFmt, WORD* pwFmtLen);

bool IsNeedRdSpec(const ToaMap* pOI);
int OIRead_Spec(ToaMap* pOI, BYTE* pbBuf, WORD wBufSize, int* piStart);
int OIFmtDataExt(BYTE* pbSrc, BYTE bsLen, BYTE* pbDst, BYTE* pbFmt, WORD wFmtLen, DWORD dwOAD);
bool OIRead_PortPara(WORD wID, BYTE bPn, TCommPara *pCommPara, BYTE *bFunc);

#endif

