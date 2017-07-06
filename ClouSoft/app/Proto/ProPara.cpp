/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�FapPara.cpp
 * ժ    Ҫ�����ļ���Ҫ�����Ѹ�Э�鲻ͬ�Ĳ���װ�ص���ͬ�Ĳ����ṹ��ȥ,
 *			 ��TSocketPara,TGprsPara��,ʹ���õ�ͨ�Ŵ��벻��ֱ����Ը���
 *			 Э��Ĳ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע��
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

//��������ȡ��̫���Ĺ���ģʽ 
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
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 0, &wLen, &bType);	//����ģʽ
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

//��������ȡ��̫��������ģʽ
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
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 1, &wLen, &bType);	//���ӷ�ʽ
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

//��������ȡ��̫��������Ӧ�÷�ʽ
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
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 2, &wLen, &bType);	//Ӧ�����ӷ�ʽ
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

//��������ȡ��̫���������˿�
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
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 3, &wLen, &bType);	//�����˿��б�
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

//��������ȡ�����������ַ
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
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 4, &wLen, &bType);	//�����˿��б�
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

//��������ȡ����������˿�
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
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 5, &wLen, &bType);	//����������˿�
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

//��������ȡ��̫���ĳ�ʱʱ��
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
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 6, &wLen, &bType);	//��ʱʱ��
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

//��������ȡ��̫�������Դ���
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
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 6, &wLen, &bType);	//���Դ���
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

//��������ȡ��̫����������(unit:s)
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
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	dwOAD = 0x45100200 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 7, &wLen, &bType);	//����
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

//��������ȡ��̫�����÷�ʽ DHCP��0������̬��1����PPPoE��2��
BYTE GetEthConfigType()
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bModule, bEthType, bLen, bNum, bType;
	BYTE *pbPtr, *pbFmt;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	dwOAD = 0x45100400 + bModule*0x00010000;	
	bEthType = 1;
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		//IP���÷�ʽ
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 0, &wLen, &bType);
		if (pbPtr != NULL)
		{
			if (*pbPtr++ == DT_ENUM)	
				bEthType = *pbPtr++;
		}
	}

	return bEthType;
}

//��������ȡ��̫������IP��ַ
BYTE GetEthLocalIp(BYTE *pAddr)
{	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bModule, bLen, bType;
	BYTE *pbPtr, *pbFmt;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	dwOAD = 0x45100400 + bModule*0x00010000;	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		//IP��ַ
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

//��������ȡ��̫����������
BYTE GetEthNetMask(BYTE *pNetMsk)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bModule, bEthType, bLen, bNum, bType;
	BYTE *pbPtr, *pbFmt;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��

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
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��

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

//��������ȡ��̫��PPPoE�û�����
BYTE GetEthPPPoEUserName(char *pszUserName)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bModule, bEthType, bLen, bNum, bType;
	BYTE *pbPtr, *pbFmt;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
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

//��������ȡ��̫��PPPoE�û�����
BYTE GetEthPPPoEUserPwd(char *pszUserPwd)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bModule, bEthType, bLen, bNum, bType;
	BYTE *pbPtr, *pbFmt;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
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

//��������ȡ��̫��MAC��ַ
BYTE GetEthMacAddr(BYTE *pbMac)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256];
	BYTE bchn, bEthType, bLen, bNum, bType;
	BYTE *pbPtr, *pbFmt;

	bchn = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bchn);	//��ȡ��̫����Ӧ��ģ��

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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 0, &wLen, &bType);	//����ģʽ
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 1, &wLen, &bType);	//���߷�ʽ
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 2, &wLen, &bType);	//��������
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 3, &wLen, &bType);	//����Ӧ�÷�ʽ
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 4, &wLen, &bType);	//�����˿��б�
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 6, &wLen, &bType);	//�û���
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 7, &wLen, &bType);	//����
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 8, &wLen, &bType);	//�����������ַ
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 9, &wLen, &bType);	//����˿� 
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 10, &wLen, &bType);	//��ʱʱ�估�ط�����
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 10, &wLen, &bType);	//��ʱʱ�估�ط�����
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000200 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 11, &wLen, &bType);	//����
		if (pbPtr != NULL)
		{
			if(*pbPtr++ == DT_LONG_U)
			{
				wBeat = OoOiToWord(pbPtr);
			}
		}
	}
	if (wBeat == 0)
		wBeat = 300; //Ĭ��5����

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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000300 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 0, &wLen, &bType);	//�������ĺ���
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000300 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 1, &wLen, &bType);	//��վ����
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
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
	dwOAD = 0x45000300 + bModule*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 1, &wLen, &bType);	//����֪ͨĿ�ĺ���
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


//������װ����̫��ͨ�����ò���
void LoadEthCommPara(TSocketPara* pPara)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bModule;

	bModule = 0;
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	DTRACE(DB_FAPROTO, ("Get ethernet module=%d.\n", bModule));

	BYTE bWorkMode;
	bWorkMode = GetEthWorkMode();
	if (bWorkMode == GPRS_CLI_MODE)	//�ͻ���ģʽ
		pPara->fSvr = false;
	else	//���ģʽ || ������ģʽ
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

	//pPara->IfPara.dwDormanInterv = 300;//����ʱ����, ��λ��, ,0��ʾ��ֹ����ģʽ
	pPara->IfPara.dwDormanInterv = 10;//����ʱ����, ��λ��, ,0��ʾ��ֹ����ģʽ
	pPara->IfPara.wReTryNum = 2;
	pPara->fEnableFluxStat = false; //�Ƿ�������������,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
	pPara->wDisConnectByPeerNum = 3;	//���Է��Ͽ����ӣ��л�������״̬�Ĵ���

	//װ��TIfPara����
	pPara->IfPara.fNeedLogin = true;	//�Ƿ���Ҫ��¼
	pPara->IfPara.wMaxFrmBytes = SOCK_MAX_BYTES; //�ӿڵ�һ֡������ֽ���,��ͬЭ����ܹ涨��һ��
	pPara->IfPara.dwRstInterv = 10; //�ӿڵĸ�λ���,��λ��
	pPara->IfPara.wConnectNum = 2;//��½���Դ���
	pPara->IfPara.wRstNum = 2;
	pPara->IfPara.wLoginRstNum = 2; 	//��¼ʧ�ܶϿ����ӵĴ���
	pPara->IfPara.wLoginNum = 2; 		//��¼ʧ���������ԵĴ���
	pPara->IfPara.dwLoginInterv = 10; 	//��¼���,��λ��
	pPara->IfPara.dwConnectInterv= 20; 	////�ӿڵ����Ӽ��,��λ��
	pPara->bSvrDisconMode = SVR_DISCON_EXIT;
}

//������װ��GPRSͨ�����ò���
void LoadGPRSCommPara(TSocketPara* pPara)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bchn;

	bchn = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bchn);	//��ȡGPRS��Ӧ��ģ��
	DTRACE(DB_FAPROTO, ("Get gprs channel=%d.\n", bchn));

	BYTE bWorkMode;
	bWorkMode = GetGprsWorkMode();
	if (bWorkMode == GPRS_CLI_MODE)	//�ͻ���ģʽ
		pPara->fSvr = false;
	else	//���ģʽ || ������ģʽ
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

	//pPara->IfPara.dwDormanInterv = 300;//����ʱ����, ��λ��, ,0��ʾ��ֹ����ģʽ
	pPara->IfPara.dwDormanInterv = 10;//����ʱ����, ��λ��, ,0��ʾ��ֹ����ģʽ
	pPara->IfPara.wReTryNum = 2;
	pPara->fEnableFluxStat = true; //�Ƿ�������������,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
	pPara->wDisConnectByPeerNum = 3;	//���Է��Ͽ����ӣ��л�������״̬�Ĵ���

	//װ��TIfPara����
	pPara->IfPara.fNeedLogin = true;	//�Ƿ���Ҫ��¼
	pPara->IfPara.wMaxFrmBytes = SOCK_MAX_BYTES; //�ӿڵ�һ֡������ֽ���,��ͬЭ����ܹ涨��һ��
	pPara->IfPara.dwRstInterv = 10; //�ӿڵĸ�λ���,��λ��
	pPara->IfPara.wConnectNum = 2;//��½���Դ���
	pPara->IfPara.wRstNum = 2;
	pPara->IfPara.wLoginRstNum = 2; 	//��¼ʧ�ܶϿ����ӵĴ���
	pPara->IfPara.wLoginNum = 2; 		//��¼ʧ���������ԵĴ���
	pPara->IfPara.dwLoginInterv = 10; 	//��¼���,��λ��
	pPara->IfPara.dwConnectInterv= 20; 	////�ӿڵ����Ӽ��,��λ��
	pPara->bSvrDisconMode = SVR_DISCON_EXIT;
}


//����:װ��ͨ�Žӿڵ�Ĭ�ϲ���
void LoadIfDefPara(TIfPara* pIfPara)
{
	//��Ϣ�궨��,�����������п��õ��ĺ궨���Ӧ�ó���һ��
	pIfPara->wInfoActive = INFO_ACTIVE;
	pIfPara->wInfoAppRst = INFO_APP_RST;
}

//�������ն�����ͨѶ����
//������ʽ��02 12 
//			0	22 **	//����ģʽenum{���ģʽ��0�����ͻ���ģʽ��1����������ģʽ��2��}��
//			1	22 **	//���߷�ʽenum{�������ߣ�0�����������1��}��
//			2	22 **	//���ӷ�ʽenum{TCP��0����UDP��1��}��
//			3	22 **	//����Ӧ�÷�ʽenum{����ģʽ��0����������ģʽ��1��}
//			4	01 **	//�����˿��б�  array long-unsigned
//					18 ** **
//					........
//					18 ** **
//			5	10 num ** ** ** ** **.......//APN  visible-string
//			6	10 num ** ** ** ** **.......//�û���         visible-string
//			7	10 num ** ** ** ** **.......//����           visible-string
//			8	09 num ** ** ** **	//�����������ַ octet-string
//			9	18 ** **	//����˿�            long-unsigned
//			10	4 **		//��ʱʱ�估�ط�����  bit-string(SIZE(8))
//			11	18 ** **	//��������(��)  long-unsigned
void LoadSocketUnrstPara(TSocketPara* pPara, BYTE bSockType)
{
	DWORD dwSvrModeBeatTimes;
	BYTE bBuf[10] = {0};

	if (bSockType == SOCK_TYPE_ETH)
		LoadEthCommPara(pPara);
	else
		LoadGPRSCommPara(pPara);

	ReadItemEx(BN1, PN0, 0x2030, bBuf);		//������ģʽ���������� NN.NN
	dwSvrModeBeatTimes = BcdToDWORD(bBuf, 2);
	g_dwSvrModeBeatTestTimes = dwSvrModeBeatTimes * (pPara->dwBeatSeconds) / 100;

}

//����:װ����̫����վͨ�Ų���
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
					if (*pbPtr++ == DT_OCT_STR)	//IP��ַ
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

						WriteItemEx(BANK17, PN0, 0x6012, &i);	//������̫����Ӧ��ͨ��
						WriteItemEx(BANK17, PN0, 0x6013, &j);	//������Ӧͨ���е���ӦIP��ַ
						DTRACE(DB_FAPROTO, ("Select ethernet channel=%d, Ip sn=%d.\n", i, j));
						return;
					}
				}
			}
		}
	}
#endif
}

//����:װ��GPRS��վͨ�Ų���
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
					if (*pbPtr++ == DT_OCT_STR)	//IP��ַ
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

						WriteItemEx(BANK17, PN0, 0x6010, &i);	//����GPRS��Ӧ��ͨ��
						WriteItemEx(BANK17, PN0, 0x6011, &j);	//������Ӧͨ���е���ӦIP��ַ
						DTRACE(DB_FAPROTO, ("Select gprs Channel=%d, Ip sn=%d.\n", i, j));
						return;
					}
				}
			}
		}
	}
#endif
}

//����:װ��socketͨ�Ų���
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
	ReadItemEx(BANK17, PN0, 0x6012, &bModule);	//��ȡ��̫����Ӧ��ģ��
	DTRACE(DB_FAPROTO, ("Get ethernet Module=%d.\n", bModule));

	//1. ����MAC��ַ
	if (!GetEthMacAddr(bMac))
		memcpy(bMac, "\x11\x22\x33\x44\x55\x66", sizeof("\x11\x22\x33\x44\x55\x66"));

	//2. ������̫��ip��ַ���������롢���ء�PPPoE
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
	case ETH_IP_CFG_STATIC:	//��̬
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
	//socketģʽ�¸��²���()
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
	LoadSocketPara(pPara, SOCK_TYPE_ETH); //װ��socketͨ�Ų���	
	LoadSocketUnrstPara(pPara, SOCK_TYPE_ETH);
	return true;
}


void LoadGprsCommPara(TGprsWorkerPara* pWorkerPara)
{
	BYTE bModuleType;
	
	//GPRS��������	
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
//	ReadItemEx(BANK1, PN0, 0x2013, &bConnectWait);  //0x2013 1 GPRS���ӵȴ�ʱ�� HEX ��
	pWorkerPara->wConnectWait = bConnectWait;		//���ӵȴ�ʱ��
	
	return;
}

void LoadGprsApnPara(TGprsWorkerPara* pWorkerPara)
{
	BYTE bModule = 0;
	BYTE bBuf[64];
	
	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
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

	ReadItemEx(BANK17, PN0, 0x6010, &bModule);	//��ȡGPRS��Ӧ��ģ��
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

	//��Ϣ�궨��,�����������п��õ��ĺ궨���Ӧ�ó���һ��
	pWorkerPara->wInfoActive = INFO_ACTIVE;

	BYTE bEmbedProtcocol = 1;
	ReadItemEx(BN1, PN0, 0x2032, &bEmbedProtcocol);
	if (bEmbedProtcocol == 0)
		pWorkerPara->fEmbedProtocol = true;
	else
		pWorkerPara->fEmbedProtocol = false;


	//���ӻ�ȡ֧�ֻ�ȡ��վ��Ϣ    
	pWorkerPara->ptNetInfo = NULL;
	//ʹ����ϸ������ϣ�������������Ҫʵʱ��Ч�Ļ������ò�������
	//�ŵ�LoadGprsWorkerUnrstPara�г�ʼ��
	pWorkerPara->fDetailDiagnose = false;
	////�Ƿ����Socket Led,ֻ��Թ�����׼ģ��
	pWorkerPara->fEnSocketLed = true;
	pWorkerPara->fEnMux = true;			//�Ƿ������ڸ���
	//��ȡ�ź�ǿ�Ⱥͷ��书�ʵ�ʱ������Ϊ0��ʾ����ȡ��fEnMux(���ڸ���)��true
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

//����:װ��GPRS����ģʽ����
void LoadGprsModePara(TGprsPara* pGprsPara)
{
	DWORD dwOAD;
	WORD wFmtLen, wLen;
	BYTE bBuf[256], bFmt[128];
	BYTE *pbFmt, *pbPtr;
	BYTE bType;
	BYTE bchn;

	bchn = 0;
	ReadItemEx(BANK17, PN0, 0x6010, &bchn);	//��ȡGPRS��Ӧ��ģ��
	DTRACE(DB_FAPROTO, ("LoadGprsModePara: Get gprs channel=%d.\n", bchn));
	dwOAD = 0x45000200 + bchn*0x00010000;	//ͨ�����ò���	
	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(dwOAD>>16, (dwOAD>>8)&0xff, bBuf, &pbFmt, &wFmtLen) > 0)
	{
		pbPtr = OoGetField(bBuf, pbFmt, wFmtLen, 1, &wLen, &bType);	//���߷�ʽ
		if (pbPtr != NULL)
		{
			if (*pbPtr++ == DT_ENUM)
			{
				BYTE bPowerupDropInterv = 5;  //0x4011 1 �ϵ缤����Զ�����ʱ��,BCD,��λ����,��Ϊ0�Զ�ȡ���ϵ缤��
				BYTE bPowerupBeatMinutes = 0;  //0x4012 1 �ϵ缤����������,BCD,��λ����

				if (*pbPtr == 1)	//����ģʽ
				{
					pGprsPara->bOnlineMode = ONLINE_M_ACTIVE;
					pGprsPara->fEnableAutoSendActive = false; 	//���������ϱ�����
					pGprsPara->SocketPara.IfPara.wReTryNum = bBuf[3];	//�ز�����(��������:��)
					pGprsPara->dwActiveDropInterv = bBuf[4];	//�Զ�����ʱ��(��������:min)

					pGprsPara->dwPowerupDropInterv = bPowerupDropInterv; 	 
					//�ϵ缤����Զ�����ʱ��,��λ����,��Ϊ0�Զ�ȡ���ϵ缤��
					pGprsPara->dwPowerupBeatMinutes = bPowerupBeatMinutes;	 
					//�ϵ缤����������

					pGprsPara->fRstOnSms = true;			//�Ƿ�λ������ģʽ����Ҫ��Լ���ģʽ��ʱ������ģʽ
				}
				else //��������
				{
					pGprsPara->bOnlineMode = ONLINE_M_PERSIST;
					pGprsPara->fEnableAutoSendActive = false; 	//���������ϱ�����
					pGprsPara->SocketPara.IfPara.wReTryNum = 2;	//�ز�����(��������:��)
					pGprsPara->dwActiveDropInterv = 5;	//�Զ�����ʱ��(��������:min)

					pGprsPara->dwPowerupDropInterv = bPowerupDropInterv; 	 
					//�ϵ缤����Զ�����ʱ��,��λ����,��Ϊ0�Զ�ȡ���ϵ缤��
					pGprsPara->dwPowerupBeatMinutes = bPowerupBeatMinutes;	 
					//�ϵ缤����������

					pGprsPara->fRstOnSms = false;			//�Ƿ�λ������ģʽ����Ҫ��Լ���ģʽ��ʱ������ģʽ
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
			j = 0;	//����,����ǰ�ĺ���ȫ���
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

//����:����SMS��ַ,�õ��ĵ�ַ��PDU��ʽ�� ����+����+��ַ����ʽ
//����:@bType ��ַ���� SMS_ADDR_SMSC, SMS_ADDR_PHONE
//	   @pbPhone �绰����,BCD��,���һλ��FΪ��β
//	   @pbSmsAddr ת���ɵĶ��ŵ�ַ��ŵĻ���
//����:��䵽pbSmsAddr���ֽ���,
BYTE PhoneToSmsAddr(BYTE bType, char* pszPhone, BYTE* pbSmsAddr)
{
	BYTE bPhoneLen = strlen(pszPhone);

	if (bPhoneLen == 0)    //�������ú���,�򳤶����0x00����
	{
		ByteToASCII(0x00, &pbSmsAddr);
		return 2;
	}

	BYTE* pbSmsAddr0 = pbSmsAddr;

	//Type-of-address ����������Ϊinternational number,����/���շ�������Ϊunknown
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
	
	for (BYTE i=0; i<bPhoneLen/2; i++)    //���������8���ֽ��ܱ�ʾ
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
	

	pModemPara->fEnSms = true;					//֧�ֶ���


	
	pModemPara->fMstSmsAddrValid = false;   //��վ���ŵ�ַ��Ч��־
	
	//if (GBReadItem(4,4,0,bBuf,0) 
	if (ReadItemEx(BN0, PN0, 0x004f, bBuf) > 0)
	{
		memset(phoneInit ,0,32) ;
		Len =PhoneToStr(bBuf, 8,phoneInit );   //�õ��ղŶ�����bBuf
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

//����:װ��GPRSͨ�Ų���
void LoadGprsPara(TGprsPara* pGprsPara, BYTE bSockType)
{
	memset(pGprsPara, 0, sizeof(TGprsPara));

	LoadGprsModePara(pGprsPara);
	LoadSocketPara(&pGprsPara->SocketPara, bSockType); //װ��socketͨ�Ų���	
	LoadSocketUnrstPara(&pGprsPara->SocketPara, bSockType);

	WORD wRstTime;
	ReadItemEx(BN10, PN0, 0xa142, (BYTE* )&wRstTime); //��ͨѶ��λ�ն�ʱ��,��λ����,HEX,Ĭ��4��
	pGprsPara->SocketPara.IfPara.dwNoRxRstAppInterv = (DWORD )wRstTime*60;
	pGprsPara->SocketPara.fEnSocketLed = true;

	//�Ƿ�������������
	DWORD dwFluxLimit = 0;
	ReadItemEx(BN0, PN0, 0x024f, (BYTE*)&dwFluxLimit); //F36���ն�����ͨ��������������,��ͨ����������Ϊ0����ʾϵͳ����Ҫ�ն˽�����������
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
	pGprsPara->SocketPara.IfPara.pszName = "gprs-master";	//�ӿ�����
	DTRACE(DB_FAPROTO, ("LoadGprsPara ok\n"));
	return;
}


//���أ������Ҫ��λ�ӿ��򷵻�true,���򷵻�false
bool LoadGprsUnrstPara(TGprsPara* pGprsPara)
{
	BYTE bBuf[4];
	//ReadItemEx(BN0, PN0, 0x4011, bBuf); //0x4011 1 �ϵ缤����Զ�����ʱ��,BCD,��λ����,��Ϊ0�Զ�ȡ���ϵ缤��
	//DWORD dwPowerupDropInterv = BcdToByte(bBuf[0]);
	//if (GetClick() < dwPowerupDropInterv*60) //�ϵ缤���ڼ�,�޸�GPRSͨ�Ų���,��������Ч,�ȹ����ϵ缤��ʱ������Ч
	//	return false;
	
	if (GetInfo(INFO_COMM_GPRS_RLD) == false)
	 	return false;

	SetInfo(INFO_WK_PARA);	//GPRS�̸߳��²���
	
	DTRACE(DB_FAPROTO, ("LoadGprsUnrstPara: para change\n"));
			
	//�ȱ���������Ҫ�ӿ����³�ʼ���Ĳ���
	DWORD dwRemoteIP = pGprsPara->SocketPara.dwRemoteIP;
	WORD wRemotePort = pGprsPara->SocketPara.wRemotePort;
	BYTE bOnlineMode = pGprsPara->bOnlineMode;	//����ģʽ
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

//����:װ��Localͨ�Ų���
void LoadLocalPara(TCommIfPara* pCommIfPara)
{
	memset(pCommIfPara, 0, sizeof(TCommIfPara));
//	pCommIfPara->IfPara.fAutoSend = false; //�Ƿ�����������͵Ĺ���
	pCommIfPara->IfPara.fNeedLogin = false;	//�Ƿ���Ҫ��¼
	pCommIfPara->IfPara.wMaxFrmBytes = COMM_MAX_BYTES; //�ӿڵ�һ֡������ֽ���,��ͬЭ����ܹ涨��һ��
	pCommIfPara->IfPara.dwRstInterv = 10; //�ӿڵĸ�λ���,��λ��
	pCommIfPara->IfPara.wRstNum = 3;
	pCommIfPara->IfPara.wLoginRstNum = 4; 	//��¼ʧ�ܶϿ����ӵĴ���
	pCommIfPara->IfPara.dwLoginInterv = 10*1000; 	//��¼���,��λ��
//	pCommIfPara->IfPara.wReconnetTimes = 0;  	//��������
//	pCommIfPara->IfPara.dwReconnetInterv = 0;	//����ʱ����, ��λ��
	
	//�������� 
	//pCommIfPara->CommPara.wPort = COMM_LOCAL; 
	//pCommIfPara->CommPara.dwBaudRate = CBR_2400; 
	//pCommIfPara->CommPara.bByteSize = 8; 
	//pCommIfPara->CommPara.bStopBits = ONESTOPBIT; 
	//pCommIfPara->CommPara.bParity = NOPARITY;
	
	pCommIfPara->pComm = &g_commLocal;		//����ô��ڹ���ģʽ
	pCommIfPara->IfPara.pszName = "Local";	//�ӿ�����
	//DTRACE(DB_FAPROTO, ("LoadLocalPara: wPort=%d, dwBaudRate=%d.\n",
	//				 	pCommIfPara->CommPara.wPort, pCommIfPara->CommPara.dwBaudRate));
}

//����:װ��Localͨ�Ų���
void LoadTestPara(TCommIfPara* pCommIfPara)
{
	memset(pCommIfPara, 0, sizeof(TCommIfPara));
//	pCommIfPara->IfPara.fAutoSend = false; //�Ƿ�����������͵Ĺ���
	pCommIfPara->IfPara.fNeedLogin = false;	//�Ƿ���Ҫ��¼
	pCommIfPara->IfPara.wMaxFrmBytes = COMM_MAX_BYTES; //�ӿڵ�һ֡������ֽ���,��ͬЭ����ܹ涨��һ��
	pCommIfPara->IfPara.dwRstInterv = 10; //�ӿڵĸ�λ���,��λ��
	pCommIfPara->IfPara.wRstNum = 3;
	pCommIfPara->IfPara.wLoginRstNum = 4; 	//��¼ʧ�ܶϿ����ӵĴ���
	pCommIfPara->IfPara.dwLoginInterv = 10*1000; 	//��¼���,��λ��
//	pCommIfPara->IfPara.wReconnetTimes = 0;  	//��������
//	pCommIfPara->IfPara.dwReconnetInterv = 0;	//����ʱ����, ��λ��
	
	//�������� 
	//pCommIfPara->CommPara.wPort = COMM_LOCAL; 
	//pCommIfPara->CommPara.dwBaudRate = CBR_2400; 
	//pCommIfPara->CommPara.bByteSize = 8; 
	//pCommIfPara->CommPara.bStopBits = ONESTOPBIT; 
	//pCommIfPara->CommPara.bParity = NOPARITY;
	
	pCommIfPara->pComm = &g_commTest;		//����ô��ڹ���ģʽ
	pCommIfPara->IfPara.pszName = "Test";	//�ӿ�����
#ifdef PRO_698
	pCommIfPara->fMutual = true;//����232����
#else
	pCommIfPara->fMutual = false;//����232����
#endif
	//DTRACE(DB_FAPROTO, ("LoadLocalPara: wPort=%d, dwBaudRate=%d.\n",
	//				 	pCommIfPara->CommPara.wPort, pCommIfPara->CommPara.dwBaudRate));
}

bool Load230mUnrstPara(TR230mIfPara* pR230mIfPara)
{
    BYTE bBuf[16];
	if (GetInfo(INFO_230M_PARA) == false)
	 	return false;

	ReadItemEx(BN0, PN0, 0x201b, bBuf); //0x201B F27 �ն˵�̨������ʱʱ��
	pR230mIfPara->dwRadioDelay = (DWORD )BcdToByte(bBuf[0]) * 20;
	if (pR230mIfPara->dwRadioDelay <= 150)
		pR230mIfPara->dwRadioDelay = 0;
	else
		pR230mIfPara->dwRadioDelay -= 150;
		
	return false;	
}

//����:װ��230M��̨ͨ�Ų���
void Load230MPara(TR230mIfPara* pR230mIfPara)
{
	memset(pR230mIfPara, 0, sizeof(TR230mIfPara));

	BYTE bBuf[16];
	ReadItemEx(BN0, PN0, 0x1001, bBuf);
	pR230mIfPara->CommIfPara.IfPara.wReSendNum = (bBuf[2]>>4) & 0x03;	//�ط�����
	
	pR230mIfPara->CommIfPara.IfPara.fNeedLogin = false;	//�Ƿ���Ҫ��¼
	pR230mIfPara->CommIfPara.IfPara.wMaxFrmBytes = COMM_MAX_BYTES; //�ӿڵ�һ֡������ֽ���,��ͬЭ����ܹ涨��һ��
	pR230mIfPara->CommIfPara.IfPara.dwRstInterv = 10; //�ӿڵĸ�λ���,��λ��
	pR230mIfPara->CommIfPara.IfPara.wRstNum = 3;
	pR230mIfPara->CommIfPara.IfPara.wLoginRstNum = 3; 	//��¼ʧ�ܶϿ����ӵĴ���
	pR230mIfPara->CommIfPara.IfPara.dwLoginInterv = 10; 	//��¼���,��λ��

	//��������
	pR230mIfPara->CommIfPara.CommPara.wPort = COMM_GPRS;  //COMM_LOCAL
	pR230mIfPara->CommIfPara.CommPara.dwBaudRate = CBR_4800; 
	pR230mIfPara->CommIfPara.CommPara.bByteSize = 8; 
	pR230mIfPara->CommIfPara.CommPara.bStopBits = ONESTOPBIT; 
	pR230mIfPara->CommIfPara.CommPara.bParity = NOPARITY;
	DTRACE(DB_FAPROTO, ("Load230MPara: wPort=%d, dwBaudRate=%d.\n",
					 	pR230mIfPara->CommIfPara.CommPara.wPort, pR230mIfPara->CommIfPara.CommPara.dwBaudRate)); 
	pR230mIfPara->CommIfPara.pComm = NULL;
	
	ReadItemEx(BN0, PN0, 0x201b, bBuf); //0x201B F27 �ն˵�̨������ʱʱ��
	pR230mIfPara->dwRadioDelay = (DWORD )BcdToByte(bBuf[0]) * 20;
	if (pR230mIfPara->dwRadioDelay <= 150)
		pR230mIfPara->dwRadioDelay = 0;
	else
		pR230mIfPara->dwRadioDelay -= 150;
	
	pR230mIfPara->CommIfPara.IfPara.pszName = "230M";	//�ӿ�����
}

//����:װ��Localͨ�Ų���
void LoadP2PPara(TP2PIfPara* pP2PIfPara)
{
	LoadIfDefPara(&pP2PIfPara->IfPara);   //װ��ͨ�Žӿڵ�Ĭ�ϲ���
	pP2PIfPara->IfPara.fNeedLogin = false;	//�Ƿ���Ҫ��¼
	pP2PIfPara->IfPara.wMaxFrmBytes = COMM_MAX_BYTES; //�ӿڵ�һ֡������ֽ���,��ͬЭ����ܹ涨��һ��
	pP2PIfPara->IfPara.dwRstInterv = 10; //�ӿڵĸ�λ���,��λ��
	pP2PIfPara->IfPara.wRstNum = 3;
	pP2PIfPara->IfPara.wLoginRstNum = 4; 	//��¼ʧ�ܶϿ����ӵĴ���
	pP2PIfPara->IfPara.dwLoginInterv = 5*1000; 	//��¼���,��λ��
	pP2PIfPara->IfPara.pszName = "P2P";

	pP2PIfPara->CommPara.dwBaudRate = CBR_9600; 
	pP2PIfPara->CommPara.bByteSize = 8; 
	pP2PIfPara->CommPara.bStopBits = ONESTOPBIT; 
	pP2PIfPara->CommPara.bParity = NOPARITY;
	DTRACE(DB_FAPROTO, ("LoadP2PPara: wPort=%d, dwBaudRate=%d.\n",
					 pP2PIfPara->CommPara.wPort, pP2PIfPara->CommPara.dwBaudRate));
}

//����:װ��Link����ͨ�Ų���
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
	else  //�����ն˵����ڲ�֧�ּ���	
	{		
		//��������
		int iPort = GetLinkPhyPort();
		if (iPort>0)	//���������ȵ�һ·485��
			pCommIfPara->CommPara.wPort = iPort;		
		else
			return false;
		pCommIfPara->CommPara.dwBaudRate = NumToBaudrate((bBuf[1]>>5)&0x07); 	//������
		bByteSize = (bBuf[1]&0x03) + 5;
		if (bByteSize!=6 && bByteSize!=7 && bByteSize!=8)
			bByteSize = 8;
		pCommIfPara->CommPara.bByteSize = bByteSize; 
		pCommIfPara->CommPara.bStopBits = NumToStopBits((bBuf[1]>>4)&0x01); //1λֹͣλ
		switch ((bBuf[1] & 0x0c) >> 2)
		{
			case 0: bCheckMode = 0;break;
			case 2: bCheckMode = 1;break;
			case 3: bCheckMode = 2;break;
			default: bCheckMode = 0;break;
		}
		pCommIfPara->CommPara.bParity = NumToParity(bCheckMode);		//ż
	}		
	pCommIfPara->IfPara.fNeedLogin = false;	//�Ƿ���Ҫ��¼
	pCommIfPara->IfPara.wMaxFrmBytes = COMM_MAX_BYTES; //�ӿڵ�һ֡������ֽ���,��ͬЭ����ܹ涨��һ��
	pCommIfPara->IfPara.dwRstInterv = 10; //�ӿڵĸ�λ���,��λ��
	pCommIfPara->IfPara.wRstNum = 3;
	pCommIfPara->IfPara.wLoginRstNum = 4; 	//��¼ʧ�ܶϿ����ӵĴ���
	pCommIfPara->IfPara.dwLoginInterv = 10*1000; 	//��¼���,��λ��
	pCommIfPara->IfPara.pszName = "Link";			//�ӿ�����
	DTRACE(DB_FAPROTO, ("LinkLoadPara: wPort=%d, dwBaudRate=%d. bParity=%d\n",
					 pCommIfPara->CommPara.wPort, pCommIfPara->CommPara.dwBaudRate, pCommIfPara->CommPara.bParity));
						
	return true;
}


//����:װ����վЭ�����
void LoadFaProPara(TFaProPara* pFaProPara)
{
	pFaProPara->ProPara.fUseLoopBuf = true;
}

bool LoadFaProUnrstPara(TFaProPara* pFaProPara)
{
	if (GetInfo(INFO_FAP_PARA) == false)
	 	return false;
	
	DTRACE(DB_FAPROTO, ("LoadFaProUnrstPara: para change\n"));
	
	//����װ�ز����Ƚϼ�,ֻ�б��ص�ַ��Ҫ��װ��,
	//���ұ���ά���ڲ���Ҫ��ַ,����ֻ��GPRS/SOCKET�˿�
	//����Ҫע��װ����վЭ������ĺ���,
	//ֻ��Ҫһ��INFO_FAP_PARA��GPRS/SOCKET�˿ھ�����,
	//����ά���ڲ�ע��װ�ز����ĺ���
	
	LoadFaProPara(pFaProPara);
	return false;	
}

