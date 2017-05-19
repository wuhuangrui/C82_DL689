/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�YK.cpp
 * ժ    Ҫ�����ļ�ʵ����ң�ز���
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2007-01-04
 * ��    ע: ��Ӳ�������BASE_ADR_YKCS1�Ķ�ӦλΪ��,BASE_ADR_YKCS2�Ķ�ӦλΪ��,
 *           �������բ�ź�,���Ӳ������ϸպ��뱾Լ���෴ʱ,����BASE_ADR_YKCS1��
 *			 BASE_ADR_YKCS2�Ķ���,���������޸�
************************************************************************************************************/
#ifndef YK_H
#define YK_H
#include "apptypedef.h"
#include "sysarch.h"
#include "DrvStruct.h"

class CYK
{
public:
	CYK(void);
	~CYK(void);
	void Init(TYkPara* pYkPara,BYTE bYKNum);
	bool IsInit() { return m_fInit; };
	void Open(WORD wID);
	void Close(WORD wID);
	void Run(void);
	WORD GetYKValue(void);
	void  SetCtrlLed(bool  fStatue,BYTE bLedType);
	void  SetAlarLed(bool  fStatue);
	void  SetBDLed(bool  fStatue);

protected:
	TYkPara* m_pYkPara;
    TSem  m_semYK;
	bool  m_fInit;
	BYTE  m_bYKNum;
    
	WORD m_wLastYKCS1;
	WORD m_wLastYKCS2;
	DWORD m_dwStartClick[8];
};

void YKInit(TYkPara* pYkPara,BYTE bYKNum);

#endif
