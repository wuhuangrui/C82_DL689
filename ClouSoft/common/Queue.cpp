// Queue.cpp: implementation of the CQueue class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Queue.h"
#include "FaConst.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*TSem NewSemaphore(WORD count);
TSem NewSemaphore(WORD count, WORD max);
TSem NewPeriodicSemaphore(DWORD dwMilliseconds);
void FreeSemaphore(TSem sem);
void WaitSemaphore(TSem sem);
WORD WaitSemaphore(TSem sem, DWORD timeout);
void SignalSemaphore(TSem sem);
*/

CQueue::CQueue()
{
}

CQueue::~CQueue()
{
	RemoveAll();
	delete m_pvMsgs;
}


bool CQueue::Init(int nMaxMsgs)
{
    m_nMaxMsgs = nMaxMsgs;
    m_pvMsgs = new void*[m_nMaxMsgs];

	if (m_pvMsgs == NULL)
	{
		//DTRACE(1, ("CQueue::Init : critical error : sys out of memory !!!!!!!!!!!\r\n"));
		return false;
	}
        
	for (int i=0; i<m_nMaxMsgs; i++) //初始化消息指针
	{
		m_pvMsgs[i] = NULL;
	}
    m_nFirst = m_nLast = 0;
	m_nMsgNum = 0;
	
    m_hsemSpace = NewSemaphore(m_nMaxMsgs, m_nMaxMsgs); 
    m_hsemMail = NewSemaphore(0, m_nMaxMsgs);
    m_hmtxQ = NewSemaphore(1);
    
    return true;
}


//描述：删除队列里的全部指针，并释放其所占用的空间
void CQueue::RemoveAll()
{
	while (Remove(1) != NULL);	//LINUX平台下0表示无限等待,所以必须用1
}

bool CQueue::Append(void* pvMsg, DWORD dwMilliseconds)
{
    if (WaitSemaphore(m_hsemSpace, dwMilliseconds) != SYS_ERR_OK)
	{	                                             //先等待邮箱里有空间放邮件
	   	//DTRACE(DB_QUEUE, ("CQueue::Append(): no space to append Queue %p. \r\n", this));
      	return false;
    } 

    WaitSemaphore(m_hmtxQ);        //取得对邮箱的占用

    m_pvMsgs[m_nLast++] = pvMsg;   //挂在消息队列的头
  
    if (m_nLast == m_nMaxMsgs)     //越过循环队列的边界
	{
      	m_nLast = 0;
	}
	
	m_nMsgNum++;
	
    SignalSemaphore(m_hsemMail);   //加一条新的消息了！
    SignalSemaphore(m_hmtxQ);      //释放对邮箱的占用

	return true;
}


void* CQueue::Remove(DWORD dwMilliseconds)
{
    if (WaitSemaphore(m_hsemMail, dwMilliseconds) != SYS_ERR_OK) //先等待邮箱里有邮件
    {
	  	//DTRACE(OP_DEBUG, ("CQueue::Remove(): no msg to remove from Queue %p. \r\n", this));
      	return NULL;
	}

    WaitSemaphore(m_hmtxQ);         //取得对邮箱的占用
    
	void* pvMsg;
    pvMsg = m_pvMsgs[m_nFirst];     //返回消息
	m_pvMsgs[m_nFirst] = NULL;
  
    m_nFirst++;                     //尾指针++，即使上面msg==NULL，++表示消费
    if (m_nFirst == m_nMaxMsgs)     //越过循环队列的边界
	{
      	m_nFirst = 0;
	}    
  	
  	m_nMsgNum--;
  	
	SignalSemaphore(m_hsemSpace);  //有新的空间了！
    SignalSemaphore(m_hmtxQ);      //释放对邮箱的占用
    
	return pvMsg;
}


