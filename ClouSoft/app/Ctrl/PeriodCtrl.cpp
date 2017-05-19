/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�PeriodCtrl.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��CPeriodCtrl����
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
#include "PeriodCtrl.h"
#include "DbOIAPI.h"

//========================================== CPeriodCtrl ==============================================
//����: 'ʱ�ο�'����.
//����: �����򷵻� true,���򷵻� false.
bool CPeriodCtrl::DoCtrl(void)
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
	if (m_tmNow.nDay!=m_tmOldTime.nDay || m_tmNow.nMonth!=m_tmOldTime.nMonth || m_tmNow.nYear!=m_tmOldTime.nYear)
	{//���緢�����л�.
		RstCtrl();
	}

	int i1;

	if ((i1=GetTimePeriod(m_tmNow)) < 0)		//��ȡ��ǰ��ʱ�κ�.
	{//ʱ�κ�С��0ʱ��ʾ��ǰʱ��������ʱ��û�й���Ͷ��.
		RstCtrl();
		SetSysCurPwrLimit(m_iGrp, m_iCurPwrLimit);
		return true;
	}

	if (m_iCurPeriodIdx != i1)
	{//ʱ�η������л�,������բ��λ.
		RstCtrl();
		m_iCurPeriodIdx = i1;		//���µ�ǰʱ�κ�.
	}

	int64 iCurPwrLimit;

	if ((m_CtrlCmd.bFlgs&(0x01<<i1))==0
		|| !GetPeriodLimit(m_iGrp, m_CtrlCmd.bScheme, i1, iCurPwrLimit))
	{//���統ǰʱ�β����ù��ػ�ָ���ܼ���ָ��������ָ��ʱ��û�й�������.
		RstCtrl();
		return true;
	}

	BYTE b = GetSysCtrlTurnsCfg(m_iGrp);

	if ((m_bTurnsStatus&~b) != 0)
	{//����ܿ��ִη�����բ��,Ӧ����Щ�ִν��и�λ(��λ�����բ״̬).
		m_bTurnsStatus &= b;
	}

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
	int i = GetIdxOfMostRight1(b & ~GetTurnsStatus());	//��ȡ��Ӧ�ܼ��鵱ǰ����բ���ִκ�.
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
//		m_dwAlrTime = 0;	//�������.		
		m_dwGuaranteeAlrTime = 0;
		if (i < 0 ) 
		{
			m_bWarnStatus = 4;
		}
		else
			m_bWarnStatus = i;
		if (m_bTurnsStatus != 0)
		{
			if(m_CtrlType != CTL_PWR_PERIOD_ALLCLOSE)
			{
				m_dwAlrTime = 0;
				m_CtrlType = CTL_PWR_PERIOD_ALLCLOSE;
			}
			else
			{
				if (m_dwAlrTime == 0)
					m_dwAlrTime = m_dwNow;
				RestoreTurnStatus();
				iTurn= GetIdxOfMostLeft1(m_bTurnsStatus);	//��ȡ��Ӧ�ܼ��鵱ǰ����բ���ִκ�.
				m_bWarnStatus = iTurn;
				dwPersistTime = GetPwrAlrPersistTime(iTurn);	//��ȡ��Ӧ�ִεĹ��ر�������ʱ��.
				//SaveDisp(dwPersistTime,m_dwAlrTime, iCurPwr);				
			}
		
		}
		else
		{
			m_dwAlrTime = 0;	//�������.
			m_CtrlType = CTL_PWR_PERIOD;
		}
	}
	else //(iCurPwr > m_iCurPwrLimit)
	{//���統ǰ���� > ��ǰ��������,��ʼ�������ʿ�������.
		char cTime[20];

		m_fIfOverLimit = true;
		if (m_CtrlType != CTL_PWR_PERIOD)
		{
			m_CtrlType = CTL_PWR_PERIOD;	
			m_dwAlrTime = 0;
			return true;
		}
		if (m_dwAlrTime > m_dwNow)	//���δ����ʱ����,ʱ����ǰ����ȥ��
			m_dwAlrTime = 0;

		if (m_dwAlrTime==0)
		{
			m_dwAlrTime = m_dwNow;
			DTRACE(DB_LOADCTRL, ("CPeriodCtrl::DoCtrl: Turn[%d] of Group[%d] PeriodCtrl Alarm start ...\n"\
								 "Current power is %lld, Period Limit is %lld\n"\
								 "Persistent time of alarm is %ld seconds\n"\
								 "at %s\n", i+TURN_START_PN, m_iGrp, iCurPwr, m_iCurPwrLimit, dwPersistTime, TimeToStr(m_tmNow, cTime)));
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
			Val64ToFmt2(iCurPwr, bRecBuf+5, 2);			//������բʱ����.
			Val64ToFmt2(m_iCurPwrLimit, bRecBuf+7, 2);	//������բʱ���ʶ�ֵ.
			WriteItemEx(BN0, (WORD)(i+TURN_START_PN), 0x0a04, bRecBuf);
			TrigerSaveBank(BN0, SECT_CTRL, 0);*/

			DTRACE(DB_LOADCTRL, ("CPeriodCtrl::DoCtrl: Turn[%d] of Group[%d] PeriodCtrl open break!\n"\
								 "at %s\n", i+TURN_START_PN, m_iGrp, TimeToStr(m_tmNow, cTime)));
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

	return true;
}

//����: ��λ�ڴ��б������״̬��.
void CPeriodCtrl::RstCtrl(void)
{
	m_bTurnsStatus	= 0x00;				//���ִ�״̬ȫ����Ϊ��բ.
	m_dwAlrTime		= 0;				//������ʱ��(��ʼ����ʱ��)����.
	m_dwGuaranteeAlrTime = 0;
	m_iCurPwrLimit	= 0;				//����ǰ���ض�ֵ��Ϊ0.

	m_iCurPeriodIdx	= -1;				//��ǰʱ����Ϊ -1.
	m_fIfOverLimit  = false;
}

//����: ��ȡĳ�ܼ���ı����������,��������ŵ� m_NewCmd ��.(ע��: �Բ�ͬ����,m_NewCmd�Ľṹ�ǲ�ͬ��)
//����:@iGrp	Ҫ��ȡ������ܼ���.
//����: �����ȡ�ɹ���Ϊ��Ч���� true,���򷵻� false.
bool CPeriodCtrl::GetSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[8];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8233, bCmd, &m_NewCmd.dwTime) <=0)	//��ȡ��Ӧ�ܼ����"ʱ�ο�����".
	{
		DTRACE(DB_LOADCTRL, ("CPeriodCtrl::GetSysCmd: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	m_NewCmd.bAct	 = bCmd[0];
	m_NewCmd.bFlgs	 = bCmd[5];
	m_NewCmd.bScheme = bCmd[7];

	if (m_NewCmd.bAct!=1 && m_NewCmd.bAct!=2)
		return false;

	if (m_NewCmd.dwTime == 0) //�ն�ʱ����ǰ����,ϵͳ���е�����ʱ�䱻�����
	{
		if (CurCmdTime()!=0 && iGrp==m_iGrp) //�ɵ������Ѿ�������,ֻ��ϵͳ���е�����ʱ�䱻�����
		{
			m_NewCmd.dwTime = GetCurTime(); //����Ϊһ������һ���ʱ��,�����Ժ�ͬ����ʱ��ıȽ�
			WriteItemEx(BN0, (WORD)iGrp, 0x8233, bCmd, m_NewCmd.dwTime);	//����Ӧ�ܼ����"ʱ�ο�����"д�����ݿ�
		}
		//else ������Ǳ���Ͷ��,ֻ�������ʱ�䱻�����,��Ҳ���ܸ�����һ����ȷ��ʱ��
		//	   ��������Ͷ���û���ü�ɨ��ͱ������,Ҳ�����Ǹ��Ѿ�����������ľ�����,
		// 	   ֻ��ͻȻ����û����ס;
		// 	   �ǳ�������ϵͳ��Ψһ��Ͷ�������,�������������ʱ�������,�����ᱻִ��
	}

	return true;
}

//����: ���ϵͳ��ָ���ܼ���'ʱ�ο�'����.
//����:@iGrp		Ҫ���������ܼ���.
//����: �������ɹ����� true,���򷵻� false.
bool CPeriodCtrl::ClrSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[8] = {0};

	WriteItemEx(BN0, (WORD)iGrp, 0x8233, bCmd);		//�����Ӧ�ܼ����"ʱ�ο�����".

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//��������.

	return true;
}

//����: ��ȡָ��ʱ��������ʱ��.
//����:@bTime	Ҫ��ȡ���ܼ���.
//����: ����ʱ���й���Ͷ��,����ʱ�κ�,���򷵻� -1.
int CPeriodCtrl::GetTimePeriod(TTime Time)
{
	BYTE b = (BYTE)Time.nHour*2 + (BYTE)(Time.nMinute>=30 ? 1 : 0);  //��һ���ڵ�ʱ��ת�����԰�Сʱ����λ
	BYTE b1 = b / 4;	//��ʱ��λ�ڵ��ֽ�ƫ��
	BYTE b2 = b % 4;	//��ʱ��λ���ֽ��ڵ�ƫ��

	BYTE bBuf[30];
	if (ReadItemEx(BN0, PN0, 0x8101, bBuf) <=0)		//����"�ն˹���ʱ��".
	{
		DTRACE(DB_LOADCTRL, ("CPeriodCtrl::GetTimePeriod: There is something wrong when call ReadItemEx() !\n"));
		return -1;
	}

	int iPeriodIdx = -1;
	bool bStatus = false;
	BYTE bCtrl = 0xff;	//ֻҪ���� 0,1,2,3 ����ֵ����.

	for (BYTE b0=0; b0<(2*24); b0++)	//ÿ��Сʱ�ؼ�����
	{
		BYTE b3 = (b0 / 4)*2 + 3;	//��ǰ���ֽ�ƫ��
		BYTE b4 = b0 % 4;	//��ǰ��λ���ֽ��ڵ�ƫ��
		BYTE bCtrl1 = (bBuf[b3]>>(b4*2)) & 0x03;
		if (bCtrl != bCtrl1)
		{
			if (bCtrl1==1 || bCtrl1==2)
			{
				iPeriodIdx++;
				bStatus = true;
			}
			else
				bStatus = false;
		}
		bCtrl = bCtrl1;
		if (b1==b3 && b2==b4)	//���˵�ǰ��ʱ��
			break;
	}

	if (bStatus)	//��ǰ���ڿ���״̬�Ĳŷ���ʱ��
		return iPeriodIdx;
	else			//��ǰ�����ڿ���״̬
		return -1;
}

//����: ��ȡָ���ܼ��鵱ǰʱ�εĹ�������.
//����:@iGrp			Ҫ��ȡ���ܼ���.
//	   @iScheme			������.
//	   @iPeriodIdx		ʱ�κ�.
//	   @riPwrLimit		���õĲ���,�������������ݺ�,ͨ���������������޴���.
//����: �����Ӧ���ܼ���,��Ӧ�ķ�����,��Ӧ��ʱ���й���Ͷ���򷵻� true, ���򷵻� false.
bool CPeriodCtrl::GetPeriodLimit(int iGrp, int iScheme, int iPeriodIdx, int64& riPwrLimit)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	WORD wLen;
	BYTE bType, bBuf[256];		//��Ȼ�ڹ��������ֻ��3�鷽��,��������չ�����8��.

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8103, bBuf) <=0)	//�䳤//��ָ���ܼ������"ʱ�ι��ض�ֵ".
	{
		DTRACE(DB_LOADCTRL, ("CPeriodCtrl::GetPeriodLimit: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	if (iScheme>2 || iPeriodIdx>7)	//�ȼ�鷽���ź�ʱ�κŵķ�Χ.
		return false;

	//2008-04-10 ��Ϊ�ò����Ǳ䳤��,���һ��ȷ��ĳ��������������ʼ��ַ.
	int i;

	if ((i=GetIdxOfAll1InPst(bBuf[7]&0x07, iScheme)) < 0)	//ֻ��0,1,2��λ.
		return false;	//û�ж�Ӧ�ķ�����ֵ,���� false.

	BYTE* pb = &bBuf[8];

	const ToaMap *p = GetOIMap(0x81030300);
	if (p ==NULL)
		return false;

	if ((pb=OoGetField(bBuf, p->pFmt, p->wFmtLen, 1+i, &wLen, &bType)) == NULL)//ȡ�ö�Ӧ�����Ĺ��ʶ�ֵ
	{
		return false;
	}

	//if ((i=GetIdxOfAll1InPst(*pb, iPeriodIdx)) < 0)
	//	return false;	//��Ӧ��ʱ��û�й��ض�ֵ,���� false.

	riPwrLimit = OoLong64ToInt64(pb+6+iPeriodIdx*9);				//��ȡ��Ӧ�Ĺ��ض�ֵ.
	if ((pb=OoGetField(bBuf, p->pFmt, p->wFmtLen, 5, &wLen, &bType)) == NULL)//ȡ��'ʱ�ο�'��ֵ����ϵ��.
	{
		DTRACE(DB_LOADCTRL, ("CPeriodCtrl::GetPeriodLimit: There is something wrong when call OoGetField() !\n"));
		return false;
	}

	riPwrLimit = riPwrLimit * (100+(int8)pb[1]) / 100;

	int64 iSafeLimit = GetPwrSafeLimit();	//��ȡ���ر�����ֵ
	//ȡ'ʱ�ο�'��ǰʱ�ι������޺�'���ر�����ֵ'�еĴ���Ϊ'��ǰ��������'.
	if (riPwrLimit < iSafeLimit)
		riPwrLimit = iSafeLimit;

	return true;
}

//����: �����ܼ���ϵͳ�Ȿ����Ʊ�־��Ϊָ��״̬.
//����:@iGrp			Ҫ�趨���ܼ���.
//	   @fStatus			״̬.
//����: ������óɹ��򷵻� true, ���򷵻� false.
bool CPeriodCtrl::SetSysCtrlFlg(int iGrp, bool fStatus)
{
	if (fStatus)
	{
		BYTE bBuf[1+1+6*8];

		if (ReadItemEx(BN0, PN0, 0x104f, bBuf) <=0)	//��"�ն˿�������״̬"ID
		{
			DTRACE(DB_LOADCTRL, ("CPeriodCtrl::SetSysCtrlFlg: There is something wrong when call ReadItemEx() !\n"));
			return false;
		}
		bBuf[1+1+6*(iGrp-GRP_START_PN)+0] = m_CtrlCmd.bScheme;	//д'���ض�ֵ������'.
		bBuf[1+1+6*(iGrp-GRP_START_PN)+1] = m_CtrlCmd.bFlgs;	//д'����ʱ����Ч��־λ'.

		WriteItemEx(BN0, PN0, 0x104f, bBuf);					//д"�ն˿�������״̬"ID

		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x01, fStatus, PWR_CTL);//ʱ�ο�ʹ��0λ.
	}
	else
		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x01, fStatus, PWR_CTL);//ʱ�ο�ʹ��0λ.
}


