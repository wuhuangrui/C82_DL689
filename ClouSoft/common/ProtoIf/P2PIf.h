#ifndef P2PIF_H
#define P2PIF_H
#include "ProtoIf.h"
#include "Comm.h"
#include "Modem.h"

typedef struct{
	TIfPara IfPara;
	TCommPara CommPara; //´®¿ÚÅäÖÃ 
	TModemPara	ModemPara;
}TP2PIfPara;

class CP2PIf : public CProtoIf
{
public:
    CComm m_Comm;
    CModem* m_pModem;
	TP2PIfPara* m_pP2PIfPara;
	void AttachModem(CModem* pModem) { m_pModem = pModem; } ;
    CP2PIf();
    virtual ~CP2PIf();
	
	//Ðéº¯Êý
	virtual bool Init(TP2PIfPara* pP2PIfPara);
	virtual int ResetIf();
    virtual bool Send(BYTE* pbTxBuf, WORD wLen);
    virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
	virtual bool Close();
	
protected:
public:
    void OnResetFail();
    void OnResetOK();	
    void KeepAlive();
};

#endif  //P2PIF_H


