/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DlmsClass.h
 * ժ    Ҫ�����ļ���Ҫʵ��LoadCtrl��Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��2��
 *********************************************************************************************************/
//#define LOADCTRL_H
#ifndef LOADCTRL_H
#define LOADCTRL_H

#include "CtrlBase.h"
#include "Guarantee.h"
#include "UrgeFee.h"
#include "YkCtrl.h"
#include "MonthCtrl.h"
#include "BuyCtrl.h"
#include "TmpCtrl.h"
#include "ShutoutCtrl.h"
#include "RestCtrl.h"
#include "PeriodCtrl.h"

class CAllPwrCtrl : public CCtrlBase
{
public:                             			
	CAllPwrCtrl(void);
	~CAllPwrCtrl();

	bool Init(void);						//���س�ʼ��.
	bool DoCtrl(void);						//����ִ��.
	int GetGrp(void) { return m_iGrp; }; 	//��ȡ��ǰ�ܼ���.
	int GetCurProGrp(void); //��ȡ���ȿ����ܼ���
	BYTE GetTurnsStatus(void) { return m_bTurnsStatus; }; //����ִ�״̬.
	bool SetSysTurnsStatus(int iGrp, BYTE bTurnsStatus);			//�趨ϵͳ��ָ���ܼ��鹦�ص������ִ�״̬.
	void StatOverLimitPara(void);				//ͳ�ƹ��س��޲���.
												 
	WORD GetOpenTimes(void)						//��ȡ������բ����.
	{
		return (m_TmpCtrl.GetOpenTimes()
				+m_ShutoutCtrl.GetOpenTimes()
				+m_RestCtrl.GetOpenTimes()
				+m_PeriodCtrl.GetOpenTimes());
	}
	bool IsBeepAlr(void)						//�Ƿ���������.
	{
		return (m_TmpCtrl.IsBeepAlr()
			   || m_ShutoutCtrl.IsBeepAlr()
			   || m_RestCtrl.IsBeepAlr()
			   || m_PeriodCtrl.IsBeepAlr());
	}	
	bool IsValid(void)
	{
		return (m_TmpCtrl.IsValid()
				||m_ShutoutCtrl.IsValid()
				||m_RestCtrl.IsValid()
				||m_PeriodCtrl.IsValid());
	}

protected:
	bool SetSysPwrAlrFlgs(int iGrp, BYTE bFlgsStatus);				//�趨ϵͳ��ָ���ܼ��鹦�ص����б���״̬.
	bool ChgSysPwrAlrFlgs(int iGrp, BYTE bFlgs, bool fStatus);		//�ı�ϵͳ��ָ���ܼ��鹦�ص���Ӧ����״̬.

	bool ChgSysTurnsStatus(int iGrp, BYTE bTurns, bool fStatus);	//�ı�ϵͳ��ָ���ܼ��鹦�ص���Ӧ�ִ�״̬.

	
protected:
	CTmpCtrl			m_TmpCtrl;
	CShutoutCtrl		m_ShutoutCtrl;
	CRestCtrl			m_RestCtrl;
	CPeriodCtrl			m_PeriodCtrl;

	int					m_iGrp;
	BYTE				m_bTurnsStatus;
	BYTE				m_bAlrsStatus;
};

class CLoadCtrl : public CCtrlBase
{
public:
	CLoadCtrl(void);
	~CLoadCtrl();

	bool Init(void);							//���س�ʼ��.
	bool DoCtrl(void);							//����ִ��.
	void InitSysCtrl();
	bool GetSoundBeepStats();
	bool GetEnableCtrlStatus(){ return m_fEnableBreakAct; };
	bool IsEnergyFee(void);
	void TrigerAlr(){m_fTrigerAlr = true;};
	void DisAlr(){m_fTrigerAlr = false;};
	int GetPwrCtrlGrp() { return m_AllPwrCtrl.GetCurProGrp(); }; 	//��ȡ��ǰ�����ܼ���
	int GetMonCtrlGrp() { return m_MonthCtrl.GetGrp(); }; 	//��ȡ��ǰ�µ���ܼ���
	int GetBuyCtrlGrp() { return m_BuyCtrl.GetGrp(); }; 	//��ȡ��ǰ������ܼ���	

protected:
	DWORD			m_dwStarupTime;				//����ģ������ʱ��.

	CGuarantee		m_Guarantee;				//����ض���.
	CUrgeFee		m_UrgeFee;					//�߷Ѹ澯����.
	CYkCtrl			m_YkCtrl[TURN_NUM];			//�����ִε�ң�ض���.

	CMonthCtrl		m_MonthCtrl;				//�µ�ض���.
	CBuyCtrl		m_BuyCtrl;					//����ض���.
	CAllPwrCtrl		m_AllPwrCtrl;				//���еĹ��ض���.

	bool			m_fEnableBreakAct;
	BYTE			m_bTurnsStatus;				//
	bool			m_fBeepAlrStatus;

	BYTE	m_bYkClosedTurns[3];	//�����Ʊ�ң�غ�բ�ϵ����Ѿ���բ���ִ�
	DWORD	m_dwWarnTime;
	bool	m_fTrigerAlr;					//�����澯״̬.
};

extern CLoadCtrl g_LoadCtrl;

#endif  //LOADCTRL_H
