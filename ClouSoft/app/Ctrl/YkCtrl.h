/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�YkCtrl.h
 * ժ    Ҫ�����ļ���Ҫʵ��CYkCtrl��Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��
 *********************************************************************************************************/
#ifndef YKCTRL_H
#define YKCTRL_H

#include "CtrlBase.h"

typedef struct
{
	BYTE 	bAct;			//����(0<�޶���>, 1<��բ>, 2<�����բ>)
	BYTE 	bAlrTime;		//������ʱ(������)
	DWORD 	dwPersistTime;	//�޵�ʱ��(0.5Сʱ��)
	DWORD 	dwTime;		//��������ʱ��(��2000��1��1��0ʱ0��0�뵽���ڵ�����)
} TYkCtrlCmd;

//============================================ CYkCtrl =================================================
class CYkCtrl : public CCtrl
{
public:                             			
	CYkCtrl(BYTE bTurn=TURN_START_PN);
	virtual ~CYkCtrl(){}

	bool SetTurn(int iTurn);							//����'ң��'�ĵ�ǰ�ִ�.
	bool DoCtrl(void);									//���п���.
	void MakeDisp();

	bool IsCloseStatus()
	{
		return (!m_fAlrStatus && !m_fTurnStatus);
	}
	bool GetTurnStatus(void)							//��ȡ�ִ�״̬.
	{
		return m_fTurnStatus;
	}
	WORD GetOpenTimes(void)								//��ȡ��բ����,Ȼ��������բ����.
	{
		WORD w = m_wOpenTimes;

		m_wOpenTimes = 0;

		return w;
	}
	bool IsBeepAlr() { return m_fAlrStatus; };		//�Ƿ���������.
	bool IsRxCloseCmd() { return m_fRxCloseCmd; }; 	//����ִ���Ƿ��յ���һ��ң�غ�բ����

protected:                          			
	void DoCmdScan(void);								//ɨ��ϵͳ���е�����.
	void ClrCmd(void)									//����ڴ��б�����ƵĿ�������.
	{
		memset(&m_CtrlCmd, 0, sizeof(TYkCtrlCmd));
	}
	void RstCtrl(void);									//��λ�ڴ��б������״̬��.
	bool ClrSysCmd(int iTurn);							//���ϵͳ�Ȿ�ִ�ң������.

	bool SetSysTurnStatus(int iTurn, bool fStatus);		//�趨ϵͳ��ָ���ܼ��鱾���������Ӧ�ִ�״̬.
	bool SetSysTurnAlrStatus(int iTurn, bool fStatus);
	bool RstSysCtrlStatus(int iTurn)					//��λϵͳ��ָ���ִ�ң��״̬(���ܰ����ִ�״̬,Ͷ���־�ȵ�).
	{
		SetSysTurnAlrStatus(m_iTurn, false);
		return SetSysTurnStatus(iTurn, false);
	}
	void DoSaveOpenRec(void);							//����ң����բ��¼.

	//��ʾ�ã� Added by Chenxi,7th,July
	void RemoveDispItem(TCtrl tInvCtrl);
	void AddDispItem(TCtrl tTopCtrl);
	void SaveDisp(DWORD dwCntDown);
	BYTE GetCtrlType() { return CTL_YkCtrl; };
	BYTE GetInvCtrlType() { return CTL_YkCtrl_CLOSE; };


protected:
	TYkCtrlCmd	m_CtrlCmd;						//��ǰ�ִ�'ң��'����.

	int			m_iTurn;						//��ǰ�ִ�(�ִα�Ŵ�0��ʼ, 0 ~ TURN_NUM-1)
	bool		m_fTurnStatus;					//��ǰ�ִ�'ң��'״̬.
	bool		m_fAlrStatus;					//��ǰ�ִ��ִ�'ң��'����״̬.
	bool		m_fRxCloseCmd;					//����ִ���Ƿ��յ���һ��ң�غ�բ����

	bool        m_fAlarmStatus;                   //�Ƿ�Ҫ��ʾң�ظ澯��
	bool        m_fCloseStatus;                  //�Ƿ�Ҫ��ʾң�غ�բ��
	bool        m_fOpenStatus;                   //�Ƿ�Ҫ��ʾң����բ��

	DWORD       m_dwOpenClick;                  //ң�غ�բʱʱ�ӵδ�
	DWORD       m_dwCloseClick;                  //ң�غ�բʱʱ�ӵδ�

	DWORD		m_dwFrzDly;						//������բ���ʶ�����ʱ.
	WORD		m_wOpenTimes;					//��բ����.
	DWORD		m_dwOpenClk;
	
};

#endif  //YKCTRL_H
