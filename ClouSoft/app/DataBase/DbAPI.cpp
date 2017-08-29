/*********************************************************************************************************
* Copyright (c) 2007,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：DbAPI.cpp
* 摘    要：本文件主要实现协议相关的数据库标准接口之外的扩展接口
* 当前版本：1.0
* 作    者：岑坚宇
* 完成日期：2007年8月
*********************************************************************************************************/
#include "stdafx.h"
#include "Info.h"
#include "FaConst.h"
#include "DbConst.h"
#include "ComStruct.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include "MeterAPI.h"
#include "TaskManager.h"
#include "DataManager.h"
#include "LibAcConst.h"
#include "DbVer.h"
#include "MtrHook.h"
#include "CctAPI.h"
#include "ParaMgr.h"
#include "SchParaCfg.h"



//描述:获取直流模拟量测量点性质
//返回:0表示无效；>1表示模拟量测量点号
BYTE GetDCPnProp(WORD wPn)
{
	if (wPn >= GB_MAXMEASURE)
		return INVALID_POINT;

	BYTE bProp[50]={0};
	if (ReadItemEx(BN0, wPn, 0x8904, bProp) <= 0)
		return INVALID_POINT;
	if (bProp[0] == 0)
		return INVALID_POINT;	

	return bProp[0];
}

//描述:直流模拟量是否有效
bool IsDCPnValid(WORD wPn)
{
	return GetDCPnProp(wPn) != INVALID_POINT;
}

//描述:直流模拟量1类Fn
bool IsDcC1Fn(BYTE bFn)
{
	if (bFn==73 || bFn==121)
		return true;

	return false;
}

//描述:直流模拟量2类Fn
bool IsDcC2Fn(BYTE bFn)
{
	if (bFn==129 || bFn==130 || bFn==138)
		return true;

	return false;
}

//描述:获取非直流模拟量测量点性质
//返回:0表示无效；2表示电表，1表示脉冲//，4表示模拟量
BYTE GetPnProp(WORD wPn)
{	
	if (wPn >= POINT_NUM)
		return INVALID_POINT;

	BYTE bProp[PNPARA_LEN]={0};
	if (ReadItemEx(BN0, wPn, 0x6000, bProp) <= 0)
		return INVALID_POINT;

	if (bProp[2]==PN_PROP_METER && IsMtrValid(ByteToWord(bProp)) )
		return PN_PROP_METER;

	else if (bProp[2] == PN_PROP_AC)
		return PN_PROP_AC;

	else if (bProp[2] == PN_PROP_PULSE)
	{
		bool fAllOk = true;

		for (BYTE i=0; i<bProp[3]; i++) //只要所配脉冲的任意属性无效，则此测量点无效
		{
			if ( !IsPluseValid(ByteToWord(&bProp[4+2*i])) ) 
				fAllOk = false;
		}

		if (fAllOk) 
			return PN_PROP_PULSE;
	}
	else if (bProp[2] == PN_PROP_CCT)
		return PN_PROP_CCT;

	else if (bProp[2] == PN_PROP_RJ45)//网络RJ45抄表
		return PN_PROP_RJ45;

	else if (bProp[2] == PN_PROP_EPON)//光纤端口
		return PN_PROP_RJ45;//PN_PROP_EPON;

	else if (bProp[2] == PN_PROP_BBCCT)//宽带载波通道
		return PN_PROP_BBCCT;

	return INVALID_POINT;
}

//描述:取PN对应的端口号
//返回:如果正确则返回端口号,否则返回0
BYTE GetPnPort(WORD wPn)
{
	TOobMtrInfo tMtrInfo;
	//BYTE bBuf[128];
	//BYTE bPortSn;

	if (!GetMeterInfo(wPn, &tMtrInfo))
		return 0xff;

	if (MtrPnToSn(wPn) > 0)
	{
// 		memset(bBuf, 0, sizeof(bBuf));
// 		if (ReadItemEx(BN0, PN0, tMtrInfo.dwPortOAD, bBuf) <= 0)
// 			return false;
// 		if (IsAllAByte(bBuf, 0, sizeof(bBuf)))	
		{
			if (tMtrInfo.dwPortOAD == 0xF2010201)	//485-1
				//return PORT_GB485;
				return LOGIC_PORT_MIN;		//1
			else if (tMtrInfo.dwPortOAD == 0xF2010202)	//485-2
				//return PORT_GB485;	
				return LOGIC_PORT_MIN+1;	//2
			else if (tMtrInfo.dwPortOAD == 0xF2080201)	//AC
				return PORT_AC;
			else if ((tMtrInfo.dwPortOAD&0xF2090200) == 0xF2090200)	//PLC
				return PORT_CCT_PLC;
		}
	}

	return 0;
}

//描述:取得测量点的电表协议类型
BYTE GetPnMtrPro(WORD wPn)
{	
	TOobMtrInfo tMtrInfo;
	//BYTE bBuf[128];

	if (!GetMeterInfo(wPn, &tMtrInfo))
		return 0xff;

	if (MtrPnToSn(wPn) > 0)
	{
		return tMtrInfo.bProType;
	}

	return 0;
}

//描述:取得有效配置测量点个数
//返回:
//配置格式如下：
//02 04 
//		12 00 04 
//		02 07 
//			85 06 10 00 00 00 22 24 
//			16 03 
//			16 03 
WORD GetValidPnNum()
{
	return g_wValidPnNum;
}

//描述:获取总加组性质
//返回:0表示无效；大于0表示有效；
BYTE GetGrpProp(WORD wPn)
{
	if (wPn >= GB_MAXSUMGROUP)
		return INVALID_POINT;

	WORD wLen;
	BYTE *pbBuf, bType, bBuf[120];
	if (ReadItemEx(BN0, wPn, 0x2301, bBuf) > 0)
	{		
		const ToaMap* pOadMap = GetOIMap(0x23010200);
		for (BYTE i=0; i<bBuf[1]; i++)
		{
			pbBuf = OoGetField(bBuf, pOadMap->pFmt, pOadMap->wFmtLen, i, &wLen, &bType);
			if (pbBuf == NULL)
				return INVALID_POINT;

			if (MtrAddrToPn(pbBuf+3, pbBuf[3]+1)>0 || PulseAddrToPn(pbBuf+3, pbBuf[3]+1)>0) //电表测量点号 || 脉冲测量点
				continue;
			else
				return INVALID_POINT;
		}
		return bBuf[1];
	}

	return INVALID_POINT;
}

#ifdef PRO_698
void SetFnFlg(BYTE* pbFnFlg, BYTE bFn)
{
	if (bFn==0 || bFn>248)
		return;

	pbFnFlg[(bFn-1)>>3] |= 1 << ((bFn-1)&0x07);
}

void SetFnFlg(BYTE* pbFnFlg, const BYTE* pbFn, WORD wNum)
{
	for (WORD i=0; i<wNum; i++)
	{
		SetFnFlg(pbFnFlg, pbFn[i]);
	}
}


//描述:设置1个大类号的默认支持FN
void SetDefFnCfgOfMain(BYTE bMain)
{
	WORD wOff;
	BYTE bBuf[C2_CFG_LEN+10];
	BYTE bVer;
	ReadItemEx(BN1,PN0,0x2000,&bVer);

	memset(bBuf, 0, sizeof(bBuf));
	bBuf[0] = bMain;   //大类
	bBuf[1] = 2;       //小类数

	//第0组
	wOff = 2;
	//bBuf[wOff++] = 0;        //小类号
	//bBuf[wOff++] = 31;       //信息类组数
	//SetFnFlg(&bBuf[wOff], bM2S1Fn, sizeof(bM2S1Fn));
	wOff += 33;

	//第1组	
	bBuf[wOff++] = 1;        //小类号
	bBuf[wOff++] = 31;       //信息类组数

#ifdef VER_HEBEI
	{
		SetFnFlg(&bBuf[wOff], g_bSingleFn_HeB, sizeof(g_bSingleFn_HeB));
	}
#endif

#ifdef VER_JIBEI
	{
		SetFnFlg(&bBuf[wOff], g_bSingleFn_JiBei, sizeof(g_bSingleFn_JiBei));
	}
#endif 

#ifdef VER_ZJ
	{
		SetFnFlg(&bBuf[wOff], g_bSingleFn, sizeof(g_bSingleFn));
	}
#endif

	wOff += 31;

	//第2组
	bBuf[wOff++] = 2;		 //小类号
	bBuf[wOff++] = 31;       //信息类组数


#ifdef VER_HEBEI
	{
		SetFnFlg(&bBuf[wOff], g_bMultiFn_HeB, sizeof(g_bMultiFn_HeB));
	}
#endif

#ifdef VER_JIBEI
	{
		SetFnFlg(&bBuf[wOff], g_bMultiFn_JiBei, sizeof(g_bMultiFn_JiBei));
	}
#endif

#ifdef VER_ZJ
	{
		SetFnFlg(&bBuf[wOff], g_bMultiFn, sizeof(g_bMultiFn));
	}
#endif

	WriteItemEx(BN0, bMain, 0x027f, bBuf);	
}

#if FA_TYPE == FA_TYPE_K32

//485端口默认参数
void SetDefault485PortPara()
{	
	int iLen;
	WORD wPn;
	BYTE bBuf[100];
	const WORD wID = 0xF201;	//485端口
	bool fTrigerSave = false;

	const BYTE bMtrRd485PortCfg[] = {
				DT_STRUCT, 0x03, 
					DT_VIS_STR, 0x10,	//端口描述符
						'0', '0', '0', '0', '0', '0', '0', '0',
						'0', '0', '0', '0', '0', '0', '0', '0',
					DT_COMDCB,	//端口参数
						0x03,	//2400bps --- 抄表口波特率默认2400
						0x02,	//偶校验
						0x08,	//8位数据位
						0x01,	//停止位
						0x00,	//流控
					DT_ENUM,	//端口功能
						0x01,	//上行通信（0），抄表（1），级联（2），停用（3）
};	//485抄表口默认参数

	const BYTE bLocal485PortCfg[] = {
				DT_STRUCT, 0x03, 
					DT_VIS_STR, 0x10,	//端口描述符
						'0', '0', '0', '0', '0', '0', '0', '0',
						'0', '0', '0', '0', '0', '0', '0', '0',
					DT_COMDCB,	//端口参数
						0x06,	//9600bps --- 维护口波特率默认9600
						0x02,	//偶校验
						0x08,	//8位数据位
						0x01,	//停止位
						0x00,	//流控
					DT_ENUM,	//端口功能
						0x00,	//上行通信（0），抄表（1），级联（2），停用（3）
	};	//485维护口默认参数

	for (wPn=0; wPn<LOGIC_PORT_NUM; wPn++)
	{
		memset(bBuf, 0, sizeof(bBuf));	
		iLen = ReadItemEx(BN0, wPn, wID, bBuf);
		if (iLen>0 && bBuf[0]!=DT_STRUCT)
		{
			if (wPn == LOGIC_PORT_NUM-1)	//第3路485口
				memcpy(bBuf, bLocal485PortCfg, sizeof(bMtrRd485PortCfg));	//485维护口
			else
				memcpy(bBuf, bMtrRd485PortCfg, sizeof(bMtrRd485PortCfg));	//485抄表口

			WriteItemEx(BN0, wPn, wID, bBuf);
			fTrigerSave = true;
		}
	}

	if (fTrigerSave)
		TrigerSaveBank(BN0, SECT11, -1);
}


//参数初始化后/硬盘格式化后默认参数
void SetDefaultPara()
{
	SetDefault485PortPara();
}

#endif


//描述:设置默认大小类支持项配置
void SetDefFnCfg()
{
	BYTE bBuf[C2_CFG_LEN+10];

	int ilen = ReadItemEx(BN0, 1, 0x027f, bBuf); //大型专变用户	
	if (ilen>0 && bBuf[0]==0xff)		
		SetDefFnCfgOfMain(1);    

	ilen = ReadItemEx(BN0, 2, 0x027f, bBuf); //中型专变用户		
	if (ilen>0 && bBuf[0]==0xff)	
		SetDefFnCfgOfMain(2);

	ilen = ReadItemEx(BN0, 3, 0x027f, bBuf); //低压三相一般工商业用户	
	if (ilen>0 && bBuf[0]==0xff)				
		SetDefFnCfgOfMain(3);

	ilen = ReadItemEx(BN0, 4, 0x027f, bBuf); //低压单相一般工商业用户		
	if (ilen>0 && bBuf[0]==0xff)
		SetDefFnCfgOfMain(4);

	ilen = ReadItemEx(BN0, 5, 0x027f, bBuf); //居民用户	
	if (ilen>0 && bBuf[0]==0xff)		
		SetDefFnCfgOfMain(5);

	ilen = ReadItemEx(BN0, 6, 0x027f, bBuf); //公用配变考核计量点		
	if (ilen>0 && bBuf[0]==0xff)		
		SetDefFnCfgOfMain(6);		
}

//描述:此测量点是否支持此Fn
bool IsFnSupport(WORD wPn, BYTE bFn, BYTE bClass)
{
	WORD i;
	BYTE bMain;
	BYTE bSub;

	BYTE bVer;
	ReadItemEx(BN1,PN0,0x2000,&bVer);
	
	if (bFn==161 && bClass==2)
		return true;

	if (!GetUserType(wPn, &bMain, &bSub)) //获取用户用户大类号和小类号
		return false;

	WORD wID = (bClass==1 ? 0x026f : 0x027f);
	const BYTE* pbCfg = GetItemRdAddr(BN0, bMain, wID);
	if (pbCfg==NULL)
		return false;

	if (bMain>=USR_MAIN_CLASS_NUM || bSub>=USR_SUB_CLASS_NUM || bFn==0)
		return false;

	if (pbCfg[0]==0xff && bMain==0)	//参数没有配置,用户大类号为0,则全部支持
	{
#ifdef VER_HEBEI
		{
			for (i=0; i<sizeof(g_bMultiFn_HeB); i++)
			{
				if (bFn == g_bMultiFn_HeB[i])
				return true;
			}
		}
#endif
#ifdef  VER_JIBEI
		{
			for (i=0; i<sizeof(g_bMultiFn_JiBei); i++)
			{
				if (bFn == g_bMultiFn_JiBei[i])
					return true;
			}
		}
#endif

#ifdef VER_ZJ
		{
			for (i=0; i<sizeof(g_bMultiFn); i++)
			{
				if (bFn == g_bMultiFn[i])
				return true;
			}
		}
#endif
		return false;
	}
	else if (pbCfg[0] >= USR_MAIN_CLASS_NUM)
		return false;

	//BYTE m = pbCfg[1];
	BYTE n;
	BYTE bMark = 1 << ((bFn-1)&0x07);
	BYTE pos = (bFn-1)>>3;
	const BYTE* p = &pbCfg[2+(WORD )bSub*33];	//指到相应组的用户小类号
	if ((bSub!=0) && (*p==bSub))	//用户小类号相等,如果小类号为0有配置,则按照这里的进行判断
	{
		n = p[1];	 //组数n
		if (pos >= n)
			return false;

		if ((bMark & p[2+pos]) != 0)
			return true;
		else
			return false;
	}
	else if (bSub == 0)	//小类号为0,默认为用户大类定义的所有数据配置项
	{
		BYTE bBuf[32];
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD sub=1; sub<USR_SUB_CLASS_NUM; sub++)
		{
			p = &pbCfg[2+(WORD )sub*33]; //指到相应组的用户小类号

			if (*p == sub)	//用户小类号相等
			{
				p++;	//跳过小类号
				n = *p++;	 //组数n
				if (n > 31)
					continue;

				for (i=0; i<n; i++)
				{
					bBuf[i] |= *p++;
				}
			}
		}

		if ((bMark & bBuf[pos]) != 0)
			return true;
		else
			return false;

	}

	return false;
}

#ifdef EN_SBJC_V2
//描述:获取用户类型
BYTE GetMeterType(WORD wPn)
{  
    TOobMtrInfo tMtrInfo;
	//BYTE bBuf[PNPARA_LEN];

	if (!IsMtrPn(wPn))
        return 0;

	if (!GetMeterInfo(wPn, &tMtrInfo))
		return 0xff;

	if (MtrPnToSn(wPn) > 0)
	{
		return tMtrInfo.bUserType;
	}

	return 0;
}

//描述:获取子协议
BYTE GetMeterSubPro(WORD wPn)
{  
	TOobMtrInfo tMtrInfo;
	//BYTE bBuf[PNPARA_LEN];

	//if (!IsMtrPn(wPn))
	//	return 0;

	if (!GetMeterInfo(wPn, &tMtrInfo))
		return 0xff;

	if (MtrPnToSn(wPn) > 0)
	{
		return tMtrInfo.bUserType;//tMtrInfo.bRate;
	}

	return 0;
}
#endif

//描述:获取用户类型
BYTE GetUserType(WORD wPn)
{
	BYTE bBuf[100];

	//TODO:是否需要判断测量点是否有效?
	// 	   目前只能假定进来前就已经判断是有效的,因为从返回值没法判断

	ReadItemEx(BN0, wPn, 0x8902, bBuf);
//	return bBuf[F10_SN_LEN+66];
	return bBuf[16];
}

//描述:获取用户用户大类号和小类号
bool GetUserType(WORD wPn, BYTE* pbMain, BYTE* pbSub)
{
	if (!IsPnValid(wPn))
		return false;

	BYTE bBuf[100];
	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return false;

//	BYTE bType = bBuf[F10_SN_LEN+66];
	BYTE bType = bBuf[16];
	*pbMain = (bType>>4) & 0x0f; 
	*pbSub = bType&0x0f;

	return true;
}

//描述:获取用户大类号
BYTE GetUserMainType(WORD wPn)
{
	BYTE bType = GetUserType(wPn);
	return (bType>>4) & 0x0f; 
}


//描述:获取用户小类号
BYTE GetUserSubType(WORD wPn)
{
	BYTE bType = GetUserType(wPn);
	return bType&0x0f; 
}
#endif

//描述:看测量点是否是某种类型的测量点
bool IsPnType(WORD wPn, WORD wType)
{
	BYTE bProp = GetPnProp(wPn);
	if (bProp == PN_PROP_METER || bProp == PN_PROP_RJ45 || bProp == PN_PROP_EPON)
	{
		if (wType & PN_TYPE_MTR)
			return true;
	}
	else if (bProp == PN_PROP_AC)
	{
		if (wType & PN_TYPE_AC)
			return true;
	}
	else if (bProp == PN_PROP_PULSE)
	{
		if (wType & PN_TYPE_PULSE)
			return true;
	}
	else if (bProp == PN_PROP_CCT)
	{
		if (wType & PN_TYPE_CCT)
			return true;
	}

	if (wType & PN_TYPE_DC)
	{
		if(IsDCPnValid(wPn))
			return true;
	}

	if (wType & PN_TYPE_GRP)
	{
		if (IsGrpValid(wPn))
			return true;
	}

	if (wType & PN_TYPE_P0)
	{
		if (wPn == PN0)
			return true;
	}

	return false;
}

bool IsMtrPn(WORD wPn)
{
	BYTE bBuf[PNPARA_LEN];
	memset(bBuf, 0,sizeof(bBuf));	

	if (ReadItemEx(BN0, wPn, 0x6000, bBuf) <= 0)
		return false;

	if (MtrPnToSn(wPn) > 0)
	{
		BYTE bAddL = bBuf[9] + 1;
		if (bBuf[14+bAddL]==0xF2 && bBuf[15+bAddL]==0x01) //485端口OAD
		{
			return true;
		}
	}

	return false;
}

//描述:总加组是否有效
bool IsGrpValid(WORD wPn)
{
	return GetGrpProp(wPn) != INVALID_POINT;
}

//描述:是否为总加组的1类数据Fn
bool IsGrpC1Fn(BYTE bFN)
{
	if ((bFN>=17 && bFN<=24) || (bFN>=81 && bFN<=84))
		return true;

	return false;
}

//描述:是否为总加组的2类数据Fn
bool IsGrpC2Fn(BYTE bFN)
{
	if ((bFN>=57 && bFN<=62) || (bFN>=65 && bFN<=66) || (bFN>=73 && bFN<=76))
		return true;

	return false;
}

//描述:获取事件的属性
//返回:0表示此事件无效,1表示重要事件,2表示一般事件
BYTE GetErcType(BYTE bErc)
{
	BYTE bErcType[20];
	if (bErc>64 || bErc==0)
		return 0;
	
	BYTE i=(bErc-1)>>3;
	BYTE bit = (bErc-1)&7;
	if (ReadItemEx(BN0, PN0, 0x40d0, bErcType) >= 0) //F9：终端事件记录配置设置
	{
		if (!((bErcType[i+1]>>bit)&1)) //事件记录有效标志位 bitstring 1字节
			return 0;
	
		if ((bErcType[i+10]>>bit)&1)	//事件重要性等级标志位
			return 1;	//1表示重要事件
		else						
			return 2;	//2表示一般事件
	}

	return 0;
}

//描述:取得测量点的接线方式
BYTE GetConnectType(WORD wPn)
{
	BYTE bBuf[32];
	ReadItemEx(BN0, wPn, 0x8902, bBuf);
	if (bBuf[17]==1)
		return CONNECT_3P3W;
	else if (bBuf[17]==2)
		return CONNECT_3P4W; 
	else
		return CONNECT_1P;
	/*
	ReadItemEx(BN0, wPn, 0x4100, bBuf);  //F25
	if (bBuf[F25_CONN_OFFSET] == 1)		 //三项三线
		return CONNECT_3P3W; //终端接线方式 1	1:单相;3:三项三线;4:三相四线
	else if (bBuf[F25_CONN_OFFSET] == 3) 	 //单相表
		return CONNECT_1P;
	else //if (bBuf[F25_CONN_OFFSET] == 2) //三相四线
		return CONNECT_3P4W; 
*/
}


#ifdef EN_AC
void CopyAcData(WORD wFromPn, WORD wToPn)
{
	WORD wID[] = {
		0x901f,0x902f,0x907f,0x908f,0x911f,0x912f,0x913f,0x914f,0x915f,0x916f,0x917f,0x918f,
		0x941f,0x942f,0x947f,0x948f,0x951f,0x952f,0x953f,0x954f,0x955f,0x956f,0x957f,0x958f,
		0x981f,0x982f,0x987f,0x988f,0x991f,0x992f,0x993f,0x994f,0x995f,0x996f,0x997f,0x998f,
		0xa01f,0xa02f,0xa11f,0xa12f,0xa13f,0xa14f,0xa15f,0xa16f,
		0xa41f,0xa42f,0xa51f,0xa52f,0xa53f,0xa54f,0xa55f,0xa56f,
		0xa81f,0xa82f,0xa91f,0xa92f,0xa93f,0xa94f,0xa95f,0xa96f,
		0xb01f,0xb02f,0xb11f,0xb12f,0xb13f,0xb14f,0xb15f,0xb16f,
		0xb41f,0xb42f,0xb51f,0xb52f,0xb53f,0xb54f,0xb55f,0xb56f,
		0xb81f,0xb82f,0xb91f,0xb92f,0xb93f,0xb94f,0xb95f,0xb96f,
		0xb21f,0xb31f,0xb32f,0xb33f,0xb34f};

		int iLen;
		BYTE bBuf[256];
		for (WORD i=0; i<sizeof(wID)/sizeof(WORD); i++)
		{
			iLen = ReadItemEx(BN0, wFromPn, wID[i], bBuf); 
			if (iLen > 0)
				WriteItemEx(BN0, wToPn, wID[i], bBuf); 
		}
}
#endif


//描述:初始化交采测量点,自动按照0xa044(交采默认测量点号)配置交采测量点参数F10
void InitAcPn()
{
//#ifndef	SYS_WIN	//在实际终端上才自动配置交采测量点	
	
//#endif	//EN_AC
}

//#ifdef VER_ZJ
//同步终端地址到扩展参数
//注意该函数必须在外部软件版本有变更
void SyncTermAddr()
{
	BYTE bTmp[60];
	BYTE bBuf[60];

	if (g_bInnerSoftVer[16]=='0' && g_bInnerSoftVer[17]=='0' && g_bInnerSoftVer[18]=='1' && g_bInnerSoftVer[19]=='g')	//内部软件版本号
	{
		DTRACE(DB_CRITICAL, ("SaveSoftVerChg update term addr to BN10, A1DO.\r\n"));
		memset(bBuf, 0, sizeof(bBuf));
		memset(bTmp, 0, sizeof(bTmp));
		ReadItemEx(BN0, PN0, 0x4001, bBuf);
		ReadItemEx(BN10, PN0, 0xa1d0, bTmp);
		if (memcmp(bBuf, bTmp, 17) != 0)	//不同
		{
			WriteItemEx(BN10, PN0, 0xa1d0, bBuf);	//将终端地址参数4001更新到BN10, a1d0
			TrigerSaveBank(BN10, 0, -1);
		}
	}
}
//#endif


//终端软件版本变更保存
void SaveSoftVerChg()
{
	BYTE bBuf[128];
	
	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BANK0, PN0, 0x4303, bBuf);
	if (memcmp(bBuf, g_bTermSoftVer, sizeof(g_bTermSoftVer)) != 0)
	{
		WriteItemEx(BANK0, PN0, 0x4303, g_bTermSoftVer);
		TrigerSaveBank(BANK0, SECT_PARAM, -1);
		//生成软件版本变更事件
		SetInfo(INFO_TERM_VER_CHG);

		//#ifdef VER_ZJ
		//特别注意：如外部软件版本或日期未变更，程序跑不到这里！！！
		SyncTermAddr();
		//#endif
		
	}
}



//开机自动应用配置文件
void ApllyCfgAuto(void)
{
	char command[64] = {0};
	BYTE bCfgBuf[256] = {0};
	char  szPathNameEx[PATHNAME_LEN+1] = { 0 };
	int iCfgLen = readfile(USER_CFG_PATH"cfgpermit.cfg", bCfgBuf, sizeof(bCfgBuf));
	int iRet = 0;

	DTRACE(DB_FA, ("ApllyCfgAuto Step0 iCfgLen=%d.\r\n", iCfgLen));
	if (iCfgLen <= 0)
		return;

	if ('\n'==bCfgBuf[iCfgLen-1] || '\a'==bCfgBuf[iCfgLen-1] || '\r'==bCfgBuf[iCfgLen-1])
	{
		bCfgBuf[iCfgLen-1] = 0;
	}

	memcpy(szPathNameEx, bCfgBuf, iCfgLen);
	DTRACE(DB_FA, ("ApllyCfgAuto szPathNameEx=%s!\r\n", szPathNameEx));
	iRet = g_pmParaMgr.LoadPara(szPathNameEx);
	if ( iRet == 0)
	{
		DTRACE(DB_FA, ("ApllyCfgAuto Step1 start to apply dft.\r\n"));
		g_pmParaMgr.Parse();

		sprintf(command, "rm -rf "USER_CFG_PATH"cfgpermit.cfg");
		system(command);

		TrigerSavePara();
		while (GetClick()<=10)	//上电超过10秒才会触发保存
		{
			Sleep(50);
		}
		
		DoFaSave();
		Sleep(100);

		ResetCPU();
	}

	DTRACE(DB_FA, ("ApllyCfgAuto Step2 iRet = %d exit.\r\n", iRet));
}


//终端软件版本变更检测
void TermSoftVerChg()
{
#if 0
	memset((BYTE*)&g_SoftVerChg, 0, sizeof(g_SoftVerChg)); //全局的缓存区

	BYTE  bSoftVer[SOFT_VER_LEN];
 
 	ReadItemEx(BN0, PN0, 0x100f, bSoftVer);
 	if (memcmp(bSoftVer+16, g_bSoftVer+16, 4) != 0)//只要比较版本号？？
 	{
		g_SoftVerChg.time = GetCurTime();
		g_SoftVerChg.bVerInfo[0] = 0x08; //bit 位数
		g_SoftVerChg.bVerInfo[1] = 0x02;
		memcpy(&g_SoftVerChg.bVerInfo[2], &bSoftVer[16], 4);		//数据库之前保存的版本
		memcpy(&g_SoftVerChg.bVerInfo[6], &g_bSoftVer[16], 4);		//当前的软件版本
		//SaveAlrData(ERC_INIT_VER, tm, bBuf);	//写版本变更事件			
		WriteItemEx(BN0, PN0, 0x100f, g_bSoftVer);
		TrigerSaveBank(BN0, SECT_KEEP_PARA, 0);
 	}

// 	int iRet= -1;
// 	iRet =WriteItemEx(BANK0, PN0, 0x4303, g_bTermSoftVer);

#endif
}

void PostDbInit()
{ 	
#ifdef PRO_698
//	SetDefFnCfg();		//设置默认大小类支持项配置
	InitMtrSnToPn();	//本函数必须放到InitAcPn()前,因为涉及到装置序号到测量点号的映射
#if FA_TYPE == FA_TYPE_K32
	SetDefaultPara();	//参数初始化/格式化硬盘后需默认的参数
#endif

#endif

	TermSoftVerChg();

	//SaveAdjParaAferFormat();
	//InitAcPn();

	UpdMeterPnMask();
	UpdPnMask();
	ClrPnChgMask();
#ifdef EN_CCT
	CctPostDbInit();
#endif

	TrigerSaveBank(BN11, 0, -1); //即时统计的起点数据
}

extern TBankCtrl g_Bank0Ctrl[SECT_NUM];
extern TBankCtrl g_BankCtrl[BANK_NUM];
//extern TPnMapCtrl g_PnMapCtrl[PNMAP_NUM];
TDbCtrl g_DbCtrl; //外界对数据库进行参数配置的数据库控制结构

//描述:系统数据库初始化
bool InitDB(void)
{
	memset(&g_DbCtrl, 0, sizeof(g_DbCtrl));

	//BANK0的控制字段
	g_DbCtrl.wSectNum = SECT_NUM;	//BANK0中的SECT数目
	g_DbCtrl.pBank0Ctrl = g_Bank0Ctrl;

	//BANK控制字段
	g_DbCtrl.wBankNum = BANK_NUM;	//支持的BANK数目
	g_DbCtrl.pBankCtrl = g_BankCtrl;

	g_DbCtrl.iSectImg = -1;			//485抄表数据镜像段,如果没有则配成-1
	g_DbCtrl.wImgNum = 0;				//485抄表数据镜像个数
	g_DbCtrl.wSectPnData = 0;	//对于485抄表镜像数据,需要有测量点数据与之对应,否则本参数配置成0即可

	//测量点动态映射控制字段
//	g_DbCtrl.wPnMapNum = PNMAP_NUM;		//支持的映射方案数目,整个数据库不支持测量点动态映射则设为0
	g_DbCtrl.wPnMapNum = 0;		//支持的映射方案数目,整个数据库不支持测量点动态映射则设为0
	g_DbCtrl.pPnMapCtrl = NULL;//g_PnMapCtrl;	//整个数据库不支持测量点动态映射则设为NULL

	g_DbCtrl.wPnMaskSize = PN_MASK_SIZE; //测量点屏蔽位的大小,用来分配电表测量点屏蔽位空间
	g_DbCtrl.pszDbPath = USER_PARA_PATH; //系统库一些控制文件的存放目录,一般配置为USER_PARA_PATH

	g_DbCtrl.wSaveInterv = 15;			//保存间隔,单位分钟
	if (!InitDbLib(&g_DbCtrl)) //版本变更事件用到任务库
		return false;

	PostDbInit();
	
	return true;
}

//清测量点(电表,交采,脉冲)数据
void ClrPnData(WORD wPn)
{
	wPn;
#if 0
	static WORD wBank0Id[] = {
		0x11Df, 0x11ef, 0x120f, 0x121f, //当日累计
		0x122f, 0x123f, 0x124f, 0x125f, //当月累计								
		0x126f, 0x127f, 0x128f, //谐波数据不在测量点数据里，要单独清除，角度？
		0x300f, 0x301f, 0x302f, 0x303f, //C2F5+,C2F6+,C2F7+,C2F8+		//上一日
		0x304f, 0x305f, 0x306f, 0x307f, //C2F21+,C2F22+,C2F23+,C2F24+ //上一月
		0x308f, 0x309f, 0x30Af, 0x30Bf, 0x30Cf, 0x30Df, 0x30Ef, //C2F25+~C2F38+ //上一日(月)统计
		0x310f, 0x311f, 0x312f, 0x313f, 0x314f, 0x315f, 
		0x316f, 0x317f, 0x318f, 0x319f, //C2F41+~C2F44+ 上一日(月)功率因素及电容
		0x32af, 0x32bf, 0x32cf, 0x32df, //C2F97+~C2F100+ 上一曲线点
		0x32ef, 0x330f, 0x331f, 0x332f, 0x333f, 0x334f, 0x335f, 0x336f, 0x337f,};////C2F113+~C2F130+ //上一日(月)谐波统计

		BYTE bBuf[256];
		if (wPn >= POINT_NUM)
			return;

		DTRACE(DB_DP, ("ClrPnData() wPn=%d******2! \r\n", wPn));

		ClearBankData(BN0, SECT_PN_DATA, wPn);
		//ClearBankData(BN0, SECT_IMG, wPn);
		ClearBankData(BN0, SECT_EXT_PN_DATA, wPn);
		memset(bBuf, 0, sizeof(bBuf));
		for (WORD i=0; i<sizeof(wBank0Id)/sizeof(WORD); i++)
		{
			WriteItemEx(BN0, wPn, wBank0Id[i], bBuf, (DWORD )0);	//清数据清时间
		}

		static WORD wBank11Id[] = {//0x003f, 0x004f, 0x005f, 0x006f, //C1F41+,C1F42+,C1F43+,C1F44+ //当日起点
			//0x009f, 0x00af, 0x00bf, 0x00cf, //C1F45+,C1F46+,C1F47+,C1F48+	 //当月起点
			0x00df, 0x00ef, 0x010f, 0x011f, 0x012f, 0x013f, 0x014f, 0x015f, //C2F27+~C2F38+ //当日统计
			0x017f, 0x018f, 0x019f, 0x01af, 0x01bf, 0x01cf, 0x01df, 0x01ef, 0x020f, 0x021f, 
			// 0x031f, 0x032f, 0x033f, 0x034f,//C2F97+~C2F100+ //曲线起点
			0x036f, 0x037f, 0x038f, 0x039f, 0x03af, 0x03bf, 0x03cf, 0x03df,}; //C2F113+~C2F123+ //当日(月)谐波统计
			for (WORD i=0; i<sizeof(wBank11Id)/sizeof(WORD); i++)
			{
				WriteItemEx(BN11, wPn, wBank11Id[i], bBuf, (DWORD )0);	//清数据清时间
			}

			static WORD wBank18Id[] = {0x003f, 0x004f, 0x005f, 0x006f, //C1F41+,C1F42+,C1F43+,C1F44+ //当日起点
				0x009f, 0x00af, 0x00bf, 0x00cf, //C1F45+,C1F46+,C1F47+,C1F48+	 //当月起点
				0x031f, 0x032f, 0x033f, 0x034f,//C2F97+~C2F100+ //曲线起点
				0x040f, 0x041f, 0x042f, 0x043f, 0x044f, 0x045f, 0x046f, 0x047f,//测量点日月起点时的累计值
			};
			for (WORD i=0; i<sizeof(wBank18Id)/sizeof(WORD); i++)
			{
				WriteItemEx(BN18, wPn, wBank18Id[i], bBuf, (DWORD )0);	//清数据清时间
			}
#endif
}



//清脉冲测量点的相关数据
#define MAX_IDS_NUM		10
typedef struct
{
	WORD	wId;						//电能示值ID
	WORD	wPnDataIds[MAX_IDS_NUM];	//电能示值ID
	WORD	wBank0Ids[MAX_IDS_NUM];		//数据项标识
	WORD	wBank11Ids[MAX_IDS_NUM];	//数据项的数据长度
	WORD	wBank18Ids[MAX_IDS_NUM];	//记录的数据长度,为有效的数据项的长度总和
}TPulseEngInfo;

void ClrPulsePnData(WORD wPn)
{
	wPn;
#if 0
	static WORD wBank0Id[] = { //非脉冲属性相关的							 							
		0x126f, 0x127f, 0x128f, //谐波数据不在测量点数据里，要单独清除，角度？
		0x302f, 0x303f, //C2F7+,C2F8+		//上一日					   		  
		0x30Af, 0x30Bf, 0x30Cf, 0x30Df, 0x30Ef, //C2F27+~C2F38+ //上一日(月)统计
		0x312f, 0x313f, 0x314f, 0x315f, 
		0x316f, 0x317f, 0x318f, 0x319f, //C2F41+~C2F44+ 上一日(月)功率因素及电容							  
		0x32ef, 0x330f, 0x331f, 0x332f, 0x333f, 0x334f, 0x335f, 0x336f, 0x337f,};////C2F113+~C2F130+ //上一日(月)谐波统计

		static WORD wBank11Id[] = {					   		 
			0x00df, 0x00ef, 0x012f, 0x013f, 0x014f, 0x015f, //C2F27+~C2F38+ //当日统计
			0x019f, 0x01af, 0x01bf, 0x01cf, 0x01df, 0x01ef, 0x020f, 0x021f, 							
			0x036f, 0x037f, 0x038f, 0x039f, 0x03af, 0x03bf, 0x03cf, 0x03df,}; //C2F113+~C2F123+ //当日(月)谐波统计

			//普通任务的控制结构:每两行一项
			static TPulseEngInfo g_PulseEngInfo[4] =
			{
				//wId;		wPnDataIds[MAX_IDS_NUM];			wBank0Ids[MAX_IDS_NUM];			wBank11Ids[MAX_IDS_NUM];			wBank18Ids[MAX_IDS_NUM]
				{0x901f,	{0x901f, 0x941f, 0xa01f, 0xa41f, 0xb01f, 0xb41f},//当前(上月)电能、当前(上月)需量、当前(上月)需量时间
				{0x11Df, 0x122f, 0x300f, 0x304f, 0x308f, 0x309f, 0x310f, 0x311f, 0x32af},//当日(月)累计、上日(月)累计、上一日(月)统计、上一曲线点	
				{0x010f, 0x011f, 0x017f, 0x018f,},//C2F25+~C2F26+ //上一日(月)统计
				{0x003f, 0x009f, 0x031f, 0x040f, 0x044f,},//C1F41+ //当日(月)起点、曲线起点、测量点日(月)起点时的累计值
				},
				{0x911f,	{0x911f, 0x951f, 0xa11f, 0xa51f, 0xb11f, 0xb51f},//当前(上月)电能、当前(上月)需量、当前(上月)需量时间
				{0x11Ef, 0x123f, 0x301f, 0x305f, 0x32bf,},//当日(月)累计、上日(月)累计、上一曲线点	
				{}, //上一日(月)统计
				{0x004f, 0x00af, 0x032f, 0x041f, 0x045f,},//C1F41+ //当日(月)起点、曲线起点、测量点日(月)起点时的累计值
				},
				{0x902f,	{0x902f, 0x942f, 0xa02f, 0xa42f, 0xb02f, 0xb42f},//当前(上月)电能、当前(上月)需量、当前(上月)需量时间
				{0x120f, 0x124f, 0x302f, 0x306f, 0x308f, 0x309f, 0x310f, 0x311f, 0x32cf},//当日(月)累计、上日(月)累计、上一日(月)统计、上一曲线点	
				{0x010f, 0x011f, 0x017f, 0x018f,},//C2F25+~C2F26+ //上一日(月)统计
				{0x005, 0x00bf, 0x033f, 0x042f, 0x046f,},//C1F41+ //当日(月)起点、曲线起点、测量点日(月)起点时的累计值
				},
				{0x912f,	{0x912f, 0x952f, 0xa12f, 0xa52f, 0xb12f, 0xb52f},//当前(上月)电能、当前(上月)需量、当前(上月)需量时间
				{0x121f, 0x125f, 0x303f, 0x307f, 0x32df,},//当日(月)累计、上日(月)累计、上一曲线点	
				{}, //上一日(月)统计
				{0x006f, 0x00cf, 0x034f, 0x043f, 0x047f,},//C1F41+ //当日(月)起点、曲线起点、测量点日(月)起点时的累计值
				}
			};

			BYTE i,n,bOff;
			BYTE bBuf[4][256], bPnChgMask = 0, bPnDataIdNum[4] = {0};
			memset(bBuf, 0, sizeof(bBuf));	

			if (wPn >= PN_NUM)
				return;

			DTRACE(DB_DP, ("ClrPulsePnData() wPn=%d******2! \r\n", wPn));

			bPnChgMask = GetPulsePnChgMask(wPn);	//读取脉冲参数变更标志	

			for (i=0; i<sizeof(wBank0Id)/sizeof(WORD); i++)
			{
				WriteItemEx(BN0, wPn, wBank0Id[i], &bBuf[0][0], (DWORD )0);	//清数据清时间
			}

			for (i=0; i<sizeof(wBank11Id)/sizeof(WORD); i++)
			{
				WriteItemEx(BN11, wPn, wBank11Id[i], &bBuf[0][0], (DWORD )0);	//清数据清时间
			}	

			for (n=0; n<4; n++) //对应正有、正无、反有、反无
			{
				if ((bPnChgMask & (1<<n)) == (1<<n)) //检测测量点脉冲属性有变化 bit0~3对应正有、正无、反有、反无
				{
					for (i=0; i<MAX_IDS_NUM; i++)
					{
						if (g_PulseEngInfo[n].wBank0Ids[i] != 0)
							WriteItemEx(BN0, wPn, g_PulseEngInfo[n].wBank0Ids[i], &bBuf[n][0], (DWORD )0);	//清BANK0数据清时间
						if (g_PulseEngInfo[n].wBank11Ids[i] != 0)
							WriteItemEx(BN0, wPn, g_PulseEngInfo[n].wBank11Ids[i], &bBuf[n][0], (DWORD )0);	//清BANK11数据清时间
						if (g_PulseEngInfo[n].wBank18Ids[i] != 0)
							WriteItemEx(BN0, wPn, g_PulseEngInfo[n].wBank18Ids[i], &bBuf[n][0], (DWORD )0);	//清BANK18数据清时间
					}
				}
				else
				{
					//要保留的测量点数据个数
					for (i=0; i<MAX_IDS_NUM; i++)
					{
						if (g_PulseEngInfo[n].wPnDataIds[i] != 0)
							bPnDataIdNum[n] ++;
					}

					//读出要保留的测量点数据 
					ReadItemEx(BN0, wPn, g_PulseEngInfo[n].wPnDataIds, bPnDataIdNum[n], &bBuf[n][0]);
				}		
			}

			ClearBankData(BN0, SECT_PN_DATA, wPn);
			//ClearBankData(BN0, SECT_IMG, wPn);
//			ClearBankData(BN0, SECT_EXT_PN_DATA, wPn);
			int iRet = -1;
			DWORD wSec = GetCurTime();

			//写入要保留的数据
			for (n=0; n<4; n++) //对应正有、正无、反有、反无
			{
				if ((bPnChgMask & (1<<n)) != (1<<n)) //检测测量点脉冲属性有变化 bit0~3对应正有、正无、反有、反无
				{
					bOff = 0;
					for (i=0; i<bPnDataIdNum[n]; i++)
					{
						iRet = WriteItemEx(BN0, wPn, g_PulseEngInfo[n].wPnDataIds[i], &bBuf[n][bOff], wSec);
						if (iRet > 0)
							bOff += iRet;
					}
				}
			}
#endif
}

//清总加组数据
void ClrGrpPnData(WORD wPn)
{
	static WORD wBank0Id[] = {0x2302, 0x2303, 0x2304, 0x2305, //总加功率
	0x2306, 0x2307, 0x2308, 0x2309, //总加电量
	0x230a, 0x230b, 0x230f, 0x2310,}; //控制状态
 
	BYTE bBuf[128];
	if (wPn >= GB_MAXSUMGROUP)
		return;

	DTRACE(DB_DP, ("ClrGrpPnData() wPn=%d******2! \r\n", wPn));

	memset(bBuf, 0, sizeof(bBuf));
	for (WORD i=0; i<sizeof(wBank0Id)/sizeof(WORD); i++)
	{
		WriteItemEx(BN0, wPn, wBank0Id[i], bBuf, (DWORD )0);	//清数据清时间
	}

	static WORD wBank18Id[] = {0x003f, 0x004f, 0x005f, 0x006f, 0x009f, 0x00af, 0x00bf, 0x00cf, //当日月累计
		0x035f, 0x036f, 0x037f, 0x038f, 0x039f, 0x03af, 0x03bf, 0x03cf};//当日(月)总加累计起点及示值起点

	for (WORD i=0; i<sizeof(wBank18Id)/sizeof(WORD); i++)
	{
		WriteItemEx(BN18, 0, wBank18Id[i], bBuf, (DWORD )0);	//清数据清时间
		WriteItemEx(BN18, wPn, wBank18Id[i], bBuf, (DWORD )0);	//清数据清时间
	}
}


//描述:更新数据库使用的电表测量点标志位
void UpdMeterPnMask()
{
	//BYTE bProp;
	BYTE bMtrPnMask[PN_MASK_SIZE];

	memset(bMtrPnMask, 0, sizeof(bMtrPnMask));

	for (WORD wPn=0; wPn<POINT_NUM; wPn++)
	{
		BYTE bPnProp = GetPnProp(wPn);
		if (bPnProp == PN_PROP_METER || bPnProp == PN_PROP_RJ45 || bPnProp == PN_PROP_EPON)
			bMtrPnMask[wPn/8] |= 1<<(wPn%8);
	}
	SetMeterPnMask(bMtrPnMask);	//更新到数据库
}

//描述:更新抄表线程使用的测量点参数变化标志位
void SetPnChgMask(WORD wPn)
{	
	BYTE bPnChgMask[PN_MASK_SIZE];

	ReadItemEx(BN11, PN0, 0x0601, bPnChgMask); //测量点变化标志,0表示无变化,1表示有变化	

	bPnChgMask[wPn/8] |= 1<<(wPn%8);

	WriteItemEx(BN11, PN0, 0x0601, bPnChgMask); //测量点变化标志,更新到数据库

	memset(bPnChgMask, 0, sizeof(bPnChgMask));
	ReadItemEx(BN17, PN0, 0x7006, bPnChgMask); //测量点抄收到过至少一次屏蔽位
	bPnChgMask[wPn/8] &= ~(1<<(wPn%8));
	WriteItemEx(BN17, PN0, 0x7006, bPnChgMask); //测量点抄收到过至少一次屏蔽位
}

//描述:查询抄表线程使用的测量点参数变化标志位
bool GetPnChgMask(WORD wPn)
{	
	BYTE bPnChgMask[PN_MASK_SIZE];

	ReadItemEx(BN11, PN0, 0x0601, bPnChgMask); //测量点变化标志,0表示无变化,1表示有变化	

	return (bPnChgMask[wPn/8] & (1<<(wPn%8))) != 0;
}

//描述:清除抄表线程使用的测量点参数变化标志位
void ClrPnChgMask()
{	
	BYTE bPnChgMask[PN_MASK_SIZE];

	memset(bPnChgMask, 0, sizeof(bPnChgMask));
	WriteItemEx(BN11, PN0, 0x0601, bPnChgMask); //测量点无变化
}

//描述:更新脉冲测量点参数变化标志位
void SetPulsePnChgMask(WORD wPn, BYTE bBit)
{	
	BYTE bPnChgMask = 0;

	ReadItemEx(BN11, wPn, 0x0602, &bPnChgMask); //测量点变化标志

	bPnChgMask |= 1<<bBit; //bit0~3对应正有、正无、反有、反无,0表示无变化,1表示有变化	

	WriteItemEx(BN11, wPn, 0x0602, &bPnChgMask); //更新到数据库
}

//描述:查询脉冲测量点参数变化标志位
BYTE GetPulsePnChgMask(WORD wPn)
{	
	BYTE bPnChgMask = 0;

	ReadItemEx(BN11, wPn, 0x0602, &bPnChgMask); //bit0~3对应正有、正无、反有、反无,0表示无变化,1表示有变化	

	return bPnChgMask;
}

//描述:清除脉冲测量点参数变化标志位
void ClrPulsePnChgMask(WORD wPn)
{	
	BYTE bPnChgMask = 0;

	WriteItemEx(BN11, wPn, 0x0602, &bPnChgMask); //测量点无变化
}

void UpdPnMask()
{
#if MTRPNMAP!=PNUNMAP
	bool fPnMapFail = false;
#endif //MTRPNMAP!=PNUNMAP
	WORD wPn;
	WORD wPos;
	BYTE bProp;
	BYTE bMask;
	BYTE bPnMask[PN_MASK_SIZE];
	BYTE bOldPnMask[PN_MASK_SIZE];

	DTRACE(DB_DP, ("UpdPnMask()******1! \r\n"));

	memset(bPnMask, 0, sizeof(bPnMask));

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		bProp = GetPnProp(wPn);
		if (bProp==PN_PROP_METER || bProp==PN_PROP_AC || bProp==PN_PROP_PULSE)
			bPnMask[wPn/8] |= 1<<(wPn%8);
	}

	ReadItemEx(BN11, PN0, 0x0600, bOldPnMask); //测量点屏蔽位

#if MTRPNMAP!=PNUNMAP
	if (NewPnMap(MTRPNMAP, PN0) < 0)	//为交采固定映射一个测量PN0
		fPnMapFail = true;
#endif //MTRPNMAP!=PNUNMAP

	for (wPn=1; wPn<POINT_NUM; wPn++)
	{
		bMask = 1<<(wPn%8);
		wPos = wPn/8;
		if ((bPnMask[wPos]&bMask) ^ (bOldPnMask[wPos]&bMask))
		{	
			DTRACE(DB_DP, ("ClrPnData() wPn=%d******1! \r\n", wPn));
			//测量点有效无效发生改变
			//SetPnChgMask(wPn);	//置变化标志(增加或删除)
			ClrPnData(wPn);			
		}
		else if (GetPnChgMask(wPn)) //测量点参数发生改变
		{
			DTRACE(DB_DP, ("ClrPnData() wPn=%d******1! \r\n", wPn));
			if (GetPnProp(wPn) == PN_PROP_PULSE)
				ClrPulsePnData(wPn);
			else
				ClrPnData(wPn);			
		}

		ClrPulsePnChgMask(wPn);

#if MTRPNMAP!=PNUNMAP
		//避免前后标志位的不一致,不管怎样,有效的测量点都重新申请一下测量点映射
		//无效的测量点都删除一下映射
		if (bPnMask[wPos] & bMask)	//测量点有效
		{
			if (NewPnMap(MTRPNMAP, wPn) < 0)
				fPnMapFail = true;
		}
		else if ((bPnMask[wPos]&bMask) == 0) //测量点无效
		{
			DeletePnMap(MTRPNMAP, wPn);
		}
#endif //MTRPNMAP!=PNUNMAP
	}

#if MTRPNMAP!=PNUNMAP
	if (fPnMapFail)	//上一轮申请失败,有可能是后面的测量点还没释放,前面的测量点又在配置
	{				//再申请一轮就行了
		NewPnMap(MTRPNMAP, PN0); //为交采固定映射一个测量PN0

		for (wPn=1; wPn<POINT_NUM; wPn++)
		{
			bMask = 1<<(wPn%8);
			wPos = wPn/8;

			if (bPnMask[wPos] & bMask)	//测量点有效
			{
				NewPnMap(MTRPNMAP, wPn);
			}
		}
	}
#endif //MTRPNMAP!=PNUNMAP

	WriteItemEx(BN11, PN0, 0x0600, bPnMask); //测量点屏蔽位
}

//描述:取得交采的测量点号
WORD GetAcPn()
{
	BYTE bProp[16];
	for (WORD wPn=1; wPn<POINT_NUM; wPn++)
	{		
		int iLen = ReadItemEx(BN0, wPn, 0x8901, bProp);
		if (iLen==12 && bProp[2]==PN_PROP_AC)
			return wPn;
	}

	return PN0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//电表档案的配置序号与内部测量点号的相关关系定义和接口函数，内部测量点号对应6000的档案索引位置(从1开始)
//序号有效值都是[1,65535],0无效
WORD g_wMtrPnToSn[POINT_NUM];
WORD g_wValidPnNum = 0;

void InitMtrSnToPn()
{
	WORD wPn, wSn;
	BYTE bBuf[PNPARA_LEN];

	g_wValidPnNum = 0;
	memset(g_wMtrPnToSn, 0, sizeof(g_wMtrPnToSn));

	for (wPn=MTR_START_PN; wPn<POINT_NUM; wPn++)
	{
		if (ReadItemEx(BN0, wPn, 0x6000, bBuf) > 0)
		{
			wSn = OoOiToWord(&bBuf[4]);
			if (wSn != 0)
			{
				g_wValidPnNum++;
				g_wMtrPnToSn[wPn] = wSn;
				if (IsMtrPn(wPn))
					Set485PnMask(wPn);
			}
		}
	}
}

//描述:通过装置序号获得测量点值
//返回:如果正确则返回测量点号,否则返回0
WORD MtrSnToPn(WORD wSn)
{
	if (wSn != 0)
	{
		for (WORD wPn=MTR_START_PN; wPn<POINT_NUM; wPn++)
		{
			if (g_wMtrPnToSn[wPn] == wSn)
			{
				return wPn;
			}
		}
	}

	return INVALID_POINT;
}

//描述:保存装置序号到测量点号的映射
void SetMtrSnToPn(WORD wPn, WORD wSn)
{
	if (wPn>=MTR_START_PN && wPn<POINT_NUM) 
	{
		if (g_wMtrPnToSn[wPn]==0 && wSn>0) //原来没有占用，现在有
		{
			g_wValidPnNum++;
		}
		else if (g_wMtrPnToSn[wPn]>0 && wSn==0 && g_wValidPnNum>0) //原来有占用，现在没有
		{
			g_wValidPnNum--;
		}

		g_wMtrPnToSn[wPn] = wSn;
		/*
		if (IsMtrPn(wPn))
			Set485PnMask(wPn);
		else
			Clr485PnMask(wPn);
		*/
	}
}

//通过测量点值获得装置序号
WORD MtrPnToSn(WORD wPn)
{
	return (wPn>=MTR_START_PN && wPn<=POINT_NUM) ? g_wMtrPnToSn[wPn] : 0;
}

//描述:取得一个空的测量点位置
//返回:如果正确则返回测量点号,否则返回0
WORD GetEmptyPn()
{
	for (WORD wPn=MTR_START_PN; wPn<POINT_NUM; wPn++)
	{
		if (g_wMtrPnToSn[wPn] == 0)
		{
			return wPn;
		}
	}

	return INVALID_POINT;
}

//描述:根据电能表装置序号删除电表档案
//参数:@wSN 电能表装置序号
//返回:如果正确删除则返回相应的测量点号,否则返回0
WORD DeletSN(WORD wSn)
{
	WORD wPn;
	BYTE bBuf[PNPARA_LEN];

	wPn = MtrSnToPn(wSn);
	if (wPn != INVALID_POINT)
	{
		memset(bBuf, 0, sizeof(bBuf));
		WriteItemEx(BN0, wPn, 0x6000, bBuf);

		SetMtrSnToPn(wPn, 0);	//清除映射
	}

	return wPn;
}

//描述:是否V2007版645协议测量点
bool IsV07Mtr(WORD wPn)
{
	BYTE bBuf[128];
//	BYTE bProp = 0;	

	if (ReadItemEx(BN0, wPn, 0x8901, bBuf) <= 0)
		return false;

	if (bBuf[2] != PN_PROP_METER)
		return false;

	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return false;	

	if (bBuf[10] == PROTOCOLNO_DLT645_V07)
		return true;
	else
		return false;	
}

//////////////////////////////////////////////////////////////////////////
//以下为黑龙江62056协议所用到的辅助接口函数
// 增加电表参数
int AddMtrPara(BYTE* pbBuf)
{	
	WORD wSn = ByteToWord(pbBuf);

	if (wSn >= POINT_NUM)
		return -1;

	for (WORD i=1; i<POINT_NUM; i++)
	{	
		if ( PnToMtrSn(i) == wSn) //原有有效计量点的相同参数
		{
			SetPnChgMask(i); //置变化标志(增加或修改)
			break;
		}
	}

#ifdef	DLMS_863_VER//此处特殊处理下,如果是单相表，子协议号用7，否则用6
	if (pbBuf[17] == 0)
		pbBuf[15] = PN_S_PHASE_SURPROID;
	else
		pbBuf[15] = PN_3_PHASE_SURPROID;
#endif	

	return WriteItemEx(BN0, wSn, 0x8902, pbBuf);
}

// 删除电表参数
int DelMtrPara(BYTE* pbBuf)
{
	int iRv = -1;
	BYTE bBuf[50];
	memset(bBuf, 0, sizeof(bBuf));

	WORD wSn = ByteToWord(pbBuf);

	if (wSn >= POINT_NUM)
		return -1;

	if (wSn != 0)
	{
		for (WORD i=1; i<POINT_NUM; i++)
		{	
			if ( PnToMtrSn(i) == wSn) //原有有效计量点的相同参数
			{
				//SetPnChgMask(i); //置变化标志(增加或修改)
				//break;
				return 0;		//2009-09-02有有效计量点的不能删
			}
		}

		return WriteItemEx(BN0, wSn, 0x8902, bBuf);
	}
	else
	{
		for (WORD j=1; j<POINT_NUM; j++)//测量点号
		{	
			if ( GetPnProp(j) == PN_PROP_METER) //原有有效的电表计量点全部变更			
				//SetPnChgMask(j); //置变化标志(增加或修改)		
				return 0;		//2009-09-02有有效计量点的不能删
		}

		for (WORD i=1; i<POINT_NUM; i++)//序号
		{
			if (WriteItemEx(BN0, i, 0x8902, bBuf) > 0)
				iRv = 1;
		}	
	}
	return iRv;
}

//增加脉冲参数
int AddPlusePara(BYTE* pbBuf)
{
	bool fChg = false;
	int iNum = 0;
	WORD wSnBuf[4];
	memset((BYTE*)&wSnBuf, 0, sizeof(wSnBuf));

	WORD wSn = ByteToWord(pbBuf);

	if (wSn >= PN_NUM)
		return -1;

	for (WORD i=1; i<PN_NUM; i++)
	{	
		if ((iNum=PnToPluseSn(i, wSnBuf)) > 0)
		{
			for(int j=0; j<iNum; j++)
			{
				if (wSnBuf[j] == wSn) //原有有效计量点的相同参数
				{
					SetPnChgMask(i); //置变化标志(增加或修改)
					SetPulsePnChgMask(i, j); //置原测量点的脉冲变化标志
					fChg = true;
					break;
				}
			}
		}
		if (fChg) break;
	}

	return WriteItemEx(BN0, wSn, 0x8903, pbBuf);
}
// 删除脉冲参数
int DelPlusePara(BYTE* pbBuf)
{
	int iRv = -1;
	BYTE bBuf[50];
	memset(bBuf, 0, sizeof(bBuf));

	bool fChg = false;
	int iNum = 0;
	WORD wSnBuf[4];
	memset((BYTE*)&wSnBuf, 0, sizeof(wSnBuf));

	WORD wSn = ByteToWord(pbBuf);

	if (wSn >= PN_NUM)
		return -1;

	if (wSn != 0)
	{
		for (WORD i=1; i<PN_NUM; i++)
		{	
			if ((iNum=PnToPluseSn(i, wSnBuf)) > 0)
			{
				for(int j=0; j<iNum; j++)
				{
					if (wSnBuf[j] == wSn) //原有有效计量点的相同参数
					{
						//SetPnChgMask(i); //置变化标志(增加或修改)
						//SetPulsePnChgMask(i, j); //置原测量点的脉冲变化标志
						//fChg = true;
						//break;
						return 0;		//2009-09-02有有效计量点的不能删
					}
				}
			}
			if (fChg) break;			
		}
		return WriteItemEx(BN0, wSn, 0x8903, bBuf);
	}
	else
	{
		for (WORD j=1; j<PN_NUM; j++)//测量点号
		{	
			if ( GetPnProp(j) == PN_PROP_PULSE) //原有有效的脉冲计量点全部变更		
			{
				return 0;		//2009-09-02有有效计量点的不能删
				//SetPnChgMask(j); //置变化标志(增加或修改)	

				//if ((iNum=PnToPluseSn(j, wSnBuf)) > 0) 
				//{
				//	for(int k=0; k<iNum; k++) //原有有效的脉冲属性全部变更	
				//	{										
				//		SetPulsePnChgMask(j, k); //置原测量点的脉冲变化标志								
				//	}
				//}
			}
		}

		for (WORD i=1; i<PN_NUM; i++)//序号
		{
			if (WriteItemEx(BN0, i, 0x8903, bBuf) > 0)
				iRv = 1;
		}
	}

	return iRv;
}


//增加总加组参数
int AddGrpPara(BYTE* pbBuf)
{
	WORD wPn = *pbBuf;

	if (wPn >= GB_MAXSUMGROUP)
		return -1;

	return WriteItemEx(BN0, wPn, 0x8905, pbBuf);
}


//删除总加组参数
int DelGrpPara(BYTE* pbBuf)
{
	int iRv = -1;
	BYTE bBuf[50];
	memset(bBuf, 0, sizeof(bBuf));

	WORD wPn = *pbBuf;

	if (wPn >= GB_MAXSUMGROUP)
		return -1;

	if (wPn != 0)
	{
		if ( IsGrpOfCtrl(wPn) ) //2009-09-02有有效控制参数的不能删
			return 0;

		return WriteItemEx(BN0, wPn, 0x8905, bBuf);
	}
	else
	{
		for (WORD i=1; i<GB_MAXSUMGROUP; i++)
		{
			if ( IsGrpOfCtrl(i) ) //2009-09-02有有效控制参数的不能删
				return 0;
		}

		for (WORD i=1; i<GB_MAXSUMGROUP; i++)
		{
			if (WriteItemEx(BN0, i, 0x8905, bBuf) > 0)
				iRv = 1;
		}
	}

	return iRv;
}

//增加计量点参数
int AddPointPara(BYTE* pbBuf)
{
	int iRv1,iRv2,iRv3,iRv4;

	WORD wPn = ByteToWord(pbBuf);

	if (wPn >= POINT_NUM)
		return -1;

	if ((iRv1=WriteItemEx(BN0, wPn, 0x8901, pbBuf)) <= 0)//测量点性质
		return -1;
	
	SetPnChgMask(wPn); //置变化标志(增加或修改)	

	if ((iRv2=WriteItemEx(BN0, wPn, 0x4100, pbBuf+iRv1)) <= 0)//测量点基本参数
		return -1;
	if ((iRv3=WriteItemEx(BN0, wPn, 0x4101, pbBuf+iRv1+iRv2)) <= 0)//测量点限值参数
		return -1;
	if ((iRv4=WriteItemEx(BN0, wPn, 0x4102, pbBuf+iRv1+iRv2+iRv3)) <= 0)//测量点异常阀值参数
		return -1;

	return iRv1+iRv2+iRv3+iRv4;
}


//删除计量点参数
int DelPointPara(BYTE* pbBuf)
{
	int iRet = -1;
	int iRv1,iRv2,iRv3,iRv4;

	BYTE bBuf[100];
	memset(bBuf, 0, sizeof(bBuf));

	WORD wPn = ByteToWord(pbBuf);
	
	if (wPn >= POINT_NUM)
		return -1;

	if (wPn != 0)
	{
		if ( IsPointOfGrp(wPn) ) //2009-09-02有有效总加组的不能删
			return 0;

		if ((iRv1=WriteItemEx(BN0, wPn, 0x8901, bBuf)) <= 0)//测量点性质
			return -1;

		SetPnChgMask(wPn); //置变化标志(增加或修改)	

		if ((iRv2=WriteItemEx(BN0, wPn, 0x4100, bBuf)) <= 0)//测量点基本参数
			return -1;
		if ((iRv3=WriteItemEx(BN0, wPn, 0x4101, bBuf)) <= 0)//测量点限值参数
			return -1;
		if ((iRv4=WriteItemEx(BN0, wPn, 0x4102, bBuf)) <= 0)//测量点异常阀值参数
			return -1;
		return iRv1+iRv2+iRv3+iRv4;
	}
	else
	{
		for (WORD i=1; i<POINT_NUM; i++)
		{
			if ( IsPointOfGrp(i) ) //2009-09-02有有效总加组的不能删
				return 0;
		}

		for (WORD i=1; i<POINT_NUM; i++)
		{
			SetPnChgMask(i); //置变化标志(增加或修改)	

			if ((iRv1=WriteItemEx(BN0, i, 0x8901, bBuf)) > 0)//测量点性质
				iRet = 1;
			if ((iRv2=WriteItemEx(BN0, i, 0x4100, bBuf)) > 0)//测量点基本参数
				iRet = 1;
			if ((iRv3=WriteItemEx(BN0, i, 0x4101, bBuf)) > 0)//测量点限值参数
				iRet = 1;
			if ((iRv4=WriteItemEx(BN0, i, 0x4102, bBuf)) > 0)//测量点异常阀值参数
				iRet = 1;
		}
		return iRet;
	}
}

//描述:电表序号是否有效
bool IsMtrValid(WORD wSn)
{
	if (wSn >= POINT_NUM)
		return false;
		
	BYTE bProp[50]={0};
	if (ReadItemEx(BN0, wSn, 0x8902, bProp) <= 0)
		return false;

	if (ByteToWord(bProp)!=wSn || wSn==0)
		return false;

	return true;
}

//描述:脉冲序号是否有效
bool IsPluseValid(WORD wSn)
{
	if (wSn >= PN_NUM)
		return false;
		
	BYTE bProp[50]={0};
	if (ReadItemEx(BN0, wSn, 0x8903, bProp) <= 0)
		return false;

	if (ByteToWord(bProp)!=wSn || wSn==0)
		return false;

	return true;
}

//描述:电表或脉冲来源是否有效
bool IsSnParaValid(WORD wID, WORD wSn)
{
	if (wID == 0x8902)
		return IsMtrValid(wSn);
	else if (wID == 0x8903)
		return IsPluseValid(wSn);
	else
		return false;
}

//描述:获取计量点号对应的数传来源编号
//返回<0表示非法
int PnToMtrSn(WORD wPn)
{	
	if (wPn >= POINT_NUM)
		return -1;
		
	BYTE bProp[128]={0};
	if (ReadItemEx(BN0, wPn, 0x8901, bProp) <= 0)
		return -1;

	if (bProp[2]==PN_PROP_METER && bProp[3]==1)
		return ByteToWord(bProp+4);	
	
	return -1;
}

//描述:获取计量点号对应的脉冲来源编号数组
//返回脉冲来源编号的个数，<0表示非法
int PnToPluseSn(WORD wPn, WORD* pwSn)
{	
	if (wPn >= PN_NUM)
		return -1;
		
	BYTE bProp[128]={0};
	if (ReadItemEx(BN0, wPn, 0x8901, bProp) <= 0)
		return -1;

	if (bProp[2]==PN_PROP_PULSE && bProp[3]>=1 && bProp[3]<=4)
	{
		for (BYTE i=0; i<bProp[3]; i++)
			pwSn[i] = ByteToWord(bProp+4+2*i);
		return bProp[3];	
	}
	
	return -1;
}
//描述:电表是否支持自身日冻结
bool IsMtrFrzSelf(WORD wPn)
{
	return IsV07Mtr(wPn);	
}



//描述：清除测量点曲线每天的96点设置，用于正常换日时
void ClearCurveFrzFlg(WORD wPn)
{
	BYTE bIsSaveFlg[19];//本数据块每天96点入库状态
	TTime tNow;		
	GetCurTime(&tNow);

	memset(bIsSaveFlg, 0, 12);
	bIsSaveFlg[12] = (BYTE)(tNow.nYear-2000); 
	bIsSaveFlg[13] = tNow.nMonth;
	bIsSaveFlg[14] = tNow.nDay;

	WriteItemEx(BN0, wPn, 0xd881, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd889, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd901, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd905, bIsSaveFlg);	
	WriteItemEx(BN0, wPn, 0xd945, bIsSaveFlg);	
	DTRACE(DB_DP, ("ClearCurveFrzFlg : day change has to clear wPn=%d\n", wPn));	
}

//描述：设置测量点费率数
void SetPnRateNum(WORD wPn, BYTE bRateNum)
{ 
	WriteItemEx(BN0, wPn, 0x8911, &bRateNum);
#ifdef EN_CCT
	WORD wBn = CctGetPnBank(wPn);
	if (wBn != BN0)
		WriteItemEx(wBn, wPn, 0x8911, &bRateNum);
#endif
}

//描述:获取测量点的费率数
BYTE GetPnRateNum(WORD wPn)
{
	BYTE bRateNum = RATE_NUM;	
#ifdef EN_CCT
	WORD wBn = CctGetPnBank(wPn);
#else
	WORD wBn = BN0;
#endif
	ReadItemEx(wBn, wPn, 0x8911, &bRateNum);	

	if (bRateNum==0 || bRateNum>RATE_NUM)
		bRateNum = RATE_NUM;

	return bRateNum;
}

//描述:是否根据电表返回电能数据的实际长度修改测量点的费率数
bool IsChgRateNumByMtr()
{
	BYTE bVal = 0;
	ReadItemEx(BN10, PN0, 0xa1a1, &bVal);
	return ((bVal==0)?false:true);
}

//描述:根据大小类号区分07单相表,以抄读曲线时转成抄读整点冻结数据
bool IsSinglePhaseV07Mtr(WORD wPn)
{
	wPn;
	return false;
}

//描述:根据大小类号区分97单相表,以抄读曲线时转成抄读整点冻结数据
bool IsSinglePhaseV97Mtr(WORD wPn)
{
	wPn;
	return false;
}
//描述:根据协议号区分是只支持单抄的645协议还是支持块抄的97版645协议
//返回:1为支持块抄的97版645协议,2为只支持单抄的645协议.3为其他协议
BYTE IsSIDV97Mtr(WORD wPn)
{
	BYTE bBuf[128];
	//BYTE bProp = 0;	

	if (ReadItemEx(BN0, wPn, 0x8901, bBuf) <= 0)
		return 0;

	if (bBuf[2] != PN_PROP_METER)
		return 0;

	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return 0;	

	if (bBuf[10] == PROTOCOLNO_DLT645)
		return 1;
#ifdef PROTOCOLNO_DLT645_SID
	else if (bBuf[10] == PROTOCOLNO_DLT645_SID)
		return 2;
#endif

	return 1;	
}

//描述:计量点号是不是属于总加组的
bool IsPointOfGrp(WORD wPn)
{
	if (wPn >= PN_NUM)
		return false;
		
	BYTE bBuf[50]={0};

	for (WORD i=1; i<GB_MAXSUMGROUP; i++)
	{
		ReadItemEx(BN0, i, 0x8905, bBuf);

		if (bBuf[1] != 0)
		{
			for (WORD j=0; j<bBuf[1]; j++)
			{
				if (ByteToWord(&bBuf[j*4+2]) == wPn) //此测量点号属于有效总加组
					return true;
			}
		}
	}

	return false;
}

//描述:总加组是否配置了控制参数
bool IsGrpOfCtrl(WORD wPn)
{
	BYTE bBuf[300];

	if (wPn==0 || wPn>=GB_MAXSUMGROUP)
		return false;

	for (WORD i=0; i<MAX_CTRLPARA_NUM; i++)
	{
		memset(bBuf, 0, sizeof(bBuf));

		if (ReadItemEx(BN0, wPn, 0x5020+i, bBuf) > 0)
		{
			if (bBuf[0] != 0)	//若该参数的控制类型不为0则参数有配		 		
				return true;	
		}
	}

	return false;
}
//返回:测量点波特率	
DWORD GetMeterBaudRate(WORD wPn)
{	
	BYTE bPort = GetPnPort(wPn);
	int iPortFum = GetLogicPortFun(bPort);
	if (iPortFum == PORT_FUN_ACQ)  //如果表接在485采集器下,波特率就固定吧.
	{
		BYTE bBuf[4];
		DWORD dwCBR = 0;		
		ReadItemEx(BN15, PN0, 0x5326, bBuf);
		dwCBR = NumToBaudrateTest(bBuf[0]);		
		return dwCBR;
	}


	BYTE bBuf[128];

	//BYTE bProp = 0;	
	if (ReadItemEx(BN0, wPn, 0x8901, bBuf) <= 0)
		return CBR_2400;

	if (ReadItemEx(BN0, wPn, 0x8902, bBuf) <= 0)
		return CBR_2400;	

	DWORD dwBaudRate = GbValToBaudrate(bBuf[9]); //42	

	return dwBaudRate;
}
DWORD NumToBaudrateTest(BYTE n)
{
	switch (n)
	{
	case 0:
		return CBR_300;
	case 1:
		return CBR_600;
	case 2:   
		return CBR_1200;
	case 3:   
		return CBR_2400;
	case 4:
		return CBR_4800;	
	case 5:
		return CBR_9600;
	case 6:
		return CBR_19200;
	default:
		return CBR_9600;
	}

	return CBR_2400;
}

//描述:该控制参数是否投入
bool IsCtrlEnable()
{	
	return false;
}



//取得有效测量点个数
WORD GetPnNum()
{
	BYTE bMtrMask[PN_MASK_SIZE];
	WORD wPnSun = 0;

	memset(bMtrMask, 0, sizeof(bMtrMask));
	memcpy(bMtrMask, GetMtrMask(BANK17, PN0, 0x6001), PN_MASK_SIZE);
	for (WORD wMtrMask=0; wMtrMask<PN_MASK_SIZE; wMtrMask++)
	{
		if (bMtrMask[wMtrMask] != 0)
		{
			for (BYTE bBit=0; bBit<8; bBit++)
			{
				if (bMtrMask[wMtrMask] & (1<<(bBit)))
				{
					wPnSun ++;
				}
			}
		}
	}
	return wPnSun;
}

TSem g_semRdMtrCtrl = NewSemaphore(1);

//描述：设置更新抄表控制结构屏蔽字
void SetRdMtrCtrlMask(WORD wPn)
{
	BYTE bMtrCtrlMask[PN_MASK_SIZE];

	WaitSemaphore(g_semRdMtrCtrl);
	memset(bMtrCtrlMask, 0, sizeof(bMtrCtrlMask));
	ReadItemEx(BANK17, PN0, 0x6005, bMtrCtrlMask);
	bMtrCtrlMask[wPn/8] |= 1<<(wPn%8);
	WriteItemEx(BANK17, PN0, 0x6005, bMtrCtrlMask);
	DTRACE(DB_CRITICAL, ("SetRdMtrCtrlMask(): wPn=%d.\r\n", wPn));
	SetDelayInfo(INFO_MTR_UPDATE);
	SignalSemaphore(g_semRdMtrCtrl);
}

//描述：处理抄表控制结构屏蔽字
void DoRdMtrCtrlMask()
{
	WORD wPn;
	BYTE bMtrCtrlMask[PN_MASK_SIZE];

	WaitSemaphore(g_semRdMtrCtrl);
	memset(bMtrCtrlMask, 0, sizeof(bMtrCtrlMask));
	ReadItemEx(BANK17, PN0, 0x6005, bMtrCtrlMask);
	for (WORD i=0; i<PN_MASK_SIZE; i++)
	{
		if (bMtrCtrlMask[i])
		{
			for (BYTE j=0; j<8; j++)
			{
				if (bMtrCtrlMask[i] & (1<<j))
				{
					wPn = i*8 + j;
					DeleteOneMtrRdCtrl(wPn);
					DTRACE(DB_CRITICAL, ("DoRdMtrCtrlMask(): wPn=%d.\r\n", wPn));
				}
			}
		}
	}
	memset(bMtrCtrlMask, 0, sizeof(bMtrCtrlMask));
	WriteItemEx(BANK17, PN0, 0x6005, bMtrCtrlMask);
	SignalSemaphore(g_semRdMtrCtrl);
}

//描述：电表信息比较是否相同
//返回：相同返回true，反之false
bool MeterInfoCompare(WORD wPn, BYTE *pbBuf)
{
	BYTE bDbBuf[128];

	memset(bDbBuf, 0, sizeof(bDbBuf));
	if (ReadItemEx(BN0, wPn, 0x6000, bDbBuf)>0 && !IsAllAByte(bDbBuf, 0, sizeof(bDbBuf)))
	{
		if (bDbBuf[0]==pbBuf[0] && memcmp(bDbBuf, pbBuf, pbBuf[0]+1)==0)
			return true;
	}

	return false;
}


typedef struct {
	WORD wOI;	//方案类型ID
	WORD wDbId;	//方案类型ID对应系统库中的ID屏蔽字
	const TSchFieldCfg *pSchFieldCfg;	//对应&g_TSchFieldCfg[]
}TOiMapSchId;

TOiMapSchId g_tOiMap[] = {	{0x6014, 0x6006, &g_TSchFieldCfg[0]},	//普通采集方案更新屏蔽字
							{0x6016, 0x6007, &g_TSchFieldCfg[1]},	//事件采集方案
							{0x6051, 0x6008, &g_TSchFieldCfg[2]},	//透明采集方案
							{0x601C, 0x6009, &g_TSchFieldCfg[3]},	//上报采集方案
							{0x6012, 0x600a, &g_TSchFieldCfg[4]}};

TSem g_semSchUdp = NewSemaphore(1);

void SetSchUpdateMask(WORD wOI, WORD wSchId)
{
	BYTE bBuf[TASK_NUM_MASK];

	WaitSemaphore(g_semSchUdp);
	for (BYTE i=0; i<sizeof(g_tOiMap)/sizeof(g_tOiMap[0]); i++)
	{
		if (g_tOiMap[i].wOI == wOI)
		{
			memset(bBuf, 0, sizeof(bBuf));
			ReadItemEx(BANK17, PN0, g_tOiMap[i].wDbId, bBuf);
			bBuf[wSchId/8] |= 1<<(wSchId%8);
			WriteItemEx(BANK17, PN0, g_tOiMap[i].wDbId, bBuf);
			break;
		}
	}
	SignalSemaphore(g_semSchUdp);
}

//描述：根据相应的屏蔽字清除相应的方案
void ClearSchData()
{
	int iRet;
	WORD wSchNo;
	BYTE bBuf[TASK_NUM_MASK];
	char pszTabName[64];
	bool fSchUdp = false;
	bool fDelAllEvtFlg = true;

	WaitSemaphore(g_semSchUdp);
	DTRACE(DB_CRITICAL, ("ClearSchData(): clear sch data.\r\n"));
	for (BYTE index=0; index<sizeof(g_tOiMap)/sizeof(g_tOiMap[0]); index++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		ReadItemEx(BANK17, PN0, g_tOiMap[index].wDbId, bBuf);
		if (!IsAllAByte(bBuf, 0, sizeof(bBuf)))
		{
			for (BYTE i=0; i<TASK_NUM_MASK; i++)
			{
				if (bBuf[i])
				{
					for (BYTE j=0; j<8; j++)
					{
						if (bBuf[i] & (1<<j))
						{
							wSchNo = i*8 + j;

							memset(pszTabName, 0, sizeof(pszTabName));
							if (g_tOiMap[index].pSchFieldCfg->bSchType == SCH_TYPE_EVENT)
							{
#ifndef SYS_WIN
								if (IsAllAByte(bBuf+1, 0xff, sizeof(bBuf)-2))	//这里表示清空所有事件方案
								{
									if (fDelAllEvtFlg)
									{
										fDelAllEvtFlg = false;
										memset(pszTabName, 0, sizeof(pszTabName));
										sprintf(pszTabName, "rm -rf %s%s_*", USER_DATA_PATH, g_tOiMap[index].pSchFieldCfg->pszTableName);
										system(pszTabName);
										DTRACE(DB_TASK, ("DelSchData all: %s.\n", pszTabName));
									}
								}
								else	//这里表示清空单个事件方案
								{
									fDelAllEvtFlg = false;
									memset(pszTabName, 0, sizeof(pszTabName));
									sprintf(pszTabName, "rm -rf %s%s_%03d_*", USER_DATA_PATH, g_tOiMap[index].pSchFieldCfg->pszTableName, wSchNo);
									system(pszTabName);
									DTRACE(DB_TASK, ("DelSchData one: %s.\n", pszTabName));
								}
#endif
								SchRefreshMtrRdCtrl((BYTE )wSchNo);
							}
							else if (g_tOiMap[index].pSchFieldCfg->bSchType == SCH_TYPE_COMM)
							{
								sprintf(pszTabName, "%s_%03d.dat", g_tOiMap[index].pSchFieldCfg->pszTableName, wSchNo);
								iRet = TdbClearRec(pszTabName); 
								DTRACE(DB_TASK, ("DelSchData: %s, iRet=%d.\n", pszTabName, iRet));
								SchRefreshMtrRdCtrl((BYTE )wSchNo);
							}
							else if (g_tOiMap[index].pSchFieldCfg->bSchType == SCH_TYPE_SCRIPT)
							{
								sprintf(pszTabName, "TaskCfgUnit_%03d.para", wSchNo);
								DTRACE(DB_TASK, ("DelSchData: %s.\n", pszTabName));
								TaskRefreshMtrRdCtrl((BYTE )wSchNo);
							}
							else if (g_tOiMap[index].pSchFieldCfg->bSchType == SCH_TYPE_REPORT)
							{
								//BANK16 0x6001~0x6008 为上报相关的参数
								for (WORD wID=0x6001; wID<=0x6008; wID++)
								{
									DWORD dwZero = 0;
									WriteItemEx(BANK16, wSchNo, wID, (BYTE*)&dwZero);
								}
							}

							fSchUdp = true;
						}
					}
				}
			}
		}
		memset(bBuf, 0, sizeof(bBuf));
		WriteItemEx(BANK17, PN0, g_tOiMap[index].wDbId, bBuf);
	}

	if (fSchUdp)
	{
		BYTE bTaskSN = 0;
		ReadItemEx(BANK16, PN0, 0x6011, &bTaskSN);
		bTaskSN++;	
		WriteItemEx(BANK16, PN0, 0x6011, &bTaskSN);
	}

	SignalSemaphore(g_semSchUdp);
}

void ClearReportParam()
{
	DWORD dwZero = 0;

	//BANK16 0x6001~0x6008 为上报相关的参数
	for (WORD wID=0x6001; wID<=0x6008; wID++)
	{
		for (WORD i=0; i<TASK_NUM; i++)
			WriteItemEx(BANK16, i, wID, (BYTE*)&dwZero);
	}
}
