/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�PeriodCtrl.h
 * ժ    Ҫ�����ļ���Ҫʵ��CPeriodCtrl��Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��
 *********************************************************************************************************/
#ifndef PERIODCTRL_H
#define PERIODCTRL_H

#include "CtrlBase.h"

typedef struct
{
	BYTE bAct;		//����(0<�޶���>, 1<ʱ�ο�Ͷ��>, 2<ʱ�οؽ��>)
	BYTE bFlgs;		//ʱ�ο�Ͷ���־(D0~D7��˳���λ��ʾ��1~��8ʱ��,��"1":��Ч,��"0":��Ч)
	BYTE bScheme;	//������
	DWORD dwTime;	//��������ʱ��(��2000��1��1��0ʱ0��0�뵽���ڵ�����)
} TPeriodCtrlCmd;

//========================================== CPeriodCtrl ==============================================
class CPeriodCtrl : public CPwrCtrl
{
	friend class CAllPwrCtrl;
public:
	CPeriodCtrl(void){}
	virtual ~CPeriodCtrl(){}

	void DoSaveOpenRec(void)							//������բ��¼.
	{
		CPwrCtrl::DoSavePwrCtrlOpenRec(0);
	}

	BYTE GetCtrlType()
	{
		return m_CtrlType;//CTL_PWR_PERIOD;
	}

	BYTE GetInvCtrlType()
	{
		return CTL_PWR_PERIOD_CLOSE;
	}

	bool DoCtrl(void);									//���п���.

protected:
	void RstCtrl(void);									//��λ�ڴ��б������״̬��.
	bool GetSysCmd(int iGrp);							//��ȡĳ�ܼ���ı����������,��������ŵ� m_NewCmd ��.(ע��: �Բ�ͬ����,m_NewCmd�Ľṹ�ǲ�ͬ��)
	bool ClrSysCmd(int iGrp);							//���ϵͳ�Ȿ�ܼ��鱾���������.

	void ClrCmd(void)									//����ڴ��б�����ƵĿ�������.
	{
		memset(&m_CtrlCmd, 0, sizeof(m_CtrlCmd));
	}
	char* CtrlType(char* psz)							//��ÿ��Ƶ�����(���ؿ������͵��ַ�������).
	{
		sprintf(psz, "PeriodCtrl");
		return psz;
	}
	int CtrlType(void)									//��ÿ��Ƶ�����(������������).
	{
		return m_CtrlType;//CTL_PWR_PERIOD;
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
	bool GetSysCtrlFlg(int iGrp)						//��ȡָ���ܼ���Ĺ��ر�־λ.
	{
		return ((CGrpCtrl::GetSysCtrlFlgs(iGrp, 0)&0x01) != 0);	//ʱ�ο�ʹ��0λ.
	}
	bool SetSysCtrlFlg(int iGrp, bool fStatus);			//�����ܼ���ϵͳ�Ȿ����Ʊ�־��Ϊ��Ч.

	//---------------------------------------------------------------
	int GetTimePeriod(TTime tmTime);					//��ȡָ��ʱ��������ʱ��.
	bool GetPeriodLimit(int iGrp, int iScheme, int iPeriodIdx, int64& riPwrLimit);	//��ȡָ���ܼ��鵱ǰʱ�εĹ�������.

protected:
	TPeriodCtrlCmd		m_NewCmd;						//����GetSysCmd(int iGrp)������,��ȡ����������浽�ñ�����.
	TPeriodCtrlCmd		m_CtrlCmd;						//��ǰʹ�õ�����.

	int					m_iCurPeriodIdx;				//��ǰʱ��.
	BYTE 				m_CtrlType;
};

#endif  //PERIODCTRL_H
