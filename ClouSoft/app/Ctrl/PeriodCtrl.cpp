/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：PeriodCtrl.cpp
 * 摘    要：本文件主要实现CPeriodCtrl的类
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
#include "PeriodCtrl.h"
#include "DbOIAPI.h"

//========================================== CPeriodCtrl ==============================================
//描述: '时段控'控制.
//返回: 正常则返回 true,否则返回 false.
bool CPeriodCtrl::DoCtrl(void)
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
	if (m_tmNow.nDay!=m_tmOldTime.nDay || m_tmNow.nMonth!=m_tmOldTime.nMonth || m_tmNow.nYear!=m_tmOldTime.nYear)
	{//假如发生日切换.
		RstCtrl();
	}

	int i1;

	if ((i1=GetTimePeriod(m_tmNow)) < 0)		//获取当前的时段号.
	{//时段号小于0时表示当前时间所处的时段没有功控投入.
		RstCtrl();
		SetSysCurPwrLimit(m_iGrp, m_iCurPwrLimit);
		return true;
	}

	if (m_iCurPeriodIdx != i1)
	{//时段发生了切换,将所有闸复位.
		RstCtrl();
		m_iCurPeriodIdx = i1;		//更新当前时段号.
	}

	int64 iCurPwrLimit;

	if ((m_CtrlCmd.bFlgs&(0x01<<i1))==0
		|| !GetPeriodLimit(m_iGrp, m_CtrlCmd.bScheme, i1, iCurPwrLimit))
	{//假如当前时段不启用功控或指定总加组指定方案的指定时段没有功控限制.
		RstCtrl();
		return true;
	}

	BYTE b = GetSysCtrlTurnsCfg(m_iGrp);

	if ((m_bTurnsStatus&~b) != 0)
	{//如非受控轮次发生跳闸的,应对这些轮次进行复位(置位允许合闸状态).
		m_bTurnsStatus &= b;
	}

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
	int i = GetIdxOfMostRight1(b & ~GetTurnsStatus());	//获取相应总加组当前可跳闸的轮次号.
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
//		m_dwAlrTime = 0;	//解除报警.		
		m_dwGuaranteeAlrTime = 0;
		if (i < 0 ) 
		{
			m_bWarnStatus = 4;
		}
		else
			m_bWarnStatus = i;
		if (m_bTurnsStatus != 0)
		{
			if(m_CtrlType != CTL_PWR_PERIOD_ALLCLOSE)
			{
				m_dwAlrTime = 0;
				m_CtrlType = CTL_PWR_PERIOD_ALLCLOSE;
			}
			else
			{
				if (m_dwAlrTime == 0)
					m_dwAlrTime = m_dwNow;
				RestoreTurnStatus();
				iTurn= GetIdxOfMostLeft1(m_bTurnsStatus);	//获取相应总加组当前可跳闸的轮次号.
				m_bWarnStatus = iTurn;
				dwPersistTime = GetPwrAlrPersistTime(iTurn);	//获取相应轮次的功控报警持续时间.
				//SaveDisp(dwPersistTime,m_dwAlrTime, iCurPwr);				
			}
		
		}
		else
		{
			m_dwAlrTime = 0;	//解除报警.
			m_CtrlType = CTL_PWR_PERIOD;
		}
	}
	else //(iCurPwr > m_iCurPwrLimit)
	{//假如当前功率 > 当前功率上限,开始启动功率控制流程.
		char cTime[20];

		m_fIfOverLimit = true;
		if (m_CtrlType != CTL_PWR_PERIOD)
		{
			m_CtrlType = CTL_PWR_PERIOD;	
			m_dwAlrTime = 0;
			return true;
		}
		if (m_dwAlrTime > m_dwNow)	//变成未来的时间了,时间往前调回去了
			m_dwAlrTime = 0;

		if (m_dwAlrTime==0)
		{
			m_dwAlrTime = m_dwNow;
			DTRACE(DB_LOADCTRL, ("CPeriodCtrl::DoCtrl: Turn[%d] of Group[%d] PeriodCtrl Alarm start ...\n"\
								 "Current power is %lld, Period Limit is %lld\n"\
								 "Persistent time of alarm is %ld seconds\n"\
								 "at %s\n", i+TURN_START_PN, m_iGrp, iCurPwr, m_iCurPwrLimit, dwPersistTime, TimeToStr(m_tmNow, cTime)));
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
		{//假如'当前时间' > '厂休控报警启动时间'+'厂休控报警持续时间'
			m_bTurnsStatus |= 0x01 << i;
			m_dwAlrTime = 0;							//该轮跳闸后,应该把跳闸报警关闭.
			m_wOpenTimes++;								//跳闸次数增加1.

			//保存跳闸记录中间数据.
			/*BYTE bRecBuf[4+1+2+2];

			memcpy(bRecBuf, &m_dwNow, 4);				//保存跳闸时间.
			bRecBuf[4] = (BYTE)m_iGrp;					//保存总加组.
			Val64ToFmt2(iCurPwr, bRecBuf+5, 2);			//保存跳闸时功率.
			Val64ToFmt2(m_iCurPwrLimit, bRecBuf+7, 2);	//保存跳闸时功率定值.
			WriteItemEx(BN0, (WORD)(i+TURN_START_PN), 0x0a04, bRecBuf);
			TrigerSaveBank(BN0, SECT_CTRL, 0);*/

			DTRACE(DB_LOADCTRL, ("CPeriodCtrl::DoCtrl: Turn[%d] of Group[%d] PeriodCtrl open break!\n"\
								 "at %s\n", i+TURN_START_PN, m_iGrp, TimeToStr(m_tmNow, cTime)));
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

	return true;
}

//描述: 复位内存中本类控制状态量.
void CPeriodCtrl::RstCtrl(void)
{
	m_bTurnsStatus	= 0x00;				//将轮次状态全部设为合闸.
	m_dwAlrTime		= 0;				//将报警时间(开始报警时间)清零.
	m_dwGuaranteeAlrTime = 0;
	m_iCurPwrLimit	= 0;				//将当前功控定值设为0.

	m_iCurPeriodIdx	= -1;				//当前时段置为 -1.
	m_fIfOverLimit  = false;
}

//描述: 获取某总加组的本类控制命令,并把命令放到 m_NewCmd 中.(注意: 对不同的类,m_NewCmd的结构是不同的)
//参数:@iGrp	要获取命令的总加组.
//返回: 如果获取成功且为有效命令 true,否则返回 false.
bool CPeriodCtrl::GetSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[8];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8233, bCmd, &m_NewCmd.dwTime) <=0)	//读取相应总加组的"时段控命令".
	{
		DTRACE(DB_LOADCTRL, ("CPeriodCtrl::GetSysCmd: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	m_NewCmd.bAct	 = bCmd[0];
	m_NewCmd.bFlgs	 = bCmd[5];
	m_NewCmd.bScheme = bCmd[7];

	if (m_NewCmd.bAct!=1 && m_NewCmd.bAct!=2)
		return false;

	if (m_NewCmd.dwTime == 0) //终端时间往前调了,系统库中的命令时间被清掉了
	{
		if (CurCmdTime()!=0 && iGrp==m_iGrp) //旧的命令已经被处理,只是系统库中的命令时间被清掉了
		{
			m_NewCmd.dwTime = GetCurTime(); //更新为一个合理一点的时间,方便以后不同命令时间的比较
			WriteItemEx(BN0, (WORD)iGrp, 0x8233, bCmd, m_NewCmd.dwTime);	//把相应总加组的"时段控命令"写会数据库
		}
		//else 让命令还是保持投入,只是命令的时间被清掉了,但也不能给命令一个明确的时间
		//	   它可能在投入后还没来得及扫描就被清除了,也可能是个已经被控制清除的旧命令,
		// 	   只是突然掉电没保存住;
		// 	   那除非它是系统中唯一被投入的命令,否则其它命令的时间比它后,它不会被执行
	}

	return true;
}

//描述: 清除系统库指定总加组'时段控'命令.
//参数:@iGrp		要清除命令的总加组.
//返回: 如果清除成功返回 true,否则返回 false.
bool CPeriodCtrl::ClrSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[8] = {0};

	WriteItemEx(BN0, (WORD)iGrp, 0x8233, bCmd);		//清除相应总加组的"时段控命令".

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//触发保存.

	return true;
}

//描述: 获取指定时间所处的时段.
//参数:@bTime	要获取的总加组.
//返回: 所处时段有功控投入,返回时段号,否则返回 -1.
int CPeriodCtrl::GetTimePeriod(TTime Time)
{
	BYTE b = (BYTE)Time.nHour*2 + (BYTE)(Time.nMinute>=30 ? 1 : 0);  //把一天内的时间转换成以半小时做单位
	BYTE b1 = b / 4;	//本时段位于的字节偏移
	BYTE b2 = b % 4;	//本时段位于字节内的偏移

	BYTE bBuf[30];
	if (ReadItemEx(BN0, PN0, 0x8101, bBuf) <=0)		//读入"终端功控时段".
	{
		DTRACE(DB_LOADCTRL, ("CPeriodCtrl::GetTimePeriod: There is something wrong when call ReadItemEx() !\n"));
		return -1;
	}

	int iPeriodIdx = -1;
	bool bStatus = false;
	BYTE bCtrl = 0xff;	//只要不是 0,1,2,3 四种值即可.

	for (BYTE b0=0; b0<(2*24); b0++)	//每半小时地检索出
	{
		BYTE b3 = (b0 / 4)*2 + 3;	//当前的字节偏移
		BYTE b4 = b0 % 4;	//当前的位于字节内的偏移
		BYTE bCtrl1 = (bBuf[b3]>>(b4*2)) & 0x03;
		if (bCtrl != bCtrl1)
		{
			if (bCtrl1==1 || bCtrl1==2)
			{
				iPeriodIdx++;
				bStatus = true;
			}
			else
				bStatus = false;
		}
		bCtrl = bCtrl1;
		if (b1==b3 && b2==b4)	//到了当前的时段
			break;
	}

	if (bStatus)	//当前处于控制状态的才返回时段
		return iPeriodIdx;
	else			//当前不处于控制状态
		return -1;
}

//描述: 获取指定总加组当前时段的功控上限.
//参数:@iGrp			要获取的总加组.
//	   @iScheme			方案号.
//	   @iPeriodIdx		时段号.
//	   @riPwrLimit		引用的参数,本函数读到数据后,通过参数将功控上限传回.
//返回: 如果相应的总加组,相应的方案号,相应的时段有功控投入则返回 true, 否则返回 false.
bool CPeriodCtrl::GetPeriodLimit(int iGrp, int iScheme, int iPeriodIdx, int64& riPwrLimit)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	WORD wLen;
	BYTE bType, bBuf[256];		//虽然在国标中最多只有3组方案,但这里扩展到最多8组.

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8103, bBuf) <=0)	//变长//从指定总加组读入"时段功控定值".
	{
		DTRACE(DB_LOADCTRL, ("CPeriodCtrl::GetPeriodLimit: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	if (iScheme>2 || iPeriodIdx>7)	//先检查方案号和时段号的范围.
		return false;

	//2008-04-10 因为该参数是变长的,需进一步确认某个方案参数的起始地址.
	int i;

	if ((i=GetIdxOfAll1InPst(bBuf[7]&0x07, iScheme)) < 0)	//只用0,1,2三位.
		return false;	//没有对应的方案定值,返回 false.

	BYTE* pb = &bBuf[8];

	const ToaMap *p = GetOIMap(0x81030300);
	if (p ==NULL)
		return false;

	if ((pb=OoGetField(bBuf, p->pFmt, p->wFmtLen, 1+i, &wLen, &bType)) == NULL)//取得对应方案的功率定值
	{
		return false;
	}

	//if ((i=GetIdxOfAll1InPst(*pb, iPeriodIdx)) < 0)
	//	return false;	//相应的时段没有功控定值,返回 false.

	riPwrLimit = OoLong64ToInt64(pb+6+iPeriodIdx*9);				//获取相应的功控定值.
	if ((pb=OoGetField(bBuf, p->pFmt, p->wFmtLen, 5, &wLen, &bType)) == NULL)//取得'时段控'定值浮动系数.
	{
		DTRACE(DB_LOADCTRL, ("CPeriodCtrl::GetPeriodLimit: There is something wrong when call OoGetField() !\n"));
		return false;
	}

	riPwrLimit = riPwrLimit * (100+(int8)pb[1]) / 100;

	int64 iSafeLimit = GetPwrSafeLimit();	//获取功控保安定值
	//取'时段控'当前时段功率上限和'功控保安定值'中的大者为'当前功率上限'.
	if (riPwrLimit < iSafeLimit)
		riPwrLimit = iSafeLimit;

	return true;
}

//描述: 将本总加组系统库本类控制标志设为指定状态.
//参数:@iGrp			要设定的总加组.
//	   @fStatus			状态.
//返回: 如果设置成功则返回 true, 否则返回 false.
bool CPeriodCtrl::SetSysCtrlFlg(int iGrp, bool fStatus)
{
	if (fStatus)
	{
		BYTE bBuf[1+1+6*8];

		if (ReadItemEx(BN0, PN0, 0x104f, bBuf) <=0)	//读"终端控制设置状态"ID
		{
			DTRACE(DB_LOADCTRL, ("CPeriodCtrl::SetSysCtrlFlg: There is something wrong when call ReadItemEx() !\n"));
			return false;
		}
		bBuf[1+1+6*(iGrp-GRP_START_PN)+0] = m_CtrlCmd.bScheme;	//写'功控定值方案号'.
		bBuf[1+1+6*(iGrp-GRP_START_PN)+1] = m_CtrlCmd.bFlgs;	//写'功控时段有效标志位'.

		WriteItemEx(BN0, PN0, 0x104f, bBuf);					//写"终端控制设置状态"ID

		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x01, fStatus, PWR_CTL);//时段控使用0位.
	}
	else
		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x01, fStatus, PWR_CTL);//时段控使用0位.
}


