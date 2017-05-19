/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ThreadMonitor.cpp
 * ժ    Ҫ�����ļ���Ҫʵ���̼߳��
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��3��
 * ��    ע��
 *********************************************************************************************************/
#include "stdafx.h"
#include "ThreadMonitor.h"

CThreadMonitor g_ThreadMonitor;
///////////////////////////////////////////////////////////////////////////////////
//CThreadMonitor


CThreadMonitor::CThreadMonitor()
{
}

CThreadMonitor::~CThreadMonitor()
{
}

//����:��ʼ���̼߳��
bool CThreadMonitor::Init()
{
	m_semMonitor = NewSemaphore(1); //��������ܵ���Դ����
	memset(&m_dwRunClick, 0, sizeof(m_dwRunClick));			//���м���
	memset(&m_dwUpdInterv, 0, sizeof(m_dwUpdInterv));	//���¼��,��λ��
	m_dwClick = 0;
	return true;
}


//����:�����̼߳��ID
//����:@dwUpdInterv �̸߳��¼��,��λ��
int CThreadMonitor::ReqMonitorID(char* pszName, DWORD dwUpdInterv)
{
	if (dwUpdInterv == 0)
		return -1;
		
	int iID = -1;
	WaitSemaphore(m_semMonitor);
	for (WORD i=0; i<THRD_MNTR_NUM; i++)
	{
		if (m_dwUpdInterv[i] == 0)	//��IDû�б�ռ��
		{
			m_dwRunClick[i] = m_dwClick;		//����ʱ��,������;�����������ϸ�λ
			m_dwUpdInterv[i] = dwUpdInterv;		//m_dwUpdInterv[i]��Ϊ0��ʾռ��
			iID = i;
			
			memset(m_szThreadName[iID], 0, THRD_NAME_LEN);
			if (pszName != NULL)
			{
				int iLen = strlen(pszName);
				if (iLen > THRD_NAME_LEN-1)
					iLen = THRD_NAME_LEN-1;
				
				iLen++;	//����'\0'
				memcpy(m_szThreadName[iID], pszName, iLen);
			}
		
			if (m_szThreadName[iID][0] == '\0')
				memcpy(m_szThreadName[iID], "unknow-thrd", strlen("unknow-thrd")+1);
		
			m_szThreadName[iID][THRD_NAME_LEN-1] = '\0';
			
			break;
		}	
	}
	
	SignalSemaphore(m_semMonitor);
	return iID;
}

//����:�ͷ��̼߳��ID	
void CThreadMonitor::ReleaseMonitorID(int iID)
{
	if (iID<0 || iID>=THRD_MNTR_NUM)
		return;
	
	WaitSemaphore(m_semMonitor);
		
	m_dwUpdInterv[iID] = 0;
	m_dwRunClick[iID] = 0;
	
	SignalSemaphore(m_semMonitor);
}


//����:�ɸ����Ѿ�ע��������̶߳��Լ�������ʱ����и���
void CThreadMonitor::UpdRunClick(int iID)
{
	if (iID<0 || iID>=THRD_MNTR_NUM)
		return;
		
	if (m_dwUpdInterv[iID] != 0)	//���̼߳��ID����ȷ����
	{	
		m_dwRunClick[iID] = m_dwClick;
	}
}

//����:���Ѿ�ע��������߳̽��м��,ÿ��ִ��һ��
//����:0��ʾ����,-(�̺߳�+1)��ʾ���߳�û�м�ʱ����,��Ҫ��λ
int CThreadMonitor::DoMonitor()
{
	m_dwClick++;
	for (WORD i=0; i<THRD_MNTR_NUM; i++)
	{
		if (m_dwUpdInterv[i] != 0)	//��IDʹ��
		{
			if (m_dwClick-m_dwRunClick[i] > m_dwUpdInterv[i])
				return -(i+1);
		}	
	}
	
	return 0;
}

//����:ȡ�ü�ص��߳�����
bool CThreadMonitor::GetThreadName(int iID, char* pszName)
{
	if (iID<0 || iID>=THRD_MNTR_NUM)
	{
		memset(pszName, 0, THRD_NAME_LEN);
		return false;
	}
	else
	{		
		memcpy(pszName, m_szThreadName[iID], THRD_NAME_LEN);
		return true;
	}
}
