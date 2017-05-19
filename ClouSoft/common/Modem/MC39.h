#ifndef FAPMC39_H
#define FAPMC39_H
#include "Modem.h"

class CMC39 : public CModem
{
public:
    CMC39(WORD wHardVer);
    virtual ~CMC39();
  	bool GprsPoweroff(void);
	
protected:
	WORD m_wHardVer;
	bool m_fPowOn;	
	WORD m_wModemTemp;
	int m_nRstIgnoreCnt;
	//Ðéº¯Êý
	bool InitGPRS();	
	int InitAPN();
	int ResetModem();
	bool ResetEmbededIP();
	bool ResetSMS();
	bool GetTemperature();
	BYTE* RxSmsHead(BYTE* bBuf, WORD wLen);	
};


#endif  //FAPMC39_H









