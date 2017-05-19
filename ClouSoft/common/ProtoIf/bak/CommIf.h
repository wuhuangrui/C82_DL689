#ifndef COMMIF_H
#define COMMIF_H
#include "ProtoIf.h"
#include "Comm.h"

typedef struct{
	TIfPara IfPara;
	TCommPara CommPara; //��������
	CComm* pComm;		//�����ΪNULL,����ô��ڹ���ģʽ
						//���������,���ڲ�����CommPara����
	bool fMutual;		//�Ƿ�ʹ�ù���ģʽ,�������Ͳ���232����һ������,�����ʱ����Ҫ���д����л�
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
	bool SetUnrstPara(void *pvPara);	//װ�طǸ�λ����
	bool GetUnrstPara(void *pvPara);	//��÷Ǹ�λ����
	//�麯��
	
    virtual bool Send(BYTE* pbTxBuf, WORD wLen);
    virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
	virtual bool Close();
	virtual void LoadUnrstPara() { };
protected:
	
};

#endif  //COMMIF_H


