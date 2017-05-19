
#include "stdafx.h"
#include "FaCfg.h"
#include "FaProto.h"
#include "ProIfAPI.h"
#include "sysarch.h" 
#include "SocketIf.h"
#include "syssock.h" 
#include "Trace.h"
#include "Modem.h"
#include "ProHook.h"
#include "ProIfConst.h"
#include "sysapi.h"
#include "GprsWorker.h"

#ifdef SYS_LINUX
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif 

/////////////////////////////////////////////////////////////////////////////////////////////////////
//CSocketIf

CSocketIf::CSocketIf()
{
	m_wIfType = IF_SOCKET;
	m_Socket = INVALID_SOCKET;
}

CSocketIf::~CSocketIf()
{
}

bool CSocketIf::Init(TSocketPara* pSocketPara)
{
	if (!CProtoIf::Init(&pSocketPara->IfPara)) //����ĳ�ʼ��
		return false;
		
	m_pSocketPara = pSocketPara;
	
	m_wState = IF_STATE_CONNECT;
	m_bBakIP = 0;
	m_wIPUseCnt = 0;
	m_wDisConnectByPeerCnt = 0;
	return true;	
}

//����:��ʼ��������
void CSocketIf::InitSvr(int socket)
{
	m_Socket = socket;
	m_wState = IF_STATE_TRANS;

	m_fExit = false;
	m_fExitDone = false;
}

void CSocketIf::LoadUnrstPara()
{
	if (m_pfnLoadUnrstPara!=NULL) // && m_fUnrstParaChg
	{
		m_fUnrstParaChg = false;
		if ((*m_pfnLoadUnrstPara)(m_pSocketPara))	//��Ҫ���³�ʼ��
		{
			//if (m_wState == IF_STATE_TRANS)
			{
				DisConnect();
			}
		}
	}	
}	

bool CSocketIf::Send(BYTE* pbTxBuf, WORD wLen)
{
	DTRACE(DB_FAPROTO, ("CSocketIf::Send: GetClick()=%d,wLen=%d.\r\n",GetClick(),wLen));
	TraceFrm("<-- CSocketIf::Send:", pbTxBuf, wLen);	
	if (m_Socket != INVALID_SOCKET)
	{
		int iReLen = 0;
		if (m_pSocketPara->fUdp)
		{
			struct  sockaddr_in to;
			to.sin_addr.s_addr = htonl(m_dwRemoteIP);
			to.sin_family = AF_INET;
			to.sin_port = htons(m_wRemotePort);	
			int tolen=sizeof(to);				
			iReLen = sendto(m_Socket, (char* )pbTxBuf, wLen, 0,(struct sockaddr *)&to,tolen);
		}
		else
		{
			iReLen = send(m_Socket, (char* )pbTxBuf, wLen, 0);
		}

		if (m_pSocketPara->fEnableFluxStat)	//�Ƿ���������ͳ��,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
			AddFlux(wLen);

		if (iReLen == wLen)
		{
			m_wGprsTxCnt++;
			//DTRACE(DB_FAPROTO, ("CSocketIf::Send : sock tx a %s frm, tx cnt=%d.\r\n", FapCmdToStr(pbTxBuf[FAP_CMD]), m_wGprsTxCnt));
			//PrintFrmCmd("CSocketIf::Send : sock tx a %s frm\n", pbTxBuf);
			return true;
		}
		else
		{
			DTRACE(DB_FAPROTO, ("CSocketIf::Send : sock tx fail iReLen=%d, wLen=%d.\r\n", iReLen, wLen));
			return false;
		}
	}
	else
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::Send : sock tx fail due to invalid sock.\r\n"));
		return false;
	}
	return false;
}

//����:���մ�����������,�������ѭ���������л�������,�򷵻�ѭ���������е�����,
//     ������ô��ڽ��պ���,ֱ�ӵȴ����ڵ����ݵ���
//����:@pbRxBuf �������շ������ݵĻ�����,
//     @wBufSize ���ջ������Ĵ�С
//����:�������ݵĳ���
WORD CSocketIf::Receive(BYTE* pbRxBuf, WORD wBufSize)
{
	int len = 0, i=0; 
	struct  sockaddr_in from; 
	i = sizeof(from);

	if (m_Socket != INVALID_SOCKET)
	{
		Sleep(100);
  		//SockSetLastError(m_Socket, 0);
  		if (m_pSocketPara->fUdp == 0x01)
			len =  recvfrom(m_Socket, (char*)pbRxBuf, wBufSize, 0,(struct sockaddr *)&from, (socklen_t* )&i);
		else
			len =  recv(m_Socket, (char *)pbRxBuf, wBufSize, 0);

	  	if (len == 0)
      	{
  	    	DTRACE(DB_FAPROTO, ("CSocketIf::Receive: close socket due to rx len=0.\r\n"));
	    	DisConnect();
            OnDisConnectByPeer();
	    	return 0;
      	}
      	else if (len == SOCKET_ERROR) 
	  	{
			int iLastErr =  SocketGetLastError(m_Socket);

	  		//DTRACE(DB_FAPROTO, ("CSocketIf::Receive: len==-1 errno=%s\n", strerror(errno)));
	  		//return 0;
			if (iLastErr == EWOULDBLOCK)   //TCP��socket��ĳ�ʱ��Ӧ�ó���Ӧִ���ش�
        	{               //Ӧ�ü���ֻҪͨ���������ܴ������ݣ�TCP�ͻ�ѽ��������ݸ��Է�
				SocketSetLastError(m_Socket, 0);
          		return 0;
			}
			else if (iLastErr == 0)
			{
				DTRACE(DB_FAPROTO, ("CSocketIf::Receive: len==-1 errno=%s\n", strerror(iLastErr)));
				return 0;
			}
			else
			{
	  			DTRACE(DB_FAPROTO, ("CSocketIf::Receive: len==-1 errno=%ld, %s\n", iLastErr, strerror(iLastErr)));
	    		DisConnect();
                OnDisConnectByPeer();
				SocketSetLastError(m_Socket, 0);
        		return 0;
			}
      	}
      	else
      	{
			if (m_pSocketPara->fEnableFluxStat)	//�Ƿ���������ͳ��,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
				AddFlux((DWORD )len);

      		DTRACE(DB_FAPROTO, ("CSocketIf::Receive:GetClick()=%d.\r\n",GetClick()));
      		TraceFrm("--> CSocketIf::Receive:", pbRxBuf, len);
      	}
	}
	else
	{
		Sleep(200);
	}

	return len; 
}

bool CSocketIf::SetSocketLed(bool fLight)
{
	if (m_pSocketPara->fUdp || !m_pSocketPara->fEnSocketLed)
		return true;
	
	//return SetSockLed(fLight);
	if (fLight)
		GprsWorkerSetSockLed(SOCK_LED_LIGHT);
	else
		GprsWorkerSetSockLed(SOCK_LED_DARK);

	return true;
}

//��������������
bool CSocketIf::Connect()
{
	struct  sockaddr_in local_addr;
	
	DisConnect();
	
	if (m_pSocketPara->fUdp == 0x01)
	{
		m_Socket = socket(PF_INET, SOCK_DGRAM, 0);
	}
	else
	{
		m_Socket = socket(PF_INET, SOCK_STREAM, 0);
	}
		
	if (m_Socket == INVALID_SOCKET)
	{
	  	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: fail to create socket.\r\n"));
		m_iLastErr = GPRS_ERR_CON;
	  	return false;
	}

	if (m_pSocketPara->fUdp == 0x01)
	{
		local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		local_addr.sin_family = AF_INET;
		local_addr.sin_port = htons(1024);
	
		if (bind(m_Socket, (struct sockaddr *)&local_addr, sizeof(local_addr)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf::Connect: Error: bind error\n"));
		  	Close();
			m_iLastErr = GPRS_ERR_CON;
		  	return false;
		}
	}
	
  	//����IP�л���
	//1����û�гɹ���½��վ������£�����ʹ����/��1/��2 IP
	//2���ڵ�½OK������£��嵱ǰIP��ʹ�ü������������Լ���ʹ��
	if (m_bBakIP > 2)
		m_bBakIP = 0;
	if (m_bBakIP == 0)	//��ǰҪʹ����IP
	{
		if (m_wIPUseCnt < m_pIfPara->wConnectNum && m_pSocketPara->dwRemoteIP!=0 && m_pSocketPara->wRemotePort!=0)//�� IP�˿���Ч
		{
	        m_wIPUseCnt++;
			m_dwRemoteIP = m_pSocketPara->dwRemoteIP;
			m_wRemotePort = m_pSocketPara->wRemotePort;
		}
        else
        {
        	m_dwRemoteIP = m_pSocketPara->dwBakIP1;
			m_wRemotePort = m_pSocketPara->wBakPort1;
            m_wIPUseCnt = 0;
           	m_bBakIP = 1; //���л�������1 IP
        }
	}
	if (m_bBakIP == 1)	//��ǰҪʹ�ñ���1IP
	{
		if (m_wIPUseCnt < m_pIfPara->wConnectNum && m_pSocketPara->dwBakIP1!=0 && m_pSocketPara->wBakPort1!=0)//����1 IP�˿���Ч
		{
	        m_wIPUseCnt++;
			m_dwRemoteIP = m_pSocketPara->dwBakIP1;
			m_wRemotePort = m_pSocketPara->wBakPort1;
		}
        else
        {
        	m_dwRemoteIP = m_pSocketPara->dwBakIP2;
			m_wRemotePort = m_pSocketPara->wBakPort2;
            m_wIPUseCnt = 0;
           	m_bBakIP = 2; //���л�������2 IP
        }
	}
	if (m_bBakIP == 2)	//��ǰҪʹ�ñ���2IP
	{
		if (m_wIPUseCnt < m_pIfPara->wConnectNum && m_pSocketPara->dwBakIP2!=0 && m_pSocketPara->wBakPort2!=0)//����2 IP�˿���Ч
		{
			m_wIPUseCnt++;
			m_dwRemoteIP = m_pSocketPara->dwBakIP2;
			m_wRemotePort = m_pSocketPara->wBakPort2;
		}
		else
        {
			m_dwRemoteIP = m_pSocketPara->dwRemoteIP;
			m_wRemotePort = m_pSocketPara->wRemotePort;        	
            m_wIPUseCnt = 1;   //�����Ѿ�ʹ�ù�һ����
            m_bBakIP = 0;  //�����л�����IP
        }
	}
	
	struct  sockaddr_in remote_addr;
	remote_addr.sin_addr.s_addr = htonl(m_dwRemoteIP);
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(m_wRemotePort);
	
	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: connecting %d.%d.%d.%d %d.\r\n",
						 (m_dwRemoteIP>>24)&0xff, (m_dwRemoteIP>>16)&0xff, (m_dwRemoteIP>>8)&0xff, m_dwRemoteIP&0xff, 
						 m_wRemotePort));
	
	if (m_pSocketPara->fUdp != 0x01)
	{
		if (connect(m_Socket, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) != 0)
    	{
    		DTRACE(DB_FAPROTO, ("CSocketIf::Connect: connect fail.\r\n"));
		  	Close();
	  		//UpdateErrRst(false);
			m_iLastErr = GPRS_ERR_CON;
		  	return false;
    	}
		DTRACE(DB_FAPROTO, ("CSocketIf::Connect: connect ok.\r\n"));	// UDP����ӡconnect ok
	}
	
	//UpdateErrRst(true);

#ifdef SYS_LINUX
	if (m_pSocketPara->fUdp!=0x01 && m_pSocketPara->fEnTcpKeepAlive) ////����TCP��keepalive����	
	{
		int keepAlive = 1; // ����keepalive����
		int keepIdle = m_pSocketPara->wKeepIdle; //���������wKeepIdle����û���κ���������,�����̽�� 
		int keepInterval = m_pSocketPara->wKeepInterv; //̽��ʱ������ʱ����ΪwKeepInterv�룬��20��
		int keepCount = m_pSocketPara->wKeepCnt; //̽�Ⳣ�ԵĴ���.�����1��̽������յ���Ӧ��,���2�εĲ��ٷ�.
	
		if (setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive, sizeof(keepAlive)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf: Set keepalive error: %s.\r\n", strerror(errno)));
			return false;
		}
		
		if (setsockopt(m_Socket, SOL_TCP, TCP_KEEPIDLE, (void*)&keepIdle, sizeof(keepIdle)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf: Set keepalive idle error: %s.\r\n", strerror(errno)));
			return false;
		}

		if (setsockopt(m_Socket, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf: Set keepalive interv error: %s.\r\n", strerror(errno)));
			return false;
		}
		
		if (setsockopt(m_Socket, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount, sizeof(keepCount)) != 0)
		{
			DTRACE(DB_FAPROTO, ("CSocketIf: Set keepalive count error: %s.\r\n", strerror(errno)));
			return false;
		}
			
		//�ڳ����б���Ϊ,��tcp��⵽�Զ�socket���ٿ���ʱ(���ܷ���̽���,��̽���û���յ�ACK����Ӧ��),select�᷵��socket�ɶ�,������recvʱ����-1,ͬʱ����errnoΪETIMEDOUT�� 
	}
#endif //SYS_LINUX
	
	//int timeout = 2000;
	//if (setsockopt(m_Socket, SOL_SOCKET, SO_RCVTIMEO, (char* )&timeout, sizeof(timeout)) != 0)
    //int optval = 1;
	//if (setsockopt(m_Socket, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval)) != 0) 
	unsigned int arg = 1;
	if (ioctlsocket(m_Socket, FIONBIO,  (ULONG* )&arg) != 0) //(u_long FAR*)
    {
    	DTRACE(DB_FAPROTO, ("CSocketIf::Connect: ioctl fail.\r\n"));
	  	Close();
		m_iLastErr = GPRS_ERR_CON;
	  	return false;
    }

	m_iLastErr = GPRS_ERR_OK;
	
	SetSocketLed(true);
  	return true;
}

//����:ȡ���Ӵ���,GPRS��socket���ӵ�ʱ��,����б���IP�˿ڵĻ�,���Ӵ�����2
WORD CSocketIf::GetConnectNum()
{
	WORD wConnectNum = m_pIfPara->wConnectNum;
	if (m_pSocketPara->dwBakIP1!=0 && m_pSocketPara->wBakPort1!=0)	//������վIP�Ͷ˿�
		wConnectNum += m_pIfPara->wConnectNum;
	if (m_pSocketPara->dwBakIP2!=0 && m_pSocketPara->wBakPort2!=0)
		wConnectNum += m_pIfPara->wConnectNum;
	return wConnectNum;
}


void CSocketIf::OnConnectFail()
{	
	//�������
	m_wConnectFailCnt++;
	
	//DTRACE(DB_FAPROTO, ("CProtoIf::OnConnectFail: m_wConnectFailCnt=%d, ConnectNum=%d\r\n",
	//					m_wConnectFailCnt, GetConnectNum()));	
	
	//��¼��ر���
	if (m_wConnectFailCnt >= GetConnectNum())
	{
		m_wConnectFailCnt = 0;
		if (m_pIfPara->dwDormanInterv != 0)
		{
			DTRACE(DB_FAPROTO, ("CProtoIf::OnConnectFail: go to dorman\n"));
			EnterDorman();
		}
		else
		{
			if (m_pSocketPara->bRandLoginFlg & RAND_ON_FAIL) //ʧ��ʱ���������ʱ
			{
				if (m_fRstInConnectFail)	//���ӿ�������ʧ�ܵ����Դ�����λ�ӿ�
				{
					DTRACE(DB_FAPROTO, ("CProtoIf::OnConnectFail: go to dorman!\n"));
					EnterDorman();
				}
				else
					StateToDorman(m_wState);	//��ĳ��״̬��ʱ�л�������״̬�������껹Ҫת����
			}
			else
			{
				//DTRACE(DB_FAPROTO, ("CProtoIf::OnConnectFail: go to rst\n"));
				Sleep(m_pIfPara->dwConnectInterv * 1000); //�ӿڵ����Ӽ��,��λ��
				if (m_fRstInConnectFail)	//���ӿ�������ʧ�ܵ����Դ�����λ�ӿ�
					m_wState = IF_STATE_RST; 
			}
		}
	}
	else
	{
		if (m_pSocketPara->bRandLoginFlg & RAND_ON_FAIL) //ʧ��ʱ���������ʱ
			StateToDorman(m_wState);	//��ĳ��״̬��ʱ�л�������״̬�������껹Ҫת����
		else
			Sleep(m_pIfPara->dwConnectInterv * 1000); //�ӿڵ����Ӽ��,��λ��
	}	
}

//����:��Э���½�ɹ�ʱ����
void CSocketIf::OnLoginOK()
{ 
	CProtoIf::OnLoginOK();
	
	m_wIPUseCnt = 0; //�ڵ�½OK������£��嵱ǰIP��ʹ�ü������������Լ���ʹ�ã�
					 //����������IPʹ����ͬ�Ĵ����������л�
}

//����:��Э���½ʧ��ʱ����,������ٴ�ʧ�ܺ�Ͽ�����
void CSocketIf::OnLoginFail() 
{	
	m_wLoginFailCnt++;

	m_iLastErr = GPRS_ERR_LOGIN;
	
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
		m_wIPUseCnt = m_pIfPara->wConnectNum;
		m_wLoginFailCnt = 0;
		DTRACE(DB_FAPROTO, ("CSocketIf::OnLoginFail: go to dorman\n"));
		EnterDorman();
	}
	else		//��¼ʧ�ܵĴ����ﵽ�˶Ͽ����ӵĴ�����������
	{			//m_wLoginFailCnt%wLoginRstNum==0 && m_wLoginFailCnt<wLoginRstNum*wLoginNum
		m_wIPUseCnt = m_pIfPara->wConnectNum;
		DisConnect();	//ֻ�Ͽ�����,�������¼ʧ�ܼ���,
						//�ﵽwLoginRstNum*wReTryNum�κ��������״̬
	}		
}

//������ȡ�����ӡ���¼ʧ�ܺ�������õ�����ʱ��(��������0.5-1.5��),��λ��
DWORD CSocketIf::GetRandDormanInterv()
{
	srand((unsigned)time(NULL));
	int iRand = rand() % 100 + 50; 
	return m_pSocketPara->dwBeatSeconds * iRand / 100;
}

//��������ĳ��״̬��ʱ�л�������״̬�������껹Ҫת����
void CSocketIf::StateToDorman(WORD wState)
{
	m_wDormanState = wState; //��ʱ���ߵ�״̬�������껹Ҫת�ص���״̬
	m_dwDormanInterv = GetRandDormanInterv();

	m_wState = IF_STATE_DORMAN;
	m_dwDormanClick = GetClick();	 //�������ߵĿ�ʼʱ��
	DTRACE(DB_FAPROTO, ("CSocketIf::StateToDorman : random login, old state=%d, m_dwDormanInterv=%ldS \n", 
						wState, m_dwDormanInterv));
}

void CSocketIf::EnterDorman()
{
	if ((m_wState==IF_STATE_CONNECT || m_wState==IF_STATE_LOGIN) && //ԭ����״̬�����ӻ��½
		 (m_pSocketPara->bRandLoginFlg & RAND_ON_DORMAN))	//���������½����Ҫ��Թ�����ÿ�ε�¼ʧ�ܺ󣬾�����������0.5-1.5���������ʱ����������ƣ������µ�¼
	{
		m_dwDormanInterv = GetRandDormanInterv();
		DTRACE(DB_FAPROTO, ("CSocketIf::EnterDorman : random login, dwDormanInterv=%ldS \n", m_dwDormanInterv));
	}

	CProtoIf::EnterDorman();
    m_wDisConnectByPeerCnt = 0;
}

void CSocketIf::OnDisConnectByPeer()
{
    m_wDisConnectByPeerCnt++;
    if (m_pSocketPara->wDisConnectByPeerNum!=0 && m_wDisConnectByPeerCnt>=m_pSocketPara->wDisConnectByPeerNum) 
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::OnDisConnectByPeer: go to dorman\n"));
        EnterDorman();
	}
}

//����:�ӿڱ���̽��
void CSocketIf::KeepAlive()
{	
	DWORD dwBeatSeconds = GetBeatSeconds();
	if (dwBeatSeconds == 0) //!m_pSocketPara->fBeatOn
		return;
	
	DWORD dwClick = GetClick();
	DWORD dwBrokenTime = dwBeatSeconds + 
					 	 m_pSocketPara->dwBeatTimeouts*m_pSocketPara->wBeatTestTimes;
	if (m_pSocketPara->wBeatTestTimes!=0 && dwClick-m_dwRxClick>dwBrokenTime)
	{	//m_pSocketPara->wBeatTestTimes����Ϊ0,��ʾ���Զ�����,ֻ���ڷ�����
		DisConnect(); //���³�ʼ��
		DTRACE(DB_FAPROTO, ("CSocketIf::KeepAlive : DisConnect at click %d\n", dwClick));
	}
	else if (dwClick-m_dwRxClick > dwBeatSeconds)//40 20�뻹û�յ���һ֡��������������
	{	//�տ�ʼʱdwBeatClickΪ0��dwNewClick��Ϊ0�������Ϸ�
		if (dwClick-m_dwBeatClick > m_pSocketPara->dwBeatTimeouts)
		{							//������ʱʱ��,��λ��
			m_pProto->Beat();
			m_dwBeatClick = dwClick;
			DTRACE(DB_FAPROTO, ("CSocketIf::KeepAlive: heart beat test at click %d\n", dwClick));
		}
	}
}


bool CSocketIf::Close()
{
	if (m_Socket != INVALID_SOCKET)
	{
#ifndef SYS_WIN
		unsigned int arg = 0;
		ioctlsocket(m_Socket, FIONBIO,  (ULONG* )&arg);//����Ϊ������		
		arg = 1;
		setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(int));
		//Ҫ�Ѿ���������״̬��soket�ڵ���closesocket��ǿ�ƹرգ�ʡ��CLOSE_WAIT�Ĺ��̣�
		struct linger so_linger;
		so_linger.l_onoff = 1;
		so_linger.l_linger = 0;
		setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger));
#endif
	  	closesocket(m_Socket);
	  	m_Socket = INVALID_SOCKET;
	  	DTRACE(DB_FAPROTO, ("CSocketIf::Close: close socket at click %d\n", GetClick()));
	}
	
	return true;
}


//����:�ڽӿ�������תΪ�Ͽ���ʱ����ã������������Ͽ����Ǳ����Ͽ�
bool CSocketIf::DisConnect()
{
	if (m_Socket != INVALID_SOCKET) //�Ͽ�socket����
	{
		SetSocketLed(false);
		
		m_pProto->OnBroken();
		Close();		
		//closesocket(m_Socket);
	  	m_Socket = INVALID_SOCKET;
	}
	
	CProtoIf::DisConnect();
	
	if (m_pSocketPara->fSvr && m_pSocketPara->bSvrDisconMode==SVR_DISCON_EXIT)
	{				//������ģʽ��,socket�Ͽ�ʱҪ���˳��߳�
					//�Ͽ����Ӻ��ڿ���ģʽSVR_DISCON_IDLE���ô���
					//��Ϊ��ʱ��ֻ���ȶϿ����ӣ���Ҫ����������ȥ
		m_fExit = true;
	}
	
	return true;
}


//����:��һЩ�����ӿ���صķǱ�׼������,
void CSocketIf::DoIfRelated()
{
	if (m_fSetIdleCmd)	//�յ��ⲿ�ĶϿ���������
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::DoIfRelated : %s rx set idle cmd.\r\n", GetName()));

		DisConnect();	//�ȶϿ�socket��ʽ�µ�����,��Ȼ�л�������ģʽ��û������

		m_fSetIdleCmd = false;

		m_dwDormanClick = 0; //û�й涨���ߵ�ʱ��,�൱�ڴ��������ڵ�����(����)״̬,
		m_wState = IF_STATE_DORMAN;
	}

	if (m_fDisConnCmd)	//�յ��ⲿ�ĶϿ���������
	{
		DTRACE(DB_FAPROTO, ("CSocketIf::DoIfRelated : %s rx disconnect cmd.\r\n", GetName()));

		DisConnect();
    	if (m_pSocketPara->fSvr) //������ģʽ��,socket�Ͽ�ʱ�˳��߳�
        {
		    m_dwDormanClick = 0; //û�й涨���ߵ�ʱ��,�൱�ڴ��������ڵ�����(����)״̬,
			m_wState = IF_STATE_DORMAN;
        }
        
        m_fDisConnCmd = false;
	}
	
	CProtoIf::DoIfRelated();
}
