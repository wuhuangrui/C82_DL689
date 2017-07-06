/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FapPara.cpp
 * 摘    要：本文件主要用来把各协议不同的参数装载到相同的参数结构中去,
 *			 如TSocketPara,TGprsPara等,使共用的通信代码不用直接面对各种
 *			 协议的差异
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：
 *********************************************************************************************************/
#include "stdafx.h"
#include "CommIf.h"
#include "GprsIf.h"
#include "ProtoIf.h"
#include "SocketIf.h"
#include "R230mIf.h"
#include "FaAPI.h"
#include "FaProto.h"
#include "ProPara.h"
#include "DbOIAPI.h"

#define FAP_TYPE_OFFSET  		0
#define FAP_IP_OFFSET  			0
#define FAP_DNS_OFFSET  		33
#define FAP_FRONT_PORT_OFFSET  	4
#define FAP_CONCENT_PORT_OFFSET	39
#define FAP_APN_OFFSET  		24
#define FAP_APN_USER_OFFSET  	0
#define FAP_APN_PSW_OFFSET  	16
#define FAP_NETMAST_OFFSET  	89
#define FAP_GATEWAY_OFFSET  	93

bool g_fUpdateFirmware = false;

//描述：获取以太网的工作模式 
BYTE GetEthWorkMode()
{
	BYTE bWorkMode[3] ={GPRS_MIX_MODE, GPRS_CLI_MODE, GPRS_SER_MODE}; 
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 0, &wLen, &bType);	//工作模式
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_ENUM)
			{
				return bWorkMode[*pbPtr++];
			}
		}
	}

	return GPRS_CLI_MODE;
}

//描述：获取以太网的链接模式
BYTE GetEthLnkType()
{
	BYTE bLnk[2] ={LK_TCP, LK_UDP}; 
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 1, &wLen, &bType);	//连接方式
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_ENUM)
			{
				return bLnk[*pbPtr++];
			}
		}
	}

	return LK_TCP;
}

//描述：获取以太网的连接应用方式
BYTE GetEthLnkAppType()
{
	BYTE bAppLnk[2] ={MAST_SUB_CONN_TYPE, MUL_CONN_TYPE}; 
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 2, &wLen, &bType);	//应用链接方式
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_ENUM)
			{
				return bAppLnk[*pbPtr++];
			}
		}
	}

	return MAST_SUB_CONN_TYPE;
}

//描述：获取以太网的侦听端口
WORD GetEthListenPort()
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	WORD wPort;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bNum;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 3, &wLen, &bType);	//侦听端口列表
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_ARRAY)
			{
				bNum = *pbPtr++;
				for (BYTE i=0; i<bNum; i++)
				{
					if (*pbPtr++ == DT_LONG_U)
					{
						wPort = OoOiToWord(pbPtr);
						pbPtr += 2;
						if (wPort != 0)
							return wPort;
					}
				}
			}
		}
	}

	return 0;
}

//描述：获取代理服务器地址
BYTE GetEthProxySerAddr(char *pAddr)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bNum;
	BYTE bLen;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 4, &wLen, &bType);	//侦听端口列表
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_OCT_STR)
			{
				bLen = *pbPtr++;
				if (bLen > 4)
					bLen = 4;
				memcpy(pAddr, pbPtr, bLen);
			}
		}
	}

	return bLen;
}

//描述：获取代理服务器端口
WORD GetEthProxySerPort()
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	WORD wPort;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bNum;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 5, &wLen, &bType);	//代理服务器端口
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_LONG_U)
			{
				wPort = OoOiToWord(pbPtr);
				return wPort;
			}
		}
	}

	return 0;
}

//描述：获取以太网的超时时间
BYTE GetEthTimeOut()
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bTimeOut = 0;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 6, &wLen, &bType);	//超时时间
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_BIT_STR)
			{
				if (*pbPtr++ == 8)
				{
					bTimeOut = (*pbPtr++>>2)&0x3f;
					if (bTimeOut == 0)
						bTimeOut = 30;
				}
			}
		}
	}

	return bTimeOut;
}

//描述：获取以太网的重试次数
BYTE GetEthTryCnt()
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bTryCnt = 0;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 6, &wLen, &bType);	//重试次数
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_UNSIGN)
				bTryCnt = (*pbPtr++)&0x03;
		}
	}

	if (bTryCnt == 0)
		bTryCnt = 1;

	return bTryCnt;
}

//描述：获取以太网心跳周期(unit:s)
DWORD GetEthBeat()
{
	DWORD dwOAD;
	WORD wBeat = 0;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 7, &wLen, &bType);	//心跳
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_LONG_U)
			{
				wBeat = OoOiToWord(pbPtr);	
			}
		}
	}
	if (wBeat == 0)
		wBeat = 300;

	return wBeat;
}

//描述：获取以太网配置方式 DHCP（0），静态（1），PPPoE（2）
BYTE GetEthConfigType()
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bModule, bEthType, bLen, bNum, bType;
	BYTE *pbPtr, *pbFmt;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100400 + bModule*0x00010000;	
	bEthType = 1;
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		//IP配置方式
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 0, &wLen, &bType);
		if (pbPtr != NULL)
		{
			if (*pbPtr++ == DT_ENUM)	
				bEthType = *pbPtr++;
		}
	}

	return bEthType;
}

//描述：获取以太网本地IP地址
BYTE GetEthLocalIp(BYTE *pAddr)
{	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bModule, bLen, bType;
	BYTE *pbPtr, *pbFmt;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100400 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		//IP地址
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 1, &wLen, &bType);
		if (pbPtr != NULL)
		{
			if (*pbPtr++ == DT_OCT_STR)	
			{
				bLen = *pbPtr++;
				if (bLen > 4)
					bLen = 4;
				memcpy(pAddr, pbPtr, bLen);
			}
		}
	}

	return bLen;
}

//描述：获取以太网子网掩码
BYTE GetEthNetMask(BYTE *pNetMsk)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bModule, bEthType, bLen, bNum, bType;
	BYTE *pbPtr, *pbFmt;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块

	dwOAD = 0x45100400 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 2, &wLen, &bType);
		if (pbPtr != NULL)
		{
			if (*pbPtr++ == DT_OCT_STR)	
			{
				bLen = *pbPtr++;
				if (bLen > 4)
					bLen = 4;
				memcpy(pNetMsk, pbPtr, bLen);
			}
		}
	}

	return bLen;
}

BYTE GetEthGataway(BYTE *pbAddr)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bModule, bEthType, bLen, bNum, bType;
	BYTE *pbPtr, *pbFmt;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块

	dwOAD = 0x45100400 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 3, &wLen, &bType);
		if (pbPtr != NULL)
		{
			if (*pbPtr++ == DT_OCT_STR)	
			{
				bLen = *pbPtr++;
				if (bLen > 4)
					bLen = 4;
				memcpy(pbAddr, pbPtr, bLen);
			}
		}
	}

	return bLen;
}

//描述：获取以太网PPPoE用户名字
BYTE GetEthPPPoEUserName(char *pszUserName)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bModule, bEthType, bLen, bNum, bType;
	BYTE *pbPtr, *pbFmt;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100400 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 4, &wLen, &bType);
		if (pbPtr != NULL)
		{
			if (*pbPtr++ == DT_VIS_STR)	
			{
				bLen = *pbPtr++;
				memcpy(pszUserName, pbPtr, bLen);
			}
		}
	}

	return bLen;
}

//描述：获取以太网PPPoE用户密码
BYTE GetEthPPPoEUserPwd(char *pszUserPwd)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bModule, bEthType, bLen, bNum, bType;
	BYTE *pbPtr, *pbFmt;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	dwOAD = 0x45100400 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 5, &wLen, &bType);
		if (pbPtr != NULL)
		{
			if (*pbPtr++ == DT_VIS_STR)	
			{
				bLen = *pbPtr++;
				memcpy(pszUserPwd, pbPtr, bLen);
			}
		}
	}

	return bLen;
}

//描述：获取以太网MAC地址
BYTE GetEthMacAddr(BYTE *pbMac)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bchn, bEthType, bLen, bNum, bType;
	BYTE *pbPtr, *pbFmt;

	bchn = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bchn);	//获取以太网相应的模块

	dwOAD = 0x45100500 + bchn*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = bBuf;
		if (*pbPtr++ == DT_OCT_STR)	//MAC
		{
			bLen = *pbPtr++;
			memcpy(pbMac, pbPtr, bLen);
		}
	}

	return bLen;
}

BYTE GetGprsWorkMode()
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bWorkMode = 0;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 0, &wLen, &bType);	//工作模式
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_ENUM)
				bWorkMode = *pbPtr++;
		}
	}

	return bWorkMode;
}

BYTE GetGprsInlineType()
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bInlineType = 0;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 1, &wLen, &bType);	//在线方式
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_ENUM)
				bInlineType = *pbPtr++;
		}
	}

	return bInlineType;
}

BYTE GetGprsLnkType()
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bLnkType = 0;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 2, &wLen, &bType);	//连接类型
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_ENUM)
				bLnkType = *pbPtr++;
		}
	}

	return bLnkType;
}

BYTE GetGprsAppLnkType()
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bAppLnkType = 0;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 3, &wLen, &bType);	//连接应用方式
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_ENUM)
				bAppLnkType = *pbPtr++;
		}
	}

	return bAppLnkType;
}

WORD GetGprsListenPort()
{
	DWORD dwOAD, wListenPort=0;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bNum;
	
	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 4, &wLen, &bType);	//侦听端口列表
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_ARRAY)
			{
				bNum = *pbPtr++;
				for (BYTE i=0; i<bNum; i++)
				{
					if (*pbPtr++ == DT_LONG_U)
					{
						wListenPort = ByteToWord(pbPtr);
						if (wListenPort != 0)
							break;
					}
					pbPtr += 2;
				}
			}
		}
	}

	return wListenPort;
}

BYTE GetGprsAPN(char *pszAPN)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bLen = 0;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 5, &wLen, &bType);	//APN
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_VIS_STR)
			{
				bLen = *pbPtr++;
				memcpy(pszAPN, pbPtr, bLen);
			}
		}
	}

	return bLen;
}

BYTE GetGprsUserName(char *pszUserName)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bLen = 0;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 6, &wLen, &bType);	//用户名
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_VIS_STR)
			{
				bLen = *pbPtr++;
				memcpy(pszUserName, pbPtr, bLen);
			}
		}
	}

	return bLen;
}

BYTE GetGprsUserPwd(char *pszUserPwd)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bLen = 0;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 7, &wLen, &bType);	//密码
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_VIS_STR)
			{
				bLen = *pbPtr++;
				memcpy(pszUserPwd, pbPtr, bLen);
			}
		}
	}

	return bLen;
}

BYTE GetGprsProxyAddr(BYTE *pbAddr)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bLen = 0;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 8, &wLen, &bType);	//代理服务器地址
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_OCT_STR)
			{
				bLen = *pbPtr++;
				if (bLen > 4)
					bLen = 4;
				memcpy(pbAddr, pbPtr, bLen);
			}
		}
	}

	return bLen;
}

WORD GetGprsProxyPort()
{
	DWORD dwOAD, dwProxyPort = 0;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 9, &wLen, &bType);	//代理端口 
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_LONG_U)
				dwProxyPort = ByteToWord(pbPtr);
		}
	}

	return dwProxyPort;
}

BYTE GetGprsTimeOut()
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bTimeOut = 0;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 10, &wLen, &bType);	//超时时间及重发次数
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_UNSIGN)
				bTimeOut = (*pbPtr++>>2)&0x3f;
		}
	}
	if (bTimeOut == 0)
		bTimeOut = 30;

	return bTimeOut;
}

BYTE GetGprsTryCnt()
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bTryCnt = 0;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 10, &wLen, &bType);	//超时时间及重发次数
		if (pbPtr != NULL)
		{
			if (*pbPtr++ == DT_UNSIGN)
				bTryCnt = *pbPtr&0x03;
		}
	}
	if (bTryCnt == 0)
		bTryCnt = 1;


	return bTryCnt;
}

DWORD GetGprsBeat()
{
	DWORD dwOAD;
	WORD wBeat = 0;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000200 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 11, &wLen, &bType);	//心跳
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_LONG_U)
			{
				wBeat = OoOiToWord(pbPtr);
			}
		}
	}
	if (wBeat == 0)
		wBeat = 300; //默认5分钟

	return wBeat;
}

BYTE GetGprsSmsCenterNo(BYTE *pbSmsNo)
{
	DWORD dwOAD;
	WORD wBeat = 0;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bLen;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000300 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 0, &wLen, &bType);	//短信中心号码
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_VIS_STR)
			{
				bLen = *pbPtr++;
				memcpy(pbSmsNo, pbPtr, bLen);
			}
		}
	}

	return bLen;
}

BYTE GetGprsSmsMastNo(BYTE *pMastNo)
{
	DWORD dwOAD;
	WORD wBeat = 0;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bNum;
	BYTE bLen;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000300 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 1, &wLen, &bType);	//主站号码
		if (*pbPtr++ == DT_ARRAY)
		{
			bNum = *pbPtr++;
			for (BYTE i=0; i<bNum; i++)
			{
				if (*pbPtr++ == DT_VIS_STR)
				{
					bLen = *pbPtr++;
					if (!IsAllAByte(pbPtr, 0, bLen))
					{
						memcpy(pMastNo, pbPtr, bLen);
						break;
					}
				}
			}
		}
	}

	return bLen;
}

BYTE GetGprsSmsDstNo(BYTE *pbDstNo)
{
	DWORD dwOAD;
	WORD wBeat = 0;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;
	BYTE bNum;
	BYTE bLen;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	dwOAD = 0x45000300 + bModule*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 1, &wLen, &bType);	//短信通知目的号码
		if (*pbPtr++ == DT_ARRAY)
		{
			bNum = *pbPtr++;
			for (BYTE i=0; i<bNum; i++)
			{
				if (*pbPtr++ == DT_VIS_STR)
				{
					bLen = *pbPtr++;
					if (!IsAllAByte(pbPtr, 0, bLen))
					{
						memcpy(pbDstNo, pbPtr, bLen);
						break;
					}
				}
			}
		}
	}

	return bLen;
}


//描述：装载以太网通信配置参数
void LoadEthCommPara(TSocketPara* pPara)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	DTRACE(DB_FAPROTO, ("Get ethernet module=%d.\n", bModule));

	BYTE bWorkMode;
	bWorkMode = GetEthWorkMode();
	if (bWorkMode == GPRS_CLI_MODE)	//客户机模式
		pPara->fSvr = false;
	else	//混合模式 || 服务器模式
		pPara->fSvr = true;
	BYTE bLnkType;
	bLnkType = GetEthLnkType();
	if (bLnkType == LK_UDP)
		pPara->fUdp = true;
	else
		pPara->fUdp = false;
	//pPara->IfPara.wReSendNum = GetEthTryCnt();
	pPara->wBeatTestTimes = GetEthTryCnt();
	pPara->dwBeatTimeouts = GetEthTimeOut();
	pPara->dwBeatSeconds = GetEthBeat();

	//pPara->IfPara.dwDormanInterv = 300;//休眠时间间隔, 单位秒, ,0表示禁止休眠模式
	pPara->IfPara.dwDormanInterv = 10;//休眠时间间隔, 单位秒, ,0表示禁止休眠模式
	pPara->IfPara.wReTryNum = 2;
	pPara->fEnableFluxStat = false; //是否允许流量控制,只有本socket用的是GPRS通道时才支持
	pPara->wDisConnectByPeerNum = 3;	//被对方断开连接，切换到休眠状态的次数

	//装入TIfPara参数
	pPara->IfPara.fNeedLogin = true;	//是否需要登录
	pPara->IfPara.wMaxFrmBytes = SOCK_MAX_BYTES; //接口的一帧最大发送字节数,不同协议可能规定不一样
	pPara->IfPara.dwRstInterv = 10; //接口的复位间隔,单位秒
	pPara->IfPara.wConnectNum = 2;//登陆重试次数
	pPara->IfPara.wRstNum = 2;
	pPara->IfPara.wLoginRstNum = 2; 	//登录失败断开连接的次数
	pPara->IfPara.wLoginNum = 2; 		//登录失败连续尝试的次数
	pPara->IfPara.dwLoginInterv = 10; 	//登录间隔,单位秒
	pPara->IfPara.dwConnectInterv= 20; 	////接口的连接间隔,单位秒
	pPara->bSvrDisconMode = SVR_DISCON_EXIT;
}

//描述：装载GPRS通信配置参数
void LoadGPRSCommPara(TSocketPara* pPara)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bchn;

	bchn = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bchn);	//获取GPRS相应的模块
	DTRACE(DB_FAPROTO, ("Get gprs channel=%d.\n", bchn));

	BYTE bWorkMode;
	bWorkMode = GetGprsWorkMode();
	if (bWorkMode == GPRS_CLI_MODE)	//客户机模式
		pPara->fSvr = false;
	else	//混合模式 || 服务器模式
		pPara->fSvr = true;
	BYTE bLnkType;
	bLnkType = GetGprsLnkType();
	if (bLnkType == LK_UDP)
		pPara->fUdp = true;
	else
		pPara->fUdp = false;
	//pPara->IfPara.wReSendNum = GetEthTryCnt();
	pPara->wBeatTestTimes = GetGprsTryCnt();
	pPara->dwBeatTimeouts = GetGprsTimeOut();
	pPara->dwBeatSeconds = GetGprsBeat();

	DTRACE(DB_FAPROTO, ("LoadGPRSCommPara: pPara->wBeatTestTimes=%d, pPara->dwBeatTimeouts=%d, pPara->dwBeatSeconds=%d.\n", \
		pPara->wBeatTestTimes, pPara->dwBeatTimeouts, pPara->dwBeatSeconds));

	//pPara->IfPara.dwDormanInterv = 300;//休眠时间间隔, 单位秒, ,0表示禁止休眠模式
	pPara->IfPara.dwDormanInterv = 10;//休眠时间间隔, 单位秒, ,0表示禁止休眠模式
	pPara->IfPara.wReTryNum = 2;
	pPara->fEnableFluxStat = true; //是否允许流量控制,只有本socket用的是GPRS通道时才支持
	pPara->wDisConnectByPeerNum = 3;	//被对方断开连接，切换到休眠状态的次数

	//装入TIfPara参数
	pPara->IfPara.fNeedLogin = true;	//是否需要登录
	pPara->IfPara.wMaxFrmBytes = SOCK_MAX_BYTES; //接口的一帧最大发送字节数,不同协议可能规定不一样
	pPara->IfPara.dwRstInterv = 10; //接口的复位间隔,单位秒
	pPara->IfPara.wConnectNum = 2;//登陆重试次数
	pPara->IfPara.wRstNum = 2;
	pPara->IfPara.wLoginRstNum = 2; 	//登录失败断开连接的次数
	pPara->IfPara.wLoginNum = 2; 		//登录失败连续尝试的次数
	pPara->IfPara.dwLoginInterv = 10; 	//登录间隔,单位秒
	pPara->IfPara.dwConnectInterv= 20; 	////接口的连接间隔,单位秒
	pPara->bSvrDisconMode = SVR_DISCON_EXIT;
}


//描述:装载通信接口的默认参数
void LoadIfDefPara(TIfPara* pIfPara)
{
	//消息宏定义,避免编译过程中库用到的宏定义跟应用程序不一致
	pIfPara->wInfoActive = INFO_ACTIVE;
	pIfPara->wInfoAppRst = INFO_APP_RST;
}

//描述：终端运行通讯配置
//参数格式：02 12 
//			0	22 **	//工作模式enum{混合模式（0），客户机模式（1），服务器模式（2）}，
//			1	22 **	//在线方式enum{永久在线（0），被动激活（1）}，
//			2	22 **	//连接方式enum{TCP（0），UDP（1）}，
//			3	22 **	//连接应用方式enum{主备模式（0），多连接模式（1）}
//			4	01 **	//侦听端口列表  array long-unsigned
//					18 ** **
//					........
//					18 ** **
//			5	10 num ** ** ** ** **.......//APN  visible-string
//			6	10 num ** ** ** ** **.......//用户名         visible-string
//			7	10 num ** ** ** ** **.......//密码           visible-string
//			8	09 num ** ** ** **	//代理服务器地址 octet-string
//			9	18 ** **	//代理端口            long-unsigned
//			10	4 **		//超时时间及重发次数  bit-string(SIZE(8))
//			11	18 ** **	//心跳周期(秒)  long-unsigned
void LoadSocketUnrstPara(TSocketPara* pPara, BYTE bSockType)
{
	DWORD dwSvrModeBeatTimes;
	BYTE bBuf[10] = {0};

	if (bSockType == SOCK_TYPE_ETH)
		LoadEthCommPara(pPara);
	else
		LoadGPRSCommPara(pPara);

	ReadItemEx(BN1, PN0, 0x2030, bBuf);		//服务器模式下心跳次数 NN.NN
	dwSvrModeBeatTimes = BcdToDWORD(bBuf, 2);
	g_dwSvrModeBeatTestTimes = dwSvrModeBeatTimes * (pPara->dwBeatSeconds) / 100;

}

//描述:装载以太网主站通信参数
void LoadEthMasterCommPara(TSocketPara* pPara)
{
#ifndef SYS_WIN
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE *pbPtr, *pbFmt;
	BYTE bType, bLen, bMastNum;

	for (BYTE i=0; i<MAX_ETH_COM_NUM; i++)
	{
		dwOAD = 0x45100300 + i*0x00010000;	
		memset(bBuf, 0, sizeof(bBuf));
		if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
		{
			bMastNum = bBuf[1];
			for (BYTE j=0; j<bMastNum; j++)
			{
				pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, j, &wLen, &bType);
				if (pbPtr != NULL)
				{
					if (*pbPtr++ != DT_STRUCT)
						continue;
					pbPtr++;
					if (*pbPtr++ == DT_OCT_STR)	//IP地址
					{
						bLen = *pbPtr++;	
						pPara->dwRemoteIP = (pbPtr[0]<<24) + (pbPtr[1]<<16) + (pbPtr[2]<<8) + pbPtr[3];	
						if (pPara->dwRemoteIP == 0)
						{
							DTRACE(DB_FAPROTO, ("Ethernet channel=%d, Ip is invalid.\n", i));
							continue;
						}
						pbPtr += bLen;
						if (*pbPtr++ != DT_LONG_U)
							continue;
						pPara->wRemotePort = OoOiToWord(pbPtr);
						if (pPara->wRemotePort == 0)
						{
							DTRACE(DB_FAPROTO, ("Ethernet channel=%d, Port is invalid.\n", i));
							continue;
						}
						pbPtr += 2;

						WriteItemEx(BANK17, PN0, 0x6012, &i);	//保存以太网相应的通道
						WriteItemEx(BANK17, PN0, 0x6013, &j);	//保存相应通道中的相应IP地址
						DTRACE(DB_FAPROTO, ("Select ethernet channel=%d, Ip sn=%d.\n", i, j));
						return;
					}
				}
			}
		}
	}
#endif
}

//描述:装载GPRS主站通信参数
void LoadGprsMasterCommPara(TSocketPara* pPara)
{
#ifndef SYS_WIN
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE *pbPtr, *pbFmt;
	BYTE bType, bLen, bMastNum;

	for (BYTE i=0; i<MAX_GPRS_COM_NUM; i++)
	{
		dwOAD = 0x45000300 + i*0x00010000;	
		memset(bBuf, 0, sizeof(bBuf));
		if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
		{
			bMastNum = bBuf[1];
			for (BYTE j=0; j<bMastNum; j++)
			{
				pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, j, &wLen, &bType);
				if (pbPtr != NULL)
				{
					if (*pbPtr++ != DT_STRUCT)
						continue;
					pbPtr++;
					if (*pbPtr++ == DT_OCT_STR)	//IP地址
					{
						bLen = *pbPtr++;	
						pPara->dwRemoteIP = (pbPtr[0]<<24) + (pbPtr[1]<<16) + (pbPtr[2]<<8) + pbPtr[3];	
						if (pPara->dwRemoteIP == 0)
						{
							DTRACE(DB_FAPROTO, ("Gprs channel=%d, Ip is invalid.\n", i));
							continue;
						}
						pbPtr += bLen;
						if (*pbPtr++ != DT_LONG_U)
							continue;
						pPara->wRemotePort = OoOiToWord(pbPtr);
						if (pPara->wRemotePort == 0)
						{
							DTRACE(DB_FAPROTO, ("Gprs channel=%d, Port is invalid.\n", i));
							continue;
						}
						pbPtr += 2;

						WriteItemEx(BANK17, PN0, 0x6010, &i);	//保存GPRS相应的通道
						WriteItemEx(BANK17, PN0, 0x6011, &j);	//保存相应通道中的相应IP地址
						DTRACE(DB_FAPROTO, ("Select gprs Channel=%d, Ip sn=%d.\n", i, j));
						return;
					}
				}
			}
		}
	}
#endif
}

//描述:装载socket通信参数
void LoadSocketPara(TSocketPara* pPara, BYTE bSockType)
{
	const char *pSockName[16] = {"Ethernet", "GPRS"};

#ifdef SYS_WIN
	pPara->dwRemoteIP = 0x7f000001;
	pPara->wRemotePort = 9200;
#else
	if (bSockType == SOCK_TYPE_ETH)
		LoadEthMasterCommPara(pPara);
	else
		LoadGprsMasterCommPara(pPara);
#endif

	pPara->dwLocalIP = 0; //INADDR_ANY;
	pPara->wLocalPort = 8000;		
	//pPara->dwBakIP = 0;
	//pPara->wBakPort = 0;
	DTRACE(DB_FAPROTO, ("SocketLoadPara %s:remote ip=%d.%d.%d.%d port=%d, local port=%d.\n",
				 		pSockName[bSockType],
						(pPara->dwRemoteIP>>24)&0xff, (pPara->dwRemoteIP>>16)&0xff, (pPara->dwRemoteIP>>8)&0xff, pPara->dwRemoteIP&0xff, 
				 		pPara->wRemotePort, pPara->wLocalPort));
}    



void LoadSocketLocalPara()             
{	
#ifndef SYS_WIN
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	char szCmd[128];
	const char *pszEthType[3] = {"DHCP", "Satic", "PPPoE"};
	BYTE bBuf[256];
	BYTE bIpAddr[8], bNetMask[8], bGatewayAddr[8], bMac[8];
	char szPPPoEUser[32], szPPPoECode[32];
	BYTE bModule, bEthType, bLen, bNum, bType;
	BYTE *pbPtr, *pbFmt;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//获取以太网相应的模块
	DTRACE(DB_FAPROTO, ("Get ethernet Module=%d.\n", bModule));

	//1. 配置MAC地址
	if (!GetEthMacAddr(bMac))
		memcpy(bMac, "\x11\x22\x33\x44\x55\x66", sizeof("\x11\x22\x33\x44\x55\x66"));

	//2. 配置以太网ip地址、子网掩码、网关、PPPoE
	bEthType = GetEthConfigType();
	GetEthLocalIp(bIpAddr);
	GetEthNetMask(bNetMask);
	GetEthGataway(bGatewayAddr);
	GetEthPPPoEUserName(szPPPoEUser);
	GetEthPPPoEUserPwd(szPPPoECode);

	DTRACE(DB_FAPROTO, ("Eth%d loadSocketPara : Ethernet config---%s.\n", bModule, pszEthType[bEthType]));
	switch (bEthType)
	{
	case ETH_IP_CFG_DHCP:	//DHCP
		break;
	case ETH_IP_CFG_STATIC:	//静态
		memset(szCmd, 0, sizeof(szCmd));
		sprintf(szCmd, "ifconfig eth%d down", bModule);
		DTRACE(DB_FAPROTO, ("Eth%d loadSocketPara : %s.\r\n", bModule, szCmd));
		system(szCmd);
		Sleep(1000);
		memset(szCmd,0,sizeof(szCmd));
		sprintf(szCmd, "ifconfig eth%d hw ether %02x:%02x:%02x:%02x:%02x:%02x", 
			bModule, bMac[0], bMac[1], bMac[2], bMac[3], bMac[4], bMac[5]);
		DTRACE(DB_FAPROTO, ("Eth%d loadSocketPara : %s.\r\n", bModule, szCmd));
		system(szCmd);
		Sleep(500);

		if (!IsAllAByte(bIpAddr, 0, sizeof(bIpAddr)))
		{
			if (!IsAllAByte(bNetMask, 0, sizeof(bNetMask)))
			{
				memset(szCmd, 0, sizeof(szCmd));
				sprintf(szCmd, "ifconfig eth%d %d.%d.%d.%d netmask %d.%d.%d.%d up", bModule,
					bIpAddr[0], bIpAddr[1], bIpAddr[2], bIpAddr[3], 
					bNetMask[0], bNetMask[1], bNetMask[2], bNetMask[3]);			
				DTRACE(DB_FAPROTO, ("Eth%d loadSocketPara : %s.\r\n", bModule, szCmd));
				system(szCmd);
				Sleep(500);

				if (!IsAllAByte(bGatewayAddr, 0x00, sizeof(bGatewayAddr)))
				{
					//sprintf(szCmd, "route add default gateway %d.%d.%d.%d dev eth0", bGatewayAddr[0], bGatewayAddr[1], bGatewayAddr[2], bGatewayAddr[3]);
					sprintf(szCmd, "route add default gateway %d.%d.%d.%d", bGatewayAddr[0], bGatewayAddr[1], bGatewayAddr[2], bGatewayAddr[3]);
					DTRACE(DB_FAPROTO, ("Eth%d loadSocketPara : %s.\r\n", bModule, szCmd));
					system(szCmd);
				}
				else
				{
					DTRACE(DB_FAPROTO, ("LoadSocketPara Eth%d set default getaway failed due to gw = 0.\r\n", bModule));
				}
			}
			else
			{
				DTRACE(DB_FAPROTO, ("LoadSocketPara Eth%d netmask is invalid.\n", bModule));
			}
		}
		else
		{
			DTRACE(DB_FAPROTO, ("LoadSocketPara Eth%d IP addr is invalid.\n", bModule));
		}
		break;
	case ETH_IP_CFG_PPPoE:	//PPPoE
		break;
	}

	return;
#endif
}

bool LoadSocketChangePara(TSocketPara* pPara)
{
	BYTE bSockType;
	//socket模式下更新参数()
	if (GetInfo(INFO_COMM_ETH_RLD) == false)
		return false;

	DTRACE(DB_FAPROTO, ("LoadSocketChangePara: %s, para change\n", pPara->IfPara.pszName));

		
	
	//BYTE bComPort = 0;
	//if (memcmp(pPara->IfPara.pszName, "Socket-2", sizeof("Socket-2")) == 0)
	//{
		//bComPort = 1;
	//}

	SetInfo(INFO_COMM_TERMIP);
	LoadSocketLocalPara();
	LoadSocketPara(pPara, SOCK_TYPE_ETH); //装载socket通信参数	
	LoadSocketUnrstPara(pPara, SOCK_TYPE_ETH);
	return true;
}


void LoadGprsCommPara(TGprsWorkerPara* pWorkerPara)
{
	BYTE bModuleType;
	
	//GPRS串口配置	
	ReadItemEx(BN2, PN0, 0x10d3, &bModuleType);
	if (bModuleType==MODULE_LC6311 || bModuleType==MODULE_LC6311_2G)
		pWorkerPara->CommPara.wPort = COMM_3GMODEM; 
	else
		pWorkerPara->CommPara.wPort = COMM_GPRS; 
	pWorkerPara->CommPara.dwBaudRate = CBR_115200; 
	pWorkerPara->CommPara.bByteSize = 8; 
	pWorkerPara->CommPara.bStopBits = ONESTOPBIT; 
	pWorkerPara->CommPara.bParity = NOPARITY;
	
	BYTE bConnectWait = 20;
//	ReadItemEx(BANK1, PN0, 0x2013, &bConnectWait);  //0x2013 1 GPRS连接等待时间 HEX 秒
	pWorkerPara->wConnectWait = bConnectWait;		//连接等待时间
	
	return;
}

void LoadGprsApnPara(TGprsWorkerPara* pWorkerPara)
{
	BYTE bModule = 0;
	BYTE bBuf[64];
	
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	DTRACE(DB_FAPROTO, ("LoadGprsApnPara: Get gprs module=%d.\n", bModule));
	if (GetGprsAPN(pWorkerPara->ModemPara.szAPN) == 0)
		strcpy(pWorkerPara->ModemPara.szAPN, "CMNET");

	DTRACE(DB_FAPROTO,("LoadGprsApnPara: APN= %s.\n",pWorkerPara->ModemPara.szAPN));

	pWorkerPara->ModemPara.wDormantInterv = 10;	
	if (ReadItemEx(BN10, PN0, 0xa1a5, bBuf) > 0)
	{
		pWorkerPara->ModemPara.wDormantInterv = (WORD)BcdToDWORD(bBuf,2);
		DTRACE(DB_FAPROTO,("LoadGprsApnPara:set Modem dormantInterV %d\n ",pWorkerPara->ModemPara.wDormantInterv));
	}
	return;
}

void LoadGprsPppPara(TGprsWorkerPara* pWorkerPara)
{
	BYTE bModule = 0;

	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//获取GPRS相应的模块
	DTRACE(DB_FAPROTO, ("LoadGprsPppPara: Get gprs module=%d.\n", bModule));
	
	if (GetGprsUserName(pWorkerPara->szPppUser) == 0)
		strcpy(pWorkerPara->szPppUser, "CARD");
	if (GetGprsUserPwd(pWorkerPara->szPppPsw) == 0)
		strcpy(pWorkerPara->szPppPsw, "CARD");

	
	DTRACE(DB_FAPROTO, ("LoadGprsPppPara: ppp user=%s, ppp psw=%s\n",
					    pWorkerPara->szPppUser, pWorkerPara->szPppPsw));
}

void LoadGprsWorkerPara(TGprsWorkerPara* pWorkerPara)
{
	memset(pWorkerPara, 0, sizeof(TGprsWorkerPara));

	//消息宏定义,避免编译过程中库用到的宏定义跟应用程序不一致
	pWorkerPara->wInfoActive = INFO_ACTIVE;

	BYTE bEmbedProtcocol = 1;
	ReadItemEx(BN1, PN0, 0x2032, &bEmbedProtcocol);
	if (bEmbedProtcocol == 0)
		pWorkerPara->fEmbedProtocol = true;
	else
		pWorkerPara->fEmbedProtocol = false;


	//增加获取支持获取基站信息    
	pWorkerPara->ptNetInfo = NULL;
	//使用详细联网诊断，这个参数如果需要实时生效的话可以用参数控制
	//放到LoadGprsWorkerUnrstPara中初始化
	pWorkerPara->fDetailDiagnose = false;
	////是否控制Socket Led,只针对国网标准模块
	pWorkerPara->fEnSocketLed = true;
	pWorkerPara->fEnMux = true;			//是否允许串口复用
	//获取信号强度和发射功率的时间间隔，为0表示不获取，fEnMux(串口复用)置true
	pWorkerPara->wUpdTxPwrInterv = 5*60;		

	LoadGprsCommPara(pWorkerPara);
	LoadGprsApnPara(pWorkerPara);
	LoadGprsPppPara(pWorkerPara);
	LoadSmsPara(&pWorkerPara->ModemPara);
}

bool LoadGprsWorkerUnrstPara(TGprsWorkerPara* pWorkerPara)
{
	if (GetInfo(INFO_WK_PARA) == false)
	 	return false;
	 	
	DTRACE(DB_FAPROTO, ("LoadGprsWorkerUnrstPara: para change\n"));
	 	
	LoadGprsApnPara(pWorkerPara);
	LoadGprsPppPara(pWorkerPara);
	LoadSmsPara(&pWorkerPara->ModemPara);
	
	return true;
}

//描述:装载GPRS工作模式参数
void LoadGprsModePara(TGprsPara* pGprsPara)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bchn;

	bchn = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bchn);	//获取GPRS相应的模块
	DTRACE(DB_FAPROTO, ("LoadGprsModePara: Get gprs channel=%d.\n", bchn));
	dwOAD = 0x45000200 + bchn*0x00010000;	//通信配置参数	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 1, &wLen, &bType);	//在线方式
		if (pbPtr != NULL)
		{
			if (*pbPtr++ == DT_ENUM)
			{
				BYTE bPowerupDropInterv = 5;  //0x4011 1 上电激活的自动掉线时间,BCD,单位分钟,设为0自动取消上电激活
				BYTE bPowerupBeatMinutes = 0;  //0x4012 1 上电激活的心跳间隔,BCD,单位分钟

				if (*pbPtr == 1)	//激活模式
				{
					pGprsPara->bOnlineMode = ONLINE_M_ACTIVE;
					pGprsPara->fEnableAutoSendActive = false; 	//允许主动上报激活
					pGprsPara->SocketPara.IfPara.wReTryNum = bBuf[3];	//重拨次数(被动激活:次)
					pGprsPara->dwActiveDropInterv = bBuf[4];	//自动掉线时间(被动激活:min)

					pGprsPara->dwPowerupDropInterv = bPowerupDropInterv; 	 
					//上电激活的自动掉线时间,单位分钟,设为0自动取消上电激活
					pGprsPara->dwPowerupBeatMinutes = bPowerupBeatMinutes;	 
					//上电激活的心跳间隔

					pGprsPara->fRstOnSms = true;			//是否复位到短信模式，主要针对激活模式和时段在线模式
				}
				else //永久在线
				{
					pGprsPara->bOnlineMode = ONLINE_M_PERSIST;
					pGprsPara->fEnableAutoSendActive = false; 	//允许主动上报激活
					pGprsPara->SocketPara.IfPara.wReTryNum = 2;	//重拨次数(被动激活:次)
					pGprsPara->dwActiveDropInterv = 5;	//自动掉线时间(被动激活:min)

					pGprsPara->dwPowerupDropInterv = bPowerupDropInterv; 	 
					//上电激活的自动掉线时间,单位分钟,设为0自动取消上电激活
					pGprsPara->dwPowerupBeatMinutes = bPowerupBeatMinutes;	 
					//上电激活的心跳间隔

					pGprsPara->fRstOnSms = false;			//是否复位到短信模式，主要针对激活模式和时段在线模式
				}
			}
		}
	}

	return;
}


WORD PhoneToStr(BYTE* pbPhone, WORD wPhoneLen, char* pszStr)
{
	int i , j, Len;
	BYTE buff[64] ,Strbuff[64] ;
	j = 0;
	memset(buff , 0x00 , 64) ;
	memset(Strbuff , 0x00 , 64) ;
	for (i=0; i<wPhoneLen; i++)
	{
		if ((pbPhone[i]&0xf0) == 0xf0)	
			break;
		else
		{
			buff[j++] = (pbPhone[i]>>4)&0x0f;
			
			if ((pbPhone[i]&0x0f) == 0x0f)
			{	
				break;
			}
			else
			{
				buff[j++] = pbPhone[i]&0x0f;
			}
		}
	}


	if (j == 0)
	{
		*pszStr = '\0';
		return 0;
	}
	
	Len = j ;
	j = 0;
	for (i=0; i<Len; i++)
	{
		if (buff[i]<=9) //buff[i]>=0 && 
		{
			Strbuff[j] = buff[i] + '0' ;
			j++;
		}
		else if (buff[i]==0x0a || buff[i]==0x0b)
		{
			if (buff[i] == 0x0a)
				Strbuff[j] = ',';
			else
				Strbuff[j] = '#' ;
			j++;
		}
		else
		{
			j = 0;	//错误,把以前的号码全清掉
		}
	}
	
	if (j > 15)
	{
		*pszStr = '\0';
		return 0;
	}
	
	memcpy(pszStr, Strbuff, j);
	*(pszStr+j) =  '\0';
	return j ;
}

//描述:生成SMS地址,得到的地址是PDU格式的 长度+类型+地址的形式
//参数:@bType 地址类型 SMS_ADDR_SMSC, SMS_ADDR_PHONE
//	   @pbPhone 电话号码,BCD码,最后一位以F为结尾
//	   @pbSmsAddr 转换成的短信地址存放的缓存
//返回:填充到pbSmsAddr的字节数,
BYTE PhoneToSmsAddr(BYTE bType, char* pszPhone, BYTE* pbSmsAddr)
{
	BYTE bPhoneLen = strlen(pszPhone);

	if (bPhoneLen == 0)    //不用设置号码,则长度设成0x00即可
	{
		ByteToASCII(0x00, &pbSmsAddr);
		return 2;
	}

	BYTE* pbSmsAddr0 = pbSmsAddr;

	//Type-of-address 短信中心设为international number,发送/接收方号码设为unknown
	if (bType == SMS_ADDR_INTER)
	{
		ByteToASCII((bPhoneLen + 1) / 2 + 1, &pbSmsAddr);
		ByteToASCII(0x91, &pbSmsAddr);  //international number
	}
	else
	{
		ByteToASCII(bPhoneLen, &pbSmsAddr);  
		ByteToASCII(0x81, &pbSmsAddr);  //unknown format
	}
	
	for (BYTE i=0; i<bPhoneLen/2; i++)    //号码最多用8个字节能表示
	{
		BYTE n = i * 2;
		*pbSmsAddr++ = pszPhone[n+1];
		*pbSmsAddr++ = pszPhone[n];
	}

	if (bPhoneLen%2 != 0)
	{
		*pbSmsAddr++ = 'F';
		*pbSmsAddr++ = pszPhone[bPhoneLen-1];
	}

	return pbSmsAddr - pbSmsAddr0;
}

void LoadSmsPara(TModemPara *pModemPara)
{
	WORD  Len =0;
	BYTE bBuf[48] ,Num  ;
	char szPhone[32] , phoneInit[32];
	

	pModemPara->fEnSms = true;					//支持短信


	
	pModemPara->fMstSmsAddrValid = false;   //主站短信地址有效标志
	
	//if (GBReadItem(4,4,0,bBuf,0) 
	if (ReadItemEx(BN0, PN0, 0x004f, bBuf) > 0)
	{
		memset(phoneInit ,0,32) ;
		Len =PhoneToStr(bBuf, 8,phoneInit );   //用到刚才读到的bBuf
		if (Len > 6)	///SMS main phone
		{
			pModemPara->fMstSmsAddrValid = true;
			Num = 0;
			if (phoneInit[0] >= 'A')
			{
				Num++;
				Len -- ;
			}
			memcpy(&szPhone[2] , phoneInit+Num ,30) ;

			if (szPhone[2]=='8' && szPhone[3]=='6')
			{
				pModemPara->bDeftMstSmsAddrLen = PhoneToSmsAddr(SMS_ADDR_INTER, &szPhone[2], pModemPara->bDeftMstSmsAddr);
				if (szPhone[4]=='1' && szPhone[5]=='3')
					pModemPara->bDeftMstSmsAddr[1] ='D' ;
			}
			else
			{
				szPhone[0]='8';
				szPhone[1]='6';
				pModemPara->bDeftMstSmsAddrLen = PhoneToSmsAddr(SMS_ADDR_INTER, szPhone, pModemPara->bDeftMstSmsAddr);
				if (szPhone[2]=='1' && szPhone[3]=='3')
					pModemPara->bDeftMstSmsAddr[1] ='D' ;
			}
		}
		memset(phoneInit ,0,32) ;
		Len =PhoneToStr(bBuf+8, 8, phoneInit);
		if (Len>6) ///SMSc
		{
			Num = 0;
			if (phoneInit[0] >='A')
			{
				Num++;
				Len -- ;
			}
			memcpy(&szPhone[2] , phoneInit+Num ,30) ;
			
			if (szPhone[2]=='8' && szPhone[3]=='6')
			{
				pModemPara->bDeftSmscLen = PhoneToSmsAddr(SMS_ADDR_INTER, &szPhone[2], pModemPara->bDeftSmsc);
			}
			else
			{
				szPhone[0]='8';
				szPhone[1]='6';
				pModemPara->bDeftSmscLen = PhoneToSmsAddr(SMS_ADDR_INTER, szPhone, pModemPara->bDeftSmsc);
			}
		}
	}
	return; 
}

//描述:装载GPRS通信参数
void LoadGprsPara(TGprsPara* pGprsPara, BYTE bSockType)
{
	memset(pGprsPara, 0, sizeof(TGprsPara));

	LoadGprsModePara(pGprsPara);
	LoadSocketPara(&pGprsPara->SocketPara, bSockType); //装载socket通信参数	
	LoadSocketUnrstPara(&pGprsPara->SocketPara, bSockType);

	WORD wRstTime;
	ReadItemEx(BN10, PN0, 0xa142, (BYTE* )&wRstTime); //无通讯复位终端时间,单位分钟,HEX,默认4天
	pGprsPara->SocketPara.IfPara.dwNoRxRstAppInterv = (DWORD )wRstTime*60;
	pGprsPara->SocketPara.fEnSocketLed = true;

	//是否允许流量控制
	DWORD dwFluxLimit = 0;
	ReadItemEx(BN0, PN0, 0x024f, (BYTE*)&dwFluxLimit); //F36：终端上行通信流量门限设置,月通信流量门限为0，表示系统不需要终端进行流量控制
	if (dwFluxLimit != 0)
		pGprsPara->fEnableFluxCtrl = true;	
	else
		pGprsPara->fEnableFluxCtrl = false;	

	pGprsPara->wRstNum = 5;

	BYTE bEmbedProtcocol = 1;
	ReadItemEx(BN1, PN0, 0x2032, &bEmbedProtcocol);
	if (bEmbedProtcocol == 0)
		pGprsPara->wCnMode = CN_MODE_EMBED;
	else
		pGprsPara->wCnMode = CN_MODE_SOCKET;
	pGprsPara->SocketPara.IfPara.pszName = "gprs-master";	//接口名称
	DTRACE(DB_FAPROTO, ("LoadGprsPara ok\n"));
	return;
}


//返回：如果需要复位接口则返回true,否则返回false
bool LoadGprsUnrstPara(TGprsPara* pGprsPara)
{
	BYTE bBuf[4];
	//ReadItemEx(BN0, PN0, 0x4011, bBuf); //0x4011 1 上电激活的自动掉线时间,BCD,单位分钟,设为0自动取消上电激活
	//DWORD dwPowerupDropInterv = BcdToByte(bBuf[0]);
	//if (GetClick() < dwPowerupDropInterv*60) //上电激活期间,修改GPRS通信参数,不马上生效,等过了上电激活时间再生效
	//	return false;
	
	if (GetInfo(INFO_COMM_GPRS_RLD) == false)
	 	return false;

	SetInfo(INFO_WK_PARA);	//GPRS线程更新参数
	
	DTRACE(DB_FAPROTO, ("LoadGprsUnrstPara: para change\n"));
			
	//先保留部分需要接口重新初始化的参数
	DWORD dwRemoteIP = pGprsPara->SocketPara.dwRemoteIP;
	WORD wRemotePort = pGprsPara->SocketPara.wRemotePort;
	BYTE bOnlineMode = pGprsPara->bOnlineMode;	//在线模式
	SetInfo(INFO_COMM_TERMIP);
	LoadGprsPara(pGprsPara, SOCK_TYPE_GPRS);
	
	if (bOnlineMode!=pGprsPara->bOnlineMode || 
		dwRemoteIP!=pGprsPara->SocketPara.dwRemoteIP ||
		wRemotePort!=pGprsPara->SocketPara.wRemotePort)
		return true;
	else
		//return false;
		return true;
}

//描述:装载Local通信参数
void LoadLocalPara(TCommIfPara* pCommIfPara)
{
	memset(pCommIfPara, 0, sizeof(TCommIfPara));
//	pCommIfPara->IfPara.fAutoSend = false; //是否具有主动上送的功能
	pCommIfPara->IfPara.fNeedLogin = false;	//是否需要登录
	pCommIfPara->IfPara.wMaxFrmBytes = COMM_MAX_BYTES; //接口的一帧最大发送字节数,不同协议可能规定不一样
	pCommIfPara->IfPara.dwRstInterv = 10; //接口的复位间隔,单位秒
	pCommIfPara->IfPara.wRstNum = 3;
	pCommIfPara->IfPara.wLoginRstNum = 4; 	//登录失败断开连接的次数
	pCommIfPara->IfPara.dwLoginInterv = 10*1000; 	//登录间隔,单位秒
//	pCommIfPara->IfPara.wReconnetTimes = 0;  	//重连次数
//	pCommIfPara->IfPara.dwReconnetInterv = 0;	//重连时间间隔, 单位秒
	
	//串口配置 
	//pCommIfPara->CommPara.wPort = COMM_LOCAL; 
	//pCommIfPara->CommPara.dwBaudRate = CBR_2400; 
	//pCommIfPara->CommPara.bByteSize = 8; 
	//pCommIfPara->CommPara.bStopBits = ONESTOPBIT; 
	//pCommIfPara->CommPara.bParity = NOPARITY;
	
	pCommIfPara->pComm = &g_commLocal;		//则采用串口共享模式
	pCommIfPara->IfPara.pszName = "Local";	//接口名称
	//DTRACE(DB_FAPROTO, ("LoadLocalPara: wPort=%d, dwBaudRate=%d.\n",
	//				 	pCommIfPara->CommPara.wPort, pCommIfPara->CommPara.dwBaudRate));
}

//描述:装载Local通信参数
void LoadTestPara(TCommIfPara* pCommIfPara)
{
	memset(pCommIfPara, 0, sizeof(TCommIfPara));
//	pCommIfPara->IfPara.fAutoSend = false; //是否具有主动上送的功能
	pCommIfPara->IfPara.fNeedLogin = false;	//是否需要登录
	pCommIfPara->IfPara.wMaxFrmBytes = COMM_MAX_BYTES; //接口的一帧最大发送字节数,不同协议可能规定不一样
	pCommIfPara->IfPara.dwRstInterv = 10; //接口的复位间隔,单位秒
	pCommIfPara->IfPara.wRstNum = 3;
	pCommIfPara->IfPara.wLoginRstNum = 4; 	//登录失败断开连接的次数
	pCommIfPara->IfPara.dwLoginInterv = 10*1000; 	//登录间隔,单位秒
//	pCommIfPara->IfPara.wReconnetTimes = 0;  	//重连次数
//	pCommIfPara->IfPara.dwReconnetInterv = 0;	//重连时间间隔, 单位秒
	
	//串口配置 
	//pCommIfPara->CommPara.wPort = COMM_LOCAL; 
	//pCommIfPara->CommPara.dwBaudRate = CBR_2400; 
	//pCommIfPara->CommPara.bByteSize = 8; 
	//pCommIfPara->CommPara.bStopBits = ONESTOPBIT; 
	//pCommIfPara->CommPara.bParity = NOPARITY;
	
	pCommIfPara->pComm = &g_commTest;		//则采用串口共享模式
	pCommIfPara->IfPara.pszName = "Test";	//接口名称
#ifdef PRO_698
	pCommIfPara->fMutual = true;//红外232公用
#else
	pCommIfPara->fMutual = false;//红外232公用
#endif
	//DTRACE(DB_FAPROTO, ("LoadLocalPara: wPort=%d, dwBaudRate=%d.\n",
	//				 	pCommIfPara->CommPara.wPort, pCommIfPara->CommPara.dwBaudRate));
}

bool Load230mUnrstPara(TR230mIfPara* pR230mIfPara)
{
    BYTE bBuf[16];
	if (GetInfo(INFO_230M_PARA) == false)
	 	return false;

	ReadItemEx(BN0, PN0, 0x201b, bBuf); //0x201B F27 终端电台数传延时时间
	pR230mIfPara->dwRadioDelay = (DWORD )BcdToByte(bBuf[0]) * 20;
	if (pR230mIfPara->dwRadioDelay <= 150)
		pR230mIfPara->dwRadioDelay = 0;
	else
		pR230mIfPara->dwRadioDelay -= 150;
		
	return false;	
}

//描述:装载230M电台通信参数
void Load230MPara(TR230mIfPara* pR230mIfPara)
{
	memset(pR230mIfPara, 0, sizeof(TR230mIfPara));

	BYTE bBuf[16];
	ReadItemEx(BN0, PN0, 0x1001, bBuf);
	pR230mIfPara->CommIfPara.IfPara.wReSendNum = (bBuf[2]>>4) & 0x03;	//重发次数
	
	pR230mIfPara->CommIfPara.IfPara.fNeedLogin = false;	//是否需要登录
	pR230mIfPara->CommIfPara.IfPara.wMaxFrmBytes = COMM_MAX_BYTES; //接口的一帧最大发送字节数,不同协议可能规定不一样
	pR230mIfPara->CommIfPara.IfPara.dwRstInterv = 10; //接口的复位间隔,单位秒
	pR230mIfPara->CommIfPara.IfPara.wRstNum = 3;
	pR230mIfPara->CommIfPara.IfPara.wLoginRstNum = 3; 	//登录失败断开连接的次数
	pR230mIfPara->CommIfPara.IfPara.dwLoginInterv = 10; 	//登录间隔,单位秒

	//串口配置
	pR230mIfPara->CommIfPara.CommPara.wPort = COMM_GPRS;  //COMM_LOCAL
	pR230mIfPara->CommIfPara.CommPara.dwBaudRate = CBR_4800; 
	pR230mIfPara->CommIfPara.CommPara.bByteSize = 8; 
	pR230mIfPara->CommIfPara.CommPara.bStopBits = ONESTOPBIT; 
	pR230mIfPara->CommIfPara.CommPara.bParity = NOPARITY;
	DTRACE(DB_FAPROTO, ("Load230MPara: wPort=%d, dwBaudRate=%d.\n",
					 	pR230mIfPara->CommIfPara.CommPara.wPort, pR230mIfPara->CommIfPara.CommPara.dwBaudRate)); 
	pR230mIfPara->CommIfPara.pComm = NULL;
	
	ReadItemEx(BN0, PN0, 0x201b, bBuf); //0x201B F27 终端电台数传延时时间
	pR230mIfPara->dwRadioDelay = (DWORD )BcdToByte(bBuf[0]) * 20;
	if (pR230mIfPara->dwRadioDelay <= 150)
		pR230mIfPara->dwRadioDelay = 0;
	else
		pR230mIfPara->dwRadioDelay -= 150;
	
	pR230mIfPara->CommIfPara.IfPara.pszName = "230M";	//接口名称
}

//描述:装载Local通信参数
void LoadP2PPara(TP2PIfPara* pP2PIfPara)
{
	LoadIfDefPara(&pP2PIfPara->IfPara);   //装载通信接口的默认参数
	pP2PIfPara->IfPara.fNeedLogin = false;	//是否需要登录
	pP2PIfPara->IfPara.wMaxFrmBytes = COMM_MAX_BYTES; //接口的一帧最大发送字节数,不同协议可能规定不一样
	pP2PIfPara->IfPara.dwRstInterv = 10; //接口的复位间隔,单位秒
	pP2PIfPara->IfPara.wRstNum = 3;
	pP2PIfPara->IfPara.wLoginRstNum = 4; 	//登录失败断开连接的次数
	pP2PIfPara->IfPara.dwLoginInterv = 5*1000; 	//登录间隔,单位秒
	pP2PIfPara->IfPara.pszName = "P2P";

	pP2PIfPara->CommPara.dwBaudRate = CBR_9600; 
	pP2PIfPara->CommPara.bByteSize = 8; 
	pP2PIfPara->CommPara.bStopBits = ONESTOPBIT; 
	pP2PIfPara->CommPara.bParity = NOPARITY;
	DTRACE(DB_FAPROTO, ("LoadP2PPara: wPort=%d, dwBaudRate=%d.\n",
					 pP2PIfPara->CommPara.wPort, pP2PIfPara->CommPara.dwBaudRate));
}

//描述:装载Link串口通信参数
bool LoadLinkCommPara(TCommIfPara* pCommIfPara)
{
	BYTE bCheckMode;
	BYTE bByteSize;
	BYTE bBuf[100];
	WORD wRet=0;
	bBuf[0]=0;	

	memset(pCommIfPara, 0, sizeof(TCommIfPara));
	//if (GBReadItem(GB_DATACLASS4,37,0,bBuf,0) < 0)
	if (ReadItemEx(BN0, PN0, 0x025f, bBuf) < 0)
	{
		DTRACE(DB_FAPROTO, ("LinkLoadPara:ReadItemEx fail,wRet=%d.\n",wRet));
		return false;		
	}	
	else  //是主终端但串口不支持级联	
	{		
		//串口配置
		int iPort = GetLinkPhyPort();
		if (iPort>0)	//级联口优先第一路485口
			pCommIfPara->CommPara.wPort = iPort;		
		else
			return false;
		pCommIfPara->CommPara.dwBaudRate = NumToBaudrate((bBuf[1]>>5)&0x07); 	//波特率
		bByteSize = (bBuf[1]&0x03) + 5;
		if (bByteSize!=6 && bByteSize!=7 && bByteSize!=8)
			bByteSize = 8;
		pCommIfPara->CommPara.bByteSize = bByteSize; 
		pCommIfPara->CommPara.bStopBits = NumToStopBits((bBuf[1]>>4)&0x01); //1位停止位
		switch ((bBuf[1] & 0x0c) >> 2)
		{
			case 0: bCheckMode = 0;break;
			case 2: bCheckMode = 1;break;
			case 3: bCheckMode = 2;break;
			default: bCheckMode = 0;break;
		}
		pCommIfPara->CommPara.bParity = NumToParity(bCheckMode);		//偶
	}		
	pCommIfPara->IfPara.fNeedLogin = false;	//是否需要登录
	pCommIfPara->IfPara.wMaxFrmBytes = COMM_MAX_BYTES; //接口的一帧最大发送字节数,不同协议可能规定不一样
	pCommIfPara->IfPara.dwRstInterv = 10; //接口的复位间隔,单位秒
	pCommIfPara->IfPara.wRstNum = 3;
	pCommIfPara->IfPara.wLoginRstNum = 4; 	//登录失败断开连接的次数
	pCommIfPara->IfPara.dwLoginInterv = 10*1000; 	//登录间隔,单位秒
	pCommIfPara->IfPara.pszName = "Link";			//接口名称
	DTRACE(DB_FAPROTO, ("LinkLoadPara: wPort=%d, dwBaudRate=%d. bParity=%d\n",
					 pCommIfPara->CommPara.wPort, pCommIfPara->CommPara.dwBaudRate, pCommIfPara->CommPara.bParity));
						
	return true;
}


//描述:装载主站协议参数
void LoadFaProPara(TFaProPara* pFaProPara)
{
	pFaProPara->ProPara.fUseLoopBuf = true;
}

bool LoadFaProUnrstPara(TFaProPara* pFaProPara)
{
	if (GetInfo(INFO_FAP_PARA) == false)
	 	return false;
	
	DTRACE(DB_FAPROTO, ("LoadFaProUnrstPara: para change\n"));
	
	//由于装载参数比较简单,只有本地地址需要重装载,
	//而且本地维护口不需要地址,所以只有GPRS/SOCKET端口
	//才需要注册装载主站协议参数的函数,
	//只需要一个INFO_FAP_PARA被GPRS/SOCKET端口就行了,
	//本地维护口不注册装载参数的函数
	
	LoadFaProPara(pFaProPara);
	return false;	
}

