/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Guarantee.cpp
 * 摘    要：本文件主要实现CGuarantee的类
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
#include "Guarantee.h"

//========================================= CGuarantee =============================================
//描述: 初始化.
//返回: 如果初始化正常返回 true,否则返回 false.
bool CGuarantee::Init(void)
{
	ClrCmd();		//清除内存中本类控制的控制命令.
	RstCtrl();		//复位内存中本类控制状态量.

	m_fUnconnect = false;

	return true;
}

//描述: '保电控'运行.
//返回: 正常则返回 true,否则返回 false.
bool CGuarantee::DoCtrl(void)
{
	char cTime[20];
	BYTE bBuf[1];

	//检测连续无通讯是否超时.
	ReadItemEx(BN2, PN0, 0x2021, bBuf); //连续无通讯超时后进入保电状态
	if (bBuf[0]==1 || IsAutoGuaranteePeriod())	//进入保电状态
	{
		if (!m_fUnconnect)
		{
			SetSysCtrlStatus(AUTO_GUARANTEE);
			SetValidStatus(true);
			ClrCmd();	//必须清除内存中的命令备份,以便在通讯恢复后,能够重新扫描系统库命令.
			m_fUnconnect = true;
			DTRACE(DB_LOADCTRL, ("CGuarantee::DoCtrl: Long time no communication with the host, Guarantee launch at %s\n", 
								 TimeToStr(m_tmNow, cTime)));
		}

		return true;
	}
	else
	{
		if (m_fUnconnect)
		{
			RstSysCtrlStatus();
			RstCtrl();
			m_fUnconnect = false;
			DTRACE(DB_LOADCTRL, ("CGuarantee::DoCtrl: Communication with the host resume, Guarantee quit at %s\n", 
								 TimeToStr(m_tmNow, cTime)));
		}
	}

	DoCmdScan();

	if (!IsValid())
	{
		RstCtrl();
		return true;
	}

	/*****如果终端时间比下发命令时间早，在命令扫描处已有判断，此处不再需要***/ //Modified by chenxi,13th,June,2008
	//if (m_dwNow < m_CtrlCmd.dwTime)
	//{
	//	char cTime1[20];

	//	DTRACE(DB_LOADCTRL, ("CGuarantee::DoCtrl: Time of receive Guarantee command is later than now!\n"\
	//						 "Time of Receive is %s\nNow is %s\n",
	//						 TimeToStr(m_CtrlCmd.dwTime, cTime1), TimeToStr(m_tmNow, cTime)));
	//	//报错;	//除非在记录下接收命令时间后,系统时间进行了调整,否则不会发生这种情况.
	//	return false;
	//}

	if (m_CtrlCmd.dwPersistTime==0 || m_dwNow<m_CtrlCmd.dwTime+m_CtrlCmd.dwPersistTime)
	{
		SetValidStatus(true);
	}
	else
	{//'保电控'持续时间已过,保电状态将从有效变为无效
		RstSysCtrlStatus();		//必须在清除系统库命令前,设置系统库'保电控'状态.
		ClrSysCmd();
		ClrCmd();
		RstCtrl();
		DTRACE(DB_LOADCTRL, ("CGuarantee::DoCtrl: Guarantee finish at %s\n", 
							 TimeToStr(m_tmNow, cTime)));
		//***记录到系统库日志中.
		//***发出声光信号;
	}

	return true;
}

//描述: 扫描系统库中的'保电控'命令.
void CGuarantee::DoCmdScan(void)
{
	TGuaranteeCmd NewCmd;
	BYTE bCmd[2];

	//先根据命令时标检查是否收到新的命令.
	if (ReadItemEx(BN0, PN0, 0x8213, bCmd, &NewCmd.dwTime) <= 0)	//读"终端保电投入命令"ID
	{
		DTRACE(DB_LOADCTRL, ("CGuarantee::DoCmdScan: There is something wrong when call ReadItemEx() !\n"));
		return;
	}

	NewCmd.bAct = bCmd[0];
	NewCmd.dwPersistTime = 0;

	if (NewCmd.bAct !=1 && NewCmd.bAct!=2 && NewCmd.bAct!=3)	//没有收到命令
		return;

	char cTime[20];
	//现在的时间比下发命令的时间要早（测试时修改终端时间），则控制命令要撤销；
	if (NewCmd.dwTime == 0)
	{
		if (NewCmd.dwPersistTime == 0) //无限期保电
		{
			NewCmd.dwTime = GetCurTime(); //更新为一个合理一点的时间,方便以后不同命令时间的比较
			WriteItemEx(BN0, PN0, 0x8213, bCmd, NewCmd.dwTime);	//把相应总加组的"时段控命令"写会数据库
		}
		else if (m_CtrlCmd.dwTime!=0 //旧的命令已经被扫描进来执行
				 && m_dwOldTime>=m_CtrlCmd.dwTime && m_dwOldTime<m_CtrlCmd.dwTime+m_CtrlCmd.dwPersistTime)
		{		//把命令的时间当成click的方式来用
			NewCmd.dwTime = GetCurTime() - (m_dwOldTime-m_CtrlCmd.dwTime); //去掉已经消耗掉的时间
			WriteItemEx(BN0, PN0, 0x8213, bCmd, NewCmd.dwTime);	//把相应总加组的"时段控命令"写会数据库
		}
		else
		{
			RstSysCtrlStatus();	 //必须在清除系统库命令前,设置系统库'保电控'状态.
			ClrSysCmd();
			ClrCmd();
			RstCtrl();
			DTRACE(DB_LOADCTRL, ("CGuarantee::DoCmdScan: Guarantee quit at %s\n",
								 TimeToStr(m_tmNow, cTime)));
		}
		return;
	}

	//如接收到的不是1或2,将不会对当前的控制状态产生影响,但在接收到新的有效命令前终端
	//一旦掉电,上电后,将会进入保电解除状态.
	if (NewCmd.dwTime != m_CtrlCmd.dwTime)
	{//时标不同,表示接收到新的命令,更新命令.
		m_CtrlCmd = NewCmd;
		DTRACE(DB_LOADCTRL, ("CGuarantee::DoCmdScan: rx new cmd at %s, act=%d, persist-time=%ld\n",
							 TimeToStr(NewCmd.dwTime, cTime), NewCmd.bAct, NewCmd.dwPersistTime));
		if (NewCmd.bAct == 1)		//新命令是'保电控'投入命令.
		{
			SetSysCtrlStatus(INPUT_GUARANTEE);
			SetValidStatus(true);
			DTRACE(DB_LOADCTRL, ("CGuarantee::DoCmdScan: guarantee launch at %s\n",
								 TimeToStr(m_tmNow, cTime)));
			//***记录到系统库日志中.
			//***发出声光信号;
		}
		else //(NewCmd.bAct == 2)	//新命令是'保电控'解除命令.
		{
			RstSysCtrlStatus();		//必须在清除系统库命令前,设置系统库'保电控'状态.
			ClrSysCmd();
			ClrCmd();
			RstCtrl();
			DTRACE(DB_LOADCTRL, ("CGuarantee::DoCmdScan: guarantee quit at %s\n",
								 TimeToStr(m_tmNow, cTime)));
			//***记录到系统库日志中.
			//***发出声光信号;
		}
	}
}

bool CGuarantee::IsAutoGuaranteePeriod()
{
	TTime tmNow;
	BYTE bBuf[150], bPeriodNum;

	if (ReadItemEx(BN0, PN0, 0x8212, bBuf) < 0)		//读自动保电时段
	{
		DTRACE(DB_LOADCTRL, ("CGuarantee::DoAutoGuarantee: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	bPeriodNum = bBuf[1];
	
	GetCurTime(&tmNow);
	for (BYTE i=0; i<bPeriodNum; i++)
	{
		if (bBuf[2+6*i+3]<=tmNow.nHour && tmNow.nHour<=bBuf[2+6*i+5])
			return true;
	}

	return false;
}

//描述: 复位'保电控'所有状态.
void CGuarantee::RstCtrl(void)
{
	SetValidStatus(false);
}

//描述: 清除系统库'保电控'命令.
//返回: 如果清除成功返回 true,否则返回 false.
bool CGuarantee::ClrSysCmd(void)
{
	BYTE bCmd[2] = {0, 0};

	WriteItemEx(BN0, PN0, 0x8213, bCmd);	//写"终端保电投入命令"ID

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//触发保存.

	return true;
}

//描述: 设定系统库'保电控'状态.设置控制状态
//参数:@bStatus	状态：解除（0），保电（1），自动保电（2）
//返回: 如果设置成功返回 true,否则返回 false.
bool CGuarantee::SetSysCtrlStatus(BYTE bStatus)
{
	BYTE bBuf[2];
	TTime t;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	//!!!如果在别的线程中会写该ID,可能需要进行信号量保护
	if (ReadItemEx(BN0, PN0, 0x8001, bBuf) < 0)		//读"终端控制设置状态"
	{
		DTRACE(DB_LOADCTRL, ("CGuarantee::SetSysCtrlStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	bBuf[0] = DT_ENUM;
	bBuf[1] = bStatus;
	WriteItemEx(BN0, PN0, 0x8001, bBuf, dwSecs);						//写"终端控制设置状态"

	return true;
}
