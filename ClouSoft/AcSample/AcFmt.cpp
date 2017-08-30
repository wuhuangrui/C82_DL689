/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcFmt.cpp
 * 摘    要：本文件主要实现交采数据项的值到格式的转换接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2008年5月
 * 备    注: 本文件主要用来屏蔽各版本间参数的差异性
 *********************************************************************************************************/
#include "Sample.h"
#include "AcSample.h"
#include "FaAPI.h"
#include "ComAPI.h"
#include "AcFmt.h"
#include "Pulse.h"
#include "OoFmt.h"

#define ONE_PN	false	//仅入库到测量点0
#define DUO_PN	true	//支持两个测量点,入库到测量点0和配置的另外一个测量点

void AcAngToFmt(int val, BYTE* pbBuf, WORD wLen);
void AcValIToFmt(int val, BYTE* pbBuf, WORD wLen);
void AcValToFmtCpy(int* piVal, BYTE* pbBuf, WORD wLen);
void AcUIFToFmt(int iVal, BYTE* pbBuf, WORD wLen);
void AcToFmt2(int iVal, BYTE* pbBuf, WORD wLen);//3个字节的
void AcPQToLFmt(int iVal, BYTE* pbBuf, WORD wLen);


void VoltValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
void CurrValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
void ZeroLineCurrValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
void PowerValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
void CosValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
void AngValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
void FValToDb(int* piVal, BYTE* pbBuf, WORD wLen);
void DemandValToDb(int* piVal, BYTE* pbBuf, WORD wLen);

TAcValToDbCtrl g_AcValToDbCtrl[] = 
{
//		PN	 BANK	ID	 内部计算的索引		子ID个数, 长度,格式转换函数
	{DUO_PN, BN0, 0x2000, AC_VAL_UA, 			1, 			11, 	VoltValToDb},
	{DUO_PN, BN0, 0x2001, AC_VAL_IA, 			1, 			22, 	CurrValToDb},//A/B/C电流
	{DUO_PN, BN0, 0x2610, AC_VAL_IA, 			1, 			5, 		ZeroLineCurrValToDb},//零线电流单独处理
	{DUO_PN, BN0, 0x2002, AC_VAL_ANG_UA,  		1, 			11, 	AngValToDb},	 //电压角度
	{DUO_PN, BN0, 0x2003, AC_VAL_ANG_IA,  		1, 			11, 	AngValToDb},	 //电流角度 要不要加零序电流?
	{DUO_PN, BN0, 0x2004, AC_VAL_P,  			1, 			22, 	PowerValToDb},
	{DUO_PN, BN0, 0x2005, AC_VAL_Q,  			1, 			22, 	PowerValToDb},
	{DUO_PN, BN0, 0x2006, AC_VAL_S,  			1, 			22, 	PowerValToDb},	 //视在功率
//		{DUO_PN, BN0, 0x2007, AC_VAL_AVG_P, 		1,			22, 	PowerValToDb},	 //一分钟平均有功功率
//		{DUO_PN, BN0, 0x2008, AC_VAL_AVG_Q, 		1,			22, 	PowerValToDb},   //一分钟平均无功功率
//		{DUO_PN, BN0, 0x2009, AC_VAL_AVG_S, 		1,			22, 	PowerValToDb},	 //一分钟平均视在功率
	{DUO_PN, BN0, 0x200a, AC_VAL_COS,  			1, 			14, 	CosValToDb},
	{ONE_PN, BN0, 0x200f, AC_VAL_F,  			1, 			2, 		FValToDb}, 		 //频率
//		{DUO_PN, BN0, 0x2017, AC_VAL_DEMAND_P,  	1, 			7, 		DemandValToDb},	 //当前有功需量
//		{DUO_PN, BN0, 0x2018, AC_VAL_DEMAND_Q,  	1, 			7, 		DemandValToDb},	 //当前无功需量

	
	{ONE_PN, BN2, 0x1120, AC_VAL_PHASESTATUS, 1, 	1, 	AcValToFmtCpy},	//相序状态
//	{ONE_PN, BN2, 0x1121, AC_VAL_MTRSTATUS,  	1, 	1, 	AcValToFmtCpy},		//电表状态字
};



#define AC_VAL2DB_NUM sizeof(g_AcValToDbCtrl)/sizeof(TAcValToDbCtrl)

TDataItem g_diHarPercent;
TDataItem g_diHarVal;
TDataItem g_diP0HarPercent;
TDataItem g_diP0HarVal;
TDataItem g_diToTalHarPercent;
TDataItem g_diToTalHarVal;

//	TDataItem g_diVoltDistortion;	//电压波形失真度
//	TDataItem g_diCurDistortion;	//电流波形失真度
//	TDataItem g_diVoltHarPercent[3];	//电压谐波含量 总、2-19次
//	TDataItem g_diCurHarPercent[3];	//电流谐波含量 总、2-19次
//	TDataItem g_diHarmonicNum;	//谐波次数





//交采电压入库
void VoltValToDb(int* piVal, BYTE* pbBuf, WORD wLen)
{
		pbBuf[0] = 0x01;
		pbBuf[1] = 0x03;
		for (BYTE b=0; b<3; b++)
		{
				pbBuf[2+b*3] = DT_LONG_U;	//格式long-unsigned
//					pbBuf[3+b*3] = (BYTE )piVal[b];	//保存到数据库的是自然格式，低位在前，高位在后
//					pbBuf[4+b*3] = (BYTE )(piVal[b]>>8);
				OoWordToLongUnsigned(piVal[b],&pbBuf[3+b*3]);

		}				
}

void CurrValToDb(int* piVal, BYTE* pbBuf, WORD wLen)
{
		pbBuf[0] = 0x01;
		pbBuf[1] = 0x03;
		for (BYTE b=0; b<3; b++)
		{
			pbBuf[2+b*5] = DT_DB_LONG;	//格式double-long
//				memcpy(&pbBuf[3+b*5], (BYTE *)&piVal[b], 4);
			OoIntToDoubleLong(piVal[b],&pbBuf[3+b*5]);
		}
}

void ZeroLineCurrValToDb(int* piVal, BYTE* pbBuf, WORD wLen)
{
		pbBuf[0] = DT_DB_LONG;	//格式double-long
		OoIntToDoubleLong(piVal[3],&pbBuf[1]);
}

void PowerValToDb(int* piVal, BYTE* pbBuf, WORD wLen)
{
		pbBuf[0] = 0x01;
		pbBuf[1] = 0x04;
		for (BYTE b=0; b<4; b++)
		{
				pbBuf[2+b*5] = DT_DB_LONG;	//格式double-long
//				revcpy(&pbBuf[3+b*5], (BYTE *)&piVal[b], 4);
//					memcpy(&pbBuf[3+b*5], (BYTE *)&piVal[b], 4);
			OoIntToDoubleLong(piVal[b],&pbBuf[3+b*5]);
		}
}

void DemandValToDb(int* piVal, BYTE* pbBuf, WORD wLen)
{
	pbBuf[0] = 0x01;
	pbBuf[1] = 0x01;
	pbBuf[2] = DT_DB_LONG;	//格式double-long
//		memcpy(&pbBuf[3], (BYTE *)&piVal[0], 4);
	OoIntToDoubleLong(piVal[0],&pbBuf[3]);
}



//总功率入库
void GeneralPowerValToDb(int* piVal, BYTE* pbBuf, WORD wLen)
{	
	pbBuf[0] = DT_DB_LONG;	//格式double-long
	OoIntToDoubleLong(piVal[0], &pbBuf[1]);
}

void CosValToDb(int* piVal, BYTE* pbBuf, WORD wLen)
{
		pbBuf[0] = 0x01;
		pbBuf[1] = 0x04;
		for (BYTE b=0; b<4; b++)
		{
				pbBuf[2+b*3] = DT_LONG;	//格式long
//				revcpy(&pbBuf[3+b*3], (BYTE *)&piVal[b], 2);
//					memcpy(&pbBuf[3+b*3], (BYTE *)&piVal[b], 2);
			OoWordToLongUnsigned(piVal[b],&pbBuf[3+b*3]);
		}
}
void AngValToDb(int* piVal, BYTE* pbBuf, WORD wLen)
{
		pbBuf[0] = 0x01;
		pbBuf[1] = 0x03;
		for (BYTE b=0; b<3; b++)
		{
				pbBuf[2+b*3] = DT_LONG_U;	//格式long-unsigned
//				revcpy(&pbBuf[3+b*3], (BYTE *)&piVal[b], 2);
//					memcpy(&pbBuf[3+b*3], (BYTE *)&piVal[b], 2);
			OoWordToLongUnsigned(piVal[b],&pbBuf[3+b*3]);
		}
}

void FValToDb(int* piVal, BYTE* pbBuf, WORD wLen)
{
	pbBuf[0] = 0x01;
	pbBuf[1] = 0x01;
	pbBuf[2] = DT_LONG_U;
//		revcpy(&pbBuf[1], (BYTE *)&piVal[0], 2);
//			memcpy(&pbBuf[1], (BYTE *)&piVal[0], 2);
	OoWordToLongUnsigned(piVal[0],&pbBuf[3]);
}

//描述：因为相序状态和电表状态字是写在扩展参数的，所以不用加格式和反序
void AcValToFmtCpy(int* piVal, BYTE* pbBuf, WORD wLen)
{
	memcpy(pbBuf, (BYTE*)piVal, wLen);
}


void AcAngToFmt(int val, BYTE* pbBuf, WORD wLen)
{
	val = val*10;	//863协议相角有两位小数
	memcpy(pbBuf, (BYTE*)&(val), 2);	
}

void AcValIToFmt(int val, BYTE* pbBuf, WORD wLen)
{
//	ValToFmt6((val+5)/10, pbBuf, wLen);
	val = (val+5)/10;
	memcpy(pbBuf, (BYTE*)&(val), wLen);
}

void AcUIFToFmt(int iVal, BYTE* pbBuf, WORD wLen)
{
	DWORD dwVal = (ABS(iVal)+5)/10;
	if (dwVal > 9999)
		dwVal = 9999;

//	DWORDToBCD(dwVal, pbBuf, 2);
	memcpy(pbBuf, (BYTE*)&dwVal, 2);
}

void AcToFmt2(int iVal, BYTE* pbBuf, WORD wLen)
{
	DWORD dwVal = ABS(iVal);
	if (dwVal > 999999)
		dwVal = 999999;

//	DWORDToBCD(dwVal, pbBuf, 3);
	memcpy(pbBuf, (BYTE*)&dwVal, 4);
}

void AcPQToLFmt(int iVal, BYTE* pbBuf, WORD wLen)
{
//	IntToBCD(iVal, pbBuf, 4);   //总
	memcpy(pbBuf, (BYTE*)&iVal, 4);
}

//描述:初始化交采数据入库的控制结构
//参数:@wPn 交采配置的测量点,不管是否配置了交采的测量点,都会写到测量点0
bool InitAcValToDb(WORD wPn)
{
	WORD i;
	for (i=0; i<AC_VAL2DB_NUM; i++)
	{
		//不管是否配置了交采的测量点,都会写到测量点0
		g_AcValToDbCtrl[i].diPn0 = GetItemEx(g_AcValToDbCtrl[i].wBn, 
									   	  	 PN0,
									   	  	 g_AcValToDbCtrl[i].wID);
		
		if (g_AcValToDbCtrl[i].fDuoPn  && wPn!=PN0)
		{ //该数据项支持写入到两个测量点,否则只写入到PN0 && 测量点配置了
			g_AcValToDbCtrl[i].diPn = GetItemEx(g_AcValToDbCtrl[i].wBn, 
									  	  	 	wPn,
									   	  	 	g_AcValToDbCtrl[i].wID);
		}
		else
		{
			memset(&g_AcValToDbCtrl[i].diPn, 0, sizeof(g_AcValToDbCtrl[i].diPn));
		}
	}
	
//		g_diHarPercent = GetItemEx(BN0, wPn, 0x128f); //当前A、B、C三相电压、电流2~N次谐波含有率
//		g_diHarVal = GetItemEx(BN0, wPn, 0x127f); //当前A、B、C三相电压、电流2~N次谐波有效值
//		if (wPn != PN0)
//		g_diVoltDistortion = GetItemEx(BN0, PN0, 0x200b);
//		g_diCurDistortion = GetItemEx(BN0, PN0, 0x200c);
//		
//		for(i=0;i<3;i++)
//		{
//			g_diVoltHarPercent[i] = GetItemEx(BN0, PN0, 0x2600+i);
//			g_diCurHarPercent[i] = GetItemEx(BN0, PN0, 0x2603+i);
//		}
//	
//		g_diHarmonicNum = GetItemEx(BN0, PN0, 0x2606);


//		{
//			g_diP0HarPercent = GetItemEx(BN0, PN0, 0x200d); //当前A、B、C三相电压、电流2~N次谐波含有率
//			g_diP0HarVal = GetItemEx(BN0, PN0, 0x260d); //当前A、B、C三相电压、电流2~N次谐波有效值
//		}
//		
//		g_diToTalHarPercent = GetItemEx(BN2, PN0, 0x200f); //总谐波含有率
//		g_diToTalHarVal = GetItemEx(BN2, PN0, 0x201f);	   //总谐波有效值

	return true;	
}


//描述:交采数据入库到数据库
//参数:@piVal 交采程序算好的采集量的值
void AcValToDb(int* piVal)
{
	BYTE* p;
	BYTE bBuf[64];
	
	for (WORD i=0; i<AC_VAL2DB_NUM; i++)
	{
		p = bBuf;
		for (WORD j=0; j<g_AcValToDbCtrl[i].wSubNum; j++)
		{
			g_AcValToDbCtrl[i].pfnAcValToFmt(&piVal[g_AcValToDbCtrl[i].wIdx+j], 
											 p, g_AcValToDbCtrl[i].wLen);
			p += g_AcValToDbCtrl[i].wLen;
		}
		WriteItemEx(BN0, PN0, g_AcValToDbCtrl[i].wID, bBuf);
//		WriteItem(g_AcValToDbCtrl[i].diPn0, bBuf);
//		if (g_AcValToDbCtrl[i].diPn.pbAddr != NULL)
//			WriteItem(g_AcValToDbCtrl[i].diPn, bBuf);
	}	
}


//描述:谐波值的入库
//参数:@pwHarPercent 谐波含有率，总及2~19次谐波含有率（）
//	   @pwHarVal 谐波有效值，总及2~19次谐波有效值
void AcHarmonicToDb(WORD* pwHarPercent, WORD* pwHarVal)
{
//		DWORD val, dwVal;
	BYTE* p;
	WORD i, j;
//		WORD* pwHarPercent0 = pwHarPercent;
//		WORD* pwHarVal0 = pwHarVal;
//		BYTE bToTalBuf[16], bTmpBuf[4];
	BYTE bBuf[(HARMONIC_NUM-1)*SCN_NUM*2+1+64]; //64为预留



	//电压波形失真度/总谐波畸变率
	memset(bBuf, 0x00, sizeof(bBuf));
	bBuf[0] = 0x01;
	bBuf[1] = 0x03;
	for(i=0;i<3;i++)
	{
		bBuf[2+i*3] = DT_LONG;
//			memcpy(&bBuf[3+i*3], (BYTE *)&pwHarPercent[HARMONIC_NUM*i], 2);
		OoInt16ToLong(pwHarPercent[(HARMONIC_NUM-1)*i],&bBuf[3+i*3]);
	}	
//		WriteItem(g_diVoltDistortion, bBuf);
	WriteItemEx(BN0, PN0, 0x200b, bBuf);
	
	//电流波形失真度/总谐波畸变率
	memset(bBuf, 0x00, sizeof(bBuf));
	bBuf[0] = 0x01;
	bBuf[1] = 0x03;
	for(i=0;i<3;i++)
	{
		bBuf[2+i*3] = DT_LONG;
//			memcpy(&bBuf[3+i*3], (BYTE *)&pwHarPercent[HARMONIC_NUM*i+3*HARMONIC_NUM], 2);
		OoInt16ToLong(pwHarPercent[(HARMONIC_NUM-1)*i+3*(HARMONIC_NUM-1)],&bBuf[3+i*3]);
	}
//		WriteItem(g_diCurDistortion, bBuf);
	WriteItemEx(BN0, PN0, 0x200c, bBuf);


	//电压总/2-N次谐波含量
	for(i=0;i<3;i++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = 0x01;
		bBuf[1] = (HARMONIC_NUM-1);
		for(j=0;j<(HARMONIC_NUM-1);j++)
		{
			bBuf[2+j*3] = DT_LONG;
//				memcpy(&bBuf[3+j*3], (BYTE *)&pwHarPercent[HARMONIC_NUM*i+j], 2);
			OoInt16ToLong(pwHarPercent[(HARMONIC_NUM-1)*i+j],&bBuf[3+j*3]);
		}
//			WriteItem(g_diVoltHarPercent[i], bBuf);
		WriteItemEx(BN0, PN0, 0x2600+i, bBuf);
	}	
	
	//电流总/2-N次谐波含量
	for(i=0;i<3;i++)
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = 0x01;
		bBuf[1] = (HARMONIC_NUM-1);
		for(j=0;j<(HARMONIC_NUM-1);j++)
		{
			bBuf[2+j*3] = DT_LONG;
//				memcpy(&bBuf[3+j*3], (BYTE *)&pwHarPercent[HARMONIC_NUM*i+j+3*HARMONIC_NUM], 2);
			OoInt16ToLong(pwHarPercent[(HARMONIC_NUM-1)*i+j+3*(HARMONIC_NUM-1)],&bBuf[3+j*3]);
		}
//			WriteItem(g_diCurHarPercent[i], bBuf);
		WriteItemEx(BN0, PN0, 0x2603+i, bBuf);
	}	

	//谐波次数
	memset(bBuf, 0x00, sizeof(bBuf));
	bBuf[0] = 0x01;
	bBuf[1] = 0x01;
	bBuf[2] = DT_UNSIGN;
	bBuf[3] = HARMONIC_NUM;
//		WriteItem(g_diHarmonicNum, bBuf);
	WriteItemEx(BN0, PN0, 0x2606, bBuf);

	
//		//07表的谐波含有率
//	//	WORD wPercent[1+6*HARMONIC_NUM];
//	//	wPercent[0] = HARMONIC_NUM;
//	//	memcpy(&wPercent[1], pwHarPercent, 6*HARMONIC_NUM);
//		p = bBuf;
//		memset(bBuf, 0x00, sizeof(bBuf));
//		*p++ = HARMONIC_NUM;
//		memcpy(p, (BYTE*)&pwHarPercent[0], HARMONIC_NUM*SCN_NUM*2);
//		if (g_diHarPercent.pbAddr != NULL)
//			WriteItem(g_diHarPercent, bBuf);	
//		if (g_diP0HarPercent.pbAddr != NULL)
//			WriteItem(g_diP0HarPercent, bBuf);	
//		
//		//谐波有效值
//		p = bBuf;
//		*p++ = HARMONIC_NUM;
//		memcpy(p, (BYTE*)&pwHarVal[0], HARMONIC_NUM*SCN_NUM*2);
//		if (g_diHarVal.pbAddr != NULL)
//			WriteItem(g_diHarVal, bBuf);	
//		if (g_diP0HarVal.pbAddr != NULL)
//			WriteItem(g_diP0HarVal, bBuf);	
//	/*
//		for (i=0; i<SCN_NUM; i++)
//		{
//			for (j=0; j<HARMONIC_NUM; j++) //2~N
//			{
//				memcpy(p, (BYTE*)&val, 2);		//直接给HEX就可以了
//				p += 2;
//			}
//		}
//		
//		if (g_diHarVal.pbAddr != NULL)
//			WriteItem(g_diHarVal, bBuf);
//			
//		if (g_diToTalHarVal.pbAddr != NULL)
//			WriteItem(g_diToTalHarVal, bToTalBuf);
//	*/
}

WORD AcEpToFmt(int64 val, BYTE* pbBuf, bool fHigPre, bool fSign)
{
	val = ABS(val);
	if (fHigPre)
	{
		if (fSign)	//带符号的
			pbBuf[0] = DT_LONG64;
		else
			pbBuf[0] = DT_LONG64_U;
			
//			memcpy(&pbBuf[1], (BYTE*)&val, 8);
		OoInt64ToLong64(val,&pbBuf[1]);
		return 9;//4;				
	}
	else
	{
		val /= 100;	//降低精度，保留两位小数即可
		if (fSign)
			pbBuf[0] = DT_DB_LONG;
		else
			pbBuf[0] = DT_DB_LONG_U;
			
//			memcpy(&pbBuf[1], (BYTE*)&val, 4);
		OoDWordToDoubleLongUnsigned(val,&pbBuf[1]);
		return 5;//4;		
	}
}

WORD AcFmtToEp(BYTE* pbBuf, int64* piVal, bool fHigPre, bool fSign)
{
	if (fHigPre)
	{
//			if (fSign)
//				pbBuf[0] = 14;
//			else
//				pbBuf[0] = 15;
		
//			memcpy((BYTE *)piVal, &pbBuf[1], 8);
		*piVal = OoLong64ToInt64(&pbBuf[1]);
		return 9;//4;
	}
	else
	{
//			if (fSign)
//				pbBuf[0] = 0x05;
//			else
//				pbBuf[0] = 0x06;
//			memcpy((BYTE *)piVal, &pbBuf[1], 8);
		*piVal = OoDoubleLongUnsignedToDWord(&pbBuf[1]);
		return 5;//9;//4;
	}
}

bool IsAcEngSign(WORD wOI)
{
	if ((wOI>=0x0030 && wOI<=0x0043) || (wOI>=0x0630 && wOI<=0x0643) 
		|| (wOI==0x0001) || (wOI==0x0601))
		return true;
	else
		return false;
}

BYTE AcEngToFmt(WORD wID, int64 *pi64E, BYTE* pbBuf, bool fHigPre, bool fSign, WORD wRateNum)
{
	BYTE *p = pbBuf;
	BYTE bSign = 0;
	WORD wLen;
	
	*p++ = 0x01;
	*p++ = wRateNum;
	for (WORD j=0; j<wRateNum; j++)
	{
		if (IsAcEngSign(wID))
			wLen = AcEpToFmt(pi64E[j], p, fHigPre, true);
		else
			wLen = AcEpToFmt(pi64E[j], p, fHigPre, false);
	
		if (fSign && pi64E[j]<0)	//本数据块支持符号 && 符号位为负
			bSign |= (1<<j);
	
		p += wLen;
	}		
	
	return bSign;
}

BYTE AcFmtToEng(WORD wID, int64 *pi64E, BYTE* pbBuf, bool fHigPre, bool fSign, BYTE bSign, WORD wRateNum)
{
	BYTE *p = pbBuf;
	WORD wLen;


	p += 2; //所有的电量数据，前面有01,05的标记
	for (WORD j=0; j<wRateNum; j++)
	{
//			if (m_pEnergyPara->fEp[i])
		if (IsAcEngSign(wID))	
			wLen = AcFmtToEp(p, &pi64E[j], fHigPre, true); //这里转出来的都是正数
		else
			wLen = AcFmtToEp(p, &pi64E[j], fHigPre, false);	//这里转出来的都是正数

		if (fSign && (bSign & (1<<j)))  //本数据块支持符号 && 符号位为负
			pi64E[j] = -pi64E[j];

		//DTRACE(DB_CRITICAL, ("j=%d, i64E[j]=%lld.\r\n",  j, pi64E[j]));
		
		p += wLen;
	}	
	
	return bSign;
}


//从高精度ID(8字节，4位小数位)数据库读出数据到脉冲电量
BYTE PulseHiFmtToLoEng(WORD wID, int64 *pi64E, BYTE* pbBuf, WORD wRateNum)
{
	BYTE *p = pbBuf;
	WORD wLen;

	p += 2; //所有的电量数据，前面有01,05的标记
	for (WORD j=0; j<wRateNum; j++)
	{	
		pi64E[j] = OoLong64ToInt64(&p[1]);
		p += 9;
	}

	return (p-pbBuf);
}


//从脉冲ID(4字节，4位小数位)数据库高精度ID读出脉冲电量
BYTE PulseFmtToEng(WORD wID, int64 *pi64E, BYTE* pbBuf, WORD wRateNum)
{
	BYTE *p = pbBuf;
	WORD wLen;

	p += 2; //所有的电量数据，前面有01,05的标记
	for (WORD j=0; j<wRateNum; j++)
	{	
		pi64E[j] = OoDoubleLongUnsignedToDWord(&p[1]);
		p += 5;
	}

	return (p-pbBuf);
}




//脉冲电量入库
//脉冲电量入库格式DT_DB_LONG（4字节, 4位小数位）
BYTE PulseEngToFmt(WORD wID, int64 *pi64E, BYTE* pbBuf, WORD wRateNum)
{
	DWORD dwVal;
	BYTE *p = pbBuf;
	
	*p++ = 0x01;
	*p++ = wRateNum;
	for (WORD j=0; j<wRateNum; j++)
	{
		p[0] = DT_DB_LONG_U;
		
		dwVal = abs(pi64E[j]);
		OoDWordToDoubleLongUnsigned(dwVal, &p[1]);

		p += 5;
	}

	return (p-pbBuf);
}


bool IsDemFmt5(WORD wOI)
{
	if ((wOI>=0x1030 && wOI<=0x1043) || (wOI>=0x1130 && wOI<=0x1143) 
		|| (wOI>=0x2017 && wOI<=0x2019) || (wOI>=0x3117 && wOI<=0x3119) 
		|| (wOI>=0x3030 && wOI<=0x3043))
		return true;
	else
		return false;		
}

WORD AcDemandAndTimeToFmt(DWORD dwDemand, BYTE* pbTime, BYTE* pbBuf, bool fFmt5)
{
	*pbBuf++ = DT_STRUCT;
	*pbBuf++ = 0x02;
	
	if (fFmt5)
		*pbBuf++ = DT_DB_LONG;
	else
		*pbBuf++ = DT_DB_LONG_U;
		
//		memcpy(pbBuf, (BYTE *)&dwDemand, 4);
	OoIntToDoubleLong(dwDemand,pbBuf);
	pbBuf += 4;
	
	*pbBuf = DT_DATE_TIME_S;//发生时间DateTimeBCD_S
	pbBuf++;
	memcpy(pbBuf, pbTime, 7);//数据入库前已经转换过，所以直接入库	
//		OoIntToDoubleLong(dwDemand,pbBuf);
	
	return 15;
}


WORD AcDemandToFmt(DWORD *pdwDemand, BYTE* pbTime, BYTE* pbBuf, WORD wDemandID, WORD wRateNum)
{
	WORD wLen;
	BYTE *p = pbBuf;
	
	*p++ = 0x01;	//格式01, array
	*p++ = 0x05;	//数量5，总,费率1,2,3,4

	for (WORD j=0; j<wRateNum; j++)
	{
		wLen = AcDemandAndTimeToFmt(pdwDemand[j], &pbTime[j*7], p, IsDemFmt5(wDemandID));
		p += wLen;
	}
	
	return (p-pbBuf);
}


WORD AcCurDemToFmt(DWORD dwDemand, BYTE* pbBuf, bool fFmt5)
{
//		*pbBuf++ = DT_ARRAY;
//		*pbBuf++ = 0x01;
	
	if (fFmt5)
		*pbBuf++ = DT_DB_LONG;
	else
		*pbBuf++ = DT_DB_LONG_U;
		
	OoIntToDoubleLong(dwDemand,pbBuf);
	
	return 5;
}


WORD AcCurDemandToFmt(DWORD dwDemand, BYTE* pbBuf, WORD wDemandID)
{//当前需量不做分费率
	WORD wLen;
	BYTE *p = pbBuf;
	
//		*p++ = 0x01;	//格式01, array
//		*p++ = 0x01;	//数量5，总,费率1,2,3,4

	wLen = AcCurDemToFmt(dwDemand, p, IsDemFmt5(wDemandID));
	p += wLen;
	
	return (p-pbBuf);
}



//时间格式：月，日，时，分
WORD AcDemTimeToFmt(BYTE* pSrcBuf, BYTE* pDstBuf)
{
	//面向对象协议	:  BIN:0-1YEAR 2 MONTH 3DAY 4HOUR 5MINUTE 6SECOND
	bool fTimeValid = true;
	WORD wYear;
	wYear = pSrcBuf[0];
	wYear <<= 8;
	wYear |= pSrcBuf[1];
	
	pDstBuf[0] = DT_DATE_TIME_S;
	
	if	((wYear>2099) || 
		 (pSrcBuf[2]>12) || 
		 (pSrcBuf[3]>31) || (pSrcBuf[4]>=24) ||
		 (pSrcBuf[5]>=60) || (pSrcBuf[6]>=60) )
		fTimeValid = false;
	
		
	if (!fTimeValid)
	{
//			BYTE bInValidDemTime[DATETIMELEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0xff};
//			revcpy(pDstBuf, bInValidDemTime, DATETIMELEN);
		memcpy(pDstBuf+1, 0, 7);
	}
	else
	{
//			TTime now;
//			GetCurTime(&now);
//		
//			pDstBuf[0] = 0x00;			//时钟状态
//			pDstBuf[1] = 0x00;			//偏差
//			pDstBuf[2] = 0x80;
//			pDstBuf[3] = 0x00;						//1%100秒
//			pDstBuf[4] = now.nSecond;				//秒
//			pDstBuf[5] = BcdToByte(pSrcBuf[0]);		//分
//			pDstBuf[6] = BcdToByte(pSrcBuf[1]);		//时
//			pDstBuf[7] = now.nWeek;					//周
//			pDstBuf[8] = BcdToByte(pSrcBuf[2]);		//日
//			pDstBuf[9] = BcdToByte(pSrcBuf[3]);		//月
//			pDstBuf[10] = BYTE((now.nYear)&0xff);		//年低字节
//			pDstBuf[11] = BYTE(((now.nYear)>>8)&0xff);	//年高字节
		// 面向对象协议直接拷贝时间即可
		memcpy(pDstBuf+1, pSrcBuf, 7);
	}
	DTRACE(DB_CRITICAL, (" \r\n AcDemTimeToFmt, Time:%d-%d-%d %d:%d:%d!! \r\n", wYear,pSrcBuf[2],pSrcBuf[3],pSrcBuf[4],pSrcBuf[5],pSrcBuf[6]));		
	
	return 8;
}

//时间格式：月，日，时，分
WORD AcDemandTimeToFmt(BYTE* pSrcBuf, BYTE* pDstBuf, WORD wRateNum, BYTE bDemTimeLen)
{
	WORD wLen;
	BYTE* p = pSrcBuf;
	
	for (WORD j=0; j<wRateNum; j++)
	{
		wLen = AcDemTimeToFmt(&pSrcBuf[j*bDemTimeLen], pDstBuf);
		p += wLen;
	}

	return p-pSrcBuf;
}






WORD AcFmtToDemand(BYTE* pbBuf, DWORD* pdwDemand, BYTE* pbTime)
{
//	*pbBuf++;	//格式01, array
//	*pbBuf++;	//数量5，总,费率1,2,3,4
	pbBuf += 3;	//数据格式02 02 05
//		*pdwDemand = pbBuf[0]+(pbBuf[1]<<8)+(pbBuf[2]<<16)+(pbBuf[3]<<24);
	*pdwDemand = OoDoubleLongToInt(pbBuf);
	pbBuf += 4;
	pbBuf++;//发生时间DateTimeBCD_S
	memcpy(pbTime, pbBuf, 7);	
	return 15;
}

WORD AcFmtToDemandAndTime(BYTE* pbBuf, DWORD* pdwDemand, BYTE* pbTime,WORD wRateNum)
{
	WORD wLen;
	BYTE* p = pbBuf;
	
	p++;	//0x01
	p++;	//0x05
	for (WORD j=0; j<wRateNum; j++) //格式02 02 06 ** ** ** ** 1E ** ** ** ** ** **
	{ 
		wLen = AcFmtToDemand(p, &pdwDemand[j], &pbTime[j*7]);
		p += wLen;
	}

	return p-pbBuf;
}


WORD AcFmtToDemandTime(BYTE* pbBuf, BYTE* pbTime,WORD wRateNum,BYTE bDemTimeLen)
{
	WORD wLen;
	BYTE* p = pbBuf;
	
	
	for (WORD j=0; j<wRateNum; j++)
	{ 
		memcpy(&pbTime[j*bDemTimeLen], p, bDemTimeLen);
		p += bDemTimeLen;
	}

	return p-pbBuf;
}


bool AcFmtToDemandTime(BYTE* pbTime, TTime *pTime )
{
	return OoDateTimeSToTime(pbTime,pTime);
}

void AcTTimeToDemandTime(BYTE* pbTime, TTime *pTime )
{
	pbTime[0] = (pTime->nYear>>8) & 0xff;
	pbTime[1] = pTime->nYear & 0xff;
	pbTime[2] = pTime->nMonth;
	pbTime[3] = pTime->nDay;
	pbTime[4] = pTime->nHour;
	pbTime[5] = pTime->nMinute;
	pbTime[6] = pTime->nSecond;
	return ;
}

BYTE AcDemandTimeGetMonth(BYTE* pbTime)
{
	//BcdToByte(pbTime[3]);
	return pbTime[2];
}


TPulseValToDbCtrl g_PulseValToDbCtrl[] = 
{
	//BANK	ID	 内部计算的索引		  子ID个数, ID长度,	格式转换函数
	{BN0, 0x2404, PULSE_VAL_P,  		1,		 7,		GeneralPowerValToDb,},
	{BN0, 0x2405, PULSE_VAL_Q,  		1,		 7,		GeneralPowerValToDb,},
};

#define PULSE_VAL2DB_NUM sizeof(g_PulseValToDbCtrl)/sizeof(TPulseValToDbCtrl)

//描述:初始化脉冲数据入库的控制结构
//参数:@wPn 脉冲配置的测量点
bool InitPulseValToDb(BYTE bPnIndex)
{
    WORD wPn = g_PulseManager.GetPulsePn(bPnIndex);
	for (WORD i=0; i<PULSE_VAL2DB_NUM; i++)
	{	 //该数据项支持写入到两个测量点,否则只写入到PN0 && 测量点配置了
		g_PulseValToDbCtrl[i].diPn[bPnIndex] = GetItemEx(g_PulseValToDbCtrl[i].wBn, 
								  	  	 	wPn,
								   	  	 	g_PulseValToDbCtrl[i].wID);
	}
	
	return true;	
}


//描述:脉冲数据入库到数据库
//参数:@piVal 交采程序算好的采集量的值
void PulseValToDb(BYTE bPnIndex, int* piVal)
{
	BYTE* p;
	BYTE bBuf[64];

	for (WORD i=0; i<PULSE_VAL2DB_NUM; i++)
	{
		p = bBuf;
		for (WORD j=0; j<g_PulseValToDbCtrl[i].wSubNum; j++)
		{
			g_PulseValToDbCtrl[i].pfnAcValToFmt(&piVal[g_PulseValToDbCtrl[i].wIdx+j], 
											 p, g_PulseValToDbCtrl[i].wLen);
			p += g_PulseValToDbCtrl[i].wLen;
		}

		if (g_PulseValToDbCtrl[i].diPn[bPnIndex].pbAddr != NULL)
			WriteItem(g_PulseValToDbCtrl[i].diPn[bPnIndex], bBuf);
	}
}


WORD AcIDToRatenum(WORD wID, WORD wRateNum)
{
//		return ( (wID&0x000f)==0x000f ? RATE_NUM+1 : 1 )
	return (wRateNum+1);//oob 不判断 id，645-97才要用到
}




void AcEngMonitor(BYTE* pSrc,WORD wRateNum, WORD wID, 
					int64 *pi64E, int64 *pm_i64E, BYTE *pbTemp,
					int64 i64MaxE,	//电能数据入库能表示的最大值
					int iMaxLoss,	//保存间隔内最大丢失的电能
					bool fSign, BYTE bSign)
{
	BYTE* p = pSrc;
	WORD wLen;
	int64 i64Temp;
	
	p+=2;
	for (WORD j=0; j<wRateNum; j++)
	{
//			if (m_pEnergyPara->fEp[i])
		if (IsAcEngSign(wID))
			wLen = AcFmtToEp(p, &pi64E[j], true, true);	//这里转出来的都是正数
		else
			wLen = AcFmtToEp(p, &pi64E[j], true, false);	//这里转出来的都是正数

		if (fSign && (bSign & (1<<j)))  //本数据块支持符号 && 符号位为负
			pi64E[j] = -pi64E[j];

		//由于外部数据精度可能比较低，防止入库有损失精度引起误判，
		//这里都按照入库后的精度进行判断
		if (IsAcEngSign(wID))//(m_pEnergyPara->fEp[i])
		{
			AcEpToFmt(pm_i64E[j], pbTemp, true, true);
			AcFmtToEp(pbTemp, &i64Temp, true, true);
		}
		else
		{
			AcEpToFmt(pm_i64E[j], pbTemp, true, false);
			AcFmtToEp(pbTemp, &i64Temp, true, false);
		}			

//			DTRACE(DB_CRITICAL, ("iMaxLoss:%d, i64E[i][j]:%lld, m_i64E[i][j]:%lld, i64Temp:%lld, i:%d, j:%d\r\n", 
//								iMaxLoss, i64E[i][j], m_i64E[i][j], i64Temp, i, j));
		
		//判断数据库中的数据和这里成员变量记录的数据不能超过
		//按15分钟的保存间隔,最大可能丢失的电能 	
		if (i64Temp != pi64E[j])
		{
			DTRACE(DB_CRITICAL, ("%s Error, j=%d,i64MaxE=%lld,i64E[i][j]=%lld,m_i64E[i][j]=%lld, i64Temp:%lld, reboot.\r\n", 
			__FUNCTION__, j, i64MaxE, pi64E[j], pm_i64E[j], i64Temp));
			ResetCPU();
			while (1);
		}

		p += wLen;
	}
}
