
#include "stdafx.h"
#include "FaCfg.h"
#include "FaProto.h"
#include "sysarch.h" 
#include "sysapi.h" 
#include "EmbedGprsIf.h"
#include "syssock.h" 
#include "Trace.h"
#include "ProHook.h"
#include "ProIfConst.h"
#include "GprsWorker.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//CEmbedGprsIf
extern CGprsWorker g_GprsWorker;

CEmbedGprsIf::CEmbedGprsIf()
{
	m_wIfType = IF_SOCKET;
	m_fIfValid = false;
}

CEmbedGprsIf::~CEmbedGprsIf()
{
}

bool CEmbedGprsIf::Init(TGprsPara* pTGprsPara)
{
	if (!CProtoIf::Init(&pTGprsPara->SocketPara.IfPara)) //����ĳ�ʼ��
		return false;
		
	m_pTEmbedGprsPara = pTGprsPara;
	
	m_wState = IF_STATE_CONNECT;
	m_bBakIP = 0;
	m_wIPUseCnt = 0;
	m_pModem = g_GprsWorker.GetModem();
	m_pProto = NULL;
	
	return true;	
}

//����:��ʼ��������
void CEmbedGprsIf::InitSvr(int socket)
{
	m_wState = IF_STATE_TRANS;

	m_fExit = false;
	m_fExitDone = false;
}

void CEmbedGprsIf::LoadUnrstPara()
{
	if (m_pfnLoadUnrstPara!=NULL) // && m_fUnrstParaChg
	{
		m_fUnrstParaChg = false;
		if ((*m_pfnLoadUnrstPara)(m_pTEmbedGprsPara))	//��Ҫ���³�ʼ��
		{
			//if (m_wState == IF_STATE_TRANS)
			{
				DisConnect();
			}
		}
	}	
}	

bool CEmbedGprsIf::Send(BYTE* pbTxBuf, WORD wLen)
{
	int iReLen = 0;
	DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Send: GetClick()=%d,wLen=%d.\r\n", GetClick(),wLen));
	TraceFrm("<-- CEmbedGprsIf::Send:", pbTxBuf, wLen);	
	if (m_pTEmbedGprsPara->SocketPara.fEnableFluxStat)	//�Ƿ���������ͳ��,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
		AddFlux(wLen);
	iReLen = m_pModem->Send(pbTxBuf, wLen);
	if (iReLen == wLen)
	{
		m_wGprsTxCnt++;	
		return true;
	}
	DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Send : sock tx fail iReLen=%d, wLen=%d.\r\n", iReLen, wLen));
	return false;
}

//����:���մ�����������,�������ѭ���������л�������,�򷵻�ѭ���������е�����,
//     ������ô��ڽ��պ���,ֱ�ӵȴ����ڵ����ݵ���
//����:@pbRxBuf �������շ������ݵĻ�����,
//     @wBufSize ���ջ������Ĵ�С
//����:�������ݵĳ���
WORD CEmbedGprsIf::Receive(BYTE* pbRxBuf, WORD wBufSize)
{
	int iRecvLen = 0; 
	iRecvLen = m_pModem->Receive(pbRxBuf, wBufSize);
	if (iRecvLen > 0)
	{
		if (m_pTEmbedGprsPara->SocketPara.fEnableFluxStat)	//�Ƿ���������ͳ��,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
			AddFlux((DWORD )iRecvLen);

   		DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Receive:GetClick()=%d.\r\n",GetClick()));
   		TraceFrm("--> CEmbedGprsIf::Receive:", pbRxBuf, iRecvLen);			
	}
	else if (iRecvLen < 0)
	{
		DisConnect();
		return 0;
	}
	return iRecvLen;
}

void CEmbedGprsIf::OnRcvFrm()
{
	CProtoIf::OnRcvFrm();
//	DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Receive:OnRcvFrm()=%d.\r\n", m_dwRxClick));
}

//��������������
bool CEmbedGprsIf::Connect()
{
	DisConnect();
	
  	//����IP�л���
	//1����û�гɹ���½��վ������£�����ʹ����/��1/��2 IP
	//2���ڵ�½OK������£��嵱ǰIP��ʹ�ü������������Լ���ʹ��
	if (m_bBakIP > 2)
		m_bBakIP = 0;
	if (m_bBakIP == 0)	//��ǰҪʹ����IP
	{
		if (m_wIPUseCnt < m_pIfPara->wConnectNum && m_pTEmbedGprsPara->SocketPara.dwRemoteIP!=0 && m_pTEmbedGprsPara->SocketPara.wRemotePort!=0)//�� IP�˿���Ч
		{
	        m_wIPUseCnt++;
			m_dwRemoteIP = m_pTEmbedGprsPara->SocketPara.dwRemoteIP;
			m_wRemotePort = m_pTEmbedGprsPara->SocketPara.wRemotePort;
		}
        else
        {
        	m_dwRemoteIP = m_pTEmbedGprsPara->SocketPara.dwBakIP1;
			m_wRemotePort = m_pTEmbedGprsPara->SocketPara.wBakPort1;
            m_wIPUseCnt = 0;
           	m_bBakIP = 1; //���л�������1 IP
        }
	}
	if (m_bBakIP == 1)	//��ǰҪʹ�ñ���1IP
	{
		if (m_wIPUseCnt < m_pIfPara->wConnectNum && m_pTEmbedGprsPara->SocketPara.dwBakIP1!=0 && m_pTEmbedGprsPara->SocketPara.wBakPort1!=0)//����1 IP�˿���Ч
		{
	        m_wIPUseCnt++;
			m_dwRemoteIP = m_pTEmbedGprsPara->SocketPara.dwBakIP1;
			m_wRemotePort = m_pTEmbedGprsPara->SocketPara.wBakPort1;
		}
        else
        {
        	m_dwRemoteIP = m_pTEmbedGprsPara->SocketPara.dwBakIP2;
			m_wRemotePort = m_pTEmbedGprsPara->SocketPara.wBakPort2;
            m_wIPUseCnt = 0;
           	m_bBakIP = 2; //���л�������2 IP
        }
	}
	if (m_bBakIP == 2)	//��ǰҪʹ�ñ���2IP
	{
		if (m_wIPUseCnt < m_pIfPara->wConnectNum && m_pTEmbedGprsPara->SocketPara.dwBakIP2!=0 && m_pTEmbedGprsPara->SocketPara.wBakPort2!=0)//����2 IP�˿���Ч
		{
			m_wIPUseCnt++;
			m_dwRemoteIP = m_pTEmbedGprsPara->SocketPara.dwBakIP2;
			m_wRemotePort = m_pTEmbedGprsPara->SocketPara.wBakPort2;
		}
		else
        {
			m_dwRemoteIP = m_pTEmbedGprsPara->SocketPara.dwRemoteIP;
			m_wRemotePort = m_pTEmbedGprsPara->SocketPara.wRemotePort;        	
            m_wIPUseCnt = 1;   //�����Ѿ�ʹ�ù�һ����
            m_bBakIP = 0;  //�����л�����IP
        }
	}
		
	DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Connect: connecting %d.%d.%d.%d %d.\r\n",
						 (m_dwRemoteIP>>24)&0xff, (m_dwRemoteIP>>16)&0xff, (m_dwRemoteIP>>8)&0xff, m_dwRemoteIP&0xff, 
						 m_wRemotePort));
	
	
	if (!m_pModem->Connect(m_pTEmbedGprsPara->SocketPara.fUdp, m_dwRemoteIP, m_wRemotePort))
	{
		DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Connect: connect failed.\r\n"));	
		return false;
	}
    DTRACE(DB_FAPROTO, ("CEmbedGprsIf::Connect: connect ok.\r\n"));
	m_fIfValid = true;
	
	m_iLastErr = GPRS_ERR_OK;
  	return true;
}

//����:ȡ���Ӵ���,GPRS��socket���ӵ�ʱ��,����б���IP�˿ڵĻ�,���Ӵ�����2
WORD CEmbedGprsIf::GetConnectNum()
{
	WORD wConnectNum = m_pIfPara->wConnectNum;
	if (m_pTEmbedGprsPara->SocketPara.dwBakIP1!=0 && m_pTEmbedGprsPara->SocketPara.wBakPort1!=0)	//������վIP�Ͷ˿�
		wConnectNum += m_pIfPara->wConnectNum;
	if (m_pTEmbedGprsPara->SocketPara.dwBakIP2!=0 && m_pTEmbedGprsPara->SocketPara.wBakPort2!=0)	//������վIP�Ͷ˿�
		wConnectNum += m_pIfPara->wConnectNum;
	return wConnectNum;
}

void CEmbedGprsIf::EnterDorman()
{
	CProtoIf::EnterDorman();
}
	
//����:�ӿڱ���̽��
void CEmbedGprsIf::KeepAlive()
{	
	DWORD dwBeatSeconds = GetBeatSeconds();
	if (dwBeatSeconds == 0) //!m_pSocketPara->fBeatOn
		return;
	
	DWORD dwClick = GetClick();
	DWORD dwBrokenTime = dwBeatSeconds + 
					 	 m_pTEmbedGprsPara->SocketPara.dwBeatTimeouts*m_pTEmbedGprsPara->SocketPara.wBeatTestTimes;
	if (m_pTEmbedGprsPara->SocketPara.wBeatTestTimes!=0 && dwClick-m_dwRxClick>dwBrokenTime)
	{	//m_pSocketPara->wBeatTestTimes����Ϊ0,��ʾ���Զ�����,ֻ���ڷ�����
		DisConnect(); //���³�ʼ��
		DTRACE(DB_FAPROTO, ("CEmbedGprsIf::KeepAlive : DisConnect at click %d\n", dwClick));
	}
	else if (dwClick-m_dwRxClick > dwBeatSeconds)//40 20�뻹û�յ���һ֡��������������
	{	//�տ�ʼʱdwBeatClickΪ0��dwNewClick��Ϊ0�������Ϸ�
		if (dwClick-m_dwBeatClick > m_pTEmbedGprsPara->SocketPara.dwBeatTimeouts)
		{							//������ʱʱ��,��λ��
			if (m_pProto != NULL)
				m_pProto->Beat();
			m_dwBeatClick = dwClick;
			DTRACE(DB_FAPROTO, ("CEmbedGprsIf::KeepAlive: heart beat test at click %d\n", dwClick));
		}
	}
}

bool CEmbedGprsIf::Close()
{
	m_pModem->Close();
	return true;
}


//����:�ڽӿ�������תΪ�Ͽ���ʱ����ã������������Ͽ����Ǳ����Ͽ�
bool CEmbedGprsIf::DisConnect()
{
	m_pModem->DisConnect();
	CProtoIf::DisConnect();
	
	if (m_pTEmbedGprsPara->SocketPara.fSvr) //������ģʽ��,socket�Ͽ�ʱ�˳��߳�
		m_fExit = true;
	m_fIfValid = false;
	return true;
}

bool CEmbedGprsIf::IsIfValid()
{
	return m_fIfValid;
}

void CEmbedGprsIf::OnConnectFail()
{
	 CProtoIf::OnConnectFail();
}

//����:��Э���½�ɹ�ʱ����
void CEmbedGprsIf::OnLoginOK()
{ 
	CProtoIf::OnLoginOK();
	
	m_wIPUseCnt = 0; //�ڵ�½OK������£��嵱ǰIP��ʹ�ü������������Լ���ʹ�ã�
					 //����������IPʹ����ͬ�Ĵ����������л�
}
