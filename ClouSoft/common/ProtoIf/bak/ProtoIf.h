/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�PotoIf.h
 * ժ    Ҫ�����ļ�ʵ����ͨ�Žӿڻ��ඨ��
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע��
 *********************************************************************************************************/
#ifndef PROTOIF_H
#define PROTOIF_H

#include "apptypedef.h"
#include "sysarch.h"
#include "Proto.h"

#define SOCK_MAX_BYTES      1024  
#define EMBED_MAX_BYTES     800       
#define COMM_MAX_BYTES      512	
#define SMS_MAX_BYTES       140
#define ETHER_MAX_BYTES     1024
#define PPP_MAX_BYTES       1024

class CProto;

typedef struct{
	char* pszName;			//�ӿ�����
	bool fNeedLogin;		//�Ƿ���Ҫ��¼
	WORD wMaxFrmBytes; 		//�ӿڵ�һ֡������ֽ���,��ͬЭ����ܹ涨��һ��
	DWORD dwRstInterv;		//�ӿڵĸ�λ���,��λ��
	DWORD dwConnectInterv;	//�ӿڵ����Ӽ��,��λ��
	WORD  wConnectNum;		//����ʧ���������ԵĴ���
	WORD  wLoginRstNum; 	//��¼ʧ�ܶϿ����ӵĴ���
	WORD  wLoginNum; 		//��¼ʧ���������ԵĴ���
	DWORD dwLoginInterv; 	//��¼���,��λ��
	WORD wRstNum;           //��λ����
	
	WORD wReSendNum;		//�ط�����

	DWORD dwNoRxRstAppInterv; //�޽��ո�λ�ն˼��,��λ��,0��ʾ����λ
	DWORD dwNoRxRstIfInterv;  //�޽��ո�λ�ӿڼ��,��λ��,0��ʾ����λ

	//���������Ĳ���
	WORD  wReTryNum;  		//��λ�����������ԵĴ���
	DWORD dwDormanInterv;	//����ʱ����, ��λ��, ,0��ʾ��ֹ����ģʽ

	//��Ϣ�궨��,�����������п��õ��ĺ궨���Ӧ�ó���һ��
	WORD wInfoActive;		//INFO_ACTIVE
	WORD wInfoAppRst;		//INFO_APP_RST
}TIfPara;

//�ӿ�����
#define IF_UNKNOWN		0
#define IF_GPRS         1
#define IF_COMM     	2
#define IF_SOCKET       3	//���ڲ���ϵͳsocket�׽���
#define IF_R230M		4	//230M��̨
#define IF_P2P			5	//ר��Modem
#define IF_SMS			6	//����ģʽ

#define IF_RST_OK  		0  	//��λ�ɹ�
#define IF_RST_HARDFAIL 1	//Ӳ��λʧ��
#define IF_RST_SOFTFAIL 2	//��λʧ��(Э���)

//�ӿ�״̬��,�ӿڵ�״̬�л�: (����)->(��λ)->(����)->(��¼)->(����)
#define IF_STATE_DORMAN  	0 //����
#define IF_STATE_RST  		1 //��λ
#define IF_STATE_CONNECT 	2 //����
#define IF_STATE_LOGIN  	3 //��¼
#define IF_STATE_TRANS  	4 //����	

#define IF_DEBUG_INTERV		(2*60)	//��������ļ��,��λ��
#define DATA_SRC_SMS		1
class CProtoIf
{
public:
    CProtoIf();
    virtual ~CProtoIf();
    
    //��������
    WORD m_wIfType;
    CProto* m_pProto;
    TIfPara* m_pIfPara;
    bool m_fExit;
    bool m_fExitDone;
	bool m_fUnrstParaChg;  	//�Ǹ�λ���������ı�
	bool m_fNeedActive;		//��Ҫ����
	WORD m_wRunCnt;         //���д���
	
	WORD  m_wCnMode;
	
	//��������
	void Init();
	char* GetName();
	WORD GetIfType() { return m_wIfType; };
	CProto* GetProto() { return m_pProto; };	
    bool CanTrans();// { return m_wState>IF_STATE_CONNECT && m_wState<=IF_STATE_TRANS; }; //�ӿ��Ƿ񻹴��ڴ���״̬
    WORD GetState() { return m_wState; };
    bool IsInDorman() { return m_dwDormanClick!=0; };
    void DoDorman();
	int	 GetLastErr() { return m_iLastErr; };
	DWORD GetWakeUpTime(void);
	void AttachProto(CProto* pProto) { m_pProto = pProto; };
			//��Э��
	bool Init(TIfPara* pIfPara);
			//�ӿڳ�ʼ��,�����Э�齨�����ӡ�������Դ�������ڴ��	
	void SetActive() { m_fNeedActive = true; };
			//���ýӿڴ��ڼ���״̬,���ڷ��������ߵĶ��ż���
	void InitRun();
	
	void SetDisConnect(DWORD dwDormanInterv=0); //���յ��ⲿ�ĶϿ���������ʱ,���ñ�����֪ͨ�ӿ�
	void SetIdle();		  //���յ��ⲿ�Ĵ��ڿ���״̬������ʱ,���ñ�����֪ͨ�ӿ�

	WORD GetCnMode() { return m_wCnMode; };

	DWORD GetRxClick() { return m_dwRxClick; };           //���һ�ν��յ����ĵ�ʱ��

    //�ӿڶ���
    //virtual bool Init(void* pvArg) = 0;
    virtual void AutoSend();
    virtual WORD GetMaxFrmBytes() { return m_pIfPara->wMaxFrmBytes; };
    virtual bool Send(BYTE* pbTxBuf, WORD wLen) = 0;
    virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize) = 0;
    virtual void KeepAlive() { };
    		//�ӿڱ���̽��,�����������
	virtual void LoadUnrstPara() { };	//װ�طǸ�λ����
	virtual bool IsNeedLogin() { return m_pIfPara->fNeedLogin; };
			//���ӿڵ�ͨ��Э���Ƿ���Ҫ��¼
    virtual bool Connect() { return true; };
    		//��������,����򿪴��ڻ��߽���socket���ӵ�
    virtual bool DisConnect();
    		//�Ͽ�����,�����������Ͽ����Ǳ����Ͽ����ɵ���,����رմ��ڻ��߶Ͽ�socket���ӵ�
    virtual int ResetIf(){ return IF_RST_OK; };
    		//��λ�ӿ�,���縴λPPP���Ӻ�MODEM��
    virtual bool RequestOffline() { return true; };
    virtual void EnterDorman();
    		//ʹ�ӿڽ������߷�ʽ
    virtual bool Close() { return true; };
			//�رսӿ�,�����ͷ���Դ��
	virtual void DoIfRelated(); 
			//��һЩ����ģ����صķǱ�׼������,������������߷�ʽ��,GPRS��SMS����л�
	virtual WORD GetConnectNum() { return m_pIfPara->wConnectNum; };
			//ȡ���Ӵ���,GPRS��socket���ӵ�ʱ��,����б���IP�˿ڵĻ�,���Ӵ�����2
			
    //�ص�����
	virtual void OnConnectOK();
			//�ڽӿ��ɶϿ�תΪ���ӵ�ʱ�����
	virtual void OnConnectFail();
			//�ڽӿ�����ʧ��ʱ����,������ٴ�ʧ�ܺ�λ�ӿ�
	virtual	void OnResetFail(); 
			//�ڽӿڸ�λʧ��ʱ����,������ٴ�ʧ�ܺ��������
	virtual	void OnResetOK();
			//�ڽӿڸ�λ�ɹ�ʱ����
	virtual void OnLoginFail();
			//��Э���½ʧ��ʱ����,������ٴ�ʧ�ܺ�Ͽ�����			
	virtual void OnLoginOK();
			//��Э���½�ɹ�ʱ����
	virtual void OnRcvFrm();
			//��ͨ��Э���յ���ȷ֡ʱ����,��Ҫ������·״̬,����������
	
	virtual bool SetUnrstPara(void *pvPara) { return false;};	//װ�طǸ�λ����
	virtual bool GetUnrstPara(void *pvPara) { return false;};	//��÷Ǹ�λ����
	virtual bool IsIfValid() { return false; };
protected:
	
	//��������
	WORD  m_wState;				 //�ӿ�״̬��
	WORD  m_wResetFailCnt;		 //��λʧ�ܴ���
	WORD  m_wConnectFailCnt;     //����ʧ�ܴ���
	WORD  m_wLoginFailCnt;       //��½ʧ�ܴ���
	DWORD m_dwRxClick;           //���һ�ν��յ����ĵ�ʱ��
	DWORD m_dwBeatClick;         //���һ�η����������ĵ�ʱ��
	
	DWORD m_dwDormanClick;		 //�������ߵĿ�ʼʱ��
	bool  m_fRstInConnectFail;	 //���ӿ�������ʧ�ܵ����Դ�����λ�ӿ�	
	DWORD m_dwDebugClick;
	int	  m_iLastErr;
	DWORD m_dwRstIfClick;		 //��¼�ӿ��ϴθ�λʱ�̻��߽��յ����ĵ�ʱ��
	WORD  m_wDormanState; 		 //��ʱ���ߵ�״̬�������껹Ҫת�ص���״̬

	bool m_fDisConnCmd;			//�յ��ⲿ�ĶϿ���������
	bool m_fSetIdleCmd;			//�յ��ⲿ�Ĵ��ڿ���״̬������
	DWORD m_dwDormanInterv;		//��̬�趨�����߼������λ��
	BYTE  m_bGprsDataSrc;	//GPRSͨ��ʱ��1:������ԴΪ���ţ���������Դsocket
};

#endif //PROTOIF_H
 
