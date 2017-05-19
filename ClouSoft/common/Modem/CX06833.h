#ifndef FAPCX06833_H
#define FAPCX06833_H
#include "Modem.h"

class CCX06833 : public CModem
{
public:
    CCX06833(WORD wModuleIdx = 0);
    virtual ~CCX06833();    
  	bool GprsPoweroff(void);
	
protected:
	WORD m_wModuleIdx;
	bool m_fPowOn;	
	WORD m_wModemTemp;
	int m_nRstIgnoreCnt;
	//Ðéº¯Êý
	bool InitGPRS();	
	int InitAPN();
	int ResetModem();
	bool TestModem();
	bool ResetEmbededIP();
	bool ResetSMS();
	BYTE* RxSmsHead(BYTE* bBuf, WORD wLen);	
	int UpdateSignStrength();
};

#endif  //FAPCX06833_H
