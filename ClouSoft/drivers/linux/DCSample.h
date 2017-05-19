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
	int m_iHandle1;//9260�Դ���AD�豸
	int m_iHandle2;//ADT7411��AD�豸��RN8209��AD�豸
	int m_iAdjPara[24];
	int m_iVal[24];
	int m_iAdjVal[24];
	TSem m_semDcProtect;
	
};

void DCInit();
void DCRun();
void DCClose();
bool DCTrigerAdj(BYTE* pbBuf);	

#endif

