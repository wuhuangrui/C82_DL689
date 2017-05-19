/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TaskStruct.h
 * 摘    要：本文件主要用来定义任务数据结构
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2016年10月
 *********************************************************************************************************/
#ifndef TASKSTRUCT_H
#define TASKSTRUCT_H
#include "apptypedef.h"
#include "TaskConst.h"
#include "MeterStruct.h"

#define TERM_MEM_SIZE 	MTR_MEM_SIZE//8192		//终端动态内存大小
#define TERM_TAB_NUM		64			//终端动态内存分配表个数
typedef struct 
{
	BYTE  bGlobal[ALLOC_TAB_LEN];	//全局内存分配表，
	//相当于allocTab[]的或
	TAllocTab allocTab[TERM_TAB_NUM];//动态内存分配表
	BYTE bMem[TERM_MEM_SIZE];	//动态内存分配空间
}TTermMem; //终端动态内存控制结构

#endif //TASKSTRUCT_H