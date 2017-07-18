/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbAPI.h
 * 摘    要：本文件主要实现数据库的公共接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年4月
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

//单个ID按缓冲区的读
int ReadItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime=0, DWORD dwEndTime=0); //指定时间读
int ReadItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD* pdwTime); //读数据同时取数据库中的时标

//多个ID的读接口,目前只支持按缓冲区的读
int ReadItemEx(WORD wBank, WORD wPn, WORD* pwID, WORD wNum, BYTE* pbBuf, DWORD dwStartTime=0, DWORD dwEndTime=0); //按相同测量点相同时间读
int ReadItemEx(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD dwStartTime=0, DWORD dwEndTime=0);	//按相同时间读
int ReadItemEx(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD* pdwTime);	//按不同时间读

//浙江版用的读BANK0接口
inline int ReadItem(WORD wPoint, WORD wID, BYTE* pbBuf)
{						
	return ReadItemEx(BN0, wPoint, wID, pbBuf, (DWORD )0);
}

//版本升级用的读旧版本接口
int UpgReadItem(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD* pdwTime);

//按照非映射的方式读
int ReadItemUnmap(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime=0, DWORD dwEndTime=0);

//浙江版用的写BANK0接口
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

//描述:取得数据项的读地址,主要针对那些内容比较长的数据项,比如测量点屏蔽位等,
//	   免去数据内容拷贝的时间消耗,直接访问只读的内存地址,这样也不会破坏系统库的内容
//返回:如果正确则返回数据项的地址,否则返回NULL
const BYTE* GetItemRdAddr(WORD wBn, WORD wPn, WORD wID);

bool UpdItemErr(WORD wBank, WORD wPn, WORD wID, WORD wErr, DWORD dwTime=0);

//数据项时间的查询查询,看数据项是否在指定时间内更新了
int QueryItemTime(DWORD dwStartTime, DWORD dwEndTime, TBankItem* pBankItem, WORD wNum, WORD* pwValidNum);
					//按照TBankItem的通用做法
int QueryItemTime(DWORD dwStartTime, DWORD dwEndTime, WORD wBn, WORD wPn, WORD* pwID, WORD wNum, WORD* pwValidNum);
					//同一个BANK同一个测量点不同ID的查询
int QueryItemTime(DWORD* pdwStartTime, DWORD* pdwEndTime, TBankItem* pBankItem, WORD wNum, WORD* pwValidNum);
					//按照不同数据项不同时间的查询
 
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

//描述:按照格式串,把数组piVal相应元素的值置为INVALID_VAL
WORD FmtToInvalidVal32(int* piVal, const BYTE* pbFmtStr, WORD* pwValNum);

//描述:按照格式串,把数组piVal相应元素的值置为INVALID_VAL
WORD FmtToInvalidVal64(int64* piVal64, const BYTE* pbFmtStr, WORD* pwValNum);

void TrigerSave();
void TrigerSavePara();

bool InitDbLib(TDbCtrl* pDbCtrl);

#endif //LIBDBAPI_H