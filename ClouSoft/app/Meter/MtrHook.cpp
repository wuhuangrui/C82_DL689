/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MtrHook.cpp
 * 摘    要：本文件主要用来定义通信接口库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年4月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *			 $本文件定义的函数,形式一致,但是在做不同版本的产品时,函数可能需要修改
 *			 $在这里不要定义成inline,方便和库文件一起编译时重定位
 *********************************************************************************************************/
#include "stdafx.h"
#include "FaCfg.h"
#include "MtrHook.h"
#include "FaAPI.h"
#include "DbCctAPI.h"
#include "MeterAPI.h"
/*
//描述:485故障确认(所有测量点,分端口),在故障的发生/恢复时候的回调函数,用来生成告警事件
void On485ErrEstb(WORD wPort)
{
	if (g_b485PortStatus == 0)
		g_b485PortStatus = 1;
}

//描述:485故障恢复(所有测量点,分端口),在故障的发生/恢复时候的回调函数,用来生成告警事件
void On485ErrRecv(WORD wPort)		
{
	if (g_b485PortStatus == 1)
		g_b485PortStatus = 0;
}*/

//描述:抄表故障确认(单个测量点),在故障的发生/恢复时候的回调函数,用来生成告警事件
void OnMtrErrEstb(WORD wPn)	
{
	
	if (wPn > POINT0 && wPn <= POINT_NUM && g_bReadMtrStatus[wPn-1] == 0)
	{
		g_bReadMtrStatus[wPn-1] = 1;
		DTRACE(DB_METER_EXC, ("OnMtrErrEstb::wPn = %d estb.\r\n", wPn));
	}

}

//描述:抄表故障恢复(单个测量点),在故障的发生/恢复时候的回调函数,用来生成告警事件
void OnMtrErrRecv(WORD wPn)	
{
	if (wPn > POINT0 && wPn <= POINT_NUM && g_bReadMtrStatus[wPn-1] == 1)
	{
		g_bReadMtrStatus[wPn-1] = 0;
		DTRACE(DB_METER_EXC, ("OnMtrErrRecv::wPn = %d Recv.\r\n", wPn));
	}
}


//描述:获取测量点抄表故障的状态
bool IsMtrErr(WORD wPn)	
{
	if (g_bReadMtrStatus[wPn-1] == 1) //故障发生
		return true;
	else							  //正常
		return false;
}


/*
void DoMtrAnd485ErrErc()
{
	TTime tmNow;
	BYTE bAlrBuf[20];
	WORD wPn, i;
	static DWORD dwTime = 0;
	TBankItem tbItem[2];
	WORD wConstID[] = {0x9010, 0x9110};
	
	for (i=0; i<PN_NUM; i++)
	{
	  	memset(bAlrBuf, INVALID_DATA, sizeof(bAlrBuf));
		if (g_bReadMtrStatus[i] == 1 && !g_fMtrFailHapFlg[i])
		{
#ifdef PRO_698
			wPn = i+1;
			bAlrBuf[0] = wPn & 0xff;
			bAlrBuf[1] = 0x80 | (wPn >> 8) & 0x0f;
			ReadItemEx(BN11, wPn, 0x0b01, bAlrBuf+2);
			ReadItemEx(BN11, wPn, 0x0b02, bAlrBuf+7);
			ReadItemEx(BN11, wPn, 0x0b03, bAlrBuf+12);
			GetCurTime(&tmNow);
//			SaveAlrData( ERC_MTRRDFAIL, tmNow, bAlrBuf);
#endif
			g_fMtrFailHapFlg[i] = true;
		}
		else if (g_bReadMtrStatus[i] == 2)
		{
#ifdef PRO_698		
			if (dwTime == 0)
				dwTime = GetCurTime();
			if (GetCurTime() - dwTime < 20)
				continue;
			wPn = i+1;
			bAlrBuf[0] = wPn & 0xff;
			bAlrBuf[1] = (wPn >> 8) & 0x0f;
			GetCurTime(&tmNow);
			TimeToFmt15(tmNow, bAlrBuf+2);
			for (int j=0; j<2; j++)
			{
				tbItem[j].wBn = BN0;
				tbItem[j].wPn = wPn;
				tbItem[j].wID = wConstID[j];
			}
//			DirectReadMtr(tbItem, 2, bAlrBuf+7);
//			SaveAlrData( ERC_MTRRDFAIL, tmNow, bAlrBuf);
#endif			
			g_bReadMtrStatus[i] = 0;
			g_fMtrFailHapFlg[i] = false;
			dwTime = 0;
		}
	}

	if (g_b485PortStatus == 1 && !g_f485FailHapFlg)
	{
		BYTE bErr = 4;
		GetCurTime(&tmNow);
//		SaveAlrData(ERC_TERMERR, tmNow, &bErr);
		g_f485FailHapFlg = true;
	}
	else if (g_b485PortStatus == 2)
	{
		g_b485PortStatus = 0;
		g_f485FailHapFlg = false;
	}
}*/

//描述:获取测量点信息
//参数:@
void Get485PnMask(BYTE* pbNodeMask)
{
	ReadItemEx(BN17, PN0, 0x6002, pbNodeMask); //485测量点屏蔽位  
}

//描述:获取测量点信息
//参数:@
void GetPlcPnMask(BYTE* pbNodeMask)
{
	ReadItemEx(BN17, PN0, 0x6003, pbNodeMask); //载波测量点屏蔽位  
}

//描述:获取测量点信息的读指针
//参数:@
const BYTE* Get485PnMask( )
{
	return GetItemRdAddr(BN17, PN0, 0x6002); //485测量点屏蔽位
}

//描述:获取测量点信息的读指针
//参数:@
const BYTE* GetPlcPnMask( )
{
	return GetItemRdAddr(BN17, PN0, 0x6003); //载波测量点屏蔽位
}

//描述:设置485测量点屏蔽位
void Set485PnMask(WORD wPn)
{
	BYTE bPnMask[PN_MASK_SIZE];

	ReadItemEx(BN17, PN0, 0x6002, bPnMask); //485测量点屏蔽位

	bPnMask[wPn/8] |= 1<<(wPn%8);

	WriteItemEx(BN17, PN0, 0x6002, bPnMask); //485测量点屏蔽位,更新到数据库
}

//描述:设置载波测量点屏蔽位
void SetPlcPnMask(WORD wPn)
{
	BYTE bPnMask[PN_MASK_SIZE];

	ReadItemEx(BN17, PN0, 0x6003, bPnMask); //载波测量点屏蔽位

	bPnMask[wPn/8] |= 1<<(wPn%8);

	WriteItemEx(BN17, PN0, 0x6003, bPnMask); //载波测量点屏蔽位,更新到数据库
}

//描述:清除485测量点屏蔽位
void Clr485PnMask(WORD wPn)
{	
	BYTE bPnMask[PN_MASK_SIZE];

	ReadItemEx(BN17, PN0, 0x6002, bPnMask); //485测量点屏蔽位

	bPnMask[wPn/8] &= ~(1<<(wPn%8));

	WriteItemEx(BN17, PN0, 0x6002, bPnMask); //485测量点屏蔽位,更新到数据库
}

//描述:清除载波测量点屏蔽位
void ClrPlcPnMask(WORD wPn)
{	
	BYTE bPnMask[PN_MASK_SIZE];

	ReadItemEx(BN17, PN0, 0x6003, bPnMask); //载波测量点屏蔽位

	bPnMask[wPn/8] &= ~(1<<(wPn%8));

	WriteItemEx(BN17, PN0, 0x6003, bPnMask); //载波测量点屏蔽位,更新到数据库
}

//描述:通过电表地址转换为测量点号
//参数:@pbTsa 电表地址,
//	   @bAddrLen 表地址长度
//返回:如果找到匹配的测量点则返回测量点号,否则返回0
WORD MtrAddrToPn(const BYTE* pbTsa, BYTE bAddrLen)
{
	BYTE bMtrAddr[17];
	WORD wPn = 0;
	const BYTE* pbPnMask = Get485PnMask();	//取得非载波的485屏蔽位.
	if (pbPnMask == NULL)
		return 0;

	while (1)
	{
		wPn = SearchPnFromMask(pbPnMask, wPn);

		if (wPn >= POINT_NUM)
			break;

		GetMeterAddr(wPn, bMtrAddr);
		if (memcmp(pbTsa, bMtrAddr, bAddrLen) == 0)
		{
			return wPn;
		}
		wPn++;
	}

	return 0;
}

//描述:查找是否脉冲测量点号
//参数:@pbTsa 电表地址,
//	   @bAddrLen 表地址长度
//返回:如果找到匹配的测量点则返回测量点号,否则返回0
WORD PulseAddrToPn(const BYTE* pbTsa, BYTE bAddrLen)
{
	BYTE bMtrAddr[TSA_LEN+1];
	WORD wPn = 0;

	while (1)
	{
		if (wPn >= PULSE_PN_NUM)
			break;

		ReadItemEx(BN0, wPn, 0x2401, bMtrAddr);
		if (memcmp(pbTsa+2, bMtrAddr+2, bAddrLen-2) == 0)
		{
			return wPn+1;
		}
		wPn++;
	}

	return 0;
}

