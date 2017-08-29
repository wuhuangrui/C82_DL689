/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�LoadCtrl.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��CLoadCtrl����
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��2��
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
#include "LoadCtrl.h"
#include "YK.h"
#include "board.h"
#include "DrvCtrlAPI.h"
#include "DpGrp.h"

#ifndef SYS_WIN
#include "bios.h"
#endif

CLoadCtrl g_LoadCtrl;

//========================================= CAllPwrCtrl ============================================
//
CAllPwrCtrl::CAllPwrCtrl(void)
{
	m_iGrp  = -1;
	m_bTurnsStatus = 0x00;
	m_bAlrsStatus = 0x00;
}

CAllPwrCtrl::~CAllPwrCtrl()
{
}

//����: ���س�ʼ��
bool CAllPwrCtrl::Init(void)
{
	m_TmpCtrl.Init();
	m_ShutoutCtrl.Init();
	m_RestCtrl.Init();
	m_PeriodCtrl.Init();

	//��Ҫɾ���������,����Ҫ��������ǰ����״̬��ʱ�򿪷���һ��
	/*BYTE bSetStatusBuf[1+1+6*8];
	BYTE bCurStatusBuf[1+1+1+8*8];

	if (ReadItemEx(BN0, PN0, 0x104f, bSetStatusBuf) != sizeof(bSetStatusBuf))	//��"�ն˿�������״̬".
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::Init: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	if (ReadItemEx(BN0, PN0, 0x105f, bCurStatusBuf) != sizeof(bCurStatusBuf))	//��"�ն˵�ǰ����״̬".
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::Init: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	BYTE bGrpFlgs = bSetStatusBuf[1];
	BYTE bPwrCtrl = 0;
	int j = -1;

	for (int i=0; i<8; i++,bGrpFlgs>>=1)
	{
		if ((bGrpFlgs&0x01)!=0 && bPwrCtrl<bSetStatusBuf[2+6*i+2])
		{//����Ч���ܼ���������,�ҵ��������ȼ���ߵĹ��ص���һ��.
			bPwrCtrl = bSetStatusBuf[2+6*i+2];
			j = i;
		}
	}
	if (j >= 0)
	{
		m_bTurnsStatus = bCurStatusBuf[3+8*j+3];
		m_bAlrsStatus  = bCurStatusBuf[3+8*j+6];
	}*/

	m_bTurnsStatus = 0;
	m_bAlrsStatus = 0;

	return true;
}

//����: ����ִ��
//����: ������������ true,���򷵻� false.
bool CAllPwrCtrl::DoCtrl(void)
{
	//m_TmpCtrl.DoSaveOpenRec();
	//m_ShutoutCtrl.DoSaveOpenRec();
	//m_RestCtrl.DoSaveOpenRec();
	//m_PeriodCtrl.DoSaveOpenRec();

	BYTE bTurnsStatus = 0x00;
	BYTE bAlrsStatus = 0x00;
	int iGrp;
#ifndef SYS_WIN
	if(m_TmpCtrl.IsValid()||m_ShutoutCtrl.IsValid()||m_RestCtrl.IsValid()||m_PeriodCtrl.IsValid())
	{//�������ص�
		SetCtrlLed(true, LED_POWERCTRL); 
	}
	else
	{
		SetCtrlLed(false, LED_POWERCTRL); 
	}
#endif
	m_TmpCtrl.DoCtrl();	//ִ��'��ʱ�¸���'.
	if (m_TmpCtrl.IsValid())
	{
//		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::DoCtrl: cur ctrl is TMP!\n"));

		bTurnsStatus = m_TmpCtrl.GetTurnsStatus();
		if (m_TmpCtrl.IsAlr())
			bAlrsStatus = 0x08;
		iGrp = m_TmpCtrl.GetGrp();
		m_ShutoutCtrl.RstCtrl();
		m_ShutoutCtrl.DoCmdScan();
		m_RestCtrl.RstCtrl();
		m_RestCtrl.DoCmdScan();
		m_PeriodCtrl.RstCtrl();
		m_PeriodCtrl.DoCmdScan();
	}
	else
	{//����'��ʱ�¸���'û��Ͷ��.
		m_ShutoutCtrl.DoCtrl();	//ִ��'Ӫҵ��ͣ��'.
		if (m_ShutoutCtrl.IsInCtrl())
		{
//			DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::DoCtrl: cur ctrl is SHUTOUT!\n"));

			bTurnsStatus = m_ShutoutCtrl.GetTurnsStatus();
			if (m_ShutoutCtrl.IsAlr())
				bAlrsStatus = 0x04;
			iGrp = m_ShutoutCtrl.GetGrp();
			m_RestCtrl.RstCtrl();
			m_RestCtrl.DoCmdScan();
			m_PeriodCtrl.RstCtrl();	//����Ч��־�������
			m_PeriodCtrl.DoCmdScan();
		}
		else
		{//����'Ӫҵ��ͣ��'û��Ͷ��.
			m_RestCtrl.DoCtrl();	//ִ��'���ݿ�'.
			if (m_RestCtrl.IsInCtrl())
			{
//				DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::DoCtrl: cur ctrl is REST!\n"));

				bTurnsStatus = m_RestCtrl.GetTurnsStatus();
				if (m_RestCtrl.IsAlr())
					bAlrsStatus = 0x02;
				iGrp = m_RestCtrl.GetGrp();
				m_PeriodCtrl.RstCtrl();
				m_PeriodCtrl.DoCmdScan();
			}
			else
			{//����'���ݿ�'û��Ͷ��.
				m_PeriodCtrl.DoCtrl();	//ִ��'ʱ�ο�'.
				if (m_PeriodCtrl.IsValid())
				{
//					DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::DoCtrl: cur ctrl is PERIOD!\n"));

					bTurnsStatus = m_PeriodCtrl.GetTurnsStatus();
					if (m_PeriodCtrl.IsAlr())
						bAlrsStatus = 0x01;
					iGrp = m_PeriodCtrl.GetGrp();
				}
				else
				{//���ض�û��Ͷ��.
					bTurnsStatus = 0x00;	//�������ִ���Ϊȫ�������բ.
					bAlrsStatus = 0x00;		//������״̬ȫ����Ϊδ����.
					iGrp = -1;				//��ʾû���κ��ܼ���Ͷ��.
				}
			}
		}
	}

	if (m_iGrp != iGrp)
	{//�鷢���任ʱ,ϵͳ���ִ�״̬�����.
		if (m_iGrp != -1)
		{
			ChgSysTurnsStatus(m_iGrp, CTL_TURN_MASK, false);	//����ǰ�ܼ���������ִ���Ϊ�����բ״̬.
			ChgSysPwrAlrFlgs(m_iGrp, 0x0f, false);				//����ǰ�ܼ�������б���״̬��Ϊδ����.
		}
		m_iGrp = iGrp;
		m_bTurnsStatus = 0x00;	//�л����µ��ܼ����,�ִ����״̬Ϊȫ�������բ.
		m_bAlrsStatus = 0x00;	//�л����µ��ܼ����,����״̬ȫ����Ϊ�Ǳ���״̬.
	}

	if (m_iGrp != -1)
	{//�鲻�䵫�ִ�״̬�����仯ʱ,Ҳ��Ҫ����ϵͳ�⹦���ִ�״̬.
		if (m_bTurnsStatus != bTurnsStatus)
		{//�ִ�״̬�����˱仯.
			m_bTurnsStatus = bTurnsStatus;
			//SetSysTurnsStatus(m_iGrp, m_bTurnsStatus);
		}
		if (m_bAlrsStatus != bAlrsStatus)
		{//����״̬�����˱仯.
			m_bAlrsStatus = bAlrsStatus;
			SetSysPwrAlrFlgs(m_iGrp, bAlrsStatus);
		}
	}

	/*m_TmpCtrl.MakeDisp(m_bTurnsStatus);
	m_RestCtrl.MakeDisp(m_bTurnsStatus);
	m_ShutoutCtrl.MakeDisp(m_bTurnsStatus);
	m_PeriodCtrl.MakeDisp(m_bTurnsStatus);*/

	return true;
}

//����: �趨ϵͳ��ָ���ܼ��鹦�ص����б���״̬.
//����:@iGrp			Ҫ�趨���ܼ���.
//	   @bFlgsStatus		Ҫ���õı���״̬.
//����: ������óɹ����� true,���򷵻� false.
bool CAllPwrCtrl::SetSysPwrAlrFlgs(int iGrp, BYTE bFlgsStatus)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	TGrpCurCtrlSta tGrpCurCtrlSta;
	memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

	if (!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//��"�ն˵�ǰ����״̬"
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::SetSysPwrAlrFlgs: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	tGrpCurCtrlSta.bPCAlarmState = bFlgsStatus & 0x0f;

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//д"�ն˵�ǰ����״̬".
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::SetSysPwrAlrFlgs: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}

	WORD wInID = 0;
	BYTE bBuf[10];
	memset(bBuf, 0, sizeof(bBuf));

	switch(bFlgsStatus)
	{
	case 1:	//ʱ�ο�
		wInID = 0x8232;
		break;
	case 2:	//���ݿ�
		wInID = 0x8242;
		break;
	case 4:	//Ӫҵ��ͣ��
		wInID = 0x8252;
		break;
	case 8:	//�¸���
		wInID = 0x8262;
		break;
	}

	BYTE *pbtr = bBuf;
	*pbtr++ = DT_STRUCT;
	*pbtr++ = 2;					//�ṹ��Ա����
	*pbtr++ = DT_OI;				//�ܼ������
	pbtr += OoWordToOi(0x2300+iGrp, pbtr);
	*pbtr++ = DT_ENUM;
	if (bFlgsStatus != 0)
		*pbtr++ = 1;
	else
		*pbtr++ = 0;

	if (wInID != 0)
	{
		bBuf[6] = 0;
		for (BYTE i=0; i<4; i++)
		{
			WriteItemEx(BN0, iGrp, 0x8232+(i<<4), bBuf); //�����һ����д
		}
		bBuf[6] = 1;
		WriteItemEx(BN0, iGrp, wInID, bBuf);
	}
	else
	{
		for (BYTE i=0; i<4; i++)
		{
			WriteItemEx(BN0, iGrp, 0x8232+(i<<4), bBuf);
		}
	}

	return true;
}

//����: �ı�ϵͳ��ָ���ܼ��鹦�ص���Ӧ����״̬.
//����:@iGrp 		Ҫ�ı���ܼ���.
//	   @bFlgs 		��Ҫ�ı�ı�־λ.
//	   @fStatus		true: ��λ��Ӧλ; false: �����Ӧλ.
//����: ������óɹ����� true,���򷵻� false.
bool CAllPwrCtrl::ChgSysPwrAlrFlgs(int iGrp, BYTE bFlgs, bool fStatus)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	TGrpCurCtrlSta tGrpCurCtrlSta;
	memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

	if (!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//��"�ն˵�ǰ����״̬"
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::ChgSysPwrAlrFlgs: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	if (fStatus)
	{
		tGrpCurCtrlSta.bPCAlarmState |= (bFlgs & 0x0f);
	}
	else
	{
		tGrpCurCtrlSta.bPCAlarmState &= ~(bFlgs & 0x0f);
	}

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//д"�ն˵�ǰ����״̬"
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::ChgSysPwrAlrFlgs: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}

	WORD wInID = 0;
	BYTE bBuf[10];
	memset(bBuf, 0, sizeof(bBuf));
	BYTE *pbtr = bBuf;
	*pbtr++ = DT_STRUCT;
	*pbtr++ = 2;					//�ṹ��Ա����
	*pbtr++ = DT_OI;				//�ܼ������
	pbtr += OoWordToOi(0x2300+iGrp, pbtr);
	*pbtr++ = DT_ENUM;
	*pbtr++ = fStatus;

	for (BYTE i=0; i<4; i++)
	{
		WriteItemEx(BN0, iGrp, 0x8232+(i<<4), bBuf);
	}

	return true;
}

//����: �趨ϵͳ��ָ���ܼ��鹦�ص������ִ�״̬.
//����:@iGrp			Ҫ�趨���ܼ���.
//	   @bTurnsStatus	Ҫ���õ��ִ�״̬.
//����: ������óɹ����� true,���򷵻� false.
bool CAllPwrCtrl::SetSysTurnsStatus(int iGrp, BYTE bTurnsStatus)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	TGrpCurCtrlSta tGrpCurCtrlSta;
	memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

	if(!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::SetSysTurnsStatus: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	tGrpCurCtrlSta.bAllPwrCtrlOutPutSta = bTurnsStatus & CTL_TURN_MASK;

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::SetSysTurnsStatus: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}

	return true;
}

//����: �ı�ϵͳ��ָ���ܼ��鹦�ص���Ӧ�ִ�״̬.
//����:@iGrp 		Ҫ�ı���ܼ���.
//	   @bTurns 		��Ҫ�ı���ִ�.
//	   @fStatus		true: ��λ��Ӧλ; false: �����Ӧλ.
//����: ������óɹ����� true,���򷵻� false.
bool CAllPwrCtrl::ChgSysTurnsStatus(int iGrp, BYTE bTurns, bool fStatus)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;


	TGrpCurCtrlSta tGrpCurCtrlSta;
	memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

	if (!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//��"�ն˵�ǰ����״̬"
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::ChgSysTurnsStatus: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	if (fStatus)
	{
		tGrpCurCtrlSta.bAllPwrCtrlOutPutSta |= (bTurns & CTL_TURN_MASK);
	}
	else
	{
		tGrpCurCtrlSta.bAllPwrCtrlOutPutSta &= ~(bTurns & CTL_TURN_MASK);
	}

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//д"�ն˵�ǰ����״̬".
	{
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::ChgSysTurnsStatus: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}

	WORD wInID = 0;
	BYTE bBuf[10];
	memset(bBuf, 0, sizeof(bBuf));
	BYTE *pbtr = bBuf;
	*pbtr++ = DT_STRUCT;
	*pbtr++ = 2;					//�ṹ��Ա����
	*pbtr++ = DT_OI;				//�ܼ������
	pbtr += OoWordToOi(0x2300+iGrp, pbtr);
	*pbtr++ = DT_BIT_STR;
	*pbtr++ = 8;
	*pbtr++ = tGrpCurCtrlSta.bAllPwrCtrlOutPutSta;

	for (BYTE i=0; i<4; i++)
	{
		WriteItemEx(BN0, iGrp, 0x8231+(i<<4), bBuf);
	}

	return true;
}

//����: ͳ�ƹ��س��޲���.
void CAllPwrCtrl::StatOverLimitPara(void)
{
	char cTime[20];
	BYTE bBuf[2+4];

	if (m_tmOldTime.nMonth!=m_tmNow.nMonth || m_tmOldTime.nYear!=m_tmNow.nYear)
	{//���緢�������л�.
		//�����¹��س��޲������Ƶ����¹��س��޲���.
		for (int i=GRP_START_PN; i<(GRP_START_PN+GRP_NUM); i++)
		{
			if (ReadItemEx(BN18, (WORD)i, 0x02cf, bBuf) <=0)
			{
				DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::StatOverLimitPara: There is something wrong when call ReadItemEx() !\n"));
				return;
			}
			WriteItemEx(BN0, (WORD)i, 0x326f, bBuf, 0, NULL, m_dwNow);
			DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::StatOverLimitPara: Save the %04d-%02d Group[%d] statistical parameter of PwrCtrl power over limit!\n"\
								 "Over time = %ld minutes\n"\
								 "Over limit energy = %lld KWH\n"\
								 "at %s\n", m_tmOldTime.nYear, m_tmOldTime.nMonth, i,
								 ByteToWord(bBuf), Fmt3ToVal64(bBuf+2, 4), TimeToStr(m_tmNow, cTime)));
		}
		//��ʼ�����¹��س��޲���.
		memset(bBuf, 0, sizeof(bBuf));	//���֮ǰ�ļ�¼.
		for (int i=GRP_START_PN; i<(GRP_START_PN+GRP_NUM); i++)
			WriteItemEx(BN18, (WORD)i, 0x02cf, bBuf);
	}

	DWORD dwTime = 0;
	int64 iEng = 0;
	int iGrp = 0;

	//��Ϊ��ĳ��ʱ��,ֻ����һ�ֹ���Ͷ��,���������ĸ�ͳ�������ֻ����һ��ͳ�Ƶ�����Ϊ0(���� dwTime, iEng ���и�д).
	m_TmpCtrl.SumOverLimitPara(iGrp, dwTime, iEng);		//ͳ����ʱ�¸��س���ʱ��,����.
	m_ShutoutCtrl.SumOverLimitPara(iGrp, dwTime, iEng);	//ͳ��Ӫҵ��ͣ�س���ʱ��,����.
	m_RestCtrl.SumOverLimitPara(iGrp, dwTime, iEng);	//ͳ�Ƴ��ݿس���ʱ��,����.
	m_PeriodCtrl.SumOverLimitPara(iGrp, dwTime, iEng);	//ͳ��ʱ�οس���ʱ��,����.

	if (dwTime!=0 || iEng!=0)
	{
		if (ReadItemEx(BN18, (WORD)iGrp, 0x02cf, bBuf) <=0)
		{
			DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::StatOverLimitPara: There is something wrong when call ReadItemEx() !\n"));
			return;
		}

		WORD w;

		w = ByteToWord(bBuf) + (WORD)(dwTime/60);	//����ʱ��.
		WordToByte(w, bBuf);
		iEng += Fmt3ToVal64(bBuf+2, 4);			//���ӳ��޵���.
		Val64ToFmt3(iEng, bBuf+2, 4);
		WriteItemEx(BN18, (WORD)iGrp, 0x02cf, bBuf);
#if 1
		DTRACE(DB_LOADCTRL, ("CAllPwrCtrl::StatOverLimitPara: Refresh Group[%d] statistical parameter of PwrCtrl power over limit!\n"\
								 "Over time = %ld minutes\n"\
								 "Over limit energy = %lld KWH\n"\
								 "at %s\n", iGrp, w, iEng, TimeToStr(m_tmNow, cTime)));
#endif
	}
}

//����: ��ȡ��ǰ���ȵĹ����ܼ��鹩��ʾ��
int CAllPwrCtrl::GetCurProGrp(void)
{
	if (m_iGrp != -1)
		return m_iGrp;
	if (m_TmpCtrl.IsValid())
		return m_TmpCtrl.GetGrp();
	if (m_ShutoutCtrl.IsValid())
		return m_ShutoutCtrl.GetGrp();
	if (m_RestCtrl.IsValid())
		return m_RestCtrl.GetGrp();
	if (m_PeriodCtrl.IsValid())
		return m_PeriodCtrl.GetGrp();
	return -1;
}

//
//========================================== CLoadCtrl =============================================
//
CLoadCtrl::CLoadCtrl(void)
{
}

CLoadCtrl::~CLoadCtrl()
{
}

//����: ���س�ʼ��
bool CLoadCtrl::Init(void)
{
	char cTime[20];
	BYTE bBuf[400];
	TTime tmNow;

	GetCurTime(&tmNow);
	m_dwStarupTime = TimeToSeconds(tmNow);

	m_tmOldTime = tmNow;
	m_dwOldTime = m_dwStarupTime;

	m_fEnableBreakAct		= false;
	m_bTurnsStatus			= 0x00;
	m_fBeepAlrStatus		= false;
	m_fTrigerAlr			= false;

	DTRACE(DB_LOADCTRL, ("CLoadCtrl::Init: Starup Time %s\n", TimeToStr(tmNow, cTime)));

	//��1�������еĿ���״̬
	/*ReadItemEx(BN0, PN0, 0x104f, bBuf);
	bBuf[0] &= 0x02;
	memset(&bBuf[1], 0, sizeof(bBuf)-1);
	WriteItemEx(BN0, PN0, 0x104f, bBuf);			//д"�ն˿�������״̬"ID
	memset(bBuf, 0, sizeof(bBuf));
	WriteItemEx(BN0, PN0, 0x105f, bBuf);	//д"�ն˵�ǰ����״̬".
	WriteItemEx(BN1, PN0, 0x3010, bBuf);	//д"�ն˵�ǰ����״̬".
	WriteItemEx(BN0, PN0, 0x5513, bBuf);*/
	
	InitSysCtrl();
	m_Guarantee.Init();
	m_UrgeFee.Init();

	int i;

	for (i=0; i<TURN_NUM; i++)
	{
		if (m_YkCtrl[i].SetTurn(i+TURN_START_PN))	//�����ڳ�ʼ��ǰ���ú��ִ�.
			m_YkCtrl[i].Init();
		else
		{
			DTRACE(DB_LOADCTRL, ("CLoadCtrl::Init: There is sth wrong when set current YkCtrl to Turn[%d].!\n", i));
			return false;
		}
	}
	m_MonthCtrl.Init();
	m_BuyCtrl.Init();
	m_AllPwrCtrl.Init();

	memset(m_bYkClosedTurns, 0, sizeof(m_bYkClosedTurns)); //�����Ʊ�ң�غ�բ�ϵ����Ѿ���բ���ִ�

	/*BYTE bBuf2[60];
	BYTE bTmp[] = {0xee,0xee,0xee,0,0,0,0,0};
	BYTE bTmp2[] = {0,0,0,0,0,0};

	memset(bBuf, 0 ,sizeof(bBuf));
	memset(bBuf2, 0 ,sizeof(bBuf2));
	for (int iGrpNum=1; iGrpNum <GB_MAXSUMGROUP; iGrpNum++)
	{
		if(IsGrpValid(iGrpNum))
		{
			bBuf[2] |= (1<< (iGrpNum-1));
			memcpy(bBuf+3+ (iGrpNum-1)*8, bTmp, 8);
			bBuf2[1] |= (1<< (iGrpNum-1));
		}
	}
	WriteItemEx(BN0, PN0, 0x105f, bBuf);	//д"�ն˵�ǰ����״̬".
	WriteItemEx(BN0, PN0, 0x104f, bBuf2);	//д"�ն˵�ǰ��������״̬".*/
	return true;
}

void CLoadCtrl::InitSysCtrl()
{
	BYTE bBuf[100];
	bool fIndex = false;
	memset(bBuf,0,sizeof(bBuf));
	if (ReadItemEx(BN1, PN0, 0x3010, bBuf)>0 && bBuf[0]>0)
	{
		BYTE bSize = bBuf[0];
		int i=0;
		while(i < bSize)
		{
			if (bBuf[1+i*2] >= CTL_YkCtrl_CLOSE)
			{
				bSize--;
				BYTE* pbBuf = &bBuf[3+i*2];
				memcpy(bBuf+1+i*2, pbBuf, sizeof(bBuf)-3-2*i);
			}
			else
			{
				i++;
			}
		}
		bBuf[0] = bSize;
		WriteItemEx(BN1, PN0, 0x3010, bBuf);
	}
	memset(bBuf, 0, sizeof(bBuf));
	WriteItemEx(BN0, PN0, 0x0910, bBuf);
	WriteItemEx(BN0, PN0, 0x0920, bBuf);
	WriteItemEx(BN0, PN0, 0x0930, bBuf);
}

bool CLoadCtrl::GetSoundBeepStats()
{
	DWORD dwAlarmFlg;
	TTime tmNow;
	GetCurTime(&tmNow);
	ReadItemEx(BN0, PN0, 0x039f, (BYTE*)&dwAlarmFlg);
	if (dwAlarmFlg & (1<<tmNow.nHour))
	{
		return true;
	}
	else
		return false;
}

//����: ����ִ��
bool CLoadCtrl::DoCtrl(void)
{
    bool fChange;
	GetCurTime(&m_tmNow);				//��ȡ��ǰʱ��.
	m_dwNow = TimeToSeconds(m_tmNow);	//ת��Ϊ DWORD ��ʽ.

	if (!m_fEnableBreakAct)
	{
		m_Guarantee.SetSysCtrlStatus(AUTO_GUARANTEE);
	}
	else
	{
		m_Guarantee.DoCtrl();
	}
	
	m_UrgeFee.DoCtrl();

	int i;
	BYTE bYkCtrlTurnsStatus = 0;	//���ȳ�ʼ��Ϊ�����ִζ������բ��״̬.
	BYTE bYkClosedTurns = 0;		//����ִ�б�ң�غ�բ���ִ�

	for (i=0; i<TURN_NUM; i++)
	{
		m_YkCtrl[i].DoCtrl();
		//m_YkCtrl[i].MakeDisp();

		if (m_YkCtrl[i].GetTurnStatus())
			bYkCtrlTurnsStatus |= 0x01<<i;

		if (m_YkCtrl[i].IsRxCloseCmd()) //����ִ���Ƿ��յ���һ��ң�غ�բ����
			bYkClosedTurns |= 0x01<<i;
	} 
	if (IsGuarantee())
	{//��������Ͷ�룬���������
		SetCtrlLed(true, LED_GUARANTEE);
	}
	else
	{
		SetCtrlLed(false, LED_GUARANTEE);
	}


	m_MonthCtrl.DoCtrl();
	//m_MonthCtrl.MakeDisp(m_bTurnsStatus);

	m_BuyCtrl.DoCtrl();
	//m_BuyCtrl.MakeDisp(m_bTurnsStatus);

	m_AllPwrCtrl.DoCtrl();
	
#ifndef SYS_WIN
	if(m_BuyCtrl.IsValid()||m_MonthCtrl.IsValid())
	{//������ص�
		SetCtrlLed(true, LED_ENERGYCTRL);
	}
	else
	{
		SetCtrlLed(false, LED_ENERGYCTRL);
	}
#endif
	char cTime[20];

	/*
	**Ŀǰ�������Э����δҪ��Ͷ�����ƹ��ܵ�����ͳ�ƣ������ͳ�ƴ��������ε�������Э��Ҫ�����ٴ򿪲��޸� -QLS 17.01.10
	*/
/*
	//ͳ�Ƴ��޲���
	m_MonthCtrl.StatOverLimitPara();
	m_AllPwrCtrl.StatOverLimitPara();

	char cTime[20];
	BYTE bBuf[128];

	//ͳ����բ����
	if (m_tmOldTime.nDay!=m_tmNow.nDay || m_tmOldTime.nMonth!=m_tmNow.nMonth || m_tmOldTime.nYear!=m_tmNow.nYear)
	{//���������л������л�
		if (m_tmOldTime.nMonth!=m_tmNow.nMonth || m_tmOldTime.nYear!=m_tmNow.nYear)
		{//���������л�
			//�����¿���ͳ�����ݸ��Ƶ����¿���ͳ������.
			if (ReadItemEx(BN11, PN0, 0x025f, bBuf) <=0)
			{
				DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: There is something wrong when call ReadItemEx() !\n"));
				return false;
			}
			WriteItemEx(BN0, PN0, 0x31ef, bBuf, 0, NULL, m_dwNow);
			WriteItemEx(BN0, PN0, 0x713c, bBuf, 0, NULL, m_dwNow);
			DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Save the %04d-%02d month's statistic of open break times!\n"\
								 "MonthCtrl open break times = %d\n"\
								 "BuyCtrl open break times = %d\n"\
								 "PowerCtrl open break times = %d\n"\
								 "YK open break times = %d\n"\
								 "at %s\n", m_tmOldTime.nYear, m_tmOldTime.nMonth,
								 bBuf[0], bBuf[1], bBuf[2], bBuf[3], TimeToStr(m_tmNow, cTime)));
			//��ʼ�����¿���ͳ������.
			memset(bBuf, 0, sizeof(bBuf));	//���֮ǰ�ļ�¼.
			WriteItemEx(BN11, PN0, 0x025f, bBuf);
		}
		//�����տ���ͳ�����ݸ��Ƶ����տ���ͳ������.
		if (ReadItemEx(BN11, PN0, 0x023f, bBuf) <=0)
		{
			DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: There is something wrong when call ReadItemEx() !\n"));
			return false;
		}
		WriteItemEx(BN0, PN0, 0x31bf, bBuf, 0, NULL, m_dwNow);
		WriteItemEx(BN0, PN0, 0x712c, bBuf, 0, NULL, m_dwNow);
		
		DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Save the %04d-%02d-%02d day's statistic of open break times!\n"\
							 "MonthCtrl open break times = %d\n"\
							 "BuyCtrl open break times = %d\n"\
							 "PowerCtrl open break times = %d\n"\
							 "YK open break times = %d\n"\
							 "at %s\n", m_tmOldTime.nYear, m_tmOldTime.nMonth, m_tmOldTime.nDay,
							 bBuf[0], bBuf[1], bBuf[2], bBuf[3], TimeToStr(m_tmNow, cTime)));
		//��ʼ�����տ���ͳ������.
		memset(bBuf, 0, sizeof(bBuf));	//���֮ǰ�ļ�¼.
		WriteItemEx(BN11, PN0, 0x023f, bBuf);
		//TrigerSave();
	}

	BYTE bYkCtrlOpenTimes = 0;

	for (i=0; i<TURN_NUM; i++)
		bYkCtrlOpenTimes += (BYTE)m_YkCtrl[i].GetOpenTimes();
	BYTE bMonthCtrlOpenTimes = (BYTE)m_MonthCtrl.GetOpenTimes();
	BYTE bBuyCtrlOpenTimes = (BYTE)m_BuyCtrl.GetOpenTimes();
	BYTE bAllPwrCtrlTimes = (BYTE)m_AllPwrCtrl.GetOpenTimes();

	if (bYkCtrlOpenTimes+bMonthCtrlOpenTimes+bBuyCtrlOpenTimes+bAllPwrCtrlTimes != 0)
	{//˵������բ����.
		//������ͳ������.
		if (ReadItemEx(BN11, PN0, 0x023f, bBuf) <=0)
		{
			DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: There is something wrong when call ReadItemEx() !\n"));
			return false;
		}
		bBuf[0]	+= bMonthCtrlOpenTimes;	//�����µ����բ����.
		bBuf[1]	+= bBuyCtrlOpenTimes;	//���¹������բ����.
		bBuf[2]	+= bAllPwrCtrlTimes;	//���¹�����բ����.
		bBuf[3]	+= bYkCtrlOpenTimes;	//����ң����բ����.
		WriteItemEx(BN11, PN0, 0x023f, bBuf);
#if 1
		DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Refresh day's statistic of open break times!\n"\
							 "MonthCtrl open break times = %d\n"\
							 "BuyCtrl open break times = %d\n"\
							 "PowerCtrl open break times = %d\n"\
							 "YK open break times = %d\n"\
							 "at %s\n", bBuf[0], bBuf[1], bBuf[2], bBuf[3], TimeToStr(m_tmNow, cTime)));
#endif
		//������ͳ������.
		if (ReadItemEx(BN11, PN0, 0x025f, bBuf) <=0)
		{
			DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: There is something wrong when call ReadItemEx() !\n"));
			return false;
		}
		bBuf[0]	+= bMonthCtrlOpenTimes;	//�����µ����բ����.
		bBuf[1]	+= bBuyCtrlOpenTimes;	//���¹������բ����.
		bBuf[2]	+= bAllPwrCtrlTimes;	//���¹�����բ����.
		bBuf[3]	+= bYkCtrlOpenTimes;	//����ң����բ����.
		WriteItemEx(BN11, PN0, 0x025f, bBuf);
#if 1
		DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Refresh month's statistic of open break times!\n"\
							 "MonthCtrl open break times = %d\n"\
							 "BuyCtrl open break times = %d\n"\
							 "PowerCtrl open break times = %d\n"\
							 "YK open break times = %d\n"\
							 "at %s\n", bBuf[0], bBuf[1], bBuf[2], bBuf[3], TimeToStr(m_tmNow, cTime)));
#endif
		//TrigerSave();
	}*/


	//�����ִο���״̬.
	//NOTE:���ڿ���״̬�ŵ�ÿ�������м��㻹�Ƿŵ���������ܵļ��������:
	// 	   ����״̬��ʵ�������ϵͳ��F6����բ���״̬���ŵ�������ʵ��,��Ϊ
	// 	   1.���صĶ��ֿ��Ƶ��ܵ�״̬���ܵ�������,��������ۺϴ���
	// 	   2.�����Ʊ�ң�غ�բ�ϵ����Ѿ���բ���ִ�,ҲҪ�����ۺϴ���,���繦�صĶ��ֿ��ƾͲ���
	// 		 �ֿ�����,���������ȼ����л��ĵ��¿��Ƶ��л���ʱ��������������
	BYTE bTurnsStatus;
	BYTE bOpenTurns[3];
	if (m_Guarantee.IsValid())
	{
		bTurnsStatus = 0x00;	//����Ͷ��,����բ�������բ.
		memset(m_bYkClosedTurns, 0, sizeof(m_bYkClosedTurns)); //�����Ʊ�ң�غ�բ�ϵ����Ѿ���բ���ִ�
		memset(bOpenTurns, 0, sizeof(bOpenTurns));
	}
	else
	{
		//bTurnsStatus = bYkCtrlTurnsStatus | m_MonthCtrl.GetTurnsStatus() | m_BuyCtrl.GetTurnsStatus() | m_AllPwrCtrl.GetTurnsStatus();
		bOpenTurns[0] = m_AllPwrCtrl.GetTurnsStatus();
		bOpenTurns[1] = m_MonthCtrl.GetTurnsStatus();
		bOpenTurns[2] = m_BuyCtrl.GetTurnsStatus();

		bTurnsStatus = bYkCtrlTurnsStatus;
		for (i=0; i<3; i++)
		{
			//��ÿ�����Ʊ�ң�غ�բ�ϵ����Ѿ���բ���ִ�
			m_bYkClosedTurns[i] = (m_bYkClosedTurns[i] | bYkClosedTurns) & bOpenTurns[i] & (~bYkCtrlTurnsStatus);
									//& bOpenTurns[i]:ֻ���Ѿ���բ���ִβż��±�ң�غ�բ�ϵ����ִ�,��֤��û��բ���ִ��ܼ�����
									//				  ���ĳ���ִκ��汻���Ʊ��������բ��,������ñ�־
									//~bYkCtrlTurnsStatus ң�������բ�ı�־λ,��ȥ�����ڱ�ң����բ���ִ�,

			//��ÿ�����Ƶ�ʵ�����
			bOpenTurns[i] &= ~m_bYkClosedTurns[i];	//������ϵ���բ

			//���ܵ�ʵ�����
			bTurnsStatus |= bOpenTurns[i];
		}
	}
	
	/*
	**�������Э����δ�����ն��ܵĿ���״̬�����������ε�����Ҫʱ�ٿ��Ų��޸� --QLS 17.01.10
	*/
	//����ϵͳ��F6����բ���״̬
	//NOTE:1.��Щ�Ѿ�ʧЧ���ܼ����״̬�ɿ��Ʊ��������,����ֻ�ܵ�ǰ��Ч���ܼ���
	// 	   2.ң�ص�״̬����ң�غ�բӰ��,�Լ����,�����������,
	int iGrp;
	iGrp = m_AllPwrCtrl.GetGrp();
	if (iGrp != -1)
		m_AllPwrCtrl.SetSysTurnsStatus(iGrp, bOpenTurns[0]);

	iGrp = m_MonthCtrl.GetGrp();
	if (iGrp != -1)
		m_MonthCtrl.SetSysTurnsStatus(iGrp, bOpenTurns[1]);

	iGrp = m_BuyCtrl.GetGrp();
	if (iGrp != -1)
		m_BuyCtrl.SetSysTurnsStatus(iGrp, bOpenTurns[2]);


	//�������������״̬.
	bool fBeepAlrStatus, fIsOpen = false;
	fBeepAlrStatus = m_UrgeFee.IsBeepAlr();
	for (i=0; i<TURN_NUM; i++)
	{
		fBeepAlrStatus = fBeepAlrStatus || m_YkCtrl[i].IsBeepAlr();
	}
	fBeepAlrStatus = fBeepAlrStatus || m_MonthCtrl.IsBeepAlr() || m_BuyCtrl.IsBeepAlr() || m_AllPwrCtrl.IsBeepAlr() || m_fTrigerAlr;

	if (m_fBeepAlrStatus != fBeepAlrStatus)
	{
		m_fBeepAlrStatus = fBeepAlrStatus;
		DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Beep alarm status changed at %s, now status is '%1d'.\n", TimeToStr(m_tmNow, cTime), (int)m_fBeepAlrStatus));
	}
	//�������״̬.
	if (m_fBeepAlrStatus)
	{
		//if (GetSoundBeepStats())
		{
			SetAlrLedCtrlMode(true);
			DoYk(true, LED_ALERT);
		}
		//else
		//{
		//	DoYk(false, LED_ALERT);
		//	SetAlrLedCtrlMode(false);
		//}
	}
	else
	{
		m_dwWarnTime = 0;

		SetAlrLedCtrlMode(false);
		DoYk(false, LED_ALERT);
	}


	if (!m_fEnableBreakAct)
	{
		//if ("�˹����")
		//{
		//	DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoLoadCtrl: Output break lock have release by Operator!\n\
		//						  Now is %s\n",
		//						  TimeToStr(m_tmNow, cTime)));
		//	m_fEnableBreakAct = true;
		//}
		m_Guarantee.SetSysCtrlStatus(AUTO_GUARANTEE);
		BYTE bBuf[3];
		WORD wPwronSafeTime = 0;
		ReadItemEx(BN0, PN0, 0x8211, bBuf);
		wPwronSafeTime = OoLongUnsignedToWord(&bBuf[1]);
		if (m_dwNow < m_dwStarupTime)
			m_dwStarupTime = m_dwNow - GetClick(); //��ǰ��ʱ��������ʼʱ��
		if (m_dwNow >= m_dwStarupTime+wPwronSafeTime*60)
		{
			DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoLoadCtrl: The time of lock break in power on is over!\n"\
								 "at %s\n", TimeToStr(m_tmNow, cTime)));
			m_fEnableBreakAct = true;
			m_Guarantee.SetSysCtrlStatus(QUIT_GUARANTEE);
			m_Guarantee.DoCtrl();
		}
		if (!m_fEnableBreakAct)
			goto LC_EndofDoCtrl;
	}
	
	fChange = false;

	if (m_bTurnsStatus != bTurnsStatus)
	{
		m_bTurnsStatus = bTurnsStatus;
		DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Turns status changed at %s, now status is '%2x'.\n", TimeToStr(m_tmNow, cTime), m_bTurnsStatus));
		fChange = true;
	}
	if (!fChange && m_tmNow.nMinute==m_tmOldTime.nMinute)
		goto LC_EndofDoCtrl;

	//���բ״̬.
	for (i=0; i<TURN_NUM; i++)
	{

		if ((bTurnsStatus&0x01) != 0)
		{
		    //DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Turn[%d] is Open\n", i+1));
			DoYk(true, i);
			fIsOpen = true;
		}
		else
		{
		    //DTRACE(DB_LOADCTRL, ("CLoadCtrl::DoCtrl: Turn[%d] is Close\n", i+1));
			//if ( fIsOpen )
			//	Sleep (2000);
			DoYk(false, i);
			fIsOpen = false;
		}

		bTurnsStatus >>= 1;
	}


LC_EndofDoCtrl:
	m_tmOldTime = m_tmNow;		//�����ϴ�ִ�й��ص�ʱ��.
	m_dwOldTime = m_dwNow;		//�����ϴ�ִ�й��ص�ʱ��.

	return true;
}
bool CLoadCtrl::IsEnergyFee(void)								//�Ƿ񹺵�ѿ���
{
	return m_BuyCtrl.IsEnergyFee();
}
     
