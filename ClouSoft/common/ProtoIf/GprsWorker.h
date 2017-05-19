/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�GprsWorker.h
 * ժ    Ҫ�����ļ���Ҫʵ��GPRS�����߳�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��8��
 * ��    ע��
 *********************************************************************************************************/
#ifndef GPRSWORKER_H
#define GPRSWORKER_H
#include "Modem.h"
#include "FrmQue.h"
#include "LibDbStruct.h"

//GPRS�ӿڵ�״̬
#define GPRS_STATE_IDLE		0
#define GPRS_STATE_OL		1
#define GPRS_STATE_SMS		2

#define GPRS_RST_ON_IDLE	0
#define GPRS_RST_ON_SMS		1

#define SOCK_LED_IDLE		0
#define SOCK_LED_LIGHT		1
#define SOCK_LED_DARK		2

typedef struct{
	TModemPara	ModemPara;
	TCommPara CommPara; //��������

	WORD  wConnectWait;	    	//���ӵȴ�ʱ��
	WORD  wRstNum;
	WORD  wDormanToPwroffCnt;	//����ʧ�ܽ�������״̬�ﵽ��ô��κ��ģ���Դ
	bool  fEnableRingActive; 	 //�������弤��
	bool  fEnableAutoSendActive; //���������ϱ�����
	bool  fEnSocketLed;		//�Ƿ����Socket Led,ֻ��Թ�����׼ģ��	
	bool  fEnMux;			//�Ƿ������ڸ���
	
	//��Ϣ�궨��,�����������п��õ��ĺ궨���Ӧ�ó���һ��
	WORD wInfoActive;		//INFO_ACTIVE

	char szPppUser[64];
	char szPppPsw[64];
	bool fEmbedProtocol;	//�Ƿ�ʹ��ģ��Э��ջ
	bool fDetailDiagnose;	//�Ƿ������ϸ���
	//TDataItem diCID;		//����SIM�����к�
	TNetInfo *ptNetInfo;	//��ȡ������Ϣ
	//TDataItem diIMSI;		//����IMSI
	//TDataItem diCGMR;		//����ģ������汾��
	TDataItem diGPS;		//����GPS״̬��Ϣ
	WORD wUpdTxPwrInterv;	//���·��书�ʵļ������λ�룬Ϊ0��ʾ������
}TGprsWorkerPara;

class CGprsWorker
{
public:
    CGprsWorker();
    virtual ~CGprsWorker();

	CModem* m_pModem;
	TGprsWorkerPara* m_pWorkerPara;
    
	bool Init(TGprsWorkerPara* pWorkerPara);
	void AttachModem(CModem* pModem) { m_pModem = pModem; } ;
	void LoadUnrstPara();
	void SetUnrstParaFunc(bool (*pfnUnrstPara)(TGprsWorkerPara* pWorkerPara))
	{		//ע��װ�طǸ�λ�����ĺ���
		m_pfnLoadUnrstPara = pfnUnrstPara;
	}
	
	WORD GetState() { return m_wState; };
    DWORD GetOnlineReqFlg() { return m_dwOnlineReqFlg;};
	bool IsOnline() { return m_wState==GPRS_STATE_OL; };
	bool IsKeepOnline() { return m_dwOnlineReqFlg != 0; };
	int  GetLastErr(void) { return m_iLastErr; }
	int  ReqUserID();
	void ReleaseUserID(int iUser);
	void ReqOnline(int iUser);
	void ReqOffline(int iUser);
	void ReqDorman(int iUser);
	void SetWorkMode(int iUser, BYTE bRstOnSms);
	void RunThread();
	void SetSockLedAct(BYTE bAct) { m_bLedAct = bAct;}	//0��ֵ��������1��ʾ������2��ʾϨ��
	
	WORD SignStrength() { return m_wSignStrength; };
	WORD GetErrBytes(); //{ return m_pModem->GetErrBytes(); };
	int GetPppPd() { return m_iPd; };
	
	WORD ReceiveSms(BYTE* pbFrm) { return m_SmsRxFrmQue.Remove(pbFrm, 100); };
	bool SendSms(BYTE* pbFrm, WORD wLen) { return m_SmsTxFrmQue.Append(pbFrm, wLen, 2000); };
	bool SendTextSms(BYTE* pbSmsAddr, BYTE bSmsAddrLen, BYTE* pbTxBuf, WORD wLen);
	CModem* GetModem() { return m_pModem; };
	//bool SetNetInfo(BYTE bNetType);
	int GetGprsWorkLastErr();
	WORD GetGprsWorkStep();
	
protected:
	bool (*m_pfnLoadUnrstPara)(TGprsWorkerPara* pWorkerPara);
	int	   m_iLastErr;//��һ�δ���
	CComm  m_Comm;
	int    m_iPd;	//ppp���豸��,ֻ��lwip���õ�
	WORD m_wSignStrength;
	TSem  m_semWorker;
	TSem  m_semWorkerLocker;
	WORD m_wState;
	bool m_fModemPwrOn;
	WORD m_wFailCnt;		//����ʧ�ܼ���
	WORD m_wDormanCnt;	//���û������������״̬����
	
	DWORD m_dwUser;
	DWORD m_dwOnlineReqFlg;	//��ӦλΪ1��ʾ���û�Ҫ����
	DWORD m_dwRstOnSmsFlag;	//��ӦλΪ1��ʾ���û����ߺ�Ҫ���ڶ���״̬
	
	DWORD m_dwDebugClick;
	DWORD m_dwSmsOverflowClick;
	DWORD m_dwSignClick;
	DWORD m_dwUpdTxPwrClick;
	DWORD m_dwMuxClick;	
	DWORD m_dwUpdSysInfoClick;

	CFrmQue m_SmsRxFrmQue;	//���Ž��յ�֡����
	CFrmQue m_SmsTxFrmQue;	//���ŷ��͵�֡����
	
	WORD m_wGprsWorkStep;
	BYTE m_bLedAct;

	//ƽ̨��صĴ���
	bool ClosePpp(void);
	bool OpenPpp(void);
	bool UpdateSignStrength();
	int ResetGprs(WORD wCnMode);
	bool ReadTxPwrAndSign(BYTE* pbTxPwr, int16* pbSign);
	void UpdTxPwr();
	void UpdSysInfo();
	void DoLed();

	void ResetToOfflineState();
	void ResetToOnlineState();

	void DoIdleState();
	void DoOnlineState();
	void DoSmsState();	
	bool DoOnlineSms();
	int ResetModem(bool fModemPwrOn, WORD wCnMode);
};

int ReqOnline(int iGprsUser);
bool ReqOffline(int iGprsUser);
bool ReqDorman(int iUser);
WORD GetSignStrength(void);
int GetGprsWorkLastErr();
WORD GetGprsWorkStep();
CGprsWorker* GetGprsWorker();
void GprsWorkerSetSockLed(BYTE bAct);

TThreadRet GprsWorkerThread(void* pvArg);

#endif  //GPRSWORKER_H
