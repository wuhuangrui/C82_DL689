/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：R230M.h
 * 摘    要：本文件主要实现对230M电台通信接口的封装
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年7月
 * 备    注：
 *********************************************************************************************************/
#ifndef R230M_H
#define R230M_H
#include "ProtoIf.h"
#include "CommIf.h"
#include "RSCoder.h"

typedef struct{
	TCommIfPara CommIfPara;
	DWORD		dwRadioDelay;	//电台延时
	BYTE        bRSCoder;//是否需要RS编码 0-不使用，1-上海编码，2-江苏编码
	bool        fNeedCtrl;//是否需要控制电台
	bool		fHeadCtrl;//是否需要添加同步头
	BYTE		bRecvCnt;
}TR230mIfPara;

class CR230mIf : public CCommIf
{
public:
    CR230mIf();
    virtual ~CR230mIf();
    
	bool Init(TR230mIfPara* pR230mIfPara);
	void SetUnrstParaFunc(bool (*pfnUnrstPara)(TR230mIfPara* pR230mIfPara))
	{		//注册装载非复位参数的函数
		m_pfnLoadUnrstPara = pfnUnrstPara;
	}
	
	//虚函数
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


