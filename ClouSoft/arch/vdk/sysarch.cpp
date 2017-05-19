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
#ifdef _WIN32	// �����win32ϵͳ
 #include "stdafx.h"
#endif //_WIN32
 
#include "sysarch.h"

#include "ThreadManager.h"
#include "const.h"
#include "tGeneral.h"

/*-----------------------------------------------------------------------------------*/
TSem NewSemaphore(WORD count)
{
    //return CreateSemaphore(NULL, count, 1, NULL);
    return VDK::CreateSemaphore(count,1,0,0);  //0x7ffffff0
}

/*-----------------------------------------------------------------------------------*/
TSem NewSemaphore(WORD count, WORD max)
{
    //return CreateSemaphore(NULL, count, max, NULL);
    return VDK::CreateSemaphore(count,max,0,0);
    //???? max�п��ܻ�����쳣
}

/*
TSem NewPeriodicSemaphore(DWORD dwMilliseconds)
{
 //   TSem sem = CreateWaitableTimer(NULL, FALSE, NULL);  //���˹�����
 //    LARGE_INTEGER li;
 //   li.QuadPart = -(5 * 10000000);
 //   SetWaitableTimer(sem, &li, dwMilliseconds, NULL, NULL, FALSE);
//    return sem;
}
*/

/*-----------------------------------------------------------------------------------*/
void FreeSemaphore(TSem sem)
{
    //CloseHandle(sem);
    VDK::DestroySemaphore(sem);
}

/*-----------------------------------------------------------------------------------*/
void WaitSemaphore(TSem sem)
{
   // WaitForSingleObject(sem, INFINITE);
   VDK::PendSemaphore(sem,0x7ffffff0);
   VDK::SystemError Error;
   Error=VDK::GetLastThreadError();
   if (Error==VDK::kSemaphoreTimeout)
   {
     VDK::DispatchThreadError((VDK::SystemError)0,0);
   }   
}


/*-----------------------------------------------------------------------------------*/
WORD WaitSemaphore(TSem sem, DWORD timeout)
{
	DWORD dw;
    VDK::DispatchThreadError((VDK::SystemError)0,0);
  
    if (timeout == NOWAIT)
    	VDK::PendSemaphore(sem, 1);
    else
   		VDK::PendSemaphore(sem, timeout*TICKPERMS);
   	
   	VDK::SystemError Error;
   	Error=VDK::GetLastThreadError();
   	dw = DWORD(Error);
   	if (Error==VDK::kSemaphoreTimeout)
   	{
     	VDK::DispatchThreadError((VDK::SystemError)0,0);
   	}   
   	
  	if (dw == 0)
   	{   	
    	return SYS_ERR_OK;
   	}
  	else
  	{
     	return SYS_ERR_TIMEOUT;
  	}
}


/*-----------------------------------------------------------------------------------*/
void SignalSemaphore(TSem sem)
{
    //ReleaseSemaphore(sem, 1, NULL);
    VDK::PostSemaphore(sem);
}

void Sleep(DWORD dwMilliseconds)
{
	VDK::Sleep(dwMilliseconds*TICKPERMS);
}

 

/*-----------------------------------------------------------------------------------*/
void NewThread(int (* function)(void *arg), void *arg, DWORD dwStackSize, TPrio prio)
{
	TThreadData ThreadData;
	ThreadData.function = function;
	ThreadData.arg = arg;

  	VDK::ThreadCreationBlock inOutTCB;
  	inOutTCB.template_id = ktGeneral;  //a ThreadType defined in the vdk.h and vdk.cpp 
  	//inOutTCB.thread_id = 0;  //an output only field
  	inOutTCB.thread_stack_size = dwStackSize;
  	inOutTCB.thread_priority = prio;
  	inOutTCB.user_data_ptr = &ThreadData;
  	//inOutTCB.pTemplate = NULL;  //is a member used by VDK internally and does not need to be intilialised
  
  	VDK::CreateThreadEx(&inOutTCB);   //VDK::ThreadID id = 
}


void SysCleanup()
{
}
