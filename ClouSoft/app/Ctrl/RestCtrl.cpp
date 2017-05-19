/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�RestCtrl.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��CRestCtrl����
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
#include "RestCtrl.h"

//========================================== CRestCtrl ==============================================
//����: ���п���.
//����: �����򷵻� true,���򷵻� false.
bool CRestCtrl::DoCtrl(void)
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
	TRestCtrlPara CtrlPara;

	if (!GetRestCtrlPara(m_iGrp, CtrlPara))
	{
		DTRACE(DB_LOADCTRL, ("CRestCtrl::DoCtrl: Group[%d] RestCtrl get parameter error !\n", m_iGrp));
		return false;	//ȡ'���ݿ�'�������ִ���.
	}

	
	BYTE bWeek = DayOfWeek(m_tmNow); //DayOfWeek()�ķ��� 1 = Sunday, 2 = Monday, ..., 7 = Saturday

	int i1 = (bWeek==1) ? 7 : (bWeek-1);

	if ((CtrlPara.bDays&(0x01<<i1)) == 0) //D1~D7��ʾ����һ��������,D0λ����
	{//������첻���޵���.
		RstCtrl();
		return true;
	}
	
	if (m_tmNow.nDay!=m_tmOldTime.nDay || m_tmNow.nMonth!=m_tmOldTime.nMonth || m_tmNow.nYear!=m_tmOldTime.nYear)
	{//���緢�����л�.
		RstCtrl();
	}

	char cTime[20];
	//��2000.1.1 00:00:00 ������'���ݿ�'����ʱ������.
	DWORD dwTime = m_dwNow - (((DWORD)m_tmNow.nHour*60+(DWORD)m_tmNow.nMinute)*60+(DWORD)m_tmNow.nSecond) + CtrlPara.dwStartTime;

	if (m_dwNow < dwTime)
	{//���绹û��'���ݿ�'����ʱ��.
		RstCtrl();
	}
	else if (m_dwNow < dwTime+CtrlPara.dwPersistTime)
	{//�����ѹ�'���ݿ�'����ʱ��,����û������ʱ��.
		m_fInCtrl = true;

		BYTE b = GetSysCtrlTurnsCfg(m_iGrp);

		if ((m_bTurnsStatus&~b) != 0)
		{//����ܿ��ִη�����բ��,Ӧ����Щ�ִν��и�λ(��λ�����բ״̬).
			m_bTurnsStatus &= b;
		}

		int64 iCurPwrLimit = GetPwrSafeLimit();	//��ȡ���ر�����ֵ
		//ȡ'���ݿع��ʶ�ֵ'��'���ر�����ֵ'�еĴ���Ϊ'��ǰ��������'.
		if (CtrlPara.iPwrLimit > iCurPwrLimit)
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
			if (i < 0 ) 
			{
				m_bWarnStatus = 4;
			}
			else
				m_bWarnStatus = i;
			if (m_bTurnsStatus != 0)
			{
				if (m_CtrlType != CTL_PWR_REST_ALLCLOSE)
				{
					m_dwAlrTime = 0;
					m_CtrlType = CTL_PWR_REST_ALLCLOSE;			
				}
				else
				{
					if (m_dwAlrTime == 0)
					m_dwAlrTime = m_dwNow;
					iTurn= GetIdxOfMostLeft1(m_bTurnsStatus);	//��ȡ��Ӧ�ܼ��鵱ǰ����բ���ִκ�.
					dwPersistTime = GetPwrAlrPersistTime(iTurn);	//��ȡ��Ӧ�ִεĹ��ر�������ʱ��.
					m_bWarnStatus = iTurn;
					RestoreTurnStatus();
					//SaveDisp(dwPersistTime,m_dwAlrTime, iCurPwr);				
				}

			}
			else
			{
				m_dwAlrTime = 0;	//�������.
				m_CtrlType = CTL_PWR_REST;
			}
		}
		else //(iCurPwr > m_iCurPwrLimit)
		{//���統ǰ���� > ��ǰ��������,��ʼ�������ʿ�������.
			m_fIfOverLimit = true;
			if (m_CtrlType != CTL_PWR_REST)
			{
				m_CtrlType = CTL_PWR_REST;	
				m_dwAlrTime = 0;
				return true;
			}			
			if (m_dwAlrTime > m_dwNow)	//���δ����ʱ����,ʱ����ǰ����ȥ��
				m_dwAlrTime = 0;

			if (m_dwAlrTime==0)
			{
				m_dwAlrTime = m_dwNow;
				DTRACE(DB_LOADCTRL, ("CRestCtrl::DoCtrl: alarm start at %s, turn=%d, grp=%d, current-power=%lld, limit=%lld, alr-persistent-time=%ldS\n",
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
			{//����'��ǰʱ��' > '���ݿر�������ʱ��'+'���ݿر�������ʱ��'
				m_bTurnsStatus |= 0x01 << i;
				m_dwAlrTime = 0;							//������բ��,Ӧ�ð���բ�����ر�.
				m_wOpenTimes++;								//��բ��������1.

				//������բ��¼�м�����.
				/*BYTE bRecBuf[4+1+2+2];

				memcpy(bRecBuf, &m_dwNow, 4);				//������բʱ��.
				bRecBuf[4] = (BYTE)m_iGrp;					//�����ܼ���.
				Val64ToFmt2(iCurPwr, bRecBuf+5, 2);		//������բʱ����.
				Val64ToFmt2(m_iCurPwrLimit, bRecBuf+7, 2);	//������բʱ���ʶ�ֵ.
				WriteItemEx(BN0, (WORD)(i+TURN_START_PN), 0x0a03, bRecBuf);
				TrigerSaveBank(BN0, SECT_CTRL, 0);*/

				DTRACE(DB_LOADCTRL, ("CRestCtrl::DoCtrl: open break at %s, turn=%d, grp=%d\n",
									 TimeToStr(m_tmNow, cTime), i+TURN_START_PN, m_iGrp));
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
	{//�����ѹ�'���ݿ�'����ʱ��.
		RstCtrl();
		//DTRACE(DB_LOADCTRL, ("CRestCtrl::DoCtrl: Today, the RestCtrl time of Group[%d] is over!\n"\
		//					 "at %s\n", m_iGrp, TimeToStr(m_tmNow, cTime)));
		//***��¼��ϵͳ����־��.
		//***���������ź�;
	}

	return true;
}

//����: ��λ'���ݿ�'����״̬.
void CRestCtrl::RstCtrl(void)
{
	m_bTurnsStatus	= 0x00;			//���ִ�״̬ȫ����Ϊ��բ.
	m_dwAlrTime		= 0;			//������ʱ��(��ʼ����ʱ��)����.
	m_dwGuaranteeAlrTime = 0;
	m_iCurPwrLimit	= 0;			//����ǰ���ض�ֵ��Ϊ0.

	m_fIfOverLimit  = false;

	m_fInCtrl = false;				//���ڱ���״̬��ʱ��Ҳ�����RstCtrl()����λ����־
									//���ܲ�һ������ȫ��Ӧ��ʵ��״̬,������ʹִ�������ȼ�
									//���͵Ŀ���,Ҳû��ϵ,��Ϊ����Ҳ���ڱ����״̬,������ɨ��һ������

}

//����: ��ȡĳ�ܼ���ı����������,��������ŵ� m_NewCmd ��.(ע��: �Բ�ͬ����,m_NewCmd�Ľṹ�ǲ�ͬ��)
//����:@iGrp	Ҫ��ȡ������ܼ���.
//����: �����ȡ�ɹ���Ϊ��Ч���� true,���򷵻� false.
bool CRestCtrl::GetSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[1];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8243, bCmd, &m_NewCmd.dwTime) <=0)	//��ȡ��Ӧ�ܼ����"���ݿ�����".
	{
		DTRACE(DB_LOADCTRL, ("CRestCtrl::GetSysCmd: There is something wrong when call ReadItemEx() !\n"));
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
			WriteItemEx(BN0, (WORD)iGrp, 0x8243, bCmd, m_NewCmd.dwTime);	//����Ӧ�ܼ����"ʱ�ο�����"д�����ݿ�
		}
		//else ������Ǳ���Ͷ��,ֻ�������ʱ�䱻�����,��Ҳ���ܸ�����һ����ȷ��ʱ��
		//	   ��������Ͷ���û���ü�ɨ��ͱ������,Ҳ�����Ǹ��Ѿ�����������ľ�����,
		// 	   ֻ��ͻȻ����û����ס;
		// 	   �ǳ�������ϵͳ��Ψһ��Ͷ�������,�������������ʱ�������,�����ᱻִ��
	}

	return true;
}

//����: ���ϵͳ��ָ���ܼ���'���ݿ�'����.
//����:@iGrp		Ҫ���������ܼ���.
//����: �������ɹ����� true,���򷵻� false.
bool CRestCtrl::ClrSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[1] = {0};

	WriteItemEx(BN0, (WORD)iGrp, 0x8243, bCmd);		//�����Ӧ�ܼ����"���ݿ�����".

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//��������.

	return true;
}

//����: ��ȡָ���ܼ���'���ݿ�'����.
//����:iGrp		Ҫ��ȡ���ܼ���.
//	   @rPara	���õĲ����ṹ,�������������ݺ�,ͨ�����ṹ����������.
//����: �ɹ����� true,���򷵻� false.
bool  CRestCtrl::GetRestCtrlPara(int iGrp, TRestCtrlPara& rPara)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bBuf[30];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8104, bBuf) <=0)	//��ȡ��Ӧ�ܼ���'���ݿ�'����.
	{
		DTRACE(DB_LOADCTRL, ("CRestCtrl::GetRestCtrlPara: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	rPara.iPwrLimit = OoLong64ToInt64(bBuf+6);
	rPara.dwStartTime = (DWORD)(bBuf[21] + bBuf[20]*60) * 60;
	rPara.dwPersistTime = (DWORD)OoLongToInt16(&bBuf[23]);
	rPara.bDays = bBuf[27];

	return true;
}
