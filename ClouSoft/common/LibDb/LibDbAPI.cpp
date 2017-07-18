/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DbAPI.cpp
 * ժ    Ҫ�����ļ���Ҫʵ�����ݿ�Ĺ����ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��4��
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
        mid = (little + big) / 2;       //����

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
        mid = (little + big) / 2;       //����

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


//����:�����������������Ԥ��ʼ��,
//	   Ŀǰ��Ҫ�Ƕ������е�һЩ�����������,����:
//	   a.��ID�е���ID�Ĳ��������ǿ��Ϊ1;
//	   b.�õ������㶯̬ӳ���,�����������ӳ�䷽����ͬ
void PreInitItemDesc(TBankCtrl* pBankCtrl)
{
	TItemDesc* pItemDesc = pBankCtrl->pItemDesc; 
	DWORD num = pBankCtrl->dwItemNum;
	DWORD dwIndex1 = 0;    //1�����ݿ�Ŀ�ʼ�������±�
	DWORD dwIndex2 = 0;    //2�����ݿ�Ŀ�ʼ�������±�
	WORD wID1 = 0;       //1�����ݿ���������ʶ����λ
	WORD wID2 = 0;       //2�����ݿ���������ʶ����λ
	int iRealNum;
	DWORD j;

	for (DWORD i=0; i<num; i++)
	{
		if ((pItemDesc[i].wID & 0xff00) != wID2)  //2�����ݿ�Ŀ�ʼ
		{                                         //(pItemDesc[i].wID & 0x00ff)== 0
			wID2 = pItemDesc[i].wID & 0xff00;
			dwIndex2 = i;
		}

		if ((pItemDesc[i].wID & 0xfff0) != wID1)  //1�����ݿ�Ŀ�ʼ
		{
			wID1 = pItemDesc[i].wID & 0xfff0;
			dwIndex1 = i;
		}

		if (pItemDesc[i].wPnNum == 0)
			pItemDesc[i].wPnNum = 1;

		if ((pItemDesc[i].wRW & DI_CMB) != 0) //���ID,���ϱ���־�������������Ϊ0,�򲻷���洢�ռ��ʱ��ռ�
			pItemDesc[i].wLen = 0;	//���ID����ǿ�ƾ���Ϊ0,pItemDesc[i].wPnNum���Ǳ���Ϊ��ȷ�ĸ���

		//�����������������ʹ�ò����㶯̬ӳ��,��pItemDesc[i].wPnNum����Ϊӳ�䷽�������õ�ʵ��֧�ֵĲ�������
		if (pBankCtrl->bPnMapSch==0 && pItemDesc[i].bPnMapSch!=0)
		{
			iRealNum = GetPnMapRealNum(pItemDesc[i].bPnMapSch);
			if (iRealNum > 0)
				pItemDesc[i].wPnNum = (WORD )iRealNum;
		}

		if ((pItemDesc[i].wID & 0x00ff)==0x00ff && pItemDesc[i].wLen==0)  //2�����ݿ�Ľ����������ݱ�ʶ�����2λΪff
		{
			//�����ID�Ĳ�����������ʼ��Ϊ1,���鱾������������������,һ����ID�Ĳ���������Ҳ��ʼ��Ϊ1
			for (j=dwIndex2; j<i; j++)
			{
				pItemDesc[j].wPnNum = 1;
			}
		}
		else if ((pItemDesc[i].wID & 0x000f)==0x000f && pItemDesc[i].wLen==0)  //1�����ݿ�Ľ����������ݱ�ʶ�����λΪf
		{
			//�����ID�Ĳ�����������ʼ��Ϊ1,���鱾������������������
			for (j=dwIndex1; j<i; j++)
			{
				pItemDesc[j].wPnNum = 1;
			}
		}
	}
}

//����:��ʼ���������������ƫ���ֶΣ����ڿ����ݣ�����ʼ���������
//����:@ pItemDesc ������������ĵ�һ������
//      @ num ������������ĸ���
//����:����ɹ��򷵻����������ݵĳ���,���򷵻�-1
//����:����ʹ����һ���ٶ����������ʶ�����λ��f���������ʾ���ݿ飬��ǰ��϶�Ҫ�и�λ������ͬ����λ��Ϊf��������
bool InitItemDesc(TBankCtrl* pBankCtrl)
{
	TItemDesc* pItemDesc = pBankCtrl->pItemDesc; 
	DWORD num = pBankCtrl->dwItemNum;
	DWORD dwOffset1 = 0;   //������¼1�����ݿ�Ŀ�ʼ
	DWORD dwOffset2 = 0;   //������¼2�����ݿ�Ŀ�ʼ
	WORD  wOffset, wOffset1;
	WORD  wLen1 = 0;       //�����ۼ�1�����ݿ�ĳ���
	WORD  wLen2 = 0;       //�����ۼ�2�����ݿ�ĳ���
	DWORD dwOffset = 0;
	DWORD dwIndex1 = 0;    //1�����ݿ�Ŀ�ʼ�������±�
	DWORD dwIndex2 = 0;    //2�����ݿ�Ŀ�ʼ�������±�
	WORD wID1 = 0;       //1�����ݿ���������ʶ����λ
	WORD wID2 = 0;       //2�����ݿ���������ʶ����λ
    WORD wBlockLen1 = 0;
    WORD wBlockLen2 = 0;
	//int iRealNum;

	DWORD dwBlkIndex1=0, dwBlkIndex2=0;
	BYTE bBlkIndexNum1=0, bBlkIndexNum2=0, bInnerIndex=0;
	DWORD dwIndex = 0;
	WORD wDefaultOffset = 0; //Ĭ��ֵ��ƫ��,ÿ��������ֻ��һ��
	bool fCmbId;

	PreInitItemDesc(pBankCtrl);

	pBankCtrl->fMutiPnInDesc = false; //�������д��ڶ�������������
	DWORD j;
	WORD wPnNum;
	for (DWORD i=0; i<num; i++)
    {
		pItemDesc[i].bItemFlg = 0;
		if ((pItemDesc[i].wID & 0xff00) != wID2)  //2�����ݿ�Ŀ�ʼ
        {                                         //(pItemDesc[i].wID & 0x00ff)== 0
			wID2 = pItemDesc[i].wID & 0xff00;
			dwIndex2 = i;

			dwOffset2 = dwOffset;   //2���Ŀ�ʼ
			wLen2 = 0;
			
		    wBlockLen2 = 0;

			dwBlkIndex2 = dwIndex;
			bBlkIndexNum2 = 0;
        }

		if ((pItemDesc[i].wID & 0xfff0) != wID1)  //1�����ݿ�Ŀ�ʼ
		{
			wID1 = pItemDesc[i].wID & 0xfff0;
			dwIndex1 = i;

			dwOffset1 = dwOffset;   //1���Ŀ�ʼ
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
			pBankCtrl->fMutiPnInDesc = true; //�������д��ڶ�������������

		fCmbId = (pItemDesc[i].wRW & DI_CMB) != 0; //���ID,���ϱ���־�������������Ϊ0,�򲻷���洢�ռ��ʱ��ռ�

		if ((pItemDesc[i].wID & 0x00ff)==0x00ff && pItemDesc[i].wLen==0 && !fCmbId)  //2�����ݿ�Ľ����������ݱ�ʶ�����2λΪff
        {
			wPnNum = pItemDesc[i].wPnNum;
			bInnerIndex = 0;
			wOffset = 0;
			wOffset1 = 0;

			//�����ID�ĳ�ʼ��
			for (j=dwIndex2; j<=i; j++)
			{
				pItemDesc[j].wPnNum = wPnNum;
				pItemDesc[j].dwBlockStart = dwIndex2;		//�����ݵĿ�ʼ����������,�ϵ�ʱ����
				pItemDesc[j].wBlockLen = wBlockLen2;	//�����ݵĳ���,�ϵ�ʱ����
				pItemDesc[j].dwBlockOffset = dwOffset2;	//�����ݵ�����ڱ����ݱ�ͷ��ƫ��,�ϵ�ʱ����

				pItemDesc[j].dwBlkIndex = dwBlkIndex2;		 //�������ݿ��е���ʼ����,���ղ�����չ����,�ϵ�ʱ����
				pItemDesc[j].bBlkIndexNum = bBlkIndexNum2;	 //�����������,�ϵ�ʱ����
				pItemDesc[j].bInnerIndex = bInnerIndex++;

				pItemDesc[j].bSelfItem = 0;  //����������Գɶ���������
				
				if (pItemDesc[j].bItemFlg & DI_FLG_BLK)	//һ����ID
				{
					pItemDesc[j].wOffset = wOffset1;	//����ƫ��:���ͬʱ����f��ff���ݿ�,��wOffset��ff���ݿ��ƫ��
					wOffset1 = wOffset;	//��2������,����һ��һ����IDʱ,������ʱ���ƫ��,��Ϊ��һ��һ����ID��ƫ��
				}
				else
				{
					pItemDesc[j].wOffset = wOffset; //pItemDesc[j].dwItemOffset - dwOffset2
					wOffset += pItemDesc[j].wLen;	//����ӵ���������ĳ���,��������������ĳ���
				}
			}

			//��ID����ĳ�ʼ��
			//pItemDesc[i].dwItemOffset = dwOffset2;	//Ϊ�˸���������
            pItemDesc[i].bItemFlg |= DI_FLG_BLK;	//�����������־λ--��ID
			pItemDesc[i].wOffset = 0;				//����������ݿ鿪ʼ��ƫ��,�ϵ�ʱ����
			pItemDesc[i].wLen = wBlockLen2;
			
			pItemDesc[i].bBlkIdIndexNum = bBlkIndexNum2; //��ID�Լ����е���������ĸ���

			//pItemDesc[i].wDefaultOffset = wDefaultOffset; //Ĭ��ֵ��ƫ��,ÿ��������ֻ��һ��
			pItemDesc[i].bSelfItem = 1;  //�����������Գɶ���������

			dwOffset += wBlockLen2 * (pItemDesc[i].wPnNum - 1);

			dwIndex++;
			dwIndex += bBlkIndexNum2 * (pItemDesc[i].wPnNum - 1);
        }
		else if ((pItemDesc[i].wID & 0x000f)==0x000f && pItemDesc[i].wLen==0  && !fCmbId)  //1�����ݿ�Ľ����������ݱ�ʶ�����λΪf
		{
			wPnNum = pItemDesc[i].wPnNum;
			bInnerIndex = 0;
			wOffset = 0;

			//�����ID�ĳ�ʼ��
			for (j=dwIndex1; j<=i; j++)
			{
				pItemDesc[j].wPnNum = wPnNum;
				pItemDesc[j].dwBlockStart = dwIndex1;	//�����ݵĿ�ʼ����������,�ϵ�ʱ����
				pItemDesc[j].wBlockLen = wBlockLen1;	//�����ݵĳ���,�ϵ�ʱ����
				pItemDesc[j].dwBlockOffset = dwOffset1;	//�����ݵ�����ڱ����ݱ�ͷ��ƫ��,�ϵ�ʱ����

				pItemDesc[j].dwBlkIndex = dwBlkIndex1;		 //�������ݿ��е���ʼ����,���ղ�����չ����,�ϵ�ʱ����
				pItemDesc[j].bBlkIndexNum = bBlkIndexNum1;	 //�����������,�ϵ�ʱ����
				pItemDesc[j].bInnerIndex = bInnerIndex++;

				pItemDesc[j].bSelfItem = 0;		//����������Գɶ���������
				
				pItemDesc[j].wOffset = wOffset; //pItemDesc[j].dwItemOffset - dwOffset1
				wOffset += pItemDesc[j].wLen;	//����ӵ���������ĳ���,��������������ĳ���
			}

			//��ID����ĳ�ʼ��
			//pItemDesc[i].dwItemOffset = dwOffset1;	//Ϊ�˸���������
            pItemDesc[i].bItemFlg |= DI_FLG_BLK;  //�����������־λ--��ID
			pItemDesc[i].wOffset = 0;		  //����������ݿ鿪ʼ��ƫ��,�ϵ�ʱ����
			pItemDesc[i].wLen = wBlockLen1;
			
			pItemDesc[i].bBlkIdIndexNum = bBlkIndexNum1; //��ID�Լ����е���������ĸ���

			//pItemDesc[i].wDefaultOffset = wDefaultOffset; //Ĭ��ֵ��ƫ��,ÿ��������ֻ��һ��
			pItemDesc[i].bSelfItem = 1;  //�����������Գɶ���������

			dwOffset += wBlockLen1 * (pItemDesc[i].wPnNum - 1);  //���wPnNumΪ1�Ͳ���,
																 //�Ѿ���ǰ��ӹ�һ��,�����ټ�һ�� 	
			dwIndex++;
			dwIndex += bBlkIndexNum1 * (pItemDesc[i].wPnNum - 1); //���wPnNumΪ1�Ͳ���
																  //�Ѿ���ǰ��ӹ�һ��,�����ټ�һ�� 	
		}
		else    //һ��������
        {
            pItemDesc[i].wOffset = 0;
			pItemDesc[i].dwBlockStart = 0;
			pItemDesc[i].dwBlockOffset = dwOffset;  //ֻ��Ե����Ĳ��ɿ����������Ч
			//pItemDesc[i].dwItemOffset = dwOffset;
			pItemDesc[i].wBlockLen = pItemDesc[i].wLen; //ֻ��Ե����Ĳ��ɿ����������Ч

			pItemDesc[i].dwBlkIndex = dwIndex;		 //�������ݿ��е���ʼ����,���ղ�����չ����,�ϵ�ʱ����
			pItemDesc[i].bBlkIndexNum = 1;	 //�����������,�ϵ�ʱ����
			pItemDesc[i].bBlkIdIndexNum = 1; //��ID�Լ����е���������ĸ���
			pItemDesc[i].bInnerIndex = 0;

			if (!fCmbId)
			{
				dwOffset += pItemDesc[i].wLen * pItemDesc[i].wPnNum; //һ�㶼ֻ��1��������ĳ���
				wBlockLen1 += pItemDesc[i].wLen;
				wBlockLen2 += pItemDesc[i].wLen;
				
				dwIndex += pItemDesc[i].wPnNum;  //����������֧�ֶ��������ľ�ֱ�Ӽ�wPnNum,
												 //һ�㶼ֻ��1
				
				//pItemDesc[i].wDefaultOffset = wDefaultOffset; //Ĭ��ֵ��ƫ��,ÿ��������ֻ��һ��
				wDefaultOffset += pItemDesc[i].wLen; 
				pItemDesc[i].bSelfItem = 1;  //����������Ĭ��Ϊ���Գɶ���������
			}
			else
			{
				pItemDesc[i].bSelfItem = 0;  //���ID�����Գɶ���������
			}
		}
	}

	pBankCtrl->dwBankSize = dwOffset;
	pBankCtrl->dwIndexNum = dwIndex;    //�����������ĸ���,��dwItemNum���ղ��������չ����ĸ���
	if (pBankCtrl->dwDefaultSize!=0 && pBankCtrl->dwDefaultSize!=wDefaultOffset)
	{
		DTRACE(DB_DB, ("InitItemDesc: error:  <%s> default len mismatch : default = %d , calcu size = %d.\r\n", 
						pBankCtrl->pszBankName, pBankCtrl->dwDefaultSize, wDefaultOffset));
		return false;
	}

	return true;
}

//����������������ʱ��
inline void UpdItemTime(WORD wPn, WORD wID, WORD wImg, TBankCtrl* pBankCtrl, TItemDesc* pItemDesc, DWORD dwTime)
{
	DWORD dwTimeIndex;
	DWORD i;

	if (pBankCtrl->pdwUpdTime != NULL) // && dwTime!=0��Ҫ����ʱ��INVALID_TIME
	{
		if (pBankCtrl->wPnNum > 1) //��������BANK��֧�ֲ�����
			dwTimeIndex = pBankCtrl->dwIndexNum*(pBankCtrl->wPnNum*wImg + wPn) + 
						  pItemDesc->dwBlkIndex + pItemDesc->bInnerIndex; //nIndex;
		else
			dwTimeIndex = pItemDesc->dwBlkIndex + wPn*pItemDesc->bBlkIndexNum + 
						  pItemDesc->bInnerIndex;
			//���ղ�����չ����һ�������������:dwBlkIndex+������*bBlkIndexNum+bInnerIndex

		if ((wID&0x000f)==0x000f && pItemDesc->bBlkIdIndexNum>1)	//��ID,��Ҫ���������ʱ��
		{									//��ID�Լ����е���������ĸ���
			i = dwTimeIndex - (pItemDesc->bBlkIdIndexNum - 1);
			for (; i<=dwTimeIndex; i++)
			{
				pBankCtrl->pdwUpdTime[i] = dwTime; //һ��ָ�����д������ݲ����ź�������
			}
		}
		else
		{
			pBankCtrl->pdwUpdTime[dwTimeIndex] = dwTime; //һ��ָ�����д������ݲ����ź�������
		}
	}
}

void SetModifiedFlg(TBankCtrl* pBankCtrl, WORD wPn)
{
	//�����ļ����޸ı�־,���ղ�����
	if (pBankCtrl->wFileNum > 1)
	{
		WORD wByte = wPn >> 3; //��8
		BYTE bMask = 1 << (wPn & 0x07);
		pBankCtrl->bModified[wByte] |= bMask;
	}
	else
	{
		pBankCtrl->bModified[0] |= 0x01;
	}
}

//����:�����ȷ�򷵻����ݵĳ��ȣ�����������8λ���ش�����룬�ε�8λ�������ݵĳ���
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

	if (pBankCtrl->pbBankData == NULL) //��BANK��ֻ��Ϊ������������,���������ݷ���Ҫ����Ӧ�Ķ�д����
		return -ERR_ITEM;

	if (pBankCtrl->wPnNum==0 && pBankCtrl->wImgNum==0)
		return -ERR_ITEM; //��BANK��ֻ��Ϊ������������,ֻ֧��DI_ACESS_INFO��ReadItem()����

	TItemDesc* pItemDesc = &pBankCtrl->pItemDesc[nIndex];

	if ((pItemDesc->wRW & DI_CMB) != 0) //���ID���ܽ��к����ķ���
		return -ERR_ITEM;

	//�����㵽ʵ�ʴ洢��(ӳ���)��ת��
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

	if (wPn>=pItemDesc->wPnNum && wPn>=pBankCtrl->wPnNum) //�������֧����ô���������
		return -ERR_ITEM;

	int nRet = PermCheck(pItemDesc, bPerm, pbPassword);
	if (nRet != ERR_OK)
		return nRet;

	BYTE* pbItemAddr;
	if (pBankCtrl->wPnNum>1 || pBankCtrl->wImgNum>1) //��������BANK��֧�ֲ�����
	{
		pbItemAddr = pBankCtrl->pbBankData + pBankCtrl->dwBankSize*(pBankCtrl->wPnNum*wImg + wPn) + 
					 pItemDesc->dwBlockOffset + pItemDesc->wOffset;
	}
	else
	{
		pbItemAddr = pBankCtrl->pbBankData + pItemDesc->dwBlockOffset + 
					 wPn*pItemDesc->wBlockLen + pItemDesc->wOffset;
					//һ���������ƫ��:dwBlockOffset+������*wBlockLen+wOffset
	}

	if (rItemAcess.bType == DI_ACESS_BUF)
	{
		WaitSemaphore(pBankCtrl->semBankRW);
		memcpy(pbItemAddr, rItemAcess.pbBuf, pItemDesc->wLen);
		UpdItemTime(wPn, wID, wImg, pBankCtrl, pItemDesc, dwTime);	//����������ʱ��
		SetModifiedFlg(pBankCtrl, wPn);
		SignalSemaphore(pBankCtrl->semBankRW);
	}
	else if (rItemAcess.bType == DI_ACESS_UPD) //����������״̬
	{
		WaitSemaphore(pBankCtrl->semBankRW);
		memset(pbItemAddr, GetInvalidData((BYTE )rItemAcess.dwVal), pItemDesc->wLen);
		UpdItemTime(wPn, wID, wImg, pBankCtrl, pItemDesc, dwTime);	//����������ʱ��
		SetModifiedFlg(pBankCtrl, wPn);
		SignalSemaphore(pBankCtrl->semBankRW);
	}
	else if (rItemAcess.bType==DI_ACESS_INT32 || rItemAcess.bType==DI_ACESS_INT64)	//��ֵ
	{
		BYTE bBuf[300];		//���ܳ���256���ֽ�
		
		TItemDesc* pItem = pItemDesc;
		WORD wBlkIndexNum = 1;
		if ((wID&0x000f)==0x000f && pItemDesc->bBlkIdIndexNum>1)
		{							//��ID�Լ����е���������ĸ���
			wBlkIndexNum = pItemDesc->bBlkIdIndexNum - 1;
			pItem = pItemDesc - wBlkIndexNum;
		}

		BYTE* pb = bBuf;
		WORD wValNum;
		rItemAcess.wValNum = 0;		//��ֵ��дʱ��ֵ�ĸ���
		rItemAcess.wValidNum = 0; //���ڰ�ֵ���ʵ����,��Ч������Ӧ���θ���

		if (rItemAcess.bType == DI_ACESS_INT32) //��������32λ��д
		{
			int* piVal32 = rItemAcess.piVal32;
			for (i=0; i<wBlkIndexNum; i++)
			{
				if (pItem->pbFmtStr != NULL)
				{	
					rItemAcess.wValidNum += ValToFmt(piVal32, pb, pItem->pbFmtStr, &wValNum);  //�������ݸ�ʽ
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
						ValToFmt(*piVal32, pb, pItem->wFormat, pItem->wLen); //��һ���ݸ�ʽ
					}
					piVal32++;
					rItemAcess.wValidNum++;	//��ʱ�Ǹ��ϸ�ʽ������Ҳֻ����һ��������
				}
				
				pb += pItem->wLen;
				pItem++;
			}
		}
		else //��������64λ��д
		{
			int64* piVal64 = rItemAcess.piVal64;
			for (i=0; i<wBlkIndexNum; i++)
			{
				if (pItem->pbFmtStr != NULL)
				{
					rItemAcess.wValidNum += Val64ToFmt(piVal64, pb, pItem->pbFmtStr, &wValNum);   //�������ݸ�ʽ
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
					rItemAcess.wValidNum++; //��ʱ�Ǹ��ϸ�ʽ������Ҳֻ����һ��������
				}
				
				pb += pItem->wLen;
				pItem++;
			}
		}
		WaitSemaphore(pBankCtrl->semBankRW);
		memcpy(pbItemAddr, bBuf, pItemDesc->wLen);
		UpdItemTime(wPn, wID, wImg, pBankCtrl, pItemDesc, dwTime);	//����������ʱ��
		SetModifiedFlg(pBankCtrl, wPn);
		SignalSemaphore(pBankCtrl->semBankRW);
	}

	//TODO�������־

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

	if ((pItemDesc->wRW & DI_CMB) != 0) //���ID���ܽ��к����ķ���
		return -ERR_ITEM;

	if (rItemAcess.bType == DI_ACESS_INFO)	//ȡ�������
	{	//���wPnNum��wImgNumͬʱ����Ϊ0,��ʾ��BANK��ֻ��Ϊ������������
		if (pBankCtrl->wPnNum > 1)
			rItemAcess.pItemInfo->wPnNum = pBankCtrl->wPnNum;
		else
			rItemAcess.pItemInfo->wPnNum = pItemDesc->wPnNum;

		rItemAcess.pItemInfo->wLen = pItemDesc->wLen;
		return pItemDesc->wLen;
	}

	if (pBankCtrl->pbBankData == NULL) //��BANK��ֻ��Ϊ������������,���������ݷ���Ҫ����Ӧ�Ķ�д����
		return -ERR_ITEM;

	if (pBankCtrl->wPnNum==0 && pBankCtrl->wImgNum==0)
		return -ERR_ITEM; //��BANK��ֻ��Ϊ������������,ֻ֧��DI_ACESS_INFO��ReadItem()����

	//�����㵽ʵ�ʴ洢��(ӳ���)��ת��
	int iPn;
	BYTE bPnMapSch = 0;
	if (pBankCtrl->bPnMapSch != 0)
		bPnMapSch = pBankCtrl->bPnMapSch;
	else if (pItemDesc->bPnMapSch != 0)
		bPnMapSch = pItemDesc->bPnMapSch;

	if (bPnMapSch!=0 && rItemAcess.bType!=DI_ACESS_RDUNMAP)	//���շ�ӳ��ķ�ʽ��
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

	if (wPn>=pItemDesc->wPnNum && wPn>=pBankCtrl->wPnNum) //�������֧����ô���������
		return -ERR_ITEM;
			
	DWORD i;
	TItemDesc* pItem = pItemDesc;
	WORD wBlkIndexNum = 1;
	if ((wID&0x000f)==0x000f && pItemDesc->bBlkIdIndexNum>1)
	{							//��ID�Լ����е���������ĸ���
		wBlkIndexNum = pItemDesc->bBlkIdIndexNum - 1;
		pItem = pItemDesc - wBlkIndexNum;
	}

	DWORD dwTimeIndex;
	if (pBankCtrl->pdwUpdTime!=NULL && (pItemDesc->wRW&DI_NTS)==0)
	{ //����BANK��֧��ʱ��			&&  �������֧��ʱ��
		if (pBankCtrl->wPnNum > 1) //��������BANK��֧�ֲ�����
			dwTimeIndex = pBankCtrl->dwIndexNum*(pBankCtrl->wPnNum*wImg + wPn) + 
						  pItemDesc->dwBlkIndex + pItemDesc->bInnerIndex; //nIndex;
		else
			dwTimeIndex = pItemDesc->dwBlkIndex + wPn*pItemDesc->bBlkIndexNum + 
						  pItemDesc->bInnerIndex;
			//���ղ�����չ����һ�������������:dwBlkIndex+������*bBlkIndexNum+bInnerIndex

		if (rItemAcess.pdwTime != NULL)
			*rItemAcess.pdwTime = pBankCtrl->pdwUpdTime[dwTimeIndex];
		
		if (dwStartTime == 0)
			dwEndTime = 0;

		if ((dwStartTime!=0 && pBankCtrl->pdwUpdTime[dwTimeIndex]<dwStartTime)  //�������ʱ����Ҫ���ʱ��ǰ��������Ҫ��
			|| (dwEndTime!=0 && pBankCtrl->pdwUpdTime[dwTimeIndex]>=dwEndTime))
		{
			if (rItemAcess.bType==DI_ACESS_BUF || rItemAcess.bType==DI_ACESS_RDUNMAP)
			{
				memset(rItemAcess.pbBuf, GetInvalidData(ERR_OK), pItemDesc->wLen);
			}
			else if (rItemAcess.bType == DI_ACESS_INT32) //��������32λ��,��ȫ��������ΪINVALID_VAL
			{
				rItemAcess.wValNum = 0;		//��ֵ��дʱ��ֵ�ĸ���
				rItemAcess.wValidNum = 0;	//���ڰ�ֵ���ʵ����,��Ч������Ӧ���θ���

				piVal32 = rItemAcess.piVal32;
				for (i=0; i<wBlkIndexNum; i++)
				{
					if (pItem->pbFmtStr != NULL)
					{	
						FmtToInvalidVal32(piVal32, pItem->pbFmtStr, &wValNum);
						rItemAcess.wValNum += wValNum;
						piVal32 += wValNum;
					}
					else //��һ���ݸ�ʽ��ֻ���Ǻ�һ������
					{
						rItemAcess.wValNum++;
						*piVal32 = INVALID_VAL;
						piVal32++;    //Ϊ������������һ��ID��׼��
					}	

					pItem++;
				}
			}
			else if (rItemAcess.bType == DI_ACESS_INT64) //��������64λ��,��ȫ��������ΪINVALID_VAL64
			{
				rItemAcess.wValNum = 0;		//��ֵ��дʱ��ֵ�ĸ���
				rItemAcess.wValidNum = 0;	//���ڰ�ֵ���ʵ����,��Ч������Ӧ���θ���

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
						piVal64++;                  //Ϊ������������һ��ID��׼��
					}

					pItem++;
				}
			}

			return -(ERR_TIME + (int )pItemDesc->wLen*0x100);
		}
	}

	BYTE* pbItemAddr;
	if (pBankCtrl->wPnNum > 1) //��������BANK��֧�ֲ�����
	{
		pbItemAddr = pBankCtrl->pbBankData + pBankCtrl->dwBankSize*(pBankCtrl->wPnNum*wImg + wPn) + 
					 pItemDesc->dwBlockOffset + pItemDesc->wOffset;
	}
	else
	{
		pbItemAddr = pBankCtrl->pbBankData + pItemDesc->dwBlockOffset + 
					 wPn*pItemDesc->wBlockLen + pItemDesc->wOffset;
					//һ���������ƫ��:dwBlockOffset+������*wBlockLen+wOffset
	}
	
	if (rItemAcess.bType==DI_ACESS_BUF || rItemAcess.bType==DI_ACESS_RDUNMAP)
	{
		WaitSemaphore(pBankCtrl->semBankRW);
		memcpy(rItemAcess.pbBuf, pbItemAddr, pItemDesc->wLen);
		SignalSemaphore(pBankCtrl->semBankRW);
	}
	else if (rItemAcess.bType==DI_ACESS_INT32 || rItemAcess.bType==DI_ACESS_INT64)	//��ֵ
	{
		BYTE bBuf[300];		//���ܳ���256���ֽ�
		WaitSemaphore(pBankCtrl->semBankRW);
		memcpy(bBuf, pbItemAddr, pItemDesc->wLen);
		SignalSemaphore(pBankCtrl->semBankRW);
		
		BYTE* pb = bBuf;
		rItemAcess.wValNum = 0;		//��ֵ��дʱ��ֵ�ĸ���
		rItemAcess.wValidNum = 0;	//���ڰ�ֵ���ʵ����,��Ч������Ӧ���θ���

		if (rItemAcess.bType == DI_ACESS_INT32) //��������32λ��
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
				else //��һ���ݸ�ʽ��ֻ���Ǻ�һ������
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
					piVal32++;    //Ϊ������������һ��ID��׼��
				}	

				pb += pItem->wLen;
				pItem++;
			}
		}
		else //��������64λ��
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
					piVal64++;                  //Ϊ������������һ��ID��׼��
				}

				pb += pItem->wLen;
				pItem++;
			}
		}
	}
	else if (rItemAcess.bType == DI_ACESS_QRY) //��ѯ�������Ƿ����,ʲô�����ø�
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
		
		//�����ļ����޸ı�־,���ղ�����
		if (pBankCtrl->wFileNum > 1)
		{
			WORD wByte = wPn >> 3; //��8
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


//������ͨ���������ڳ������֧�ֵ�������򳭱�ʧ�ܵ�ʱ��,����������ʱ��,
//		�Լӿ�Ӧ�ò�ѯ���ݵļ�ʱ��
//������@wErr �������,ERR_UNSUP���֧�ֵ�������,ERR_FAIL����ʧ��
//��ע��Ŀǰֻ֧�ֵ�����������ʱ��,��֧�ָ��´������
bool UpdItemErr(WORD wBank, WORD wPn, WORD wID, WORD wErr, DWORD dwTime)
{
	if (wBank != BN0)
		return true;

	TItemAcess ItemAcess;
	ItemAcess.bType = DI_ACESS_UPD;	//����������״̬
	ItemAcess.dwVal = wErr;

	return g_DataManager.WriteItemEx(wBank, wPn, wID, ItemAcess, DI_LOW_PERM, NULL, dwTime)>0;
}


//��������ѯ�������Ƿ���dwStartTime�󱻸��¹�
//������@dwStartTime ����������ʼʱ��,��2000��1��1��0��0��0���������
//					 ע����SubmitMeterReq()��dwStartTime������ͬ
//		@dwEndTime   С����Ӧ�Ľ���ʱ��
//      @pBank0Item ָ�������������ָ��
//      @wNum ����Ԫ�صĸ���
//		@pwValidNum �������غϷ�������ĸ���
//���أ��Ѿ���ȷ�ϵ����������,�����Ѿ�����,ȷ�ϲ�֧�ֵĻ�ȷ�ϳ�������������
//��ע����������¹���һ�������������ݺϷ�,���֧�ֻ򳭱�ʧ�ܵ����������
//		��UpdItemErr()����ʱ��,�Լӿ�Ӧ�ò�ѯ���ݵļ�ʱ��
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
			int iLen = 0;	//�����������������ĳ���
			WORD wID;
			bool fInvalid = false;
			iRet = 0;

			while ((wID=*pwSubID++) != 0)	//�����IDת�������ζ���ID�Ķ�
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
					if ((iLen&0xff) != ERR_INVALID)  //ʱ�䱻������,��������������Ч,������֧�ֵ�������򳭱�ʧ�ܵ�
					{
						//��������������ERR_ITEM/ERR_TIME�ȴ���,�����ID��ʣ��ID�Ͳ����ٲ�ѯ��,
						//��Ϊ���������Ѿ����ײ�����Ҫ����
						fInvalid = false;	//��������,���ܹ�ΪERR_INVALID
						iRet = -iLen;
						break;	
					}
					
					iRet += (iLen>>8);
					fInvalid = true;
				}
				else //iLen==0
				{
					fInvalid = false;	//��������,���ܹ�ΪERR_INVALID
					iRet = 0;
					break;
				}
			}

			if (fInvalid && wID==0)	//ȫ��������������,ȷ��ֻʣ��ERR_INVALID����
				iRet = -(ERR_INVALID + iLen*0x100);
		}

		if (iRet > 0)
		{
			iConfirmNum++;
			wValidNum++;
		}
		else
		{
			iRet = (-iRet) & 0xff;	  //ȡ�������
			if (iRet == ERR_INVALID)  //ʱ�䱻������,��������������Ч,������֧�ֵ�������򳭱�ʧ�ܵ�
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

//����:���ղ�ͬ�����ͬʱ��Ĳ�ѯ,���������Ƿ���pdwStartTime�󱻸��¹�
//����:@pdwStartTime ������ʱ�������
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
			BankItem[i].wBn = wBn;  	//�������
			BankItem[i].wPn = wPn;  	//�������
			BankItem[i].wID = *pwID++;  //BN0����645ID
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
		iRet = PostWriteItemExHook(wBank, wPn, wID, pbBuf, bPerm, pbPassword, iRet); //���ùҹ�����Ӧ�����⴦��
	}
		 
	return iRet;
}

int ReadItemEx(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	int iRet;
	WORD* pwSubID = CmbToSubID(wBank, wID);
	TItemAcess ItemAcess;

	if (pwSubID == NULL)	//��ID,�������ID
	{
		ItemAcess.bType = DI_ACESS_BUF;
		ItemAcess.pbBuf = pbBuf;
		ItemAcess.pdwTime = NULL;
		iRet = g_DataManager.ReadItemEx(wBank, wPn, wID, ItemAcess, dwStartTime, dwEndTime);
		if (iRet > 0)
			iRet = PostReadItemExHook(wBank, wPn, wID, ItemAcess.pbBuf, iRet);
	}
	else //���ID
	{
		int iLen; //�����������������ĳ���
		WORD id;
		BYTE bErr = 0;
		iRet = 0;

		while ((id=*pwSubID++) != 0)	//�����IDת�������ζ���ID�Ķ�
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
				if (iLen == -ERR_ITEM)	//��֧�ֵ�������
					return -ERR_ITEM;
				
				//֧�ֵ�������,�������˴���
				iLen = -iLen;	//-(ERR_TIME + (int )pItemDesc->wLen*0x100);
				bErr = iLen & 0xff; //Ŀǰ����ֻ�ܱ���һ��
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
				//��ʹ�ڲ��ִ���������,��Ҫ�������������������Ӧ����ID
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


//���շ�ӳ��ķ�ʽ��
int ReadItemUnmap(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(TItemAcess));
	ItemAcess.bType = DI_ACESS_RDUNMAP;		//���շ�ӳ��ķ�ʽ��
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

//����:���ID�İ���������
int ReadItemEx(WORD wBank, WORD wPn, WORD* pwID, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	int iLen = 0;
	for (WORD i=0; i<wNum; i++)
	{
		int iRet = ReadItemEx(wBank, wPn, *pwID++, pbBuf, dwStartTime, dwEndTime);
		if (iRet < 0)
		{
			if (iRet == -ERR_ITEM)		//����ȷ���������
				return -ERR_ITEM;

			iRet = (-iRet >> 8) & 0xff;	//ȡ�������
										//���������Ѿ����������Ч����
		}

		iLen += iRet;
		pbBuf += iRet;
	}

	return iLen;
}

//����:���ID�İ���������
int ReadItemEx(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD dwStartTime, DWORD dwEndTime)
{
	int iLen = 0;
	for (WORD i=0; i<wNum; i++)
	{
		int iRet = ReadItemEx(pBankItem->wBn, pBankItem->wPn, pBankItem->wID, pbBuf, dwStartTime, dwEndTime);
		if (iRet < 0)
		{
			if (iRet == -ERR_ITEM)		//����ȷ���������
				return -ERR_ITEM;

			iRet = (-iRet >> 8) & 0xff;	//ȡ�������
										//���������Ѿ����������Ч����
		}

		iLen += iRet;
		pbBuf += iRet;
		pBankItem++;
	}

	return iLen;
}

//����:���ID����ͬ��ʱ��Ļ�������
int ReadItemEx(TBankItem* pBankItem, WORD wNum, BYTE* pbBuf, DWORD* pdwTime)
{
	int iLen = 0;
	for (WORD i=0; i<wNum; i++)
	{
		int iRet = ReadItemEx(pBankItem->wBn, pBankItem->wPn, pBankItem->wID, pbBuf, *pdwTime++);
		if (iRet < 0)
		{
			if (iRet == -ERR_ITEM)		//����ȷ���������
				return -ERR_ITEM;

			iRet = (-iRet >> 8) & 0xff;	//ȡ�������
										//���������Ѿ����������Ч����
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

	if (pwSubID == NULL)	//��ID,�������ID
	{
		g_DataManager.ReadItemEx(wBank, wPn, wID, ItemAcess, dwStartTime, dwEndTime);
		if (pwValNum != NULL)
			*pwValNum = ItemAcess.wValNum;

		return ItemAcess.wValidNum;
	}
	else //���ID
	{
		WORD wValNum = 0;
		WORD wValidNum = 0;
		int iLen; //�����������������ĳ���
		WORD id;

		while ((id=*pwSubID++) != 0)	//�����IDת�������ζ���ID�Ķ�
		{
			iLen = g_DataManager.ReadItemEx(wBank, wPn, id, ItemAcess, dwStartTime, dwEndTime);
			//WARNING:��ֵ��ȡ����Ŀǰ��ִ��PostReadItemExHook()
			
			if (iLen == 0)
			{
				return -ERR_ITEM;
			}
			else if (iLen < 0)
			{
				if (iLen == -ERR_ITEM)	//��֧�ֵ�������
					return -ERR_ITEM;
				
				//֧�ֵ�������,�������˴���,ҲûʲôҪ����,���ݱ����Ѿ������ݿ��ó���Ч����
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

//��ע:$ȡ������ʱ��Ķ�Ŀǰ����֧�����ID�Ķ�ȡ,��Ϊ���ID���ɶ����ID���,���ܴ��ڶ����ͬ��ID��ʱ��
//	   ���·���ʱ��Ĳ�׼ȷ,����һ��Ҫ��,���Կ��Ƿ���������IDʱ�������һ��,����������ID��ʱ�����ܴ�ʱ
//	   ����Ƚ�����
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

	if (pwSubID == NULL)	//��ID,�������ID
	{
		g_DataManager.ReadItemEx(wBank, wPn, wID, ItemAcess, dwStartTime, dwEndTime);
		if (pwValNum != NULL)
			*pwValNum = ItemAcess.wValNum;

		return ItemAcess.wValidNum;
	}
	else //���ID
	{
		WORD wValNum = 0;
		WORD wValidNum = 0;
		int iLen; //�����������������ĳ���
		WORD id;

		while ((id=*pwSubID++) != 0)	//�����IDת�������ζ���ID�Ķ�
		{
			iLen = g_DataManager.ReadItemEx(wBank, wPn, id, ItemAcess, dwStartTime, dwEndTime);
			//WARNING:��ֵ��ȡ����Ŀǰ��ִ��PostReadItemExHook()
			
			if (iLen == 0)
			{
				return -ERR_ITEM;
			}
			else if (iLen < 0)
			{
				if (iLen == -ERR_ITEM)	//��֧�ֵ�������
					return -ERR_ITEM;
				
				//֧�ֵ�������,�������˴���,ҲûʲôҪ����,���ݱ����Ѿ������ݿ��ó���Ч����
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

//��ע:$ȡ������ʱ��Ķ�Ŀǰ����֧�����ID�Ķ�ȡ,��Ϊ���ID���ɶ����ID���,���ܴ��ڶ����ͬ��ID��ʱ��
//	   ���·���ʱ��Ĳ�׼ȷ,����һ��Ҫ��,���Կ��Ƿ���������IDʱ�������һ��,����������ID��ʱ�����ܴ�ʱ
//	   ����Ƚ�����
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

//��ע:$��ֵд������Ŀǰ������������֧�����ID��д�ı�Ҫ,�ݲ�ʵ��
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

//��ע:$��ֵд������Ŀǰ������������֧�����ID��д�ı�Ҫ,�ݲ�ʵ��
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
	ItemAcess.bType = DI_ACESS_INFO;	//ȡ��������Ϣ(���ȺͶ�)
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

	if (pwSubID == NULL)	//��ID,�������ID
	{	
		return GetItemInfo(wBn, wID, &ItemInfo);
	}
	else //���ID
	{
		int iLen; //�����������������ĳ���
		WORD id;
		iRet = 0;

		while ((id=*pwSubID++) != 0) //�����IDת�������ζ���ID�Ķ�
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
		if (wID==0x8ffe && nLastLen!=0) //����Ƕ�Ӧ��
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
		if (pBankItem->wBn==BN0 && pBankItem->wID==0x8ffe && nLastLen!=0) //����Ƕ�Ӧ��
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

//����:ȡ��������Ķ���ַ,��Ҫ�����Щ���ݱȽϳ���������,�������������λ��,
//	   ��ȥ�������ݿ�����ʱ������,ֱ�ӷ���ֻ�����ڴ��ַ,����Ҳ�����ƻ�ϵͳ�������
//����:�����ȷ�򷵻�������ĵ�ַ,���򷵻�NULL
const BYTE* GetItemRdAddr(WORD wBn, WORD wPn, WORD wID)
{
	TDataItem di = GetItemEx(wBn, wPn, wID);
	return di.pbAddr;
}

//����:���ո�ʽ��,������piVal��ӦԪ�ص�ֵ��ΪINVALID_VAL
WORD FmtToInvalidVal32(int* piVal, const BYTE* pbFmtStr, WORD* pwValNum)
{
	BYTE  bFmt, bLen;
	WORD  wValidNum = 0;
	*pwValNum = 0;

	while (*pbFmtStr != 0xff)
	{
		bFmt = *pbFmtStr++; //��ʽ  
		bLen = *pbFmtStr++; //�ֽ���

		*piVal++ = INVALID_VAL;
		(*pwValNum)++;
	}

	return wValidNum;
}

//����:���ո�ʽ��,������piVal��ӦԪ�ص�ֵ��ΪINVALID_VAL
WORD FmtToInvalidVal64(int64* piVal64, const BYTE* pbFmtStr, WORD* pwValNum)
{
	BYTE  bFmt, bLen;
	WORD  wValidNum = 0;
	*pwValNum = 0;

	while (*pbFmtStr != 0xff)
	{
		bFmt = *pbFmtStr++; //��ʽ  
		bLen = *pbFmtStr++; //�ֽ���
							 
		*piVal64++ = INVALID_VAL64;			
		(*pwValNum)++;
	}

	return wValidNum;
}


//�汾�����õĶ��ɰ汾�ӿ�
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

//����:��ʼ��ϵͳ��Ĵ����
bool InitDbLib(TDbCtrl* pDbCtrl)
{
	g_semDataRW = NewSemaphore(1);
	g_semDbSave = NewSemaphore(1);

	return g_DataManager.Init(pDbCtrl);
}