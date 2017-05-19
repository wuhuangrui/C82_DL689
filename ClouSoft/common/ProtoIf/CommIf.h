#ifndef COMMIF_H
#define COMMIF_H
#include "ProtoIf.h"
#include "Comm.h"

typedef struct{
	TIfPara IfPara;
	TCommPara CommPara; //串口配置
	CComm* pComm;		//如果不为NULL,则采用串口共享模式
						//否则独享串口,串口参数由CommPara传入
	bool fMutual;		//是否使用共用模式,比如红外和测试232共用一个串口,输出的时候需要进行串口切换
}TCommIfPara;

class CCommIf : public CProtoIf
{
public:
    CComm m_Comm;
    CComm* m_pComm;
	TCommIfPara* m_pCommIfPara;
	
    CCommIf();
    virtual ~CCommIf();
	
	bool Init(TCommIfPara* pCommIfPara);
	bool SetUnrstPara(void *pvPara);	//装载非复位参数
	bool GetUnrstPara(void *pvPara);	//获得非复位参数
	//虚函数
	
    virtual bool Send(BYTE* pbTxBuf, WORD wLen);
    virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
	virtual bool Close();
	virtual void LoadUnrstPara() { };
protected:
	
};

#endif  //COMMIF_H


