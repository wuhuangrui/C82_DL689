/*********************************************************************************************************
 * Copyright (c) 2008,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcSample.cpp
 * 摘    要：本文件主要实现对交流采样参数的装载和保存
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2008年5月
 * 备    注: 本文件主要用来屏蔽各版本间参数的差异性
 *********************************************************************************************************/
#include "syscfg.h"
#include "Sample.h"
#include "ComAPI.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "Pulse.h"

const static WORD g_wPulseCurEnergyID[2][MAX_PULSE_TYPE] = 
{
	//入库的数据项ID
	{
		0x2414, 0x2415,	//正有、正无
		0x2416, 0x2417, //反有、反无
	},	//高精度
	
	//对应的内部计算ID
	{
		EP_POS_ABC, EQ_POS_ABC,	//正有、正无
		EP_NEG_ABC, EQ_NEG_ABC, //反有、反无
	},	
};

/*static WORD g_wPulseCurDemandID[2][MAX_PULSE_TYPE] =
{
	//入库的数据项ID
	{
		0xa01f, 0xa11f, //正有、正无最大需量
		0xa02f, 0xa12f, //反有、反无最大需量
	},
	
	//对应的内部计算ID
	{
		EP_POS_ABC, EQ_POS_ABC,	//正有、正无最大需量
		EP_NEG_ABC, EQ_NEG_ABC, //反有、反无最大需量
	}
};

static WORD g_wPulseCurTimeID[MAX_PULSE_TYPE] = 
{
	0xb01f, 0xb11f,  //正有、正无最大需量时间
	0xb02f, 0xb12f,  //反有、反无最大需量时间
};*/

const static WORD g_wAcCurEnergyID[3][AC_ENERGY_NUM] = 
{
	//入库的数据项ID
	{/*
		0x901f, 0x902f, 								//正、反向有功
		0x911f, 0x912f, 								//组合无功1、2
		0x913f, 0x915f, 0x916f, 0x914f, 				//一二三四象限无功
		
		0x9070,0x9071,0x9072,							//A/B/C分相正向有功电能
		0x9080,0x9081,0x9082,							//A/B/C分相反向有功电能
		0x9170,0x9171,0x9172,							//A/B/C分相正向无功电能,容性
		0x9180,0x9181,0x9182,							//A/B/C分相反向无功电能,感性
		
		0x900f,//组合有功
*/
/*		
		0x0010, 0x0020,				//正、反向有功
		0x0030, 0x0040, 			//组合无功1、2
		0x0050, 0x0060, 0x0070, 0x0080, 		//一二三四象限无功
		
		0x0011, 0x0012, 0x0013,							//A/B/C分相正向有功电能
		0x0021, 0x0022, 0x0023,							//A/B/C分相正向有功电能
		0x0031, 0x0032, 0x0033,							//A/B/C分相正向有功电能
		0x0041, 0x0042, 0x0043,							//A/B/C分相正向有功电能
		
		0x0001,//组合有功
	*/
		0x0610, 0x0620,							//正、反向有功
		0x0630, 0x0640, 						//组合无功1、2
		0x0650, 0x0660, 0x0670, 0x0680, 		//一二三四象限无功
		
		0x0611, 0x0612, 0x0613,					//A/B/C分相正向有功电能
		0x0621, 0x0622, 0x0623,					//A/B/C分相反向有功电能
		0x0631, 0x0632, 0x0633,					//A/B/C组合无功1电能
		0x0641, 0x0642, 0x0643,					//A/B/C组合无功2电能
		
		0x0601,									//组合有功	

		0x0631, 0x0632, 0x0633,					//A/B/C分相组合无功1
		0x0641, 0x0642, 0x0643, 				//A/B/C分相组合无功2

		0x0651, 0x0652, 0x0653,					//A/B/C分相一象限无功
		0x0661, 0x0662, 0x0663, 				//A/B/C分相二象限无功
		0x0671, 0x0672, 0x0673, 				//A/B/C分相三象限无功
		0x0681, 0x0682, 0x0683, 				//A/B/C分相四象限无功
//暂时不做视在
//			0x0690,	0x06a0,							//正/反向视在电能
//			0x0691, 0x0692, 0x0693, 				//A/B/C分相正向视在电能
//			0x06a1, 0x06a2, 0x06a3, 				//A/B/C分相反向视在电能
		
	}, 
	
	//对应的内部计算ID
	{
		EP_POS_ABC, EP_NEG_ABC,					//正、方向有功
		EQ_COM_ABC1, EQ_COM_ABC2, 				//总组合无功1、2
		EQ_Q1, EQ_Q2, EQ_Q3, EQ_Q4, 			//一二三四象限无功

		EP_POS_A,EP_POS_B,EP_POS_C,				//A/B/C分相正向有功电能
		EP_NEG_A,EP_NEG_B,EP_NEG_C,				//A/B/C分相反向有功电能

		EQ_POS_A,EQ_POS_B,EQ_POS_C,				//A/B/C分相感性无功电能
		EQ_NEG_A,EQ_NEG_B,EQ_NEG_C,				//A/B/C分相容性无功电能		
		EP_COM_ABC,								//组合有功总
		EQ_COM_A1,EQ_COM_B1,EQ_COM_C1, 			//A/B/C分相组合无功1
		EQ_COM_A2,EQ_COM_B2,EQ_COM_C2,			//A/B/C分相组合无功2

		EQ_Q1_A, EQ_Q1_B, EQ_Q1_C, 				//A/B/C分相一象限无功
		EQ_Q2_A, EQ_Q2_B, EQ_Q2_C,				//A/B/C分相二象限无功
		EQ_Q3_A, EQ_Q3_B, EQ_Q3_C,				//A/B/C分相三象限无功
		EQ_Q4_A, EQ_Q4_B, EQ_Q4_C,				//A/B/C分相四象限无功

//			ES_POS_ABC, ES_NEG_ABC,					//正、方向视在
//			ES_POS_A,ES_POS_B,ES_POS_C, 			//A/B/C分相正向视在电能
//			ES_NEG_A,ES_NEG_B,ES_NEG_C, 			//A/B/C分相反向视在电能

		
	},
	{
		0x0010, 0x0020,									//正、反向有功
		0x0030, 0x0040, 								//组合无功1、2
		0x0050, 0x0060, 0x0070, 0x0080, //一二三四象限无功
		
		0x0011, 0x0012, 0x0013,					//A/B/C分相正向有功电能
		0x0021, 0x0022, 0x0023,					//A/B/C分相正向有功电能
		0x0031, 0x0032, 0x0033,					//A/B/C分相正向有功电能
		0x0041, 0x0042, 0x0043,					//A/B/C分相正向有功电能
		
		0x0001,//组合有功	
		0x0031, 0x0032, 0x0033, 		//A/B/C分相组合无功1
		0x0041, 0x0042, 0x0043, 		//A/B/C分相组合无功2

		0x0051, 0x0052, 0x0053,					//A/B/C分相一象限无功
		0x0061, 0x0062, 0x0063, 				//A/B/C分相二象限无功
		0x0071, 0x0072, 0x0073, 				//A/B/C分相三象限无功
		0x0081, 0x0082, 0x0083, 				//A/B/C分相四象限无功

//			0x0090,	0x00a0,							//正/反向视在电能
//			0x0091, 0x0092, 0x0093, 				//A/B/C分相正向视在电能
//			0x00a1, 0x00a2, 0x00a3, 				//A/B/C分相反向视在电能
	}
};

const static WORD g_wAcCurDemandID[3][AC_DEMAND_NUM] =
{
	//入库的数据项ID
	{
		0x1010, 0x1020, 						//正、反向有功最大需量
		0x1030, 0x1040, 						//组合无功1、2最大需量
		0x1050, 0x1060, 0x1070, 0x1080, 		//一二三四象限无功最大需量
		
		0x1011, 0x1012, 0x1013,					//A/B/C分相正向有功电能
		0x1021, 0x1022, 0x1023,					//A/B/C分相反向有功电能
		0x1031, 0x1032, 0x1033,					//A/B/C组合无功1电能
		0x1041, 0x1042, 0x1043,					//A/B/C组合无功2电能	
		0x1051, 0x1052, 0x1053, 				//A/B/C一象限无功	
		0x1061, 0x1062, 0x1063, 				//A/B/C二象限无功	
		0x1071, 0x1072, 0x1073, 				//A/B/C三象限无功	
		0x1081, 0x1082, 0x1083, 				//A/B/C四象限无功	

//			0x2017, 0x2018,							//绝对值有功无功需量	
//			0x1090,	0x10a0,							//正/反向视在电能
//			0x1091, 0x1092, 0x1093, 				//A/B/C分相正向视在电能
//			0x10a1, 0x10a2, 0x10a3, 				//A/B/C分相反向视在电能

	},
	
	//对应的内部计算ID
	{
		EP_POS_ABC, EP_NEG_ABC,			//正、方向有功
		EQ_COM_ABC1, EQ_COM_ABC2, 		//组合无功1、2最大需量
		EQ_Q1, EQ_Q2, EQ_Q3, EQ_Q4, 	//一二三四象限无功最大需量
		
		EP_POS_A,EP_POS_B,EP_POS_C,		//A/B/C分相正向有功电能
		EP_NEG_A,EP_NEG_B,EP_NEG_C,		//A/B/C分相反向有功电能

		EQ_COM_A1,EQ_COM_B1,EQ_COM_C1,		//A/B/C分相组合无功1电能
		EQ_COM_A2,EQ_COM_B2,EQ_COM_C2,		//A/B/C分相组合无功2电能	
		
		EQ_Q1_A, EQ_Q1_B, EQ_Q1_C,				//A/B/C分相一象限无功
		EQ_Q2_A, EQ_Q2_B, EQ_Q2_C,				//A/B/C分相二象限无功
		EQ_Q3_A, EQ_Q3_B, EQ_Q3_C,				//A/B/C分相三象限无功
		EQ_Q4_A, EQ_Q4_B, EQ_Q4_C,				//A/B/C分相四象限无功
		EP_ABS_ABC,EQ_ABS_ABC,					//绝对值有功无功需量	
//			ES_POS_ABC, ES_NEG_ABC,					//正、方向视在
//			ES_POS_A,ES_POS_B,ES_POS_C, 			//A/B/C分相正向视在电能
//			ES_NEG_A,ES_NEG_B,ES_NEG_C, 			//A/B/C分相反向视在电能
	},


	//以下为当前实时需量入库的数据项ID，不是最大需量,入库BANK2
	{
		0x3010, 0x3020, 						//正、反向有功最大需量
		0x3030, 0x3040, 						//组合无功1、2最大需量
		0x3050, 0x3060, 0x3070, 0x3080, 		//一二三四象限无功最大需量
		
		0x3011, 0x3012, 0x3013,					//A/B/C分相正向有功电能
		0x3021, 0x3022, 0x3023,					//A/B/C分相反向有功电能
		0x3031, 0x3032, 0x3033,					//A/B/C组合无功1电能
		0x3041, 0x3042, 0x3043,					//A/B/C组合无功2电能	
		0x3051, 0x3052, 0x3053, 				//A/B/C一象限无功	
		0x3061, 0x3062, 0x3063, 				//A/B/C二象限无功	
		0x3071, 0x3072, 0x3073, 				//A/B/C三象限无功	
		0x3081, 0x3082, 0x3083, 				//A/B/C四象限无功	

		0x3117, 0x3118,							//绝对值有功无功需量	
//			0x1090,	0x10a0,							//正/反向视在电能
//			0x1091, 0x1092, 0x1093, 				//A/B/C分相正向视在电能
//			0x10a1, 0x10a2, 0x10a3, 				//A/B/C分相反向视在电能

	},
		
};

//要按照1,2,3,4象限的顺序
//	static WORD g_wAcCurTimeID[AC_DEMAND_NUM] = 
//	{
//		0xb01f, 0xb02f, //正、反向有功最大需量时间
//		0xb11f, 0xb12f, //正、反向无功最大需量时间
//		0xb13f, 0xb15f, 0xb16f, 0xb14f, //一二三四象限无功最大需量时间
//	};
const static WORD g_wAcCurTimeID[AC_DEMAND_NUM] = 
{
		0x1010, 0x1020, 						//正、反向有功最大需量
		0x1030, 0x1040, 						//组合无功1、2最大需量
		0x1050, 0x1060, 0x1070, 0x1080, 		//一二三四象限无功最大需量
		
		0x1011, 0x1012, 0x1013,					//A/B/C分相正向有功电能
		0x1021, 0x1022, 0x1023,					//A/B/C分相反向有功电能
		0x1031, 0x1032, 0x1033,					//A/B/C组合无功1电能
		0x1041, 0x1042, 0x1043,					//A/B/C组合无功2电能	
		0x1051, 0x1052, 0x1053, 				//A/B/C一象限无功	
		0x1061, 0x1062, 0x1063, 				//A/B/C二象限无功	
		0x1071, 0x1072, 0x1073, 				//A/B/C三象限无功	
		0x1081, 0x1082, 0x1083, 				//A/B/C四象限无功	

//			0x2017, 0x2018,							//绝对值有功无功需量	
//			0x1090,	0x10a0,							//正/反向视在电能
//			0x1091, 0x1092, 0x1093, 				//A/B/C分相正向视在电能
//			0x10a1, 0x10a2, 0x10a3, 				//A/B/C分相反向视在电能

};


//分相电能以0xff表示结束,组合的计算方式需要配置
//wDelta 差值表示电能的最小刻度的整数倍
//wCalcPara 用于表示合ID于分ID的累加方式，例：分相ID的第一个加FLAG_ADD，减FLAG_SUB，第二个分相ID加FLAG_ADD<<2,减FLAG_SUB<<2
//												若是第一、二个ID加(FLAG_ADD | FLAG_ADD<<2),
// 写0表示使用默认参数，系统内部默认参数只有组合有功、无功
//分相ID结束标识使用0xff表示结束，0xff后ID不累计入合ID
static TEnergyCorrect g_tEnergyCorrect[] = {
//	差值	计算方式		总和ID				分相ID
	{3, 	0,				EQ_COM_ABC1, 	{EQ_Q1, EQ_Q2, EQ_Q3, EQ_Q4, 0xff}},
	{3, 	0,				EQ_COM_ABC2, 	{EQ_Q1, EQ_Q2, EQ_Q3, EQ_Q4, 0xff},},
	{3, 	0,				EP_COM_ABC, 	{EP_POS_ABC, EP_NEG_ABC, 0xff},},
};

//校准时的温度读写接口
BYTE GetTempValue()
{
	BYTE bBuf[4];
	ReadItemEx(BN25, PN0, 0x5022, bBuf);
	return bBuf[0];
}

void SaveTempValue(BYTE bValue)
{
	WriteItemEx(BN25, PN0, 0x5022, &bValue);
	TrigerSaveBank(BN25, 0, -1);
}


//组合电能累加方式的判断
//组合有功、无功的组合方式参数可以使用默认的参数
void LoadDefaultCorrectPara(TEnergyCorrect* pCorrect, TAcPara* pAcPara)
{
	int i = 0;

	for (i=0; i<pAcPara->EnergyPara.wEnergyCorrectNum; i++)
	{
		if (pCorrect->wCalcPara == 0)//0的时候表示使用默认参数
		{
			if (pCorrect->wSumID>=EP_COM_A && pCorrect->wSumID<=EP_COM_ABC)//组合有功
				pCorrect->wCalcPara = pAcPara->bCombEpMode;
			else if (pCorrect->wSumID>=EQ_COM_A1 && pCorrect->wSumID<=EQ_COM_ABC1)//组合无功
				pCorrect->wCalcPara = pAcPara->bAEqMode;
			else if (pCorrect->wSumID>=EQ_COM_A2 && pCorrect->wSumID<=EQ_COM_ABC2)//组合无功
				pCorrect->wCalcPara = pAcPara->bREqMode;
		}

		pCorrect++;
	}
}

//脉冲参数的初始化
bool PulseLoadPara(TPulseCfg* pPulseCfg, TPulsePara* pPulsePara)
{
    WORD i, wOI;
    int iLen;
    bool fValid = true;
    BYTE bBuf[32];
    
    WORD wPn = pPulseCfg->wPn;
    BYTE bPulseType = pPulseCfg->bType;
    BYTE bPortIndex = pPulseCfg->bPortNo - 1;

    //if (wPn == PN0)		//交采测量点
    //	return false;

	wOI = OI_PULSE_BASE + bPortIndex;
    memset(pPulsePara, 0, sizeof(TPulsePara));
	pPulsePara->wPn = wPn;
	//pPulsePara->wRate = GetRate(wPn);
	//-------电能参数---------------------------
	if (ReadItemEx(BN0, PN0, 0x400C, bBuf)<=0)  //费率
	{
		/*
		年时区数(p≤14) 				unsigned，
		日时段表数（q≤8）				unsigned，
		日时段数(每日切换数)（m≤14）	unsigned，
		费率数（k≤63） 				unsigned，
		公共假日数（n≤254）			unsigned
		*/
		//DT_STRUCT,05,
		bBuf[9] = RATE_NUM; 
	}
	pPulsePara->EnergyPara.wRateNum = bBuf[9];

	//-------电能参数---------------------------
	pPulsePara->EnergyPara.wPn0 = wPn;  	//脉冲测量点
	pPulsePara->EnergyPara.wPn1 = 0xffff;

	pPulsePara->EnergyPara.wRate = 1;   	//当前的费率,可即时刷新	
	pPulsePara->EnergyPara.dwConst= pPulseCfg->i64Const; 	//脉冲常数
	pPulsePara->EnergyPara.wEpFrac = 4;	//有功电能的小数位数
	pPulsePara->EnergyPara.wEqFrac = 4;	//无功电能的小数位数
#ifdef ACLOG_ENABLE
	pPulsePara->EnergyPara.fEnableLog = true; //支持数据写到铁电
#else
	pPulsePara->EnergyPara.fEnableLog = false; //不支持数据写到铁电
#endif
	pPulsePara->EnergyPara.wLogID = LOG_PULSE_ENERGY1 + bPortIndex;	 //日志文件ID
	pPulsePara->EnergyPara.wSignID = 0;  //保存符号的数据项的ID,放到测量点数据里,0表示不保存符号
	pPulsePara->EnergyPara.i64EpMax = EP_MAX; //有功电能的最大值
	pPulsePara->EnergyPara.i64EqMax = EQ_MAX; //无功电能的最大值

	pPulsePara->EnergyPara.wTypeNum = PULSE_ENERGY_NUM; //wID[3][ENERGY_TYPE_MAX]里实际电能类型的个数
	pPulsePara->EnergyPara.wLogNum = PULSE_ENERGY_NUM;//铁电保存个数
	for (i=0; i<PULSE_ENERGY_NUM; i++)
	{
		pPulsePara->EnergyPara.wInnerID[i] = g_wPulseCurEnergyID[1][bPulseType]; //内部计算的电能ID
		pPulsePara->EnergyPara.wID[0][i] = g_wPulseCurEnergyID[0][bPulseType];  //本月	高精度电能
		pPulsePara->EnergyPara.wPlusID[i] = 0;  //本月	低精度电能
		pPulsePara->EnergyPara.wID[1][i] = 0;	 //上日
		pPulsePara->EnergyPara.wID[2][i] = 0;	 //g_wPulseCurEnergyID[0][bPulseType] + 0x0400  //上月
		pPulsePara->EnergyPara.wID[3][i] = 0;	//g_wPulseCurEnergyID[0][bPulseType] + 0x0800;  //上上月
		
		if ((g_wPulseCurEnergyID[0][bPulseType]&0x1) == 0)  //有功
		{
			pPulsePara->EnergyPara.fEp[i] = true;
			pPulsePara->EnergyPara.fSign[i] = false;	//有功不支持符号
		}
		else
		{	
			pPulsePara->EnergyPara.fEp[i] = false;
			pPulsePara->EnergyPara.fSign[i] = false;		//无功不支持符号
		}
	}
	
	//------------需量参数----------------------
	//需量周期及滑差时间
	/*WORD wDemandPeriod, wSlideInterv, wSlideNum;
	ReadItemEx(BN0, wPn, 0xc111, bBuf); //需量周期
	wDemandPeriod = BcdToByte(bBuf[0]);

	ReadItemEx(BN0, wPn, 0xc112, bBuf); //滑差时间
	wSlideInterv = BcdToByte(bBuf[0]);
	
	if (wDemandPeriod==0 || wSlideInterv==0 || 
		wDemandPeriod>60)
	{
		wDemandPeriod = 15;	//15
		wSlideInterv = 1;
	}

	//滑差式的需量周期与滑差时间的校验
	if (wDemandPeriod!=wSlideInterv && //采用区间式计算需量，需量周期与滑差时间必须设置成同一数值
		(wDemandPeriod%wSlideInterv!=0 || wDemandPeriod/wSlideInterv<5))
	{	//需量周期必须是滑差时间的5倍及以上(整数倍)
		wDemandPeriod = 15;
		wSlideInterv = 1;
	}

	wSlideNum = wDemandPeriod/wSlideInterv; //一个需量周期内的滑差时间的个数
	
	pPulsePara->DemandPara.wPn0 = wPn; //脉冲测量点
	pPulsePara->DemandPara.wPn1 = 0xffff;	
	
	pPulsePara->DemandPara.wRate = 1;    //当前的费率,可即时刷新
	pPulsePara->DemandPara.dwConst = pPulseCfg->i64Const; 	//脉冲常数取正向有功
	pPulsePara->DemandPara.wFrac = 4;		//需量小数位数,标准格式是645里的NN.NNNN(kw/kvar),配为4
#ifdef ACLOG_ENABLE
	pPulsePara->DemandPara.fEnableLog = true; //支持数据写到铁电
#else
	pPulsePara->DemandPara.fEnableLog = false; //不支持数据写到铁电
#endif
	pPulsePara->DemandPara.wLogID = LOG_PULSE_DEMAND1 + bPortIndex;	 //日志文件ID
	pPulsePara->DemandPara.wMeteringDay = 1;	//抄表日
	pPulsePara->DemandPara.wMeteringHour = 0;	//抄表日的小时
	pPulsePara->DemandPara.wSlipNum = wSlideNum;	//滑差数,通过最大需量周期/滑差时间求得,可即时刷新 15
	pPulsePara->DemandPara.dwSlipInterv = wSlideInterv;	//滑差时间,单位分钟 1
	
	pPulsePara->DemandPara.wTypeNum = PULSE_DEMAND_NUM; //wID[3][DEMAND_TYPE_MAX]里实际电能类型的个数
	pPulsePara->DemandPara.wLogNum = PULSE_DEMAND_NUM;
	for (i=0; i<PULSE_DEMAND_NUM; i++)
	{
		pPulsePara->DemandPara.wInnerID[i] = g_wPulseCurDemandID[1][bPulseType]; //内部计算的电能ID
		
		pPulsePara->DemandPara.wDemandID[0][i] = g_wPulseCurDemandID[0][bPulseType]; //本月  
		pPulsePara->DemandPara.wDemandID[1][i] = g_wPulseCurDemandID[0][bPulseType] + 0x0400; //上月 
		pPulsePara->DemandPara.wDemandID[2][i] = g_wPulseCurDemandID[0][bPulseType] + 0x0800; //上上月
		
		pPulsePara->DemandPara.wTimeID[0][i] = g_wPulseCurTimeID[bPulseType]; //本月  
		pPulsePara->DemandPara.wTimeID[1][i] = g_wPulseCurTimeID[bPulseType] + 0x0400; //上月 
		pPulsePara->DemandPara.wTimeID[2][i] = 0;	//g_wPulseCurTimeID[bPulseType] + 0x0800; //上上月
	}

	//自动抄表日,高字节表示抄表日,低字节表示抄表日的小时
	if (ReadItemEx(BN0, PN0, 0xc117, bBuf) <= 0)  //自动抄表日
	{
		bBuf[1] = 0x01; bBuf[0] = 0x00;	//1号零点
	}
	
	if (bBuf[1]==0x00 && bBuf[0]==0x00)
	{
		bBuf[1] = 0x01; bBuf[0] = 0x00;	//1号零点
	}
		
	memset(pPulsePara->wAutoDate, 0xff, sizeof(pPulsePara->wAutoDate));
	pPulsePara->wAutoDate[0] = (WORD )BcdToByte(bBuf[1])*0x100 + BcdToByte(bBuf[0]);
	//pPulsePara->wDayFrzTime = 0xffff;  //日结算时刻,小时,BIN,0xffff表示不冻结,
	
	iLen = OoReadAttr(wOI, ATTR2, bBuf, &pbFmt, &wFmtLen);	//通信地址
	if (iLen > 0)
	{
		memset(pPulsePara->bAddr, 0, PUSLE_ADDR_LEN);	//参数无效时 初始化为全0
		if (bBuf[0] == DT_OCT_STR)
		{
			iLen = (bBuf[1]<PUSLE_ADDR_LEN) ? bBuf[1] : PUSLE_ADDR_LEN;		//bBuf[1]为实际长度
			if (iLen > 0)
				memcpy(pPulsePara->bAddr, bBuf+2, iLen);	
		}
	}*/

	return true;
}

//是否为当前组合电能ID
bool IsCombCurEnergyID(WORD wID)
{
	WORD i;
	const WORD wCombEngID[] = {0x900f,0x911f, 0x912f,};
				
	for (i=0; i<sizeof(wCombEngID)/sizeof(WORD); i++)
	{
		if (wID == wCombEngID[i])
			return true;
	}

	return false;	
}

//交采参数的初始化
bool AcLoadPara(WORD wPn, TAcPara* pAcPara)
{
	WORD i;
	BYTE bBuf[32];
	int iRet,iTmp;

	memset(pAcPara, 0, sizeof(TAcPara));
	pAcPara->wPoint = wPn;
	pAcPara->wRate = 1;   	//当前的费率,可即时刷新
	
	//----------脉冲常数-----------
	iRet = ReadItemEx(BN25, PN0, 0x5004, bBuf); //0x5004 3 脉冲常数,BCD码
	if (iRet<=0 || IsAllAByte(bBuf, 0, 3))
		ReadItemEx(BN4, wPn, 0xc030, bBuf); //0xa010 3 脉冲常数,BCD码
	pAcPara->dwConst = BcdToDWORD(bBuf, 3);
	if (pAcPara->dwConst == 0)
	{
		pAcPara->dwConst = 6400;
		DWORDToBCD(pAcPara->dwConst, bBuf, 3);
//			WriteItemEx(BN4, PN0, 0xc030, bBuf); //0xa010 3 脉冲常数,BCD码
//			WriteItemEx(BN4, PN0, 0xc031, bBuf); //0xa010 3 脉冲常数,BCD码
//			WriteItemEx(BN4, wPn, 0xc030, bBuf); //0xa010 3 脉冲常数,BCD码
//			WriteItemEx(BN4, wPn, 0xc031, bBuf); //0xa010 3 脉冲常数,BCD码
	}
	
	g_iEPerPulse = 3600L * 1000L * 10 * 8 / pAcPara->dwConst * 10L * 1000L; //0x1C9C3800; (10L*1000L*3600L*1000L*10*8/6000)    
					//每个脉冲等于多少个 瓦/100 * 毫秒/8

	if (pAcPara->dwConst > 10000)   //按3相4线12A,220V时每秒必须输出 (脉冲常数/454) 个脉冲算
	{			  //22.02
		g_PulseWidthTop = 10*8;     //每秒最多50个脉冲
		g_PulseWidthBottom = 10*8;
	}
	else if (pAcPara->dwConst > 4800)   
	{				//10.57	个脉冲
		g_PulseWidthTop = 20*8;  	  //每秒最多25个脉冲
		g_PulseWidthBottom = 20*8;
	}
	else if (pAcPara->dwConst > 2400)   
	{				   //5.28	个脉冲
		g_PulseWidthTop = 40*8;    //每秒最多12个脉冲
		g_PulseWidthBottom = 40*8;
	}
	else   //==2400
	{
		g_PulseWidthTop = 10*8;    //每秒最多6个脉冲
		g_PulseWidthBottom = 10*8;
	}
	
/*	
	//----------接线方式------------
	pAcPara->bConnectType = GetConnectType(wPn); //终端接线方式 1	1:单相;3:三项三线;4:三相四线

	//----------额定电压,电流---------
	if (ReadItemEx(BN0, wPn, 0x019f, bBuf) > 0)
	{		
		pAcPara->dwUn = (DWORD )Fmt7ToVal(bBuf+4, 2); //额定电压值
		pAcPara->dwIn = (DWORD )Fmt22ToVal(bBuf+6, 1)*100; //最大电流值(1个字节，1个小数位）
		if (pAcPara->dwUn == 0)
			pAcPara->dwUn = 2200;	//额定电压,格式NNNNN.N
		if (pAcPara->dwIn == 0)
			pAcPara->dwIn = 5000;	//额定电流,格式NNN.NNN
	}
	else
	{
		pAcPara->dwUn = 2200;	//额定电压,格式NNNNN.N
		pAcPara->dwIn = 5000;	//额定电流,格式NNN.NNN
	}*/
	
	ReadItemEx(BN25, PN0, 0x5003, bBuf);
	if (bBuf[0] == 0 || bBuf[0] > 4)
		pAcPara->bConnectType = 4;
	else
		pAcPara->bConnectType = bBuf[0];
	
	ReadItemEx(BN25, PN0, 0x5001, bBuf);
	pAcPara->dwUn = (DWORD )BcdToDWORD(bBuf, 3)/10; //额定电压值2200,一位小数
	ReadItemEx(BN25, PN0, 0x5002, bBuf);
	pAcPara->dwIn = (DWORD )BcdToDWORD(bBuf, 3); //额定电流值
	if (pAcPara->dwUn == 0)
		pAcPara->dwUn = 2200;	//额定电压,格式NNNNN.N
	if (pAcPara->dwIn == 0)
		pAcPara->dwIn = 1500;	//额定电流,格式NNN.NNN

	//有功电能累加模式
	
	if (ReadItemEx(BN0, PN0, 0x4112, bBuf) <= 0)  //有功电能计量方式选择
		pAcPara->bCombEpMode |= (P_POSADD|P_NEGADD);
	else
	{
		pAcPara->bCombEpMode = 0;
		//组合有功的累积方式
		if ((bBuf[2] & 0x03) == 0x01)
			pAcPara->bCombEpMode |= P_POSADD;
		else if ((bBuf[2] & 0x03) == 0x02)
			pAcPara->bCombEpMode |= P_POSSUB;
		
		if (((bBuf[2]>>2) & 0x03) == 0x01)
			pAcPara->bCombEpMode |= P_NEGADD;
		else if (((bBuf[2]>>2) & 0x03) == 0x02)
			pAcPara->bCombEpMode |= P_NEGSUB;
	}
	DTRACE(DB_AC,("\r\n####AcLoadPara pAcPara->bCombEpMode=%d", pAcPara->bCombEpMode));
				//		D2			|		D1			|			D0			|
				// 反向计入正向		| 	反向不计量		| 		各相累加模式	|
				// 0:不计入;1计入	| 0:计量;1不计量	| 0:代数和;1绝对值和	|
				// D2D1=00:正->正,反->反;正反向分别计量
				// D2D1=01:正->正;		只计正向,反向不计
				// D2D1=10:正反->正,反->反;
				// D2D1=11:正反->正		反向不计
	//有功电能累加模式
	if (ReadItemEx(BN10, PN0, 0xa122, bBuf) <= 0)  //有功电能计量方式选择
		bBuf[0] = 0;
	pAcPara->bEpMode = bBuf[0];

	//无功电量累加标志
	//正/反向无功电量累加标志,D7D6,D5D4,D3D2,D1D0,分别对应4~1象限无功累加标志,
	//低位:1-加; 0-减;	
	//高位:1-计算; 0-不计算
	if (ReadItemEx(BN0, PN0, 0x4113, bBuf) <= 0)  //正向无功电量累加标志
		pAcPara->bAEqMode = Q1ADD | Q2ADD;
	else
	{
		for (int i = 0; i < 4; i++)//分别表示第一、二、三、四象限
		{
			if (((bBuf[2]>>i*2) & 0x03) == 0x01)
				pAcPara->bAEqMode |= (QADD<<i*2);
			else if (((bBuf[2]>>i*2) & 0x03) == 0x02)
				pAcPara->bAEqMode |= (QSUB<<i*2);
		}
	}
	DTRACE(DB_AC,("\r\n####AcLoadPara pAcPara->bAEqMode=%d", pAcPara->bAEqMode));

	if (ReadItemEx(BN0, PN0, 0x4114, bBuf) <= 0)  //反向无功电量累加标志
		pAcPara->bREqMode = Q3ADD | Q4ADD;
	else
	{
		for (int i = 0; i < 4; i++)
		{
			if (((bBuf[2]>>i*2) & 0x03) == 0x01)
				pAcPara->bREqMode |= (QADD<<i*2);
			else if (((bBuf[2]>>i*2) & 0x03) == 0x02)
				pAcPara->bREqMode |= (QSUB<<i*2);
		}
	}
	DTRACE(DB_AC,("\r\n####AcLoadPara pAcPara->bREqMode=%d", pAcPara->bREqMode));


	//自动抄表日,高字节表示抄表日,低字节表示抄表日的小时
	if (ReadItemEx(BN0, PN0, 0x4116, bBuf) <= 0)  //自动抄表日
	{
		bBuf[5] = 0x01; bBuf[7] = 0x00;	//1号零点
	}
//		bBuf[0]= DT_ARRAY;
//		bBuf[1]= 0x03;
//		bBuf[2]= DT_STRUCT;
//		bBuf[3]= 0x02;
//		bBuf[4]= DT_UNSIGN;
//		bBuf[5]= data;
//		bBuf[6]= DT_UNSIGN;
//		bBuf[7]= data;
	if (bBuf[5]==0x00 && bBuf[7]==0x00)
	{
		bBuf[5] = 0x01; bBuf[7] = 0x00;	//1号零点
	}
	memset(pAcPara->wAutoDate, 0xff, sizeof(pAcPara->wAutoDate));

	pAcPara->wAutoDate[0] = (WORD )(bBuf[5])*0x100 + bBuf[7];//(WORD )BcdToByte(bBuf[1])*0x100 + BcdToByte(bBuf[0]);
	pAcPara->wDayFrzTime = 0xffff;  //日结算时刻,小时,BIN,0xffff表示不冻结,								   
	
	//角度方向							   
	ReadItemEx(BN10, PN0, 0xa015, &pAcPara->bAngleClockwise);
			   	//0xa015 1	角度方向,0表示角度按照逆时针方向表示,Ua,Ub,Uc分别为0,240,120
				//					 1表示角度按照顺时针方向表示,Ua,Ub,Uc分别为0,120,240
	//AcLoadAdj(pAcPara); //校正参数和相位校正角度
	
	pAcPara->fCalcuHarmonic = true;		//是否计算谐波
	pAcPara->wHarmNum = HARMONIC_NUM;	//谐波计算次数
	
	//-------电能参数---------------------------
	if (ReadItemEx(BN0, PN0, 0x400C, bBuf)<=0)  //费率
	{
		/*
		年时区数(p≤14) 				unsigned，
		日时段表数（q≤8）				unsigned，
		日时段数(每日切换数)（m≤14）	unsigned，
		费率数（k≤63） 				unsigned，
		公共假日数（n≤254）			unsigned
		*/
		//DT_STRUCT,05,
		bBuf[9] = RATE_NUM; 
	}
	pAcPara->EnergyPara.wRateNum = bBuf[9];
		
	pAcPara->EnergyPara.wPn0 = PN0;  	//默认测量点
	if (wPn == PN0)	//备选测量点也是PN0,则不需要重复写入
		pAcPara->EnergyPara.wPn1 = 0xffff;
	else	
		pAcPara->EnergyPara.wPn1 = wPn;
		
	pAcPara->EnergyPara.wRate = 1;   	//当前的费率,可即时刷新
	pAcPara->EnergyPara.dwConst= pAcPara->dwConst; 	//脉冲常数
	pAcPara->EnergyPara.wEpFrac = 4;	//有功电能的小数位数
	pAcPara->EnergyPara.wEqFrac = 4;//2;	//无功电能的小数位数
#ifdef ACLOG_ENABLE
	pAcPara->EnergyPara.fEnableLog = true; //支持数据写到铁电
#else
	pAcPara->EnergyPara.fEnableLog = false; //不支持数据写到铁电
#endif
	pAcPara->EnergyPara.wLogID = LOG_ENERGY;	 //日志文件ID
	pAcPara->EnergyPara.wSignID = 0xc800;  //保存符号的数据项的ID,放到测量点数据里,0表示不保存符号
	pAcPara->EnergyPara.i64EpMax = EP_MAX; //有功电能的最大值
	pAcPara->EnergyPara.i64EqMax = EQ_MAX; //无功电能的最大值

	pAcPara->EnergyPara.wLogBarID = LOG_ENERGY_BAR; //交采电能不足最小计量单位的日志文件ID
	pAcPara->EnergyPara.wLogBarNum = AC_ENERGY_BAR_NUM; ////保存不足最小计量单位的数字，保存到42个

	pAcPara->EnergyPara.wTypeNum = AC_ENERGY_NUM; //wID[3][ENERGY_TYPE_MAX]里实际电能类型的个数
	pAcPara->EnergyPara.wLogNum = AC_ENG_LOG_NUM;//铁电保存电能类型的个数

	pAcPara->EnergyPara.ptCorrect = (TEnergyCorrect *)g_tEnergyCorrect;//用于计算组合电能出现差值的问题
	pAcPara->EnergyPara.wEnergyCorrectNum = sizeof(g_tEnergyCorrect)/sizeof(TEnergyCorrect);

	for (i=0; i<AC_ENERGY_NUM; i++)
	{
		pAcPara->EnergyPara.wInnerID[i] = g_wAcCurEnergyID[1][i]; //内部计算的电能ID
		pAcPara->EnergyPara.wPlusID[i] = g_wAcCurEnergyID[2][i];  //低精度
		pAcPara->EnergyPara.wID[0][i] = g_wAcCurEnergyID[0][i];  //高精度
		pAcPara->EnergyPara.wID[1][i] = 0;	 //上日
		pAcPara->EnergyPara.wID[2][i] = 0;//g_wAcCurEnergyID[0][i] + 0x0600;	//上月
		pAcPara->EnergyPara.wID[3][i] = 0;//g_wAcCurEnergyID[0][i] + 0x0a00;  	//上上月
		
		if (g_wAcCurEnergyID[2][i]<0x0030)//|| g_wAcCurEnergyID[0][i]==0x900f)  //本月
			pAcPara->EnergyPara.fEp[i] = true;
		else
			pAcPara->EnergyPara.fEp[i] = false;
	}
	
	//------------需量参数----------------------
	pAcPara->DemandPara.wRateNum = pAcPara->EnergyPara.wRateNum;
	pAcPara->DemandPara.fSingleDemandId = false;
	pAcPara->DemandPara.bDemTimeLen = 7;

	//需量周期及滑差时间
	WORD wDemandPeriod, wSlideInterv, wSlideNum;
	ReadItemEx(BN0, wPn, 0x4100, bBuf); //需量周期
	wDemandPeriod = bBuf[1];//BcdToByte(bBuf[0]);

	ReadItemEx(BN0, wPn, 0x4101, bBuf); //滑差时间
	wSlideInterv = bBuf[1];//BcdToByte(bBuf[0]);
	//DTRACE(DB_CRITICAL,("\r\n####AcLoadPara wDemandPeriod=%d ,wSlideInterv=%d", wDemandPeriod,wSlideInterv));
	
	if (wDemandPeriod==0 || wSlideInterv==0 || 
		wDemandPeriod>60)
	{
		wDemandPeriod = 15;	//15
		wSlideInterv = 1;
	}

	//滑差式的需量周期与滑差时间的校验
	if (wDemandPeriod!=wSlideInterv && //采用区间式计算需量，需量周期与滑差时间必须设置成同一数值
		(wDemandPeriod%wSlideInterv!=0 || wDemandPeriod/wSlideInterv<5))
	{	//需量周期必须是滑差时间的5倍及以上(整数倍)
		wDemandPeriod = 15;
		wSlideInterv = 1;
	}

	wSlideNum = wDemandPeriod/wSlideInterv; //一个需量周期内的滑差时间的个数
	
	pAcPara->DemandPara.wPn0 = PN0; //默认测量点
	if (wPn == PN0)	//备选测量点也是PN0,则不需要重复写入
		pAcPara->DemandPara.wPn1 = 0xffff;
	else	
		pAcPara->DemandPara.wPn1 = wPn;
	
	pAcPara->DemandPara.wRate = 1;    //当前的费率,可即时刷新
	pAcPara->DemandPara.dwConst = pAcPara->dwConst; 	//脉冲常数
	pAcPara->DemandPara.wFrac = 4;		//需量小数位数,标准格式是645里的NN.NNNN(kw/kvar),配为4
#ifdef ACLOG_ENABLE
	pAcPara->DemandPara.fEnableLog = true; //支持数据写到铁电
#else
	pAcPara->DemandPara.fEnableLog = false; //不支持数据写到铁电
#endif
	pAcPara->DemandPara.wLogID = LOG_DEMAND;	 //日志文件ID
	pAcPara->DemandPara.wMeteringDay = 1;	//抄表日
	pAcPara->DemandPara.wMeteringHour = 0;	//抄表日的小时
	pAcPara->DemandPara.wSlipNum = wSlideNum;	//滑差数,通过最大需量周期/滑差时间求得,可即时刷新 15
	pAcPara->DemandPara.dwSlipInterv = wSlideInterv;	//滑差时间,单位分钟 1
	
	pAcPara->DemandPara.wTypeNum = AC_DEMAND_NUM; //wID[3][DEMAND_TYPE_MAX]里实际电能类型的个数
	pAcPara->DemandPara.wLogNum = AC_DMD_LOG_NUM;//要保存到铁电的个数
	pAcPara->DemandPara.bDemTimeLen = DMTLEN;
	for (i=0; i<AC_DEMAND_NUM; i++)
	{
		pAcPara->DemandPara.wInnerID[i] = g_wAcCurDemandID[1][i]; //内部计算的电能ID
		
		pAcPara->DemandPara.wDemandID[0][i] = g_wAcCurDemandID[0][i]; //本月  
		pAcPara->DemandPara.wDemandID[1][i] = 0;//g_wAcCurDemandID[0][i] + 0x0600; //上月 
		pAcPara->DemandPara.wDemandID[2][i] = 0;//g_wAcCurDemandID[0][i] + 0x0a00; //上上月
		pAcPara->DemandPara.wDemandID[3][i] = g_wAcCurDemandID[2][i]; //当前实时需量
		
//			pAcPara->DemandPara.wTimeID[0][i] = g_wAcCurTimeID[i]; //本月  
//			pAcPara->DemandPara.wTimeID[1][i] = g_wAcCurTimeID[i] + 0x0600; //上月 
//			pAcPara->DemandPara.wTimeID[2][i] = g_wAcCurTimeID[i] + 0x0a00; //上上月
//			pAcPara->DemandPara.wTimeID[0][i] = g_wAcCurDemandID[0][i]; //本月  
//			pAcPara->DemandPara.wTimeID[1][i] = g_wAcCurDemandID[0][i] + 0x0600; //上月 
//			pAcPara->DemandPara.wTimeID[2][i] = g_wAcCurDemandID[0][i] + 0x0a00; //上上月
	}
	LoadDefaultCorrectPara(g_tEnergyCorrect, pAcPara);//对于累加的数据进行校验的参数进行默认处理
	
	return true;
}

///////////////////////////////////////////////////////////////////////////////
//时段费率
TDataItem m_diRatePeriod; //需量周期


//从645费率时段表中取费率，交采支持8套日时段表，脉冲测量点只支持第1日时段表0xc33f
bool InitRatePeriod(BYTE *pSrc, TRatePeriod* pRatePeriod, WORD wRatePeriodNum)
{
    BYTE bRateNum=0; 
    TRatePeriod* pRatePeriod0 = pRatePeriod;


	//实际时段数
	DTRACE(DB_AC, ("InitRatePeriod:wRatePeriodNum=%d.\r\n",wRatePeriodNum));
	bRateNum = wRatePeriodNum;
	if (bRateNum>RATE_PERIOD_NUM || bRateNum==0)
		bRateNum = RATE_PERIOD_NUM;
	DTRACE(DB_AC, ("InitRatePeriod pSrc[0]=%d.pSrc[1]=%d.\r\n",pSrc[0],pSrc[1]));
	/*
//			0x01,MAX_DAY_CHART_NUM,//最多日时段数暂定为8
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
	*/

	BYTE* pbBuf = pSrc+2;//0x01,RATE_PERIOD_NUM,//日时段表，最多8个
	for (WORD i=0; i<bRateNum; i++)
	{
		DTRACE(DB_AC, ("InitRatePeriod 0: i=%d pbBuf[0]=%d.pbBuf[1]=%d.\r\n",i,pbBuf[0],pbBuf[1]));
		pbBuf += 2;//0x02,0x03
		DTRACE(DB_AC, ("InitRatePeriod 1: i=%d pbBuf[1]=%d.pbBuf[3]=%d.\r\n",i,pbBuf[1],pbBuf[3]));
		pRatePeriod->dwStartTime = pbBuf[1]*100+pbBuf[3];
		pRatePeriod->wRate = pbBuf[5];
		DTRACE(DB_AC, ("InitRatePeriod 2: i=%d pbBuf[5]=%d.\r\n",i,pbBuf[5]));
		if (pRatePeriod->wRate > RATE_NUM)
			pRatePeriod->wRate = 1;
		pbBuf += 6;
		pRatePeriod++;
	}
	
	pRatePeriod = pRatePeriod0;
	WORD i,j,k,t;
	for (i=0; i<bRateNum-1; i++)
	{
	    if (!pRatePeriod[i].wRate)
	    	break;

	    k = i;
	    for (j=i+1; j<bRateNum; j++)
	    {
	        if (!pRatePeriod[j].wRate)
	        	break;
	    	if (pRatePeriod[j].dwStartTime < pRatePeriod[k].dwStartTime)
	    		k = j;
	    }
	    if (k != i)
	    {
	    	t = pRatePeriod[i].dwStartTime;
	    	pRatePeriod[i].dwStartTime = pRatePeriod[k].dwStartTime;
	    	pRatePeriod[k].dwStartTime = t;

	    	t = pRatePeriod[i].wRate;
	    	pRatePeriod[i].wRate = pRatePeriod[k].wRate;
	    	pRatePeriod[k].wRate = t;	    		    	
	    }
	}
	return true;
}



bool LoadTOU(WORD wPoint, TTOU* pTOU)
{
	WORD i,wDI;
	BYTE* p;
	BYTE bBuf[550];
	TTime now,tTime;
	GetCurTime(&now);
	
	memset(pTOU, 0, sizeof(TTOU));
	
	//-------电能参数---------------------------
	if (ReadItemEx(BN0, PN0, 0x400C, bBuf))  //费率
	{
		/*
		年时区数(p≤14) 				unsigned，
		日时段表数（q≤8）				unsigned，
		日时段数(每日切换数)（m≤14）	unsigned，
		费率数（k≤63） 				unsigned，
		公共假日数（n≤254）			unsigned
		*/
		//DT_STRUCT,05,
		//年时区数
		pTOU->wZoneNum = bBuf[3];//BcdToByte(bBuf[0]);	  
		if (pTOU->wZoneNum > MAX_ZONE_NUM)
			pTOU->wZoneNum = MAX_ZONE_NUM;
		DTRACE(DB_AC, ("LoadTOU pTOU->wZoneNum=%d.\r\n",pTOU->wZoneNum));
		
		//日时段表数
		pTOU->wDayChartNum = bBuf[5];//BcdToByte(bBuf[1]);	  
		if (pTOU->wDayChartNum > MAX_DAY_CHART_NUM)
			pTOU->wDayChartNum = MAX_DAY_CHART_NUM;
		DTRACE(DB_AC, ("LoadTOU pTOU->wDayChartNum=%d.\r\n",pTOU->wDayChartNum));
		
		//日时段数
		pTOU->wPeriodNum = bBuf[7];//BcdToByte(bBuf[2]);
		if (pTOU->wPeriodNum>RATE_PERIOD_NUM || pTOU->wPeriodNum==0)
			pTOU->wPeriodNum = RATE_PERIOD_NUM;
		DTRACE(DB_AC, ("LoadTOU pTOU->wPeriodNum=%d.\r\n",pTOU->wPeriodNum));
		
		//费率数
		pTOU->wRateNum = bBuf[9];//RATE_NUM;
		if (pTOU->wRateNum>RATE_NUM || pTOU->wRateNum==0)
			pTOU->wRateNum = RATE_NUM;
		DTRACE(DB_AC, ("LoadTOU pTOU->wRateNum=%d.\r\n",pTOU->wRateNum));

		//公共假日数
		pTOU->wHolidayNum = bBuf[11];//BcdToDWORD(bBuf, 2);
		if (pTOU->wHolidayNum > MAX_HOLIDAY_NUM)
			pTOU->wHolidayNum = MAX_HOLIDAY_NUM;
		DTRACE(DB_AC, ("LoadTOU pTOU->wHolidayNum=%d.\r\n",pTOU->wHolidayNum));
	}


	//公共假日表
	if (ReadItemEx(BN0, PN0, 0x4011, bBuf)) 
	{
		p = bBuf+2;//01,0x14
		for (i=0; i<pTOU->wHolidayNum; i++) 
		{
			p += 3;//0x02,0x02,DT_DATE
			pTOU->zHoliday[i].nYear = ((WORD )p[0]<<8) | p[1];
			pTOU->zHoliday[i].nMonth = p[2];
			pTOU->zHoliday[i].nDay =  p[3];
			pTOU->zHoliday[i].nWeek =  p[4];
			//p[5] DT_UNSIGN
			pTOU->zHoliday[i].wDayChart = p[6];
			p += 7;
		}
	}
	

	//周休日状态字
	if (ReadItemEx(BN0, PN0, 0x4012, bBuf) != 3)
		goto fail;
	pTOU->bRestStatus = bBuf[2];
	
	//周休日采用的日时段表号		
	if (ReadItemEx(BN0, PN0, 0x4013, bBuf) != 2)
		goto fail;

	pTOU->wRestDayChart = bBuf[1];

	//两套时区表切换时间
	if (ReadItemEx(BN0, PN0, 0x4008, bBuf)==8)  
	{
		memcpy(pTOU->bTimeZoneSwitchTime, bBuf+1, 7);
	}
	OoDateTimeSToTime(bBuf+1,&tTime);
	tTime.nSecond = 0;
	if(IsInvalidTime(tTime)==false)
	{
		if(MinutesFrom2000(now)>=MinutesFrom2000(tTime))
		{//需要切换
			DTRACE(DB_AC, ("LoadTOU bTimeZoneSwitchTime.\n"));
			//切换备用套
			if (ReadItemEx(BN0, PN0, 0x4015, bBuf))
			{
				WriteItemEx(BN0, PN0, 0x4014, bBuf);
			}
			//切换标志
			bBuf[0] = 0;
			ReadItemEx(BN4, PN0, 0xc900, bBuf);
			bBuf[0] |= 0x01;
			WriteItemEx(BN4, PN0, 0xc900, bBuf);
			memset(bBuf,0,8);
			bBuf[0] = DT_DATE_TIME_S;
			WriteItemEx(BN0, PN0, 0x4008, bBuf);
		}
	}


	//当前套年时区表
	if (ReadItemEx(BN0, PN0, 0x4014, bBuf))
	{
		p = bBuf+2;//01,0x0e
		for (i=0; i<pTOU->wZoneNum; i++) 
		{
			p += 2;//0x02,0x03
			pTOU->zZone[i].wDayChart = p[1];
			pTOU->zZone[i].nDay =  p[3];
			pTOU->zZone[i].nMonth =  p[5];
			p += 6;
		}
	}


	//两套日时段切换时间
	if (ReadItemEx(BN0, PN0, 0x4009, bBuf)==8)  
	{
		memcpy(pTOU->bDayChartSwitchTime, bBuf+1, 7);
	}
	OoDateTimeSToTime(bBuf+1,&tTime);
	tTime.nSecond = 0;
	if(IsInvalidTime(tTime)==false)
	{
		if(MinutesFrom2000(now)>=MinutesFrom2000(tTime))
		{//需要切换
			DTRACE(DB_AC, ("LoadTOU bDayChartSwitchTime.\n"));
			//切换备用套
			ReadItemEx(BN0, PN0, 0x4017, bBuf);
			WriteItemEx(BN0, PN0, 0x4016, bBuf);

			//切换标志
			bBuf[0] = 0;
			ReadItemEx(BN4, PN0, 0xc900, bBuf);
			bBuf[0] |= 0x02;
			WriteItemEx(BN4, PN0, 0xc900, bBuf);
			
			memset(bBuf,0,8);
			bBuf[0] = DT_DATE_TIME_S;
			WriteItemEx(BN0, PN0, 0x4009, bBuf);
		}
	}

//		DTRACE(DB_AC, ("LoadTOU 10.\n"));
	//当前套日时段表
	if (ReadItemEx(BN0, PN0, 0x4016, bBuf)<=0)
	{
		goto fail;
	}
	
	/*
	0x01,MAX_DAY_CHART_NUM,//最多日时段数暂定为8
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
	*/
	p = bBuf+2;//0x01,MAX_DAY_CHART_NUM,//最多日时段数暂定为8
	for (i=0; i<pTOU->wDayChartNum; i++) 
	{
		if (InitRatePeriod(p, &pTOU->rpDayChart[i][0],pTOU->wPeriodNum) == false)
		{
			goto fail;
		}
		p += 66;
	}
	DTRACE(DB_AC, ("LoadTOU true.\n"));
	return true;

fail:
	DTRACE(DB_AC, ("LoadTOU fail.\n"));
	memset(pTOU, 0, sizeof(TTOU));
	return false;
}




WORD GetRate(TRatePeriod* pRatePeriod, WORD wPeriodNum)
{
	TTime now;
	GetCurTime(&now);
	DWORD dwTime = now.nHour*100 + now.nMinute;	
	WORD i;
	DTRACE(DB_AC, ("GetRate wPeriodNum=%d.\r\n",wPeriodNum));
	for (i=0; i<wPeriodNum; i++)
	{
		DTRACE(DB_AC, ("GetRate pRatePeriod[%d].dwStartTime=%d.\r\n",i,pRatePeriod[i].dwStartTime));
		DTRACE(DB_AC, ("GetRate pRatePeriod[%d].wRate=%d.\r\n",i,pRatePeriod[i].wRate));
		if (pRatePeriod[i].dwStartTime>dwTime || 
			(pRatePeriod[i].dwStartTime==0 && pRatePeriod[i].wRate==0))
		{
			break;
		}
	}

	WORD wRate;
	if (i == 0)   //比第一个还早        //辽宁要求费率时段可跨日设置，
	{                                  // 若当前时间比第一个时段还早则取最后一个费率
	    if (wPeriodNum < 1)
		{
	    	wRate = 3;	//1		默认为平电量
		}
	    else
		{
			  //wRate = pRatePeriod[wPeriodNum-1].wRate;
			wRate = 0;
			for (WORD k=wPeriodNum; k>0; k--)
			{
				WORD wLastValidRate = pRatePeriod[k-1].wRate;	//从后面往前找有效的费率
				if (wLastValidRate>=1 && wLastValidRate<=RATE_NUM)
				{
					wRate = wLastValidRate;
					break;
				}
			}
		}
	}	
	else
		wRate = pRatePeriod[i-1].wRate;  //最后一个被访问的时段的开始时间比当前时间还大，
										 //则取该时段的前一个时段
		
	if (wRate<1 || wRate>RATE_NUM)
		wRate = 3;	//1		默认为平电量
		
	BYTE bTmp[10];
	bTmp[0] = wRate;
	WriteItemEx(BANK2, POINT0, 0x1042, bTmp);
	DTRACE(DB_AC, ("GetRate::wRate=%d.\n", wRate));

	return wRate;
}


//描述:从645协议规定的整套时段费率表中根据当前时间选择当前费率号
WORD GetRate(TTOU* pTOU)
{
	WORD i;
	WORD wDayChart = 0; //日时段表号,0表示无效
	TTime now;
	GetCurTime(&now);
	
	//选择日时段表号,顺序是:公共假日->周休日->年时区
	
	//首先从公共假日表选择
	for (i=0; i<pTOU->wHolidayNum; i++)
	{	
		if (now.nMonth==pTOU->zHoliday[i].nMonth &&
			now.nDay==pTOU->zHoliday[i].nDay && now.nYear==pTOU->zHoliday[i].nYear)
		{
			wDayChart = pTOU->zHoliday[i].wDayChart;
			break;
		}
	}
	DTRACE(DB_AC, ("GetRate wDayChart0=%d.\r\n",wDayChart));
	
	if (wDayChart == 0)  //还没确定日时段表号,则从周休日中选择
	{
		if ((pTOU->bRestStatus & (1<<now.nWeek-1)) == 0)  //休息
		{
			wDayChart = pTOU->wRestDayChart;  //周休日采用的日时段表号
		}
	}
	DTRACE(DB_AC, ("GetRate wDayChart1=%d.\r\n",wDayChart));
	if (wDayChart == 0)  //还没确定日时段表号,则从年时区中选择
	{
		DWORD dwNowday = now.nMonth*31 + now.nDay;
		for (i=0; i<pTOU->wZoneNum; i++)
		{	
			if (dwNowday < pTOU->zZone[i].nMonth*31+pTOU->zZone[i].nDay)
			{
				break;
			}
		}
		
		if (i != 0)
		{
			wDayChart = pTOU->zZone[i-1].wDayChart;
		}
	}
	DTRACE(DB_AC, ("GetRate wDayChart2=%d.\r\n",wDayChart));

	if (wDayChart > pTOU->wDayChartNum)
		wDayChart = 0;
	DTRACE(DB_AC, ("GetRate wDayChart3=%d.\r\n",wDayChart));
		
	if (wDayChart != 0)  
	{
		wDayChart--;  //调整为数组的下标
	}
	DTRACE(DB_AC, ("GetRate wDayChart4=%d.\r\n",wDayChart));
	//else //在选不到日时段表号的情况下,取第一日时段表号
	BYTE bTmp[10];
	bTmp[0] = wDayChart;
	WriteItemEx(BANK2, POINT0, 0x1043, bTmp);
	
	DTRACE(DB_AC, ("GetRate wDayChart5=%d.\r\n",wDayChart));
	return GetRate(pTOU->rpDayChart[wDayChart], pTOU->wPeriodNum);
}

//描述:取得测量点wPn的当前费率号
//备注:对于国标的交采和脉冲来说,所有测量点都取F21,本函数不区分测量点
WORD GetRate(WORD wPn)
{
	DTRACE(DB_AC, ("GetRate:: wPn = %d, AcPn=%d.\n", wPn, GetAcPn()));
//	    if (wPn == PN0 || wPn == GetAcPn())	//交采测量点
    {
    	TTOU  m_Tou;
    	DTRACE(DB_AC, ("acpn:: wPn = %d.\n", wPn));
        memset((BYTE* )&m_Tou, 0, sizeof(TTOU));
        //InitRatePeriod(wPn);
        LoadTOU(PN0, &m_Tou);	//645参数只存在测量点0
    	return GetRate(&m_Tou);
    }

//		DTRACE(DB_AC, ("not ac pn!!!\n"));
//		BYTE bRate;
//		TTime now;
//		BYTE bBuf[50];
//		ReadItem(m_diRatePeriod, bBuf);
//		
//		GetCurTime(&now);
//		WORD wIdx = ((WORD )now.nHour*60 + now.nMinute) / 30;
//		if (wIdx >= 48)	//时间错乱
//			return 1;	//返回费率1
//	
//	#ifdef PRO_698
//		bRate = bBuf[wIdx];
//	#else
//		bRate = bBuf[wIdx>>1];
//		if (wIdx & 1)
//			bRate >>= 4;
//			
//		bRate &= 0x0f;
//	#endif
//			
//		if (bRate >= 4)	//最大支持4费率
//			return 1;
//		else
//			return bRate+1;
}

//描述:初始化测量点wPn的时段费率
//备注:对于国标的交采和脉冲来说,所有测量点都取F21,只需要初始化一遍
bool InitRatePeriod(WORD wPn)
{
//		m_diRatePeriod = GetItemEx(BN0, PN0, 0x015f);	//F21 终端电能量费率时段和费率数
	return true;
}
#ifdef SYS_LINUX
void SaveAttPara(TAttPara* pAcPara)
{
	BYTE bBuf[100];
	BYTE* pbBuf = bBuf;

	ReadItemEx(BN25, PN0, 0x500f, bBuf);

	DWORDToBCD(pAcPara->dwUn*10, pbBuf, 3);
	pbBuf += 3;
	
	DWORDToBCD(pAcPara->dwIn, pbBuf, 3);
	pbBuf += 3;

	*pbBuf = pAcPara->bConnectType;
	pbBuf++;
	
	DWORDToBCD(pAcPara->dwConst, pbBuf, 3);
	pbBuf += 3;

	WriteItemEx(BN25, PN0, 0x500f, bBuf);	//更新BN25 0x500f

	bBuf[0] = DT_UNSIGN;
	if(pAcPara->bConnectType==3)
	{
		bBuf[1] = 2;//三相三线
	}
	else
	{
		bBuf[1] = 3;//三相四线
	}
	WriteItemEx(BN0, PN0, 0x4010, bBuf);	
	TrigerSaveBank(BN25, 0, -1);
	DoTrigerSaveBank();
}

void LoadAttPara(WORD wPn, TAttPara* pAcPara)
{
	BYTE bBuf[100];
	BYTE bZeroBuf[10];
	
	memset(bBuf, 0, sizeof(bBuf));
	memset(bZeroBuf, 0, sizeof(bZeroBuf));

	//----------额定电压,电流,接线方式,脉冲常数---------
	int iLen = ReadItemEx(BN25, PN0, 0x500f, bBuf);
	if (iLen > 0)
	{
		if (memcmp(bBuf, bZeroBuf, 10) == 0)	//BN25扩展参数500f没有,则取参数3101
		{
		}
		else
		{
			pAcPara->dwUn = BcdToDWORD(bBuf, 3)/10; //NNNN.NN
			pAcPara->dwIn = BcdToDWORD(bBuf+3, 3); //NNN.NNN

			if (bBuf[6] == CONNECT_3P3W)
				pAcPara->bConnectType = CONNECT_3P3W;
			else if (bBuf[6] == CONNECT_1P)
				pAcPara->bConnectType = CONNECT_1P;
			else
				pAcPara->bConnectType = CONNECT_3P4W;

			pAcPara->dwConst = BcdToDWORD(bBuf+7, 3);
		}
	}

	/*if (iLen <=0 )	//参数500f没设置,则取F25
	{
		ReadItemEx(BN0, wPn, 0x019f, bBuf);
		pAcPara->dwUn =  Fmt7ToVal(bBuf+4, 2); //额定电压值
		pAcPara->dwIn =  Fmt22ToVal(bBuf+6, 1) * 100; //额定电流值
		pAcPara->bConnectType = GetConnectType(wPn);
		if (ReadItemEx(BN_645_PARA, MTRPN, 0xc030, bBuf) > 0)  //0xa010 3 脉冲常数,BCD码
			pAcPara->dwConst = BcdToDWORD(bBuf, 3);
		else
			pAcPara->dwConst = 6400;
		ReadItemEx(BN3, PN0, 0x30c0, bBuf);
		WriteItemEx(BN25, PN0, 0x5005, bBuf);

		SaveAttPara(pAcPara);
	}*/

	if (pAcPara->dwUn == 0)
		pAcPara->dwUn = 2200;	//额定电压,格式NNNNN.N
	if (pAcPara->dwIn == 0)
		pAcPara->dwIn = 5000;	//额定电流,格式NNN.NNN
	if (pAcPara->bConnectType==0 || pAcPara->bConnectType > 4)
		pAcPara->bConnectType=CONNECT_3P4W;
	if (pAcPara->dwConst == 0)
		pAcPara->dwConst = 6400;

	//有功电能累加模式
	if (ReadItemEx(BN10, PN0, 0xa122, bBuf) <= 0)  //有功电能计量方式选择
		bBuf[0] = 0;

				//		D2			|		D1			|			D0			|
				// 反向计入正向		| 	反向不计量		| 		各相累加模式	|
				// 0:不计入;1计入	| 0:计量;1不计量	| 0:代数和;1绝对值和	|
				// D2D1=00:正->正,反->反;正反向分别计量
				// D2D1=01:正->正;		只计正向,反向不计
				// D2D1=10:正反->正,反->反;
				// D2D1=11:正反->正		反向不计

	pAcPara->bEpMode = bBuf[0];
}

void SaveAdjPara(BYTE *pbBuf)
{
	WriteItemEx(BN25, PN0, 0x5005, pbBuf);  //0x5005 72 ATT7022校正参数
	TrigerSaveBank(BN25, 0, -1);
}

void LoadAdjPara(BYTE *pbBuf)
{
	ReadItemEx(BN25, PN0, 0x5005, pbBuf);  //0x5005 72 ATT7022校正参数
}

int LoadAdjParak(DWORD *dwK) //放大了100000倍
{
    BYTE bBuf[4];
    if (ReadItemEx(BN25, PN0, 0x5023, bBuf) > 0)
    {
        *dwK = bBuf[0]|(bBuf[1]<<8)|(bBuf[2]<<16)|(bBuf[3]<<24);
        return 4;
    }
    return -1;
}

void SaveNewAdjPara(BYTE* pbBuf)
{
	WriteItemEx(BN28, PN0, 0x001f, pbBuf);	//更新BN3 0x503f
	TrigerSaveBank(BN28, 0, -1);
	DoTrigerSaveBank();
}

void LoadNewAdjPara(BYTE *pbBuf)
{
	ReadItemEx(BN28, PN0, 0x001f, pbBuf);  //0x503f 
}


#endif
