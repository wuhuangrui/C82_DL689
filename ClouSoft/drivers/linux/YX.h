/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�YX.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��YX���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��3�� 
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