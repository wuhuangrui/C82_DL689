/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：CalcPulse.h
 * 摘    要：本文件主要实现遥脉的统计
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2009年4月 
*********************************************************************************************************/
#ifndef _CALCPULSE_H
#define _CALCPULSE_H
#include "apptypedef.h"
#define YM_NUM	4	//遥信1-4和遥脉公用，遥信5作为计量门 遥信6备用

class CCalcPulse
{
public:
	CCalcPulse();
	virtual ~CCalcPulse();
	bool Init(WORD wYMNum);
	bool Run();
	bool Close();

private:
	bool m_fInit;
	int m_iHandle;
	BYTE m_bLastPtr;
	WORD m_wYMNum;
	BYTE m_bPulseNum;
	DWORD m_dwLastVal[8];//最近一次遥信变位记录
	DWORD m_dwLastTicks[8];//最近一次遥信变位记录发生时刻
};
#endif
