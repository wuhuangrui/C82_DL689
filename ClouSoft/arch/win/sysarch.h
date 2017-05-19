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
#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__
#include "apptypedef.h"
#include "ComStruct.h"
#include "sysdebug.h"
#include "sysapi.h"

#define SYS_ERR_OK       	0		//�޴���
#define SYS_ERR_INVALID  	1       //�������ݷǷ�
#define SYS_ERR_TIMEOUT  	0x20	//��ʱ
#define SYS_ERR_SYS      	3		//ϵͳ����

typedef HANDLE TSem;

#define THREAD_RET_OK	0
#define THREAD_RET_ERR	1
typedef int TThreadRet;

typedef int TPrio;
#define SENDTIMEOUT                                    -0xfe//���ͳ�ʱ

TSem NewSemaphore(WORD count);
TSem NewSemaphore(WORD count, WORD max);
TSem NewPeriodicSemaphore(DWORD dwMilliseconds);
void FreeSemaphore(TSem sem);
void WaitSemaphore(TSem sem);
WORD WaitSemaphore(TSem sem, DWORD timeout);
void SignalSemaphore(TSem sem);
void NewThread(TThreadRet (* function)(void *arg), void *arg, DWORD nStackSize=0, TPrio prio=THREAD_PRIORITY_NORMAL);
//void Sleep(DWORD dwMilliseconds);

inline void PushUnscheduledRegion()
{
}

inline void PopUnscheduledRegion()
{
}

inline void InitArch() {  };

#endif //__ARCH_SYS_ARCH_H__ 

