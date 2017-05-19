#ifndef FAPWAVECOM_H
#define FAPWAVECOM_H
#include "Modem.h"

class CWavecom : public CModem
{
public:
    CWavecom();
    virtual ~CWavecom();
	bool GprsPoweroff(void);
			
protected:
	bool  m_fCmdMode;
	int m_nRstIgnoreCnt;

	//Ðéº¯Êý
	bool InitGPRS();	
	int InitAPN() { return MODEM_NO_ERROR; };
	int ResetModem();
	bool ResetEmbededIP();
	bool ResetSMS();
	int  RxSmsFrm(BYTE* pbSms, WORD wSmsLen, BYTE* pbSmsc, BYTE* pbSmscLen, BYTE* pbSender, BYTE* pbSenderLen, BYTE* pbFrm); 
	bool SendSms(BYTE* pbTxBuf, WORD wLen);
};


#endif  //FAPWAVECOM_H









 