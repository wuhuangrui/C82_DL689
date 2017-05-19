#ifndef FAPMC2106_H
#define FAPMC2106_H
#include "Modem.h"

class CMC2106 : public CModem
{
public:
    CMC2106();
    virtual ~CMC2106();   
	bool GprsPoweroff(void);
	
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
	bool IsReged();
};


#endif  //FAPMG815_H









 