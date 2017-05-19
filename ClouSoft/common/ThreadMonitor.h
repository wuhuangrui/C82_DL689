/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ThreadMonitor.h
 * ժ    Ҫ�����ļ���Ҫʵ���̼߳��
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2009��3��
 * ��    ע��
 *********************************************************************************************************/
#ifndef THREADMONITRO_H
#define THREADMONITRO_H
#include "sysarch.h"

#define THRD_MNTR_NUM			64	//������߳�����
#define THRD_NAME_LEN			32

class CThreadMonitor
{
public:
	CThreadMonitor();
	virtual ~CThreadMonitor();
	
	bool Init();
	int ReqMonitorID(char* pszName, DWORD dwUpdInterv);
	void ReleaseMonitorID(int iID);
	void UpdRunClick(int iID);
	int DoMonitor();
	bool GetThreadName(int iID, char* pszName);
		
protected:
	DWORD	m_dwClick;		//�ڲ��������
	TSem 	m_semMonitor;	//��������ܵ���Դ����
	DWORD 	m_dwRunClick[THRD_MNTR_NUM];	//���м���
	DWORD 	m_dwUpdInterv[THRD_MNTR_NUM];	//���¼��,��λ��,��Ϊ0��ʾռ��
	char 	m_szThreadName[THRD_MNTR_NUM][THRD_NAME_LEN];	//�߳�����
};

extern CThreadMonitor g_ThreadMonitor;
inline bool InitThreadMonitor()
{
	return g_ThreadMonitor.Init();
}

inline int ReqThreadMonitorID(char* pszName, DWORD dwUdpInterv)
{
	return g_ThreadMonitor.ReqMonitorID(pszName, dwUdpInterv);
}

inline void ReleaseThreadMonitorID(int iID)
{
	g_ThreadMonitor.ReleaseMonitorID(iID);
}

inline void UpdThreadRunClick(int iID)
{
	g_ThreadMonitor.UpdRunClick(iID);
}

inline int DoThreadMonitor()
{
	return g_ThreadMonitor.DoMonitor();
}

inline bool GetMonitorThreadName(int iID, char* pszName)
{
	return g_ThreadMonitor.GetThreadName(iID, pszName);
}

#endif //THREADMONITRO_H
