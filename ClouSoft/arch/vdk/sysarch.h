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

#include "vdk.h"
#include "apptypedef.h"
#include "ComStruct.h"
#include "sysapi.h"

#define INFINITE 	0x7ffffff0
#define NOWAIT      0  

#define SYS_ERR_OK  	 0x00
#define SYS_ERR_TIMEOUT  0x20

#ifndef TPrio
typedef VDK::Priority  TPrio;
#define THREAD_PRIORITY_IDLE			(VDK::kPriority8)
#define THREAD_PRIORITY_LOWEST			(VDK::kPriority7)
#define THREAD_PRIORITY_BELOW_NORMAL    (VDK::kPriority6)
#define THREAD_PRIORITY_NORMAL          (VDK::kPriority5)
#define THREAD_PRIORITY_ABOVE_NORMAL    (VDK::kPriority4)
#define THREAD_PRIORITY_HIGHEST         (VDK::kPriority3)
#define THREAD_PRIORITY_TIME_CRITICAL   (VDK::kPriority2)
#endif

#define THREAD_RET_OK	0
#define THREAD_RET_ERR	1
typedef int TThreadRet;

typedef VDK::SemaphoreID TSem;

TSem NewSemaphore(WORD count);
TSem NewSemaphore(WORD count, WORD max);
TSem NewPeriodicSemaphore(DWORD dwMilliseconds);
void FreeSemaphore(TSem sem);
void WaitSemaphore(TSem sem);
WORD WaitSemaphore(TSem sem, DWORD timeout);
void SignalSemaphore(TSem sem);
void NewThread(int (* function)(void *arg), void *arg, DWORD dwStackSize, TPrio prio);
void Sleep(DWORD dwMilliseconds);

inline void PushCriticalRegion()
{
	VDK::PushCriticalRegion();
}

inline void PopCriticalRegion()
{
	VDK::PopCriticalRegion();
}

inline void PushUnscheduledRegion()
{
    VDK::PushUnscheduledRegion();
}

inline void PopUnscheduledRegion()
{
    VDK::PopUnscheduledRegion();
}

	
#endif //SYSARCH_H 

