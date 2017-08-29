/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MonthCtrl.cpp
 * 摘    要：本文件主要实现CMonthCtrl的类
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
#include "TaskManager.h"
#include "MonthCtrl.h"
#include "DbOIAPI.h"

//========================================= CMonthCtrl =============================================
CMonthCtrl::CMonthCtrl(void)
{
	memset(&m_OLStat, 0, sizeof(m_OLStat));
	m_OLStat.iGrp = -1;
    m_iCurMonthEng = 0;	
    m_iCurEngLimit = 0;	
    m_iCurAlarmLimit = 0;
	m_fAlrStauts = false;			//将报警状态取消.
	m_dwAlrStartTime = 0;
}

//描述: 初始化.
//返回: 如果初始化正常返回 true,否则返回 false.
bool CMonthCtrl::Init(void)
{
	ClrCmd();		//清除内存中本类控制的控制命令.
	RstCtrl();		//复位内存中本类控制状态量.
	SetValidStatus(false);	//设定控制退出状态.

	return true; //return GetSysStatus();
}

//描述: '月电控'控制.
//返回: 正常则返回 true,否则返回 false.
bool CMonthCtrl::DoCtrl(void)
{
	DoCmdScan();		//扫描系统库中的命令.
	BeepAlrCtrl();

	if (!IsValid())		//检测是否投入控制.
	{
		RstCtrl();
		return true;
	}
/*
	if (IsGuarantee())
	{//如'处在保电状态'.
		RstCtrl();					//复位内存中本类控制的所有相关状态.
		return true;
	}
*/
	TTime tmCmd;
	SecondsToTime(m_CtrlCmd.dwTime, &tmCmd);

	if (m_tmNow.nMonth!=m_tmOldTime.nMonth || m_tmNow.nYear!=m_tmOldTime.nYear)
	{//如发生了月切换(检测到现在的时间和接受到命令的时间月不同),'月电控'复位
		RstCtrl();					//复位内存中本类控制的所有相关状态.
		DTRACE(DB_LOADCTRL, ("CMonthCtrl::DoCtrl: grp%d reset for month change\n", m_iGrp));
		return true;
	}

	int i;
	BYTE b = GetSysCtrlTurnsCfg(m_iGrp);

	if ((m_bTurnsStatus&~b) != 0)
	{//如非受控轮次发生跳闸的,应对这些轮次进行复位(置位允许合闸状态).
		m_bTurnsStatus &= b;
	}
	//检测是否有可跳闸.
	i = GetIdxOfMostRight1(b & ~GetTurnsStatus());	//获取相应总加组当前可跳闸的轮次号.
	m_bWarnStatus = i+1;
	char cTime[20];
	m_iCurMonthEng = GetMonthEng(m_iGrp);	//获取当前总加组本月已用电量.	目前面向对象程序里没有做总加组的本月已用电量 --QLS
	m_iCurEngLimit = GetMonthLimit(m_iGrp);	//获取当前总加组本月月控定值.
	m_iCurAlarmFactor = GetMonthAlarmFactor(m_iGrp);	//获取当前总加组本月月控定值.
	m_iCurAlarmLimit = m_iCurEngLimit * m_iCurAlarmFactor / 100;	//目前在华北负控中没有找到报警值的定义,暂时按照江苏负控中将 80% 作为月电量的报警值.

	if (m_iCurEngLimit<0 || m_iCurEngLimit<0)
		return true;

	if (m_iCurMonthEng<m_iCurAlarmLimit)
	{//电量没到报警线.
		RstCtrl();					//复位内存中本类控制的所有相关状态.
	}
	else if (m_iCurMonthEng < m_iCurEngLimit)
	{//如果月电量已超报警线,但未到跳闸线.
		SubRstCtrl();				//复位内存中本类控制的部分相关状态.
		if (i >= 0)
		{//还有可跳闸.
			if (!m_fAlrStauts)
			{
				SetSysCtrlAlr(m_iGrp, true);
#ifdef PRO_698
						//保存跳闸记录.
				/*BYTE bBuf[20];

				bBuf[0] = (BYTE)m_iGrp;					//总加组
				bBuf[1] = GetSysCtrlTurnsCfg(m_iGrp);		//轮次
				bBuf[2] = 0x01;							//电控类别
				Val64ToFmt3(m_iCurMonthEng, bBuf+3, 4);//告警时电能量
				Val64ToFmt3(m_iCurEngLimit, bBuf+7, 4);//告警时电能量定值*/
				//记录当前告警记录到系统库中.
				//SaveAlrData(ERC_ENGALARM, m_tmNow, bBuf);
#endif
				DTRACE(DB_LOADCTRL, ("CMonthCtrl::DoCtrl: alarm start at %s, grp=%d, current-energy=%lld, alarm-limit=%lld, act-Limit=%lld\n",
									 TimeToStr(m_tmNow, cTime), m_iGrp, m_iCurMonthEng, m_iCurAlarmLimit, m_iCurEngLimit));
			}
			m_fAlrStauts = true;
			//SaveDisp();
		}
		else
		{//无闸可跳.
			if (m_fAlrStauts)
				SetSysCtrlAlr(m_iGrp, false);
			m_fAlrStauts = false;
		}
	}
	else if (IsGuarantee())
	{//如'处在保电状态'.
		RstCtrl();					//复位内存中本类控制的所有相关状态.
		m_fIfOverLimit = true;
		return true;
	}
	else	//(m_iCurMonthEng >= m_iCurEngLimit)
	{//如果月电量已超限.
		m_fIfOverLimit = true;

		if (i < 0)									//检测是否有可跳闸.
		{
			if (m_fAlrStauts)
				SetSysCtrlAlr(m_iGrp, false);
			m_fAlrStauts = false;					//无闸可跳了,报警没有意义,禁止报警.
			return true;
		}

		if (m_dwOpenTurnTime > m_dwNow)	//变成未来的时间了,时间往前调回去了
			m_dwOpenTurnTime = 0;

		if (!m_fAlrStauts)
			SetSysCtrlAlr(m_iGrp, true);
		m_fAlrStauts = true;
		DWORD dwTurnInv = GetEngTurnInv(i+TURN_START_PN);	//获取相应轮次的功控报警持续时间.		
		if (m_dwOpenTurnTime == 0)					//本月从来没跳过闸,则直接跳最小轮次的闸,同时记录下本次跳闸时间.
			m_dwOpenTurnTime = m_dwNow;
		else if (m_dwNow < m_dwOpenTurnTime+dwTurnInv)		//上次跳闸后,必须隔60秒才能再跳下一轮闸
			return true;
		else
			m_dwOpenTurnTime = m_dwNow;				//距离上次跳闸已超过60秒,可以再次跳闸,同时记录下本次跳闸时间.

		m_bTurnsStatus |= 0x01 << i;
		//SaveDisp();
		m_wOpenTimes++;								//跳闸次数增加1.

		/*//保存跳闸记录.
		BYTE bBuf[1+1+1+4+4];

		bBuf[0] = (BYTE)m_iGrp;					//总加组
		bBuf[1] = (BYTE)(0x01<<i);				//轮次
		bBuf[2] = 0x01;							//电控类别
		Val64ToFmt3(m_iCurMonthEng, bBuf+3, 4);//跳闸时电能量
		Val64ToFmt3(m_iCurEngLimit, bBuf+7, 4);//跳闸时电能量定值
		//记录当前跳闸记录到系统库中.
		//SaveAlrData(ERC_ENGCTL, m_tmNow, bBuf);
		
		DTRACE(DB_LOADCTRL, ("CMonthCtrl::DoCtrl: turn=%d open at %s, grp=%d, current-energy=%lld, act-Limit is %lld\n",
							 i+TURN_START_PN, TimeToStr(m_tmNow, cTime), m_iGrp, m_iCurMonthEng, m_iCurEngLimit));*/
		//***发出声光信号;
	}

	return true;
}

//描述：生成月电控越限时的显示参数；
void  CMonthCtrl::SaveDisp()
{
	BYTE bBuf[13];
	bBuf[0] = m_bTurnsStatus;
	Val64ToFmt(m_iCurMonthEng, bBuf+1, FMT3, 4);
	Val64ToFmt(m_iCurAlarmLimit, bBuf+5, FMT3, 4);
	Val64ToFmt(m_iCurEngLimit, bBuf+9, FMT3, 4);
	WriteItemEx(BN0, PN0, 0x0930, bBuf);
}

void CMonthCtrl::SubRstCtrl(void)
{
	m_bTurnsStatus	 = 0x00;			//将轮次状态全部设为合闸.
	m_dwOpenTurnTime = 0;				//上次跳闸时间设为0;
	m_fIfOverLimit	 = false;			//超限状态设为未超限.
}

//描述: 复位内存中本类控制状态量.
void CMonthCtrl::RstCtrl(void)
{
	if (m_fAlrStauts)
	{	
		SetSysCtrlAlr(m_iGrp, false);
		m_fAlrStauts = false;			//将报警状态取消.
	}

	m_dwAlrStartTime = 0;
	SubRstCtrl();
}

//描述: 获取某总加组的本类控制命令,并把命令放到 m_NewCmd 中.(注意: 对不同的类,m_NewCmd的结构是不同的)
//参数:@iGrp	要获取命令的总加组.
//返回: 如果获取成功且为有效命令 true,否则返回 false.
bool CMonthCtrl::GetSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[1];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8283, bCmd, &m_NewCmd.dwTime) <=0)	//读取相应总加组的"月电控命令".
	{
		DTRACE(DB_LOADCTRL, ("CMonthCtrl::GetSysCmd: There is something wrong when call ReadItemEx() !\n"));
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
			WriteItemEx(BN0, (WORD)iGrp, 0x8283, bCmd, m_NewCmd.dwTime);	//把相应总加组的"时段控命令"写会数据库
		}
		//else 让命令还是保持投入,只是命令的时间被清掉了,但也不能给命令一个明确的时间
		//	   它可能在投入后还没来得及扫描就被清除了,也可能是个已经被控制清除的旧命令,
		// 	   只是突然掉电没保存住;
		// 	   那除非它是系统中唯一被投入的命令,否则其它命令的时间比它后,它不会被执行
	}

	return true;
}

//描述: 清除系统库本总加组本类控制命令.
//参数:@bGrp		要清除命令的总加组.
//返回: 如果清除成功返回 true,否则返回 false.
bool CMonthCtrl::ClrSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[1] = {0};

	WriteItemEx(BN0, (WORD)iGrp, 0x8283, bCmd);	//写相应轮次的"月电控投入投入命令"ID

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//触发保存.

	return true;
}

//描述: 设置系统库本类控制轮次输出状态.
//参数:@iGrp	当前控制的总加组.
//		@bTurnsStatus	轮次状态
//返回: 如果设置成功返回 true,否则返回 false.
bool CMonthCtrl::SetSysCtrlTurnsStatus(int iGrp, BYTE bTurnsStatus)
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
	WriteItemEx(BN0, iGrp, 0x8281, bBuf);

	return true;
}

//描述: 获取指定总加组本月用电限额.
//参数:@iGrp	要获取的总加组.
//返回: 成功则返回本月电能量限额,否则返回int64类型最大的整数.
int64 CMonthCtrl::GetMonthLimit(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return -1;

	BYTE bBuf[18];
	ReadItemEx(BN0, (WORD)iGrp, 0x8108, bBuf);	//从相应的总加组相应的PN读"月电量控定值"ID
	if (IsAllAByte(&bBuf[6], INVALID_DATA, 8) || IsAllAByte(&bBuf[6], 0, 8))
		return -1;

	int iAlrFactor = bBuf[17];
	return (OoLong64ToInt64(&bBuf[6]) * (100+iAlrFactor) / 100);	//进行浮动调整.
}


//描述: 获取指定总加组本月用电报警系数.
//参数:@iGrp	要获取的总加组.
//返回: 成功则返回本月电能量报警系数,否则返回int64类型最大的整数.
int64 CMonthCtrl::GetMonthAlarmFactor(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return -1;

	BYTE bBuf[18];
	if(ReadItemEx(BN0, (WORD)iGrp, 0x8108, bBuf)>0)	//从相应的总加组相应的PN读"月电量控定值"ID
	{
		if(bBuf[16] == 0)
			return 80;
		else
			return abs((int)bBuf[16]);
	}
	return 80;	//默认80%.
}
//描述: 获取指定总加组本月用电限额报警浮动系数.
//参数:@iGrp	要获取的总加组.
//返回: 浮动系数.
int CMonthCtrl::GetAlrFltQuotiety(void)
{
	BYTE bBuf[1];

	ReadItemEx(BN0, PN0, 0x014f, bBuf);			//从PN0读"终端月电能量控定值浮动系数"ID
	
	return Fmt4ToVal(bBuf, 1);
}

//描述: 月电控蜂鸣器报警控制.
void CMonthCtrl::BeepAlrCtrl(void)
{
	char cTime[20];

	if (m_fAlrStauts)
	{
		BYTE bBuf[3] = {0xff, 0xff, 0xff};
		ReadItemEx(BN10, PN0, 0xa121, bBuf); //0xa121 3 月电控告警时间,D0~D23分别对应0点~23点

		if (GetBitStatus(bBuf, sizeof(bBuf), m_tmNow.nHour))
		{	
			if (m_dwAlrStartTime == 0)
			{
				m_dwAlrStartTime = m_dwNow;
				DTRACE(DB_LOADCTRL, ("CMonthCtrl::BeepAlrCtrl: beep alarm start at %s\n", TimeToStr(m_tmNow, cTime)));
			}
		}
		else
		{
			if (m_dwAlrStartTime != 0)
				DTRACE(DB_LOADCTRL, ("CMonthCtrl::BeepAlrCtrl: beep alarm stop at %s, grp=%d\n", TimeToStr(m_tmNow, cTime), m_iGrp));

			m_dwAlrStartTime = 0;
		}
	}
	else
	{
		if (m_dwAlrStartTime != 0)
			DTRACE(DB_LOADCTRL, ("CMonthCtrl::BeepAlrCtrl: beep alarm stop at %s, grp=%d\n", TimeToStr(m_tmNow, cTime), m_iGrp));

		m_dwAlrStartTime = 0;
	}
}

//描述: 获取指定总加组本月已用正向有功总电能.
//参数:@iGrp	要获取的总加组.
//返回: 本月已用正向有功电能量.
int64 CMonthCtrl::GetMonthEng(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return -1;

	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;
	BYTE bBuf[50] = {0}; 
	int64 i64Value = 0;

	if (OoReadAttr(0x2300+(WORD)iGrp, 0x09, bBuf, &pbFmt, &wFmtLen) < 0)
	{
		DTRACE(DB_LOADCTRL, ("GetMonthEng: fail to get month energy!\n"));
		return -1;
	}

	i64Value = OoLong64ToInt64(&bBuf[3]);
	
	return i64Value;
}

//描述: 统计月电量超限参数(超限时间及超限电量).
void CMonthCtrl::StatOverLimitPara(void)
{
	char cTime[20];
	BYTE bBuf[2+4];

	if (m_tmOldTime.nMonth!=m_tmNow.nMonth || m_tmOldTime.nYear!=m_tmNow.nYear)
	{//假如发生了月切换.
		//将本月月电量超限参数复制到上月月电量超限参数.
		for (int i=GRP_START_PN; i<(GRP_START_PN+GRP_NUM); i++)
		{
			if (ReadItemEx(BN18, (WORD)i, 0x02df, bBuf) <=0)
			{
				DTRACE(DB_LOADCTRL, ("CMonthCtrl::StatOverLimitPara: There is something wrong when call ReadItemEx() !\n"));
				return;
			}
			WriteItemEx(BN0, (WORD)i, 0x327f, bBuf, 0, NULL, m_dwNow);
			DTRACE(DB_LOADCTRL, ("CMonthCtrl::StatOverLimitPara: Save the %04d-%02d Group[%d] statistical parameter of MonthCtrl energy over limit!\n"\
								 "Over time = %ld minutes\n"\
								 "Over limit energy = %lld KWH\n"\
								 "at %s\n", m_tmOldTime.nYear, m_tmOldTime.nMonth, i,
								 ByteToWord(bBuf), Fmt3ToVal64(bBuf+2, 4), TimeToStr(m_tmNow, cTime)));
		}
		//初始化本月控制统计数据.
		memset(bBuf, 0, sizeof(bBuf));	//清空之前的记录.
		for (int i=GRP_START_PN; i<(GRP_START_PN+GRP_NUM); i++)
			WriteItemEx(BN18, (WORD)i, 0x02df, bBuf);
	}

	int iOldGrp = m_OLStat.iGrp;

	int64 iEng = 0;
	DWORD dwTime = 0;//统计超限时间.
	int64 iTmpEnergy;//当前总加组的正向有功电能量，在刚上电时一分钟后能获得。

	if (m_OLStat.fIfOverLimit)
	{//假如之前的状态是超限状态.
		DWORD dwClick = GetClick();

		if (!m_fIfOverLimit
			|| m_OLStat.iGrp!=m_iGrp
			|| (dwClick - m_OLStat.dwClick)>60)	//忽略时,日,月,年的检测,提高效率且不会影响精度.
		{//假如超限状态或分钟或总加组发生了变化,则需累加统计数据.
			dwTime = dwClick - m_OLStat.dwClick;//统计超限时间.
			iTmpEnergy = GetCurEng(m_OLStat.iGrp);
			iEng = iTmpEnergy - m_OLStat.iEng;	//统计超限月电量.

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
				m_OLStat.fIfOverLimit = true;
				m_OLStat.iGrp = m_iGrp;
				m_OLStat.dwClick = GetClick();
				m_OLStat.iEng = iTmpEnergy;
			}
			else
				m_OLStat.fIfOverLimit = false;
		}
	}

	if (dwTime!=0 || iEng!=0)
	{
		if (ReadItemEx(BN18, (WORD)iOldGrp, 0x02df, bBuf) <=0)
		{
			DTRACE(DB_LOADCTRL, ("CMonthCtrl::StatOverLimitPara: There is something wrong when call ReadItemEx() !\n"));
			return;
		}

		WORD w;

		w = ByteToWord(bBuf) + (WORD)(dwTime/60);	//增加时间.
		WordToByte(w, bBuf);
		iEng += Fmt3ToVal64(bBuf+2, 4);				//增加超限电量.
		Val64ToFmt3(iEng, bBuf+2, 4);
		WriteItemEx(BN18, (WORD)iOldGrp, 0x02df, bBuf);
#if 1
		DTRACE(DB_LOADCTRL, ("CMonthCtrl::StatOverLimitPara: Refresh Group[%d] statistical parameter of MonthCtrl energy over limit!\n"\
							 "Over time = %ld minutes\n"\
							 "Over limit energy = %lld KWH\n"\
							 "at %s\n", iOldGrp, w, iEng, TimeToStr(m_tmNow, cTime)));
#endif
	}
}
