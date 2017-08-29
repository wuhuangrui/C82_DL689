/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ShutoutCtrl.cpp
 * 摘    要：本文件主要实现CShutoutCtrl的类
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
#include "DbConst.h"
#include "ComAPI.h"
#include "TaskDB.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "ShutoutCtrl.h"

//========================================== CShutoutCtrl ==============================================
//描述: 运行控制.
//返回: 正常则返回 true,否则返回 false.
bool CShutoutCtrl::DoCtrl(void)
{
	DoCmdScan();		//扫描系统库中的命令.

	if (!IsValid())
	{
		RstCtrl();
		m_dwPwrStartClick = 0; //记下功率开始计算的时间,0表示之前没投入,从没投入转为投入,要等待功控滑差时间才能取功率
		return true;
	}
/*
	if (IsGuarantee())	//检测是否处在保电状态.
	{
		RstCtrl();					//复位内存中本类控制的所有相关状态.
		return true;
	}
*/
	TShutoutCtrlPara CtrlPara;

	if (!GetShutoutCtrlPara(m_iGrp, CtrlPara))
		return false;	//取'营业报停控'参数出现错误.

	char cTime[20];

	if (m_dwNow < CtrlPara.dwStartTime)
	{//假如还没到'营业报停控'启动时间.
		RstCtrl();
	}
	else if (m_dwNow < CtrlPara.dwEndTime)
	{//假如已过'营业报停控'启动时间,但还没到结束时间.
		m_fInCtrl = true;

		BYTE b = GetSysCtrlTurnsCfg(m_iGrp);

		if ((m_bTurnsStatus&~b) != 0)
		{//如非受控轮次发生跳闸的,应对这些轮次进行复位(置位允许合闸状态).
			m_bTurnsStatus &= b;
		}

		int64 iCurPwrLimit = GetPwrSafeLimit();	//获取功控保安定值.
		//取'营业报停控功率定值'和'功控保安定值'中的大者为'当前功率上限'.
		if (iCurPwrLimit < CtrlPara.iPwrLimit)
			iCurPwrLimit = CtrlPara.iPwrLimit;

		if (m_iCurPwrLimit != iCurPwrLimit)
		{
			SetSysCurPwrLimit(m_iGrp, iCurPwrLimit);
			m_iCurPwrLimit = iCurPwrLimit;
		}

		if (m_dwPwrStartClick == 0) //0表示之前功控没投入,从没投入转为投入,要等待功控滑差时间才能取功率
		{
			m_dwPwrStartClick = GetClick(); //记下功率开始计算的时间
			return true;
		}
		else if (GetClick()-m_dwPwrStartClick < GetPwrSlideInterv(m_iGrp))
		{
			return true;	//等功控滑差时间才能取功率
		}

		int64 iCurPwr = GetCurPwr(m_iGrp);	// 获取当前功率
		if (iCurPwr < 0)
			return false;
		int i = GetIdxOfMostRight1(b & ~GetTurnsStatus());		//获取相应总加组当前可跳闸的轮次号.

		if (i<0 && iCurPwr>m_iCurPwrLimit)									//检测是否有可跳闸.
		{
			m_dwAlrTime = 0;						//无闸可跳了,报警没有意义,禁止报警.
			m_dwGuaranteeAlrTime = 0;
			return true;
		}
		m_bWarnStatus = i+1;
		int iTurn;
		DWORD dwPersistTime = GetPwrAlrPersistTime(i+TURN_START_PN);	//获取相应轮次的功控报警持续时间.
		
		if (iCurPwr <= m_iCurPwrLimit)
		{
			m_fIfOverLimit = false;
//			m_dwAlrTime = 0;	//解除报警.
			m_dwGuaranteeAlrTime = 0;
//			m_bTurnsStatus = 0;  //合闸
			if (i < 0) 
			{
				m_bWarnStatus = 4;
			}
			else
			{
				m_bWarnStatus = i;
			}
			if (m_bTurnsStatus != 0)
			{
				if (m_CtrlType != CTL_PWR_SHUTOUT_ALLCLOSE)
				{
					m_dwAlrTime = 0;
					m_CtrlType = CTL_PWR_SHUTOUT_ALLCLOSE;				
				}
				else
				{
					if (m_dwAlrTime == 0)
					m_dwAlrTime = m_dwNow;
					
					iTurn= GetIdxOfMostLeft1(m_bTurnsStatus);	//获取相应总加组当前可跳闸的轮次号.
					dwPersistTime = GetPwrAlrPersistTime(iTurn);	//获取相应轮次的功控报警持续时间.
					m_bWarnStatus = iTurn;
					//SaveDisp(dwPersistTime,m_dwAlrTime, iCurPwr);
					RestoreTurnStatus();			
				}
			}
			else
			{
				m_dwAlrTime = 0;
				m_CtrlType = CTL_PWR_SHUTOUT;
			}
		}
		else //(iCurPwr > m_iCurPwrLimit)
		{//假如当前功率 > 当前功率上限,开始启动功率控制流程.
			m_fIfOverLimit = true;
			if (m_CtrlType != CTL_PWR_SHUTOUT)
			{
				m_CtrlType = CTL_PWR_SHUTOUT;	
				m_dwAlrTime = 0;
				return true;
			}

			if (m_dwAlrTime > m_dwNow)	//变成未来的时间了,时间往前调回去了
				m_dwAlrTime = 0;

			if (m_dwAlrTime==0)
			{
				m_dwAlrTime = m_dwNow;
				DTRACE(DB_LOADCTRL, ("CShutoutCtrl::DoCtrl: alarm start at %s, turn=%d, grp=%d, current-power=%lld, limit=%lld, alr-persistent-time=%ldS\n",
									 TimeToStr(m_tmNow, cTime), i+TURN_START_PN, m_iGrp, iCurPwr, m_iCurPwrLimit, dwPersistTime));
				//***记录到系统库日志中.
				//***发出声光信号;
			}
			if (IsGuarantee())
			{
				if (m_dwGuaranteeAlrTime==0)
				{
					m_dwGuaranteeAlrTime = m_dwNow;
					m_dwAlrTime = m_dwNow;
				}
				m_bTurnsStatus = 0;
				if (m_dwNow > m_dwGuaranteeAlrTime+dwPersistTime)
				{
					m_dwAlrTime = 0;
				}
				else
				{
					DWORD dwCntDown;
					if (m_dwNow <= m_dwAlrTime+dwPersistTime)
						dwCntDown = m_dwAlrTime + dwPersistTime - m_dwNow;
					else
						dwCntDown = 0;
					//SaveDisp(dwPersistTime,m_dwAlrTime, iCurPwr);
				}
				return true;
			}
			if (m_dwNow > m_dwAlrTime+dwPersistTime)
			{//假如'当前时间' > '营业报停控报警启动时间'+'营业报停控报警持续时间'
				m_bTurnsStatus |= 0x01 << i;
				m_dwAlrTime = 0;							//该轮跳闸后,应该把跳闸报警关闭.
				m_wOpenTimes++;								//跳闸次数增加1.

				//保存跳闸记录中间数据.
				/*BYTE bRecBuf[4+1+2+2];

				memcpy(bRecBuf, &m_dwNow, 4);				//保存跳闸时间.
				bRecBuf[4] = (BYTE)m_iGrp;					//保存总加组.
				Val64ToFmt2(iCurPwr, bRecBuf+5, 2);		//保存跳闸时功率.
				Val64ToFmt2(m_iCurPwrLimit, bRecBuf+7, 2);	//保存跳闸时功率定值.
				WriteItemEx(BN0, (WORD)(i+TURN_START_PN), 0x0a02, bRecBuf);
				TrigerSaveBank(BN0, SECT_CTRL, 0);*/

				DTRACE(DB_LOADCTRL, ("CShutoutCtrl::DoCtrl: open break at %s, turn=%d, grp=%d\n", TimeToStr(m_tmNow, cTime), i+TURN_START_PN, m_iGrp));
				//***发出声光信号;
			}

			//设置功控显示参数；
			DWORD dwCntDown;
			if (m_dwNow <= m_dwAlrTime+dwPersistTime)
				dwCntDown = m_dwAlrTime + dwPersistTime - m_dwNow;
			else
				dwCntDown = 0;
			//SaveDisp(dwPersistTime,m_dwAlrTime, iCurPwr);
		}
	}
	else
	{//假如已过'营业报停控'结束时间.
		if (m_iCurPwrLimit!=0 && m_iGrp>=0)
		{
			//SetSysCurPwrLimit(m_iGrp, INVALID_VAL64);
			m_iCurPwrLimit = 0;
			DTRACE(DB_LOADCTRL, ("CShutoutCtrl::DoCtrl: out of date, quit at %s, grp=%d\n", TimeToStr(m_tmNow, cTime), m_iGrp));
		}

		RstCtrl();					//复位内存中本类控制的所有相关状态.
		//***记录到系统库日志中.
		//***发出声光信号;
	}

	return true;
}

//描述: 复位内存中本类控制状态量.
void CShutoutCtrl::RstCtrl(void)
{
	m_bTurnsStatus	= 0x00;			//将轮次状态全部设为合闸.
	m_dwAlrTime		= 0;			//将报警时间(开始报警时间)清零.
	m_dwGuaranteeAlrTime = 0;
	m_iCurPwrLimit		= 0;		//将当前功控定值设为0.

	m_fIfOverLimit  = false;
	m_fInCtrl = false;				//处于保电状态的时候也会调用RstCtrl()来复位本标志
									//可能不一定会完全反应真实的状态,不过即使执行了优先级
									//更低的控制,也没关系,因为它们也处于保电的状态,顶多是扫描一下命令
}

//描述: 获取某总加组的本类控制命令,并把命令放到 m_NewCmd 中.(注意: 对不同的类,m_NewCmd的结构是不同的)
//参数:@iGrp	要获取命令的总加组.
//返回: 如果获取成功且为有效命令 true,否则返回 false.
bool CShutoutCtrl::GetSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[1];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8253, bCmd, &m_NewCmd.dwTime) <=0)	//读取相应总加组的"营业报停控命令".
	{
		DTRACE(DB_LOADCTRL, ("CShutoutCtrl::GetSysCmd: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	m_NewCmd.bAct = bCmd[0];
	if (m_NewCmd.bAct!=1 && m_NewCmd.bAct!=2)
		return false;

	if (m_NewCmd.dwTime == 0) //终端时间往前调了,系统库中的命令时间被清掉了
	{
		if (CurCmdTime()!=0 && iGrp==m_iGrp) //旧的命令已经被处理,只是系统库中的命令时间被清掉了
		{
			m_NewCmd.dwTime = GetCurTime(); //更新为一个合理一点的时间,方便以后不同命令时间的比较
			WriteItemEx(BN0, (WORD)iGrp, 0x8253, bCmd, m_NewCmd.dwTime);	//把相应总加组的"营业报停控命令"写会数据库
		}
		//else 让命令还是保持投入,只是命令的时间被清掉了,但也不能给命令一个明确的时间
		//	   它可能在投入后还没来得及扫描就被清除了,也可能是个已经被控制清除的旧命令,
		// 	   只是突然掉电没保存住;
		// 	   那除非它是系统中唯一被投入的命令,否则其它命令的时间比它后,它不会被执行
	}

	return true;
}

//描述: 清除系统库指定总加组'营业报停控'命令.
//参数:@iGrp		要清除命令的总加组.
//返回: 如果清除成功返回 true,否则返回 false.
bool CShutoutCtrl::ClrSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[1] = {0};

	WriteItemEx(BN0, (WORD)iGrp, 0x8253, bCmd);		//清除相应总加组的"营业报停控命令".

	TrigerSaveBank(BN0, SECT_CTRL, 0); //触发保存.

	return true;
}

//描述: 设置系统库本类控制轮次输出状态.
//参数:@iGrp	当前控制的总加组.
//		@bTurnsStatus	轮次状态
//返回: 如果设置成功返回 true,否则返回 false.
bool CShutoutCtrl::SetSysCtrlTurnsStatus(int iGrp, BYTE bTurnsStatus)
{
	BYTE bBuf[10];
	memset(bBuf, 0, sizeof(bBuf));

	BYTE *pbtr = bBuf;
	*pbtr++ = DT_STRUCT;
	*pbtr++ = 2;					//结构成员个数
	*pbtr++ = DT_OI;				//总加组对象
	pbtr += OoWordToOi(0x2300+iGrp, pbtr);
	*pbtr++ = DT_BIT_STR;
	*pbtr++ = 8;
	*pbtr++ = bTurnsStatus;
	WriteItemEx(BN0, iGrp, 0x8251, bBuf);

	return true;
}

//描述: 获取指定总加组'营业报停控'参数.
//参数:@iGrp	要获取的总加组.
//	   @rPara	引用的参数结构,本函数读到数据后,通过本结构将参数传回.
//返回: 成功返回 true,否则返回 false.
bool CShutoutCtrl::GetShutoutCtrlPara(int iGrp, TShutoutCtrlPara& rPara)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bBuf[30];
	TTime tm;

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8105, bBuf) <=0)	//F44,读取相应总加组的"营业报停控参数"
	{
		DTRACE(DB_LOADCTRL, ("CShutoutCtrl::GetShutoutCtrlPara: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	OoDateTimeSToTime(bBuf+6, &tm);
	tm.nSecond = 0;
	tm.nMinute = 0;
	tm.nHour = 0;
	rPara.dwStartTime	= TimeToSeconds(tm);

	OoDateTimeSToTime(bBuf+14, &tm);
	tm.nSecond = 0;
	tm.nMinute = 0;
	tm.nHour = 0;
	rPara.dwEndTime		= TimeToSeconds(tm);
	rPara.iPwrLimit		= OoLong64ToInt64(bBuf+22);

	if (rPara.dwStartTime > rPara.dwEndTime)
	{
		DTRACE(DB_LOADCTRL, ("CShutoutCtrl::GetShutoutCtrlPara: err! start time %02x-%02x-%02x > end time %02x-%02x-%02x\n",
							 bBuf[2], bBuf[1], bBuf[0], 
							 bBuf[5], bBuf[4], bBuf[3]));
		return false;
	}

	if (rPara.iPwrLimit==0 || IsAllAByte(bBuf+6, INVALID_DATA, 2)) //定值没设
	{
		DTRACE(DB_LOADCTRL, ("CShutoutCtrl::GetShutoutCtrlPara: err! limit not set\n"));
		return false;
	}

	return true;
}
