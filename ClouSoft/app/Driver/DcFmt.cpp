#include "stdafx.h"
#include "DrvStruct.h"
#include "DcStruct.h"
#include "DrvConst.h"
#include "ComAPI.h"
#include "DcFmt.h"
#include "DbAPI.h"

#define DC_ONE_PN	false	//仅入库到测量点0
#define DC_DUO_PN	true	//支持两个测量点,入库到测量点0和配置的另外一个测量点

void ValToDcFmt1(int val, BYTE* pbBuf, WORD wLen);
void ValToDcFmt2(int val, BYTE* pbBuf, WORD wLen);

TDcValToDbCtrl g_DcValToDbCtrl[] = 
{
//		PN	 BANK	ID	 内部计算的索引		子ID个数, 长度,格式转换函数
	{DC_ONE_PN, BN0, 0x2011, DC_VAL_CLKBAT, 			1, 		3, 	ValToDcFmt1},
	{DC_ONE_PN, BN0, 0x2012, DC_VAL_GPRSBAT, 			1, 		3, 	ValToDcFmt1},
	{DC_ONE_PN, BN2, 0x1046, DC_VAL_INNDC1,  			1, 		4, 	ValToDcFmt2},
	{DC_ONE_PN, BN2, 0x1047, DC_VAL_INNDC2,  			1, 		4, 	ValToDcFmt2},
	{DC_ONE_PN, BN2, 0x2041, DC_VAL_OUTDC1,  			1, 		4, 	ValToDcFmt2},
	{DC_ONE_PN, BN2, 0x2042, DC_VAL_OUTDC2,  			1, 		4, 	ValToDcFmt2},
	{DC_ONE_PN, BN2, 0x2043, DC_VAL_OUTDC3,  			1, 		4, 	ValToDcFmt2},
	{DC_ONE_PN, BN2, 0x2044, DC_VAL_OUTDC4,  			1, 		4, 	ValToDcFmt2},
	{DC_ONE_PN, BN2, 0x2045, DC_VAL_OUTDC5,  			1, 		4, 	ValToDcFmt2},
	{DC_ONE_PN, BN2, 0x2046, DC_VAL_OUTDC6,  			1, 		4, 	ValToDcFmt2},
	{DC_ONE_PN, BN2, 0x2047, DC_VAL_OUTDC7,  			1, 		4, 	ValToDcFmt2},
	{DC_ONE_PN, BN2, 0x2048, DC_VAL_OUTDC8,  			1, 		4, 	ValToDcFmt2},
};


void ValToDcFmt1(int val, BYTE* pbBuf, WORD wLen)
{
	if (val > 9999)
		val = 9999;
	else if (val < -9999)
		val = -9999;
		
	if (val < 0)
		val = -val;
	
//	DWORDToBCD((DWORD )val, pbBuf, 2);
//		memcpy(pbBuf, (BYTE*)&val, 2);
	pbBuf[0] = DT_LONG_U;	//格式double-long
	OoWordToLongUnsigned(val, &pbBuf[1]);
}

void ValToDcFmt2(int val, BYTE* pbBuf, WORD wLen)
{
	if (val > 99999999)
		val = 99999999;
	else if (val < -99999999)
		val = -99999999;
		
	if (val < 0)
		val = -val;
	
//	DWORDToBCD((DWORD )val, pbBuf, 4);
	memcpy(pbBuf, (BYTE*)&val, 4);
}

#define DC_VAL2DB_NUM sizeof(g_DcValToDbCtrl)/sizeof(TDcValToDbCtrl)

bool InitDcValToDb(WORD wPn)
{
	for (WORD i=0; i<DC_VAL2DB_NUM; i++)
	{
		//不管是否配置了交采的测量点,都会写到测量点0
		g_DcValToDbCtrl[i].diPn0 = GetItemEx(g_DcValToDbCtrl[i].wBn, 
									   	  	 PN0,
									   	  	 g_DcValToDbCtrl[i].wID);
		
		if (g_DcValToDbCtrl[i].fDuoPn  && wPn!=PN0)
		{ //该数据项支持写入到两个测量点,否则只写入到PN0 && 测量点配置了
			g_DcValToDbCtrl[i].diPn = GetItemEx(g_DcValToDbCtrl[i].wBn, 
									  	  	 	wPn,
									   	  	 	g_DcValToDbCtrl[i].wID);
		}
		else
		{
			memset(&g_DcValToDbCtrl[i].diPn, 0, sizeof(g_DcValToDbCtrl[i].diPn));
		}
	}	
	return true;
}

bool DcValToDb(int* piVal)
{
	BYTE* p;
	BYTE bBuf[64];
	
	for (WORD i=0; i<DC_VAL2DB_NUM; i++)
	{
		p = bBuf;
		for (WORD j=0; j<g_DcValToDbCtrl[i].wSubNum; j++)
		{
			g_DcValToDbCtrl[i].pfnDcValToFmt(piVal[g_DcValToDbCtrl[i].wIdx+j], 
											 p, g_DcValToDbCtrl[i].wLen);
			p += g_DcValToDbCtrl[i].wLen;
		}
		WriteItem(g_DcValToDbCtrl[i].diPn0, bBuf);
		if (g_DcValToDbCtrl[i].diPn.pbAddr != NULL)
			WriteItem(g_DcValToDbCtrl[i].diPn, bBuf);	
	}		
	return true;
}
