#ifndef GPRSIF_H
#define GPRSIF_H
#include "ProtoIf.h"
#include "SocketIf.h"
#include "Modem.h"
#include "GprsWorker.h"
#include "EmbedGprsIf.h"

//ͨ����ͨ��ģʽ
#define CN_MODE_SOCKET      0	//����TCP/IP��ͨ��ģʽ
#define CN_MODE_SMS      	1	//����
#define CN_MODE_EMBED     	2	//ģ��Ƕ��ʽЭ��ջ
#define CN_MODE_COMM     	3	//����ͨ��ģʽ
#define CN_MODE_CMD     	4	//����ģʽ
#define CN_MODE_ETHSCK      5	//��̫��SOCKETͨ��ģʽ

#define CN_MODE_NUM      	6

//����ģʽ
#define ONLINE_M_PERSIST    1	//��������ģʽ
#define ONLINE_M_ACTIVE     2	//����ģʽ/����������ģʽ
#define ONLINE_M_PERIOD		3	//ʱ������ģʽ
#define ONLINE_M_SMS		4   //���ŷ�ʽ
#define ONLINE_M_JIT		5	//JUST IN TIME ����Ҫ��ʱ����,�絥�����ϱ��˿�
#define ONLINE_M_DMINSMS	6   //����ʱ�������ģʽ

#define ONLINE_MODE			0
#define NONONLINE_MODE		1


class CGprsIf : public CSocketIf
{
public:
    CGprsIf();
    virtual ~CGprsIf();

	//CModem* m_pModem;
	TGprsPara* m_pGprsPara;
    
	bool Init(TGprsPara* pGprsPara);
	bool ReInit(TGprsPara* pGprsPara);
	bool ResetCnMode(WORD wCnMode); //�����й����п�����������ͨ��ģʽ��������GPRS socket/ģ��Э��ջ����̫�����л�
	void LoadUnrstPara();
	WORD GetParaCnMode() { return m_pGprsPara->wCnMode; }	//ȡ��ͨ��ģʽ�����ò���ֵ
	
	void SetUnrstParaFunc(bool (*pfnUnrstPara)(TGprsPara* pGprsPara))
	{		//ע��װ�طǸ�λ�����ĺ���
		m_pfnLoadUnrstPara = pfnUnrstPara;
	}
	
	//�麯��
    virtual WORD GetMaxFrmBytes();
    virtual int ResetIf();
    virtual bool RequestOffline();
	virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
	virtual bool Send(BYTE* pbTxBuf, WORD wLen);
	virtual bool Connect();
	virtual	bool DisConnect();
	virtual	void EnterDorman();
    virtual void OnDisConnectByPeer();
	virtual	bool Close();
	virtual	void OnResetFail(); 
	virtual	void OnConnectFail();
	virtual void KeepAlive();
	virtual bool IsNeedLogin();
	virtual DWORD GetBeatSeconds();
	virtual void OnLoginOK();
	virtual void OnLoginFail();
	virtual void DoIfRelated();
	virtual void OnRcvFrm();
	WORD SignStrength() { return m_wSignStrength;};
    CEmbedGprsIf* GetEmbedGprsIf();
	
protected:
	CEmbedGprsIf m_embdGprsIf;
	bool (*m_pfnLoadUnrstPara)(TGprsPara* pGprsPara);
	
	//CComm  m_Comm;
	//int    m_iPd;	//ppp���豸��,ֻ��lwip���õ�
	int m_iGprsUser;
	
	WORD  m_wBaseCnType;  //����ͨ������,�Ӷ��ŷ�ʽ�л���ȥʱ���ص���ͨ������
	BYTE  m_bRstMode;	  //GPRSģ��ĸ�λģʽ,��λ���ڿ���״̬���Ƕ���״̬

	//ͳ������
	WORD m_wGprsTxCnt;
	WORD m_wGprsRxCnt;
	WORD m_wSmsTxCnt;
	WORD m_wSmsRxCnt;

	DWORD m_dwErrRstClick;
	DWORD m_dwSignClick;
	DWORD m_dwSmsOverflowClick;
	
	DWORD m_dwPeriodDropInterv; //ʱ������ģʽ�ļ��ʽ�Զ�����ʱ��,��λ����
	DWORD m_dwFluxOverClick;	//�����������ʼʱ��

	WORD  m_wSignStrength;

	BYTE GetGprsRstMode();
	bool CheckActivation();
	//void DoIfRelated();
	char* CnModeToStr(WORD wMode);
	char* OnlineModeToStr(WORD wMode);
};

#endif  //GPRSIF_H


