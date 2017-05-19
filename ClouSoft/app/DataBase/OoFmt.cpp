

#include "stdafx.h"
#include "apptypedef.h"
#include "FaCfg.h"
#include "FaAPI.h"
#include "sysfs.h"
#include "DbOIAPI.h"
#include "OoFmt.h"
//#include "DbAPI.h"

//00 10 02 00
//高字节在前，低字节在后
WORD OoOiToWord(BYTE* pbBuf)
{
	return ((WORD )pbBuf[0]<<8) | pbBuf[1];
}

//00 10 02 00
DWORD OoOadToDWord(BYTE* pbBuf)
{	
	return ((DWORD )pbBuf[0]<<24) | ((DWORD )pbBuf[1]<<16) | ((DWORD )pbBuf[2]<<8) | pbBuf[3];
}

//07 E0 01 14 00 00 00		2016-1-20 00:00:00
bool OoDateTimeSToTime(BYTE* pbBuf, TTime* pTime)
{
	pTime->nYear   = ((WORD )pbBuf[0]<<8) | pbBuf[1];
	pTime->nMonth  = pbBuf[2];
	pTime->nDay    = pbBuf[3];
	pTime->nHour   = pbBuf[4];
	pTime->nMinute = pbBuf[5];
	pTime->nSecond = pbBuf[6];
	pTime->nWeek   = OoDayOfWeek(*pTime);

	if (IsInvalidTime(*pTime))
		return false;
	else
		return true;
}

bool OoDateToTime(BYTE* pbBuf, TTime* pTime)
{
	pTime->nYear   = ((WORD )pbBuf[0]<<8) | pbBuf[1];
	pTime->nMonth  = pbBuf[2];
	pTime->nDay    = pbBuf[3];
	pTime->nHour   = 0;
	pTime->nMinute = 0;
	pTime->nSecond = 0;
	pTime->nWeek   = OoDayOfWeek(*pTime);

	if (IsInvalidTime(*pTime))
		return false;
	else
		return true;
}


//只有时分秒,没有年月日
bool OoFmtTmToTime(BYTE* pbBuf, TTime* pTime)
{
	pTime->nHour   = pbBuf[0];
	pTime->nMinute = pbBuf[1];
	pTime->nSecond = pbBuf[2];

	if (pTime->nHour>23 ||
		pTime->nMinute>=60 ||
		pTime->nSecond>=60)
		return false;
	else
		return true;
}


bool OoDateTimeToMillTime(BYTE* pbBuf, TMillTime* pTime)
{
	TTime time;

	time.nYear = pTime->nYear = ((WORD )pbBuf[0]<<8) | pbBuf[1];
	time.nMonth = pTime->nMonth = pbBuf[2];
	time.nDay = pTime->nDay = pbBuf[3];
	time.nHour = pTime->nHour  = pbBuf[5];
	time.nMinute = pTime->nMinute = pbBuf[6];
	time.nSecond = pTime->nSecond = pbBuf[7];
	
	if (IsInvalidTime(time))
		return false;
	
	pTime->nDayOfWeek = pbBuf[4];
	pTime->nMilliseconds = ((WORD )pbBuf[8]<<8) | pbBuf[9];

	return true;
}


WORD OoWordToOi(WORD wOi, BYTE* pbBuf)
{
	pbBuf[0] = (wOi>>8) & 0xff;
	pbBuf[1] = wOi & 0xff;

	return 2;
}

WORD OoDWordToOad(DWORD dwOad, BYTE* pbBuf)
{
	pbBuf[0] = (dwOad>>24) & 0xff;
	pbBuf[1] = (dwOad>>16) & 0xff;
	pbBuf[2] = (dwOad>>8) & 0xff;
	pbBuf[3] = dwOad & 0xff;

	return 4;
}

WORD OoTimeToDateTimeS(TTime* pTime, BYTE* pbBuf)
{
	pbBuf[0] = (pTime->nYear>>8) & 0xff;
	pbBuf[1] = pTime->nYear & 0xff;
	pbBuf[2] = pTime->nMonth;
	pbBuf[3] = pTime->nDay;
	pbBuf[4] = pTime->nHour;
	pbBuf[5] = pTime->nMinute;
	pbBuf[6] = pTime->nSecond;

	return 7;
}

WORD OoTimeToDate(TTime* pTime, BYTE* pbBuf)
{
	pbBuf[0] = (pTime->nYear>>8) & 0xff;
	pbBuf[1] = pTime->nYear & 0xff;
	pbBuf[2] = pTime->nMonth;
	pbBuf[3] = pTime->nDay;	

	return 4;
}

WORD OoTimeToFmtTm(TTime* pTime,BYTE* pbBuf)
{
	pbBuf[0] = pTime->nHour;
	pbBuf[1] = pTime->nMinute;
	pbBuf[2] = pTime->nSecond;

	return 3;
}

WORD OoMillTimeToDateTime(TMillTime* pTime, BYTE* pbBuf)
{
	TTime time;
	time.nYear = pTime->nYear;
	time.nMonth = pTime->nMonth;
	time.nDay = pTime->nDay;

	pbBuf[0] = (pTime->nYear>>8) & 0xff;
	pbBuf[1] = pTime->nYear & 0xff;
	pbBuf[2] = pTime->nMonth;
	pbBuf[3] = pTime->nDay;
	pbBuf[4] = OoDayOfWeek(time);
	pbBuf[5] = pTime->nHour;
	pbBuf[6] = pTime->nMinute;
	pbBuf[7] = pTime->nSecond;
	pbBuf[8] = (pTime->nMilliseconds>>8) & 0xff;
	pbBuf[9] = pTime->nMilliseconds & 0xff;

	return 10;
}

WORD OoIntToDoubleLong(int nVal, BYTE* pbBuf)
{	
	*pbBuf++ = (nVal>>24) & 0xff;
	*pbBuf++ = (nVal>>16) & 0xff;
	*pbBuf++ = (nVal>>8) & 0xff;
	*pbBuf++ = nVal & 0xff;

	return 4;
}

//高字节在前 低字节在后
WORD OoDWordToDoubleLongUnsigned(DWORD dwVal, BYTE* pbBuf)
{
	*pbBuf++ = (dwVal>>24) & 0xff;
	*pbBuf++ = (dwVal>>16) & 0xff;
	*pbBuf++ = (dwVal>>8) & 0xff;
	*pbBuf++ = dwVal & 0xff;

	return 4;
}

WORD OoInt16ToLong(int16 val, BYTE* pbBuf)
{
	*pbBuf++ = (val>>8) & 0xff;
	*pbBuf++ = val & 0xff;

	return 2;
}

WORD OoWordToLongUnsigned(WORD wVal, BYTE* pbBuf)
{
	*pbBuf++ = (wVal>>8) & 0xff;
	*pbBuf++ = wVal & 0xff;

	return 2;
}

WORD OoInt64ToLong64(int64 val, BYTE* pbBuf)
{
	*pbBuf++ = (val>>56) & 0xff;
	*pbBuf++ = (val>>48) & 0xff;
	*pbBuf++ = (val>>40) & 0xff;
	*pbBuf++ = (val>>32) & 0xff;
	*pbBuf++ = (val>>24) & 0xff;
	*pbBuf++ = (val>>16) & 0xff;
	*pbBuf++ = (val>>8) & 0xff;
	*pbBuf++ = val & 0xff;

	return 8;
}

WORD OoUint64ToLong64Unsigned(uint64 val, BYTE* pbBuf)
{
	*pbBuf++ = (val>>56) & 0xff;
	*pbBuf++ = (val>>48) & 0xff;
	*pbBuf++ = (val>>40) & 0xff;
	*pbBuf++ = (val>>32) & 0xff;
	*pbBuf++ = (val>>24) & 0xff;
	*pbBuf++ = (val>>16) & 0xff;
	*pbBuf++ = (val>>8) & 0xff;
	*pbBuf++ = val & 0xff;

	return 8;
}

int OoDoubleLongToInt(BYTE* pbBuf)
{
	return ((int )pbBuf[0]<<24) | ((int )pbBuf[1]<<16) | ((int )pbBuf[2]<<8) | pbBuf[3];
}


DWORD OoDoubleLongUnsignedToDWord(BYTE* pbBuf)
{
	return ((DWORD )pbBuf[0]<<24) | ((DWORD )pbBuf[1]<<16) | ((DWORD )pbBuf[2]<<8) | pbBuf[3];
}


int16 OoLongToInt16(BYTE* pbBuf)
{
	return ((int16 )pbBuf[0]<<8) | pbBuf[1];
}

WORD OoLongUnsignedToWord(BYTE* pbBuf)
{
	return ((WORD )pbBuf[0]<<8) | pbBuf[1];
}

int64 OoLong64ToInt64(BYTE* pbBuf)
{
	return ((int64 )pbBuf[0]<<56) | ((int64 )pbBuf[1]<<48) | ((int64 )pbBuf[2]<<40) | ((int64 )pbBuf[3]<<32) | 
				((int64 )pbBuf[4]<<24) | ((int64 )pbBuf[5]<<16) | ((int64 )pbBuf[6]<<8) | pbBuf[7];
}

uint64 OoLong64UnsignedTouUint64(BYTE* pbBuf)
{
	return ((uint64 )pbBuf[0]<<56) | ((uint64 )pbBuf[1]<<48) | ((uint64 )pbBuf[2]<<40) | ((uint64 )pbBuf[3]<<32) | 
				((uint64 )pbBuf[4]<<24) | ((uint64 )pbBuf[5]<<16) | ((uint64 )pbBuf[6]<<8) | pbBuf[7];
}
