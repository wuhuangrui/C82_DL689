/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�PotoIf.cpp
 * ժ    Ҫ�����ļ�ʵ����ͨ�Žӿڻ���
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע���ӿڵ�״̬�л�: (����)->(��λ)->(����)->(��¼)->(����)
 *          ��״̬�´���������������:��״̬�Ĵ�������ɸ�״̬�Լ�����,ĳ״̬��������״̬�ļ���
 *********************************************************************************************************/
#include "stdafx.h"
#include "ProtoIf.h"
#include "FaCfg.h"
#include "FaConst.h"
#include "Info.h"
#include "Trace.h"
#include "sysapi.h"
#include "ProIfConst.h"

////////////////////////////////////////////////////////////////////////////////////////////
//CProtoIf

CProtoIf::CProtoIf()
{
	m_fRstInConnectFail = false;	//������ʧ�ܵ����Դ����󲻸�λ�ӿ�
	m_wState = IF_STATE_RST;
	m_wIfType = IF_UNKNOWN;
	m_dwDebugClick = 0;
	m_iLastErr = 0;
	m_dwRstIfClick = 0;		 //��¼�ӿ��ϴθ�λʱ�̻��߽��յ����ĵ�ʱ��
}

CProtoIf::~CProtoIf()
{

}


bool CProtoIf::Init(TIfPara* pIfPara)
{
	m_pIfPara = pIfPara;
	
	m_fExit = false;
	m_fExitDone = false;
	m_fUnrstParaChg = false;  	//�Ǹ�λ���������ı�
	m_fNeedActive = false;		//��Ҫ����	
	m_wRunCnt = 0;
	m_dwDormanClick = 0;	    //�������ߵĿ�ʼʱ��
	
	//�������
	m_dwRxClick = 0;
	m_dwBeatClick = 0;
	
	//ʧ�ܼ�����
	m_wResetFailCnt = 0;		 
	m_wConnectFailCnt = 0; 
	m_wLoginFailCnt = 0;

	m_fDisConnCmd = false;			//�յ��ⲿ�ĶϿ���������
	m_fSetIdleCmd = false;			//�յ��ⲿ�Ĵ��ڿ���״̬������
	m_dwDormanInterv = 0;		//��ʱ�趨�����߼������λ��
	m_wDormanState = IF_STATE_DORMAN; //��ʱ���ߵ�״̬�������껹Ҫת�ص���״̬

	DTRACE(DB_FAPROTO, ("CProtoIf::Init: if(%s) init to fNeedLogin=%d, wMaxFrmBytes=%d, dwRstInterv=%ld, dwConnectInterv=%ld, wLoginRstNum=%ld, dwLoginInterv=%ld, wRstNum=%d, wReSendNum=%d, wReTryNum=%d, dwDormanInterv=%ld\n", 
						GetName(),
						m_pIfPara->fNeedLogin,
						m_pIfPara->wMaxFrmBytes, 		
						m_pIfPara->dwRstInterv,		
						m_pIfPara->dwConnectInterv,	
						m_pIfPara->wLoginRstNum, 	
						m_pIfPara->dwLoginInterv, 	
						m_pIfPara->wRstNum,           
						m_pIfPara->wReSendNum,		
						m_pIfPara->wReTryNum,
						m_pIfPara->dwDormanInterv));

	return true;
}

char* CProtoIf::GetName()
{
	char* pszName = "unknown";
	
	if (m_pIfPara->pszName != NULL)
		pszName = m_pIfPara->pszName;
	
	return pszName;
}
	
void CProtoIf::OnResetOK()
{
	//��λ���
	m_wState = IF_STATE_CONNECT;
	m_wResetFailCnt = 0;
	m_dwRstIfClick = GetClick(); //��¼�ӿ��ϴθ�λʱ�̻��߽��յ����ĵ�ʱ��
}

void CProtoIf::OnResetFail()
{
	m_wResetFailCnt++;
	m_dwRstIfClick = GetClick(); //��¼�ӿ��ϴθ�λʱ�̻��߽��յ����ĵ�ʱ��

	if (m_pIfPara->dwDormanInterv!=0 && m_wResetFailCnt>=m_pIfPara->wRstNum)
	{
		DTRACE(DB_FAPROTO, ("CProtoIf::OnResetFail: go to dorman\n"));
		m_wResetFailCnt = 0;
		EnterDorman();
	}
	else
	{
		Sleep(m_pIfPara->dwRstInterv * 1000);
	}	
}

//����:�ڽӿ��ɶϿ�תΪ���ӵ�ʱ�����
void CProtoIf::OnConnectOK()
{
	if (IsNeedLogin())
		m_wState = IF_STATE_LOGIN;
	else	
		m_wState = IF_STATE_TRANS;

	//�������
	m_dwRxClick = GetClick();
	m_dwBeatClick = 0;
	
	//�������
	m_wConnectFailCnt = 0;
}

void CProtoIf::OnConnectFail()
{	
	//�������
	m_wConnectFailCnt++;
	
	//��¼��ر���
	if (m_wConnectFailCnt >= GetConnectNum())
	{
		m_wConnectFailCnt = 0;
		if (m_pIfPara->dwDormanInterv != 0)
		{
			DTRACE(DB_FAPROTO, ("CProtoIf::OnConnectFail: go to dorman\n"));
			EnterDorman();
		}
		else
		{
			Sleep(m_pIfPara->dwConnectInterv * 1000); //�ӿڵ����Ӽ��,��λ��
			if (m_fRstInConnectFail)	//���ӿ�������ʧ�ܵ����Դ�����λ�ӿ�
				m_wState = IF_STATE_RST; 
		}
	}
	else
	{
		Sleep(m_pIfPara->dwConnectInterv * 1000); //�ӿڵ����Ӽ��,��λ��
	}	
}


//����:�ڽӿ�������תΪ�Ͽ���ʱ����ã������������Ͽ����Ǳ����Ͽ�
bool CProtoIf::DisConnect()
{
	if (m_wState > IF_STATE_CONNECT)
		m_wState = IF_STATE_CONNECT;
	
	//�������
	//m_dwRxClick = 0;	//������m_dwRxClick,��Ȼ��������m_pIfPara->dwNoRxRstAppInterv���,
						//ֻҪ��һ��û�������ɹ��ͻ������ն˸�λ
	m_dwBeatClick = 0;
	
	//�������
	//m_wConnectFailCnt = 0; //�Ͽ����Ӳ���Ҫ��λʧ�ܼ���,��Ϊ����socket
							 //��ʽ��,ÿ������ǰ�������س��ԶϿ�֮ǰ��
							 //����,�����ʹ����ʧ�ܼ���ʧЧ,������ֻ��
							 //�ڽӿڸ�λ�ɹ������ۼƵ����Դ����������
	return true;
}

//����:���յ��ⲿ�ĶϿ���������ʱ,���ñ�����֪ͨ�ӿ�
//����:@dwDormanInterv ��̬�趨�����߼������λ��
void CProtoIf::SetDisConnect(DWORD dwDormanInterv)
{
	m_fDisConnCmd = true;	//�յ��ⲿ�ĶϿ���������
	m_dwDormanInterv = dwDormanInterv;	//��ʱ�趨�����߼������λ��
}

//����:���յ��ⲿ�Ĵ��ڿ���״̬������ʱ,���ñ�����֪ͨ�ӿ�
void CProtoIf::SetIdle()
{
	m_fSetIdleCmd = true;			//�յ��ⲿ�Ĵ��ڿ���״̬������
}

//ʹ�ӿڽ������߷�ʽ
void CProtoIf::EnterDorman()
{
	DTRACE(DB_FAPROTO, ("CProtoIf::EnterDorman : enter dorman mode for %ldS \n", 
						m_dwDormanInterv!=0 ? m_dwDormanInterv : m_pIfPara->dwDormanInterv));

	DisConnect();
	m_wState = IF_STATE_DORMAN;
	m_dwDormanClick = GetClick();	 //�������ߵĿ�ʼʱ��
}    		

//����:��������ģʽ,ÿ��˯5��,���ж�����ʱ�䵽��û��
void CProtoIf::DoDorman()
{
	Sleep(5000);
	DWORD dwClick = GetClick();
	DWORD dwDormanInterv;

	if (m_dwDormanClick != 0)
	{
		if (m_dwDormanInterv != 0)	//��ʱ�趨�����߼������λ��
			dwDormanInterv = m_dwDormanInterv;
		else
			dwDormanInterv = m_pIfPara->dwDormanInterv;

		if (dwClick-m_dwDormanClick < dwDormanInterv)
		{
			DTRACE(DB_FAPROTO, ("CProtoIf::DoDorman: remain %ld\n", dwDormanInterv + m_dwDormanClick - dwClick));
		}
		else
		{
			if (m_wDormanState != IF_STATE_DORMAN) //��ʱ���ߵ�״̬�������껹Ҫת�ص���״̬
			{
				m_wState = m_wDormanState;
				DTRACE(DB_FAPROTO, ("CProtoIf::DoDorman: wake up to %d\n", m_wState));
			}
			else
			{
				m_wState = IF_STATE_RST;
				DTRACE(DB_FAPROTO, ("CProtoIf::DoDorman: wake up to rst\n"));
			}

			m_dwDormanClick = 0;
			m_dwDormanInterv = 0;	//��ʱ�趨�����߼������λ��
			m_wDormanState = IF_STATE_DORMAN;
		}		
	}
	else //m_dwDormanClick==0 
	{	 //û�й涨���ߵ�ʱ��,�൱�ڴ��������ڵ�����(����)״̬,
		 //Ҫ�˳�����״̬,��Ҫ����DoIfRelated()�и��ݽӿ���ص�
		 //�����m_wState״̬�ı�,����Ӳ�����ʱ���л�������ʱ��
		 //�ӿڵĲ�������
		if (dwClick-m_dwDebugClick > IF_DEBUG_INTERV)
		{
			m_dwDebugClick = dwClick;
			DTRACE(DB_FAPROTO, ("CProtoIf::DoDorman: if(%s)in idle mode\n", GetName()));
		}
	}
}	

DWORD CProtoIf::GetWakeUpTime()
{
	return m_pIfPara->dwDormanInterv + m_dwDormanClick - GetClick();
}

//����:��Э���½�ɹ�ʱ����
void CProtoIf::OnLoginOK()
{ 
	m_wState = IF_STATE_TRANS;
	m_wLoginFailCnt = 0; 
	m_iLastErr = GPRS_ERR_OK;
}

//����:��Э���½ʧ��ʱ����,������ٴ�ʧ�ܺ�Ͽ�����
void CProtoIf::OnLoginFail() 
{	
	m_wLoginFailCnt++;
	m_iLastErr = GPRS_ERR_LOGIN;
	
	WORD wLoginRstNum = m_pIfPara->wLoginRstNum;
	if (wLoginRstNum == 0)
		wLoginRstNum = 1;
		
	WORD wLoginNum = m_pIfPara->wLoginNum;
	if (wLoginNum == 0)
		wLoginNum = 1;
	
	if (m_wLoginFailCnt%wLoginRstNum != 0)
	{					//��¼ʧ�ܵĴ�����û���Ͽ����ӵĴ���
		Sleep(m_pIfPara->dwLoginInterv*1000); //��¼���
	}
	else if (m_wLoginFailCnt >= wLoginRstNum*wLoginNum)
	{					
		m_wLoginFailCnt = 0;
		DTRACE(DB_FAPROTO, ("CProtoIf::OnLoginFail: go to dorman\n"));
		EnterDorman();
	}
	else		//��¼ʧ�ܵĴ����ﵽ�˶Ͽ����ӵĴ�����������
	{			//m_wLoginFailCnt%wLoginRstNum==0 && m_wLoginFailCnt<wLoginRstNum*wLoginNum
		DisConnect();	//ֻ�Ͽ�����,�������¼ʧ�ܼ���,
						//�ﵽwLoginRstNum*wReTryNum�κ��������״̬
	}		
}

void CProtoIf::OnRcvFrm()
{
	m_dwRstIfClick = m_dwRxClick = GetClick();
}
  
void CProtoIf::AutoSend()
{
	if (m_pProto->m_pProPara->fAutoSend) //�Ƿ�����������͵Ĺ���
	{	
		m_pProto->AutoSend();
	}
}

//��ʼ�����ӿ�ʱ��ʼ��
void CProtoIf::InitRun()
{
	m_dwRstIfClick = GetClick();
}


//����:�ӿ���ز�������,Ŀǰʵ�ֵĲ�����:
// 		1.�������û��ͨѶ��λ�ն�/ģ��
void CProtoIf::DoIfRelated()
{
	DWORD dwClick = GetClick();
	if (m_pIfPara->dwNoRxRstAppInterv != 0) //�޽��ո�λ�ն˼��,��λ��,0��ʾ����λ
	{
		if (dwClick-m_dwRxClick > m_pIfPara->dwNoRxRstAppInterv)
		{
			DTRACE(DB_FAPROTO, ("CProtoIf::DoIfRelated: if(%s) no rx reset app!\n", GetName()));
			SetInfo(m_pIfPara->wInfoAppRst);	//CPU��λ INFO_APP_RST
			return;
		}
	}

	if (m_pIfPara->dwNoRxRstIfInterv != 0) //�޽��ո�λ�ӿڼ��,��λ��,0��ʾ����λ
	{
		if (dwClick-m_dwRstIfClick > m_pIfPara->dwNoRxRstIfInterv)
		{
			DTRACE(DB_FAPROTO, ("CProtoIf::DoIfRelated: if(%s) no rx reset!\n", GetName()));
			DisConnect();
			m_wState = IF_STATE_RST; 
			m_dwRstIfClick = dwClick; //��¼�ӿ��ϴθ�λʱ�̻��߽��յ����ĵ�ʱ��
			return;
		}
	}
}

//�ӿ��Ƿ񻹴��ڴ���״̬
bool CProtoIf::CanTrans() 
{ 
	WORD wState = GetState();
	if (m_bGprsDataSrc == DATA_SRC_SMS)
		return true;
	else if (wState>IF_STATE_CONNECT && wState<=IF_STATE_TRANS)
		return true;

	return false;
}; 
