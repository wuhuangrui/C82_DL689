/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Guarantee.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��CGuarantee����
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
#include "Guarantee.h"

//========================================= CGuarantee =============================================
//����: ��ʼ��.
//����: �����ʼ���������� true,���򷵻� false.
bool CGuarantee::Init(void)
{
	ClrCmd();		//����ڴ��б�����ƵĿ�������.
	RstCtrl();		//��λ�ڴ��б������״̬��.

	m_fUnconnect = false;

	return true;
}

//����: '�����'����.
//����: �����򷵻� true,���򷵻� false.
bool CGuarantee::DoCtrl(void)
{
	char cTime[20];
	BYTE bBuf[1];

	//���������ͨѶ�Ƿ�ʱ.
	ReadItemEx(BN2, PN0, 0x2021, bBuf); //������ͨѶ��ʱ����뱣��״̬
	if (bBuf[0]==1 || IsAutoGuaranteePeriod())	//���뱣��״̬
	{
		if (!m_fUnconnect)
		{
			SetSysCtrlStatus(AUTO_GUARANTEE);
			SetValidStatus(true);
			ClrCmd();	//��������ڴ��е������,�Ա���ͨѶ�ָ���,�ܹ�����ɨ��ϵͳ������.
			m_fUnconnect = true;
			DTRACE(DB_LOADCTRL, ("CGuarantee::DoCtrl: Long time no communication with the host, Guarantee launch at %s\n", 
								 TimeToStr(m_tmNow, cTime)));
		}

		return true;
	}
	else
	{
		if (m_fUnconnect)
		{
			RstSysCtrlStatus();
			RstCtrl();
			m_fUnconnect = false;
			DTRACE(DB_LOADCTRL, ("CGuarantee::DoCtrl: Communication with the host resume, Guarantee quit at %s\n", 
								 TimeToStr(m_tmNow, cTime)));
		}
	}

	DoCmdScan();

	if (!IsValid())
	{
		RstCtrl();
		return true;
	}

	/*****����ն�ʱ����·�����ʱ���磬������ɨ�账�����жϣ��˴�������Ҫ***/ //Modified by chenxi,13th,June,2008
	//if (m_dwNow < m_CtrlCmd.dwTime)
	//{
	//	char cTime1[20];

	//	DTRACE(DB_LOADCTRL, ("CGuarantee::DoCtrl: Time of receive Guarantee command is later than now!\n"\
	//						 "Time of Receive is %s\nNow is %s\n",
	//						 TimeToStr(m_CtrlCmd.dwTime, cTime1), TimeToStr(m_tmNow, cTime)));
	//	//����;	//�����ڼ�¼�½�������ʱ���,ϵͳʱ������˵���,���򲻻ᷢ���������.
	//	return false;
	//}

	if (m_CtrlCmd.dwPersistTime==0 || m_dwNow<m_CtrlCmd.dwTime+m_CtrlCmd.dwPersistTime)
	{
		SetValidStatus(true);
	}
	else
	{//'�����'����ʱ���ѹ�,����״̬������Ч��Ϊ��Ч
		RstSysCtrlStatus();		//���������ϵͳ������ǰ,����ϵͳ��'�����'״̬.
		ClrSysCmd();
		ClrCmd();
		RstCtrl();
		DTRACE(DB_LOADCTRL, ("CGuarantee::DoCtrl: Guarantee finish at %s\n", 
							 TimeToStr(m_tmNow, cTime)));
		//***��¼��ϵͳ����־��.
		//***���������ź�;
	}

	return true;
}

//����: ɨ��ϵͳ���е�'�����'����.
void CGuarantee::DoCmdScan(void)
{
	TGuaranteeCmd NewCmd;
	BYTE bCmd[2];

	//�ȸ�������ʱ�����Ƿ��յ��µ�����.
	if (ReadItemEx(BN0, PN0, 0x8213, bCmd, &NewCmd.dwTime) <= 0)	//��"�ն˱���Ͷ������"ID
	{
		DTRACE(DB_LOADCTRL, ("CGuarantee::DoCmdScan: There is something wrong when call ReadItemEx() !\n"));
		return;
	}

	NewCmd.bAct = bCmd[0];
	NewCmd.dwPersistTime = 0;

	if (NewCmd.bAct !=1 && NewCmd.bAct!=2 && NewCmd.bAct!=3)	//û���յ�����
		return;

	char cTime[20];
	//���ڵ�ʱ����·������ʱ��Ҫ�磨����ʱ�޸��ն�ʱ�䣩�����������Ҫ������
	if (NewCmd.dwTime == 0)
	{
		if (NewCmd.dwPersistTime == 0) //�����ڱ���
		{
			NewCmd.dwTime = GetCurTime(); //����Ϊһ������һ���ʱ��,�����Ժ�ͬ����ʱ��ıȽ�
			WriteItemEx(BN0, PN0, 0x8213, bCmd, NewCmd.dwTime);	//����Ӧ�ܼ����"ʱ�ο�����"д�����ݿ�
		}
		else if (m_CtrlCmd.dwTime!=0 //�ɵ������Ѿ���ɨ�����ִ��
				 && m_dwOldTime>=m_CtrlCmd.dwTime && m_dwOldTime<m_CtrlCmd.dwTime+m_CtrlCmd.dwPersistTime)
		{		//�������ʱ�䵱��click�ķ�ʽ����
			NewCmd.dwTime = GetCurTime() - (m_dwOldTime-m_CtrlCmd.dwTime); //ȥ���Ѿ����ĵ���ʱ��
			WriteItemEx(BN0, PN0, 0x8213, bCmd, NewCmd.dwTime);	//����Ӧ�ܼ����"ʱ�ο�����"д�����ݿ�
		}
		else
		{
			RstSysCtrlStatus();	 //���������ϵͳ������ǰ,����ϵͳ��'�����'״̬.
			ClrSysCmd();
			ClrCmd();
			RstCtrl();
			DTRACE(DB_LOADCTRL, ("CGuarantee::DoCmdScan: Guarantee quit at %s\n",
								 TimeToStr(m_tmNow, cTime)));
		}
		return;
	}

	//����յ��Ĳ���1��2,������Ե�ǰ�Ŀ���״̬����Ӱ��,���ڽ��յ��µ���Ч����ǰ�ն�
	//һ������,�ϵ��,������뱣����״̬.
	if (NewCmd.dwTime != m_CtrlCmd.dwTime)
	{//ʱ�겻ͬ,��ʾ���յ��µ�����,��������.
		m_CtrlCmd = NewCmd;
		DTRACE(DB_LOADCTRL, ("CGuarantee::DoCmdScan: rx new cmd at %s, act=%d, persist-time=%ld\n",
							 TimeToStr(NewCmd.dwTime, cTime), NewCmd.bAct, NewCmd.dwPersistTime));
		if (NewCmd.bAct == 1)		//��������'�����'Ͷ������.
		{
			SetSysCtrlStatus(INPUT_GUARANTEE);
			SetValidStatus(true);
			DTRACE(DB_LOADCTRL, ("CGuarantee::DoCmdScan: guarantee launch at %s\n",
								 TimeToStr(m_tmNow, cTime)));
			//***��¼��ϵͳ����־��.
			//***���������ź�;
		}
		else //(NewCmd.bAct == 2)	//��������'�����'�������.
		{
			RstSysCtrlStatus();		//���������ϵͳ������ǰ,����ϵͳ��'�����'״̬.
			ClrSysCmd();
			ClrCmd();
			RstCtrl();
			DTRACE(DB_LOADCTRL, ("CGuarantee::DoCmdScan: guarantee quit at %s\n",
								 TimeToStr(m_tmNow, cTime)));
			//***��¼��ϵͳ����־��.
			//***���������ź�;
		}
	}
}

bool CGuarantee::IsAutoGuaranteePeriod()
{
	TTime tmNow;
	BYTE bBuf[150], bPeriodNum;

	if (ReadItemEx(BN0, PN0, 0x8212, bBuf) < 0)		//���Զ�����ʱ��
	{
		DTRACE(DB_LOADCTRL, ("CGuarantee::DoAutoGuarantee: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	bPeriodNum = bBuf[1];
	
	GetCurTime(&tmNow);
	for (BYTE i=0; i<bPeriodNum; i++)
	{
		if (bBuf[2+6*i+3]<=tmNow.nHour && tmNow.nHour<=bBuf[2+6*i+5])
			return true;
	}

	return false;
}

//����: ��λ'�����'����״̬.
void CGuarantee::RstCtrl(void)
{
	SetValidStatus(false);
}

//����: ���ϵͳ��'�����'����.
//����: �������ɹ����� true,���򷵻� false.
bool CGuarantee::ClrSysCmd(void)
{
	BYTE bCmd[2] = {0, 0};

	WriteItemEx(BN0, PN0, 0x8213, bCmd);	//д"�ն˱���Ͷ������"ID

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//��������.

	return true;
}

//����: �趨ϵͳ��'�����'״̬.���ÿ���״̬
//����:@bStatus	״̬�������0�������磨1�����Զ����磨2��
//����: ������óɹ����� true,���򷵻� false.
bool CGuarantee::SetSysCtrlStatus(BYTE bStatus)
{
	BYTE bBuf[2];
	TTime t;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	//!!!����ڱ���߳��л�д��ID,������Ҫ�����ź�������
	if (ReadItemEx(BN0, PN0, 0x8001, bBuf) < 0)		//��"�ն˿�������״̬"
	{
		DTRACE(DB_LOADCTRL, ("CGuarantee::SetSysCtrlStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	bBuf[0] = DT_ENUM;
	bBuf[1] = bStatus;
	WriteItemEx(BN0, PN0, 0x8001, bBuf, dwSecs);						//д"�ն˿�������״̬"

	return true;
}
