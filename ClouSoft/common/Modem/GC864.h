#ifndef FAPGC864_H
#define FAPGC864_H
#include "Modem.h"

class CGC864 : public CModem
{
public:
    CGC864(WORD wModuleVer, WORD wModuleArea);
    virtual ~CGC864();
    WORD Send(BYTE* pbTxBuf, WORD wLen);
	bool GprsPoweroff(void);
    bool Connect(bool fUdp, DWORD dwRemoteIP, WORD wRemotePort);
	int Receive(BYTE* pbRxBuf, WORD wBufSize);
	bool Close();
	bool DisConnect();
	bool PPPOpen(char* pszNum, char* pszName, char* pszPass);
	bool PPPClose();
	int GetNetInfo(TNetInfo *pNetInfo);
	bool GetGPS(TDataItem diGPS);
	bool GetSYSINFO();
protected:
	bool m_fUdp;
	bool m_fPowOn;
	bool m_fGmrOk;
	WORD m_wModuleVer;
	WORD m_wModuleArea;
	WORD m_wBaud;
	DWORD m_dwRstClick;
	TModemInfo m_ModemInfo;	
	//bool m_fChnUnicom;//中国联通
	//bool m_fChnCdma;//中国电信 用来初始化cdma的短信
	//虚函数
	bool InitGPRS();
	int InitAPN(char *cAPN);
	int ResetModem();
	bool ResetEmbededIP();
	bool ResetSMS();
	bool ResetCdmaSMS();
	bool GetCCID();
	bool GetCNUM();
	bool GetGMR();
	bool GetCIMI();
	bool GetMYMODEM();
	bool SetMYURCSYSINFO();
	bool SetMYNETINFO();
	bool m_fGetGps;
			
	BYTE* RxSmsHead(BYTE* bBuf, WORD wLen);
};


#endif  //FAPGC864_H









 