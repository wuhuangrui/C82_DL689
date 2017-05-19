#ifndef FAPMG815_H
#define FAPMG815_H
#include "Modem.h"

class CMG815 : public CModem
{
public:
    CMG815(WORD wModuleVer);
    virtual ~CMG815();
    
    bool Connect(bool fUdp, DWORD dwRemoteIP, WORD wRemotePort);
    WORD Send(BYTE* pbTxBuf, WORD wLen);
	bool GprsPoweroff(void);
	int Receive(BYTE* pbRxBuf, WORD wBufSize);
	bool Close();
	bool DisConnect();
	bool PPPOpen(char* pszNum, char* pszName, char* pszPass);
	bool PPPClose();

protected:
	bool  m_fCmdMode;
	bool  m_fPowOn;
	bool  m_fUdp;

	//Ðéº¯Êý
	bool InitGPRS();	
	int InitAPN() { return MODEM_NO_ERROR; };
	int ResetModem();
	bool ResetEmbededIP();
	bool ResetSMS();
	int  RxSmsFrm(BYTE* pbSms, WORD wSmsLen, BYTE* pbSmsc, BYTE* pbSmscLen, BYTE* pbSender, BYTE* pbSenderLen, BYTE* pbFrm); 
	bool SendSms(BYTE* pbTxBuf, WORD wLen);
};


#endif  //FAPMG815_H









 