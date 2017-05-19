#ifndef FAPLC6311_H
#define FAPLC6311_H
#include "Modem.h"

class CL6311 : public CModem
{
public:
    CL6311(WORD wModuleVer);
    virtual ~CL6311();
  	bool GprsPoweroff(void);
	
protected:
	WORD m_wModuleVer;
	bool m_fPowOn;	
	WORD m_wModemTemp;
	int m_nRstIgnoreCnt;
	//Ðéº¯Êý
	bool InitGPRS();	
	int InitAPN();
	int ResetModem();
	bool ResetEmbededIP();
	bool ResetSMS();
	BYTE* RxSmsHead(BYTE* bBuf, WORD wLen);	
};


#endif  //FAPLC6311_H









