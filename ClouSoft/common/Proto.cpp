/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Proto.cpp
 * 摘    要：本文件实现了与主站通信的协议基类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：
 *********************************************************************************************************/

#include "stdafx.h"
#include "string.h"
#include "Proto.h"
#include "Trace.h"
#include "FaCfg.h"
#include "sysapi.h"
#include "ComAPI.h"

////////////////////////////////////////////////////////////////////////////////////////////
//CProto

CProto::CProto()
{
	m_pIf = NULL;	
}

CProto::~CProto()
{

}

bool CProto::Init(TProPara* pProPara)
{
	m_pProPara = pProPara;
		
	m_fConnected = false;	//协议是否已经连接上
	
	m_nRxStep = 0;
	m_wRxPtr = 0;
	m_fUseLoopBuf = false;
	m_wRxHead = 0;
	m_wRxTail = 0;	
	
	m_dwRcvClick = 0;
	return true;
}

				
//描述:在接口建立连接后回调本函数,协议做相应的初始化
void CProto::OnConnectOK()
{
	m_nRxStep = 0;
	m_wRxHead = m_wRxTail = 0;
}

WORD CProto::GetMaxFrmBytes() 
{	
	return m_pIf==NULL ? 256 : m_pIf->GetMaxFrmBytes(); 
}

WORD CProto::GetLoopBufLen()
{
	if (m_wRxHead >= m_wRxTail)
		return m_wRxHead - m_wRxTail;
	else
		return m_wRxHead + LOOP_BUF_SIZE - m_wRxTail;
}

//描述:把串口接收到的数据放到接收循环缓冲区中去
void CProto::PutToLoopBuf(BYTE* pbBuf, WORD wLen)
{
	WORD WBuffLen=0;
	if (wLen > LOOP_BUF_SIZE)
	{
		DTRACE(DB_FAPROTO, ("CProto::PutToLoopBuf over,wLen>LOOP_BUF_SIZE.\n"));
		return;
	}
	WBuffLen=(m_wRxHead+LOOP_BUF_SIZE-m_wRxTail)%LOOP_BUF_SIZE;
	if (wLen+WBuffLen > LOOP_BUF_SIZE)
	{
		DTRACE(DB_FAPROTO, ("CProto::PutToLoopBuf over,(wLen+WBuffLen)>LOOP_BUF_SIZE.\n"));
		return;
	}		

	if (m_wRxHead+wLen <= LOOP_BUF_SIZE)
	{
		memcpy(&m_bLoopBuf[m_wRxHead], pbBuf, wLen);
	}
	else
	{
		memcpy(&m_bLoopBuf[m_wRxHead], pbBuf, LOOP_BUF_SIZE-m_wRxHead);
		memcpy(m_bLoopBuf, pbBuf+LOOP_BUF_SIZE-m_wRxHead, wLen-(LOOP_BUF_SIZE-m_wRxHead));
	}

	m_wRxHead += wLen;
	if (m_wRxHead >= LOOP_BUF_SIZE)
		m_wRxHead -= LOOP_BUF_SIZE;
}


WORD CProto::RxFromLoopBuf(BYTE* pbRxBuf, WORD wBufSize)
{
	WORD wRetLen;
	if (m_wRxHead != m_wRxTail)   //现在认为接收缓冲区大于等于循环缓冲区
	{
		if (m_wRxHead > m_wRxTail)
		{
			wRetLen = m_wRxHead - m_wRxTail;
			if (wRetLen > wBufSize)
				wRetLen = wBufSize;

			memcpy(pbRxBuf, &m_bLoopBuf[m_wRxTail], wRetLen);
			return wRetLen;
		}
		else
		{
			//这里把数据的循序从循环缓冲区转换成一般缓冲区的顺序
			wRetLen = LOOP_BUF_SIZE - m_wRxTail;
			if (wRetLen >= wBufSize)
			{
				memcpy(pbRxBuf, &m_bLoopBuf[m_wRxTail], wBufSize);
				return wBufSize;
			}
			else
			{
				memcpy(pbRxBuf, &m_bLoopBuf[m_wRxTail], wRetLen);

				if (wRetLen + m_wRxHead > wBufSize)
				{
					memcpy(pbRxBuf+wRetLen, m_bLoopBuf, wBufSize-wRetLen);
					return wBufSize;
				}
				else
				{
					memcpy(pbRxBuf+wRetLen, m_bLoopBuf, m_wRxHead);
					return wRetLen+m_wRxHead;
				}
			}
		}

		//取走接收循环缓冲区中的数据后,还不更新循环缓冲区的头尾指针,
		//等RcvBlock()函数真正消费掉后才把该数据从缓冲区中删除
	}
	else
	{
		return 0;
	}
}

//删除已经扫描的数据
void CProto::DeleteFromLoopBuf(WORD wLen)
{
	WORD wLeft = GetLoopBufLen();
    if (wLen > wLeft)
        wLen = wLeft;
            
	m_wRxTail = (m_wRxTail + wLen) % LOOP_BUF_SIZE;	
}


bool CProto::Send(BYTE* pbTxBuf, WORD wLen)
{
	if (m_pIf && m_pIf->CanTrans())   //接口是否还处于传输状态
	{	
		 if(m_pIf->GetIfType()==IF_SMS)
        {  
            BYTE bSendBuf[5000];
            if(wLen>=5000/2)
            {
                return false;
            }
			DTRACE(DB_CRITICAL, ("GPRS is outline, terminal send data to station by SMS mode\r\n"));
            HexToASCII(pbTxBuf, bSendBuf, wLen );              
            return m_pIf->Send(bSendBuf, wLen*2);
        }
        else
        {
		    return m_pIf->Send(pbTxBuf, wLen);
        }
	}
	else
	{
		return false;
	}
}

WORD CProto::Receive(BYTE* pbRxBuf, WORD wBufSize) 
{
	if (m_pIf && m_pIf->CanTrans()) //接口是否还处于传输状态
		return m_pIf->Receive(pbRxBuf, wBufSize);
	else
		return 0;
}

	
//接收并处理帧
bool CProto::RcvFrm()
{
	bool fRet = false;
	if (m_pIf == NULL)
		return false;

	WORD len = 0;
	BYTE  bBuf[PRO_FRM_SIZE];
	
	len = Receive(bBuf, PRO_FRM_SIZE-10);
	if (len > 0) 
	{
		DTRACE(DB_FAPROTO, ("RcvFrm: rx from if len=%d\n", len));
	}

	if (m_pProPara->fUseLoopBuf)
	{
		if (len > 0)
			PutToLoopBuf(bBuf, len);
			
		len = GetLoopBufLen();	//在没接收的时候直接从loopbuf里取
	}

	if (len <= 0)
	{
		#ifdef SYS_WIN	
			Sleep(300); //如果不加,在VC socket方式下不睡眠会导致CPU利用率高达99%,原因待查
		#endif //SYS_WIN

		return false;
	}
	
	while (len > 0)
	{
		if (m_pProPara->fUseLoopBuf)
		{ 
			len = RxFromLoopBuf(bBuf, PRO_FRM_SIZE-10);		
			if (len > 0) 				
			{
				DTRACE(DB_FAPROTO, ("RcvFrm: rx from loopbuf len=%d\n", len));
			}
		}
			
		if (len > 0)    //收到数据
		{
			int nScanLen = RcvBlock(bBuf, len);
			if (nScanLen > 0)   //成功组成一帧
			{
				HandleFrm();   //帧处理
				
				if (m_pProPara->fUseLoopBuf)
				{
					DeleteFromLoopBuf(nScanLen); //删除已经扫描的数据
				}
				else
				{
					len = len - nScanLen;
				}
				m_pIf->OnRcvFrm(); //在通信协议收到正确帧时调用,主要更新链路状态,比如心跳等
				fRet = true;
				m_dwRcvClick = GetClick();
			}
			else if (nScanLen < 0)   //不全的报文
			{
				if (m_pProPara->fUseLoopBuf)
					DeleteFromLoopBuf(-nScanLen); //删除已经扫描的数据
					
				break;
			}
		}
		else
		{
			break;
		}
	}
	
	return fRet;
}

bool CProto::Login()
{
	return false;
}

bool CProto::Beat()
{
	return false;
}

bool CProto::IsConnected() 
{
    if (m_pIf != NULL)
    	return m_pIf->GetState() == IF_STATE_TRANS;
    else
    	return false;
}
