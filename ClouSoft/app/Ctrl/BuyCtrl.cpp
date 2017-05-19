/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�BuyCtrl.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��CBuyCtrl����
 * ��ǰ�汾��1.0
 * ��    �ߣ��Ž���
 * ������ڣ�2008��3��
 * ��ע����ȡʣ���������1������F23,���ݿ�û�г�ʼ�������û��Ͷ�빺�絥�ţ����ȡֵΪ0,᯹�˵��ȡΪ��Чֵ����֪��Ҫ��Ҫ����19th,June����
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
#include "TaskManager.h"
#include "BuyCtrl.h"
#include "DpGrp.h"
#include "DbOIAPI.h"

//========================================== CBuyCtrl ==============================================
CBuyCtrl::CBuyCtrl(void)
{
	for (int i=0; i<GRP_NUM; i++)
	{
		m_iCurBuyRemainEng[i]	= 0;	//��ʼ��Ϊ��ֵ.
		m_iBaseEng[i]			= 0;	//��ʼ��Ϊ��ֵ.
		m_dwBillIdx[i]			= 0;	//��ʼ��Ϊ��ֵ.
		m_fUpBaseEng[i]			= false;
	}

	m_fAlrStauts = false;		//������״̬ȡ��.
	m_fEnergyFeeFlag = false;
	m_iCurFeeRatio = 0;
}

//����: ��ʼ��.
//����: �����ʼ���������� true,���򷵻� false.
bool CBuyCtrl::Init(void)
{
	ClrCmd();		//����ڴ��б�����ƵĿ�������.
	RstCtrl();		//��λ�ڴ��б������״̬��.
	SetValidStatus(false);	//�趨�����˳�״̬.

	TBuyCtrlPara BuyCtrlPara;

	for (WORD i=1; i<=GRP_NUM; i++) //�ϵ�ʱ��ʼ��
	{
		if (IsGrpValid(i) && GetBuyCtrlPara(i, BuyCtrlPara))
		{
			m_dwBillIdx[i] = BuyCtrlPara.dwBillIdx;
			ReadItemVal64(BN0, i, 0x0a05, &m_iCurBuyRemainEng[i]); // �ն˵�ǰʣ��������ѣ�
			ReadItemVal64(BN0, i, 0x087f, &m_iBaseEng[i]);			//�ܼ��� �����Ͷ��ʱ��һ���ӵ�����
		}
	}

	return true; //return GetSysStatus();
}

//����: '�����'����.
//����: �����򷵻� true,���򷵻� false.
bool CBuyCtrl::DoCtrl(void)
{
	BYTE bBuf1[100],bBuf2[100];
	DoCmdScan();		//ɨ��ϵͳ���е�����.
	UpdateBuyRemainEng();
	if (!IsValid())
	{
		RstCtrl();
		return true;
	}
	UpdateSysRemainEng();		//���������ܼ���ʣ�����.
/*	if (IsGuarantee())	//����Ƿ��ڱ���״̬.
	{
		RstCtrl();					//��λ�ڴ��б�����Ƶ��������״̬.
		return true;
	}
*/
	int i;
	BYTE b = GetSysCtrlTurnsCfg(m_iGrp); //ȡ����ִ��趨

	if ((m_bTurnsStatus&~b) != 0)
	{//����ܿ��ִη�����բ��,Ӧ����Щ�ִν��и�λ(��λ�����բ״̬).
		m_bTurnsStatus &= b;
	}
	//����Ƿ��п���բ.
	i = GetIdxOfMostRight1(b & ~GetTurnsStatus());	//��ȡ��Ӧ�ܼ��鵱ǰ����բ���ִκ�.
	m_bWarnStatus = i+1;
	if (!GetBuyCtrlPara(m_iGrp, m_BuyCtrlPara))
	{
		DTRACE(DB_LOADCTRL, ("CBuyCtrl::DoCtrl: There is somethig wrong when get BuyCtrl parameter of Group[%d] !\n", m_iGrp));
		return false;	//��ȡ����ز�������.
	}

	char cTime[20];

	m_iBuyRemain = m_iCurBuyRemainEng[m_iGrp];
	
	if (m_iBuyRemain > m_BuyCtrlPara.iAlarmLimit)
	{//ʣ�����û��������.
		RstCtrl();		//��λ�ڴ��б�����Ƶ��������״̬.
	}
	else if (m_iBuyRemain > m_BuyCtrlPara.iActLimit)
	{//ʣ������ѵ�������,����û����բ��.

		SubRstCtrl();				//��λ�ڴ��б�����ƵĲ������״̬.
		if (i >= 0)
		{//���п���բ.
			if (!m_fAlrStauts)
			{
				SetSysCtrlAlr(m_iGrp, true); //�趨��ظ澯״̬�Ĺ����Խ��
#ifdef PRO_698
				//����澯��¼.
				/*BYTE bBuf[20];

				bBuf[0] = (BYTE)m_iGrp;					//�ܼ���
				bBuf[1] = GetSysCtrlTurnsCfg(m_iGrp);		//�ִ�
				bBuf[2] = 0x02;							//������
				Val64ToFmt3(m_iBuyRemain, bBuf+3, 4);//�澯ʱʣ�������
				Val64ToFmt3(m_BuyCtrlPara.iActLimit, bBuf+7, 4);//�������բ���ޣ�*/
				//��¼��ǰ��բ��¼��ϵͳ����.
				//SaveAlrData(ERC_ENGALARM, m_tmNow, bBuf);
#endif
				DTRACE(DB_LOADCTRL, ("CBuyCtrl::DoCtrl: Grp%d alarm start at %s, remain-energy is %lld, alarm-limit is %lld, act-limit is %lld\n",
									 m_iGrp, TimeToStr(m_tmNow, cTime), m_iBuyRemain, m_BuyCtrlPara.iAlarmLimit, m_BuyCtrlPara.iActLimit));
			}
			m_fAlrStauts = true;
			//SaveDisp();
		}
		else
		{//��բ����.
			if (m_fAlrStauts)
				SetSysCtrlAlr(m_iGrp, false); //���ظ澯״̬�Ĺ����Խ��
			m_fAlrStauts = false;
		}
	}
	else if (IsGuarantee())	//����Ƿ��ڱ���״̬.
	{
		RstCtrl();					//��λ�ڴ��б�����Ƶ��������״̬.
		m_fIfOverLimit = true;
		return true;
	}
	else	//(m_iBuyRemain <= m_BuyCtrlPara.iActLimit)
	{
		m_fIfOverLimit = true;

		if (i < 0)									//����Ƿ��п���բ.
		{
			if (m_fAlrStauts)
				SetSysCtrlAlr(m_iGrp, false); //���ظ澯״̬�Ĺ����Խ��
			m_fAlrStauts = false; //��բ������,����û������,��ֹ����.
			return true;
		}

		if (!m_fAlrStauts)
			SetSysCtrlAlr(m_iGrp, true);
		m_fAlrStauts = true;
		//SaveDisp();

		DWORD dwTurnInv = GetEngTurnInv(i+TURN_START_PN);	//��ȡ��Ӧ�ִεĹ��ر�������ʱ��.
		if (m_dwOpenTurnTime > m_dwNow)	//ʱ����ǰ����ȥ��
			m_dwOpenTurnTime = 0;

		if (m_dwOpenTurnTime == 0)					//��������û����բ,��ֱ������С�ִε�բ,ͬʱ��¼�±�����բʱ��.
			m_dwOpenTurnTime = m_dwNow;
		else if (m_dwNow < m_dwOpenTurnTime+dwTurnInv)		//�ϴ���բ��,�����60�����������һ��բ
			return true;
		else
			m_dwOpenTurnTime = m_dwNow;				//�����ϴ���բ�ѳ���60��,�����ٴ���բ,ͬʱ��¼�±�����բʱ��.

		m_bTurnsStatus |= 0x01 << i;
		//SaveDisp();
		//SetSysTurnsStatus(m_iGrp, m_bTurnsStatus);	//����Щբ������բ,(ʵ��������ÿ��ֻ��1��,��˶�ԭ����״̬����˵,ֻ��ı�1λ)
		m_wOpenTimes++;								//��բ��������1.

		//������բ��¼.
		/*BYTE bBuf[1+1+1+4+4];

		bBuf[0] = (BYTE)m_iGrp;					//�ܼ���
		bBuf[1] = (BYTE)(0x01<<i);				//�ִ�
		bBuf[2] = 0x02;							//������
		Val64ToFmt3(m_iBuyRemain, bBuf+3, 4);//��բʱʣ�������
		Val64ToFmt3(m_BuyCtrlPara.iActLimit, bBuf+7, 4);//�������բ���ޣ�*/
		//��¼��ǰ��բ��¼��ϵͳ����.
		//SaveAlrData(ERC_ENGCTL, m_tmNow, bBuf);

		DTRACE(DB_LOADCTRL, ("CBuyCtrl::DoCtrl: Turn%d of Grp%d open at %s, remain-energy is %lld, act-limit is %lld\n",
							 i+GRP_START_PN, m_iGrp, TimeToStr(m_tmNow, cTime), m_iBuyRemain, m_BuyCtrlPara.iActLimit));
		//***���������ź�;
	}

	return true;
}

//���������ɹ����Խ��ʱ����ʾ������
void CBuyCtrl::SaveDisp()
{
	BYTE bBuf[13];
	bBuf[0] = m_bTurnsStatus;
	Val64ToFmt(m_iBuyRemain, bBuf+1, FMT3, 4);
	Val64ToFmt(m_BuyCtrlPara.iAlarmLimit, bBuf+5, FMT3, 4);
	Val64ToFmt(m_BuyCtrlPara.iActLimit, bBuf+9, FMT3, 4);
	WriteItemEx(BN0, PN0, 0x0940, bBuf); //����ظ澯��Ϣ����
}

void CBuyCtrl::SubRstCtrl(void)
{
	m_bTurnsStatus	 = 0x00;			//���ִ�״̬ȫ����Ϊ��բ.
	m_dwOpenTurnTime = 0;				//�ϴ���բʱ����Ϊ0;
	m_fIfOverLimit	 = false;
}

//����: ��λ�ڴ��б������״̬��.
void CBuyCtrl::RstCtrl(void)
{
	
	SetSysCtrlAlr(m_iGrp, false); //��C1F6'�ն˵�ǰ����״̬'�е�ظ澯״̬�Ĺ����Խ��
	m_fAlrStauts = false;		//������״̬ȡ��.
	SubRstCtrl();
}

//����: ��ȡĳ�ܼ���ı����������,��������ŵ� m_NewCmd��.(ע��: �Բ�ͬ����,m_NewCmd�Ľṹ�ǲ�ͬ��)
//����:@iGrp	Ҫ��ȡ������ܼ���.
//����: �����ȡ�ɹ���Ϊ��Ч���� true,���򷵻� false.
bool CBuyCtrl::GetSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[1];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8273, bCmd, &m_NewCmd.dwTime) != sizeof(bCmd)) //��ȡ��Ӧ�ܼ����"�����Ͷ������".
	{
		DTRACE(DB_LOADCTRL, ("CBuyCtrl::GetSysCmd: There is something wrong when call ReadItemEx() !\n"));
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
			WriteItemEx(BN0, (WORD)iGrp, 0x8273, bCmd, m_NewCmd.dwTime);	//����Ӧ�ܼ����"ʱ�ο�����"д�����ݿ�
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
bool CBuyCtrl::ClrSysCmd(int iGrp)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bCmd[10];
	m_fUpBaseEng[m_iGrp] = false;
	m_iBaseEng[m_iGrp]=0;
	WriteItemVal64(BN0, m_iGrp, 0x087f, &m_iBaseEng[m_iGrp]); //�����ʼ�ĵ���

	memset( bCmd, 0, sizeof(bCmd) );
	WriteItemEx(BN0, (WORD)iGrp, 0x8273, bCmd);	//д��Ӧ�ִε�"�����Ͷ��Ͷ������"ID

	TrigerSaveBank(BN0, SECT_CTRL, 0); //��������.

	return true;
}

//����: ����ϵͳ�⵱ǰ�ܼ���ʣ�๺����(ʣ����� = ʣ�������ʼֵ - (��ǰ���� - ��ǰ������ʼֵ)).
//����:@iGrp		Ҫ��ȡ���ܼ���.
//����: ��ǰ��ʣ�����.
void CBuyCtrl::UpdateSysRemainEng()
{
    bool fTrigerSave = false;
    int64 iEng = 0;
	BYTE bTmpBuf[9] = {0};
	BYTE bBuf[50] = {0};
	if (IsGrpValid(m_iGrp) )
	{
		if (m_iBaseEng[m_iGrp] == 0) //�ϵ���״δ�ϵͳ���ж�ȡ��
		{
			//ReadItemVal64(BN0, m_iGrp, 0x0a05, &m_iCurBuyRemainEng[m_iGrp]); // �ն˵�ǰʣ��������ѣ�
			ReadItemEx(BN0, m_iGrp, 0x230a, bTmpBuf);	// �ն˵�ǰʣ��������ѣ�
			m_iCurBuyRemainEng[m_iGrp] = OoLong64ToInt64(bTmpBuf+1);

			iEng = GetGroupEng(m_iGrp); //������ǰ��ȡ�ܼ���ʾֵ��ȷ���ܼ���ʾֵ��Чʱ�ȸ��¸���ʼֵm_iBaseEng
			if (iEng>=0 && (!m_fUpBaseEng[m_iGrp]))
			{
				m_iBaseEng[m_iGrp] = iEng; //������ʼֵ
				m_fUpBaseEng[m_iGrp] = true;
			}
		}
		if (IsCtrlGrpParaChg()) //�ܼ�����������־����ֹͣ��ǰ����
		{			
			//if (GetInfo(INFO_GRP_CHG_OVER)) //�ܼ���������
			{
				if (m_iBaseEng[m_iGrp] !=0)
				{
					m_iBaseEng[m_iGrp] = 0;
					m_fUpBaseEng[m_iGrp] = false;
					ReadItemEx(BN0, m_iGrp, 0x2308, bBuf);
					OoInt64ToLong64(m_iBaseEng[m_iGrp], bBuf+3);
					WriteItemEx(BN0, m_iGrp, 0x2308, bBuf);//�����ʼ�ĵ���
					bTmpBuf[0] = DT_LONG64;
					OoInt64ToLong64(m_iCurBuyRemainEng[m_iGrp], bTmpBuf+1);
					WriteItemEx(BN0, m_iGrp, 0x230a, bTmpBuf);//�ܼ��� �ն˵�ǰʣ��������ѣ�

					/*WriteItemVal64(BN0, m_iGrp, 0x087f, &m_iBaseEng[m_iGrp]); //�����ʼ�ĵ���
					WriteItemVal64(BN0, m_iGrp, 0x110f, &m_iCurBuyRemainEng[m_iGrp]); //C1F23 �ܼ��� �ն˵�ǰʣ��������ѣ�
					WriteItemVal64(BN0, m_iGrp, 0x0a05, &m_iCurBuyRemainEng[m_iGrp]); // �ܼ��� �ն˵�ǰʣ��������ѣ�*/				
					TrigerSaveBank(BN0, SECT_CTRL, 0);	//��������,��ֹ���綪ʧ���絥.
				}
				SetCtrlGrpParaChg(false); //��������ܼ�������־
				DTRACE(DB_LOADCTRL, ("CBuyCtrl::UpdateSysRemainEng: m_iBaseEng=%lld,iEng=%lld. !\r\n", m_iBaseEng[m_iGrp],iEng));	
				DTRACE(DB_LOADCTRL, ("CBuyCtrl::UpdateSysRemainEng: m_iCurBuyRemainEng[m_iGrp]=%lld. !\r\n", m_iCurBuyRemainEng[m_iGrp]));	
			}
			return;
		}
		if (m_tmNow.nMinute != m_tmOldTime.nMinute)	//����ʱ,��,��,��ļ��,���Ч��,�Ҳ����������.
		{	//�������µĹ��絥��ʱ�ӷ��ӷ������л�,�������ʣ�����.

			iEng = GetGroupEng(m_iGrp);
			if (iEng>0 && iEng>m_iBaseEng[m_iGrp] && m_iBaseEng[m_iGrp]>=0 && IsValid()) //chenxi,�������������ܼ������ʱ�ķ�����
			{
				if (m_fEnergyFeeFlag)
				{
					int64 iTmpEng = (iEng - m_iBaseEng[m_iGrp]) * m_iCurFeeRatio;
					m_iCurBuyRemainEng[m_iGrp] -= iTmpEng;
				}
				else
					m_iCurBuyRemainEng[m_iGrp] -= (iEng - m_iBaseEng[m_iGrp]);
			}

			if (iEng < m_iBaseEng[m_iGrp]) //���ֵ�����С�������
			{
				m_bCount++;
				DTRACE(DB_LOADCTRL, ("CBuyCtrl::Eng ERR: m_iBaseEng=%lld,iEng=%lld. !\r\n", m_iBaseEng[m_iGrp],iEng));	
				if (m_bCount < 3 ) return;
			}
			m_bCount = 0;
			//����ϵͳ���е�ʣ�����.
			//WriteItemVal64(BN0, m_iGrp, 0x110f, &m_iCurBuyRemainEng[m_iGrp]); //C1F23 �ܼ��� �ն˵�ǰʣ��������ѣ�
			//WriteItemVal64(BN0, m_iGrp, 0x0a05, &m_iCurBuyRemainEng[m_iGrp]); // �ܼ��� �ն˵�ǰʣ��������ѣ�
			bTmpBuf[0] = DT_LONG64;
			OoInt64ToLong64(m_iCurBuyRemainEng[m_iGrp], bTmpBuf+1);
			WriteItemEx(BN0, m_iGrp, 0x230a, bTmpBuf);	// �ܼ��� �ն˵�ǰʣ��������ѣ�

			if (iEng > 0 && IsValid())
			{
				m_iBaseEng[m_iGrp] = iEng; //������һ�����ܼ��������
				ReadItemEx(BN0, m_iGrp, 0x2308, bBuf);
				OoInt64ToLong64(m_iBaseEng[m_iGrp], &bBuf[3]);
				WriteItemEx(BN0, m_iGrp, 0x2308, bBuf);				//������һ�����ܼ������,��ֹ���磻
				//WriteItemVal64(BN0, m_iGrp, 0x087f, &m_iBaseEng[m_iGrp]); //������һ�����ܼ������,��ֹ���磻
				TTime tmNow;
				GetCurTime(&tmNow);
				
				if (tmNow.nSecond == 0)//ÿ���ӱ���һ��ʣ�����
				{
					fTrigerSave = true;
				}
			}
		}
	}
		
	if (fTrigerSave)
		TrigerSaveBank(BN0, SECT_CTRL, 0);	//��������,��ֹ���綪ʧ���絥.
}
//����: ����ϵͳ�⵱ǰ�ܼ���ʣ�๺����(ʣ����� = ʣ�������ʼֵ - (��ǰ���� - ��ǰ������ʼֵ)).
//����:@iGrp		Ҫ��ȡ���ܼ���.
//����: ��ǰ��ʣ�����.
void CBuyCtrl::UpdateBuyRemainEng()
{
    bool fTrigerSave = false;
	TBuyCtrlPara BuyCtrlPara;
	BYTE bTmpBuf[9] = {0};
	for (WORD i=1; i<=GRP_NUM; i++)
	{
		if (IsGrpValid(i) && GetBuyCtrlPara(i, BuyCtrlPara))
		{
			if (m_iBaseEng[i] == 0) //�ϵ���״δ�ϵͳ���ж�ȡ��
			{
				//ReadItemVal64(BN0, i, 0x0a05, &m_iCurBuyRemainEng[i]); // �ն˵�ǰʣ��������ѣ�
				memset(bTmpBuf, 0, sizeof(bTmpBuf));
				ReadItemEx(BN0, i, 0x230a, bTmpBuf);
				m_iCurBuyRemainEng[i] = OoLong64ToInt64(&bTmpBuf[1]);
			}

			if (BuyCtrlPara.dwBillIdx!=m_dwBillIdx[i])	
			{	//�������µĹ��絥��ʱ�ӷ��ӷ������л�,�������ʣ�����.
				int64 iEng = GetGroupEng(i);
				if (BuyCtrlPara.dwBillIdx != m_dwBillIdx[i])
				{//���µ���Ч���絥.
					//WriteItemVal64(BN0, i, 0x082f, &m_iCurBuyRemainEng[i]);	//���汾�ι���ǰʣ�����.

					//���湺��������ü�¼�¼�(Erc19)
					/*BYTE bBuf[26];
					bBuf[0] = (i) & 0x3f;                      //�����ܼ���ţ�
					memcpy(bBuf+1, &BuyCtrlPara.dwBillIdx, 4);              //���絥�ţ�
					bBuf[5] = BuyCtrlPara.bFlag;						    //׷��/ˢ�±�־��
					Val64ToFmt(BuyCtrlPara.iBuyEng, bBuf+6, FMT3, 4);		//������ֵ��
					Val64ToFmt(BuyCtrlPara.iAlarmLimit, bBuf+10, FMT3, 4);  //�������ޣ�
					Val64ToFmt(BuyCtrlPara.iActLimit, bBuf+14, FMT3, 4);    //��բ���ޣ�
					Val64ToFmt(m_iCurBuyRemainEng[i], bBuf+18, FMT3, 4);    //���ι���ǰʣ�������*/

					if (BuyCtrlPara.bFlag == 0x55)//׷�ӹ���
					{
						if (m_iBaseEng[i]>0 && iEng>0 && iEng>=m_iBaseEng[i] && IsValid())
							m_iCurBuyRemainEng[i] -= (iEng - m_iBaseEng[i]);
						m_iCurBuyRemainEng[i] += BuyCtrlPara.iBuyEng;
						DTRACE(DB_LOADCTRL, ("CBuyCtrl::UpdateSysBuyRemainEng: rx a buy energy bill that increase energy %lld, current remain energy = %lld !\n", 
											 BuyCtrlPara.iBuyEng, m_iCurBuyRemainEng[i]));
					}
					else //(BuyCtrlPara.bFlag == 0xaa)//ˢ�¹���
					{
						m_iCurBuyRemainEng[i] = BuyCtrlPara.iBuyEng;
						DTRACE(DB_LOADCTRL, ("CBuyCtrl::UpdateSysBuyRemainEng: rx a buy energy bill that change energy to %lld, current remain energy = %lld !\n", 
											 BuyCtrlPara.iBuyEng, m_iCurBuyRemainEng[i]));
					}

					//Val64ToFmt(m_iCurBuyRemainEng[i], bBuf+22, FMT3, 4);   //���ι����ʣ�������
					//SaveAlrData(ERC_BUYPARA, m_tmNow, bBuf);

					//WriteItemVal64(BN0, i, 0x083f, &m_iCurBuyRemainEng[i]); //���湺����ʣ�����.
					m_dwBillIdx[i] = BuyCtrlPara.dwBillIdx; //���¹��絥�ţ�
				}				
				//����ϵͳ���е�ʣ�����.
				//WriteItemVal64(BN0, i, 0x110f, &m_iCurBuyRemainEng[i]); //C1F23 �ܼ��� �ն˵�ǰʣ��������ѣ�
				//WriteItemVal64(BN0, i, 0x0a05, &m_iCurBuyRemainEng[i]); // �ܼ��� �ն˵�ǰʣ��������ѣ�	
				if (bTmpBuf[0] != DT_LONG64)
					bTmpBuf[0] = DT_LONG64;
				OoInt64ToLong64(m_iCurBuyRemainEng[i], &bTmpBuf[1]);
				WriteItemEx(BN0, i, 0x230a, bTmpBuf); // �ܼ��� �ն˵�ǰʣ��������ѣ�
				fTrigerSave = true;
			}
			//m_fEnergyFeeFlag = GetCurFeeRatio(&m_iCurFeeRatio);		//�������Э����δҪ��ѿأ���ʱע�͵�	--QLS 17.01.16
		}
	}
	
	if (fTrigerSave)
		TrigerSaveBank(BN0, SECT_CTRL, 0);	//��������,��ֹ���綪ʧ���絥.
}
//����: ��ȡ��ǰ�ܼ���'�����'����.
//����:@iGrp		Ҫ��ȡ���ܼ���.
//	   @rPara		���õĲ����ṹ,�������������ݺ�,ͨ�����ṹ����������.
//����: �ɹ����� true,���򷵻� false.
bool CBuyCtrl::GetBuyCtrlPara(int iGrp, TBuyCtrlPara& rPara)
{
	if (iGrp<GRP_START_PN || (GRP_START_PN+GRP_NUM)<=iGrp)
		return false;

	BYTE bBuf[43];

	if (ReadItemEx(BN0, (WORD)iGrp, 0x8107, bBuf) != sizeof(bBuf))		//����Ӧ�Ĳ������"����ز���"ID
	{
		DTRACE(DB_LOADCTRL, ("CBuyCtrl::GetBuyCtrlPara: There is something wrong when call ReadItemEx() !\n"));
		return false;
	}

	rPara.dwBillIdx = OoDoubleLongUnsignedToDWord(&bBuf[6]);
	rPara.bFlag		  = (bBuf[11]==0) ? 0x55:0xaa;
	rPara.iBuyEng	  = OoLong64ToInt64(&bBuf[15]);
	rPara.iAlarmLimit = OoLong64ToInt64(&bBuf[24]);
	rPara.iActLimit	  = OoLong64ToInt64(&bBuf[33]);

//	if (rPara.iActLimit > rPara.iAlarmLimit)
//		return false;

	if (rPara.bFlag!=0x55 && rPara.bFlag!=0xaa)   //׷��/ˢ�±�־
		return false;

	return true;
}

int64 CBuyCtrl::GetGroupEng(int iGrp)
{
	BYTE *pbFmt = NULL;
	WORD wFmtLen = 0;
	BYTE bBuf[50] = {0};
	int64 i64Value = 0;

	if (OoReadAttr(0x2300+(WORD)iGrp, 0x09, bBuf, &pbFmt, &wFmtLen) < 0)
	{
		DTRACE(DB_LOADCTRL, ("GetSelEng: There is something wrong when call OoReadAttr() !\n"));
		return -1;
	}

	i64Value = OoLong64ToInt64(&bBuf[3]);

	if (i64Value < 0) //ȡ�þ���ֵ
		i64Value = -i64Value;

	return i64Value;
}

//����: ��ȡ��ǰ����������
//����:@iCurFeeRatio Ҫ��ȡ�ĵ�ǰ����������
//����: ����ѿط���true,�������ط��� false.
bool CBuyCtrl::GetCurFeeRatio(int64 *piCurFeeRatio)
{
	BYTE bBuf[200], bEnergyFeeTime[48];
	int i, n;
	int64 iEnergyFeeRatio[50];
	bool fEnergyFeeFlag = false;
	TTime now;
	WORD minutes;
	memset( bBuf, 0, sizeof(bBuf));
	memset( bEnergyFeeTime, 0, sizeof(bEnergyFeeTime));

	if (ReadItemEx(BN0, PN0, 0x016f, bBuf) > 0)
	{
#ifdef PRO_698	
		BYTE bFeeRatioNum = bBuf[0];
		BYTE bStartNum = 1;
#else
		BYTE bFeeRatioNum = 14;
		BYTE bStartNum = 0;
#endif
		for(i=0;i<bFeeRatioNum;i++)
		{
			iEnergyFeeRatio[i] = Fmt3ToVal64(bBuf+bStartNum+4*i, 4);
			if (iEnergyFeeRatio[i] == 0)
				iEnergyFeeRatio[i] = 1;
			if (iEnergyFeeRatio[i] != 1)
				fEnergyFeeFlag = true;
		}
	}
#ifdef PRO_698	
	if (ReadItemEx(BN0, PN0, 0x015f, bBuf) > 0)
	{
		for(i=0;i<48;i++)
		{
			bEnergyFeeTime[i] = bBuf[i];
			//if(bEnergyFeeTime[i]>47)  //Old 376.1
			if(bEnergyFeeTime[i]>11)   //New 376.1
				bEnergyFeeTime[i] = 0;
		}
	}
#else
	if (ReadItemEx(BN0, PN0, 0x015f, bBuf) > 0)
	{
		for(i=0,n=0;i<48;i+=2,n++)
		{
			bEnergyFeeTime[i] = (bBuf[n]&0x0f);
			bEnergyFeeTime[i+1] = ((bBuf[n]&0xf0)>>4);
		}
	}
#endif
	GetCurTime(&now);
	minutes=now.nHour*60+now.nMinute;
	n=minutes/30;
	*piCurFeeRatio = iEnergyFeeRatio[(bEnergyFeeTime[n])];
	
	return fEnergyFeeFlag;
}
