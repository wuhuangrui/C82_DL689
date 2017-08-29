/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ShutoutCtrl.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��CShutoutCtrl����
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
#include "ShutoutCtrl.h"

//========================================== CShutoutCtrl ==============================================
//����: ���п���.
//����: �����򷵻� true,���򷵻� false.
bool CShutoutCtrl::DoCtrl(void)
{
	DoCmdScan();		//ɨ��ϵͳ���е�����.

	if (!IsValid())
	{
		RstCtrl();
		m_dwPwrStartClick = 0; //���¹��ʿ�ʼ�����ʱ��,0��ʾ֮ǰûͶ��,��ûͶ��תΪͶ��,Ҫ�ȴ����ػ���ʱ�����ȡ����
		return true;
	}
/*
	if (IsGuarantee())	//����Ƿ��ڱ���״̬.
	{
		RstCtrl();					//��λ�ڴ��б�����Ƶ��������״̬.
		return true;
	}
*/
	TShutoutCtrlPara CtrlPara;

	if (!GetShutoutCtrlPara(m_iGrp, CtrlPara))
		return false;	//ȡ'Ӫҵ��ͣ��'�������ִ���.

	char cTime[20];

	if (m_dwNow < CtrlPara.dwStartTime)
	{//���绹û��'Ӫҵ��ͣ��'����ʱ��.
		RstCtrl();
	}
	else if (m_dwNow < CtrlPara.dwEndTime)
	{//�����ѹ�'Ӫҵ��ͣ��'����ʱ��,����û������ʱ��.
		m_fInCtrl = true;

		BYTE b = GetSysCtrlTurnsCfg(m_iGrp);

		if ((m_bTurnsStatus&~b) != 0)
		{//����ܿ��ִη�����բ��,Ӧ����Щ�ִν��и�λ(��λ�����բ״̬).
			m_bTurnsStatus &= b;
		}

		int64 iCurPwrLimit = GetPwrSafeLimit();	//��ȡ���ر�����ֵ.
		//ȡ'Ӫҵ��ͣ�ع��ʶ�ֵ'��'���ر�����ֵ'�еĴ���Ϊ'��ǰ��������'.
		if (iCurPwrLimit < CtrlPara.iPwrLimit)
			iCurPwrLimit = CtrlPara.iPwrLimit;

		if (m_iCurPwrLimit != iCurPwrLimit)
		{
			SetSysCurPwrLimit(m_iGrp, iCurPwrLimit);
			m_iCurPwrLimit = iCurPwrLimit;
		}

		if (m_dwPwrStartClick == 0) //0��ʾ֮ǰ����ûͶ��,��ûͶ��תΪͶ��,Ҫ�ȴ����ػ���ʱ�����ȡ����
		{
			m_dwPwrStartClick = GetClick(); //���¹��ʿ�ʼ�����ʱ��
			return true;
		}
		else if (GetClick()-m_dwPwrStartClick < GetPwrSlideInterv(m_iGrp))
		{
			return true;	//�ȹ��ػ���ʱ�����ȡ����
		}

		int64 iCurPwr = GetCurPwr(m_iGrp);	// ��ȡ��ǰ����
		if (iCurPwr < 0)
			return false;
		int i = GetIdxOfMostRight1(b & ~GetTurnsStatus());		//��ȡ��Ӧ�ܼ��鵱ǰ����բ���ִκ�.

		if (i<0 && iCurPwr>m_iCurPwrLimit)									//����Ƿ��п���բ.
		{
			m_dwAlrTime = 0;						//��բ������,����û������,��ֹ����.
			m_dwGuaranteeAlrTime = 0;
			return true;
		}
		m_bWarnStatus = i+1;
		int iTurn;
		DWORD dwPersistTime = GetPwrAlrPersistTime(i+TURN_START_PN);	//��ȡ��Ӧ�ִεĹ��ر�������ʱ��.
		
		if (iCurPwr <= m_iCurPwrLimit)
		{
			m_fIfOverLimit = false;
//			m_dwAlrTime = 0;	//�������.
			m_dwGuaranteeAlrTime = 0;
//			m_bTurnsStatus = 0;  //��բ
			if (i < 0) 
			{
				m_bWarnStatus = 4;
			}
			else
			{
				m_bWarnStatus = i;
			}
			if (m_bTurnsStatus != 0)
			{
				if (m_CtrlType != CTL_PWR_SHUTOUT_ALLCLOSE)
				{
					m_dwAlrTime = 0;
					m_CtrlType = CTL_PWR_SHUTOUT_ALLCLOSE;				
				}
				else
				{
					if (m_dwAlrTime == 0)
					m_dwAlrTime = m_dwNow;
					
					iTurn= GetIdxOfMostLeft1(m_bTurnsStatus);	//��ȡ��Ӧ�ܼ��鵱ǰ����բ���ִκ�.
					dwPersistTime = GetPwrAlrPersistTime(iTurn);	//��ȡ��Ӧ�ִεĹ��ر�������ʱ��.
					m_bWarnStatus = iTurn;
					//SaveDisp(dwPersistTime,m_dwAlrTime, iCurPwr);
					RestoreTurnStatus();			
				}
			}
			else
			{
				m_dwAlrTime = 0;
				m_CtrlType = CTL_PWR_SHUTOUT;
			}
		}
		else //(iCurPwr > m_iCurPwrLimit)
		{//���統ǰ���� > ��ǰ��������,��ʼ�������ʿ�������.
			m_fIfOverLimit = true;
			if (m_CtrlType != CTL_PWR_SHUTOUT)
			{
				m_CtrlType = CTL_PWR_SHUTOUT;	
				m_dwAlrTime = 0;
				return true;
			}

			if (m_dwAlrTime > m_dwNow)	//���δ����ʱ����,ʱ����ǰ����ȥ��
				m_dwAlrTime = 0;

			if (m_dwAlrTime==0)
			{
				m_dwAlrTime = m_dwNow;
				DTRACE(DB_LOADCTRL, ("CShutoutCtrl::DoCtrl: alarm start at %s, turn=%d, grp=%d, current-power=%lld, limit=%lld, alr-persistent-time=%ldS\n",
									 TimeToStr(m_tmNow, cTime), i+TURN_START_PN, m_iGrp, iCurPwr, m_iCurPwrLimit, dwPersistTime));
				//***��¼��ϵͳ����־��.
				//***���������ź�;
			}
			if (IsGuarantee())
			{
				if (m_dwGuaranteeAlrTime==0)
				{
					m_dwGuaranteeAlrTime = m_dwNow;
					m_dwAlrTime = m_dwNow;
				}
				m_bTurnsStatus = 0;
				if (m_dwNow > m_dwGuaranteeAlrTime+dwPersistTime)
				{
					m_dwAlrTime = 0;
				}
				else
				{
					DWORD dwCntDown;
					if (m_dwNow <= m_dwAlrTime+dwPersistTime)
						dwCntDown = m_dwAlrTime + dwPersistTime - m_dwNow;
					else
						dwCntDown = 0;
					//SaveDisp(dwPersistTime,m_dwAlrTime, iCurPwr);
				}
				return true;
			}
			if (m_dwNow > m_dwAlrTime+dwPersistTime)
			{//����'��ǰʱ��' > 'Ӫҵ��ͣ�ر�������ʱ��'+'Ӫҵ��ͣ�ر�������ʱ��'
				m_bTurnsStatus |= 0x01 << i;
				m_dwAlrTime = 0;							//������բ��,Ӧ�ð���բ�����ر�.
				m_wOpenTimes++;								//��բ��������1.

				//������բ��¼�м�����.
				/*BYTE bRecBuf[4+1+2+2];

				memcpy(bRecBuf, &m_dwNow, 4);				//������բʱ��.
				bRecBuf[4] = (BYTE)m_iGrp;					//�����ܼ���.
				Val64ToFmt2(iCurPwr, bRecBuf+5, 2);		//������բʱ����.
				Val64ToFmt2(m_iCurPwrLimit, bRecBuf+7, 2);	//������բʱ���ʶ�ֵ.
				WriteItemEx(BN0, (WORD)(i+TURN_START_PN), 0x0a02, bRecBuf);
				TrigerSaveBank(BN0, SECT_CTRL, 0);*/

				DTRACE(DB_LOADCTRL, ("CShutoutCtrl::DoCtrl: open break at %s, turn=%d, grp=%d\n", TimeToStr(m_tmNow, cTime), i+TURN_START_PN, m_iGrp));
				//***���������ź�;
			}

			//���ù�����ʾ������
			DWORD dwCntDown;
			if (m_dwNow <= m_dwAlrTime+dwPersistTime)
				dwCntDown = m_dwAlrTime + dwPersistTime - m_dwNow;
			else
				dwCntDown = 0;
			//SaveDisp(dwPersistTime,m_dwAlrTime, iCurPwr);
		}
	}
	else
	{//�����ѹ�'Ӫҵ��ͣ��'����ʱ��.
		if (m_iCurPwrLimit!=0 && m_iGrp>=0)
		{
			//SetSysCurPwrLimit(m_iGrp, INVALID_VAL64);
			m_iCurPwrLimit = 0;
			DTRACE(DB_LOADCTRL, ("CShutoutCtrl::DoCtrl: out of date, quit at %s, grp=%d\n", TimeToStr(m_tmNow, cTime), m_iGrp));
		}

		RstCtrl();					//��λ�ڴ��б�����Ƶ��������״̬.
		//***��¼��ϵͳ����־��.
		//***���������ź�;
	}

	return true;
}

//����: ��λ�ڴ��б������״̬��.
void CShutoutCtrl::RstCtrl(void)
{
	m_bTurnsStatus	= 0x00;			//���ִ�״̬ȫ����Ϊ��բ.
	m_dwAlrTime		= 0;			//������ʱ��(��ʼ����ʱ��)����.
	m_dwGuaranteeAlrTime = 0;
	m_iCurPwrLimit		= 0;		//����ǰ���ض�ֵ��Ϊ0.

	m_fIfOverLimit  = false;
	m_fInCtrl = false;				//���ڱ���״̬��ʱ��Ҳ�����RstCtrl()����λ����־
									//���ܲ�һ������ȫ��Ӧ��ʵ��״̬,������ʹִ�������ȼ�
									//���͵Ŀ���,Ҳû��ϵ,��Ϊ����Ҳ���ڱ����״̬,������ɨ��һ������
}

//����: ��ȡĳ�ܼ���ı����������,��������ŵ� m_NewCmd ��.(ע��: �Բ�ͬ����,m_NewCmd�Ľṹ�ǲ�ͬ��)
//����:@iGrp	Ҫ��ȡ������ܼ���.
//����: �����ȡ�ɹ���Ϊ��Ч���� true,���򷵻� false.
bool CShutoutCtrl::GetSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[1];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8253, bCmd, &m_NewCmd.dwTime) <=0)	//��ȡ��Ӧ�ܼ����"Ӫҵ��ͣ������".
	{
		DTRACE(DB_LOADCTRL, ("CShutoutCtrl::GetSysCmd: There is something wrong when call ReadItemEx() !\n"));
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
			WriteItemEx(BN0, (WORD)iGrp, 0x8253, bCmd, m_NewCmd.dwTime);	//����Ӧ�ܼ����"Ӫҵ��ͣ������"д�����ݿ�
		}
		//else ������Ǳ���Ͷ��,ֻ�������ʱ�䱻�����,��Ҳ���ܸ�����һ����ȷ��ʱ��
		//	   ��������Ͷ���û���ü�ɨ��ͱ������,Ҳ�����Ǹ��Ѿ�����������ľ�����,
		// 	   ֻ��ͻȻ����û����ס;
		// 	   �ǳ�������ϵͳ��Ψһ��Ͷ�������,�������������ʱ�������,�����ᱻִ��
	}

	return true;
}

//����: ���ϵͳ��ָ���ܼ���'Ӫҵ��ͣ��'����.
//����:@iGrp		Ҫ���������ܼ���.
//����: �������ɹ����� true,���򷵻� false.
bool CShutoutCtrl::ClrSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[1] = {0};

	WriteItemEx(BN0, (WORD)iGrp, 0x8253, bCmd);		//�����Ӧ�ܼ����"Ӫҵ��ͣ������".

	TrigerSaveBank(BN0, SECT_CTRL, 0); //��������.

	return true;
}

//����: ����ϵͳ�Ȿ������ִ����״̬.
//����:@iGrp	��ǰ���Ƶ��ܼ���.
//		@bTurnsStatus	�ִ�״̬
//����: ������óɹ����� true,���򷵻� false.
bool CShutoutCtrl::SetSysCtrlTurnsStatus(int iGrp, BYTE bTurnsStatus)
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
	WriteItemEx(BN0, iGrp, 0x8251, bBuf);

	return true;
}

//����: ��ȡָ���ܼ���'Ӫҵ��ͣ��'����.
//����:@iGrp	Ҫ��ȡ���ܼ���.
//	   @rPara	���õĲ����ṹ,�������������ݺ�,ͨ�����ṹ����������.
//����: �ɹ����� true,���򷵻� false.
bool CShutoutCtrl::GetShutoutCtrlPara(int iGrp, TShutoutCtrlPara& rPara)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bBuf[30];
	TTime tm;

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8105, bBuf) <=0)	//F44,��ȡ��Ӧ�ܼ����"Ӫҵ��ͣ�ز���"
	{
		DTRACE(DB_LOADCTRL, ("CShutoutCtrl::GetShutoutCtrlPara: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	OoDateTimeSToTime(bBuf+6, &tm);
	tm.nSecond = 0;
	tm.nMinute = 0;
	tm.nHour = 0;
	rPara.dwStartTime	= TimeToSeconds(tm);

	OoDateTimeSToTime(bBuf+14, &tm);
	tm.nSecond = 0;
	tm.nMinute = 0;
	tm.nHour = 0;
	rPara.dwEndTime		= TimeToSeconds(tm);
	rPara.iPwrLimit		= OoLong64ToInt64(bBuf+22);

	if (rPara.dwStartTime > rPara.dwEndTime)
	{
		DTRACE(DB_LOADCTRL, ("CShutoutCtrl::GetShutoutCtrlPara: err! start time %02x-%02x-%02x > end time %02x-%02x-%02x\n",
							 bBuf[2], bBuf[1], bBuf[0], 
							 bBuf[5], bBuf[4], bBuf[3]));
		return false;
	}

	if (rPara.iPwrLimit==0 || IsAllAByte(bBuf+6, INVALID_DATA, 2)) //��ֵû��
	{
		DTRACE(DB_LOADCTRL, ("CShutoutCtrl::GetShutoutCtrlPara: err! limit not set\n"));
		return false;
	}

	return true;
}
