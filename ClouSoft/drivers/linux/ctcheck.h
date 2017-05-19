/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ctcheck.h
 * 摘    要：本文件主要实现CT检测
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2009年4月 
*********************************************************************************************************/
#ifndef CHECKCT_H
#define CHECHCT_H
#include "syscfg.h"
#include "bios.h"
#include "DbAPI.h"
#include "Sample.h"
#include "filter2.h"

#define CT_SAMPLE_NUM	240

class CCtcheck
{
public:
 	void Init();
 	void Run();
 	void LoadPara();
 	void Select(unsigned short chn);
private:
	WORD m_wCtCn;
    WORD m_wCtCalcuCnt;
	fract16 m_fInput[CT_SAMPLE_NUM];
    TDataItem m_diB62X[3];	
	DWORD m_dwCT2OpenVal;
	DWORD m_dwCT1ShortVal;
	DWORD m_dwCT2ShortVal;
	WORD m_wCtResult[4];
};

extern CCtcheck g_CTcheck;

void CTInit();
void CTRun();

#endif


