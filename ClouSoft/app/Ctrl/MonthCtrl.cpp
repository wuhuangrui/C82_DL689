/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MonthCtrl.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��CMonthCtrl����
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
#include "DbConst.h"
#include "ComAPI.h"
#include "TaskDB.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "TaskManager.h"
#include "MonthCtrl.h"
#include "DbOIAPI.h"

//========================================= CMonthCtrl =============================================
CMonthCtrl::CMonthCtrl(void)
{
	memset(&m_OLStat, 0, sizeof(m_OLStat));
	m_OLStat.iGrp = -1;
    m_iCurMonthEng = 0;	
    m_iCurEngLimit = 0;	
    m_iCurAlarmLimit = 0;
	m_fAlrStauts = false;			//������״̬ȡ��.
	m_dwAlrStartTime = 0;
}

//����: ��ʼ��.
//����: �����ʼ���������� true,���򷵻� false.
bool CMonthCtrl::Init(void)
{
	ClrCmd();		//����ڴ��б�����ƵĿ�������.
	RstCtrl();		//��λ�ڴ��б������״̬��.
	SetValidStatus(false);	//�趨�����˳�״̬.

	return true; //return GetSysStatus();
}

//����: '�µ��'����.
//����: �����򷵻� true,���򷵻� false.
bool CMonthCtrl::DoCtrl(void)
{
	DoCmdScan();		//ɨ��ϵͳ���е�����.
	BeepAlrCtrl();

	if (!IsValid())		//����Ƿ�Ͷ�����.
	{
		RstCtrl();
		return true;
	}
/*
	if (IsGuarantee())
	{//��'���ڱ���״̬'.
		RstCtrl();					//��λ�ڴ��б�����Ƶ��������״̬.
		return true;
	}
*/
	TTime tmCmd;
	SecondsToTime(m_CtrlCmd.dwTime, &tmCmd);

	if (m_tmNow.nMonth!=m_tmOldTime.nMonth || m_tmNow.nYear!=m_tmOldTime.nYear)
	{//�緢�������л�(��⵽���ڵ�ʱ��ͽ��ܵ������ʱ���²�ͬ),'�µ��'��λ
		RstCtrl();					//��λ�ڴ��б�����Ƶ��������״̬.
		DTRACE(DB_LOADCTRL, ("CMonthCtrl::DoCtrl: grp%d reset for month change\n", m_iGrp));
		return true;
	}

	int i;
	BYTE b = GetSysCtrlTurnsCfg(m_iGrp);

	if ((m_bTurnsStatus&~b) != 0)
	{//����ܿ��ִη�����բ��,Ӧ����Щ�ִν��и�λ(��λ�����բ״̬).
		m_bTurnsStatus &= b;
	}
	//����Ƿ��п���բ.
	i = GetIdxOfMostRight1(b & ~GetTurnsStatus());	//��ȡ��Ӧ�ܼ��鵱ǰ����բ���ִκ�.
	m_bWarnStatus = i+1;
	char cTime[20];
	m_iCurMonthEng = GetMonthEng(m_iGrp);	//��ȡ��ǰ�ܼ��鱾�����õ���.	Ŀǰ������������û�����ܼ���ı������õ��� --QLS
	m_iCurEngLimit = GetMonthLimit(m_iGrp);	//��ȡ��ǰ�ܼ��鱾���¿ض�ֵ.
	m_iCurAlarmFactor = GetMonthAlarmFactor(m_iGrp);	//��ȡ��ǰ�ܼ��鱾���¿ض�ֵ.
	m_iCurAlarmLimit = m_iCurEngLimit * m_iCurAlarmFactor / 100;	//Ŀǰ�ڻ���������û���ҵ�����ֵ�Ķ���,��ʱ���ս��ո����н� 80% ��Ϊ�µ����ı���ֵ.

	if (m_iCurEngLimit<0 || m_iCurEngLimit<0)
		return true;

	if (m_iCurMonthEng<m_iCurAlarmLimit)
	{//����û��������.
		RstCtrl();					//��λ�ڴ��б�����Ƶ��������״̬.
	}
	else if (m_iCurMonthEng < m_iCurEngLimit)
	{//����µ����ѳ�������,��δ����բ��.
		SubRstCtrl();				//��λ�ڴ��б�����ƵĲ������״̬.
		if (i >= 0)
		{//���п���բ.
			if (!m_fAlrStauts)
			{
				SetSysCtrlAlr(m_iGrp, true);
#ifdef PRO_698
						//������բ��¼.
				/*BYTE bBuf[20];

				bBuf[0] = (BYTE)m_iGrp;					//�ܼ���
				bBuf[1] = GetSysCtrlTurnsCfg(m_iGrp);		//�ִ�
				bBuf[2] = 0x01;							//������
				Val64ToFmt3(m_iCurMonthEng, bBuf+3, 4);//�澯ʱ������
				Val64ToFmt3(m_iCurEngLimit, bBuf+7, 4);//�澯ʱ��������ֵ*/
				//��¼��ǰ�澯��¼��ϵͳ����.
				//SaveAlrData(ERC_ENGALARM, m_tmNow, bBuf);
#endif
				DTRACE(DB_LOADCTRL, ("CMonthCtrl::DoCtrl: alarm start at %s, grp=%d, current-energy=%lld, alarm-limit=%lld, act-Limit=%lld\n",
									 TimeToStr(m_tmNow, cTime), m_iGrp, m_iCurMonthEng, m_iCurAlarmLimit, m_iCurEngLimit));
			}
			m_fAlrStauts = true;
			//SaveDisp();
		}
		else
		{//��բ����.
			if (m_fAlrStauts)
				SetSysCtrlAlr(m_iGrp, false);
			m_fAlrStauts = false;
		}
	}
	else if (IsGuarantee())
	{//��'���ڱ���״̬'.
		RstCtrl();					//��λ�ڴ��б�����Ƶ��������״̬.
		m_fIfOverLimit = true;
		return true;
	}
	else	//(m_iCurMonthEng >= m_iCurEngLimit)
	{//����µ����ѳ���.
		m_fIfOverLimit = true;

		if (i < 0)									//����Ƿ��п���բ.
		{
			if (m_fAlrStauts)
				SetSysCtrlAlr(m_iGrp, false);
			m_fAlrStauts = false;					//��բ������,����û������,��ֹ����.
			return true;
		}

		if (m_dwOpenTurnTime > m_dwNow)	//���δ����ʱ����,ʱ����ǰ����ȥ��
			m_dwOpenTurnTime = 0;

		if (!m_fAlrStauts)
			SetSysCtrlAlr(m_iGrp, true);
		m_fAlrStauts = true;
		DWORD dwTurnInv = GetEngTurnInv(i+TURN_START_PN);	//��ȡ��Ӧ�ִεĹ��ر�������ʱ��.		
		if (m_dwOpenTurnTime == 0)					//���´���û����բ,��ֱ������С�ִε�բ,ͬʱ��¼�±�����բʱ��.
			m_dwOpenTurnTime = m_dwNow;
		else if (m_dwNow < m_dwOpenTurnTime+dwTurnInv)		//�ϴ���բ��,�����60�����������һ��բ
			return true;
		else
			m_dwOpenTurnTime = m_dwNow;				//�����ϴ���բ�ѳ���60��,�����ٴ���բ,ͬʱ��¼�±�����բʱ��.

		m_bTurnsStatus |= 0x01 << i;
		//SaveDisp();
		m_wOpenTimes++;								//��բ��������1.

		/*//������բ��¼.
		BYTE bBuf[1+1+1+4+4];

		bBuf[0] = (BYTE)m_iGrp;					//�ܼ���
		bBuf[1] = (BYTE)(0x01<<i);				//�ִ�
		bBuf[2] = 0x01;							//������
		Val64ToFmt3(m_iCurMonthEng, bBuf+3, 4);//��բʱ������
		Val64ToFmt3(m_iCurEngLimit, bBuf+7, 4);//��բʱ��������ֵ
		//��¼��ǰ��բ��¼��ϵͳ����.
		//SaveAlrData(ERC_ENGCTL, m_tmNow, bBuf);
		
		DTRACE(DB_LOADCTRL, ("CMonthCtrl::DoCtrl: turn=%d open at %s, grp=%d, current-energy=%lld, act-Limit is %lld\n",
							 i+TURN_START_PN, TimeToStr(m_tmNow, cTime), m_iGrp, m_iCurMonthEng, m_iCurEngLimit));*/
		//***���������ź�;
	}

	return true;
}

//�����������µ��Խ��ʱ����ʾ������
void  CMonthCtrl::SaveDisp()
{
	BYTE bBuf[13];
	bBuf[0] = m_bTurnsStatus;
	Val64ToFmt(m_iCurMonthEng, bBuf+1, FMT3, 4);
	Val64ToFmt(m_iCurAlarmLimit, bBuf+5, FMT3, 4);
	Val64ToFmt(m_iCurEngLimit, bBuf+9, FMT3, 4);
	WriteItemEx(BN0, PN0, 0x0930, bBuf);
}

void CMonthCtrl::SubRstCtrl(void)
{
	m_bTurnsStatus	 = 0x00;			//���ִ�״̬ȫ����Ϊ��բ.
	m_dwOpenTurnTime = 0;				//�ϴ���բʱ����Ϊ0;
	m_fIfOverLimit	 = false;			//����״̬��Ϊδ����.
}

//����: ��λ�ڴ��б������״̬��.
void CMonthCtrl::RstCtrl(void)
{
	if (m_fAlrStauts)
	{	
		SetSysCtrlAlr(m_iGrp, false);
		m_fAlrStauts = false;			//������״̬ȡ��.
	}

	m_dwAlrStartTime = 0;
	SubRstCtrl();
}

//����: ��ȡĳ�ܼ���ı����������,��������ŵ� m_NewCmd ��.(ע��: �Բ�ͬ����,m_NewCmd�Ľṹ�ǲ�ͬ��)
//����:@iGrp	Ҫ��ȡ������ܼ���.
//����: �����ȡ�ɹ���Ϊ��Ч���� true,���򷵻� false.
bool CMonthCtrl::GetSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[1];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8283, bCmd, &m_NewCmd.dwTime) <=0)	//��ȡ��Ӧ�ܼ����"�µ������".
	{
		DTRACE(DB_LOADCTRL, ("CMonthCtrl::GetSysCmd: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	m_NewCmd.bAct = bCmd[0];
	
	if (m_NewCmd.bAct!=1 && m_NewCmd.bAct!=2)
		return false;

	if (m_NewCmd.dwTime == 0) //�ն�ʱ����ǰ����,ϵͳ���е�����ʱ�䱻�����
	{
		if (CurCmdTime()!=0 && iGrp==m_iGrp) //�ɵ������Ѿ�������,ֻ��ϵͳ���е�����ʱ�䱻�����
		{
			m_NewCmd.dwTime = GetCurTime(); //����Ϊһ������һ���ʱ��,�����Ժ�ͬ����ʱ��ıȽ�
			WriteItemEx(BN0, (WORD)iGrp, 0x8283, bCmd, m_NewCmd.dwTime);	//����Ӧ�ܼ����"ʱ�ο�����"д�����ݿ�
		}
		//else ������Ǳ���Ͷ��,ֻ�������ʱ�䱻�����,��Ҳ���ܸ�����һ����ȷ��ʱ��
		//	   ��������Ͷ���û���ü�ɨ��ͱ������,Ҳ�����Ǹ��Ѿ�����������ľ�����,
		// 	   ֻ��ͻȻ����û����ס;
		// 	   �ǳ�������ϵͳ��Ψһ��Ͷ�������,�������������ʱ�������,�����ᱻִ��
	}

	return true;
}

//����: ���ϵͳ�Ȿ�ܼ��鱾���������.
//����:@bGrp		Ҫ���������ܼ���.
//����: �������ɹ����� true,���򷵻� false.
bool CMonthCtrl::ClrSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[1] = {0};

	WriteItemEx(BN0, (WORD)iGrp, 0x8283, bCmd);	//д��Ӧ�ִε�"�µ��Ͷ��Ͷ������"ID

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//��������.

	return true;
}

//����: ����ϵͳ�Ȿ������ִ����״̬.
//����:@iGrp	��ǰ���Ƶ��ܼ���.
//		@bTurnsStatus	�ִ�״̬
//����: ������óɹ����� true,���򷵻� false.
bool CMonthCtrl::SetSysCtrlTurnsStatus(int iGrp, BYTE bTurnsStatus)
{
	BYTE bBuf[10];
	memset(bBuf, 0, sizeof(bBuf));

	BYTE *pbtr = bBuf;
	*pbtr++ = DT_STRUCT;
	*pbtr++ = 2;					//�ṹ��Ա����
	*pbtr++ = DT_OI;				//�ܼ������
	pbtr += OoWordToOi(0x2300+iGrp, pbtr);
	*pbtr++ = DT_BIT_STR;
	*pbtr++ = 8;
	*pbtr++ = bTurnsStatus;
	WriteItemEx(BN0, iGrp, 0x8281, bBuf);

	return true;
}

//����: ��ȡָ���ܼ��鱾���õ��޶�.
//����:@iGrp	Ҫ��ȡ���ܼ���.
//����: �ɹ��򷵻ر��µ������޶�,���򷵻�int64������������.
int64 CMonthCtrl::GetMonthLimit(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return -1;

	BYTE bBuf[18];
	ReadItemEx(BN0, (WORD)iGrp, 0x8108, bBuf);	//����Ӧ���ܼ�����Ӧ��PN��"�µ����ض�ֵ"ID
	if (IsAllAByte(&bBuf[6], INVALID_DATA, 8) || IsAllAByte(&bBuf[6], 0, 8))
		return -1;

	int iAlrFactor = bBuf[17];
	return (OoLong64ToInt64(&bBuf[6]) * (100+iAlrFactor) / 100);	//���и�������.
}


//����: ��ȡָ���ܼ��鱾���õ籨��ϵ��.
//����:@iGrp	Ҫ��ȡ���ܼ���.
//����: �ɹ��򷵻ر��µ���������ϵ��,���򷵻�int64������������.
int64 CMonthCtrl::GetMonthAlarmFactor(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return -1;

	BYTE bBuf[18];
	if(ReadItemEx(BN0, (WORD)iGrp, 0x8108, bBuf)>0)	//����Ӧ���ܼ�����Ӧ��PN��"�µ����ض�ֵ"ID
	{
		if(bBuf[16] == 0)
			return 80;
		else
			return abs((int)bBuf[16]);
	}
	return 80;	//Ĭ��80%.
}
//����: ��ȡָ���ܼ��鱾���õ��޶������ϵ��.
//����:@iGrp	Ҫ��ȡ���ܼ���.
//����: ����ϵ��.
int CMonthCtrl::GetAlrFltQuotiety(void)
{
	BYTE bBuf[1];

	ReadItemEx(BN0, PN0, 0x014f, bBuf);			//��PN0��"�ն��µ������ض�ֵ����ϵ��"ID
	
	return Fmt4ToVal(bBuf, 1);
}

//����: �µ�ط�������������.
void CMonthCtrl::BeepAlrCtrl(void)
{
	char cTime[20];

	if (m_fAlrStauts)
	{
		BYTE bBuf[3] = {0xff, 0xff, 0xff};
		ReadItemEx(BN10, PN0, 0xa121, bBuf); //0xa121 3 �µ�ظ澯ʱ��,D0~D23�ֱ��Ӧ0��~23��

		if (GetBitStatus(bBuf, sizeof(bBuf), m_tmNow.nHour))
		{	
			if (m_dwAlrStartTime == 0)
			{
				m_dwAlrStartTime = m_dwNow;
				DTRACE(DB_LOADCTRL, ("CMonthCtrl::BeepAlrCtrl: beep alarm start at %s\n", TimeToStr(m_tmNow, cTime)));
			}
		}
		else
		{
			if (m_dwAlrStartTime != 0)
				DTRACE(DB_LOADCTRL, ("CMonthCtrl::BeepAlrCtrl: beep alarm stop at %s, grp=%d\n", TimeToStr(m_tmNow, cTime), m_iGrp));

			m_dwAlrStartTime = 0;
		}
	}
	else
	{
		if (m_dwAlrStartTime != 0)
			DTRACE(DB_LOADCTRL, ("CMonthCtrl::BeepAlrCtrl: beep alarm stop at %s, grp=%d\n", TimeToStr(m_tmNow, cTime), m_iGrp));

		m_dwAlrStartTime = 0;
	}
}

//����: ��ȡָ���ܼ��鱾�����������й��ܵ���.
//����:@iGrp	Ҫ��ȡ���ܼ���.
//����: �������������й�������.
int64 CMonthCtrl::GetMonthEng(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return -1;

	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;
	BYTE bBuf[50] = {0}; 
	int64 i64Value = 0;

	if (OoReadAttr(0x2300+(WORD)iGrp, 0x09, bBuf, &pbFmt, &wFmtLen) < 0)
	{
		DTRACE(DB_LOADCTRL, ("GetMonthEng: fail to get month energy!\n"));
		return -1;
	}

	i64Value = OoLong64ToInt64(&bBuf[3]);
	
	return i64Value;
}

//����: ͳ���µ������޲���(����ʱ�估���޵���).
void CMonthCtrl::StatOverLimitPara(void)
{
	char cTime[20];
	BYTE bBuf[2+4];

	if (m_tmOldTime.nMonth!=m_tmNow.nMonth || m_tmOldTime.nYear!=m_tmNow.nYear)
	{//���緢�������л�.
		//�������µ������޲������Ƶ������µ������޲���.
		for (int i=GRP_START_PN; i<(GRP_START_PN+GRP_NUM); i++)
		{
			if (ReadItemEx(BN18, (WORD)i, 0x02df, bBuf) <=0)
			{
				DTRACE(DB_LOADCTRL, ("CMonthCtrl::StatOverLimitPara: There is something wrong when call ReadItemEx() !\n"));
				return;
			}
			WriteItemEx(BN0, (WORD)i, 0x327f, bBuf, 0, NULL, m_dwNow);
			DTRACE(DB_LOADCTRL, ("CMonthCtrl::StatOverLimitPara: Save the %04d-%02d Group[%d] statistical parameter of MonthCtrl energy over limit!\n"\
								 "Over time = %ld minutes\n"\
								 "Over limit energy = %lld KWH\n"\
								 "at %s\n", m_tmOldTime.nYear, m_tmOldTime.nMonth, i,
								 ByteToWord(bBuf), Fmt3ToVal64(bBuf+2, 4), TimeToStr(m_tmNow, cTime)));
		}
		//��ʼ�����¿���ͳ������.
		memset(bBuf, 0, sizeof(bBuf));	//���֮ǰ�ļ�¼.
		for (int i=GRP_START_PN; i<(GRP_START_PN+GRP_NUM); i++)
			WriteItemEx(BN18, (WORD)i, 0x02df, bBuf);
	}

	int iOldGrp = m_OLStat.iGrp;

	int64 iEng = 0;
	DWORD dwTime = 0;//ͳ�Ƴ���ʱ��.
	int64 iTmpEnergy;//��ǰ�ܼ���������й����������ڸ��ϵ�ʱһ���Ӻ��ܻ�á�

	if (m_OLStat.fIfOverLimit)
	{//����֮ǰ��״̬�ǳ���״̬.
		DWORD dwClick = GetClick();

		if (!m_fIfOverLimit
			|| m_OLStat.iGrp!=m_iGrp
			|| (dwClick - m_OLStat.dwClick)>60)	//����ʱ,��,��,��ļ��,���Ч���Ҳ���Ӱ�쾫��.
		{//���糬��״̬����ӻ��ܼ��鷢���˱仯,�����ۼ�ͳ������.
			dwTime = dwClick - m_OLStat.dwClick;//ͳ�Ƴ���ʱ��.
			iTmpEnergy = GetCurEng(m_OLStat.iGrp);
			iEng = iTmpEnergy - m_OLStat.iEng;	//ͳ�Ƴ����µ���.

			if (m_fIfOverLimit)
			{
				m_OLStat.iGrp = m_iGrp;
				m_OLStat.dwClick = dwClick;
				m_OLStat.iEng = iTmpEnergy;
			}
			else
			{
				m_OLStat.fIfOverLimit = false;
				m_OLStat.iGrp = -1;
				m_OLStat.dwClick = 0;
				m_OLStat.iEng = 0;
			}
		}
	}
	else //(!m_OLStat.fIfOverLimit)
	{//����֮ǰ��״̬��δ����״̬.
		if (m_fIfOverLimit)
		{//���ڵ�״̬�ǳ���״̬,������һ��ͳ��.
			iTmpEnergy = GetCurEng(m_iGrp);
			if (iTmpEnergy >= 0)
			{
				m_OLStat.fIfOverLimit = true;
				m_OLStat.iGrp = m_iGrp;
				m_OLStat.dwClick = GetClick();
				m_OLStat.iEng = iTmpEnergy;
			}
			else
				m_OLStat.fIfOverLimit = false;
		}
	}

	if (dwTime!=0 || iEng!=0)
	{
		if (ReadItemEx(BN18, (WORD)iOldGrp, 0x02df, bBuf) <=0)
		{
			DTRACE(DB_LOADCTRL, ("CMonthCtrl::StatOverLimitPara: There is something wrong when call ReadItemEx() !\n"));
			return;
		}

		WORD w;

		w = ByteToWord(bBuf) + (WORD)(dwTime/60);	//����ʱ��.
		WordToByte(w, bBuf);
		iEng += Fmt3ToVal64(bBuf+2, 4);				//���ӳ��޵���.
		Val64ToFmt3(iEng, bBuf+2, 4);
		WriteItemEx(BN18, (WORD)iOldGrp, 0x02df, bBuf);
#if 1
		DTRACE(DB_LOADCTRL, ("CMonthCtrl::StatOverLimitPara: Refresh Group[%d] statistical parameter of MonthCtrl energy over limit!\n"\
							 "Over time = %ld minutes\n"\
							 "Over limit energy = %lld KWH\n"\
							 "at %s\n", iOldGrp, w, iEng, TimeToStr(m_tmNow, cTime)));
#endif
	}
}
