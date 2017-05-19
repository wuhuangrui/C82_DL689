/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�zjmeter.cpp
 * ժ    Ҫ�����ļ�����㽭���ܱ�Ĺ�������
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��5��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
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
	bool  m_fHandClrDmd;	//10���ֶ���������־
	int  m_iHandClrDmdCnt;
	WORD m_wPrgMinute;
};
#endif //METERAUTH_H

