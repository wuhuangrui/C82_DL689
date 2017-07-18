/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�bios.h
 * ժ    Ҫ�����ļ���Ҫʵ��ϵͳ������������ӿڼ��ж���Ӧ������
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��7��
 * ��    ע: ���ʵ���������򼶵ĺ����ŵ�drivers.cpp�����������ļ�,
 *           Ӧ�ó���Ҫֱ�Ӱ������ļ�,���ļ���drivers.h����,
 *           ����drivers.h������
*********************************************************************************************************/
#ifndef  BIOS_H
#define  BIOS_H

/////////////////////////////////////////////////////////////////////////////////
//����IO����
#define GPIO_L	 	0
#define GPIO_H 		1
#define GPIO_O 	  	2
#define GPIO_I 	  	3
#define GPIO_G		4

#define LED_POWERCTRL    0//����
#define LED_ENERGYCTRL   1//���
#define LED_GUARANTEE    2//����
#define LED_ALERT        3//�澯
//cl790E7
#define LED_RECEIVE      4//����
#define LED_SEND         5//����
#define LED_CALL         6//ͨ��
#define LED_RUN          7//����
#define LED_TURN1		 8
#define LED_TURN2		 9
#define LED_LOOP_TURN1	 10
#define LED_LOOP_TURN2	 11
#define CMD_ENCRST_L  13//����ģ�鸴λ�͵�ƽ
#define CMD_ENCRST_H  14//����ģ�鸴λ�ߵ�ƽ


void ClearWDG();
void ResetCPU(void);

void InitMIO();
void InitGPIO();
inline void GpioSetBit(WORD wPos)
{
	wPos;
}

inline void GpioClrBit(WORD wPos)
{
	wPos;
}

inline void GpioToggleBit(WORD wPos)
{
	wPos;
}

inline bool GpioGetBit(WORD wPos)
{
	wPos;
}

bool ESRouterPowerOn();
bool ESRouterReset();

bool InitInfrared();
bool ResetGR47();
bool ResetWavecom();
bool ResetMC39();
bool ResetCX06833(WORD wIndex);
bool ResetGC864();
bool ResetME3000();
bool ResetMG815();
bool ResetLC6311();
bool PowerOnCW();
bool PowerOffCW();
bool ResetPlc();
BYTE GetLidStatus();

void ModemInit();
void ModemPowerOn();
void ModemPowerOff();
bool ModemClrDTR();
bool ModemSetDTR();

void BeepPulse(void);

//���뺯��
WORD GetYxInput();
bool IsProgKeyEnable();
bool IsSecretKeyEnable();
BYTE GetModemType();
BYTE GetDoorStatus();//�������Žڵ㵥����IO�ſ��Ƶ�ƽ̨

//��̨���ƽӿں���
bool R230mIsBusy();
void R230MRequestSend();
void R230MEndSend();
void SetCallStatus(BYTE bStatus);//��̨ͨ������ ������0/1
void SetTxLed(WORD wDelay);//���͵ƿ���
void SetRxLed(WORD wDelay);//���յƿ���
void DoTxRxLed();//����˸
void R230mLightTxLed(WORD wLen);
void R230mLightRxLed(WORD wLen);
inline void R230mDarkTxLed();
inline void R230mDarkRxLed();
bool CheckR230Error();

BYTE GetCommRxCn(WORD wPort);
void SetCommTxCn(WORD wPort, BYTE bCn);

////////////////////////////////////////////////////////////////////////////////////
//��ƽ̨���еĽӿں�������
void DelayMs(int ms);
void ConfigInterrupt(void);
void InitSDRAM(void);
void InitTimers(int nTimer,long int mS);
void InitTimer2ToPWM(void);
void InitClock(WORD VcoClkInDiv,WORD ClkSclkDiv);
 
extern void ClearWDG(void);
extern void SetUart(void);
extern void ResetUart(void);
extern void SetLed(bool fOn, BYTE bID);
extern void LedAlert(bool fOn);
void SetRunLed(bool fOn);//true ���е���  false  ����
void ToggleRunLed();

//���������ƺ���
extern void ResetBeep(void);
extern void SetBeep(void);
extern void OutBeepMs(unsigned long  Ms);

void EnableCtDetect(bool fEnable);
void InitCtDetect();
void SelectCtCn(unsigned short chn);
void CheckPower();
bool IsPoweredByAC();
void GprsBatOnOff(bool fOn);
void BackBatOnOff(bool fOn);
void GprsBatCharge(bool fOn);
void PulseInitPrg();
void PulseOutPrg(bool fEnable);
void  ResetEncMoudle(void);
BYTE  GetHardVer(void);
BYTE GetPlcStatus();
#endif
