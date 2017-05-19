/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ�����:Ctrl.cpp
 * ժ    Ҫ:���ļ���Ҫʵ��CCtrl����
 * ��ǰ�汾:1.0
 * ��    ��:�Ž���
 * �������:2008��3��
*********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "sysfs.h"
#include "FaCfg.h"
#include "FaConst.h"
#include "DbConst.h"
#include "ComAPI.h"
#include "TaskDB.h"
#include "DbAPI.h"
#include "DbFmt.h"
#include "TaskManager.h"
#include "CtrlBase.h"
#include "FaAPI.h"
#include "DpGrp.h"
#include "DbOIAPI.h"

BYTE g_bBitMask[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

//����: ��ȡĳ��������ָ��λ��״̬.
//����:@pbBuf	������ָ��.
//	   @wLen	����������.
//	   @wIdx	bit��λ������.
//����: ָ��λ��״̬.
bool GetBitStatus(BYTE* pbBuf, WORD wLen, WORD wIdx)
{
	WORD w = wIdx / 8;

	if (wLen <= w)
		return false;

	return ((*(pbBuf+w)&g_bBitMask[wIdx%8]) != 0);
}

//����: ����ĳ��������ָ��λ��״̬.
//����:@pbBuf	������ָ��.
//	   @wLen	����������.
//	   @wIdx	bit��λ������.
//����: �����Ƿ�ɹ�.
bool SetBitStatus(BYTE* pbBuf, WORD wLen, WORD wIdx, bool fStatus)
{
	WORD w = wIdx / 8;

	if (wLen <= w)
		return false;

	if (fStatus)
		*(pbBuf+w) |= g_bBitMask[wIdx%8];
	else
		*(pbBuf+w) &= ~g_bBitMask[wIdx%8];

	return true;
}

//����: ת�����ݸ�ʽ19
//����:@pb		������ָ��
//����: ����ת����� DWORD ��ʽ������
DWORD TranDataFmt19(BYTE* pb)
{
	return (BcdToDWORD(pb, 1) + BcdToDWORD(pb+1, 1)*60) * 60;
}

//����: ת�����ݸ�ʽ20
//����:@pb		������ָ��
//����: ����ת����� DWORD ��ʽ������
DWORD TranDataFmt20(BYTE* pb)
{
	TTime Time;

	Time.nSecond = 0;
	Time.nMinute = 0;
	Time.nHour = 0;
	Time.nDay = BcdToByte(pb[0]);
	Time.nMonth = BcdToByte(pb[1]);
	Time.nYear = 2000 + BcdToByte(pb[2]);

	return TimeToSeconds(Time);
}

//����: ��ȡ�ֽ������ұ�1��λ��(0 ~ 7).
//����:@bFlgs	Ҫ��ѯ���ֽ�.
//����: D0~D7��˳���λ��ʾ0~7,��һ��1��û���򷵻� -1.
int GetIdxOfMostRight1(BYTE bFlgs)
{
	int i = -1;

	for (int i1=0; i1<8; i1++)
	{
		if ((bFlgs&1) != 0)
		{
			i = i1;
			break;
		}
		bFlgs >>= 1;
	}

	return i;
}

//����: ��ȡ�ֽ������ұ�1��λ��(0 ~ 7).
//����:@bFlgs	Ҫ��ѯ���ֽ�.
//����: D0~D7��˳���λ��ʾ0~7,��һ��1��û���򷵻� -1.
int GetIdxOfMostLeft1(BYTE bFlgs)
{
	int i = 0;

	for (int i1=8; i1>0; i1--)
	{
		if ((bFlgs&0x80) != 0)
		{
			i = i1;
			break;
		}
		bFlgs <<= 1;
	}

	return i;
}

//����: ��ȡָ��λ���������ұߵ�1�е�λ�ñ��(0 ~ 7).
//		01011010
//	��[1]λ�ı��Ϊ0,��[3]λ�ı��Ϊ1,��[4]λ�ı��Ϊ2,
//	��[6]λ�ı��Ϊ3,��������Ϊ0��λ������-1.
//����:@bFlgs	Ҫ��ѯ���ֽ�.
//	   @iIdx	Ҫ��ѯ��λ��.
//����: ��ָ��λ�õ�ֵΪ1,�򷵻�����λ�ñ��,���򷵻� -1.
int GetIdxOfAll1InPst(BYTE bFlgs, int iIdx)
{
	int i, i1 = -1;

	if (iIdx > 7)
		return -1;
	for (i=0; i<iIdx; i++)
	{
		if ((bFlgs&0x01) != 0)
			i1++;
		bFlgs >>= 1;
	}
	if ((bFlgs&0x01) != 0)
		return (i1+1);
	else
		return -1;
}

//����: ��ȡ����1�ĸ���.
//		01011010
//	1�ĸ���Ϊ4.
//����:@bFlgs	Ҫ��ѯ���ֽ�.
//����: ��ָ��λ�õ�ֵΪ1,�򷵻�����λ�ñ��,���򷵻� -1.
int GetSumOf1(BYTE bFlgs)
{
	int i, i1 = 0;

	for (i=0; i<8; i++)
	{
		if ((bFlgs&0x01) != 0)
			i1++;
		bFlgs >>= 1;
	}

	return i1;
}

//����: ��ȡָ���ܼ�����Ч��ǰ�й�����.
//����:@iGrp 	Ҫ��ȡ���ܼ���.
//����: ���ָ���ܼ�����Ч,���й�������Ч������ָ���ܼ��鵱ǰ�й�����,���򷵻� -1.
int64 GetValidCurPwr(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return -1;
	
	int64 iPwr;
	BYTE bBuf[9] = {0};

	if (ReadItemEx(BN0, iGrp, 0x2302, bBuf) <= 0)
	{
		DTRACE(DB_LOADCTRL, ("GetValidCurPwr: There is something wrong when call ReadItemEx() !\n"));
		return -1;
	}

	iPwr = OoLong64ToInt64(bBuf+1);
	/*if (ReadItemVal64(BN0, (WORD)iGrp, 0x109f, &iPwr) <= 0)	//��ȡָ���ܼ���"�й�����".C1F17
	{
		DTRACE(DB_LOADCTRL, ("GetCurPwr: There is something wrong when call ReadItemEx() !\n"));
		return -1;
	}*/

	/*if (iPwr == INVALID_VAL64)
		return -1;*/

	return iPwr;
}

//����: ��ȡָ���ܼ��鵱ǰ�й�����.
//����:@iGrp 	Ҫ��ȡ���ܼ���.
//����: ���ָ���ܼ�����Ч,���й�������Ч������ָ���ܼ��鵱ǰ�й�����,���򷵻� 0.
int64 GetCurPwr(int iGrp)
{
	int64 iPwr = GetValidCurPwr(iGrp);

//	if (iPwr < 0)
//		return 0;

	return iPwr;
}

//����: ��ȡָ���ܼ���ָ�����������й��ܵ���.
//����:@iGrp	Ҫ��ȡ���ܼ���.
//����: ָ�����������й��ܵ���,����������ݻ������Ч����,���ظ���.
int64 GetSelEng(int iGrp, int iSel)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return -1;

	WORD wID;
	int64 i64Buf[1+1+RATE_NUM];
	SetArrVal64(i64Buf, INVALID_VAL64, 1+1+RATE_NUM);

	switch(iSel)
	{
	case 0:	//��ǰ�ܼ��й����������ܡ�����1~M��ʾֵ
		wID = 0x130F;
		break;
	case 1:	//���������й��ܵ���.
		wID = 0x10bf;
		break;
	case 2:	//���������й��ܵ���.
		wID = 0x10df;
		break;
	default:
		return -1;
	}

	if (ReadItemVal64(BN0, (WORD)iGrp, wID, i64Buf) <= 0)	//����Ӧ���ܼ������Ӧ��PN��"���������й��ܵ���"ID
	{
		DTRACE(DB_LOADCTRL, ("GetSelEng: There is something wrong when call ReadItemVal64() !\n"));
		return -1;
	}

	if (i64Buf[1] == INVALID_VAL64)
		return -1;

	return i64Buf[1];
}

//����: �ж��Ƿ��ڱ���״̬.
//����: ������ڱ���״̬���� true,���򷵻� false.
bool IsGuarantee(void)
{
	BYTE bBuf[2];

	if (ReadItemEx(BN0, PN0, 0x8001, bBuf) <=0)	//��"�ն˿�������״̬"ID
	{
		DTRACE(DB_LOADCTRL, ("CCtrl::IsGuarantee: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	return (bBuf[1] != 0);	//�����0�������磨1�����Զ����磨2��
}


//========================================== CCtrlBase =============================================
TTime CCtrlBase::m_tmNow;
DWORD CCtrlBase::m_dwNow;

TTime CCtrlBase::m_tmOldTime;
DWORD CCtrlBase::m_dwOldTime;

//============================================ CCtrl ===============================================
//����: ��ʼ��.
//����: �����ʼ���������� true,���򷵻� false.
bool CCtrl::Init(void)
{
	m_fInCtrl = false;		//�ÿ����Ƿ��ڿ���״̬
	m_fGuarantee = false;	//�ÿ����Ƿ��Ѿ����ڱ���״̬,��Ҫ�����жϱ�����л�
	ClrCmd();		//����ڴ��б�����ƵĿ�������.
	RstCtrl();		//��λ�ڴ��б������״̬��.
	SetValidStatus(false);	//�ָ������˳�״̬.
	return true;
}

//����: �ж��Ƿ��ڱ���״̬.
//����: ������ڱ���״̬���� true,���򷵻� false.
//bool CCtrl::IsGuarantee(void)
//{
//	BYTE bBuf[1+1+6*8];
//
//	if (ReadItemEx(BN0, PN0, 0x104f, bBuf) <=0)	//��"�ն˿�������״̬"ID
//	{
//		DTRACE(DB_LOADCTRL, ("CCtrl::IsGuarantee: There is something wrong when call ReadItemEx() !\n"));
//		return 0x00;
//	}
//
//	return ((bBuf[0]&0x01) != 0);	//�����ʹ�õ�0λ.
//}

//========================================= CGrpCtrl ===============================================
//����: ɨ��ϵͳ����ܼ���ı����������.
//		����������������¼�������
//		1. ��ִ֤�и��ܼ��������µ���Ч����.
//			�������ǽ������,Ӧ��λ����״̬,����ϵͳ����ɾ����ǰ�������.
//			��������Ͷ������,���֮ǰ�յ������ͬ(ͨ�����������ʱ������
//			��),��λ����״̬,Ȼ�󽫸�����浽m_'Ctrl'Cmd('Ctrl'�ڸ�����
//			�����е����ֲ�ͬ)��,�������֮ǰ�յ���������ͬ,������ı��κ�
//			״̬.
//		2. ��֤ɾ����Ч�ĺ͹�ʱ������.
CGrpCtrl::CGrpCtrl()
{
	 m_iGrp = -1; 
	 m_wOpenTimes = 0; 
	 m_bTurnsStatus = 0;
	 m_fIfOverLimit = false;
	 m_wOpenTimes = 0;

	 m_dwInitClick = 0;
	 m_fAlarmStatus = false;
	 m_fOpenStatus = false;
	 m_iCtrlGrp = -1;
	 m_bCloseTurn = 0;
}

void CGrpCtrl::DoCmdScan(void)
{
	char cCtrlType[20];
	char cTime[20];
	int i;

	for (i=GRP_START_PN; i<(GRP_START_PN+GRP_NUM); i++)
	{
		if (!IsGrpValid(i))
			continue;

		if (!GetSysCmd(i))	//��ȡĳ�ܼ���ı����������,��������ŵ� m_NewCmd ��.(ע��: �Բ�ͬ����,m_NewCmd�Ľṹ�ǲ�ͬ��)
			continue;	//����û����������������Ĵ������,ֱ��ɨ����һ���ܼ���Ŀ������Ԫ.

		if (m_iGrp != i)
		{
			if (NewCmdAct() == 1)
			{//������'Ͷ��'����.
				DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: %s rx new cmd at %s, grp=%d, act=%d\n",
									 CtrlType(cCtrlType), TimeToStr(NewCmdTime(), cTime), i, NewCmdAct()));
				
				if (GRP_START_PN<=m_iGrp && m_iGrp<(GRP_START_PN+GRP_NUM))
				{//�統ǰȷʵͶ����ĳ���ܼ���'�������'.
					RstSysCtrlStatus(m_iGrp);			//��λϵͳ�⵱ǰ�ܼ��鱾�����״̬(���ܰ����ִ�״̬,Ͷ���־�ȵ�).
					ClrSysCmd(m_iGrp);					//���ϵͳ�⵱ǰ�ܼ��鱾���������.
					DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: %s quit at %s due to grp change, new grp=%d, old grp=%d\n",
										 CtrlType(cCtrlType), TimeToStr(m_tmNow, cTime), i, m_iGrp));
					//***��¼��ϵͳ����־��.
					//***���������ź�;
				}
				RstCtrl();					//��λ�ڴ��б�����Ƶ��������״̬.
				SetValidStatus(false);		//�ָ������˳�״̬.
				SaveNewCmd();				//������������.
				SetSysCtrlFlg(i, true);		//�����ܼ���ϵͳ�Ȿ����Ʊ�־��Ϊ��Ч.
				SetValidStatus(true);		//���ڴ��еı�����Ƶ�Ͷ��״̬��ΪͶ��.
				m_iGrp = i;					//�����ܼ��������Ϊ��ǰ�ܼ���.
				DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: %s launch at %s, grp=%d\n",
									 CtrlType(cCtrlType), TimeToStr(m_tmNow, cTime), i));
				//***��¼��ϵͳ����־��.
				//***���������ź�;
			}
			else if (NewCmdAct() >= 2)
			{//��⵽��������Ƿ�����,��Ϊ֮ǰ���鲢û��Ͷ��,�������Ҳ��Ϊ�Ƿ�����ɾ��.
				RstSysCtrlStatus(i);	//��λϵͳ�⵱ǰ�ܼ��鱾�����״̬(���ܰ����ִ�״̬,Ͷ���־�ȵ�).
				ClrSysCmd(i);
				DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: %s rx a cancle cmd, grp=%d!\n",
									 CtrlType(cCtrlType), i));
			}
		}
		else if (NewCmdTime() != CurCmdTime())
		{//�ڵ�ǰ�ܼ����⵽�µ�����.
			if (NewCmdAct() == 1)	//���������ǰ��Ͷ�������Ҳ����Ͷ��,ֻ������Ͷ���������ʱ�䷢���ı�,���ø�λ����
			{//������'Ͷ��'����.
				if (NewCmdAct()!=CurCmdAct() || CtrlType()==CTL_PWR_TMP) //��ʱ�޵���յ�ͬһ���ܼ��������ܲ�����û�����ı䶼��������
					RstCtrl();				//��λ�ڴ��б�����Ƶ��������״̬.

				SaveNewCmd();			//������������.
				SetSysCtrlFlg(i, true);	//�ڸ���һ��1�������е�F5,F6,��Щʱ����Ƶ��ܼ���û��,
										//������Щ���Ʋ������ܷ����˸ı�,Ҫ���½�ȥ
				SetValidStatus(true);	//���ڴ��еı�����Ƶ�Ͷ��״̬��ΪͶ��.
				DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: %s rx new cmd of the same grp=%d at %s, reset ctrl!\n",
									 CtrlType(cCtrlType), i, CtrlType(cCtrlType), TimeToStr(m_tmNow, cTime)));
				//***��¼��ϵͳ����־��.
				//***���������ź�;
			}
			else if (NewCmdAct() == 2)	//��ǰ�ܼ���������
			{
				RstSysCtrlStatus(i);	//��λϵͳ�⵱ǰ�ܼ��鱾�����״̬(���ܰ����ִ�״̬,Ͷ���־�ȵ�).
				ClrSysCmd(i);			//���ϵͳ�⵱ǰ�ܼ��鱾���������.
				RstCtrl();				//��λ�ڴ��б�����Ƶ��������״̬.
				SetValidStatus(false);	//�ָ������˳�״̬.
				ClrCmd();				//����ڴ��б�����ƵĿ�������.
				m_iGrp = -1;			//����ǰ�ܼ�����Ϊ -1,��ʾ��ǰû���ܼ���Ͷ��.
				DTRACE(DB_LOADCTRL, ("CGrpCtrl::DoCmdScan: %s quit at %s, grp=%d\n",
									 CtrlType(cCtrlType), TimeToStr(m_tmNow, cTime), i));
				//***��¼��ϵͳ����־��.
				//***���������ź�;
			}
		}
	}
}

//����: ��ȡϵͳ��ָ���ܼ���ָ���������ִ�����״��.
//����:@iGrp	Ҫ��ȡ���ܼ���.
//	   @iSel	������(0: ����; 1: ���).
//����: D0~D7��˳���λ��ʾ1~8�ִο��صĵ���ܿ�״̬;��"1":�ܿ�,��"0":���ܿ�.
BYTE CGrpCtrl::GetSysCtrlTurnsCfg(int iGrp, int iSel)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return 0x00;

	BYTE bCfg[3] = {0};
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;
	TGrpCtrlSetSta tGrpCtrlSetSta;
	memset(&tGrpCtrlSetSta, 0, sizeof(TGrpCtrlSetSta));

	if (!GetGrpCtrlSetSta(iGrp, &tGrpCtrlSetSta))
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::GetSysCtrlTurnsCfg: There is something wrong when call GetGrpCtrlSetSta() !\n"));
		return 0x00;
	}

	switch (iSel)
	{
	case 0:	//����
		OoReadAttr(0x2300+iGrp,	0x0E, bCfg, &pbFmt, &wFmtLen);	//�ܼ��鹦���ִ�����
		tGrpCtrlSetSta.bPwrCtrlTurnSta = bCfg[2] & CTL_TURN_MASK;
		break;
	case 1:	//���
		OoReadAttr(0x2300+iGrp,	0x0F, bCfg, &pbFmt, &wFmtLen); //�ܼ������ִ�����
		tGrpCtrlSetSta.bEngCtrlTurnSta = bCfg[2] & CTL_TURN_MASK;
		break;
	default:
		return 0x00;
	}

	if (!SetGrpCtrlSetSta(iGrp, &tGrpCtrlSetSta))
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::GetSysCtrlTurnsCfg: There is something wrong when call SetGrpCtrlSetSta() !\n"));
		return 0x00;
	}

	return (bCfg[2] & CTL_TURN_MASK);
}

//����: ��ȡϵͳ��ָ���ܼ���ָ������������б�־λ.
//����:@iGrp	Ҫ��ȡ���ܼ���.
//	   @iSel	������(0: ����; 1: ���).
//����: �������еı�־λ״̬.
BYTE CGrpCtrl::GetSysCtrlFlgs(int iGrp, int iSel)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return 0x00;
	if (iSel<0 || 1<iSel)
		return 0x00;

	BYTE bBuf[1+1+6*8];

	if (ReadItemEx(BN0, PN0, 0x104f, bBuf) <=0)	//��"�ն˿�������״̬"ID
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::GetSysCtrlFlgs: There is something wrong when call ReadItemEx() !\n"));
		return 0x00;
	}
	if (iSel == 0)
		return (bBuf[1+1+6*(iGrp-GRP_START_PN)+2] & 0x0f);	//Ŀǰֻ��0,1,3,4λ��ʹ��.
	else
		return (bBuf[1+1+6*(iGrp-GRP_START_PN)+3] & 0x03);	//Ŀǰֻ��0,1λ��ʹ��.
}

//����: �ı�ϵͳ��ָ���ܼ���ָ���������ָ����־λ״̬,����F5(�ն˿�������״̬)��F6(�ն˵�ǰ����״̬)
//����:@iGrp	Ҫ�ı���ܼ���.
//	   @bFlgs	Ҫ�ı�ı�־λ.
//	   @fStatus	Ҫ��ɵ�״̬.
//	   @iCtrlType ������(0: ����; 1: ���).
//����: ��ı�ɹ����� true,���򷵻� false.
bool CGrpCtrl::ChgSysCtrlFlgs(int iGrp, BYTE bFlgs, bool fStatus, int iCtrlType)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;
	if (iCtrlType<0 || 1<iCtrlType)
		return false;

	int i = iGrp - GRP_START_PN;
	/*BYTE bSetStatusBuf[1+1+6*8];
	BYTE bCurStatusBuf[1+1+1+8*8];*/
	TGrpCtrlSetSta tGrpCtrlSetSta;
	TGrpCurCtrlSta tGrpCtrlCtrlSta;
	memset(&tGrpCtrlSetSta, 0, sizeof(TGrpCtrlSetSta));
	memset(&tGrpCtrlCtrlSta, 0, sizeof(tGrpCtrlCtrlSta));

	if (iCtrlType == 0)
		bFlgs &= 0x0f;	//Ŀǰֻ��0,1,3,4λ��ʹ��.
	else
		bFlgs &= 0x03;	//Ŀǰֻ��0,1λ��ʹ��.

	if (!GetGrpCtrlSetSta(iGrp, &tGrpCtrlSetSta)) //��"�ն˿�������״̬".
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::ChgSysCtrlFlgs: There is something wrong when call GetGrpCtrlSetSta() !\n"));
		return false;
	}

	if (!GetGrpCurCtrlSta(iGrp, &tGrpCtrlCtrlSta)) //��"�ն˵�ǰ����״̬".
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::ChgSysCtrlFlgs: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	/*if (ReadItemEx(BN0, PN0, 0x104f, bSetStatusBuf) <=0)	//��"�ն˿�������״̬".
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::ChgSysCtrlFlgs: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	if (ReadItemEx(BN0, PN0, 0x105f, bCurStatusBuf) <=0)	//��"�ն˵�ǰ����״̬".
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::ChgSysCtrlFlgs: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}*/

	int i1;
	//�ı����״̬.
	if (fStatus)
	{
		//bSetStatusBuf[1+1+6*i+2+iCtrlType] |= bFlgs; //���״̬/����״̬
		if (iCtrlType == 0)
			tGrpCtrlSetSta.bPwrCtrlSta |= bFlgs;	//����״̬
		else
			tGrpCtrlSetSta.bEngCtrlSta |= bFlgs;	//���״̬


		//��Ӷ���ʱ�¸���ϵ���ĸ�ֵ����ʱ�¸�������Ŀ���Ͷ��ʱ����ʱ�¸���ϵ������Ϊ��Ч����.
		if (iCtrlType == 0)
		{
			for (i1=0; i1<4; i1++,bFlgs>>=1)
			{
				if ((bFlgs&0x01) == 0)
					continue;
				switch (i1)
				{
				case 0:	//ʱ�ο�
				case 1:	//���ݿ�
				case 2:	//Ӫҵ��ͣ��
					//bCurStatusBuf[1+1+1+8*i+2] = 0xee;	//��ʱ�¸��ظ���ϵ����Ϊ��Ч����.
					tGrpCtrlCtrlSta.FloatRate = 0xee;
					break;
				case 3:	//��ʱ�¸���
					break;
				default:
					return false;
				}
			}
		}
		/*else
		{
			for (i1=0; i1<2; i1++,bFlgs>>=1)
			{
				if ((bFlgs&0x01) == 0)
					continue;

				switch (i1)
				{
				case 0:	//�µ��
				case 1:	//�����
					bCurStatusBuf[1+1+1+8*i+2] = 0xee;	//��ʱ�¸��ظ���ϵ����Ϊ��Ч����.
					break;
				default:
					return false;
				}
			}
		}*/
	}
	else
	{
		//bSetStatusBuf[1+1+6*i+2+iCtrlType] &= ~bFlgs; //���״̬/����״̬
		if (iCtrlType == 0)
			tGrpCtrlSetSta.bPwrCtrlSta &= ~bFlgs;//����״̬
		else
			tGrpCtrlSetSta.bEngCtrlSta &= ~bFlgs;//���״̬

		//��Խ���Ŀ��ƽ�����ز���.
		if (iCtrlType == 0)
		{//����
			for (i1=0; i1<4; i1++,bFlgs>>=1)
			{
				if ((bFlgs&0x01) == 0)
					continue;
				switch (i1)
				{
				case 0:	//ʱ�ο�
					tGrpCtrlSetSta.bSchemeNum = 0xee;	//ʱ�οط�������Ϊ��Ч����.
					tGrpCtrlSetSta.bValidFlag = 0x00;	//ʱ�ο���Ч��־λȫ��0.
					break;
				case 1:	//���ݿ�
					break;
				case 2:	//Ӫҵ��ͣ��
					break;
				case 3:	//��ʱ�¸���
					//bCurStatusBuf[1+1+1+8*i+2] = 0xee;	//��ʱ�¸��ظ���ϵ����Ϊ��Ч����.
					tGrpCtrlCtrlSta.FloatRate = 0xee;
					break;
				default:
					return false;
				}
			}
			//��⵱ǰ�ܼ���Ĺ���״̬.
			if (tGrpCtrlSetSta.bPwrCtrlSta == 0x00)
			{
				tGrpCtrlSetSta.bPwrCtrlTurnSta = 0x00;		//�������ִ�״̬��Ϊȫ�����ɿ�.
				tGrpCtrlCtrlSta.CurPwrVal = 0xee;		//����ǰ���ض�ֵ��Ϊ��Ч����.
				//bCurStatusBuf[1+1+1+8*i+1] = 0xee;
				tGrpCtrlCtrlSta.bAllPwrCtrlOutPutSta = 0x00;		//������բ���״̬ȫ����Ϊ�����բ״̬.
			}
			//�����߼��ϵ������,������Ϊ,���ĳ�����������Ļ�,��ô�ÿ������Ӧ�ı���״̬ҲӦ��λ.
			//bCurStatusBuf[1+1+1+8*i+6] &= bSetStatusBuf[1+1+6*i+2];
			tGrpCtrlCtrlSta.bPCAlarmState &= tGrpCtrlSetSta.bPwrCtrlSta;
		}
		else
		{//���
			for (i1=0; i1<2; i1++,bFlgs>>=1)
			{
				if ((bFlgs&0x01) == 0)
					continue;
				switch (i1)
				{
				case 0:	//�µ��
					//bCurStatusBuf[1+1+1+8*i+4] = 0x00;	//�µ����բ���״̬ȫ����Ϊ�����բ״̬.
					tGrpCtrlCtrlSta.bMonthCtrlOutPutSta = 0x00;
					break;
				case 1:	//�����
					//bCurStatusBuf[1+1+1+8*i+5] = 0x00;	//�������բ���״̬ȫ����Ϊ�����բ״̬.
					tGrpCtrlCtrlSta.bBuyCtrlOutPutSta = 0x00;
					break;
				default:
					return false;
				}
			}
			//��⵱ǰ�ܼ���ĵ��״̬.
			if (tGrpCtrlSetSta.bEngCtrlSta == 0x00)
				tGrpCtrlSetSta.bEngCtrlTurnSta = 0x00;		//������ִ�״̬��Ϊȫ�����ɿ�.
			//�����߼��ϵ������,������Ϊ,���ĳ�����������Ļ�,��ô�ÿ������Ӧ�ı���״̬ҲӦ��λ.
			//bCurStatusBuf[1+1+1+8*i+7] &= bSetStatusBuf[1+1+6*i+3];
			tGrpCtrlCtrlSta.bECAlarmState &= tGrpCtrlSetSta.bEngCtrlSta;
		}
	}

	//��⵱ǰ�ܼ���ĵ�غ͹����Ƿ���Ͷ��Ŀ���,�Դ�Ϊ���ݸı䵱ǰ�ܼ������Ч��־.
//	if ((bSetStatusBuf[1+1+6*i+2] | bSetStatusBuf[1+1+6*i+3]) == 0x00)
//		bSetStatusBuf[1] &= ~(0x01 << i);	//����ܼ����û�й�����û�е��Ͷ��,��ô��ʾ���ܼ���δͶ��.
//	else

	/*for(int in=0;in<i+1;in++)  //�����ܼ�����Ч��ʶ
		bSetStatusBuf[1] |= (0x01 << in);
	bCurStatusBuf[2] = bSetStatusBuf[1];*/

	if (!SetGrpCtrlSetSta(iGrp, &tGrpCtrlSetSta)) //д"�ն˿�������״̬".
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::ChgSysCtrlFlgs: There is something wrong when call SetGrpCtrlSetSta() !\n"));
		return false;
	}

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCtrlCtrlSta)) //д"�ն˵�ǰ����״̬".
	{
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::ChgSysCtrlFlgs: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}

	/*WriteItemEx(BN0, PN0, 0x104f, bSetStatusBuf);	//д"�ն˿�������״̬".
	WriteItemEx(BN0, PN0, 0x105f, bCurStatusBuf);	//д"�ն˵�ǰ����״̬".*/

	return true;
}



//��������ϵͳ����ɾ����Ч�Ŀ���״̬��
//������@tInvCtrl   ��Ч�Ŀ���
//���أ���
void CGrpCtrl::RemoveDispItem(TCtrl tInvCtrl)
{
	BYTE bBuf[21];
	memset(bBuf, 0, sizeof(bBuf));

	if (ReadItemEx(BN1, PN0, 0x3010, bBuf)>0 && bBuf[0]>0)
	{
		BYTE bSize = bBuf[0]; //�澯�ĸ�����
		BYTE bInvCtrlType = tInvCtrl.bCtrlType;
		int iIndex = 0; //ʧЧ����ʾ�澯�����ڶ����е�λ�ã�

		//����ʧЧ����ʾ�澯�����������е�����
/*		for (BYTE i=0; i<bSize; i++)
		{
			if (bBuf[i*2+1] == bInvCtrlType)
			{
				iIndex = i;
				break;
			}
		}
*/
		//��������
		if (iIndex >= 0 && bSize > 0)
		{
			bSize --;
			bBuf[0] = bSize;
			BYTE* pbBuf = &bBuf[iIndex*2+3];
			memcpy(bBuf+iIndex*2+1, pbBuf, (bSize-iIndex)*2);
			memset(bBuf+bSize*2+1, 0, 20-bSize*2); 
			WriteItemEx(BN1, PN0, 0x3010, bBuf);
			DTRACE(DB_LOADCTRL, ("CGrpCtrl::RemoveDispItem: tCtrlType=%d, iGrp=%d!\n", tInvCtrl.bCtrlType, m_iCtrlGrp));
		}
	}
}

//��������ϵͳ���������µĿ���״̬��
//������@tTopCtrl  �µĿ��ƣ�����������λ����
//���أ���
void CGrpCtrl::AddDispItem(TCtrl tTopCtrl)
{
	BYTE bBuf[21], bTmpBuf[21];
	memset(bBuf, 0, sizeof(bBuf));

	if (ReadItemEx(BN1, PN0, 0x3010, bTmpBuf) > 0)
	{
		BYTE bSize = bTmpBuf[0]; //��ǰ�ĸ�����

		for (BYTE i=0; i<bSize; i++)
		{
			if (bTmpBuf[1+2*i]==tTopCtrl.bCtrlType)
				return;
		}

		BYTE *pbBuf = &bTmpBuf[1];
		bBuf[1] = tTopCtrl.bCtrlType;
		bBuf[2] = tTopCtrl.bCtrlTurn;
		if (bSize == 10)
		{
			memcpy(bBuf+3, pbBuf,18);
		}
		else
		{
			memcpy(bBuf+3, pbBuf, bSize*2);
			memset(bBuf+bSize*2+3, 0, 18-bSize*2);
			bSize++;
		}
		bBuf[0] = bSize; 
		WriteItemEx(BN1, PN0, 0x3010, bBuf);
		DTRACE(DB_LOADCTRL, ("CGrpCtrl::AddDispItem: tCtrlType=%d, iGrp=%d!\n",tTopCtrl.bCtrlType, m_iGrp));
	}
}



//����������ϵͳ��Ŀ���״̬��
void CGrpCtrl::MakeDisp(BYTE bTurnsStatus)
{
	TCtrl tCtrl;
	TCtrl tInvCtrl;

	bool fAlarmStatus = IsAlarmStatus();
	bool fOpenStatus = IsOpenStatus();

	if (m_iCtrlGrp != m_iGrp)
	{//�ܼ����л���ɾ��������ǰ���д����͵Ŀ���״̬��
		tInvCtrl.bCtrlType = GetCtrlType();
		RemoveDispItem(tInvCtrl);
		tInvCtrl.bCtrlType = GetInvCtrlType();
		RemoveDispItem(tInvCtrl);
		
		if (fAlarmStatus)
		{//����µ��ܼ��鴦�ڸ澯״̬��
			tCtrl.bCtrlType = GetCtrlType();
			
			int iSel;
			if (tCtrl.bCtrlType > 3) //����
				iSel = 0;
			else                   //��أ�
				iSel = 1;
			m_bCloseTurn = GetSysCtrlTurnsCfg(m_iGrp, iSel); //��բ�澯���ִΣ�
			tCtrl.bCtrlTurn = m_bCloseTurn;
			AddDispItem(tCtrl);
		}
		else if (m_iGrp==-1 && m_bTurnsStatus==0 && m_fOpenStatus)
		{//��ǰ���ڹ�����բ״̬�����ڴ��ں�բ״̬���򷢳���բ�澯��Ϣ��
			tCtrl.bCtrlType = GetInvCtrlType();
			tCtrl.bCtrlTurn = bTurnsStatus;
			AddDispItem(tCtrl);
			m_dwInitClick = GetClick();

			DTRACE(DB_LOADCTRL, ("CGrpCtrl::MakeDisp: The Turns Cancelled is = %d!\n", tCtrl.bCtrlTurn));
		}
	}
	else
	{
		if (fAlarmStatus != m_fAlarmStatus)//ͬһ�ܼ���Ŀ���״̬�����仯��
		{
			//ɾ����ǰ�Ŀ���״̬��
			tInvCtrl.bCtrlType = m_fAlarmStatus ? GetCtrlType(): GetInvCtrlType();
			RemoveDispItem(tInvCtrl);

			if (fAlarmStatus)
			{
				tCtrl.bCtrlType =  GetCtrlType();

				int iSel;
				if (tCtrl.bCtrlType > 3) //����
					iSel = 0;
				else                   //��أ�
					iSel = 1;
				m_bCloseTurn = GetSysCtrlTurnsCfg(m_iGrp, iSel);
				tCtrl.bCtrlTurn = m_bCloseTurn;
				AddDispItem(tCtrl);
			}
			else if (m_bTurnsStatus==0 && m_fOpenStatus)
			{
				tCtrl.bCtrlTurn = bTurnsStatus;
				tCtrl.bCtrlType = GetInvCtrlType();
				AddDispItem(tCtrl);

				m_dwInitClick = GetClick();//�������״̬Ϊ��բ״̬����¼�º�բ�տ�ʼ�ĵδ�
				DTRACE(DB_LOADCTRL, ("CGrpCtrl::MakeDisp: The Turns Cancelled is = %d!\n", m_bCloseTurn));
			}
			
		}
		else if (!fAlarmStatus)
		{//����Ǻ�բ״̬������������Զ���ϵͳ����ɾ����
			if (m_dwInitClick>0 && (GetClick()- m_dwInitClick)>=CTL_TURNCLOSE_TICK)
			{
				tInvCtrl.bCtrlType = GetInvCtrlType();
				RemoveDispItem(tInvCtrl);
				m_dwInitClick = 0;
			}
		}
	}
	m_fAlarmStatus = fAlarmStatus;
	m_fOpenStatus = fOpenStatus;
	m_iCtrlGrp = m_iGrp;
}

//========================================= CEngCtrl ===============================================
//����: ���ڳ�ʼ��ʱ,��ϵͳ����ָ������Ƶ��ִ�״̬,����״̬��ͬ�����ڴ��ж�Ӧ�ı���.
//����:@iSel	��������(0: �µ��; 1: �����).
//����: �ɹ����� true, ���򷵻� false.
bool CEngCtrl::GetSysEngStatus(int iSel)
{
	BYTE bSetStatusBuf[1+1+6*8];
	BYTE bCurStatusBuf[1+1+1+8*8];

	if (ReadItemEx(BN0, PN0, 0x104f, bSetStatusBuf) <=0)	//��"�ն˿�������״̬".
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::GetSysEngStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	if (ReadItemEx(BN0, PN0, 0x105f, bCurStatusBuf) <=0)	//��"�ն˵�ǰ����״̬".
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::GetSysEngStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	BYTE bMask;

	switch (iSel)
	{
	case 0:	//�µ��
		bMask = 0x01;
		break;
	case 1:	//�����
		bMask = 0x02;
		break;
	default:
		return false;
	}

	BYTE bGrpFlgs = bSetStatusBuf[1];

	for (int i=0; i<8; i++,bGrpFlgs>>=1)
	{
		if ((bGrpFlgs&0x01)!=0 && (bSetStatusBuf[2+6*i+3]&bMask)!=0)
		{
			m_bTurnsStatus = bCurStatusBuf[3+8*i+4];
			if ((bCurStatusBuf[3+8*i+7]&bMask) != 0)
				m_fAlrStauts = true;
			break;
		}
	}

	return true;
}

//����: ��ȡϵͳ��ָ���ܼ������������������б���״̬��־λ.
//����:@iGrp	Ҫ��ȡ���ܼ���.
//����: �������еı�־λ״̬.
BYTE CEngCtrl::GetSysEngAlrFlgs(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return 0x00;

	BYTE bBuf[1+1+1+8*8];

	if (ReadItemEx(BN0, PN0, 0x105f, bBuf) <=0)	//��"�ն˵�ǰ����״̬"ID
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::GetSysEngAlrFlgs: There is something wrong when call ReadItemEx() !\n"));
		return 0x00;
	}

	return (bBuf[1+1+1+8*(iGrp-GRP_START_PN)+7] & 0x03);	//Ŀǰֻ��0,1λ��ʹ��.
}

//����: �ı�ϵͳ��ָ���ܼ���������������ָ������״̬��־.
//����:@iGrp	Ҫ�ı���ܼ���.
//	   @bFlgs	Ҫ�ı�ı�־λ.
//	   @fStatus	Ҫ��ɵ�״̬.
//����: ��ı�ɹ����� true,���򷵻� false.
bool CEngCtrl::ChgSysEngAlrFlgs(int iGrp, BYTE bFlgs, bool fStatus)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	//int i = iGrp - GRP_START_PN;
	//BYTE bBuf[1+1+1+8*8];
	TGrpCurCtrlSta tGrpCurCtrlSta;
	memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

	bFlgs &= 0x03;	//Ŀǰֻ��0,1λ��ʹ��.
	
	if(!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::ChgSysEngAlrFlgs: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	if (fStatus)
		tGrpCurCtrlSta.bECAlarmState |= bFlgs;
	else
		tGrpCurCtrlSta.bECAlarmState &= ~bFlgs;

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::ChgSysEngAlrFlgs: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}
	return true;

	/*//!!!����ڱ���߳��л�д��ID,������Ҫ�����ź�������.
	if (ReadItemEx(BN0, PN0, 0x105f, bBuf)<=0)	//��"�ն˵�ǰ����״̬".
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::ChgSysEngAlrFlgs: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	if (fStatus)
		bBuf[1+1+1+8*i+7] |= bFlgs;
	else
		bBuf[1+1+1+8*i+7] &= ~bFlgs;

	WriteItemEx(BN0, PN0, 0x105f, bBuf);	//д"�ն˵�ǰ����״̬".
	return true;*/
}

//����: �趨ϵͳ��ָ���ܼ���ָ����������ִ�״̬.
//����:@iGrp 			Ҫ���õ��ܼ���.
//	   @bTurnsStatus	�趨���ִ�״̬.
//	   @iSel			��������(0: �µ��; 1: �����).
//����: ������óɹ����� true,���򷵻� false.
bool CEngCtrl::SetSysEngTurnsStatus(int iGrp, BYTE bTurnsStatus, int iSel)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;
	if (iSel<0 || 1<iSel)
		return false;

	BYTE bBuf[1+1+1+8*8];	//���8������.

	//!!!����ڱ���߳��л�д��ID,������Ҫ�����ź�������
	if (ReadItemEx(BN0, PN0, 0x105f, bBuf) <=0)	//��"�ն˵�ǰ����״̬".
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::ChgSysEngTurnsStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	bBuf[3+(8*(iGrp-GRP_START_PN))+4+iSel] = bTurnsStatus & CTL_TURN_MASK;

	WriteItemEx(BN0, PN0, 0x105f, bBuf);	//д"�ն˵�ǰ����״̬".

	return true;
}

//����: �ı�ϵͳ��ָ���ܼ���ָ�����������Ӧ�ִ�״̬.
//����:@iGrp 		Ҫ���õ��ܼ���.
//	   @bTurns 		��Ҫ���õ��ִ�.
//	   @fStatus		true: ��λ��Ӧλ; false: �����Ӧλ.
//	   @iSel		��������(0: �µ��; 1: �����).
//����: ������óɹ����� true,���򷵻� false.
bool CEngCtrl::ChgSysEngTurnsStatus(int iGrp, BYTE bTurns, bool fStatus, int iSel)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;
	if (iSel<0 || 1<iSel)
		return false;

	BYTE bBuf[1+1+1+8*8];	//���8������.

	//!!!����ڱ���߳��л�д��ID,������Ҫ�����ź�������
	if (ReadItemEx(BN0, PN0, 0x105f, bBuf) <=0)	//��"�ն˵�ǰ����״̬".
	{
		DTRACE(DB_LOADCTRL, ("CEngCtrl::ChgSysEngTurnsStatus: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}
	if (fStatus)
		bBuf[3+(8*(iGrp-GRP_START_PN))+4+iSel] |= (bTurns & CTL_TURN_MASK);
	else
		bBuf[3+(8*(iGrp-GRP_START_PN))+4+iSel] &= ~(bTurns & CTL_TURN_MASK);

	WriteItemEx(BN0, PN0, 0x105f, bBuf);	//д"�ն˵�ǰ����״̬".

	return true;
}

//����: ��ȡָ���ִεĵ����բ���
//����:@iTurn 	�ִκ�.
//����: ����ִ���Ч,������Ӧ�ı�������ʱ��,���򷵻� DWORD �����ֵ.
DWORD CEngCtrl::GetEngTurnInv(int iTurn)
{
	if (iTurn<TURN_START_PN || iTurn>TURN_START_PN+TURN_NUM)
		return ((DWORD)-1);

	BYTE bBuf[10] = {0};
	BYTE *ptr = bBuf+2;
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;

	if (OoReadAttr(0x8102, 0x02, bBuf, &pbFmt, &wFmtLen) < 0)
	{
		DTRACE(DB_LOADCTRL, ("CPwrCtrl::GetPwrAlrPersistTime: There is something wrong when call OoReadAttr() !\n"));
		return ((DWORD)-1);
	}

	DWORD dwTime = (DWORD)ptr[iTurn-1] * 60;

	return dwTime;


	/*BYTE b;

	if (ReadItemEx(BN0, (WORD)iTurn, 0x0a06, &b) <=0)	//��ȡָ���ִ�"���ظ澯ʱ��".
	{
		DTRACE(DB_LOADCTRL, ("CPwrCtrl::GetPwrAlrPersistTime: There is something wrong when call ReadItemEx() !\n"));
		return ((DWORD)-1);
	}

//	if (b == 0)
//		b = 1;
	DWORD dwTime = (DWORD)b * 60;

//	if (dwTime < CTL_POWER_ALR_MIN_TIME)	//ʱ�����С����С�ӳ�ʱ��,����С�ӳ�ʱ��Ϊ׼.
//		dwTime = CTL_POWER_ALR_MIN_TIME;

	return dwTime;*/
}

//========================================= CPwrCtrl ===============================================
CPwrCtrl::CPwrCtrl(void)
{	
	m_dwFrzDly = 60 * 2;	//������բ���ʶ�����ʱ.������ʱ�¸���,�������ض�����ʱ2����.
	m_iPwrLimit = 0;		//��ǰ�Ĺ��ʶ�ֵ,������.
	m_dwAlrTime = 0;		//����������ʱ��.
	m_dwGuaranteeAlrTime = 0;

	m_iCurPwrLimit = 0;		//��ǰ���ʶ�ֵ.

	memset(&m_OLStat, 0, sizeof(m_OLStat));			//����ͳ��.

	m_dwPwrStartClick = 0;
}

//����: ���湦����բ��¼.
//����:@iSel	��������(0:ʱ�ο�; 1:���ݿ�; 2:Ӫҵ��ͣ��; 3:��ʱ�¸���).
void CPwrCtrl::DoSavePwrCtrlOpenRec(int iSel)
{
	WORD wID;
	BYTE bPwrCtrlType;

	switch (iSel)
	{
	case 0:	//ʱ�ο�
		wID			 = 0x0a04;
		bPwrCtrlType = 0x01;
		break;
	case 1:	//���ݿ�
		wID			 = 0x0a03;
		bPwrCtrlType = 0x02;
		break;
	case 2:	//Ӫҵ��ͣ��
		wID			 = 0x0a02;
		bPwrCtrlType = 0x04;
		break;
	case 3:	//��ʱ�¸���
		wID			 = 0x0a01;
		bPwrCtrlType = 0x08;
		break;
	default:
		return;
	}
	for (int i=TURN_START_PN; i<TURN_START_PN+TURN_NUM; i++)
	{
		BYTE bRecBuf[32];
		DWORD dwTime;

		if (ReadItemEx(BN0, (WORD)i, wID, bRecBuf) <=0)
		{
			DTRACE(DB_LOADCTRL, ("CPwrCtrl::DoSaveOpenRec: There is something wrong when call ReadItemEx() !\n"));
			return;
		}
		memcpy(&dwTime, bRecBuf, 4);
		if (dwTime > m_dwNow)	//���δ����ʱ����,ʱ����ǰ����ȥ��
		{	
			dwTime = m_dwNow;
			memcpy(bRecBuf, &dwTime, 4);
			WriteItemEx(BN0, (WORD)i, wID, bRecBuf);	//�����ݿ��ʱ��Ҳ����
		}

		if (dwTime != 0)
		{//������բ�������,��������բ��2���Ӽ�¼���Ṧ��.
			if (m_dwNow >= dwTime+m_dwFrzDly)
			{
				BYTE bBuf[32];
				int64 iCurPwr;
				TTime tm;

				SecondsToTime(dwTime, &tm);
				bBuf[0] = bRecBuf[4];						//�ܼ���
				bBuf[1] = 8;								//��բ�ִθ�ʽbitstring,����1���ֽ�
				bBuf[2] = (BYTE)(0x01<<(i-TURN_START_PN));	//�ִ�
				bBuf[3] = 8;								//��������ʽbitstring,����1���ֽ�
				bBuf[4] = bPwrCtrlType;						//�������

				memcpy(bBuf+5, bRecBuf+5, 4);				//��բǰ����
				iCurPwr = GetCurPwr(bRecBuf[4]);			//��õ�ǰ����
				if (bPwrCtrlType == 0x08)	//��ʱ�¸���
				{
					WriteItemVal64(BN0, (WORD)m_iGrp, 0x111f, &iCurPwr);
				}

				Val32ToBin(iCurPwr, bBuf+9, 4);		
				memcpy(bBuf+13, bRecBuf+9, 4);				//��բʱ���ʶ�ֵ

				memset(bRecBuf, 0, sizeof(bRecBuf));
				WriteItemEx(BN0, (WORD)i, wID, bRecBuf);		//�����բ�м�����.
				TrigerSaveBank(BN0, SECT_CTRL, 0);

				//��¼��ǰ��բ��¼��ϵͳ����.
				//SaveAlrData(ERC_PWRCTL, tm, bBuf, 17);

				char cTime[20];
				char cTime1[20];

				DTRACE(DB_LOADCTRL, ("CPwrCtrl::DoSaveOpenRec: In the %d seconds after open break of Turn[%d], recorded the power.\n"\
									 "Time of open break is %s, power is %lld\n"\
									 "at %s, power is %lld,bPwrCtrlType=%d.\n", m_dwFrzDly, i,
									 TimeToStr(tm, cTime1), Fmt2ToVal64(bBuf+3, 2),
									 TimeToStr(m_tmNow, cTime), iCurPwr,bPwrCtrlType));
			}
		}
	}
}

//����: ��ȡ���ر�����ֵ.
//����: ���ر�����ֵ.
int64 CPwrCtrl::GetPwrSafeLimit(void)
{
	BYTE bBuf[10];

	if (ReadItemEx(BN0, PN0, 0x8100, bBuf) <=0)	//��ȡ"�ն˱�����ֵ".
	{
		DTRACE(DB_LOADCTRL, ("CPwrCtrl::GetPwrSafeLimit: There is something wrong when call ReadItemEx() !\n"));
		return 0;
	}

	return OoLong64ToInt64(&bBuf[1]);
}

//����: ��ȡָ���ִεĹ��ر�������ʱ��.
//����:@iTurn 	�ִκ�.
//����: ����ִ���Ч,������Ӧ�ı�������ʱ��,���򷵻� DWORD �����ֵ.
DWORD CPwrCtrl::GetPwrAlrPersistTime(int iTurn)
{
	if (iTurn<TURN_START_PN || iTurn>TURN_START_PN+TURN_NUM)
		iTurn = TURN_NUM;

	BYTE bBuf[20];

	if (ReadItemEx(BN0, PN0, 0x8102, bBuf) <=0)	//��ȡָ���ִ�"���ظ澯ʱ��".
	{
		DTRACE(DB_LOADCTRL, ("CPwrCtrl::GetPwrAlrPersistTime: There is something wrong when call ReadItemEx() !\n"));
		return ((DWORD)-1);
	}

	DWORD dwTime = (DWORD)bBuf[1+iTurn*2] * 60;

	if (dwTime < CTL_POWER_ALR_MIN_TIME)	//ʱ�����С����С�ӳ�ʱ��,����С�ӳ�ʱ��Ϊ׼.
		dwTime = CTL_POWER_ALR_MIN_TIME;

	return dwTime;
}

//����: ��ȡָ���ִεĹ��ع��ʻ���ʱ��,��λ��
//����:@iTurn 	�ִκ�.
//����: ���ع��ʻ���ʱ��,��λ��
DWORD CPwrCtrl::GetPwrSlideInterv(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return 0;

	BYTE bBuf[2] = {0};

	//if (ReadItemEx(BN0, (WORD)iGrp, 0x02bf, &b) <=0)	//F43
	if (ReadItemEx(BN0, (WORD)iGrp, 0x230c, bBuf) <= 0)
		return 0;

	if (bBuf[1] > 60)
		bBuf[1] = 60;

	return (DWORD)bBuf[1] * 60;
}

//����: �趨ָ���ܼ��鵱ǰ���ض�ֵ.
//����:@iGrp		Ҫ�趨���ܼ���
//     @iPwrLimit	�趨�Ĺ��ض�ֵ
bool CPwrCtrl::SetSysCurPwrLimit (int iGrp, int64 iPwrLimit)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	TGrpCurCtrlSta tGrpCurCtrlSta;
	memset(&tGrpCurCtrlSta, 0, sizeof(TGrpCurCtrlSta));

	if (!GetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//��"�ն˵�ǰ����״̬".
	{
		DTRACE(DB_LOADCTRL, ("CPwrCtrl::SetSysCurPwrLimit: There is something wrong when call GetGrpCurCtrlSta() !\n"));
		return false;
	}

	tGrpCurCtrlSta.CurPwrVal = iPwrLimit;

	if (!SetGrpCurCtrlSta(iGrp, &tGrpCurCtrlSta))//д"�ն˵�ǰ����״̬".
	{
		DTRACE(DB_LOADCTRL, ("CPwrCtrl::SetSysCurPwrLimit: There is something wrong when call SetGrpCurCtrlSta() !\n"));
		return false;
	}

	return true;
}

//����: �ۼӹ��ʶ�ֵ���޲���(����ʱ�估���޵���).
//����:@dwTime		�ۼ�ʱ�������
//     @iEng		�ۼ��µ���������
void CPwrCtrl::SumOverLimitPara(int& riGrp, DWORD& rdwTime, int64& riEng)
{
	int64 iTmpEnergy;
	if (m_OLStat.fIfOverLimit)
	{//����֮ǰ��״̬�ǳ���״̬.
		DWORD dwClick = GetClick();

		if (!m_fIfOverLimit	|| m_OLStat.iGrp!=m_iGrp || (dwClick - m_OLStat.dwClick)>60)
		{//���糬��״̬����ӻ��ܼ��鷢���˱仯,�����ۼ�ͳ������.
			rdwTime = dwClick - m_OLStat.dwClick;				//ͳ�Ƴ���ʱ��.
			iTmpEnergy = GetCurEng(m_OLStat.iGrp);
			riEng = iTmpEnergy - m_OLStat.iEng;	//ͳ�Ƴ����µ���.
			riGrp = m_OLStat.iGrp;

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
				m_OLStat.iEng = iTmpEnergy;
				m_OLStat.fIfOverLimit = true;
				m_OLStat.iGrp = m_iGrp;
				m_OLStat.dwClick = GetClick();
			}
			else
				m_OLStat.fIfOverLimit = false;
			  
		}
	}
}

//����:�Ƿ��բ
//����: ������óɹ��򷵻� true, ���򷵻� false.
bool CPwrCtrl::RestoreTurnStatus()
{
	BYTE bEnableClose = 0;
	DWORD dwPersistTime;
	char msg[100];
	int i;

	//ReadItemEx(BN0, PN0,0x0a08, &bEnableClose);//�Ƿ������բ
	//if (bEnableClose) 
	{
		i = GetIdxOfMostLeft1(m_bTurnsStatus);	//��ȡ��Ӧ�ܼ��鵱ǰ����բ���ִκ�.		
		dwPersistTime = GetPwrAlrPersistTime(i);	//��ȡ��Ӧ�ִεĹ��ر�������ʱ��.	

		if (m_dwNow > m_dwAlrTime+dwPersistTime)
		{
			m_bTurnsStatus &= ~(1<<(i-1));
			m_dwAlrTime = 0;
		}
	}
//	DTRACE(DB_LOADCTRL, ("CPwrCtrl::RestoreTurnStatus: i=%d, dwPersistTime=%d,m_bTurnsStatus=%d!\n", i, dwPersistTime,m_bTurnsStatus));
	return true;
	
}

//���������ɹ���Խ��ʱ����ʾ״̬��
//������@dwStime  ������բ�澯��ʼʱ�䣻
//@wDelayTime ������բ�澯��ʱʱ�䣻
//@iCurPwr      ���ص�ǰ����ֵ��
void CPwrCtrl::SaveDisp(WORD wDelayTime, DWORD dwStime, int64 iCurPwr)
{
	BYTE bBuf[25];
	
	bBuf[0] = GetCtrlType();
	bBuf[1] = m_bWarnStatus;
	wDelayTime = wDelayTime;
	memcpy(bBuf+2,&wDelayTime,2);
	Val64ToFmt(iCurPwr, bBuf+4, FMT2, 2);
	Val64ToFmt(m_iCurPwrLimit, bBuf+6, FMT2, 2);
	memcpy(bBuf+8,&dwStime,4);
	WriteItemEx(BN0, PN0, 0x0920, bBuf); //���ظ澯��Ϣ����

	DTRACE(DB_LOADCTRL, ("CPwrCtrl::SavePwrCtrl: m_iCurPwrLimit=%lld, iCurPwr=%lld,CtrlType=%d!\n", m_iCurPwrLimit, iCurPwr,bBuf[0]));
}


