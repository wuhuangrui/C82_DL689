/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：YK.cpp
 * 摘    要：本文件实现了遥控操作
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2009-04-14
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
