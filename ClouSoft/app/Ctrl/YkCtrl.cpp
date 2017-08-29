/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�YkCtrl.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��CYkCtrl����
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��
*********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "sysfs.h"
#include "FaCfg.h"
#include "DbConst.h"
#include "FaConst.h"
#include "ComAPI.h"
#include "TaskDB.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "TaskManager.h"
#include "YkCtrl.h"
#include "FaAPI.h"
#include "TermEvtTask.h"

//========================================= CYkCtrl =============================================
CYkCtrl::CYkCtrl(BYTE bTurn)
: m_iTurn(bTurn), m_wOpenTimes(0), m_dwFrzDly(60 * 2)	//ң������ʱ2����.
{
	if (m_iTurn<TURN_START_PN && m_iTurn>TURN_START_PN+TURN_NUM)
		m_iTurn = TURN_START_PN;

	m_fTurnStatus = false;					
	m_fAlrStatus = false;
	m_fRxCloseCmd = false;
	m_fAlarmStatus = false;
	m_fCloseStatus = true;
	m_fOpenStatus = false;
	m_dwOpenClick = 0;
	m_dwCloseClick = 0;
	m_dwOpenClk = 0;
}

//����: ����'ң��'�ĵ�ǰ�ִ�.
//����:@iTurn	�趨���ִ�.
//����: ������óɹ����� true,���򷵻� false.
bool CYkCtrl::SetTurn(int iTurn)
{
	if (iTurn<TURN_START_PN || iTurn>TURN_START_PN+TURN_NUM)
		return false;

	m_iTurn = iTurn;

	return true;
}

//����: ɨ��ϵͳ���е�����.
void CYkCtrl::DoCmdScan(void)
{
	TYkCtrlCmd NewCmd;
	BYTE bCmd[10];

	m_fRxCloseCmd = false;					//����ִ���Ƿ��յ���һ��ң�غ�բ����
	if (ReadItemEx(BN0, (WORD)m_iTurn, 0x8203, bCmd, &NewCmd.dwTime) <= 0)	//����ӦID�Ĳ������"�ն�ң��Ͷ������"
	{
		DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCmdScan: There is something wrong when call ReadItemEx() !\n"));
		return;
	}

	if (bCmd[0] != 1 && bCmd[0] != 2)
		return ;

	NewCmd.bAct = bCmd[0];
//	if (NewCmd.bAct!=1 && NewCmd.bAct!=2) //��ʾδ�յ�ң�����
//		return;
	NewCmd.bAlrTime = bCmd[2];
	NewCmd.dwPersistTime = OoLongUnsignedToWord(bCmd+4)*60;
	
	char cTime[20];
	//���ڵ�ʱ����·������ʱ��Ҫ�磨����ʱ�޸��ն�ʱ�䣩�����������Ҫ������
	if (NewCmd.dwTime == 0) //�ն�ʱ����ǰ����,ϵͳ���е�����ʱ�䱻�����
	{	
		TTime tmCmd;
		SecondsToTime(m_CtrlCmd.dwTime, &tmCmd);
		if (m_tmNow.nDay!=tmCmd.nDay || m_tmNow.nMonth!=tmCmd.nMonth || m_tmNow.nYear!=tmCmd.nYear)
		{
			RstSysCtrlStatus(m_iTurn);	//���������ϵͳ������ǰ,����ϵͳ��'ң��'״̬.
			ClrSysCmd(m_iTurn);
			ClrCmd();
			RstCtrl(); 
			SetValidStatus(false);	//�趨�����˳�״̬.
			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCmdScan: turn=%d quit due to cmd time==0 and dan change\n", m_iTurn));
			return;
		}

		if (NewCmd.dwPersistTime == 0)	//��ʱ���޵�
		{
			NewCmd.dwTime = GetCurTime(); //����Ϊһ������һ���ʱ��,�����Ժ�ͬ����ʱ��ıȽ�
			WriteItemEx(BN0, (WORD)m_iTurn, 0x8203, bCmd, NewCmd.dwTime);	//����Ӧ�ܼ����"ʱ�ο�����"д�����ݿ�
		}
		else if (m_CtrlCmd.dwTime!=0 //ָ���޵�ʱ��,�Ҿɵ������Ѿ���ɨ�����ִ��
				 && m_dwOldTime>=m_CtrlCmd.dwTime && m_dwOldTime<m_CtrlCmd.dwTime+m_CtrlCmd.dwPersistTime)
		{		//�������ʱ�䵱��click�ķ�ʽ����
			NewCmd.dwTime = GetCurTime() - (m_dwOldTime-m_CtrlCmd.dwTime); //ȥ���Ѿ����ĵ���ʱ��
			WriteItemEx(BN0, PN0, 0x8203, bCmd, NewCmd.dwTime);	//����Ӧ�ܼ����"ʱ�ο�����"д�����ݿ�
		}
		else
		{
			RstSysCtrlStatus(m_iTurn);	//���������ϵͳ������ǰ,����ϵͳ��'ң��'״̬.
			ClrSysCmd(m_iTurn);
			ClrCmd();
			RstCtrl();
			SetValidStatus(false);	//�趨�����˳�״̬.
			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCmdScan: turn=%d quit due to cmd time==0 and ctrl timeout\n", m_iTurn));
			return;
		}
	}

	//����յ��Ĳ���1��2,������Ե�ǰ�Ŀ���״̬����Ӱ��,���ڽ��յ��µ���Ч����ǰ�ն�
	//һ������,�ϵ��,��ǰ�ִν������ң�ؽ��״̬(��Ϊ�󱻳�ʼ��Ϊң�ؽ��״̬).
	if (NewCmd.dwTime != m_CtrlCmd.dwTime)
	{//ʱ�겻ͬ,��ʾ���յ��µ�����,��������.
		m_CtrlCmd = NewCmd;
		DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCmdScan: rx new cmd at %s, turn=%d, act=%d, alr-time=%d, persist-time=%ld\n",
							 TimeToStr(NewCmd.dwTime, cTime), m_iTurn, NewCmd.bAct, NewCmd.bAlrTime, NewCmd.dwPersistTime));
		if (NewCmd.bAct == 1)		//ң��Ͷ������.
		{
			SetValidStatus(true);
			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCmdScan: turn=%d launch at %s\n", 
								 m_iTurn, TimeToStr(m_tmNow, cTime)));
			//***��¼��ϵͳ����־��.
			//***���������ź�;
		}
		else //(NewCmd.bAct == 2)	//ң�ؽ������.
		{
			m_fRxCloseCmd = true;		//����ִ���Ƿ��յ���һ��ң�غ�բ����
			RstSysCtrlStatus(m_iTurn);	//���������ϵͳ������ǰ,����ϵͳ��'ң��'״̬.
			ClrSysCmd(m_iTurn);
			ClrCmd();
			RstCtrl();
			SetValidStatus(false);	//�趨�����˳�״̬.
			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCmdScan: Turn[%d] YkCtrl quit at %s\n", 
								 m_iTurn, TimeToStr(m_tmNow, cTime)));
			//***��¼��ϵͳ����־��.
			//***���������ź�;
		}
	}
}

//����: ����ң����բ��¼.
void CYkCtrl::DoSaveOpenRec(void)
{
	if (m_dwOpenClk != 0)
	{//������բ�������,��������բ��2���Ӽ�¼���Ṧ��.
		if (GetClick() >= m_dwOpenClk+120)
		{
			BYTE *pbPtr = g_YKCtrl.bArrayPow;
			memset(g_YKCtrl.bArrayPow, 0, sizeof(g_YKCtrl.bArrayPow));
			*pbPtr++ = DT_ARRAY;
			*pbPtr++ = 1;
			pbPtr += ReadItemEx(BN0, PN1, 0x2302, pbPtr);
			m_dwOpenClk = 0;
		}
	}
}

//����: ���п���.
//����: �����򷵻� true,���򷵻� false.
bool CYkCtrl::DoCtrl(void)
{
	DoSaveOpenRec();	//����ң����բ��¼.
	DoCmdScan();		//ɨ��ϵͳ���е�����.

	if (!IsValid())
	{
		RstCtrl();
		return true;
	}

	char cTime[20];
	TTime tmCmd;
/*
	if (IsGuarantee())	//���ڱ���״̬,ң�ظ�λ,������ң������
	{
		if (!m_fGuarantee)
		{
			m_fGuarantee = true;
			RstSysCtrlStatus(m_iTurn);	//����ϵͳ��'ң��'״̬.
			ClrCmd();
			RstCtrl();
			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: YK%d quit for Guarantee\n", m_iTurn));
		}

		return true;
	}

	m_fGuarantee = false;
*/
	SecondsToTime(m_CtrlCmd.dwTime, &tmCmd);
	if (m_tmNow.nDay!=tmCmd.nDay || m_tmNow.nMonth!=tmCmd.nMonth || m_tmNow.nYear!=tmCmd.nYear)
	{	//���������л�,ң�ظ�λ.
		RstSysCtrlStatus(m_iTurn);	//���������ϵͳ������ǰ,����ϵͳ��'ң��'״̬.
		ClrSysCmd(m_iTurn);
		ClrCmd();
		RstCtrl();
		SetValidStatus(false);	//�趨�����˳�״̬.
		DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: turn=%d quit for day change\n", m_iTurn));
		//***��¼��ϵͳ����־��.
		//***���������ź�;
		return true;
	}

	DWORD dwCntDown;
	DWORD dwTime = m_CtrlCmd.dwTime + (DWORD)m_CtrlCmd.bAlrTime*60; //��բ��ʼʱ��

	if (m_dwNow < dwTime)
	{//���ڸ澯��ʱʱ����.
		if (!m_fAlrStatus)
		{
			SetSysTurnStatus(m_iTurn, false);	//�ڸ澯ʱ����,Ӧ�����բ.
			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: YK%d alarm start at %s\n",
								 m_iTurn, TimeToStr(m_tmNow, cTime)));
			//***���������ź�;
			SetSysTurnAlrStatus(m_iTurn, true);
		}
		RstCtrl();
		m_fAlrStatus = true;	//�ָ�'ң��'����״̬.

		dwCntDown = dwTime - m_dwNow;
		//SaveDisp(dwCntDown);
	}
	else if (m_dwNow <= (m_CtrlCmd.dwTime+m_CtrlCmd.dwPersistTime) || m_CtrlCmd.dwPersistTime==0)
	{//�����޵�ʱ���ڻ��޵�ʱ��Ϊ0.
		if (IsGuarantee())	//���ڱ���״̬,ң�ظ�λ,������ң������
		{
			if (!m_fGuarantee)
			{
				m_fGuarantee = true;
				RstSysCtrlStatus(m_iTurn);	//����ϵͳ��'ң��'״̬.
				ClrCmd();	
				RstCtrl();
				DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: YK%d quit for Guarantee\n", m_iTurn));
			}
			return true;
		}
		m_fGuarantee = false;

		dwCntDown = 0;
		//SaveDisp(dwCntDown);

		m_fAlrStatus = false;	//��ΪҪ��բ,���Ա���û������,�رձ���.
		if (!m_fTurnStatus)		//�ո澯�껹û��բ,�����ھ�Ҫ��
		{
			SetSysTurnStatus(m_iTurn, true);	//ң����բ.
			SetSysTurnAlrStatus(m_iTurn, false);
			m_wOpenTimes++;						//��բ��������1.

			g_YKCtrl.bEvtSrcOAD[0] = DT_OAD;
			DWORD dwOad = 0xF2050200 + m_iTurn;
			OoDWordToOad(dwOad, &g_YKCtrl.bEvtSrcOAD[1]);
			SetInfo(INFO_YK_REC);
			m_dwOpenClk = GetClick();
			//BYTE bBuf[1+1+6*8];

			/*if (ReadItemEx(BN0, PN0, 0x104f, bBuf) <= 0)//��"�ն˿�������״̬".
			{
				DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: There is something wrong when call ReadItemEx() !\n"));
				return false;
			}*/

			//BYTE bRecBuf[12];

			//memcpy(bRecBuf, &m_dwNow, 4);	//������բʱ��.

			/*if (IsGrpValid(1))   //ң����բ�м�����
			{
				bRecBuf[4] = 1;		//�ܼ����
				DWORD wPwr = GetCurPwr(1);
				Val32ToBin(wPwr, bRecBuf+5, 4);//��բʱ���ʣ��ܼӹ��ʣ�
			}
			else
			{
				memset(bRecBuf+5, INVALID_DATA, 4); //�ܼ����+��բʱ���ʣ��ܼӹ��ʣ�
			}
			WriteItemEx(BN0, (WORD)m_iTurn, 0x0a00, bRecBuf);*/

			//TrigerSaveBank(BN0, SECT_CTRL, 0);

			DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: turn=%d open at %s\n", 
								 m_iTurn, TimeToStr(m_tmNow, cTime)));
			//***���������ź�;�ִ� bTurn ң������բ.
		}
		m_fTurnStatus = true;
	}
	else
	{//�޵�ʱ���ѽ���.
		RstSysCtrlStatus(m_iTurn);	//���������ϵͳ������ǰ,����ϵͳ��'ң��'״̬.
		ClrSysCmd(m_iTurn);
		ClrCmd();
		RstCtrl();
		SetValidStatus(false);	//�趨�����˳�״̬.
		DTRACE(DB_LOADCTRL, ("CYkCtrl::DoCtrl: YK%d finish at %s\n", 
							 m_iTurn, TimeToStr(m_tmNow, cTime)));
		//***�����ź�.
	}

	return true;
}

//����:����ң����բ����ʱʱ�䣻
//������@dwCntDown - ң����բ����ʱ������
void CYkCtrl::SaveDisp(DWORD dwCntDown)
{
	BYTE bBuf[20];
	WORD wAlrtime;
	memset(bBuf, 0, sizeof(bBuf));
	//д����ʼ����ʱʱ��
	memcpy(bBuf,&m_CtrlCmd.dwTime,sizeof(m_CtrlCmd.dwTime));	
	wAlrtime = m_CtrlCmd.bAlrTime*60;
	
	memcpy(bBuf+4,&wAlrtime,sizeof(wAlrtime));
	WriteItemEx(BN0, m_iTurn, 0x0910, bBuf); //ң�ظ澯��Ϣ����
}

//��������ϵͳ����ɾ����Ч��ң��״̬������ʾ�ã���
//������@tInvCtrl  ��Ч��ң��
void CYkCtrl::RemoveDispItem(TCtrl tInvCtrl)
{
	BYTE bBuf[21];
	memset(bBuf, 0, sizeof(bBuf));

	if (ReadItemEx(BN1, PN0, 0x3010, bBuf)>0 && bBuf[0]>0)
	{
		BYTE bSize = bBuf[0]; //�澯�ĸ�����
		int iIndex = -1; //ʧЧ����ʾ�澯�����ڶ����е�λ�ã�

		//����ʧЧ����ʾ�澯�����������е�����
		for (BYTE i=0; i<bSize; i++)
		{
			if (bBuf[i*2+1]==tInvCtrl.bCtrlType && bBuf[i*2+2]==tInvCtrl.bCtrlTurn)
			{
				iIndex = i;
				break;
			}
		}

		//��������
		if (iIndex >= 0)
		{
			bSize --;
			bBuf[0] = bSize;
			BYTE* pbBuf = &bBuf[iIndex*2+3];
			memcpy(bBuf+iIndex*2+1, pbBuf, (bSize-iIndex)*2);
			memset(bBuf+bSize*2+1, 0, 20-bSize*2); 
			WriteItemEx(BN1, PN0, 0x3010, bBuf);
		}
	}

}

//��������ϵͳ���������µ�ң��״̬������ʾ�ã���
//������@tTopCtrl �µ�ң�أ�����������ǰ�棩
void CYkCtrl::AddDispItem(TCtrl tTopCtrl)
{
	BYTE bBuf[21], bTmpBuf[21];
	memset(bBuf, 0, sizeof(bBuf));

	if (ReadItemEx(BN1, PN0, 0x3010, bTmpBuf) > 0)
	{
		BYTE bSize = bTmpBuf[0]; //��ǰ�ĸ�����

		//��������ѱ�������͵���ʾ��Ϣ��������ӣ�
		for (BYTE i=0; i<bSize; i++)
		{
			if (bTmpBuf[1+2*i]==tTopCtrl.bCtrlType && bTmpBuf[2+2*i]==tTopCtrl.bCtrlTurn)
				return;
		}

		BYTE *pbBuf = &bTmpBuf[1];
		bBuf[1] = tTopCtrl.bCtrlType;
		bBuf[2] = tTopCtrl.bCtrlTurn;
		if (bSize == 10)
		{
			memcpy(bBuf+3, pbBuf, 18);
		}
		else
		{
			memcpy(bBuf+3, pbBuf, bSize*2);
			memset(bBuf+bSize*2+3, 0, 18-bSize*2);
			bSize++;
		}
		bBuf[0] = bSize; 
		WriteItemEx(BN1, PN0, 0x3010, bBuf);
	}
}

//��������ϵͳ���������µ�ң�أ�����ʾ�ã���
//ң����ʾ��Ϊ�������֣�1.ң�ظ澯����ʱ���棻 2.ң����բ(30s)��3.ң�غ�բ(30s).
void CYkCtrl::MakeDisp()
{
	TCtrl tCtrl;
	TCtrl tInvCtrl;

	bool fCloseStatus = IsCloseStatus();
	bool fAlarmStatus = IsBeepAlr();	//ȡm_fAlrStatus
	bool fOpenStatus = GetTurnStatus(); //ȡm_fTurnStatus

	//�澯
	tCtrl.bCtrlTurn = m_iTurn;
	tInvCtrl.bCtrlTurn = tCtrl.bCtrlTurn;
	if (fAlarmStatus != m_fAlarmStatus)	//m_fAlarmStatusֻ����ʾ����
	{
		if (fAlarmStatus)
		{//�澯��ʾ��ʼ��
			tCtrl.bCtrlType = GetCtrlType();
			AddDispItem(tCtrl);
		}
	}

	//��բ
	if (fOpenStatus != m_fOpenStatus) //m_fOpenStatusֻ����ʾ����
	{//�澯��ʾ��������բ��ʾ��ʼ��
		if (fOpenStatus)
		{
			tCtrl.bCtrlType = GetCtrlType();
			AddDispItem(tCtrl);	//�������բ������澯������һ����,������Щ�澯��ֱ����բ��
			m_dwOpenClick = GetClick();
		}
	}
	else 
	{
		if (m_dwOpenClick > 0)
		{
			m_dwOpenClick++;
			if (GetClick()-m_dwOpenClick > CTL_TURNCLOSE_TICK)
			{//��բ��ʾ������
				tInvCtrl.bCtrlType = GetCtrlType();
				RemoveDispItem(tInvCtrl);
				m_dwOpenClick = 0;
			}
		}
	}

	//��բ
	if (fCloseStatus)
	{//ֻҪ������բ�źţ�ң����բ��ʾ��ʧ��
		tCtrl.bCtrlType = GetCtrlType();
		RemoveDispItem(tCtrl);
		m_dwOpenClick = 0;
	}
	
	if (fCloseStatus != m_fCloseStatus)
	{
		if (fCloseStatus)
		{//��ң��״̬��Ϊ��բ��������բ��ʾ���棻
			tCtrl.bCtrlType = GetInvCtrlType();	//��բ����
			AddDispItem(tCtrl);
			m_dwCloseClick = GetClick();
		}
		else
		{//ֻҪң��״̬�У�ң�غ�բ�ź���ʧ����ң�غ�բǰ�ض���ң��Ͷ�룻
			tInvCtrl.bCtrlType = GetInvCtrlType();
			RemoveDispItem(tInvCtrl);
			m_dwCloseClick = 0;
		}
	}
	else if (fCloseStatus)
	{
		if (GetClick()-m_dwCloseClick >= CTL_TURNCLOSE_TICK)
		{//��բ��ʾ30s��������
			tInvCtrl.bCtrlType = GetInvCtrlType();
			RemoveDispItem(tInvCtrl);
			m_dwCloseClick = 0;
		}
	}

	m_fAlarmStatus = fAlarmStatus;
	m_fCloseStatus = fCloseStatus;
	m_fOpenStatus = fOpenStatus;
}


//����: ��λ'ң��'����״̬.
void CYkCtrl::RstCtrl(void)
{
	m_fTurnStatus  = false;
	m_fAlrStatus   = false;
}

//����: ���ϵͳ��'ң��'����.
bool CYkCtrl::ClrSysCmd(int iTurn)
{
	if (m_iTurn<TURN_START_PN || m_iTurn>TURN_START_PN+TURN_NUM)
		return false;

	BYTE bCmd[10];

	memset(bCmd, 0, sizeof(bCmd));
	WriteItemEx(BN0, (BYTE)iTurn, 0x8203, bCmd);	//д��Ӧ�ִε�"�ն�ң��Ͷ������"ID

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//��������.

	return true;
}

//����: �趨ϵͳ��ָ���ܼ��鱾���������Ӧ�ִ�״̬.
//����:@fStatus		true: ��λ; false: ���.
//����: ������óɹ����� true,���򷵻� false.
bool CYkCtrl::SetSysTurnStatus(int iTurn, bool fStatus)
{
	if (iTurn<TURN_START_PN || iTurn>TURN_START_PN+TURN_NUM)
		return false;

	BYTE bRelayMode;
	BYTE bBuf[32];	//���8������

	//!!!����ڱ���߳��л�д��ID,������Ҫ�����ź�������
	if (ReadItemEx(BN0, PN0, 0x8200, bBuf) <= 0)	//��"�̵������״̬".
	{
		DTRACE(DB_LOADCTRL, ("CYkCtrl::SetSysTurnStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	bBuf[0] = DT_BIT_STR;
	bBuf[1] = 8;
	if (fStatus)
		bBuf[2] |= (0x01<<(iTurn-TURN_START_PN)) & CTL_TURN_MASK;		//���浱ǰ�ִ�'ң����բ���״̬'.
	else
		bBuf[2] &= ~((0x01<<(iTurn-TURN_START_PN)) & CTL_TURN_MASK);	//���浱ǰ�ִ�'ң����բ���״̬'.
	WriteItemEx(BN0, PN0, 0x8200, bBuf);	//д"�̵������״̬".

	if (ReadItemEx(BN0, iTurn-1, 0xF205, bBuf) <= 0)	//��"�̵������״̬".
	{
		DTRACE(DB_LOADCTRL, ("CYkCtrl::SetSysTurnStatus: There is something wrong !\n"));
		return false;
	}

	BYTE *pbPtr = bBuf;
	*pbPtr++ = DT_STRUCT;
	*pbPtr++ = 4;
	*pbPtr++ = DT_VIS_STR;
	*pbPtr++ = 16;
	memset(pbPtr, 0x30, 16);
	pbPtr += 16;
	*pbPtr++ = DT_ENUM;
	*pbPtr++ = fStatus;
	ReadItemEx(BN1, PN0, 0x2022, &bRelayMode);
	*pbPtr++ = DT_ENUM;
	*pbPtr++ = (bRelayMode==1) ? 0:1;
	*pbPtr++ = DT_ENUM;
	*pbPtr++ = 0;

	WriteItemEx(BN0, iTurn-1, 0xF205, bBuf);	//д"�̵������״̬".

	return true;
}

//����: �趨ϵͳ�Ȿ���������Ӧ�ִθ澯״̬.
//����:@fStatus		true: ��λ; false: ���.
//����: ������óɹ����� true,���򷵻� false.
bool CYkCtrl::SetSysTurnAlrStatus(int iTurn, bool fStatus)
{
	if (iTurn<TURN_START_PN || iTurn>TURN_START_PN+TURN_NUM)
		return false;

	BYTE bBuf[4];

	//!!!����ڱ���߳��л�д��ID,������Ҫ�����ź�������
	if (ReadItemEx(BN0, PN0, 0x8201, bBuf) <= 0)	//��"ң�ظ澯״̬".
	{
		DTRACE(DB_LOADCTRL, ("CYkCtrl::SetSysTurnAlrStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	bBuf[0] = DT_BIT_STR;
	bBuf[1] = 8;
	if (fStatus)
		bBuf[2] |= (0x01<<(iTurn-TURN_START_PN)) & CTL_TURN_MASK;		//���浱ǰ�ִ�'ң�ظ澯���״̬'.
	else
		bBuf[2] &= ~((0x01<<(iTurn-TURN_START_PN)) & CTL_TURN_MASK);	//���浱ǰ�ִ�'ң�ظ澯���״̬'.
	WriteItemEx(BN0, PN0, 0x8201, bBuf);	//д"ң�ظ澯״̬".

	return true;
}
