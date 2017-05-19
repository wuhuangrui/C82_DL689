/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：UrgeFee.h
 * 摘    要：本文件主要实现CUrgeFee类的定义
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年3月

- 2008-04-17 10:25
疑问:
当前催费告警状态：
由D0表示终端当前是否处于催费告警状态，置“1”：终端处于催费告警状态，置“0”：终端未处于催费告警状态。

这个状态是如何确定：
是催费告警投入了就置位，还是在告警时段置位(如8时 ~ 20时),或者是正在报警的那1分钟内置位.
*********************************************************************************************************/
#ifndef URGEFEE_H
#define URGEFEE_H

#include "CtrlBase.h"

typedef struct
{
	BYTE	bAct;			//动作(0<无动作>, 1<催费告警投入>, 2<催费告警解除>)
	BYTE    bFlag[3];		//催费告警投入标志
	DWORD	dwTime;			//接收命令时间(自2000年1月1日0时0分0秒到现在的秒数)
} TUrgeFeeCmd;

//========================================= CUrgeFee =============================================
class CUrgeFee : public CCtrl
{
public:
	CUrgeFee(void);
	virtual ~CUrgeFee();

	bool DoCtrl(void);									//'催费告警'运行.
	bool IsBeepAlr(void)								//是否声音报警.
	{
		return m_fAlrStatus;
	}

protected:                          			
	void DoCmdScan(void);								//扫描系统库中的'催费告警'命令.
	void ClrCmd(void)									//清除内存中'催费告警'的控制命令.
	{
		memset(&m_CtrlCmd, 0, sizeof(TUrgeFeeCmd));
	}
	void RstCtrl(void);									//复位'催费告警'所有状态.
	bool ClrSysCmd(void);								//清除系统库'催费告警'命令.

	bool SetSysCtrlStatus(bool fStatus);				//设定系统库'催费告警'状态.
	bool RstSysCtrlStatus(void)							//复位系统库'催费告警'状态.
	{
		return SetSysCtrlStatus(false);
	}
	bool SetSysCurStatus(bool fStatus);

protected:
	TUrgeFeeCmd		m_CtrlCmd;							//'催费告警'命令.
	bool			m_fLaunch;
	DWORD			m_dwAlrStartTime;					//报警启动时间.
	bool			m_fAlrStatus;						//当前小时是否处于报警状态.
};

#endif  //URGEFEE_H
