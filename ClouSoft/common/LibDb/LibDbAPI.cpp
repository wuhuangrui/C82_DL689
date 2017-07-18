/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbAPI.cpp
 * 摘    要：本文件主要实现数据库的公共接口
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年4月
 *********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "sysfs.h"
#include "DbHook.h"
#include "DbFmt.h"
#include "LibDbAPI.h"
#include "ComAPI.h"

#include <assert.h>

TSem   g_semDataRW;
TSem   g_semDbSave;
bool g_fLockDB = false;
TTime g_tmAccessDenied;
CDataManager g_DataManager;

TItemDesc* BinarySearchItem(TItemDesc* pItemDesc, WORD num, WORD wID)
{
    int little, big, mid;
	if (wID<pItemDesc[0].wID  || wID>pItemDesc[num-1].wID)
		return NULL;

    little = 0;
    big = num;
    while (little <= big)
    {                               
        mid = (little + big) / 2;       //二分

        if (pItemDesc[mid].wID == wID) 
		{
			return pItemDesc + mid;
		}
		else if (wID > pItemDesc[mid].wID)
        {
          little = mid + 1;
        } 
        else  //if (wID < pItemDesc[mid].wID)
        {
          big = mid - 1;
        }

        mid = (little + big) / 2;
	}

	return NULL;
}


int BinarySearchIndex(TItemDesc* pItemDesc, DWORD num, WORD wID)
{
    int little, big, mid;
	if (wID<pItemDesc[0].wID  || wID>pItemDesc[num-1].wID)
		return -1;

    little = 0;
    big = num;
    while (little <= big)
    {                               
        mid = (little + big) / 2;       //二分

        if (pItemDesc[mid].wID == wID) 
		{
			return mid;
		}
		else if (wID > pItemDesc[mid].wID)
        {
			little = mid + 1;
        } 
        else  //if (wID < pItemDesc[mid].wID)
        {
			big = mid - 1;
        }

        mid = (little + big) / 2;
	}

	return -1;
}


//描述:对数据项描述表进行预初始化,
//	   目前主要是对配置中的一些错误进行修正,比如:
//	   a.块ID中的子ID的测量点个数强制为1;
//	   b.用到测量点动态映射的,测量点个数跟映射方案相同
void PreInitItemDesc(TBankCtrl* pBankCtrl)
{
	TItemDesc* pItemDesc = pBankCtrl->pItemDesc; 
	DWORD num = pBankCtrl->dwItemNum;
	DWORD dwIndex1 = 0;    //1级数据块的开始数据项下标
	DWORD dwIndex2 = 0;    //2级数据块的开始数据项下标
	WORD wID1 = 0;       //1级数据块的数据项标识高三位
	WORD wID2 = 0;       //2级数据块的数据项标识高两位
	int iRealNum;
	DWORD j;

	for (DWORD i=0; i<num; i++)
	{
		if ((pItemDesc[i].wID & 0xff00) != wID2)  //2级数据块的开始
		{                                         //(pItemDesc[i].wID & 0x00ff)== 0
			wID2 = pItemDesc[i].wID & 0xff00;
			dwIndex2 = i;
		}

		if ((pItemDesc[i].wID & 0xfff0) != wID1)  //1级数据块的开始
		{
			wID1 = pItemDesc[i].wID & 0xfff0;
			dwIndex1 = i;
		}

		if (pItemDesc[i].wPnNum == 0)
			pItemDesc[i].wPnNum = 1;

		if ((pItemDesc[i].wRW & DI_CMB) != 0) //组合ID,加上本标志且数据项长度配置为0,则不分配存储空间和时标空间
			pItemDesc[i].wLen = 0;	//组合ID长度强制纠正为0,pItemDesc[i].wPnNum还是保留为正确的个数

		//如果该数据项配置了使用测量点动态映射,则pItemDesc[i].wPnNum调整为映射方案中配置的实际支持的测量点数
		if (pBankCtrl->bPnMapSch==0 && pItemDesc[i].bPnMapSch!=0)
		{
			iRealNum = GetPnMapRealNum(pItemDesc[i].bPnMapSch);
			if (iRealNum > 0)
				pItemDesc[i].wPnNum = (WORD )iRealNum;
		}

		if ((pItemDesc[i].wID & 0x00ff)==0x00ff && pItemDesc[i].wLen==0)  //2级数据块的结束，即数据标识的最低2位为ff
		{
			//块的子ID的测量点数量初始化为1,但块本身保留测量点数量不变,一级块ID的测量点数量也初始化为1
			for (j=dwIndex2; j<i; j++)
			{
				pItemDesc[j].wPnNum = 1;
			}
		}
		else if ((pItemDesc[i].wID & 0x000f)==0x000f && pItemDesc[i].wLen==0)  //1级数据块的结束，即数据标识的最低位为f
		{
			//块的子ID的测量点数量初始化为1,但块本身保留测量点数量不变
			for (j=dwIndex1; j<i; j++)
			{
				pItemDesc[j].wPnNum = 1;
			}
		}
	}
}

//描述:初始化数据项描述表的偏移字段，对于块数据，还初始化数据项长度
//参数:@ pItemDesc 数据项描述表的第一个参数
//      @ num 数据项描述表的个数
//返回:如果成功则返回所描述数据的长度,否则返回-1
//备忘:这里使用了一个假定：数据项标识的最低位是f的数据项表示数据块，它前面肯定要有高位跟它相同，低位不为f的数据项
bool InitItemDesc(TBankCtrl* pBankCtrl)
{
	TItemDesc* pItemDesc = pBankCtrl->pItemDesc; 
	DWORD num = pBankCtrl->dwItemNum;
	DWORD dwOffset1 = 0;   //用来记录1级数据块的开始
	DWORD dwOffset2 = 0;   //用来记录2级数据块的开始
	WORD  wOffset, wOffset1;
	WORD  wLen1 = 0;       //用来累加1级数据块的长度
	WORD  wLen2 = 0;       //用来累加2级数据块的长度
	DWORD dwOffset = 0;
	DWORD dwIndex1 = 0;    //1级数据块的开始数据项下标
	DWORD dwIndex2 = 0;    //2级数据块的开始数据项下标
	WORD wID1 = 0;       //1级数据块的数据项标识高三位
	WORD wID2 = 0;       //2级数据块的数据项标识高两位
    WORD wBlockLen1 = 0;
    WORD wBlockLen2 = 0;
	//int iRealNum;

	DWORD dwBlkIndex1=0, dwBlkIndex2=0;
	BYTE bBlkIndexNum1=0, bBlkIndexNum2=0, bInnerIndex=0;
	DWORD dwIndex = 0;
	WORD wDefaultOffset = 0; //默认值的偏移,每个测量点只存一份
	bool fCmbId;

	PreInitItemDesc(pBankCtrl);

	pBankCtrl->fMutiPnInDesc = false; //描述表中存在多个测量点的描述
	DWORD j;
	WORD wPnNum;
	for (DWORD i=0; i<num; i++)
    {
		pItemDesc[i].bItemFlg = 0;
		if ((pItemDesc[i].wID & 0xff00) != wID2)  //2级数据块的开始
        {                                         //(pItemDesc[i].wID & 0x00ff)== 0
			wID2 = pItemDesc[i].wID & 0xff00;
			dwIndex2 = i;

			dwOffset2 = dwOffset;   //2级的开始
			wLen2 = 0;
			
		    wBlockLen2 = 0;

			dwBlkIndex2 = dwIndex;
			bBlkIndexNum2 = 0;
        }

		if ((pItemDesc[i].wID & 0xfff0) != wID1)  //1级数据块的开始
		{
			wID1 = pItemDesc[i].wID & 0xfff0;
			dwIndex1 = i;

			dwOffset1 = dwOffset;   //1级的开始
			wLen1 = 0;

			wBlockLen1 = 0;

			dwBlkIndex1 = dwIndex;
			bBlkIndexNum1 = 0;
		}

		bBlkIndexNum1++;
		bBlkIndexNum2++;

		if (pItemDesc[i].wPnNum == 0)
			pItemDesc[i].wPnNum = 1;


		if (pItemDesc[i].wPnNum > 1)
			pBankCtrl->fMutiPnInDesc = true; //描述表中存在多个测量点的描述

		fCmbId = (pItemDesc[i].wRW & DI_CMB) != 0; //组合ID,加上本标志且数据项长度配置为0,则不分配存储空间和时标空间

		if ((pItemDesc[i].wID & 0x00ff)==0x00ff && pItemDesc[i].wLen==0 && !fCmbId)  //2级数据块的结束，即数据标识的最低2位为ff
        {
			wPnNum = pItemDesc[i].wPnNum;
			bInnerIndex = 0;
			wOffset = 0;
			wOffset1 = 0;

			//块的子ID的初始化
			for (j=dwIndex2; j<=i; j++)
			{
				pItemDesc[j].wPnNum = wPnNum;
				pItemDesc[j].dwBlockStart = dwIndex2;		//块数据的开始数据项索引,上电时计算
				pItemDesc[j].wBlockLen = wBlockLen2;	//块数据的长度,上电时计算
				pItemDesc[j].dwBlockOffset = dwOffset2;	//块数据的相对于本数据表开头的偏移,上电时计算

				pItemDesc[j].dwBlkIndex = dwBlkIndex2;		 //块在数据库中的起始索引,按照测量点展开后,上电时计算
				pItemDesc[j].bBlkIndexNum = bBlkIndexNum2;	 //块的数据项数,上电时计算
				pItemDesc[j].bInnerIndex = bInnerIndex++;

				pItemDesc[j].bSelfItem = 0;  //本数据项不能自成独立数据项
				
				if (pItemDesc[j].bItemFlg & DI_FLG_BLK)	//一级块ID
				{
					pItemDesc[j].wOffset = wOffset1;	//块内偏移:如果同时存在f和ff数据块,则wOffset是ff数据块的偏移
					wOffset1 = wOffset;	//在2级块中,遇到一个一级块ID时,记下这时候的偏移,作为下一个一级块ID的偏移
				}
				else
				{
					pItemDesc[j].wOffset = wOffset; //pItemDesc[j].dwItemOffset - dwOffset2
					wOffset += pItemDesc[j].wLen;	//这里加单个数据项的长度,不能算多个测量点的长度
				}
			}

			//块ID本身的初始化
			//pItemDesc[i].dwItemOffset = dwOffset2;	//为了给二级块用
            pItemDesc[i].bItemFlg |= DI_FLG_BLK;	//数据项特殊标志位--块ID
			pItemDesc[i].wOffset = 0;				//相对所属数据块开始的偏移,上电时计算
			pItemDesc[i].wLen = wBlockLen2;
			
			pItemDesc[i].bBlkIdIndexNum = bBlkIndexNum2; //块ID自己含有的子数据项的个数

			//pItemDesc[i].wDefaultOffset = wDefaultOffset; //默认值的偏移,每个测量点只存一份
			pItemDesc[i].bSelfItem = 1;  //本数据项能自成独立数据项

			dwOffset += wBlockLen2 * (pItemDesc[i].wPnNum - 1);

			dwIndex++;
			dwIndex += bBlkIndexNum2 * (pItemDesc[i].wPnNum - 1);
        }
		else if ((pItemDesc[i].wID & 0x000f)==0x000f && pItemDesc[i].wLen==0  && !fCmbId)  //1级数据块的结束，即数据标识的最低位为f
		{
			wPnNum = pItemDesc[i].wPnNum;
			bInnerIndex = 0;
			wOffset = 0;

			//块的子ID的初始化
			for (j=dwIndex1; j<=i; j++)
			{
				pItemDesc[j].wPnNum = wPnNum;
				pItemDesc[j].dwBlockStart = dwIndex1;	//块数据的开始数据项索引,上电时计算
				pItemDesc[j].wBlockLen = wBlockLen1;	//块数据的长度,上电时计算
				pItemDesc[j].dwBlockOffset = dwOffset1;	//块数据的相对于本数据表开头的偏移,上电时计算

				pItemDesc[j].dwBlkIndex = dwBlkIndex1;		 //块在数据库中的起始索引,按照测量点展开后,上电时计算
				pItemDesc[j].bBlkIndexNum = bBlkIndexNum1;	 //块的数据项数,上电时计算
				pItemDesc[j].bInnerIndex = bInnerIndex++;

				pItemDesc[j].bSelfItem = 0;		//本数据项不能自成独立数据项
				
				pItemDesc[j].wOffset = wOffset; //pItemDesc[j].dwItemOffset - dwOffset1
				wOffset += pItemDesc[j].wLen;	//这里加单个数据项的长度,不能算多个测量点的长度
			}

			//块ID本身的初始化
			//pItemDesc[i].dwItemOffset = dwOffset1;	//为了给二级块用
            pItemDesc[i].bItemFlg |= DI_FLG_BLK;  //数据项特殊标志位--块ID
			pItemDesc[i].wOffset = 0;		  //相对所属数据块开始的偏移,上电时计算
			pItemDesc[i].wLen = wBlockLen1;
			
			pItemDesc[i].bBlkIdIndexNum = bBlkIndexNum1; //块ID自己含有的子数据项的个数

			//pItemDesc[i].wDefaultOffset = wDefaultOffset; //默认值的偏移,每个测量点只存一份
			pItemDesc[i].bSelfItem = 1;  //本数据项能自成独立数据项

			dwOffset += wBlockLen1 * (pItemDesc[i].wPnNum - 1);  //如果wPnNum为1就不加,
																 //已经在前面加过一份,这里少加一份 	
			dwIndex++;
			dwIndex += bBlkIndexNum1 * (pItemDesc[i].wPnNum - 1); //如果wPnNum为1就不加
																  //已经在前面加过一份,这里少加一份 	
		}
		else    //一般数据项
        {
            pItemDesc[i].wOffset = 0;
			pItemDesc[i].dwBlockStart = 0;
			pItemDesc[i].dwBlockOffset = dwOffset;  //只针对单个的不成块的数据项有效
			//pItemDesc[i].dwItemOffset = dwOffset;
			pItemDesc[i].wBlockLen = pItemDesc[i].wLen; //只针对单个的不成块的数据项有效

			pItemDesc[i].dwBlkIndex = dwIndex;		 //块在数据库中的起始索引,按照测量点展开后,上电时计算
			pItemDesc[i].bBlkIndexNum = 1;	 //块的数据项数,上电时计算
			pItemDesc[i].bBlkIdIndexNum = 1; //块ID自己含有的子数据项的个数
			pItemDesc[i].bInnerIndex = 0;

			if (!fCmbId)
			{
				dwOffset += pItemDesc[i].wLen * pItemDesc[i].wPnNum; //一般都只加1个测量点的长度
				wBlockLen1 += pItemDesc[i].wLen;
				wBlockLen2 += pItemDesc[i].wLen;
				
				dwIndex += pItemDesc[i].wPnNum;  //单独数据项支持多个测量点的就直接加wPnNum,
												 //一般都只加1
				
				//pItemDesc[i].wDefaultOffset = wDefaultOffset; //默认值的偏移,每个测量点只存一份
				wDefaultOffset += pItemDesc[i].wLen; 
				pItemDesc[i].bSelfItem = 1;  //本数据项先默认为能自成独立数据项
			}
			else
			{
				pItemDesc[i].bSelfItem = 0;  //组合ID不能自成独立数据项
			}
		}
	}

	pBankCtrl->dwBankSize = dwOffset;
	pBankCtrl->dwIndexNum = dwIndex;    //数据项索引的个数,即dwItemNum按照测量点个数展开后的个数
	if (pBankCtrl->dwDefaultSize!=0 && pBankCtrl->dwDefaultSize!=wDefaultOffset)
	{
		DTRACE(DB_DB, ("InitItemDesc: error:  <%s> default len mismatch : default = %d , calcu size = %d.\r\n", 
						pBankCtrl->pszBankName, pBankCtrl->dwDefaultSize, wDefaultOffset));
		return false;
	}

	return true;
}

//描述：更新数据项时标
inline void UpdItemTime(WORD wPn, WORD wID, WORD wImg, TBankCtrl* pBankCtrl, TItemDesc* pItemDesc, DWORD dwTime)
{
	DWORD dwTimeIndex;
	DWORD i;

	if (pBankCtrl->pdwUpdTime != NULL) // && dwTime!=0需要更新时间INVALID_TIME
	{
		if (pBankCtrl->wPnNum > 1) //按照整个BANK来支持测量点
			dwTimeIndex = pBankCtrl->dwIndexNum*(pBankCtrl->wPnNum*wImg + wPn) + 
						  pItemDesc->dwBlkIndex + pItemDesc->bInnerIndex; //nIndex;
		else
			dwTimeIndex = pItemDesc->dwBlkIndex + wPn*pItemDesc->bBlkIndexNum + 
						  pItemDesc->bInnerIndex;
			//按照测量点展开后一个数据项的索引:dwBlkIndex+测量点*bBlkIndexNum+bInnerIndex

		if ((wID&0x000f)==0x000f && pItemDesc->bBlkIdIndexNum>1)	//块ID,需要更新子项的时间
		{									//块ID自己含有的子数据项的个数
			i = dwTimeIndex - (pItemDesc->bBlkIdIndexNum - 1);
			for (; i<=dwTimeIndex; i++)
			{
				pBankCtrl->pdwUpdTime[i] = dwTime; //一条指令就能写入的数据不用信号量保护
			}
		}
		else
		{
			pBankCtrl->pdwUpdTime[dwTimeIndex] = dwTime; //一条指令就能写入的数据不用信号量保护
		}
	}
}

void SetModifiedFlg(TBankCtrl* pBankCtrl, WORD wPn)
{
	//数据文件的修改标志,按照测量点
	if (pBankCtrl->wFileNum > 1)
	{
		WORD wByte = wPn >> 3; //除8
		BYTE bMask = 1 << (wPn & 0x07);
		pBankCtrl->bModified[wByte] |= bMask;
	}
	else
	{
		pBankCtrl->bModified[0] |= 0x01;
	}
}

//返回:如果正确则返回数据的长度，如果错误则低8位返回错误代码，次低8位返回数据的长度
int WriteItem(WORD wImg, WORD wPn, WORD wID, TItemAcess& rItemAcess, 
			  BYTE bPerm, BYTE* pbPassword, DWORD dwTime,
			  TBankCtrl* pBankCtrl)
{
	DWORD i;
	if (pBankCtrl->pItemDesc == NULL)
		return -ERR_ITEM;

	int nIndex = BinarySearchIndex(pBankCtrl->pItemDesc, pBankCtrl->dwItemNum, wID);
	if (nIndex < 0)
	    return -ERR_ITEM;

	if (pBankCtrl->pbBankData == NULL) //本BANK的只作为数据项描述用,真正的数据访问要靠相应的读写函数
		return -ERR_ITEM;

	if (pBankCtrl->wPnNum==0 && pBankCtrl->wImgNum==0)
		return -ERR_ITEM; //本BANK的只作为数据项描述用,只支持DI_ACESS_INFO的ReadItem()访问

	TItemDesc* pItemDesc = &pBankCtrl->pItemDesc[nIndex];

	if ((pItemDesc->wRW & DI_CMB) != 0) //组合ID不能进行后续的访问
		return -ERR_ITEM;

	//测量点到实际存储号(映射号)的转换
	int iPn;
	BYTE bPnMapSch = 0;
	if (pBankCtrl->bPnMapSch != 0)
		bPnMapSch = pBankCtrl->bPnMapSch;
	else if (pItemDesc->bPnMapSch != 0)
		bPnMapSch = pItemDesc->bPnMapSch;

	if (bPnMapSch != 0)
	{
		iPn = SearchPnMap(bPnMapSch, wPn);
		if (iPn < 0)
		{
			DTRACE(DB_DB, ("WriteItem: pnmap not found, wPn=%d, sch=%d, wID=%04x\n", 
						   wPn, bPnMapSch, wID));

			return -ERR_ITEM; //return -(ERR_PNUNMAP + (int )pItemDesc->wLen*0x100);
		}

		wPn = (WORD )iPn;
	}

	if (wPn>=pItemDesc->wPnNum && wPn>=pBankCtrl->wPnNum) //本数据项不支持那么多个测量点
		return -ERR_ITEM;

	int nRet = PermCheck(pItemDesc, bPerm, pbPassword);
	if (nRet != ERR_OK)
		return nRet;

	BYTE* pbItemAddr;
	if (pBankCtrl->wPnNum>1 || pBankCtrl->wImgNum>1) //按照整个BANK来支持测量点
	{
		pbItemAddr = pBankCtrl->pbBankData + pBankCtrl->dwBankSize*(pBankCtrl->wPnNum*wImg + wPn) + 
					 pItemDesc->dwBlockOffset + pItemDesc->wOffset;
	}
	else
	{
		pbItemAddr = pBankCtrl->pbBankData + pItemDesc->dwBlockOffset + 
					 wPn*pItemDesc->wBlockLen + pItemDesc->wOffset;
					//一个数据项的偏移:dwBlockOffset+测量点*wBlockLen+wOffset
	}

	if (rItemAcess.bType == DI_ACESS_BUF)
	{
		WaitSemaphore(pBankCtrl->semBankRW);
		memcpy(pbItemAddr, rItemAcess.pbBuf, pItemDesc->wLen);
		UpdItemTime(wPn, wID, wImg, pBankCtrl, pItemDesc, dwTime);	//更新数据项时标
		SetModifiedFlg(pBankCtrl, wPn);
		SignalSemaphore(pBankCtrl->semBankRW);
	}
	else if (rItemAcess.bType == DI_ACESS_UPD) //更新数据项状态
	{
		WaitSemaphore(pBankCtrl->semBankRW);
		memset(pbItemAddr, GetInvalidData((BYTE )rItemAcess.dwVal), pItemDesc->wLen);
		UpdItemTime(wPn, wID, wImg, pBankCtrl, pItemDesc, dwTime);	//更新数据项时标
		SetModifiedFlg(pBankCtrl, wPn);
		SignalSemaphore(pBankCtrl->semBankRW);
	}
	else if (rItemAcess.bType==DI_ACESS_INT32 || rItemAcess.bType==DI_ACESS_INT64)	//求值
	{
		BYTE bBuf[300];		//不能超过256个字节
		
		TItemDesc* pItem = pItemDesc;
		WORD wBlkIndexNum = 1;
		if ((wID&0x000f)==0x000f && pItemDesc->bBlkIdIndexNum>1)
		{							//块ID自己含有的子数据项的个数
			wBlkIndexNum = pItemDesc->bBlkIdIndexNum - 1;
			pItem = pItemDesc - wBlkIndexNum;
		}

		BYTE* pb = bBuf;
		WORD wValNum;
		rItemAcess.wValNum = 0;		//按值读写时数值的个数
		rItemAcess.wValidNum = 0; //对于按值访问的情况,有效个数对应整形个数

		if (rItemAcess.bType == DI_ACESS_INT32) //按照整形32位读写
		{
			int* piVal32 = rItemAcess.piVal32;
			for (i=0; i<wBlkIndexNum; i++)
			{
				if (pItem->pbFmtStr != NULL)
				{	
					rItemAcess.wValidNum += ValToFmt(piVal32, pb, pItem->pbFmtStr, &wValNum);  //复合数据格式
					rItemAcess.wValNum += wValNum;
					piVal32 += wValNum;
				}
				else       
				{
					rItemAcess.wValNum++;
				
					if (*piVal32 == INVALID_VAL)
					{			
						memset(pb, GetInvalidData(ERR_OK), pItem->wLen);							
					}
					else
					{
						ValToFmt(*piVal32, pb, pItem->wFormat, pItem->wLen); //单一数据格式
					}
					piVal32++;
					rItemAcess.wValidNum++;	//即时是复合格式数据项也只当成一个数据项
				}
				
				pb += pItem->wLen;
				pItem++;
			}
		}
		else //按照整形64位读写
		{
			int64* piVal64 = rItemAcess.piVal64;
			for (i=0; i<wBlkIndexNum; i++)
			{
				if (pItem->pbFmtStr != NULL)
				{
					rItemAcess.wValidNum += Val64ToFmt(piVal64, pb, pItem->pbFmtStr, &wValNum);   //复合数据格式
					rItemAcess.wValNum += wValNum;
					piVal64 += wValNum;
				}
				else
				{
					rItemAcess.wValNum++;

					if ( *piVal64 == INVALID_VAL64 )
					{			
						memset(pb, GetInvalidData(ERR_OK), pItem->wLen);							
					}
					else
					{
						Val64ToFmt(*piVal64, pb, pItem->wFormat, pItem->wLen);
					}
					piVal64++;
					rItemAcess.wValidNum++; //即时是复合格式数据项也只当成一个数据项
				}
				
				pb += pItem->wLen;
				pItem++;
			}
		}
		WaitSemaphore(pBankCtrl->semBankRW);
		memcpy(pbItemAddr, bBuf, pItemDesc->wLen);
		UpdItemTime(wPn, wID, wImg, pBankCtrl, pItemDesc, dwTime);	//更新数据项时标
		SetModifiedFlg(pBankCtrl, wPn);
		SignalSemaphore(pBankCtrl->semBankRW);
	}

	//TODO：编程日志

	if (pItemDesc->wWrOp != INFO_NONE) // && pbPassword!=NULL
	{
		SetDelayInfo(pItemDesc->wWrOp);
	}	
	
	return pItemDesc->wLen;
}


int ReadItem(WORD wImg, WORD wPn, WORD wID, TItemAcess& rItemAcess, 
			 DWORD dwStartTime, DWORD dwEndTime, 
			 TBankCtrl* pBankCtrl)
{
	int* piVal32;
	int64* piVal64;
	WORD wValNum;

	if (pBankCtrl->pItemDesc == NULL)
		return -ERR_ITEM;
		
    int nIndex = BinarySearchIndex(pBankCtrl->pItemDesc, pBankCtrl->dwItemNum, wID);
	if (nIndex < 0)
	    return -ERR_ITEM;

	TItemDesc* pItemDesc = &pBankCtrl->pItemDesc[nIndex];

	if ((pItemDesc->wRW & DI_CMB) != 0) //组合ID不能进行后续的访问
		return -ERR_ITEM;

	if (rItemAcess.bType == DI_ACESS_INFO)	//取数据项长度
	{	//如果wPnNum和wImgNum同时配置为0,表示本BANK的只作为数据项描述用
		if (pBankCtrl->wPnNum > 1)
			rItemAcess.pItemInfo->wPnNum = pBankCtrl->wPnNum;
		else
			rItemAcess.pItemInfo->wPnNum = pItemDesc->wPnNum;

		rItemAcess.pItemInfo->wLen = pItemDesc->wLen;
		return pItemDesc->wLen;
	}

	if (pBankCtrl->pbBankData == NULL) //本BANK的只作为数据项描述用,真正的数据访问要靠相应的读写函数
		return -ERR_ITEM;

	if (pBankCtrl->wPnNum==0 && pBankCtrl->wImgNum==0)
		return -ERR_ITEM; //本BANK的只作为数据项描述用,只支持DI_ACESS_INFO的ReadItem()访问

	//测量点到实际存储号(映射号)的转换
	int iPn;
	BYTE bPnMapSch = 0;
	if (pBankCtrl->bPnMapSch != 0)
		bPnMapSch = pBankCtrl->bPnMapSch;
	else if (pItemDesc->bPnMapSch != 0)
		bPnMapSch = pItemDesc->bPnMapSch;

	if (bPnMapSch!=0 && rItemAcess.bType!=DI_ACESS_RDUNMAP)	//按照非映射的方式读
	{
		iPn = SearchPnMap(bPnMapSch, wPn);
		if (iPn < 0)
		{
			DTRACE(DB_DB, ("ReadItem: pnmap not found, wPn=%d, sch=%d, wID=%04x\n", 
						   wPn, bPnMapSch, wID));
			
			return -ERR_ITEM; //-(ERR_PNUNMAP + (int )pItemDesc->wLen*0x100);
		}

		wPn = (WORD )iPn;
	}

	if (wPn>=pItemDesc->wPnNum && wPn>=pBankCtrl->wPnNum) //本数据项不支持那么多个测量点
		return -ERR_ITEM;
			
	DWORD i;
	TItemDesc* pItem = pItemDesc;
	WORD wBlkIndexNum = 1;
	if ((wID&0x000f)==0x000f && pItemDesc->bBlkIdIndexNum>1)
	{							//块ID自己含有的子数据项的个数
		wBlkIndexNum = pItemDesc->bBlkIdIndexNum - 1;
		pItem = pItemDesc - wBlkIndexNum;
	}

	DWORD dwTimeIndex;
	if (pBankCtrl->pdwUpdTime!=NULL && (pItemDesc->wRW&DI_NTS)==0)
	{ //整个BANK不支持时标			&&  本数据项不支持时标
		if (pBankCtrl->wPnNum > 1) //按照整个BANK来支持测量点
			dwTimeIndex = pBankCtrl->dwIndexNum*(pBankCtrl->wPnNum*wImg + wPn) + 
						  pItemDesc->dwBlkIndex + pItemDesc->bInnerIndex; //nIndex;
		else
			dwTimeIndex = pItemDesc->dwBlkIndex + wPn*pItemDesc->bBlkIndexNum + 
						  pItemDesc->bInnerIndex;
			//按照测量点展开后一个数据项的索引:dwBlkIndex+测量点*bBlkIndexNum+bInnerIndex

		if (rItemAcess.pdwTime != NULL)
			*rItemAcess.pdwTime = pBankCtrl->pdwUpdTime[dwTimeIndex];
		
		if (dwStartTime == 0)
			dwEndTime = 0;

		if ((dwStartTime!=0 && pBankCtrl->pdwUpdTime[dwTimeIndex]<dwStartTime)  //数据项的时间在要求的时间前，不符合要求
			|| (dwEndTime!=0 && pBankCtrl->pdwUpdTime[dwTimeIndex]>=dwEndTime))
		{
			if (rItemAcess.bType==DI_ACESS_BUF || rItemAcess.bType==DI_ACESS_RDUNMAP)
			{
				memset(rItemAcess.pbBuf, GetInvalidData(ERR_OK), pItemDesc->wLen);
			}
			else if (rItemAcess.bType == DI_ACESS_INT32) //按照整形32位读,把全部数据置为INVALID_VAL
			{
				rItemAcess.wValNum = 0;		//按值读写时数值的个数
				rItemAcess.wValidNum = 0;	//对于按值访问的情况,有效个数对应整形个数

				piVal32 = rItemAcess.piVal32;
				for (i=0; i<wBlkIndexNum; i++)
				{
					if (pItem->pbFmtStr != NULL)
					{	
						FmtToInvalidVal32(piVal32, pItem->pbFmtStr, &wValNum);
						rItemAcess.wValNum += wValNum;
						piVal32 += wValNum;
					}
					else //单一数据格式，只考虑含一个数据
					{
						rItemAcess.wValNum++;
						*piVal32 = INVALID_VAL;
						piVal32++;    //为组合数据项的下一个ID做准备
					}	

					pItem++;
				}
			}
			else if (rItemAcess.bType == DI_ACESS_INT64) //按照整形64位读,把全部数据置为INVALID_VAL64
			{
				rItemAcess.wValNum = 0;		//按值读写时数值的个数
				rItemAcess.wValidNum = 0;	//对于按值访问的情况,有效个数对应整形个数

				piVal64 = rItemAcess.piVal64;
				for (i=0; i<wBlkIndexNum; i++)
				{
					if (pItem->pbFmtStr != NULL)
					{
						FmtToInvalidVal64(piVal64, pItem->pbFmtStr, &wValNum);
						rItemAcess.wValNum += wValNum;
						piVal64 += wValNum;
					}
					else
					{
						rItemAcess.wValNum++;
						*piVal64 = INVALID_VAL64;			
						piVal64++;                  //为组合数据项的下一个ID做准备
					}

					pItem++;
				}
			}

			return -(ERR_TIME + (int )pItemDesc->wLen*0x100);
		}
	}

	BYTE* pbItemAddr;
	if (pBankCtrl->wPnNum > 1) //按照整个BANK来支持测量点
	{
		pbItemAddr = pBankCtrl->pbBankData + pBankCtrl->dwBankSize*(pBankCtrl->wPnNum*wImg + wPn) + 
					 pItemDesc->dwBlockOffset + pItemDesc->wOffset;
	}
	else
	{
		pbItemAddr = pBankCtrl->pbBankData + pItemDesc->dwBlockOffset + 
					 wPn*pItemDesc->wBlockLen + pItemDesc->wOffset;
					//一个数据项的偏移:dwBlockOffset+测量点*wBlockLen+wOffset
	}
	
	if (rItemAcess.bType==DI_ACESS_BUF || rItemAcess.bType==DI_ACESS_RDUNMAP)
	{
		WaitSemaphore(pBankCtrl->semBankRW);
		memcpy(rItemAcess.pbBuf, pbItemAddr, pItemDesc->wLen);
		SignalSemaphore(pBankCtrl->semBankRW);
	}
	else if (rItemAcess.bType==DI_ACESS_INT32 || rItemAcess.bType==DI_ACESS_INT64)	//求值
	{
		BYTE bBuf[300];		//不能超过256个字节
		WaitSemaphore(pBankCtrl->semBankRW);
		memcpy(bBuf, pbItemAddr, pItemDesc->wLen);
		SignalSemaphore(pBankCtrl->semBankRW);
		
		BYTE* pb = bBuf;
		rItemAcess.wValNum = 0;		//按值读写时数值的个数
		rItemAcess.wValidNum = 0;	//对于按值访问的情况,有效个数对应整形个数

		if (rItemAcess.bType == DI_ACESS_INT32) //按照整形32位读
		{
			piVal32 = rItemAcess.piVal32;
			for (i=0; i<wBlkIndexNum; i++)
			{
				if (pItem->pbFmtStr != NULL)
				{	
					rItemAcess.wValidNum += FmtToVal(pb, piVal32, pItem->pbFmtStr, &wValNum);
					rItemAcess.wValNum += wValNum;
					piVal32 += wValNum;
				}
				else //单一数据格式，只考虑含一个数据
				{
					rItemAcess.wValNum++;

					if (IsInvalidData(pb, pItem->wLen))
					{			
						*piVal32 = INVALID_VAL;			
					}
					else
					{
						*piVal32 = FmtToVal(pb, pItem->wFormat, pItem->wLen);
					}
					if (*piVal32 != INVALID_VAL)
						rItemAcess.wValidNum++;
					piVal32++;    //为组合数据项的下一个ID做准备
				}	

				pb += pItem->wLen;
				pItem++;
			}
		}
		else //按照整形64位读
		{
			piVal64 = rItemAcess.piVal64;
			for (i=0; i<wBlkIndexNum; i++)
			{
				if (pItem->pbFmtStr != NULL)
				{
					rItemAcess.wValidNum += FmtToVal64(pb, piVal64, pItem->pbFmtStr, &wValNum);
					rItemAcess.wValNum += wValNum;
					piVal64 += wValNum;
				}
				else
				{
					rItemAcess.wValNum++;

					if (IsInvalidData(pb, pItem->wLen))
					{			
						*piVal64 = INVALID_VAL64;			
					}
					else
					{
						*piVal64 = FmtToVal64(pb, pItem->wFormat, pItem->wLen);
					}

					if (*piVal64 != INVALID_VAL64)
						rItemAcess.wValidNum++;
					piVal64++;                  //为组合数据项的下一个ID做准备
				}

				pb += pItem->wLen;
				pItem++;
			}
		}
	}
	else if (rItemAcess.bType == DI_ACESS_QRY) //查询数据项是否更新,什么都不用干
	{
		bool fInvalid = false;

		WaitSemaphore(pBankCtrl->semBankRW);
		fInvalid = IsInvalidData(pbItemAddr, pItemDesc->wLen);
		SignalSemaphore(pBankCtrl->semBankRW);

		if (fInvalid)
			return -(ERR_INVALID + (int )pItemDesc->wLen*0x100);
	}
	else if (rItemAcess.bType == DI_ACESS_GI)
	{
		TDataItem* pDI = (TDataItem* )rItemAcess.pbBuf;
		pDI->pbAddr = pbItemAddr;
		pDI->wLen = pItemDesc->wLen;
		pDI->pBankCtrl = pBankCtrl;
		
		//数据文件的修改标志,按照测量点
		if (pBankCtrl->wFileNum > 1)
		{
			WORD wByte = wPn >> 3; //除8
			BYTE bMask = 1 << (wPn & 0x07);
			pDI->pbModified = &pBankCtrl->bModified[wByte];
			pDI->bModifiedMask = bMask;
		}
		else
		{
			pDI->pbModified = &pBankCtrl->bModified[0];
			pDI->bModifiedMask = 0x01;
		}
	}

	return pItemDesc->wLen;
}


void ReadItem(const TDataItem& di, BYTE* pbBuf)
{
	if (di.pbAddr == NULL)
		return;

    WaitSemaphore(di.pBankCtrl->semBankRW);
	memcpy(pbBuf, di.pbAddr, di.wLen);
	SignalSemaphore(di.pBankCtrl->semBankRW);
}


void WriteItem(const TDataItem& di, BYTE* pbBuf)
{
	if (IsDbLocked())
		return;

	if (di.pbAddr == NULL)
		return;

    WaitSemaphore(di.pBankCtrl->semBankRW);
	memcpy(di.pbAddr, pbBuf, di.wLen);
	if (di.pbModified != NULL)
	{
		*(di.pbModified) |= di.bModifiedMask;
	}
	SignalSemaphore(di.pBankCtrl->semBankRW);
}


//描述：通常是用来在抄到电表不支持的数据项或抄表失败的时候,更新数据项时间,
//		以加快应用查询数据的及时性
//参数：@wErr 错误代码,ERR_UNSUP电表不支持的数据项,ERR_FAIL抄表失败
//备注：目前只支持到更新数据项时间,不支持更新错误代码
bool UpdItemErr(WORD wBank, WORD wPn, WORD wID, WORD wErr, DWORD dwTime)
{
	if (wBank != BN0)
		return true;

	TItemAcess ItemAcess;
	ItemAcess.bType = DI_ACESS_UPD;	//更新数据项状态
	ItemAcess.dwVal = wErr;

	return g_DataManager.WriteItemEx(wBank, wPn, wID, ItemAcess, DI_LOW_PERM, NULL, dwTime)>0;
}


//描述：查询数据项是否在dwStartTime后被更新过
//参数：@dwStartTime 抄表间隔的起始时间,从2000年1月1日0点0分0秒算起的秒
//					 注意与SubmitMeterReq()的dwStartTime不尽相同
//		@dwEndTime   小于相应的结束时间
//      @pBank0Item 指向数据项数组的指针
//      @wNum 数组元素的个数
//		@pwValidNum 用来返回合法数据项的个数
//返回：已经被确认的数据项个数,包括已经抄到,确认不支持的或确认抄不到的数据项
//备注：数据项被更新过不一定代表数据内容合法,电表不支持或抄表失败的数据项可以
//		用UpdItemErr()更新时间,以加快应用查询数据的及时性
int QueryItemTime(DWORD dwStartTime, DWORD dwEndTime, TBankItem* pBankItem, WORD wNum, WORD* pwValidNum)
{
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_QRY;

	int iRet;
	int iConfirmNum = 0;
	WORD wValidNum = 0;
	for (WORD i=0; i<wNum; i++)
	{
		WORD* pwSubID = CmbToSubID(pBankItem->wBn, pBankItem->wID);
		if (pwSubID == NULL)
		{
			iRet = g_DataManager.ReadItemEx(pBankItem->wBn, pBankItem->wPn, pBankItem->wID, 
											ItemAcess, dwStartTime, dwEndTime);
		}
		else
		{
			int iLen = 0;	//用来计算组合数据项的长度
			WORD wID;
			bool fInvalid = false;
			iRet = 0;

			while ((wID=*pwSubID++) != 0)	//把组合ID转换成依次对子ID的读
			{
				iLen = g_DataManager.ReadItemEx(pBankItem->wBn, pBankItem->wPn, wID,
												ItemAcess, dwStartTime, dwEndTime);
				
				if (iLen > 0)
				{
					iRet += iLen;
				}
				else if (iLen < 0)
				{
					iLen = -iLen;	//-(ERR_TIME + (int )pItemDesc->wLen*0x100);
					if ((iLen&0xff) != ERR_INVALID)  //时间被更新了,但数据项内容无效,比如电表不支持的数据项或抄表失败等
					{
						//错误代码如果返回ERR_ITEM/ERR_TIME等错误,该组合ID的剩余ID就不用再查询了,
						//因为该数据项已经彻底不符合要求了
						fInvalid = false;	//其它错误,不能归为ERR_INVALID
						iRet = -iLen;
						break;	
					}
					
					iRet += (iLen>>8);
					fInvalid = true;
				}
				else //iLen==0
				{
					fInvalid = false;	//其它错误,不能归为ERR_INVALID
					iRet = 0;
					break;
				}
			}

			if (fInvalid && wID==0)	//全部子数据项查完后,确定只剩下ERR_INVALID错误
				iRet = -(ERR_INVALID + iLen*0x100);
		}

		if (iRet > 0)
		{
			iConfirmNum++;
			wValidNum++;
		}
		else
		{
			iRet = (-iRet) & 0xff;	  //取错误代码
			if (iRet == ERR_INVALID)  //时间被更新了,但数据项内容无效,比如电表不支持的数据项或抄表失败等
				iConfirmNum++;
			else if (iRet == ERR_ITEM)
			{
				*pwValidNum = 0;
				return -ERR_ITEM;
			}
		}

		pBankItem++;
	}

	*pwValidNum = wValidNum;
	return iConfirmNum;
}

//描述:按照不同数据项不同时间的查询,看数据项是否在pdwStartTime后被更新过
//参数:@pdwStartTime 数据项时间的数组
int QueryItemTime(DWORD* pdwStartTime, DWORD* pdwEndTime, TBankItem* pBankItem, WORD wNum, WORD* pwValidNum)
{
	int iRet = 0;
	WORD wValidRet=0, wValidNum=0;
	for (WORD i=0; i<wNum; i++,pBankItem++)
	{
		int iConfirmNum = QueryItemTime(*pdwStartTime++, *pdwEndTime++, pBankItem, 1, &wValidNum);
		if (iConfirmNum == -ERR_ITEM)
		{
			*pwValidNum = 0;
			return -ERR_ITEM;
		}

		iRet += iConfirmNum;
		wValidRet += wValidNum;
	}
	
	*pwValidNum = wValidRet;
	return iRet;
}

int QueryItemTime(DWORD dwStartTime, DWORD dwEndTime, WORD wBn, WORD wPn, WORD* pwID, WORD wNum, WORD* pwValidNum)
{
	TBankItem BankItem[50];
	int iRet = 0;
	WORD wValidRet=0, wValidNum=0;
	while (wNum > 0)
	{
		WORD n = wNum>=50 ? 50 : wNum;

		for (WORD i=0; i<n; i++)
		{
			BankItem[i].wBn = wBn;  	//测量点号
			BankItem[i].wPn = wPn;  	//测量点号
			BankItem[i].wID = *pwID++;  //BN0或者645ID
		}

		int iConfirmNum = QueryItemTime(dwStartTime, dwEndTime, BankItem, n, &wValidNum);
		if (iConfirmNum == -ERR_ITEM)
		{
			*pwValidNum = 0;
			return -ERR_ITEM;
		}

		iRet += iConfirmNum;
		wValidRet += wValidNum;

		wNum -= n;
	}

	*pwValidNum = wValidRet;

	return iRet;
}

int WriteItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, BYTE bPerm, BYTE* pbPassword, DWORD dwTime)
{
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_BUF;
	ItemAcess.pbBuf = pbBuf;
	
	int iRet = g_DataManager.WriteItemEx(wBank, wPn, wID, ItemAcess, bPerm, pbPassword, dwTime);
	if (iRet > 0)
	{
		iRet = PostWriteItemExHook(wBank, wPn, wID, pbBuf, bPerm, pbPassword, iRet); //调用挂钩进行应用特殊处理
	}
		 
	return iRet;
}

int ReadItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	int iRet;
	WORD* pwSubID = CmbToSubID(wBank, wID);
	TItemAcess ItemAcess;

	if (pwSubID == NULL)	//单ID,不是组合ID
	{
		ItemAcess.bType = DI_ACESS_BUF;
		ItemAcess.pbBuf = pbBuf;
		ItemAcess.pdwTime = NULL;
		iRet = g_DataManager.ReadItemEx(wBank, wPn, wID, ItemAcess, dwStartTime, dwEndTime);
		if (iRet > 0)
			iRet = PostReadItemExHook(wBank, wPn, wID, ItemAcess.pbBuf, iRet);
	}
	else //组合ID
	{
		int iLen; //用来计算组合数据项的长度
		WORD id;
		BYTE bErr = 0;
		iRet = 0;

		while ((id=*pwSubID++) != 0)	//把组合ID转换成依次对子ID的读
		{
			ItemAcess.bType = DI_ACESS_BUF;
			ItemAcess.pbBuf = pbBuf + iRet;
			ItemAcess.pdwTime = NULL;
			iLen = g_DataManager.ReadItemEx(wBank, wPn, id, ItemAcess, dwStartTime, dwEndTime);
			if (iLen > 0)
			{	
				iLen = PostReadItemExHook(wBank, wPn, id, ItemAcess.pbBuf, iLen);
			}
			
			if (iLen > 0)
			{
				iRet += iLen;
			}
			else if (iLen < 0)
			{
				if (iLen == -ERR_ITEM)	//不支持的数据项
					return -ERR_ITEM;
				
				//支持的数据项,但发生了错误
				iLen = -iLen;	//-(ERR_TIME + (int )pItemDesc->wLen*0x100);
				bErr = iLen & 0xff; //目前错误只能保留一个
				iRet += (iLen>>8);
			}
			else //iLen==0
			{
				return -ERR_ITEM;
			}
		}

		if (bErr != 0)
			iRet = -(bErr + iRet*0x100);

		PostReadCmbIdHook(wBank, wPn, wID, pbBuf, dwStartTime, iRet); 
				//即使在部分错误的情况下,都要调用这个函数来调整相应的子ID
	}	

	return iRet;
}

int ReadItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD* pdwTime)
{
	TItemAcess ItemAcess;
	//memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_BUF;
	ItemAcess.pbBuf = pbBuf;
	ItemAcess.pdwTime = pdwTime;
	int iRet = g_DataManager.ReadItemEx(wBank, wPn, wID, ItemAcess, 
										INVALID_TIME, INVALID_TIME);
	if (iRet > 0)
	{
		iRet = PostReadItemExHook(wBank, wPn, wID, pbBuf, iRet);
	}
	
	return iRet;
}


//按照非映射的方式读
int ReadItemUnmap(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_RDUNMAP;		//按照非映射的方式读
	ItemAcess.pbBuf = pbBuf;
	ItemAcess.pdwTime = NULL;
	int iRet = g_DataManager.ReadItemEx(wBank, wPn, wID, ItemAcess, 
										dwStartTime, dwEndTime);
	if (iRet > 0)
	{
		iRet = PostReadItemExHook(wBank, wPn, wID, pbBuf, iRet);
	}

	return iRet;
}

//描述:多个ID的按缓冲区读
int ReadItemEx(WORD wBank, WORD wPn, WORD* pwID, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	int iLen = 0;
	for (WORD i=0; i<wNum; i++)
	{
		int iRet = ReadItemEx(wBank, wPn, *pwID++, pbBuf, dwStartTime, dwEndTime);
		if (iRet < 0)
		{
			if (iRet == -ERR_ITEM)		//不能确定数据项长度
				return -ERR_ITEM;

			iRet = (-iRet >> 8) & 0xff;	//取数据项长度
										//数据内容已经被填成了无效数据
		}

		iLen += iRet;
		pbBuf += iRet;
	}

	return iLen;
}

//描述:多个ID的按缓冲区读
int ReadItemEx(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	int iLen = 0;
	for (WORD i=0; i<wNum; i++)
	{
		int iRet = ReadItemEx(pBankItem->wBn, pBankItem->wPn, pBankItem->wID, pbBuf, dwStartTime, dwEndTime);
		if (iRet < 0)
		{
			if (iRet == -ERR_ITEM)		//不能确定数据项长度
				return -ERR_ITEM;

			iRet = (-iRet >> 8) & 0xff;	//取数据项长度
										//数据内容已经被填成了无效数据
		}

		iLen += iRet;
		pbBuf += iRet;
		pBankItem++;
	}

	return iLen;
}

//描述:多个ID按不同的时标的缓冲区读
int ReadItemEx(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD* pdwTime)
{
	int iLen = 0;
	for (WORD i=0; i<wNum; i++)
	{
		int iRet = ReadItemEx(pBankItem->wBn, pBankItem->wPn, pBankItem->wID, pbBuf, *pdwTime++);
		if (iRet < 0)
		{
			if (iRet == -ERR_ITEM)		//不能确定数据项长度
				return -ERR_ITEM;

			iRet = (-iRet >> 8) & 0xff;	//取数据项长度
										//数据内容已经被填成了无效数据
		}

		iLen += iRet;
		pbBuf += iRet;
		pBankItem++;
	}

	return iLen;
}

int ReadItemVal(WORD wBank, WORD wPn, WORD wID, int* piVal32, DWORD dwStartTime, DWORD dwEndTime, WORD* pwValNum)
{
	TItemAcess ItemAcess;
	WORD* pwSubID = CmbToSubID(wBank, wID);
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_INT32;
	ItemAcess.pdwTime = NULL;
	ItemAcess.piVal32 = piVal32;

	if (pwSubID == NULL)	//单ID,不是组合ID
	{
		g_DataManager.ReadItemEx(wBank, wPn, wID, ItemAcess, dwStartTime, dwEndTime);
		if (pwValNum != NULL)
			*pwValNum = ItemAcess.wValNum;

		return ItemAcess.wValidNum;
	}
	else //组合ID
	{
		WORD wValNum = 0;
		WORD wValidNum = 0;
		int iLen; //用来计算组合数据项的长度
		WORD id;

		while ((id=*pwSubID++) != 0)	//把组合ID转换成依次对子ID的读
		{
			iLen = g_DataManager.ReadItemEx(wBank, wPn, id, ItemAcess, dwStartTime, dwEndTime);
			//WARNING:按值读取函数目前不执行PostReadItemExHook()
			
			if (iLen == 0)
			{
				return -ERR_ITEM;
			}
			else if (iLen < 0)
			{
				if (iLen == -ERR_ITEM)	//不支持的数据项
					return -ERR_ITEM;
				
				//支持的数据项,但发生了错误,也没什么要做的,数据本身已经被数据库置成无效数据
			}
			
			wValidNum += ItemAcess.wValidNum;
			wValNum += ItemAcess.wValNum;
			ItemAcess.piVal32 += ItemAcess.wValNum;
		}
		
		if (pwValNum != NULL)
			*pwValNum = wValNum;

		return PostReadItemValHook(wBank, wPn, wID, piVal32, dwStartTime, wValidNum);
	}
}

//备注:$取数据项时间的读目前还不支持组合ID的读取,因为组合ID是由多个子ID组成,可能存在多个不同子ID的时间
//	   导致返回时间的不准确,将来一定要做,可以考虑返回所有子ID时间最早的一个,但这样在子ID的时间相距很大时
//	   问题比较明显
int ReadItemVal(WORD wBank, WORD wPn, WORD wID, int* piVal32, DWORD* pdwTime, WORD* pwValNum)
{
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_INT32;
	ItemAcess.piVal32 = piVal32;
	ItemAcess.pdwTime = pdwTime;
	g_DataManager.ReadItemEx(wBank, wPn, wID, ItemAcess, INVALID_TIME, INVALID_TIME);

	if (pwValNum != NULL)
		*pwValNum = ItemAcess.wValNum;

	return ItemAcess.wValidNum;
}

int ReadItemVal64(WORD wBank, WORD wPn, WORD wID, int64* piVal64, DWORD dwStartTime, DWORD dwEndTime, WORD* pwValNum)
{
	TItemAcess ItemAcess;
	WORD* pwSubID = CmbToSubID(wBank, wID);
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_INT64;
	ItemAcess.pdwTime = NULL;
	ItemAcess.piVal64 = piVal64;

	if (pwSubID == NULL)	//单ID,不是组合ID
	{
		g_DataManager.ReadItemEx(wBank, wPn, wID, ItemAcess, dwStartTime, dwEndTime);
		if (pwValNum != NULL)
			*pwValNum = ItemAcess.wValNum;

		return ItemAcess.wValidNum;
	}
	else //组合ID
	{
		WORD wValNum = 0;
		WORD wValidNum = 0;
		int iLen; //用来计算组合数据项的长度
		WORD id;

		while ((id=*pwSubID++) != 0)	//把组合ID转换成依次对子ID的读
		{
			iLen = g_DataManager.ReadItemEx(wBank, wPn, id, ItemAcess, dwStartTime, dwEndTime);
			//WARNING:按值读取函数目前不执行PostReadItemExHook()
			
			if (iLen == 0)
			{
				return -ERR_ITEM;
			}
			else if (iLen < 0)
			{
				if (iLen == -ERR_ITEM)	//不支持的数据项
					return -ERR_ITEM;
				
				//支持的数据项,但发生了错误,也没什么要做的,数据本身已经被数据库置成无效数据
			}
			
			wValidNum += ItemAcess.wValidNum;
			wValNum += ItemAcess.wValNum;
			ItemAcess.piVal64 += ItemAcess.wValNum;
		}
		
		if (pwValNum != NULL)
			*pwValNum = wValNum;
		
		return PostReadItemVal64Hook(wBank, wPn, wID, piVal64, dwStartTime, wValidNum);
	}
}

//备注:$取数据项时间的读目前还不支持组合ID的读取,因为组合ID是由多个子ID组成,可能存在多个不同子ID的时间
//	   导致返回时间的不准确,将来一定要做,可以考虑返回所有子ID时间最早的一个,但这样在子ID的时间相距很大时
//	   问题比较明显
int ReadItemVal64(WORD wBank, WORD wPn, WORD wID, int64* piVal64, DWORD* pdwTime, WORD* pwValNum)
{
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_INT64;
	ItemAcess.piVal64 = piVal64;
	ItemAcess.pdwTime = pdwTime;
	g_DataManager.ReadItemEx(wBank, wPn, wID, ItemAcess, INVALID_TIME, INVALID_TIME);

	if (pwValNum != NULL)
		*pwValNum = ItemAcess.wValNum;

	return ItemAcess.wValidNum;
}

//备注:$按值写数据项目前还看不出来有支持组合ID的写的必要,暂不实现
int WriteItemVal(WORD wBank, WORD wPn, WORD wID, int* piVal32, BYTE bPerm, BYTE* pbPassword, DWORD dwTime, WORD* pwValNum)
{
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_INT32;
	ItemAcess.piVal32 = piVal32;
	g_DataManager.WriteItemEx(wBank, wPn, wID, ItemAcess, bPerm, pbPassword, dwTime);

	if (pwValNum != NULL)
		*pwValNum = ItemAcess.wValNum;

	return ItemAcess.wValidNum;
}

//备注:$按值写数据项目前还看不出来有支持组合ID的写的必要,暂不实现
int WriteItemVal64(WORD wBank, WORD wPn, WORD wID, int64* piVal64, BYTE bPerm, BYTE* pbPassword, DWORD dwTime, WORD* pwValNum)
{
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_INT64;
	ItemAcess.piVal64 = piVal64;
	g_DataManager.WriteItemEx(wBank, wPn, wID, ItemAcess, bPerm, pbPassword, dwTime);

	if (pwValNum != NULL)
		*pwValNum = ItemAcess.wValNum;

	return ItemAcess.wValidNum;
}


int GetItemInfo(WORD wBn, WORD wID, TItemInfo* pItemInfo)
{
	TItemAcess ItemAcess;
	ItemAcess.bType = DI_ACESS_INFO;	//取数据项信息(长度和段)
	ItemAcess.pItemInfo = pItemInfo;	
	pItemInfo->wSect = 0;
	int iItemLen = g_DataManager.ReadItemEx(wBn, PN0, wID, ItemAcess, 
											INVALID_TIME, INVALID_TIME);
	if (iItemLen > 0)
	{
		pItemInfo->wLen = iItemLen;
		return iItemLen;
	}
	else 
	{
		return -ERR_ITEM;
	}
}


int GetItemLen(WORD wBn, WORD wID)
{
	int iRet = 0;
   	TItemInfo ItemInfo;
	WORD* pwSubID = CmbToSubID(wBn, wID);

	if (pwSubID == NULL)	//单ID,不是组合ID
	{	
		return GetItemInfo(wBn, wID, &ItemInfo);
	}
	else //组合ID
	{
		int iLen; //用来计算组合数据项的长度
		WORD id;
		iRet = 0;

		while ((id=*pwSubID++) != 0) //把组合ID转换成依次对子ID的读
		{
			iLen = GetItemInfo(wBn, id, &ItemInfo);

			if (iLen > 0)
				iRet += iLen;
			else if (iLen <= 0)
				return iLen;
		}
	}	

	return iRet;
}

int GetItemsLen(WORD* pwItemID, WORD wLen)
{
	int nTotalLen = 0;
	int nLastLen = 0;
	for (int i=0; i<wLen; i++)
	{
		WORD wID = *pwItemID++;
		if (wID==0x8ffe && nLastLen!=0) //如果是对应量
		{
			nTotalLen += nLastLen;
		}
		else
		{	
			int iRet = GetItemLen(BN0, wID);
			if (iRet > 0)
			{	
				nTotalLen += iRet;
				nLastLen = iRet;
			}
			else
			{
				return -i; //-ERR_ITEM;
			}
		}
	}

	return nTotalLen;
}


int GetItemsLen(TBankItem* pBankItem, WORD wNum)
{
	int nTotalLen = 0;
	int nLastLen = 0;
	for (int i=0; i<wNum; i++,pBankItem++)
	{
		if (pBankItem->wBn==BN0 && pBankItem->wID==0x8ffe && nLastLen!=0) //如果是对应量
		{
			nTotalLen += nLastLen;
		}
		else
		{	
			int iRet = GetItemLen(pBankItem->wBn, pBankItem->wID);
			if (iRet > 0)
			{	
				nTotalLen += iRet;
				nLastLen = iRet;
			}
			else
			{
				return -i; //-ERR_ITEM;
			}
		}
	}

	return nTotalLen;
}

int GetItemPnNum(WORD wBn, WORD wID)
{
	TItemInfo ItemInfo;

	if (GetItemInfo(wBn, wID, &ItemInfo) > 0)
		return ItemInfo.wPnNum;
	else
		return -ERR_ITEM;
}

//描述:取得数据项的读地址,主要针对那些内容比较长的数据项,比如测量点屏蔽位等,
//	   免去数据内容拷贝的时间消耗,直接访问只读的内存地址,这样也不会破坏系统库的内容
//返回:如果正确则返回数据项的地址,否则返回NULL
const BYTE* GetItemRdAddr(WORD wBn, WORD wPn, WORD wID)
{
	TDataItem di = GetItemEx(wBn, wPn, wID);
	return di.pbAddr;
}

//描述:按照格式串,把数组piVal相应元素的值置为INVALID_VAL
WORD FmtToInvalidVal32(int* piVal, const BYTE* pbFmtStr, WORD* pwValNum)
{
	BYTE  bFmt, bLen;
	WORD  wValidNum = 0;
	*pwValNum = 0;

	while (*pbFmtStr != 0xff)
	{
		bFmt = *pbFmtStr++; //格式  
		bLen = *pbFmtStr++; //字节数

		*piVal++ = INVALID_VAL;
		(*pwValNum)++;
	}

	return wValidNum;
}

//描述:按照格式串,把数组piVal相应元素的值置为INVALID_VAL
WORD FmtToInvalidVal64(int64* piVal64, const BYTE* pbFmtStr, WORD* pwValNum)
{
	BYTE  bFmt, bLen;
	WORD  wValidNum = 0;
	*pwValNum = 0;

	while (*pbFmtStr != 0xff)
	{
		bFmt = *pbFmtStr++; //格式  
		bLen = *pbFmtStr++; //字节数
							 
		*piVal64++ = INVALID_VAL64;			
		(*pwValNum)++;
	}

	return wValidNum;
}


//版本升级用的读旧版本接口
int UpgReadItem(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD* pdwTime)
{
	return g_DataManager.UpgReadItem(wBank, wPn, wID, pbBuf, pdwTime);
}

void TrigerSave()
{
	g_DataManager.TrigerSaveAll();
}

void TrigerSavePara()
{
	g_DataManager.TrigerSavePara();
}


TDataItem GetItemEx(WORD wBank, WORD wPoint, WORD wID)
{
	return g_DataManager.GetItemEx(wBank, wPoint, wID);
}

bool ClearBankData(WORD wBank, WORD wSect, int iFile)
{
	return g_DataManager.ClearBankData(wBank, wSect, iFile);
}

bool DbClrPnData(WORD wBank, WORD wSect, WORD wPnNum, WORD wPn)
{
	return g_DataManager.ClrPnData(wBank, wSect, wPnNum, wPn);
}

bool IsImgItem(WORD wBank, WORD wPn, WORD wID)
{
	return g_DataManager.IsImgItem(wBank, wPn, wID);
}

void SetMeterPnMask(BYTE* pbMeterPnMask)
{
	g_DataManager.SetMeterPnMask(pbMeterPnMask);
}


void LockDB()
{
	g_fLockDB = true;
}

void UnLockDB()
{
	g_fLockDB = false;
}

bool IsDbLocked()
{
	return g_fLockDB;
}

void TrigerSaveBank(WORD wBank, WORD wSect, int iFile)
{
	g_DataManager.TrigerSaveBank(wBank, wSect, iFile);
}

void DoTrigerSaveBank()
{
	g_DataManager.DoTrigerSaveBank();
}

int DbSave(bool fSaveAll)
{
	return g_DataManager.Save(fSaveAll);
}

int DbSavePara()
{
	return g_DataManager.SavePara();
}

int DbSaveData(bool fSaveAll)
{
	return g_DataManager.SaveData(fSaveAll);
}

void DbDoSave()
{
	return g_DataManager.DoSave();
}

int SearchPnMap(BYTE bSch, WORD wPn)
{
	return g_DataManager.SearchPnMap(bSch, wPn);
}

int MapToPn(BYTE bSch, WORD wMn)
{
	return g_DataManager.MapToPn(bSch, wMn);
}

int NewPnMap(BYTE bSch, WORD wPn)
{
	return g_DataManager.NewPnMap(bSch, wPn);
}

bool DeletePnMap(BYTE bSch, WORD wPn)
{
	return g_DataManager.DeletePnMap(bSch, wPn);
}

int GetPnMapRealNum(BYTE bSch)
{
	return g_DataManager.GetPnMapRealNum(bSch);
}

void DbTimeAdjBackward(DWORD dwTime)
{
	g_DataManager.TimeAdjBackward(dwTime);
}

void DbNewImg(DWORD dwStartTime, WORD wInterval)
{
	g_DataManager.NewImg(dwStartTime, wInterval);
}

//描述:初始化系统库的代码库
bool InitDbLib(TDbCtrl* pDbCtrl)
{
	g_semDataRW = NewSemaphore(1);
	g_semDbSave = NewSemaphore(1);

	return g_DataManager.Init(pDbCtrl);
}