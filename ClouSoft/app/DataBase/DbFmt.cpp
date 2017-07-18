/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbFmt.cpp
 * 摘    要：本文件主要实现数据库中数据项的值到格式、格式到值的转换接口
 * 当前版本：1.0
 * 作    者：岑坚宇、杨凡
 * 完成日期：2008年3月
 *********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "FaAPI.h"
#include "sysfs.h"
#include "DbAPI.h"
#include "DbOIAPI.h"
#include "OIObjInfo.h"

//描述：按照bFmt格式求pbBuf的64位有符号整形,只求一个数据项,不支持块
//参数：@pbBuf 存放数据项内容的缓冲区
//      @wLen 数据项长度,对于定长的数据项(比如国标规定的23种格式)填0即可,
//			  对于变长的数据项,比如DI_FMT_BCD,DI_FMT_HEX等,需要填长度
//返回：如果读到且正确求值,返回数据项的值,如果错误则返回INVALID_VAL或INVALID_VAL64

int64 Fmt2ToVal64(BYTE* pbBuf, WORD wLen)
{
	int64 iVal = 0;
	BYTE bExp;

	iVal = BcdToDWORD(pbBuf, wLen-1);
	pbBuf += wLen-1;

	iVal += (*pbBuf & 0x0f) * Pow(100, wLen-1);

	bExp = 7 - ((*pbBuf >> 5) & 0x07);

	iVal *= Pow(10, bExp);

	if (*pbBuf & 0x10)
		iVal = -iVal;

	return iVal;
}

int Fmt2ToVal(BYTE* pbBuf, WORD wLen)
{
	return (int )Fmt2ToVal64(pbBuf, wLen);
}


int64 Fmt3ToVal64(BYTE* pbBuf, WORD wLen)
{	
	int64 iVal = BcdToDWORD(pbBuf, 3);
	pbBuf += 3;

	wLen;  //编译占位语句,消除编译告警   added by whr 20170718
 
	iVal += (*pbBuf & 0x0f) * 1000000;
	
	if (*pbBuf & 0x40)
		iVal *= 1000;
		
	if (*pbBuf & 0x10)
		iVal = -iVal;
	
	return iVal;
}

int Fmt3ToVal(BYTE* pbBuf, WORD wLen)
{
	return (int )Fmt3ToVal64(pbBuf, wLen);
}

int Fmt4ToVal(BYTE *pbBuf, WORD wLen)
{
	int iVal = BcdToByte(*pbBuf & 0x7f);

	wLen;  //编译占位语句,消除编译告警   added by whr 20170718
	
	if (*pbBuf & 0x80)
		iVal = -iVal;

	return iVal;
}

int64 Fmt4ToVal64(BYTE *pbBuf, WORD wLen)
{
	return Fmt4ToVal(pbBuf, wLen);
}

int Fmt5ToVal(BYTE *pbBuf, WORD wLen)
{
	int iVal = BcdToByte(*pbBuf);
	pbBuf++;
	wLen;  //编译占位语句,消除编译告警   added by whr 20170718
	
	iVal += (int )BcdToByte(*pbBuf & 0x7f) * 100;
	
	if (*pbBuf & 0x80)
		iVal = -iVal;

	return iVal;
}

int64 Fmt5ToVal64(BYTE *pbBuf, WORD wLen)
{
	return Fmt5ToVal(pbBuf, wLen);
}

int Fmt6ToVal(BYTE *pbBuf, WORD wLen)
{
	int iVal = BcdToByte(*pbBuf);
	wLen;
	pbBuf++;
	
	iVal += (int )BcdToByte(*pbBuf & 0x7f) * 100;
	
	if (*pbBuf & 0x80)
		iVal = -iVal;

	return iVal;
}


int64 Fmt6ToVal64(BYTE *pbBuf, WORD wLen)
{
	return Fmt6ToVal(pbBuf, wLen);
}
int Fmt25ToVal(BYTE *pbBuf, WORD wLen)
{
	wLen;
	int iVal = BcdToByte(*pbBuf);
	pbBuf++;
	iVal += (int )BcdToByte(*pbBuf) * 100;
	pbBuf++;
	iVal += (int )BcdToByte(*pbBuf & 0x7f) * 10000;
	
	if (*pbBuf & 0x80)
		iVal = -iVal;

	return iVal;
}


int64 Fmt25ToVal64(BYTE *pbBuf, WORD wLen)
{
	return Fmt25ToVal(pbBuf, wLen);
}
int Fmt7ToVal(BYTE *pbBuf, WORD wLen)
{
	wLen;  //编译占位语句,消除编译告警   added by whr 20170718
	return BcdToDDWORD(pbBuf, 2);
}


int64 Fmt7ToVal64(BYTE *pbBuf, WORD wLen)
{
	wLen; //编译占位语句,消除编译告警   added by whr 20170718
	return BcdToDDWORD(pbBuf, 2);
}

int Fmt8ToVal(BYTE *pbBuf,WORD wLen)
{
	wLen;  //编译占位语句,消除编译告警   added by whr 20170718
	return BcdToDDWORD(pbBuf, 2);
}


int64 Fmt8ToVal64(BYTE *pbBuf,WORD wLen)
{
	wLen;  //编译占位语句,消除编译告警   added by whr 20170718
	return BcdToDDWORD(pbBuf, 2);
}

int Fmt9ToVal(BYTE* pbBuf, WORD wLen)
{
	int iVal = BcdToDDWORD(pbBuf, 2);
	pbBuf += 2;

	 wLen; //编译占位语句,消除编译告警   added by whr 20170718
	
	iVal += (int )BcdToByte(*pbBuf & 0x7f) * 10000;
	
	if (*pbBuf & 0x80)
		iVal = -iVal;

	return iVal;
}

int64 Fmt9ToVal64(BYTE* pbBuf, WORD wLen)
{
	wLen;  //编译占位语句,消除编译告警   added by whr 20170718
	return Fmt9ToVal(pbBuf, wLen);	
}

int Fmt10ToVal(BYTE* pbBuf, WORD wLen)
{
	wLen;  //编译占位语句,消除编译告警   added by whr 20170718
	return BcdToDDWORD(pbBuf, 3);
}

int64 Fmt10ToVal64(BYTE* pbBuf, WORD wLen)
{
	wLen;  //编译占位语句,消除编译告警   added by whr 20170718
	return BcdToDDWORD(pbBuf, 3);
}

int Fmt11ToVal(BYTE* pbBuf, WORD wLen)
{
	return BcdToDWORD(pbBuf, 4); 
}

int64 Fmt11ToVal64(BYTE* pbBuf, WORD wLen)
{
	return BcdToDWORD(pbBuf, 4); 
}

int Fmt12ToVal(BYTE* pbBuf, WORD wLen)
{
	return (int )BcdToDDWORD(pbBuf, 6);
}

int64 Fmt12ToVal64(BYTE* pbBuf, WORD wLen)
{
	return BcdToDDWORD(pbBuf, 6);
}

int Fmt13ToVal(BYTE* pbBuf, WORD wLen)
{
	return BcdToDWORD(pbBuf, 4);
}

int64 Fmt13ToVal64(BYTE* pbBuf, WORD wLen)
{
	return BcdToDWORD(pbBuf, 4);
}

int Fmt14ToVal(BYTE* pbBuf, WORD wLen)
{
	return (int )BcdToDDWORD(pbBuf, 5);
}

int64 Fmt14ToVal64(BYTE* pbBuf, WORD wLen)
{
	return BcdToDDWORD(pbBuf, 5);
}


int Fmt22ToVal(BYTE* pbBuf,WORD wLen)
{
	return BcdToByte(*pbBuf);
}

int64 Fmt22ToVal64(BYTE* pbBuf, WORD wLen)
{
	return BcdToByte(*pbBuf);
}

int Fmt23ToVal(BYTE* pbBuf, WORD wLen)
{
	return BcdToDWORD(pbBuf, 3);
}

int64 Fmt23ToVal64(BYTE* pbBuf, WORD wLen)
{
	return BcdToDWORD(pbBuf, 3);
}

int BinToVal32(BYTE* pbBuf, WORD  wLen)
{
	int val = 0;
	memcpy(&val, pbBuf, wLen);
	return val;
}

DWORD OIBinToVal(BYTE* pbBuf, WORD  wLen)
{
	DWORD dwVal;
	BYTE bBuf[12];
	revcpy(bBuf, pbBuf, wLen);
	memcpy(&dwVal, pbBuf, wLen);
	return dwVal;
}

int64 BinToVal64(BYTE* pbBuf, WORD wLen)
{
	int64 val = 0;
	memcpy(&val, pbBuf, wLen);
	return val;
}

int UnknownFmtToVal(BYTE* pbBuf, WORD wLen)
{
	return  INVALID_VAL;
}

int64 UnknownFmtToVal64(BYTE* pbBuf, WORD wLen)
{
	return INVALID_VAL64;
}

int ValCopy(BYTE* pbIn, WORD wLen)
{
	//assert(wLen < 5 && wLen > 0);
	int  val = 0;
	BYTE  *pbVal = (BYTE *)&val;

	while (wLen--)
	{
		*pbVal++ = *pbIn++;
	}
	return  val;
}

int64 ValCopy64(BYTE* pbIn, WORD wLen)
{
	//assert(wLen < 9 && wLen > 0);
	int64  val =0;
	BYTE   *pbVal = (BYTE *)&val;

	while (wLen--)
	{
		*pbVal++ = *pbIn++;
	}
	return  val;
}

//以下是值到格式的转换
void ValToBCD(int val, BYTE* bcd, WORD len)
{
	bool fNeg = false;
	if (val < 0)
	{
		val = - val;
		fNeg = true;
	}

	int power = 1;
	for (WORD i=0; i<len-1; i++)
	{
		power *= 100; 
	}

	power *= 10;  
	BYTE  tmp = val / power;      //最高字节含两个BCD位
	val %= power;
	power /= 10;

	if (fNeg)
		bcd[len - 1] = 0x80 |((val / power) | (tmp <<4));        
	else
		bcd[len - 1] = (val / power) | (tmp <<4);

	len--;

	for (; len>0; len--)
	{
		BYTE bHigh, bLow;
		val %= power;
		power /= 10;
		bHigh = val / power;

		val %= power;
		power /= 10;
		bLow = val / power;
		bcd[len - 1] = (bHigh << 4) | bLow;
	}
}

void OIValToBin(DWORD dwVal, BYTE* pbBuf, WORD wLen)
{
	BYTE bBuf[12];
	memcpy(bBuf, &dwVal, wLen);
	revcpy(pbBuf, bBuf, wLen);
}

//以下是值到格式的转换
void Val64ToBCD(int64 val, BYTE* bcd, WORD len)
{
	bool fNeg = false;
	if (val < 0)
	{
		val = - val;
		fNeg = true;
	}

	int power = 1;
	for (WORD i=0; i<len-1; i++)
	{
		power *= 100; 
	}

	power *= 10;  
	int64  tmp = val / power;      //最高字节含两个BCD位
	val %= power;
	power /= 10;

	if (fNeg)
		bcd[len - 1] = 0x80 |((val / power) | (tmp <<4));        
	else
		bcd[len - 1] = (val / power) | (tmp <<4);

	len--;

	for (; len>0; len--)
	{
		BYTE bHigh, bLow;
		val %= power;
		power /= 10;
		bHigh = val / power;

		val %= power;
		power /= 10;
		bLow = val / power;
		bcd[len - 1] = (bHigh << 4) | bLow;
	}
}

void ValToFmt2(int val, BYTE* pbBuf, WORD  wLen)
{
	BYTE  bRound = FMT_ROUND;

	if (val == 0)
	{
		memset(pbBuf, 0, wLen);
		pbBuf[wLen - 1] |= 0x80;
		return;
	}

	int64  iBase = Pow(10, wLen*2-1);
	bool fNeg = false;
	if (val < 0)
	{		
		fNeg = true;
		val = -val;
	}

	int  tmp;
	WORD wPow = 0;
	while (val >= iBase) 
	{
		tmp = val / 10;
		if (tmp<iBase && bRound==FMT_ROUND) //要求对数据进行四舍五入
		{
			tmp = (val + 5) / 10;
			bRound = FMT_NROUND;
		}

		val = tmp;
		wPow++;
	}

	DWORDToBCD((DWORD )val, pbBuf, wLen);
	wPow = 7 - wPow;
	pbBuf += wLen - 1;
	*pbBuf |= (wPow << 5);
	if (fNeg)
		*pbBuf |= 0x10;
}

void ValToFmt3(int val, BYTE* pbBuf, WORD wLen)
{
	if (val == 0)
	{
		memset(pbBuf, 0, wLen);
		return;
	}

	bool fNeg = false;
	if (val < 0)
	{		
		fNeg = true;
		val = -val;
	}

	bool fMillion = false;
	while (val > 9999999)      //999999999
	{
		val /= 1000;
		if (fMillion == false)
			fMillion = true;
	}

	DWORDToBCD((DWORD )val, pbBuf, wLen);
	BYTE bFlag = 0;
	if (fNeg)
		bFlag = 0x10;

	if (fMillion)
		bFlag |= 0x40;

	pbBuf += wLen - 1;
	*pbBuf = (*pbBuf & 0x0f) | bFlag;
}

void ValToFmt4(int val, BYTE* pbBuf, WORD wLen)
{
	bool  fNeg = false;

	if (val > 99 || val < -99)
		return;
	if (val < 0)
	{
		val = -val;
		fNeg = true;
	}
	*pbBuf = (BYTE)(val%10) | ((BYTE)(val/10)<<4);
	if (fNeg)
		*pbBuf |= 0x80;
}

void ValToFmt5(int val, BYTE* pbBuf, WORD wLen)
{
	bool  fNeg = false;

	if (val > 9999)
		val = 9999;
	else if (val < -9999)
		val = -9999;
		
	if (val < 0)
	{
		val = -val;
		fNeg = true;
	}
	
	DWORDToBCD((DWORD )val, pbBuf, 2);
	
	if (fNeg)
		pbBuf[1] |= 0x80;
}

void ValToFmt6(int val, BYTE* pbBuf, WORD wLen)
{
	bool  fNeg = false;

	if (val > 9999)
		val = 9999;
	else if (val < -9999)
		val = -9999;
		
	if (val < 0)
	{
		val = -val;
		fNeg = true;
	}
	
	DWORDToBCD((DWORD )val, pbBuf, 2);
	
	if (fNeg)
		pbBuf[1] |= 0x80;
}


void ValToFmt7(int val, BYTE* pbBuf, WORD wLen)
{
	if (val < 0) 
		val = 0;
	else if (val > 9999)
		val = 9999;
		
	DWORDToBCD((DWORD )val, pbBuf, 2);
}


void ValToFmt8(int val, BYTE* pbBuf, WORD wLen)
{
	ValToFmt7(val, pbBuf, wLen);
}

void ValToFmt9(int val, BYTE* pbBuf, WORD wLen)
{
	bool fNeg = false;

	if (val > 999999)
		val = 999999;
	else if (val < -999999)
		val = -999999;
		
	if (val < 0)
	{
		val = -val;
		fNeg = true;
	}
	
	DWORDToBCD((DWORD )val, pbBuf, 3);
	
	if (fNeg)
		pbBuf[2] |= 0x80;
}


void ValToFmt10(int val, BYTE* pbBuf, WORD wLen)
{
	if (val < 0)
		return;
	else
		ValToFmt9(val, pbBuf, wLen);
}

void ValToFmt11(int val, BYTE* pbBuf, WORD wLen)
{
	ValToBCD(val, pbBuf, 4);
}


void ValToFmt12(int val, BYTE* pbBuf, WORD wLen)
{
	ValToBCD(val, pbBuf, 6);        //这里用做占位程序，因为对>4字节的数据采用64位值读取方式
}

void ValToFmt13(int val, BYTE* pbBuf, WORD wLen)
{
	ValToBCD(val, pbBuf, 4);
}


void ValToFmt14(int val, BYTE* pbBuf, WORD wLen)
{
	ValToBCD(val, pbBuf, 5);    //一般用做占位程序，因为对>4字节的数据采用64位值读取方式
}

void ValToFmt22(int val, BYTE* pbBuf, WORD wLen)
{
	if (val < 0 || val > 99)
		return;
	*pbBuf = (BYTE)(val % 10) | (BYTE)((val / 10)<<4);
}

void ValToFmt23(int val, BYTE* pbBuf, WORD wLen)
{
	ValToBCD(val, pbBuf, 3);
}

void ValToFmt25(int val, BYTE* pbBuf, WORD wLen)
{
	bool  fNeg = false;

	if (val > 999999)
		val = 999999;
	else if (val < -999999)
		val = -999999;
		
	if (val < 0)
	{
		val = -val;
		fNeg = true;
	}
	
	DWORDToBCD((DWORD )val, pbBuf, 3);
	
	if (fNeg)
		pbBuf[2] |= 0x80;
}

static void ValCopy1(int val, BYTE* pbBuf, WORD wLen)
{
	//assert(wLen < 5 && wLen > 0);
	BYTE *pbVal = (BYTE *)&val;
	while (wLen--)
	{
		*pbBuf++ = *pbVal++; 
	}
}

static void ValCopy64_1(int64 val, BYTE *pbBuf, WORD wLen)
{
	//assert(wLen < 9 && wLen > 0);
	BYTE *pbVal = (BYTE *)&val;
	while (wLen--)
	{
		*pbBuf++ = *pbVal++;
	}
}

void ValToUnknownFmt(int val, BYTE* pbBuf, WORD wLen)
{
	memset((void *)pbBuf, INVALID_VAL, (size_t)(wLen));  //?
}

void Val64ToUnknownFmt(int64 val, BYTE* pbBuf, WORD wLen)
{
	memset((void *)pbBuf, INVALID_VAL, (size_t)(wLen));  //?
}



void Val32ToBin(int val, BYTE* pbBuf, WORD wLen)
{
	memcpy(pbBuf, (BYTE *)&val, wLen);
}

void Val64ToBin(int64 val, BYTE* pbBuf, WORD wLen)
{
	memcpy(pbBuf, (BYTE *)&val, wLen);
}

typedef int (* TPfnFmtToVal32)(BYTE* pbBuf, WORD wLen);   
typedef int64 (* TPfnFmtToVal64)(BYTE* pbBuf, WORD wLen); 
typedef void  (*TPfnVal32ToFmt)(int val, BYTE *pbBuf, WORD wLen);
typedef void  (*TPfnVal64ToFmt)(int64 val64, BYTE *pbBuf, WORD wLen);

#define FMT_OTHER_TO_VAL_MAX    20
static TPfnFmtToVal32 g_pfnFmtToVal32[FMT_NUM] = {
	UnknownFmtToVal,
	(TPfnFmtToVal32)ValCopy64,    //FMT1
	Fmt2ToVal,
	Fmt3ToVal,
	Fmt4ToVal,
	Fmt5ToVal,
	Fmt6ToVal,
	Fmt7ToVal,
	Fmt8ToVal,
	Fmt9ToVal,
	Fmt10ToVal,
	Fmt11ToVal,
	Fmt12ToVal,       //数据格式12含6字节
	Fmt13ToVal,
	Fmt14ToVal,       //数据格式14含5字节
	(TPfnFmtToVal32)ValCopy64,      //FMT15
	ValCopy,      //FMT16
	ValCopy,      //FMT17
	ValCopy,      //FMT18
	ValCopy,      //FMT19
	ValCopy,      //FMT20
	ValCopy,      //FMT21
	Fmt22ToVal,   //FMT22
	Fmt23ToVal,    //FMT23
	ValCopy,		//FMT24
	Fmt25ToVal,     //FMT25
};


static TPfnFmtToVal64 g_pfnFmtToVal64[FMT_NUM] = {
	UnknownFmtToVal64,
	ValCopy64,
	Fmt2ToVal64,
	Fmt3ToVal64,
	Fmt4ToVal64,
	Fmt5ToVal64,
	Fmt6ToVal64,
	Fmt7ToVal64,
	Fmt8ToVal64,
	Fmt9ToVal64,
	Fmt10ToVal64,
	Fmt11ToVal64,
	Fmt12ToVal64,       //数据格式12含6字节
	Fmt13ToVal64,
	Fmt14ToVal64,       //数据格式14含5字节
	ValCopy64,      //FMT15
	ValCopy64,       //FMT16
	ValCopy64,      //FMT17
	ValCopy64,      //FMT18
	ValCopy64,      //FMT19
	ValCopy64,      //FMT20
	ValCopy64,      //FMT21
	Fmt22ToVal64,   //FMT22
	Fmt23ToVal64,    //FMT23
	ValCopy64,		//FMT24
	Fmt25ToVal64,     //FMT25
};

static TPfnVal32ToFmt g_pfnVal32ToFmt[FMT_NUM] = {
	ValToUnknownFmt,
	(TPfnVal32ToFmt)ValCopy64_1,
	ValToFmt2,
	ValToFmt3,
	ValToFmt4,
	ValToFmt5,
	ValToFmt6,
	ValToFmt7,
	ValToFmt8,
	ValToFmt9,
	ValToFmt10,
	ValToFmt11,
	ValToFmt12,
	ValToFmt13,
	ValToFmt14,
	(TPfnVal32ToFmt)ValCopy64_1,     //FMT15
	ValCopy1,
	ValCopy1,
	ValCopy1,
	ValCopy1,
	ValCopy1,
	ValCopy1,
	ValToFmt22,
	ValToFmt23,
	ValCopy1,
	ValToFmt25,     //FMT25
};

static TPfnVal64ToFmt g_pfnVal64ToFmt[FMT_NUM] = {
	Val64ToUnknownFmt,
	ValCopy64_1,
	Val64ToFmt2,
	Val64ToFmt3,
	Val64ToFmt4,
	Val64ToFmt5,
	Val64ToFmt6,
	Val64ToFmt7,
	Val64ToFmt8,
	Val64ToFmt9,
	Val64ToFmt10,
	Val64ToFmt11,
	Val64ToFmt12,
	Val64ToFmt13,
	Val64ToFmt14,
	ValCopy64_1,     //FMT15
	ValCopy64_1,
	ValCopy64_1,
	ValCopy64_1,
	ValCopy64_1,
	ValCopy64_1,
	ValCopy64_1,
	Val64ToFmt22,
	Val64ToFmt23,
};


static TPfnFmtToVal32 g_pfnFmtToVal32Ex[] = {      //非附录格式->32位整型
	BinToVal32,
};

static TPfnFmtToVal64 g_pfnFmtToVal64Ex[] = {   //非附录格式->64位整型
	BinToVal64
};

static TPfnVal32ToFmt  g_pfnVal32ToFmtEx[] = {   //32位整型->非附录格式
	Val32ToBin,
};

static TPfnVal64ToFmt g_pfnVal64ToFmtEx[] = {
	Val64ToBin
};


//描述：按照bFmt格式把32位有符号整形值转换到pbBuf的,只求一个数据项,不支持块
//参数：@iVal 32位或者64位有符号整形值
//		@pbBuf 存放数据项内容的缓冲区
//      @bFmt 数据项格式
//      @wLen 数据项长度,对于定长的数据项(比如国标规定的23种格式)填0即可,
//			  对于变长的数据项,比如FMT_BCD,DI_FMT_HEX等,需要填长度
//返回：数据项长度
int ValToFmt(int iVal, BYTE* pbBuf, BYTE bFmt, WORD wLen)
{
	//g_pfnVal32ToFmt[bFmt](iVal, pbBuf, wLen);
	if (bFmt>=0 && bFmt<FMT_NUM)
	{
		g_pfnVal32ToFmt[bFmt](iVal, pbBuf, wLen);
	}
	else if (bFmt>=FMTEX_START && bFmt<FMTEX_START+FMTEX_NUM)
	{
		g_pfnVal32ToFmtEx[bFmt-FMTEX_START](iVal, pbBuf, wLen);
	}
	else	//未知格式
	{
		return 0;
	}
	return 0;
}


WORD ValToFmt(int* piVal, BYTE* pbBuf, const BYTE* pbFmtStr, WORD* pwValNum)
{
	WORD  wValidNum = 0;
	BYTE  bFmt, bLen;
	*pwValNum = 0;

	while (*pbFmtStr != 0xff)
	{
		bFmt = *pbFmtStr++;	//格式
		bLen = *pbFmtStr++;	//长度
		
		if ( *piVal == INVALID_VAL )
		{			
			memset(pbBuf, INVALID_DATA, bLen);		
		}
		else
		{
			if (bFmt>=0 && bFmt<FMT_NUM)
			{
				g_pfnVal32ToFmt[bFmt](*piVal, pbBuf, bLen);
			}
			else if (bFmt>=FMTEX_START && bFmt<FMTEX_START+FMTEX_NUM)
			{
				g_pfnVal32ToFmtEx[bFmt-FMTEX_START](*piVal, pbBuf, bLen);
			}
			else	//未知格式
			{
				*pwValNum = 0;
				return 0;
			}
		}

		piVal++;
		pbBuf += bLen;
		wValidNum++;
		(*pwValNum)++;
	}	
	
	return wValidNum;
}


//描述:整形64位到格式数据的转换
WORD Val64ToFmt(int64* piVal64, BYTE* pbBuf, const BYTE* pbFmtStr, WORD* pwValNum)
{
	WORD  wValidNum = 0;
	BYTE  bFmt, bLen;
	*pwValNum = 0;

	while (*pbFmtStr != 0xff)
	{
		bFmt = *pbFmtStr++;	//格式
		bLen = *pbFmtStr++;	//长度
	
		if ( *piVal64 == INVALID_VAL64 )
		{			
			memset(pbBuf, INVALID_DATA, bLen);		
		}
		else
		{
			if (bFmt>=0 && bFmt<FMT_NUM)
			{
				Val64ToFmt(*piVal64, pbBuf, bFmt, bLen);
			}
			else if (bFmt>=FMTEX_START && bFmt<FMTEX_START+FMTEX_NUM)
			{	
				g_pfnVal64ToFmtEx[bFmt-FMTEX_START](*piVal64, pbBuf, bLen);
			}
			else	//未知格式
			{
				*pwValNum = 0;
				return 0;
			}
		}

		piVal64++;
		pbBuf += bLen;
		wValidNum++;
		(*pwValNum)++;
	}

	return wValidNum;
}

//描述：按照bFmt格式把64位有符号整形值转换到pbBuf的,只求一个数据项,不支持块
//参数：@iVal 32位或者64位有符号整形值
//		@pbBuf 存放数据项内容的缓冲区
//      @bFmt 数据项格式
//      @wLen 数据项长度,对于定长的数据项(比如国标规定的23种格式)填0即可,
//			  对于变长的数据项,比如FMT_BCD,DI_FMT_HEX等,需要填长度
//返回：数据项长度
int Val64ToFmt(int64 iVal, BYTE* pbBuf, BYTE bFmt, WORD wLen)
{	
	if (bFmt<FMT_NUM)
	{
		g_pfnVal64ToFmt[bFmt](iVal, pbBuf, wLen);
	}
	else if (bFmt>=FMTEX_START && bFmt<FMTEX_START+FMTEX_NUM)
	{
		g_pfnVal64ToFmtEx[bFmt-FMTEX_START](iVal, pbBuf, wLen);
	}
	else	//未知格式
	{
		return 0;
	}
	return 0;
}

//描述：按照bFmt格式求pbBuf的32位有符号整形,只求一个数据项,不支持块
//参数：@pbBuf 存放数据项内容的缓冲区
//      @bFmt 数据项格式
//      @wLen 数据项长度,对于定长的数据项(比如国标规定的23种格式)填0即可,
//			  对于变长的数据项,比如FMT_BCD,DI_FMT_HEX等,需要填长度
//返回：如果读到且正确求值,返回数据项的值,如果错误则返回INVALID_VAL或INVALID_VAL64
int FmtToVal(BYTE* pbBuf, BYTE bFmt, WORD wLen)
{
	if (bFmt>=0 && bFmt<FMT_NUM)
	{
		return g_pfnFmtToVal32[bFmt](pbBuf, wLen);
	}
	else if (bFmt>=FMTEX_START && bFmt<FMTEX_START+FMTEX_NUM)
	{
		return g_pfnFmtToVal32Ex[bFmt-FMTEX_START](pbBuf, wLen);
	}
	
	return INVALID_VAL;
}


WORD FmtToVal(BYTE* pbBuf, int* piVal, const BYTE* pbFmtStr, WORD* pwValNum)
{
	BYTE  bFmt, bLen;
	WORD  wValidNum = 0;
	*pwValNum = 0;

	while (*pbFmtStr != 0xff)
	{
		bFmt = *pbFmtStr++; //格式  
		bLen = *pbFmtStr++; //字节数
	
		if (IsAllAByte(pbBuf, INVALID_DATA, bLen))
		{			
			*piVal++ = INVALID_VAL;			
		}
		else
		{
			if (bFmt>=0 && bFmt<FMT_NUM)
			{
				*piVal++ = g_pfnFmtToVal32[bFmt](pbBuf, bLen);
				wValidNum++;
			}
			else if (bFmt>=FMTEX_START && bFmt<FMTEX_START+FMTEX_NUM)
			{
				*piVal++ = g_pfnFmtToVal32Ex[bFmt-FMTEX_START](pbBuf, bLen);
				wValidNum++;
			}
			else	//未知格式
			{
				*pwValNum = 0;
				return 0;
			}
		}

		pbBuf += bLen; //因为这里的pbBuf包含数据项所有格式的内容，所以应该调整数据指针,以便各格式转换函数处理属于自己范围的内容 
		(*pwValNum)++;
	}

	return wValidNum;
}


WORD FmtToVal64(BYTE* pbBuf, int64* piVal64, const BYTE* pbFmtStr, WORD* pwValNum)
{
	BYTE  bFmt, bLen;
	WORD  wValidNum = 0;
	*pwValNum = 0;

	while (*pbFmtStr != 0xff)
	{
		bFmt = *pbFmtStr++; //格式  
		bLen = *pbFmtStr++; //字节数
	
		if (IsAllAByte(pbBuf, INVALID_DATA, bLen))
		{			
			*piVal64++ = INVALID_VAL64;			
		}
		else
		{
			if (bFmt>=0 && bFmt<FMT_NUM)
			{
				*piVal64++ = g_pfnFmtToVal64[bFmt](pbBuf, bLen);
				wValidNum++;
			}
			else if (bFmt>=FMTEX_START && bFmt<FMTEX_START+FMTEX_NUM)
			{
				*piVal64++ = g_pfnFmtToVal64Ex[bFmt-FMTEX_START](pbBuf, bLen);
				wValidNum++;
			}
			else	//未知格式
			{
				*pwValNum = 0;
				return 0;
			}
		}

		pbBuf += bLen;
		(*pwValNum)++;

	}

	return wValidNum;
}

//描述：按照bFmt格式求pbBuf的64位有符号整形,只求一个数据项,不支持块
//参数：@pbBuf 存放数据项内容的缓冲区
//      @bFmt 数据项格式
//      @wLen 数据项长度,对于定长的数据项(比如国标规定的23种格式)填0即可,
//			  对于变长的数据项,比如FMT_BCD,DI_FMT_HEX等,需要填长度
//返回：如果读到且正确求值,返回数据项的值,如果错误则返回INVALID_VAL或INVALID_VAL64
int64 FmtToVal64(BYTE* pbBuf, BYTE bFmt, WORD wLen)
{	
	if (bFmt<FMT_NUM)
	{
		return g_pfnFmtToVal64[bFmt](pbBuf, wLen);
	}
	else if (bFmt>=FMTEX_START && bFmt<FMTEX_START+FMTEX_NUM)
	{
		return g_pfnFmtToVal64Ex[bFmt-FMTEX_START](pbBuf, wLen);
	}
	
	return INVALID_VAL64;
}


//FMT1
int Fmt1ToTime(BYTE* pbBuf, TTime& time)
{
	time.nSecond = BcdToByte(pbBuf[0]);
	time.nMinute = BcdToByte(pbBuf[1]);
	time.nHour = BcdToByte(pbBuf[2]);
	time.nDay = BcdToByte(pbBuf[3]);
	time.nMonth = BcdToByte(pbBuf[4]&0x1f);
	time.nYear = 2000 + BcdToByte(pbBuf[5]);

	time.nWeek = pbBuf[4]>>5;	//1~7表示星期一至星期日
	time.nWeek++;
	if (time.nWeek == 8) //DayOfWeek()的返回 1 = Sunday, 2 = Monday, ..., 7 = Saturday
		time.nWeek = 1;

	return 6;
}

int TimeToYMDHMS(TTime& time, BYTE* pbBuf)
{
	pbBuf[0] = ByteToBcd(time.nSecond);
	pbBuf[1] = ByteToBcd(time.nMinute);
	pbBuf[2] = ByteToBcd(time.nHour);
	pbBuf[3] = ByteToBcd(time.nDay);
	pbBuf[4] = ByteToBcd(time.nMonth);
	pbBuf[5] = ByteToBcd(time.nYear%100);

	return 6;
}

int TimeToFmt1(TTime& time, BYTE* pbBuf)
{
	BYTE bWeek = DayOfWeek(time); //DayOfWeek()的返回 1 = Sunday, 2 = Monday, ..., 7 = Saturday
	if (bWeek == 1)
		bWeek = 8;

	bWeek--;	

	pbBuf[0] = ByteToBcd(time.nSecond);
	pbBuf[1] = ByteToBcd(time.nMinute);
	pbBuf[2] = ByteToBcd(time.nHour);
	pbBuf[3] = ByteToBcd(time.nDay);
	pbBuf[4] = ByteToBcd(time.nMonth);

	pbBuf[4] |= bWeek<<5; 

	pbBuf[5] = ByteToBcd(time.nYear%100);

	return 6;
}

//FMT15
int TimeToFmt15(TTime& time, BYTE* pbBuf)
{
	pbBuf[0] = ByteToBcd(time.nMinute);
	pbBuf[1] = ByteToBcd(time.nHour);
	pbBuf[2] = ByteToBcd(time.nDay);
	pbBuf[3] = ByteToBcd(time.nMonth);
	pbBuf[4] = ByteToBcd(time.nYear%100);

	return 5;
}

//FMT16
int TimeToFmt16(TTime& time, BYTE* pbBuf)
{
	pbBuf[0] = ByteToBcd(time.nSecond);
	pbBuf[1] = ByteToBcd(time.nMinute);
	pbBuf[2] = ByteToBcd(time.nHour);
	pbBuf[3] = ByteToBcd(time.nDay);

	return 4;
}

//FMT17
int TimeToFmt17(TTime& time, BYTE* pbBuf)
{
	pbBuf[0] = ByteToBcd(time.nMinute);
	pbBuf[1] = ByteToBcd(time.nHour);
	pbBuf[2] = ByteToBcd(time.nDay);
	pbBuf[3] = ByteToBcd(time.nMonth);

	return 4;
}


//FMT18
int TimeToFmt18(TTime& time, BYTE* pbBuf)
{
	pbBuf[0] = ByteToBcd(time.nMinute);
	pbBuf[1] = ByteToBcd(time.nHour);
	pbBuf[2] = ByteToBcd(time.nDay);

	return  3;
}

//FMT19
int TimeToFmt19(TTime& time, BYTE* pbBuf)
{
	pbBuf[0] = ByteToBcd(time.nMinute);
	pbBuf[1] = ByteToBcd(time.nHour);

	return 2;
}


//FMT20
int TimeToFmt20(TTime& time, BYTE* pbBuf)
{
	pbBuf[0] = ByteToBcd(time.nDay);
	pbBuf[1] = ByteToBcd(time.nMonth);
	pbBuf[2] = ByteToBcd(time.nYear%100);

	return 3;
}

//FMT21
int TimeToFmt21(TTime& time, BYTE* pbBuf)
{
	pbBuf[0] = ByteToBcd(time.nMonth);
	pbBuf[1] = ByteToBcd(time.nYear%100);

	return 2;
}

//描述：把TTime结构时间time按照时间格式bFmt转换到pbBuf的
//参数：@time 时间
//		@pbBuf 存放转换后数据项内容的缓冲区
//      @bFmt 数据项格式
//返回：如果正确则返回pbBuf里数据项长度,否则返回负数
int TimeToFmt(TTime& time, BYTE* pbBuf, BYTE bFmt)
{
	switch (bFmt)
	{
	case  FMT1:
		TimeToFmt1(time, pbBuf);
		break;
	case  FMT15:
		TimeToFmt15(time, pbBuf);
		break;
	case  FMT16:
		TimeToFmt16(time, pbBuf);
		break;
	case  FMT17:
		TimeToFmt17(time, pbBuf);
		break;
	case  FMT18:
		TimeToFmt18(time, pbBuf);
		break;
	case  FMT19:
		TimeToFmt19(time, pbBuf);
		break;
	case  FMT20:
		TimeToFmt20(time, pbBuf);
		break;
	case  FMT21:
		TimeToFmt21(time, pbBuf);
		break;
	}
	return 0;
}


//描述：把pbBuf的内容按照时间格式bFmt转换到TTime结构时间time
//参数：@pbBuf 数据项内容的缓冲区
//		@time 存放转换后的时间
//      @bFmt 数据项格式
//返回：如果正确则返回pbBuf里数据项长度,否则返回负数
int FmtToTime(BYTE* pbBuf, TTime& time, BYTE bFmt)
{
	return 0;
}

//描述：按分钟转换为时间格式
int MinToFmt(DWORD min, BYTE *pbBuf, BYTE bFmt)
{
	TTime  now;
	SecondsToTime(min * 60, &now);
	
	return TimeToFmt(now, pbBuf, bFmt);
}



int64 FmtG3ToVal64(BYTE* pbBuf, WORD wLen)
{
	int64 iVal = 0;
	BYTE bExp;

	iVal = BcdToDWORD(pbBuf, wLen-1);
	pbBuf += wLen-1;
	
	iVal += (*pbBuf & 0x0f) * Pow(100, wLen-1);

	bExp = 7 - ((*pbBuf >> 5) & 0x07);
	
	iVal *= Pow(10, bExp);

	if (*pbBuf & 0x10)
		iVal = -iVal;
	
	return iVal;
}

//备注：val在传进来前要注意数据的单位换算,val应该按照该类数据的最小
//	    量纲传进来，比如功率的单位是kw,用G3格式表达的数据最小量纲是10E-3
//		即0.001kw, 所以1kw应该传进来的是1000
void Val64ToFmtG3(int64 val, BYTE* pbBuf, WORD wLen, BYTE bRound)
{
	if (val == 0)
	{
		memset(pbBuf, 0, wLen-1);
		*(pbBuf+wLen-1) = 0x80;
		return;
	}
		
	int64 iBase = Pow(10, wLen*2-1);
	bool fNeg = false;
	if (val < 0)
	{		
		fNeg = true;
		val = -val;
	}
	
	int64 tmp;
	WORD wPow = 0;
	while (val >= iBase) 
	{
		tmp = val / 10;
		if (tmp<iBase && bRound==FMT_ROUND) //要求对数据进行四舍五入
		{
			tmp = (val + 5) / 10;
			bRound = FMT_NROUND;
		}
			
		val = tmp;
		wPow++;
	}
		
	DWORDToBCD((DWORD )val, pbBuf, wLen);
	wPow = 7 - wPow;
	pbBuf += wLen - 1;
	*pbBuf |= (wPow << 5);
	if (fNeg)
		*pbBuf |= 0x10;
}

int64 FmtG1ToVal64(BYTE* pbBuf, WORD wLen)
{
	int64 iVal = BcdToDWORD(pbBuf, wLen-1);
	pbBuf += wLen-1;
	iVal += (*pbBuf & 0x0f) * Pow(100, wLen-1);
	
	if (*pbBuf & 0x40)
		iVal *= 1000;

	if (*pbBuf & 0x10)
		iVal = -iVal;
	
	return iVal;
}

void Val64ToFmtG1(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val == 0)
	{
		memset(pbBuf, 0, wLen);
		return;
	}
		
	bool fNeg = false;
	if (val < 0)
	{		
		fNeg = true;
		val = -val;
	}
	
	bool fMillion = false;
	while (val > 999999999)
	{
		val /= 1000;
		fMillion = true;
	}
		
	DWORDToBCD((DWORD )val, pbBuf, wLen);
	BYTE bFlag = 0;
	if (fNeg)
		bFlag = 0x10;
	
	if (fMillion)
		bFlag |= 0x40;

	pbBuf += wLen - 1;
	*pbBuf = (*pbBuf & 0x0f) | bFlag;
}


int Fmt15ToTime(BYTE* pbBuf, TTime& time)
{
	time.nMinute = BcdToByte(pbBuf[0]);
	time.nHour = BcdToByte(pbBuf[1]);
	time.nDay = BcdToByte(pbBuf[2]);
	time.nMonth = BcdToByte(pbBuf[3]);
	time.nYear = (WORD )BcdToByte(pbBuf[4]) + 2000;

	return 5;
}

int Fmt17ToTime(BYTE* pbBuf, TTime& time)
{
	GetCurTime(&time);
	time.nMinute = BcdToByte(pbBuf[0]);
	time.nHour = BcdToByte(pbBuf[1]);
	time.nDay = BcdToByte(pbBuf[2]);
	time.nMonth = BcdToByte(pbBuf[3]);

	return 4;
}

int Fmt20ToTime(BYTE* pbBuf, TTime& time)
{
	time.nSecond = 0;
	time.nMinute = 0;
	time.nHour = 0;
	time.nDay = BcdToByte(pbBuf[0]);
	time.nMonth = BcdToByte(pbBuf[1]);
	time.nYear = (WORD )BcdToByte(pbBuf[2]) + 2000;

	return 3;
}

void Val64ToFmt2(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val == INVALID_VAL64)
	{
		memset(pbBuf, 0xee, wLen);
		return;
	}

	memset(pbBuf, 0, wLen);

	bool fNeg;
	int64 iTmp;
	int iPow = 7;	// 表示 pow(10, -3);

	if (val < 0)
	{
		val = -val;
		fNeg = true;
	}
	else
		fNeg = false;
	iTmp = val;	// iTmp = |val|
	while (iTmp >= 1000)
	{
		iTmp = (val+5) / 10;
		val /= 10;
		iPow--;
	}
	if (iPow < 0)
		iPow = 0;	//如数值超范围,目前没有其它办法,只能简单地缩小数据.
	DWORDToBCD((DWORD)iTmp, pbBuf, wLen);
	pbBuf += wLen - 1;
	*pbBuf &= 0x0f;
	*pbBuf |= (iPow << 5);
	if (fNeg)
		*pbBuf |= 0x10;
}

void Val64ToFmt3(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val == INVALID_VAL64)
	{
		memset(pbBuf, 0xee, wLen);
		return;
	}

	memset(pbBuf, 0, wLen);

	bool fNeg, fMillion;

	if (val < 0)
	{
		val = -val;
		fNeg = true;
	}
	else
		fNeg = false;
	if (val >= 10000000)
	{
		val = (val+500) / 1000;
		fMillion = true;
	}
	else
		fMillion = false;
	DWORDToBCD((DWORD)val, pbBuf, wLen);
	pbBuf += wLen - 1;
	*pbBuf &= 0x0f;
	if (fMillion)
		*pbBuf |= 0x40;
	if (fNeg)
		*pbBuf |= 0x10;
}
/*****************************************/
//64位值转换函数
void Val64ToFmt4(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val>99 || val<-99)
		val = 99;
	ValToFmt4((int)val, pbBuf, wLen);
}

void Val64ToFmt5(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val>9999 || val<-9999)
		val = 9999;
	ValToFmt5((int)val, pbBuf, wLen);	
}

void Val64ToFmt6(int64 val, BYTE* pbBuf, WORD wLen)
{
	Val64ToFmt5(val, pbBuf, wLen);
}

void Val64ToFmt7(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val < 0)  val = 0;
	if (val > 9999)		val = 9999;

	Val64ToBCD(val, pbBuf, 2);    
}


void Val64ToFmt8(int64 val, BYTE* pbBuf, WORD wLen)
{
	Val64ToFmt7(val, pbBuf, wLen);
}

void Val64ToFmt9(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val>999999 || val<-999999)
		val = 999999;
		
	Val64ToBCD(val, pbBuf, 3);   
}

void Val64ToFmt10(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val < 0)  val = 0;
	if (val > 999999)	val = 999999;

	Val64ToBCD(val, pbBuf, 3);    
}

void Val64ToFmt11(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val < 0)  val = 0;
	if (val > 99999999)		val = 99999999;

	Val64ToBCD(val, pbBuf, 4);    
}
void Val64ToFmt12(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val < 0)  val = 0;
	if (val > 999999999999LL) 	val = 999999999999LL;

	Val64ToBCD(val, pbBuf, 6);    
}
void Val64ToFmt13(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val < 0)  val = 0;
	if (val > 99999999)		val = 99999999;

	Val64ToBCD(val, pbBuf, 4);    
}
void Val64ToFmt14(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val < 0)  val = 0;
	if (val > 9999999999LL)	val = 9999999999LL;

	Val64ToBCD(val, pbBuf, 5);    
}
void Val64ToFmt22(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val < 0)  val = 0;
	if (val > 99)		val = 99;

	Val64ToBCD(val, pbBuf, 1);    
}
void Val64ToFmt23(int64 val, BYTE* pbBuf, WORD wLen)
{
	if (val < 0)  val = 0;
	if (val > 9999)		val = 9999;

	Val64ToBCD(val, pbBuf, 2);    
}

void Val64ToFmt30(int64 val, BYTE* pbBuf, WORD wLen)
{
	BYTE bFlag = 0;

	if (val < 0) 
	{
		val = -val;
		bFlag = 1;
	}

	if (val > 9999999999LL)	val = 9999999999LL;

	Val64ToBCD(val, pbBuf, 5);  

	if (bFlag != 0)	
		pbBuf[4] = (pbBuf[4]&0x7f) + 0x80;			
}

int64 Fmt30ToVal64(BYTE* pbBuf, WORD wLen)
{
	int64 val;

	if (wLen < 5)
		return BcdToDDWORD(pbBuf, wLen);

	BYTE bFlag = 0;

	if ((pbBuf[4]&0x80) == 0x80) 
	{		
		bFlag = 1;
		pbBuf[4] = pbBuf[4]&0x7f;
	}

	val = BcdToDDWORD(pbBuf, 5);

	if (bFlag != 0)	
		val = -val;		

	return val;
}


int TimeToOctet(TTime& time, BYTE* pbBuf)
{
	pbBuf[5] = time.nMinute;
	pbBuf[4] = time.nHour;
	pbBuf[3] = time.nDay;
	pbBuf[2] = time.nMonth;
	pbBuf[1] = time.nYear&0xff;
	pbBuf[0] = time.nYear>>8;

	return 6;
}

int OctetToTime(BYTE* pbBuf, TTime& time)
{//高字节在前，低字节在后
	time.nMinute	= pbBuf[5];
	time.nHour		= pbBuf[4];
	time.nDay		= pbBuf[3];
	time.nMonth		= pbBuf[2];
	time.nYear		= pbBuf[1]+(pbBuf[0]<<8);

	return 6;
}

int GetTsaData(BYTE* pSrc, BYTE* bTsa)
{
	BYTE bLen = (pSrc[1]&0x0f) + 1;		//地址长度
	if (bLen < 18)
		memcpy(bTsa, &pSrc[1], bLen + 1);

	return bLen + 2;
}




