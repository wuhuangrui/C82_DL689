/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ComDebug.h
 * 摘    要：本文件主要包含各系统下调试输出的公共接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年8月
 * 备    注：
 *********************************************************************************************************/
#ifndef DEBUG_H
#define DEBUG_H
#include "apptypedef.h"
#include "sysdebug.h"
#include "FaCfg.h"

bool SysInitDebug();
bool InitDebug();
bool IsDebugOn(BYTE bType);

WORD PrintBuf(BYTE* out, BYTE* in, WORD wInLen);
WORD PrintBuf(BYTE* out, WORD wOutLen, BYTE* in, WORD wInLen);
void TraceBuf(WORD wSwitch, char* szHeadStr, BYTE* p, WORD wLen);
void TraceFrm(char* pszHeader, BYTE* pbBuf, WORD wLen);
void EnableTrace(bool fEnable);
bool IsTraceEnable();

#endif //DEBUG_H
