/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Mem.h
 * 摘    要：本文件实现动态内存空间管理
 * 当前版本：1.0
 * 作    者：李锦仙
 * 完成日期：2016年10月
 * 备注：
 *********************************************************************************************************/
#ifndef MEM_H
#define MEM_H

#include "apptypedef.h"
#include "MeterStruct.h"

//分配动态内存空间
bool AllocMem(BYTE*pbGlobal, TAllocTab* pAllocTab,WORD wTabNum, BYTE bType, DWORD dwId,WORD wDataLen);
//释放动态内存空间
bool FreeMem(BYTE*pbGlobal, TAllocTab* pAllocTab, WORD wTabNum, BYTE bType, DWORD dwId);
//读取动态内存
int ReadMem(TAllocTab* pAllocTab, WORD wTabNum, BYTE* pbMem, BYTE bType, DWORD dwId, BYTE* pbData);
//写动态内存
int WriteMem(TAllocTab* pAllocTab, WORD wTabNum, BYTE* pbMem, BYTE bType, DWORD dwId, BYTE* pbData);

//描述：取得动态内存的有效数据长度
int GetMemLen(TAllocTab* pAllocTab, WORD wTabNum, BYTE bType, DWORD dwId);
//描述：把bType和dwId合并为：
//		高1字节为类型，低3字节为函数传递过来的dwId的低3字节
DWORD MemTypeIdToId(BYTE bType, DWORD dwId);
WORD SchIndexInAllocTab(BYTE *pbAllocTab, WORD wSect);

#endif //MEM_H
