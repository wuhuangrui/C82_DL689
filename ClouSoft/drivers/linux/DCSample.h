/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：dcsample.h
 * 摘    要：本文件主要实现YX检测
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2009年5月 
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
	int m_iHandle1;//9260自带的AD设备
	int m_iHandle2;//ADT7411的AD设备，RN8209的AD设备
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

