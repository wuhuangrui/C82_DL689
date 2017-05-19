/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称:Ctrl.cpp
 * 摘    要:本文件主要实现CCtrl的类
 * 当前版本:1.0
 * 作    者:张建德
 * 完成日期:2008年3月
*********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "sysfs.h"
#include "FaCfg.h"
#include "FaConst.h"
#include "DbConst.h"
#include "ComAPI.h"
#include "TaskDB.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "TaskManager.h"
#include "CtrlBase.h"
#include "FaAPI.h"
#include "DpGrp.h"
#include "DbOIAPI.h"

BYTE g_bBitMask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

//描述: 获取某个缓冲区指定位的状态.
//参数:@pbBuf	数据区指针.
//	   @wLen	缓冲区长度.
//	   @wIdx	bit的位置索引.
//返回: 指定位的状态.
bool GetBitStatus(BYTE* pbBuf, WORD wLen, WORD wIdx)
{
	WORD w = wIdx / 8;

	if (wLen <= w)
		return false;

	return ((*(pbBuf+w)&g_bBitMask[wIdx%8]) != 0);
}

//描述: 设置某个缓冲区指定位的状态.
//参数:@pbBuf	数据区指针.
//	   @wLen	缓冲区长度.
//	   @wIdx	bit的位置索引.
//返回: 设置是否成功.
bool SetBitStatus(BYTE* pbBuf, WORD wLen, WORD wIdx, bool fStatus)
{
	WORD w = wIdx / 8;

	if (wLen <= w)
		return false;

	if (fStatus)
		*(pbBuf+w) |= g_bBitMask[wIdx%8];
	else
		*(pbBuf+w) &= ~g_bBitMask[wIdx%8];

	return true;
}

//描述: 转换数据格式19
//参数:@pb		数据区指针
//返回: 返回转换后的 DWORD 格式的数据
DWORD TranDataFmt19(BYTE* pb)
{
	return (BcdToDWORD(pb, 1) + BcdToDWORD(pb+1, 1)*60) * 60;
}

//描述: 转换数据格式20
//参数:@pb		数据区指针
//返回: 返回转换后的 DWORD 格式的数据
DWORD TranDataFmt20(BYTE* pb)
{
	TTime Time;

	Time.nSecond = 0;
	Time.nMinute = 0;
	Time.nHour = 0;
	Time.nDay = BcdToByte(pb[0]);
	Time.nMonth = BcdToByte(pb[1]);
	Time.nYear = 2000 + BcdToByte(pb[2]);

	return TimeToSeconds(Time);
}

//描述: 获取字节中最右边1的位置(0 ~ 7).
//参数:@bFlgs	要查询的字节.
//返回: D0~D7按顺序对位表示0~7,如一个1都没有则返回 -1.
int GetIdxOfMostRight1(BYTE bFlgs)
{
	int i = -1;

	for (int i1=0; i1<8; i1++)
	{
		if ((bFlgs&1) != 0)
		{
			i = i1;
			break;
		}
		bFlgs >>= 1;
	}

	return i;
}

//描述: 获取字节中最右边1的位置(0 ~ 7).
//参数:@bFlgs	要查询的字节.
//返回: D0~D7按顺序对位表示0~7,如一个1都没有则返回 -1.
int GetIdxOfMostLeft1(BYTE bFlgs)
{
	int i = 0;

	for (int i1=8; i1>0; i1--)
	{
		if ((bFlgs&0x80) != 0)
		{
			i = i1;
			break;
		}
		bFlgs <<= 1;
	}

	return i;
}

//描述: 获取指定位置在所有右边的1中的位置编号(0 ~ 7).
//		01011010
//	第[1]位的编号为0,第[3]位的编号为1,第[4]位的编号为2,
//	第[6]位的编号为3,其他各个为0的位都返回-1.
//参数:@bFlgs	要查询的字节.
//	   @iIdx	要查询的位置.
//返回: 如指定位置的值为1,则返回它的位置编号,否则返回 -1.
int GetIdxOfAll1InPst(BYTE bFlgs, int iIdx)
{
	int i, i1 = -1;

	if (iIdx > 7)
		return -1;
	for (i=0; i<iIdx; i++)
	{
		if ((bFlgs&0x01) != 0)
			i1++;
		bFlgs >>= 1;
	}
	if ((bFlgs&0x01) != 0)
		return (i1+1);
	else
		return -1;
}

//描述: 获取所有1的个数.
//		01011010
//	1的个数为4.
//参数:@bFlgs	要查询的字节.
//返回: 如指定位置的值为1,则返回它的位置编号,否则返回 -1.
int GetSumOf1(BYTE bFlgs)
{
	int i, i1 = 0;

	for (i=0; i<8; i++)
	{
		if ((bFlgs&0x01) != 0)
			i1++;
		bFlgs >>= 1;
	}

	return i1;
}

//描述: 获取指定总加组有效当前有功功率.
//参数:@iGrp 	要获取的总加组.
//返回: 如果指定总加组有效,且有功功率有效，返回指定总加组当前有功功率,否则返回 -1.
int64 GetValidCurPwr(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return -1;
	
	int64 iPwr;
	BYTE bBuf[9] = {0};

	if (ReadItemEx(BN0, iGrp, 0x2302, bBuf) <= 0)
	{
		DTRACE(DB_LOADCTRL, ("GetValidCurPwr: There is something wrong when call ReadItemEx() !\n"));
		return -1;
	}

	iPwr = OoLong64ToInt64(bBuf+1);
	/*if (ReadItemVal64(BN0, (WORD)iGrp, 0x109f, &iPwr) <= 0)	//读取指定总加组"有功功率".C1F17
	{
		DTRACE(DB_LOADCTRL, ("GetCurPwr: There is something wrong when call ReadItemEx() !\n"));
		return -1;
	}*/

	/*if (iPwr == INVALID_VAL64)
		return -1;*/

	return iPwr;
}

//描述: 获取指定总加组当前有功功率.
//参数:@iGrp 	要获取的总加组.
//返回: 如果指定总加组有效,且有功功率有效，返回指定总加组当前有功功率,否则返回 0.
int64 GetCurPwr(int iGrp)
{
	int64 iPwr = GetValidCurPwr(iGrp);

//	if (iPwr < 0)
//		return 0;

	return iPwr;
}

//描述: 获取指定总加组指定类型正向有功总电能.
//参数:@iGrp	要获取的总加组.
//返回: 指定类型正向有功总电能,如读不到数据或读到无效数据,返回负数.
int64 GetSelEng(int iGrp, int iSel)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return -1;

	WORD wID;
	int64 i64Buf[1+1+RATE_NUM];
	SetArrVal64(i64Buf, INVALID_VAL64, 1+1+RATE_NUM);

	switch(iSel)
	{
	case 0:	//当前总加有功电能量（总、费率1~M）示值
		wID = 0x130F;
		break;
	case 1:	//当日正向有功总电能.
		wID = 0x10bf;
		break;
	case 2:	//当月正向有功总电能.
		wID = 0x10df;
		break;
	default:
		return -1;
	}

	if (ReadItemVal64(BN0, (WORD)iGrp, wID, i64Buf) <= 0)	//对相应的总加组从相应的PN读"当月正向有功总电能"ID
	{
		DTRACE(DB_LOADCTRL, ("GetSelEng: There is something wrong when call ReadItemVal64() !\n"));
		return -1;
	}

	if (i64Buf[1] == INVALID_VAL64)
		return -1;

	return i64Buf[1];
}

//描述: 判断是否处在保电状态.
//返回: 如果处在保电状态返回 true,否则返回 false.
bool IsGuarantee(void)
{
	BYTE bBuf[2];

	if (ReadItemEx(BN0, PN0, 0x8001, bBuf) <=0)	//读"终端控制设置状态"ID
	{
		DTRACE(DB_LOADCTRL, ("CCtrl::IsGuarantee: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	return (bBuf[1] != 0);	//解除（0），保电（1），自动保电（2）
}


//========================================== CCtrlBase =============================================
TTime CCtrlBase::m_tmNow;
DWORD CCtrlBase::m_dwNow;

TTime CCtrlBase::m_tmOldTime;
DWORD CCtrlBase::m_dwOldTime;

//============================================ CCtrl ===============================================
//描述: 初始化.
//返回: 如果初始化正常返回 true,否则返回 false.
bool CCtrl::Init(void)
{
	m_fInCtrl = false;		//该控制是否处于控制状态
	m_fGuarantee = false;	//该控制是否已经处于保电状态,主要用来判断保电的切换
	ClrCmd();		//清除内存中本类控制的控制命令.
	RstCtrl();		//复位内存中本类控制状态量.
	SetValidStatus(false);	//恢复控制退出状态.
	return true;
}

//描述: 判断是否处在保电状态.
//返回: 如果处在保电状态返回 true,否则返回 false.
//bool CCtrl::IsGuarantee(void)
//{
//	BYTE bBuf[1+1+6*8];
//
//	if (ReadItemEx(BN0, PN0, 0x104f, bBuf) <=0)	//读"终端控制设置状态"ID
//	{
//		DTRACE(DB_LOADCTRL, ("CCtrl::IsGuarantee: There is something wrong when call ReadItemEx() !\n"));
//		return 0x00;
//	}
//
//	return ((bBuf[0]&0x01) != 0);	//保电控使用第0位.
//}

//========================================= CGrpCtrl ===============================================
//描述: 扫描系统库各总加组的本类控制命令.
//		本函数必须完成如下几个功能
//		1. 保证执行各总加组中最新的有效命令.
//			如命令是解除命令,应复位控制状态,并从系统库中删除当前组的命令.
//			如命令是投入命令,如和之前收到的命令不同(通过接收命令的时间来判
//			断),复位控制状态,然后将该命令保存到m_'Ctrl'Cmd('Ctrl'在各个控
//			制类中的名字不同)中,如命令和之前收到的命令相同,则无需改变任何
//			状态.
//		2. 保证删除无效的和过时的命令.
CGrpCtrl::CGrpCtrl()
{
	 m_iGrp = -1; 
	 m_wOpenTimes = 0; 
	 m_bTurnsStatus = 0;
	 m_fIfOverLimit = false;
	 m_wOpenTimes = 0;

	 m_dwInitClick = 0;
	 m_fAlarmStatus = false;
	 m_fOpenStatus = false;
	 m_iCtrlGrp = -1;
	 m_bCloseTurn = 0;
}

void CGrpCtrl::DoCmdScan(void)
{
	char cCtrlType[20];
	char cTime[20];
	int i;

	for (i=GRP_START_PN; i<(GRP_START_PN+GRP_NUM); i++)
	{
		if (!IsGrpValid(i))
			continue;

		if (!GetSysCmd(i))	//获取某总加组的本类控制命令,并把命令放到 m_NewCmd 中.(注意: 对不同的类,m_NewCmd的结构是不同的)
			continue;	//假如没有命令则跳过下面的处理代码,直接扫描下一个总加组的控制命令单元.

		if (m_iGrp != i)
		{
			if (NewCmdAct() == 1)
			{//假如是'投入'命令.
				DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: %s rx new cmd at %s, grp=%d, act=%d\n",
									 CtrlType(cCtrlType), TimeToStr(NewCmdTime(), cTime), i, NewCmdAct()));
				
				if (GRP_START_PN<=m_iGrp && m_iGrp<(GRP_START_PN+GRP_NUM))
				{//如当前确实投入了某个总加组'本类控制'.
					RstSysCtrlStatus(m_iGrp);			//复位系统库当前总加组本类控制状态(可能包括轮次状态,投入标志等等).
					ClrSysCmd(m_iGrp);					//清除系统库当前总加组本类控制命令.
					DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: %s quit at %s due to grp change, new grp=%d, old grp=%d\n",
										 CtrlType(cCtrlType), TimeToStr(m_tmNow, cTime), i, m_iGrp));
					//***记录到系统库日志中.
					//***发出声光信号;
				}
				RstCtrl();					//复位内存中本类控制的所有相关状态.
				SetValidStatus(false);		//恢复控制退出状态.
				SaveNewCmd();				//保存最新命令.
				SetSysCtrlFlg(i, true);		//将本总加组系统库本类控制标志设为有效.
				SetValidStatus(true);		//将内存中的本类控制的投入状态设为投入.
				m_iGrp = i;					//将本总加组号声明为当前总加组.
				DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: %s launch at %s, grp=%d\n",
									 CtrlType(cCtrlType), TimeToStr(m_tmNow, cTime), i));
				//***记录到系统库日志中.
				//***发出声光信号;
			}
			else if (NewCmdAct() >= 2)
			{//检测到解除命令或非法命令,因为之前该组并没有投入,解除命令也做为非法命令删除.
				RstSysCtrlStatus(i);	//复位系统库当前总加组本类控制状态(可能包括轮次状态,投入标志等等).
				ClrSysCmd(i);
				DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: %s rx a cancle cmd, grp=%d!\n",
									 CtrlType(cCtrlType), i));
			}
		}
		else if (NewCmdTime() != CurCmdTime())
		{//在当前总加组检测到新的命令.
			if (NewCmdAct() == 1)	//如果控制以前是投入的现在也还是投入,只是重新投入或者命令时间发生改变,则不用复位控制
			{//假如是'投入'命令.
				if (NewCmdAct()!=CurCmdAct() || CtrlType()==CTL_PWR_TMP) //临时限电控收到同一个总加组的命令不管参数有没发生改变都重新再来
					RstCtrl();				//复位内存中本类控制的所有相关状态.

				SaveNewCmd();			//保存最新命令.
				SetSysCtrlFlg(i, true);	//在更新一下1类数据中的F5,F6,有些时候控制的总加组没变,
										//但是有些控制参数可能发生了改变,要更新进去
				SetValidStatus(true);	//将内存中的本类控制的投入状态设为投入.
				DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: %s rx new cmd of the same grp=%d at %s, reset ctrl!\n",
									 CtrlType(cCtrlType), i, CtrlType(cCtrlType), TimeToStr(m_tmNow, cTime)));
				//***记录到系统库日志中.
				//***发出声光信号;
			}
			else if (NewCmdAct() == 2)	//当前总加组解除命令
			{
				RstSysCtrlStatus(i);	//复位系统库当前总加组本类控制状态(可能包括轮次状态,投入标志等等).
				ClrSysCmd(i);			//清除系统库当前总加组本类控制命令.
				RstCtrl();				//复位内存中本类控制的所有相关状态.
				SetValidStatus(false);	//恢复控制退出状态.
				ClrCmd();				//清除内存中本类控制的控制命令.
				m_iGrp = -1;			//将当前总加组设为 -1,表示当前没有总加组投入.
				DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: %s quit at %s, grp=%d\n",
									 CtrlType(cCtrlType), TimeToStr(m_tmNow, cTime), i));
				//***记录到系统库日志中.
				//***发出声光信号;
			}
		}
	}
}

//描述: 获取系统库指定总加组指定控制类轮次配置状况.
//参数:@iGrp	要获取的总加组.
//	   @iSel	控制类(0: 功控; 1: 电控).
//返回: D0~D7按顺序对位表示1~8轮次开关的电控受控状态;置"1":受控,置"0":不受控.
BYTE CGrpCtrl::GetSysCtrlTurnsCfg(int iGrp, int iSel)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return 0x00;

	BYTE bCfg[3] = {0};
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;
	TGrpCtrlSetSta tGrpCtrlSetSta;
	memset(&tGrpCtrlSetSta, 0, sizeof(TGrpCtrlSetSta));

	if (!GetGrpCtrlSetSta(iGrp, &tGrpCtrlSetSta))
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::GetSysCtrlTurnsCfg: There is something wrong when call GetGrpCtrlSetSta() !\n"));
		return 0x00;
	}

	switch (iSel)
	{
	case 0:	//功控
		OoReadAttr(0x2300+iGrp,	0x0E, bCfg, &pbFmt, &wFmtLen);	//总加组功控轮次配置
		tGrpCtrlSetSta.bPwrCtrlTurnSta = bCfg[2] & CTL_TURN_MASK;
		break;
	case 1:	//电控
		OoReadAttr(0x2300+iGrp,	0x0F, bCfg, &pbFmt, &wFmtLen); //总加组电控轮次配置
		tGrpCtrlSetSta.bEngCtrlTurnSta = bCfg[2] & CTL_TURN_MASK;
		break;
	default:
		return 0x00;
	}

	if (!SetGrpCtrlSetSta(iGrp, &tGrpCtrlSetSta))
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::GetSysCtrlTurnsCfg: There is something wrong when call SetGrpCtrlSetSta() !\n"));
		return 0x00;
	}

	return (bCfg[2] & CTL_TURN_MASK);
}

//描述: 获取系统库指定总加组指定控制类的所有标志位.
//参数:@iGrp	要获取的总加组.
//	   @iSel	控制类(0: 功控; 1: 电控).
//返回: 返回所有的标志位状态.
BYTE CGrpCtrl::GetSysCtrlFlgs(int iGrp, int iSel)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return 0x00;
	if (iSel<0 || 1<iSel)
		return 0x00;

	BYTE bBuf[1+1+6*8];

	if (ReadItemEx(BN0, PN0, 0x104f, bBuf) <=0)	//读"终端控制设置状态"ID
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::GetSysCtrlFlgs: There is something wrong when call ReadItemEx() !\n"));
		return 0x00;
	}
	if (iSel == 0)
		return (bBuf[1+1+6*(iGrp-GRP_START_PN)+2] & 0x0f);	//目前只有0,1,3,4位被使用.
	else
		return (bBuf[1+1+6*(iGrp-GRP_START_PN)+3] & 0x03);	//目前只有0,1位被使用.
}

//描述: 改变系统库指定总加组指定控制类的指定标志位状态,包括F5(终端控制设置状态)和F6(终端当前控制状态)
//参数:@iGrp	要改变的总加组.
//	   @bFlgs	要改变的标志位.
//	   @fStatus	要变成的状态.
//	   @iCtrlType 控制类(0: 功控; 1: 电控).
//返回: 如改变成功返回 true,否则返回 false.
bool CGrpCtrl::ChgSysCtrlFlgs(int iGrp, BYTE bFlgs, bool fStatus, int iCtrlType)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;
	if (iCtrlType<0 || 1<iCtrlType)
		return false;

	int i = iGrp - GRP_START_PN;
	/*BYTE bSetStatusBuf[1+1+6*8];
	BYTE bCurStatusBuf[1+1+1+8*8];*/
	TGrpCtrlSetSta tGrpCtrlSetSta;
	TGrpCurCtrlSta tGrpCtrlCtrlSta;
	memset(&tGrpCtrlSetSta, 0, sizeof(TGrpCtrlSetSta));
	memset(&tGrpCtrlCtrlSta, 0, sizeof(tGrpCtrlCtrlSta));

	if (iCtrlType == 0)
		bFlgs &= 0x0f;	//目前只有0,1,3,4位被使用.
	else
		bFlgs &= 0x03;	//目前只有0,1位被使用.

	if (!GetGrpCtrlSetSta(iGrp, &tGrpCtrlSetSta)) //读"终端控制设置状态".
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::ChgSysCtrlFlgs: There is something wrong when call GetGrpCtrlSetSta() !\n"));
		return false;
	}

	if (!GetGrpCurCtrlSta(iGrp, &tGrpCtrlCtrlSta)) //读"终端当前控制状态".
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::ChgSysCtrlFlgs: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	/*if (ReadItemEx(BN0, PN0, 0x104f, bSetStatusBuf) <=0)	//读"终端控制设置状态".
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::ChgSysCtrlFlgs: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	if (ReadItemEx(BN0, PN0, 0x105f, bCurStatusBuf) <=0)	//读"终端当前控制状态".
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::ChgSysCtrlFlgs: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}*/

	int i1;
	//改变控制状态.
	if (fStatus)
	{
		//bSetStatusBuf[1+1+6*i+2+iCtrlType] |= bFlgs; //电控状态/功控状态
		if (iCtrlType == 0)
			tGrpCtrlSetSta.bPwrCtrlSta |= bFlgs;	//功控状态
		else
			tGrpCtrlSetSta.bEngCtrlSta |= bFlgs;	//电控状态


		//添加对临时下浮控系数的赋值，临时下浮控以外的控制投入时，临时下浮控系数都置为无效数据.
		if (iCtrlType == 0)
		{
			for (i1=0; i1<4; i1++,bFlgs>>=1)
			{
				if ((bFlgs&0x01) == 0)
					continue;
				switch (i1)
				{
				case 0:	//时段控
				case 1:	//厂休控
				case 2:	//营业报停控
					//bCurStatusBuf[1+1+1+8*i+2] = 0xee;	//临时下浮控浮动系数置为无效数据.
					tGrpCtrlCtrlSta.FloatRate = 0xee;
					break;
				case 3:	//临时下浮控
					break;
				default:
					return false;
				}
			}
		}
		/*else
		{
			for (i1=0; i1<2; i1++,bFlgs>>=1)
			{
				if ((bFlgs&0x01) == 0)
					continue;

				switch (i1)
				{
				case 0:	//月电控
				case 1:	//购电控
					bCurStatusBuf[1+1+1+8*i+2] = 0xee;	//临时下浮控浮动系数置为无效数据.
					break;
				default:
					return false;
				}
			}
		}*/
	}
	else
	{
		//bSetStatusBuf[1+1+6*i+2+iCtrlType] &= ~bFlgs; //电控状态/功控状态
		if (iCtrlType == 0)
			tGrpCtrlSetSta.bPwrCtrlSta &= ~bFlgs;//功控状态
		else
			tGrpCtrlSetSta.bEngCtrlSta &= ~bFlgs;//电控状态

		//针对解除的控制进行相关操作.
		if (iCtrlType == 0)
		{//功控
			for (i1=0; i1<4; i1++,bFlgs>>=1)
			{
				if ((bFlgs&0x01) == 0)
					continue;
				switch (i1)
				{
				case 0:	//时段控
					tGrpCtrlSetSta.bSchemeNum = 0xee;	//时段控方案号置为无效数据.
					tGrpCtrlSetSta.bValidFlag = 0x00;	//时段控有效标志位全置0.
					break;
				case 1:	//厂休控
					break;
				case 2:	//营业报停控
					break;
				case 3:	//临时下浮控
					//bCurStatusBuf[1+1+1+8*i+2] = 0xee;	//临时下浮控浮动系数置为无效数据.
					tGrpCtrlCtrlSta.FloatRate = 0xee;
					break;
				default:
					return false;
				}
			}
			//检测当前总加组的功控状态.
			if (tGrpCtrlSetSta.bPwrCtrlSta == 0x00)
			{
				tGrpCtrlSetSta.bPwrCtrlTurnSta = 0x00;		//将功控轮次状态设为全部不可控.
				tGrpCtrlCtrlSta.CurPwrVal = 0xee;		//将当前功控定值置为无效数据.
				//bCurStatusBuf[1+1+1+8*i+1] = 0xee;
				tGrpCtrlCtrlSta.bAllPwrCtrlOutPutSta = 0x00;		//功控跳闸输出状态全部置为允许合闸状态.
			}
			//根据逻辑上的相关性,作者认为,如果某个控制类解除的话,那么该控制类对应的报警状态也应复位.
			//bCurStatusBuf[1+1+1+8*i+6] &= bSetStatusBuf[1+1+6*i+2];
			tGrpCtrlCtrlSta.bPCAlarmState &= tGrpCtrlSetSta.bPwrCtrlSta;
		}
		else
		{//电控
			for (i1=0; i1<2; i1++,bFlgs>>=1)
			{
				if ((bFlgs&0x01) == 0)
					continue;
				switch (i1)
				{
				case 0:	//月电控
					//bCurStatusBuf[1+1+1+8*i+4] = 0x00;	//月电控跳闸输出状态全部置为允许合闸状态.
					tGrpCtrlCtrlSta.bMonthCtrlOutPutSta = 0x00;
					break;
				case 1:	//购电控
					//bCurStatusBuf[1+1+1+8*i+5] = 0x00;	//购电控跳闸输出状态全部置为允许合闸状态.
					tGrpCtrlCtrlSta.bBuyCtrlOutPutSta = 0x00;
					break;
				default:
					return false;
				}
			}
			//检测当前总加组的电控状态.
			if (tGrpCtrlSetSta.bEngCtrlSta == 0x00)
				tGrpCtrlSetSta.bEngCtrlTurnSta = 0x00;		//将电控轮次状态设为全部不可控.
			//根据逻辑上的相关性,作者认为,如果某个控制类解除的话,那么该控制类对应的报警状态也应复位.
			//bCurStatusBuf[1+1+1+8*i+7] &= bSetStatusBuf[1+1+6*i+3];
			tGrpCtrlCtrlSta.bECAlarmState &= tGrpCtrlSetSta.bEngCtrlSta;
		}
	}

	//检测当前总加组的电控和功控是否还有投入的控制,以此为依据改变当前总加组的有效标志.
//	if ((bSetStatusBuf[1+1+6*i+2] | bSetStatusBuf[1+1+6*i+3]) == 0x00)
//		bSetStatusBuf[1] &= ~(0x01 << i);	//如该总加组既没有功控又没有电控投入,那么表示该总加组未投入.
//	else

	/*for(int in=0;in<i+1;in++)  //设置总加组有效标识
		bSetStatusBuf[1] |= (0x01 << in);
	bCurStatusBuf[2] = bSetStatusBuf[1];*/

	if (!SetGrpCtrlSetSta(iGrp, &tGrpCtrlSetSta)) //写"终端控制设置状态".
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::ChgSysCtrlFlgs: There is something wrong when call SetGrpCtrlSetSta() !\n"));
		return false;
	}

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCtrlCtrlSta)) //写"终端当前控制状态".
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::ChgSysCtrlFlgs: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}

	/*WriteItemEx(BN0, PN0, 0x104f, bSetStatusBuf);	//写"终端控制设置状态".
	WriteItemEx(BN0, PN0, 0x105f, bCurStatusBuf);	//写"终端当前控制状态".*/

	return true;
}



//描述：从系统库中删除无效的控制状态；
//参数：@tInvCtrl   无效的控制
//返回：无
void CGrpCtrl::RemoveDispItem(TCtrl tInvCtrl)
{
	BYTE bBuf[21];
	memset(bBuf, 0, sizeof(bBuf));

	if (ReadItemEx(BN1, PN0, 0x3010, bBuf)>0 && bBuf[0]>0)
	{
		BYTE bSize = bBuf[0]; //告警的个数；
		BYTE bInvCtrlType = tInvCtrl.bCtrlType;
		int iIndex = 0; //失效的显示告警类型在队列中的位置；

		//查找失效的显示告警类型在数组中的索引
/*		for (BYTE i=0; i<bSize; i++)
		{
			if (bBuf[i*2+1] == bInvCtrlType)
			{
				iIndex = i;
				break;
			}
		}
*/
		//重新排序
		if (iIndex >= 0 && bSize > 0)
		{
			bSize --;
			bBuf[0] = bSize;
			BYTE* pbBuf = &bBuf[iIndex*2+3];
			memcpy(bBuf+iIndex*2+1, pbBuf, (bSize-iIndex)*2);
			memset(bBuf+bSize*2+1, 0, 20-bSize*2); 
			WriteItemEx(BN1, PN0, 0x3010, bBuf);
			DTRACE(DB_LOADCTRL, ("CGrpCtrl::RemoveDispItem: tCtrlType=%d, iGrp=%d!\n", tInvCtrl.bCtrlType, m_iCtrlGrp));
		}
	}
}

//描述：向系统库中增加新的控制状态；
//参数：@tTopCtrl  新的控制（置于数组首位）；
//返回：无
void CGrpCtrl::AddDispItem(TCtrl tTopCtrl)
{
	BYTE bBuf[21], bTmpBuf[21];
	memset(bBuf, 0, sizeof(bBuf));

	if (ReadItemEx(BN1, PN0, 0x3010, bTmpBuf) > 0)
	{
		BYTE bSize = bTmpBuf[0]; //以前的个数；

		for (BYTE i=0; i<bSize; i++)
		{
			if (bTmpBuf[1+2*i]==tTopCtrl.bCtrlType)
				return;
		}

		BYTE *pbBuf = &bTmpBuf[1];
		bBuf[1] = tTopCtrl.bCtrlType;
		bBuf[2] = tTopCtrl.bCtrlTurn;
		if (bSize == 10)
		{
			memcpy(bBuf+3, pbBuf,18);
		}
		else
		{
			memcpy(bBuf+3, pbBuf, bSize*2);
			memset(bBuf+bSize*2+3, 0, 18-bSize*2);
			bSize++;
		}
		bBuf[0] = bSize; 
		WriteItemEx(BN1, PN0, 0x3010, bBuf);
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::AddDispItem: tCtrlType=%d, iGrp=%d!\n",tTopCtrl.bCtrlType, m_iGrp));
	}
}



//描述：生成系统库的控制状态；
void CGrpCtrl::MakeDisp(BYTE bTurnsStatus)
{
	TCtrl tCtrl;
	TCtrl tInvCtrl;

	bool fAlarmStatus = IsAlarmStatus();
	bool fOpenStatus = IsOpenStatus();

	if (m_iCtrlGrp != m_iGrp)
	{//总加组切换，删除库中以前含有此类型的控制状态；
		tInvCtrl.bCtrlType = GetCtrlType();
		RemoveDispItem(tInvCtrl);
		tInvCtrl.bCtrlType = GetInvCtrlType();
		RemoveDispItem(tInvCtrl);
		
		if (fAlarmStatus)
		{//如果新的总加组处于告警状态；
			tCtrl.bCtrlType = GetCtrlType();
			
			int iSel;
			if (tCtrl.bCtrlType > 3) //功控
				iSel = 0;
			else                   //电控；
				iSel = 1;
			m_bCloseTurn = GetSysCtrlTurnsCfg(m_iGrp, iSel); //合闸告警的轮次；
			tCtrl.bCtrlTurn = m_bCloseTurn;
			AddDispItem(tCtrl);
		}
		else if (m_iGrp==-1 && m_bTurnsStatus==0 && m_fOpenStatus)
		{//以前处于功控跳闸状态，现在处于合闸状态，则发出合闸告警信息；
			tCtrl.bCtrlType = GetInvCtrlType();
			tCtrl.bCtrlTurn = bTurnsStatus;
			AddDispItem(tCtrl);
			m_dwInitClick = GetClick();

			DTRACE(DB_LOADCTRL, ("CGrpCtrl::MakeDisp: The Turns Cancelled is = %d!\n", tCtrl.bCtrlTurn));
		}
	}
	else
	{
		if (fAlarmStatus != m_fAlarmStatus)//同一总加组的控制状态发生变化；
		{
			//删除以前的控制状态；
			tInvCtrl.bCtrlType = m_fAlarmStatus ? GetCtrlType(): GetInvCtrlType();
			RemoveDispItem(tInvCtrl);

			if (fAlarmStatus)
			{
				tCtrl.bCtrlType =  GetCtrlType();

				int iSel;
				if (tCtrl.bCtrlType > 3) //功控
					iSel = 0;
				else                   //电控；
					iSel = 1;
				m_bCloseTurn = GetSysCtrlTurnsCfg(m_iGrp, iSel);
				tCtrl.bCtrlTurn = m_bCloseTurn;
				AddDispItem(tCtrl);
			}
			else if (m_bTurnsStatus==0 && m_fOpenStatus)
			{
				tCtrl.bCtrlTurn = bTurnsStatus;
				tCtrl.bCtrlType = GetInvCtrlType();
				AddDispItem(tCtrl);

				m_dwInitClick = GetClick();//如果控制状态为合闸状态，记录下合闸刚开始的滴答；
				DTRACE(DB_LOADCTRL, ("CGrpCtrl::MakeDisp: The Turns Cancelled is = %d!\n", m_bCloseTurn));
			}
			
		}
		else if (!fAlarmStatus)
		{//如果是合闸状态，过若干秒后自动从系统库中删除；
			if (m_dwInitClick>0 && (GetClick()- m_dwInitClick)>=CTL_TURNCLOSE_TICK)
			{
				tInvCtrl.bCtrlType = GetInvCtrlType();
				RemoveDispItem(tInvCtrl);
				m_dwInitClick = 0;
			}
		}
	}
	m_fAlarmStatus = fAlarmStatus;
	m_fOpenStatus = fOpenStatus;
	m_iCtrlGrp = m_iGrp;
}

//========================================= CEngCtrl ===============================================
//描述: 用于初始化时,将系统库中指定类控制的轮次状态,报警状态等同步到内存中对应的变量.
//参数:@iSel	控制类型(0: 月电控; 1: 购电控).
//返回: 成功返回 true, 否则返回 false.
bool CEngCtrl::GetSysEngStatus(int iSel)
{
	BYTE bSetStatusBuf[1+1+6*8];
	BYTE bCurStatusBuf[1+1+1+8*8];

	if (ReadItemEx(BN0, PN0, 0x104f, bSetStatusBuf) <=0)	//读"终端控制设置状态".
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::GetSysEngStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	if (ReadItemEx(BN0, PN0, 0x105f, bCurStatusBuf) <=0)	//读"终端当前控制状态".
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::GetSysEngStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	BYTE bMask;

	switch (iSel)
	{
	case 0:	//月电控
		bMask = 0x01;
		break;
	case 1:	//购电控
		bMask = 0x02;
		break;
	default:
		return false;
	}

	BYTE bGrpFlgs = bSetStatusBuf[1];

	for (int i=0; i<8; i++,bGrpFlgs>>=1)
	{
		if ((bGrpFlgs&0x01)!=0 && (bSetStatusBuf[2+6*i+3]&bMask)!=0)
		{
			m_bTurnsStatus = bCurStatusBuf[3+8*i+4];
			if ((bCurStatusBuf[3+8*i+7]&bMask) != 0)
				m_fAlrStauts = true;
			break;
		}
	}

	return true;
}

//描述: 获取系统库指定总加组电能量控制类的所有报警状态标志位.
//参数:@iGrp	要获取的总加组.
//返回: 返回所有的标志位状态.
BYTE CEngCtrl::GetSysEngAlrFlgs(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return 0x00;

	BYTE bBuf[1+1+1+8*8];

	if (ReadItemEx(BN0, PN0, 0x105f, bBuf) <=0)	//读"终端当前控制状态"ID
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::GetSysEngAlrFlgs: There is something wrong when call ReadItemEx() !\n"));
		return 0x00;
	}

	return (bBuf[1+1+1+8*(iGrp-GRP_START_PN)+7] & 0x03);	//目前只有0,1位被使用.
}

//描述: 改变系统库指定总加组电能量控制类的指定报警状态标志.
//参数:@iGrp	要改变的总加组.
//	   @bFlgs	要改变的标志位.
//	   @fStatus	要变成的状态.
//返回: 如改变成功返回 true,否则返回 false.
bool CEngCtrl::ChgSysEngAlrFlgs(int iGrp, BYTE bFlgs, bool fStatus)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	//int i = iGrp - GRP_START_PN;
	//BYTE bBuf[1+1+1+8*8];
	TGrpCurCtrlSta tGrpCurCtrlSta;
	memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

	bFlgs &= 0x03;	//目前只有0,1位被使用.
	
	if(!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::ChgSysEngAlrFlgs: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	if (fStatus)
		tGrpCurCtrlSta.bECAlarmState |= bFlgs;
	else
		tGrpCurCtrlSta.bECAlarmState &= ~bFlgs;

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::ChgSysEngAlrFlgs: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}
	return true;

	/*//!!!如果在别的线程中会写该ID,可能需要进行信号量保护.
	if (ReadItemEx(BN0, PN0, 0x105f, bBuf)<=0)	//读"终端当前控制状态".
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::ChgSysEngAlrFlgs: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	if (fStatus)
		bBuf[1+1+1+8*i+7] |= bFlgs;
	else
		bBuf[1+1+1+8*i+7] &= ~bFlgs;

	WriteItemEx(BN0, PN0, 0x105f, bBuf);	//写"终端当前控制状态".
	return true;*/
}

//描述: 设定系统库指定总加组指定控制类的轮次状态.
//参数:@iGrp 			要设置的总加组.
//	   @bTurnsStatus	设定的轮次状态.
//	   @iSel			控制类型(0: 月电控; 1: 购电控).
//返回: 如果设置成功返回 true,否则返回 false.
bool CEngCtrl::SetSysEngTurnsStatus(int iGrp, BYTE bTurnsStatus, int iSel)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;
	if (iSel<0 || 1<iSel)
		return false;

	BYTE bBuf[1+1+1+8*8];	//最多8组数据.

	//!!!如果在别的线程中会写该ID,可能需要进行信号量保护
	if (ReadItemEx(BN0, PN0, 0x105f, bBuf) <=0)	//读"终端当前控制状态".
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::ChgSysEngTurnsStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	bBuf[3+(8*(iGrp-GRP_START_PN))+4+iSel] = bTurnsStatus & CTL_TURN_MASK;

	WriteItemEx(BN0, PN0, 0x105f, bBuf);	//写"终端当前控制状态".

	return true;
}

//描述: 改变系统库指定总加组指定控制类的相应轮次状态.
//参数:@iGrp 		要设置的总加组.
//	   @bTurns 		所要设置的轮次.
//	   @fStatus		true: 置位相应位; false: 清除相应位.
//	   @iSel		控制类型(0: 月电控; 1: 购电控).
//返回: 如果设置成功返回 true,否则返回 false.
bool CEngCtrl::ChgSysEngTurnsStatus(int iGrp, BYTE bTurns, bool fStatus, int iSel)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;
	if (iSel<0 || 1<iSel)
		return false;

	BYTE bBuf[1+1+1+8*8];	//最多8组数据.

	//!!!如果在别的线程中会写该ID,可能需要进行信号量保护
	if (ReadItemEx(BN0, PN0, 0x105f, bBuf) <=0)	//读"终端当前控制状态".
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::ChgSysEngTurnsStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	if (fStatus)
		bBuf[3+(8*(iGrp-GRP_START_PN))+4+iSel] |= (bTurns & CTL_TURN_MASK);
	else
		bBuf[3+(8*(iGrp-GRP_START_PN))+4+iSel] &= ~(bTurns & CTL_TURN_MASK);

	WriteItemEx(BN0, PN0, 0x105f, bBuf);	//写"终端当前控制状态".

	return true;
}

//描述: 获取指定轮次的电控跳闸间隔
//参数:@iTurn 	轮次号.
//返回: 如果轮次有效,返回相应的报警持续时间,否则返回 DWORD 型最大值.
DWORD CEngCtrl::GetEngTurnInv(int iTurn)
{
	if (iTurn<TURN_START_PN || iTurn>TURN_START_PN+TURN_NUM)
		return ((DWORD)-1);

	BYTE bBuf[10] = {0};
	BYTE *ptr = bBuf+2;
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;

	if (OoReadAttr(0x8102, 0x02, bBuf, &pbFmt, &wFmtLen) < 0)
	{
		DTRACE(DB_LOADCTRL, ("CPwrCtrl::GetPwrAlrPersistTime: There is something wrong when call OoReadAttr() !\n"));
		return ((DWORD)-1);
	}

	DWORD dwTime = (DWORD)ptr[iTurn-1] * 60;

	return dwTime;


	/*BYTE b;

	if (ReadItemEx(BN0, (WORD)iTurn, 0x0a06, &b) <=0)	//读取指定轮次"功控告警时间".
	{
		DTRACE(DB_LOADCTRL, ("CPwrCtrl::GetPwrAlrPersistTime: There is something wrong when call ReadItemEx() !\n"));
		return ((DWORD)-1);
	}

//	if (b == 0)
//		b = 1;
	DWORD dwTime = (DWORD)b * 60;

//	if (dwTime < CTL_POWER_ALR_MIN_TIME)	//时间如过小于最小延迟时间,以最小延迟时间为准.
//		dwTime = CTL_POWER_ALR_MIN_TIME;

	return dwTime;*/
}

//========================================= CPwrCtrl ===============================================
CPwrCtrl::CPwrCtrl(void)
{	
	m_dwFrzDly = 60 * 2;	//功控跳闸后功率冻结延时.除了临时下浮控,其它功控都是延时2分钟.
	m_iPwrLimit = 0;		//当前的功率定值,即上限.
	m_dwAlrTime = 0;		//报警启动的时间.
	m_dwGuaranteeAlrTime = 0;

	m_iCurPwrLimit = 0;		//当前功率定值.

	memset(&m_OLStat, 0, sizeof(m_OLStat));			//超限统计.

	m_dwPwrStartClick = 0;
}

//描述: 保存功控跳闸记录.
//参数:@iSel	控制类型(0:时段控; 1:厂休控; 2:营业报停控; 3:临时下浮控).
void CPwrCtrl::DoSavePwrCtrlOpenRec(int iSel)
{
	WORD wID;
	BYTE bPwrCtrlType;

	switch (iSel)
	{
	case 0:	//时段控
		wID			 = 0x0a04;
		bPwrCtrlType = 0x01;
		break;
	case 1:	//厂休控
		wID			 = 0x0a03;
		bPwrCtrlType = 0x02;
		break;
	case 2:	//营业报停控
		wID			 = 0x0a02;
		bPwrCtrlType = 0x04;
		break;
	case 3:	//临时下浮控
		wID			 = 0x0a01;
		bPwrCtrlType = 0x08;
		break;
	default:
		return;
	}
	for (int i=TURN_START_PN; i<TURN_START_PN+TURN_NUM; i++)
	{
		BYTE bRecBuf[32];
		DWORD dwTime;

		if (ReadItemEx(BN0, (WORD)i, wID, bRecBuf) <=0)
		{
			DTRACE(DB_LOADCTRL, ("CPwrCtrl::DoSaveOpenRec: There is something wrong when call ReadItemEx() !\n"));
			return;
		}
		memcpy(&dwTime, bRecBuf, 4);
		if (dwTime > m_dwNow)	//变成未来的时间了,时间往前调回去了
		{	
			dwTime = m_dwNow;
			memcpy(bRecBuf, &dwTime, 4);
			WriteItemEx(BN0, (WORD)i, wID, bRecBuf);	//把数据库的时间也改了
		}

		if (dwTime != 0)
		{//如有跳闸情况发生,必须在跳闸后2分钟记录冻结功率.
			if (m_dwNow >= dwTime+m_dwFrzDly)
			{
				BYTE bBuf[32];
				int64 iCurPwr;
				TTime tm;

				SecondsToTime(dwTime, &tm);
				bBuf[0] = bRecBuf[4];						//总加组
				bBuf[1] = 8;								//跳闸轮次格式bitstring,长度1个字节
				bBuf[2] = (BYTE)(0x01<<(i-TURN_START_PN));	//轮次
				bBuf[3] = 8;								//功控类别格式bitstring,长度1个字节
				bBuf[4] = bPwrCtrlType;						//功控类别

				memcpy(bBuf+5, bRecBuf+5, 4);				//跳闸前功率
				iCurPwr = GetCurPwr(bRecBuf[4]);			//获得当前功率
				if (bPwrCtrlType == 0x08)	//临时下浮控
				{
					WriteItemVal64(BN0, (WORD)m_iGrp, 0x111f, &iCurPwr);
				}

				Val32ToBin(iCurPwr, bBuf+9, 4);		
				memcpy(bBuf+13, bRecBuf+9, 4);				//跳闸时功率定值

				memset(bRecBuf, 0, sizeof(bRecBuf));
				WriteItemEx(BN0, (WORD)i, wID, bRecBuf);		//清空跳闸中间数据.
				TrigerSaveBank(BN0, SECT_CTRL, 0);

				//记录当前跳闸记录到系统库中.
				//SaveAlrData(ERC_PWRCTL, tm, bBuf, 17);

				char cTime[20];
				char cTime1[20];

				DTRACE(DB_LOADCTRL, ("CPwrCtrl::DoSaveOpenRec: In the %d seconds after open break of Turn[%d], recorded the power.\n"\
									 "Time of open break is %s, power is %lld\n"\
									 "at %s, power is %lld,bPwrCtrlType=%d.\n", m_dwFrzDly, i,
									 TimeToStr(tm, cTime1), Fmt2ToVal64(bBuf+3, 2),
									 TimeToStr(m_tmNow, cTime), iCurPwr,bPwrCtrlType));
			}
		}
	}
}

//描述: 获取功控保安定值.
//返回: 功控保安定值.
int64 CPwrCtrl::GetPwrSafeLimit(void)
{
	BYTE bBuf[10];

	if (ReadItemEx(BN0, PN0, 0x8100, bBuf) <=0)	//读取"终端保安定值".
	{
		DTRACE(DB_LOADCTRL, ("CPwrCtrl::GetPwrSafeLimit: There is something wrong when call ReadItemEx() !\n"));
		return 0;
	}

	return OoLong64ToInt64(&bBuf[1]);
}

//描述: 获取指定轮次的功控报警持续时间.
//参数:@iTurn 	轮次号.
//返回: 如果轮次有效,返回相应的报警持续时间,否则返回 DWORD 型最大值.
DWORD CPwrCtrl::GetPwrAlrPersistTime(int iTurn)
{
	if (iTurn<TURN_START_PN || iTurn>TURN_START_PN+TURN_NUM)
		iTurn = TURN_NUM;

	BYTE bBuf[20];

	if (ReadItemEx(BN0, PN0, 0x8102, bBuf) <=0)	//读取指定轮次"功控告警时间".
	{
		DTRACE(DB_LOADCTRL, ("CPwrCtrl::GetPwrAlrPersistTime: There is something wrong when call ReadItemEx() !\n"));
		return ((DWORD)-1);
	}

	DWORD dwTime = (DWORD)bBuf[1+iTurn*2] * 60;

	if (dwTime < CTL_POWER_ALR_MIN_TIME)	//时间如过小于最小延迟时间,以最小延迟时间为准.
		dwTime = CTL_POWER_ALR_MIN_TIME;

	return dwTime;
}

//描述: 获取指定轮次的功控功率滑差时间,单位秒
//参数:@iTurn 	轮次号.
//返回: 功控功率滑差时间,单位秒
DWORD CPwrCtrl::GetPwrSlideInterv(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return 0;

	BYTE bBuf[2] = {0};

	//if (ReadItemEx(BN0, (WORD)iGrp, 0x02bf, &b) <=0)	//F43
	if (ReadItemEx(BN0, (WORD)iGrp, 0x230c, bBuf) <= 0)
		return 0;

	if (bBuf[1] > 60)
		bBuf[1] = 60;

	return (DWORD)bBuf[1] * 60;
}

//描述: 设定指定总加组当前功控定值.
//参数:@iGrp		要设定的总加组
//     @iPwrLimit	设定的功控定值
bool CPwrCtrl::SetSysCurPwrLimit (int iGrp, int64 iPwrLimit)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	TGrpCurCtrlSta tGrpCurCtrlSta;
	memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

	if (!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//读"终端当前控制状态".
	{
		DTRACE(DB_LOADCTRL, ("CPwrCtrl::SetSysCurPwrLimit: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	tGrpCurCtrlSta.CurPwrVal = iPwrLimit;

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//写"终端当前控制状态".
	{
		DTRACE(DB_LOADCTRL, ("CPwrCtrl::SetSysCurPwrLimit: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}

	return true;
}

//描述: 累加功率定值超限参数(超限时间及超限电量).
//参数:@dwTime		累加时间的引用
//     @iEng		累加月电量的引用
void CPwrCtrl::SumOverLimitPara(int& riGrp, DWORD& rdwTime, int64& riEng)
{
	int64 iTmpEnergy;
	if (m_OLStat.fIfOverLimit)
	{//假如之前的状态是超限状态.
		DWORD dwClick = GetClick();

		if (!m_fIfOverLimit	|| m_OLStat.iGrp!=m_iGrp || (dwClick - m_OLStat.dwClick)>60)
		{//假如超限状态或分钟或总加组发生了变化,则需累加统计数据.
			rdwTime = dwClick - m_OLStat.dwClick;				//统计超限时间.
			iTmpEnergy = GetCurEng(m_OLStat.iGrp);
			riEng = iTmpEnergy - m_OLStat.iEng;	//统计超限月电量.
			riGrp = m_OLStat.iGrp;

			if (m_fIfOverLimit)
			{
				m_OLStat.iGrp = m_iGrp;
				m_OLStat.dwClick = dwClick;
				m_OLStat.iEng = iTmpEnergy;
			}
			else
			{
				m_OLStat.fIfOverLimit = false;
				m_OLStat.iGrp = -1;
				m_OLStat.dwClick = 0;
				m_OLStat.iEng = 0;
			}
		}
	}
	else //(!m_OLStat.fIfOverLimit)
	{//假如之前的状态是未超限状态.
		if (m_fIfOverLimit)
		{//现在的状态是超限状态,将启动一次统计.
			iTmpEnergy = GetCurEng(m_iGrp);
			if (iTmpEnergy >= 0)
			{
				m_OLStat.iEng = iTmpEnergy;
				m_OLStat.fIfOverLimit = true;
				m_OLStat.iGrp = m_iGrp;
				m_OLStat.dwClick = GetClick();
			}
			else
				m_OLStat.fIfOverLimit = false;
			  
		}
	}
}

//描述:是否合闸
//返回: 如果设置成功则返回 true, 否则返回 false.
bool CPwrCtrl::RestoreTurnStatus()
{
	BYTE bEnableClose = 0;
	DWORD dwPersistTime;
	char msg[100];
	int i;

	//ReadItemEx(BN0, PN0,0x0a08, &bEnableClose);//是否允许合闸
	//if (bEnableClose) 
	{
		i = GetIdxOfMostLeft1(m_bTurnsStatus);	//获取相应总加组当前可跳闸的轮次号.		
		dwPersistTime = GetPwrAlrPersistTime(i);	//获取相应轮次的功控报警持续时间.	

		if (m_dwNow > m_dwAlrTime+dwPersistTime)
		{
			m_bTurnsStatus &= ~(1<<(i-1));
			m_dwAlrTime = 0;
		}
	}
//	DTRACE(DB_LOADCTRL, ("CPwrCtrl::RestoreTurnStatus: i=%d, dwPersistTime=%d,m_bTurnsStatus=%d!\n", i, dwPersistTime,m_bTurnsStatus));
	return true;
	
}

//描述：生成功控越限时的显示状态；
//参数：@dwStime  功控跳闸告警开始时间；
//@wDelayTime 功控跳闸告警延时时间；
//@iCurPwr      功控当前功率值；
void CPwrCtrl::SaveDisp(WORD wDelayTime, DWORD dwStime, int64 iCurPwr)
{
	BYTE bBuf[25];
	
	bBuf[0] = GetCtrlType();
	bBuf[1] = m_bWarnStatus;
	wDelayTime = wDelayTime;
	memcpy(bBuf+2,&wDelayTime,2);
	Val64ToFmt(iCurPwr, bBuf+4, FMT2, 2);
	Val64ToFmt(m_iCurPwrLimit, bBuf+6, FMT2, 2);
	memcpy(bBuf+8,&dwStime,4);
	WriteItemEx(BN0, PN0, 0x0920, bBuf); //功控告警信息界面

	DTRACE(DB_LOADCTRL, ("CPwrCtrl::SavePwrCtrl: m_iCurPwrLimit=%lld, iCurPwr=%lld,CtrlType=%d!\n", m_iCurPwrLimit, iCurPwr,bBuf[0]));
}


