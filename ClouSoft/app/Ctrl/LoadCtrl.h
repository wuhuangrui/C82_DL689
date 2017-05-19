/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DlmsClass.h
 * 摘    要：本文件主要实现LoadCtrl类的定义
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年2月
 *********************************************************************************************************/
//#define LOADCTRL_H
#ifndef LOADCTRL_H
#define LOADCTRL_H

#include "CtrlBase.h"
#include "Guarantee.h"
#include "UrgeFee.h"
#include "YkCtrl.h"
#include "MonthCtrl.h"
#include "BuyCtrl.h"
#include "TmpCtrl.h"
#include "ShutoutCtrl.h"
#include "RestCtrl.h"
#include "PeriodCtrl.h"

class CAllPwrCtrl : public CCtrlBase
{
public:                             			
	CAllPwrCtrl(void);
	~CAllPwrCtrl();

	bool Init(void);						//功控初始化.
	bool DoCtrl(void);						//功控执行.
	int GetGrp(void) { return m_iGrp; }; 	//获取当前总加组.
	int GetCurProGrp(void); //获取优先控制总加组
	BYTE GetTurnsStatus(void) { return m_bTurnsStatus; }; //获得轮次状态.
	bool SetSysTurnsStatus(int iGrp, BYTE bTurnsStatus);			//设定系统库指定总加组功控的所有轮次状态.
	void StatOverLimitPara(void);				//统计功控超限参数.
												 
	WORD GetOpenTimes(void)						//获取功控跳闸次数.
	{
		return (m_TmpCtrl.GetOpenTimes()
				+m_ShutoutCtrl.GetOpenTimes()
				+m_RestCtrl.GetOpenTimes()
				+m_PeriodCtrl.GetOpenTimes());
	}
	bool IsBeepAlr(void)						//是否声音报警.
	{
		return (m_TmpCtrl.IsBeepAlr()
			   || m_ShutoutCtrl.IsBeepAlr()
			   || m_RestCtrl.IsBeepAlr()
			   || m_PeriodCtrl.IsBeepAlr());
	}	
	bool IsValid(void)
	{
		return (m_TmpCtrl.IsValid()
				||m_ShutoutCtrl.IsValid()
				||m_RestCtrl.IsValid()
				||m_PeriodCtrl.IsValid());
	}

protected:
	bool SetSysPwrAlrFlgs(int iGrp, BYTE bFlgsStatus);				//设定系统库指定总加组功控的所有报警状态.
	bool ChgSysPwrAlrFlgs(int iGrp, BYTE bFlgs, bool fStatus);		//改变系统库指定总加组功控的相应报警状态.

	bool ChgSysTurnsStatus(int iGrp, BYTE bTurns, bool fStatus);	//改变系统库指定总加组功控的相应轮次状态.

	
protected:
	CTmpCtrl			m_TmpCtrl;
	CShutoutCtrl		m_ShutoutCtrl;
	CRestCtrl			m_RestCtrl;
	CPeriodCtrl			m_PeriodCtrl;

	int					m_iGrp;
	BYTE				m_bTurnsStatus;
	BYTE				m_bAlrsStatus;
};

class CLoadCtrl : public CCtrlBase
{
public:
	CLoadCtrl(void);
	~CLoadCtrl();

	bool Init(void);							//负控初始化.
	bool DoCtrl(void);							//负控执行.
	void InitSysCtrl();
	bool GetSoundBeepStats();
	bool GetEnableCtrlStatus(){ return m_fEnableBreakAct; };
	bool IsEnergyFee(void);
	void TrigerAlr(){m_fTrigerAlr = true;};
	void DisAlr(){m_fTrigerAlr = false;};
	int GetPwrCtrlGrp() { return m_AllPwrCtrl.GetCurProGrp(); }; 	//获取当前功控总加组
	int GetMonCtrlGrp() { return m_MonthCtrl.GetGrp(); }; 	//获取当前月电控总加组
	int GetBuyCtrlGrp() { return m_BuyCtrl.GetGrp(); }; 	//获取当前购电控总加组	

protected:
	DWORD			m_dwStarupTime;				//控制模块启动时间.

	CGuarantee		m_Guarantee;				//保电控对象.
	CUrgeFee		m_UrgeFee;					//催费告警对象.
	CYkCtrl			m_YkCtrl[TURN_NUM];			//各个轮次的遥控对象.

	CMonthCtrl		m_MonthCtrl;				//月电控对象.
	CBuyCtrl		m_BuyCtrl;					//购电控对象.
	CAllPwrCtrl		m_AllPwrCtrl;				//所有的工控对象.

	bool			m_fEnableBreakAct;
	BYTE			m_bTurnsStatus;				//
	bool			m_fBeepAlrStatus;

	BYTE	m_bYkClosedTurns[3];	//各控制被遥控合闸合掉的已经跳闸的轮次
	DWORD	m_dwWarnTime;
	bool	m_fTrigerAlr;					//触发告警状态.
};

extern CLoadCtrl g_LoadCtrl;

#endif  //LOADCTRL_H
