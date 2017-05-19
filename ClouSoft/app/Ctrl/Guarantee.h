/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Guarantee.h
 * 摘    要：本文件主要实现CGuarantee类的定义
 * 当前版本：1.0
 * 作    者：张建德
 * 完成日期：2008年3月
 *********************************************************************************************************/
#ifndef GUARANTEE_H
#define GUARANTEE_H

#include "CtrlBase.h"

typedef struct
{
	BYTE	bAct;			//动作(0<无动作>, 1<保电投入>, 2<保电解除>)
	DWORD	dwPersistTime;	//持续时间(0.5小时数)
	DWORD	dwTime;			//接收命令时间(自2000年1月1日0时0分0秒到现在的秒数)
} TGuaranteeCmd;

//========================================= CGuarantee =============================================
class CGuarantee : public CCtrl
{
public:
	CGuarantee(void){}
	virtual ~CGuarantee(){}

	bool Init(void);
	bool DoCtrl(void);									//'保电控'运行.
	bool SetSysCtrlStatus(BYTE bStatus);	//设定系统库'保电控'状态.
protected:                          			
	void DoCmdScan(void);								//扫描系统库中的'保电控'命令.
	bool IsAutoGuaranteePeriod();								//是否自动保电时段
	void ClrCmd(void)									//清除内存中'保电控'的控制命令.
	{
		memset(&m_CtrlCmd, 0, sizeof(TGuaranteeCmd));
	}
	void RstCtrl(void);									//复位'保电控'所有状态.
	bool ClrSysCmd(void);								//清除系统库'保电控'命令.


	bool RstSysCtrlStatus(void)							//复位系统库'保电控'状态.
	{
		return SetSysCtrlStatus(QUIT_GUARANTEE);
	}

protected:
	TGuaranteeCmd		m_CtrlCmd;						//'保电控'命令.
	bool				m_fUnconnect;					//是否未连接(即连续无通讯是否超时).
};

#endif  //GUARANTEE_H
