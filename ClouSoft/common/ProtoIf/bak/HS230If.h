#ifndef HS230IF_H
#define HS230IF_H
#include "ProtoIf.h"
#include "Comm.h"

typedef struct{
	TIfPara IfPara;
	TCommPara CommPara; //´®¿ÚÅäÖÃ 
	DWORD dwAddr;	
	BYTE bChn;
	WORD wPow;
}THS230IfPara;

class CHS230If : public CProtoIf
{
public:
    CComm m_Comm;
	THS230IfPara* m_pHS230IfPara;
    CHS230If();
    virtual ~CHS230If();
	
	//Ðéº¯Êý
	virtual bool Init(THS230IfPara* pHS230IfPara);
	virtual int ResetIf();
    virtual bool Send(BYTE* pbTxBuf, WORD wLen);
    virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
	virtual bool Close();
	
private:
	char m_cCSQ;
	int ATCommand(BYTE* pbCmd, WORD wCmdLen, BYTE* pszAnsOK, WORD wAnsLen, WORD nWaitSeconds=0);
	
protected:
public:
	char GetCSQ() { return m_cCSQ; };
    void OnResetFail();
    void OnResetOK();	
    void KeepAlive();
};

#endif  //HS230IF_H


