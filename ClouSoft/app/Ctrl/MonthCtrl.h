/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MonthCtrl.h
 * ժ    Ҫ�����ļ���Ҫʵ��CMonthCtrl��Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��
 *********************************************************************************************************/
#ifndef MONTHCTRL_H
#define MONTHCTRL_H

#include "CtrlBase.h"

typedef struct
{
	BYTE bAct;		//����(0<�޶���>, 1<�¿�Ͷ��>, 2<�¿ؽ��>)
	DWORD dwTime;	//��������ʱ��(��2000��1��1��0ʱ0��0�뵽���ڵ�����)
} TMonthCtrlCmd;

//========================================= CMonthCtrl =============================================
class CMonthCtrl : public CEngCtrl
{
public:
	CMonthCtrl(void);
	virtual ~CMonthCtrl(){}

	bool Init(void);
	bool DoCtrl(void);									//���п���.
	BYTE GetCtrlType()
	{
		return CTL_ENG_MONTH;
	}

	BYTE GetInvCtrlType()
	{
		return CTL_ENG_MONTH_CLOSE;
	}

	bool IsBeepAlr(void)								//�Ƿ���������.
	{
		return (m_dwAlrStartTime != 0);
	}

	bool SetSysTurnsStatus(int iGrp, BYTE bTurnsStatus)	//�趨ϵͳ��ָ���ܼ��鱾��������ִ�״̬.
	{
		return CEngCtrl::SetSysEngTurnsStatus(iGrp, bTurnsStatus, 0);
	}

	void StatOverLimitPara(void);						//ͳ�Ƴ��޲���.

protected:
	void SubRstCtrl(void);
	void RstCtrl(void);									//��λ�ڴ��б������״̬��.
	bool GetSysCmd(int iGrp);							//��ȡĳ�ܼ���ı����������,��������ŵ� m_NewCmd ��.(ע��: �Բ�ͬ����,m_NewCmd�Ľṹ�ǲ�ͬ��)
	bool ClrSysCmd(int iGrp);							//���ϵͳ�Ȿ�ܼ��鱾���������.
	bool SetSysCtrlTurnsStatus(int iGrp, BYTE bTurnsStatus);

	bool GetSysStatus(void)								//���ڳ�ʼ��ʱ,��ϵͳ���б�����Ƶ��ִ�״̬,����״̬��ͬ�����ڴ��ж�Ӧ�ı���.
	{
		return GetSysEngStatus(0);	//�µ��Ϊ0
	}
	void ClrCmd(void)									//����ڴ��б�����ƵĿ�������.
	{
		memset(&m_CtrlCmd, 0, sizeof(m_CtrlCmd));
	}
	char* CtrlType(char* psz)							//��ÿ��Ƶ�����(���ؿ������͵��ַ�������).
	{
		sprintf(psz, "MonthCtrl");
		return psz;
	}
	int CtrlType(void)									//��ÿ��Ƶ�����(������������).
	{
		return CTL_ENG_MONTH;
	}
	int NewCmdAct(void)									//��ȡ������Ķ�����.
	{
		return m_NewCmd.bAct;
	}
	DWORD NewCmdTime(void)								//��ȡ������Ľ���ʱ��.
	{
		return m_NewCmd.dwTime;
	}
	int CurCmdAct(void)									//��ȡ��ǰ����Ķ�����.
	{
		return m_CtrlCmd.bAct;
	}
	DWORD CurCmdTime(void)								//��ȡ��ǰ����Ľ���ʱ��.
	{
		return m_CtrlCmd.dwTime;
	}
	void SaveNewCmd(void)								//������������.
	{
		m_CtrlCmd = m_NewCmd;
	}
	bool GetSysCtrlFlg(int iGrp)						//��ȡϵͳ��ָ���ܼ�����µ�ر�־λ.
	{
		return ((CGrpCtrl::GetSysCtrlFlgs(iGrp, 1)&0x01) != 0);	//�µ��ʹ��0λ.
	}
	bool SetSysCtrlFlg(int iGrp, bool fStatus)			//����ϵͳ��ָ���ܼ�����µ�ر�־λ.
	{
		if (fStatus)
			m_fLaunch = true;
		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x01, fStatus, ENG_CTL);//�µ��ʹ��0λ.
	}
	bool GetSysCtrlAlr(int iGrp)						//��ȡָ���ܼ�����µ�ر�����־λ.
	{
		return ((CEngCtrl::GetSysEngAlrFlgs(iGrp)&0x01) != 0);	//�µ��ʹ��0λ.
	}
	bool SetSysCtrlAlr(int iGrp, bool fStatus)			//����ϵͳ��ָ���ܼ�����µ�ر�����־λ.
	{
		return CEngCtrl::ChgSysEngAlrFlgs(iGrp, 0x01, fStatus);//�µ��ʹ��0λ.
	}
	bool ChgSysTurnsStatus(int iGrp, BYTE bTurns, bool fStatus)	//�ı�ϵͳ��ָ���ܼ��鱾���������Ӧ�ִ�״̬.
	{
		return CEngCtrl::ChgSysEngTurnsStatus(iGrp, bTurns, fStatus, 0);
	}

	int64 GetMonthEng(int iGrp);
	int64 GetMonthLimit(int iGrp);						//��ȡָ���ܼ��鱾���õ��޶�.
	int64 GetMonthAlarmFactor(int iGrp);                //��ȡָ���ܼ��鱾���õ籨��ϵ��
	int GetAlrFltQuotiety(void);						//��ȡָ���ܼ��鱾���õ��޶������ϵ��.

	void BeepAlrCtrl(void);								//�µ�ط�������������.

	void SaveDisp();

protected:
	TMonthCtrlCmd		m_NewCmd;						//����GetSysCmd(int iGrp)������,��ȡ����������浽�ñ�����.
	TMonthCtrlCmd		m_CtrlCmd;						//��ǰʹ�õ�����.
	bool				m_fLaunch;
	DWORD				m_dwAlrStartTime;				//����������ʼʱ��.

	TEngOverLimitStat	m_OLStat;						//���޼�¼.

	int64 m_iCurMonthEng;	                          //��ȡ��ǰ�ܼ��鱾�����õ���.
	int64 m_iCurEngLimit;	                         //��ȡ��ǰ�ܼ��鱾���¿ض�ֵ.
	int64   m_iCurAlarmFactor;                         //��ȡ��ǰ�ܼ����µ�ظ澯ϵ��.
	int64 m_iCurAlarmLimit;							 //��ȡ��ǰ�ܼ����µ�ظ澯��ֵ.
};

#endif  //MONTHCTRL_H
