/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：bios.h
 * 摘    要：本文件主要实现系统基本输入输出接口及中断响应程序定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年7月
 * 备    注: 请把实现驱动程序级的函数放到drivers.cpp或另立单独文件,
 *           应用程序不要直接包含本文件,本文件被drivers.h包含,
 *           包含drivers.h就行了
*********************************************************************************************************/
#ifndef  BIOS_H
#define  BIOS_H
#include "apptypedef.h"
#include "at91_gpio.h"
#include "board.h"
#include "DrvStruct.h"

/////////////////////////////////////////////////////////////////////////////////
//基本IO定义
#define GPIO_L	 	_IOW('g', 0, char) 
#define GPIO_H 		_IOW('g', 1, char) 
#define GPIO_O 	  	_IOW('g', 2, char) 
#define GPIO_I 	  	_IOW('g', 3, char) 
#define GPIO_I_P    _IOW('g', 4, char) 
#define GPIO_G		_IOW('g', 5, char) 
#define PRINT_ETH0_STATUS  _IOW('g', 6, char)
#define GET_ETH0_STATUS	   _IOW('g', 7, char)
#define SET_ETH0_STATUS	   _IOW('g', 8, char)

#define LED_POWERCTRL    0//功控
#define LED_ENERGYCTRL   1//电控
#define LED_GUARANTEE    2//保电
#define LED_ALERT        3//告警
//cl790E7
#define LED_RECEIVE      4//接收
#define LED_SEND         5//发送
#define LED_CALL         6//通话
#define LED_RUN          7//运行

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

//输入函数
WORD GetYxInput();
bool IsProgKeyEnable();
bool IsSecretKeyEnable();
BYTE GetModemType();
BYTE GetDoorStatus();//适用于门节点单独由IO脚控制的平台

//电台控制接口函数
bool R230mIsBusy();
void R230MRequestSend();
void R230MEndSend();
void SetCallStatus(BYTE bStatus);//电台通话控制 参数传0/1
void SetTxLed(WORD wDelay);//发送灯控制
void SetRxLed(WORD wDelay);//接收灯控制
void DoTxRxLed();//灯闪烁
void R230mLightTxLed(WORD wLen);
void R230mLightRxLed(WORD wLen);
inline void R230mDarkTxLed();
inline void R230mDarkRxLed();
bool CheckR230Error();

BYTE GetCommRxCn(WORD wPort);
void SetCommTxCn(WORD wPort, BYTE bCn);

////////////////////////////////////////////////////////////////////////////////////
//本平台才有的接口函数定义
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
void SetRunLed(bool fOn);//true 运行灯亮  false  灯灭
void ToggleRunLed();

//蜂鸣器控制函数
extern void ResetBeep(void);
extern void SetBeep(void);
extern void OutBeepMs(unsigned long  Ms);

void EnableCtDetect(bool fEnable);
void InitCtDetect();
void SelectCtCn(unsigned short chn);
void CheckPower();
bool IsPoweredByAC();
bool IsPowerOffByIoRaw();//通过IO检测交流供电是否已经没有了。原生的结果，没有经过消抖
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
