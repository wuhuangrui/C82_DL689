/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ThreadMonitor.h
 * 摘    要：本文件主要实现线程监控
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年3月
 * 备    注：
 *********************************************************************************************************/
#ifndef THREADMONITRO_H
#define THREADMONITRO_H
#include "sysarch.h"

#define THRD_MNTR_NUM			64	//最大监控线程数量
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
	DWORD	m_dwClick;		//内部的秒计数
	TSem 	m_semMonitor;	//监控器的总的资源保护
	DWORD 	m_dwRunClick[THRD_MNTR_NUM];	//运行计数
	DWORD 	m_dwUpdInterv[THRD_MNTR_NUM];	//更新间隔,单位秒,不为0表示占用
	char 	m_szThreadName[THRD_MNTR_NUM][THRD_NAME_LEN];	//线程名称
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
