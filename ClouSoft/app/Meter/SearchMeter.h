#ifndef SEARCHMETER_H
#define SEARCHMETER_H

#include "apptypedef.h"

#define STACK_SIZE  8
#define EXT_STACK_SIZE 26
#define MTR_PORT_NUM	3

#define PRO97METORNOT     0
#define PRO97             1
#define PRO07METORNOT     2
#define PRO07             3
#define PRONJSLMETORNOT   4
#define PRONJSL           5
#define PROT188WATERORNOT 6
#define PROT188WATER      7 //水表
#define PROT188GAS        8 //气表
#define PROT188GASORNOT   9
#define PROT188HEATORNOT  10 //热表
#define PROT188HEAT       11
#define PRO69845ORNOT     12
#define PRO69845          13
#define SEARCHOVER        14
#define SEARCHWAIT        15

#define WATER_TYPE10      0x10
#define WATER_TYPE11      0x11
#define WATER_TYPE12      0x12
#define WATER_TYPE13      0x13

#define HEAT_TYPE20      0x20
#define HEAT_TYPE21      0x21

#define GAS_TYPE30       0x30


#define DISABLE_DATA    0xff

#define M97MET   0x01
#define M07MET   0x02
#define MNJSLMET 0x04

#define SEARCH_UNDOEN   0
#define SEARCH_OVER     1
#define SEARCH_ERROR    2

typedef struct
{
    WORD wBaud;
    BYTE bProto;
}TMeterPro;

typedef struct
{
    //BYTE bPn;      //测量点号
    BYTE bEn;      //测量点有效
    BYTE bAddr[6]; //测量点地址  
    BYTE bBaud;
    BYTE bProto;
	BYTE bPort;
//    DWORD dwTime;  //
}TMeterAddrTab;     

typedef struct
{
    BYTE bData[STACK_SIZE+EXT_STACK_SIZE];
    int iTop;  //先压栈后移动，先移动后出栈。
}TStack;

//typedef struct
//{
//	BYTE bLowBit:4;
//	BYTE bHignBit:4;
//}T698Addr;

typedef struct
{
	BYTE bFinish;
	BYTE bCurTry; 
	BYTE bCurTryLevel;

	BYTE bSearchState;
	BYTE bAddrPatten[16];//为了兼容698.45 所以拓展到16个字节
	//T698Addr tAddrPatten[16];

	TStack tStack ;
	//TMeterAddrTab tMeterAddrTab[32] ;
}TMtrSchInf;//搜表的记录变量


extern TMtrSchInf g_tMtrRdSchInf[MTR_PORT_NUM];
void InitSearch(BYTE bPort, BYTE bStartSer);
void DoSearch(BYTE bPort);
void ReverBuff(BYTE *pbBuf, WORD bLen);

//BYTE GetAddrPage(BYTE bPort, BYTE bCnt, BYTE bMtrAddrNum, BYTE *pbBuf, BYTE bBufSize);

#endif
