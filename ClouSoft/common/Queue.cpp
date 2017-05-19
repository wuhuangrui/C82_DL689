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
        
	for (int i=0; i<m_nMaxMsgs; i++) //��ʼ����Ϣָ��
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


//������ɾ���������ȫ��ָ�룬���ͷ�����ռ�õĿռ�
void CQueue::RemoveAll()
{
	while (Remove(1) != NULL);	//LINUXƽ̨��0��ʾ���޵ȴ�,���Ա�����1
}

bool CQueue::Append(void* pvMsg, DWORD dwMilliseconds)
{
    if (WaitSemaphore(m_hsemSpace, dwMilliseconds) != SYS_ERR_OK)
	{	                                             //�ȵȴ��������пռ���ʼ�
	   	//DTRACE(DB_QUEUE, ("CQueue::Append(): no space to append Queue %p. \r\n", this));
      	return false;
    } 

    WaitSemaphore(m_hmtxQ);        //ȡ�ö������ռ��

    m_pvMsgs[m_nLast++] = pvMsg;   //������Ϣ���е�ͷ
  
    if (m_nLast == m_nMaxMsgs)     //Խ��ѭ�����еı߽�
	{
      	m_nLast = 0;
	}
	
	m_nMsgNum++;
	
    SignalSemaphore(m_hsemMail);   //��һ���µ���Ϣ�ˣ�
    SignalSemaphore(m_hmtxQ);      //�ͷŶ������ռ��

	return true;
}


void* CQueue::Remove(DWORD dwMilliseconds)
{
    if (WaitSemaphore(m_hsemMail, dwMilliseconds) != SYS_ERR_OK) //�ȵȴ����������ʼ�
    {
	  	//DTRACE(OP_DEBUG, ("CQueue::Remove(): no msg to remove from Queue %p. \r\n", this));
      	return NULL;
	}

    WaitSemaphore(m_hmtxQ);         //ȡ�ö������ռ��
    
	void* pvMsg;
    pvMsg = m_pvMsgs[m_nFirst];     //������Ϣ
	m_pvMsgs[m_nFirst] = NULL;
  
    m_nFirst++;                     //βָ��++����ʹ����msg==NULL��++��ʾ����
    if (m_nFirst == m_nMaxMsgs)     //Խ��ѭ�����еı߽�
	{
      	m_nFirst = 0;
	}    
  	
  	m_nMsgNum--;
  	
	SignalSemaphore(m_hsemSpace);  //���µĿռ��ˣ�
    SignalSemaphore(m_hmtxQ);      //�ͷŶ������ռ��
    
	return pvMsg;
}


