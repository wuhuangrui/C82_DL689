/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：BuyCtrl.h
 * 摘    要：本文件主要实现CBuyCtrl类的定义
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年3月
 *********************************************************************************************************/
#ifndef BUYCTRL_H
#define BUYCTRL_H

#include "CtrlBase.h"

typedef struct
{
	BYTE bAct;			//动作(0<无动作>, 1<购电控投入>, 2<购电控解除>)
	DWORD dwTime;		//接收命令时间(自2000年1月1日0时0分0秒到现在的秒数)
} TBuyCtrlCmd;

typedef struct
{
	DWORD dwBillIdx;	//订单号
	BYTE bFlag;			//购电标志
	int64 iBuyEng;		//购电量
	int64 iAlarmLimit;	//剩余电量报警限
	int64 iActLimit;	//剩余电量跳闸限
} TBuyCtrlPara;

//========================================== CBuyCtrl ==============================================
class CBuyCtrl : public CEngCtrl
{
public:
	CBuyCtrl(void);
	virtual ~CBuyCtrl(){}

	bool Init(void);
	bool DoCtrl(void);									//运行控制.
	bool IsBeepAlr(void)								//是否声音报警.
	{
		return m_fAlrStauts;
	}

	BYTE GetCtrlType()
	{
		return CTL_ENG_BUY;
	}

	BYTE GetInvCtrlType()
	{
		return CTL_ENG_BUY_CLOSE;
	}

	bool SetSysTurnsStatus(int iGrp, BYTE bTurnsStatus)			//设定系统库指定总加组本控制类的轮次状态.
	{
		return CEngCtrl::SetSysEngTurnsStatus(iGrp, bTurnsStatus, 1);
	}
	bool IsEnergyFee(void)								//是否购电费控制
	{
		return m_fEnergyFeeFlag;
	}

protected:
	void SubRstCtrl(void);
	void RstCtrl(void);									//复位内存中本类控制状态量.
	bool GetSysCmd(int iGrp);							//获取某总加组的本类控制命令,并把命令放到 m_NewCmd 中.(注意: 对不同的类,m_NewCmd的结构是不同的)
	bool ClrSysCmd(int iGrp);							//清除系统库本总加组本类控制命令.
	bool SetSysCtrlTurnsStatus(int iGrp, BYTE bTurnsStatus);

	bool GetSysStatus(void)								//用于初始化时,将系统库中本类控制的轮次状态,报警状态等同步到内存中对应的变量.
	{
		return GetSysEngStatus(1);	//购电控为1
	}
	void ClrCmd(void)									//清除内存中本类控制的控制命令.
	{
		memset(&m_CtrlCmd, 0, sizeof(m_CtrlCmd));
	}
	char* CtrlType(char* psz)							//获得控制的类型(返回控制类型的字符串描述).
	{
		sprintf(psz, "BuyCtrl");
		return psz;
	}
	int CtrlType(void)									//获得控制的类型(返回整数类型).
	{
		return CTL_ENG_BUY;
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
		return ((CGrpCtrl::GetSysCtrlFlgs(iGrp, 1)&0x02) != 0);	//购电控使用1位.
	}
	bool SetSysCtrlFlg(int iGrp, bool fStatus)			//将本总加组系统库本类控制标志设为有效.
	{
		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x02, fStatus, ENG_CTL); //购电控使用1位.
	}
	bool GetSysCtrlAlr(int iGrp)						//获取指定总加组的购电控报警标志位.
	{
		return ((CEngCtrl::GetSysEngAlrFlgs(iGrp)&0x02) != 0);	//购电控使用1位.
	}
	bool SetSysCtrlAlr(int iGrp, bool fStatus)			//设置系统库指定总加组的购电控报警标志位.
	{
		return CEngCtrl::ChgSysEngAlrFlgs(iGrp, 0x02, fStatus);//购电控使用1位.
	}
	bool ChgSysTurnsStatus(int iGrp, BYTE bTurns, bool fStatus)	//改变系统库指定总加组本控制类的相应轮次状态.
	{
		return CEngCtrl::ChgSysEngTurnsStatus(iGrp, bTurns, fStatus, 1);
	}

	void UpdateSysRemainEng(void);					//更新系统库所有总加组剩余购电量.
	void UpdateBuyRemainEng(void);					//更新系统库所有总加组剩余购电量.	
	bool GetBuyCtrlPara(int iGrp, TBuyCtrlPara& rPara);//获取当前总加组'购电控'参数.
	bool GetCurFeeRatio(int64 *iCurFeeRatio);

	void SaveDisp();
	int64 GetGroupEng(int iGrp);


protected:
	TBuyCtrlCmd			m_NewCmd;						//调用GetSysCmd(int iGrp)函数后,读取到的命令将保存到该变量中.
	TBuyCtrlCmd			m_CtrlCmd;						//当前使用的命令.

	int64				m_iBuyRemain;					//剩余购电量.
	TBuyCtrlPara		m_BuyCtrlPara;					//'购电控'参数.

	//int64				m_iBaseBuyRemainEng[GRP_NUM];	//各个总加组购电剩余电量初始值.
	int64				m_iCurBuyRemainEng[GRP_NUM+1];	//各个总加组购电剩余电量当前值.
	int64				m_iBaseEng[GRP_NUM+1];			//各个总加组上电时的基本用电量.
	DWORD				m_dwBillIdx[GRP_NUM+1];			//各个总加组已处理的购电单号.
	bool				m_fUpBaseEng[GRP_NUM+1];			//各个总加组上一分钟电量是否更新成功标志.
	BYTE 				m_bCount;
	bool				m_fEnergyFeeFlag;					//购电费控标志
	int64				m_iCurFeeRatio;						//当前费率
};

#endif  //BUYCTRL_H
