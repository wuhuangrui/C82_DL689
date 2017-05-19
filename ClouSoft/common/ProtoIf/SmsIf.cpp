#include "stdafx.h"
#include <stdio.h>
#include "FaCfg.h"
#include "FaConst.h"
#include "sysarch.h" 
#include "sysapi.h"
#include "SmsIf.h"
#include "ProHook.h"
#include "ProIfConst.h"
#include "Info.h"

#ifdef SYS_VDK
#include "lwip/ctrl.h"
#include "ppp.h"
#endif //SYS_VDK

	
/////////////////////////////////////////////////////////////////////////////////////////////////////
//CGprsIf

extern CGprsWorker g_GprsWorker;

CSmsIf::CSmsIf()
{
	//m_wCnMode = CN_MODE_SOCKET;
	//m_bRstMode = GPRS_RST_ON_IDLE;	  //GPRSģ��ĸ�λģʽ,��λ���ڿ���״̬���Ƕ���״̬
	m_wIfType = IF_SMS;
}


CSmsIf::~CSmsIf()
{
	
}


bool CSmsIf::Init(TSmsIfPara* pSmsIfPara)
{
	if (!CProtoIf::Init(&pSmsIfPara->IfPara)) //����ĳ�ʼ��
		return false;
	
    m_pSmsIfPara = pSmsIfPara;
	//m_iGprsUser = g_GprsWorker.ReqUserID();
	m_wState = IF_STATE_TRANS; 
	return true;//ReInit(m_pGprsPara);
}

//����:���մ�����������,�������ѭ���������л�������,�򷵻�ѭ���������е�����,
//     ������ô��ڽ��պ���,ֱ�ӵȴ����ڵ����ݵ���
//����:@pbRxBuf �������շ������ݵĻ�����,
//     @wBufSize ���ջ������Ĵ�С
//����:�������ݵĳ���
WORD CSmsIf::Receive(BYTE* pbRxBuf, WORD wBufSize)
{
	WORD wLen = 0;
	
	wLen = g_GprsWorker.ReceiveSms(pbRxBuf);
	if (wLen > 0)
		DTRACE(DB_FAPROTO, ("CSmsIf::Receive : rx len=%d in sms mode\n", wLen));
			
	return wLen;
}

bool CSmsIf::Send(BYTE* pbTxBuf, WORD wLen)
{
	DTRACE(DB_FAPROTO, ("CSmsIf::Send : tx len=%d in sms mode\n", wLen));
	return g_GprsWorker.SendSms(pbTxBuf, wLen);
}

