/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：sysarch.h
 * 摘    要：本文件主要实现操作系统接口的封装,如信号量,线程等
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年7月
 *********************************************************************************************************/
#ifndef SYSARCH_H
#define SYSARCH_H

#include <strings.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pthread.h>
#include <semaphore.h>
#include "apptypedef.h"
#include "ComStruct.h"
#include "Trace.h"

#define INFINITE 	0x7ffffff0
#define NOWAIT      0  

#define SYS_ERR_OK       	0		//无错误
#define SYS_ERR_INVALID  	1       //设置内容非法
#define SYS_ERR_TIMEOUT  	0x20	//超时
#define SYS_ERR_SYS      	3		//系统出错

struct linux_sem {
  unsigned int c;
  unsigned int max;
  pthread_cond_t  cond;  /*  */
  pthread_mutex_t mutex;
};


typedef linux_sem*  TSem;

#ifndef TPrio
typedef unsigned int  TPrio;
#define THREAD_PRIORITY_IDLE			(0)
#define THREAD_PRIORITY_LOWEST			(1)
#define THREAD_PRIORITY_BELOW_NORMAL    (2)
#define THREAD_PRIORITY_NORMAL          (3)
#define THREAD_PRIORITY_ABOVE_NORMAL    (4)
#define THREAD_PRIORITY_HIGHEST         (5)
#define THREAD_PRIORITY_TIME_CRITICAL   (6)
#endif

#define THREAD_RET_OK	NULL
#define THREAD_RET_ERR	NULL
typedef void* TThreadRet;

TSem NewSemaphore(WORD count);
TSem NewSemaphore(WORD count, WORD max);
TSem NewPeriodicSemaphore(DWORD dwMilliseconds);
void FreeSemaphore(TSem sem);
void WaitSemaphore(TSem sem);
WORD WaitSemaphore(TSem sem, DWORD timeout);
void SignalSemaphore(TSem sem);
//void NewThread(void* (* function)(void *arg), void *arg, DWORD nStackSize);
void NewThread(TThreadRet (* function)(void *arg), void *arg, DWORD nStackSize=0, TPrio prio=THREAD_PRIORITY_NORMAL);

void Sleep(DWORD dwMilliseconds);

void InitArch();

#endif //SYSARCH_H 

