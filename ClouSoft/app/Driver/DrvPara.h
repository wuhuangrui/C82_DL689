 /*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DrvPara.h
 * 摘    要：实现加载驱动参数，需要由用户传入参数的驱动的参数加载都放在这里实现
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2009年7月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/
#ifndef DRVPARA_H
#define DRVPARA_H
#include "DrvStruct.h"

#ifdef SYS_LINUX
	#ifndef BIG_LCD
		#define BIG_LCD				//大液晶屏的显示参考华北显示
	#endif
#endif

void InitLcd();
void InitYX();
void InitYK();
void YXLoadPara(TYxPara* pYxPara);
void TransferLidStatus();
void DcLoadAdjPara(BYTE* pbBuf);
bool SaveTrigerAdj(BYTE* pbBuf);
WORD GetTermCurrent(WORD wCtCn);
bool InitDrvPara();
extern WORD g_wLogFileAddr[512];
#endif

