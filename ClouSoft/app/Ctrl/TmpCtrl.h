/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TmpCtrl.h
 * ժ    Ҫ�����ļ���Ҫʵ��CTmpCtrl��Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��
 *********************************************************************************************************/
#ifndef TMPCTRL_H
#define TMPCTRL_H

#include "CtrlBase.h"

typedef struct
{
	BYTE bAct;		//����(0<�޶���>, 1<Ӫҵ��ͣ��Ͷ��>, 2<Ӫҵ��ͣ�ؽ��>)
	BYTE bWndTime;	//����ʱ��
	BYTE bQuotiety;	//�¸�ϵ��
	BYTE bDelayTime;//�����ӳ�ʱ��
//#ifdef PRO_698
	BYTE bCtrlTime;	//����ʱ��
	BYTE bAlrTime[TURN_NUM];	//�ִθ澯ʱ��
//#endif
	DWORD dwTime;	//��������ʱ��(��2000��1��1��0ʱ0��0�뵽���ڵ�����)
} TTmpCtrlCmd;

//========================================== CTmpCtrl ==============================================
class CTmpCtrl : public CPwrCtrl
{
	friend class CAllPwrCtrl;
public:
	CTmpCtrl(void){}
	virtual ~CTmpCtrl(){}

	void DoSaveOpenRec(void)							//������բ��¼.
	{
		CPwrCtrl::DoSavePwrCtrlOpenRec(3);
	}

	BYTE GetCtrlType()
	{
		return m_CtrlType;
	}

	BYTE GetInvCtrlType()
	{
		return CTL_PWR_TMP_CLOSE;
	}

	bool DoCtrl(void);									//���п���.

protected:
	void RstCtrl(void);									//��λ�ڴ��б������״̬��.
	bool GetSysCmd(int iGrp);							//��ȡĳ�ܼ���ı����������,��������ŵ� m_NewCmd ��.(ע��: �Բ�ͬ����,m_NewCmd�Ľṹ�ǲ�ͬ��)
	bool ClrSysCmd(int iGrp);							//���ϵͳ�Ȿ�ܼ��鱾���������.
	bool SetSysCtrlTurnsStatus(int iGrp, BYTE bTurnsStatus);

	void ClrCmd(void)									//����ڴ��б�����ƵĿ�������.
	{
		memset(&m_CtrlCmd, 0, sizeof(m_CtrlCmd));
	}
	char* CtrlType(char* psz)							//��ÿ��Ƶ�����(���ؿ������͵��ַ�������).
	{
		sprintf(psz, "TmpCtrl");
		return psz;
	}
	int CtrlType(void)									//��ÿ��Ƶ�����(������������).
	{
		return m_CtrlType;//CTL_PWR_TMP;
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
		return ((CGrpCtrl::GetSysCtrlFlgs(iGrp, 0)&0x08) != 0);	//��ʱ�¸���ʹ��3λ.
	}
	bool SetSysCtrlFlg(int iGrp, bool fStatus);			//�����ܼ���ϵͳ�Ȿ����Ʊ�־��Ϊ��Ч.
	bool RstSysCtrlStatus(int iGrp)						//��λϵͳ�⵱ǰ�ܼ��鱾�����״̬(���ܰ����ִ�״̬,Ͷ���־�ȵ�).
	{
		return SetSysCtrlFlg(iGrp, false);
	}
#ifdef PRO_698
	bool RestoreTurnStatus();
#endif
	

protected:
	TTmpCtrlCmd			m_NewCmd;						//����GetSysCmd(int iGrp)������,��ȡ����������浽�ñ�����.
	TTmpCtrlCmd			m_CtrlCmd;						//��ǰʹ�õ�����.

	bool				m_fCalLimitFinish;				//��ǰ�ܼ���'��ʱ�¸���'����ֵ�������״̬.
	DWORD				m_dwCalLimitStartClick;			//��ǰ�ܼ���'��ʱ�¸���'����ֵ������ʼʱ��.
	DWORD				m_dwCalLimitTmpClick;			//��ǰ�ܼ���'��ʱ�¸���'����ֵ��������ʱʱ�����.
	WORD				m_wCalLimitTimes;				//��ǰ�ܼ���'��ʱ�¸���'����ֵ����ʱ,�����ۼӴ���.
	int64				m_iTmpCtrlLimit;				//��ǰ�ܼ���'��ʱ�¸���'����ֵ(ȡ���ڵ�ǰ�¸�ֵ�ͱ���ֵ).

	DWORD				m_dwOpenBreakTime;				//��բʱ��.
	BYTE				m_CtrlType;
};

#endif  //TMPCTRL_H
