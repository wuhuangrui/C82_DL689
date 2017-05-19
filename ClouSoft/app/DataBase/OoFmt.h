
#ifndef OOFMT_H
#define OOFMT_H

#include "apptypedef.h"
#include "FaCfg.h"
#include "ComStruct.h"
#include "ComConst.h"
#include "apptypedef.h"



WORD OoOiToWord(BYTE* pbBuf);
DWORD OoOadToDWord(BYTE* pbBuf);
bool OoDateTimeSToTime(BYTE* pbBuf, TTime* pTime);
bool OoDateToTime(BYTE* pbBuf, TTime* pTime);
bool OoFmtTmToTime(BYTE* pbBuf, TTime* pTime);
bool OoDateTimeToMillTime(BYTE* pbBuf, TMillTime* pTime);

WORD OoWordToOi(WORD wOi, BYTE* pbBuf);
WORD OoDWordToOad(DWORD dwOad, BYTE* pbBuf);
WORD OoTimeToDateTimeS(TTime* pTime, BYTE* pbBuf);
WORD OoTimeToDate(TTime* pTime, BYTE* pbBuf);
WORD OoTimeToFmtTm(TTime* pTime,BYTE* pbBuf);
WORD OoMillTimeToDateTime(TMillTime* pTime, BYTE* pbBuf);

int OoDoubleLongToInt(BYTE* pbBuf);
DWORD OoDoubleLongUnsignedToDWord(BYTE* pbBuf);
int16 OoLongToInt16(BYTE* pbBuf);
WORD OoLongUnsignedToWord(BYTE* pbBuf);
int64 OoLong64ToInt64(BYTE* pbBuf);
uint64 OoLong64UnsignedTouUint64(BYTE* pbBuf);

WORD OoIntToDoubleLong(int nVal, BYTE* pbBuf);
WORD OoDWordToDoubleLongUnsigned(DWORD dwVal, BYTE* pbBuf);
WORD OoInt16ToLong(int16 val, BYTE* pbBuf);
WORD OoWordToLongUnsigned(WORD val, BYTE* pbBuf);
WORD OoInt64ToLong64(int64 val, BYTE* pbBuf);
WORD OoUint64ToLong64Unsigned(uint64 val, BYTE* pbBuf);


#endif	//OOFMT_H