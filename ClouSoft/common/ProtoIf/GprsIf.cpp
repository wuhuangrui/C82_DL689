#include "stdafx.h"
#include <stdio.h>
#include "FaCfg.h"
#include "FaConst.h"
#include "sysarch.h" 
#include "sysapi.h"
#include "GprsIf.h"
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

CGprsIf::CGprsIf()
{
	m_wSignStrength = 0;
	m_wCnMode = CN_MODE_SOCKET;
	m_bRstMode = GPRS_RST_ON_IDLE;	  //GPRSģ��ĸ�λģʽ,��λ���ڿ���״̬���Ƕ���״̬

	//ͳ������
	m_wGprsTxCnt = 0;
	m_wGprsRxCnt = 0;
	m_wSmsTxCnt = 0;
	m_wSmsRxCnt = 0;
	
	m_dwErrRstClick = 0;
	m_dwSignClick = 0;
	m_dwSmsOverflowClick = 0;
	m_dwPeriodDropInterv = 0; //ʱ������ģʽ�ļ��ʽ�Զ�����ʱ��,��λ����
	
	m_fRstInConnectFail = true;	//������ʧ�ܵ����Դ�����λ�ӿ�
	//m_iPd = -1;	//ppp���豸��,ֻ��lwip���õ�
	m_iGprsUser = -1;
	
	m_wIfType = IF_GPRS;
}


CGprsIf::~CGprsIf()
{
	
}


bool CGprsIf::ReInit(TGprsPara* pGprsPara)
{	
	m_pGprsPara = pGprsPara;
	m_wCnMode = m_pGprsPara->wCnMode; 
	
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)
	{
		if (!CSocketIf::Init(&pGprsPara->SocketPara)) //����ĳ�ʼ��
			return false;
	}
	else if (m_wCnMode == CN_MODE_EMBED)	
	{		
		m_pSocketPara = &pGprsPara->SocketPara;
		if (!CProtoIf::Init(&pGprsPara->SocketPara.IfPara)) //����ĳ�ʼ��
			return false;
		
		if (!m_embdGprsIf.Init(pGprsPara)) //����ĳ�ʼ��
			return false;		
	}
	else
	{
		if (!CProtoIf::Init(&pGprsPara->SocketPara.IfPara)) //����ĳ�ʼ��
			return false;		
	}
				
	//�ڷ��������ߵ������,wCnMode����ΪGPRS��ͨ��ģʽ,
	//���Է����������ϵ���������GPRS��¼һ��
	if (m_wCnMode == CN_MODE_ETHSCK)	//��̫��ֻ����������
	{
		m_wState = IF_STATE_CONNECT;
	}
	else if (m_pGprsPara->bOnlineMode == ONLINE_M_JIT)	//��ʱ����,����Ҫ������
	{
		m_wState = IF_STATE_DORMAN;
	}
	else if (m_pGprsPara->bOnlineMode == ONLINE_M_PERSIST) //��������
	{
	    m_wState = IF_STATE_RST;
	}
	else //�����ʱ������
	{
		m_wState = IF_STATE_RST;
		
        m_wCnMode = m_pGprsPara->wCnMode;
        if (m_pGprsPara->fRstOnSms)
        {    
            if (m_pGprsPara->dwPowerupDropInterv == 0) //�ϵ缤����Զ�����ʱ��,��Ϊ0�Զ�ȡ���ϵ缤��
                m_wCnMode = CN_MODE_SMS;
            else if (GetClick() >= m_pGprsPara->dwPowerupDropInterv*60)	//m_pGprsPara->dwPowerupDropInterv==0 ����������������ӿ����³�ʼ��,�������ϵ缤��ʱ��Ҳ������
                m_wCnMode = CN_MODE_SMS;
            //else							//�����������Ҫ�ϵ缤��
            //	m_wCnMode = m_pGprsPara->wCnMode;
        }		
			//ʱ������ģʽ�Ȱ�ģ�����ڿ���״̬,
			//��DoIfRelated()�ȵ����ߵ�ʱ�ε���,�Ÿ�λģ��
	}	
	
	m_bRstMode = GetGprsRstMode();	  //GPRSģ��ĸ�λģʽ,��λ���ڿ���״̬���Ƕ���״̬
	g_GprsWorker.SetWorkMode(m_iGprsUser, m_bRstMode);

	DTRACE(DB_FAPROTO, ("CGprsIf::Init: if(%s) init to CN mode = %s, online mode = %s, BeatSeconds=%ld, m_iGprsUser=%d\n", 
						GetName(),
						CnModeToStr(m_wCnMode), 
						OnlineModeToStr(m_pGprsPara->bOnlineMode),
						GetBeatSeconds(),
						m_iGprsUser));
	m_dwFluxOverClick = 0;	//�����������ʼʱ��,һ�����������ж�
	return true;
}

//����:ȡ��GPRSģ��ĸ�λģʽ,��λ���ڿ���״̬���Ƕ���״̬
//����:GPRS_RST_ON_IDLE��λ���ڿ���״̬,GPRS_RST_ON_SMS��λ���ڶ���״̬
BYTE CGprsIf::GetGprsRstMode()
{
	BYTE bRstMode = GPRS_RST_ON_IDLE;
	if (m_wCnMode == CN_MODE_ETHSCK)	//��̫��ֻ����������
		return bRstMode;
		
	if ((m_pGprsPara->bOnlineMode==ONLINE_M_PERIOD || m_pGprsPara->bOnlineMode==ONLINE_M_ACTIVE) &&
		!m_pGprsPara->fRstOnSms)			//�Ƿ�λ������ģʽ����Ҫ��Լ���ģʽ��ʱ������ģʽ
	{
		return bRstMode;
	}

	if (m_pGprsPara->bOnlineMode==ONLINE_M_PERIOD 
		|| m_pGprsPara->bOnlineMode==ONLINE_M_ACTIVE 
		|| m_pGprsPara->bOnlineMode==ONLINE_M_SMS 
		|| (m_pGprsPara->bOnlineMode==ONLINE_M_PERSIST && m_pGprsPara->fEnableFluxCtrl)
		|| m_pGprsPara->bOnlineMode==ONLINE_M_DMINSMS) 
	{	//						����ģʽ			&&		������������
	    bRstMode = GPRS_RST_ON_SMS;
	}

	return bRstMode;
}

bool CGprsIf::Init(TGprsPara* pGprsPara)
{
    m_pGprsPara = pGprsPara;
	m_iGprsUser = g_GprsWorker.ReqUserID();
	return ReInit(m_pGprsPara);
}	

//�����������й����п�����������ͨ��ģʽ��������GPRS socket/ģ��Э��ջ����̫�����л�
bool CGprsIf::ResetCnMode(WORD wCnMode)
{
	m_pGprsPara->wCnMode = wCnMode;
	return ReInit(m_pGprsPara); 
}

char* CGprsIf::CnModeToStr(WORD wMode)
{
	static char* pszCnModeToStr[] = {"socket",			//0
					  	  			"sms",				//1
						    		"embed tcp/ip",		//2
						    		"comm",				//3
						    		"at cmd",			//4
						    		"eth socket",		//5
						    	  };
	
	if (wMode < CN_MODE_NUM)
		return pszCnModeToStr[wMode];
	else
		return "unknown";
}	


char* CGprsIf::OnlineModeToStr(WORD wMode)
{
	static char* pszOnlineModeToStr[] = {"unknown",
										 "persist",
					  	  				 "active(nononline)",
						    			 "period",
						    			 "sms"};
	
	if (wMode <= 3)
		return pszOnlineModeToStr[wMode];
	else
		return "unknown";
}

WORD CGprsIf::GetMaxFrmBytes()
{ 
	return m_wCnMode==CN_MODE_SMS ? m_pGprsPara->wSmsMaxFrmBytes :
					  m_pGprsPara->SocketPara.IfPara.wMaxFrmBytes; 
}

bool CGprsIf::Close()
{
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)
	{
		return CSocketIf::Close();
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{
		return m_embdGprsIf.Close();
	}
	return true;
}

//����:���մ�����������,�������ѭ���������л�������,�򷵻�ѭ���������е�����,
//     ������ô��ڽ��պ���,ֱ�ӵȴ����ڵ����ݵ���
//����:@pbRxBuf �������շ������ݵĻ�����,
//     @wBufSize ���ջ������Ĵ�С
//����:�������ݵĳ���
WORD CGprsIf::Receive(BYTE* pbRxBuf, WORD wBufSize)
{
	WORD wLen = 0;
	//if (g_GprsWorker.IsOnline() && m_wCnMode!=CN_MODE_SMS)
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)
	{
		return CSocketIf::Receive(pbRxBuf, wBufSize);
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{
		wLen = m_embdGprsIf.Receive(pbRxBuf, wBufSize);
		if (!m_embdGprsIf.IsIfValid())
			CProtoIf::DisConnect();
		return wLen;
	}
	else if (m_wCnMode == CN_MODE_SMS)
	{
		wLen = g_GprsWorker.ReceiveSms(pbRxBuf);
		if (wLen > 0)
			DTRACE(DB_FAPROTO, ("CGprsIf::Receive : rx len=%d in sms mode\n", wLen));
			
		return wLen;
	}
	else
	{
		Sleep(1000);
		return 0;
	}
	return 0;
}

void CGprsIf::OnRcvFrm()
{
	m_dwRstIfClick = m_dwRxClick = GetClick();
	m_embdGprsIf.OnRcvFrm();
}

bool CGprsIf::Send(BYTE* pbTxBuf, WORD wLen)
{
	//if (g_GprsWorker.IsOnline())
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)
	{
		if (m_bGprsDataSrc == DATA_SRC_SMS)//������Դ
			return g_GprsWorker.SendSms(pbTxBuf, wLen);
		else
			return CSocketIf::Send(pbTxBuf, wLen);
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{	
		return m_embdGprsIf.Send(pbTxBuf, wLen);
	}
	else if (m_wCnMode == CN_MODE_SMS)
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::Send : tx len=%d in sms mode\n", wLen));
		return g_GprsWorker.SendSms(pbTxBuf, wLen);
	}
	else
	{
		return false;
	}
	return false;
}

//��������λ����
//����: IF_RST_OK��λ�ɹ�,IF_RST_HARDFAILӲ��λʧ��,
//		IF_RST_SOFTFAIL��λʧ��(Э���)
int CGprsIf::ResetIf()
{
	WORD wTryCnt = 0;
	
again:
	if (m_wState == IF_STATE_DORMAN)	//�Ѿ���������ģʽ���ʹ˷���
	{
		DisConnect();
		ReqDorman(m_iGprsUser); //ReqOffline(m_iGprsUser);

		return IF_RST_OK;
	}
	
	if (m_wCnMode == CN_MODE_ETHSCK) //��̫��SOCKETͨ��ģʽ
	{
#ifndef SYS_WIN		
		//ModemPowerOff();	//��ģ���Դ����Ҫ2������
#endif
		
		return IF_RST_OK;
	}
	
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_EMBED) 	//����TCP/IP��ͨ��ģʽ
	{
		DisConnect();
		ReqOffline(m_iGprsUser);

		m_iLastErr = ReqOnline(m_iGprsUser);
		if(m_iLastErr == GPRS_ERR_OK)
			return IF_RST_OK;
		else
			return IF_RST_SOFTFAIL;
	}
	else
	{
		DisConnect();
		ReqOffline(m_iGprsUser);

		return IF_RST_OK;
	}
	
	wTryCnt++;
	if (wTryCnt >= 2)
		return IF_RST_SOFTFAIL;
		 
	goto again;
}

//����:���ⲿ���ã��ýӿڵ���
bool CGprsIf::RequestOffline()
{	
	DisConnect();
 	return ReqOffline(m_iGprsUser);
}

bool CGprsIf::Connect()
{
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)  //����TCP/IP��ͨ��ģʽ
	{
		return CSocketIf::Connect();
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{
		return m_embdGprsIf.Connect();
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::Connect : nothing to do in %s mode\n", CnModeToStr(m_wCnMode)));
	}	

	return true; 
}

bool CGprsIf::DisConnect()
{	
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)  //����TCP/IP��ͨ��ģʽ
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::DisConnect .\r\n"));
		if (CSocketIf::DisConnect() == false)
			return false;
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::DisConnect .\r\n"));
		CProtoIf::DisConnect();
		if (m_embdGprsIf.DisConnect() == false)
			return false;
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::DisConnect : nothing to do in %s mode\n", CnModeToStr(m_wCnMode)));
	}	

	return 	CProtoIf::DisConnect();
}

void CGprsIf::KeepAlive()
{
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)  //����TCP/IP��ͨ��ģʽ
	{
		if ((m_pGprsPara->bOnlineMode == ONLINE_M_PERSIST) && 
		(m_pGprsPara->fEnableFluxCtrl) && 
		(m_wCnMode==CN_MODE_SOCKET) && //��ǰ�Ѿ���������״̬
		IsFluxOver())	//��������
		{
			return;  //���ٷ�������Ҳ���ټ��
		}

		return CSocketIf::KeepAlive();
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{
		//m_embdGprsIf�������ֻ����ΪCGprsIf�ĳ�Ա��
		//CGprsIf��Э���ʱ��û�и����ԱҲ��Э�飬
		//�������������
		if (m_embdGprsIf.GetProto() == NULL)
			m_embdGprsIf.AttachProto(m_pProto);	

		if (!((m_pGprsPara->bOnlineMode == ONLINE_M_PERSIST) && 
		(m_pGprsPara->fEnableFluxCtrl) && 
		IsFluxOver()))	//��������	
		{
			m_embdGprsIf.KeepAlive();
		}

		if (!m_embdGprsIf.IsIfValid())
			CProtoIf::DisConnect();
	}	
}
	
void CGprsIf::OnConnectFail() 
{	
	if ((m_pGprsPara->bOnlineMode==ONLINE_M_PERIOD
		 || m_pGprsPara->bOnlineMode==ONLINE_M_ACTIVE
		 || m_pGprsPara->bOnlineMode==ONLINE_M_DMINSMS) && //����ģʽ/����������ģʽ
		m_pGprsPara->fRstOnSms && m_wCnMode!=CN_MODE_SMS &&	m_wCnMode!=CN_MODE_ETHSCK && //����GPRSģʽ
		m_wConnectFailCnt+1 >= GetConnectNum())
	{			
		//GPRS���Ӳ��ɹ�,�Զ��л�������ģʽ,����������״̬
		
		DTRACE(DB_FAPROTO, ("CGprsIf::OnConnectFail : %s switch to sms mode\n", GetName()));
		
		DisConnect();	//�ȶϿ�socket��ʽ�µ�����,��Ȼ�л�������ģʽ��û������
		m_wState = IF_STATE_RST; 
		m_wCnMode = CN_MODE_SMS; 
		m_wConnectFailCnt = 0; 
			//����GPRS���Ӳ��ɹ�, �ڵ���CProtoIf::OnConnectFail()ʱ
			//��������״̬,m_wConnectFailCnt���¿�ʼ����,
			//����ڶ��ŷ�ʽ�»���ʧ�ܳ���/���Դ���,
			//��Ҫ��������ģʽ,������ģ��
		if (m_pGprsPara->bOnlineMode == ONLINE_M_DMINSMS)
			m_dwDormanClick = GetClick();
	}
	
	if(m_wCnMode == CN_MODE_EMBED)
	{	
		CProtoIf::OnConnectFail();
		m_embdGprsIf.OnConnectFail();
	}
	else
		CSocketIf::OnConnectFail();
}


void CGprsIf::OnResetFail() 
{	
	if ((m_pGprsPara->bOnlineMode==ONLINE_M_PERIOD 
		|| m_pGprsPara->bOnlineMode==ONLINE_M_ACTIVE
		|| m_pGprsPara->bOnlineMode==ONLINE_M_DMINSMS) && //����ģʽ/����������ģʽ
		m_pGprsPara->fRstOnSms && m_wCnMode!=CN_MODE_SMS &&	m_wCnMode!=CN_MODE_ETHSCK && //����GPRSģʽ
		m_wResetFailCnt+1 >= m_pIfPara->wRstNum)
	{			
		//GPRS���Ӳ��ɹ�,�Զ��л�������ģʽ
		m_wCnMode = CN_MODE_SMS; 
		m_wResetFailCnt = 0; 
			//����GPRS���Ӳ��ɹ�, �ڵ���CProtoIf::OnConnectFail()ʱ
			//��������״̬,m_wConnectFailCnt���¿�ʼ����,
			//����ڶ��ŷ�ʽ�»���ʧ�ܳ���/���Դ���,
			//��Ҫ��������ģʽ,������ģ��
		if (m_pGprsPara->bOnlineMode == ONLINE_M_DMINSMS)
			m_dwDormanClick = GetClick();			
	}
	
	CProtoIf::OnResetFail();
}

//����:��Э���½�ɹ�ʱ����
void CGprsIf::OnLoginOK()
{ 
	if (m_wCnMode == CN_MODE_EMBED)
	{
		CProtoIf::OnLoginOK();
		m_embdGprsIf.ResetIPUseCnt();
	}
	else
		CSocketIf::OnLoginOK();
}


//����:��Э���½ʧ��ʱ����,������ٴ�ʧ�ܺ�Ͽ�����
void CGprsIf::OnLoginFail() 
{	
	m_wLoginFailCnt++;
	
	WORD wLoginRstNum = m_pIfPara->wLoginRstNum;
	if (wLoginRstNum == 0)
		wLoginRstNum = 1;
		
	WORD wLoginNum = m_pIfPara->wLoginNum;
	if (wLoginNum == 0)
		wLoginNum = 1;
	
	if (m_wLoginFailCnt%wLoginRstNum != 0)
	{					//��¼ʧ�ܵĴ�����û���Ͽ����ӵĴ���
		if (m_pSocketPara->bRandLoginFlg & RAND_ON_FAIL) //ʧ��ʱ���������ʱ
			StateToDorman(m_wState);	//��ĳ��״̬��ʱ�л�������״̬�������껹Ҫת����
		else
			Sleep(m_pIfPara->dwLoginInterv*1000); //��¼���
	}
	else if (m_wLoginFailCnt >= wLoginRstNum*wLoginNum)
	{					
		m_wLoginFailCnt = 0;
		
		if ((m_pGprsPara->bOnlineMode==ONLINE_M_PERIOD 			 
			|| m_pGprsPara->bOnlineMode==ONLINE_M_ACTIVE
			|| m_pGprsPara->bOnlineMode==ONLINE_M_DMINSMS) && //����ģʽ/����������ģʽ
			m_pGprsPara->fRstOnSms && m_wCnMode!=CN_MODE_SMS && m_wCnMode!=CN_MODE_ETHSCK)	//����GPRSģʽ
		{
			//GPRS���Ӳ��ɹ�,�Զ��л�������ģʽ
			DTRACE(DB_FAPROTO, ("CGprsIf::OnLoginFail : %s switch to sms mode\n", GetName()));
		
			DisConnect();	//�ȶϿ�socket��ʽ�µ�����,��Ȼ�л�������ģʽ��û������
			m_wState = IF_STATE_RST; 
			m_wCnMode = CN_MODE_SMS; 
			if (m_pGprsPara->bOnlineMode == ONLINE_M_DMINSMS)
				m_dwDormanClick = GetClick();
		}
		else
		{
			if (m_wCnMode == CN_MODE_EMBED)
				m_embdGprsIf.SetMaxIpUseCnt();
			else
				SetMaxIpUseCnt();
				
			DTRACE(DB_FAPROTO, ("CGprsIf::OnLoginFail: go to dorman\n"));
			EnterDorman();
		}
	}
	else		//��¼ʧ�ܵĴ����ﵽ�˶Ͽ����ӵĴ�����������
	{			//m_wLoginFailCnt%wLoginRstNum==0 && m_wLoginFailCnt<wLoginRstNum*wLoginNum
		if (m_wCnMode == CN_MODE_EMBED)
			m_embdGprsIf.SetMaxIpUseCnt();
		else
			SetMaxIpUseCnt();
		
		DisConnect();	//ֻ�Ͽ�����,�������¼ʧ�ܼ���,
						//�ﵽwLoginRstNum*wReTryNum�κ��������״̬
	}		
}

//����:����Ƿ���Ҫ����,
//����:�����Ҫ�����򷵻�true,���򷵻�false
//��ע:�������������Ҫ����:1.���ռ�����Ϣ(����/����);2.�����ϱ�
bool CGprsIf::CheckActivation()
{
	//return GetInfo(INFO_ACTIVE);
	
	if (GetInfo(m_pGprsPara->SocketPara.IfPara.wInfoActive)) //�յ��˶��ż���֡ INFO_ACTIVE
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::CheckActivation : switch to gprs mode due to rx activate info\n"));
		return true;
	}	
	
	/*if (m_pGprsPara->fEnableRingActive) //�������弤��  ��INFO_ACTIVEʵ��
	{
		if (m_pModem->HaveRing()) //�Ƿ��յ��������ź�
		{
			DTRACE(DB_FAPROTO, ("CGprsIf::CheckActivation : switch to gprs mode due to ring\n"));
			return true;
		}
	}*/
	
	if (m_pGprsPara->fEnableAutoSendActive) //���������ϱ�����
	{
		if (m_pProto->IsNeedAutoSend()) //��Ҫ�����ϱ�
		{
			DTRACE(DB_FAPROTO, ("CGprsIf::CheckActivation : switch to gprs mode due to auto send\n"));
			return true;
		}
	}	

	return false;
}

//����:��һЩ�����ӿ���صķǱ�׼������,
//		���ӿ�Ҫ����������:
//		1.ģʽ�л�,������������߷�ʽ��,GPRS��SMS����л�
void CGprsIf::DoIfRelated()
{
	//����������ģʽ��,Ҫʵ�ֵ��л���:
	//GPRSһ��ʱ��û��ͨ��,�л���SMS��ʽ
	//SMS�յ����Ż��ѻ�绰����,�л���GPRS��ʽ
	
	DWORD dwClick = GetClick();

	if (m_fSetIdleCmd)	//�յ��ⲿ�ĶϿ���������
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s rx set idle cmd.\r\n", GetName()));

		if ((m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK || m_wCnMode==CN_MODE_EMBED) && 
			m_wState>=IF_STATE_CONNECT) //��ǰ�Ѿ���������״̬IF_STATE_TRANS
		{
			DisConnect();	//�ȶϿ�socket��ʽ�µ�����,��Ȼ�л�������ģʽ��û������
		}

		m_fSetIdleCmd = false;

		m_dwDormanClick = 0; //û�й涨���ߵ�ʱ��,�൱�ڴ��������ڵ�����(����)״̬,
		m_wState = IF_STATE_DORMAN;
		ResetIf();
		m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ
	}

	if (m_fDisConnCmd)	//�յ��ⲿ�ĶϿ���������
	{
		DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s rx disconnect cmd, m_wCnMode : %d, m_wState : %d.\r\n", 
							GetName(), m_wCnMode, m_wState));

		if ((m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK || m_wCnMode==CN_MODE_EMBED) && 
			m_wState>=IF_STATE_CONNECT) //��ǰ�Ѿ���������״̬IF_STATE_TRANS
		{
			DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s disconnect from sockect mode, m_dwDormanInterv=%ld...\r\n", 
								GetName(), m_dwDormanInterv));
			CProto* pProto = GetProto();
			if (pProto!=NULL && m_wState==IF_STATE_TRANS) 
				pProto->Logoff();

			DisConnect();	//�ȶϿ�socket��ʽ�µ�����,��Ȼ�л�������ģʽ��û������
			if (m_dwDormanInterv != 0)	//��ʱ�趨�����߼������λ��
			{
				m_dwDormanClick = GetClick();	 //�������ߵĿ�ʼʱ��
				m_wState = IF_STATE_DORMAN;
			}
			else
			{
				m_wState = IF_STATE_RST; 
			}

			if (m_bRstMode == GPRS_RST_ON_SMS)
				m_wCnMode = CN_MODE_SMS; //�л���SMSģʽ
			else
				m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ

			//һЩģʽ���õ��ı���Ҳͳһ����һ��
			m_dwSignClick = 0;
			m_dwSmsOverflowClick = 0;
			m_dwPeriodDropInterv = 0;
			m_dwFluxOverClick = 0;	//�����������ʼʱ��,һ�����������ж�
		}
		else
		{
			if (m_pGprsPara->bOnlineMode != ONLINE_M_DMINSMS)//����ʱ�������ģʽ�������ߣ�����һֱ�ڶ���ģʽ������
				m_dwDormanInterv = 0;	//��ʱ�趨�����߼������λ��
		}

		m_fDisConnCmd = false;
	}

	if (m_wCnMode == CN_MODE_ETHSCK)	//�������̫�������������ģʽ�����޹أ��ʹ˷���
	{
		CProtoIf::DoIfRelated();
		return;
	}

	if (m_pGprsPara->bOnlineMode == ONLINE_M_ACTIVE)
	{						//����ģʽ/����������ģʽ
		if (m_wCnMode == CN_MODE_SMS)	//��ǰ���ڶ���ģʽ��
		{
			if (CheckActivation()) //��Ҫ����
			{	
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to gprs mode\n", GetName()));
				DisConnect();
				m_wState = IF_STATE_RST; 
				m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ
												   //ģ�鸴λ���̺߳���������
			}
		}
		else	//��ǰ����GPRSģʽ��
		{
			//��dwActiveDropInterv������û�н��յ���վ�ı���,����������
			if (m_wState==IF_STATE_TRANS && dwClick-m_dwRxClick>m_pGprsPara->dwActiveDropInterv*60)
			{	
				if (dwClick > m_pGprsPara->dwPowerupDropInterv*60) 
				{			//�ϵ缤����Զ�����ʱ��,��Ϊ0�Զ�ȡ���ϵ缤��
					DisConnect();
					
					if (m_pGprsPara->fRstOnSms)
					{
						DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to sms mode due to active timeouts\r\n", GetName()));
						m_wState = IF_STATE_RST; 
						m_wCnMode = CN_MODE_SMS; //�л���SMSģʽ
										 	 //ģ�鸴λ���̺߳���������
						m_dwSignClick = 0;
						m_dwSmsOverflowClick = 0;
					}
					else
					{
						DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to idle mode due to active timeouts\r\n", GetName()));
						m_dwDormanClick = 0; //û�й涨���ߵ�ʱ��,�൱�ڴ��������ڵ�����(����)״̬,
						m_wState = IF_STATE_DORMAN;
						ResetIf();
						m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ
					}
				}
			}
			else if (m_wState==IF_STATE_DORMAN && !m_pGprsPara->fRstOnSms)
			{
				if (CheckActivation()) //��Ҫ����
				{	
					DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s active to gprs mode from dorman\n", GetName()));
					m_dwDormanClick = 0;
					DisConnect();
					m_wState = IF_STATE_RST; 
					m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ
													   //ģ�鸴λ���̺߳���������
				}
			}
		}		
	}
	else if (m_pGprsPara->bOnlineMode == ONLINE_M_PERIOD)
	{									//ʱ������ģʽ
		//TTime time;
		//GetCurTime(&time);
		bool fInPeriod = GprsIsInPeriod();

		if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_EMBED) //��ǰ�Ѿ���������״̬
		{
			if (m_wState==IF_STATE_TRANS && //ֻ���ڴ��ڴ���״̬�����л�,����״̬���ʧ���Զ����л�������״̬
				!fInPeriod && //��ǰʱ��Ҫ����
				dwClick>m_pGprsPara->dwPowerupDropInterv*60 &&
				dwClick-m_dwRxClick>m_dwPeriodDropInterv*60)
			{	//�ϵ缤����Զ�����ʱ��,��Ϊ0�Զ�ȡ���ϵ缤��
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to period offline\n", GetName()));
				
				DisConnect();	//�ȶϿ�socket��ʽ�µ�����,��Ȼ�л�������ģʽ��û������
				m_wState = IF_STATE_RST; 
				m_wCnMode = CN_MODE_SMS; //�л���SMSģʽ
										 //ģ�鸴λ���̺߳���������
				m_dwPeriodDropInterv = 0;
				m_dwSignClick = 0;
				m_dwSmsOverflowClick = 0;
			}
		}
		else //��ǰ���ڶ���ģʽ�� ���� ��ǰ���ڵ���״̬
		{
			if (fInPeriod) //��ǰҪʱ������
			{ 	
				m_dwPeriodDropInterv = 0;
						
				m_wState = IF_STATE_RST; 
				m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ
												   //ģ�鸴λ���̺߳���������
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to period online\n", GetName()));
			}
			else if (CheckActivation())
			{
				m_dwPeriodDropInterv = m_pGprsPara->dwActiveDropInterv;
				
				m_wState = IF_STATE_RST; 
				m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ
												   //ģ�鸴λ���̺߳���������
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s active to online in period mode\n", GetName()));
			}	
		}
	}
	else if (m_pGprsPara->bOnlineMode == ONLINE_M_JIT)
	{		//JUST IN TIME ����Ҫ��ʱ����,�絥�����ϱ��˿�
		bool fNeedSend = m_pProto->IsNeedAutoSend();
		if (m_dwDormanClick==0 && m_wState==IF_STATE_DORMAN)
		{	//ģ�鴦�ڿ���״̬ ����������״̬
			if (fNeedSend)
			{
				m_wState = IF_STATE_RST;	//�˳�IF_STATE_DORMAN״̬
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to JIT mode\n", GetName()));
			}	
		}
		else if (m_wState == IF_STATE_TRANS)
		{	//��ǰ�Ѿ���������״̬ m_wCnMode == CN_MODE_SOCKET
			if (!fNeedSend)
			{
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to idle mode\n", GetName()));
				m_dwDormanClick = 0; //û�й涨���ߵ�ʱ��,�൱�ڴ��������ڵ�����(����)״̬,
				m_wState = IF_STATE_DORMAN;
				ResetIf();
				m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ
												   //ģ�鸴λ���̺߳���������
			}	
		}	
	}
	else if (m_pGprsPara->bOnlineMode==ONLINE_M_PERSIST) //��������ģʽ����������
	{
		if (m_pGprsPara->fEnableFluxCtrl) //������������
		{
			if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_EMBED) //��ǰ�Ѿ���������״̬
			{
				if (m_wState==IF_STATE_TRANS && //ֻ���ڴ��ڴ���״̬�����л�,����״̬���ʧ���Զ����л�������״̬
					IsFluxOver())	//��������
				{	
					if (m_dwFluxOverClick == 0)
					{
						m_dwFluxOverClick = GetClick();	//�����������ʼʱ��
						GprsOnFluxOver();		//�ص�����,�������ɸ澯��¼����;
					}

					//�ϵ缤����Զ�����ʱ��,��Ϊ0�Զ�ȡ���ϵ缤��
					if (GprsIsTxComplete(m_dwFluxOverClick) && //�澯�������������ݶ�����
						dwClick>m_pGprsPara->dwPowerupDropInterv*60 && dwClick-m_dwRxClick>m_pGprsPara->dwActiveDropInterv*60) 
					{	//�ϵ缤����Զ�����ʱ��,��Ϊ0�Զ�ȡ���ϵ缤��
						DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to sms mode due to flux over\n", GetName()));
						CProto* pProto = GetProto();
						if (pProto != NULL) 
							pProto->Logoff();

						DisConnect();
						m_wState = IF_STATE_RST; 
						m_wCnMode = CN_MODE_SMS; //�л���SMSģʽ
												 //ģ�鸴λ���̺߳���������
						m_dwSignClick = 0;
						m_dwSmsOverflowClick = 0;
						m_dwFluxOverClick = 0;	//�����������ʼʱ��,һ�����������ж�
						//GprsOnFluxOver();		//�ص�����,�������ɸ澯��¼����;
					}
				}
			}
			else //��ǰ���ڶ���ģʽ�� ���� ��ǰ���ڵ���״̬
			{
				m_dwFluxOverClick = 0;	//�����������ʼʱ��,һ�����������ж�
										
				if (CheckActivation())
				{
					DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to gprs mode due to active in flux ctrl\n", GetName()));
					DisConnect();
					m_wState = IF_STATE_RST; 
					m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ
				}
				else if (!IsFluxOver())	//�������л�������������Ϊ0����������ø���,���л�������ģʽ
				{
					DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to gprs mode to flux not over\n", GetName()));
					DisConnect();
					m_wState = IF_STATE_RST; 
					m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ
				}
			}
		}
		else if (m_wCnMode == CN_MODE_SMS) //��ֹ����������;�л���������,һ�´��ڶ���״̬
		{
			m_dwFluxOverClick = 0;	//�����������ʼʱ��
									
			DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to gprs mode due to exit flux ctrl\n", GetName()));
			DisConnect();
			m_wState = IF_STATE_RST; 
			m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ
		}		
	}
	else if (m_pGprsPara->bOnlineMode == ONLINE_M_DMINSMS) //����ʱ����GPRS
	{
		if (m_wCnMode == CN_MODE_SMS)
		{
			if (GetClick()-m_dwDormanClick >= m_pIfPara->dwDormanInterv)
			{
				DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : %s switch to gprs mode, %d seconds exit dormant.\n", GetName(), m_pIfPara->dwDormanInterv));
				DisConnect();
				m_wState = IF_STATE_RST; 
				m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ
			}
			else
			{
				Sleep(1000);
				if ((GetClick()%5) == 0)
					DTRACE(DB_FAPROTO, ("CGprsIf::DoIfRelated : do dorman remain %ld\n", m_pIfPara->dwDormanInterv + m_dwDormanClick - dwClick));
			}
		}
	}	
	
	CProtoIf::DoIfRelated();
}


//ʹ�ӿڽ������߷�ʽ
void CGprsIf::EnterDorman()
{
	if (m_wCnMode==CN_MODE_SOCKET || m_wCnMode==CN_MODE_ETHSCK)
	{
		CSocketIf::EnterDorman();
	}
	else if (m_wCnMode == CN_MODE_EMBED)
	{
		CProtoIf::EnterDorman();
		m_embdGprsIf.EnterDorman();
	}			
	ResetIf();
    m_wDisConnectByPeerCnt = 0;	
	//m_fRstOk = false;  //�������˺�ӿ���Ҫ��λ
}    		

void CGprsIf::OnDisConnectByPeer()
{
    m_wDisConnectByPeerCnt++;
    if (m_pSocketPara->wDisConnectByPeerNum!=0 && m_wDisConnectByPeerCnt>=m_pSocketPara->wDisConnectByPeerNum) 
    {
        m_wDisConnectByPeerCnt = 0;	
        
      	if (m_pGprsPara->bOnlineMode==ONLINE_M_ACTIVE || 
            m_pGprsPara->bOnlineMode==ONLINE_M_JIT || 
            m_pGprsPara->bOnlineMode == ONLINE_M_PERIOD ||
            m_pGprsPara->bOnlineMode == ONLINE_M_DMINSMS)
        {
            if (m_pGprsPara->fRstOnSms && m_wCnMode!=CN_MODE_ETHSCK)
            {
                DTRACE(DB_FAPROTO, ("CGprsIf::OnDisConnectByPeer : %s switch to sms mode \r\n", GetName()));
                m_wState = IF_STATE_RST; 
                m_wCnMode = CN_MODE_SMS; //�л���SMSģʽ
                                     //ģ�鸴λ���̺߳���������
                m_dwPeriodDropInterv = 0;
                m_dwSignClick = 0;
                m_dwSmsOverflowClick = 0;
           		if (m_pGprsPara->bOnlineMode == ONLINE_M_DMINSMS)
					m_dwDormanClick = GetClick();
            }
            else
            {
                DTRACE(DB_FAPROTO, ("CGprsIf::OnDisConnectByPeer : %s switch to idle mode \r\n", GetName()));
                m_dwDormanClick = 0; //û�й涨���ߵ�ʱ��,�൱�ڴ��������ڵ�����(����)״̬,
                m_wState = IF_STATE_DORMAN;
                ResetIf();
                m_wCnMode = m_pGprsPara->wCnMode;  //�л���GPRS��ͨ��ģʽ
            }
        }
        else
        {
			DTRACE(DB_FAPROTO, ("CGprsIf::OnDisConnectByPeer: go to dorman\n"));
            EnterDorman();
        }
    }
}

void CGprsIf::LoadUnrstPara()
{
	if (m_pfnLoadUnrstPara != NULL)  //&& m_fUnrstParaChg //m_fUnrstParaChg = false;
	{
		if ((*m_pfnLoadUnrstPara)(m_pGprsPara))	//��Ҫ���³�ʼ��
		{
			//if (m_wState == IF_STATE_TRANS)
			{
				DisConnect();
				ReqOffline(m_iGprsUser);
				ReInit(m_pGprsPara);
			}				
		}
		else	//����Ҫ���³�ʼ��
		{
			BYTE bRstMode = GetGprsRstMode();	  //GPRSģ��ĸ�λģʽ,��λ���ڿ���״̬���Ƕ���״̬
			if (bRstMode != m_bRstMode)
			{
				m_bRstMode = bRstMode;
				g_GprsWorker.SetWorkMode(m_iGprsUser, m_bRstMode);
			}
		}
	}	
}

//���ӿڵ�ͨ��Э���Ƿ���Ҫ��¼
bool CGprsIf::IsNeedLogin() 
{
	if (m_wCnMode == CN_MODE_SMS)	//����ͨ�ŷ�ʽ���߷����������л������ŷ�ʽ
		return false;				//����Ҫ��¼
	else	
		return m_pIfPara->fNeedLogin;
}


//����:ȡ�������,����0��ʾ��������
DWORD CGprsIf::GetBeatSeconds() 
{
	DWORD dwClick = GetClick();
	if (m_pGprsPara->bOnlineMode == ONLINE_M_ACTIVE) //����ģʽ/����������ģʽ
	{									
		if (dwClick < m_pGprsPara->dwPowerupDropInterv*60)
		{			//�ϵ缤����Զ�����ʱ��,��Ϊ0�Զ�ȡ���ϵ缤��
			return m_pGprsPara->dwPowerupBeatMinutes;	 //�ϵ缤����������
		}
		else
		{
			return 0;
		}	
	}
	else if (m_pGprsPara->bOnlineMode == ONLINE_M_PERIOD) //ʱ������ģʽ
	{
		if (dwClick < m_pGprsPara->dwPowerupDropInterv*60)
		{			//�ϵ缤����Զ�����ʱ��,��Ϊ0�Զ�ȡ���ϵ缤��
			return m_pGprsPara->dwPowerupBeatMinutes;	 //�ϵ缤����������
		}
		else
		{
			return m_pGprsPara->SocketPara.dwBeatSeconds;
		}	
	}
	else
	{
		return m_pGprsPara->SocketPara.dwBeatSeconds;
	}
}

CEmbedGprsIf* CGprsIf::GetEmbedGprsIf()
{
    return &m_embdGprsIf;
}
