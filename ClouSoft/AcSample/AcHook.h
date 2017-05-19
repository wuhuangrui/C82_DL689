/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：AcHook.h
 * 摘    要：本文件主要用来定义交采库的挂钩/回调函数
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年2月
 * 备    注：$本文件主要用来与标准库接口,请不要将不相关的代码加入到本文件
 *			 $本文件定义的函数,形式一致,但是在做不同版本的产品时,函数可能需要修改
 *********************************************************************************************************/
#ifndef ACHOOK_H
#define ACHOOK_H
#include "apptypedef.h"
#include "ComStruct.h"
#include "AcSample.h"

/////////////////////////////////////////////////////////////////////////
//交采库的代码库需要标准的挂钩/回调函数定义
void AcOnClrDemand(WORD wPn0, WORD wPn1);
void AcOnDayFrz(WORD wPn, const TTime& time);
bool AcIsDayFrozen(WORD wPn, const TTime& time);
void AcOnMonthFrz(WORD wPn, const TTime& time, BYTE bIdx);
bool AcIsMonthFrozen(WORD wPn, const TTime& time, BYTE bIdx);
void AcTrigerSavePn(WORD wPn);
void AcTrigerSavePn(WORD wPn0, WORD wPn1);
void AcCaluPhaseStatus(const TAcPara& rAcPara, const int* piValue, const int* piAngle, const int* piCos, BYTE* pbPhaseStatus, WORD* pwPnStatus);
void AcCaluImbalanceRate(const TAcPara& rAcPara, const int* piValue);
void AcTransSaveCurDemand(WORD wId, BYTE* pbValue);
void AcCaluAmpereHours(const TAcPara& rAcPara, const int* piValue, BYTE bOpt);

//描述:设定可编程脉冲输出类型
//参数:无
void PgmPulseInit(); 

//描述:启动可编程脉冲输出
//参数:@wType 脉冲类型,为需量周期、时段投切
void PgmPulseOut(WORD wType);
#endif //ACHOOK_H
