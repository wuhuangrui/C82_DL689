/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Guarantee.h
 * ժ    Ҫ�����ļ���Ҫʵ��CGuarantee��Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��
 *********************************************************************************************************/
#ifndef GUARANTEE_H
#define GUARANTEE_H

#include "CtrlBase.h"

typedef struct
{
	BYTE	bAct;			//����(0<�޶���>, 1<����Ͷ��>, 2<������>)
	DWORD	dwPersistTime;	//����ʱ��(0.5Сʱ��)
	DWORD	dwTime;			//��������ʱ��(��2000��1��1��0ʱ0��0�뵽���ڵ�����)
} TGuaranteeCmd;

//========================================= CGuarantee =============================================
class CGuarantee : public CCtrl
{
public:
	CGuarantee(void){}
	virtual ~CGuarantee(){}

	bool Init(void);
	bool DoCtrl(void);									//'�����'����.
	bool SetSysCtrlStatus(BYTE bStatus);	//�趨ϵͳ��'�����'״̬.
protected:                          			
	void DoCmdScan(void);								//ɨ��ϵͳ���е�'�����'����.
	bool IsAutoGuaranteePeriod();								//�Ƿ��Զ�����ʱ��
	void ClrCmd(void)									//����ڴ���'�����'�Ŀ�������.
	{
		memset(&m_CtrlCmd, 0, sizeof(TGuaranteeCmd));
	}
	void RstCtrl(void);									//��λ'�����'����״̬.
	bool ClrSysCmd(void);								//���ϵͳ��'�����'����.


	bool RstSysCtrlStatus(void)							//��λϵͳ��'�����'״̬.
	{
		return SetSysCtrlStatus(QUIT_GUARANTEE);
	}

protected:
	TGuaranteeCmd		m_CtrlCmd;						//'�����'����.
	bool				m_fUnconnect;					//�Ƿ�δ����(��������ͨѶ�Ƿ�ʱ).
};

#endif  //GUARANTEE_H
