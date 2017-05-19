/*********************************************************************************************************
 * Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�CvtExtPro.h
 * ժ    Ҫ�����ļ������˽ӿ�ת������չЭ�����ض���
 * ��ǰ�汾��1.0
 * ��    �ߣ�������
 * ������ڣ�2016��03��
 * ��    ע��
 *********************************************************************************************************/
#ifndef CVTEXTPRO_H
#define CVTEXTPRO_H
#include "stdafx.h"
#include "MeterPro.h"

#define PN_NUM_IN_CVT	  64
#define CVT_MAX_NUM		  10
#define CVT_ADDR_LEN	  6

//#define CVT_MAX_NUM		  1

typedef struct  
{
	BYTE bCvtAddr[6];
	BYTE bPort;
	BYTE bMtrType[PN_NUM_IN_CVT];
	BYTE bMtrAddr[PN_NUM_IN_CVT][7];
	BYTE bPnNum;
	WORD wCrc; //��ΪУ��ͣ�CRCҪ�󵵰�˳��һ�²������
}TCvtInfo;

typedef struct
{
	BYTE bCvtNum;
	BYTE bAllCvtAddr[CVT_MAX_NUM][CVT_ADDR_LEN];
	BYTE bAllPort[CVT_MAX_NUM];
}TAllCvtInfo;

int MakeCvtExtFrm(BYTE* pbAddr, const BYTE* pbDataArea, BYTE bDataAreaLen, BYTE* pbFrm); //��ӿ�ת������չЭ��֡
int DoCvtFwdFunc(BYTE bPort, DWORD dwCvtID, BYTE* pbCvtAddr, BYTE* pbRxFrm, DWORD dwRxBufSize, BYTE* pbTxBuf, WORD wTxBufLen);//485�˿�ת������
int CvtExtHandleFrm(DWORD dwCvtID, BYTE* pbRxFrm, BYTE* pbTxFrm, BYTE* pbDataBuf, DWORD dwRxSize);//�ӿ�ת������չЭ��֡����
void InitTCvtInfo(TCvtInfo *tpCvtInfo); //��ʼ��ת������Ϣ
bool SyncDocsToCvt(TAllCvtInfo *ptAllCvt); //ͬ�������ľ���ʵ��
int SelectDocs(TCvtInfo *tpCvtInfo, BYTE *bpBuf, WORD wBufLen, BYTE *bpAddBuf, BYTE *bpDelBuf, WORD *wpAddBufLen, WORD *wpDelBufLen);//�Ƚϼ�������ת�����ĵ������ҵ�ת������Ҫ���ӵĺ�Ҫɾ���ĵ���
int GetAllDocs(TCvtInfo *tpCvtInfo, BYTE *pbTxBuf); //������е��������
void DoSyncDocs(); //ִ��ͬ������
int CvtRcvBlock(BYTE* pbBlock, DWORD dwLen, BYTE* pbRxBuf, DWORD* pwRxLen, DWORD dwBufSize);//�������Ӵ��ڻ�������������ת������չЭ��ı���
int CheckNumAndCrc(TCvtInfo *tpCvtInfo, BYTE *pbDocNum);//��ѯ����������CRC
int QueryDocs(TCvtInfo *tpCvtInfo, BYTE bDocNum, BYTE *pbRxFrmBuf, BYTE *pbRxBuf, BYTE *pbTxBuf, BYTE *pbDataBuf, DWORD dwRxBufSize); //��ѯ��ת����������е���
int AddDocs(TCvtInfo tCvt, BYTE *pbRxFrmBuf, BYTE *pbRxBuf, BYTE *pbAddBuf, BYTE *pbDataBuf, DWORD dwRxBufSize); //���ӵ���
int DelDocs(TCvtInfo tCvt, BYTE *pbRxFrmBuf, BYTE *pbRxBuf, BYTE *pbDelBuf, BYTE *pbDataBuf, DWORD dwRxBufSize); //ɾ������
int ClearDocs(BYTE bPort, BYTE *pbCvtAddr, BYTE *pbRxFrmBuf, BYTE *pbRxBuf, BYTE *pbTxBuf, BYTE *pbDataBuf, DWORD dwRxBufSize); //��յ���
bool PortInit(); //��ʼ���˿��ź���
int GetCvtNums(TAllCvtInfo *ptAllCvt); //��ȡת�����������͵�ַ��Ϣ
void GetCvtInfo(TCvtInfo *ptCvtInfo, BYTE *pbInfobuf); //�Ӽ�������ͳ�Ƴ�ת�����µ�������Ϣ
int FindCvtAddrToClear(TAllCvtInfo *tBackCvt, const BYTE bBackNum, TAllCvtInfo *ptAllCvt, const BYTE bCmpNum, TAllCvtInfo *tClearCvt); //�ҵ���Ҫ��յ�����ת������ַ����������Ҫ��յ�ת��������

#endif //CVTEXTPRO_H

