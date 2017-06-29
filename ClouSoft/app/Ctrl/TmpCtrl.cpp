/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TmpCtrl.cpp
 * 摘    要：本文件主要实现CTmpCtrl的类
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
#include "TmpCtrl.h"
#include "DpGrp.h"

//========================================== CTmpCtrl ==============================================
//描述: 运行控制.
//返回: 正常则返回 true,否则返回 false.
bool CTmpCtrl::DoCtrl(void)
{
	DoCmdScan();		//扫描系统库中的命令.

	if (!IsValid())
	{
		RstCtrl();
		return true;
	}

	char cTime[20], cStr[20];
	TTime tmCmd;
	int iTurn;
	BYTE bTmpBuf[9] = {0};
	SecondsToTime(m_CtrlCmd.dwTime, &tmCmd);
/*
	if (IsGuarantee())
	{//保电命令投入，命令保存，状态复位；
		RstCtrl();					//复位内存中本类控制的所有相关状态.

		DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: quit for guarantee at %s, grp=%d\n", 
							 TimeToStr(m_tmNow, cTime), m_iGrp));
		return true;
	}
*/
#ifdef PRO_698
	if ((m_tmNow.nDay!=tmCmd.nDay || m_tmNow.nMonth!=tmCmd.nMonth || m_tmNow.nYear!=tmCmd.nYear) ||
		(m_dwNow-m_CtrlCmd.dwTime>((DWORD )((DWORD)m_CtrlCmd.bCtrlTime*30*60)) && m_CtrlCmd.dwTime!=0) )	
#else
	if (m_tmNow.nDay!=tmCmd.nDay || m_tmNow.nMonth!=tmCmd.nMonth || m_tmNow.nYear!=tmCmd.nYear)	
#endif
	{//临时下浮控只是当天有效，改日后控制命令撤销，控制状态复位；
		RstSysCtrlStatus(m_iGrp);	//复位系统库当前总加组本类控制状态(可能包括轮次状态,投入标志等等).
		ClrSysCmd(m_iGrp);			//清除系统库本总加组本类控制命令.
		RstCtrl();					//复位内存中本类控制的所有相关状态.
		SetValidStatus(false);		//恢复退出状态. 
		ClrCmd();					//清除内存中本类控制的控制命令.
#ifdef PRO_698
		DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: quit for day chang at %s,tmCmd:%s grp=%d\n",
							 TimeToStr(m_tmNow, cTime), TimeToStr(tmCmd, cStr), m_iGrp));
#else
		DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: quit for day chang at %s, grp=%d\n",
							 TimeToStr(m_tmNow, cTime), m_iGrp));
#endif
		m_iGrp = -1;				//将当前总加组设为 -1,表示当前没有总加组投入.
		//***记录到系统库日志中.
		//***发出声光信号;
		return true;
	}

	if (!m_fCalLimitFinish)
	{//临时下浮控的功控定值还没有计算出来,将在滑差时段内以分钟为间隔对当前
	 //功率进行采样累加,并将最后得到的平均值做为临时下浮控的功率值上限.
		DWORD dwClick = GetClick();

		if (m_dwCalLimitStartClick == 0)
		{
			m_dwCalLimitStartClick = dwClick;				//获取当前的 Click.
			m_dwCalLimitTmpClick = m_dwCalLimitStartClick;
			m_iTmpCtrlLimit = 0;
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: start calculate limit, grp=%d\n", m_iGrp));
		}

		if (dwClick >= m_dwCalLimitTmpClick)
		{
			int64 iPwr = GetValidCurPwr(m_iGrp);
			if (iPwr >= 0)
			{
				m_iTmpCtrlLimit += iPwr;
				m_dwCalLimitTmpClick = dwClick + CTL_TMPCTRL_SLIP_INTERVAL;
				m_wCalLimitTimes++;
			}
		}

		if (m_wCalLimitTimes > (WORD)m_CtrlCmd.bWndTime) //m_wCalLimitTimes实际计算的是分钟数
		{
			m_iTmpCtrlLimit = m_iTmpCtrlLimit * (100+Fmt4ToVal64(&m_CtrlCmd.bQuotiety, 1)) / (100*(int64)m_wCalLimitTimes);
			//WriteItemVal64(BN0, (WORD)m_iGrp, 0x084f, &m_iTmpCtrlLimit);	//将计算得到的下浮控功率定值写入系统库,供显示系统使用.
			m_fCalLimitFinish = true;
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: finish calculate limit=%lld of grp%d !\n", m_iTmpCtrlLimit, m_iGrp));
		}
		if (!m_fCalLimitFinish)
			return true;
	}

	BYTE b = GetSysCtrlTurnsCfg(m_iGrp);

	if ((m_bTurnsStatus&~b) != 0)
	{//如非受控轮次发生跳闸的,应对这些轮次进行复位(置位允许合闸状态).
		m_bTurnsStatus &= b;
	}

	int64 iCurPwrLimit = GetPwrSafeLimit();
	//取当前功率定值与保安定值中的大者.
	if (iCurPwrLimit < m_iTmpCtrlLimit)
		iCurPwrLimit = m_iTmpCtrlLimit;

	if (m_iCurPwrLimit != iCurPwrLimit)
	{
		SetSysCurPwrLimit(m_iGrp, iCurPwrLimit);
		m_iCurPwrLimit = iCurPwrLimit;
	}

	int64 iCurPwr = GetCurPwr(m_iGrp);	// 获取当前功率
	if (iCurPwr < 0)
		return false;

	//CEN:这个功率不一定就是在功率下降后记录
	//下面处理控制生效后,功率下降到限定值以下的状况.
	if (m_dwOpenBreakTime!=0 && m_dwNow>=(m_dwOpenBreakTime+(DWORD)m_CtrlCmd.bDelayTime*60))
	{//如果跳闸时间不为0,则认为已经跳过闸,当前功率下降是跳闸的结果,应在规定的延时后记录下控后功率.
		//WriteItemVal64(BN0, (WORD)m_iGrp, 0x111f, &iCurPwr);
		memset(bTmpBuf, 0, sizeof(bTmpBuf));
		bTmpBuf[0] = DT_LONG64;
		OoInt64ToLong64(iCurPwr, bTmpBuf+1);
		WriteItemEx(BN0, (WORD)m_iGrp, 0x230b, bTmpBuf);//记录下当前功率下浮控控后总加有功功率冻结值
		m_dwOpenBreakTime = 0;
		DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: in %d minutes after control of grp%d, rec current power=%lld at %s\n",
			m_CtrlCmd.bDelayTime, m_iGrp, iCurPwr, TimeToStr(m_tmNow, cTime)));
	}
	
	int i = GetIdxOfMostRight1(b & ~GetTurnsStatus());		//获取相应总加组当前可跳闸的轮次号.

	if (i<0 && iCurPwr>m_iCurPwrLimit)									//检测是否有可跳闸.
	{
		m_dwAlrTime = 0;						//无闸可跳了,报警没有意义,禁止报警.
		m_dwGuaranteeAlrTime = 0;
		return true;
	}
	m_bWarnStatus = i+1;
#ifdef PRO_698
	DWORD dwPersistTime = (DWORD )m_CtrlCmd.bAlrTime[i + TURN_START_PN - 1] *60;	//获取相应轮次的功控报警持续时间.
#else
	DWORD dwPersistTime = GetPwrAlrPersistTime(i+TURN_START_PN);	//获取相应轮次的功控报警持续时间.
#endif

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
			if (m_CtrlType != CTL_PWR_TMP_ALLCLOSE)
			{
				m_dwAlrTime = 0;
				m_CtrlType = CTL_PWR_TMP_ALLCLOSE;
			}
			else
			{
				if (m_dwAlrTime == 0)
				m_dwAlrTime = m_dwNow;
				int iTurn= GetIdxOfMostLeft1(m_bTurnsStatus)-1;	//获取相应总加组当前可跳闸的轮次号.
				m_bWarnStatus = iTurn+1;
				dwPersistTime = (DWORD )m_CtrlCmd.bAlrTime[iTurn] *60;	//获取相应轮次的功控报警持续时间.
				//SaveDisp(dwPersistTime,m_dwAlrTime, iCurPwr);
				RestoreTurnStatus();			
			}

		}
		else
		{
			m_dwAlrTime = 0;	//解除报警.
			m_CtrlType = CTL_PWR_TMP;
		}
	}
	else //(iCurPwr > m_iCurPwrLimit)
	{//假如当前功率 > 当前功率上限,开始启动功率控制流程.
		m_fIfOverLimit = true;
		if (m_CtrlType != CTL_PWR_TMP)
		{
			m_CtrlType = CTL_PWR_TMP;
			m_dwAlrTime = 0;
			return true;
		}
		if (m_dwAlrTime > m_dwNow)	//变成未来的时间了,时间往前调回去了
			m_dwAlrTime = 0;

		if (m_dwAlrTime==0)
		{
			m_dwAlrTime = m_dwNow;
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: alarm start at %s, turn=%d grp=%d, current power is %lld, limit is %lld, persistent time is %ld seconds\n",
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
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: quit for guarantee at %s, grp=%d\n", 
								 TimeToStr(m_tmNow, cTime), m_iGrp));
			return true;
		}
		if (m_dwNow > m_dwAlrTime+dwPersistTime)
		{//假如'当前时间' > '临时下浮控报警启动时间'+'临时下浮控报警持续时间'
			m_bTurnsStatus	|= 0x01 << i;
			m_dwAlrTime		 = 0;							//该轮跳闸后,应该把跳闸报警关闭.
			m_wOpenTimes++;									//跳闸次数增加1.
			m_dwOpenBreakTime = m_dwNow;					//记录下跳闸时间.

			//保存跳闸记录中间数据.
			/*BYTE bRecBuf[4+1+2+2];

			memcpy(bRecBuf, &m_dwNow, 4);				//保存跳闸时间.
			bRecBuf[4] = (BYTE)m_iGrp;					//保存总加组.
			Val64ToFmt2(iCurPwr, bRecBuf+5, 2);			//保存跳闸时功率.
			Val64ToFmt2(m_iCurPwrLimit, bRecBuf+7, 2);	//保存跳闸时功率定值.
			WriteItemEx(BN0, (WORD)(i+TURN_START_PN), 0x0a01, bRecBuf);
			TrigerSaveBank(BN0, SECT_CTRL, 0);*/

			DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: turn%d of grp%d open break at %s\n", 
								 i+TURN_START_PN, m_iGrp, TimeToStr(m_tmNow, cTime)));
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
void CTmpCtrl::RstCtrl(void)
{
	m_bTurnsStatus	= 0x00;				//将轮次状态全部设为合闸.
	m_dwAlrTime		= 0;				//将报警时间(开始报警时间)清零.
	m_dwGuaranteeAlrTime = 0;
	m_iCurPwrLimit	= 0;				//将当前功控定值设为0.

	m_fCalLimitFinish		= false;	//当前总加组'临时下浮控'上限值计算完成状态.
	m_dwCalLimitStartClick	= 0;		//当前总加组'临时下浮控'上限值计算起始时间.
	m_dwCalLimitTmpClick	= 0;		//当前总加组'临时下浮控'上限值计算用临时时间变量.
	m_wCalLimitTimes		= 0;		//当前总加组'临时下浮控'上限值计算时,功率累加次数.
	m_iTmpCtrlLimit			= 0;		//当前总加组'临时下浮控'上限值(取决于当前下浮值和保安值).

	m_fIfOverLimit			= false;
	m_dwOpenBreakTime		= 0;
}

//描述: 获取某总加组的本类控制命令,并把命令放到 m_NewCmd 中.(注意: 对不同的类,m_NewCmd的结构是不同的)
//参数:@iGrp	要获取命令的总加组.
//返回: 如果获取成功且为有效命令 true,否则返回 false.
bool CTmpCtrl::GetSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[20];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8263, bCmd, &m_NewCmd.dwTime) <= 0)	//读取相应总加组的"临时下浮控命令".
	{
		DTRACE(DB_LOADCTRL, ("CTmpCtrl::GetSysCmd: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	m_NewCmd.bAct		= bCmd[0];
	m_NewCmd.bWndTime	= bCmd[4];
	m_NewCmd.bQuotiety	= bCmd[6];
	m_NewCmd.bDelayTime	= bCmd[8];
#ifdef PRO_698
	m_NewCmd.bCtrlTime = bCmd[10];
	m_NewCmd.bAlrTime[0] = bCmd[12];
	m_NewCmd.bAlrTime[1] = bCmd[14];
#endif

	if (m_NewCmd.bAct!=1 && m_NewCmd.bAct!=2)
		return false;

	char cTime[20];

	
	/*if (NewCmdTime() == 0)
	{ //命令接收到的时间比现在还晚，且控制类型是“临时下浮控”，则控制命令要撤销；
		ClrSysCmd(iGrp);	//删除本总加组的命令.
		if (iGrp == m_iGrp)
		{
			RstSysCtrlStatus(iGrp);	//复位系统库当前总加组本类控制状态(可能包括轮次状态,投入标志等等).
			RstCtrl();				//复位内存中本类控制的所有相关状态.
			ClrCmd();				//清除内存中本类控制的控制命令.
			m_iGrp = -1;			//将当前总加组设为 -1,表示当前没有总加组投入.
			DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: ctrl quit at %s, gpr=%d\n",
								 TimeToStr(m_tmNow, cTime), iGrp));
			return false;
		}
	}*/

	//旧的命令还在执行,证明没有过日,只是终端时间往前调了,系统库中的命令时间被清掉了
	//保留旧的命令,让控制继续执行,把命令时间更新为一个合理一点的时间
	if (m_NewCmd.dwTime == 0) //终端时间往前调了,系统库中的命令时间被清掉了
	{
		if (CurCmdTime()!=0 && iGrp==m_iGrp) //旧的命令已经被处理,只是系统库中的命令时间被清掉了
		{
			m_NewCmd.dwTime = GetCurTime(); //更新为一个合理一点的时间,方便以后不同命令时间的比较
			WriteItemEx(BN0, (WORD)iGrp, 0x8263, bCmd, m_NewCmd.dwTime);	//把相应总加组的"临时下浮控命令"写会数据库
		}
		//else 让命令还是保持投入,只是命令的时间被清掉了,但也不能给命令一个明确的时间
		//	   它可能在投入后还没来得及扫描就被清除了,也可能是个已经被控制清除的旧命令,
		// 	   只是突然掉电没保存住;
		// 	   那除非它是系统中唯一被投入的命令,否则其它命令的时间比它后,它不会被执行
	}

	return true;
}

//描述: 清除系统库本总加组本类控制命令.
//参数:@iGrp	要清除命令的总加组.
//返回: 如果清除成功返回 true,否则返回 false.
bool CTmpCtrl::ClrSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[20];
	memset( bCmd, 0, sizeof(bCmd) );

	WriteItemEx(BN0, (WORD)iGrp, 0x8263, bCmd);		//清除相应总加组的"临时下浮控命令".

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//触发保存.

	return true;
}

//描述: 设置系统库本类控制轮次输出状态.
//参数:@iGrp	当前控制的总加组.
//		@bTurnsStatus	轮次状态
//返回: 如果设置成功返回 true,否则返回 false.
bool CTmpCtrl::SetSysCtrlTurnsStatus(int iGrp, BYTE bTurnsStatus)
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
	WriteItemEx(BN0, iGrp, 0x8261, bBuf);

	return true;
}

//描述: 将本总加组系统库本类控制标志设为指定状态.
//参数:@iGrp			要设定的总加组.
//	   @fStatus			状态.
//返回: 如果设置成功则返回 true, 否则返回 false.
bool CTmpCtrl::SetSysCtrlFlg(int iGrp, bool fStatus)
{
	if (fStatus)
	{
		TGrpCurCtrlSta tGrpCurCtrlSta;
		memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

		if (!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))
		{
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::SetSysCtrlFlg: There is something wrong when call GetGrpCurCtrlSta() !\n"));
			return false;
		}
	
		tGrpCurCtrlSta.FloatRate = m_CtrlCmd.bQuotiety;

		if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))
		{
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::SetSysCtrlFlg: There is something wrong when call SetGrpCurCtrlSta() !\n"));
			return false;
		}
/*

		BYTE bBuf[1+1+1+8*8];	//最多8组数据.

		//!!!如果在别的线程中会写该ID,可能需要进行信号量保护.
		if (ReadItemEx(BN0, PN0, 0x105f, bBuf) <=0)	//读"终端当前控制状态".
		{
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::SetSysCtrlFlg: There is something wrong when call ReadItemEx() !\n"));
			return false;
		}
		memcpy(bBuf+3+(8*(iGrp-GRP_START_PN))+2, &m_CtrlCmd.bQuotiety, 1);

		WriteItemEx(BN0, PN0, 0x105f, bBuf);	//写"终端当前控制状态".
*/

		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x08, fStatus, PWR_CTL);//临时下浮控使用3位.
	}
	else
	{
		//int64 i = INVALID_VAL64;

		//WriteItemVal64(BN0, (WORD)m_iGrp, 0x084f, &i);	//将下浮控功率定值置为无效值.
		//WriteItemVal64(BN0, (WORD)m_iGrp, 0x111f, &i);	//将当前总加组的功率下浮控后总加有功功率冻结值置为无效值.

		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x08, fStatus, PWR_CTL);//临时下浮控使用3位.
	}
}
#ifdef PRO_698
//描述:是否合闸
//返回: 如果设置成功则返回 true, 否则返回 false.
bool CTmpCtrl::RestoreTurnStatus()
{
	BYTE bEnableClose = 0;
	DWORD dwPersistTime;
	char msg[100];
	//ReadItemEx(BN0, PN0,0x0a08, &bEnableClose);//是否允许合闸
	//if (bEnableClose) 
	{
		int iTurn= GetIdxOfMostLeft1(m_bTurnsStatus)-1;	//获取相应总加组当前可跳闸的轮次号.

		dwPersistTime = (DWORD )m_CtrlCmd.bAlrTime[iTurn] *60;

		if (m_dwNow > m_dwAlrTime+dwPersistTime)
		{
			m_bTurnsStatus &= ~(1<<(iTurn));
			m_dwAlrTime = 0;
		}
	}
	return true;
}
#endif
