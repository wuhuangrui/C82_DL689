/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Proto.cpp
 * ժ    Ҫ�����ļ�ʵ��������վͨ�ŵ�Э�����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע��
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
		
	m_fConnected = false;	//Э���Ƿ��Ѿ�������
	
	m_nRxStep = 0;
	m_wRxPtr = 0;
	m_fUseLoopBuf = false;
	m_wRxHead = 0;
	m_wRxTail = 0;	
	
	m_dwRcvClick = 0;
	return true;
}

				
//����:�ڽӿڽ������Ӻ�ص�������,Э������Ӧ�ĳ�ʼ��
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

//����:�Ѵ��ڽ��յ������ݷŵ�����ѭ����������ȥ
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
	if (m_wRxHead != m_wRxTail)   //������Ϊ���ջ��������ڵ���ѭ��������
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
			//��������ݵ�ѭ���ѭ��������ת����һ�㻺������˳��
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

		//ȡ�߽���ѭ���������е����ݺ�,��������ѭ����������ͷβָ��,
		//��RcvBlock()�����������ѵ���ŰѸ����ݴӻ�������ɾ��
	}
	else
	{
		return 0;
	}
}

//ɾ���Ѿ�ɨ�������
void CProto::DeleteFromLoopBuf(WORD wLen)
{
	WORD wLeft = GetLoopBufLen();
    if (wLen > wLeft)
        wLen = wLeft;
            
	m_wRxTail = (m_wRxTail + wLen) % LOOP_BUF_SIZE;	
}


bool CProto::Send(BYTE* pbTxBuf, WORD wLen)
{
	if (m_pIf && m_pIf->CanTrans())   //�ӿ��Ƿ񻹴��ڴ���״̬
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
	if (m_pIf && m_pIf->CanTrans()) //�ӿ��Ƿ񻹴��ڴ���״̬
		return m_pIf->Receive(pbRxBuf, wBufSize);
	else
		return 0;
}

	
//���ղ�����֡
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
			
		len = GetLoopBufLen();	//��û���յ�ʱ��ֱ�Ӵ�loopbuf��ȡ
	}

	if (len <= 0)
	{
		#ifdef SYS_WIN	
			Sleep(300); //�������,��VC socket��ʽ�²�˯�߻ᵼ��CPU�����ʸߴ�99%,ԭ�����
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
			
		if (len > 0)    //�յ�����
		{
			int nScanLen = RcvBlock(bBuf, len);
			if (nScanLen > 0)   //�ɹ����һ֡
			{
				HandleFrm();   //֡����
				
				if (m_pProPara->fUseLoopBuf)
				{
					DeleteFromLoopBuf(nScanLen); //ɾ���Ѿ�ɨ�������
				}
				else
				{
					len = len - nScanLen;
				}
				m_pIf->OnRcvFrm(); //��ͨ��Э���յ���ȷ֡ʱ����,��Ҫ������·״̬,����������
				fRet = true;
				m_dwRcvClick = GetClick();
			}
			else if (nScanLen < 0)   //��ȫ�ı���
			{
				if (m_pProPara->fUseLoopBuf)
					DeleteFromLoopBuf(-nScanLen); //ɾ���Ѿ�ɨ�������
					
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
