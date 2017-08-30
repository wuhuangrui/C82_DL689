/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbAPI.h
 * 摘    要：本文件主要实现协议相关的数据库标准接口之外的扩展接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年8月
 *********************************************************************************************************/
#ifndef DBAPI_H
#define DBAPI_H
#include "apptypedef.h"
#include "FaCfg.h"
#include "sysarch.h"
#include "sysapi.h"
#include "LibDbAPI.h"
#include "DbHook.h"
#include "DbFmt.h"
#ifdef EN_CCT
#include "DbCctAPI.h"
#endif


extern BYTE g_bTermSoftVer[OOB_SOFT_VER_LEN];
extern BYTE g_bInnerSoftVer[INN_SOFT_VER_LEN];
extern TBankCtrl g_Bank0Ctrl[SECT_NUM];
extern TBankCtrl g_BankCtrl[BANK_NUM];
extern WORD g_wValidPnNum;
//extern TPnMapCtrl g_PnMapCtrl[PNMAP_NUM];

BYTE GetPnProp(WORD wPn);
BYTE GetPnPort(WORD wPn);
BYTE GetPnMtrPro(WORD wPn);
WORD GetValidPnNum();
bool IsPnType(WORD wPn, WORD wType);
bool IsMtrPn(WORD wPn);
bool IsGrpValid(WORD wPn);
bool IsGrpC1Fn(BYTE bFN);
bool IsGrpC2Fn(BYTE bFN);
bool IsDCPnValid(WORD wPn);
bool IsDcC1Fn(BYTE bFn);
bool IsDcC2Fn(BYTE bFn);
BYTE GetErcType(BYTE bErc);
BYTE GetConnectType(WORD wPn);
#ifdef PRO_698
bool IsFnSupport(WORD wPn, BYTE bFn, BYTE bClass);//描述:此测量点是否支持此Fn
BYTE GetUserType(WORD wPn);	//获取用户类型
bool GetUserType(WORD wPn, BYTE* pbMain, BYTE* pbSub); //获取用户用户大类号和小类号
#ifdef EN_SBJC_V2
BYTE GetMeterType(WORD wPn);
BYTE GetMeterSubPro(WORD wPn);
#endif
BYTE GetUserMainType(WORD wPn); //获取用户大类号
BYTE GetUserSubType(WORD wPn); //获取用户小类号
#endif

void UpdPnMask();
void UpdMeterPnMask();
WORD GetAcPn();
void ClrPnData(WORD wPn);
void ClrGrpPnData(WORD wPn);

void SetPnChgMask(WORD wPn);
bool GetPnChgMask(WORD wPn);
void Set485PnMask(WORD wPn);
void Clr485PnMask(WORD wPn);
void ClrPnChgMask();
void SetPulsePnChgMask(WORD wPn, BYTE bBit);
BYTE GetPulsePnChgMask(WORD wPn);
void ClrPulsePnChgMask(WORD wPn);

void SaveSoftVerChg();
bool InitDB(void);


//电表的装置序号与测量点值
void InitMtrSnToPn();
WORD MtrSnToPn(WORD wSn);
void SetMtrSnToPn(WORD wPn, WORD wSn);
WORD MtrPnToSn(WORD wPn);
WORD GetEmptyPn();
WORD DeletSN(WORD wSn);


bool IsMtrFrzSelf(WORD wPn);//电表是否支持自身日冻结
void ClearCurveFrzFlg(WORD wPn);//清除测量点曲线每天的96点设置，用于正常换日时

void SetPnRateNum(WORD wPn, BYTE bRateNum);//设置测量点费率数
BYTE GetPnRateNum(WORD wPn);//获取测量点的费率数
bool IsChgRateNumByMtr();//是否根据电表返回电能数据的实际长度修改测量点的费率数

bool IsSinglePhaseV07Mtr(WORD wPn);
bool IsSinglePhaseV97Mtr(WORD wPn);
BYTE IsSIDV97Mtr(WORD wPn);
DWORD GetMeterBaudRate(WORD wPn);//返回测量点波特率
DWORD NumToBaudrateTest(BYTE n);
WORD GetAcMtrSn();
//////////////////////////////////////////////////////////////////////////
//以下为黑龙江62056协议所用到的辅助接口函数
int AddMtrPara(BYTE* pbBuf);
int DelMtrPara(BYTE* pbBuf);
int AddPlusePara(BYTE* pbBuf);
int DelPlusePara(BYTE* pbBuf);
int AddGrpPara(BYTE* pbBuf);
int DelGrpPara(BYTE* pbBuf);
int AddPointPara(BYTE* pbBuf);
int DelPointPara(BYTE* pbBuf);

bool IsMtrValid(WORD wSn);
bool IsPluseValid(WORD wSn);
bool IsSnParaValid(WORD wID, WORD wSn);
int PnToMtrSn(WORD wPn);
int PnToPluseSn(WORD wPn, WORD* pwSn);
bool IsPointOfGrp(WORD wPn);
bool IsGrpOfCtrl(WORD wPn);
//开机自动应用配置文件
void ApllyCfgAuto(void);
void SetDefaultPara();	//2?êy3?ê??ˉ/??ê??ˉó2?ìoóDè??è?μ?2?êy
WORD GetPnNum();

void SetRdMtrCtrlMask(WORD wPn);
void DoRdMtrCtrlMask();
bool MeterInfoCompare(WORD wPn, BYTE *pbBuf);

void SetSchUpdateMask(WORD wOI, WORD wSchId);
void ClearSchData();

void ClearReportParam();

#endif //DBAPI_H
