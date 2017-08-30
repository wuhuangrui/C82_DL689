/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbAPI.h
 * ժ    Ҫ�����ļ���Ҫʵ��Э����ص����ݿ��׼�ӿ�֮�����չ�ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��8��
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
bool IsFnSupport(WORD wPn, BYTE bFn, BYTE bClass);//����:�˲������Ƿ�֧�ִ�Fn
BYTE GetUserType(WORD wPn);	//��ȡ�û�����
bool GetUserType(WORD wPn, BYTE* pbMain, BYTE* pbSub); //��ȡ�û��û�����ź�С���
#ifdef EN_SBJC_V2
BYTE GetMeterType(WORD wPn);
BYTE GetMeterSubPro(WORD wPn);
#endif
BYTE GetUserMainType(WORD wPn); //��ȡ�û������
BYTE GetUserSubType(WORD wPn); //��ȡ�û�С���
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


//����װ������������ֵ
void InitMtrSnToPn();
WORD MtrSnToPn(WORD wSn);
void SetMtrSnToPn(WORD wPn, WORD wSn);
WORD MtrPnToSn(WORD wPn);
WORD GetEmptyPn();
WORD DeletSN(WORD wSn);


bool IsMtrFrzSelf(WORD wPn);//����Ƿ�֧�������ն���
void ClearCurveFrzFlg(WORD wPn);//�������������ÿ���96�����ã�������������ʱ

void SetPnRateNum(WORD wPn, BYTE bRateNum);//���ò����������
BYTE GetPnRateNum(WORD wPn);//��ȡ������ķ�����
bool IsChgRateNumByMtr();//�Ƿ���ݵ���ص������ݵ�ʵ�ʳ����޸Ĳ�����ķ�����

bool IsSinglePhaseV07Mtr(WORD wPn);
bool IsSinglePhaseV97Mtr(WORD wPn);
BYTE IsSIDV97Mtr(WORD wPn);
DWORD GetMeterBaudRate(WORD wPn);//���ز����㲨����
DWORD NumToBaudrateTest(BYTE n);
WORD GetAcMtrSn();
//////////////////////////////////////////////////////////////////////////
//����Ϊ������62056Э�����õ��ĸ����ӿں���
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
//�����Զ�Ӧ�������ļ�
void ApllyCfgAuto(void);
void SetDefaultPara();	//2?��y3?��??��/??��??����2?��o��D��??��?��?2?��y
WORD GetPnNum();

void SetRdMtrCtrlMask(WORD wPn);
void DoRdMtrCtrlMask();
bool MeterInfoCompare(WORD wPn, BYTE *pbBuf);

void SetSchUpdateMask(WORD wOI, WORD wSchId);
void ClearSchData();

void ClearReportParam();

#endif //DBAPI_H
