#ifndef SOCKETIF_H
#define SOCKETIF_H
#include "syssock.h" 
#include "ProtoIf.h"

//�������Ͽ�����ģʽ��
#define SVR_DISCON_EXIT		0	//�Ͽ����Ӻ��˳��߳�
#define SVR_DISCON_IDLE		1	//�Ͽ����Ӻ��ڿ���ģʽ

#define RAND_ON_DORMAN			0x01	//����ʱ���������ʱ
#define RAND_ON_FAIL			0x02	//ʧ��ʱ���������ʱ

typedef struct{
	TIfPara IfPara;
	bool 	fSvr;			//�Ƿ��Ƿ�����ģʽ
	bool 	fUdp;			//�Ƿ���UDPͨ�ŷ�ʽ
	BYTE  	bRandLoginFlg;	//�����½���Ʊ�־λ:BIT0�Ƿ����������ʱ��BIT1�Ƿ�ʧ�������ʱ������λ����
							//��Ҫ��Թ�����ÿ�ε�¼ʧ�ܺ󣬾�����������0.5-1.5���������ʱ����������ƣ������µ�¼
	BYTE	bSvrDisconMode;	//�������Ͽ�����ģʽ��
	DWORD 	dwRemoteIP;		//��վIP
	WORD 	wRemotePort;	//��վ�˿�
	DWORD 	dwBakIP1;		//������վIP
	WORD 	wBakPort1;		//������վ�˿�
	DWORD 	dwBakIP2;		//������վIP
	WORD 	wBakPort2;		//������վ�˿�
	
	DWORD 	dwLocalIP;
	WORD 	wLocalPort;
	bool	fEnableFluxStat;	//�Ƿ�������������,ֻ�б�socket�õ���GPRSͨ��ʱ��֧��
	bool	fEnSocketLed;		//�Ƿ����Socket Led,ֻ��Թ�����׼ģ��
	
	WORD 	wDisConnectByPeerNum;	//���Է��Ͽ����ӣ��л�������״̬�Ĵ���
	
	bool 	fEnTcpKeepAlive; 		//�Ƿ���TCP��keepalive����
	WORD 	wKeepIdle; 				//���������wKeepIdle����û���κ���������,�����̽�� 
	WORD 	wKeepInterv; 			//̽��ʱ������ʱ����ΪwKeepInterv�룬��20��
	WORD 	wKeepCnt; 				//̽�Ⳣ�ԵĴ���.�����1��̽������յ���Ӧ��,���2�εĲ��ٷ�.
	
	//�Ǹ�λ����
	WORD  wBeatTestTimes;	//�������Դ���,Ϊ0,��ʾ���Զ�����,ֻ���ڷ�����		
	DWORD dwBeatSeconds;	//�������
	DWORD dwBeatTimeouts;	//������ʱʱ��,��λ��
}TSocketPara;

class CSocketIf : public CProtoIf
{
public:
    CSocketIf();
    virtual ~CSocketIf();

	TSocketPara* m_pSocketPara;
	
    bool Init(TSocketPara* pSocketPara);
	void SetUnrstParaFunc(bool (*pfnUnrstPara)(TSocketPara* pPara))
	{		//ע��װ�طǸ�λ�����ĺ���
		m_pfnLoadUnrstPara = pfnUnrstPara;
	}
	
	void InitSvr(int socket);
	void SetMaxIpUseCnt() { m_wIPUseCnt = m_pIfPara->wConnectNum;  };

	//�麯��
	virtual void KeepAlive();
	virtual bool Send(BYTE* pbTxBuf, WORD wLen);
    virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
    virtual void LoadUnrstPara();
    virtual bool Connect();
    virtual bool Close();
	virtual bool DisConnect();
	virtual DWORD GetBeatSeconds() { return m_pSocketPara->dwBeatSeconds; };
	virtual WORD GetConnectNum();	//ȡ���Ӵ���,GPRS��socket���ӵ�ʱ��,����б���IP�˿ڵĻ�,���Ӵ�����2
	virtual void EnterDorman();
    virtual void OnDisConnectByPeer();
	virtual void OnConnectFail();
	virtual void OnLoginOK();
	virtual void OnLoginFail();
	virtual bool IsIfValid() { return m_Socket != INVALID_SOCKET; };
    virtual void DoIfRelated(); 
    
protected:
	bool (*m_pfnLoadUnrstPara)(TSocketPara* pPara);
	
	int  m_Socket;
	DWORD m_dwRemoteIP;
	WORD  m_wRemotePort;
	BYTE m_bMasterAddr[9];
	BYTE m_bBakIP;		// ��ǰʹ�õ�IP	0--��IP,  1--��1,  2--��2
	WORD m_wIPUseCnt;	//��ǰIPʹ�ô���
	WORD m_wDisConnectByPeerCnt;

	//ͳ������
	WORD m_wGprsTxCnt;
	WORD m_wGprsRxCnt;
	WORD m_wSmsTxCnt;
	WORD m_wSmsRxCnt;

	DWORD m_dwErrRstClick;

	bool m_fServerMode;
	BYTE m_bMasterIP[4];
	WORD m_wMasterPort;
	//BYTE m_bLocalIP[4];
	WORD m_wLocalPort;
	char m_szAPN[32];
	char m_szPppUser[64];
	char m_szPppPsw[64];
	
	char* m_pszCSQ;
	char* m_pszRxSms;
	char* m_pszRxSmsHead;
	char* m_pszATDT;
	
	void PutToLoopBuf(BYTE* pbBuf, WORD wLen);
	void RxToLoopBuf();
	WORD RxFromLoopBuf(BYTE* pbRxBuf, WORD wBufSize);
	bool InitSock();
	bool LoadPara();
	DWORD GetRandDormanInterv();
	void StateToDorman(WORD wState);
	bool SetSocketLed(bool fLight);
};


#endif  //SOCKETIF_H




