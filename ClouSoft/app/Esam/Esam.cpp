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


// IEC7816��������
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
BYTE g_bCertState = 0;	//֤��״̬��00Ϊ����֤�飻01Ϊ��ʽ֤��
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
	{0x90,0x00,"OK"},//ָ��ִ�гɹ�
	{0x63,0xcf,"Cert Or Swt failed"},//��֤ʧ��
	{0x64,0x00,"InExcu wrong"},//�ڲ�ִ�г���
	{0x65,0x81,"Card locked"},//��������
	{0x67,0x00,"Len wrong"},//Lc��Le���ȴ�
	{0x69,0x01,"Cntzero Or Cmd wrong"},//���߼�����Ϊ0���������
	{0x69,0x82,"No Safe sta"},//�����㰲ȫ״̬
	{0x69,0x83,"Kut zero"},//Kutʹ�ô���Ϊ0
	{0x69,0x84,"Use Data invalid"},//����������Ч
	{0x69,0x85,"Condition Incomplete"},//ʹ������������
	{0x69,0x86,"Online Cnt zero"},//���߼�����Ϊ0
	{0x69,0x88,"MAC wrong"},//MAC����
	{0x6a,0x80,"Para wrong"},//��������
	{0x6a,0x86,"P1P2 wrong"},//��������
	{0x6a,0x88,"No Find data"},//δ�ҵ���������
	{0x6d,0x00,"Cmd No exist"},//�������
	{0x6e,0x00,"Cmd Or CLA wrong "},//�����CLA��
	{0x6f,0x00,"Data invalid"},//������Ч
	{0x90,0x86,"CheckSign wrong"},//��ǩ����
	{0x9e,0x2f,"File wrong"},//�ļ�����
	{0x9e,0x3f,"Calc wrong"},//�㷨�������
	{0x9e,0x57,"Cert wrong"},//��֤����
	{0x9e,0x60,"Session wrong"},//�����Ự����
	{0x9e,0x5e,"CA wrong"},//CA֤�����
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

// ��һά����Ԫ�ص���
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
	BYTE bWeek = DayOfWeek(time); //DayOfWeek()�ķ��� 1 = Sunday, 2 = Monday, ..., 7 = Saturday
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


	//������ȡ��ȫоƬ��Ϣ:
	BYTE bAttributeLen,bAttributePos;
	iDataLen = Esam_GetEsamInfo(AllEsamInfo, bBuf1, sizeof(bBuf1));

	//DTRACE(DB_CRITICAL,("EsamInfo len %d(0x%x).\r\n", iDataLen, iDataLen));
	if (iDataLen > 0)
	{
		TraceBuf(DB_ESAM, "EsamInfo:", bBuf1, iDataLen);


		//ESAM���к� //g_bEsamSerialNumFmt
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

	//��ȡ�ն�֤��
	iDataLen = Esam_GetEsamInfo(TermCertificate, bBuf1, sizeof(bBuf1));
	if (iDataLen > 0)
	{	
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(iDataLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], bBuf1, iDataLen);
		WriteItemEx(BANK0, PN0, 0xF10A, bBuf2); //g_bEsamTermCertificateFmt
	}
	
	//��ȡ��վ֤��
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
	//֤��״̬��00Ϊ����֤�飻01Ϊ��ʽ֤��
	//��ȡоƬ״̬��Ϣ1�ֽ�
	//���ͣ�800E00050000
	//���أ�9000+LEN+оƬ״̬��Ϣ
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

	//������ȡ��ȫоƬ��Ϣ:
	BYTE bAttributeLen,bAttributePos;
	iDataLen = Esam_GetEsamInfo(AllEsamInfo, bBuf1, sizeof(bBuf1));

	//DTRACE(DB_CRITICAL,("EsamInfo len %d(0x%x).\r\n", iDataLen, iDataLen));
	if (iDataLen > 0)
	{
		TraceBuf(DB_ESAM, "EsamInfo:", bBuf1, iDataLen);


		//ESAM���к� //g_bEsamSerialNumFmt
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

	//��ȡ�ն�֤��
	iDataLen = Esam_GetEsamInfo(TermCertificate, bBuf1, sizeof(bBuf1));
	if (iDataLen > 0)
	{	
		bBuf2[0] = DT_OCT_STR;
		bOctLen = EncodeLength(iDataLen, &bBuf2[1]);
		memcpy(&bBuf2[bOctLen+1], bBuf1, iDataLen);
		WriteItemEx(BANK0, PN0, 0xF10A, bBuf2); //g_bEsamTermCertificateFmt
	}
	
	//��ȡ��վ֤��
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
	//֤��״̬��00Ϊ����֤�飻01Ϊ��ʽ֤��
	//��ȡоƬ״̬��Ϣ1�ֽ�
	//���ͣ�800E00050000
	//���أ�9000+LEN+оƬ״̬��Ϣ
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

//������@pbBuf ���ջ�����
//		@wExpLen �ڴ��Ľ��ճ���
int EsamRead(BYTE* pbBuf, WORD wExpLen)
{
	int iLen = 0; 
	if (wExpLen > 0) 
	{
		for (int i=0; i<6; i++) 
		{
			iLen = read(g_ifd, pbBuf, wExpLen);	//д��ʱ���Զ��壬ƽ�����Զ�ζ�
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
	//�������ݵĽṹΪ��SW1 SW2 Len1 Len2 DATA LRC2
	//LRC2�ļ��㷽������Len1 Len2 DATA���ݣ�ÿ���ֽڵ����ֵ����ȡ����
	//Len1�����ȵĸ��ֽڣ�Len2�����ȵĵ��ֽ�
	//Len1 Len2����DATA��ĳ��ȣ�������LRC1��LRC2
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
	
	//����4-22 ���µ�esamģ�����ϣ�LRC2У�������SW1��SW2 modify by CPJ at 2013-4-22 17:01:00
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

//��ע��CLA INS P1 P2 Len1 Len2 DATA���Ѿ��źã�����ʣ��Ĳ���
WORD EsamMakeTxFrm(BYTE* pbFrm, WORD wDataLen)
{
	//�������ݵĽṹΪ��55 CLA INS P1 P2 Len1 Len2 DATA LRC1
	//Len1 Len2�Ǻ���DATA�ĳ��ȣ�������LRC1�������ֽڱ�ʾ
	//LRC1�ļ��㷽������CLA INS P1 P2 Len1 Len2 DATA���ݣ�ÿ���ֽڵ����ֵ����ȡ����
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


//���������ͽ���һ��֡��ֱ��ʹ��pbTx�Ļ��壬�����ⶨ�建����
//����	@wExpLen �ڴ��Ľ��ճ���
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

	/************/ //д֮ǰ�����Ļ��ղ������ݣ��е���֣��������ʺ������
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
	if(wTxLen > 500)  //��֡���ӡ����������ɴ�����ʷֿ���ӡ��
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
	
	//���4-22��esamоƬ���µ�Ƭ����ĵȴ���ʱ������1.3s���ϣ���Ϊͳһ�����ϲ�Ͷ��һ�ᣬд���ݺ�ֻҪ��3s�ڶ��꼴��
//	Sleep(1300); 

	wExpLen += 32;		//�����ж������ݣ������һЩ�ֽ�
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
            g_wEsamCmpDataLen = g_wEsamLastRxDataLen = 0;   //ֻ����һ�δ����ط�һ�Σ��ٷ�������Ͳ�����
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

//���������ͽ���һ�����������������ݳ���Ϊ0
int EsamTxRxCmd(const BYTE* pbTx, BYTE* pbRx, WORD wExpLen)
{
	BYTE bBuf[16];
	memcpy(&bBuf[CLA_OFF], pbTx, 6);
	return EsamTxRxFrm(bBuf, 0, pbRx, wExpLen);
}


//�¼ӳ���



const WORD wEsamInfoLen[] = {0,82,8,4,16,2,4,4,12,16,16,1494,1494};

//������F11	��ȡ�ն���Ϣ
//���أ������ȷ�������ݳ��ȣ����󷵻ظ���
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

//����Ӧ�����ӣ� �Ự��ԿЭ�̣�
//����:ucOutSessionInit:32 �ֽ�
//     ucOutSign: N �ֽ�
//���:ucSessionData�� ������������� 48 �ֽ�
//     ucSign:������ǩ����Ϣ�� N�ֽ�
int Esam_InitSession(BYTE* pbOutSessionInit, BYTE bOutSessionInitLen, BYTE* pbOutSign, 
		BYTE bOutSignLen, BYTE* pbSessionData, BYTE* pbSessionDataLen, BYTE* pbSign, BYTE* pbSignLen)
{
	//���ͣ�800200000060+ucOutSessionInit+ucOutSign
	//���أ�9000+0070+ucSessionData+ucSign
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
		
		*p++ = ((WORD)bOutSessionInitLen+bOutSignLen) >> 8;	//Len1�����ȵĸ��ֽڣ�
		*p++ = bOutSessionInitLen+bOutSignLen;	//Len2�����ȵĵ��ֽ�
		
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

/*��ע��
����Ӧ�����ݵ�Ԫ   [0]
����Ӧ�����ݵ�Ԫ   [1]

��ȫ����:
������֤��      [0]  SID_MAC��
�����          [1]  RN��
�����+����MAC  [2]  RN_MAC��
��ȫ��ʶ        [3]  SID

��ȫ��Ӧ:
����MAC         [0]  MAC
��				FALSE

P2:
����+MAC ��ʽ�� 11
���ģ� 96
����+MAC�� 97

*/

//����������+RN --> MAC (������Ӧ����MAC��������վ�����)
/*��ע��
	��վ����:(����)
		����Ӧ�����ݵ�Ԫ   [0]
		�����          [1]  RN��
	�ն˻ظ�:(��Ӧ)
		����Ӧ�����ݵ�Ԫ   [0]
		����MAC         [0]  MAC	
*/
int Esam_PlnDatResCalMac(BYTE* pbData, WORD wDataLen, BYTE* pbRN, BYTE* pbMac)
{
	//���ͣ� 800E4002+LC+RN+PlainData
	//���أ� 9000+0004+MAC
	BYTE* p;
	int iRetLen;
	BYTE bcont = 0;
	BYTE bBuf[2048];

	WORD wTxLen = wDataLen+16; //RN����Ϊ16�ֽ�
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
	
		*p++ = wTxLen>>8;	//Len1�����ȵĸ��ֽڣ�
		*p++ = wTxLen;		//Len2�����ȵĵ��ֽ�

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

//���������� --> RN+MAC (���ļ���RN_MAC)
/*��ע��
	�ն������ϱ�:(����)
		����Ӧ�����ݵ�Ԫ   [0]
		�����+����MAC  [2]  RN_MAC��
*/
int Esam_PlainDataCalRnMac(BYTE* pbData, WORD wDataLen, BYTE* pbRN, BYTE* pbMac)
{
	//���ͣ� 80140103+LC+PlainData
	//���أ� 9000+LEN+12 �ֽ������+4 �ֽ� MAC
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
	
		*p++ = wTxLen>>8;	//Len1�����ȵĸ��ֽڣ�
		*p++ = wTxLen;		//Len2�����ȵĵ��ֽ�
		
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

//������RN+����+MAC --> ��ȷ/����
/*��ע��
	��վ����Ӧ:
		����Ӧ�����ݵ�Ԫ   [0]
		����MAC         [0]  MAC
*/
bool Esam_CheckResponseMac(BYTE* pbData, WORD wDataLen, BYTE* pbRN, BYTE* pbMac)
{
	//���ͣ� ��ʶ???+�����(12)+��0(4�ֽ�)+����+MAC
	//���أ� ???
	return true;
}

//������SID+(RN+����)+MAC --> ����
/*��ע��
	��վ�㲥:(����)
		����Ӧ�����ݵ�Ԫ   [0]
		������֤��      [0]  SID_MAC��
*/
int Esam_Broadcast(BYTE* pbSIDMAC, BYTE* pbData, WORD wDataLen, BYTE* pbPlainData, WORD wRxBufLen)
{
	//���ͣ� SID��ʶ+SID��������+Data+MAC
	//���أ� 9000+LEN+PlainData
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
	
	WORD wTxLen = bAttachDataLen+wDataLen+bMacLen-2; //����ֻ����data�ĳ��ȣ���������ʶ��L1��L2, SID���������а�����LRC1��LRC2���ʼ�ȥ���ĳ��ȡ�
	BYTE *pbRx = pbPlainData;

	WaitSemaphore(g_semEsam);
	do
	{
		bcont++;
		memset(bBuf, 0x00, sizeof(bBuf));
		p = &bBuf[CLA_OFF];

		//��ʶ
		memcpy(p, bID, 4);
		p += 4;

		//��������
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


//������SID+���� --> ����
/*��ע��
	��վ��������:(����)
		����Ӧ�����ݵ�Ԫ   [1]
		��ȫ��ʶ        [3]  SID
*/
int Esam_SIDDecode(BYTE* pbSID, BYTE* pbAttachData, WORD wAttachDataLen, 
		BYTE* pbData, WORD wDataLen, BYTE* pbPlainData, WORD wRxBufLen)
{
	//���ͣ� SID��ʶ+SID��������+Data
	//���أ� 9000+LEN+PlainData
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


	WORD wTxLen = wAttachDataLen+wDataLen-2; //����ֻ����data�ĳ��ȣ���������ʶ��L1��L2, SID���������а�����LRC1��LRC2���ʼ�ȥ���ĳ��ȡ�
	BYTE *pbRx = pbPlainData;

	WaitSemaphore(g_semEsam);
	do
	{
		bcont++;
		memset(bBuf, 0x00, sizeof(bBuf));
		p = &bBuf[CLA_OFF];

		//��ʶ
		memcpy(p, pbSID, 4);
		p += 4;

		//��������
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


//������SID+����+MAC --> ����
/*��ע��
	��վ����+������֤������:(����)
		����Ӧ�����ݵ�Ԫ   [1]
		������֤��      [0]  SID_MAC��
*/
int Esam_SIDMACDecode(BYTE* pbSID, BYTE* pbAttachData, WORD wAttachDataLen, BYTE* pbMAC,
			BYTE* pbData, WORD wDataLen, BYTE* pbPlainData, WORD wRxBufLen)
{
	//���ͣ� SID��ʶ+SID��������+Data+MAC
	//���أ� 9000+LEN+PlainData
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
	
	WORD wTxLen = wAttachDataLen+wDataLen+4-2; //����ֻ����data�ĳ��ȣ���������ʶ��L1��L2, SID���������а�����LRC1��LRC2���ʼ�ȥ���ĳ��ȡ�
	BYTE *pbRx = pbPlainData;

	WaitSemaphore(g_semEsam);
	do
	{
		bcont++;
		memset(bBuf, 0x00, sizeof(bBuf));
		p = &bBuf[CLA_OFF];

		//��ʶ
		memcpy(p, pbSID, 4);
		p += 4;

		//��������
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



//���������� --> ����
/*��ע��
	�ն����Ļظ�:(��Ӧ)
		����Ӧ�����ݵ�Ԫ   [1]
		������֤��Ϣ=FALSE
*/
int Esam_ResMakeEndata(BYTE* pbData, WORD wDataLen, BYTE* pbEnData, WORD wRxBufLen)
{
	//���ͣ� 801C0096+Len+PlainData
	//���أ� 9000+LEN+EnData
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
	
		*p++ = wDataLen>>8;	//Len1�����ȵĸ��ֽڣ�
		*p++ = wDataLen&0xff;		//Len2�����ȵĵ��ֽ�
		
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


//���������� --> ����+MAC
/*��ע��
	�ն�����+������֤��ظ�:(��Ӧ)
		����Ӧ�����ݵ�Ԫ   [1]
		����MAC         [0]  MAC
*/
int Esam_ResMakeEndataMac(BYTE* pbData, WORD wDataLen, BYTE* pbEnData, WORD wRxBufLen)
{
	//���ͣ� 801C0096+Len+PlainData
	//���أ� 9000+LEN+EnData
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
	
		*p++ = wDataLen>>8; //Len1�����ȵĸ��ֽڣ�
		*p++ = wDataLen;		//Len2�����ȵĵ��ֽ�
		
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
	//���ͣ� 801C0011+Len+PlainData
	//���أ� 9000+LEN+EnData
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
	
		*p++ = wDataLen>>8; //Len1�����ȵĸ��ֽڣ�
		*p++ = wDataLen;		//Len2�����ȵĵ��ֽ�
		
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

//������ȡ�����
//���أ������ȷ�������ݳ��ȣ����󷵻ظ���
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
* ����:BYTE bOpType,��������,0-����+MAC,1-����,2-����+MAC
* ����:BYTE *pMac,������ģʽΪ1-����ʱ,�ò�������NULL.
* ����:BYTE *pDstData,������ɵõ����������ݣ�������ģʽΪ0-����+MACʱ,�ò�������NULL.
* ����ֵ: pDstData�ĳ���,������ģʽΪ0-����+MACʱ,����0;��������ģʽ,����pDstData�����ݳ���.
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
	
		*p++ = wTxLen>>8;	//Len1�����ȵĸ��ֽڣ�
		*p++ = wTxLen;		//Len2�����ȵĵ��ֽ�

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
��Կ���£�������
������=structure
{
��Կ����   octet-string��
������֤�� SID_MAC
}
*/
	const BYTE c_bKeyUpdateSID[4] = {0x81, 0x2E, 0x00, 0x00};
	int iPos;
	DWORD dwKeyLen, dwAttLen, dwMacLen;
	BYTE *pKey, *pAttData, *pMac, *pSID;
	BYTE bTempBuf[16];
	WORD wBufLen = 16;
	
	pbPara += 3; //������Կ����
	
	iPos = DecodeLength(pbPara, &dwKeyLen);
	pbPara += iPos;
	
	pKey = pbPara; // ENDATA1
	pbPara += dwKeyLen;
	
	pbPara++; //����SID_MAC
	
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
֤����£�������
������=structure
{
    ֤������  octet-string��
    ��ȫ��ʶ  SID
}
*/
	const BYTE c_bCerUpdateSID[4] = {0x81, 0x30, 0x02, 0x03};
	int iPos;
	DWORD dwCerLen, dwAttLen;
	BYTE *pCertificate, *pAttData, *pSID;
	BYTE bTempBuf[16];
	WORD wBufLen = 16;
	
	pbPara += 3; //����֤������
	
	iPos = DecodeLength(pbPara, &dwCerLen);
	pbPara += iPos;
	
	pCertificate = pbPara; // ENDATA1
	pbPara += dwCerLen;
	
	pbPara++; //����SID
	
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
����Э��ʱЧ��������
������=structure 
{
��������  octet-string��
��ȫ��ʶ  SID
} 
*/
	const BYTE c_bTimeBarUpdateSID[4] = {0x81, 0x34, 0x01, 0x05};
	int iPos;
	DWORD dwTimeBarLen, dwAttLen;
	BYTE *pTimeBar, *pAttData, *pSID;
	BYTE bTempBuf[16];
	WORD wBufLen = 16;
	
	pbPara += 3; //����֤������
	
	iPos = DecodeLength(pbPara, &dwTimeBarLen);
	pbPara += iPos;
	
	pTimeBar = pbPara; // ENDATA1
	pbPara += dwTimeBarLen;
	
	pbPara++; //����SID
	
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
