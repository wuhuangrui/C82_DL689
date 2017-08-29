/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�TmpCtrl.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��CTmpCtrl����
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
#include "TmpCtrl.h"
#include "DpGrp.h"

//========================================== CTmpCtrl ==============================================
//����: ���п���.
//����: �����򷵻� true,���򷵻� false.
bool CTmpCtrl::DoCtrl(void)
{
	DoCmdScan();		//ɨ��ϵͳ���е�����.

	if (!IsValid())
	{
		RstCtrl();
		return true;
	}

	char cTime[20], cStr[20];
	TTime tmCmd;
	int iTurn;
	BYTE bTmpBuf[9] = {0};
	SecondsToTime(m_CtrlCmd.dwTime, &tmCmd);
/*
	if (IsGuarantee())
	{//��������Ͷ�룬����棬״̬��λ��
		RstCtrl();					//��λ�ڴ��б�����Ƶ��������״̬.

		DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: quit for guarantee at %s, grp=%d\n", 
							 TimeToStr(m_tmNow, cTime), m_iGrp));
		return true;
	}
*/
#ifdef PRO_698
	if ((m_tmNow.nDay!=tmCmd.nDay || m_tmNow.nMonth!=tmCmd.nMonth || m_tmNow.nYear!=tmCmd.nYear) ||
		(m_dwNow-m_CtrlCmd.dwTime>((DWORD )((DWORD)m_CtrlCmd.bCtrlTime*30*60)) && m_CtrlCmd.dwTime!=0) )	
#else
	if (m_tmNow.nDay!=tmCmd.nDay || m_tmNow.nMonth!=tmCmd.nMonth || m_tmNow.nYear!=tmCmd.nYear)	
#endif
	{//��ʱ�¸���ֻ�ǵ�����Ч�����պ���������������״̬��λ��
		RstSysCtrlStatus(m_iGrp);	//��λϵͳ�⵱ǰ�ܼ��鱾�����״̬(���ܰ����ִ�״̬,Ͷ���־�ȵ�).
		ClrSysCmd(m_iGrp);			//���ϵͳ�Ȿ�ܼ��鱾���������.
		RstCtrl();					//��λ�ڴ��б�����Ƶ��������״̬.
		SetValidStatus(false);		//�ָ��˳�״̬. 
		ClrCmd();					//����ڴ��б�����ƵĿ�������.
#ifdef PRO_698
		DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: quit for day chang at %s,tmCmd:%s grp=%d\n",
							 TimeToStr(m_tmNow, cTime), TimeToStr(tmCmd, cStr), m_iGrp));
#else
		DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: quit for day chang at %s, grp=%d\n",
							 TimeToStr(m_tmNow, cTime), m_iGrp));
#endif
		m_iGrp = -1;				//����ǰ�ܼ�����Ϊ -1,��ʾ��ǰû���ܼ���Ͷ��.
		//***��¼��ϵͳ����־��.
		//***���������ź�;
		return true;
	}

	if (!m_fCalLimitFinish)
	{//��ʱ�¸��صĹ��ض�ֵ��û�м������,���ڻ���ʱ�����Է���Ϊ����Ե�ǰ
	 //���ʽ��в����ۼ�,�������õ���ƽ��ֵ��Ϊ��ʱ�¸��صĹ���ֵ����.
		DWORD dwClick = GetClick();

		if (m_dwCalLimitStartClick == 0)
		{
			m_dwCalLimitStartClick = dwClick;				//��ȡ��ǰ�� Click.
			m_dwCalLimitTmpClick = m_dwCalLimitStartClick;
			m_iTmpCtrlLimit = 0;
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: start calculate limit, grp=%d\n", m_iGrp));
		}

		if (dwClick >= m_dwCalLimitTmpClick)
		{
			int64 iPwr = GetValidCurPwr(m_iGrp);
			if (iPwr >= 0)
			{
				m_iTmpCtrlLimit += iPwr;
				m_dwCalLimitTmpClick = dwClick + CTL_TMPCTRL_SLIP_INTERVAL;
				m_wCalLimitTimes++;
			}
		}

		if (m_wCalLimitTimes > (WORD)m_CtrlCmd.bWndTime) //m_wCalLimitTimesʵ�ʼ�����Ƿ�����
		{
			m_iTmpCtrlLimit = m_iTmpCtrlLimit * (100+Fmt4ToVal64(&m_CtrlCmd.bQuotiety, 1)) / (100*(int64)m_wCalLimitTimes);
			//WriteItemVal64(BN0, (WORD)m_iGrp, 0x084f, &m_iTmpCtrlLimit);	//������õ����¸��ع��ʶ�ֵд��ϵͳ��,����ʾϵͳʹ��.
			m_fCalLimitFinish = true;
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: finish calculate limit=%lld of grp%d !\n", m_iTmpCtrlLimit, m_iGrp));
		}
		if (!m_fCalLimitFinish)
			return true;
	}

	BYTE b = GetSysCtrlTurnsCfg(m_iGrp);

	if ((m_bTurnsStatus&~b) != 0)
	{//����ܿ��ִη�����բ��,Ӧ����Щ�ִν��и�λ(��λ�����բ״̬).
		m_bTurnsStatus &= b;
	}

	int64 iCurPwrLimit = GetPwrSafeLimit();
	//ȡ��ǰ���ʶ�ֵ�뱣����ֵ�еĴ���.
	if (iCurPwrLimit < m_iTmpCtrlLimit)
		iCurPwrLimit = m_iTmpCtrlLimit;

	if (m_iCurPwrLimit != iCurPwrLimit)
	{
		SetSysCurPwrLimit(m_iGrp, iCurPwrLimit);
		m_iCurPwrLimit = iCurPwrLimit;
	}

	int64 iCurPwr = GetCurPwr(m_iGrp);	// ��ȡ��ǰ����
	if (iCurPwr < 0)
		return false;

	//CEN:������ʲ�һ�������ڹ����½����¼
	//���洦�������Ч��,�����½����޶�ֵ���µ�״��.
	if (m_dwOpenBreakTime!=0 && m_dwNow>=(m_dwOpenBreakTime+(DWORD)m_CtrlCmd.bDelayTime*60))
	{//�����բʱ�䲻Ϊ0,����Ϊ�Ѿ�����բ,��ǰ�����½�����բ�Ľ��,Ӧ�ڹ涨����ʱ���¼�¿غ���.
		//WriteItemVal64(BN0, (WORD)m_iGrp, 0x111f, &iCurPwr);
		memset(bTmpBuf, 0, sizeof(bTmpBuf));
		bTmpBuf[0] = DT_LONG64;
		OoInt64ToLong64(iCurPwr, bTmpBuf+1);
		WriteItemEx(BN0, (WORD)m_iGrp, 0x230b, bTmpBuf);//��¼�µ�ǰ�����¸��ؿغ��ܼ��й����ʶ���ֵ
		m_dwOpenBreakTime = 0;
		DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: in %d minutes after control of grp%d, rec current power=%lld at %s\n",
			m_CtrlCmd.bDelayTime, m_iGrp, iCurPwr, TimeToStr(m_tmNow, cTime)));
	}
	
	int i = GetIdxOfMostRight1(b & ~GetTurnsStatus());		//��ȡ��Ӧ�ܼ��鵱ǰ����բ���ִκ�.

	if (i<0 && iCurPwr>m_iCurPwrLimit)									//����Ƿ��п���բ.
	{
		m_dwAlrTime = 0;						//��բ������,����û������,��ֹ����.
		m_dwGuaranteeAlrTime = 0;
		return true;
	}
	m_bWarnStatus = i+1;
#ifdef PRO_698
	DWORD dwPersistTime = (DWORD )m_CtrlCmd.bAlrTime[i + TURN_START_PN - 1] *60;	//��ȡ��Ӧ�ִεĹ��ر�������ʱ��.
#else
	DWORD dwPersistTime = GetPwrAlrPersistTime(i+TURN_START_PN);	//��ȡ��Ӧ�ִεĹ��ر�������ʱ��.
#endif

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
			if (m_CtrlType != CTL_PWR_TMP_ALLCLOSE)
			{
				m_dwAlrTime = 0;
				m_CtrlType = CTL_PWR_TMP_ALLCLOSE;
			}
			else
			{
				if (m_dwAlrTime == 0)
				m_dwAlrTime = m_dwNow;
				int iTurn= GetIdxOfMostLeft1(m_bTurnsStatus)-1;	//��ȡ��Ӧ�ܼ��鵱ǰ����բ���ִκ�.
				m_bWarnStatus = iTurn+1;
				dwPersistTime = (DWORD )m_CtrlCmd.bAlrTime[iTurn] *60;	//��ȡ��Ӧ�ִεĹ��ر�������ʱ��.
				//SaveDisp(dwPersistTime,m_dwAlrTime, iCurPwr);
				RestoreTurnStatus();			
			}

		}
		else
		{
			m_dwAlrTime = 0;	//�������.
			m_CtrlType = CTL_PWR_TMP;
		}
	}
	else //(iCurPwr > m_iCurPwrLimit)
	{//���統ǰ���� > ��ǰ��������,��ʼ�������ʿ�������.
		m_fIfOverLimit = true;
		if (m_CtrlType != CTL_PWR_TMP)
		{
			m_CtrlType = CTL_PWR_TMP;
			m_dwAlrTime = 0;
			return true;
		}
		if (m_dwAlrTime > m_dwNow)	//���δ����ʱ����,ʱ����ǰ����ȥ��
			m_dwAlrTime = 0;

		if (m_dwAlrTime==0)
		{
			m_dwAlrTime = m_dwNow;
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: alarm start at %s, turn=%d grp=%d, current power is %lld, limit is %lld, persistent time is %ld seconds\n",
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
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: quit for guarantee at %s, grp=%d\n", 
								 TimeToStr(m_tmNow, cTime), m_iGrp));
			return true;
		}
		if (m_dwNow > m_dwAlrTime+dwPersistTime)
		{//����'��ǰʱ��' > '��ʱ�¸��ر�������ʱ��'+'��ʱ�¸��ر�������ʱ��'
			m_bTurnsStatus	|= 0x01 << i;
			m_dwAlrTime		 = 0;							//������բ��,Ӧ�ð���բ�����ر�.
			m_wOpenTimes++;									//��բ��������1.
			m_dwOpenBreakTime = m_dwNow;					//��¼����բʱ��.

			//������բ��¼�м�����.
			/*BYTE bRecBuf[4+1+2+2];

			memcpy(bRecBuf, &m_dwNow, 4);				//������բʱ��.
			bRecBuf[4] = (BYTE)m_iGrp;					//�����ܼ���.
			Val64ToFmt2(iCurPwr, bRecBuf+5, 2);			//������բʱ����.
			Val64ToFmt2(m_iCurPwrLimit, bRecBuf+7, 2);	//������բʱ���ʶ�ֵ.
			WriteItemEx(BN0, (WORD)(i+TURN_START_PN), 0x0a01, bRecBuf);
			TrigerSaveBank(BN0, SECT_CTRL, 0);*/

			DTRACE(DB_LOADCTRL, ("CTmpCtrl::DoCtrl: turn%d of grp%d open break at %s\n", 
								 i+TURN_START_PN, m_iGrp, TimeToStr(m_tmNow, cTime)));
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
void CTmpCtrl::RstCtrl(void)
{
	m_bTurnsStatus	= 0x00;				//���ִ�״̬ȫ����Ϊ��բ.
	m_dwAlrTime		= 0;				//������ʱ��(��ʼ����ʱ��)����.
	m_dwGuaranteeAlrTime = 0;
	m_iCurPwrLimit	= 0;				//����ǰ���ض�ֵ��Ϊ0.

	m_fCalLimitFinish		= false;	//��ǰ�ܼ���'��ʱ�¸���'����ֵ�������״̬.
	m_dwCalLimitStartClick	= 0;		//��ǰ�ܼ���'��ʱ�¸���'����ֵ������ʼʱ��.
	m_dwCalLimitTmpClick	= 0;		//��ǰ�ܼ���'��ʱ�¸���'����ֵ��������ʱʱ�����.
	m_wCalLimitTimes		= 0;		//��ǰ�ܼ���'��ʱ�¸���'����ֵ����ʱ,�����ۼӴ���.
	m_iTmpCtrlLimit			= 0;		//��ǰ�ܼ���'��ʱ�¸���'����ֵ(ȡ���ڵ�ǰ�¸�ֵ�ͱ���ֵ).

	m_fIfOverLimit			= false;
	m_dwOpenBreakTime		= 0;
}

//����: ��ȡĳ�ܼ���ı����������,��������ŵ� m_NewCmd ��.(ע��: �Բ�ͬ����,m_NewCmd�Ľṹ�ǲ�ͬ��)
//����:@iGrp	Ҫ��ȡ������ܼ���.
//����: �����ȡ�ɹ���Ϊ��Ч���� true,���򷵻� false.
bool CTmpCtrl::GetSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[20];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8263, bCmd, &m_NewCmd.dwTime) <= 0)	//��ȡ��Ӧ�ܼ����"��ʱ�¸�������".
	{
		DTRACE(DB_LOADCTRL, ("CTmpCtrl::GetSysCmd: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	m_NewCmd.bAct		= bCmd[0];
	m_NewCmd.bWndTime	= bCmd[4];
	m_NewCmd.bQuotiety	= bCmd[6];
	m_NewCmd.bDelayTime	= bCmd[8];
#ifdef PRO_698
	m_NewCmd.bCtrlTime = bCmd[10];
	m_NewCmd.bAlrTime[0] = bCmd[12];
	m_NewCmd.bAlrTime[1] = bCmd[14];
#endif

	if (m_NewCmd.bAct!=1 && m_NewCmd.bAct!=2)
		return false;

	char cTime[20];

	
	/*if (NewCmdTime() == 0)
	{ //������յ���ʱ������ڻ����ҿ��������ǡ���ʱ�¸��ء������������Ҫ������
		ClrSysCmd(iGrp);	//ɾ�����ܼ��������.
		if (iGrp == m_iGrp)
		{
			RstSysCtrlStatus(iGrp);	//��λϵͳ�⵱ǰ�ܼ��鱾�����״̬(���ܰ����ִ�״̬,Ͷ���־�ȵ�).
			RstCtrl();				//��λ�ڴ��б�����Ƶ��������״̬.
			ClrCmd();				//����ڴ��б�����ƵĿ�������.
			m_iGrp = -1;			//����ǰ�ܼ�����Ϊ -1,��ʾ��ǰû���ܼ���Ͷ��.
			DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: ctrl quit at %s, gpr=%d\n",
								 TimeToStr(m_tmNow, cTime), iGrp));
			return false;
		}
	}*/

	//�ɵ������ִ��,֤��û�й���,ֻ���ն�ʱ����ǰ����,ϵͳ���е�����ʱ�䱻�����
	//�����ɵ�����,�ÿ��Ƽ���ִ��,������ʱ�����Ϊһ������һ���ʱ��
	if (m_NewCmd.dwTime == 0) //�ն�ʱ����ǰ����,ϵͳ���е�����ʱ�䱻�����
	{
		if (CurCmdTime()!=0 && iGrp==m_iGrp) //�ɵ������Ѿ�������,ֻ��ϵͳ���е�����ʱ�䱻�����
		{
			m_NewCmd.dwTime = GetCurTime(); //����Ϊһ������һ���ʱ��,�����Ժ�ͬ����ʱ��ıȽ�
			WriteItemEx(BN0, (WORD)iGrp, 0x8263, bCmd, m_NewCmd.dwTime);	//����Ӧ�ܼ����"��ʱ�¸�������"д�����ݿ�
		}
		//else ������Ǳ���Ͷ��,ֻ�������ʱ�䱻�����,��Ҳ���ܸ�����һ����ȷ��ʱ��
		//	   ��������Ͷ���û���ü�ɨ��ͱ������,Ҳ�����Ǹ��Ѿ�����������ľ�����,
		// 	   ֻ��ͻȻ����û����ס;
		// 	   �ǳ�������ϵͳ��Ψһ��Ͷ�������,�������������ʱ�������,�����ᱻִ��
	}

	return true;
}

//����: ���ϵͳ�Ȿ�ܼ��鱾���������.
//����:@iGrp	Ҫ���������ܼ���.
//����: �������ɹ����� true,���򷵻� false.
bool CTmpCtrl::ClrSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[20];
	memset( bCmd, 0, sizeof(bCmd) );

	WriteItemEx(BN0, (WORD)iGrp, 0x8263, bCmd);		//�����Ӧ�ܼ����"��ʱ�¸�������".

	TrigerSaveBank(BN0, SECT_CTRL, 0);	//��������.

	return true;
}

//����: ����ϵͳ�Ȿ������ִ����״̬.
//����:@iGrp	��ǰ���Ƶ��ܼ���.
//		@bTurnsStatus	�ִ�״̬
//����: ������óɹ����� true,���򷵻� false.
bool CTmpCtrl::SetSysCtrlTurnsStatus(int iGrp, BYTE bTurnsStatus)
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
	WriteItemEx(BN0, iGrp, 0x8261, bBuf);

	return true;
}

//����: �����ܼ���ϵͳ�Ȿ����Ʊ�־��Ϊָ��״̬.
//����:@iGrp			Ҫ�趨���ܼ���.
//	   @fStatus			״̬.
//����: ������óɹ��򷵻� true, ���򷵻� false.
bool CTmpCtrl::SetSysCtrlFlg(int iGrp, bool fStatus)
{
	if (fStatus)
	{
		TGrpCurCtrlSta tGrpCurCtrlSta;
		memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

		if (!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))
		{
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::SetSysCtrlFlg: There is something wrong when call GetGrpCurCtrlSta() !\n"));
			return false;
		}
	
		tGrpCurCtrlSta.FloatRate = m_CtrlCmd.bQuotiety;

		if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))
		{
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::SetSysCtrlFlg: There is something wrong when call SetGrpCurCtrlSta() !\n"));
			return false;
		}
/*

		BYTE bBuf[1+1+1+8*8];	//���8������.

		//!!!����ڱ���߳��л�д��ID,������Ҫ�����ź�������.
		if (ReadItemEx(BN0, PN0, 0x105f, bBuf) <=0)	//��"�ն˵�ǰ����״̬".
		{
			DTRACE(DB_LOADCTRL, ("CTmpCtrl::SetSysCtrlFlg: There is something wrong when call ReadItemEx() !\n"));
			return false;
		}
		memcpy(bBuf+3+(8*(iGrp-GRP_START_PN))+2, &m_CtrlCmd.bQuotiety, 1);

		WriteItemEx(BN0, PN0, 0x105f, bBuf);	//д"�ն˵�ǰ����״̬".
*/

		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x08, fStatus, PWR_CTL);//��ʱ�¸���ʹ��3λ.
	}
	else
	{
		//int64 i = INVALID_VAL64;

		//WriteItemVal64(BN0, (WORD)m_iGrp, 0x084f, &i);	//���¸��ع��ʶ�ֵ��Ϊ��Чֵ.
		//WriteItemVal64(BN0, (WORD)m_iGrp, 0x111f, &i);	//����ǰ�ܼ���Ĺ����¸��غ��ܼ��й����ʶ���ֵ��Ϊ��Чֵ.

		return CGrpCtrl::ChgSysCtrlFlgs(iGrp, 0x08, fStatus, PWR_CTL);//��ʱ�¸���ʹ��3λ.
	}
}
#ifdef PRO_698
//����:�Ƿ��բ
//����: ������óɹ��򷵻� true, ���򷵻� false.
bool CTmpCtrl::RestoreTurnStatus()
{
	BYTE bEnableClose = 0;
	DWORD dwPersistTime;
	char msg[100];
	//ReadItemEx(BN0, PN0,0x0a08, &bEnableClose);//�Ƿ������բ
	//if (bEnableClose) 
	{
		int iTurn= GetIdxOfMostLeft1(m_bTurnsStatus)-1;	//��ȡ��Ӧ�ܼ��鵱ǰ����բ���ִκ�.

		dwPersistTime = (DWORD )m_CtrlCmd.bAlrTime[iTurn] *60;

		if (m_dwNow > m_dwAlrTime+dwPersistTime)
		{
			m_bTurnsStatus &= ~(1<<(iTurn));
			m_dwAlrTime = 0;
		}
	}
	return true;
}
#endif
