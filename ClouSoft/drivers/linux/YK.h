/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�YK.cpp
 * ժ    Ҫ�����ļ�ʵ����ң�ز���
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2009-04-14
************************************************************************************************************/
#ifndef YK_H
#define YK_H
#include "apptypedef.h"
#include "sysarch.h"
#include "DrvStruct.h"
#include "DrvConst.h"

class CYK
{
public:
	CYK(void);
	~CYK(void);
	bool Init(TYkPara* pYkPara, BYTE bYKNum);
	bool IsInit() { return m_fInit; };
	void Open(WORD wID);
	void Close(WORD wID);
	void Run(void);
	WORD GetYKValue(void) { return m_dwYKValue; };

private:
	int m_iHandle;
	BYTE m_bYKNum;
protected:
    TSem  m_semYK;
	bool  m_fInit;
    
	DWORD m_dwYKValue;	
};

/*
bool YKInit(TYkPara* pYkPara, BYTE bYKNum);
void YKOpen(WORD wID);
void YKClose(WORD wID);
void YKRun(void);
*/

#endif
