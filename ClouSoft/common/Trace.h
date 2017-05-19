/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ComDebug.h
 * ժ    Ҫ�����ļ���Ҫ������ϵͳ�µ�������Ĺ����ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��8��
 * ��    ע��
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
