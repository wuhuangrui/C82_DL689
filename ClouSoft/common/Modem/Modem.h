/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Modem.h
 * ժ    Ҫ�����ļ�ʵ����ͨ��MODEM�Ļ��ඨ��
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע������ΪGPRS,CDMA�͵绰MODEM�Ļ���
 *********************************************************************************************************/
#ifndef MODEM_H
#define MODEM_H
#include "DrvConst.h"
#include "Comm.h"
#include "drivers.h"
#include "LibDbStruct.h"
#include "FaConst.h"

#define GSN_LEN    		 15
#define PHONE_NUM_LEN    32
#define ACTIVE_SMS_LEN   32

#define MODEM_STEP_INIT	 0	//��ʼ��
#define MODEM_STEP_RST	 1	//��λģ��
#define MODEM_STEP_SIM	 2	//���SIM��
#define MODEM_STEP_REG	 3	//ע������

#define MODEM_NO_ERROR	 0	//û�д���
#define MODEM_RST_FAIL	 1	//ģ�鸴λʧ��
#define MODEM_SIM_FAIL	 2  //���SIMʧ��
#define	MODEM_REG_FAIL	 3	//ע������ʧ��

typedef struct {
	//CComm*  pComm;
	char szAPN[64];
	
	bool fEnSms;			//�Ƿ��������
	bool fMstSmsAddrValid;   //��վ���ŵ�ַ��Ч��־
	BYTE bDeftSmsc[32];
	BYTE bDeftSmscLen;
	BYTE bDeftMstSmsAddr[32];
	BYTE bDeftMstSmsAddrLen;
	BYTE bMstAddr[9];
	char szMstPhone[32];
	
	//�������
	char szActiveSms[ACTIVE_SMS_LEN];
	char szActiveNumber[PHONE_NUM_LEN];
	WORD wDormantInterv;
	bool fSetXgauth;//�ѷ�ģ���Ƿ���Ҫ�����û���������
}TModemPara;  

//LAC��С����CELL_ID��Ƶ���ARFCN�����չ���Rx level�����书��Rx level
typedef struct {
	DWORD dwCellID;	// 16����
	WORD wLac;		// 16����
	WORD wArfCN;
	WORD wRxLev;	// ��λ:dBm
	WORD wTxPwr;	// ��λ:dBm
} TNetInfo;

typedef struct {
	BYTE bManuftr[4];	//���̴���	ASCII	4
	BYTE bmodel[8];		//ģ���ͺ�	ASCII	8
	BYTE bSoftVer[4];	//����汾��	ASCII	4
	BYTE bSoftDate[6];	//����������ڣ�������	����¼A.20	3
	BYTE bHardVer[4];	//Ӳ���汾��	ASCII	4
	BYTE bHardDate[6];	//Ӳ���������ڣ�������	����¼A.20	3
	BYTE bCCID[20];					//�ӣɣͿ�ICCID	ASCII	20
	BYTE bCNUM[8];		//��������
	BYTE bSysmode;
	BYTE bMnc;
}TModemInfo;	//ģ����Ϣ

class CModem
{
public:
    CModem();
    virtual ~CModem();
  	
  	TModemPara*	m_pModemPara;
  	
  	char* GetATDT() { return m_pszATDT; };  	
  	void SetComm(CComm* pComm) { m_pComm = pComm; };
  	bool Init(TModemPara* pModemPara);
	bool AvoidSmsOverflow(void);
	int ATCommand(char* pszCmd, char* pszAnsOK, char* pszAnsErr1=NULL, char* pszAnsErr2=NULL, WORD nWaitSeconds=0);
	int ATCommand(char* pszCmd, const char** ppszAns, WORD wNum, WORD nWaitSeconds);
  	bool HaveRing() //�Ƿ��յ��������ź�
  	{ 
  		if (m_fHaveRing) { m_fHaveRing = false; return true; }
  		else return false;
  	}
	bool CheckActive();
	WORD GetModemStep() { return m_wModemStep; };
	WORD GetErrBytes() {return m_wPreErrByte;};
	bool EnMux();
	//bool GetSYSINFO();
	//bool RcvSYSINFO();
	bool UpdateMyTime();
	//�麯��
	virtual bool Connect(bool fUdp, DWORD dwRemoteIP, WORD wRemotePort) { return false; };
	virtual int InitAPN(char *cAPN){ return MODEM_NO_ERROR; };
	virtual bool TestModem() { return false; };
	virtual int ResetModem(){ return MODEM_NO_ERROR; };
	virtual bool ResetEmbed() { return false; };
	virtual bool ResetSMS(){ return false; };
	virtual bool ConnectEmbed(){ return false; };
	virtual bool GprsPoweroff(void){ return false; };
	virtual int RxSmsFrm(BYTE* pbSms, WORD wSmsLen, BYTE* pbSmsc, BYTE* pbSmscLen, BYTE* pbSender, BYTE* pbSenderLen, BYTE* pbFrm);
    virtual bool SendSms(BYTE* pbTxBuf, WORD wLen);
    virtual bool SendTextSms(BYTE* pbSmsAddr, BYTE bSmsAddrLen, BYTE* pbTxBuf, WORD wLen);
	virtual bool SendEmbed(BYTE* pbTxBuf, WORD wLen) { return false; };
	virtual WORD ReceiveEmbed(BYTE* pbRxBuf, WORD wBufSize) { return 0; };
    virtual WORD ReceiveSms(BYTE* pbRxBuf, WORD wBufSize, bool fQuerySms);
	virtual BYTE* RxSmsHead(BYTE* bBuf, WORD wLen){ return 0; };
#ifdef SYS_LINUX
	virtual void PowerOff() { ModemPowerOff(); };
	virtual void PowerOn() { ModemPowerOn(); };
#else
	virtual void PowerOff() { ; };
	virtual void PowerOn() { ; };
#endif
	BYTE GetModuleType() { return m_wModuleType; };
	BYTE GetModulemRegion(){ return m_wModuleregion; };
	virtual int UpdateSignStrength();
	virtual bool IsSignValid(WORD wSignStrength);// { return m_fSignValid; };
	virtual WORD Send(BYTE* pbTxBuf, WORD wLen) { return wLen; };
	virtual int Receive(BYTE* pbRxBuf, WORD wBufSize) { return 0; }
	virtual bool Close() { return true; };
	virtual bool DisConnect() { return true; };
	virtual bool PPPOpen(char* pszNum, char* pszName, char* pszPass) { return true; };
	virtual bool PPPClose() { return true; };
	//virtual bool GetCID(TDataItem diCID) { return true; };
	virtual int GetNetInfo(TNetInfo *pNetInfo) { return 0; };
    virtual bool GetIMSI(TDataItem diIMSI);
    virtual bool GetGMR(TDataItem diCGMR);	
    virtual bool GetGPS(TDataItem diGPS) { return true; };
	virtual bool GetSYSINFO(){ return true; };
    DWORD GetRxSmsClick();
    int GetRxSmsAddr(unsigned char* pbBuf);
    bool GetSmscNumber();
	//virtual bool SetMYNETINFO(BYTE bNetType);
protected:
	CComm*  m_pComm;
	
	WORD  m_wModuleType;
	WORD  m_wModuleregion;
	BYTE  m_bGSN[GSN_LEN];
	//WORD  m_wSignStrength;
	char* m_pszCSQ;
	char* m_pszRxSms;
	char* m_pszRxSmsHead;
	char* m_pszATDT;
	bool m_fChnUnicom;//�й���ͨ
	bool m_fChnCdma;//�й����� ������ʼ��cdma�Ķ���
	
	DWORD m_dwRxSmsClick;  	 //���յ����ŵ�ʱ��,Ϊ0ʱ��ʾû�ڱ��η���ǰû�յ�������
	BYTE  m_bRxSmsc[32];	 //���յ����ŵĶ������ĺ���	
	BYTE  m_bRxSmscLen;		 //���յ����ŵĶ������ĺ���ĳ���	
	BYTE  m_bRxMasterSmsAddr[32]; //���յ����ŵĵ绰����
	BYTE  m_bRxMasterSmsAddrLen;  //���յ����ŵĵ绰����ĳ���
	WORD  m_wPreErrByte; //GPRS������

	DWORD m_dwSmsTxClick;
	bool m_fQuerySms;		//�Ƿ��Բ�ѯ�ķ�ʽ���ն���
	bool m_fHaveRing;		//�յ�����
	
	bool m_fSmsDelAllSupported;  //֧�ֶ���ȫ��ɾ��ָ��
	
	char m_szRingNumber[PHONE_NUM_LEN+1];
	//bool m_fSignValid;
	WORD m_wModemStep;
	TNetInfo m_tNetInfo[16];
	
	int WaitModemAnswer(char* pszAnsOK, char* pszAnsErr1, char* pszAnsErr2, WORD nWaitSeconds);
	int WaitModemAnswer(const char** ppszAns, WORD wNum, WORD nWaitSeconds);
	bool ATCmdTest(WORD wTimes);
	WORD MakeSMS(BYTE* pbSmsc, BYTE bSmscLen, BYTE* pbDest, BYTE bDestLen, BYTE* pbFrm, BYTE bFrmLen, BYTE* pbSms, BYTE* pbCmgsLen, BYTE bDCS=0x04);
	int RxSmsPdu(WORD wNO, BYTE* pbBuf);
	void DeleteSms(WORD wMaxNO);
	WORD HandleSms(BYTE* pbInfo, WORD wInfoLen, BYTE* pbRxBuf);
	WORD HandleSms(WORD wNo, BYTE* pbRxBuf);
	void GetRingNumber(BYTE* pbInfo, WORD wLen);
};


#endif  //MODEM_H



