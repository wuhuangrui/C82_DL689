/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：YX.cpp
 * 摘    要：本文件主要实现YX检测
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年3月 
*********************************************************************************************************/
#ifndef YX_H
#define YX_H

#include "apptypedef.h"
#include "DrvStruct.h"
#include "DrvConst.h"

class CYX{
public:
	CYX();
	virtual ~CYX();
	bool Init(TYxPara* pYxPara, BYTE bYXNum);
	void Run(void);
	
private:
	int m_iHandle;
	TYxPara* m_pYxPara;
	WORD m_wLastYXInput;
	WORD m_wYxVal;
	WORD m_wYXWobble;
	BYTE m_bYXNum;
	
	WORD GetYxInput();
};

bool YXInit(TYxPara* pYxPara, BYTE bYXNum);
void YXRun(void);

#endif //YX_H