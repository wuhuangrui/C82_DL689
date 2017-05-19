/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�UrgeFee.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��CUrgeFee����
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��
*********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "sysfs.h"
#include "FaCfg.h"
#include "FaConst.h"
#include "FaAPI.h"
#include "ComAPI.h"
#include "TaskDB.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "UrgeFee.h"
#include "DrvAPI.h"

//========================================= CUrgeFee =============================================

CUrgeFee::CUrgeFee(void)
{
	memset(&m_CtrlCmd, 0, sizeof(m_CtrlCmd));	//'�߷Ѹ澯'����.
	m_fLaunch = false;
	m_fAlrStatus = false;						//��ǰСʱ�Ƿ��ڱ���״̬.
}

CUrgeFee::~CUrgeFee()
{

}

 
//����: '�߷Ѹ澯'����.
//����: �����򷵻� true,���򷵻� false.
bool CUrgeFee::DoCtrl(void)
{
	DoCmdScan();

	if (!IsValid())
	{
		RstCtrl();
		return true;
	}

	char cTime[20];

	bool fAlrStatus;
	if (GetBitStatus(m_CtrlCmd.bFlag, sizeof(m_CtrlCmd.bFlag), m_tmNow.nHour))
		fAlrStatus = true;		//��ǰСʱ�Ƿ��ڱ���״̬.
	else
		fAlrStatus = false;		//��ǰСʱ�Ƿ��ڱ���״̬.

	if (fAlrStatus != m_fAlrStatus)
	{
		m_fAlrStatus = fAlrStatus;
		SetSysCurStatus(m_fAlrStatus);
		SetSysCtrlStatus(m_fAlrStatus);
	}
	/*if ((m_fLaunch || m_tmNow.nHour!=m_tmOldTime.nHour) && GetBitStatus(bBuf, sizeof(bBuf), m_tmNow.nHour))
	{
		m_dwAlrStartTime = m_dwNow;
		m_fLaunch = false;
		DTRACE(DB_LOADCTRL, ("CUrgeFee::DoCtrl: alr start at %s\n", TimeToStr(m_tmNow, cTime)));
	}

	if (m_dwAlrStartTime!=0 && m_dwNow>m_dwAlrStartTime+60)	//Ĭ�ϸ澯����ʱ����1����.
	{
		m_dwAlrStartTime = 0;
		DTRACE(DB_LOADCTRL, ("CUrgeFee::DoCtrl: alr stop at %s\n", TimeToStr(m_tmNow, cTime)));
	}*/

	return true;
}

//����: ɨ��ϵͳ���е�'�߷Ѹ澯'����.
void CUrgeFee::DoCmdScan(void)
{
	char cTime[20];
	TUrgeFeeCmd NewCmd;

	BYTE bCmd[210];
	memset(bCmd, 0, sizeof(bCmd));
	//�ȸ�������ʱ�����Ƿ��յ��µ�����.
	if (ReadItemEx(BN0, PN0, 0x8220, bCmd, &NewCmd.dwTime) < 0)	//��"�ն˴߷Ѹ澯Ͷ������"ID
	{
		DTRACE(DB_LOADCTRL, ("CUrgeFee::DoCmdScan: There is something wrong when call ReadItemEx() !\n"));
		return;
	}
	if (bCmd[0] != 1 && bCmd[0] != 2)
		return ;
	NewCmd.bAct = bCmd[0];
	memcpy(NewCmd.bFlag, bCmd+5, 3);//bCmd[1],��ʾbitstring����24
	if (NewCmd.dwTime == 0) //�ն�ʱ����ǰ����,ϵͳ���е�����ʱ�䱻�����
	{
		if (m_CtrlCmd.dwTime != 0) //�ɵ������Ѿ�������,ֻ��ϵͳ���е�����ʱ�䱻�����
		{
			NewCmd.dwTime = GetCurTime(); //����Ϊһ������һ���ʱ��,�����Ժ�ͬ����ʱ��ıȽ�
			WriteItemEx(BN0, PN0, 0x8220, bCmd, NewCmd.dwTime);	//����Ӧ�ܼ����"Ӫҵ��ͣ������"д�����ݿ�
		}
		//else ������Ǳ���Ͷ��,ֻ�������ʱ�䱻�����,��Ҳ���ܸ�����һ����ȷ��ʱ��
		//	   ��������Ͷ���û���ü�ɨ��ͱ������,Ҳ�����Ǹ��Ѿ�����������ľ�����,
		// 	   ֻ��ͻȻ����û����ס;
		// 	   �ǳ�������ϵͳ��Ψһ��Ͷ�������,�������������ʱ�������,�����ᱻִ��
	}

	//����յ��Ĳ���1��2,������Ե�ǰ�Ŀ���״̬����Ӱ��,���ڽ��յ��µ���Ч����ǰ�ն�
	//һ������,�ϵ��,�������߷Ѹ澯���״̬.
	if (NewCmd.dwTime != m_CtrlCmd.dwTime)
	{//ʱ�겻ͬ,��ʾ���յ��µ�����,��������.
		DTRACE(DB_LOADCTRL, ("CUrgeFee::DoCmdScan: Receive new UrgeFee command, time of Receive is %s, .bAct=%d\n",
							 TimeToStr(NewCmd.dwTime, cTime), NewCmd.bAct));

		m_CtrlCmd = NewCmd;

		if (NewCmd.bAct == 1)		//��������'�߷Ѹ澯'Ͷ������.
		{
			SetSysCtrlStatus(true);
			SetValidStatus(true);
			m_fLaunch = true;
			DTRACE(DB_LOADCTRL, ("CUrgeFee::DoCmdScan: UrgeFee launch at %s\n",
								 TimeToStr(m_tmNow, cTime)));
			//***��¼��ϵͳ����־��.
			//***���������ź�;
		}
		else //(NewCmd.bAct == 2)	//��������'�߷Ѹ澯'�������.
		{
			RstSysCtrlStatus();		//���������ϵͳ������ǰ,����ϵͳ��'�߷Ѹ澯'״̬.
			ClrSysCmd();
			ClrCmd();
			RstCtrl();
			DTRACE(DB_LOADCTRL, ("CUrgeFee::DoCmdScan: UrgeFee quit at %s\n",
								 TimeToStr(m_tmNow, cTime)));
			//***��¼��ϵͳ����־��.
			//***���������ź�;
		}
	}
}

//����: ��λ'�߷Ѹ澯'����״̬.
void CUrgeFee::RstCtrl(void)
{
	SetValidStatus(false);
	if (m_fAlrStatus)
	{
		m_fAlrStatus = false;
		SetSysCtrlStatus(false);
	}
}

//����: ���ϵͳ��'�߷Ѹ澯'����.
//����: �������ɹ����� true,���򷵻� false.
bool CUrgeFee::ClrSysCmd(void)
{
	BYTE bCmd[210] = {0};

	WriteItemEx(BN0, PN0, 0x8220, bCmd);	//д"�ն˴߷Ѹ澯Ͷ������"ID

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//��������.

	return true;
}

//����: �趨ϵͳ��'�߷Ѹ澯'״̬.
//����:@fStatus		true: ��λ; false: ���.
//����: ������óɹ����� true,���򷵻� false.
bool CUrgeFee::SetSysCtrlStatus(bool fStatus)
{
	//����߷Ѹ澯����Ϊ���,�����"�ն˵�ǰ����״̬"�еĸ澯״̬
	if (!fStatus)
		SetSysCurStatus(false);

	return true;
}

bool CUrgeFee::SetSysCurStatus(bool fStatus)
{
	BYTE bBuf[2];	//���8������

	if (ReadItemEx(BN0, PN0, 0x8002, bBuf) <= 0)		//��"�߷Ѹ澯״̬"
	{
		DTRACE(DB_LOADCTRL, ("CUrgeFee::SetSysCtrlStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	bBuf[0] = DT_ENUM;
	if (fStatus)
		bBuf[1] = 1; //��ǰ�߷Ѹ澯״̬
	else
		bBuf[1] = 0; //��ǰ�߷Ѹ澯״̬

	WriteItemEx(BN0, PN0, 0x8002, bBuf);						//д"�߷Ѹ澯״̬"
	return true;
}

