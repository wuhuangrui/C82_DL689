 /*********************************************************************************************************
 * Copyright (c) 2013,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DrvCtrlAPI.cpp
 * 摘    要：本文件主要实现了控制模块（单片机类型）的所有功能
 * 当前版本：1.0
 * 作    者：周治金
 * 完成日期：2013.06.25
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/
#ifndef DRVCTRLAPI_H
#define DRVCTRLAPI_H

#define LED_MODE_OFF		0
#define LED_MODE_ON			1

#define CTRL_TURN1	0	//轮次1
#define CTRL_TURN2	1	//轮次2
#define CTRL_ALERT	2	//告警


#define CTRL_TURN1_CLOSE	0x01	//轮次1 合闸
#define CTRL_TURN2_CLOSE	0x08	//轮次2	合闸
#define CTRL_ALERT_CLOSE	0x01	//告警 合闸

#define LED_TURN1_GREEN_ON	0x04//轮次1绿色灯 点亮
#define LED_TURN1_RED_ON	0x02//轮次1红色灯 点亮
#define LED_TURN2_GREEN_ON	0x20//轮次2绿色灯 点亮
#define LED_TURN2_RED_ON	0x10//轮次2红色灯 点亮

#define LED_POWERCTRL_ON    0x40//功控灯 点亮
#define LED_ENERGYCTRL_ON   0x80//电控灯 点亮
#define LED_GUARANTEE_ON    0x02//保电灯 点亮

#define LED_TURN1		 8
#define LED_TURN2		 9
#define LED_LOOP_TURN1	 10
#define LED_LOOP_TURN2	 11
#define CMD_ENCRST_L  13//加密模块复位低电平
#define CMD_ENCRST_H  14//加密模块复位高电平

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