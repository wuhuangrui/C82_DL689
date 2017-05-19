#ifndef CSTDMODEM_H
#define CSTDMODEM_H
#include "Modem.h"

class CSTDMODEM : public CModem
{
public:
    CSTDMODEM(WORD wModuleVer,WORD wModuleArea, WORD wId);
    virtual ~CSTDMODEM();
    WORD Send(BYTE* pbTxBuf, WORD wLen);
	bool GprsPoweroff(void);
    bool Connect(bool fUdp, DWORD dwRemoteIP, WORD wRemotePort);
	int Receive(BYTE* pbRxBuf, WORD wBufSize);
	bool Close();
	bool DisConnect();
	
	bool PPPOpen(char* pszNum, char* pszName, char* pszPass);
	bool PPPClose();
	int GetNetInfo(TNetInfo *pNetInfo);
	bool GetMODEMIP(char* pszCmd, char* pszAnsOk, WORD nWaitSeconds);
	
protected:
	bool  m_fUdp;
	bool m_fPowOn;
	WORD m_wModuleVer;
	WORD m_wModuleArea;
	WORD m_wBaud;
	WORD m_wIpId;
	DWORD m_dwRstClick;
	TModemInfo m_ModemInfo;	
	
	//Ðéº¯Êý
	bool InitGPRS();
	int InitAPN();
	int ResetModem();
	bool ResetEmbededIP();
	bool ResetSMS();
	bool ResetCdmaSMS();
	bool GetCCID();
	bool GetGMR();
	bool GetCNUM();
	bool GetCIMI();
			
	BYTE* RxSmsHead(BYTE* bBuf, WORD wLen);
	int GetAck();
};


#endif  //FAPGC864_H









 