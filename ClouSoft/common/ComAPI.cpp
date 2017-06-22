/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ComAPI.cpp
 * 摘    要：本文件主要包含common目录下API函数和全局变量的定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：在common目录下放平台和应用无关的公用源文件和头文件
 *********************************************************************************************************/
#include "stdafx.h"
#include "syscfg.h"
#include "ComAPI.h"
#include "FaAPI.h"
#include <string.h>
#include <stdio.h>
#include "sysfs.h"
#include "DrvAPI.h"

#ifdef SYS_WIN
#include <windows.h>
#endif

using namespace std;

bool IsBcdCode(BYTE *p, WORD num)
{
	BYTE ch;
	for( int i=0; i<num; i++)
	{
		ch = p[i]&0xf;
		if (ch > 9)
			return false;

		ch = p[i]>>4;
		if (ch > 9)
			return false;
	}
	return true;
}

BYTE BcdToByte(BYTE bcd) 
{
	return ((bcd >> 4) & 0x0f) * 10 + (bcd & 0x0f);
}

bool BcdToByte(BYTE* pbDest, BYTE* pbSrc, WORD wLen)
{
    for(WORD i=0; i<wLen; i++)
    {
        *(pbDest+i) = ((*(pbSrc+i) >> 4) & 0x0f) * 10 + (*(pbSrc+i) & 0x0f);
    }
    return true;
}

BYTE ByteToBcd(BYTE b)
{
	return (b/10<<4) + b%10;
}


DWORD BcdToDWORD(BYTE* p, WORD len)
{
	BYTE* p0 = p;
	p += len - 1;

	DWORD val = 0;
	for (; p>=p0; p--)
	{
		val = val * 100 + ((*p >> 4) & 0x0f) * 10 + (*p & 0x0f);
	}

	return val;
}

DDWORD BcdToDDWORD(BYTE* p, WORD len)
{
	BYTE* p0 = p;
	p += len - 1;

	DDWORD val = 0;
	for (; p>=p0; p--)
	{
		val = val * 100 + ((*p >> 4) & 0x0f) * 10 + (*p & 0x0f);
	}

	return val; 
}

WORD ByteToWord(BYTE* pbBuf, WORD wLen)
{
	WORD val = 0;
	if (wLen > 2)
		wLen = 2;

	memcpy(&val, pbBuf, wLen);
	return val;
}

DWORD ByteToDWORD(BYTE* pbBuf, WORD wLen)
{
	DWORD val = 0;
	if (wLen > 4)
		wLen = 4;
		
	memcpy(&val, pbBuf, wLen);
	return val;
}

uint64 BcdToUint64(BYTE* p, WORD len)
{
	BYTE* p0 = p;
	p += len - 1;

	uint64 val = 0;
	for (; p>=p0; p--)
	{
		val = val * 100 + ((*p >> 4) & 0x0f) * 10 + (*p & 0x0f);
	}

	return val;
}


//参数：@len bcd的字节长度
int BcdToInt(BYTE* p, WORD len)
{
	BYTE* p0 = p;

	p += len - 1;
	int val = (*p & 0x0f);

	bool fNeg;
	if ((*p&0xf0) != 0)
		fNeg = true;
	else
		fNeg = false;

	p--;

	for (; p>=p0; p--)
	{
		val = val * 100 + ((*p >> 4) & 0x0f) * 10 + (*p & 0x0f);
	}

	if (fNeg)
	    return -val;
	else
		return val;
}


void IntToBCD(int val, BYTE* bcd, WORD len)
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
	val %= power;

	power /= 10;

	if (fNeg)
		bcd[len - 1] = 0x10 + val / power;
	else
		bcd[len - 1] = val / power;

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

void DWORDToBCD(DWORD val, BYTE* bcd, WORD len)
{
	DWORD power;
	BYTE bHigh, bLow;

	memset(bcd,0,len);
	if (len > 4)//处理最高字节
	{
		power = 1000000000;
		bHigh = (BYTE )(val / power);
	    val %= power;

		power = 100000000;
		bLow = (BYTE )(val / power);
	    val %= power;
		bcd[4] = (bHigh << 4) | bLow;

		len = 4;
	}

	power = 1;
	for (WORD i=0; i<len; i++)
	{
		power *= 100; 
	}

	for (; len>0; len--)
	{
	    val %= power;
	    power /= 10;
		bHigh = (BYTE )(val / power);

	    val %= power;
	    power /= 10;
		bLow = (BYTE )(val / power);
		bcd[len - 1] = (bHigh << 4) | bLow;
	}
}

void DWordToByte(DWORD val, BYTE* byte, WORD len)
{
	if( len > 4 ) len = 4;
	for(; len>0; byte++, len--)
	{
		*byte = (BYTE)val%0x100;
		val = val/0x100;
	}
}

WORD DWordToByte(DWORD dw, BYTE* p)
{
	*p++ = dw & 0xff;
	*p++ = (dw >> 8) & 0xff;
	*p++ = (dw >> 16) & 0xff;
	*p = (dw >> 24) & 0xff;
	return 4;
}

DWORD ByteToDWord(BYTE* p)
{
	DWORD dw = 0;
	memcpy(&dw, p, 4);
	return dw;
	//return *p + (DWORD )*(p + 1)*0x100 + (DWORD )*(p + 2)*0x10000 + (DWORD )*(p + 3)*0x1000000;
}

void Uint64ToBCD(uint64 val, BYTE* bcd, WORD len)
{
	uint64 power;
	BYTE bHigh, bLow;

	memset(bcd, 0, len);

	power = 1;
	for (WORD i=0; i<len; i++)
	{
		power *= 100; 
	}

	for (; len>0; len--)
	{
	    val %= power;
	    power /= 10;
		bHigh = (BYTE )(val / power);

	    val %= power;
	    power /= 10;
		bLow = (BYTE )(val / power);
		bcd[len - 1] = (bHigh << 4) | bLow;
	}
}

void HexToASCII(BYTE* in, BYTE* out, WORD wInLen)
{
    for (WORD i=0; i<wInLen; i++)
    {
		BYTE b = *in++;
		BYTE hi = b >> 4;
		BYTE lo = b & 0x0f;
		if (hi >= 0x0a)
			*out++ = hi - 0x0a + 'A';
		else
			*out++ = hi + '0';

		if (lo >= 0x0a)
			*out++ = lo - 0x0a + 'A';
		else
			*out++ = lo + '0';
	}
}


void ByteToASCII(BYTE b, BYTE** pp)
{
	BYTE* p = *pp;

	BYTE hi = b >> 4;
	BYTE lo = b & 0x0f;
	if (hi >= 0x0a)
		*p++ = hi - 0x0a + 'A';
	else
		*p++ = hi + '0';

	if (lo >= 0x0a)
		*p++ = lo - 0x0a + 'A';
	else
		*p++ = lo + '0';

	*pp = p;
}


void ByteXtoASCII(BYTE b, BYTE** pp)
{
	BYTE* p = *pp;

	BYTE lo = b >> 4;
	BYTE hi = b & 0x0f;

	if (hi >= 0x0a)
		*p++ = hi - 0x0a + 'A';
	else
		*p++ = hi + '0';

	if (lo >= 0x0a)
		*p++ = lo - 0x0a + 'A';
	else
		*p++ = lo + '0';

	*pp = p;
}


BYTE AsciiToByte(BYTE** pp)
{
	BYTE* p = *pp;
	BYTE hi = 0, lo = 0;

	if (*p>='A' && *p<='F')
		hi = *p - 'A' + 0x0a;
	else if (*p>='a' && *p<='f')
		hi = *p - 'a' + 0x0a;
	else if (*p>='0' && *p<='9')
		hi = *p - '0';
	else
		return 0;

	p++;

	if (*p>='A' && *p<='F')
		lo = *p - 'A' + 0x0a;
	else if (*p>='a' && *p<='f')
		lo = *p - 'a' + 0x0a;
	else if (*p>='0' && *p<='9')
		lo = *p - '0'; 
	else
		return hi;

	p++;

	*pp = p;

	return (hi << 4) | lo;
}

//将一串指定长度的ascii字符串转换为byte串
bool AsciiToByte(BYTE* pBufAscii, WORD wAsciiLen, BYTE* bOutBuf)
{
	if(pBufAscii==NULL || bOutBuf==NULL || wAsciiLen==0)
		return false;

	for(WORD i=0;i<wAsciiLen/2;i++)
	{
       *bOutBuf++ = AsciiToByte(&pBufAscii);
	}

}

bool IsAllAByte(const BYTE* p, BYTE b, WORD len)
{
	for (WORD i=0; i<len; i++)
	{
		if (*p++ != b)
			return false;
	}
	
	return true;
}
//是否存在半字节b
bool IsExistHalfAByte(const BYTE* p, BYTE b, WORD len)
{
	for (WORD i = 0; i < len; i++)
	{
		if ((((*p) & 0x0f) == (b & 0x0f)) || (((*p++) & 0xf0) == (b<<4)))
		{
			return true;
		}
	}
	return false;

}


// 描述：判断是否全部VAL数组均为无效值
bool IsAllAVal32(const int* piVal32 ,int32 iDstVal32, WORD wNum)
{
	for (WORD i=0; i<wNum; i++)
	{
		if (piVal32[i] != iDstVal32)

		  return false;			
	}

	return true;
}

//描述：判断是否全部VAL64数组均为无效值
bool IsAllAVal64(const int64* piVal64, int64 iDstVal64, WORD wNum)
{
	for (WORD i=0; i<wNum; i++)
	{
		if (piVal64[i] != iDstVal64)

		  return false;			
	}

	return true;
}


//描述：设置全部VAL64数组均为目标值
void SetArrVal64(int64* piVal64, int64 iDstVal64, WORD wNum)
{
	for (WORD i=0; i<wNum; i++)
	{
		piVal64[i] = iDstVal64;		
	}
}

//描述：设置全部VAL64数组均为目标值
void SetArrVal32(int32* piVal32, int32 iDstVal32, WORD wNum)
{
	for (WORD i=0; i<wNum; i++)
	{
		piVal32[i] = iDstVal32;		
	}
}


WORD SearchStrVal(char* pStart, char* pEnd)
{
	bool fGetFirst = false;
	WORD val = 0;
	while (pStart < pEnd)
	{
		char c = *pStart++;
		if (!fGetFirst)
		{
			if (c>='0' && c<='9')
			{
				fGetFirst = true;
			}
		}

		if (fGetFirst)
		{
			if (c>='0' && c<='9')
			{
				val = val*10 + c - '0';
			}
			else
			{
				break;
			}
		}

	}

	return val;
}



BYTE* bufbuf(BYTE* pbSrc, WORD wSrcLen, BYTE* pbSub, WORD wSubLen)
{
	BYTE* pbSrcEnd = pbSrc + wSrcLen;
	while (pbSrc+wSubLen <= pbSrcEnd)
	{
		if (memcmp(pbSrc, pbSub, wSubLen) == 0)
			return pbSrc;
		
		pbSrc++;
	}
	
	return NULL;
}

BYTE CheckSum(BYTE* p, WORD wLen)
{
    BYTE bSum = 0;	
    for (; wLen > 0; wLen--)
	{
  	    bSum += *p++;
	}	
    return bSum;
}
BYTE CRCCheck(BYTE bytDir, BYTE *abytCommOrder , WORD nStartPos, WORD nCheckLen)
{
   static int16 MSBInfo ;
   static WORD wCrcData;
   static WORD nIndexI,nIndexJ;

   wCrcData=0xffff;
   for(nIndexI = nStartPos; nIndexI < (nCheckLen+nStartPos); nIndexI ++)
   {
      wCrcData = wCrcData ^ abytCommOrder[nIndexI];
      for(nIndexJ = 0; nIndexJ < 8; nIndexJ ++)
      {
         MSBInfo = wCrcData & 0x0001;
         wCrcData = wCrcData  >> 1;
         if(MSBInfo != 0 )
         {
            wCrcData = wCrcData ^ 0xa001;
         }
      }
   }
   if (bytDir== 0)
   {
      abytCommOrder[nIndexI ] = wCrcData % 0x100;
      abytCommOrder[nIndexI + 1] = wCrcData / 0x100;
      return(0);
   }
   if (abytCommOrder[nIndexI ] != (wCrcData % 0x100)) return (1);
   if (abytCommOrder[nIndexI + 1] != (wCrcData / 0x100)) return(1);
   return(0);
}

void GetCurTime(TTime* pTime)
{
	GetSysTime(pTime);

	/*printf("GetCurTime : %02d/%02d/%02d %02d::%02d::%02d.\r\n", 
							pTime->nYear, pTime->nMonth, pTime->nDay, 
					  		pTime->nHour, pTime->nMinute, pTime->nSecond);*/

	if (IsInvalidTime(*pTime))
	{
		DTRACE(DB_GLOBAL, ("GetCurTime : error time %02d/%02d/%02d %02d::%02d::%02d.\r\n", 
					  		pTime->nYear, pTime->nMonth, pTime->nDay, 
					  		pTime->nHour, pTime->nMinute, pTime->nSecond));
	}
}


//描述：返回当前距离2000/1/1 00:00:00 的秒数
DWORD GetCurTime()
{
	TTime now;
	GetCurTime(&now);

	return DaysFrom2000(now)*60*60*24 +
		   (DWORD )(now.nHour)*60*60 +
		   (DWORD )now.nMinute*60 + now.nSecond;
}

//描述：返回当前距离2000/1/1 00:00:00 的分钟数
DWORD GetCurMinute()
{
	TTime now;
	GetCurTime(&now);

	return DaysFrom2000(now)*60*24 +
		   (DWORD )(now.nHour)*60 +
		   now.nMinute;
}


WORD g_wDaysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
                         //1   2   3   4   5   6   7   8   9   10  11  12   

//描述：计算从2000/1/1到year/month/day的天数
DWORD DaysFrom2000(const TTime& time)
{
	if (IsTimeEmpty(time))
	{
		return 0;
	}
	
	int y, m, d, days;
	y=BASETIME;
	m=1;
	d=1;

	days = 0;
	while (y < time.nYear)
	{
		if ((y%4==0 && y%100!=0) || y%400==0)   //闰年
			days += 366;
		else
			days += 365;

		y++;
	}

	//现在：y == year
	while (m < time.nMonth)
	{
		if (m == 2)
		{
			if ((y%4==0 && y%100!=0) || y%400==0)   //闰年
				days += 29;
			else
				days += 28;

		}
		else
		{
			days += g_wDaysOfMonth[m-1];
		}

		m++;
	}

	//现在：m == month
	days += time.nDay - d;

	return days;
}

DWORD MinutesFrom2000(const TTime& rPast)
{
	DWORD dwPast = DaysFrom2000(rPast) * 60 * 24;
	dwPast += (DWORD )(rPast.nHour)*60 + rPast.nMinute;

	return dwPast;
}

DWORD MonthFrom2000(const TTime& rPast)
{
	return (rPast.nYear-BASETIME)*12+rPast.nMonth-1;
}


//一年的月数：0x1D5'5600
DWORD MonthsPast(const TTime& rPast, const TTime& rNow)
{
	DWORD dwPast = (rPast.nYear - BASETIME) * 12 + rPast.nMonth - 1;
	DWORD dwNow = (rNow.nYear - BASETIME) * 12 + rNow.nMonth - 1;
	if (dwNow <= dwPast)
		return 0;
	else
		return dwNow - dwPast;
}


//一年的秒数：0x1D5'5600
int DaysPast(const TTime& rPast, const TTime& rNow)
{
	int iPast = (int )DaysFrom2000(rPast);
	int iNow = (int )DaysFrom2000(rNow);
	return iNow - iPast;
}

//一年的秒数：0x1D5'5600
DWORD HoursPast(const TTime& rPast, const TTime& rNow)
{
	DWORD dwPast = DaysFrom2000(rPast) * 24;
	dwPast += rPast.nHour;

	DWORD dwNow = DaysFrom2000(rNow) * 24;
	dwNow += rNow.nHour;

	if (dwNow <= dwPast)
		return 0;
	else
		return dwNow - dwPast;
}

//一年的秒数：0x1D5'5600
DWORD MinutesPast(const TTime& rPast, const TTime& rNow)
{
	DWORD dwPast = DaysFrom2000(rPast) * 60 * 24;
	dwPast += (DWORD )(rPast.nHour)*60 + rPast.nMinute;

	DWORD dwNow = DaysFrom2000(rNow) * 60 * 24;
	dwNow += (DWORD )(rNow.nHour)*60 + rNow.nMinute;

	if (dwNow <= dwPast)
		return 0;
	else
		return dwNow - dwPast;
}

/*
int MunitesSub(const TTime& time1, const TTime& time2)
{
	int t1 = DaysFrom2000(time1) * 60 * 24;
	t1 += (int )(time1.nHour)*60 + time1.nMinute;

	int t2 = DaysFrom2000(time2) * 60 * 24;
	t2 += (int )(time2.nHour)*60 + time2.nMinute;
	
	return t1 - t2;
}*/


DWORD SecondsPast(const TTime& rPast, const TTime& rNow)
{
	DWORD dwPast = DaysFrom2000(rPast) * 60 * 60 * 24;
	dwPast += (DWORD )(rPast.nHour)*60*60 + (DWORD )rPast.nMinute*60 + rPast.nSecond;

	DWORD dwNow = DaysFrom2000(rNow) * 60 * 60 * 24;
	dwNow += (DWORD )(rNow.nHour)*60*60 + (DWORD )rNow.nMinute*60 + rNow.nSecond;

	if (dwNow <= dwPast)
		return 0;
	else
		return dwNow - dwPast;
}

bool IsInvalidTime(const TTime& time)
{
	if (time.nYear<2000 || time.nYear>2100 ||
		time.nMonth<1 || time.nMonth>12 ||
		time.nDay<1 || time.nDay>31 ||
		time.nHour>23 ||
		time.nMinute >= 60 ||
		time.nSecond >= 60)
	{
		return true;
	}
	else
	{
		if(time.nMonth==4 || time.nMonth==6 || time.nMonth==9 || time.nMonth==11)
		{
			if(time.nDay >= 31)
				return true;
		}

		if(time.nMonth == 2)
		{
			if ((time.nYear%4==0 && time.nYear%100!=0) || time.nYear%400==0)   //闰年
			{
				if(time.nDay >= 30)
					return true;
			}
			else
			{
				if(time.nDay >= 29)
					return true;
			}
		}
		return false;
	}
}



void SecondsToTime(DWORD dwSeconds, TTime* pTime)
{
	int year=BASETIME, month=1, day=1, hour=0, minute=0, second=0;
	DWORD delta;

	while (dwSeconds > 0)
	{
		if ((year%4==0 && year%100!=0) || year%400==0)   //闰年
			delta = 366*24*60*60;
		else
			delta = 365*24*60*60;

		if (dwSeconds < delta)
		{
			break;
		}
		else
		{
			dwSeconds -= delta;
			year++;
		}
	}
	

	while (dwSeconds > 0)
	{
		if (month == 2)
		{
			if ((year%4==0 && year%100!=0) || year%400==0)   //闰年
				delta = 29*24*60*60;
			else
				delta = 28*24*60*60;
		}
		else
		{
			delta = g_wDaysOfMonth[month-1]*24*60*60;
		}
		
		if (dwSeconds < delta)
		{
			break;
		}
		else
		{
			dwSeconds -= delta;
			month++;
		}
	}

	if (dwSeconds > 0)
	{
		day = dwSeconds / (24*60*60);
		
		dwSeconds -= day * 24 * 60 * 60;
		
		day++;
		
		if (dwSeconds > 0)
		{
			hour = dwSeconds / (60*60);
			
			dwSeconds -= hour * 60 * 60;
			
			if (dwSeconds > 0)
			{
				minute = dwSeconds / 60;
				second = dwSeconds - minute * 60;
			}
		}
	}
	
	pTime->nYear = year;
	pTime->nMonth = month;
	pTime->nDay = day;
	pTime->nHour = hour;
	pTime->nMinute = minute;
	pTime->nSecond = second;
//	pTime->nWeek = dayOfWeek(year, month, day) + 1;
	pTime->nWeek = DayOfWeek(*pTime);
	
}

void MinutesToTime(DWORD dwMins, TTime* pTime)
{
	SecondsToTime(dwMins*60, pTime);
}

void DaysToTime(DWORD dwDays, TTime* pTime)
{
	SecondsToTime( dwDays * 24 * 3600, pTime );
}

void MonthsToTime(DWORD dwMonths, TTime* pTime)
{
	pTime->nYear = (WORD)( ( dwMonths / 12 ) + BASETIME );
	pTime->nMonth = (BYTE)( ( dwMonths % 12 ) + 1 );
	pTime->nDay = 1;
	pTime->nHour = 0;
	pTime->nMinute = 0;
	pTime->nSecond = 0;
}

/*
int dayOfWeek(int year,int month,int day)
{   
	int _month[12]={31,0,31,30,31,30,31,31,30,31,30,31};
    if (year%4==0 && year%100!=0 || year%400==0)
       _month[1]=29;
    else _month[1]=28;
    int C=0;
    for (int i=0;i<month-1;++i)
      C+=_month[i];
    C+=day;
    int S=year-1+(year-1)/4-(year-1)/100+(year-1)/400+C;
    return S%7;
}
*/

DWORD TimeToSeconds(const TTime& time)
{
	return DaysFrom2000(time)*60*60*24 +
		   (DWORD )(time.nHour)*60*60 +
		   (DWORD )time.nMinute*60 + time.nSecond;
}

DWORD MilTimeToSeconds(const TMillTime& time)
{
	TTime tTime;
	tTime.nYear = time.nYear;
	tTime.nMonth = time.nMonth;
	tTime.nDay = time.nDay;
	tTime.nHour = time.nHour;
	tTime.nMinute = time.nMinute;
	tTime.nSecond = time.nSecond;

	return DaysFrom2000(tTime)*60*60*24 +
		(DWORD )(tTime.nHour)*60*60 +
		(DWORD )tTime.nMinute*60 + tTime.nSecond;
}

DWORD TimeToMinutes(const TTime& time)
{
	return DaysFrom2000(time)*60*24 +
		   (DWORD )(time.nHour)*60 +
		   time.nMinute;
}

bool IsTimeEmpty(const TTime& time)
{
	if (time.nYear==0 && time.nMonth==0 && time.nDay==0 && 
		time.nHour==0 && time.nMinute==0 && time.nSecond==0)
		return true;
	else
		return false;
}

//描述:是否是不同的一天
bool IsDiffDay(const TTime& time1, const TTime& time2)
{
	if (time1.nDay!=time2.nDay || time1.nMonth!=time2.nMonth || time1.nYear!=time2.nYear)
		return true;
	else
		return false;
}

//描述:是否是同一天
bool IsSameDay(const TTime& time1, const TTime& time2)
{
	if (time1.nDay==time2.nDay && time1.nMonth==time2.nMonth && time1.nYear==time2.nYear)
		return true;
	else
		return false;
}
//描述:是否是同一个月
bool IsSameMon(const TTime& time1, const TTime& time2)
{
	if (time1.nMonth==time2.nMonth && time1.nYear==time2.nYear)
		return true;
	else
		return false;
}

//描述:是否是不同的一天
bool IsDiffHour(const TTime& time1, const TTime& time2)
{
	if (time1.nHour!=time2.nHour || time1.nDay!=time2.nDay || time1.nMonth!=time2.nMonth || time1.nYear!=time2.nYear)
		return true;
	else
		return false;
}

void MinuteToBuf(const TTime& time, BYTE* pbBuf)
{
	*pbBuf++ = ByteToBcd(time.nYear%100);
 	*pbBuf++ = ByteToBcd(time.nMonth);
	*pbBuf++ = ByteToBcd(time.nDay);
	*pbBuf++ = ByteToBcd(time.nHour);
	*pbBuf++ = ByteToBcd(time.nMinute);
}

void BufToMinute(BYTE* pbBuf, TTime& time)
{
	time.nYear = 2000 + BcdToByte(*pbBuf++);
	time.nMonth = BcdToByte(*pbBuf++);
	time.nDay = BcdToByte(*pbBuf++);
	time.nHour = BcdToByte(*pbBuf++);
	time.nMinute = BcdToByte(*pbBuf++);
	time.nSecond = 0;
}

//DayOfWeek()的返回 1 = Sunday, 2 = Monday, ..., 7 = Saturday
BYTE DayOfWeek(const TTime& time)
{
	DWORD dwDays = DaysFrom2000(time);
	WORD nWeek = (WORD )(dwDays % 7);
		
	nWeek = (nWeek + BASEWEEK) % 7;	
	return nWeek + 1;
}

//DayOfWeek()的返回 0 = Sunday, 1 = Monday, ..., 6 = Saturday
BYTE OoDayOfWeek(const TTime& time)
{
	return DayOfWeek(time) - 1;
}


//描述:
//		@iIntervV 递增或递减的时间间隔,取决于正负
bool AddIntervs(TTime& time, BYTE bIntervU, int iIntervV)
{
	int nYear, nMonth;
	DWORD dwSec = TimeToSeconds(time);
	switch (bIntervU)
	{
		case TIME_UNIT_MINUTE:
			dwSec += (DWORD )60*iIntervV;
			SecondsToTime(dwSec, &time);
			return true;
		
		case TIME_UNIT_HOUR:
			dwSec += (DWORD )60*60*iIntervV;
			SecondsToTime(dwSec, &time);
			return true;
		
		case TIME_UNIT_DAY:
			dwSec += (DWORD )24*60*60*iIntervV;
			SecondsToTime(dwSec, &time);
			return true;
		
		case TIME_UNIT_MONTH: 
			nYear = iIntervV / 12;
			time.nYear = time.nYear + nYear;
			
			iIntervV = iIntervV % 12;
			nMonth = iIntervV + time.nMonth;   //先把month当有符号数看待
			if (nMonth > 12)
			{
				time.nYear++;
				nMonth -= 12;
			}
			else if (nMonth < 1)
			{
				time.nYear--;
				nMonth += 12;
			}
			
			time.nMonth = nMonth;
			
			return true;
			
		default: return false;
	}
	
	return false;
}


//描述:求现在的时间tmNow相对过去的时间tmPast已经消逝的采样间隔个数,本函数会把间隔单位以下
//		的时间忽略掉,比如如果间隔时间是天,则把时分秒忽略掉
int IntervsPast(const TTime& tmPast, const TTime& tmNow, BYTE bIntervU, BYTE bIntervV)
{
	if (bIntervV == 0)
		return 0;
		
	DWORD dwPast = TimeToSeconds(tmPast);
	DWORD dwNow = TimeToSeconds(tmNow);
	TTime now, past;
	int iSign = 1;
	if (dwNow == dwPast)
	{
		return 0;
	}
	else if (dwNow > dwPast)
	{
		now = tmNow;
		past = tmPast;
	}
	else
	{
		iSign = -1;
		now = tmPast;
		past = tmNow;
	}
		
	switch (bIntervU)
	{
		case TIME_UNIT_MINUTE:
			return iSign * (int )(MinutesPast(past, now) / bIntervV);
		
		case TIME_UNIT_HOUR:
			return iSign * (int )(HoursPast(past, now) / bIntervV);
		
		case TIME_UNIT_DAY:
			return iSign * (int )(DaysPast(past, now) / bIntervV);
		
		case TIME_UNIT_MONTH:
			return iSign * (int )(MonthsPast(past, now) / bIntervV);
		default: return 0;
	}
	
	return 0;
}

//描述:获取当月的天数
BYTE DaysOfMonth(TTime time)
{
	TTime tm = time;
	DWORD dwDays1, dwDays2;

	tm.nDay = 1;	
	dwDays1 = DaysFrom2000(tm);
	
	if (tm.nMonth == 12)
	{
		tm.nYear ++;
		tm.nMonth = 1;
	}
	else
		tm.nMonth ++;
	dwDays2 = DaysFrom2000(tm);

	return (BYTE)(dwDays2-dwDays1);
}

DWORD GetMonthStart(TTime time)
{
	time.nDay = 1;
	time.nHour = 0;
	time.nMinute = 0;
	time.nSecond = 0;

	return TimeToSeconds(time);
}

DWORD GetMonthEnd(TTime time)
{
	time.nDay = 1;
	time.nHour = 0;
	time.nMinute = 0;
	time.nSecond = 0;
	AddIntervs(time, TIME_UNIT_MONTH, 1);

	return TimeToSeconds(time);
}

//描述：某时刻对应的间隔的开始时间及间隔结束时间的秒数
//@time		某时刻的时间
//@bType	要转换的间隔的类型
//@dwStartS 返回起本间隔类型的起始时间的秒数
//@dwEndS   返回起本间隔类型的结束时间的秒数
//@dwIntvT  抄表间隔时间的分钟数，只用于间隔类型为分钟或小时时（如抄表间隔或15分钟间隔）,其他日月可缺省不输入
void TimeToIntervS(TTime time, BYTE bType, DWORD& dwStartS, DWORD& dwEndS, DWORD dwIntvT)
{
	TTime tm = time;
	switch (bType)
	{
		case TIME_UNIT_MINUTE:
			if (dwIntvT != 0)
			{
				dwStartS = TimeToMinutes(tm)/dwIntvT*dwIntvT*60;
				dwEndS = dwStartS+dwIntvT*60;
			}
			break;	
		case TIME_UNIT_HOUR:
			if (dwIntvT != 0)
			{
				dwStartS = TimeToMinutes(tm)/(60*dwIntvT)*dwIntvT*60*60;
				dwEndS = dwStartS+dwIntvT*60*60;
			}
			break;	
		case TIME_UNIT_DAY:
			tm.nHour = 0;
			tm.nMinute = 0;
			tm.nSecond = 0;
			dwStartS = TimeToSeconds(tm);	
			dwEndS = dwStartS+24*60*60;
			break;
		case TIME_UNIT_MONTH:
			tm.nDay = 1;
			tm.nHour = 0;
			tm.nMinute = 0;
			tm.nSecond = 0;
			dwStartS = TimeToSeconds(tm);	
			dwEndS = dwStartS+(DWORD)DaysOfMonth(tm)*24*60*60;	
			break;
		default:
			break;
	}
}


bool WriteFile(char* pszPathName, BYTE* pbData, DWORD dwLen)
{
	bool fRet = true;
	int f = open(pszPathName, O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE);  
			//windows下必须使用O_BINARY,否则读出来的长度为空,其它系统下O_BINARY为空
    
    if (f < 0)
	{
	    //DTRACE(DB_GLOBAL, ("SaveFile : error : fail to open %s.\r\n", pszPathName));
		return 	false;
	}
	else
	{
		int nFileSize = lseek(f, 0, SEEK_END);
		if (nFileSize > (int )dwLen)
		{
			close(f);
			unlink(pszPathName);   //删除
			f = open(pszPathName, O_CREAT|O_RDWR, S_IREAD|S_IWRITE);  //重新创建 , S_IREAD|S_IWRITE
			if (f < 0)
				return false;
		}
		else
		{
			lseek(f, 0, SEEK_SET);
		}

	    if (write(f, pbData, dwLen) != (int )dwLen)
		{
		    DTRACE(DB_GLOBAL, ("SaveFile : error:  fail to write %s .\r\n", pszPathName));
			fRet = false;
		}
	}
    close(f);

    return fRet;
}

bool PartWriteFile(char* pszPathName, DWORD dwOff,BYTE* pbData, DWORD wLen)
{
	bool fRet = true;

	int f = open(pszPathName, O_CREAT|O_RDWR|O_BINARY, S_IREAD|S_IWRITE);
    if (f < 0)
	{
		//DTRACE(DB_GLOBAL, ("SaveFile : error : fail to open %s.\r\n", pszPathName));
		return 	false;
	}
	else
	{
		lseek(f, dwOff, SEEK_SET);
	    if (write(f, pbData, wLen) != (int )wLen)
		{			
			DTRACE(DB_GLOBAL, ("SaveFile : error:  fail to write %s .\r\n", pszPathName));
			fRet = false;
		}
	}
    close(f);

#ifdef SYS_WIN32
		FILE *p;
		char name[200];
		GetWin32FileName(pszPathName,name);
		if( (p=fopen(name,"w+"))!=NULL )
		{
			fseek(p,dwOff,SEEK_SET);
			fwrite(pbData,1,wLen,p);
			fclose(p);
		}
#endif
    return fRet;
}

bool PartReadFile(char* pszPathName, DWORD dwOffset, BYTE *pbData, DWORD dwLen)
{
	bool fRet = true;

	int f = open(pszPathName, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);
	if (f < 0)
	{
		DTRACE(DB_GLOBAL, ("PartReadFile : error : fail to open %s.\r\n", pszPathName));
		return 	false;
	}
	else
	{
		int nFileSize = lseek(f,0,SEEK_END);
		if (dwOffset>nFileSize)//读的偏移量超过了文件的大小
		{
			close(f);
			return false;
		}
		lseek(f, dwOffset, SEEK_SET);
		if (read(f, pbData, dwLen) != (int )dwLen)
		{			
			DTRACE(DB_GLOBAL, ("PartRaedFile : error:  fail to read %s .\r\n", pszPathName));
			fRet = false;
		}
	}
	close(f);

	return fRet;
}

bool DeleteFile(char* pszPathName)
{
	bool fRet = true;
	int f = open(pszPathName, O_RDWR, S_IREAD|S_IWRITE);
    if (f < 0)return true;

	close(f);
	if( unlink(pszPathName) < 0 )   //删除
		return false;
	else
	{
#ifdef SYS_WIN32
		FILE *p;
		char name[200];
		GetWin32FileName(pszPathName,name);
		//delete(name);		
		if( (p=fopen(name,"w"))!=NULL )
		{
			fclose(p);
		}
#endif
		return true;
	}
}

bool ReadFile(char* pszPathName, BYTE* pbData, DWORD dwBytesToRead)
{
	bool fRet = false;
	int nFileSize;
    int f = open(pszPathName, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);  //,|O_BINARY S_IREAD|S_IWRITE
	if (f >= 0)
	{
		nFileSize = lseek(f, 0, SEEK_END);
		if (nFileSize == dwBytesToRead)
		{                                   
			lseek(f, 0, SEEK_SET);
			if (read(f, pbData, dwBytesToRead) == dwBytesToRead)
			{
				fRet = true;
			}
			else
			{
				DTRACE(DB_GLOBAL, ("ReadFile : error : fail to read %s.\r\n", pszPathName));
			}
			close(f);
		}
		else
		{	
			close(f);
			unlink(pszPathName);   //删除
		}
	}

	return fRet;
}

//描述:读取文件,文件长度不符合的时候不删除文件
//参数:@pszPathName 路径文件名
//	   @pbData 存放读到的文件内容
//	   @dwMaxBytesToRead 最大允许的文件长度
//返回:如果正确则返回读到的长度,否则返回-1表示文件打开或者读取错误,-2表示文件过大
int readfile(char* pszPathName, BYTE* pbData, DWORD dwMaxBytesToRead)
{
	int iRet = -1;
	int nFileSize;
    int f = open(pszPathName, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);  //,|O_BINARY S_IREAD|S_IWRITE
	if (f >= 0)
	{
		nFileSize = lseek(f, 0, SEEK_END);
		if (nFileSize <= (int )dwMaxBytesToRead)
		{                                   
			lseek(f, 0, SEEK_SET);
			if (read(f, pbData, nFileSize) == nFileSize)	
			{
				iRet = nFileSize;
			}
			else
			{
				DTRACE(DB_GLOBAL, ("ReadFile : error : fail to read %s.\r\n", pszPathName));
			}
			close(f);
		}
		else
		{	
			iRet = -2;
			close(f);
			unlink(pszPathName);   //删除
		}
	}

	return iRet;
}

void SearchPost(BYTE bFlag, BYTE bPhase, TTime time, BYTE* pbBuf)
{
	BYTE i = 0;
	BYTE bBuf[512];
	if(pbBuf[0] > 0 && (bFlag&0x80)==0x00)
	{
		for(i=0; i<40; i++)
		{
			if(pbBuf[11*i+1]==bPhase && IsAllAByte(pbBuf+11*i+7, INVALID_DATA, 5))
			{
				TimeToFmt15(time, pbBuf+11*i+7);
				break;
			}
		}

		if(i!=40 && i!=0)
		{
			bBuf[0] = pbBuf[0];
			//bBuf[1] = bPhase;
			//TimeToFmt15(time, bBuf+2);
			//memset(bBuf+7, 0xee, 5);

			memcpy(bBuf+1, pbBuf+11*i+1, 11);
			memcpy(bBuf+12, pbBuf+1, i*11);
			memcpy(bBuf+11*(i+1)+1, pbBuf+11*(i+1)+1, (39-i-1)*11);
			memcpy(pbBuf, bBuf, 512);
		}
	}
	else
	{
		bBuf[0] = pbBuf[0]+1;
		if(bBuf[0] > 40)
			bBuf[0] = 40;

		bBuf[1] = bPhase;

		TimeToFmt15(time, bBuf+2);
		memset(bBuf+7, 0xee, 5);

		memcpy(bBuf+12, pbBuf+1, 429);
		memcpy(pbBuf, bBuf, 512);
	}

}

int GetFileLen(char* pszPathName)
{
	int iFileSize = -1;
    int f = open(pszPathName, O_RDWR, S_IREAD|S_IWRITE); //, S_IREAD|S_IWRITE
	if (f >= 0)
	{
		iFileSize = lseek(f, 0, SEEK_END);
		close(f);
	}
	
	return iFileSize;
}

/*
 * FCS lookup table as calculated by genfcstab.
 */
static const u_short fcstab[256] = {
    0x0000, 0x1189, 0x2312, 0x329b, 0x4624, 0x57ad, 0x6536, 0x74bf,
    0x8c48, 0x9dc1, 0xaf5a, 0xbed3, 0xca6c, 0xdbe5, 0xe97e, 0xf8f7,
    0x1081, 0x0108, 0x3393, 0x221a, 0x56a5, 0x472c, 0x75b7, 0x643e,
    0x9cc9, 0x8d40, 0xbfdb, 0xae52, 0xdaed, 0xcb64, 0xf9ff, 0xe876,
    0x2102, 0x308b, 0x0210, 0x1399, 0x6726, 0x76af, 0x4434, 0x55bd,
    0xad4a, 0xbcc3, 0x8e58, 0x9fd1, 0xeb6e, 0xfae7, 0xc87c, 0xd9f5,
    0x3183, 0x200a, 0x1291, 0x0318, 0x77a7, 0x662e, 0x54b5, 0x453c,
    0xbdcb, 0xac42, 0x9ed9, 0x8f50, 0xfbef, 0xea66, 0xd8fd, 0xc974,
    0x4204, 0x538d, 0x6116, 0x709f, 0x0420, 0x15a9, 0x2732, 0x36bb,
    0xce4c, 0xdfc5, 0xed5e, 0xfcd7, 0x8868, 0x99e1, 0xab7a, 0xbaf3,
    0x5285, 0x430c, 0x7197, 0x601e, 0x14a1, 0x0528, 0x37b3, 0x263a,
    0xdecd, 0xcf44, 0xfddf, 0xec56, 0x98e9, 0x8960, 0xbbfb, 0xaa72,
    0x6306, 0x728f, 0x4014, 0x519d, 0x2522, 0x34ab, 0x0630, 0x17b9,
    0xef4e, 0xfec7, 0xcc5c, 0xddd5, 0xa96a, 0xb8e3, 0x8a78, 0x9bf1,
    0x7387, 0x620e, 0x5095, 0x411c, 0x35a3, 0x242a, 0x16b1, 0x0738,
    0xffcf, 0xee46, 0xdcdd, 0xcd54, 0xb9eb, 0xa862, 0x9af9, 0x8b70,
    0x8408, 0x9581, 0xa71a, 0xb693, 0xc22c, 0xd3a5, 0xe13e, 0xf0b7,
    0x0840, 0x19c9, 0x2b52, 0x3adb, 0x4e64, 0x5fed, 0x6d76, 0x7cff,
    0x9489, 0x8500, 0xb79b, 0xa612, 0xd2ad, 0xc324, 0xf1bf, 0xe036,
    0x18c1, 0x0948, 0x3bd3, 0x2a5a, 0x5ee5, 0x4f6c, 0x7df7, 0x6c7e,
    0xa50a, 0xb483, 0x8618, 0x9791, 0xe32e, 0xf2a7, 0xc03c, 0xd1b5,
    0x2942, 0x38cb, 0x0a50, 0x1bd9, 0x6f66, 0x7eef, 0x4c74, 0x5dfd,
    0xb58b, 0xa402, 0x9699, 0x8710, 0xf3af, 0xe226, 0xd0bd, 0xc134,
    0x39c3, 0x284a, 0x1ad1, 0x0b58, 0x7fe7, 0x6e6e, 0x5cf5, 0x4d7c,
    0xc60c, 0xd785, 0xe51e, 0xf497, 0x8028, 0x91a1, 0xa33a, 0xb2b3,
    0x4a44, 0x5bcd, 0x6956, 0x78df, 0x0c60, 0x1de9, 0x2f72, 0x3efb,
    0xd68d, 0xc704, 0xf59f, 0xe416, 0x90a9, 0x8120, 0xb3bb, 0xa232,
    0x5ac5, 0x4b4c, 0x79d7, 0x685e, 0x1ce1, 0x0d68, 0x3ff3, 0x2e7a,
    0xe70e, 0xf687, 0xc41c, 0xd595, 0xa12a, 0xb0a3, 0x8238, 0x93b1,
    0x6b46, 0x7acf, 0x4854, 0x59dd, 0x2d62, 0x3ceb, 0x0e70, 0x1ff9,
    0xf78f, 0xe606, 0xd49d, 0xc514, 0xb1ab, 0xa022, 0x92b9, 0x8330,
    0x7bc7, 0x6a4e, 0x58d5, 0x495c, 0x3de3, 0x2c6a, 0x1ef1, 0x0f78
};

WORD pppfcs16(WORD fcs,unsigned char * cp,int len)
{
	while (len--)
		fcs = (fcs >> 8) ^ fcstab[(fcs ^ *cp++) & 0xff];
	return (fcs);
}

WORD CheckCrc16(unsigned char *pbyt,int iLen)
{
	WORD trialfcs;

	trialfcs = pppfcs16( PPPINITFCS16, pbyt, iLen );
	trialfcs ^= 0xffff; 
	return trialfcs;
}

static unsigned short crc_16_table[16] = {
  0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
  0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
};
  

WORD get_crc_16 (WORD start, BYTE *p, int n)
{ 
	WORD crc = start;
	register WORD r;
	/* while there is more data to process */
	while (n-- > 0) 
	{
		/* compute checksum of lower four bits of *p */
		r = crc_16_table[crc & 0xF];
		crc = (crc >> 4) & 0x0FFF;
		crc = crc ^ r ^ crc_16_table[*p & 0xF];
		
		/* now compute checksum of upper four bits of *p */
		r = crc_16_table[crc & 0xF];
		crc = (crc >> 4) & 0x0FFF;
		crc = crc ^ r ^ crc_16_table[(*p >> 4) & 0xF];
		
		/* next... */
		p++;
	}
   return(crc);
}


#define CRC_CODE ((unsigned long)0x04C11DB7)

DWORD get_crc_32 (BYTE * data, int data_len)
{
	DWORD crc = 0x00000000;
	int i,j;
	BYTE dat;
	for (i=0;i<data_len;i++)
	{
		dat =  data[i];
		crc ^= ((DWORD)dat)<<24;
		for (j=0;j<8;j++)
		{
			if (crc&0x80000000)
			{ 
				crc <<=1;
				crc ^=CRC_CODE;
			}
			else  crc <<=1;
		}
	}
	return crc;
}

//CRC-8/MAXIM         x8+x5+x4+1
static const unsigned char CRC8_TAB[256] = 
{  
	0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83, 
	0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41, 
	0x9d, 0xc3, 0x21, 0x7f, 0xfc, 0xa2, 0x40, 0x1e, 
	0x5f, 0x01, 0xe3, 0xbd, 0x3e, 0x60, 0x82, 0xdc, 
	0x23, 0x7d, 0x9f, 0xc1, 0x42, 0x1c, 0xfe, 0xa0, 
	0xe1, 0xbf, 0x5d, 0x03, 0x80, 0xde, 0x3c, 0x62, 
	0xbe, 0xe0, 0x02, 0x5c, 0xdf, 0x81, 0x63, 0x3d, 
	0x7c, 0x22, 0xc0, 0x9e, 0x1d, 0x43, 0xa1, 0xff, 
	0x46, 0x18, 0xfa, 0xa4, 0x27, 0x79, 0x9b, 0xc5, 
	0x84, 0xda, 0x38, 0x66, 0xe5, 0xbb, 0x59, 0x07, 
	0xdb, 0x85, 0x67, 0x39, 0xba, 0xe4, 0x06, 0x58, 
	0x19, 0x47, 0xa5, 0xfb, 0x78, 0x26, 0xc4, 0x9a, 
	0x65, 0x3b, 0xd9, 0x87, 0x04, 0x5a, 0xb8, 0xe6, 
	0xa7, 0xf9, 0x1b, 0x45, 0xc6, 0x98, 0x7a, 0x24, 
	0xf8, 0xa6, 0x44, 0x1a, 0x99, 0xc7, 0x25, 0x7b, 
	0x3a, 0x64, 0x86, 0xd8, 0x5b, 0x05, 0xe7, 0xb9, 
	0x8c, 0xd2, 0x30, 0x6e, 0xed, 0xb3, 0x51, 0x0f, 
	0x4e, 0x10, 0xf2, 0xac, 0x2f, 0x71, 0x93, 0xcd, 
	0x11, 0x4f, 0xad, 0xf3, 0x70, 0x2e, 0xcc, 0x92, 
	0xd3, 0x8d, 0x6f, 0x31, 0xb2, 0xec, 0x0e, 0x50, 
	0xaf, 0xf1, 0x13, 0x4d, 0xce, 0x90, 0x72, 0x2c, 
	0x6d, 0x33, 0xd1, 0x8f, 0x0c, 0x52, 0xb0, 0xee, 
	0x32, 0x6c, 0x8e, 0xd0, 0x53, 0x0d, 0xef, 0xb1, 
	0xf0, 0xae, 0x4c, 0x12, 0x91, 0xcf, 0x2d, 0x73, 
	0xca, 0x94, 0x76, 0x28, 0xab, 0xf5, 0x17, 0x49, 
	0x08, 0x56, 0xb4, 0xea, 0x69, 0x37, 0xd5, 0x8b, 
	0x57, 0x09, 0xeb, 0xb5, 0x36, 0x68, 0x8a, 0xd4, 
	0x95, 0xcb, 0x29, 0x77, 0xf4, 0xaa, 0x48, 0x16, 
	0xe9, 0xb7, 0x55, 0x0b, 0x88, 0xd6, 0x34, 0x6a, 
	0x2b, 0x75, 0x97, 0xc9, 0x4a, 0x14, 0xf6, 0xa8, 
	0x74, 0x2a, 0xc8, 0x96, 0x15, 0x4b, 0xa9, 0xf7, 
	0xb6, 0xe8, 0x0a, 0x54, 0xd7, 0x89, 0x6b, 0x35 
}; 

unsigned char CRC8_Tab(unsigned char* ucPtr, unsigned char ucLen)
{ 
	//查表计算方法
	unsigned char ucIndex; //CRC8校验表格索引
	unsigned char ucCRC8 = 0; //CRC8字节初始化

	//进行CRC8位校验
	while (ucLen--) 
	{ 
		ucIndex = ucCRC8 ^(*ucPtr);
		ucPtr++;
		ucCRC8 = CRC8_TAB[ucIndex];
	}

	return(ucCRC8);	//返回CRC8校验数据
}

//描述:逆序拷贝
void revcpy(BYTE* pbDst, const BYTE* pbSrc, WORD wLen)
{
	pbSrc += wLen-1;
	for (WORD i=0; i<wLen; i++)
	{
		*pbDst++ = *pbSrc--;
	}
}

//描述:逆序比较
//返回:< 0 buf1 less than buf2
//		 0 buf1 identical to buf2
//	   > 0 buf1 greater than buf2
int revcmp(const void *buf1, const void *buf2, int count)
{
	const BYTE* p1 = (BYTE* )buf1;
	const BYTE* p2 = (BYTE* )buf2;
	
	p1 += count-1;
	p2 += count-1;

	for (; count>0; count--)
	{
		if (*p1 > *p2)
			return 1;
		else if (*p1 < *p2)
			return -1;

		p1--;
		p2--;
	}

	return 0;
}

//描述:对缓冲区按字节进行倒序拷贝
void RevBuf(BYTE* pbBuf, WORD wLen)
{
	BYTE* pbEnd = pbBuf + wLen - 1;
	wLen /= 2;

	BYTE b;
	for (WORD i=0; i<wLen; i++,pbBuf++,pbEnd--)
	{
		b = *pbBuf;
		*pbBuf = *pbEnd;
		*pbEnd = b;	
	}
}

BYTE memrcpy(BYTE *pbDst,BYTE *pbSrc,WORD wLen)
{
	for (int i=0;i<wLen;i++) 
	{
		pbDst[i] = pbSrc[wLen-1-i];
	}
	return wLen;
}

//将一维数组元素倒序
void Swap(BYTE *pbBuf, WORD wLen)
{
	BYTE bTemp;
	WORD wSwapTimes = wLen>>1;
	for (WORD i=0; i<wSwapTimes; i++)
	{
		bTemp = pbBuf[i];
		pbBuf[i] = pbBuf[wLen-i-1];
		pbBuf[wLen-i-1] = bTemp;
	}
}

//X-DER 根据长度完成长度编码
BYTE EncodeLength(int len, BYTE *p)
{
	if (len < 128)
	{
		*p = len;
		return 1;
	}
	else
	{
		BYTE i, k=4, ch[4];
		ch[0] = (BYTE)(len>>24);
		ch[1] = (BYTE)(len>>16);
		ch[2] = (BYTE)(len>>8);
		ch[3] = (BYTE)len;

		for (i=0; i<4; i++)
		{
			if (ch[i] == 0)
				k--;
			else
				break;
		}

		*p++ = 0x80|k;
		for (i=0; i<k; i++)
		{
			*p++ = ch[4-k+i];
		}

		return k+1;
	}
}

//X-DER 对长度完成解码
int DecodeLength(BYTE *pbBuf,DWORD *pdwNum)
{
	if ( *pbBuf < 128 )
	{
		*pdwNum = *pbBuf;
		return 1;
	}
	else
	{
		BYTE n=(*pbBuf++)&0x7f;
		if ( n > 4 )
			return -1;

		DWORD val=0;
		for ( BYTE i = 0; i < n; i++ )
		{
			val = (val<<8)|(*pbBuf++);
		}
		*pdwNum = val;
		return (n+1);
	}
}

//对字节按位进行倒序
BYTE ByteBitReverse(BYTE b)
{
	BYTE b0 = 0;
	BYTE bMark = 0x80;

	for (BYTE b1=8; b1>0; b1--)
	{
		if ((b&0x01) == 0x01)
			b0 |= bMark;

		bMark >>= 1;
		b >>= 1;
	}

	return b0;
}

//对字节串以字节为单位进行倒序
void BufByteReverse(BYTE *bBuf, WORD wLen)
{
	BYTE *bBuf0 = bBuf+wLen-1;

	while (bBuf0 > bBuf)
	{
		*bBuf ^= *bBuf0;
		*bBuf0 ^= *bBuf;
		*bBuf++ ^= *bBuf0--;
	}
}

//对字节串以位为单位进行倒序
void BufBitReverse(BYTE *bBuf, WORD wLen)
{
	BufByteReverse(bBuf, wLen);
	while (wLen-- > 0)
	{
		*bBuf = ByteBitReverse(*bBuf);
		bBuf++;
	}
}

int64 Pow(int iBase, WORD wExp)
{
	int64 iVal=1;
	for (WORD i=0; i<wExp; i++)
	{
		iVal *= iBase;
	}
	
	return iVal;
}

//描述:把时间转换成可以显示的字符串
//参数:@time 待转换的时间
//	   @psz 用来存放转换后的字符串
//返回:转换后得到的字符串
char* TimeToStr(const TTime& time, char* psz)
{
	sprintf(psz, "%04d-%02d-%02d %02d:%02d:%02d", 
			time.nYear, time.nMonth, time.nDay, 
			time.nHour, time.nMinute, time.nSecond);
			
	return psz;
}

//描述:把时间转换成可以显示的字符串
//参数:@dwTime	待转换的时间
//	   @psz		用来存放转换后的字符串
//返回:转换后得到的字符串
char* TimeToStr(DWORD dwTime, char* psz)
{
	TTime tmTime;

	SecondsToTime(dwTime, &tmTime);

	return TimeToStr(tmTime, psz);
}

//描述:把带毫秒的时间转换成可以显示的字符串
//参数:@time 待转换的时间
//	   @psz 用来存放转换后的字符串
//返回:转换后得到的字符串
char* MillTimeToStr(TMillTime tMillTime, char *psz)
{
	sprintf(psz, "%04d-%02d-%02d %02d:%02d:%02d.%03d", 
		tMillTime.nYear, tMillTime.nMonth, tMillTime.nDay, 
		tMillTime.nHour, tMillTime.nMinute, tMillTime.nSecond,
		tMillTime.nMilliseconds);

	return psz;
}

char* MtrAddrToStr(const BYTE* pbAddr, char* psz)
{
	sprintf(psz, "%02x%02x%02x%02x%02x%02x", 
			pbAddr[5], pbAddr[4], pbAddr[3], pbAddr[2], pbAddr[1], pbAddr[0]);

	return psz;
}

//描述:把申请的内存类型转换成可以显示的字符串
//参数:@bType	待转换的类型
//	   @psz		用来存放转换后的字符串
//返回:转换后得到的字符串
char* MemTypeToStr(BYTE bType, char* psz)
{
	switch (bType)
	{
	case MEM_TYPE_TASK:
		sprintf(psz, "MEM_TYPE_TASK");
		break;

	case MEM_TYPE_MTREXC:
		sprintf(psz, "MEM_TYPE_MTREXC");
		break;

	case MEM_TYPE_EVT_ACQ:
		sprintf(psz, "MEM_TYPE_EVT_ACQ");
		break;

	case MEM_TYPE_TERM_EVTREC:
		sprintf(psz, "MEM_TYPE_TERM_EVTREC");
		break;

	case MEM_TYPE_TERM_EVTITEM:
		sprintf(psz, "MEM_TYPE_TERM_EVTITEM");
		break;

	case MEM_TYPE_CURVE_FLG:
		sprintf(psz, "MEM_TYPE_CURVE_FLG");
		break;

	default:
		sprintf(psz, "MEM_TYPE_NONE");
		break;
	}

	return psz;
}

BYTE* GetSubPos(BYTE *src, WORD wSrcLen, BYTE *dst, WORD wDstLen)
{
	int i, j;
	for (i=0; i<=wSrcLen-wDstLen; i++)
	{
		if (src[i] == dst[0])
		{
			j=1;
			while(src[i+j]==dst[j] && j<wDstLen)
				j++;
			if (j==wDstLen)
				return src+i;
		}
	}
	return NULL;
}


//描述 :将时间TTime格式转换为DateTime格式
void  TimeToDateTime(const TTime &time, BYTE *pbBuf)
{		 
	if (ReadItemEx(BN0, PN0, 0x4512, pbBuf) != 1)//时钟状态
		*pbBuf = 0xFF;

	pbBuf++;

	*pbBuf++ = 0x00;		//偏差低字节
	*pbBuf++ = 0x80;		//偏差高字节
	*pbBuf++ = 0xFF;
	*pbBuf++ = (BYTE)time.nSecond;
	*pbBuf++ = (BYTE)time.nMinute;
	*pbBuf++ = (BYTE)time.nHour;
	*pbBuf++ = (BYTE)time.nWeek;
	*pbBuf++ = (BYTE)time.nDay;
	*pbBuf++ = (BYTE)time.nMonth;
	*pbBuf++ = time.nYear & 0xff;
	*pbBuf   = (time.nYear >> 8) & 0xff;
}

//描述 :将时间DateTime格式转换为TTime格式
void  DateTimeToTime(BYTE *pbBuf, TTime &time)
{
	time.nYear = OoOiToWord(pbBuf);
	pbBuf += 2;
	time.nMonth = *pbBuf++;
	time.nDay = *pbBuf++;
	time.nHour = *pbBuf++;
	time.nMinute = *pbBuf++;
	time.nSecond = *pbBuf++;
#if 0	//以下代码先屏蔽，与主站访问任务库的时间顺序相反 CL 20161117
	pbBuf += 4;//偏差等字节
	time.nSecond = *pbBuf++;
	time.nMinute = *pbBuf++;
	time.nHour   = *pbBuf++;
	time.nWeek   = *pbBuf++;
	time.nDay	 = *pbBuf++;
	time.nMonth  = *pbBuf++;
	time.nYear	 =  *pbBuf + (*(pbBuf+1)<<8);	
	if (time.nWeek == 0xff)
	{
		time.nWeek = DayOfWeek(time);
	}
#endif
}

int ByteToInt(BYTE* p, WORD wLen)
{
	int	nRet;
	bool fNeg;
	if ((*(p+wLen-1))&0x80)
		fNeg = true;
	else
		fNeg = false;

	char bBuf[4] = {0};
	memcpy(bBuf, p, wLen);
	if (wLen < 4)
	{
		if (fNeg)
			memset(bBuf+wLen, 0xff, 4-wLen);
	}
	memcpy(&nRet, bBuf, 4);
	return (int )nRet;
}

//描述:计算一个缓冲区中的比特为1的位的个数
WORD CalcuBitNum(const BYTE* pbBuf, WORD wSize)
{
	static WORD wBitNumArr[16] = {0, 1, 1, 2, 1, 2, 2, 3, //0~7
								  1, 2, 2, 3, 2, 3, 3, 4};//8~15
	
	WORD wBitNum = 0;
	BYTE b;
	for (WORD i=0; i<wSize; i++)
	{
		b = *pbBuf++;
		wBitNum += wBitNumArr[b&0x0f];
		wBitNum += wBitNumArr[(b>>4)&0x0f];
	}
	
	return wBitNum;
}

//描述:两个缓冲区与操作,结果返回到第一个缓冲区
void AndTwoBuf(BYTE* p1, const BYTE* p2, WORD wSize)
{
	for (WORD i=0; i<wSize; i++, p1++, p2++)
	{
		*p1 = *p1 & *p2;
	}
}

//描述:两个缓冲区异或操作,结果返回到第一个缓冲区
void XorTwoBuf(BYTE* p1, const BYTE* p2, WORD wSize)
{
	for (WORD i=0; i<wSize; i++, p1++, p2++)
	{
		*p1 = *p1 ^ *p2;
	}
}

//描述:两个缓冲区或操作,结果返回到第一个缓冲区
void OrTwoBuf(BYTE* p1, const BYTE* p2, WORD wSize)
{
	for (WORD i=0; i<wSize; i++, p1++, p2++)
	{
		*p1 = *p1 | *p2;
	}
}

void TestCommPort(WORD wPort, DWORD dwBaudRate)
{
	printf("testing comm%d\n", wPort); 
	
	CComm comm;
	comm.Open(wPort, dwBaudRate, 8, ONESTOPBIT, NOPARITY);
	
	BYTE bBuf[100];
	BYTE b = 0;
	while (1)
	{
		for (WORD i=0; i<100; i++)
			bBuf[i] = b++;
			
		comm.Write(bBuf, 100);
		Sleep(1000);
	}
}

//描述:组645抄读数据帧
BYTE Make645AskItemFrm(BYTE bMtrPro, BYTE* pbAddr, DWORD dwID, BYTE* pbFrm, BYTE bCtrl)
{ 
	WORD i;
	//WORD wPn = PlcAddrToPn(pbAddr, NULL);
	BYTE bDataLen, bCS = 0;

    pbFrm[0] = 0x68;
	memcpy(pbFrm+1, pbAddr, 6);
    pbFrm[7] = 0x68;

	pbFrm[8] = bCtrl;

	if (bMtrPro == CCT_MTRPRO_07)	
	{
		if(bCtrl==0)
			pbFrm[8] = 0x11;		//命令

		memcpy(pbFrm+10, &dwID, 4);
		bDataLen = 4;
	}
	else
    {
		if(bCtrl==0)
			pbFrm[8] = 0x01;		//命令

		memcpy(pbFrm+10, &dwID, 2);
		bDataLen = 2;
	}

	//bDataLen = MeterIdToBuf(wPn, dwID, pbFrm+10, m_pStdPara->RdrPara.bMtrPro);
	pbFrm[9] = bDataLen;

	for (i=0; i<bDataLen; i++)
		pbFrm[10+i] += 0x33;

	for (i=0; i<10+bDataLen; i++)
		bCS += pbFrm[i];

	pbFrm[10+bDataLen] = bCS;
	pbFrm[11+bDataLen] = 0x16;

	return 12+bDataLen;
}

//在pbBuf后面增加6字节采集器地址并+33
bool AddAcqAddress(BYTE *pbBuf, BYTE *pbAcqAddr)
{
  if(pbBuf==NULL || pbAcqAddr==NULL)
	  return false;

  for(BYTE b=0; b<6;b++)
	  *pbBuf++ = (*pbAcqAddr++) + 0x33;
}

//描述:组645写数据帧
BYTE Make645WriteItemFrm(BYTE bMtrPro, BYTE* pbAddr, DWORD dwID, BYTE* pbFrm,BYTE* pbData,BYTE bLen)
{ 
	WORD i;
	//WORD wPn = PlcAddrToPn(pbAddr, NULL);
	BYTE bDataLen, bCS = 0;

    pbFrm[0] = 0x68;
	memcpy(pbFrm+1, pbAddr, 6);
    pbFrm[7] = 0x68;

	if (bMtrPro == CCT_MTRPRO_07)	
	{
		pbFrm[8] = 0x14;		//命令
		memcpy(pbFrm+10, &dwID, 4);
        memcpy(pbFrm+14, pbData, bLen);
		bDataLen = 4+bLen;
	}
	else
    {
		pbFrm[8] = 0x04;		//命令
		memcpy(pbFrm+10, &dwID, 2);
        memcpy(pbFrm+12, pbData, bLen);
		bDataLen = 2+bLen;
	}

	//bDataLen = MeterIdToBuf(wPn, dwID, pbFrm+10, m_pStdPara->RdrPara.bMtrPro);
	pbFrm[9] = bDataLen;

	for (i=0; i<bDataLen; i++)
		pbFrm[10+i] += 0x33;

	for (i=0; i<10+bDataLen; i++)
		bCS += pbFrm[i];

	pbFrm[10+bDataLen] = bCS;
	pbFrm[11+bDataLen] = 0x16;

	return 12+bDataLen;
}

//描述:组645帧,数据已经放到帧中
//返回:帧长度
WORD Make645Frm(BYTE* pbFrm, const BYTE* pbAddr, BYTE bCmd, BYTE bDataLen)
{
	pbFrm[0] = 0x68;
	memcpy(&pbFrm[1], pbAddr, 6);
	pbFrm[7] = 0x68;
	pbFrm[8] = bCmd;
	pbFrm[9] = bDataLen;

    //+0x33
    for (WORD i=10; i<(WORD)bDataLen+10; i++)
	{
  	    pbFrm[i] += 0x33;
	}	 
	
	pbFrm[10+(WORD)bDataLen] = CheckSum(pbFrm, (WORD)bDataLen+10);
	pbFrm[11+(WORD)bDataLen] = 0x16;

	return bDataLen+12;
}  

//描述:从645读数据帧中提取数据项ID,帧中的0x33还没减去
//返回:数据项ID
DWORD GetIdFrom645AskItemFrm(BYTE* pbFrm)
{
	DWORD dw = 0;
	BYTE* p = (BYTE *)&dw;
	if (pbFrm[8] == 0x11) //07协议
	{
		*p++ = pbFrm[10]-0x33;
		*p++ = pbFrm[11]-0x33;
		*p++ = pbFrm[12]-0x33;
		*p = pbFrm[13]-0x33;
	}
	else	//97协议
	{
		*p++ = pbFrm[10]-0x33;
		*p++ = pbFrm[11]-0x33;
	}

	return dw;
}


bool IsMountedOK(char *str)
{
#ifdef SYS_LINUX
	DIR *d = opendir(str);//str====/mnt/usb
	if (d == NULL)
	{
		DTRACE(DB_CRITICAL, ("%s no exit\n", str));
		return false;
	}
	closedir(d);
	char str2[64];
	sprintf(str2,"%s/..", str);
	struct stat s1;
	struct stat s2;
	lstat(str,&s1);	
	lstat(str2,&s2);
	if (s1.st_dev != s2.st_dev)
		return true;
	if (s1.st_ino == s2.st_ino)
		return true;
#endif
	return false;
}


bool UsbUpdate(char* szPath)
{
#ifdef SYS_LINUX
	WORD i;
	char str[64];
	strcpy(str, "/mnt/usb");
	DIR *p;
	for (i=0; i<10; i++)
	{
		//先检查U盘是否已经挂载成功 
		if (IsMountedOK(str))
		{
			DTRACE(DB_CRITICAL, ("USB mounted OK\n"));
			break;
		}
		system("mkdir /mnt/usb");
		system("mount -t vfat /dev/sda1 /mnt/usb");
		system("mount -t vfat /dev/sda /mnt/usb");
		Sleep(1500);
		continue;
	}
	if (i == 10)//挂载U盘失败
		return false;
	sprintf(str, "/mnt/usb/%s", szPath);

	char command[64];
	sprintf(command, "cp -f %s/update /mnt/app",str);
	system(command);

	system("chmod +x /mnt/app/update");
	sprintf(command, "source /mnt/app/update %s",str);
	system(command);
	Sleep(5000);
	system("umount /mnt/usb");
	strcpy(str, "/mnt/app/update");
	int f = open(str, O_RDWR|O_BINARY, S_IREAD|S_IWRITE);
	if (f >= 0)
	{//升级成功
		close(f);
		system("rm -Rf /mnt/app/update");
		return true;
	}
	else
	{//升级失败
		return false;
	}
#endif
	return true;
}

//描述:	接收是否完整645帧.
bool Check645Frm(BYTE* pbBuf, int nLen)
{
	int iFrmLen = 0;
	if (pbBuf[0]==0x68 && pbBuf[7]==0x68)
	{
		iFrmLen = pbBuf[9];
		if ((iFrmLen+12) != nLen)
			return false;
	   	if (pbBuf[nLen-1] != 0x16)
			return false;
		if (pbBuf[nLen-2] != CheckSum(pbBuf, iFrmLen+10))
			return false;
		else
			return true;
		
	}
	return false;	
}	


bool GetBufFrom645Buf(BYTE *pb645Buf, BYTE bLen, BYTE *pbBuf)
{
	if(pb645Buf==NULL || pbBuf==NULL || bLen==0)
		return false;

	for(BYTE b=0;b<bLen;b++)
		*pbBuf++ = (*pb645Buf++) - 0x33;
}

bool BufTo645Buf(BYTE *pbBuf, BYTE bLen, BYTE *pb645Buf)
{
	if(pb645Buf==NULL || pbBuf==NULL || bLen==0)
		return false;

	for(BYTE b=0;b<bLen;b++)
		*pb645Buf++ = (*pbBuf++) + 0x33;
}

DWORD SearchStrValHex(char* pStart, char* pEnd)
{
	bool fGetFirst = false;
	DWORD val = 0;
	while (pStart < pEnd)
	{
		char c = *pStart++;
		if (!fGetFirst)
		{
			if ((c>='0'&&c<='9') 
				|| (c>='a'&&c<='f')
				|| (c>='A'&&c<='F'))
			{
				fGetFirst = true;
			}
		}

		if (fGetFirst)
		{
			if ((c>='0'&&c<='9') 
			 || (c>='a'&&c<='f')
			 || (c>='A'&&c<='F'))
			{
				if (c>='0' && c<='9')
					val = (val<<4) + c - '0';
				else if (c>='a' && c<='f')
					val = (val<<4) + c - 'a' + 0x0A;
				else if (c>='A' && c<='F')
					val = (val<<4) + c - 'A' + 0x0A;					
			}
			else
			{
				break;
			}
		}

	}

	return val;
}

BYTE ASCII2BCD(BYTE bLow, BYTE bHigh)
{
	return (bLow-'0') + ((bHigh-'0')<<4);
}

void AsciiStr2BCD(BYTE* pAscii, BYTE* pBcd, BYTE bBcdLen)
{
	WORD i;
	for (i=0; i<bBcdLen; i++)
	{
		*pBcd++ = ASCII2BCD(*(pAscii+1), *pAscii);
		pAscii += 2;
	}
}

WORD CpyUntilCR(BYTE* pDst, BYTE* pSrc, WORD wLen)
{
	WORD i=0;
	for (i=0; i<wLen; i++)
	{
		if (*pSrc!='\r' && *pSrc!='\n')
			*pDst++ = *pSrc++;
		else
			return i;
	}
	
	return i;
}

//将通讯库中获取到的模块信号强度和发射功率写入数据库
//@bTxPwr: 发射功率
//@bSign:  信号强度
void UpdateTxPwr(BYTE bTxPwr, int16 bSign)	
{
	/*BYTE bTemp;
	bTemp = bTxPwr;
	WriteItemEx(BN2, PN0, 0x1059, &bTemp);
	bTemp = bSign;
	WriteItemEx(BN2, PN0, 0x1058, &bTemp);*/
	BYTE bChannel = 0, bBuf[4];
	
	DTRACE(DB_FAPROTO, ("UpdateTxPwr:bTxPwr %d.  bSign %d\n", bTxPwr, bSign)); 
	memset(bBuf, 0, sizeof(bBuf));
	//WriteItemEx(BN2, PN0, 0x2057, (BYTE *)pModemInfo->bCNUM);
	ReadItemEx(BANK17, PN0, 0x6010, &bChannel);
	bBuf[0] = DT_LONG;
	OoInt16ToLong(bSign, &bBuf[1]);
	WriteItemEx(BN0, bChannel, 0x4507, bBuf);
}

//将ppp拨号获取到的IP地址写入数据库
void UpdateDialIP()
{
	/*BYTE bChannel = 0, bBuf[20];
	memset(bBuf, 0, sizeof(bBuf));
	if (IfReadListProc("ppp0", &bBuf[2]) != 0)
	{
		ReadItemEx(BANK17, PN0, 0x6010, &bChannel);
		bBuf[0] = DT_OCT_STR;
		bBuf[1] = 0x04;
		TraceBuf(DB_FAPROTO, "UpdateDialIP: bBuf is -> ", bBuf, 10);
		WriteItemEx(BN0, bChannel, 0x4509, bBuf);
	}
	else
	{
		DTRACE(DB_FAPROTO,("UpdateDialIp: Get DialIP fail.\r\n"));
	}*/
}

int ReadBankId(WORD wBank, WORD wPn, WORD wID, BYTE *pbBuf)
{
	return ReadItemEx(wBank, wPn, wID, pbBuf);
}


bool IsPhaseEngId(WORD wID)
{
	if ((wID>>4)==0x907 || (wID>>4)==0x908
		|| (wID>>4)==0x917 || (wID>>4)==0x918
		|| (wID>>4)==0x947 || (wID>>4)==0x948
		|| (wID>>4)==0x957 || (wID>>4)==0x958)
	{		
		return true;
	}
	else 
		return false;
}

BYTE GetBlockIdNum(WORD wID)
{
	BYTE num = 0;

	if ( IsPhaseEngId(wID) ) //分相电能
		num = 3;
	else if ((wID>>12)==0x9 || (wID>>12)==0xa)
		num = TOTAL_RATE_NUM;
	else if ((wID>>8)==0xb0 || (wID>>8)==0xb1
		|| (wID>>8)==0xb4 || (wID>>8)==0xb5)
		num = TOTAL_RATE_NUM;
	else if ((wID>>4)==0xb61 || (wID>>4)==0xb62)
		num = 3;
	else if ((wID>>4)==0xb63 || (wID>>4)==0xb64 || (wID>>4)==0xb65 || (wID>>4)==0xb67)
		num = 4;
	else if ((wID>>4)==0xb66) //电压电流相位角
		num = 6;
	else if ((wID>>8)==0xb3) //断相
		num = 4;
//	else if (wID == 0xc01f)
//		num = 2;

	return num;
}

//描述:	判断电表是否通过网络抄表
bool IsRJLink(WORD wPn)
{
	BYTE bpnProp = GetPnProp(wPn);
	if (bpnProp==PN_PROP_RJ45 || bpnProp==PN_PROP_EPON)
		return true;

	return false;
}

void GetCurMillTime(TMillTime* pMillTime)
{
#ifdef SYS_WIN
	SYSTEMTIME currentTime;
	GetLocalTime(&currentTime);
	pMillTime->nYear = currentTime.wYear;
	pMillTime->nMonth = (BYTE )currentTime.wMonth;
	pMillTime->nDay = (BYTE )currentTime.wDay;
	pMillTime->nDayOfWeek = (BYTE)currentTime.wDayOfWeek;
	pMillTime->nHour = (BYTE )currentTime.wHour;
	pMillTime->nMinute = (BYTE )currentTime.wMinute;
	pMillTime->nSecond = (BYTE )currentTime.wSecond;
	pMillTime->nMilliseconds = currentTime.wMilliseconds;
#else
	TTime now;
	GetCurTime(&now);
	pMillTime->nYear = now.nYear;
	pMillTime->nMonth = now.nMonth;
	pMillTime->nDay = now.nDay;
	pMillTime->nDayOfWeek = now.nWeek;
	pMillTime->nHour = now.nHour;
	pMillTime->nMinute = now.nMinute;
	pMillTime->nSecond = now.nSecond;

	struct timeval timeMillSec;
	gettimeofday( &timeMillSec, NULL );
	pMillTime->nMilliseconds = timeMillSec.tv_usec / 1000;
#endif
}

void SetCurMillTime(TMillTime* pMillTime)
{
#ifdef SYS_WIN
	TTime time;
	time.nYear = pMillTime->nYear;
	time.nMonth = (BYTE )pMillTime->nMonth;
	time.nDay = (BYTE )pMillTime->nDay;
	time.nHour = (BYTE )pMillTime->nHour;
	time.nMinute = (BYTE )pMillTime->nMinute;
	time.nSecond = (BYTE )pMillTime->nSecond;
	
	SetSysTime(time);
#else
	struct timeval timeMillSec;
	settimeofday( &timeMillSec, NULL );
#endif
}

//7bit 解码
WORD gsmDecode7bit(const BYTE* pSrc, BYTE* pDst, WORD wSrcLen)
{
	WORD wSrc;
	WORD wDst;
	BYTE bByte;
	BYTE bLeft;
	
	wSrc = 0;
	wDst = 0;
	
	bByte = 0;
	bLeft = 0;
	
	while(wSrc < wSrcLen)
	{
		*pDst = ((*pSrc << bByte) | bLeft) & 0x7f;

		bLeft = *pSrc >> (7-bByte);

		pDst++;
		wDst++;

		bByte++;

		if(bByte == 7)
		{
			*pDst = bLeft;

			pDst++;
			wDst++;

			bByte = 0;
			bLeft = 0;
		}

		pSrc++;
		wSrc++;
	}

  *pDst = '\0';


  return wDst;
}


//描述：把TSA转换成字符串，方便打印
//参数：@ pbTSA 表地址
//		@pszBuf 用来装字符串的缓冲区
//返回：返回转换后字符串指针，其实就是pszBuf
char* TsaToStr(BYTE* pbTSA, char* pszBuf)
{
	sprintf(pszBuf, "%02x%02x%02x%02x", pbTSA[0],pbTSA[1],pbTSA[2],pbTSA[3],pbTSA[4],pbTSA[5]);

	return pszBuf;
}

//描述：把DWORD转换成字符串，方便打印
//参数：@ dw 值
//		@pszBuf 用来装字符串的缓冲区
//返回：返回转换后字符串指针，其实就是pszBuf
char* DwordToStr(DWORD dw, char* pszBuf)
{
	sprintf(pszBuf, "%02x%02x%02x%02x", (BYTE)dw, (BYTE)(dw>>8), (BYTE)(dw>>16), (BYTE)(dw>>24));

	return pszBuf;
}

//描述：把字节缓冲转换成字符串，方便打印
//参数：@ p字节缓冲指针
//		@ wLen 缓冲区的长度
//		@pszBuf 用来装字符串的缓冲区
//返回：返回转换后字符串指针，其实就是pszBuf
char* HexToStr(BYTE* p, WORD wLen, char* pszBuf, bool fRevs)
{
	for(WORD i=0; i<wLen; i++)
		if (fRevs)
			sprintf(&pszBuf[i*2], "%02x", p[wLen-1-i]);
		else
			sprintf(&pszBuf[i*2], "%02x", p[i]);

	return pszBuf;
}

//描述：排序,数组大小低到高
void IntSort(int *pibuf, WORD wLen)
{
	int j, i;
	int iTmp;

	for(j=0;j<wLen;j++)    
	{
		for(i=0;i<wLen-j;i++)   
		{
			if(pibuf[i] > pibuf[i+1]) 
			{ 
				iTmp = pibuf[i];          
				pibuf[i] = pibuf[i+1];         
				pibuf[i+1] = iTmp;       
			}
		}
	}
}

//将字节中的位顺序倒序
BYTE BitReverse(BYTE b)
{
	WORD i;
	BYTE bTemp = 0;

	for (i=0; i<8; i++)
	{
		if (b & (0x1<<i))
			bTemp |= (0x1<<(7-i));
	}

	return bTemp;
}