#ifndef SMSIF_H
#define SMSIF_H
#include "ProtoIf.h"
#include "SocketIf.h"
#include "Modem.h"
#include "GprsWorker.h"
#include "EmbedGprsIf.h"
//#include "GprsIf.h"

typedef struct{
	TIfPara IfPara;
	TCommPara CommPara; //串口配置
	CComm* pComm;		//如果不为NULL,则采用串口共享模式
						//否则独享串口,串口参数由CommPara传入
	bool fMutual;		//是否使用共用模式,比如红外和测试232共用一个串口,输出的时候需要进行串口切换
}TSmsIfPara;


class CSmsIf : public CSocketIf
{	
public:
    CSmsIf();
    virtual ~CSmsIf();

	//CModem* m_pModem;
	TSmsIfPara* m_pSmsIfPara;
    
	bool Init(TSmsIfPara* pSmsIfPara);
	//void LoadUnrstPara();
	//WORD GetParaCnMode() { return m_pGprsPara->wCnMode; }	//取得通道模式的设置参数值
	//bool Close();
	
	//void SetUnrstParaFunc(bool (*pfnUnrstPara)(TGprsPara* pGprsPara))
	//{		//注册装载非复位参数的函数
	//	m_pfnLoadUnrstPara = pfnUnrstPara;
	//}
	virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
	virtual bool Send(BYTE* pbTxBuf, WORD wLen);
protected:
	//bool (*m_pfnLoadUnrstPara)(TGprsPara* pGprsPara);
};

#endif  //GPRSIF_H