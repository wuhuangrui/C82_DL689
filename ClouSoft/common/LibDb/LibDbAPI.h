/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbAPI.h
 * ժ    Ҫ�����ļ���Ҫʵ�����ݿ�Ĺ����ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��4��
 *********************************************************************************************************/
#ifndef LIBDBAPI_H
#define LIBDBAPI_H

#include "apptypedef.h"
#include "sysarch.h"
#include "LibDbStruct.h"
#include "DataManager.h"


TItemDesc* BinarySearchItem(TItemDesc* pItemDesc, WORD num, WORD wID);
int BinarySearchIndex(TItemDesc* pItemDesc, DWORD num, WORD wID);
bool InitItemDesc(TBankCtrl* pBankCtrl);
int ReadItem(WORD wImg, WORD wPn, WORD wID, TItemAcess& rItemAcess, 
			 DWORD dwStartTime, DWORD dwEndTime, TBankCtrl* pBankCtrl);
int WriteItem(WORD wImg, WORD wPn, WORD wID, TItemAcess& rItemAcess, 
			  BYTE bPerm, BYTE* pbPassword, DWORD dwTime,
			  TBankCtrl* pBankCtrl);
void ReadItem(const TDataItem& di, BYTE* pbBuf);
void WriteItem(const TDataItem& di, BYTE* pbBuf);

int WriteItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, BYTE bPerm=0, BYTE* pbPassword=NULL, DWORD dwTime=0);
inline int WriteItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwTime)
{
	return WriteItemEx(wBank, wPn, wID, pbBuf, 0, NULL, dwTime);
}

//����ID���������Ķ�
int ReadItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime=0, DWORD dwEndTime=0); //ָ��ʱ���
int ReadItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD* pdwTime); //������ͬʱȡ���ݿ��е�ʱ��

//���ID�Ķ��ӿ�,Ŀǰֻ֧�ְ��������Ķ�
int ReadItemEx(WORD wBank, WORD wPn, WORD* pwID, WORD wNum, BYTE* pbBuf, DWORD dwStartTime=0, DWORD dwEndTime=0); //����ͬ��������ͬʱ���
int ReadItemEx(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD dwStartTime=0, DWORD dwEndTime=0);	//����ͬʱ���
int ReadItemEx(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD* pdwTime);	//����ͬʱ���

//�㽭���õĶ�BANK0�ӿ�
inline int ReadItem(WORD wPoint, WORD wID, BYTE* pbBuf)
{						
	return ReadItemEx(BN0, wPoint, wID, pbBuf, (DWORD )0);
}

//�汾�����õĶ��ɰ汾�ӿ�
int UpgReadItem(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD* pdwTime);

//���շ�ӳ��ķ�ʽ��
int ReadItemUnmap(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime=0, DWORD dwEndTime=0);

//�㽭���õ�дBANK0�ӿ�
inline int WriteItem(WORD wPoint, WORD wID, BYTE* pbBuf, BYTE bPerm=0, BYTE* pbPassword=NULL)
{
	return WriteItemEx(BN0, wPoint, wID, pbBuf, bPerm, pbPassword, 0);
}

int ReadItemVal(WORD wBank, WORD wPn, WORD wID, int* piVal32, DWORD dwStartTime=0, DWORD dwEndTime=0, WORD* pwValNum=NULL);
int ReadItemVal64(WORD wBank, WORD wPn, WORD wID, int64* piVal64, DWORD dwStartTime=0, DWORD dwEndTime=0, WORD* pwValNum=NULL);

int ReadItemVal(WORD wBank, WORD wPn, WORD wID, int* piVal32, DWORD* pdwTime, WORD* pwValNum=NULL);
int ReadItemVal64(WORD wBank, WORD wPn, WORD wID, int64* piVal64, DWORD* pdwTime, WORD* pwValNum=NULL);

int WriteItemVal(WORD wBank, WORD wPn, WORD wID, int* piVal32, BYTE bPerm=0, BYTE* pbPassword=NULL, DWORD dwTime=0, WORD* pwValNum=NULL);
int WriteItemVal64(WORD wBank, WORD wPn, WORD wID, int64* piVal64, BYTE bPerm=0, BYTE* pbPassword=NULL, DWORD dwTime=0, WORD* pwValNum=NULL);


inline int WriteItemVal(WORD wBank, WORD wPn, WORD wID, int* piVal32, DWORD dwTime, WORD* pwValNum=NULL)
{
	return WriteItemVal(wBank, wPn, wID, piVal32, 0, NULL, dwTime, pwValNum);
}


inline int WriteItemVal64(WORD wBank, WORD wPn, WORD wID, int64* piVal64, DWORD dwTime, WORD* pwValNum=NULL)
{
	return WriteItemVal64(wBank, wPn, wID, piVal64, 0, NULL, dwTime, pwValNum);
}



void ReadItem(const TDataItem& di, BYTE* pbBuf);
void WriteItem(const TDataItem& di, BYTE* pbBuf);

TDataItem GetItemEx(WORD wBank, WORD wPoint, WORD wID);
bool ClearBankData(WORD wBank, WORD wSect, int iFile=-1);
bool DbClrPnData(WORD wBank, WORD wSect, WORD wPnNum, WORD wPn);

int GetItemInfo(WORD wBn, WORD wID, TItemInfo* pItemInfo);
int GetItemLen(WORD wBn, WORD wID);
int GetItemsLen(WORD* pwItemID, WORD wLen);
int GetItemsLen(TBankItem* pBankItem, WORD wNum);
int GetItemPnNum(WORD wBn, WORD wID);

//����:ȡ��������Ķ���ַ,��Ҫ�����Щ���ݱȽϳ���������,�������������λ��,
//	   ��ȥ�������ݿ�����ʱ������,ֱ�ӷ���ֻ�����ڴ��ַ,����Ҳ�����ƻ�ϵͳ�������
//����:�����ȷ�򷵻�������ĵ�ַ,���򷵻�NULL
const BYTE* GetItemRdAddr(WORD wBn, WORD wPn, WORD wID);

bool UpdItemErr(WORD wBank, WORD wPn, WORD wID, WORD wErr, DWORD dwTime=0);

//������ʱ��Ĳ�ѯ��ѯ,���������Ƿ���ָ��ʱ���ڸ�����
int QueryItemTime(DWORD dwStartTime, DWORD dwEndTime, TBankItem* pBankItem, WORD wNum, WORD* pwValidNum);
					//����TBankItem��ͨ������
int QueryItemTime(DWORD dwStartTime, DWORD dwEndTime, WORD wBn, WORD wPn, WORD* pwID, WORD wNum, WORD* pwValidNum);
					//ͬһ��BANKͬһ�������㲻ͬID�Ĳ�ѯ
int QueryItemTime(DWORD* pdwStartTime, DWORD* pdwEndTime, TBankItem* pBankItem, WORD wNum, WORD* pwValidNum);
					//���ղ�ͬ�����ͬʱ��Ĳ�ѯ
 
bool IsImgItem(WORD wBank, WORD wPn, WORD wID);
void SetMeterPnMask(BYTE* pbMeterPnMask);


void LockDB();
void UnLockDB();
bool IsDbLocked();
void TrigerSaveBank(WORD wBank, WORD wSect, int iFile);
void DoTrigerSaveBank();
int SearchPnMap(BYTE bSch, WORD wPn);
int MapToPn(BYTE bSch, WORD wMn);
int NewPnMap(BYTE bSch, WORD wPn);
bool DeletePnMap(BYTE bSch, WORD wPn);
int GetPnMapRealNum(BYTE bSch);
void DbTimeAdjBackward(DWORD dwTime);
void DbNewImg(DWORD dwStartTime, WORD wInterval);

int DbSave(bool fSaveAll=true);
int DbSavePara();
int DbSaveData(bool fSaveAll=true);
void DbDoSave();

//����:���ո�ʽ��,������piVal��ӦԪ�ص�ֵ��ΪINVALID_VAL
WORD FmtToInvalidVal32(int* piVal, const BYTE* pbFmtStr, WORD* pwValNum);

//����:���ո�ʽ��,������piVal��ӦԪ�ص�ֵ��ΪINVALID_VAL
WORD FmtToInvalidVal64(int64* piVal64, const BYTE* pbFmtStr, WORD* pwValNum);

void TrigerSave();
void TrigerSavePara();

bool InitDbLib(TDbCtrl* pDbCtrl);

#endif //LIBDBAPI_H