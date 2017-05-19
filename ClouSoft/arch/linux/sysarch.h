/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�sysarch.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֲ���ϵͳ�ӿڵķ�װ,���ź���,�̵߳�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��7��
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

#define SYS_ERR_OK       	0		//�޴���
#define SYS_ERR_INVALID  	1       //�������ݷǷ�
#define SYS_ERR_TIMEOUT  	0x20	//��ʱ
#define SYS_ERR_SYS      	3		//ϵͳ����

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

