/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：zjmeter.cpp
 * 摘    要：本文件完成浙江电能表的功能需求
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年5月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/


#ifndef METERAUTH_H
#define METERAUTH_H

#include "sysarch.h"

bool IsTimeEmpty(const TTime& time);

class CMeterAuth{
	
public:
	CMeterAuth();
	virtual ~CMeterAuth();
	
	bool Init(WORD wPrgMinute);
	bool Close();
	void DoOneSecond();
	bool IsInLabState() { return false;/*IsTimeEmpty(m_tmLabState)==false; */};
	bool IsInProgState() { return m_fProgState; };
	bool IsInHandClrDmd() { return m_fHandClrDmd; };
	bool IsProgramming() ;
	void MonitorKey();
	
private:
	int m_iHandle;
	bool  m_fProgState;	
	bool  m_fHandClrDmd;	//10秒手动清需量标志
	int  m_iHandClrDmdCnt;
	WORD m_wPrgMinute;
};
#endif //METERAUTH_H

