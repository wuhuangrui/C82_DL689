/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�MeterPro.h
 * ժ    Ҫ�����ļ���Ҫ��������Э��Ļ���API������ȫ�ֱ����Ķ���
 * ��ǰ�汾��1.0
 * ��    �ߣ������
 * ������ڣ�2016��8��
 * ��    ע��
 *********************************************************************************************************/
#ifndef METERPRO_H
#define METERPRO_H

#include "ComAPI.h"
#include "Trace.h"
#include "DbConst.h"
#include "FaConst.h"
#include "sysdebug.h"
#include "Comm.h"
#include "MeterStruct.h"
#include "DbOIAPI.h"

#define MET_INVALID_VAL		0xffffffff  //��Ч����

#define TXRX_RETRYNUM	2	//�ط�����

#define MTR_FRM_SIZE  512

#define COM_TIMEOUT  1000	//����ȱʡ��ʱ

////////////////////////////////
//���Э��ָ��
struct TMtrPro
{
	TMtrPara* pMtrPara;	//������
	BYTE	bThrId;
	//void*	pvMtrPro;	//����thisָ��
	BYTE*	pbRxBuf; 
	BYTE*	pbTxBuf;
	BYTE*	pbCurve;

	//�����Э���Ա����
	int	(* pfnAskItem)(struct TMtrPro* pMtrPro, BYTE bRespType, DWORD dwOAD,  BYTE* pbData, BYTE* pbRSD, BYTE bLenRSD, BYTE* pbRCSD, BYTE bLenRCSD); //��ȡ���ݵĶ���ӿں���
	int	(* pfnDirAskItem)(struct TMtrPro* pMtrPro, BYTE bRespType, BYTE bChoice, BYTE* pbTx, WORD wTxLen, BYTE* pbData); //����ӿں���

	bool (* pfnRcvBlock)(struct TMtrPro* pMtrPro, void* pTmpInf, BYTE* pbBlock, DWORD dwLen, DWORD dwBufSize); //�������պ���
	void (* pfnGetProPrintType)(BYTE* pbPrintPro, char* pszProName);//��ȡ��ӡЭ������
	int	(* pfnWriteItem)(struct TMtrPro* pMtrPro, DWORD dwOAD, DWORD dwId, WORD wLen); //��ȡ���ݵĶ���ӿں���
}; 

////////////////////////////////////////////////////////////////////////////////////////////
//MtrProIf˽�г�Ա����
//�����շ�����
extern BYTE m_bInvdData;
extern BYTE m_MtrTxBuf[LOGIC_PORT_NUM][MTR_FRM_SIZE];
extern BYTE m_MtrRxBuf[LOGIC_PORT_NUM][MTR_FRM_SIZE];
extern BYTE m_CurveBuf[LOGIC_PORT_NUM][MTR_FRM_SIZE];

/////////////////////////////////////////////////////////////////////////////////////
//�����Э�鹫������
void InitMeterPro();
int SchStrPos(char* pStr, int iStrLen, char c);
WORD GetCRC16(BYTE* pbyt,int iCount);
float POW10(signed char n); //10��n�η�
int64 POW10N(signed char n);//10��n�η���n> 0��
int64 Round(int64 iVal64);  //��������	
bool IsRateId(WORD wID);		//�Ƿ�������йصļ���ID
bool IsDemdId(WORD wID);		//�Ƿ�����ID������������ʱ�䣩
bool IsDemdTime(WORD wID);		//�Ƿ�����ʱ��
bool IsLastMonthId(WORD wID);	//�Ƿ�����ID	
bool IsPhaseEngId(WORD wID);	//�Ƿ�������ID
bool Is645NotSuptId(WORD wID);	//�Ƿ���645��97�棩��֧����Ҫ���ٷ��ص�ID
BYTE GetBlockIdNum(WORD wID);	//ȡ��ID����ID����
BYTE Get645TypeLength(WORD wID);//ȡ645���͵����ݳ���
WORD SetCommDefault(WORD wID, BYTE* pbBuf); //����ͨ�ø�ʽID����Чֵ
void CheckRate(BYTE* pbRateTab, BYTE* pbData, BYTE nlen);	//�����ַ���
void CheckDecimal(BYTE bToltLen, BYTE bItemLen, BYTE bNewDec, BYTE bOldDec, BYTE* pbBuf); //����С��λ	
void CheckDecimalNew(BYTE bDstLen, BYTE bSrcLen, BYTE bDstDec, BYTE bSrcDec, BYTE* pbDstBuf, BYTE* pbSrcBuf);//����ĳ���С��λ
WORD Data645ToComm(WORD wID, BYTE* pbBuf, WORD wLen);//97��645Э���ʽ����ת������ʽ

WORD Id645V07toDL645(WORD wExtId);//����2007��645Э���ȡ����չIDתΪ����Ӧ645ID���Լ���698�ն��ϵĶ�ȡ
WORD Data645to645V07(WORD w645Id, BYTE* pbBuf, WORD wLen);//��97��645���ص�����תΪ2007��64�����ݸ�ʽ���Լ���698�ն��ϵĶ�ȡ
Toad645Map* GetOad645Map(DWORD dwOAD);
Toad645Map* GetOad645ExtMap(DWORD dwOAD);
TErcRdCtrl* GetOad07645ErcMap(DWORD dwOAD);
DWORD GetMtrEvtTimesID(BYTE bBit);
TErcRdCtrl* GetRd07645ErcMap(DWORD dwErcNumID);

/////////////////////////////////////////////////////////////////////////////////////
//���ڲ�������
bool MtrProOpenComm(CComm* pComm, TCommPara* pCommPara);//Ҫ�жϴ����Ƿ��Ѿ���,�����Ƿ���Ҫ�ı�
DWORD MtrProSend(CComm* pComm, BYTE* pbTxBuf, DWORD wLen); //Ҫ��ӷ������ӡ
DWORD MtrProRcv(CComm* pComm, BYTE* pbRxBuf, DWORD dwBufSize); //Ҫ��ӽ������ӡ

/////////////////////////////////////////////////////////////////////////////////////////
//�ڲ��ӿں���
bool ReadCommFrm(struct TMtrPro* pMtrPro, void* pTmpInf, DWORD dwDelayTime, DWORD dwMaxSec, DWORD dwMinSec, DWORD dwTimeOut, 
				 DWORD dwReadLength, DWORD dwBufSize, BYTE* pbBuf, DWORD dwFrmSize);

#endif //METERPRO_H


