/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TmpCtrl.h
 * 摘    要：本文件主要实现CTmpCtrl类的定义
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年3月
 *********************************************************************************************************/
#ifndef TMPCTRL_H
#define TMPCTRL_H

#include "CtrlBase.h"

typedef struct
{
	BYTE bAct;		//动作(0<无动作>, 1<营业报停控投入>, 2<营业报停控解除>)
	BYTE bWndTime;	//滑差时间
	BYTE bQuotiety;	//下浮系数
	BYTE bDelayTime;//冻结延迟时间
//#ifdef PRO_698
	BYTE bCtrlTime;	//控制时间
	BYTE bAlrTime[TURN_NUM];	//轮次告警时间
//#endif
	DWORD dwTime;	//接收命令时间(自2000年1月1日0时0分0秒到现在的秒数)
} TTmpCtrlCmd;

//========================================== CTmpCtrl ==============================================
class CTmpCtrl : public CPwrCtrl
{
	friend class CAllPwrCtrl;
public:
	CTmpCtrl(void){}
	virtual ~CTmpCtrl(){}

	void DoSaveOpenRec(void)							//保存跳闸记录.
	{
		CPwrCtrl::DoSavePwrCtrlOpenRec(3);
	}

	BYTE GetCtrlType()
	{
		return m_CtrlType;
	}

	BYTE GetInvCtrlType()
	{
		return CTL_PWR_TMP_CLOSE;
	}

	bool DoCtrl(void);									//运行控制.

protected:
	void RstCtrl(void);									//复位内存中本类控制状态量.
	bool GetSysCmd(int iGrp);							//获取某总加组的本类控制命令,并把命令放到 m_NewCmd 中.(注意: 对不同的类,m_NewCmd的结构是不同的)
	bool ClrSysCmd(int iGrp);							//清除系统库本总加组本类控制命令.
	bool SetSysCtrlTurnsStatus(int iGrp, BYTE bTurnsStatus);

	void ClrCmd(void)									//清除内存中本类控制的控制命令.
	{
		memset(&m_CtrlCmd, 0, sizeof(m_CtrlCmd));
	}
	char* CtrlType(char* psz)							//获得控制的类型(返回控制类型的字符串描述).
	{
		sprintf(psz, "TmpCtrl");
		return psz;
	}
	int CtrlType(void)									//获得控制的类型(返回整数类型).
	{
		return m_CtrlType;//CTL_PWR_TMP;
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
	bool GetSysCtrlFlg(int iGrp)						//获取指定总加组的功控标志位.
	{
		return ((CGrpCtrl::GetSysCtrlFlgs(iGrp, 0)&0x08) != 0);	//临时下浮控使用3位.
	}
	bool SetSysCtrlFlg(int iGrp, bool fStatus);			//将本总加组系统库本类控制标志设为有效.
	bool RstSysCtrlStatus(int iGrp)						//复位系统库当前总加组本类控制状态(可能包括轮次状态,投入标志等等).
	{
		return SetSysCtrlFlg(iGrp, false);
	}
#ifdef PRO_698
	bool RestoreTurnStatus();
#endif
	

protected:
	TTmpCtrlCmd			m_NewCmd;						//调用GetSysCmd(int iGrp)函数后,读取到的命令将保存到该变量中.
	TTmpCtrlCmd			m_CtrlCmd;						//当前使用的命令.

	bool				m_fCalLimitFinish;				//当前总加组'临时下浮控'上限值计算完成状态.
	DWORD				m_dwCalLimitStartClick;			//当前总加组'临时下浮控'上限值计算起始时间.
	DWORD				m_dwCalLimitTmpClick;			//当前总加组'临时下浮控'上限值计算用临时时间变量.
	WORD				m_wCalLimitTimes;				//当前总加组'临时下浮控'上限值计算时,功率累加次数.
	int64				m_iTmpCtrlLimit;				//当前总加组'临时下浮控'上限值(取决于当前下浮值和保安值).

	DWORD				m_dwOpenBreakTime;				//跳闸时间.
	BYTE				m_CtrlType;
};

#endif  //TMPCTRL_H
