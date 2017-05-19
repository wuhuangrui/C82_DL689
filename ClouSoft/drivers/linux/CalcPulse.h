/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CalcPulse.h
 * ժ    Ҫ�����ļ���Ҫʵ��ң����ͳ��
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2009��4�� 
*********************************************************************************************************/
#ifndef _CALCPULSE_H
#define _CALCPULSE_H
#include "apptypedef.h"
#define YM_NUM	4	//ң��1-4��ң�����ã�ң��5��Ϊ������ ң��6����

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
	DWORD m_dwLastVal[8];//���һ��ң�ű�λ��¼
	DWORD m_dwLastTicks[8];//���һ��ң�ű�λ��¼����ʱ��
};
#endif
