 /*********************************************************************************************************
 * Copyright (c) 2013,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DrvCtrlAPI.cpp
 * ժ    Ҫ�����ļ���Ҫʵ���˿���ģ�飨��Ƭ�����ͣ������й���
 * ��ǰ�汾��1.0
 * ��    �ߣ����ν�
 * ������ڣ�2013.06.25
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
*********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "DrvCtrlAPI.h"
#include "bios.h"
#include "Comm.h"
#include "Trace.h"
#include "ComAPI.h"
#include "sysapi.h"
#include "LibDbAPI.h"
#include "bios.h"
#include "FaAPI.h"


BYTE g_bLastState[2] = {0, 0};
BYTE g_bMode = 1;// 0:��ȡ����ģ��״̬ 1���ն˼�¼����״̬
BYTE g_bCtrlMode = 0;
DWORD g_wPulseWide = 0;

//g_bAlarmLedCtrlÿ1λ��Ӧһ��Ӧ�ò㹦�ܶԸ澯�ƵĿ���
BYTE g_bAlarmLedCtrl = 0xff;
bool g_fAlrLedCtrlMode = false;
bool g_fStartThread = false;
void AlarmLedCtrl()
{
    bool fCurCtrlState;
	static bool fLastCtrlState = false;		//LED OFF
	
	BYTE bLedCtrlState = g_bAlarmLedCtrl;	
	if ((bLedCtrlState&0x3) == 0x3)	//645�¼��Ϳ����̶߳��أ��Źرո澯��
		fCurCtrlState = false;	//LED OFF
	else	// if (((bLedCtrlState&0x1) == 0) || ((bLedCtrlState&0x2) == 0))	//���澯��
		fCurCtrlState = true;	//LED ON
	
	if (fCurCtrlState != fLastCtrlState)
	{
		fLastCtrlState = fCurCtrlState;
		LedAlert(fCurCtrlState);		
	}
}

//�ظ澯��
void CloseAlarmLed(WORD wOffset)
{
	g_bAlarmLedCtrl |= (1<<wOffset);
	AlarmLedCtrl();
}

//���澯��
void OpenAlarmLed(WORD wOffset)
{
	g_bAlarmLedCtrl &= ~(1<<wOffset);
	AlarmLedCtrl();
}
extern CComm g_commYK;
//���𣺿���LED��
void SetCtrlLed(bool fOn, BYTE bId)
{
	BYTE bBuf[2], bRxBuf[20];
	static bool fPo = false;
	static bool fEn = false;
	static bool fGu =  false;
	static bool fT1 =  false;
	static bool fT2 =  false;



	static bool fPoInit = false;
	static bool fEnInit = false;
	static bool fGuInit =  false;
	static bool fT1Init =  false;
	static bool fT2Init =  false;


	bool fSend = false;

	memset(bBuf, 0, sizeof(bBuf));
	memset(bRxBuf, 0, sizeof(bRxBuf));

	switch(bId)
	{
	case LED_POWERCTRL://����
		if ((fOn != fPo) || !fPoInit)
		{
			fPoInit = true;
			fPo = fOn;
			if (ReadCtrlState(bBuf))//��ȡ֮ǰ��״̬
				fSend = true;
		}

		if (fOn)
		{
			bBuf[0] |= LED_POWERCTRL_ON;
			g_bLastState[0] |= LED_POWERCTRL_ON;
		}
		else
		{
			bBuf[0] &= ~LED_POWERCTRL_ON;
			g_bLastState[0] &= ~LED_POWERCTRL_ON;
		}

		
		break;
	case LED_ENERGYCTRL://���
		if ((fOn != fEn) || !fEnInit)
		{
			fEnInit = true;
			fEn = fOn;
			if (ReadCtrlState(bBuf))//��ȡ֮ǰ��״̬
				fSend = true;
		}

		if (fOn)
		{
			bBuf[0] |= LED_ENERGYCTRL_ON;
			g_bLastState[0] |= LED_ENERGYCTRL_ON;
		}
		else
		{
			bBuf[0] &= ~LED_ENERGYCTRL_ON;
			g_bLastState[0] &= ~LED_ENERGYCTRL_ON;
		}

		
		
		break;
	case LED_GUARANTEE://����
		if ((fOn != fGu) || !fGuInit)
		{
			fGuInit = true;
			fGu = fOn;
			if (ReadCtrlState(bBuf))//��ȡ֮ǰ��״̬
				fSend = true;
		}

		if (fOn)
		{
			bBuf[1] |= LED_GUARANTEE_ON;
			g_bLastState[1] |= LED_GUARANTEE_ON;
		}
		else
		{
			bBuf[1] &= ~LED_GUARANTEE_ON;
			g_bLastState[1] &= ~LED_GUARANTEE_ON;
		}

		break;
	case LED_LOOP_TURN1:
		 bRxBuf[0] = 1;

	case LED_TURN1://�ִ�1
	
		/*if (GetCtrlCfg(1) == 0)
		{
			return;
		}*/
		if (ReadCtrlState(bBuf))//��ȡ֮ǰ��״̬
			fSend = true;

		if (fOn)
		{
			bBuf[0] |= LED_TURN1_RED_ON;
			bBuf[0] &= ~LED_TURN1_GREEN_ON;

			g_bLastState[0] |= LED_TURN1_RED_ON;
			g_bLastState[0] &= ~LED_TURN1_GREEN_ON;
			
		}
		else
		{
			bBuf[0] |= LED_TURN1_GREEN_ON;
			bBuf[0] &= ~LED_TURN1_RED_ON;

			g_bLastState[0] |= LED_TURN1_GREEN_ON;
			g_bLastState[0] &= ~LED_TURN1_RED_ON;
		}
		
		break;
	case LED_LOOP_TURN2:
		 bRxBuf[0] = 1;

	case LED_TURN2://�ִ�2
		
		/*if (GetCtrlCfg(2) == 0)
		{
			return;
		}*/
		if (ReadCtrlState(bBuf))//��ȡ֮ǰ��״̬
			fSend = true;

		if (fOn)
		{
			bBuf[0] |= LED_TURN2_RED_ON;
			bBuf[0] &= ~LED_TURN2_GREEN_ON;

			g_bLastState[0] |= LED_TURN2_RED_ON;
			g_bLastState[0] &= ~LED_TURN2_GREEN_ON;
		}
		else
		{
			bBuf[0] |= LED_TURN2_GREEN_ON;
			bBuf[0] &= ~LED_TURN2_RED_ON;

			g_bLastState[0] |= LED_TURN2_GREEN_ON;
			g_bLastState[0] &= ~LED_TURN2_RED_ON;
			
		}

		break;
	default:
		break;
	}

	if (fSend)
	{
		if (g_bMode)
			MakeSendFrm(g_bLastState, bRxBuf, 0xA3);
		else
			MakeSendFrm(bBuf, bRxBuf, 0xA3);
	}

}
//���𣺿��Ƽ̵������ִε�
void DoYk(bool fOpen, BYTE bId, BYTE bMode)
{
	BYTE bBuf[2], bRxBuf[20];

	static bool fN1 = false;
	static bool fN2 = false;
	static bool fA = false;

	static bool fInitN1 = false;
	static bool fInitN2 = false;
	static bool fInitA = false;

	bool fSend = false;

	memset(bBuf, 0, sizeof(bBuf));
	memset(bRxBuf, 0, sizeof(bRxBuf));

	switch(bId)
	{
	case CTRL_TURN1://�ִ�1
		/*if (GetCtrlCfg(1) == 0)
		{
			fInitN1 =  false;
			return;
		}*/
		if (!bMode)
		{
			if (!fInitN1 || (fOpen != fN1))
			{
				fN1 = fOpen;
				fInitN1 = true;
				if (ReadCtrlState(bBuf))//��ȡ֮ǰ��״̬
					fSend = true;
			}
		}
		else
		{
			fSend = true;
		}

		if (!fSend)
			break;

		if (fOpen)
		{
			if (!bMode)
			{
				bBuf[0] |= LED_TURN1_RED_ON;
				bBuf[0] &= ~LED_TURN1_GREEN_ON;
				g_bLastState[0] |= LED_TURN1_RED_ON;
				g_bLastState[0] &= ~LED_TURN1_GREEN_ON;
			}
			

			g_bLastState[0] &= ~CTRL_TURN1_CLOSE;
			bBuf[0] &= ~CTRL_TURN1_CLOSE;
		}
		else
		{
			if (!bMode)
			{
				bBuf[0] &= ~LED_TURN1_RED_ON;
				bBuf[0] |= LED_TURN1_GREEN_ON;
				g_bLastState[0] &= ~LED_TURN1_RED_ON;
				g_bLastState[0] |= LED_TURN1_GREEN_ON;
			}

			g_bLastState[0] |= CTRL_TURN1_CLOSE;
			bBuf[0] |= CTRL_TURN1_CLOSE;
		}
		
		break;
	case CTRL_TURN2://�ִ�2
		/*if (GetCtrlCfg(2) == 0)
		{
			fInitN2 = false;
			return;
		}*/
		if (!bMode)
		{
			if (!fInitN2 || (fOpen != fN2))
			{
				fN2 = fOpen;
				fInitN2 = true;
				if (ReadCtrlState(bBuf))//��ȡ֮ǰ��״̬
					fSend = true;
			}
		}
		else
		{
			fSend = true;
		}

		if (!fSend)
			break;

		if (fOpen)
		{
			if (!bMode)
			{
				bBuf[0] |= LED_TURN2_RED_ON;
				bBuf[0] &= ~LED_TURN2_GREEN_ON;
				g_bLastState[0] |= LED_TURN2_RED_ON;
				g_bLastState[0] &= ~LED_TURN2_GREEN_ON;
			}

			g_bLastState[0] &= ~CTRL_TURN2_CLOSE;
			bBuf[0] &= ~CTRL_TURN2_CLOSE;
		}
		else
		{
			if (!bMode)
			{
				bBuf[0] &= ~LED_TURN2_RED_ON;
				bBuf[0] |= LED_TURN2_GREEN_ON;
				g_bLastState[0] &= ~LED_TURN2_RED_ON;
				g_bLastState[0] |= LED_TURN2_GREEN_ON;
			}

			g_bLastState[0] |= CTRL_TURN2_CLOSE;
			bBuf[0] |= CTRL_TURN2_CLOSE;
		}
		
		break;
	case LED_ALERT://�澯
		if (!fInitA || (fOpen != fA))
		{
			fA = fOpen;
			fInitA = true;
			if (ReadCtrlState(bBuf))//��ȡ֮ǰ��״̬
				fSend = true;
		}
		if (fOpen)
		{
			bBuf[1] &= ~CTRL_ALERT_CLOSE;
			g_bLastState[1] &= ~CTRL_ALERT_CLOSE;
		}
		else
		{
			bBuf[1] |= CTRL_ALERT_CLOSE;
			g_bLastState[1] |= CTRL_ALERT_CLOSE;
		}

		break;
	default:
		break;
	}

	if (fSend)
	{
		if (g_bMode)
			MakeSendFrm(g_bLastState, bRxBuf, 0xA3);
		else
			MakeSendFrm(bBuf, bRxBuf, 0xA3);
	}
}

BYTE MakeSendFrm(BYTE *pbTx, BYTE *pbRx, BYTE bFN)
{
	BYTE bCmdFrm[7];
	int i = 0;
	WORD wCrc = 0;
	memset(bCmdFrm, 0, sizeof(bCmdFrm));

	bCmdFrm[0]  = 0x68;
	bCmdFrm[1]  = bFN;//������

	bCmdFrm[2]  = pbTx[0] ;//���ݵ��ֽ�
	bCmdFrm[3]  = pbTx[1] ;//���ݸ��ֽ�

	wCrc = get_crc_16(wCrc, bCmdFrm, 4);
	bCmdFrm[4]  = (wCrc>>8) & 0x00ff;
	bCmdFrm[5]  =  wCrc & 0x00ff;
	bCmdFrm[6]  = 0x16;

	return TxRxComm(bCmdFrm, pbRx);
}

BYTE TxRxComm(BYTE *pbTx, BYTE *pbRx)
{
	WORD wLen = 0, i = 0;
	BYTE bDebug = pbRx[0];

	WaitSemaphore(g_semRWCtrlModl);

	wLen = g_commYK.Write(pbTx, 7);
	if (wLen == 7)
	{
		if (bDebug != 1)
		{
			if (pbTx[1]!=0xa5 || IsDebugOn(113))
				TraceBuf(DB_CTRL, "send-->", pbTx, 7);
		}
		else
		{
			TraceBuf(114, "send-->", pbTx, 7);
		}

	}
	else
	{
		SignalSemaphore(g_semRWCtrlModl);
		return 0;
	}

	Sleep(50);

	wLen = g_commYK.Read(pbRx, 7);

	SignalSemaphore(g_semRWCtrlModl);

	if (wLen >0)
	{
		if (bDebug != 1)
		{
			if (pbRx[1]!=0x5a || IsDebugOn(113))
			{
				TraceBuf(DB_CTRL, "recv<--", pbRx, 7);
				DTRACE(DB_CTRL, ("\n"));
			}
		}
		else
		{
			TraceBuf(114, "recv<--", pbRx, 7);
			DTRACE(114, ("\n"));
		}
			
		
	}
		
	return wLen;
	
}

BYTE ReadCtrlLoop(BYTE &bRx)
{
	BYTE bRxBuf[20];
	BYTE bLen = 0, i = 0, bH =0, bL = 0;
	BYTE bFN = 0xa5;
	//BYTE bCmdFrm[7] = {0x68, 0x9c, 0xff, 0xff, 0xfe, 0xdd, 0x16};
	BYTE bTxBuf[2] = {0, 0};

	memset(bRxBuf, 0, sizeof(bRxBuf));
	bLen = MakeSendFrm(bTxBuf, bRxBuf, bFN);

	if (CheckRxFrm(bRxBuf, bLen, i, bFN))
	{
		bRx = bRxBuf[i+2];//���ֽ�
		return bLen;
	}
	
	return 0;
}

bool ReadCtrlState(BYTE *p)
{
	BYTE bRxBuf[20];
	BYTE bLen = 0, i = 0, j = 0;
	//BYTE bCmdFrm[7] = {0x68, 0x3a, 0xff, 0xff, 0xdd, 0x3d, 0x16};
	BYTE bTxBuf[2] = {0, 0};
	BYTE bFN = 0xA4;

	if (g_bMode == 1)
		return true;

	memset(bRxBuf, 0, sizeof(bRxBuf));
	for(j=0; j<3; j++)
	{
		bLen = MakeSendFrm(bTxBuf, bRxBuf, bFN);
		if (CheckRxFrm(bRxBuf, bLen, i, bFN))
		{
			p[0] = bRxBuf[i+2];//���ֽ�
			p[1] = bRxBuf[i+2+1];//���ֽ�
			return true;
		}

		Sleep(500);
	}
	
	return false;
}

bool CheckRxFrm(BYTE *pbRx, BYTE bLen, BYTE &bFlag, BYTE bFN)
{
	BYTE i, bH=0, bL=0;
	WORD wCrc = 0;
	BYTE bRetFn = 0;
	if (bLen < 7)
		return false;

	if (bFN == 0xa3)
	{
		bRetFn = 0x3a;
	}
	else if (bFN == 0xa4)
	{
		bRetFn = 0x4a;
	}
	else if (bFN == 0xa5)
	{
		bRetFn = 0x5a;
	}

	for (i=0; i<bLen; i++)
	{
		if (pbRx[i]==0x68)//
		{
			if (i+6 >= bLen)
				return false;

			wCrc = get_crc_16(wCrc, pbRx, 4);
			bH =(wCrc>>8) & 0x00ff;
			bL = wCrc & 0x00ff;

			if (pbRx[i+1]==bRetFn && bH==pbRx[4] && bL==pbRx[5] && pbRx[i+6]==0x16)
			{
				bFlag = i;
				return true;
			}
		}
	}

	return false;
}
void InitCtrlModule()
{
	BYTE bRxBuf[20];
	BYTE bBuf[10];

	DTRACE(DB_CTRL, ("InitCtrlModule��>--------Init Start------> \n"));
	memset(g_bLastState, 0, sizeof(g_bLastState));
	ReadItemEx(BN1, PN0, 0x2022, bBuf);  //�̵��������ʽ,0��ƽ,1����, 2��ֹ���м̵�������
	g_bCtrlMode = bBuf[0];   //ң�������ʽ:YK_MODE_LEVEL | YK_MODE_PULSE

#ifndef SYS_WIN
	if (g_bCtrlMode == 2)
	{
		Ctrl_OnOff(false);//�̵���ʹ�ܿ���
	}
	else
	{
		Ctrl_OnOff(true);
	}

#endif

	SetAlrLedCtrlMode(false);
	if(ReadItemEx(BN24, PN0, 0x5023, bBuf) > 0)
	{
		g_wPulseWide = BcdToDWORD(bBuf, 2);
	}
	else
	{
		g_wPulseWide = 800;
	}

	DTRACE(DB_CTRL, ("g_wPulseWide = %d \n", g_wPulseWide));
	memset(bRxBuf, 0, sizeof(bRxBuf));

//-------------------------------------------�ִ�1
	if (GetCtrlCfg(1) != 0)
	{
		g_bLastState[0] |= LED_TURN1_GREEN_ON;
	}
	else
	{	
		g_bLastState[0] &= ~LED_TURN1_GREEN_ON;
	}
	g_bLastState[0] &= ~LED_TURN1_RED_ON;
	g_bLastState[0] |= CTRL_TURN1_CLOSE;

//-------------------------------------------�ִ�2
	if (GetCtrlCfg(2) != 0)
	{
		g_bLastState[0] |= LED_TURN2_GREEN_ON;
	}
	else
	{
		g_bLastState[0] &= ~LED_TURN2_GREEN_ON;
	}
	g_bLastState[0] &= ~LED_TURN2_RED_ON;
	g_bLastState[0] |= CTRL_TURN2_CLOSE;

//---------------------------------------------
	g_bLastState[0] &= ~LED_POWERCTRL_ON;
	g_bLastState[0] &= ~LED_ENERGYCTRL_ON;
	g_bLastState[1] &= ~LED_GUARANTEE_ON;
	g_bLastState[1] |= CTRL_ALERT_CLOSE;

	MakeSendFrm(g_bLastState, bRxBuf, 0xa3);

	//DoTurnLedCtrl();
	DTRACE(DB_CTRL, ("InitCtrlModule��<--------Init End------< \n"));

}

BYTE GetCtrlCfg(BYTE bTurn)
{
	BYTE bCfgF45 = 0, bCfgF48 = 0, bCfg = 0;
	int iGrp =0;
	for (iGrp=1; iGrp<=GRP_NUM; iGrp++)
	{
		ReadItemEx(BN0, (WORD)iGrp, 0x02df, &bCfgF45); //F45�����ִ��趨
		ReadItemEx(BN0, (WORD)iGrp, 0x030f, &bCfgF48); //F48����ִ��趨

		bCfg |= bCfgF45|bCfgF48;

		bCfgF45 = 0;
		bCfgF48 = 0;
	}
		
//	return bCfg & (1<<(bTurn-1));
	return 1;
}

void DoTurnLedCtrl()
{
	BYTE bRxBuf[20];
	memset(bRxBuf, 0, sizeof(bRxBuf));

	//-------------------------------------------�ִ�1
	if (GetCtrlCfg(1) == 0)
	{
		g_bLastState[0] &= ~LED_TURN1_GREEN_ON;
		g_bLastState[0] &= ~LED_TURN1_RED_ON;
		g_bLastState[0] |= CTRL_TURN1_CLOSE;
	}
	else
	{
		if ((g_bLastState[0]&LED_TURN1_RED_ON)==LED_TURN1_RED_ON && 
			(g_bLastState[0]&LED_TURN1_GREEN_ON)==LED_TURN1_GREEN_ON)
		{
			g_bLastState[0] |= LED_TURN1_GREEN_ON;
		}
	}
	
	

	//-------------------------------------------�ִ�2
	if (GetCtrlCfg(2) == 0)
	{
		g_bLastState[0] &= ~LED_TURN2_GREEN_ON;
		g_bLastState[0] &= ~LED_TURN2_RED_ON;
		g_bLastState[0] |= CTRL_TURN2_CLOSE;
	}
	else
	{
		if ((g_bLastState[0]&LED_TURN2_RED_ON)==LED_TURN2_RED_ON && 
			(g_bLastState[0]&LED_TURN2_GREEN_ON)==LED_TURN2_GREEN_ON)
		{
			g_bLastState[0] |= LED_TURN2_GREEN_ON;
		}
	}

	MakeSendFrm(g_bLastState, bRxBuf, 0xa3);

}

void DoLoopLed(BYTE &bFrm)
{
	//return ;
	BYTE bRxBuf[20];
	static bool fN1 = true;
	static bool fN2 = true;
	static bool fErrN1 = true;
	static bool fErrN2 = true;

	BYTE bBuf[1+1+1+8*8];
	memset(bBuf, 0, sizeof(bBuf));

	if (ReadItemEx(BN0, PN0, 0x105f, bBuf) <=0)	//��"�ն˵�ǰ����״̬"ID
	{
		
	}

	BYTE bStatus = 0;
	BYTE bGrpFlg = 0;

	bStatus |= bBuf[0];//ң����բ
	bGrpFlg = bBuf[2];//�ܼ�����Ч��־λ

	for (int i=0; i<GRP_NUM; i++, bGrpFlg>>=1)
	{
		if ((bGrpFlg&0x01) != 0)
		{
			bStatus |= bBuf[3+2+1+8*i];//������բ
			bStatus |= bBuf[3+2+1+8*i+1];//�µ����բ
			bStatus |= bBuf[3+2+1+8*i+2];//�������բ
		}

	}

	if ((GetCtrlCfg(1)==0) || (bStatus&0x01))//δͶ����� ���� ���ִδ�����բ״̬
	{
		bFrm &= ~0x01;
	}
	if ((GetCtrlCfg(2)==0) || (bStatus&0x02))
	{
		bFrm &= ~0x02;
	}

	if ((bFrm&1) == 1)//��һ·
	{
		SetCtrlLed(fN1, LED_LOOP_TURN1);
		if (fN1)
			fN1 = false;
		else
			fN1 = true;
		fErrN1 = true;
	}
	else
	{
		if (fErrN1 && GetCtrlCfg(1)!=0)
		{
			SetCtrlLed(bStatus&1 , LED_LOOP_TURN1);
			fErrN1 = false;
		}
	}

	if ((bFrm&2) == 2)//�ڶ�·
	{
		SetCtrlLed(fN2, LED_LOOP_TURN2);
		if (fN2)
			fN2 = false;
		else
			fN2 = true;
		fErrN2 = true;
	}
	else
	{
		if (fErrN2 && GetCtrlCfg(2)!=0)
		{
			SetCtrlLed((bStatus>>1)&1 , LED_LOOP_TURN2);
			fErrN2 = false;
		}
	}

};
void GetCtrlVer()
{
	BYTE bRxBuf[20];
	BYTE bTxBuf[2] = {0x00, 0x00};
	BYTE bFN = 0xff;

	memset(bRxBuf, 0, sizeof(bRxBuf));
	memset(bTxBuf, 0, sizeof(bTxBuf));

	if (MakeSendFrm(bTxBuf, bRxBuf, bFN) > 0)
		TraceBuf(DB_CRITICAL, "CtrlModelVer=", bRxBuf, 7);

};


//���巽ʽ���Ƽ̵���
void DoLedByPulse()
{

	BYTE bBuf[1+1+1+8*8];
	BYTE bStatus = 0, bGrpFlg = 0;
	BYTE bVal = 0;
	DWORD dwSec = 0;

	static DWORD dwSecCloseN1 = 0;
	static DWORD dwSecOpenN1 = 0;

	static DWORD dwSecCloseN2 = 0;
	static DWORD dwSecOpenN2 = 0;

	static bool fInit = false;

	if (g_bCtrlMode != 1)
	{
		return ;
	}

	memset(bBuf, 0, sizeof(bBuf));

	dwSec = GetTick();

	if (!fInit)
	{
		dwSecOpenN1 = dwSec;
		dwSecCloseN1 = dwSec;
		dwSecOpenN2 = dwSec;
		dwSecCloseN2 = dwSec;
		fInit = true;
	}
	
	if (ReadItemEx(BN0, PN0, 0x105f, bBuf) <=0)	//��"�ն˵�ǰ����״̬"ID
	{
		return;
	}

	bStatus |= bBuf[0];//ң����բ
	bGrpFlg = bBuf[2];//�ܼ�����Ч��־λ
	for (int i=0; i<GRP_NUM; i++, bGrpFlg>>=1)
	{
		if ((bGrpFlg&0x01) != 0)
		{
			bStatus |= bBuf[3+2+1+8*i];//������բ
			bStatus |= bBuf[3+2+1+8*i+1];//�µ����բ
			bStatus |= bBuf[3+2+1+8*i+2];//�������բ
		}
		
	}

	if (bStatus == 0)
	{
		dwSecOpenN1 = dwSec;
		dwSecCloseN1 = dwSec;
		dwSecOpenN2 = dwSec;
		dwSecCloseN2 = dwSec;
		return;
	}

	if ((bStatus&1) == 1)//��һ��
	{
		bVal = g_bLastState[0]&CTRL_TURN1_CLOSE;
		
		if (bVal)
		{
			if ((dwSec - dwSecOpenN1) >= 60*1000)//1����
			{
				dwSecCloseN1 = dwSec;
				DoYk(true, CTRL_TURN1, 1);//��բ
			}
		}
		else//��բ
		{
			if ((dwSec - dwSecCloseN1) >= g_wPulseWide)
			{
				dwSecOpenN1 = dwSec;
				DoYk(false, CTRL_TURN1, 1);//��բ
			}
		}
	}
	else
	{
		dwSecOpenN1 = dwSec;
		dwSecCloseN1 = dwSec;
	}
	if ((bStatus&2) == 2)//�ڶ���
	{
		bVal = (g_bLastState[0]&CTRL_TURN2_CLOSE)>>3;
		
		if (bVal)
		{
			if ((dwSec - dwSecOpenN2) >= 60*1000)//1����
			{
				dwSecCloseN2 = dwSec;
				DoYk(true, CTRL_TURN2, 1);//��բ
			}
		}
		else//��բ
		{
			if ((dwSec - dwSecCloseN2) >= g_wPulseWide)
			{
				dwSecOpenN2 = dwSec;
				DoYk(false, CTRL_TURN2, 1);//��բ
			}
		}
	}
	else
	{
		dwSecOpenN2 = dwSec;
		dwSecCloseN2 = dwSec;
	}
	

	

		
}

bool SetAlrLedCtrlMode(bool fMode)
{		
	g_fAlrLedCtrlMode = fMode;
	return true;
}

bool GetAlrLedCtrlMode()
{
	return g_fAlrLedCtrlMode;
}

bool SetCtrlThreadStart(bool fStart)
{
	g_fStartThread = fStart;
	return true;
}
bool GetCtrlThreadState()
{
	return g_fStartThread;
}

void DoAlrLedCtrl()
{
	static bool fOn = true;
	static bool fFlag = true;

	static DWORD dwLastSec = 0;

	DWORD dwSec = GetTick();
	
	if (GetAlrLedCtrlMode())
	{
		if ((dwSec - dwLastSec) >= 500)//Ҫ��Ϊ1���ӣ�����ʵ������ʱ�����ͺ�
		{
			if (fOn)//�������� LED����
			{
				fOn = false;
				OutBeepMs(150);
				//LedAlert(true);
				SetLed(true, LED_ALERT);
			}
			else
			{
				fOn = true;
				//LedAlert(false);
				SetLed(false, LED_ALERT);
			}

			dwLastSec = dwSec;
		}

		fFlag = true;
	}
	else
	{
		if (fFlag)
		{
			//LedAlert(false);
			SetLed(false, LED_ALERT);
			fFlag = false;
		}
		
	}
	
}
