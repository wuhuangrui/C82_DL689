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
#include "stdafx.h"
#include "Mem.h"
#include "FaCfg.h"
#include "Trace.h"
#include "ComAPI.h"


//描述：分配动态内存空间
//参数：@pbGlobal 全局内存分配表
//		@pAllocTab 动态内存分配表
//		@wTabNum 分配表空间长度
//		@bType 动态内存类型定义
//		@dwId 该类型对应的具体ID等
//			任务记录:任务ID、
//			抄表异常:事件OI
//			终端内部事件:事件OI+分相属性
//			全事件采集：采集方案编号
//		@wDataLen 需要分配的内存长度
//返回：如果正确申请到空间，返回true
//      如果申请失败，会返回false
bool AllocMem(BYTE*pbGlobal, TAllocTab* pAllocTab, WORD wTabNum, BYTE bType, DWORD dwId, WORD wDataLen)
{
	char str[30];
	int iByte, iBit, iIndex;
	WORD wAllocSects = 0;	//已经分配的块数
	DWORD dwTmpId = MemTypeIdToId(bType, dwId);
	WORD wNeedBlk = (wDataLen+MEMORY_BLOCK_SIZE-1) / MEMORY_BLOCK_SIZE;

	for (iIndex=0; iIndex<wTabNum; iIndex++)
	{
		if (pAllocTab[iIndex].dwId == dwTmpId)
		{
			return true; //已经分配
		}
	}

	for (iIndex=0; iIndex<wTabNum; iIndex++)
	{
		if (pAllocTab[iIndex].dwId==0 && pAllocTab[iIndex].wDataLen==0)
		{
			break; //找到一个空表
		}
	}

	if (iIndex == wTabNum)
		return false; //没有分配表可用了

	//从全局分配表中搜集空间
	for (iByte=0; iByte<ALLOC_TAB_LEN; iByte++)
	{
		if (pbGlobal[iByte] == 0xff)
			continue;

		for (iBit=0; iBit<8; iBit++)
		{
			if (wAllocSects == wNeedBlk) //凑齐了足够的空间，记录并返回
				goto Malloc_end;	//有可能会是在最后一块内存才刚好分配完，因此完成申请分配的判断需要放到外面做

			if ((pbGlobal[iByte] & (1<<iBit)) == 0) //该内存未被使用，占用它
			{
				pAllocTab[iIndex].bAllocTab[iByte] |= (0x01<<iBit);
				wAllocSects++;
			}
		}	//for (bBit=0; bBit<8; bBit++)
	}	//for (iByte=0; iByte<ALLOC_TAB_LEN; iByte++)

Malloc_end:
	if (wAllocSects == wNeedBlk)
	{
		for (iByte=0; iByte<ALLOC_TAB_LEN; iByte++)
			pbGlobal[iByte] |= pAllocTab[iIndex].bAllocTab[iByte];//记录全局变量

		pAllocTab[iIndex].dwId = dwTmpId;
		pAllocTab[iIndex].wDataLen = wDataLen;
		DTRACE(DB_CRITICAL, ("AllocMem: bType=%s dwId=0x%x, Sucess\r\n", MemTypeToStr(bType, str), dwId));
		return true;
	}

	memset(pAllocTab[iIndex].bAllocTab, 0, ALLOC_TAB_LEN);
	DTRACE(DB_CRITICAL, ("AllocMem: bType=%s dwId=0x%x fail, Because buf is full\r\n", MemTypeToStr(bType, str), dwId));
	return false;
}

//描述：释放动态内存空间
//参数：@pbGlobal 全局内存分配表
//		@pAllocTab 动态内存分配表
//		@wTabNum 分配表空间长度
//		@bType 动态内存类型定义
//		@dwId 该类型对应的具体ID等
//			任务记录:任务ID、
//			抄表异常:事件OI
//			终端内部事件:事件OI+分相属性
//			全事件采集：采集方案编号
//返回：如果正确释放空间，返回true
//      如果释放失败，会返回false
bool FreeMem(BYTE*pbGlobal, TAllocTab* pAllocTab, WORD wTabNum, BYTE bType, DWORD dwId)
{
	DWORD dwTmpId = MemTypeIdToId(bType, dwId);

	for (int i=0; i<wTabNum; i++)
	{
		if (pAllocTab[i].dwId == dwTmpId)
		{
			for (int iByte=0; iByte<ALLOC_TAB_LEN; iByte++)
				pbGlobal[iByte] &= (~pAllocTab[i].bAllocTab[iByte]);//清除记录全局变量
			memset((BYTE *)&pAllocTab[i], 0, sizeof(TAllocTab));
			return true;
		}
	}

	return false;
}

//描述：读取动态内存数据内容
//参数：@pAllocTab 动态内存分配表
//		@wTabNum 分配表空间长度
//		@pbMem 动态内存指针
//		@bType 动态内存类型定义
//		@dwId 该类型对应的具体ID等
//			任务记录:任务ID、
//			抄表异常:事件OI
//			终端内部事件:事件OI+分相属性
//			全事件采集：采集方案编号
//		@pbData 接收的缓存区
//返回：如果读取正确，返回正确长度
//      如果读取错误，返回-1
int ReadMem(TAllocTab* pAllocTab, WORD wTabNum, BYTE* pbMem, BYTE bType, DWORD dwId, BYTE* pbData)
{
	WORD wIndex;
	DWORD dwTmpId = MemTypeIdToId(bType, dwId);

	for (int i=0; i<wTabNum; i++)
	{
		if (pAllocTab[i].dwId == dwTmpId)
		{
			WORD wAllocBlk = CalcuBitNum(pAllocTab[i].bAllocTab, ALLOC_TAB_LEN);
			for (WORD wSect=0; wSect<wAllocBlk; wSect++)
			{
				wIndex = SchIndexInAllocTab(pAllocTab[i].bAllocTab, wSect);
				if (wIndex != 0xffff)
				{
					if (wSect < wAllocBlk-1)
						memcpy(pbData+wSect*MEMORY_BLOCK_SIZE, &pbMem[wIndex*MEMORY_BLOCK_SIZE], MEMORY_BLOCK_SIZE);
					else
						memcpy(pbData+wSect*MEMORY_BLOCK_SIZE, &pbMem[wIndex*MEMORY_BLOCK_SIZE], pAllocTab[i].wDataLen - wSect*MEMORY_BLOCK_SIZE);
				}
				else
				{
					return -1;
				}
			}
			return pAllocTab[i].wDataLen;
		}
	}

	return -1;
}

//描述：向动态内存写入数据
//参数：@pAllocTab 动态内存分配表
//		@wTabNum 分配表空间长度
//		@pbMem 动态内存指针
//		@bType 动态内存类型定义
//		@dwId 该类型对应的具体ID等
//			任务记录:任务ID、
//			抄表异常:事件OI
//			终端内部事件:事件OI+分相属性
//			全事件采集：采集方案编号
//		@pbData 要写入的缓存区
//返回：如果写入正确，返回正确长度
//      如果写入错误，返回-1
int WriteMem(TAllocTab* pAllocTab, WORD wTabNum, BYTE* pbMem, BYTE bType, DWORD dwId, BYTE* pbData)
{
	WORD wIndex;
	DWORD dwTmpId = MemTypeIdToId(bType, dwId);

	for (int i=0; i<wTabNum; i++)
	{
		if (pAllocTab[i].dwId == dwTmpId)
		{
			WORD wAllocBlk = CalcuBitNum(pAllocTab[i].bAllocTab, ALLOC_TAB_LEN);
			for (WORD wSect=0; wSect<wAllocBlk; wSect++)
			{
				wIndex = SchIndexInAllocTab(pAllocTab[i].bAllocTab, wSect);
				if (wIndex != 0xffff)
				{
					if (wSect < wAllocBlk-1)
						memcpy(&pbMem[wIndex*MEMORY_BLOCK_SIZE], pbData+wSect*MEMORY_BLOCK_SIZE, MEMORY_BLOCK_SIZE);
					else
						memcpy(&pbMem[wIndex*MEMORY_BLOCK_SIZE], pbData+wSect*MEMORY_BLOCK_SIZE, pAllocTab[i].wDataLen - wSect*MEMORY_BLOCK_SIZE);
				}
				else
				{
					return -1;
				}
			}
			return pAllocTab[i].wDataLen;
		}
	}

	return -1;
}

//描述：取得动态内存的有效数据长度
//返回：正确则返回动态内存的有效数据长度，否则-1
int GetMemLen(TAllocTab* pAllocTab, WORD wTabNum, BYTE bType, DWORD dwId)
{
	DWORD dwTmpId = MemTypeIdToId(bType, dwId);

	for (int i=0; i<wTabNum; i++)
	{
		if (pAllocTab[i].dwId == dwTmpId)
		{
			return pAllocTab[i].wDataLen;
		}
	}

	return -1;
}

//描述：把bType和dwId合并为：
//		高1字节为类型，低3字节为函数传递过来的dwId的低3字节
DWORD MemTypeIdToId(BYTE bType, DWORD dwId)
{
	DWORD dwTmpId = ((bType<<24) | (dwId&0x00ffffff));
	return dwTmpId;
}

//描述：找到内存分配表pbAllocTab首个动态分配空间的扇区偏移
//参数：@pbAllocTab 内存分配表
//		@wSect 需要查找的第wSect个有效内存区
//返回：返回第wSect个动态分配内存的偏移号,第1个内存的偏移为0
WORD SchIndexInAllocTab(BYTE *pbAllocTab, WORD wSect)
{
	WORD wIdx = 0;	//内存区索引数
	WORD wByte;
	BYTE bBit;

	for (wByte=0; wByte<ALLOC_TAB_LEN; wByte++)
	{
		if (pbAllocTab[wByte] == 0)
			continue;

		for (bBit=0; bBit<8; bBit++)
		{
			if (pbAllocTab[wByte] & (0x01<<bBit))
			{
				wIdx++;		//内存区索引数加1
				if (wIdx == (wSect+1))
					return (wByte<<3)+bBit;	//找到就返回内存区偏移
			}
		}
	}

	return 0xffff;
}