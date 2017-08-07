/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ComAPI.h
 * 摘    要：本文件主要包含common目录下API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：在common目录下放平台和应用无关的公用源文件和头文件
 *********************************************************************************************************/
#ifndef COMAPI_H
#define COMAPI_H

#include "ComConst.h"
#include "ComStruct.h"
#include "apptypedef.h"
#include "Info.h"
#include <string.h>

#define BASETIME          1999
#define BASEWEEK		  5			//1999/1/1是星期5 用5表示

bool IsBcdCode(BYTE *p, WORD num);
BYTE BcdToByte(BYTE bcd);
BYTE ByteToBcd(BYTE b);
DWORD BcdToDWORD(BYTE* p, WORD len);
DDWORD BcdToDDWORD(BYTE* p, WORD len);
WORD ByteToWord(BYTE* pbBuf, WORD wLen);
DWORD ByteToDWORD(BYTE* pbBuf, WORD wLen);
void DWordToByte(DWORD val, BYTE* byte, WORD len);
WORD DWordToByte(DWORD dw, BYTE* p);
DWORD ByteToDWord(BYTE* p);
uint64 BcdToUint64(BYTE* p, WORD len);
int BcdToInt(BYTE* p, WORD len);
void IntToBCD(int val, BYTE* bcd, WORD len);
void DWORDToBCD(DWORD val, BYTE* bcd, WORD len);
void Uint64ToBCD(uint64 val, BYTE* bcd, WORD len);
void HexToASCII(BYTE* in, BYTE* out, WORD wInLen);
void ByteToASCII(BYTE b, BYTE** pp);
void ByteXtoASCII(BYTE b, BYTE** pp);
BYTE AsciiToByte(BYTE** pp);
bool AsciiToByte(BYTE* pBufAscii, WORD wAsciiLen, BYTE* bOutBuf);
bool IsAllAByte(const BYTE* p, BYTE b, WORD len);
bool IsExistHalfAByte(const BYTE* p, BYTE b, WORD len);

bool IsAllAVal32(const int* piVal32 ,int iDstVal32, WORD wNum);
bool IsAllAVal64(const int64* piVal64, int64 iDstVal64, WORD wNum);
void SetArrVal32(int* piVal32, int iDstVal32, WORD wNum);
void SetArrVal64(int64* piVal64, int64 iDstVal64, WORD wNum);

WORD SearchStrVal(char* pStart, char* pEnd);
BYTE* bufbuf(BYTE* pbSrc, WORD wSrcLen, BYTE* pbSub, WORD wSubLen);
BYTE CheckSum(BYTE* p, WORD wLen);
BYTE CRCCheck(BYTE bytDir, BYTE *abytCommOrder , WORD nStartPos, WORD nCheckLen);

void GetCurTime(TTime* pTime);
DWORD GetCurTime();
DWORD GetCurMinute();

DWORD DaysFrom2000(const TTime& time);
DWORD MinutesFrom2000(const TTime& rPast);
DWORD MonthFrom2000(const TTime& rPast);
DWORD MonthsPast(const TTime& rPast, const TTime& rNow);
int DaysPast(const TTime& rPast, const TTime& rNow);
DWORD HoursPast(const TTime& rPast, const TTime& rNow);
DWORD MinutesPast(const TTime& rPast, const TTime& rNow);
DWORD SecondsPast(const TTime& rPast, const TTime& rNow);
bool IsInvalidTime(const TTime& time);
DWORD TimeToSeconds(const TTime& time);
DWORD MilTimeToSeconds(const TMillTime& time);
DWORD TimeToMinutes(const TTime& time);
int dayOfWeek(int year,int month,int day);
void SecondsToTime(DWORD dwSeconds, TTime* pTime);
void MinutesToTime(DWORD dwMins, TTime* pTime);
void DaysToTime(DWORD dwDays, TTime* pTime);
void MonthsToTime(DWORD dwMonths, TTime* pTime);
bool IsTimeEmpty(const TTime& time);
bool IsDiffDay(const TTime& time1, const TTime& time2);
bool IsSameDay(const TTime& time1, const TTime& time2);
bool IsSameMon(const TTime& time1, const TTime& time2);
bool IsDiffHour(const TTime& time1, const TTime& time2);
int MunitesSub(const TTime& time1, const TTime& time2);
void MinuteToBuf(const TTime& time, BYTE* pbBuf);
BYTE DayOfWeek(const TTime& time);
int DaysPast(int year, int month, int day);
bool AddIntervs(TTime& time, BYTE bIntervU, int iIntervV);
int IntervsPast(const TTime& tmPast, const TTime& tmNow, BYTE bIntervU, BYTE bIntervV);
BYTE DaysOfMonth(TTime time);
DWORD GetMonthStart(TTime time);
DWORD GetMonthEnd(TTime time);
void TimeToIntervS(TTime time, BYTE bType, DWORD& dwStartS, DWORD& dwEndS, DWORD dwIntvT = 0);

bool ReadFile(char* pszPathName, BYTE* pbData, DWORD dwBytesToRead);
int readfile(char* pszPathName, BYTE* pbData, DWORD dwMaxBytesToRead);
bool WriteFile(char* pszPathName, BYTE* pbData, DWORD dwLen);
bool PartWriteFile(char* pszPathName, DWORD dwOff,BYTE* pbData, DWORD wLen);
bool PartReadFile(char* pszPathName, DWORD dwOffset, BYTE *pbData, DWORD dwLen);
bool DeleteFile(char* pszPathName);
int GetFileLen(char* pszPathName);
void UpdateTxPwr(BYTE bTxPwr, int16 bSign);
void UpdateDialIP();

inline void SetEmptyTime(TTime* pTime)
{
	memset(pTime, 0, sizeof(TTime));
}

inline WORD ByteToWord(BYTE* p)
{
	return (WORD )*(p + 1) * 0x00100 + *p; 
}

inline WORD WordToByte(WORD w, BYTE* p)
{
	*p++ = (BYTE )(w & 0xff);
	*p = (BYTE )(w >> 8);

	return 2;
}

inline int Abs(int x)
{
	return x>=0 ? x : -x;
}

inline DWORD GetAbsGap(DWORD dwVal1, DWORD dwVal2)
{	
	return dwVal1>=dwVal2 ? dwVal1-dwVal2 : dwVal2-dwVal1;
}

#define ABS(x) ((x)>=0 ? (x) : -(x))

#define PPPINITFCS16 0xffff /* Initial FCS value */
#define PPPGOODFCS16 0xf0b8 /* Good final FCS value */
WORD pppfcs16(WORD fcs,unsigned char * cp,int len);
WORD CheckCrc16(unsigned char *pbyt,int iLen);
WORD get_crc_16 (WORD start, BYTE *p, int n);
DWORD get_crc_32 (BYTE * data, int data_len);
unsigned char CRC8_Tab(unsigned char* ucPtr, unsigned char ucLen);

void revcpy(BYTE* pbDst, const BYTE* pbSrc, WORD wLen);
int revcmp(const void *buf1, const void *buf2, int count);
void RevBuf(BYTE* pbBuf, WORD wLen);
BYTE memrcpy(BYTE *pbDst,BYTE *pbSrc,WORD wLen);
void Swap(BYTE *pbBuf, WORD wLen);

BYTE EncodeLength(int len,BYTE *p);	//根据长度完成长度编码
int DecodeLength(BYTE *pbBuf,DWORD *pdwNum); //对长度完成解码

BYTE ByteBitReverse(BYTE b);	//对字节按位进行倒序
void BufByteReverse(BYTE *bBuf, WORD wLen);	//对字节串以字节为单位进行倒序
void BufBitReverse(BYTE *bBuf, WORD wLen);	//对字节串以位为单位进行倒序

int64 Pow(int iBase, WORD wExp);
bool IsTimeValid(BYTE *pBuf);
bool Isdate_time_sValid(BYTE *pBuf);
char* TimeToStr(const TTime& time, char* psz);
char* TimeToStr(DWORD dwTime, char* psz);
char* MillTimeToStr(TMillTime tMillTime, char *psz);
char* MtrAddrToStr(const BYTE* pbAddr, char* psz);
char* MemTypeToStr(BYTE bType, char* psz);
BYTE* GetSubPos(BYTE *src, WORD wSrcLen, BYTE *dst, WORD wDstLen);
WORD CalcuBitNum(const BYTE* pbBuf, WORD wSize);
void AndTwoBuf(BYTE* p1, const BYTE* p2, WORD wSize);
void XorTwoBuf(BYTE* p1, const BYTE* p2, WORD wSize);
void OrTwoBuf(BYTE* p1, const BYTE* p2, WORD wSize);

//描述 :将时间TTime格式转换为DateTime格式
void  TimeToDateTime(const TTime &time, BYTE *pbBuf);
//描述 :将时间DateTime格式转换为TTime格式
void  DateTimeToTime(BYTE *pbBuf, TTime &time);
int ByteToInt(BYTE* p, WORD wLen);
void TestCommPort(WORD wPort, DWORD dwBaudRate);

//645组帧函数
BYTE Make645AskItemFrm(BYTE bMtrPro, BYTE* pbAddr, DWORD dwID, BYTE* pbFrm, BYTE bCtrl=0);
BYTE Make645WriteItemFrm(BYTE bMtrPro, BYTE* pbAddr, DWORD dwID, BYTE* pbFrm,BYTE* pbData,BYTE bLen);
WORD Make645Frm(BYTE* pbFrm, const BYTE* pbAddr, BYTE bCmd, BYTE bDataLen);
DWORD GetIdFrom645AskItemFrm(BYTE* pbFrm);
DWORD SearchStrValHex(char* pStart, char* pEnd);

WORD gsmDecode7bit(const BYTE* pSrc, BYTE* pDst, WORD wSrcLen);
bool IsMountedOK(char *str);
#ifdef SYS_LINUX
bool UsbUpdate(char* szPath);
#endif
bool Check645Frm(BYTE* pbBuf, int nLen);

bool AddAcqAddress(BYTE *pbBuf, BYTE *pbAcqAddr);  //在pbBuf后面增加6字节采集器地址并+33
BYTE ASCII2BCD(BYTE bLow, BYTE bHigh);
void AsciiStr2BCD(BYTE* pAscii, BYTE* pBcd, BYTE bBcdLen);
WORD CpyUntilCR(BYTE* pDst, BYTE* pSrc, WORD wLen);
int ReadBankId(WORD wBank, WORD wPn, WORD wID, BYTE *pbBuf);
BYTE GetBlockIdNum(WORD wID);
bool IsRJLink(WORD wPn);

void GetCurMillTime(TMillTime* pMillTime);
void SetCurMillTime(TMillTime* pMillTime);
BYTE OoDayOfWeek(const TTime& time);

//描述：把TSA转换成字符串，方便打印
//参数：@ pbTSA 表地址
//		@pszBuf 用来装字符串的缓冲区
//返回：返回转换后字符串指针，其实就是pszBuf
char* TsaToStr(BYTE* pbTSA, char* pszBuf);

//描述：把DWORD转换成字符串，方便打印
//参数：@ dw 值
//		@pszBuf 用来装字符串的缓冲区
//返回：返回转换后字符串指针，其实就是pszBuf
char* DwordToStr(DWORD dw, char* pszBuf);

//描述：把字节缓冲转换成字符串，方便打印
//参数：@ p字节缓冲指针
//		@ wLen 缓冲区的长度
//		@pszBuf 用来装字符串的缓冲区
//		@ fRevs 字符串是否反向
//返回：返回转换后字符串指针，其实就是pszBuf
char* HexToStr(BYTE* p, WORD wLen, char* pszBuf, bool fRevs = false);

//描述：排序,数组大小低到高
void IntSort(int *pibuf, WORD wLen);

extern BYTE BitReverse(BYTE b);
#endif //COMAPI_H

