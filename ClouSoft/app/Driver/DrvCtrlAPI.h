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
#ifndef DRVCTRLAPI_H
#define DRVCTRLAPI_H

#define LED_MODE_OFF		0
#define LED_MODE_ON			1

#define CTRL_TURN1	0	//�ִ�1
#define CTRL_TURN2	1	//�ִ�2
#define CTRL_ALERT	2	//�澯


#define CTRL_TURN1_CLOSE	0x01	//�ִ�1 ��բ
#define CTRL_TURN2_CLOSE	0x08	//�ִ�2	��բ
#define CTRL_ALERT_CLOSE	0x01	//�澯 ��բ

#define LED_TURN1_GREEN_ON	0x04//�ִ�1��ɫ�� ����
#define LED_TURN1_RED_ON	0x02//�ִ�1��ɫ�� ����
#define LED_TURN2_GREEN_ON	0x20//�ִ�2��ɫ�� ����
#define LED_TURN2_RED_ON	0x10//�ִ�2��ɫ�� ����

#define LED_POWERCTRL_ON    0x40//���ص� ����
#define LED_ENERGYCTRL_ON   0x80//��ص� ����
#define LED_GUARANTEE_ON    0x02//����� ����

#define LED_TURN1		 8
#define LED_TURN2		 9
#define LED_LOOP_TURN1	 10
#define LED_LOOP_TURN2	 11
#define CMD_ENCRST_L  13//����ģ�鸴λ�͵�ƽ
#define CMD_ENCRST_H  14//����ģ�鸴λ�ߵ�ƽ

extern void SetCtrlLed(bool fOn, BYTE bId);
extern void DoYk(bool fOpen, BYTE bId, BYTE bMode = 0);
BYTE MakeSendFrm(BYTE *pbTx, BYTE *pbRx, BYTE bFN);
BYTE TxRxComm(BYTE *pbTx, BYTE *pbRx);
bool ReadCtrlState(BYTE *p);
BYTE ReadCtrlLoop(BYTE &pbRx);
void InitCtrlModule();
BYTE GetCtrlCfg(BYTE bTurn);
void DoTurnLedCtrl();
void DoLoopLed(BYTE &bFrm);
bool CheckRxFrm(BYTE *pbRx, BYTE bLen, BYTE &bFlag, BYTE bFN);
void GetCtrlVer();
void DoLedByPulse();
bool SetAlrLedCtrlMode(bool fMode);
bool GetAlrLedCtrlMode();
extern bool SetCtrlThreadStart(bool fStart);
extern bool GetCtrlThreadState();
void DoAlrLedCtrl();
#endif