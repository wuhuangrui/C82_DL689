/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�RestCtrl.h
 * ժ    Ҫ�����ļ���Ҫʵ��CRestCtrl��Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��
 *********************************************************************************************************/
#ifndef RESTCTRL_H
#define RESTCTRL_H

#include "CtrlBase.h"

typedef struct
{
	BYTE bAct;		//����(0<�޶���>, 1<���ݿ�Ͷ��>, 2<���ݿؽ��>)
	DWORD dwTime;	//��������ʱ��(��2000��1��1��0ʱ0��0�뵽���ڵ�����)
} TRestCtrlCmd;

typedef struct
{
	int64 iPwrLimit;	//'���ݿ�'���ʶ�ֵ.
	DWORD dwStartTime;	//'���ݿ�'�޵���ʼʱ��(��00:00����������).
	DWORD dwPersistTime;//'���ݿ�'�޵�����ʱ��(����).
	BYTE bDays;			//'���ݿ�'�޵���.
} TRestCtrlPara;

//========================================== CRestCtrl ==============================================
class CRestCtrl : public CPwrCtrl
{
	friend class CAllPwrCtrl;
public:
	CRestCtrl(void){}
	virtual ~CRestCtrl(){}

	void DoSaveOpenRec(void)							//������բ��¼.
	{
		CPwrCtrl::DoSavePwrCtrlOpenRec(1);
	}

	BYTE GetCtrlType()
	{
		return m_CtrlType;//CTL_PWR_REST;
	}

	BYTE GetInvCtrlType()
	{
		return CTL_PWR_REST_CLOSE;	
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
		sprintf(psz, "RestCtrl");
		return psz;
	}
	int CtrlType(void)									//��ÿ��Ƶ�����(������������).
	{
		return m_CtrlType;//CTL_PWR_REST;
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
		return ((CGrpCtrl::GetSysCtrlFlgs(iGrp, 0)&0x02) != 0);	//���ݿ�ʹ��1λ.
	}
	bool SetSysCtrlFlg(int iGrp, bool fStatus)			//�����ܼ���ϵͳ�Ȿ����Ʊ�־��Ϊ��Ч.
	{
		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x02, fStatus, PWR_CTL);//���ݿ�ʹ��1λ.
	}

	//---------------------------------------------------------------
	bool GetRestCtrlPara(int iGrp, TRestCtrlPara& rPara);	//��ȡָ���ܼ���'���ݿ�'����.

protected:
	TRestCtrlCmd		m_NewCmd;						//����GetSysCmd(int iGrp)������,��ȡ����������浽�ñ�����.
	TRestCtrlCmd		m_CtrlCmd;						//��ǰʹ�õ�����.
	BYTE 			m_CtrlType;
};

#endif  //RESTCTRL_H
