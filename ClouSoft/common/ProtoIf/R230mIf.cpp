/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：R230M.cpp
 * 摘    要：本文件主要实现对230M电台通信接口的封装
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年7月
 * 备    注：
 *********************************************************************************************************/
#include "stdafx.h"
#include "R230mIf.h"
#include "drivers.h"
#include "sysapi.h"
#include "Trace.h"
#include "RSCoderSH.h"
#include "RSCoderJS.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//CR230mIf

CR230mIf::CR230mIf()
{
	//CCommIf::CCommIf();
	m_wIfType = IF_R230M;
	m_pR230mIfPara = NULL;
	m_pRSCoder = NULL;
	m_fRSCoded = false;
}

CR230mIf::~CR230mIf()
{
	if (m_pRSCoder != NULL)
	{
		delete m_pRSCoder;
		m_pRSCoder = NULL;
	}
}

bool CR230mIf::Init(TR230mIfPara* pR230mIfPara)
{
	if (!CCommIf::Init(&pR230mIfPara->CommIfPara)) //基类的初始化
		return false;
	
	m_Comm.SetTimeouts(150);
	
	m_pR230mIfPara = pR230mIfPara;
	if (m_pR230mIfPara->bRSCoder == 1)
		m_pRSCoder = new CRSCoderSH();
	else if (m_pR230mIfPara->bRSCoder == 2)	
		m_pRSCoder = new CRSCoderJS();
		
	return true;
}

void CR230mIf::LoadUnrstPara()
{
	if (m_pfnLoadUnrstPara != NULL)
	{
		(*m_pfnLoadUnrstPara)(m_pR230mIfPara);
	}	
}

bool CR230mIf::Send(BYTE* pbTxBuf, WORD wLen)
{
	//return m_pComm->Write(pbTxBuf, wLen) == wLen;
	
	BYTE bBuf[2048];
	int iLen = 0;
	if (m_pR230mIfPara->fHeadCtrl)
	{
		iLen = 7;
		bBuf[0] = 0xaa;
		bBuf[1] = 0xaa;
		bBuf[2] = 0xaa;
		bBuf[3] = 0xaa;
		bBuf[4] = 0x7e;
		bBuf[5] = 0x7e;
		bBuf[6] = 0x7e;
	}

	memcpy(bBuf+iLen, pbTxBuf, wLen);

	if (m_pR230mIfPara->bRSCoder!=0 && m_pRSCoder!=NULL && m_fRSCoded)
		iLen += m_pRSCoder->Pack(pbTxBuf, wLen, bBuf+iLen);
	else
		iLen += wLen;

	if (m_pR230mIfPara->fNeedCtrl)
	{
		DWORD dwTick = GetTick();
		while (R230mIsBusy())
		{
			if (GetTick()-dwTick > 10*1000) 
			{
					return false;
			}
	
			Sleep(50);
		}

		R230MRequestSend();
	
		R230mLightTxLed(iLen);
	}

	if (m_pR230mIfPara->dwRadioDelay != 0)
		Sleep(m_pR230mIfPara->dwRadioDelay);	//电台延时

	TraceFrm("<-- C230MIf::Send:", bBuf, iLen);
	int iSndLen = m_pComm->Write(bBuf, iLen);

	if (m_pR230mIfPara->fNeedCtrl)
	{
#ifdef SYS_LINUX
		Sleep(iLen*10+100);
#endif
		R230MEndSend();
	}

	return iSndLen == iLen;
}


WORD CR230mIf::Receive(BYTE* pbRxBuf, WORD wBufSize)
{
	int i = 0;
	BYTE bBuf[2048];	
	int iLen = m_Comm.Read(bBuf, wBufSize,1000);
	int iHead = -1;
	int iRxNothingCnt = 0;
	DWORD dwLen;
	int iTmpLen = 0;
	BYTE bTmpBuf[2048];
	int iRetLen = 0;
	
	if (iLen > 0)
	{
		while (1)
		{
			//不用支持RS编码，可以快点返回，因为这个时候一个报文分两次接收没有关系
			if (m_pR230mIfPara->bRSCoder==0 || m_pRSCoder==NULL)
				dwLen = m_Comm.Read(bBuf+iLen, wBufSize-iLen, 100);
			else
				dwLen = m_Comm.Read(bBuf+iLen, wBufSize-iLen, 300);
			if (dwLen > 0)
			{
				DTRACE(DB_FAPROTO,("try Receive %dth time %d BYTEs\n", i, dwLen));
				iLen += dwLen;
				iRxNothingCnt = 0;
			}
			else
			{
				iRxNothingCnt++;
			}
			i++;
			if (iRxNothingCnt >= m_pR230mIfPara->bRecvCnt)
				break;
		}		
	}
	else
	{
		return 0;
	}
	
	if (iLen <= 0)
		return 0;
		
	if (m_pR230mIfPara->fNeedCtrl)
		R230mLightRxLed(iLen);
	
	if (m_pR230mIfPara->bRSCoder==0 || m_pRSCoder==NULL)
	{
		memcpy(pbRxBuf, bBuf, iLen);
		TraceFrm("--> C230MIf::Receive:", bBuf, iLen);
		return iLen;
	}

	for (i=0; i<iLen; i++)
	{
		if (bBuf[i]==0x68 || bBuf[i]==0x10)
		{
			iHead = i;
			break;
		}
	}
		
	iRetLen = m_pRSCoder->UnPack(bBuf+iHead, iLen-iHead, pbRxBuf);	
	if (iRetLen > 0)
	{		
		//看下测试是否使用RS编码
		if (m_pR230mIfPara->bRSCoder!=0 && m_pRSCoder!=NULL)
		{
			iTmpLen = m_pRSCoder->Pack(pbRxBuf, iRetLen, bTmpBuf);
			if (iTmpLen >= 15)
			{
				if (memcmp(bTmpBuf, bBuf+iHead, 15) == 0)
				{
					TraceFrm("--> C230MIf::Receive:UnPacked", pbRxBuf, iRetLen);
					m_fRSCoded = true;
					return iRetLen;
				}
			}
		}
	}
		
	memcpy(pbRxBuf, bBuf, iLen);
//	m_fRSCoded = false;
	
	TraceFrm("--> C230MIf::Receive:Do not use Rs Code", bBuf, iLen);

	return iLen;
}
