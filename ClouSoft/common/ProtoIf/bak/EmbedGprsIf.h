#ifndef EMBEDGPRSIF_H
#define EMBEDGPRSIF_H
#include "syssock.h" 
#include "ProtoIf.h"
#include "SocketIf.h"
#include "Modem.h"


typedef struct{
	//CModem* pModem;  //MODEM�ڴ�������ʱ���Ѿ������˱�Ҫ�ĳ�ʼ����ֻ�ǲ��û��ʼ����
	TSocketPara SocketPara;
	//TModemPara	ModemPara;
	//TCommPara CommPara; //��������
	
	WORD  wCnMode;	//ͨ��ģʽ,���ŷ�ʽ������ΪCN_MODE_SMS,
					//GPRS��ʽ������ΪCN_MODE_SOCKET��CN_MODE_EMBED,
					//���������߷�ʽ����ָGPRS��ͨ��ģʽ,����ΪCN_MODE_SOCKET��CN_MODE_EMBED,
	
	WORD  wSmsMaxFrmBytes;	//���ŷ�ʽ�µ����֡����,������ʽ��֡���Ȼ���ȡIfPara.wMaxFrmBytes
	WORD  wRstNum;
	bool  fEnableRingActive; 	 //�������弤��
	bool  fEnableAutoSendActive; //���������ϱ�����
	DWORD dwActiveDropInterv; 	 //����������ģʽ���Զ�����ʱ��,��λ����
	DWORD dwPowerupDropInterv; 	 //�ϵ缤����Զ�����ʱ��,��λ����
								 //��Ϊ0�Զ�ȡ���ϵ缤�� 	
	DWORD dwPowerupBeatMinutes;	 //�ϵ缤����������

	BYTE bOnlineMode;	//����ģʽ
	//BYTE bOnlinePeriod[24];
		//��ģ�鹤����ʱ�����߷�ʽ
		//ÿ��λ����������ʾ��30����ʱ���ڵ��������ֵ����λ��5���ӣ���
		//�ӵ��ֽڵ����ֽ����α�ʾ48��ʱ�ε�����ʱ������
	bool fEnableFluxCtrl;	//�Ƿ�������������
	bool fRstOnSms;			//�Ƿ�λ������ģʽ����Ҫ��Լ���ģʽ��ʱ������ģʽ
	//char szPppUser[64];
	//char szPppPsw[64];
}TGprsPara;


class CEmbedGprsIf : public CProtoIf
{
public:
    CEmbedGprsIf();
    virtual ~CEmbedGprsIf();

	CModem* m_pModem;
	TGprsPara* m_pTEmbedGprsPara;
	
    bool Init(TGprsPara* pTGprsPara);
	void SetUnrstParaFunc(bool (*pfnUnrstPara)(TGprsPara* pPara))
	{		//ע��װ�طǸ�λ�����ĺ���
		m_pfnLoadUnrstPara = pfnUnrstPara;
	}
	
	void InitSvr(int socket);
	void ResetIPUseCnt() {m_wIPUseCnt = 0;};
	void SetMaxIpUseCnt() { m_wIPUseCnt = m_pIfPara->wConnectNum; };
	
	//�麯��
	virtual void KeepAlive();
	virtual bool Send(BYTE* pbTxBuf, WORD wLen);
    virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
    virtual void LoadUnrstPara();
    virtual bool Connect();
    virtual bool Close();
	virtual bool DisConnect();
	virtual DWORD GetBeatSeconds() { return m_pTEmbedGprsPara->SocketPara.dwBeatSeconds; };
	virtual WORD GetConnectNum();	//ȡ���Ӵ���,GPRS��socket���ӵ�ʱ��,����б���IP�˿ڵĻ�,���Ӵ�����2
	virtual void EnterDorman();
	virtual bool IsIfValid();
	virtual void OnRcvFrm();
	virtual void OnConnectFail();
	virtual void OnLoginOK();
	//virtual void OnConnectOK();
		
protected:
	bool (*m_pfnLoadUnrstPara)(TGprsPara* pPara);
	
	bool  m_fIfValid;
	DWORD m_dwRemoteIP;
	WORD  m_wRemotePort;
	BYTE m_bMasterAddr[9];
	bool m_fBakIP;
	WORD m_wIPUseCnt;	//��ǰIPʹ�ô���
	
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
};


#endif  //EMBEDGPRSIF_H




