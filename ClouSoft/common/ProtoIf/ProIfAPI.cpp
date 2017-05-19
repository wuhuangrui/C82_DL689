/*********************************************************************************************************
 * Copyright (c) 2010,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProIfAPI.cpp
 * 摘    要：本文件实现了通信接口函数定义
 * 当前版本：见本文VER_STR
 * 作    者：岑坚宇
 * 完成日期：2010年1月
 * 备    注：
 *********************************************************************************************************/
#include "stdafx.h"
#include "FaCfg.h"
#include "sysarch.h" 
#include "stdio.h"
#include "CommIf.h"
#include "FaConst.h"
#include "ComAPI.h"
#include "drivers.h"
#include "Trace.h"
#include "ProIfAPI.h"

#define VER_STR	"Ver1.6.f_nuc"

void InitProIf()
{
	DTRACE(DB_CRITICAL, ("InitProIf: "VER_STR" init ok.\r\n"));
}

bool SetSockLed(bool fLight)
{
	CComm comGSM;
	char* p;
	WORD i;
	bool fRet = false;
	BYTE bBuf[128];
	
	if (comGSM.Open(19, CBR_115200, 8, ONESTOPBIT, NOPARITY) == false)
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::SetSocketLed : fail to open GSM mux comm\r\n"));
		return false;
	}	
	
	char* pszLight = "AT$MYSOCKETLED=1\r\n";
	char* pszDark = "AT$MYSOCKETLED=0\r\n";
	if (fLight)
		p = pszLight;
	else
		p = pszDark;

	
	for (i=0; i<2; i++)
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::SetSocketLed : tx %s\r\n", p));
		comGSM.Write((BYTE* )p, strlen(p));
		Sleep(500);
		int len = comGSM.Read(bBuf, sizeof(bBuf));
		if (len > 0)
		{
			bBuf[len] = 0;	
			DTRACE(DB_FAPROTO, ("CSocketIf::SetSocketLed : rx %s\r\n", bBuf));
			
			if (strstr((char* )bBuf, "OK") != NULL)   //接收到正确回答
			{
				fRet = true;
				break;
			}		
		}
		else
		{
			DTRACE(DB_FAPROTO, ("CSocketIf::SetSocketLed : rx no ans\r\n", bBuf));
		}
		
		Sleep(500);
	}
/*	
#if 1
	p = "AT+CNSMOD?\r\n";
	if(fLight)
	{
		for (i=0; i<2; i++)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf::GetCNMOD : tx %s\r\n", p));
			comGSM.Write((BYTE* )p, strlen(p));
			Sleep(500);
			int len = comGSM.Read(bBuf, sizeof(bBuf));
			if (len > 0)
			{
				bBuf[len] = 0;	
				DTRACE(DB_FAPROTO, ("CSocketIf::GetCNMOD : rx %s\r\n", bBuf));
				
				if (strstr((char* )bBuf, "OK") != NULL)   //接收到正确回答
				{
					fRet = true;
					break;
				}		
			}
			else
			{
				DTRACE(DB_FAPROTO, ("CSocketIf::GetCNMOD : rx no ans\r\n", bBuf));
			}
			
			Sleep(500);
		}
	}
	
	p = "AT+CNSMOD=1\r\n";
	if(fLight)
	{
		for (i=0; i<2; i++)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf::Set Auto Rpt CNMOD : tx %s\r\n", p));
			comGSM.Write((BYTE* )p, strlen(p));
			Sleep(500);
			int len = comGSM.Read(bBuf, sizeof(bBuf));
			if (len > 0)
			{
				bBuf[len] = 0;	
				DTRACE(DB_FAPROTO, ("CSocketIf::Set Auto Rpt CNMOD : rx %s\r\n", bBuf));
				
				if (strstr((char* )bBuf, "OK") != NULL)   //接收到正确回答
				{
					fRet = true;
					break;
				}		
			}
			else
			{
				DTRACE(DB_FAPROTO, ("CSocketIf::Set Auto Rpt CNMOD : rx no ans\r\n", bBuf));
			}
			
			Sleep(500);
		}
	}
#endif
*/
	comGSM.Close();
	
	return fRet;
	//Sleep(1000);
}
