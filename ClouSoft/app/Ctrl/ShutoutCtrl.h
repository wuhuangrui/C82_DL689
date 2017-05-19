/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ShutoutCtrl.h
 * 摘    要：本文件主要实现CShutoutCtrl类的定义
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年3月
 *********************************************************************************************************/
#ifndef SHUTOUTCTRL_H
#define SHUTOUTCTRL_H

#include "CtrlBase.h"

typedef struct
{
	BYTE bAct;		//动作(0<无动作>, 1<营业报停控投入>, 2<营业报停控解除>)
	DWORD dwTime;	//接收命令时间(自2000年1月1日0时0分0秒到现在的秒数)
} TShutoutCtrlCmd;

typedef struct
{
	DWORD dwStartTime;	//'营业报停控'起始时间
	DWORD dwEndTime;	//'营业报停控'结束时间
	int64 iPwrLimit;	//'营业报停控'功率定值
} TShutoutCtrlPara;

//========================================== CShutoutCtrl ==============================================
class CShutoutCtrl : public CPwrCtrl
{
	friend class CAllPwrCtrl;
public:
	CShutoutCtrl(void){}
	virtual ~CShutoutCtrl(){}

	void DoSaveOpenRec(void)							//保存跳闸记录.
	{
		CPwrCtrl::DoSavePwrCtrlOpenRec(2);
	}

	BYTE GetCtrlType()
	{
		return m_CtrlType;
	}

	BYTE GetInvCtrlType()
	{
		return CTL_PWR_SHUTOUT_CLOSE;
	}

	bool DoCtrl(void);									//运行控制.

protected:
	void RstCtrl(void);									//复位内存中本类控制状态量.
	bool GetSysCmd(int iGrp);							//获取某总加组的本类控制命令,并把命令放到 m_NewCmd 中.(注意: 对不同的类,m_NewCmd的结构是不同的)
	bool ClrSysCmd(int iGrp);							//清除系统库本总加组本类控制命令.

	void ClrCmd(void)									//清除内存中本类控制的控制命令.
	{
		memset(&m_CtrlCmd, 0, sizeof(m_CtrlCmd));
	}
	char* CtrlType(char* psz)							//获得控制的类型(返回控制类型的字符串描述).
	{
		sprintf(psz, "ShutoutCtrl");
		return psz;
	}
	int CtrlType(void)									//获得控制的类型(返回整数类型).
	{
		return m_CtrlType;//CTL_PWR_SHUTOUT;
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
		return ((CGrpCtrl::GetSysCtrlFlgs(iGrp, 0)&0x04) != 0);	//营业报停控使用2位.
	}
	bool SetSysCtrlFlg(int iGrp, bool fStatus)			//将本总加组系统库本类控制标志设为有效.
	{
		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x04, fStatus, PWR_CTL);//营业报停控使用2位.
	}

	//---------------------------------------------------------------
	bool GetShutoutCtrlPara(int iGrp, TShutoutCtrlPara& rPara);	//获取指定总加组'营业报停控'参数.

protected:
	TShutoutCtrlCmd		m_NewCmd;						//调用GetSysCmd(int iGrp)函数后,读取到的命令将保存到该变量中.
	TShutoutCtrlCmd		m_CtrlCmd;						//当前使用的命令.
	BYTE 				m_CtrlType;
};

#endif  //SHUTOUTCTRL_H
