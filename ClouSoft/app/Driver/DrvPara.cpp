 /*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DrvPara.cpp
 * ժ    Ҫ��ʵ�ּ���������������Ҫ���û���������������Ĳ������ض���������ʵ��
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2009��7��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
 * ��    ע�����ڲ�ͬ��Ӧ�ü��صĲ�����ʽ�ȶ�����ͬ��Ϊ�˱�֤������ͨ���ԣ��ⲿ�����ݶ��ŵ����
*********************************************************************************************************/
#include "stdafx.h"
#include "DbAPI.h"
#include "YX.h"
#include "YK.h"
#include "Pulse.h"
#include "bios.h"
#include "DrvAPI.h"
//#include "DrvStruct.h"
#include "DrvConst.h"
#include "DrvPara.h"
#include "DcFmt.h"
#include "ComAPI.h"
#include "DbOIAPI.h"
#include "FaAPI.h"
#ifndef SYS_WIN
	#ifdef BIG_LCD
		#include "st7529.h"
	#endif
	#ifdef SMALL_LCD
		#include "ks0108.h"
	#endif
#endif

#ifdef SYS_WIN
	extern TYxPara g_YxPara;
	extern TYkPara g_YkPara;
#else
	TYxPara g_YxPara;
	TYkPara g_YkPara;
#endif

#define	BIT_REVERSE		1		//oct-stringλ˳���Ƿ���

void InitLcd()
{
	//���Ը��ݲ�ͬ�Ĳ������г�ʼ��Һ������
#ifndef SYS_WIN
	#ifdef BIG_LCD
		g_pLcd = new CST7529();
	#endif
	#ifdef  SMALL_LCD
		g_pLcd = new CKS0108();
	#endif
	
	if (g_pLcd != NULL)
	{
		//g_pLcd->Reset();
		g_pLcd->Init();
		g_pLcd->InitHzk();
	}
#endif
}

void InitYX()
{
    //ң�Ų���
	YXLoadPara(&g_YxPara);
	YXInit(&g_YxPara,4);
}

void InitYK()
{
	BYTE bBuf[10];
	ReadItemEx(BN1, PN0, 0x2022, bBuf);  //�̵��������ʽ,0��ƽ,1����
	g_YkPara.wMode = bBuf[0];   //ң�������ʽ:YK_MODE_LEVEL | YK_MODE_PULSE
	
	WORD wPulseWide = 0;
	if(ReadItemEx(BN10, PN0, 0xa1be, bBuf) > 0)
	{
		wPulseWide = BcdToDWORD(bBuf, 2);
	}
	else
	{
		wPulseWide = 3000;
	}
	g_YkPara.wPulseWidth = wPulseWide;			//������,��λ����
	g_YkPara.wSafeTime = 0;	//10�ϵ籣��ʱ��,��λ����,0��ʾ������
	g_YkPara.dwFastDist = 0;  //�������,��λ����,�ڴ�ʱ����,����ÿ������һ��
							   //������ʱ��,����ÿdwSlowInterv������һ��
							   //�������Ϊ0,�򶼰���ÿ������һ��
	g_YkPara.dwSlowInterv = 1; //�������,��λ����
		
	g_YkPara.dwValidTurn = 4; //��Ч�ִ� YK�ɵ�Ƭ�����ƣ�������ʵ������ȥ����
	YKInit(&g_YkPara,4);
}




//02 02 04 08 FF 04 08 FF
//0  1  2  3  4  5  6  7
void YXLoadPara(TYxPara* pYxPara)
{
	int iLen;
    BYTE bTmp = 0;
	BYTE bBuf[10];
#ifndef SYS_WIN
	BYTE bYMFlag = g_PulseManager.GetYMFlag();	//����ռ�ñ�־
#else
	BYTE bYMFlag = 0;
#endif
	const WORD wOI = OI_YX;
	
	bYMFlag = ~bYMFlag&0x0f;		//��λȡ��	
 
	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, ATTR4, bBuf, NULL, NULL);	//ȡ��������������Բ���
	if (iLen>0 && bBuf[2]==DT_BIT_STR && bBuf[5]==DT_BIT_STR)
	{
	    DTRACE(DB_CRITICAL, ("YXLoadPara : bBuf[4]=%x, bBuf[7] =%x.\n", bBuf[4], bBuf[7]));

		#ifdef BIT_REVERSE
		bBuf[4] = BitReverse(bBuf[4]);
		bBuf[7] = BitReverse(bBuf[7]);
		DTRACE(DB_CRITICAL, ("YXLoadPara : after bit reverse bBuf[4]=%x, bBuf[7] =%x.\n", bBuf[4], bBuf[7]));
		#endif

		iLen = ReadItemEx(BN1, PN0, 0x2040, &bTmp);	//ң�������Ƿ��� 0�������� 1������ 
		if (iLen < 0)
			bTmp = 0;

	    if (bTmp != 0)	//ң�����干��
			pYxPara->wYxFlag = (bYMFlag & (bBuf[4]&0x000f)) & 0x000f;
		else	//������
			pYxPara->wYxFlag = bBuf[4] & 0x000f;

		pYxPara->wYxFlag |= 0x10; //�Ž���̶���Ч

		BYTE bVal = 0;
		ReadItemEx(BN10, PN0, 0xa1a6, &bVal);
		if (bVal > 0)
			pYxPara->wYxPolar = (~bBuf[7]) & 0x000f;
		else
			pYxPara->wYxPolar = (bBuf[7]) & 0x000f;
		DTRACE(DB_CRITICAL, ("YXLoadPara : pYxPara->wYxFlag=%x, pYxPara->wYxPolar =%x.\n", pYxPara->wYxFlag, pYxPara->wYxPolar));
	}
	else
	{
		pYxPara->wYxFlag = bYMFlag & 0x000f;
		pYxPara->wYxFlag |= 0x10; //�Ž���̶���Ч
		pYxPara->wYxPolar = 0x000f;		//Ĭ��a�ʹ���
	}

	SetYxInitFlag(false);	//��������־��ʼ����ÿ���޸Ĳ��������³�ʼ��һ��
}

void TransferLidStatus()
{
	//������Ҫ�洢���
	//����GetLidStatus()������1β�Ǳ��򿪣�0�ر�
	//printf("@@@@@@@@@@@@@@@@@@@%d@@@@@@@@@@@@@@@@\r\n", GetLidStatus());
}

void DcLoadAdjPara(BYTE* pbBuf)
{
	ReadItemEx(BN25, PN0, 0x5011, pbBuf);
}

bool SaveTrigerAdj(BYTE* pbBuf)
{
	WriteItemEx(BN25, PN0, 0x5011, pbBuf);
	TrigerSaveBank(BN25, 0, -1);
	return true;
}

WORD GetTermCurrent(WORD wCtCn)
{
	BYTE bBuf[16];
	ReadItem(PN0,0xb621+wCtCn,bBuf);
#ifdef PRO_698
	return BcdToDWORD(bBuf, 3)/10;
#else
	return BcdToDWORD(bBuf, 2);
#endif
}

bool InitDrvPara()
{
	BYTE bMode = 1;
	WriteItemEx(BN2, BN0, 0x2040, &bMode);
#ifndef SYS_WIN
	InitDcValToDb(PN0);
	SetLogFileAddr(g_wLogFileAddr, sizeof(g_wLogFileAddr));
#endif
	return true;
}


//������־�����  ��ƽ̨����Ҫ�޸�
/*WORD g_wLogFileAddr[512] = {0x00, 		256, 	 	//0���ɵ���256,���30*5+1
							512, 		512+256,  	//1��������256,���10*20+1
							1024, 	1024+32, 	//2ͳ��32
							1088, 	1088+32,	 	//3����1����32
							1088+32*2,	1088+32*3,	//4����2����32
							1088+32*4,	1088+32*5,	//5����3����32
							1088+32*6,	1088+32*7,	//6����4����32
							1344,		1344+40*1,	//7����1����40
							1344+40*2,	1344+40*3,	//8����2����40
							1344+40*4,	1344+40*5,	//9����3����40
							1344+40*6,	1344+40*7,	//10����4����40
							1664,		1664+172};	//11���ɵ���δ������С���㣬һ��42������ÿ����4�ֽڣ���һ������
*/
//������־�����  ��ƽ̨����Ҫ�޸�
WORD g_wLogFileAddr[512] = {0x00,		256,		//0���ɵ���256,���39*5+1
							512,		512+380,	//1��������256,���34*11+1
							1272,	1272+32,		//2ͳ��32-5
							1336,	1336+32,		//3����1����32
							1400+32*2,	1400+32*3,	//4����2����32
							1464,		1464+40*1,	//5����1����40
							1608+40*2,	1608+40*3,	//6����2����40
							1688,		1688+174};	//7 174���ɵ���δ������С���㣬һ��42������ÿ����4�ֽڣ���һ������

