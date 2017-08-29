/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MonthCtrl.h
 * 摘    要：本文件主要实现CMonthCtrl类的定义
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年3月
 *********************************************************************************************************/
#ifndef MONTHCTRL_H
#define MONTHCTRL_H

#include "CtrlBase.h"

typedef struct
{
	BYTE bAct;		//动作(0<无动作>, 1<月控投入>, 2<月控解除>)
	DWORD dwTime;	//接收命令时间(自2000年1月1日0时0分0秒到现在的秒数)
} TMonthCtrlCmd;

//========================================= CMonthCtrl =============================================
class CMonthCtrl : public CEngCtrl
{
public:
	CMonthCtrl(void);
	virtual ~CMonthCtrl(){}

	bool Init(void);
	bool DoCtrl(void);									//运行控制.
	BYTE GetCtrlType()
	{
		return CTL_ENG_MONTH;
	}

	BYTE GetInvCtrlType()
	{
		return CTL_ENG_MONTH_CLOSE;
	}

	bool IsBeepAlr(void)								//是否声音报警.
	{
		return (m_dwAlrStartTime != 0);
	}

	bool SetSysTurnsStatus(int iGrp, BYTE bTurnsStatus)	//设定系统库指定总加组本控制类的轮次状态.
	{
		return CEngCtrl::SetSysEngTurnsStatus(iGrp, bTurnsStatus, 0);
	}

	void StatOverLimitPara(void);						//统计超限参数.

protected:
	void SubRstCtrl(void);
	void RstCtrl(void);									//复位内存中本类控制状态量.
	bool GetSysCmd(int iGrp);							//获取某总加组的本类控制命令,并把命令放到 m_NewCmd 中.(注意: 对不同的类,m_NewCmd的结构是不同的)
	bool ClrSysCmd(int iGrp);							//清除系统库本总加组本类控制命令.
	bool SetSysCtrlTurnsStatus(int iGrp, BYTE bTurnsStatus);

	bool GetSysStatus(void)								//用于初始化时,将系统库中本类控制的轮次状态,报警状态等同步到内存中对应的变量.
	{
		return GetSysEngStatus(0);	//月电控为0
	}
	void ClrCmd(void)									//清除内存中本类控制的控制命令.
	{
		memset(&m_CtrlCmd, 0, sizeof(m_CtrlCmd));
	}
	char* CtrlType(char* psz)							//获得控制的类型(返回控制类型的字符串描述).
	{
		sprintf(psz, "MonthCtrl");
		return psz;
	}
	int CtrlType(void)									//获得控制的类型(返回整数类型).
	{
		return CTL_ENG_MONTH;
	}
	int NewCmdAct(void)									//获取新命令的动作码.
	{
		return m_NewCmd.bAct;
	}
	DWORD NewCmdTime(void)								//获取新命令的接收时间.
	{
		return m_NewCmd.dwTime;
	}
	int CurCmdAct(void)									//获取当前命令的动作码.
	{
		return m_CtrlCmd.bAct;
	}
	DWORD CurCmdTime(void)								//获取当前命令的接收时间.
	{
		return m_CtrlCmd.dwTime;
	}
	void SaveNewCmd(void)								//保存最新命令.
	{
		m_CtrlCmd = m_NewCmd;
	}
	bool GetSysCtrlFlg(int iGrp)						//获取系统库指定总加组的月电控标志位.
	{
		return ((CGrpCtrl::GetSysCtrlFlgs(iGrp, 1)&0x01) != 0);	//月电控使用0位.
	}
	bool SetSysCtrlFlg(int iGrp, bool fStatus)			//设置系统库指定总加组的月电控标志位.
	{
		if (fStatus)
			m_fLaunch = true;
		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x01, fStatus, ENG_CTL);//月电控使用0位.
	}
	bool GetSysCtrlAlr(int iGrp)						//获取指定总加组的月电控报警标志位.
	{
		return ((CEngCtrl::GetSysEngAlrFlgs(iGrp)&0x01) != 0);	//月电控使用0位.
	}
	bool SetSysCtrlAlr(int iGrp, bool fStatus)			//设置系统库指定总加组的月电控报警标志位.
	{
		return CEngCtrl::ChgSysEngAlrFlgs(iGrp, 0x01, fStatus);//月电控使用0位.
	}
	bool ChgSysTurnsStatus(int iGrp, BYTE bTurns, bool fStatus)	//改变系统库指定总加组本控制类的相应轮次状态.
	{
		return CEngCtrl::ChgSysEngTurnsStatus(iGrp, bTurns, fStatus, 0);
	}

	int64 GetMonthEng(int iGrp);
	int64 GetMonthLimit(int iGrp);						//获取指定总加组本月用电限额.
	int64 GetMonthAlarmFactor(int iGrp);                //获取指定总加组本月用电报警系数
	int GetAlrFltQuotiety(void);						//获取指定总加组本月用电限额报警浮动系数.

	void BeepAlrCtrl(void);								//月电控蜂鸣器报警控制.

	void SaveDisp();

protected:
	TMonthCtrlCmd		m_NewCmd;						//调用GetSysCmd(int iGrp)函数后,读取到的命令将保存到该变量中.
	TMonthCtrlCmd		m_CtrlCmd;						//当前使用的命令.
	bool				m_fLaunch;
	DWORD				m_dwAlrStartTime;				//声音报警起始时间.

	TEngOverLimitStat	m_OLStat;						//超限记录.

	int64 m_iCurMonthEng;	                          //获取当前总加组本月已用电量.
	int64 m_iCurEngLimit;	                         //获取当前总加组本月月控定值.
	int64   m_iCurAlarmFactor;                         //获取当前总加组月电控告警系数.
	int64 m_iCurAlarmLimit;							 //获取当前总加组月电控告警定值.
};

#endif  //MONTHCTRL_H
