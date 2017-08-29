/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProHook.cpp
 * 摘    要：本文件主要用来定义通信接口库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年3月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *			 $本文件定义的函数,形式一致,但是在做不同版本的产品时,函数可能需要修改
 *********************************************************************************************************/
#include "stdafx.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "ProAPI.h"
#include "Modem.h"
#include "Trace.h"

//描述:累计流量的接口函数
//备注:可以转向调用统计类的相关函数,
// 	   最好只是先暂时更新统计类的成员变量,否则每次接收发送都写系统库,会影响通信效率
void AddFlux(DWORD dwLen)
{
//	if (!IsDownSoft())
		g_StatMgr.AddFlux(dwLen);
}

//描述:累计流量的接口函数
bool IsFluxOver()	
{
    if (IsDownSoft())
    	return false;
	DWORD dwMonFlux = 0;
    ReadItemEx(BN0, PN0, 0x1501, (BYTE*)&dwMonFlux);

	DWORD dwFluxLimit = 0;
	ReadItemEx(BN0, PN0, 0x024f, (BYTE*)&dwFluxLimit); //F36：终端上行通信流量门限设置,月通信流量门限为0，表示系统不需要终端进行流量控制

	if (dwFluxLimit>0 && dwMonFlux>dwFluxLimit)
		return true;
	else
		return false;
}

//描述:回调函数,用于生成告警记录等用途
//备注:只需在第一次超流量的时候生成告警记录,这个由本函数或者它的调用函数来判断
void GprsOnFluxOver()	
{

	BYTE bAlrBuf[16];
    
	
	
	TTime now;
	memset(bAlrBuf, 0, sizeof(bAlrBuf));
	GetCurTime(&now);

	ReadItemEx(BN0, PN0, 0x1501, &bAlrBuf[0]); //当月已发生的通信流量
	ReadItemEx(BN0, PN0, 0x024f, &bAlrBuf[4]); //月通信流量门限,F36：终端上行通信流量门限设置

//	SaveAlrData(ERC_FLUXOVER, now, bAlrBuf);
	DTRACE(DB_METER_EXC, ("GprsOnFluxOver: ########## \n"));
}


//描述:GPRS是否处于在线时段
bool GprsIsInPeriod()
{
	BYTE bBuf[16];
	if (ReadItemEx(BN0, PN0, 0x008f, bBuf) > 0) //C1F8
	{
        //时段在线模式允许在线时段标志：D0~D23按位顺序对应表示0~23点
		//置"1"表示允许在线时段，置"0"表示禁止在线时段，当相邻时段的设定值相同时，合并为一个长时段。

		TTime now;
		GetCurTime(&now);
		if (bBuf[5+now.nHour/8] & (1<<(now.nHour%8)))
			return true;
	}

	return false;
}

//描述:GPRS通道的告警和主动上送数据是否都送完,遇到需要掉线的时候,等待告警和主动上送数据
//	   全部送完再掉线.如果不用等待告警和主动数据全部送完就可以掉线,直接返回true
//参数:@dwStartClick 流量超标的起始时标,用来控制在多少分钟内没送完也必须掉线
//返回:如果告警和主动数据全部送完则返回true,否则返回false
bool GprsIsTxComplete(DWORD dwStartClick) 
{
	return true;
}

void ProThrdHook(CProtoIf* pIf, CProto* pProto)
{
	static BYTE bDispCnt = 0;
	
	if (pIf->GetIfType()==IF_GPRS || pIf->GetIfType()==IF_SOCKET)
	{
		WORD wState=pIf->GetState();
		if (wState!=IF_STATE_TRANS && wState!=IF_STATE_LOGIN)
		{
			if (bDispCnt != 1)
			{
				char szTmp[21];
				memset(szTmp, 0, sizeof(szTmp));
#ifdef ENGLISH_DISP
				strcpy(szTmp, "OffL");
#else
				strcpy(szTmp, "掉线");
#endif
				WriteItemEx(BN2, PN0, 0x2033, (BYTE *)szTmp);	
				bDispCnt = 1;
			}
		}
		else
		{
			char szTmp[21];
			memset(szTmp, 0, sizeof(szTmp));
			if ((GetClick()-pProto->GetRcvClick()) > 12)
			{
				if (bDispCnt != 2)
				{
#ifdef ENGLISH_DISP
					strcpy(szTmp, "Idle");
#else
					strcpy(szTmp, "空闲");
#endif
					WriteItemEx(BN2, PN0, 0x2033, (BYTE *)szTmp);
					bDispCnt = 2;
				}
			}
			else
			{
				if (bDispCnt != 3)
				{
#ifdef ENGLISH_DISP
					strcpy(szTmp, "Comu");
#else
					strcpy(szTmp, "通讯");
#endif
					WriteItemEx(BN2, PN0, 0x2033, (BYTE *)szTmp);
					bDispCnt = 3;
				}
			}
		}
	}
}



//描述：把模块版本、CCID等信息更新到系统库，给MODEM库的回调接口，在更新完相关信息后调用
void UpdModemInfo(TModemInfo* pModemInfo)
{
	DWORD dwOAD;
	BYTE bBuf[128] = {0};
	BYTE bTmpBuf[128] = {0};
	BYTE *p = bBuf;
	BYTE bChannel = 0;
	int iLen;

	if (pModemInfo != NULL)	//先判断是否为空指针
	{
		ReadItemEx(BANK17, PN0, 0x6010, &bChannel);
		dwOAD = 0x45000500 + bChannel*0x00010000;

		*p++ = DT_STRUCT;
		*p++ = 0x06;
		*p++ = DT_VIS_STR;
		*p++ = 0x04;
		memcpy(p, pModemInfo->bManuftr, 4);
		p += 4;
		*p++ = DT_VIS_STR;
		*p++ = 0x04;
		memcpy(p, pModemInfo->bSoftVer, 4);
		p += 4;
		*p++ = DT_VIS_STR;
		*p++ = 0x06;
		memcpy(p, pModemInfo->bSoftDate, 6);
		p += 6;
		*p++ = DT_VIS_STR;
		*p++ = 0x04;
		memcpy(p, pModemInfo->bHardVer, 4);
		p += 4;
		*p++ = DT_VIS_STR;
		*p++ = 0x06;
		memcpy(p, pModemInfo->bHardDate, 6);
		p += 6;
		*p++ = DT_VIS_STR;
		*p++ = 0x08;
		memset(p, '0', 8);
		p += 8;

		iLen = ReadItemEx(BN0, bChannel, 0x4503, bTmpBuf);
		if (iLen>0 && memcmp(bTmpBuf, bBuf, iLen)!=0)
			WriteItemEx(BN0, bChannel, 0x4503, bBuf);

		TraceBuf(DB_FAPROTO, "UpdModemInfo:", bBuf, p-bBuf);

		memset(bBuf, 0, sizeof(bBuf));
		memset(bTmpBuf, 0, sizeof(bTmpBuf));
		p = bBuf;
		*p++ = DT_VIS_STR;
		*p++ = 20;
		memcpy(p, pModemInfo->bCCID, 20);

		iLen = ReadItemEx(BN0, bChannel, 0x4505, bTmpBuf);
		if (iLen>0 && memcmp(bTmpBuf, bBuf, iLen)!=0)
			WriteItemEx(BN0, bChannel, 0x4505, bBuf);
	}
}

void UpdSIMNum(TModemInfo* pModemInfo)
{
	BYTE bChannel = 0, bBuf[20] = {0};
	
	if (pModemInfo == NULL)	//先判断是否为空指针
		return;

	ReadItemEx(BANK17, PN0, 0x6010, &bChannel);

	ReadItemEx(BN0, bChannel, 0x4508, bBuf);
	if (memcmp(&bBuf[2], pModemInfo->bCNUM, 16) == 0)	//相同不刷新
		return;

	memset(bBuf, 0, sizeof(bBuf));
	bBuf[0] = DT_VIS_STR;
	bBuf[1] = 16;
	memcpy(&bBuf[2], pModemInfo->bCNUM, 16);
	WriteItemEx(BN0, bChannel, 0x4508, bBuf);
}

void UpdSIMCIMI(BYTE* pbBuf)
{
	BYTE bChannel = 0, bBuf[20] = {0};
	if (pbBuf == NULL)	//先判断是否为空指针
		return;
	
	ReadItemEx(BANK17, PN0, 0x6010, &bChannel);

	ReadItemEx(BN0, bChannel, 0x4506, bBuf);
	if (memcmp(&bBuf[2], pbBuf, 15) == 0)	//相同不刷新
		return;

	memset(bBuf, 0, sizeof(bBuf));
	bBuf[0] = DT_VIS_STR;
	bBuf[1] = 15;
	memcpy(&bBuf[2], pbBuf, 15);
	WriteItemEx(BN0, bChannel, 0x4506, bBuf);
}

void UpdSysInfo(BYTE* pbBuf)
{
	//如不需要网络制式，此函数可直接return

	//Modem库获取运营商及网络制式信息，调此函数写入
	//pbBuf[0]--bSysMode网络制式，0:No service  2:2G(包括GSM,EDGE,CDMA)  3:3G(包括 WCDMA,TD-SCDMA,EVDO)  4:4G(包括 FDD-LTE,TDD-LTE) 
	//pbBuf[1]--bMnc网络运营商代码，00:注册失败  01:中国移动  02:中国联通  03:中国电信 
	if(pbBuf == NULL)
		return;
	WriteItemEx(BN2, PN0, 0x2108, pbBuf);
	//TraceBuf(DB_FAFRM, "UpdSysInfo: SysMode and Mnc is -> ", pbBuf, 2);

}

void GetSysInfo(BYTE* pbBuf)
{
	//如不需要网络制式，此函数可直接return

	if(pbBuf == NULL)
		return;
	ReadItemEx(BN2, PN0, 0x2108, pbBuf);	//此处ID同UpdSysInfo中的ID
	//TraceBuf(DB_FAFRM, "UpdSysInfo: SysMode and Mnc is -> ", pbBuf, 2);
}

BYTE GetNetStandard(void)
{
	//如不需要锁定网络制式，此函数可直接return

	//需要锁定制式时使用,供Modem库调用
	BYTE bNetType = 0x01;	// 02: 2G; 03: 3G; 04: 4G; 其它为自适应,不锁定
	//ReadItemEx(BN10, PN0, 0xxxxx, &bNetType);	//显示界面选取要锁定的制式写入此ID，此ID请自行扩展
	return bNetType;
}
