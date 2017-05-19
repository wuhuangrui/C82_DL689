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
#include "apptypedef.h"
#include "at91_gpio.h"
#include "board.h"
#include "DrvStruct.h"

/////////////////////////////////////////////////////////////////////////////////
//����IO����
#define GPIO_L	 	_IOW('g', 0, char) 
#define GPIO_H 		_IOW('g', 1, char) 
#define GPIO_O 	  	_IOW('g', 2, char) 
#define GPIO_I 	  	_IOW('g', 3, char) 
#define GPIO_I_P    _IOW('g', 4, char) 
#define GPIO_G		_IOW('g', 5, char) 
#define PRINT_ETH0_STATUS  _IOW('g', 6, char)
#define GET_ETH0_STATUS	   _IOW('g', 7, char)
#define SET_ETH0_STATUS	   _IOW('g', 8, char)

#define LED_POWERCTRL    0//����
#define LED_ENERGYCTRL   1//���
#define LED_GUARANTEE    2//����
#define LED_ALERT        3//�澯
//cl790E7
#define LED_RECEIVE      4//����
#define LED_SEND         5//����
#define LED_CALL         6//ͨ��
#define LED_RUN          7//����

void ClearWDG();
void ResetCPU(void);

void SetAttConnectTypePin(BYTE bConnectType);
void InitMIO();
void InitGPIO();
inline void GpioSetBit(WORD wPos)
{
}

inline void GpioClrBit(WORD wPos)
{
}

inline void GpioToggleBit(WORD wPos)
{
}

inline bool GpioGetBit(WORD wPos)
{
}

bool ESRouterPowerOn();
bool ESRouterReset();

bool InitInfrared();
bool ResetGR47();
bool ResetWavecom();
bool ResetMC39();
bool ResetCX06833(WORD wModuleIdx);
bool ResetGC864();
bool ResetME3000();
bool ResetMG815();
bool ResetLC6311();
bool PowerOnCW();
bool PowerOffCW();
bool ResetPlc();
bool ResetPlc_GD2016();
BYTE GetLidStatus();

void ModemInit();
bool ModemPowerOn();
void ModemPowerOff();
void ModemPowerOff_GD2016(bool opt);

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
bool IsPowerOffByIoRaw();//ͨ��IO��⽻�������Ƿ��Ѿ�û���ˡ�ԭ���Ľ����û�о�������
bool IsPoweredByDC();
void GprsBatOnOff(bool fOn);
void BackBatOnOff(bool fOn);
void GprsBatCharge(bool fOn);
void PulseInitPrg();
void PulseOutPrg(bool fEnable);
void  ResetEncMoudle(void);
BYTE GetHardVer(void);
void EnableFIR(bool fEnable);
BYTE GetPlcStatus();
void Ctrl_OnOff(bool fOn);
void Debug_485En();
void Debug_485Dis();
BYTE GetModemState();
bool YKInit(TYkPara* pYkPara, BYTE bYKNum);
void YKOpen(WORD wID);
void YKClose(WORD wID);
void YKRun(void);
void AttGpioDisable(void);
bool LocalModuleIsPlugin(void);
bool ModemPowerOn_GD2016(void);
void EthernetLinkCheck(void);
void YK_PowerCtrl(bool bfOn);
void EthernetPhyRegCheck(void);

#endif
