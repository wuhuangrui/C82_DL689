 /*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DrvPara.cpp
 * 摘    要：实现加载驱动参数，需要由用户传入参数的驱动的参数加载都放在这里实现
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2009年7月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
 * 备    注：由于不同的应用加载的参数格式等都不相同，为了保证驱动的通用性，这部分内容都放到这里。
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

#define	BIT_REVERSE		1		//oct-string位顺序是否倒序

void InitLcd()
{
	//可以根据不同的参数进行初始化液晶类型
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
    //遥信参数
	YXLoadPara(&g_YxPara);
	YXInit(&g_YxPara,4);
}

void InitYK()
{
	BYTE bBuf[10];
	ReadItemEx(BN1, PN0, 0x2022, bBuf);  //继电器输出方式,0电平,1脉冲
	g_YkPara.wMode = bBuf[0];   //遥控输出方式:YK_MODE_LEVEL | YK_MODE_PULSE
	
	WORD wPulseWide = 0;
	if(ReadItemEx(BN10, PN0, 0xa1be, bBuf) > 0)
	{
		wPulseWide = BcdToDWORD(bBuf, 2);
	}
	else
	{
		wPulseWide = 3000;
	}
	g_YkPara.wPulseWidth = wPulseWide;			//脉冲宽度,单位毫秒
	g_YkPara.wSafeTime = 0;	//10上电保电时间,单位分钟,0表示不保电
	g_YkPara.dwFastDist = 0;  //快跳间隔,单位分钟,在此时间内,脉冲每分钟跳一次
							   //超过此时间,脉冲每dwSlowInterv分钟跳一次
							   //如果设置为0,则都按照每分钟跳一次
	g_YkPara.dwSlowInterv = 1; //慢跳间隔,单位分钟
		
	g_YkPara.dwValidTurn = 4; //有效轮次 YK由单片机控制，这里其实都可以去掉了
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
	BYTE bYMFlag = g_PulseManager.GetYMFlag();	//脉冲占用标志
#else
	BYTE bYMFlag = 0;
#endif
	const WORD wOI = OI_YX;
	
	bYMFlag = ~bYMFlag&0x0f;		//按位取反	
 
	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, ATTR4, bBuf, NULL, NULL);	//取开关量接入和属性参数
	if (iLen>0 && bBuf[2]==DT_BIT_STR && bBuf[5]==DT_BIT_STR)
	{
	    DTRACE(DB_CRITICAL, ("YXLoadPara : bBuf[4]=%x, bBuf[7] =%x.\n", bBuf[4], bBuf[7]));

		#ifdef BIT_REVERSE
		bBuf[4] = BitReverse(bBuf[4]);
		bBuf[7] = BitReverse(bBuf[7]);
		DTRACE(DB_CRITICAL, ("YXLoadPara : after bit reverse bBuf[4]=%x, bBuf[7] =%x.\n", bBuf[4], bBuf[7]));
		#endif

		iLen = ReadItemEx(BN1, PN0, 0x2040, &bTmp);	//遥信脉冲是否共用 0：不公用 1：共用 
		if (iLen < 0)
			bTmp = 0;

	    if (bTmp != 0)	//遥信脉冲共用
			pYxPara->wYxFlag = (bYMFlag & (bBuf[4]&0x000f)) & 0x000f;
		else	//不共用
			pYxPara->wYxFlag = bBuf[4] & 0x000f;

		pYxPara->wYxFlag |= 0x10; //门禁点固定有效

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
		pYxPara->wYxFlag |= 0x10; //门禁点固定有效
		pYxPara->wYxPolar = 0x000f;		//默认a型触点
	}

	SetYxInitFlag(false);	//开关量标志初始化，每次修改参数都重新初始化一次
}

void TransferLidStatus()
{
	//根据需要存储结果
	//调用GetLidStatus()，返回1尾盖被打开，0关闭
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


//铁电日志保存表  各平台按需要修改
/*WORD g_wLogFileAddr[512] = {0x00, 		256, 	 	//0交采电能256,最大30*5+1
							512, 		512+256,  	//1交采需量256,最大10*20+1
							1024, 	1024+32, 	//2统计32
							1088, 	1088+32,	 	//3脉冲1电能32
							1088+32*2,	1088+32*3,	//4脉冲2电能32
							1088+32*4,	1088+32*5,	//5脉冲3电能32
							1088+32*6,	1088+32*7,	//6脉冲4电能32
							1344,		1344+40*1,	//7脉冲1需量40
							1344+40*2,	1344+40*3,	//8脉冲2需量40
							1344+40*4,	1344+40*5,	//9脉冲3需量40
							1344+40*6,	1344+40*7,	//10脉冲4需量40
							1664,		1664+172};	//11交采电能未计量的小数点，一共42个量，每个量4字节，留一个余量
*/
//铁电日志保存表  各平台按需要修改
WORD g_wLogFileAddr[512] = {0x00,		256,		//0交采电能256,最大39*5+1
							512,		512+380,	//1交采需量256,最大34*11+1
							1272,	1272+32,		//2统计32-5
							1336,	1336+32,		//3脉冲1电能32
							1400+32*2,	1400+32*3,	//4脉冲2电能32
							1464,		1464+40*1,	//5脉冲1需量40
							1608+40*2,	1608+40*3,	//6脉冲2需量40
							1688,		1688+174};	//7 174交采电能未计量的小数点，一共42个量，每个量4字节，留一个余量

