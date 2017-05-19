/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbCctAPI.cpp
 * 摘    要：本文件主要实现协议中集抄相关的数据库标准接口之外的扩展接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年4月
 * 备    注：$为了避免集中器的系统库接口函数跟DbAPI.cpp中的接口函数混杂在一起
 			  特意定义本文件
 *********************************************************************************************************/
#include "stdafx.h"
#include "Info.h"
#include "FaConst.h"
#include "DbConst.h"
#include "ComStruct.h"
#include "FaAPI.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include "DbCctAPI.h"
#include "TaskConst.h"

//////////////////////////////////////////////////////////////////////////////////////////////
//集中器接口

//描述:获取测量点性质
//返回:0表示无效；1表示交采；2表示电表，3表示脉冲，4表示模拟量，5表示集抄
BYTE CctGetPnProp(WORD wPn)
{
	if (wPn >= POINT_NUM)
		return INVALID_POINT;
		
	BYTE bProp[16];
	if (ReadItemEx(BN0, wPn, 0x8901, bProp) <= 0)
		return INVALID_POINT;

	return bProp[2];
}


bool IsCctPn(WORD wPn)
{
	return GetPnProp(wPn)==PN_PROP_CCT;
}

bool IsRJ45Pn(WORD wPn)
{
	return GetPnProp(wPn)==PN_PROP_RJ45;
}

bool IsBBCctPn(WORD wPn)
{
	return GetPnProp(wPn)==PN_PROP_BBCCT;
}

bool CctIsPnValid(WORD wPn) 
{ 
	return IsCctPn(wPn); 
}

//描述:是否为单相表
bool IsSinglePhase(WORD wPn)
{	
	if (GetUserSubType(wPn) == 1)
		return true;
		
	return false;
}

//描述:是否为多功能表
bool IsMultiFunc(WORD wPn)
{
	if (IsSinglePhase(wPn))
		return false;
	else
		return true;
}


//描述:是否为多费率表
//备注:目前基本上多功能表跟多费率表是等同的概念,所以目前推荐首先都使用IsMultiRate()作为判断,
// 		避免同时使用多个类似的函数来判断同一个概念
bool IsMultiRate(WORD wPn)
{
	BYTE bTmp[200];
	if (IsCctPn(wPn))
	{
		if (ReadItemEx(BN0, wPn, 0x8902, bTmp) >= 0)
	    {
			if ((bTmp[59]&0x3f)>1 || bTmp[59]==0) //费率为0时与GetPnRateNum(WORD wPn)保持一致，当做多费率来处理
			    return true;
		    else
			    return false;
		}
	}

	return false;
}

//描述:是否是集抄的1类抄表数据项,不包括那些终端自己生成的集抄数据项,而不是通过抄表得到的
bool IsCctClass1MtrId(WORD wPn, WORD wID)
{
	static WORD wCctClass1MtrId[] = 
	{
		0x112f, 0x113f, 0x114f, 0x160f, 0x161f, 0x163f, 0x164f, 0x165f,//C1F25,C1F26,C1F27,C1F28 ,C1F29,C1F30,C1F31,C1F32
		0x115f, 0x116f, 0x117f, 0x118f, 0x119f,	0x11Af, 0x11Bf, 0x11Cf,	0x126f, //C1F33,C1F34,C1F35,C1F36,C1F37,C1F38,C1F39,C1F40,C1F49
		0x166f,	0x167f, 0x168f, 0x169f, 0x16af, 0x16bf, 0x16cf, 0x16df, //C1F129,C1F130,C1F131,C1F132,C1F133,C1F134,C1F135,C1F136
		0x200f, 0x201f, 0x202f, 0x203f, 0x204f, 0x205f, 0x206f, 0x207f, //C1F137,C1F138,C1F139,C1F140,C1F141,C1F142,C1F143,C1F144
		0x208f, 0x209f, 0x20af, 0x20bf, 0x20cf, 0x20df, 0x20ef, 0x210f, //C1F145,C1F146,C1F147,C1F148,C1F149,C1F150,C1F151,C1F152
	    0x220f, 0x221f, 0x222f, 0x223f, 0x224f,//C1F161,C1F165,C1F165,C1F167,C1F168
	};

	if (!IsCctPn(wPn))
		return false;

	for (WORD i=0; i<sizeof(wCctClass1MtrId)/sizeof(WORD); i++)
	{
		if (wID == wCctClass1MtrId[i])
			return true;
	}

	return false;
}


//描述:确定是否是集抄测量点数据BANK的ID
bool IsCctDataBankId(WORD wBn, WORD wID)
{
	if (/*wBn==CCT_BN_SPM ||*/ wBn==CCT_BN_MFM)
		return true;
	else
		return false;
}

//描述:根据电表类型,取得集抄测量点的数据存储BANK
WORD CctGetPnBank(WORD wPn)
{
	if (IsCctPn(wPn) || IsRJ45Pn(wPn) || IsBBCctPn(wPn))
	{
		/*if (IsMultiFunc(wPn)) //低压三相一般工商业用户 或 电压表
			return CCT_BN_MFM;		//多功能表数据BANK
		else
			return CCT_BN_SPM;		//单相表数据BANK*/

		return CCT_BN_MFM;	//目前不分BANK存储,统一按照多功能表来存,都支持POINT_NUM个
	}

	return BN0;
}

//描述:根据不同测量点类型数据分BANK存储原则,对ID的BANK进行重新映射
WORD CctGetIdBank(WORD wBn, WORD wPn, WORD wID)
{
	if (IsCctDataBankId(wBn, wID))
		return CctGetPnBank(wPn);	//根据电表类型,取得集抄测量点的数据存储BANK
	else
		return wBn; //默认情况下返回原来的BANK号
}

void CctPostDbInit()
{
	WORD wPn;
	BYTE bRate = RATE_NUM; //费率数
		
	//把单相表的费率数都初始化为4,(0x8911-费率数,用作组合ID中的其中一个子ID)
	for (wPn=0; wPn<POINT_NUM; wPn++)
	{
		bRate = RATE_NUM; //费率数
		ReadItemEx(BN0, wPn, 0x8911, &bRate);
		WriteItemEx(CCT_BN_SPM, wPn, 0x8911, &bRate);
	}

#if PNMAP_CCTMFM==PNUNMAP	//多功能表的费率数,在不需要测量点动态映射的情况下可以直接初始化为4
							//在需要测量点动态映射的情况下,要等到测量点映射后才能写,否则系统库找不到
	for (wPn=0; wPn<MFM_NUM; wPn++)
	{
		bRate = RATE_NUM; //费率数
		ReadItemEx(BN0, wPn, 0x8911, &bRate);
		WriteItemEx(CCT_BN_MFM, wPn, 0x8911, &bRate);
	}
#endif 

	//初始化测量点屏蔽位
	//CctUpdPnMask();	 //更新载波节点屏蔽位比较耗时,有可能引起看门狗复位
						 //放到InitCct里初始化好些
}

//描述:搜索测量点
WORD SearchPnFromMask(const BYTE* pbPnMask, WORD wStartPn)
{
	WORD i, j;
	i = wStartPn / 8;
	j = wStartPn % 8;
	for (; i<PN_MASK_SIZE; i++)
	{
		if (pbPnMask[i] != 0)
		{
			BYTE bBitMask = 1 << j;
			for (; j<8; j++,bBitMask<<=1)
			{
				if (pbPnMask[i] & bBitMask)
					return i*8+j;
			}
		}
		
		j = 0;
	}
	
	return POINT_NUM;
}

//描述:从测量点屏蔽位中搜索测量点最后一个测量点
//返回:如果正确则测量点号,否则返回0
WORD SchLastPnFromMask(const BYTE* pbPnMask)
{
	int i, j;
	i = (POINT_NUM-1) / 8;
	j = (POINT_NUM-1) % 8;
	for (; i>=0; i--)
	{
		if (pbPnMask[i] != 0)
		{
			for (; j>=0; j--)
			{
				BYTE bBitMask = 1 << j;
				if (pbPnMask[i] & bBitMask)
					return (WORD )(i*8+j);
			}
		}
		
		j = 7;
	}
	
	return 0;
}


//描述:是否已经存在该采集终端的节点
//参数:@pbNodeMask 扫描过程中已经扫描到的通过采集终端的载波节点屏蔽位
//	   @pbAddr 载波终端的地址
bool IsAcqNodeExist(BYTE* pbNodeMask, BYTE* pbAddr)
{
	BYTE bAddr[6];
	WORD wNode = 0;
	while (1)
	{
		wNode = SearchPnFromMask(pbNodeMask, wNode);
		if (wNode >= POINT_NUM)
			return false;
		
//		if (GetPlcNodeAddr(wNode, bAddr))
		{
			if (memcmp(pbAddr, bAddr, 6) == 0)	//以前就出现过,不是第一个
				return true;
		}

		wNode++;
	}
	
	return false;
}

//描述:重点户测量点屏蔽位
void UpdateVipMask()
{	
	WORD wPn, wSn;
	WORD wPos;
	BYTE bMask;
	BYTE bNum;
	BYTE bBuf[50];
	BYTE bVipMask[PN_MASK_SIZE];
	BYTE bOldMask[PN_MASK_SIZE];

	memset(bOldMask, 0, sizeof(bOldMask));
	ReadItemEx(BN17, PN0, 0x7031, bOldMask); //先保留旧的 0x7031 重点户测量点屏蔽位

	memset(bVipMask, 0, sizeof(bVipMask));

	if (ReadItemEx(BANK0, PN0, 0x023f, bBuf) > 0)
	{
		WORD wPn;
		bNum = bBuf[0];
		if (bNum > 20)
			bNum = 20;

		for (WORD i=0; i<bNum; i++)
		{
			wSn = ByteToWord(&bBuf[1+2*i]);
			wPn = wSn;//MtrSnToPn(wSn); //通过装置序号获得测量点值,如果正确则返回装置序号,否则返回0
//			if (!IsPlcLink(wPn))		//考虑到负控测量点也在此处，需要判断，只有PLC测量点才有重点户一说
				continue;

			if (wPn!=0 && IsCctPn(wPn))	//只有集抄测量点才能做重点户
				bVipMask[wPn/8] |= 1 << (wPn%8);
		}
	}

	WriteItemEx(BN17, PN0, 0x7031, bVipMask); //0x7031 重点户测量点屏蔽位

#if PNMAP_VIP!=PNUNMAP
	for (WORD wPn=0; wPn<POINT_NUM; wPn++)
	{
		bMask = 1<<(wPn&0x07);
		wPos = wPn>>3;
		if (bVipMask[wPos] & bMask)
		{	//有效的测量点不管以前有没申请,都重新申请一下测量点映射,不会有什么副作用
			
			if (NewPnMap(PNMAP_VIP, wPn) < 0)	//为交采固定映射一个测量PN0
			{	
				DTRACE(DB_CRITICAL, ("UpdateVipMask: fail to new vip PnMap for wPn=%d\r\n", wPn));
			}

			if ((bOldMask[wPos] & bMask) == 0) //以前没有配置过,新配置的重点户
				CctVipPnData(wPn);

//			UpdVipRdFlg(wPn);	//必须在建立映射后才更新抄读标志,否则测量点会读错
		}
		else
		{
			DeletePnMap(PNMAP_VIP, wPn);	//非多功能表或者无效的测量点都删除一下映射,避免有泄漏
		}
	}
#endif 
}

//描述：初始化测量点性质 add by CPJ at 2012-06-15
void InitPnProp()
{
    BYTE bPort = 0, bProp = PN_PROP_UNSUP, bPortFun = 0;
	for(WORD wPn=0; wPn<POINT_NUM; wPn++)
	{
		if(!IsPnValid(wPn))
			continue;

		bPort = GetPnPort(wPn);
		if(bPort>=LOGIC_PORT_MIN && bPort<=LOGIC_PORT_MAX) //只影响逻辑端口2、3、4
		{
			bPortFun = GetLogicPortFun(bPort);
			if(GetPnProp(wPn)==PN_PROP_METER && (bPortFun==PORT_FUN_ACQ||bPortFun==PORT_FUN_JC485)) //负控测量点配的逻辑端口功能为集抄485
		   {
               bProp = PN_PROP_CCT;
			   WriteItemEx(BN0, wPn, 0x8901, &bProp);
		   }
		   else if(GetPnProp(wPn)==PN_PROP_CCT && bPortFun==PORT_FUN_RDMTR) //载波测量点配的逻辑端口功能为负控抄表口
		   {
			   bProp = PN_PROP_METER;
			   WriteItemEx(BN0, wPn, 0x8901, &bProp);
		   }
		}	
	}
}


//描述:清集抄测量点数据
void CctClrPnData(WORD wPn)
{
	WORD i;						
	BYTE bBuf[PN_MASK_SIZE+10];
	if (wPn >= POINT_NUM)
		return;

	DTRACE(DB_CCT, ("CctClrPnData: wPn=%d\r\n", wPn));
/*
	//清BN16(当前间隔)集抄任务的测量点抄收标志
	WORD wComTaskNum = CctGetComTaskNum();
	for (i=0; i<wComTaskNum; i++)
	{
		ReadItemEx(BN16, i, 0x6001, bBuf);  //0x6001 PN_MASK_SIZE+5 (当前间隔)集抄任务的测量点抄收标志,每个任务对应一个测量点
		bBuf[wPn/8] &= ~(1<<(wPn%8));
		WriteItemEx(BN16, i, 0x6001, bBuf);
	}*/

	//清BN17集抄非保存数据:
	const static WORD wBank17Id[] = {0x7010, 0x7011, 0x7012, 0x7013, 0x7014};
	memset(bBuf, 0, sizeof(bBuf));
	for (WORD i=0; i<sizeof(wBank17Id)/sizeof(WORD); i++)
	{
		WriteItemEx(BN17, wPn, wBank17Id[i], bBuf, (DWORD )0);	//清数据清时间
	}

	DbClrPnData(CCT_BN_SPM, SECT0, POINT_NUM, wPn);		//单相表数据BANK
	BYTE bRate = RATE_NUM; //费率数
	ReadItemEx(BN0, wPn, 0x8911, &bRate);	
	WriteItemEx(CCT_BN_SPM, wPn, 0x8911, &bRate); //费率数重写一次，不然费率数=0
}


void CctVipPnData(WORD wPn)
{
	BYTE bBuf[PN_MASK_SIZE+10];
	if (wPn >= POINT_NUM)
		return;

	DTRACE(DB_CCT, ("CctVipPnData: wPn=%d\r\n", wPn));

	//清BN16集抄临时数据
	const static WORD wBank16Id[] = {0x6110, 0x6111, 0x6112, 0x6113, 0x6114, 0x6115, 0x6116, 0x6117, 0x6120};
	memset(bBuf, 0, sizeof(bBuf));
	for (WORD i=0; i<sizeof(wBank16Id)/sizeof(WORD); i++)
	{
		WriteItemEx(BN16, wPn, wBank16Id[i], bBuf, (DWORD )0);	//清数据清时间
	}
}
