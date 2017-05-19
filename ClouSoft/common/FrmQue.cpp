/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FrmQue.cpp
 * 摘    要：本文件主要用来实现类CFrmQue(帧队列)
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年11月
 * 备    注：
 *********************************************************************************************************/
#include "stdafx.h"
#include "FrmQue.h"
#include "FaConst.h"

/////////////////////////////////////////////////////////////////////////
//CFrmQue

CFrmQue::CFrmQue()
{
	m_pwFrmBytes = NULL;
	m_pbFrms = NULL;
}

CFrmQue::~CFrmQue()
{
	if (m_pwFrmBytes != NULL)
		delete [] m_pwFrmBytes;
	
	if (m_pbFrms != NULL)
		delete [] m_pbFrms;
}

//描述:初始化
//参数:@wMaxFrms 最大能缓存的帧数量
//	   @wFrmSize 每帧最大能缓存的字节数
bool CFrmQue::Init(WORD wMaxFrms, WORD wFrmSize)
{
	m_wMaxFrms = wMaxFrms;		//最大能缓存的帧数量
	m_wFrmSize = wFrmSize;		//每帧最大能缓存的字节数
	
	m_pwFrmBytes = new WORD[m_wMaxFrms]; 		 //每个帧的实际字节数
    m_pbFrms = new BYTE[(DWORD )m_wFrmSize*m_wMaxFrms]; //用来存放一个个的帧
	
	if (m_pwFrmBytes==NULL || m_pbFrms==NULL)
	{
		//DTRACE(1, ("CFrmQue::Init : critical error : sys out of memory !!!!!!!!!!!\r\n"));
		return false;
	}
    
    m_wFirst = m_wLast = 0;
	m_wFrmNum = 0;
	
    m_hsemSpace = NewSemaphore(m_wMaxFrms, m_wMaxFrms); 
    m_hsemMail = NewSemaphore(0, m_wMaxFrms);
    m_hmtxQ = NewSemaphore(1);
    
    return true;
}

//描述：删除队列里的全部指针，并释放其所占用的空间
void CFrmQue::RemoveAll()
{
	while (Remove(NULL, 1) != 0);	//LINUX平台下0表示无限等待,所以必须用1
}

bool CFrmQue::Append(BYTE* pbFrm, WORD wLen, DWORD dwMilliseconds)
{
    if (WaitSemaphore(m_hsemSpace, dwMilliseconds) != SYS_ERR_OK)
	{                                 //先等待邮箱里有空间放邮件
	   	//DTRACE(DB_QUEUE, ("CFrmQue::Append(): no space to append Queue %p. \r\n", this));
      	return false;
    } 

    WaitSemaphore(m_hmtxQ);        //取得对邮箱的占用
	
	memcpy(&m_pbFrms[(DWORD )m_wFrmSize*m_wLast], pbFrm, wLen);
  	m_pwFrmBytes[m_wLast] = wLen;
  	
  	m_wLast++;
    if (m_wLast >= m_wMaxFrms)     //越过循环队列的边界
      	m_wLast = 0;
	
	m_wFrmNum++;
	
    SignalSemaphore(m_hsemMail);   //加一条新的消息了！
    SignalSemaphore(m_hmtxQ);      //释放对邮箱的占用

	return true;
}


WORD CFrmQue::Remove(BYTE* pbFrm, DWORD dwMilliseconds)
{
    if (WaitSemaphore(m_hsemMail, dwMilliseconds) != SYS_ERR_OK) //先等待邮箱里有邮件
    {
	  	//DTRACE(OP_DEBUG, ("CFrmQue::Remove(): no msg to remove from Queue %p. \r\n", this));
      	return 0;
	}

    WaitSemaphore(m_hmtxQ);            //取得对邮箱的占用
    
    WORD wLen = m_pwFrmBytes[m_wFirst];
	if (pbFrm != NULL)
		memcpy(pbFrm, &m_pbFrms[(DWORD )m_wFrmSize*m_wFirst], wLen); //返回消息
  
    m_wFirst++;                     //尾指针++，即使上面msg==NULL，++表示消费
    if (m_wFirst >= m_wMaxFrms)     //越过循环队列的边界
      	m_wFirst = 0;
  	
  	m_wFrmNum--;
  	
	SignalSemaphore(m_hsemSpace);  //有新的空间了！
    SignalSemaphore(m_hmtxQ);      //释放对邮箱的占用
    
	return wLen;
}


