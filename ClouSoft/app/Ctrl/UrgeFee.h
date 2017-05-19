/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�UrgeFee.h
 * ժ    Ҫ�����ļ���Ҫʵ��CUrgeFee��Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��

- 2008-04-17 10:25
����:
��ǰ�߷Ѹ澯״̬��
��D0��ʾ�ն˵�ǰ�Ƿ��ڴ߷Ѹ澯״̬���á�1�����ն˴��ڴ߷Ѹ澯״̬���á�0�����ն�δ���ڴ߷Ѹ澯״̬��

���״̬�����ȷ����
�Ǵ߷Ѹ澯Ͷ���˾���λ�������ڸ澯ʱ����λ(��8ʱ ~ 20ʱ),���������ڱ�������1��������λ.
*********************************************************************************************************/
#ifndef URGEFEE_H
#define URGEFEE_H

#include "CtrlBase.h"

typedef struct
{
	BYTE	bAct;			//����(0<�޶���>, 1<�߷Ѹ澯Ͷ��>, 2<�߷Ѹ澯���>)
	BYTE    bFlag[3];		//�߷Ѹ澯Ͷ���־
	DWORD	dwTime;			//��������ʱ��(��2000��1��1��0ʱ0��0�뵽���ڵ�����)
} TUrgeFeeCmd;

//========================================= CUrgeFee =============================================
class CUrgeFee : public CCtrl
{
public:
	CUrgeFee(void);
	virtual ~CUrgeFee();

	bool DoCtrl(void);									//'�߷Ѹ澯'����.
	bool IsBeepAlr(void)								//�Ƿ���������.
	{
		return m_fAlrStatus;
	}

protected:                          			
	void DoCmdScan(void);								//ɨ��ϵͳ���е�'�߷Ѹ澯'����.
	void ClrCmd(void)									//����ڴ���'�߷Ѹ澯'�Ŀ�������.
	{
		memset(&m_CtrlCmd, 0, sizeof(TUrgeFeeCmd));
	}
	void RstCtrl(void);									//��λ'�߷Ѹ澯'����״̬.
	bool ClrSysCmd(void);								//���ϵͳ��'�߷Ѹ澯'����.

	bool SetSysCtrlStatus(bool fStatus);				//�趨ϵͳ��'�߷Ѹ澯'״̬.
	bool RstSysCtrlStatus(void)							//��λϵͳ��'�߷Ѹ澯'״̬.
	{
		return SetSysCtrlStatus(false);
	}
	bool SetSysCurStatus(bool fStatus);

protected:
	TUrgeFeeCmd		m_CtrlCmd;							//'�߷Ѹ澯'����.
	bool			m_fLaunch;
	DWORD			m_dwAlrStartTime;					//��������ʱ��.
	bool			m_fAlrStatus;						//��ǰСʱ�Ƿ��ڱ���״̬.
};

#endif  //URGEFEE_H
