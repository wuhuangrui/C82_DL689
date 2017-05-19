#ifndef FAPGR47_H
#define FAPGR47_H
#include "Modem.h"


class CGR47 : public CModem
{
public:
    CGR47(WORD wHardVer);
    virtual ~CGR47();
	bool UpdateSignStrengthEx();
	bool SetDataMode();
	bool SetCmdMode();
	bool PeekSms();
	
protected:
	bool m_fCmdMode;
	WORD m_wHardVer;
	int m_nRstIgnoreCnt;

	//Ðéº¯Êý
	bool ConnectEmbed(){return false; };
	int InitAPN();
	int ResetModem();
	bool ResetEmbed();
	bool ResetSMS();
};


#endif  //FAPGR47_H
