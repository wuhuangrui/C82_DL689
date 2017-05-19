/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ShutoutCtrl.h
 * ժ    Ҫ�����ļ���Ҫʵ��CShutoutCtrl��Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��
 *********************************************************************************************************/
#ifndef SHUTOUTCTRL_H
#define SHUTOUTCTRL_H

#include "CtrlBase.h"

typedef struct
{
	BYTE bAct;		//����(0<�޶���>, 1<Ӫҵ��ͣ��Ͷ��>, 2<Ӫҵ��ͣ�ؽ��>)
	DWORD dwTime;	//��������ʱ��(��2000��1��1��0ʱ0��0�뵽���ڵ�����)
} TShutoutCtrlCmd;

typedef struct
{
	DWORD dwStartTime;	//'Ӫҵ��ͣ��'��ʼʱ��
	DWORD dwEndTime;	//'Ӫҵ��ͣ��'����ʱ��
	int64 iPwrLimit;	//'Ӫҵ��ͣ��'���ʶ�ֵ
} TShutoutCtrlPara;

//========================================== CShutoutCtrl ==============================================
class CShutoutCtrl : public CPwrCtrl
{
	friend class CAllPwrCtrl;
public:
	CShutoutCtrl(void){}
	virtual ~CShutoutCtrl(){}

	void DoSaveOpenRec(void)							//������բ��¼.
	{
		CPwrCtrl::DoSavePwrCtrlOpenRec(2);
	}

	BYTE GetCtrlType()
	{
		return m_CtrlType;
	}

	BYTE GetInvCtrlType()
	{
		return CTL_PWR_SHUTOUT_CLOSE;
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
		sprintf(psz, "ShutoutCtrl");
		return psz;
	}
	int CtrlType(void)									//��ÿ��Ƶ�����(������������).
	{
		return m_CtrlType;//CTL_PWR_SHUTOUT;
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
		return ((CGrpCtrl::GetSysCtrlFlgs(iGrp, 0)&0x04) != 0);	//Ӫҵ��ͣ��ʹ��2λ.
	}
	bool SetSysCtrlFlg(int iGrp, bool fStatus)			//�����ܼ���ϵͳ�Ȿ����Ʊ�־��Ϊ��Ч.
	{
		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x04, fStatus, PWR_CTL);//Ӫҵ��ͣ��ʹ��2λ.
	}

	//---------------------------------------------------------------
	bool GetShutoutCtrlPara(int iGrp, TShutoutCtrlPara& rPara);	//��ȡָ���ܼ���'Ӫҵ��ͣ��'����.

protected:
	TShutoutCtrlCmd		m_NewCmd;						//����GetSysCmd(int iGrp)������,��ȡ����������浽�ñ�����.
	TShutoutCtrlCmd		m_CtrlCmd;						//��ǰʹ�õ�����.
	BYTE 				m_CtrlType;
};

#endif  //SHUTOUTCTRL_H
