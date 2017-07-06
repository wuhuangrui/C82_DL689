/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcHook.cpp
 * 摘    要：本文件主要用来定义交采库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年2月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *			 $本文件定义的函数,形式一致,但是在做不同版本的产品时,函数可能需要修改
 *********************************************************************************************************/
#include "AcHook.h"
#include "DbAPI.h"
//#include "Sample.h"
#include "AcSample.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "DbOIAPI.h"

//描述:交采库在发生需量清理的回调函数,一般用来生成需量清理记录
//	   提供两个互为备份的测量点,对不等于0xffff的测量点进行处理
//参数:@wPn0 交采的测量点0,如果不等于0xffff则进行处理
//	   @wPn1 交采的测量点1,如果不等于0xffff则进行处理
void AcOnClrDemand(WORD wPn0, WORD wPn1)
{
	
}

//描述:在交采执行日冻结时的回调函数,只提供交采测量点,默认测量点由本函数决定
//	   需不需要也同时执行
//参数:@wPn 交采的测量点,如果支持双测量点,则wPn指的应该是非默认测量点,
//			但在外部没有配置交采测量点的时候,wPn也可能被赋为默认测量点(如PN0)
//			在支持双测量点的情况下,如果函数判断到wPn为非默认测量点,在执行
//			wPn的相应操作的同时,也应该为默认测量点执行相应操作
//	   @time 日冻结的时标,只使用年、月、日、时几个字段	
void AcOnDayFrz(WORD wPn, const TTime& time)
{
	DWORD dwTmp = DaysFrom2000(time);
	WriteItemEx(BN18, wPn, 0x0610, (BYTE *)&dwTmp);	//最后一次日冻结时间
	TrigerSaveBank(BN18, 0, -1); 
}

//描述:交采库用来判断某个日冻结是否已经冻结的回调函数,只提供交采测量点,默认测量点由本函数决定
//	   需不需要也同时执行
//参数:@wPn 交采的测量点,如果支持双测量点,则wPn指的应该是非默认测量点,
//			但在外部没有配置交采测量点的时候,wPn也可能被赋为默认测量点(如PN0)
//			在支持双测量点的情况下,如果函数判断到wPn为非默认测量点,在执行
//			wPn的相应操作的同时,也应该为默认测量点执行相应操作
//	   @time 日冻结的时标,只使用年、月、日、时几个字段	
bool AcIsDayFrozen(WORD wPn, const TTime& time)
{
	DWORD dwTmp;
	ReadItemEx(BN18, wPn, 0x0610, (BYTE *)&dwTmp);	//最后一次日冻结时间
	return (dwTmp == DaysFrom2000(time));
}

//描述:在交采执行月冻结时的回调函数,只提供交采测量点,默认测量点由本函数决定
//	   需不需要也同时执行
//参数:@wPn 交采的测量点,如果支持双测量点,则wPn指的应该是非默认测量点,
//			但在外部没有配置交采测量点的时候,wPn也可能被赋为默认测量点(如PN0)
//			在支持双测量点的情况下,如果函数判断到wPn为非默认测量点,在执行
//			wPn的相应操作的同时,也应该为默认测量点执行相应操作
//	   @time 日冻结的时标,只使用年、月、日、时几个字段	
void AcOnMonthFrz(WORD wPn, const TTime& time, BYTE bIdx)
{
	DWORD dwTmp = MonthFrom2000(time);
	WriteItemEx(BN18, wPn, 0x0611+bIdx, (BYTE *)&dwTmp); 	//最后一次月冻结时间
	TrigerSaveBank(BN18, 0, -1); 
}

//描述:交采库用来判断某个月冻结是否已经冻结的回调函数,只提供交采测量点,默认测量点由本函数决定
//	   需不需要也同时执行
//参数:@wPn 交采的测量点,如果支持双测量点,则wPn指的应该是非默认测量点,
//			但在外部没有配置交采测量点的时候,wPn也可能被赋为默认测量点(如PN0)
//			在支持双测量点的情况下,如果函数判断到wPn为非默认测量点,在执行
//			wPn的相应操作的同时,也应该为默认测量点执行相应操作
//	   @time 日冻结的时标,只使用年、月、日、时几个字段	
bool AcIsMonthFrozen(WORD wPn, const TTime& time, BYTE bIdx)
{
	DWORD dwTmp;
	ReadItemEx(BN18, wPn, 0x0611+bIdx, (BYTE *)&dwTmp);	//最后一次月冻结时间
	return (dwTmp == MonthFrom2000(time));
}

//描述:交采库用来触发保存测量点数据的回调函数,只提供交采测量点,默认测量点由本函数决定
//	   需不需要也同时执行
//参数:@wPn 交采的测量点,如果支持双测量点,则wPn指的应该是非默认测量点,
//			但在外部没有配置交采测量点的时候,wPn也可能被赋为默认测量点(如PN0)
//			在支持双测量点的情况下,如果函数判断到wPn为非默认测量点,在执行
//			wPn的相应操作的同时,也应该为默认测量点执行相应操作
//	   @time 日冻结的时标,只使用年、月、日、时几个字段	
void AcTrigerSavePn(WORD wPn)
{
	if (wPn == PN0)
	{
		TrigerSaveBank(BN0, SECT_ENERGY, PN0);	//触发保存
		TrigerSaveBank(BN0, SECT_DEMAND, PN0);	//触发保存
		TrigerSaveBank(BN0, SECT_VARIABLE, PN0);	//触发保存
	}
	else
	{
		TrigerSaveBank(BN0, SECT_ENERGY, PN0);	//触发保存
		TrigerSaveBank(BN0, SECT_DEMAND, PN0);	//触发保存
		TrigerSaveBank(BN0, SECT_VARIABLE, PN0);	//触发保存
		TrigerSaveBank(BN0, SECT_ENERGY, wPn);	//触发保存
		TrigerSaveBank(BN0, SECT_DEMAND, wPn);	//触发保存
		TrigerSaveBank(BN0, SECT_VARIABLE, wPn);	//触发保存
	}
}


//描述:交采库用来触发保存测量点数据的回调函数,提供两个互为备份的测量点,对不等于0xffff
//	   的测量点进行触发保存
//参数:@wPn0 交采的测量点0,如果不等于0xffff则进行触发保存
//	   @wPn1 交采的测量点1,如果不等于0xffff则进行触发保存
void AcTrigerSavePn(WORD wPn0, WORD wPn1)
{
	if (wPn0 != 0xffff)
	{
		TrigerSaveBank(BN0, SECT_ENERGY, wPn0);	//触发保存
		TrigerSaveBank(BN0, SECT_DEMAND, wPn0);	//触发保存
		TrigerSaveBank(BN0, SECT_VARIABLE, wPn0);	//触发保存
	}
	
	if (wPn1 != 0xffff)
	{
		TrigerSaveBank(BN0, SECT_ENERGY, wPn1);	//触发保存
		TrigerSaveBank(BN0, SECT_DEMAND, wPn1);	//触发保存
		TrigerSaveBank(BN0, SECT_VARIABLE, wPn1);	//触发保存
	}
}


//参数:@bType =0则进行电压判断, =1则进行电流判断
bool CaluDisOrder34(const int* piAngle)
{	//3相4线电压和电流逆相序
	DWORD iAngle[3];
	iAngle[0] = (piAngle[0] + 3600 - piAngle[1]) % 3600;
	iAngle[1] = (piAngle[1] + 3600 - piAngle[2]) % 3600;
	iAngle[2] = (piAngle[2] + 3600 - piAngle[0]) % 3600;
	
    if ((iAngle[0]>=2400-50 && iAngle[0]<=2400+50) ||  
    	(iAngle[1]>=2400-50 && iAngle[1]<=2400+50) ||  
    	(iAngle[2]>=2400-50 && iAngle[2]<=2400+50))
	{
    	return true;
	}
	else
	{
		return false;
	}
}


bool CaluDisOrderU33(const int* piAngle)
{
	int iAngle  = (short)(piAngle[0] + 3600 - piAngle[2]) % 3600;
   	if ((iAngle>550 && iAngle<650) || iAngle>3580 || iAngle<50)   //逆相序的时候为60度
   		return true;
	else
		return false;
}


bool CaluDisOrderI33(const int* piAngle)
{
	int iAngle  = (short)(piAngle[0] + 3600 - piAngle[2]) % 3600;
   	if (iAngle>1200-50 && iAngle<1200+50)  //fAngle<120+20 || fAngle>120-20
   		return true;
	else
		return false;
}


BYTE CalCurPolar(const int* piAngle, BYTE bConnectType, BYTE bLastPolar)
{
	const WORD wAngleMin34[3] = {900, 900, 900}; 
	const WORD wAngleMax34[3] = {2700, 2700, 2700};
	const WORD wAngleMin33[3] = {1200, 2700, 600}; 
	const WORD wAngleMax33[3] = {3000, 2700, 2400};
	const WORD* pwAngleMin;
	const WORD* pwAngleMax;
	
	BYTE bPolar = bLastPolar;
   	
   	if (bConnectType == CONNECT_3P4W)
   	{
		pwAngleMin = wAngleMin34;
		pwAngleMax = wAngleMax34;
   	}
   	else
   	{
   		//三相三线的判断依据是:  90+45 < Uab-Ia < 270-45
   		//						 90+15 < Ucb-Ic < 180+15
		pwAngleMin = wAngleMin33;
		pwAngleMax = wAngleMax33;
   	}
   	
	for (WORD i=0; i<3; i++)
	{
		int iAngle = (piAngle[i]  + 3600 - piAngle[i+3]) % 3600;  //电压减电流相角
  		if (iAngle>pwAngleMin[i] && iAngle<pwAngleMax[i])	
		{	
   			bPolar |= 0x01<<i;
		}
		else if (iAngle<=pwAngleMin[i]-1 || iAngle>=pwAngleMax[i]+1)	//差一度才置恢复
		{
			bPolar &= ~(0x01<<i);
		}
	}
   	
   	if (bConnectType == CONNECT_3P3W)
   	{
   		bPolar &= ~0x02;
   	}
   	return bPolar;
}


//描述:计算与相序有关的异常,
//参数:@pbPhaseStatus 用来传递原来的值,并且返回新的值
//备注:相序状态如下
//	   D7	D6	D5			D4			D3			D2			D1			D0
//						Ic反极性	Ib反极性	Ia反极性	电流逆相序	电压逆相序	
void AcCaluPhaseStatus(const TAcPara& rAcPara, const int* piValue, const int* piAngle, const int* piCos, BYTE* pbPhaseStatus, WORD* pwPnStatus)
{
	DWORD dwNormU = rAcPara.dwUn * 7 / 10;
	DWORD dwNormI = rAcPara.dwIn * 5 / 100;
	bool fDisOrderU = (*pbPhaseStatus & DISORDER_U) != 0;
	bool fDisOrderI = (*pbPhaseStatus & DISORDER_I) != 0; 
	BYTE bCurPolar = (*pbPhaseStatus>>2) & 0x07;
	
 	if (rAcPara.bConnectType == CONNECT_3P4W)
  	{
		if (piValue[0]>dwNormU && piValue[1]>dwNormU && piValue[2]>dwNormU)  //电流的判断只有在电压合格时才判断
		{
			fDisOrderU = CaluDisOrder34(piAngle);
			if (fDisOrderU)
				goto end;
			
			if (piValue[3]>dwNormI && piValue[4]>dwNormI && piValue[5]>dwNormI)   //电流的判断
			{
				fDisOrderI = CaluDisOrder34(&piAngle[3]);
				if (fDisOrderI == false)
					bCurPolar = CalCurPolar(piAngle, rAcPara.bConnectType, bCurPolar);
				else
					bCurPolar = 0;
			}
		}
		//else 维持原来的判断,不要复位原来的判断结果,避免因为电压电流的波动引起反复地上报
  	}
  	else if (rAcPara.bConnectType == CONNECT_3P3W)
    { 
    	//三相三线
    	if (piValue[0]>dwNormU && piValue[2]>dwNormU)  //电流的判断只有在电压合格时才判断
    	{
			fDisOrderU = CaluDisOrderU33(piAngle);
			if (fDisOrderU)
				goto end;
			
			if (piValue[3]>dwNormI && piValue[5]>dwNormI)   //电流的判断
			{
				fDisOrderI = CaluDisOrderI33(&piAngle[3]);
				if (fDisOrderI == false)
					bCurPolar = CalCurPolar(piAngle, rAcPara.bConnectType, bCurPolar);
				else
					bCurPolar = 0;
			}
    	}
    	//else 维持原来的判断,不要复位原来的判断结果,避免因为电压电流的波动引起反复地上报
    }
    
end:
	if (fDisOrderU)
    {
    	*pbPhaseStatus = DISORDER_U; //在发生电压逆相序的时候,电流逆相序和反极性标志位都清除
    }
    else
    {	
    	*pbPhaseStatus &= ~DISORDER_U;
    	
		if (fDisOrderI)
	    	*pbPhaseStatus |= DISORDER_I; 
	    else
	    	*pbPhaseStatus &= ~DISORDER_I;
	    	
	    *pbPhaseStatus = (*pbPhaseStatus & 0x03) | (bCurPolar << 2); //如果发生了电流逆相序,bCurPolar在上面已经清零
	}
	
	WriteItemEx(BANK2, POINT0, 0x1120, pbPhaseStatus); //电压电流逆向序
}

//描述:电压电流不平衡的计算方法, |UImax - UImin| / UImax
//参数:@rAcPara 用来传递计量参数
//		@piValue 用来传递电流数据
//备注:
static WORD CalculateImbalance(const TAcPara& rAcPara, const int* piValue)
{
	DWORD	dwValueA, dwValueB, dwValueC, dwMax, dwMin;
	WORD wImbalance;

	// 不平衡 ，电流可能有符号
	dwValueA = labs(piValue[0]);
	dwValueB = labs(piValue[1]);
	dwValueC = labs(piValue[2]);
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance dwValueA =%d ", dwValueA));
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance dwValueB =%d ", dwValueB));
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance dwValueC =%d ", dwValueC));
	
	if (dwValueA > dwValueC)
	{
		dwMax = dwValueA;	dwMin = dwValueC;
	}
	else
	{
		dwMax = dwValueC;	dwMin = dwValueA;
	}
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance dwMax =%d ", dwMax));
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance dwMin =%d ", dwMin));
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance rAcPara.bConnectType =%d ", rAcPara.bConnectType));
	if (rAcPara.bConnectType == CONNECT_3P4W)		// 三相四线才比较B相
	{
		if (dwMax < dwValueB)		dwMax = dwValueB;
		if (dwMin > dwValueB)		dwMin = dwValueB;
	}
//		DTRACE(DB_CRITICAL,("\r\n####CalculateImbalance dwMax 1 =%d ", dwMax));
	if(dwMax == 0)
	{
		wImbalance = 0;
	}
	else
	{
		wImbalance = (WORD)((float)(dwMax - dwMin) / dwMax * 10000);
		if (wImbalance > 9999)
			wImbalance = 9999;
	}
	return wImbalance;
}



//描述:电压电流不平衡的计算方法, |UImax - UImin| / UImax
//参数:@rAcPara 用来传递计量参数
//		@piValue 用来传递电流数据
//备注:
void AcCaluImbalanceRate(const TAcPara& rAcPara, const int* piValue)
{
	WORD wImbalance;
	BYTE bBuf[10];
	
	// 2026 6 电压不平衡率
	// 数据类型：long-unsigned，单位：%，换算：-2
	wImbalance = CalculateImbalance(rAcPara,&piValue[0]);
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate piValue[0] =%d ", piValue[0]));
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate piValue[1] =%d ", piValue[1]));
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate piValue[2] =%d ", piValue[2]));
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate U_Imbalance =%d ", wImbalance));
	bBuf[0] = DT_LONG_U;
	OoWordToLongUnsigned(wImbalance, &bBuf[1]);
	OoWriteAttr(0x2026, 0x02, bBuf);
//		TraceBuf(DB_CRITICAL, "\r\n####AcCaluImbalanceRate-> ", bBuf, 3); 		

	// 2027 6 电流不平衡率
	// 数据类型：long-unsigned，单位：%，换算：-2
	wImbalance = CalculateImbalance(rAcPara,&piValue[3]);
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate piValue[3] =%d ", piValue[3]));
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate piValue[4] =%d ", piValue[4]));
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate piValue[5] =%d ", piValue[5]));
//		DTRACE(DB_CRITICAL,("\r\n####AcCaluImbalanceRate I_Imbalance =%d ", wImbalance));

	bBuf[0] = DT_LONG_U;
	OoWordToLongUnsigned(wImbalance, &bBuf[1]);
	OoWriteAttr(0x2027, 0x02, bBuf);
//		TraceBuf(DB_CRITICAL, "\r\n####AcCaluImbalanceRate-> ", bBuf, 3); 		
	
}

//描述:转存当前需量
//参数:@wId 用来传递需量ID
//		@piValue 用来传递带格式需量数据
//备注:
void AcTransSaveCurDemand(WORD wId, BYTE* pbValue)
{
	switch(wId)
	{
		case 0x3117:
		OoWriteAttr(0x2017, 0x02, pbValue);
		break;
		case 0x3118:
		OoWriteAttr(0x2018, 0x02, pbValue);
		break;
		default:
			break;
	}	
}





//描述:计算安时数
//参数:@rAcPara 用来传递计量参数
//		@piValue 用来传递电流数据
//		@piValue 用来传递操作，0:清零重新累计，其他值为累加
//备注:
void AcCaluAmpereHours(const TAcPara& rAcPara, const int* piValue, BYTE bOpt)
{
	int iAHs[4];
	BYTE bBuf[30];
	int i;
	
	memset(iAHs, 0, sizeof(iAHs));
	// 2029 6 安培小时数
	// 数据类型：double-long-unsigned，单位：Ah，换算：-2
	if(bOpt!=0)
	{
		OoReadVal(0x20290200, iAHs, 4);
		for(i=0;i<4;i++)
		{
			iAHs[i] *= 10;
			DTRACE(DB_AC,("\r\n####AcCaluAmpereHours 0 dwAHs[%d] =%d ", i, iAHs[i]));
		}
		for(i=0;i<3;i++)
		{
			iAHs[0] += labs(piValue[i]);
			iAHs[i+1] += labs(piValue[i]);
			DTRACE(DB_AC,("\r\n####AcCaluAmpereHours 1 dwAHs[0] =%d,dwAHs[%d] =%d, piValue[%d]=%d ", iAHs[0], i, iAHs[i], i, piValue[i]));
		}
		
	}
	for(i=0;i<4;i++)
	{
		if ((i==1) && (rAcPara.dwConst == CONNECT_3P3W))		// 三相三线不计算B相
		{
			iAHs[2] = 0;
			continue;
		}
		iAHs[i] /= 10;
		DTRACE(DB_AC,("\r\n####AcCaluAmpereHours 2 dwAHs[%d] =%d ", i, iAHs[i]));
	}
	
	//入库
	bBuf[0] = DT_ARRAY;
	bBuf[1] = 4;
	for(i=0;i<4;i++)
	{
		bBuf[2+i*5] = DT_DB_LONG_U;
		OoDWordToDoubleLongUnsigned(iAHs[i],&bBuf[3+i*5]);
	}
	OoWriteAttr(0x2029, 0x02, bBuf);
	TraceBuf(DB_AC, "\r\n####AcCaluAmpereHours-> ", bBuf, 22); 	
}





//描述:设定可编程脉冲输出类型
//参数:无
void PgmPulseInit()
{
}   

//描述:启动可编程脉冲输出
//参数:@wType 脉冲类型,为需量周期、时段投切
void PgmPulseOut(WORD wType)
{
}

