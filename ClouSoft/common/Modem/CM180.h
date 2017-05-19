#ifndef FAPCM180_H
#define FAPCM180_H
#include "Modem.h"

class CCM180 : public CModem
{
public:
    CCM180();
    virtual ~CCM180();   
	bool GprsPoweroff(void);
	
	int GetNetInfo(TNetInfo *pNetInfo);

protected:
	bool  m_fPowOn;

	//Ðéº¯Êý
	bool InitGPRS();	
	int InitAPN();
	int ResetModem();
	bool ResetEmbededIP();
	bool ResetSMS();
	int  RxSmsFrm(BYTE* pbSms, WORD wSmsLen, BYTE* pbSmsc, BYTE* pbSmscLen, BYTE* pbSender, BYTE* pbSenderLen, BYTE* pbFrm); 
	bool SendSms(BYTE* pbTxBuf, WORD wLen);
};


#endif  //FAPMG815_H









 