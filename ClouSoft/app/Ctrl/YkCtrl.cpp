/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：YkCtrl.cpp
 * 摘    要：本文件主要实现CYkCtrl的类
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年3月
*********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "sysfs.h"
#include "FaCfg.h"
#include "DbConst.h"
#include "FaConst.h"
#include "ComAPI.h"
#include "TaskDB.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "TaskManager.h"
#include "YkCtrl.h"
#include "FaAPI.h"
#include "TermEvtTask.h"

//========================================= CYkCtrl =============================================
CYkCtrl::CYkCtrl(BYTE bTurn)
: m_iTurn(bTurn), m_wOpenTimes(0), m_dwFrzDly(60 * 2)	//遥控是延时2分钟.
{
	if (m_iTurn<TURN_START_PN && m_iTurn>TURN_START_PN+TURN_NUM)
		m_iTurn = TURN_START_PN;

	m_fTurnStatus = false;					
	m_fAlrStatus = false;
	m_fRxCloseCmd = false;
	m_fAlarmStatus = false;
	m_fCloseStatus = true;
	m_fOpenStatus = false;
	m_dwOpenClick = 0;
	m_dwCloseClick = 0;
	m_dwOpenClk = 0;
}

//描述: 设置'遥控'的当前轮次.
//参数:@iTurn	设定的轮次.
//返回: 如果设置成功返回 true,否则返回 false.
bool CYkCtrl::SetTurn(int iTurn)
{
	if (iTurn<TURN_START_PN || iTurn>TURN_START_PN+TURN_NUM)
		return false;

	m_iTurn = iTurn;

	return true;
}

//描述: 扫描系统库中的命令.
void CYkCtrl::DoCmdScan(void)
{
	TYkCtrlCmd NewCmd;
	BYTE bCmd[10];

	m_fRxCloseCmd = false;					//本轮执行是否收到了一个遥控合闸命令
	if (ReadItemEx(BN0, (WORD)m_iTurn, 0x8203, bCmd, &NewCmd.dwTime) <= 0)	//从相应ID的测量点读"终端遥控投入命令"
	{
		DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCmdScan: There is something wrong when call ReadItemEx() !\n"));
		return;
	}

	if (bCmd[0] != 1 && bCmd[0] != 2)
		return ;

	NewCmd.bAct = bCmd[0];
//	if (NewCmd.bAct!=1 && NewCmd.bAct!=2) //表示未收到遥控命令；
//		return;
	NewCmd.bAlrTime = bCmd[2];
	NewCmd.dwPersistTime = OoLongUnsignedToWord(bCmd+4)*60;
	
	char cTime[20];
	//现在的时间比下发命令的时间要早（测试时修改终端时间），则控制命令要撤销；
	if (NewCmd.dwTime == 0) //终端时间往前调了,系统库中的命令时间被清掉了
	{	
		TTime tmCmd;
		SecondsToTime(m_CtrlCmd.dwTime, &tmCmd);
		if (m_tmNow.nDay!=tmCmd.nDay || m_tmNow.nMonth!=tmCmd.nMonth || m_tmNow.nYear!=tmCmd.nYear)
		{
			RstSysCtrlStatus(m_iTurn);	//必须在清除系统库命令前,设置系统库'遥控'状态.
			ClrSysCmd(m_iTurn);
			ClrCmd();
			RstCtrl(); 
			SetValidStatus(false);	//设定控制退出状态.
			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCmdScan: turn=%d quit due to cmd time==0 and dan change\n", m_iTurn));
			return;
		}

		if (NewCmd.dwPersistTime == 0)	//长时间限电
		{
			NewCmd.dwTime = GetCurTime(); //更新为一个合理一点的时间,方便以后不同命令时间的比较
			WriteItemEx(BN0, (WORD)m_iTurn, 0x8203, bCmd, NewCmd.dwTime);	//把相应总加组的"时段控命令"写会数据库
		}
		else if (m_CtrlCmd.dwTime!=0 //指定限电时间,且旧的命令已经被扫描进来执行
				 && m_dwOldTime>=m_CtrlCmd.dwTime && m_dwOldTime<m_CtrlCmd.dwTime+m_CtrlCmd.dwPersistTime)
		{		//把命令的时间当成click的方式来用
			NewCmd.dwTime = GetCurTime() - (m_dwOldTime-m_CtrlCmd.dwTime); //去掉已经消耗掉的时间
			WriteItemEx(BN0, PN0, 0x8203, bCmd, NewCmd.dwTime);	//把相应总加组的"时段控命令"写会数据库
		}
		else
		{
			RstSysCtrlStatus(m_iTurn);	//必须在清除系统库命令前,设置系统库'遥控'状态.
			ClrSysCmd(m_iTurn);
			ClrCmd();
			RstCtrl();
			SetValidStatus(false);	//设定控制退出状态.
			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCmdScan: turn=%d quit due to cmd time==0 and ctrl timeout\n", m_iTurn));
			return;
		}
	}

	//如接收到的不是1或2,将不会对当前的控制状态产生影响,但在接收到新的有效命令前终端
	//一旦掉电,上电后,当前轮次将会进入遥控解除状态(因为象被初始化为遥控解除状态).
	if (NewCmd.dwTime != m_CtrlCmd.dwTime)
	{//时标不同,表示接收到新的命令,更新命令.
		m_CtrlCmd = NewCmd;
		DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCmdScan: rx new cmd at %s, turn=%d, act=%d, alr-time=%d, persist-time=%ld\n",
							 TimeToStr(NewCmd.dwTime, cTime), m_iTurn, NewCmd.bAct, NewCmd.bAlrTime, NewCmd.dwPersistTime));
		if (NewCmd.bAct == 1)		//遥控投入命令.
		{
			SetValidStatus(true);
			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCmdScan: turn=%d launch at %s\n", 
								 m_iTurn, TimeToStr(m_tmNow, cTime)));
			//***记录到系统库日志中.
			//***发出声光信号;
		}
		else //(NewCmd.bAct == 2)	//遥控解除命令.
		{
			m_fRxCloseCmd = true;		//本轮执行是否收到了一个遥控合闸命令
			RstSysCtrlStatus(m_iTurn);	//必须在清除系统库命令前,设置系统库'遥控'状态.
			ClrSysCmd(m_iTurn);
			ClrCmd();
			RstCtrl();
			SetValidStatus(false);	//设定控制退出状态.
			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCmdScan: Turn[%d] YkCtrl quit at %s\n", 
								 m_iTurn, TimeToStr(m_tmNow, cTime)));
			//***记录到系统库日志中.
			//***发出声光信号;
		}
	}
}

//描述: 保存遥控跳闸记录.
void CYkCtrl::DoSaveOpenRec(void)
{
	if (m_dwOpenClk != 0)
	{//如有跳闸情况发生,必须在跳闸后2分钟记录冻结功率.
		if (GetClick() >= m_dwOpenClk+120)
		{
			BYTE *pbPtr = g_YKCtrl.bArrayPow;
			memset(g_YKCtrl.bArrayPow, 0, sizeof(g_YKCtrl.bArrayPow));
			*pbPtr++ = DT_ARRAY;
			*pbPtr++ = 1;
			pbPtr += ReadItemEx(BN0, PN1, 0x2302, pbPtr);
			m_dwOpenClk = 0;
		}
	}
}

//描述: 运行控制.
//返回: 正常则返回 true,否则返回 false.
bool CYkCtrl::DoCtrl(void)
{
	DoSaveOpenRec();	//保存遥控跳闸记录.
	DoCmdScan();		//扫描系统库中的命令.

	if (!IsValid())
	{
		RstCtrl();
		return true;
	}

	char cTime[20];
	TTime tmCmd;
/*
	if (IsGuarantee())	//处在保电状态,遥控复位,但不清遥控命令
	{
		if (!m_fGuarantee)
		{
			m_fGuarantee = true;
			RstSysCtrlStatus(m_iTurn);	//设置系统库'遥控'状态.
			ClrCmd();
			RstCtrl();
			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: YK%d quit for Guarantee\n", m_iTurn));
		}

		return true;
	}

	m_fGuarantee = false;
*/
	SecondsToTime(m_CtrlCmd.dwTime, &tmCmd);
	if (m_tmNow.nDay!=tmCmd.nDay || m_tmNow.nMonth!=tmCmd.nMonth || m_tmNow.nYear!=tmCmd.nYear)
	{	//发生了日切换,遥控复位.
		RstSysCtrlStatus(m_iTurn);	//必须在清除系统库命令前,设置系统库'遥控'状态.
		ClrSysCmd(m_iTurn);
		ClrCmd();
		RstCtrl();
		SetValidStatus(false);	//设定控制退出状态.
		DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: turn=%d quit for day change\n", m_iTurn));
		//***记录到系统库日志中.
		//***发出声光信号;
		return true;
	}

	DWORD dwCntDown;
	DWORD dwTime = m_CtrlCmd.dwTime + (DWORD)m_CtrlCmd.bAlrTime*60; //跳闸起始时间

	if (m_dwNow < dwTime)
	{//处在告警延时时段内.
		if (!m_fAlrStatus)
		{
			SetSysTurnStatus(m_iTurn, false);	//在告警时段内,应允许合闸.
			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: YK%d alarm start at %s\n",
								 m_iTurn, TimeToStr(m_tmNow, cTime)));
			//***发出声光信号;
			SetSysTurnAlrStatus(m_iTurn, true);
		}
		RstCtrl();
		m_fAlrStatus = true;	//恢复'遥控'报警状态.

		dwCntDown = dwTime - m_dwNow;
		//SaveDisp(dwCntDown);
	}
	else if (m_dwNow <= (m_CtrlCmd.dwTime+m_CtrlCmd.dwPersistTime) || m_CtrlCmd.dwPersistTime==0)
	{//处在限电时段内或限电时间为0.
		if (IsGuarantee())	//处在保电状态,遥控复位,但不清遥控命令
		{
			if (!m_fGuarantee)
			{
				m_fGuarantee = true;
				RstSysCtrlStatus(m_iTurn);	//设置系统库'遥控'状态.
				ClrCmd();	
				RstCtrl();
				DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: YK%d quit for Guarantee\n", m_iTurn));
			}
			return true;
		}
		m_fGuarantee = false;

		dwCntDown = 0;
		//SaveDisp(dwCntDown);

		m_fAlrStatus = false;	//因为要跳闸,所以报警没有意义,关闭报警.
		if (!m_fTurnStatus)		//刚告警完还没跳闸,则现在就要跳
		{
			SetSysTurnStatus(m_iTurn, true);	//遥控跳闸.
			SetSysTurnAlrStatus(m_iTurn, false);
			m_wOpenTimes++;						//跳闸次数增加1.

			g_YKCtrl.bEvtSrcOAD[0] = DT_OAD;
			DWORD dwOad = 0xF2050200 + m_iTurn;
			OoDWordToOad(dwOad, &g_YKCtrl.bEvtSrcOAD[1]);
			SetInfo(INFO_YK_REC);
			m_dwOpenClk = GetClick();
			//BYTE bBuf[1+1+6*8];

			/*if (ReadItemEx(BN0, PN0, 0x104f, bBuf) <= 0)//读"终端控制设置状态".
			{
				DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: There is something wrong when call ReadItemEx() !\n"));
				return false;
			}*/

			//BYTE bRecBuf[12];

			//memcpy(bRecBuf, &m_dwNow, 4);	//保存跳闸时间.

			/*if (IsGrpValid(1))   //遥控跳闸中间数据
			{
				bRecBuf[4] = 1;		//总加组号
				DWORD wPwr = GetCurPwr(1);
				Val32ToBin(wPwr, bRecBuf+5, 4);//跳闸时功率（总加功率）
			}
			else
			{
				memset(bRecBuf+5, INVALID_DATA, 4); //总加组号+跳闸时功率（总加功率）
			}
			WriteItemEx(BN0, (WORD)m_iTurn, 0x0a00, bRecBuf);*/

			//TrigerSaveBank(BN0, SECT_CTRL, 0);

			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: turn=%d open at %s\n", 
								 m_iTurn, TimeToStr(m_tmNow, cTime)));
			//***发出声光信号;轮次 bTurn 遥控已跳闸.
		}
		m_fTurnStatus = true;
	}
	else
	{//限电时段已结束.
		RstSysCtrlStatus(m_iTurn);	//必须在清除系统库命令前,设置系统库'遥控'状态.
		ClrSysCmd(m_iTurn);
		ClrCmd();
		RstCtrl();
		SetValidStatus(false);	//设定控制退出状态.
		DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: YK%d finish at %s\n", 
							 m_iTurn, TimeToStr(m_tmNow, cTime)));
		//***声光信号.
	}

	return true;
}

//描述:生成遥控跳闸倒计时时间；
//参数：@dwCntDown - 遥控跳闸倒计时秒数；
void CYkCtrl::SaveDisp(DWORD dwCntDown)
{
	BYTE bBuf[20];
	WORD wAlrtime;
	memset(bBuf, 0, sizeof(bBuf));
	//写入起始和延时时间
	memcpy(bBuf,&m_CtrlCmd.dwTime,sizeof(m_CtrlCmd.dwTime));	
	wAlrtime = m_CtrlCmd.bAlrTime*60;
	
	memcpy(bBuf+4,&wAlrtime,sizeof(wAlrtime));
	WriteItemEx(BN0, m_iTurn, 0x0910, bBuf); //遥控告警信息界面
}

//描述：从系统库中删除无效的遥控状态（供显示用）；
//参数：@tInvCtrl  无效的遥控
void CYkCtrl::RemoveDispItem(TCtrl tInvCtrl)
{
	BYTE bBuf[21];
	memset(bBuf, 0, sizeof(bBuf));

	if (ReadItemEx(BN1, PN0, 0x3010, bBuf)>0 && bBuf[0]>0)
	{
		BYTE bSize = bBuf[0]; //告警的个数；
		int iIndex = -1; //失效的显示告警类型在队列中的位置；

		//查找失效的显示告警类型在数组中的索引
		for (BYTE i=0; i<bSize; i++)
		{
			if (bBuf[i*2+1]==tInvCtrl.bCtrlType && bBuf[i*2+2]==tInvCtrl.bCtrlTurn)
			{
				iIndex = i;
				break;
			}
		}

		//重新排序
		if (iIndex >= 0)
		{
			bSize --;
			bBuf[0] = bSize;
			BYTE* pbBuf = &bBuf[iIndex*2+3];
			memcpy(bBuf+iIndex*2+1, pbBuf, (bSize-iIndex)*2);
			memset(bBuf+bSize*2+1, 0, 20-bSize*2); 
			WriteItemEx(BN1, PN0, 0x3010, bBuf);
		}
	}

}

//描述：向系统库中增加新的遥控状态（供显示用）；
//参数：@tTopCtrl 新的遥控（放在数组最前面）
void CYkCtrl::AddDispItem(TCtrl tTopCtrl)
{
	BYTE bBuf[21], bTmpBuf[21];
	memset(bBuf, 0, sizeof(bBuf));

	if (ReadItemEx(BN1, PN0, 0x3010, bTmpBuf) > 0)
	{
		BYTE bSize = bTmpBuf[0]; //以前的个数；

		//如果库中已保存该类型的显示信息，则不再添加；
		for (BYTE i=0; i<bSize; i++)
		{
			if (bTmpBuf[1+2*i]==tTopCtrl.bCtrlType && bTmpBuf[2+2*i]==tTopCtrl.bCtrlTurn)
				return;
		}

		BYTE *pbBuf = &bTmpBuf[1];
		bBuf[1] = tTopCtrl.bCtrlType;
		bBuf[2] = tTopCtrl.bCtrlTurn;
		if (bSize == 10)
		{
			memcpy(bBuf+3, pbBuf, 18);
		}
		else
		{
			memcpy(bBuf+3, pbBuf, bSize*2);
			memset(bBuf+bSize*2+3, 0, 18-bSize*2);
			bSize++;
		}
		bBuf[0] = bSize; 
		WriteItemEx(BN1, PN0, 0x3010, bBuf);
	}
}

//描述：向系统库中生成新的遥控（供显示用）；
//遥控显示分为三个部分：1.遥控告警倒计时画面； 2.遥控跳闸(30s)；3.遥控合闸(30s).
void CYkCtrl::MakeDisp()
{
	TCtrl tCtrl;
	TCtrl tInvCtrl;

	bool fCloseStatus = IsCloseStatus();
	bool fAlarmStatus = IsBeepAlr();	//取m_fAlrStatus
	bool fOpenStatus = GetTurnStatus(); //取m_fTurnStatus

	//告警
	tCtrl.bCtrlTurn = m_iTurn;
	tInvCtrl.bCtrlTurn = tCtrl.bCtrlTurn;
	if (fAlarmStatus != m_fAlarmStatus)	//m_fAlarmStatus只是显示在用
	{
		if (fAlarmStatus)
		{//告警显示开始；
			tCtrl.bCtrlType = GetCtrlType();
			AddDispItem(tCtrl);
		}
	}

	//跳闸
	if (fOpenStatus != m_fOpenStatus) //m_fOpenStatus只是显示在用
	{//告警显示结束，跳闸显示开始；
		if (fOpenStatus)
		{
			tCtrl.bCtrlType = GetCtrlType();
			AddDispItem(tCtrl);	//这里的跳闸界面跟告警界面是一样的,避免有些告警是直接拉闸的
			m_dwOpenClick = GetClick();
		}
	}
	else 
	{
		if (m_dwOpenClick > 0)
		{
			m_dwOpenClick++;
			if (GetClick()-m_dwOpenClick > CTL_TURNCLOSE_TICK)
			{//跳闸显示结束；
				tInvCtrl.bCtrlType = GetCtrlType();
				RemoveDispItem(tInvCtrl);
				m_dwOpenClick = 0;
			}
		}
	}

	//合闸
	if (fCloseStatus)
	{//只要发出合闸信号，遥控跳闸显示消失；
		tCtrl.bCtrlType = GetCtrlType();
		RemoveDispItem(tCtrl);
		m_dwOpenClick = 0;
	}
	
	if (fCloseStatus != m_fCloseStatus)
	{
		if (fCloseStatus)
		{//有遥控状态变为合闸，发出合闸显示画面；
			tCtrl.bCtrlType = GetInvCtrlType();	//合闸类型
			AddDispItem(tCtrl);
			m_dwCloseClick = GetClick();
		}
		else
		{//只要遥控状态中，遥控合闸信号消失；且遥控合闸前必定有遥控投入；
			tInvCtrl.bCtrlType = GetInvCtrlType();
			RemoveDispItem(tInvCtrl);
			m_dwCloseClick = 0;
		}
	}
	else if (fCloseStatus)
	{
		if (GetClick()-m_dwCloseClick >= CTL_TURNCLOSE_TICK)
		{//合闸显示30s，结束；
			tInvCtrl.bCtrlType = GetInvCtrlType();
			RemoveDispItem(tInvCtrl);
			m_dwCloseClick = 0;
		}
	}

	m_fAlarmStatus = fAlarmStatus;
	m_fCloseStatus = fCloseStatus;
	m_fOpenStatus = fOpenStatus;
}


//描述: 复位'遥控'所有状态.
void CYkCtrl::RstCtrl(void)
{
	m_fTurnStatus  = false;
	m_fAlrStatus   = false;
}

//描述: 清除系统库'遥控'命令.
bool CYkCtrl::ClrSysCmd(int iTurn)
{
	if (m_iTurn<TURN_START_PN || m_iTurn>TURN_START_PN+TURN_NUM)
		return false;

	BYTE bCmd[10];

	memset(bCmd, 0, sizeof(bCmd));
	WriteItemEx(BN0, (BYTE)iTurn, 0x8203, bCmd);	//写相应轮次的"终端遥控投入命令"ID

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//触发保存.

	return true;
}

//描述: 设定系统库指定总加组本控制类的相应轮次状态.
//参数:@fStatus		true: 置位; false: 清除.
//返回: 如果设置成功返回 true,否则返回 false.
bool CYkCtrl::SetSysTurnStatus(int iTurn, bool fStatus)
{
	if (iTurn<TURN_START_PN || iTurn>TURN_START_PN+TURN_NUM)
		return false;

	BYTE bRelayMode;
	BYTE bBuf[32];	//最多8组数据

	//!!!如果在别的线程中会写该ID,可能需要进行信号量保护
	if (ReadItemEx(BN0, PN0, 0x8200, bBuf) <= 0)	//读"继电器输出状态".
	{
		DTRACE(DB_LOADCTRL, ("CYkCtrl::SetSysTurnStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	bBuf[0] = DT_BIT_STR;
	bBuf[1] = 8;
	if (fStatus)
		bBuf[2] |= (0x01<<(iTurn-TURN_START_PN)) & CTL_TURN_MASK;		//保存当前轮次'遥控跳闸输出状态'.
	else
		bBuf[2] &= ~((0x01<<(iTurn-TURN_START_PN)) & CTL_TURN_MASK);	//保存当前轮次'遥控跳闸输出状态'.
	WriteItemEx(BN0, PN0, 0x8200, bBuf);	//写"继电器输出状态".

	if (ReadItemEx(BN0, iTurn-1, 0xF205, bBuf) <= 0)	//读"继电器输出状态".
	{
		DTRACE(DB_LOADCTRL, ("CYkCtrl::SetSysTurnStatus: There is something wrong !\n"));
		return false;
	}

	BYTE *pbPtr = bBuf;
	*pbPtr++ = DT_STRUCT;
	*pbPtr++ = 4;
	*pbPtr++ = DT_VIS_STR;
	*pbPtr++ = 16;
	memset(pbPtr, 0x30, 16);
	pbPtr += 16;
	*pbPtr++ = DT_ENUM;
	*pbPtr++ = fStatus;
	ReadItemEx(BN1, PN0, 0x2022, &bRelayMode);
	*pbPtr++ = DT_ENUM;
	*pbPtr++ = (bRelayMode==1) ? 0:1;
	*pbPtr++ = DT_ENUM;
	*pbPtr++ = 0;

	WriteItemEx(BN0, iTurn-1, 0xF205, bBuf);	//写"继电器输出状态".

	return true;
}

//描述: 设定系统库本控制类的相应轮次告警状态.
//参数:@fStatus		true: 置位; false: 清除.
//返回: 如果设置成功返回 true,否则返回 false.
bool CYkCtrl::SetSysTurnAlrStatus(int iTurn, bool fStatus)
{
	if (iTurn<TURN_START_PN || iTurn>TURN_START_PN+TURN_NUM)
		return false;

	BYTE bBuf[4];

	//!!!如果在别的线程中会写该ID,可能需要进行信号量保护
	if (ReadItemEx(BN0, PN0, 0x8201, bBuf) <= 0)	//读"遥控告警状态".
	{
		DTRACE(DB_LOADCTRL, ("CYkCtrl::SetSysTurnAlrStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	bBuf[0] = DT_BIT_STR;
	bBuf[1] = 8;
	if (fStatus)
		bBuf[2] |= (0x01<<(iTurn-TURN_START_PN)) & CTL_TURN_MASK;		//保存当前轮次'遥控告警输出状态'.
	else
		bBuf[2] &= ~((0x01<<(iTurn-TURN_START_PN)) & CTL_TURN_MASK);	//保存当前轮次'遥控告警输出状态'.
	WriteItemEx(BN0, PN0, 0x8201, bBuf);	//写"遥控告警状态".

	return true;
}
