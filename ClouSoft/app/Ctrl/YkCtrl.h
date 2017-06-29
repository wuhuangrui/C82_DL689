/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：YkCtrl.h
 * 摘    要：本文件主要实现CYkCtrl类的定义
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年3月
 *********************************************************************************************************/
#ifndef YKCTRL_H
#define YKCTRL_H

#include "CtrlBase.h"

typedef struct
{
	BYTE 	bAct;			//动作(0<无动作>, 1<跳闸>, 2<允许合闸>)
	BYTE 	bAlrTime;		//报警延时(分钟数)
	DWORD 	dwPersistTime;	//限电时间(0.5小时数)
	DWORD 	dwTime;		//接收命令时间(自2000年1月1日0时0分0秒到现在的秒数)
} TYkCtrlCmd;

//============================================ CYkCtrl =================================================
class CYkCtrl : public CCtrl
{
public:                             			
	CYkCtrl(BYTE bTurn=TURN_START_PN);
	virtual ~CYkCtrl(){}

	bool SetTurn(int iTurn);							//设置'遥控'的当前轮次.
	bool DoCtrl(void);									//运行控制.
	void MakeDisp();

	bool IsCloseStatus()
	{
		return (!m_fAlrStatus && !m_fTurnStatus);
	}
	bool GetTurnStatus(void)							//获取轮次状态.
	{
		return m_fTurnStatus;
	}
	WORD GetOpenTimes(void)								//获取跳闸次数,然后清零跳闸次数.
	{
		WORD w = m_wOpenTimes;

		m_wOpenTimes = 0;

		return w;
	}
	bool IsBeepAlr() { return m_fAlrStatus; };		//是否声音报警.
	bool IsRxCloseCmd() { return m_fRxCloseCmd; }; 	//本轮执行是否收到了一个遥控合闸命令

protected:                          			
	void DoCmdScan(void);								//扫描系统库中的命令.
	void ClrCmd(void)									//清除内存中本类控制的控制命令.
	{
		memset(&m_CtrlCmd, 0, sizeof(TYkCtrlCmd));
	}
	void RstCtrl(void);									//复位内存中本类控制状态量.
	bool ClrSysCmd(int iTurn);							//清除系统库本轮次遥控命令.

	bool SetSysTurnStatus(int iTurn, bool fStatus);		//设定系统库指定总加组本控制类的相应轮次状态.
	bool SetSysTurnAlrStatus(int iTurn, bool fStatus);
	bool RstSysCtrlStatus(int iTurn)					//复位系统库指定轮次遥控状态(可能包括轮次状态,投入标志等等).
	{
		SetSysTurnAlrStatus(m_iTurn, false);
		return SetSysTurnStatus(iTurn, false);
	}
	void DoSaveOpenRec(void);							//保存遥控跳闸记录.

	//显示用， Added by Chenxi,7th,July
	void RemoveDispItem(TCtrl tInvCtrl);
	void AddDispItem(TCtrl tTopCtrl);
	void SaveDisp(DWORD dwCntDown);
	BYTE GetCtrlType() { return CTL_YkCtrl; };
	BYTE GetInvCtrlType() { return CTL_YkCtrl_CLOSE; };


protected:
	TYkCtrlCmd	m_CtrlCmd;						//当前轮次'遥控'命令.

	int			m_iTurn;						//当前轮次(轮次编号从0开始, 0 ~ TURN_NUM-1)
	bool		m_fTurnStatus;					//当前轮次'遥控'状态.
	bool		m_fAlrStatus;					//当前轮次轮次'遥控'报警状态.
	bool		m_fRxCloseCmd;					//本轮执行是否收到了一个遥控合闸命令

	bool        m_fAlarmStatus;                   //是否要显示遥控告警；
	bool        m_fCloseStatus;                  //是否要显示遥控合闸；
	bool        m_fOpenStatus;                   //是否要显示遥控跳闸；

	DWORD       m_dwOpenClick;                  //遥控合闸时时钟滴答；
	DWORD       m_dwCloseClick;                  //遥控合闸时时钟滴答；

	DWORD		m_dwFrzDly;						//功控跳闸后功率冻结延时.
	WORD		m_wOpenTimes;					//跳闸次数.
	DWORD		m_dwOpenClk;
	
};

#endif  //YKCTRL_H
