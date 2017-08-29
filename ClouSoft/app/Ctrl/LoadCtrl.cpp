/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：LoadCtrl.cpp
 * 摘    要：本文件主要实现CLoadCtrl的类
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年2月
*********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "sysfs.h"
#include "FaCfg.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "DbConst.h"
#include "ComAPI.h"
#include "TaskDB.h"
#include "DbAPI.h"
#include "LoadCtrl.h"
#include "YK.h"
#include "board.h"
#include "DrvCtrlAPI.h"
#include "DpGrp.h"

#ifndef SYS_WIN
#include "bios.h"
#endif

CLoadCtrl g_LoadCtrl;

//========================================= CAllPwrCtrl ============================================
//
CAllPwrCtrl::CAllPwrCtrl(void)
{
	m_iGrp  = -1;
	m_bTurnsStatus = 0x00;
	m_bAlrsStatus = 0x00;
}

CAllPwrCtrl::~CAllPwrCtrl()
{
}

//描述: 功控初始化
bool CAllPwrCtrl::Init(void)
{
	m_TmpCtrl.Init();
	m_ShutoutCtrl.Init();
	m_RestCtrl.Init();
	m_PeriodCtrl.Init();

	//不要删除下面这段,在需要保留掉电前控制状态的时候开放这一段
	/*BYTE bSetStatusBuf[1+1+6*8];
	BYTE bCurStatusBuf[1+1+1+8*8];

	if (ReadItemEx(BN0, PN0, 0x104f, bSetStatusBuf) != sizeof(bSetStatusBuf))	//读"终端控制设置状态".
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::Init: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	if (ReadItemEx(BN0, PN0, 0x105f, bCurStatusBuf) != sizeof(bCurStatusBuf))	//读"终端当前控制状态".
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::Init: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	BYTE bGrpFlgs = bSetStatusBuf[1];
	BYTE bPwrCtrl = 0;
	int j = -1;

	for (int i=0; i<8; i++,bGrpFlgs>>=1)
	{
		if ((bGrpFlgs&0x01)!=0 && bPwrCtrl<bSetStatusBuf[2+6*i+2])
		{//从有效的总加组数据中,找到包含优先级最高的功控的那一组.
			bPwrCtrl = bSetStatusBuf[2+6*i+2];
			j = i;
		}
	}
	if (j >= 0)
	{
		m_bTurnsStatus = bCurStatusBuf[3+8*j+3];
		m_bAlrsStatus  = bCurStatusBuf[3+8*j+6];
	}*/

	m_bTurnsStatus = 0;
	m_bAlrsStatus = 0;

	return true;
}

//描述: 功控执行
//返回: 运行正常返回 true,否则返回 false.
bool CAllPwrCtrl::DoCtrl(void)
{
	//m_TmpCtrl.DoSaveOpenRec();
	//m_ShutoutCtrl.DoSaveOpenRec();
	//m_RestCtrl.DoSaveOpenRec();
	//m_PeriodCtrl.DoSaveOpenRec();

	BYTE bTurnsStatus = 0x00;
	BYTE bAlrsStatus = 0x00;
	int iGrp;
#ifndef SYS_WIN
	if(m_TmpCtrl.IsValid()||m_ShutoutCtrl.IsValid()||m_RestCtrl.IsValid()||m_PeriodCtrl.IsValid())
	{//点亮功控灯
		SetCtrlLed(true, LED_POWERCTRL); 
	}
	else
	{
		SetCtrlLed(false, LED_POWERCTRL); 
	}
#endif
	m_TmpCtrl.DoCtrl();	//执行'临时下浮控'.
	if (m_TmpCtrl.IsValid())
	{
//		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::DoCtrl: cur ctrl is TMP!\n"));

		bTurnsStatus = m_TmpCtrl.GetTurnsStatus();
		if (m_TmpCtrl.IsAlr())
			bAlrsStatus = 0x08;
		iGrp = m_TmpCtrl.GetGrp();
		m_ShutoutCtrl.RstCtrl();
		m_ShutoutCtrl.DoCmdScan();
		m_RestCtrl.RstCtrl();
		m_RestCtrl.DoCmdScan();
		m_PeriodCtrl.RstCtrl();
		m_PeriodCtrl.DoCmdScan();
	}
	else
	{//假如'临时下浮控'没有投入.
		m_ShutoutCtrl.DoCtrl();	//执行'营业报停控'.
		if (m_ShutoutCtrl.IsInCtrl())
		{
//			DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::DoCtrl: cur ctrl is SHUTOUT!\n"));

			bTurnsStatus = m_ShutoutCtrl.GetTurnsStatus();
			if (m_ShutoutCtrl.IsAlr())
				bAlrsStatus = 0x04;
			iGrp = m_ShutoutCtrl.GetGrp();
			m_RestCtrl.RstCtrl();
			m_RestCtrl.DoCmdScan();
			m_PeriodCtrl.RstCtrl();	//把有效标志给清掉了
			m_PeriodCtrl.DoCmdScan();
		}
		else
		{//假如'营业报停控'没有投入.
			m_RestCtrl.DoCtrl();	//执行'厂休控'.
			if (m_RestCtrl.IsInCtrl())
			{
//				DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::DoCtrl: cur ctrl is REST!\n"));

				bTurnsStatus = m_RestCtrl.GetTurnsStatus();
				if (m_RestCtrl.IsAlr())
					bAlrsStatus = 0x02;
				iGrp = m_RestCtrl.GetGrp();
				m_PeriodCtrl.RstCtrl();
				m_PeriodCtrl.DoCmdScan();
			}
			else
			{//假如'厂休控'没有投入.
				m_PeriodCtrl.DoCtrl();	//执行'时段控'.
				if (m_PeriodCtrl.IsValid())
				{
//					DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::DoCtrl: cur ctrl is PERIOD!\n"));

					bTurnsStatus = m_PeriodCtrl.GetTurnsStatus();
					if (m_PeriodCtrl.IsAlr())
						bAlrsStatus = 0x01;
					iGrp = m_PeriodCtrl.GetGrp();
				}
				else
				{//功控都没有投入.
					bTurnsStatus = 0x00;	//将功控轮次置为全部允许合闸.
					bAlrsStatus = 0x00;		//将报警状态全部置为未报警.
					iGrp = -1;				//表示没有任何总加组投入.
				}
			}
		}
	}

	if (m_iGrp != iGrp)
	{//组发生变换时,系统库轮次状态需更新.
		if (m_iGrp != -1)
		{
			ChgSysTurnsStatus(m_iGrp, CTL_TURN_MASK, false);	//将当前总加组的所有轮次设为允许合闸状态.
			ChgSysPwrAlrFlgs(m_iGrp, 0x0f, false);				//将当前总加组的所有报警状态置为未报警.
		}
		m_iGrp = iGrp;
		m_bTurnsStatus = 0x00;	//切换到新的总加组后,轮次最初状态为全部允许合闸.
		m_bAlrsStatus = 0x00;	//切换到新的总加组后,报警状态全部设为非报警状态.
	}

	if (m_iGrp != -1)
	{//组不变但轮次状态发生变化时,也需要更新系统库功控轮次状态.
		if (m_bTurnsStatus != bTurnsStatus)
		{//轮次状态发生了变化.
			m_bTurnsStatus = bTurnsStatus;
			//SetSysTurnsStatus(m_iGrp, m_bTurnsStatus);
		}
		if (m_bAlrsStatus != bAlrsStatus)
		{//报警状态发生了变化.
			m_bAlrsStatus = bAlrsStatus;
			SetSysPwrAlrFlgs(m_iGrp, bAlrsStatus);
		}
	}

	/*m_TmpCtrl.MakeDisp(m_bTurnsStatus);
	m_RestCtrl.MakeDisp(m_bTurnsStatus);
	m_ShutoutCtrl.MakeDisp(m_bTurnsStatus);
	m_PeriodCtrl.MakeDisp(m_bTurnsStatus);*/

	return true;
}

//描述: 设定系统库指定总加组功控的所有报警状态.
//参数:@iGrp			要设定的总加组.
//	   @bFlgsStatus		要设置的报警状态.
//返回: 如果设置成功返回 true,否则返回 false.
bool CAllPwrCtrl::SetSysPwrAlrFlgs(int iGrp, BYTE bFlgsStatus)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	TGrpCurCtrlSta tGrpCurCtrlSta;
	memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

	if (!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//读"终端当前控制状态"
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::SetSysPwrAlrFlgs: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	tGrpCurCtrlSta.bPCAlarmState = bFlgsStatus & 0x0f;

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//写"终端当前控制状态".
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::SetSysPwrAlrFlgs: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}

	WORD wInID = 0;
	BYTE bBuf[10];
	memset(bBuf, 0, sizeof(bBuf));

	switch(bFlgsStatus)
	{
	case 1:	//时段控
		wInID = 0x8232;
		break;
	case 2:	//厂休控
		wInID = 0x8242;
		break;
	case 4:	//营业报停控
		wInID = 0x8252;
		break;
	case 8:	//下浮控
		wInID = 0x8262;
		break;
	}

	BYTE *pbtr = bBuf;
	*pbtr++ = DT_STRUCT;
	*pbtr++ = 2;					//结构成员个数
	*pbtr++ = DT_OI;				//总加组对象
	pbtr += OoWordToOi(0x2300+iGrp, pbtr);
	*pbtr++ = DT_ENUM;
	if (bFlgsStatus != 0)
		*pbtr++ = 1;
	else
		*pbtr++ = 0;

	if (wInID != 0)
	{
		bBuf[6] = 0;
		for (BYTE i=0; i<4; i++)
		{
			WriteItemEx(BN0, iGrp, 0x8232+(i<<4), bBuf); //先清除一下再写
		}
		bBuf[6] = 1;
		WriteItemEx(BN0, iGrp, wInID, bBuf);
	}
	else
	{
		for (BYTE i=0; i<4; i++)
		{
			WriteItemEx(BN0, iGrp, 0x8232+(i<<4), bBuf);
		}
	}

	return true;
}

//描述: 改变系统库指定总加组功控的相应报警状态.
//参数:@iGrp 		要改变的总加组.
//	   @bFlgs 		所要改变的标志位.
//	   @fStatus		true: 置位相应位; false: 清除相应位.
//返回: 如果设置成功返回 true,否则返回 false.
bool CAllPwrCtrl::ChgSysPwrAlrFlgs(int iGrp, BYTE bFlgs, bool fStatus)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	TGrpCurCtrlSta tGrpCurCtrlSta;
	memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

	if (!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//读"终端当前控制状态"
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::ChgSysPwrAlrFlgs: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	if (fStatus)
	{
		tGrpCurCtrlSta.bPCAlarmState |= (bFlgs & 0x0f);
	}
	else
	{
		tGrpCurCtrlSta.bPCAlarmState &= ~(bFlgs & 0x0f);
	}

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//写"终端当前控制状态"
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::ChgSysPwrAlrFlgs: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}

	WORD wInID = 0;
	BYTE bBuf[10];
	memset(bBuf, 0, sizeof(bBuf));
	BYTE *pbtr = bBuf;
	*pbtr++ = DT_STRUCT;
	*pbtr++ = 2;					//结构成员个数
	*pbtr++ = DT_OI;				//总加组对象
	pbtr += OoWordToOi(0x2300+iGrp, pbtr);
	*pbtr++ = DT_ENUM;
	*pbtr++ = fStatus;

	for (BYTE i=0; i<4; i++)
	{
		WriteItemEx(BN0, iGrp, 0x8232+(i<<4), bBuf);
	}

	return true;
}

//描述: 设定系统库指定总加组功控的所有轮次状态.
//参数:@iGrp			要设定的总加组.
//	   @bTurnsStatus	要设置的轮次状态.
//返回: 如果设置成功返回 true,否则返回 false.
bool CAllPwrCtrl::SetSysTurnsStatus(int iGrp, BYTE bTurnsStatus)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	TGrpCurCtrlSta tGrpCurCtrlSta;
	memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

	if(!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::SetSysTurnsStatus: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	tGrpCurCtrlSta.bAllPwrCtrlOutPutSta = bTurnsStatus & CTL_TURN_MASK;

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::SetSysTurnsStatus: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}

	return true;
}

//描述: 改变系统库指定总加组功控的相应轮次状态.
//参数:@iGrp 		要改变的总加组.
//	   @bTurns 		所要改变的轮次.
//	   @fStatus		true: 置位相应位; false: 清除相应位.
//返回: 如果设置成功返回 true,否则返回 false.
bool CAllPwrCtrl::ChgSysTurnsStatus(int iGrp, BYTE bTurns, bool fStatus)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;


	TGrpCurCtrlSta tGrpCurCtrlSta;
	memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

	if (!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//读"终端当前控制状态"
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::ChgSysTurnsStatus: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	if (fStatus)
	{
		tGrpCurCtrlSta.bAllPwrCtrlOutPutSta |= (bTurns & CTL_TURN_MASK);
	}
	else
	{
		tGrpCurCtrlSta.bAllPwrCtrlOutPutSta &= ~(bTurns & CTL_TURN_MASK);
	}

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//写"终端当前控制状态".
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::ChgSysTurnsStatus: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}

	WORD wInID = 0;
	BYTE bBuf[10];
	memset(bBuf, 0, sizeof(bBuf));
	BYTE *pbtr = bBuf;
	*pbtr++ = DT_STRUCT;
	*pbtr++ = 2;					//结构成员个数
	*pbtr++ = DT_OI;				//总加组对象
	pbtr += OoWordToOi(0x2300+iGrp, pbtr);
	*pbtr++ = DT_BIT_STR;
	*pbtr++ = 8;
	*pbtr++ = tGrpCurCtrlSta.bAllPwrCtrlOutPutSta;

	for (BYTE i=0; i<4; i++)
	{
		WriteItemEx(BN0, iGrp, 0x8231+(i<<4), bBuf);
	}

	return true;
}

//描述: 统计功控超限参数.
void CAllPwrCtrl::StatOverLimitPara(void)
{
	char cTime[20];
	BYTE bBuf[2+4];

	if (m_tmOldTime.nMonth!=m_tmNow.nMonth || m_tmOldTime.nYear!=m_tmNow.nYear)
	{//假如发生了月切换.
		//将本月功控超限参数复制到上月功控超限参数.
		for (int i=GRP_START_PN; i<(GRP_START_PN+GRP_NUM); i++)
		{
			if (ReadItemEx(BN18, (WORD)i, 0x02cf, bBuf) <=0)
			{
				DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::StatOverLimitPara: There is something wrong when call ReadItemEx() !\n"));
				return;
			}
			WriteItemEx(BN0, (WORD)i, 0x326f, bBuf, 0, NULL, m_dwNow);
			DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::StatOverLimitPara: Save the %04d-%02d Group[%d] statistical parameter of PwrCtrl power over limit!\n"\
								 "Over time = %ld minutes\n"\
								 "Over limit energy = %lld KWH\n"\
								 "at %s\n", m_tmOldTime.nYear, m_tmOldTime.nMonth, i,
								 ByteToWord(bBuf), Fmt3ToVal64(bBuf+2, 4), TimeToStr(m_tmNow, cTime)));
		}
		//初始化本月功控超限参数.
		memset(bBuf, 0, sizeof(bBuf));	//清空之前的记录.
		for (int i=GRP_START_PN; i<(GRP_START_PN+GRP_NUM); i++)
			WriteItemEx(BN18, (WORD)i, 0x02cf, bBuf);
	}

	DWORD dwTime = 0;
	int64 iEng = 0;
	int iGrp = 0;

	//因为在某个时刻,只能有一种功控投入,因此下面的四个统计中最多只会有一个统计的量不为0(即对 dwTime, iEng 进行改写).
	m_TmpCtrl.SumOverLimitPara(iGrp, dwTime, iEng);		//统计临时下浮控超限时间,电量.
	m_ShutoutCtrl.SumOverLimitPara(iGrp, dwTime, iEng);	//统计营业报停控超限时间,电量.
	m_RestCtrl.SumOverLimitPara(iGrp, dwTime, iEng);	//统计厂休控超限时间,电量.
	m_PeriodCtrl.SumOverLimitPara(iGrp, dwTime, iEng);	//统计时段控超限时间,电量.

	if (dwTime!=0 || iEng!=0)
	{
		if (ReadItemEx(BN18, (WORD)iGrp, 0x02cf, bBuf) <=0)
		{
			DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::StatOverLimitPara: There is something wrong when call ReadItemEx() !\n"));
			return;
		}

		WORD w;

		w = ByteToWord(bBuf) + (WORD)(dwTime/60);	//增加时间.
		WordToByte(w, bBuf);
		iEng += Fmt3ToVal64(bBuf+2, 4);			//增加超限电量.
		Val64ToFmt3(iEng, bBuf+2, 4);
		WriteItemEx(BN18, (WORD)iGrp, 0x02cf, bBuf);
#if 1
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::StatOverLimitPara: Refresh Group[%d] statistical parameter of PwrCtrl power over limit!\n"\
								 "Over time = %ld minutes\n"\
								 "Over limit energy = %lld KWH\n"\
								 "at %s\n", iGrp, w, iEng, TimeToStr(m_tmNow, cTime)));
#endif
	}
}

//描述: 获取当前优先的功控总加组供显示用
int CAllPwrCtrl::GetCurProGrp(void)
{
	if (m_iGrp != -1)
		return m_iGrp;
	if (m_TmpCtrl.IsValid())
		return m_TmpCtrl.GetGrp();
	if (m_ShutoutCtrl.IsValid())
		return m_ShutoutCtrl.GetGrp();
	if (m_RestCtrl.IsValid())
		return m_RestCtrl.GetGrp();
	if (m_PeriodCtrl.IsValid())
		return m_PeriodCtrl.GetGrp();
	return -1;
}

//
//========================================== CLoadCtrl =============================================
//
CLoadCtrl::CLoadCtrl(void)
{
}

CLoadCtrl::~CLoadCtrl()
{
}

//描述: 负控初始化
bool CLoadCtrl::Init(void)
{
	char cTime[20];
	BYTE bBuf[400];
	TTime tmNow;

	GetCurTime(&tmNow);
	m_dwStarupTime = TimeToSeconds(tmNow);

	m_tmOldTime = tmNow;
	m_dwOldTime = m_dwStarupTime;

	m_fEnableBreakAct		= false;
	m_bTurnsStatus			= 0x00;
	m_fBeepAlrStatus		= false;
	m_fTrigerAlr			= false;

	DTRACE(DB_LOADCTRL, ("CLoadCtrl::Init: Starup Time %s\n", TimeToStr(tmNow, cTime)));

	//清1类数据中的控制状态
	/*ReadItemEx(BN0, PN0, 0x104f, bBuf);
	bBuf[0] &= 0x02;
	memset(&bBuf[1], 0, sizeof(bBuf)-1);
	WriteItemEx(BN0, PN0, 0x104f, bBuf);			//写"终端控制设置状态"ID
	memset(bBuf, 0, sizeof(bBuf));
	WriteItemEx(BN0, PN0, 0x105f, bBuf);	//写"终端当前控制状态".
	WriteItemEx(BN1, PN0, 0x3010, bBuf);	//写"终端当前控制状态".
	WriteItemEx(BN0, PN0, 0x5513, bBuf);*/
	
	InitSysCtrl();
	m_Guarantee.Init();
	m_UrgeFee.Init();

	int i;

	for (i=0; i<TURN_NUM; i++)
	{
		if (m_YkCtrl[i].SetTurn(i+TURN_START_PN))	//必须在初始化前设置好轮次.
			m_YkCtrl[i].Init();
		else
		{
			DTRACE(DB_LOADCTRL, ("CLoadCtrl::Init: There is sth wrong when set current YkCtrl to Turn[%d].!\n", i));
			return false;
		}
	}
	m_MonthCtrl.Init();
	m_BuyCtrl.Init();
	m_AllPwrCtrl.Init();

	memset(m_bYkClosedTurns, 0, sizeof(m_bYkClosedTurns)); //各控制被遥控合闸合掉的已经跳闸的轮次

	/*BYTE bBuf2[60];
	BYTE bTmp[] = {0xee,0xee,0xee,0,0,0,0,0};
	BYTE bTmp2[] = {0,0,0,0,0,0};

	memset(bBuf, 0 ,sizeof(bBuf));
	memset(bBuf2, 0 ,sizeof(bBuf2));
	for (int iGrpNum=1; iGrpNum <GB_MAXSUMGROUP; iGrpNum++)
	{
		if(IsGrpValid(iGrpNum))
		{
			bBuf[2] |= (1<< (iGrpNum-1));
			memcpy(bBuf+3+ (iGrpNum-1)*8, bTmp, 8);
			bBuf2[1] |= (1<< (iGrpNum-1));
		}
	}
	WriteItemEx(BN0, PN0, 0x105f, bBuf);	//写"终端当前控制状态".
	WriteItemEx(BN0, PN0, 0x104f, bBuf2);	//写"终端当前控制设置状态".*/
	return true;
}

void CLoadCtrl::InitSysCtrl()
{
	BYTE bBuf[100];
	bool fIndex = false;
	memset(bBuf,0,sizeof(bBuf));
	if (ReadItemEx(BN1, PN0, 0x3010, bBuf)>0 && bBuf[0]>0)
	{
		BYTE bSize = bBuf[0];
		int i=0;
		while(i < bSize)
		{
			if (bBuf[1+i*2] >= CTL_YkCtrl_CLOSE)
			{
				bSize--;
				BYTE* pbBuf = &bBuf[3+i*2];
				memcpy(bBuf+1+i*2, pbBuf, sizeof(bBuf)-3-2*i);
			}
			else
			{
				i++;
			}
		}
		bBuf[0] = bSize;
		WriteItemEx(BN1, PN0, 0x3010, bBuf);
	}
	memset(bBuf, 0, sizeof(bBuf));
	WriteItemEx(BN0, PN0, 0x0910, bBuf);
	WriteItemEx(BN0, PN0, 0x0920, bBuf);
	WriteItemEx(BN0, PN0, 0x0930, bBuf);
}

bool CLoadCtrl::GetSoundBeepStats()
{
	DWORD dwAlarmFlg;
	TTime tmNow;
	GetCurTime(&tmNow);
	ReadItemEx(BN0, PN0, 0x039f, (BYTE*)&dwAlarmFlg);
	if (dwAlarmFlg & (1<<tmNow.nHour))
	{
		return true;
	}
	else
		return false;
}

//描述: 负控执行
bool CLoadCtrl::DoCtrl(void)
{
    bool fChange;
	GetCurTime(&m_tmNow);				//获取当前时间.
	m_dwNow = TimeToSeconds(m_tmNow);	//转换为 DWORD 形式.

	if (!m_fEnableBreakAct)
	{
		m_Guarantee.SetSysCtrlStatus(AUTO_GUARANTEE);
	}
	else
	{
		m_Guarantee.DoCtrl();
	}
	
	m_UrgeFee.DoCtrl();

	int i;
	BYTE bYkCtrlTurnsStatus = 0;	//首先初始化为所有轮次都允许合闸的状态.
	BYTE bYkClosedTurns = 0;		//本轮执行被遥控合闸的轮次

	for (i=0; i<TURN_NUM; i++)
	{
		m_YkCtrl[i].DoCtrl();
		//m_YkCtrl[i].MakeDisp();

		if (m_YkCtrl[i].GetTurnStatus())
			bYkCtrlTurnsStatus |= 0x01<<i;

		if (m_YkCtrl[i].IsRxCloseCmd()) //本轮执行是否收到了一个遥控合闸命令
			bYkClosedTurns |= 0x01<<i;
	} 
	if (IsGuarantee())
	{//保电命令投入，点亮保电灯
		SetCtrlLed(true, LED_GUARANTEE);
	}
	else
	{
		SetCtrlLed(false, LED_GUARANTEE);
	}


	m_MonthCtrl.DoCtrl();
	//m_MonthCtrl.MakeDisp(m_bTurnsStatus);

	m_BuyCtrl.DoCtrl();
	//m_BuyCtrl.MakeDisp(m_bTurnsStatus);

	m_AllPwrCtrl.DoCtrl();
	
#ifndef SYS_WIN
	if(m_BuyCtrl.IsValid()||m_MonthCtrl.IsValid())
	{//点亮电控灯
		SetCtrlLed(true, LED_ENERGYCTRL);
	}
	else
	{
		SetCtrlLed(false, LED_ENERGYCTRL);
	}
#endif
	char cTime[20];

	/*
	**目前面向对象协议暂未要求和定义控制功能的数据统计，下面的统计代码先屏蔽掉，后面协议要求了再打开并修改 -QLS 17.01.10
	*/
/*
	//统计超限参数
	m_MonthCtrl.StatOverLimitPara();
	m_AllPwrCtrl.StatOverLimitPara();

	char cTime[20];
	BYTE bBuf[128];

	//统计跳闸参数
	if (m_tmOldTime.nDay!=m_tmNow.nDay || m_tmOldTime.nMonth!=m_tmNow.nMonth || m_tmOldTime.nYear!=m_tmNow.nYear)
	{//发生了日切换或月切换
		if (m_tmOldTime.nMonth!=m_tmNow.nMonth || m_tmOldTime.nYear!=m_tmNow.nYear)
		{//发生了月切换
			//将本月控制统计数据复制到上月控制统计数据.
			if (ReadItemEx(BN11, PN0, 0x025f, bBuf) <=0)
			{
				DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: There is something wrong when call ReadItemEx() !\n"));
				return false;
			}
			WriteItemEx(BN0, PN0, 0x31ef, bBuf, 0, NULL, m_dwNow);
			WriteItemEx(BN0, PN0, 0x713c, bBuf, 0, NULL, m_dwNow);
			DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Save the %04d-%02d month's statistic of open break times!\n"\
								 "MonthCtrl open break times = %d\n"\
								 "BuyCtrl open break times = %d\n"\
								 "PowerCtrl open break times = %d\n"\
								 "YK open break times = %d\n"\
								 "at %s\n", m_tmOldTime.nYear, m_tmOldTime.nMonth,
								 bBuf[0], bBuf[1], bBuf[2], bBuf[3], TimeToStr(m_tmNow, cTime)));
			//初始化本月控制统计数据.
			memset(bBuf, 0, sizeof(bBuf));	//清空之前的记录.
			WriteItemEx(BN11, PN0, 0x025f, bBuf);
		}
		//将本日控制统计数据复制到上日控制统计数据.
		if (ReadItemEx(BN11, PN0, 0x023f, bBuf) <=0)
		{
			DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: There is something wrong when call ReadItemEx() !\n"));
			return false;
		}
		WriteItemEx(BN0, PN0, 0x31bf, bBuf, 0, NULL, m_dwNow);
		WriteItemEx(BN0, PN0, 0x712c, bBuf, 0, NULL, m_dwNow);
		
		DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Save the %04d-%02d-%02d day's statistic of open break times!\n"\
							 "MonthCtrl open break times = %d\n"\
							 "BuyCtrl open break times = %d\n"\
							 "PowerCtrl open break times = %d\n"\
							 "YK open break times = %d\n"\
							 "at %s\n", m_tmOldTime.nYear, m_tmOldTime.nMonth, m_tmOldTime.nDay,
							 bBuf[0], bBuf[1], bBuf[2], bBuf[3], TimeToStr(m_tmNow, cTime)));
		//初始化本日控制统计数据.
		memset(bBuf, 0, sizeof(bBuf));	//清空之前的记录.
		WriteItemEx(BN11, PN0, 0x023f, bBuf);
		//TrigerSave();
	}

	BYTE bYkCtrlOpenTimes = 0;

	for (i=0; i<TURN_NUM; i++)
		bYkCtrlOpenTimes += (BYTE)m_YkCtrl[i].GetOpenTimes();
	BYTE bMonthCtrlOpenTimes = (BYTE)m_MonthCtrl.GetOpenTimes();
	BYTE bBuyCtrlOpenTimes = (BYTE)m_BuyCtrl.GetOpenTimes();
	BYTE bAllPwrCtrlTimes = (BYTE)m_AllPwrCtrl.GetOpenTimes();

	if (bYkCtrlOpenTimes+bMonthCtrlOpenTimes+bBuyCtrlOpenTimes+bAllPwrCtrlTimes != 0)
	{//说明有跳闸发生.
		//更新日统计数据.
		if (ReadItemEx(BN11, PN0, 0x023f, bBuf) <=0)
		{
			DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: There is something wrong when call ReadItemEx() !\n"));
			return false;
		}
		bBuf[0]	+= bMonthCtrlOpenTimes;	//更新月电控跳闸次数.
		bBuf[1]	+= bBuyCtrlOpenTimes;	//更新购电控跳闸次数.
		bBuf[2]	+= bAllPwrCtrlTimes;	//更新功控跳闸次数.
		bBuf[3]	+= bYkCtrlOpenTimes;	//更新遥控跳闸次数.
		WriteItemEx(BN11, PN0, 0x023f, bBuf);
#if 1
		DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Refresh day's statistic of open break times!\n"\
							 "MonthCtrl open break times = %d\n"\
							 "BuyCtrl open break times = %d\n"\
							 "PowerCtrl open break times = %d\n"\
							 "YK open break times = %d\n"\
							 "at %s\n", bBuf[0], bBuf[1], bBuf[2], bBuf[3], TimeToStr(m_tmNow, cTime)));
#endif
		//更新月统计数据.
		if (ReadItemEx(BN11, PN0, 0x025f, bBuf) <=0)
		{
			DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: There is something wrong when call ReadItemEx() !\n"));
			return false;
		}
		bBuf[0]	+= bMonthCtrlOpenTimes;	//更新月电控跳闸次数.
		bBuf[1]	+= bBuyCtrlOpenTimes;	//更新购电控跳闸次数.
		bBuf[2]	+= bAllPwrCtrlTimes;	//更新功控跳闸次数.
		bBuf[3]	+= bYkCtrlOpenTimes;	//更新遥控跳闸次数.
		WriteItemEx(BN11, PN0, 0x025f, bBuf);
#if 1
		DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Refresh month's statistic of open break times!\n"\
							 "MonthCtrl open break times = %d\n"\
							 "BuyCtrl open break times = %d\n"\
							 "PowerCtrl open break times = %d\n"\
							 "YK open break times = %d\n"\
							 "at %s\n", bBuf[0], bBuf[1], bBuf[2], bBuf[3], TimeToStr(m_tmNow, cTime)));
#endif
		//TrigerSave();
	}*/


	//计算轮次控制状态.
	//NOTE:对于控制状态放到每个控制中计算还是放到这里进行总的计算的问题:
	// 	   控制状态的实际输出和系统库F6中跳闸输出状态都放到这里来实现,因为
	// 	   1.功控的多种控制的总的状态不能单独计算,必须进行综合处理
	// 	   2.各控制被遥控合闸合掉的已经跳闸的轮次,也要进行综合处理,比如功控的多种控制就不能
	// 		 分开处理,否则再优先级的切换的导致控制的切换的时候容易引起问题
	BYTE bTurnsStatus;
	BYTE bOpenTurns[3];
	if (m_Guarantee.IsValid())
	{
		bTurnsStatus = 0x00;	//保电投入,所有闸都允许合闸.
		memset(m_bYkClosedTurns, 0, sizeof(m_bYkClosedTurns)); //各控制被遥控合闸合掉的已经跳闸的轮次
		memset(bOpenTurns, 0, sizeof(bOpenTurns));
	}
	else
	{
		//bTurnsStatus = bYkCtrlTurnsStatus | m_MonthCtrl.GetTurnsStatus() | m_BuyCtrl.GetTurnsStatus() | m_AllPwrCtrl.GetTurnsStatus();
		bOpenTurns[0] = m_AllPwrCtrl.GetTurnsStatus();
		bOpenTurns[1] = m_MonthCtrl.GetTurnsStatus();
		bOpenTurns[2] = m_BuyCtrl.GetTurnsStatus();

		bTurnsStatus = bYkCtrlTurnsStatus;
		for (i=0; i<3; i++)
		{
			//求每个控制被遥控合闸合掉的已经跳闸的轮次
			m_bYkClosedTurns[i] = (m_bYkClosedTurns[i] | bYkClosedTurns) & bOpenTurns[i] & (~bYkCtrlTurnsStatus);
									//& bOpenTurns[i]:只有已经跳闸的轮次才记下被遥控合闸合掉的轮次,保证还没跳闸的轮次能继续跳
									//				  如果某个轮次后面被控制本身允许合闸了,则清除该标志
									//~bYkCtrlTurnsStatus 遥控允许合闸的标志位,即去掉现在被遥控跳闸的轮次,

			//求每个控制的实际输出
			bOpenTurns[i] &= ~m_bYkClosedTurns[i];	//清掉被合掉的闸

			//求总的实际输出
			bTurnsStatus |= bOpenTurns[i];
		}
	}
	
	/*
	**面向对象协议里未定义终端总的控制状态，这里先屏蔽掉，需要时再开放并修改 --QLS 17.01.10
	*/
	//更新系统库F6中跳闸输出状态
	//NOTE:1.那些已经失效的总加组的状态由控制本身来清除,这里只管当前有效的总加组
	// 	   2.遥控的状态不受遥控合闸影响,自己入库,不在这里入库,
	int iGrp;
	iGrp = m_AllPwrCtrl.GetGrp();
	if (iGrp != -1)
		m_AllPwrCtrl.SetSysTurnsStatus(iGrp, bOpenTurns[0]);

	iGrp = m_MonthCtrl.GetGrp();
	if (iGrp != -1)
		m_MonthCtrl.SetSysTurnsStatus(iGrp, bOpenTurns[1]);

	iGrp = m_BuyCtrl.GetGrp();
	if (iGrp != -1)
		m_BuyCtrl.SetSysTurnsStatus(iGrp, bOpenTurns[2]);


	//计算蜂鸣器报警状态.
	bool fBeepAlrStatus, fIsOpen = false;
	fBeepAlrStatus = m_UrgeFee.IsBeepAlr();
	for (i=0; i<TURN_NUM; i++)
	{
		fBeepAlrStatus = fBeepAlrStatus || m_YkCtrl[i].IsBeepAlr();
	}
	fBeepAlrStatus = fBeepAlrStatus || m_MonthCtrl.IsBeepAlr() || m_BuyCtrl.IsBeepAlr() || m_AllPwrCtrl.IsBeepAlr() || m_fTrigerAlr;

	if (m_fBeepAlrStatus != fBeepAlrStatus)
	{
		m_fBeepAlrStatus = fBeepAlrStatus;
		DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Beep alarm status changed at %s, now status is '%1d'.\n", TimeToStr(m_tmNow, cTime), (int)m_fBeepAlrStatus));
	}
	//输出报警状态.
	if (m_fBeepAlrStatus)
	{
		//if (GetSoundBeepStats())
		{
			SetAlrLedCtrlMode(true);
			DoYk(true, LED_ALERT);
		}
		//else
		//{
		//	DoYk(false, LED_ALERT);
		//	SetAlrLedCtrlMode(false);
		//}
	}
	else
	{
		m_dwWarnTime = 0;

		SetAlrLedCtrlMode(false);
		DoYk(false, LED_ALERT);
	}


	if (!m_fEnableBreakAct)
	{
		//if ("人工解除")
		//{
		//	DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoLoadCtrl: Output break lock have release by Operator!\n\
		//						  Now is %s\n",
		//						  TimeToStr(m_tmNow, cTime)));
		//	m_fEnableBreakAct = true;
		//}
		m_Guarantee.SetSysCtrlStatus(AUTO_GUARANTEE);
		BYTE bBuf[3];
		WORD wPwronSafeTime = 0;
		ReadItemEx(BN0, PN0, 0x8211, bBuf);
		wPwronSafeTime = OoLongUnsignedToWord(&bBuf[1]);
		if (m_dwNow < m_dwStarupTime)
			m_dwStarupTime = m_dwNow - GetClick(); //往前对时，修正起始时间
		if (m_dwNow >= m_dwStarupTime+wPwronSafeTime*60)
		{
			DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoLoadCtrl: The time of lock break in power on is over!\n"\
								 "at %s\n", TimeToStr(m_tmNow, cTime)));
			m_fEnableBreakAct = true;
			m_Guarantee.SetSysCtrlStatus(QUIT_GUARANTEE);
			m_Guarantee.DoCtrl();
		}
		if (!m_fEnableBreakAct)
			goto LC_EndofDoCtrl;
	}
	
	fChange = false;

	if (m_bTurnsStatus != bTurnsStatus)
	{
		m_bTurnsStatus = bTurnsStatus;
		DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Turns status changed at %s, now status is '%2x'.\n", TimeToStr(m_tmNow, cTime), m_bTurnsStatus));
		fChange = true;
	}
	if (!fChange && m_tmNow.nMinute==m_tmOldTime.nMinute)
		goto LC_EndofDoCtrl;

	//输出闸状态.
	for (i=0; i<TURN_NUM; i++)
	{

		if ((bTurnsStatus&0x01) != 0)
		{
		    //DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Turn[%d] is Open\n", i+1));
			DoYk(true, i);
			fIsOpen = true;
		}
		else
		{
		    //DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Turn[%d] is Close\n", i+1));
			//if ( fIsOpen )
			//	Sleep (2000);
			DoYk(false, i);
			fIsOpen = false;
		}

		bTurnsStatus >>= 1;
	}


LC_EndofDoCtrl:
	m_tmOldTime = m_tmNow;		//更新上次执行功控的时间.
	m_dwOldTime = m_dwNow;		//更新上次执行功控的时间.

	return true;
}
bool CLoadCtrl::IsEnergyFee(void)								//是否购电费控制
{
	return m_BuyCtrl.IsEnergyFee();
}
     
