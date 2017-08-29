/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�BuyCtrl.h
 * ժ    Ҫ�����ļ���Ҫʵ��CBuyCtrl��Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��
 *********************************************************************************************************/
#ifndef BUYCTRL_H
#define BUYCTRL_H

#include "CtrlBase.h"

typedef struct
{
	BYTE bAct;			//����(0<�޶���>, 1<�����Ͷ��>, 2<����ؽ��>)
	DWORD dwTime;		//��������ʱ��(��2000��1��1��0ʱ0��0�뵽���ڵ�����)
} TBuyCtrlCmd;

typedef struct
{
	DWORD dwBillIdx;	//������
	BYTE bFlag;			//�����־
	int64 iBuyEng;		//������
	int64 iAlarmLimit;	//ʣ�����������
	int64 iActLimit;	//ʣ�������բ��
} TBuyCtrlPara;

//========================================== CBuyCtrl ==============================================
class CBuyCtrl : public CEngCtrl
{
public:
	CBuyCtrl(void);
	virtual ~CBuyCtrl(){}

	bool Init(void);
	bool DoCtrl(void);									//���п���.
	bool IsBeepAlr(void)								//�Ƿ���������.
	{
		return m_fAlrStauts;
	}

	BYTE GetCtrlType()
	{
		return CTL_ENG_BUY;
	}

	BYTE GetInvCtrlType()
	{
		return CTL_ENG_BUY_CLOSE;
	}

	bool SetSysTurnsStatus(int iGrp, BYTE bTurnsStatus)			//�趨ϵͳ��ָ���ܼ��鱾��������ִ�״̬.
	{
		return CEngCtrl::SetSysEngTurnsStatus(iGrp, bTurnsStatus, 1);
	}
	bool IsEnergyFee(void)								//�Ƿ񹺵�ѿ���
	{
		return m_fEnergyFeeFlag;
	}

protected:
	void SubRstCtrl(void);
	void RstCtrl(void);									//��λ�ڴ��б������״̬��.
	bool GetSysCmd(int iGrp);							//��ȡĳ�ܼ���ı����������,��������ŵ� m_NewCmd ��.(ע��: �Բ�ͬ����,m_NewCmd�Ľṹ�ǲ�ͬ��)
	bool ClrSysCmd(int iGrp);							//���ϵͳ�Ȿ�ܼ��鱾���������.
	bool SetSysCtrlTurnsStatus(int iGrp, BYTE bTurnsStatus);

	bool GetSysStatus(void)								//���ڳ�ʼ��ʱ,��ϵͳ���б�����Ƶ��ִ�״̬,����״̬��ͬ�����ڴ��ж�Ӧ�ı���.
	{
		return GetSysEngStatus(1);	//�����Ϊ1
	}
	void ClrCmd(void)									//����ڴ��б�����ƵĿ�������.
	{
		memset(&m_CtrlCmd, 0, sizeof(m_CtrlCmd));
	}
	char* CtrlType(char* psz)							//��ÿ��Ƶ�����(���ؿ������͵��ַ�������).
	{
		sprintf(psz, "BuyCtrl");
		return psz;
	}
	int CtrlType(void)									//��ÿ��Ƶ�����(������������).
	{
		return CTL_ENG_BUY;
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
		return ((CGrpCtrl::GetSysCtrlFlgs(iGrp, 1)&0x02) != 0);	//�����ʹ��1λ.
	}
	bool SetSysCtrlFlg(int iGrp, bool fStatus)			//�����ܼ���ϵͳ�Ȿ����Ʊ�־��Ϊ��Ч.
	{
		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x02, fStatus, ENG_CTL); //�����ʹ��1λ.
	}
	bool GetSysCtrlAlr(int iGrp)						//��ȡָ���ܼ���Ĺ���ر�����־λ.
	{
		return ((CEngCtrl::GetSysEngAlrFlgs(iGrp)&0x02) != 0);	//�����ʹ��1λ.
	}
	bool SetSysCtrlAlr(int iGrp, bool fStatus)			//����ϵͳ��ָ���ܼ���Ĺ���ر�����־λ.
	{
		return CEngCtrl::ChgSysEngAlrFlgs(iGrp, 0x02, fStatus);//�����ʹ��1λ.
	}
	bool ChgSysTurnsStatus(int iGrp, BYTE bTurns, bool fStatus)	//�ı�ϵͳ��ָ���ܼ��鱾���������Ӧ�ִ�״̬.
	{
		return CEngCtrl::ChgSysEngTurnsStatus(iGrp, bTurns, fStatus, 1);
	}

	void UpdateSysRemainEng(void);					//����ϵͳ�������ܼ���ʣ�๺����.
	void UpdateBuyRemainEng(void);					//����ϵͳ�������ܼ���ʣ�๺����.	
	bool GetBuyCtrlPara(int iGrp, TBuyCtrlPara& rPara);//��ȡ��ǰ�ܼ���'�����'����.
	bool GetCurFeeRatio(int64 *iCurFeeRatio);

	void SaveDisp();
	int64 GetGroupEng(int iGrp);


protected:
	TBuyCtrlCmd			m_NewCmd;						//����GetSysCmd(int iGrp)������,��ȡ����������浽�ñ�����.
	TBuyCtrlCmd			m_CtrlCmd;						//��ǰʹ�õ�����.

	int64				m_iBuyRemain;					//ʣ�๺����.
	TBuyCtrlPara		m_BuyCtrlPara;					//'�����'����.

	//int64				m_iBaseBuyRemainEng[GRP_NUM];	//�����ܼ��鹺��ʣ�������ʼֵ.
	int64				m_iCurBuyRemainEng[GRP_NUM+1];	//�����ܼ��鹺��ʣ�������ǰֵ.
	int64				m_iBaseEng[GRP_NUM+1];			//�����ܼ����ϵ�ʱ�Ļ����õ���.
	DWORD				m_dwBillIdx[GRP_NUM+1];			//�����ܼ����Ѵ���Ĺ��絥��.
	bool				m_fUpBaseEng[GRP_NUM+1];			//�����ܼ�����һ���ӵ����Ƿ���³ɹ���־.
	BYTE 				m_bCount;
	bool				m_fEnergyFeeFlag;					//����ѿر�־
	int64				m_iCurFeeRatio;						//��ǰ����
};

#endif  //BUYCTRL_H
