/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�dcsample.h
 * ժ    Ҫ�����ļ���Ҫʵ��YX���
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2009��5�� 
*********************************************************************************************************/
#ifndef DCSAMPLE_H
#define DCSAMPLE_H
#include "apptypedef.h"
#include "sysarch.h"

class CDCSample
{
public:
	CDCSample();
	virtual ~CDCSample();
	void Init();
	void Run();
	void Close();
	bool TrigerAdj(BYTE* pbBuf);	

private:
	WORD m_wAdjStep;
	int m_iHandle1;
	int m_iHandle2;
	int m_iAdjPara[12];
	int m_iVal[12];
	int m_iAdjVal[12];
	TSem m_semDcProtect;
	
};
#endif

