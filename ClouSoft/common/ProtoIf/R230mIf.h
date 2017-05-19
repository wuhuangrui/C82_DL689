/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�R230M.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֶ�230M��̨ͨ�Žӿڵķ�װ
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��7��
 * ��    ע��
 *********************************************************************************************************/
#ifndef R230M_H
#define R230M_H
#include "ProtoIf.h"
#include "CommIf.h"
#include "RSCoder.h"

typedef struct{
	TCommIfPara CommIfPara;
	DWORD		dwRadioDelay;	//��̨��ʱ
	BYTE        bRSCoder;//�Ƿ���ҪRS���� 0-��ʹ�ã�1-�Ϻ����룬2-���ձ���
	bool        fNeedCtrl;//�Ƿ���Ҫ���Ƶ�̨
	bool		fHeadCtrl;//�Ƿ���Ҫ���ͬ��ͷ
	BYTE		bRecvCnt;
}TR230mIfPara;

class CR230mIf : public CCommIf
{
public:
    CR230mIf();
    virtual ~CR230mIf();
    
	bool Init(TR230mIfPara* pR230mIfPara);
	void SetUnrstParaFunc(bool (*pfnUnrstPara)(TR230mIfPara* pR230mIfPara))
	{		//ע��װ�طǸ�λ�����ĺ���
		m_pfnLoadUnrstPara = pfnUnrstPara;
	}
	
	//�麯��
    virtual bool Send(BYTE* pbTxBuf, WORD wLen);
    virtual WORD Receive(BYTE* pbRxBuf, WORD wBufSize);
    virtual void LoadUnrstPara();
    
protected:
	TR230mIfPara* m_pR230mIfPara;
	CRSCoder* m_pRSCoder;
	bool (*m_pfnLoadUnrstPara)(TR230mIfPara* pR230mIfPara);
	
private:
	bool m_fRSCoded;
};

#endif  //R230M_H


