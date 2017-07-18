#include "stdafx.h"
#include "sysapi.h"
#include "Esam.h"
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
//#include <pthread.h> 
#include <fcntl.h>
//#include <unistd.h>
#include <signal.h>
//#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h> 
#include <stdarg.h> 
#include <time.h> 
//#include <unistd.h>
//#include <sys/errno.h>
#include <errno.h>
//#include <sys/ioctl.h>
#include "sysarch.h"
#include "Trace.h"
#include "sysdebug.h"
#include "ComAPI.h"
#include "sysfs.h"
#include "DbConst.h"
#include "LibDbAPI.h"


// IEC7816操作命令
#define WARMACTIVATE            0


#define CASE1  1
#define CASE2  2
#define CASE3  3
#define CASE4  4

#define P1     1
#define P2     2

#define CLA_OFF 1
#define ESAM_STD_TIMEOUT 3000

//CESAM g_ESAM;
int g_ifd = -1;
BYTE g_bCertState = 0;	//证书状态：00为测试证书；01为正式证书
TSem g_semEsam;
BYTE g_bEsamCmpDataBuf[20];
WORD g_wEsamCmpDataLen = 0;
WORD g_wEsamLastRxDataLen = 0;


extern BYTE g_bEsamSerialNumFmt[];
extern BYTE g_bEsamVerisonFmt[];
extern BYTE g_bEsamKeyVersionFmt[];
extern BYTE g_bEsamCerVersionFmt[];
extern BYTE g_bEsamSessionMaxTimeFmt[];
extern BYTE g_bEsamSessionRemainTimeFmt[];
extern BYTE g_bEsamCounterFmt[];
extern BYTE g_bEsamTermCerSerNumFmt[];
extern BYTE g_bEsamMSCerSerNumFmt[];
extern BYTE g_bEsamTermCertificateFmt[];
extern BYTE g_bEsamMSCertificateFmt[];
extern BYTE g_bEsamSercureModeChoiceFmt[];
extern BYTE g_bEsamSercureModeParamFmt[];


static const TSWTab g_tSwTable[]=
{
	{0x90,0x00,"OK"},//指令执行成功
	{0x63,0xcf,"Cert Or Swt failed"},//认证失败
	{0x64,0x00,"InExcu wrong"},//内部执行出错
	{0x65,0x81,"Card locked"},//卡被锁死
	{0x67,0x00,"Len wrong"},//Lc或Le长度错
	{0x69,0x01,"Cntzero Or Cmd wrong"},//离线计数器为0或命令不接受
	{0x69,0x82,"No Safe sta"},//不满足安全状态
	{0x69,0x83,"Kut zero"},//Kut使用次数为0
	{0x69,0x84,"Use Data invalid"},//引用数据无效
	{0x69,0x85,"Condition Incomplete"},//使用条件不满足
	{0x69,0x86,"Online Cnt zero"},//在线计数器为0
	{0x69,0x88,"MAC wrong"},//MAC错误
	{0x6a,0x80,"Para wrong"},//参数错误
	{0x6a,0x86,"P1P2 wrong"},//参数错误
	{0x6a,0x88,"No Find data"},//未找到引用数据
	{0x6d,0x00,"Cmd No exist"},//命令不存在
	{0x6e,0x00,"Cmd Or CLA wrong "},//命令或CLA错
	{0x6f,0x00,"Data invalid"},//数据无效
	{0x90,0x86,"CheckSign wrong"},//验签错误
	{0x9e,0x2f,"File wrong"},//文件错误
	{0x9e,0x3f,"Calc wrong"},//算法计算错误
	{0x9e,0x57,"Cert wrong"},//认证错误
	{0x9e,0x60,"Session wrong"},//建立会话错误
	{0x9e,0x5e,"CA wrong"},//CA证书错误
};

static bool IsAllAByte(BYTE* p, BYTE b, WORD len)
{
	for (WORD i=0; i<len; i++)
	{
		if (*p++ != b)
			return false;
	}
	
	return true;
}


// BYTE memrcpy(BYTE *pbDst,BYTE *pbSrc,WORD wLen)
// {
// 	for (int i=0;i<wLen;i++) 
// 	{
// 		pbDst[i] = pbSrc[wLen-1-i];
// 	}
// 	return wLen;
// }

// 将一维数组元素倒序
// void Swap(BYTE *pbBuf, WORD wLen)
// {
// 	BYTE bTemp;
// 	WORD wSwapTimes = wLen>>1;
// 	for (WORD i=0; i<wSwapTimes; i++)
// 	{
// 		bTemp = pbBuf[i];
// 		pbBuf[i] = pbBuf[wLen-i-1];
// 		pbBuf[wLen-i-1] = bTemp;
// 	}
// }

BYTE dayofweek(const TTime& time)
{
	BYTE bWeek = DayOfWeek(time); //DayOfWeek()的返回 1 = Sunday, 2 = Monday, ..., 7 = Saturday
	if (bWeek != 0)
		bWeek--;

	return bWeek;
}


void EsamClose()
{
    if (g_ifd > 0)
    {
		close(g_ifd);
		g_ifd = -1;
    }
}

bool EsamInit()
{
    int iDataLen;
	BYTE bBuf1[2048],bBuf2[2048];
	BYTE bOctLen;

	g_semEsam = NewSemaphore(1);
	
	//const BYTE bCmdGetEsamVer[] = {0x80, 0x36, 0x00, 0x03, 0x00, 0x00};  
#ifndef  SYS_WIN 
	g_ifd = open("/dev/esam", O_RDWR|O_NONBLOCK|O_NOCTTY|O_NDELAY, 0); 
#endif   
	if (g_ifd < 0)
	{
		DTRACE(DB_CRITICAL,("Open esam dev failed\r\n"));
		return false;
	}


	//批量获取安全芯片信息:
	BYTE bAttributeLen,bAttributePos;
	iDataLen = Esam_GetEsamInfo(AllEsamInfo, bBuf1, sizeof(bBuf1));

	//DTRACE(DB_CRITICAL,("EsamInfo len %d(0x%x).\r\n", iDataLen, iDataLen));
	if (iDataLen > 0)
	{
		TraceBuf(DB_ESAM, "EsamInfo:", bBuf1, iDataLen);


		//ESAM序列号 //g_bEsamSerialNumFmt
		bAttributePos = 0;
		bAttributeLen = 8;
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamSerialNumFmt:", bBuf2, bOctLen+1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF102, bBuf2); 

		//g_bEsamVerisonFmt
		bAttributePos += bAttributeLen;
		bAttributeLen = 4;
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamVerisonFmt:", bBuf2, bOctLen+1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF103, bBuf2); 
		

		//g_bEsamKeyVersionFmt
		bAttributePos += bAttributeLen;
		bAttributeLen = 16;
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamKeyVersionFmt:", bBuf2, bOctLen+1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF104, bBuf2); 

		//g_bEsamCerVersionFmt
		bBuf2[0] = g_bEsamCerVersionFmt[0];
		bBuf2[1] = g_bEsamCerVersionFmt[1];
		bBuf2[2] = DT_OCT_STR;	

		bAttributePos += bAttributeLen;
		bAttributeLen = 1;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[3]);
		memcpy(&bBuf2[bOctLen+3], &bBuf1[bAttributePos], bAttributeLen);
		
		BYTE bPos2 = bOctLen+3+bAttributeLen;
		bBuf2[bPos2] = DT_OCT_STR;
		bAttributePos += bAttributeLen;
		bAttributeLen = 1;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[bPos2+1]);
		memcpy(&bBuf2[bPos2+1+bOctLen], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamCerVersionFmt:", bBuf2, bPos2+1+bOctLen+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF105, bBuf2);

		//g_bEsamSessionMaxTimeFmt
		bAttributePos += bAttributeLen;
		bAttributeLen = 4;
		bBuf2[0] = DT_DB_LONG_U;
		memcpy(&bBuf2[1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamSessionMaxTimeFmt:", bBuf2, 1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF106, bBuf2);

		//g_bEsamSessionRemainTimeFmt
		bAttributePos += bAttributeLen;
		bAttributeLen = 4;
		bBuf2[0] = DT_DB_LONG_U;
		memcpy(&bBuf2[1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamSessionRemainTimeFmt:", bBuf2, 1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF107, bBuf2);

		//g_bEsamCounterFmt
		bBuf2[0] = g_bEsamCounterFmt[0];
		bBuf2[1] = g_bEsamCounterFmt[1];
		
		bAttributePos += bAttributeLen;
		bAttributeLen = 4;
		bBuf2[2] = DT_DB_LONG_U;
		memcpy(&bBuf2[3], &bBuf1[bAttributePos], bAttributeLen);
		bAttributePos += bAttributeLen;
		bBuf2[7] = DT_DB_LONG_U;
		memcpy(&bBuf2[8], &bBuf1[bAttributePos], bAttributeLen);
		bAttributePos += bAttributeLen;
		bBuf2[12] = DT_DB_LONG_U;
		memcpy(&bBuf2[13], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamCounterFmt:", bBuf2, 13+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF108, bBuf2);

		//g_bEsamTermCerSerNumFmt
		bAttributePos += bAttributeLen;
		bAttributeLen = 16;
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamTermCerSerNumFmt:", bBuf2, bOctLen+1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF109, bBuf2); 

		//g_bEsamMSCerSerNumFmt
		bAttributePos += bAttributeLen;
		bAttributeLen = 16;
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamMSCerSerNumFmt:", bBuf2, bOctLen+1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF10B, bBuf2); 

	}

	//获取终端证书
	iDataLen = Esam_GetEsamInfo(TermCertificate, bBuf1, sizeof(bBuf1));
	if (iDataLen > 0)
	{	
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(iDataLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], bBuf1, iDataLen);
		WriteItemEx(BANK0, PN0, 0xF10A, bBuf2); //g_bEsamTermCertificateFmt
	}
	
	//获取主站证书
	iDataLen = Esam_GetEsamInfo(MasterStationCer, bBuf1, sizeof(bBuf1));
	if (iDataLen > 0)
	{	
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(iDataLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], bBuf1, iDataLen);
		WriteItemEx(BANK0, PN0, 0xF10C, bBuf2); //g_bEsamMSCertificateFmt
	}
	
	TrigerSaveBank(BANK0, SECT_ESAM, -1);
/*
	//证书状态：00为测试证书；01为正式证书
	//获取芯片状态信息1字节
	//发送：800E00050000
	//返回：9000+LEN+芯片状态信息
	do
	{
		bcont++;
		iDataLen = EsamTxRxCmd(bCmdGetEsamVer, &g_bCertState, 7, 3000);
	} while(iDataLen==-2 && bcont<2);
	if (iDataLen <= 0)
	{
		DTRACE(DB_ESAM,("Get esam certsta failed\r\n"));
		return false;
	}
*/
	DTRACE(DB_CRITICAL, ("Esam driver init OK.\r\n"));
	return true;
}

bool EsamReInit()
{
    int iDataLen;
	BYTE bBuf1[2048],bBuf2[2048];
	BYTE bOctLen;

	//批量获取安全芯片信息:
	BYTE bAttributeLen,bAttributePos;
	iDataLen = Esam_GetEsamInfo(AllEsamInfo, bBuf1, sizeof(bBuf1));

	//DTRACE(DB_CRITICAL,("EsamInfo len %d(0x%x).\r\n", iDataLen, iDataLen));
	if (iDataLen > 0)
	{
		TraceBuf(DB_ESAM, "EsamInfo:", bBuf1, iDataLen);


		//ESAM序列号 //g_bEsamSerialNumFmt
		bAttributePos = 0;
		bAttributeLen = 8;
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamSerialNumFmt:", bBuf2, bOctLen+1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF102, bBuf2); 

		//g_bEsamVerisonFmt
		bAttributePos += bAttributeLen;
		bAttributeLen = 4;
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamVerisonFmt:", bBuf2, bOctLen+1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF103, bBuf2); 
		

		//g_bEsamKeyVersionFmt
		bAttributePos += bAttributeLen;
		bAttributeLen = 16;
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamKeyVersionFmt:", bBuf2, bOctLen+1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF104, bBuf2); 

		//g_bEsamCerVersionFmt
		bBuf2[0] = g_bEsamCerVersionFmt[0];
		bBuf2[1] = g_bEsamCerVersionFmt[1];
		bBuf2[2] = DT_OCT_STR;	

		bAttributePos += bAttributeLen;
		bAttributeLen = 1;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[3]);
		memcpy(&bBuf2[bOctLen+3], &bBuf1[bAttributePos], bAttributeLen);
		
		BYTE bPos2 = bOctLen+3+bAttributeLen;
		bBuf2[bPos2] = DT_OCT_STR;
		bAttributePos += bAttributeLen;
		bAttributeLen = 1;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[bPos2+1]);
		memcpy(&bBuf2[bPos2+1+bOctLen], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamCerVersionFmt:", bBuf2, bPos2+1+bOctLen+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF105, bBuf2);

		//g_bEsamSessionMaxTimeFmt
		bAttributePos += bAttributeLen;
		bAttributeLen = 4;
		bBuf2[0] = DT_DB_LONG_U;
		memcpy(&bBuf2[1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamSessionMaxTimeFmt:", bBuf2, 1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF106, bBuf2);

		//g_bEsamSessionRemainTimeFmt
		bAttributePos += bAttributeLen;
		bAttributeLen = 4;
		bBuf2[0] = DT_DB_LONG_U;
		memcpy(&bBuf2[1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamSessionRemainTimeFmt:", bBuf2, 1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF107, bBuf2);

		//g_bEsamCounterFmt
		bBuf2[0] = g_bEsamCounterFmt[0];
		bBuf2[1] = g_bEsamCounterFmt[1];
		
		bAttributePos += bAttributeLen;
		bAttributeLen = 4;
		bBuf2[2] = DT_DB_LONG_U;
		memcpy(&bBuf2[3], &bBuf1[bAttributePos], bAttributeLen);
		bAttributePos += bAttributeLen;
		bBuf2[7] = DT_DB_LONG_U;
		memcpy(&bBuf2[8], &bBuf1[bAttributePos], bAttributeLen);
		bAttributePos += bAttributeLen;
		bBuf2[12] = DT_DB_LONG_U;
		memcpy(&bBuf2[13], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamCounterFmt:", bBuf2, 13+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF108, bBuf2);

		//g_bEsamTermCerSerNumFmt
		bAttributePos += bAttributeLen;
		bAttributeLen = 16;
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamTermCerSerNumFmt:", bBuf2, bOctLen+1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF109, bBuf2); 

		//g_bEsamMSCerSerNumFmt
		bAttributePos += bAttributeLen;
		bAttributeLen = 16;
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(bAttributeLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], &bBuf1[bAttributePos], bAttributeLen);
		TraceBuf(DB_ESAM, "g_bEsamMSCerSerNumFmt:", bBuf2, bOctLen+1+bAttributeLen);
		WriteItemEx(BANK0, PN0, 0xF10B, bBuf2); 

	}

	//获取终端证书
	iDataLen = Esam_GetEsamInfo(TermCertificate, bBuf1, sizeof(bBuf1));
	if (iDataLen > 0)
	{	
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(iDataLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], bBuf1, iDataLen);
		WriteItemEx(BANK0, PN0, 0xF10A, bBuf2); //g_bEsamTermCertificateFmt
	}
	
	//获取主站证书
	iDataLen = Esam_GetEsamInfo(MasterStationCer, bBuf1, sizeof(bBuf1));
	if (iDataLen > 0)
	{	
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(iDataLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], bBuf1, iDataLen);
		WriteItemEx(BANK0, PN0, 0xF10C, bBuf2); //g_bEsamMSCertificateFmt
	}
	
	TrigerSaveBank(BANK0, SECT_ESAM, -1);
/*
	//证书状态：00为测试证书；01为正式证书
	//获取芯片状态信息1字节
	//发送：800E00050000
	//返回：9000+LEN+芯片状态信息
	do
	{
		bcont++;
		iDataLen = EsamTxRxCmd(bCmdGetEsamVer, &g_bCertState, 7, 3000);
	} while(iDataLen==-2 && bcont<2);
	if (iDataLen <= 0)
	{
		DTRACE(DB_ESAM,("Get esam certsta failed\r\n"));
		return false;
	}
*/
	DTRACE(DB_CRITICAL, ("Esam driver Reinit OK.\r\n"));
	return true;
}


void Esam_ReflashSessionMaxTime(void)
{
	int iDataLen;
	BYTE bBuf1[128];
	BYTE bBuf2[128];

	iDataLen = Esam_GetEsamInfo(SessionMaxTime, bBuf1, sizeof(bBuf1));
	if (iDataLen < 4)
		return;

	//g_bEsamSessionMaxTimeFmt
	bBuf2[0] = DT_DB_LONG_U;
	memcpy(&bBuf2[1], &bBuf1, 4);
	TraceBuf(DB_ESAM, "g_bEsamSessionMaxTimeFmt:", bBuf2, 5);
	iDataLen = ReadItemEx(BN0, PN0, 0xF106, bBuf1);
	if ((iDataLen<0) || (memcmp(bBuf1, bBuf2, 5)!=0))
	{
		DTRACE(DB_ESAM, ("WriteItemEx: BANK0, PN0, 0xF106 .\r\n"));
		WriteItemEx(BANK0, PN0, 0xF106, bBuf2);
		TrigerSaveBank(BANK0, SECT_ESAM, -1);
	}
}

void Esam_ReflashSessionRemainTime(void)
{
	int iDataLen;
	BYTE bBuf1[128];
	BYTE bBuf2[128];

	iDataLen = Esam_GetEsamInfo(SessionRemainTime, bBuf1, sizeof(bBuf1));
	if (iDataLen < 4)
		return;
	
	//g_bEsamSessionRemainTimeFmt
	bBuf2[0] = DT_DB_LONG_U;
	memcpy(&bBuf2[1], bBuf1, 4);
	TraceBuf(DB_ESAM, "g_bEsamSessionRemainTimeFmt:", bBuf2, 5);
	iDataLen = ReadItemEx(BN0, PN0, 0xF107, bBuf1);
	if ((iDataLen<0) || (memcmp(bBuf1, bBuf2, 5)!=0))
	{
		DTRACE(DB_ESAM, ("WriteItemEx: BANK0, PN0, 0xF107 .\r\n"));
		WriteItemEx(BANK0, PN0, 0xF107, bBuf2);
		TrigerSaveBank(BANK0, SECT_ESAM, -1);
	}

}

void Esam_ReflashCounter(void)
{
	int iDataLen;
	BYTE bBuf1[128];
	BYTE bBuf2[128];
	BYTE bAttributePos,bAttributeLen;
	iDataLen = Esam_GetEsamInfo(Counter, bBuf1, sizeof(bBuf1));
	if (iDataLen < 4)
		return;

	//g_bEsamCounterFmt
	bBuf2[0] = g_bEsamCounterFmt[0];
	bBuf2[1] = g_bEsamCounterFmt[1];
	
	bAttributePos = 0;
	bAttributeLen = 4;
	bBuf2[2] = DT_DB_LONG_U;
	memcpy(&bBuf2[3], &bBuf1[bAttributePos], bAttributeLen);
	bAttributePos += bAttributeLen;
	bBuf2[7] = DT_DB_LONG_U;
	memcpy(&bBuf2[8], &bBuf1[bAttributePos], bAttributeLen);
	bAttributePos += bAttributeLen;
	bBuf2[12] = DT_DB_LONG_U;
	memcpy(&bBuf2[13], &bBuf1[bAttributePos], bAttributeLen);
	
	TraceBuf(DB_ESAM, "g_bEsamCounterFmt:", bBuf2, 13+bAttributeLen);
	WriteItemEx(BANK0, PN0, 0xF108, bBuf2);
	TrigerSaveBank(BANK0, SECT_ESAM, -1);

}


bool EsamReset()
{
	return true;
}

//参数：@pbBuf 接收缓冲区
//		@wExpLen 期待的接收长度
int EsamRead(BYTE* pbBuf, WORD wExpLen)
{
	int iLen = 0; 
	if (wExpLen > 0) 
	{
		for (int i=0; i<6; i++) 
		{
			iLen = read(g_ifd, pbBuf, wExpLen);	//写的时候自动清，平常可以多次读
			if (iLen >= wExpLen)
			{
				if(IsAllAByte(pbBuf, 0xff, iLen))
				{
					DTRACE(DB_ESAM, ("EsamRead:times=%d ,All of Bytes is 0xff\r\n", i));
					/*return -1;*/
					continue;
				}
				return iLen;
			}

			Sleep(500);
		}
	}

	return iLen;
}


int EsamCheckRxFrm(BYTE* pbFrm, WORD wLen, BYTE bSW1, BYTE bSW2, int* piStart)
{
	//接收数据的结构为：SW1 SW2 Len1 Len2 DATA LRC2
	//LRC2的计算方法：对Len1 Len2 DATA数据，每个字节的异或值，再取反。
	//Len1代表长度的高字节，Len2代表长度的低字节
	//Len1 Len2代表DATA域的长度，不包括LRC1或LRC2
	//if (pbFrm[0]!=bSW1 || pbFrm[1]!=bSW2)
	//	return -1;

	int iStart = -1;
	int iCmdHead = -1;
	WORD i;

	*piStart = -1;
	for (i=0; i<wLen-1; i++)
	{
		if (pbFrm[i]==0x55 && iCmdHead==-1)
			iCmdHead = i;
		if (pbFrm[i]==bSW1 && pbFrm[i+1]==bSW2)
		{
			iStart = i;
			break;
		}
	}

	if (iStart == -1)
	{
		if (iCmdHead >= 0)
		{
			for (i=0; i<sizeof(g_tSwTable); i++) 
			{
				if ((pbFrm[iCmdHead+1]==g_tSwTable[i].bSW1)&&(pbFrm[iCmdHead+2]==g_tSwTable[i].bSW2))
				{
					DTRACE(DB_ESAM,("SW: %s\r\n", g_tSwTable[i].str));
					break;
				}
			}
		}
		else
			DTRACE(DB_ESAM,("SW: failed\r\n"));
		return -1;
	}

	WORD wDataLen = ((WORD )pbFrm[iStart+2]<<8) + pbFrm[iStart+3];
	if ((int )wLen-iStart < wDataLen+5)
		return -1;

/*
	BYTE bLRC2 = pbFrm[iStart+2];
	for (i=0; i<wDataLen+1; i++)
	{
		bLRC2 ^= pbFrm[iStart+3+i];
	}
	*/
	
	//根据4-22 最新的esam模块资料，LRC2校验包括了SW1和SW2 modify by CPJ at 2013-4-22 17:01:00
	BYTE bLRC2 = pbFrm[iStart];
	for (i=0; i<wDataLen+3; i++)
	{
		bLRC2 ^= pbFrm[iStart+1+i];
	}

	bLRC2 = ~bLRC2;
	
	//DTRACE(DB_ESAM,("bLRC2: %x\r\n", bLRC2));

	if (pbFrm[iStart+4+wDataLen] != bLRC2)
	{
		DTRACE(DB_ESAM,("EsamCheckRxFrm: CheckRxFrm fail. \r\n"));
		return -1;
	}

	*piStart = iStart;
	return wDataLen;
}

//备注：CLA INS P1 P2 Len1 Len2 DATA都已经放好，补充剩余的部分
WORD EsamMakeTxFrm(BYTE* pbFrm, WORD wDataLen)
{
	//发送数据的结构为：55 CLA INS P1 P2 Len1 Len2 DATA LRC1
	//Len1 Len2是后续DATA的长度，不包含LRC1，由两字节表示
	//LRC1的计算方法：对CLA INS P1 P2 Len1 Len2 DATA数据，每个字节的异或值，再取反。
	WORD i;
	pbFrm[0] = 0x55;
	BYTE bLRC1 = pbFrm[1];
	for (i=0; i<wDataLen+5; i++)
	{
		bLRC1 ^= pbFrm[2+i];
	}
	
	bLRC1 = ~bLRC1;

	pbFrm[7+wDataLen] = bLRC1;

	return wDataLen+8;
}


//描述：发送接收一个帧，直接使用pbTx的缓冲，不另外定义缓冲区
//参数	@wExpLen 期待的接收长度
int EsamTxRxFrm(BYTE* pbTx, WORD wDataLen, BYTE* pbRx, WORD wExpLen)
{
	int iLen, iDataLen, iStart = -1;
	WORD wTxLen;
	BYTE bBuf[1024*3] = {0};

	if (g_ifd < 0)
	{
		DTRACE(DB_ESAM,("EsamTxRxFrm: failed due to device not exit\r\n"));
		return -1;
	}

	wTxLen = EsamMakeTxFrm(pbTx, wDataLen);

	/************/ //写之前不读的话收不到数据，有点奇怪，留个疑问后面待查
	//iLen = EsamRead(bBuf, 60);
	//if(iLen>0)
		//TraceBuf(DB_ESAM, "EsamTxRxFrm: read", bBuf, iLen);
		
	Sleep(100);
	memset(bBuf, 0x00, sizeof(bBuf)); 
	/***********/
	
	if (write(g_ifd, pbTx, wTxLen) != wTxLen)
	{
		DTRACE(DB_ESAM,("EsamTxRxFrm: write failed %d\r\n", wTxLen));
		return -1;
	}

	//TraceBuf(DB_ESAM, "EsamTxRxFrm: write", pbTx, wTxLen);
	if(wTxLen > 500)  //长帧会打印不完整，造成错觉，故分开打印下
	{
		BYTE bCircle = wTxLen/500; WORD wLeft = wTxLen%500;
		for(BYTE n=0;n<bCircle;n++ )
		{
			TraceBuf(DB_ESAM, "EsamTxRxFrm: write", pbTx+n*500, 500);
			Sleep(100);	
		}

		if(wLeft > 0)
		{
			TraceBuf(DB_ESAM, "EsamTxRxFrm: write", pbTx+bCircle*500, wLeft);
			Sleep(100);
		}
	}
	else
		TraceBuf(DB_ESAM, "EsamTxRxFrm: write", pbTx, wTxLen);
		
/*
	if (wDataLen>1000 || wExpLen>1000)
		Sleep(800);
	else
		Sleep(300);
*/
	
	//配合4-22的esam芯片，新的片子最长的等待的时间须在1.3s以上，故为统一处理，上层就多等一会，写数据后只要在3s内读完即可
//	Sleep(1300); 

	wExpLen += 32;		//返回有多余数据，多接收一些字节
	if (wExpLen >= sizeof(bBuf))
		wExpLen = sizeof(bBuf);

	iLen = EsamRead(bBuf, wExpLen);
	if (iLen <= 0)
	{
		DTRACE(DB_ESAM,("EsamTxRxFrm: read failed\r\n"));
		return -1;
	}

	

	if ((iDataLen=EsamCheckRxFrm(bBuf, iLen, 0x90, 0x00, &iStart)) < 0)
	{

		//TraceBuf(DB_ESAM, "EsamTxRxFrm: read", bBuf, iLen); //cck-20161213
        g_wEsamCmpDataLen = g_wEsamLastRxDataLen = 0;
		return -1;
	}

	TraceBuf(DB_ESAM, "EsamTxRxFrm: read", bBuf+iStart-1, iDataLen+6);

	if (pbRx != NULL)
		memcpy(pbRx, &bBuf[iStart+4], iDataLen);
	if (g_wEsamLastRxDataLen!=0 && iDataLen==g_wEsamLastRxDataLen)
	{
		if (memcmp(g_bEsamCmpDataBuf, &bBuf[iStart+4], g_wEsamCmpDataLen) == 0)
        {    
            g_wEsamCmpDataLen = g_wEsamLastRxDataLen = 0;   //只捕获一次错误，重发一次，再发生错误就不管了
            DTRACE(DB_ESAM, ("EsamTxRxFrm:Esam retry!\r\n"));
            return -2;
        }
	}
	g_wEsamLastRxDataLen = iDataLen;
	if (iDataLen < 20)
		g_wEsamCmpDataLen = iDataLen;
	else
		g_wEsamCmpDataLen = 20;
	memcpy(g_bEsamCmpDataBuf, &bBuf[iStart+4], g_wEsamCmpDataLen);	
	
	return iDataLen;
}

//描述：发送接收一个命令，这种命令的数据长度为0
int EsamTxRxCmd(const BYTE* pbTx, BYTE* pbRx, WORD wExpLen)
{
	BYTE bBuf[16];
	memcpy(&bBuf[CLA_OFF], pbTx, 6);
	return EsamTxRxFrm(bBuf, 0, pbRx, wExpLen);
}


//新加程序



const WORD wEsamInfoLen[] = {0,82,8,4,16,2,4,4,12,16,16,1494,1494};

//描述：F11	获取终端信息
//返回：如果正确返回数据长度，错误返回负数
int Esam_GetEsamInfo(EsamInfoList tInfo, BYTE* pbRx, WORD wBufLen)
{
    int iDataLen;
	BYTE bcont = 0;

	if(wBufLen < wEsamInfoLen[tInfo])
		return -1;
	
	WaitSemaphore(g_semEsam);

	BYTE bCmdGetEsamInfo[] = {0x80, 0x36, 0x00, 0xFF, 0x00, 0x00};
	if (tInfo == 0x01)
	{
		bCmdGetEsamInfo[3] = 0xFF;
	}
	else
	{
		bCmdGetEsamInfo[3] = tInfo;
	}
	
	do
	{
		bcont++;
		iDataLen = EsamTxRxCmd(bCmdGetEsamInfo, pbRx, wEsamInfoLen[tInfo]);
	}while(iDataLen==-2 && bcont<2);
	if (iDataLen <= 0)	//
	{
		DTRACE(DB_ESAM,("bCmdGetEsamInfo fail. \r\n"));
		goto GetTermInfo_fail_ret;
	}
    

	SignalSemaphore(g_semEsam);
	return iDataLen;

GetTermInfo_fail_ret:
	SignalSemaphore(g_semEsam);
	return -1;
}

//建立应用连接（ 会话密钥协商）
//输入:ucOutSessionInit:32 字节
//     ucOutSign: N 字节
//输出:ucSessionData： 服务器随机数， 48 字节
//     ucSign:服务器签名信息， N字节
int Esam_InitSession(BYTE* pbOutSessionInit, BYTE bOutSessionInitLen, BYTE* pbOutSign, 
		BYTE bOutSignLen, BYTE* pbSessionData, BYTE* pbSessionDataLen, BYTE* pbSign, BYTE* pbSignLen)
{
	//发送：800200000060+ucOutSessionInit+ucOutSign
	//返回：9000+0070+ucSessionData+ucSign
	int iDataLen;
	BYTE bcont = 0;
	BYTE bBuf[256];
	BYTE bRxBuf[128];
	
	WaitSemaphore(g_semEsam);

	BYTE* p;

	do
	{
	    bcont++;
		memset(bBuf, 0x00, sizeof(bBuf));
		p = &bBuf[CLA_OFF];

		*p++ = 0x80;
		*p++ = 0x02;
		*p++ = 0x00;
		*p++ = 0x00;
		
		*p++ = ((WORD)bOutSessionInitLen+bOutSignLen) >> 8;	//Len1代表长度的高字节，
		*p++ = bOutSessionInitLen+bOutSignLen;	//Len2代表长度的低字节
		
		memcpy(p, pbOutSessionInit, bOutSessionInitLen);
		p += bOutSessionInitLen;
		memcpy(p, pbOutSign, bOutSignLen);
		p += bOutSignLen;

		iDataLen = EsamTxRxFrm(bBuf, bOutSessionInitLen+bOutSignLen, bRxBuf, sizeof(bRxBuf));
	}while(iDataLen==-2 && bcont<2 && iDataLen<=48);

	if(iDataLen <= 48)
	{
		DTRACE(DB_ESAM, ("Esam_InitSession: Esam return too lack (%d)  !!\r\n", iDataLen));
		SignalSemaphore(g_semEsam);
		return -1;
	}

	SignalSemaphore(g_semEsam);

	if (*pbSessionDataLen < 48)
	{
		DTRACE(DB_ESAM, ("Esam_InitSession: pbSessionData buf len too lack (%d)  !!\r\n", *pbSessionDataLen));
		return -1;
	}
	*pbSessionDataLen = 48;
	memcpy(pbSessionData, bRxBuf, 48);

	if (*pbSignLen < (iDataLen - 48))
	{
		DTRACE(DB_ESAM, ("Esam_InitSession: pbSign buf len too lack (%d)  !!\r\n", *pbSignLen));
		return -1;
	}
	*pbSignLen = iDataLen - 48;
	memcpy(pbSign, &bRxBuf[48], *pbSignLen);
	
	return iDataLen;
}

/*备注：
明文应用数据单元   [0]
密文应用数据单元   [1]

安全请求:
数据验证码      [0]  SID_MAC，
随机数          [1]  RN，
随机数+数据MAC  [2]  RN_MAC，
安全标识        [3]  SID

安全响应:
数据MAC         [0]  MAC
无				FALSE

P2:
明文+MAC 方式： 11
密文： 96
密文+MAC： 97

*/

//描述：明文+RN --> MAC (明文响应计算MAC，根据主站随机数)
/*备注：
	主站请求:(请求)
		明文应用数据单元   [0]
		随机数          [1]  RN，
	终端回复:(响应)
		明文应用数据单元   [0]
		数据MAC         [0]  MAC	
*/
int Esam_PlnDatResCalMac(BYTE* pbData, WORD wDataLen, BYTE* pbRN, BYTE* pbMac)
{
	//发送： 800E4002+LC+RN+PlainData
	//返回： 9000+0004+MAC
	BYTE* p;
	int iRetLen;
	BYTE bcont = 0;
	BYTE bBuf[2048];

	WORD wTxLen = wDataLen+16; //RN长度为16字节
	BYTE *pbRx = pbMac;

	WaitSemaphore(g_semEsam);
	do
	{
	  	bcont++;
		memset(bBuf, 0x00, sizeof(bBuf));
		p = &bBuf[CLA_OFF];
	
		*p++ = 0x80;
		*p++ = 0x0E;
		*p++ = 0x40;
		*p++ = 0x02;
	
		*p++ = wTxLen>>8;	//Len1代表长度的高字节，
		*p++ = wTxLen;		//Len2代表长度的低字节

		memcpy(p, pbRN, 16);
		p += 16;
		memcpy(p, pbData, wDataLen);
		
		iRetLen = EsamTxRxFrm(bBuf, wTxLen, pbRx, ESAM_MAC_LEN);
		
	}while(iRetLen==-2 && bcont<2);

	if(iRetLen <= 0) 
	{
		DTRACE(DB_ESAM,("EsamOperation fail.\r\n"));
		SignalSemaphore(g_semEsam);
		return -1;
	}

	SignalSemaphore(g_semEsam);
	return iRetLen;
}

//描述：明文 --> RN+MAC (明文计算RN_MAC)
/*备注：
	终端主动上报:(请求)
		明文应用数据单元   [0]
		随机数+数据MAC  [2]  RN_MAC，
*/
int Esam_PlainDataCalRnMac(BYTE* pbData, WORD wDataLen, BYTE* pbRN, BYTE* pbMac)
{
	//发送： 80140103+LC+PlainData
	//返回： 9000+LEN+12 字节随机数+4 字节 MAC
	BYTE* p;
	int iRetLen;
	BYTE bcont = 0;
	BYTE bBuf[2048];
	BYTE bBufRecv[32];
	
	WORD wTxLen = wDataLen; 
	BYTE *pbRx = bBufRecv;

	WaitSemaphore(g_semEsam);
	do
	{
		bcont++;
		memset(bBuf, 0x00, sizeof(bBuf));
		p = &bBuf[CLA_OFF];
	
		*p++ = 0x80;
		*p++ = 0x14;
		*p++ = 0x01;
		*p++ = 0x03;
	
		*p++ = wTxLen>>8;	//Len1代表长度的高字节，
		*p++ = wTxLen;		//Len2代表长度的低字节
		
		memcpy(p, pbData, wDataLen);
		
		iRetLen = EsamTxRxFrm(bBuf, wTxLen, pbRx, 16);
		
	}while(iRetLen==-2 && bcont<2);

	if(iRetLen <= 0) 
	{
		DTRACE(DB_ESAM,("EsamOperation fail.\r\n"));
		SignalSemaphore(g_semEsam);
		return -1;
	}

	SignalSemaphore(g_semEsam);

	memcpy(pbRN, pbRx, 12);
	memcpy(pbMac, pbRx+12, ESAM_MAC_LEN);
	
	return iRetLen;
}

//描述：RN+明文+MAC --> 正确/错误
/*备注：
	主站的响应:
		明文应用数据单元   [0]
		数据MAC         [0]  MAC
*/
bool Esam_CheckResponseMac(BYTE* pbData, WORD wDataLen, BYTE* pbRN, BYTE* pbMac)
{
	//发送： 标识???+随机数(12)+补0(4字节)+明文+MAC
	//返回： ???
	return true;
}

//描述：SID+(RN+明文)+MAC --> 明文
/*备注：
	主站广播:(请求)
		明文应用数据单元   [0]
		数据验证码      [0]  SID_MAC，
*/
int Esam_Broadcast(BYTE* pbSIDMAC, BYTE* pbData, WORD wDataLen, BYTE* pbPlainData, WORD wRxBufLen)
{
	//发送： SID标识+SID附加数据+Data+MAC
	//返回： 9000+LEN+PlainData
	BYTE* p;
	int iRetLen;
	BYTE bcont = 0;
	BYTE bBuf[2048];
	
	BYTE bID[4];
	BYTE bAttachData[256];
	BYTE bAttachDataLen;
	BYTE bMac[256];
	BYTE bMacLen;

	memcpy(bID, pbSIDMAC, 4);
	
	bAttachDataLen = pbSIDMAC[4];
	memset(bAttachData, 0, sizeof(bAttachData));
	memcpy(bAttachData, &pbSIDMAC[5], bAttachDataLen);

	bMacLen = pbSIDMAC[5+bAttachDataLen];
	memset(bMac, 0, sizeof(bMac));
	memcpy(bMac, &pbSIDMAC[5+bAttachDataLen+1], bMacLen);
	
	WORD wTxLen = bAttachDataLen+wDataLen+bMacLen-2; //这里只计算data的长度，不包含标识和L1、L2, SID附加数据中包含了LRC1与LRC2，故减去它的长度。
	BYTE *pbRx = pbPlainData;

	WaitSemaphore(g_semEsam);
	do
	{
		bcont++;
		memset(bBuf, 0x00, sizeof(bBuf));
		p = &bBuf[CLA_OFF];

		//标识
		memcpy(p, bID, 4);
		p += 4;

		//附加数据
		memcpy(p, bAttachData, bAttachDataLen);
		p += bAttachDataLen;

		//DATA
		memcpy(p, pbData, wDataLen);
		p += wDataLen;

		//MAC
		memcpy(p, bMac, bMacLen);
		
		iRetLen = EsamTxRxFrm(bBuf, wTxLen, pbRx, wRxBufLen);
		
	}while(iRetLen==-2 && bcont<2);

	if(iRetLen <= 0) 
	{
		DTRACE(DB_ESAM,("EsamOperation fail.\r\n"));
		SignalSemaphore(g_semEsam);
		return -1;
	}

	SignalSemaphore(g_semEsam);
	
	return iRetLen;
}


//描述：SID+密文 --> 明文
/*备注：
	主站密文请求:(请求)
		密文应用数据单元   [1]
		安全标识        [3]  SID
*/
int Esam_SIDDecode(BYTE* pbSID, BYTE* pbAttachData, WORD wAttachDataLen, 
		BYTE* pbData, WORD wDataLen, BYTE* pbPlainData, WORD wRxBufLen)
{
	//发送： SID标识+SID附加数据+Data
	//返回： 9000+LEN+PlainData
	BYTE* p;
	int iRetLen;
	BYTE bcont = 0;
	BYTE bBuf[2048];
	
	//BYTE bID[4];
	//BYTE bAttachData[256];
	//BYTE bAttachDataLen;

	//memcpy(bID, pbSID, 4);
	
	//bAttachDataLen = pbSID[4];
	//memset(bAttachData, 0, sizeof(bAttachData));
	//memcpy(bAttachData, &pbSID[5], bAttachDataLen);


	WORD wTxLen = wAttachDataLen+wDataLen-2; //这里只计算data的长度，不包含标识和L1、L2, SID附加数据中包含了LRC1与LRC2，故减去它的长度。
	BYTE *pbRx = pbPlainData;

	WaitSemaphore(g_semEsam);
	do
	{
		bcont++;
		memset(bBuf, 0x00, sizeof(bBuf));
		p = &bBuf[CLA_OFF];

		//标识
		memcpy(p, pbSID, 4);
		p += 4;

		//附加数据
		memcpy(p, pbAttachData, wAttachDataLen);
		p += wAttachDataLen;

		//DATA
		memcpy(p, pbData, wDataLen);

		iRetLen = EsamTxRxFrm(bBuf, wTxLen, pbRx, wRxBufLen);
		
	}while(iRetLen==-2 && bcont<2);

	if (iRetLen < 0) //(iRetLen <= 0)
	{
		DTRACE(DB_ESAM,("EsamOperation fail.\r\n"));
		SignalSemaphore(g_semEsam);
		return -1;
	}

	SignalSemaphore(g_semEsam);
	
	return iRetLen;
}


//描述：SID+密文+MAC --> 明文
/*备注：
	主站密文+数据验证码请求:(请求)
		密文应用数据单元   [1]
		数据验证码      [0]  SID_MAC，
*/
int Esam_SIDMACDecode(BYTE* pbSID, BYTE* pbAttachData, WORD wAttachDataLen, BYTE* pbMAC,
			BYTE* pbData, WORD wDataLen, BYTE* pbPlainData, WORD wRxBufLen)
{
	//发送： SID标识+SID附加数据+Data+MAC
	//返回： 9000+LEN+PlainData
	BYTE* p;
	int iRetLen;
	BYTE bcont = 0;
	BYTE bBuf[2048];
	
	//BYTE bID[4];
	//BYTE bAttachData[256];
	//BYTE bAttachDataLen;
	//BYTE bMac[256];
	//BYTE bMacLen;

	//memcpy(bID, pbSIDMAC, 4);
	
	//bAttachDataLen = pbSIDMAC[4];
	//memset(bAttachData, 0, sizeof(bAttachData));
	//memcpy(bAttachData, &pbSIDMAC[5], bAttachDataLen);

	//bMacLen = pbSIDMAC[5+bAttachDataLen];
	//memset(bMac, 0, sizeof(bMac));
	//memcpy(bMac, &pbSIDMAC[5+bAttachDataLen+1], bMacLen);
	
	WORD wTxLen = wAttachDataLen+wDataLen+4-2; //这里只计算data的长度，不包含标识和L1、L2, SID附加数据中包含了LRC1与LRC2，故减去它的长度。
	BYTE *pbRx = pbPlainData;

	WaitSemaphore(g_semEsam);
	do
	{
		bcont++;
		memset(bBuf, 0x00, sizeof(bBuf));
		p = &bBuf[CLA_OFF];

		//标识
		memcpy(p, pbSID, 4);
		p += 4;

		//附加数据
		memcpy(p, pbAttachData, wAttachDataLen);
		p += wAttachDataLen;

		//DATA
		memcpy(p, pbData, wDataLen);
		p += wDataLen;

		//MAC
		memcpy(p, pbMAC, ESAM_MAC_LEN);
		
		iRetLen = EsamTxRxFrm(bBuf, wTxLen, pbRx, wRxBufLen);
		
	}while(iRetLen==-2 && bcont<2);

	if(iRetLen < 0) //(iRetLen <= 0)
	{
		DTRACE(DB_ESAM,("Esam_SIDMACDecode fail.\r\n"));
		SignalSemaphore(g_semEsam);
		return -1;
	}

	SignalSemaphore(g_semEsam);
	
	return iRetLen;
}



//描述：明文 --> 密文
/*备注：
	终端密文回复:(响应)
		密文应用数据单元   [1]
		数据验证信息=FALSE
*/
int Esam_ResMakeEndata(BYTE* pbData, WORD wDataLen, BYTE* pbEnData, WORD wRxBufLen)
{
	//发送： 801C0096+Len+PlainData
	//返回： 9000+LEN+EnData
	BYTE* p;
	int iRetLen;
	BYTE bcont = 0;
	BYTE bBuf[2048];
	
	WaitSemaphore(g_semEsam);
	do
	{
		bcont++;
		memset(bBuf, 0x00, sizeof(bBuf));
		p = &bBuf[CLA_OFF];
	
		*p++ = 0x80;
		*p++ = 0x1C;
		*p++ = 0x00;
		*p++ = 0x96;
	
		*p++ = wDataLen>>8;	//Len1代表长度的高字节，
		*p++ = wDataLen&0xff;		//Len2代表长度的低字节
		
		memcpy(p, pbData, wDataLen);
		
		iRetLen = EsamTxRxFrm(bBuf, wDataLen, pbEnData, wRxBufLen);
		
	}while(iRetLen==-2 && bcont<2);

	if(iRetLen <= 0) 
	{
		DTRACE(DB_ESAM,("EsamOperation fail.\r\n"));
		SignalSemaphore(g_semEsam);
		return -1;
	}

	SignalSemaphore(g_semEsam);
	
	return iRetLen;


}


//描述：明文 --> 密文+MAC
/*备注：
	终端密文+数据验证码回复:(响应)
		密文应用数据单元   [1]
		数据MAC         [0]  MAC
*/
int Esam_ResMakeEndataMac(BYTE* pbData, WORD wDataLen, BYTE* pbEnData, WORD wRxBufLen)
{
	//发送： 801C0096+Len+PlainData
	//返回： 9000+LEN+EnData
	BYTE* p;
	int iRetLen;
	BYTE bcont = 0;
	BYTE bBuf[2048];
	
	WaitSemaphore(g_semEsam);
	do
	{
		bcont++;
		memset(bBuf, 0x00, sizeof(bBuf));
		p = &bBuf[CLA_OFF];
	
		*p++ = 0x80;
		*p++ = 0x1C;
		*p++ = 0x00;
		*p++ = 0x97;
	
		*p++ = wDataLen>>8; //Len1代表长度的高字节，
		*p++ = wDataLen;		//Len2代表长度的低字节
		
		memcpy(p, pbData, wDataLen);
		
		iRetLen = EsamTxRxFrm(bBuf, wDataLen, pbEnData, wRxBufLen);
		
	}while(iRetLen==-2 && bcont<2);

	if(iRetLen <= 0) 
	{
		DTRACE(DB_ESAM,("EsamOperation fail.\r\n"));
		SignalSemaphore(g_semEsam);
		return -1;
	}

	SignalSemaphore(g_semEsam);
	
	return iRetLen;

}

int Esam_ResMakePlndataMac(BYTE* pbData, WORD wDataLen, BYTE* pbEnData, WORD wRxBufLen)
{
	//发送： 801C0011+Len+PlainData
	//返回： 9000+LEN+EnData
	BYTE* p;
	int iRetLen;
	BYTE bcont = 0;
	BYTE bBuf[2048];
	
	WaitSemaphore(g_semEsam);
	do
	{
		bcont++;
		memset(bBuf, 0x00, sizeof(bBuf));
		p = &bBuf[CLA_OFF];
	
		*p++ = 0x80;
		*p++ = 0x1C;
		*p++ = 0x00;
		*p++ = 0x11;
	
		*p++ = wDataLen>>8; //Len1代表长度的高字节，
		*p++ = wDataLen;		//Len2代表长度的低字节
		
		memcpy(p, pbData, wDataLen);
		
		iRetLen = EsamTxRxFrm(bBuf, wDataLen, pbEnData, wRxBufLen);
		
	}while(iRetLen==-2 && bcont<2);

	if(iRetLen <= 0) 
	{
		DTRACE(DB_ESAM,("EsamOperation fail.\r\n"));
		SignalSemaphore(g_semEsam);
		return -1;
	}

	SignalSemaphore(g_semEsam);
	
	return iRetLen;


}

//描述：取随机数
//返回：如果正确返回数据长度，错误返回负数
int EsamGetRandom(BYTE* pbRx)
{
	int iDataLen;
	BYTE bcont = 0;

	WaitSemaphore(g_semEsam);

	const BYTE bCmdGetRn[] = {0x80, 0x04, 0x00, 0x10, 0x00, 0x00};
	
	do
	{
		bcont++;
		iDataLen = EsamTxRxCmd(bCmdGetRn, pbRx, 32);
	}while(iDataLen==-2 && bcont<2);
	if (iDataLen <= 0)	//
	{
		DTRACE(DB_ESAM,("bCmdGetRandom fail. \r\n"));
		goto GetTermInfo_fail_ret;
	}

	SignalSemaphore(g_semEsam);
	return iDataLen;

GetTermInfo_fail_ret:
	SignalSemaphore(g_semEsam);
	return -1;
}

/*
* 参数:BYTE bOpType,操作类型,0-明文+MAC,1-密文,2-密文+MAC
* 参数:BYTE *pMac,当操作模式为1-密文时,该参数传入NULL.
* 参数:BYTE *pDstData,解密完成得到的明文数据，当操作模式为0-明文+MAC时,该参数传入NULL.
* 返回值: pDstData的长度,当操作模式为0-明文+MAC时,返回0;其他操作模式,返回pDstData的数据长度.
*/
int Esam_ReadMtrDataVerify(BYTE bOpType, BYTE *pMtrNum, WORD wMtrNumLen, BYTE *pRn,
		WORD wRnLen, BYTE *pSrcData, WORD wSrcDataLen, BYTE *pMac, BYTE *pDstData)
{
	BYTE* p;
	int iRetLen;
	BYTE bcont = 0;
	BYTE bTxBuf[2048];
	WORD wTxLen;
	const BYTE bOpCmd[3][4] = {	{0x80, 0x0E, 0x48, 0x87},
								{0x80, 0x0C, 0x48, 0x07},
								{0x80, 0x12, 0x48, 0x07}};

	if ((bOpType!=1) && (pMac==NULL))
		return -1;
	if((bOpType!=0) && (pDstData==NULL))
		return -1;
	
	switch(bOpType)
	{
	case 0:
	case 2:
		wTxLen = wMtrNumLen+wRnLen+wSrcDataLen+ESAM_MAC_LEN;
		break;
	case 1:
		wTxLen = wMtrNumLen+wRnLen+wSrcDataLen;
		break;
	default:
		return -1;
	}

	WaitSemaphore(g_semEsam);
	do
	{
	  	bcont++;
		memset(bTxBuf, 0x00, sizeof(bTxBuf));
		p = &bTxBuf[CLA_OFF];

		memcpy(p, bOpCmd[bOpType], 4);
		p += 4;
	
		*p++ = wTxLen>>8;	//Len1代表长度的高字节，
		*p++ = wTxLen;		//Len2代表长度的低字节

		memcpy(p, pMtrNum, wMtrNumLen);
		p += wMtrNumLen;
		memcpy(p, pRn, wRnLen);
		p += wRnLen;
		memcpy(p, pSrcData, wSrcDataLen);
		p += wSrcDataLen;

		if (bOpType != 1)
		{
			memcpy(p, pMac, ESAM_MAC_LEN);
			p += ESAM_MAC_LEN;
		}
		
		iRetLen = EsamTxRxFrm(bTxBuf, wTxLen, pDstData, 2048);
		
	}while(iRetLen==-2 && bcont<2);

	if(iRetLen < 0) //(iRetLen <= 0)
	{
		DTRACE(DB_ESAM,("EsamOperation fail.\r\n"));
		SignalSemaphore(g_semEsam);
		return -1;
	}

	SignalSemaphore(g_semEsam);
	return iRetLen;
}


// OI Method
int EsamResetMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	return true;
}

int EsamExeMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	return true;
}

int EsamDataReadMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	return true;
}

int EsamDataUpdateMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	return true;
}

int EsamConsultFailMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	return true;
}

int EsamWalletOpMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	return true;
}

int EsamKeyUpdateMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
/*
密钥更新（参数）
参数∷=structure
{
密钥密文   octet-string，
数据验证码 SID_MAC
}
*/
	const BYTE c_bKeyUpdateSID[4] = {0x81, 0x2E, 0x00, 0x00};
	int iPos;
	DWORD dwKeyLen, dwAttLen, dwMacLen;
	BYTE *pKey, *pAttData, *pMac, *pSID;
	BYTE bTempBuf[16];
	WORD wBufLen = 16;
	
	pbPara += 3; //跳到密钥密文
	
	iPos = DecodeLength(pbPara, &dwKeyLen);
	pbPara += iPos;
	
	pKey = pbPara; // ENDATA1
	pbPara += dwKeyLen;
	
	pbPara++; //跳到SID_MAC
	
	pSID = pbPara; // SID
	if (memcmp(c_bKeyUpdateSID, pSID, 4) != 0)
		return false;
	pbPara += 4;

	iPos = DecodeLength(pbPara, &dwAttLen); // ATTLEN
	pbPara += iPos;

	pAttData = pbPara; //ATTDATA
	pbPara += dwAttLen;

	iPos = DecodeLength(pbPara, &dwMacLen); // MACLEN
	pbPara += iPos;

	pMac= pbPara; //MAC
	pbPara += dwMacLen;
	
	if (Esam_SIDMACDecode(pSID, pAttData, dwAttLen, pMac, pKey, dwKeyLen, 
		bTempBuf, wBufLen) >= 0)
	{
		EsamReInit();
		return true;
	}
	else
	{
		EsamReInit();
		return false;
	}
}

int EsamCerUpdateMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
/*	
证书更新（参数）
参数∷=structure
{
    证书内容  octet-string，
    安全标识  SID
}
*/
	const BYTE c_bCerUpdateSID[4] = {0x81, 0x30, 0x02, 0x03};
	int iPos;
	DWORD dwCerLen, dwAttLen;
	BYTE *pCertificate, *pAttData, *pSID;
	BYTE bTempBuf[16];
	WORD wBufLen = 16;
	
	pbPara += 3; //跳到证书密文
	
	iPos = DecodeLength(pbPara, &dwCerLen);
	pbPara += iPos;
	
	pCertificate = pbPara; // ENDATA1
	pbPara += dwCerLen;
	
	pbPara++; //跳到SID
	
	pSID = pbPara; // SID
	if (memcmp(c_bCerUpdateSID, pSID, 4) != 0)
		return false;
	pbPara += 4;

	iPos = DecodeLength(pbPara, &dwAttLen); // ATTLEN
	pbPara += iPos;

	pAttData = pbPara; //ATTDATA
	pbPara += dwAttLen;

	if (Esam_SIDDecode(pSID, pAttData, dwAttLen, pCertificate, dwCerLen, 
		bTempBuf, wBufLen) >= 0)
	{
		EsamReInit();
		return true;
	}
	else
	{
		EsamReInit();
		return false;
	}

}

int EsamSetConsultTimeBarMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
/*	
设置协商时效（参数）
参数∷=structure 
{
参数内容  octet-string，
安全标识  SID
} 
*/
	const BYTE c_bTimeBarUpdateSID[4] = {0x81, 0x34, 0x01, 0x05};
	int iPos;
	DWORD dwTimeBarLen, dwAttLen;
	BYTE *pTimeBar, *pAttData, *pSID;
	BYTE bTempBuf[16];
	WORD wBufLen = 16;
	
	pbPara += 3; //跳到证书密文
	
	iPos = DecodeLength(pbPara, &dwTimeBarLen);
	pbPara += iPos;
	
	pTimeBar = pbPara; // ENDATA1
	pbPara += dwTimeBarLen;
	
	pbPara++; //跳到SID
	
	pSID = pbPara; // SID
	if (memcmp(c_bTimeBarUpdateSID, pSID, 4) != 0)
		return false;
	pbPara += 4;

	iPos = DecodeLength(pbPara, &dwAttLen); // ATTLEN
	pbPara += iPos;

	pAttData = pbPara; //ATTDATA
	pbPara += dwAttLen;

	if (Esam_SIDDecode(pSID, pAttData, dwAttLen, pTimeBar, dwTimeBarLen, 
		bTempBuf, wBufLen) >= 0)
	{
		Esam_ReflashSessionMaxTime();
		return true;
	}
	else
	{
		Esam_ReflashSessionMaxTime();
		return false;
	}

}

int EsamWalletInitMethod(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	return true;
}
