#include "stdafx.h"
#include "ComAPI.h"
#include "Info.h"
#include "DbAPI.h"
#include "DCProc.h"

typedef void (* TPfnDcValToFmt)(int val, BYTE* pbBuf, WORD wLen);
typedef struct{
	WORD wBn;  		//BANK号
	WORD wID;     	//数据项ID,
	WORD wIdx;		//内部计算的索引
	WORD wSubNum;	//子ID的个数
	WORD wLen;		//单个数据项的长度
	TPfnDcValToFmt pfnDcValToFmt;	//格式转换函数
	TDataItem diPn[DC_CHN_MAX];	//DC_CHN_MAX
}TDcValToFinalDbCtrl;	//直流模拟数据入库控制

typedef struct{
    WORD wPn;
    BYTE bPortNo;
    BYTE bAtr;	
}TDcSampleDesc;

TDcValToFinalDbCtrl g_DcValToFinalDbCtrl[] = 
{
	{BN0, 0x12cf, 	0, 			1, 		2, 	Val32ToBin/*ValToFmt2*/,}
};

//STADJPARA g_stAdjPara = {1, 1, 1, 1};
BYTE g_bAdjPortNo = 0;
BYTE g_bMaxDcChn;	//有效的直流模拟量路数

#define DC_VAL2DB_NUM 	sizeof(g_DcValToFinalDbCtrl)/sizeof(TDcValToFinalDbCtrl)
TDcSampleDesc g_DcSampleDesc[DC_CHN_MAX];

int CDCProc::Init()
{
    BYTE bBuf[60];
	//获取直流模拟量的各路配置参数;
	memset(bBuf, 0, sizeof(bBuf));
	memset(g_DcSampleDesc, 0, sizeof(g_DcSampleDesc));
	memset(m_dDcParaKp, 0, sizeof(m_dDcParaKp));
	memset(m_dDcParaB, 0, sizeof(m_dDcParaB));
			
	BYTE bPortNo = 0;	//直流模拟量的输入端口号；
	BYTE bPn = 0;		//直流模拟量的测量点号；
	BYTE bPnIndex = 0;	//直流模拟量测量点序号
	BYTE bAtr=0;
	g_bMaxDcChn = 0;
	for (BYTE bNum=1; bNum<PN_NUM; bNum++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BN0, bNum, 0x8904, bBuf);
					
		bPortNo = bBuf[1];
		bPn = bBuf[0]; 
		bAtr = bBuf[2];
		if (bPortNo <1 || bPortNo>8 || bPn <1 || bPn >64)//直流模拟量端口号有效值（1～8），测量点号有效值(1~64)
		{
//		    DTRACE(DB_CRITICAL, ("InitDcSample : bPorNo = %d, bPn = %d, InValid!\n", bPortNo, bPn));	
			continue;
		}	
		g_DcSampleDesc[bPnIndex].wPn = bPn;
		g_DcSampleDesc[bPnIndex].bPortNo = bPortNo;
		g_DcSampleDesc[bPnIndex].bAtr = bAtr;
		DTRACE(DB_CRITICAL, ("InitDcSample : bPnIndex =%d, InitDcSample : bPorNo = %d, bPn = %d,bAtr= %d. \n", bPnIndex, bPortNo, bPn,bAtr));	
		InitDcValToDb(bPnIndex);
		
		ReadItemEx(BN0, bNum, 0x051f, bBuf);
		int iK1 = Fmt2ToVal(bBuf, 2);
		int iK2 = Fmt2ToVal(bBuf+2, 2);
		DTRACE(DB_CRITICAL, ("InitDcSample : InitDcSample : iK1 = %d, iK2 = %d\n", iK1, iK2));	
		if (g_DcSampleDesc[bPnIndex].bAtr > 2)//电流
		{
			//m_iDcParaKp[bPnIndex] = (iK2 - iK1)/16;
			//m_iDcParaB[bPnIndex] = (5*iK1 - iK2)/4*1000;	
			if (iK1==0 || iK2==0)
			{
				m_dDcParaKp[bPnIndex] = 1;
				m_dDcParaB[bPnIndex] = 0;
			}
			else
			{
				m_dDcParaKp[bPnIndex] = (double)(iK2 - iK1)/16000;
				m_dDcParaB[bPnIndex] = (5*iK1 - iK2)/4;
			}
		}
		else//电压
		{
			//m_iDcParaKp[bPnIndex] = (iK2 - iK1)/5;
			//m_iDcParaB[bPnIndex] = iK1*1000;	
			if (iK1==0 || iK2==0)
			{
				m_dDcParaKp[bPnIndex] = 1;
				m_dDcParaB[bPnIndex] = 0;
			}
			else
			{	
				m_dDcParaKp[bPnIndex] = (iK2 - iK1)/5000;
				m_dDcParaB[bPnIndex] = iK1;
			}
		}
		DTRACE(DB_CRITICAL, ("InitDcSample : InitDcSample : m_dDcParaKp=%f, m_dDcParaB=%f\n", m_dDcParaKp[bPnIndex], m_dDcParaB[bPnIndex]));
		bPnIndex++;
		if (bPnIndex >= DC_CHN_MAX)
			break;
	}
	
	g_bMaxDcChn = bPnIndex;
	//DTRACE(DB_CRITICAL, ("InitDcSample : g_bMaxDcChn = %d\n", g_bMaxDcChn));
		
	return 0;
}

WORD CDCProc::GetDcSamplePn(BYTE bPnIndex)
{
    if (bPnIndex < DC_CHN_MAX)
		return g_DcSampleDesc[bPnIndex].wPn;
	else
		return 0xff;
}

bool CDCProc::InitDcValToDb(BYTE bPnIndex)
{
    WORD wPn = GetDcSamplePn(bPnIndex);
    if (wPn == 0xff)
    	return false;
	for (WORD i=0; i<DC_VAL2DB_NUM; i++)
	{
		g_DcValToFinalDbCtrl[i].diPn[bPnIndex] = GetItemEx(g_DcValToFinalDbCtrl[i].wBn, wPn, g_DcValToFinalDbCtrl[i].wID);
	}
	return true;	
}

//直流模拟量数据入库
void CDCProc::ValToDb(BYTE bPnIndex, int iVal)
{
	WORD i;
	BYTE bBuf[60];
	BYTE* p = bBuf;

	for (i=0; i<DC_VAL2DB_NUM; i++)
	{
		g_DcValToFinalDbCtrl[i].pfnDcValToFmt(iVal, p, g_DcValToFinalDbCtrl[i].wLen);
		if (g_DcValToFinalDbCtrl[i].diPn[bPnIndex].pbAddr != NULL)
			WriteItem(g_DcValToFinalDbCtrl[i].diPn[bPnIndex], bBuf);
	}
}

WORD g_wDcDLMSVal[2] = {0};
int CDCProc::DoDCProc()
{
	WORD i;
	int64 iVal, iDbVal;
	BYTE bBuf[16];
	static DWORD dwLastClick=0;
	if (GetInfo(INFO_DC_SAMPLE))	//配置参数8904更改
	{
		DTRACE(DB_CRITICAL, ("DoDcSample : Re InitDcSample!\n"));
		Init();
	}

	//if (g_bAdjPortNo)
	//	AdjustPara();

	if (g_bMaxDcChn <= 0 && GetClick()-dwLastClick < 2)	//执行间隔5s钟
	{
		//DTRACE(DB_CRITICAL, ("DoDcSample : g_bMaxDcChn = %d!\n",g_bMaxDcChn));
		return 0;
	}
	if (GetClick()-dwLastClick < 1)	//执行间隔1s钟
		return 0;
	dwLastClick = GetClick();
	for (i=0; i<g_bMaxDcChn; i++)
	{
		if (g_DcSampleDesc[i].bPortNo == 1)	//第1路直流量为电流
		{
			ReadItemEx(BN2, PN0, 0x1047, bBuf);
			/*iVal = BcdToDWORD(bBuf, 4);
			if (g_DcSampleDesc[i].bAtr > 2)
			{
				if (g_stAdjPara.iAdjPort1Data != g_stAdjPara.iAdjPort1ExtData)
					iDbVal = (iVal - g_stAdjPara.iAdjPort1Data)*10000/(g_stAdjPara.iAdjPort1ExtData - g_stAdjPara.iAdjPort1Data) + 5000;
				else
					iDbVal = 0;
			}
			else
			{
				if (g_stAdjPara.iAdjPort1Data != g_stAdjPara.iAdjPort1ExtData)
					iDbVal = (iVal - g_stAdjPara.iAdjPort1Data)*2000/(g_stAdjPara.iAdjPort1ExtData - g_stAdjPara.iAdjPort1Data) + 2000;
				else
					iDbVal = 0;			
			}*/
			iDbVal = BcdToDWORD(bBuf, 4);
		}
		else if (g_DcSampleDesc[i].bPortNo == 2)	//第2路直流量为电压
		{
			ReadItemEx(BN2, PN0, 0x1046, bBuf);
			/*iVal = BcdToDWORD(bBuf, 4);
			if (g_DcSampleDesc[i].bAtr > 2)
			{
				if (g_stAdjPara.iAdjPort2Data != g_stAdjPara.iAdjPort2ExtData)
					iDbVal = (iVal - g_stAdjPara.iAdjPort2Data)*10000/(g_stAdjPara.iAdjPort2ExtData - g_stAdjPara.iAdjPort2Data) + 5000;
				else
					iDbVal = 0;
			}
			else
			{
				if (g_stAdjPara.iAdjPort2Data != g_stAdjPara.iAdjPort2ExtData)
					iDbVal = (iVal - g_stAdjPara.iAdjPort2Data)*2000/(g_stAdjPara.iAdjPort2ExtData - g_stAdjPara.iAdjPort2Data) + 2000;
				else
					iDbVal = 0;			
			}*/
			iDbVal = BcdToDWORD(bBuf, 4);
		}
		else
			iDbVal = 0;
		//y = kp*x + b;
		iDbVal = (double)(m_dDcParaKp[i]*iDbVal) + m_dDcParaB[i] + 0.5;
		ValToDb(i, (int)iDbVal);
		g_wDcDLMSVal[i] = iDbVal;
		/*
		if (dwLastClick % 60 == 0)
			DTRACE(DB_CRITICAL, ("DoDcSample : Dc Sample Val = %lld bPortNo = %d!\n",iDbVal,g_DcSampleDesc[i].bPortNo));
	    */
	}
	memcpy(bBuf, (BYTE*)&g_wDcDLMSVal[0], 2);
	memcpy(&bBuf[2], (BYTE*)&g_wDcDLMSVal[1], 2);
	WriteItemEx(BN0, PN0, 0x371f, bBuf);

	return 0;
}
