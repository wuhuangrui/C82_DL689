#ifndef CCTAPI_H
#define CCTAPI_H

#include "CctTaskMangerOob.h"
#include "StdReader.h"

//电表规约类型
#define PRO_TYPE_UNKNOW	0	//未知
#define PRO_TYPE_97		1	//97
#define	PRO_TYPE_07		2	//07
#define PRO_TYPE_69845	3	//698.45
#define PRO_TYPE_T188	4	//T188

//面向对象电表档案，波特率定义
#define OOB_BPS_300			0
#define OOB_BPS_600			1
#define OOB_BPS_1200		2
#define OOB_BPS_2400		3
#define OOB_BPS_4800		4
#define OOB_BPS_7200		5
#define OOB_BPS_9600		6
#define OOB_BPS_19200		7
#define OOB_BPS_38400		8
#define OOB_BPS_57600		9
#define OOB_BPS_115200		10
#define OOB_BPS_OTHER		255

//接线方式
#define OOB_LINE_UNKNOW				0	//未知
#define OOB_LINE_ONE_PHASE			1	//单相
#define OOB_LINE_THREE_PHASE_THREE	2	//三相三线
#define OOB_LINE_THREE_PHASE_FOUR	3	//三相四线


#define GET_REQUEST				0x05
#define GET_REQUEST_NORMAL		0x01
#define GET_REQUEST_RECORD		0x03

#define GET_RESPONSE			0x85
#define GET_RESPONCE_NORMAL		0x01
#define GET_RESPONCE_RECORD		0x03


typedef struct {
	BYTE bTsa[TSA_LEN];
	BYTE bTsaLen;
	BYTE bMtrPro;	//0--unknown, 1--97, 2--07, 3--698.45, 4--T118
}TMtrInfo;



int AarryCompare(BYTE *pbSrc, WORD wSrcLen, BYTE *pbCmp, WORD wCmpLen);

int IntCompare(WORD wSrc1, WORD wSrc2, WORD wLen);

WORD CheckCRC16(BYTE *pInBuf, int iInLen);

BYTE CheckCS(BYTE *pInBuf, WORD wInLen);

int SchFirstMtrSn(const BYTE *pbMsk, WORD wLen);

int SchVlidMtrSn(WORD wMtrSn, const BYTE *pbMsk, WORD wLen);

bool DelMeterInfo(BYTE *pbTsa, BYTE bTsaLen);

bool DelMeterInfo(WORD wPn);

bool GetMeterInfo(BYTE *pbTsa, BYTE bTsaLen, TOobMtrInfo *pTMtrInfo);

bool GetMeterInfo(WORD wPn, TOobMtrInfo *pTMtrInfo);

bool SetMeterInfo(WORD wMtrSn, TOobMtrInfo tTMtrInfo);

bool IsMtrSnValid(WORD wMtrSn);

BYTE GetMeterTsaLen(WORD wMtrSn);

BYTE GetMeterTsa(WORD wMtrSn, BYTE *pbTsa, bool fRev=false);

WORD GetMeterSn(BYTE *pbTsa, BYTE bLen=6);

WORD GetMeterLen(BYTE *pbTsa);

WORD GetMeterPn(BYTE *pbTsa, BYTE bLen=6, bool fRev=false);

int GetMeterAddrLen(WORD wPn);

bool SetMeterTsa(WORD wMtrSn, BYTE *pbTsa, BYTE bMtrLen);

BYTE GetMeterBps(WORD wMtrSn);

bool SetMeterBps(WORD wMtrSn, BYTE bBps);

BYTE GetMeterPro(WORD wMtrSn);

bool SetMeterPro(WORD wMtrSn, BYTE bPro);

BYTE GetMeterPort(WORD wMtrSn, TPORT_PARAM &tTPORT_PARAM);

bool SetMeterPort(WORD wMtrSn, BYTE bPort);

BYTE GetMeterPwd(WORD wMtrSn, BYTE *pbBuf);

bool SetMeterPwd(WORD wMtrSn, BYTE *pbBuf, BYTE bLen);

BYTE GetMeterRate(WORD wMtrSn);

bool SetMeterRate(WORD wMtrSn, BYTE bRate);

BYTE GetMeterUserType(WORD wMtrSn);

bool SetMeterUserType(WORD wMtrSn, BYTE bUserType);

BYTE GetMeterLine(WORD wMtrSn);

bool SetMeterLine(WORD wMtrSn, BYTE bLine);

BYTE GetMeterAcqTsa(WORD wMtrSn, BYTE *pbAcqAddr);

bool SetMeterAcqTsa(WORD wMtrSn, BYTE *pbAcqAddr, BYTE bAcqLen);

BYTE GetMeterAsset(WORD wMtrSn, BYTE *pbAsset);		

bool SetMeterAsset(WORD wMtrSn, BYTE *pbAsset, BYTE bLen);

WORD GetMeterPT(WORD wMtrSn);

bool SetMeterPT(WORD wMtrSn, WORD wPT);

WORD GetMeterCT(WORD wMtrSn);

bool SetMeterCT(WORD wMtrSn, WORD wCT);

DWORD DataTimeToSysTime(BYTE *pbBuf, TTime &tTime);

DWORD TiToSecondes(TTimeInterv *pTI);

void PeriodAdj(TPeriod *p);

int GetTaskCurExeTime(TTaskCfg* pTaskCfg, DWORD* pdwCurSec, DWORD* pdwStartSec, DWORD* pdwEndSec);

int SearchNextPnFromMask(BYTE *pbMtrMask, WORD wPn);

void SetTaskUpdateTime(BYTE bTaskId, DWORD dwExeTime);

DWORD GetTaskUpdateTime(BYTE bTaskId);

void ClearTaskIdUpateTime(BYTE bTaskId);

int CctProxy(BYTE bType, BYTE bChoice, BYTE* bTsa, BYTE bTsaLen, BYTE* pApdu, WORD wApduLen, WORD wTimeOut, BYTE* pbData);

int CctTransmit(BYTE *pbTsa, BYTE bTsaLen, BYTE *pbReqBuf, WORD wReqLen, WORD wTimeOut, BYTE *pbRespBuf);

int MaskToNum(BYTE *pbMask, WORD wLen);

WORD GetPlcNodeAddr(TMtrInfo *pMtrInfo, WORD wMtrNum);

BYTE EncodeRouterMtrAddr(BYTE *pbTsa, BYTE bTsaLen);

BYTE DecodeRouterMtrAddr(BYTE *pbTsa, BYTE bTsaLen);

WORD RouterMtrAddrConvertPn(BYTE *pbTsa);

void GetRooterTermAddr(BYTE *pbTermAddr, BYTE &bTermAddrLen);

void PrintInfo(TRdItem *pRdItem, TMtrPara* pMtrPara);

#endif 