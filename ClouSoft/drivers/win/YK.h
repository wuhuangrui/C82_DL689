/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：YK.cpp
 * 摘    要：本文件实现了遥控操作
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2007-01-04
 * 备    注: 在硬件设计上BASE_ADR_YKCS1的对应位为高,BASE_ADR_YKCS2的对应位为低,
 *           则输出跳闸信号,如果硬件设计上刚好与本约定相反时,调整BASE_ADR_YKCS1和
 *			 BASE_ADR_YKCS2的定义,本程序不作修改
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
