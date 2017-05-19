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
	TCommPara CommPara; //��������
	CComm* pComm;		//�����ΪNULL,����ô��ڹ���ģʽ
						//���������,���ڲ�����CommPara����
	bool fMutual;		//�Ƿ�ʹ�ù���ģʽ,�������Ͳ���232����һ������,�����ʱ����Ҫ���д����л�
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
	//WORD GetParaCnMode() { return m_pGprsPara->wCnMode; }	//ȡ��ͨ��ģʽ�����ò���ֵ
	//bool Close();
	
	//void SetUnrstParaFunc(bool (*pfnUnrstPara)(TGprsPara* pGprsPara))
	//{		//ע��װ�طǸ�λ�����ĺ���
	//	m_pfnLoadUnrstPara = pfnUnrstPara;
	//}
	virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
	virtual bool Send(BYTE* pbTxBuf, WORD wLen);
protected:
	//bool (*m_pfnLoadUnrstPara)(TGprsPara* pGprsPara);
};

#endif  //GPRSIF_H