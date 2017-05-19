/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：UrgeFee.cpp
 * 摘    要：本文件主要实现CUrgeFee的类
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年3月
*********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "sysfs.h"
#include "FaCfg.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "ComAPI.h"
#include "TaskDB.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "UrgeFee.h"
#include "DrvAPI.h"

//========================================= CUrgeFee =============================================

CUrgeFee::CUrgeFee(void)
{
	memset(&m_CtrlCmd, 0, sizeof(m_CtrlCmd));	//'催费告警'命令.
	m_fLaunch = false;
	m_fAlrStatus = false;						//当前小时是否处于报警状态.
}

CUrgeFee::~CUrgeFee()
{

}

 
//描述: '催费告警'运行.
//返回: 正常则返回 true,否则返回 false.
bool CUrgeFee::DoCtrl(void)
{
	DoCmdScan();

	if (!IsValid())
	{
		RstCtrl();
		return true;
	}

	char cTime[20];

	bool fAlrStatus;
	if (GetBitStatus(m_CtrlCmd.bFlag, sizeof(m_CtrlCmd.bFlag), m_tmNow.nHour))
		fAlrStatus = true;		//当前小时是否处于报警状态.
	else
		fAlrStatus = false;		//当前小时是否处于报警状态.

	if (fAlrStatus != m_fAlrStatus)
	{
		m_fAlrStatus = fAlrStatus;
		SetSysCurStatus(m_fAlrStatus);
		SetSysCtrlStatus(m_fAlrStatus);
	}
	/*if ((m_fLaunch || m_tmNow.nHour!=m_tmOldTime.nHour) && GetBitStatus(bBuf, sizeof(bBuf), m_tmNow.nHour))
	{
		m_dwAlrStartTime = m_dwNow;
		m_fLaunch = false;
		DTRACE(DB_LOADCTRL, ("CUrgeFee::DoCtrl: alr start at %s\n", TimeToStr(m_tmNow, cTime)));
	}

	if (m_dwAlrStartTime!=0 && m_dwNow>m_dwAlrStartTime+60)	//默认告警持续时间是1分钟.
	{
		m_dwAlrStartTime = 0;
		DTRACE(DB_LOADCTRL, ("CUrgeFee::DoCtrl: alr stop at %s\n", TimeToStr(m_tmNow, cTime)));
	}*/

	return true;
}

//描述: 扫描系统库中的'催费告警'命令.
void CUrgeFee::DoCmdScan(void)
{
	char cTime[20];
	TUrgeFeeCmd NewCmd;

	BYTE bCmd[210];
	memset(bCmd, 0, sizeof(bCmd));
	//先根据命令时标检查是否收到新的命令.
	if (ReadItemEx(BN0, PN0, 0x8220, bCmd, &NewCmd.dwTime) < 0)	//读"终端催费告警投入命令"ID
	{
		DTRACE(DB_LOADCTRL, ("CUrgeFee::DoCmdScan: There is something wrong when call ReadItemEx() !\n"));
		return;
	}
	if (bCmd[0] != 1 && bCmd[0] != 2)
		return ;
	NewCmd.bAct = bCmd[0];
	memcpy(NewCmd.bFlag, bCmd+5, 3);//bCmd[1],表示bitstring个数24
	if (NewCmd.dwTime == 0) //终端时间往前调了,系统库中的命令时间被清掉了
	{
		if (m_CtrlCmd.dwTime != 0) //旧的命令已经被处理,只是系统库中的命令时间被清掉了
		{
			NewCmd.dwTime = GetCurTime(); //更新为一个合理一点的时间,方便以后不同命令时间的比较
			WriteItemEx(BN0, PN0, 0x8220, bCmd, NewCmd.dwTime);	//把相应总加组的"营业报停控命令"写会数据库
		}
		//else 让命令还是保持投入,只是命令的时间被清掉了,但也不能给命令一个明确的时间
		//	   它可能在投入后还没来得及扫描就被清除了,也可能是个已经被控制清除的旧命令,
		// 	   只是突然掉电没保存住;
		// 	   那除非它是系统中唯一被投入的命令,否则其它命令的时间比它后,它不会被执行
	}

	//如接收到的不是1或2,将不会对当前的控制状态产生影响,但在接收到新的有效命令前终端
	//一旦掉电,上电后,将会进入催费告警解除状态.
	if (NewCmd.dwTime != m_CtrlCmd.dwTime)
	{//时标不同,表示接收到新的命令,更新命令.
		DTRACE(DB_LOADCTRL, ("CUrgeFee::DoCmdScan: Receive new UrgeFee command, time of Receive is %s, .bAct=%d\n",
							 TimeToStr(NewCmd.dwTime, cTime), NewCmd.bAct));

		m_CtrlCmd = NewCmd;

		if (NewCmd.bAct == 1)		//新命令是'催费告警'投入命令.
		{
			SetSysCtrlStatus(true);
			SetValidStatus(true);
			m_fLaunch = true;
			DTRACE(DB_LOADCTRL, ("CUrgeFee::DoCmdScan: UrgeFee launch at %s\n",
								 TimeToStr(m_tmNow, cTime)));
			//***记录到系统库日志中.
			//***发出声光信号;
		}
		else //(NewCmd.bAct == 2)	//新命令是'催费告警'解除命令.
		{
			RstSysCtrlStatus();		//必须在清除系统库命令前,设置系统库'催费告警'状态.
			ClrSysCmd();
			ClrCmd();
			RstCtrl();
			DTRACE(DB_LOADCTRL, ("CUrgeFee::DoCmdScan: UrgeFee quit at %s\n",
								 TimeToStr(m_tmNow, cTime)));
			//***记录到系统库日志中.
			//***发出声光信号;
		}
	}
}

//描述: 复位'催费告警'所有状态.
void CUrgeFee::RstCtrl(void)
{
	SetValidStatus(false);
	if (m_fAlrStatus)
	{
		m_fAlrStatus = false;
		SetSysCtrlStatus(false);
	}
}

//描述: 清除系统库'催费告警'命令.
//返回: 如果清除成功返回 true,否则返回 false.
bool CUrgeFee::ClrSysCmd(void)
{
	BYTE bCmd[210] = {0};

	WriteItemEx(BN0, PN0, 0x8220, bCmd);	//写"终端催费告警投入命令"ID

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//触发保存.

	return true;
}

//描述: 设定系统库'催费告警'状态.
//参数:@fStatus		true: 置位; false: 清除.
//返回: 如果设置成功返回 true,否则返回 false.
bool CUrgeFee::SetSysCtrlStatus(bool fStatus)
{
	//如果催费告警控制为解除,则清除"终端当前控制状态"中的告警状态
	if (!fStatus)
		SetSysCurStatus(false);

	return true;
}

bool CUrgeFee::SetSysCurStatus(bool fStatus)
{
	BYTE bBuf[2];	//最多8组数据

	if (ReadItemEx(BN0, PN0, 0x8002, bBuf) <= 0)		//读"催费告警状态"
	{
		DTRACE(DB_LOADCTRL, ("CUrgeFee::SetSysCtrlStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	bBuf[0] = DT_ENUM;
	if (fStatus)
		bBuf[1] = 1; //当前催费告警状态
	else
		bBuf[1] = 0; //当前催费告警状态

	WriteItemEx(BN0, PN0, 0x8002, bBuf);						//写"催费告警状态"
	return true;
}

