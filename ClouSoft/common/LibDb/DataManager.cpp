/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DataManager.cpp
 * ժ    Ҫ�����ļ���Ҫʵ��ϵͳ���ݿ���������д�洢����
 * ��ǰ�汾��������VER_STR
 * ��    �ߣ�᯼���
 * ������ڣ�2009��2��
 * ��    ע��$�����㶯̬ӳ��ʵ��˵��
			  1.�������:
				a.����������صĲ���,����,��չ����,��ʱ���ݵ�,��Ҫ��ͳһ�ķ�������ӳ�����.
				b.�п���ͬһ������������ݴ��ڶ���ӳ�䷽��,������������(����F10)����1024�����ӳ��,
				  ���������ݷֲ�ͬ�����ԵĲ�������ӳ��,���ر�ӳ��64��,�ز���ӳ��1024��;�����������,
				  ���ǰѱ�������ز������ݹ̶�����1024��,�����ж�̬ӳ��,ֻ�Ǹ��ر������ӳ���64��;
				  ������������֧�ֺܶ��������ʵ����ֻ֧����������������,�Ž���ӳ��
				c.��Ҫӳ��Ĳ�����Ĵ�����ɾ��Ҫ��ȷ��������ͷ�ӳ�����Դ
				d.���ڲ����㶯̬ӳ��,����ϵͳ���ݿ�ֻ֧�����޵ļ��׷���,��ͬ�ķ���
			      ��������֧�ֵĲ������ӳ���ʵ��֧�ֵĲ�������,�������ü�TPnMapCtrl
				e.�漰���Ĳ��������ݹ���ʽ��:����������ֱ����������������,���������ݷֲ�������,
				  ������������,���Ƕ�ͳһ��������ϵͳ��ӳ�䷽��������
				f.�����㵽�洢�ŵ�ӳ���Ҫ���浽�ļ�ϵͳ,�ϵ�ʱҪ���ļ�ϵͳ�ָ�
				g.�����㵽�洢�ŵ�ӳ���ʹ�ö��ַ�������,ͬʱʹ�ô洢�ռ��������ӿ�հ׿ռ�Ĳ���
				h.��������д���̵�Ӱ�������int WriteItem(WORD wImg, WORD wPn, ...)��
				  int ReadItem(WORD wImg, WORD wPn,...)����������,�������ж�̬ӳ���������,
				  ��Ҫ����ʹ��SearchPnMap()���в����㵽ʵ�ʴ洢��(ӳ���)��ת��
			  2.ӳ�������ϵ�ʱ���ļ�ϵͳ�ָ���RAM��˵��:
				a.�������ݷֲ������ŵ�SECT,�����������,�洢�ļ�������point%d.dat,
				  ���еĲ������ָ���Ǵ洢��,���ļ�ϵͳ�ָ�ʱֱ�Ӹ��ݴ洢�Żָ�����Ӧ���ڴ���,
				  �����Ĳ������Ҫ���ݴ��ļ�ϵͳ�лָ��Ĳ����㵽�洢�ŵ�ӳ���
				b.���ڲ���������������������ID�����õ����,�ָ�ʱҲ��ֱ�Ӱ��ļ����ݿ�����RAM��,
				  ���ݶ�Ӧ�Ĳ�����Ҳ�Ǹ��ݲ����㵽�洢�ŵ�ӳ���
			  3.����������ӳ���,���ݵ������ͬ������?
  				�������ӳ�䶼������Ų�����ı�������½���ӳ���,���Բ�����ӳ�����������������
				���ݵ����,�����ⲿ�����������ò����������ӳ���,���²�����Ŀռ����һ��
			  4.������ӳ�䷢���ı��ʱ�������������ݷ��ʵ�����:
---------------------------------------------------------------------------------------------------------
* �汾��Ϣ:
 ---2009-2-9:---V1.1.01----᯼���---
 	1.���Ӳ����㶯̬ӳ��Ĺ���
 	2.ϵͳ�����ݿ��һ�������������÷���������ļ�,ϵͳ��ʹ�ò�������ʼ��,ϵͳ�������������ǰ�̶�
	  ����õ����ݽṹ
 ---2009-2-9:---V1.1.05----���---
 	1.ȥ��IsMeterPn()�ж�PN_NUM������
 	2.����ʱ���ⲿ����IsSectHaveSampleData()��Ϊ��inline����
 ---2009-4-16:---V1.1.06----᯼���---
 	1.��Ч�ڶ����ΪGetDbInvalidData(),���INVALID_DATA�궨��,���㲻ͬ�İ汾���岻ͬ����Ч�ֽ�
 ---2009-4-17:---V1.1.07----᯼���---
	1.����ϵͳ�ⰴֵ��������ӿ�,��ʱ�䲻����ʱ,����������ʽ��������,û�а��ո�ʽ����ÿ�����ֵΪ��Ч���ݵ�BUG
 ---2009-4-17:---V1.1.08----᯼���---
	1.�汾����֧�ֿ�ʼû������֧�ְ汾����,����������֧�֣��ļ��оɳ��ȵ���0,Ҳ��Ϊ���ļ���Ч
 ---2009-4-17:---V1.1.09----᯼���---
 	1.ֻ����һ��BANK��SECT���ļ�����С�ڵ���8ʱ,�Ŵ�ӡ�ļ��Ҳ�������Ϣ,�����ļ����ӡ����
 ---2009-7-30:---V1.1.10----᯼���---
 	1.����������ִ��DoTrigerSaveBank()ʱ,�Դ�����־���б���,������ԭ��־,�ٽ����ļ��������,
 	  ���Ᵽ���������־,���º����´���������ļ������±�־����,ʵ����û������
 ---2009-8-13:---V1.1.11----᯼���---
	1.����������Ⱥ���GetItemLen(),�������ID,���ȵļ�������ID�ĳ��Ⱥ�,��������ǰ��������ID��ϵͳ��
	  �е����ó���,�����ĺô������ID��ϵͳ���еĳ�������Ϊ1����,�����˷Ѵ洢��Դ
 ---2009-9-24:---V1.1.12----᯼���---
	1.���ӽӿ�int GetItemsLen(TBankItem* pBankItem, WORD wNum);
	2.TItemDesc��wRW�ֶ�,����ʱ֧��DI_NTS,��ʾ�������֧��ʱ��,
	  ��Ҫ����BANK1�����BANK,�����д�ʱ��Ͳ���ʱ���,�����ֲ��ֿܷ�SECT�����,
	  ��������BANK���ó�֧��ʱ�����,���������������óɲ�֧��ʱ��
 ---2009-10-12:---V1.1.13----᯼���---
	1.����ʹ�ò����㶯̬ӳ��Ŀ�ID�е���ID,ƫ�Ƽ��㲻�Ե�����,��ID��ƫ��ʹ���µļ��㷽��
	2.��ϵͳ�����ñ��е�һЩ��������Զ�����,����:
	  a.��ID�е���ID�Ĳ��������ǿ��Ϊ1;
	  b.�õ������㶯̬ӳ���,�����������ӳ�䷽����ͬ
 *********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "sysfs.h"
#include "LibDbAPI.h"
#include "ComAPI.h"
#include "DbHook.h"
#include "Trace.h"
#include "sysapi.h"

#define VER_STR	"Ver1.1.40"

extern TSem   g_semDataRW;
extern TTime g_tmAccessDenied;
///////////////////////////////////////////////////////////////////////////////////////////////////////////
//CDataManager

CDataManager::CDataManager()
{
	m_pDbCtrl = NULL;
	m_pImgCtrl = NULL;
}

CDataManager::~CDataManager()
{
}

int CDataManager::Save(bool fSaveAll)
{
	int iRet = 0;
	//�ȱ������
	iRet += SavePara();
	
	//�ٱ�������
	iRet += SaveData(fSaveAll);
	
    return iRet;
}

		
//����:�������,��׺��Ϊ.cfg���ļ�
int CDataManager::SavePara()
{
	WORD i;
	int iLen;
	int iRet = 0;
	TBankCtrl* pBankCtrl;
	
	for (i=0; i<m_wSectNum; i++)
	{
		pBankCtrl = &m_pBank0Ctrl[i];
		if (pBankCtrl->pItemDesc!=NULL && pBankCtrl->pszPathName!=NULL)   //��BANK����Ҫ������
		{
			if ((iLen=strlen(pBankCtrl->pszPathName)) > 0)
			{
				if (strcmp(&pBankCtrl->pszPathName[iLen-4], ".cfg") == 0)
				{
					if (SaveBank(pBankCtrl) < 0)
						iRet--;
				}
			}
		}
	}

	for (i=0; i<m_wBankNum; i++)
	{
		pBankCtrl = &m_pBankCtrl[i];
		if (pBankCtrl->pItemDesc!=NULL && pBankCtrl->pszPathName!=NULL)   //��BANK����Ҫ������
		{
			if ((iLen=strlen(pBankCtrl->pszPathName)) > 0)
			{
				if (strcmp(&pBankCtrl->pszPathName[iLen-4], ".cfg") == 0)
				{
					if (SaveBank(pBankCtrl) < 0)
						iRet--;
				}
			}
		}
	}
	
	return iRet;
}

//����:��������,��׺������.cfg���ļ�
int CDataManager::SaveData(bool fSaveAll)
{
	WORD i;
	int iLen;
	int iRet = 0;
	TBankCtrl* pBankCtrl;
	
	for (i=0; i<m_wSectNum; i++)
	{
		pBankCtrl = &m_pBank0Ctrl[i];
		if (pBankCtrl->pItemDesc!=NULL && pBankCtrl->pszPathName!=NULL)   //��BANK����Ҫ������
		{
			if (!fSaveAll && pBankCtrl->wSaveInterv!=0)	//�������屣�� && ��BANK���ݰ��յ����ļ�����б���
				continue;

			if ((iLen=strlen(pBankCtrl->pszPathName)) > 0)
			{
				if (strcmp(&pBankCtrl->pszPathName[iLen-4], ".cfg") != 0)
				{
					if (SaveBank(pBankCtrl) < 0)
						iRet--;
				}
			}
		}
	}

	for (i=1; i<m_wBankNum; i++)
	{
		pBankCtrl = &m_pBankCtrl[i];
		if (pBankCtrl->pItemDesc!=NULL && pBankCtrl->pszPathName!=NULL)   //��BANK����Ҫ������
		{
			if (!fSaveAll && pBankCtrl->wSaveInterv!=0)	//�������屣�� && ��BANK���ݰ��յ����ļ�����б���
				continue;

			if ((iLen=strlen(pBankCtrl->pszPathName)) > 0)
			{
				if (strcmp(&pBankCtrl->pszPathName[iLen-4], ".cfg") != 0)
				{
					if (SaveBank(pBankCtrl) < 0)
						iRet--;
				}
			}
		}
	}
	
	return iRet;
}

int CDataManager::SaveBank(TBankCtrl* pBankCtrl, int iFile)
{
	if (pBankCtrl->pItemDesc==NULL || pBankCtrl->pszPathName==NULL)   //��BANK����Ҫ������
		return 0;

	int iRet = 0;
	WORD wStartFile = 0;
	if (iFile >= 0)
		wStartFile = (WORD )iFile;

	for (WORD i=wStartFile; i<pBankCtrl->wFileNum; i++)
	{
		//�����ļ����޸ı�־,���ղ�����
		WORD wByte = i >> 3; //��8
		BYTE bMask = 1 << (i & 0x07);
		if ((pBankCtrl->bModified[wByte] & bMask) == 0)
		{
			if (iFile >= 0)	//ֻ����һ���ļ�
				break;
				
			continue;
		}

		DWORD dwIndexNum;
		char szPathName[128];
		char szTimeFileName[128];
		sprintf(szPathName, pBankCtrl->pszPathName, i);

		WaitSemaphore(pBankCtrl->semBankRW);
		
		if (WriteFile(szPathName,
						pBankCtrl->pbBankData+pBankCtrl->dwFileSize*i, 
						pBankCtrl->dwFileSize))
		{
			if (pBankCtrl->pszBakPathName!=NULL && i==0)	//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ�
			{												//�����ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�����֧��ʱ��ı���
				char szBakPathName[128];	//�����ļ�������
				sprintf(szBakPathName, pBankCtrl->pszBakPathName, i);
				WriteFile(szBakPathName,
							pBankCtrl->pbBankData+pBankCtrl->dwFileSize*i, 
							pBankCtrl->dwFileSize);
			}

			DTRACE(DB_DB, ("CDataManager::SaveBank: save %s ok, dwFileSize=%ld, Click=%ld\n", 
				   			szPathName, pBankCtrl->dwFileSize, GetClick()));

			if (pBankCtrl->fUpdTime) //��BANK�����Ƿ���Ҫ����ʱ��
			{
				if (pBankCtrl->wImgNum > 1) //���������������
					dwIndexNum = pBankCtrl->dwIndexNum * pBankCtrl->wPnNum;
				else //if (pBankCtrl->wPnNum > 1) //���������
					dwIndexNum = pBankCtrl->dwIndexNum;
				
				sprintf(szTimeFileName, "%s.tm", szPathName);
				if (WriteFile(szTimeFileName, 
								(BYTE* )&pBankCtrl->pdwUpdTime[dwIndexNum*i], 
								dwIndexNum*sizeof(DWORD)))
				{
					pBankCtrl->bModified[wByte] &= ~bMask;
				}
				else
				{
					iRet--;
				}
			}
			else
			{
				pBankCtrl->bModified[wByte] &= ~bMask;
			}
		}
		else
		{
			iRet--;
		}
		
		SignalSemaphore(pBankCtrl->semBankRW);
		
		if (iFile >= 0)	//ֻ����һ���ļ�
			break;
	}

	pBankCtrl->dwSaveClick = GetClick();

	return iRet;
}

void CDataManager::DoTrigerSaveBank()
{
	WORD i, j;
	WORD wByte;
	BYTE bMask;

	SavePnMap(); //ֻҪ������ӳ�䷢���˸ı�,��Ӧ�����ϱ��浽�ļ�ϵͳ��ȥ,����ӳ�䷢������

	if (!m_fTrigerSaveBank)
		return;
		
	WaitSemaphore(g_semDataRW);
	m_fTrigerSaveBank = false;
	memcpy(m_bTmpSectSaveFlg, m_bSectSaveFlg, sizeof(m_bSectSaveFlg));
	memcpy(m_bTmpBankSaveFlg, m_bBankSaveFlg, sizeof(m_bBankSaveFlg));
	
	memset(m_bSectSaveFlg, 0, sizeof(m_bSectSaveFlg));
	memset(m_bBankSaveFlg, 0, sizeof(m_bBankSaveFlg));
	SignalSemaphore(g_semDataRW);
	
	DTRACE(DB_DB, ("CDataManager::DoTrigerSaveBank: start at click %ld\n", GetClick()));
	
	TBankCtrl* pBankCtrl;
	
	for (i=0; i<m_wSectNum; i++)
	{
		pBankCtrl = &m_pBank0Ctrl[i];
		if (pBankCtrl->pItemDesc == NULL)
			continue;
		
		for (j=0; j<pBankCtrl->wFileNum; j++)
		{
			wByte = j >> 3; //��8
			bMask = 1 << (j & 0x07);
			if (m_bTmpSectSaveFlg[i][wByte] & bMask)
			{
				SaveBank(pBankCtrl, j);
			}
		}
	}

	for (i=1; i<m_wBankNum; i++)
	{
		pBankCtrl = &m_pBankCtrl[i];
		if (pBankCtrl->pItemDesc == NULL)
			continue;
		
		for (j=0; j<pBankCtrl->wFileNum; j++)
		{
			wByte = j >> 3; //��8
			bMask = 1 << (j & 0x07);
			if (m_bTmpBankSaveFlg[i][wByte] & bMask)
			{
				SaveBank(pBankCtrl, j);
			}
		}
	}
	
	DTRACE(DB_DB, ("CDataManager::DoTrigerSaveBank: done at click %ld\n", GetClick()));
}

//����:�������м������
void CDataManager::DoSelfIntervSave()
{
	WORD i;
	TBankCtrl* pBankCtrl;

	DWORD dwClick = GetClick();
	for (i=0; i<m_wSectNum; i++)
	{
		pBankCtrl = &m_pBank0Ctrl[i];
		if (pBankCtrl->pItemDesc!=NULL && pBankCtrl->pszPathName!=NULL && pBankCtrl->wSaveInterv!=0)   
		{														//��BANK���ݰ������еļ�����б���
			if (pBankCtrl->wSaveInterv>m_pDbCtrl->wSaveInterv && dwClick<pBankCtrl->wSaveInterv*60)
			{	//��BANK�ı�������ϵͳ�����屣������ && ϵͳ��������ʱ�仹û�ﵽ��BANK�ı�����
				if (dwClick-pBankCtrl->dwSaveClick > m_pDbCtrl->wSaveInterv*60)	//�Ȱ���ϵͳ������������
					SaveBank(pBankCtrl);
			}
			else if (dwClick-pBankCtrl->dwSaveClick > pBankCtrl->wSaveInterv*60)
			{		//ϵͳ��������ʱ���Ѿ��ﵽ��BANK�ı�����,���ձ�BANK�ļ������
				SaveBank(pBankCtrl);
			}
		}
	}

	for (i=1; i<m_wBankNum; i++)
	{
		pBankCtrl = &m_pBankCtrl[i];
		if (pBankCtrl->pItemDesc!=NULL && pBankCtrl->pszPathName!=NULL && pBankCtrl->wSaveInterv!=0)
		{														//��BANK���ݰ������еļ�����б���
			if (pBankCtrl->wSaveInterv>m_pDbCtrl->wSaveInterv && dwClick<pBankCtrl->wSaveInterv*60)
			{	//��BANK�ı�������ϵͳ�����屣������ && ϵͳ��������ʱ�仹û�ﵽ��BANK�ı�����
				if (dwClick-pBankCtrl->dwSaveClick > m_pDbCtrl->wSaveInterv*60)	//�Ȱ���ϵͳ������������
					SaveBank(pBankCtrl);
			}
			else if (dwClick-pBankCtrl->dwSaveClick > pBankCtrl->wSaveInterv*60)
			{		//ϵͳ��������ʱ���Ѿ��ﵽ��BANK�ı�����,���ձ�BANK�ļ������
				SaveBank(pBankCtrl);
			}
		}
	}
}

//����:���һ��BANK�����ݣ������浽�ļ�ϵͳ��ȥ
//����:@wBank BANK��
//	   @wSect �����BANK0,��ʾBANK0�ڵĶκ�
//	   @iFile �ļ���,�ֱ��Ӧ������Ż��߾����,С��0ʱ��ʾȫ���ļ�
//��ע:Ŀǰ��֧��ָ����ĳ��IMG������,��ΪIMG�Ƕ�̬���ɵ�,Ҫ��͸�SECT�����о������ݶ���
void CDataManager::TrigerSaveBank(WORD wBank, WORD wSect, int iFile)
{
	WORD i;
	WORD wByte;
	BYTE bMask;
	int iPn;

	DTRACE(DB_DB, ("CDataManager::TrigerSaveBank: wBank=%d, wSect=%d, iFile=%ld\n", wBank, wSect, iFile));
	
	if (wBank>=m_wBankNum || wSect>=m_wSectNum)   //��BANK��Ҫ������
		return;

	TBankCtrl* pBankCtrl;
	if (wBank == BANK0)
		pBankCtrl = &m_pBank0Ctrl[wSect];
	else
		pBankCtrl = &m_pBankCtrl[wBank];

	if (pBankCtrl->pItemDesc == NULL) //�յ�BANK
		return;

	//�����BANKʹ�ò����㶯̬ӳ���Ұ��ղ����������ļ�����,����ļ���iFile����Ϊ��Ӧ�������ӳ���
	if (iFile>=0	//ָ������ĳ���ļ�
		&& pBankCtrl->bPnMapSch>0 && pBankCtrl->bPnMapSch<=m_wPnMapNum //������Ӧ����1~m_wPnMapNum��
		&& pBankCtrl->wPnNum>1 && pBankCtrl->wImgNum==1)	//���ղ����������ļ�����
	{	
		iPn = SearchPnMap(pBankCtrl->bPnMapSch, (WORD )iFile);	//��ʱiFileָ�����ǲ������
		if (iPn < 0)
			return;

		iFile = iPn;
	}

	if (iFile >= pBankCtrl->wFileNum)
		return;
		
	WaitSemaphore(g_semDataRW);
	
	if (wBank == BANK0)
	{
		if (iFile < 0)	//��������ļ�(ÿ�����������)
		{
			for (i=0; i<pBankCtrl->wFileNum; i++)
			{
				wByte = i >> 3; //��8
				bMask = 1 << (i & 0x07);
				m_bSectSaveFlg[wSect][wByte] |= bMask;
			}
		}
		else
		{
			wByte = (WORD )(iFile >> 3); //��8
			bMask = 1 << (iFile & 0x07);
			m_bSectSaveFlg[wSect][wByte] |= bMask;
		}	
	}
	else
	{
		if (iFile < 0)	//��������ļ�(ÿ�����������)
		{
			for (i=0; i<pBankCtrl->wFileNum; i++)
			{
				wByte = i >> 3; //��8
				bMask = 1 << (i & 0x07);
				m_bBankSaveFlg[wBank][wByte] |= bMask;
			}
		}
		else
		{
			wByte = (WORD )(iFile >> 3); //��8
			bMask = 1 << (iFile & 0x07);
			m_bBankSaveFlg[wBank][wByte] |= bMask;
		}	
	}	
	
	m_fTrigerSaveBank = true;
		
	SignalSemaphore(g_semDataRW);
}

//����:��������㶯̬ӳ���
int CDataManager::SavePnMap()
{
	WORD iRet = 0;
	if (m_dwPnMapFileFlg != 0) //�в�����ӳ���ļ��������޸�
	{
		DWORD dwFlg = 1;
		char szPathName[128];
		
		WaitSemaphore(m_semPnMap);
		
		for (WORD i=0; i<m_wPnMapNum; i++, dwFlg=dwFlg<<1)
		{
			if (m_dwPnMapFileFlg & dwFlg)
			{
				m_dwPnMapFileFlg &= ~dwFlg; //�ڱ���ǰ�����־,�Ա��ڱ���Ĺ����п��Է�Ӧ�µı仯
				//���ļ�ϵͳ�Ѳ�����ŵ��洢�ŵ�ӳ���ָ���pwPnToMemMap
				sprintf(szPathName, "%sPNMAP%d.cfg", m_pDbCtrl->pszDbPath, i); 
				if (!WriteFile(szPathName, (BYTE* )m_pPnMapCtrl[i].pwPnToMemMap, m_pPnMapCtrl[i].dwFileSize))
				{
					DTRACE(DB_DB, ("CDataManager::SavePnMap: fail to save %s\n", szPathName));
					iRet--;
				}
			}
		}
		
		SignalSemaphore(m_semPnMap);
	}

	return iRet;
}


void CDataManager::TrigerSaveAll()
{
	m_fTrigerSaveAll = true;
}

void CDataManager::TrigerSavePara()
{
	m_fTrigerSavePara = true;
}


void CDataManager::DoSave()
{
	bool fTrigerSaveAll;
	DWORD dwClick = GetClick();
	if (dwClick > 10)  //��������10�봥��һ�α�������,��Ҫ�����ڸ��ϵ��Դ���ȶ���ʱ�򱣴�����
	{				   
		if (m_fTrigerSaveAll || dwClick-m_dwSaveClick>m_pDbCtrl->wSaveInterv*60) //������,��λ����
		{	
			DTRACE(DB_DB, ("DoSave: @@@@@@@@@@@@ save start at click=%d.\r\n", dwClick));
			fTrigerSaveAll = m_fTrigerSaveAll;	//m_fTrigerSaveAllҪ����,����ʱ����
			m_fTrigerSaveAll = false;	//NOTICE:��־������ٽ���ʵ�ʱ���,�����������ò�����ʱ��,����ı�־�����Ͼͱ�һ�����,����ʵ��û����
			m_fTrigerSavePara = false;
			Save(fTrigerSaveAll);
			m_dwSaveClick = dwClick;
			DTRACE(DB_DB, ("DoSave: @@@@@@@@@@@@ save done at click=%d.\r\n", GetClick()));
		}

		if (m_fTrigerSavePara)
		{
			DTRACE(DB_DB, ("DoSave: @@@@@@@@@@@@ save para start at click=%d.\r\n", dwClick));
			m_fTrigerSavePara = false; //NOTICE:��־������ٽ���ʵ�ʱ���,�����������ò�����ʱ��,����ı�־�����Ͼͱ�һ�����,����ʵ��û����
			SavePara();
			DTRACE(DB_DB, ("DoSave: @@@@@@@@@@@@ save para done at click=%d.\r\n", GetClick()));
		}
	}
	
	DoTrigerSaveBank();
	DoSelfIntervSave();
}


//����:װ��1���ļ���Ĭ������,
//		1.���������д��ڶ�������������,��ʵװ���������BANK�Ķ���������Ĭ��ֵ,
//		  ��װ����Ǹ��������������Ķ���������Ĭ������;
//		2.���������в����ڶ�������������ʱ,��װ����Ǹ���������������1���������Ĭ������
//		  (������TBankCtrl�еĶ���������е�һ��)
//����:@pBankCtrl BANK���ƽṹ
//	   @wFile �ļ���,�ֱ��Ӧ������Ż��߾���� 
//	   @dwOffset һ���ļ����ƫ��,��Ҫ�����Щ�汾�����˸ı��BANK,
//				��������չ���µ�������,��װ���ļ����ݵ�ʱ���Ѿ�װ����
//				ǰ��ԭ�еĲ���,������¼ӵĲ���װ��Ĭ����
//				������Щ�汾û�з����ı��BANK,�ѱ�������Ϊ0
void CDataManager::LoadOneFileDefault(TBankCtrl* pBankCtrl, WORD wFile, DWORD dwOffset)
{
	if (pBankCtrl->pbDefault == NULL)	//��BANKû��Ĭ��ֵ,ֱ��ȡ0
	{
		memset(pBankCtrl->pbBankData+pBankCtrl->dwFileSize*wFile+dwOffset, 
				0, pBankCtrl->dwFileSize-dwOffset);
		return;
	}

	if (!pBankCtrl->fMutiPnInDesc)  //�������в����ڶ�������������
	{
		memcpy(pBankCtrl->pbBankData+pBankCtrl->dwFileSize*wFile+dwOffset, 
			   pBankCtrl->pbDefault+dwOffset,
			   pBankCtrl->dwFileSize-dwOffset);

		return;
	}

	TItemDesc* pItemDesc = pBankCtrl->pItemDesc;
	DWORD num = pBankCtrl->dwItemNum;
	BYTE* pbDst = pBankCtrl->pbBankData + pBankCtrl->dwFileSize*wFile;
	BYTE* pbDst0 = pbDst;
	BYTE* pbSrc = pBankCtrl->pbDefault;
	for (DWORD i=0; i<num; i++)
    {
		if (pItemDesc[i].bSelfItem)		 //�����������Գɶ���������
		{
			for (WORD j=0; j<pItemDesc[i].wPnNum; j++)
			{
				if ((DWORD )(pbDst-pbDst0) >= dwOffset)
					memcpy(pbDst, pbSrc, pItemDesc[i].wLen);
					
				pbDst += pItemDesc[i].wLen;
			}

			pbSrc += pItemDesc[i].wLen;
		}
	}
}


//����:װ��1��BANK��Ĭ������,
//����:@pBankCtrl BANK���ƽṹ
//	   @iFile �ļ���,�ֱ��Ӧ������Ż��߾����,С��0ʱ��ʾװ����BANKĬ������ 
//	   @dwOffset һ���ļ����ƫ��,��Ҫ�����Щ�汾�����˸ı��BANK,
//				��������չ���µ�������,��װ���ļ����ݵ�ʱ���Ѿ�װ����
//				ǰ��ԭ�еĲ���,������¼ӵĲ���װ��Ĭ����
//				������Щ�汾û�з����ı��BANK,�ѱ�������Ϊ0
void CDataManager::LoadBankDefault(TBankCtrl* pBankCtrl, int iFile, DWORD dwOffset)
{
	if (iFile < 0)  //װ������BANK��Ĭ��ֵ
	{
		for (WORD i=0; i<pBankCtrl->wFileNum; i++)
			LoadOneFileDefault(pBankCtrl, i, dwOffset);
	}
	else
	{
		LoadOneFileDefault(pBankCtrl, iFile, dwOffset);
	}
}

//����:��ʼ��һ��Bank���ݿ�
bool CDataManager::InitBank(TBankCtrl* pBankCtrl)
{
	pBankCtrl->dwMemUsage = 0; 	//�ڴ�ʹ����,��λ�ֽ�,�������ݺ�ʱ��洢�ռ�
	pBankCtrl->dwSaveClick = 0; //��BANK���ݱ����ʱ��

	if (pBankCtrl->pItemDesc == NULL)  //��Ч�����ݿ��������
		return true;	
	
	if (!InitItemDesc(pBankCtrl)) //��ʼ��������������
		return false;

	if (pBankCtrl->pbBankData)
		delete []  pBankCtrl->pbBankData;

	pBankCtrl->pbBankData = NULL;
	if (pBankCtrl->wPnNum==0 && pBankCtrl->wImgNum==0)
	{	//��BANK��ֻ��Ϊ������������,���������ݷ���Ҫ����Ӧ�Ķ�д����
		DTRACE(DB_DB, ("CDataManager::InitBank: <%s> ---just for item desc--- init ok, dwItemNum=%ld, dwBankSize=%ld, wPnNum=%d, wImgNum=%d\n", 
				   		pBankCtrl->pszBankName,
				   		pBankCtrl->dwItemNum,
				   		pBankCtrl->dwBankSize,
				   		pBankCtrl->wPnNum, pBankCtrl->wImgNum));

		return true;
	}	

	//�����BANKʹ�ò����㶯̬ӳ��,��pBankCtrl->wPnNum����Ϊӳ�䷽�������õ�ʵ��֧�ֵĲ�������
	if (pBankCtrl->bPnMapSch>0 && pBankCtrl->bPnMapSch<=m_wPnMapNum) //������Ӧ����1~m_wPnMapNum��
	{	
		pBankCtrl->wPnNum = m_pPnMapCtrl[pBankCtrl->bPnMapSch-1].wRealNum;
	}

	pBankCtrl->dwTotalSize = pBankCtrl->dwBankSize * pBankCtrl->wPnNum * pBankCtrl->wImgNum;
	pBankCtrl->pbBankData = new BYTE[pBankCtrl->dwTotalSize];
						  //����ж��������,��ֻ����һ���������в�����Ĵ󻺳�,
						  //pbBankDataָ���ܵ���ʼ��ַ

	if (pBankCtrl->pbBankData == NULL)
	{
		DTRACE(1, ("CDataManager::InitBank : critical error : sys out of memory.\r\n"));
		return false;
	}

	pBankCtrl->dwMemUsage += pBankCtrl->dwTotalSize; //�ڴ�ʹ����,��λ�ֽ�,�������ݺ�ʱ��洢�ռ�
	
	pBankCtrl->semBankRW = NewSemaphore(1);	//TODO:�ź����Ƿ񴴽��ɹ�

	if (pBankCtrl->wImgNum > 1)
	{
		pBankCtrl->wFileNum = pBankCtrl->wImgNum; //һ���ֳɶ��ٸ��ļ�
		pBankCtrl->dwFileSize = pBankCtrl->dwBankSize*pBankCtrl->wPnNum; //ÿ���ļ��Ĵ�С
	}
	else
	{
		pBankCtrl->wFileNum = pBankCtrl->wPnNum;
		pBankCtrl->dwFileSize = pBankCtrl->dwBankSize;
	}

	if (pBankCtrl->wFileNum > BANK_FILE_MAX)
	{
		DTRACE(DB_DB, ("CDataManager::InitBank : bank<%s>'s file num=%d can't over %d\n", 
					   pBankCtrl->pszBankName, pBankCtrl->wFileNum, BANK_FILE_MAX));
		return false;
	}

	DWORD dwIndexNum = pBankCtrl->dwIndexNum;
	if (pBankCtrl->fUpdTime)
	{
		DWORD dwTmIdxNum = pBankCtrl->dwIndexNum*pBankCtrl->wPnNum*pBankCtrl->wImgNum;
		pBankCtrl->pdwUpdTime = new DWORD[dwTmIdxNum];
		if (pBankCtrl->pdwUpdTime == NULL)
		{
			DTRACE(1, ("CDataManager::InitBank : critical error : sys out of memory.\r\n"));
			return false;
		}
		
		pBankCtrl->dwMemUsage += dwTmIdxNum*sizeof(DWORD); //�ڴ�ʹ����,��λ�ֽ�,�������ݺ�ʱ��洢�ռ�
		
		memset(pBankCtrl->pdwUpdTime, 0, sizeof(DWORD)*dwTmIdxNum);
		//TODO:����ʱ�����ļ��ж���
		
		if (pBankCtrl->wImgNum > 1) //���������������
			dwIndexNum = pBankCtrl->dwIndexNum * pBankCtrl->wPnNum;
		//else //if (pBankCtrl->wPnNum > 1) //���������
		//	dwIndexNum = pBankCtrl->dwIndexNum;
	}
	
	int len;
	BYTE bVer;
	DWORD dwVerLen, dwVerItemNum;
	BYTE* pbFileBuf;
	pBankCtrl->fOldFileExist = false;
	if (pBankCtrl->pszPathName != NULL)   //��BANK��Ҫ������
	{
		bool fLoadOk;
		char szPathName[128];
		char szTimeFileName[128];
		for (WORD i=0; i<pBankCtrl->wFileNum; i++)
		{
			fLoadOk = true;
			sprintf(szPathName, pBankCtrl->pszPathName, i);
			
			//��ʼ��Bank3����
			pbFileBuf = pBankCtrl->pbBankData + pBankCtrl->dwFileSize*i;
			len = readfile(szPathName, pbFileBuf, pBankCtrl->dwFileSize);
						//Ԥ����󳤶ȶ�ȡ�ļ�,�ļ����Ȳ����ϵ�ʱ��ɾ���ļ�
						//������������Ȳ����ϵ��ļ���ʱ��ɾ��Ҳû��ϵ,
						//��ΪWriteFile()�жϵ��ļ����Ȳ����ϵ�ʱ�����дǰ��ɾ���ļ�


			if (len<=0 && pBankCtrl->pszBakPathName!=NULL && i==0)	//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ�
			{														//�����ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�����֧��ʱ��ı���
				char szBakPathName[128];	//�����ļ�������
				sprintf(szBakPathName, pBankCtrl->pszBakPathName, i);
				len = readfile(szBakPathName, pbFileBuf, pBankCtrl->dwFileSize);
				if (len > 0)
				{
					DTRACE(DB_DB, ("CDataManager::InitBank: use bak file %s\n", szBakPathName));
				}
			}

			if (len == (int )pBankCtrl->dwFileSize)	//�ļ����ȸպ����,�汾û�з����ı�
			{
				pBankCtrl->fOldFileExist = true;	 //�ɰ汾�ļ�����,���ȱ�����ȫ����
				if (pBankCtrl->fUpdTime) //��BANK�����Ƿ���Ҫ����ʱ��
				{
					sprintf(szTimeFileName, "%s.tm", szPathName);
					if (!ReadFile(szTimeFileName, 
								  (BYTE* )&pBankCtrl->pdwUpdTime[dwIndexNum*i], 
								  dwIndexNum*sizeof(DWORD)))
					{
						if (pBankCtrl->pszBakPathName!=NULL && i==0) //Ϊ�����ӿɿ��ԣ������ļ����Բ�����ʱ�꣬��Ҫ��Բ�����0��������
						{
							memset((BYTE* )&pBankCtrl->pdwUpdTime[dwIndexNum*i], 0, dwIndexNum*sizeof(DWORD));
						}
						else
						{
							fLoadOk = false;
							DTRACE(DB_DB, ("CDataManager::InitBank: fail to read time file %s, clr all data\n", szTimeFileName, i));
						}
					}
				}
			}
			else if (len>0 && len<(int )pBankCtrl->dwFileSize && pBankCtrl->bVer!=0)
			{		//�汾�����˸ı�	&& ��BANK֧�ְ汾����
				bVer = pbFileBuf[0];
				memcpy(&dwVerLen, pbFileBuf+1, sizeof(DWORD));
				memcpy(&dwVerItemNum, pbFileBuf+5, sizeof(DWORD));

				if (bVer<pBankCtrl->bVer && (((DWORD )len==dwVerLen) || (dwVerLen==0)))//(DWORD )len==dwVerLen)
				{	//�汾�����˵��� && (�������ȵ��ھɵĳ���||���߸տ�ʼû�а汾����ʱ���ɳ��ȵ���0)
					DTRACE(DB_DB, ("CDataManager::InitBank: <%s> version change, fix old data with new default, ver old=%d new=%d, file len=%ld dwVerLen=%ld\n", 
									pBankCtrl->pszBankName, 
									bVer, pBankCtrl->bVer, len, dwVerLen));
					LoadBankDefault(pBankCtrl, i, len);

					if (pBankCtrl->fUpdTime) //��BANK�����Ƿ���Ҫ����ʱ��
					{
						sprintf(szTimeFileName, "%s.tm", szPathName);
						BYTE* pbUpdTime = (BYTE* )&pBankCtrl->pdwUpdTime[dwIndexNum*i];
						int iTimeLen = readfile(szTimeFileName, 
												pbUpdTime, 
												dwIndexNum*sizeof(DWORD)); //��Ԥ����󳤶ȶ�ȡ�ļ�
						if (iTimeLen==dwVerItemNum*sizeof(DWORD) 
							&& (DWORD )iTimeLen<dwIndexNum*sizeof(DWORD))	//�ɵ�ʱ���ļ��Ϸ�
						{
							memset(pbUpdTime+iTimeLen, 0, dwIndexNum*sizeof(DWORD)-iTimeLen);
										//�������ֵ�ʱ����0
						}
						else
						{
							fLoadOk = false;
							DTRACE(DB_DB, ("CDataManager::InitBank: fail to read time file %s, clr all data\n", szTimeFileName));
						}
					}
				}
				else
				{
					DTRACE(DB_DB, ("CDataManager::InitBank: <%s> version change, but verinfo mismatch, ver old=%d new=%d, file len=%ld dwVerLen=%ld\n", 
									pBankCtrl->pszBankName, 
									bVer, pBankCtrl->bVer, len, dwVerLen));
					fLoadOk = false;
				}
			}
			else
			{
				fLoadOk = false;
				if (pBankCtrl->wFileNum <= 8) //�����ļ����ӡ����
					DTRACE(DB_DB, ("CDataManager::InitBank: fail to read %s, use default\n", szPathName, i));
			}
			
			if (!fLoadOk)
			{
				LoadBankDefault(pBankCtrl, i, 0);
				if (pBankCtrl->fUpdTime)
					memset((BYTE* )&pBankCtrl->pdwUpdTime[dwIndexNum*i], 0, dwIndexNum*sizeof(DWORD));
			}

			if (pBankCtrl->bVer != 0) //��BANK֧�ְ汾����,������µİ汾��Ϣ���µ����ݿ�
			{
				memset(pbFileBuf, 0, BN_VER_LEN);
				pbFileBuf[0] = pBankCtrl->bVer;	//�汾
				memcpy(pbFileBuf+1, &pBankCtrl->dwFileSize, sizeof(DWORD));	//�ļ�����
				memcpy(pbFileBuf+5, &dwIndexNum, sizeof(DWORD));	//�ļ�����
			}
		} //for (i=0; i<pBankCtrl->wFileNum; i++)
	}
	else
	{
		LoadBankDefault(pBankCtrl, -1, 0); //-1 С��0ʱ��ʾװ����BANKĬ������ 

		//����Ҫ�����ļ���BANK����Ҫ�汾����
	}

	memset(pBankCtrl->bModified, 0, sizeof(pBankCtrl->bModified));
	
	DTRACE(DB_DB, ("CDataManager::InitBank: <%s> init ok, dwItemNum=%ld, dwIndexNum=%ld, dwDefaultSize=%ld, dwBankSize=%ld, wPnNum=%d, wImgNum=%d, dwTotalSize=%ld, wFileNum=%d, dwFileSize=%ld, dwMemUsage=%ld\n", 
				   pBankCtrl->pszBankName,
				   pBankCtrl->dwItemNum,
				   pBankCtrl->dwIndexNum,
				   pBankCtrl->dwDefaultSize, pBankCtrl->dwBankSize,
				   pBankCtrl->wPnNum, pBankCtrl->wImgNum,
				   pBankCtrl->dwTotalSize, pBankCtrl->wFileNum, pBankCtrl->dwFileSize,
				   pBankCtrl->dwMemUsage));

	return true;
}

//����:ɾ��һ��Bank���������Դ
void CDataManager::DeleteBank(TBankCtrl* pBankCtrl)
{
	if (pBankCtrl->pbBankData)	//��BANK���ݿ������,
	{	
		FreeSemaphore(pBankCtrl->semBankRW);	//BANK���ݵĶ�д����

		delete [] pBankCtrl->pbBankData;
		pBankCtrl->pbBankData = NULL;	
	}

	if (pBankCtrl->pdwUpdTime)	  //��BANK���ݵĸ���ʱ��
	{	
		delete [] pBankCtrl->pdwUpdTime;
		pBankCtrl->pdwUpdTime = NULL;	
	}
}

//����:ɾ��һ��Bank���ļ�
void CDataManager::DeleteBankFile(TBankCtrl* pBankCtrl)
{
	if (pBankCtrl->pszPathName != NULL)   //��BANK��Ҫ������
	{
		char szPathName[128];
		char szTimeFileName[128];
		for (WORD i=0; i<pBankCtrl->wFileNum; i++)
		{
			sprintf(szPathName, pBankCtrl->pszPathName, i);
			DTRACE(DB_DB, ("CDataManager::DeleteBankFile: delete %s\r\n", szPathName));
			unlink(szPathName);

			if (pBankCtrl->pszBakPathName!=NULL && i==0)	//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ�
			{												//�����ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�����֧��ʱ��ı���
				char szBakPathName[128];	//�����ļ�������
				sprintf(szBakPathName, pBankCtrl->pszBakPathName, i);
				unlink(szBakPathName);
			}

			if (pBankCtrl->fUpdTime) //��BANK�����Ƿ���Ҫ����ʱ��
			{
				sprintf(szTimeFileName, "%s.tm", szPathName);
				DTRACE(DB_DB, ("CDataManager::DeleteBankFile: delete %s\r\n", szTimeFileName));
				unlink(szTimeFileName);
			}
		}
	}
}

//����:���һ��BANK�����ݣ������浽�ļ�ϵͳ��ȥ
//����:@wBank BANK��
//	   @wSect �����BANK0,��ʾBANK0�ڵĶκ�
//	   @iPn �������,С��0ʱ��ʾȫ���ļ�
//��ע:Ŀǰ֧����IMG�е�ĳ�������������,Ҫ�����о��������ĳ�������������
bool CDataManager::ClearBankData(WORD wBank, WORD wSect, int iPn)
{
	char szPathName[128];
	char szTimeFileName[128];
	if (wBank>=m_wBankNum || wSect>=m_wSectNum)   //��BANK��Ҫ������
		return false;

	TBankCtrl* pBankCtrl;
	if (wBank == BANK0)
		pBankCtrl = &m_pBank0Ctrl[wSect];
	else
		pBankCtrl = &m_pBankCtrl[wBank];

	if (pBankCtrl->pItemDesc == NULL) //�յ�BANK
		return true;

	if (iPn >= pBankCtrl->wPnNum)
		return false;
		
	WaitSemaphore(pBankCtrl->semBankRW);

	if (iPn < 0)	//��������ļ�(ÿ�����������)
	{
		LoadBankDefault(pBankCtrl, -1, 0); //-1 С��0ʱ��ʾװ����BANKĬ������   //memset(pBankCtrl->pbBankData, 0, pBankCtrl->dwTotalSize);

		if (pBankCtrl->fUpdTime)
			memset(pBankCtrl->pdwUpdTime, 0, pBankCtrl->dwIndexNum*pBankCtrl->wPnNum*pBankCtrl->wImgNum*sizeof(DWORD));
	
		for (WORD i=0; i<pBankCtrl->wFileNum; i++)
		{
			//ɾ�������ļ�
			sprintf(szPathName, pBankCtrl->pszPathName, i);
			unlink(szPathName);
			
			if (pBankCtrl->pszBakPathName!=NULL && i==0)	//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ�
			{												//�����ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�����֧��ʱ��ı���
				char szBakPathName[128];	//�����ļ�������
				sprintf(szBakPathName, pBankCtrl->pszBakPathName, i);
				unlink(szBakPathName);
			}

			//ɾ��ʱ���ļ�
			if (pBankCtrl->fUpdTime) //��BANK�����Ƿ���Ҫ����ʱ��
			{	
				sprintf(szTimeFileName, "%s.tm", szPathName);
				unlink(szTimeFileName);
			}
			
			pBankCtrl->bModified[i/8] &= ~(1<<(i%8));
		}
	}
	else	//���ĳ���ļ�(��ĳ��������)
	{
		DWORD dwIndexNum = pBankCtrl->dwIndexNum;
		if (pBankCtrl->wImgNum > 1) //���������������
		{	
			//NOTICE:
			//�����������ĳ������������ݼ�ʱ���ʱ��,����������������ϵͳ���ڴ��е����ݲ���,
			//�������ݼ�ʱ�걣����ļ����ֲ�����,ȫ�����,������Ҫ��Ϊ�˼򻯲���,��������ı���ʱ��
			//�����������󲻸�λ,��ϵͳ�����ڴ��е���������ȷ��,��ʹ�����������λ
			//���ݲ�����ʧ,������������������
			
			for (DWORD dwImg=0; dwImg<pBankCtrl->wImgNum; dwImg++) //��ÿ��������ɾ��ÿ������������ݼ�ʱ��
			{
				//ɾ������
				BYTE* pbPnAddr = pBankCtrl->pbBankData + pBankCtrl->dwBankSize*(pBankCtrl->wPnNum*dwImg + (DWORD )iPn);
				memset(pbPnAddr, 0, pBankCtrl->dwBankSize);
				sprintf(szPathName, pBankCtrl->pszPathName, dwImg);	
				unlink(szPathName);	//�ļ����ֲ�����,ȫ�����

				//ɾ��ʱ��
				DWORD dwTimeIndex = dwIndexNum * (pBankCtrl->wPnNum*dwImg + (DWORD )iPn);
				if (pBankCtrl->fUpdTime)
				{
					memset(pBankCtrl->pdwUpdTime+dwTimeIndex, 0, dwIndexNum*sizeof(DWORD));
					sprintf(szTimeFileName, "%s.tm", szPathName);
					unlink(szTimeFileName);	//�ļ����ֲ�����,ȫ�����
				}

				pBankCtrl->bModified[iPn/8] |= (1<<(iPn%8));	//�����Զ������ʱ�����д�ļ�
			}
		}
		else //�Բ����������ļ�
		{
			LoadBankDefault(pBankCtrl, iPn, 0); //-1 С��0ʱ��ʾװ����BANKĬ������   //memset(pBankCtrl->pbBankData, 0, pBankCtrl->dwTotalSize);

			if (pBankCtrl->fUpdTime)
				memset(pBankCtrl->pdwUpdTime+dwIndexNum*(DWORD )iPn, 0, dwIndexNum*sizeof(DWORD));
			
			//ɾ�������ļ�
			sprintf(szPathName, pBankCtrl->pszPathName, iPn);	
			unlink(szPathName);
			if (pBankCtrl->pszBakPathName!=NULL && iPn==0)	//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ�
			{												//�����ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�����֧��ʱ��ı���
				char szBakPathName[128];	//�����ļ�������
				sprintf(szBakPathName, pBankCtrl->pszBakPathName, iPn);
				unlink(szBakPathName);
			}
			
						
			//ɾ��ʱ���ļ�
			if (pBankCtrl->fUpdTime) //��BANK�����Ƿ���Ҫ����ʱ��
			{	
				sprintf(szTimeFileName, "%s.tm", szPathName);
				unlink(szTimeFileName);
			}
	
			pBankCtrl->bModified[iPn/8] &= ~(1<<(iPn%8));
		}
	}
	
	SignalSemaphore(pBankCtrl->semBankRW);

	//if (pBankCtrl->pszPathName != NULL)
	//	TrigerSave();

	return true;
}


//����:���ָ��BANK/SECT,��������������ΪwPnNum��,ָ�������������
//��ע:���������ݵĳ��Ȳ��ܳ���256���ֽ�
bool CDataManager::ClrPnData(WORD wBank, WORD wSect, WORD wPnNum, WORD wPn)
{
	BYTE bBuf[256];
	if (wBank>=m_wBankNum || wSect>=m_wSectNum)
		return false;

	TBankCtrl* pBankCtrl;
	if (wBank == BANK0)
		pBankCtrl = &m_pBank0Ctrl[wSect];
	else
		pBankCtrl = &m_pBankCtrl[wBank];

	if (pBankCtrl->pItemDesc == NULL) //�յ�BANK
		return true;

	if (pBankCtrl->wPnNum > 1)	//��������֧�ְ�����BANK���ò�����������BANK
		return false;
	
	memset(bBuf, 0, sizeof(bBuf));	//Ŀǰ�ٶ����������ݶ�����̫��

	for (DWORD i=0; i<pBankCtrl->dwItemNum; i++)
	{
		if (pBankCtrl->pItemDesc[i].wPnNum == wPnNum)	//��������������ΪwPnNum
		{
			::WriteItemEx(wBank, wPn, pBankCtrl->pItemDesc[i].wID, bBuf, (DWORD )0);	//��������ʱ��
		}
	}

	return true;
}

//����:��ʼ��ϵͳ��TPnMapCtrl�ṹ,�����ļ�ϵͳ�Ѳ�����ŵ��洢�ŵ�ӳ���ָ���pwPnToMemMap
bool CDataManager::InitPnMap(TPnMapCtrl* pPnMapCtrl, WORD wNum)
{
	WORD i, j;
	WORD wMapNum;	//�Ѿ�ӳ��ĸ���
	WORD wMN;		//�洢��
	char szPathName[128];
	char szHeader[160];
	for (i=0; i<wNum; i++)
	{
		//��ʼ��TPnMapCtrl�ṹ
		pPnMapCtrl[i].dwFileSize = (pPnMapCtrl[i].wRealNum*2 + 2)*sizeof(WORD);	//ӳ�䱣����ļ���С,
																	//ǰ������WORD�������������Ϣ,���е�һ��WORD���Ѿ�ӳ��ĸ���,�ڶ�������
		pPnMapCtrl[i].wAllocSize = (pPnMapCtrl[i].wRealNum+7)/8;	//�洢�ռ�����Ĵ�С

		pPnMapCtrl[i].pwPnToMemMap = new WORD[pPnMapCtrl[i].dwFileSize]; //������ŵ��洢�ŵ�ӳ���(��Ҫ���浽�ļ�ϵͳ)
		pPnMapCtrl[i].pbAllocTab = new BYTE[pPnMapCtrl[i].wAllocSize];	 //�洢�ռ�����(�����浽�ļ�ϵͳ,��̬����)
		if (pPnMapCtrl[i].pwPnToMemMap==NULL || pPnMapCtrl[i].pbAllocTab==NULL)
		{
			DTRACE(DB_DB, ("CDataManager::InitBank : critical error : sys out of memory.\r\n"));
			return false;
		}

		memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);
		memset(pPnMapCtrl[i].pbAllocTab, 0, pPnMapCtrl[i].wAllocSize);

		//���ļ�ϵͳ�Ѳ�����ŵ��洢�ŵ�ӳ���ָ���pwPnToMemMap
		sprintf(szPathName, "%sPNMAP%d.cfg", m_pDbCtrl->pszDbPath, i);
		if (!ReadFile(szPathName, (BYTE* )pPnMapCtrl[i].pwPnToMemMap, pPnMapCtrl[i].dwFileSize))
		{
			DTRACE(DB_DB, ("CDataManager::InitPnMap: fail to read %s\n", szPathName));
			memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);

			if (pPnMapCtrl[i].fGenerateMapWhenNoneExist)	//��û��ӳ����ʱ���Զ�����һһ��Ӧ��ӳ�����Ҫ��Ӧ�԰汾����
			{
				wMapNum = pPnMapCtrl[i].pwPnToMemMap[0] = pPnMapCtrl[i].wRealNum; //�Ѿ�ӳ��ĸ���
				for (j=0; j<wMapNum; j++)
				{
					wMN = pPnMapCtrl[i].pwPnToMemMap[2+j*2+1] = j; //�洢�� wMN<pPnMapCtrl[i].wRealNum
					pPnMapCtrl[i].pwPnToMemMap[2+j*2] = j;		//�������<pPnMapCtrl[i].wMaxPn
					pPnMapCtrl[i].pbAllocTab[wMN/8] |= 1<<(wMN%8);	//��ʾ�ô洢�ռ��Ѿ�����
				}
			}
		}
		else
		{
			sprintf(szHeader, "%s :", szPathName);
			TraceBuf(DB_DB, szHeader, (BYTE* )pPnMapCtrl[i].pwPnToMemMap, pPnMapCtrl[i].dwFileSize);

			//���ݲ�����ŵ��洢�ŵ�ӳ���,��ʼ���洢�ռ�����
			wMapNum = pPnMapCtrl[i].pwPnToMemMap[0]; //�Ѿ�ӳ��ĸ���
			if (wMapNum > pPnMapCtrl[i].wRealNum)	//ӳ���ļ���Ϣ����
			{
				DTRACE(DB_DB, ("CDataManager::InitPnMap: err! wMapNum(%d)>wRealNum(%d) in %s\n", 
							   wMapNum, pPnMapCtrl[i].wRealNum, szPathName));
				memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);
			}
			else
			{
				for (j=0; j<wMapNum; j++)
				{
					wMN = pPnMapCtrl[i].pwPnToMemMap[2+j*2+1]; //�洢��
					if (pPnMapCtrl[i].pwPnToMemMap[2+j*2]<pPnMapCtrl[i].wMaxPn 	//�������
						&& wMN<pPnMapCtrl[i].wRealNum) 	//�洢��
					{
						pPnMapCtrl[i].pbAllocTab[wMN/8] |= 1<<(wMN%8);	//��ʾ�ô洢�ռ��Ѿ�����
					}
					else	//ӳ���ļ���Ϣ����
					{
						DTRACE(DB_DB, ("CDataManager::InitPnMap: err! wMN=%d, Pn=%d, wMaxPn=%d, wRealNum=%d in %s\n", 
									   wMN, pPnMapCtrl[i].pwPnToMemMap[2+j*2], pPnMapCtrl[i].wMaxPn,
									   pPnMapCtrl[i].wRealNum, szPathName));

						memset(pPnMapCtrl[i].pwPnToMemMap, 0, pPnMapCtrl[i].dwFileSize);
						memset(pPnMapCtrl[i].pbAllocTab, 0, pPnMapCtrl[i].wAllocSize);
						break;
					}
				}
			}
		}
	}

	return true;
}

//����:���ݿ�ĳ�ʼ��
//����:@pDbCtrl �������ݿ���в������õ����ݿ���ƽṹ
//����:����ɹ��򷵻�true,���򷵻�false
bool CDataManager::Init(TDbCtrl* pDbCtrl)
{
	m_pDbCtrl = pDbCtrl; //�������ݿ���в������õ����ݿ���ƽṹ

	//Ϊ�˷��ʷ���,����m_pDbCtrl�еĲ��ֱ�����������ֱ��ʹ��
	m_wSectNum = m_pDbCtrl->wSectNum;		//BANK0�е�SECT��Ŀ
	m_pBank0Ctrl = m_pDbCtrl->pBank0Ctrl;
	m_wBankNum = m_pDbCtrl->wBankNum;		//֧�ֵ�BANK��Ŀ
	m_pBankCtrl = m_pDbCtrl->pBankCtrl;
	m_iSectImg = m_pDbCtrl->iSectImg;		//485�������ݾ����,���û�������-1
	m_wImgNum = m_pDbCtrl->wImgNum;			//485�������ݾ������
	m_wPnMapNum = m_pDbCtrl->wPnMapNum;  	//֧�ֵ�ӳ�䷽����Ŀ,�������ݿⲻ֧�ֲ����㶯̬ӳ������Ϊ0
	m_pPnMapCtrl = m_pDbCtrl->pPnMapCtrl; 	//�������ݿⲻ֧�ֲ����㶯̬ӳ������ΪNULL
	m_pDbUpgCtrl = m_pDbCtrl->pDbUpgCtrl;

	if (m_wSectNum>SECT_MAX || m_wBankNum>BANK_MAX || m_wPnMapNum>PNMAP_MAX)
	{
		DTRACE(DB_DB, ("CDataManager::Init: the following var over max, wSectNum=%d(%d), wBankNum=%d(%d), m_wPnMapNum=%d(%d)\r\n",
					   m_wSectNum, SECT_MAX, 
					   m_wBankNum, BANK_MAX,
					   m_wPnMapNum, PNMAP_MAX));
		return false;
	}
	
	if (m_pDbCtrl->wSaveInterv == 0) //������,��λ����
		m_pDbCtrl->wSaveInterv = 15;	

	m_semPnMap = NewSemaphore(1);

	WORD i;
	m_fTrigerSaveBank = false;
	memset(m_bSectSaveFlg, 0, sizeof(m_bSectSaveFlg));
	memset(m_bBankSaveFlg, 0, sizeof(m_bBankSaveFlg));
	m_dwPnMapFileFlg = 0;

	m_fDbUpg = InitUpgrade(m_pDbUpgCtrl);

	m_dwMemUsage = 0;	  //�ڴ�ʹ����,��λ�ֽ�,�������ݺ�ʱ��洢�ռ�
	
	for (i=0; i<m_wSectNum; i++) 
	{
		if (InitBank(&m_pBank0Ctrl[i]) == false)
			return false;
		
		m_dwMemUsage += m_pBank0Ctrl[i].dwMemUsage;
	}

	for (i=0; i<m_wBankNum; i++)
	{
		if (InitBank(&m_pBankCtrl[i]) == false)
			return false;
	
		m_dwMemUsage += m_pBankCtrl[i].dwMemUsage;
	}
	
	if (m_wImgNum > 0)
	{	
		m_pImgCtrl = new TImgCtrl[m_wImgNum];
		if (m_pImgCtrl == NULL)
			return false;

		memset(m_pImgCtrl, 0, sizeof(TImgCtrl)*m_wImgNum);
	}

	if (!InitPnMap(m_pPnMapCtrl, m_wPnMapNum))
		return false;

	m_pbMeterPnMask = new BYTE[m_pDbCtrl->wPnMaskSize];
	if (m_pbMeterPnMask == NULL)
		return false;

	memset(m_pbMeterPnMask, 0, sizeof(m_pbMeterPnMask));

	DWORD dwTime = GetCurTime();
	TimeAdjBackward(dwTime); //����Ҫ����ʱ����������ʱ��ȫ��У��һ��

	if (m_fDbUpg)
	{
		DoUpgrade(m_pDbUpgCtrl);
		m_fDbUpg = false;
	}

	m_dwSaveClick = 0;
	m_fTrigerSaveAll = false;
	m_fTrigerSavePara = false;

	SetEmptyTime(&g_tmAccessDenied);
	DTRACE(DB_CRITICAL, ("CDataManager::Init: "VER_STR" init ok, MemUsage=%ld.\r\n", m_dwMemUsage));
	return true;
}


//����:��ʼ�����ݿ�汾����
bool CDataManager::InitUpgrade(TDbUpgCtrl* pDbUpgCtrl)
{
	if (pDbUpgCtrl == NULL)
		return false;

	WORD i;
	char szLog[UPG_LOG_LEN];
	char szFileBuf[UPG_LOG_LEN];
	char szPathName[128];

	//�ȽϾɵ�������¼,����ǰ�汾��û������,���������Ͳ�����
	sprintf(szPathName, "%sUpgLog.txt", m_pDbCtrl->pszDbPath);
	int len = readfile(szPathName, (BYTE* )szFileBuf, UPG_LOG_LEN); //Ԥ����󳤶ȶ�ȡ�ļ�,�ļ����Ȳ����ϵ�ʱ��ɾ���ļ�
	if (len > 0)
	{
		szFileBuf[UPG_LOG_LEN-1] = '\0';
		sprintf(szLog, "DbUpgVer%d", pDbUpgCtrl->bSchVer);
		if (strcmp(szLog, szFileBuf) == 0)	//��ǰ�汾�����Ѿ�����
		{
			DTRACE(DB_CRITICAL, ("CDataManager::InitUpgrade: upgrade undo due to %s already exist!\r\n", szLog));
			return false;
		}
	}

	for (i=0; i<pDbUpgCtrl->wBankNum; i++)
	{
		if (InitBank(&pDbUpgCtrl->pBankCtrl[i]) == false)
			break;

		if (!pDbUpgCtrl->pBankCtrl[i].fOldFileExist && 	 //�ɰ汾�ļ�������
			pDbUpgCtrl->pBankCtrl[i].pbBankData!=NULL)	//�ҷ�������Դ
		{
			DeleteBank(&pDbUpgCtrl->pBankCtrl[i]);
		}


		//��ע:
		//���ȫ��BANK��û�ҵ����ļ���û���䵽��Դ,Ҳ����true,��DoUpgrade()ִ��һ��,
		//��������־����,�»ؾͲ�������ִ������������
	}

	if (i < pDbUpgCtrl->wBankNum)	//û��ȫ��ʼ����ȷ
	{
		for (i=0; i<pDbUpgCtrl->wBankNum; i++)
		{
			if (pDbUpgCtrl->pBankCtrl[i].pbBankData != NULL)	//�Ѿ�������Դ
				DeleteBank(&pDbUpgCtrl->pBankCtrl[i]);
		}

		DTRACE(DB_CRITICAL, ("CDataManager::InitUpgrade: fail to init bank.\r\n"));
		return false;
	}
	else
	{
		return true;
	}
}

void CDataManager::DoUpgrade(TDbUpgCtrl* pDbUpgCtrl)
{
	DWORD i;
	if (pDbUpgCtrl == NULL)
		return;

	//�ɰ汾ID�����°汾ID
	DTRACE(DB_DB, ("CDataManager::DoUpgrade: ID->ID upgrade!\r\n"));
	TIdUpgMap* pIdUpgMap = pDbUpgCtrl->pIdUpgMap;	//ID�л�ӳ���
	for (i=0; i<pDbUpgCtrl->dwIdNum; i++,pIdUpgMap++)
	{
		if (pIdUpgMap->pfnUpgFun != NULL)
		{	
			pIdUpgMap->pfnUpgFun(pIdUpgMap->wFrmBn, pIdUpgMap->wFrmId, 
								 pIdUpgMap->wToBn, pIdUpgMap->wToId,
								 GetItemPnNum(pIdUpgMap->wToBn, pIdUpgMap->wToId));
		}
		else
		{
			DefaultUpgFun(pIdUpgMap->wFrmBn, pIdUpgMap->wFrmId, 
						  pIdUpgMap->wToBn, pIdUpgMap->wToId,
						  GetItemPnNum(pIdUpgMap->wToBn, pIdUpgMap->wToId));
		}
	}

	//ɾ���ɰ汾�ļ�
	if (pDbUpgCtrl->fDelFile)
	{
		DTRACE(DB_DB, ("CDataManager::DoUpgrade: del files...\r\n"));
		for (i=0; i<pDbUpgCtrl->wBankNum; i++)
		{
			if (pDbUpgCtrl->pBankCtrl[i].pbBankData)	//��Դ��û�ͷ�
			{
				DeleteBankFile(&pDbUpgCtrl->pBankCtrl[i]);
			}
		}
	}

	//�����°汾�ļ�
	DTRACE(DB_DB, ("CDataManager::DoUpgrade: save files...\r\n"));
	Save();

	//�ͷžɰ汾��Դ
	DTRACE(DB_DB, ("CDataManager::DoUpgrade: free resources...\r\n"));
	for (i=0; i<pDbUpgCtrl->wBankNum; i++)
	{
		if (pDbUpgCtrl->pBankCtrl[i].pbBankData)	//��Դ��û�ͷ�
		{
			DeleteBank(&pDbUpgCtrl->pBankCtrl[i]);
		}
	}

	//����������־
	char szLog[UPG_LOG_LEN];
	char szPathName[128];
	DTRACE(DB_DB, ("CDataManager::DoUpgrade: save log...\r\n"));
	sprintf(szPathName, "%sUpgLog.txt", m_pDbCtrl->pszDbPath);

	memset(szLog, 0, sizeof(szLog));
	sprintf(szLog, "DbUpgVer%d", pDbUpgCtrl->bSchVer);
	WriteFile(szPathName, (BYTE* )szLog, UPG_LOG_LEN);

	//��λ�ն�
	if (pDbUpgCtrl->fRstCPU)
	{
		DTRACE(DB_DB, ("CDataManager::DoUpgrade: reset cpu...\r\n"));
		Sleep(3000);
		ResetApp();
	}
}

//����:�´���һ������,�ɳ����������ÿ�����򳭱��߳��ύ���������ʱ��,�ռ����ύ���ݵ���ʼʱ��ͼ��,
//	   ��Ϊ���Ǵ�������.���ݿ�����Ѿ����ڸ�������ľ���,���������л���ʱ������ͺ�1~2�����ύ
//     Ҫ��0������,��0�㾵�������00:00�Ѿ�����.������ڲ�ͬ����ʼʱ��,�����Ϊ���Ǵ�����ͬ�ľ���,
//	   ��ʼʱ����ͬ��,����һ�����������,���ȡ���ֵ
//����:@dwStartTime �������ʼʱ��,��λS
//	   @wInterval ����ļ��,��λ��
void CDataManager::NewImg(DWORD dwStartTime, WORD wInterval)
{
	if (m_wImgNum==0 || m_iSectImg<0)
		return;

	WORD i;
	int iEmptyImg = -1;
	WORD wOldImg = 0;
	DWORD dwEndTime = 0;

	WaitSemaphore(m_pBank0Ctrl[m_iSectImg].semBankRW);

	//�ҳ�����ʱ����ǰ��һ��
	for (i=0; i<m_wImgNum; i++)
	{
		if (m_pImgCtrl[i].dwStartTime == dwStartTime) //�ҵ���ʱ����ͬ�ľ���
			break;
		
		if (iEmptyImg < 0)  //ֻ����û�ҵ��˿վ���������,���ҽ���ʱ����ǰ�ľ���
		{
			if (m_pImgCtrl[i].dwStartTime == 0) //�ҵ��˿յľ���
			{	
				iEmptyImg = i;
			}
			else if (dwEndTime==0 || m_pImgCtrl[i].dwEndTime<dwEndTime)
			{
				wOldImg = i;
				dwEndTime = m_pImgCtrl[i].dwEndTime;
			}
		}
	}
	
	//���¾���ĳ�ʱʱ��
	dwEndTime = (wInterval>>1) * 60;	//����Ľ���ʱ��,��λS,ȡ���ʱΪ1/2���,
	if (dwEndTime > IMG_MAX_TIMEOUTS)   //����5���ӵ�ȡ5����
		dwEndTime = IMG_MAX_TIMEOUTS;
	else if (dwEndTime == 0)			//��СҲҪȡ1����
		dwEndTime = 60;

	dwEndTime += dwStartTime + wInterval*60;

	if (i < m_wImgNum)  //�ҵ���ʱ����ͬ�ľ���
	{
		if (dwEndTime > m_pImgCtrl[i].dwEndTime)
			m_pImgCtrl[i].dwEndTime = dwEndTime;

		DTRACE(DB_DB, ("CDataManager::NewImg: get same img for %ld~%ldS\n", dwStartTime, m_pImgCtrl[i].dwEndTime));
	}
	else if (iEmptyImg >= 0) //�ҵ��˿յľ���
	{
		m_pImgCtrl[iEmptyImg].dwStartTime = dwStartTime;
		m_pImgCtrl[iEmptyImg].dwEndTime = dwEndTime;

		DTRACE(DB_DB, ("CDataManager::NewImg: get empty img for %ld~%ldS\n", dwStartTime, dwEndTime));
	}
	else  //ʹ�ý���ʱ����ǰ�ľ���
	{
		DTRACE(DB_DB, ("CDataManager::NewImg: replace %ld~%ldS' img with %ld~%ldS\n", 
						m_pImgCtrl[wOldImg].dwStartTime, m_pImgCtrl[wOldImg].dwEndTime,
						dwStartTime, dwEndTime));

		m_pImgCtrl[wOldImg].dwStartTime = dwStartTime;
		m_pImgCtrl[wOldImg].dwEndTime = dwEndTime;
	}

	SignalSemaphore(m_pBank0Ctrl[m_iSectImg].semBankRW);
}

//����:��ʱ����ǰ������dwTime(��),���ݿ���Ӧ�����ĵ���
void CDataManager::TimeAdjBackward(DWORD dwTime)
{
	DWORD i;
	TBankCtrl* pBankCtrl;

	if (m_wImgNum > 0)
	{
		WaitSemaphore(m_pBank0Ctrl[m_iSectImg].semBankRW);
		
		//����ʱ�̺�ľ���ɾ��
		for (i=0; i<m_wImgNum; i++)
		{
			if (m_pImgCtrl[i].dwStartTime >= dwTime) 
			{
				DTRACE(DB_DB, ("CDataManager::TimeAdjBackward: delet img %ld~%ldS at new time %ldS\n", 
							   m_pImgCtrl[i].dwStartTime, m_pImgCtrl[i].dwEndTime, dwTime));
		
				m_pImgCtrl[i].dwStartTime = 0;
				m_pImgCtrl[i].dwEndTime = 0;
				
				memset(&m_pBank0Ctrl[m_iSectImg].pdwUpdTime[m_pBank0Ctrl[m_iSectImg].dwItemNum*m_pBank0Ctrl[m_iSectImg].wPnNum*i], 
					   0, m_pBank0Ctrl[m_iSectImg].dwItemNum*m_pBank0Ctrl[m_iSectImg].wPnNum); 
			}
		}
		
		SignalSemaphore(m_pBank0Ctrl[m_iSectImg].semBankRW);
	}

	//������ʱ��ĵ���
	for (WORD wSect=0; wSect<m_wSectNum; wSect++) //�ն˲�����
	{
		if (m_pBank0Ctrl[wSect].fUpdTime && m_pBank0Ctrl[wSect].wImgNum<=1)
		{ //��Ҫ����ʱ��				&& ���Ǿ�������
			//WaitSemaphore(m_pBank0Ctrl[wSect].semBankRW); 
			//����pBankCtrl->pdwUpdTime[i] = 0;�ǵ�ָ�����,���Բ����ź�������
			 
			pBankCtrl = &m_pBank0Ctrl[wSect];
			DWORD dwTimeNum = pBankCtrl->dwIndexNum* pBankCtrl->wPnNum * pBankCtrl->wImgNum;
			for (i=0; i<dwTimeNum; i++)
			{
				if (pBankCtrl->pdwUpdTime[i] >= dwTime)
				{
					pBankCtrl->pdwUpdTime[i] = 0;
				}
			}

			//SignalSemaphore(m_pBank0Ctrl[wSect].semBankRW);
		}
	}

	for (WORD wBank=1; wBank<m_wBankNum; wBank++) //�ն˲�����
	{
		if (m_pBankCtrl[wBank].fUpdTime && m_pBankCtrl[wBank].wImgNum<=1)
		{ //��Ҫ����ʱ��				&& ���Ǿ�������
			//WaitSemaphore(m_pBankCtrl[wBank].semBankRW);
			//����pBankCtrl->pdwUpdTime[i] = 0;�ǵ�ָ�����,���Բ����ź�������
			 
			pBankCtrl = &m_pBankCtrl[wBank];
			DWORD dwTimeNum = pBankCtrl->dwIndexNum* pBankCtrl->wPnNum * pBankCtrl->wImgNum;
			for (i=0; i<dwTimeNum; i++)
			{
				if (pBankCtrl->pdwUpdTime[i] >= dwTime)
				{
					pBankCtrl->pdwUpdTime[i] = 0;
				}
			}

			//SignalSemaphore(m_pBankCtrl[wBank].semBankRW);
		}
	}
}

//����:ȡ������ĵ�ַ,��Ҫע�ⲻҪֱ���øõ�ַ��������,����Ҫʹ�����ݿ��ṩ�ĺ���ReadItem()��WriteItem()
TDataItem CDataManager::GetItem(WORD wPoint, WORD wID)
{
	TDataItem di;
	memset(&di, 0, sizeof(di));
	
	/*TItemDesc* pItemDesc = BinarySearchItem(g_TermnParaDesc, sizeof(g_TermnParaDesc)/sizeof(TItemDesc), wID);
	if (pItemDesc != NULL)
	{
		di.pbAddr = m_pbTermnPara + pItemDesc->wOffset;
		di.wLen = pItemDesc->wLen;
		di.pfModified = &m_fParaModified;
		return di;
	}*/

	return di;
}

//����:�������Ƿ���Ҫ����
bool CDataManager::IsMeterPn(WORD wPn)
{
	WORD wByte = wPn >> 3; //��8
	BYTE bMask = 1 << (wPn & 0x07);
	
	if (wByte >= m_pDbCtrl->wPnMaskSize)
	    return false;
	
	if (m_pbMeterPnMask[wByte] & bMask)
		return true;
	else
		return false;
}

//����:���þ������������λ
void CDataManager::SetMeterPnMask(BYTE* pbMeterPnMask)
{
	memcpy(m_pbMeterPnMask, pbMeterPnMask, m_pDbCtrl->wPnMaskSize);
}


bool CDataManager::IsImgItem(WORD wBank, WORD wPn, WORD wID)
{
	if (wBank == BANK0)
	{
		int iIndex;
		if (m_wImgNum>0 && m_iSectImg>=0 && IsMeterPn(wPn) && 
			(iIndex=BinarySearchIndex(m_pBank0Ctrl[m_iSectImg].pItemDesc, m_pBank0Ctrl[m_iSectImg].dwItemNum, wID))>=0)
		{
			return true;
		}
	}

	return false;
}

//��ע:$����ʲôʱ����Ҫ����
//		1.��Ҫ����Գ���Ĳ�����,���ͬʱ���ڲ�ͬʱ�����ĳ�������,����15����
//		  ��60���ӵ�,�����ʹ�þ���,����60����������,��15���ӹ��е�������
//		  ���ϵر����ȼ��ߵ�15��������ˢ��,����60�������ݵ�һ���Բ�̫��;
//		  ���ʹ���˾���,������������ͬʱҪ��901f,�����60���ӵ�������˵,
//		  ���ɼ�901f�����ȼ�����,��֤��901f�Ĳɼ�����ÿ��Сʱ��ǰ15�����ڲɼ�
//		2.���ڽ���/���������,��ͨ����Ӧ�ð��ղ�����ֿ�,�������ɺ����������
//		  �Ĳɼ��������յ�����������Ӱ��,�������ú�׼ʱ,���Ծ�û�б�Ҫʹ�þ���
//		3.����ͬʱ���ڳ��������ͽ���/���������ļ�������,�����ܼ���ļ���
//		  Ŀǰ�Ĳ���Ҫ�����������������ͬһʱ�̲ɼ�����,����Ҫ�������
//		  ������һ������ڱ��ֲ���,����/����ʹ�����µ������ϵ�ˢ���ܼӹ���,
//		  ���Գ��������ʹ�þ���,������/��������㲻ʹ�þ���,����Ŀǰ��ʹ��
//		  �����˵���ʺϵ�
int CDataManager::ReadItemEx(WORD wBank, WORD wPn, WORD wID, TItemAcess& rItemAcess, DWORD dwStartTime, DWORD dwEndTime)
{
	int iRet;

	if (wBank == BANK0)
	{
		int iIndex;
		if (m_wImgNum>0 && m_iSectImg>=0 && dwStartTime!=0 && IsMeterPn(wPn) 
			&& (iIndex=BinarySearchIndex(m_pBank0Ctrl[m_iSectImg].pItemDesc, m_pBank0Ctrl[m_iSectImg].dwItemNum, wID))>=0)
		{	//��ʱ�̵Ķ����������Ǿ����е�������
			
			//NOTE:ʲô��������Ҫ�Ӿ����ж�
			//1.���ɺ��������ݲ����ھ��������,IsMeterPn()����false,�������ݶ�����ǰ���ж�ȡ
			//2.���ɺ�����ĵ�ǰ������д��ʱ�򲻴�ʱ��,��WriteItemEx()��,д�뵽��ǰ��
			//3.���ɺ���������ݸ���������һ�����ύ��ѯ����,����������ʱ�����
			//4.ֱ�������ݶ�ͨ����ǰ��,��ͨ������,Ϊ�˱��⵽�����в���,
			//  ��ѯ��ʱ�궼Ҫ��������,��������һ�����շ��ӹ����Ĳ�ѯʱ��

			//���ʱ����Ӧ���ύ�����ĳ������ʱ��,��һ��Ҫ����������ȡ
			//���Ӧ�ÿ϶��Ǹ������ִ�е�����,������ֱ��
			for (WORD wImg=0; wImg<m_wImgNum; wImg++)
			{
				if (dwStartTime == m_pImgCtrl[wImg].dwStartTime) //�и�ʱ�̵ľ���
				{		
					return ::ReadItem(wImg, wPn, wID, rItemAcess, dwStartTime, dwEndTime, &m_pBank0Ctrl[m_iSectImg]);
				}
			}
			
			//����������Ǿ����е�������,�������ڸ�ʱ�̵ľ���,һ����ʱ��û����
			//��Ӧ����ֱ�ӳ���,ȥ��ǰ������
			//ֱ������Ӧ�����ᱣ֤�����Ĳ�ѯʱ��dwTime�������ӹ���,
			//������ɳ�����ʱ�̾��Ǿ���ʱ��,���һ��,��֤���ݵĲ�ѯ����ǰ���в�
			return ::ReadItem(IMG0, wPn, wID, rItemAcess, 
							  dwStartTime, dwEndTime, 
							  &m_pBank0Ctrl[m_pDbCtrl->wSectPnData]);
		}
		else
		{
			//�ٵ���ǰ������
			for (WORD wSect=0; wSect<m_wSectNum; wSect++) //�ն˲�����
			{
				if (wSect != m_iSectImg)
				{
					
					//ֻ�����ն��н���,��Ҫ���������²Ž�������
					
					//NOTE:���ں�ʱ�Ѳ�ѯ��ʱ���Զ���Ϊ0
					//���ɺ���������������,ʱ�궼�������ж�,��Ϊ��ǰ���е�����
					//�������µ�,���൱�ڵ�����˱�����;
					//Ӧ�ó����ڱ�д��ʱ�������������ݵĸ���ʱ��,����������ݵ�
					//ʱ��,�Ͳ����ж����ݵĸ���ʱ�����ж����ݽ�������ת��,
					//����ͨ���涨ִ�е���ʼʱ��������������
					
					DWORD dwStartTm = dwStartTime;
					DWORD dwEndTm = dwEndTime;
					if (IsSectHaveSampleData(wSect) && !IsMeterPn(wPn)) 
					{	//�κ��н�����������������   && �������������
						dwStartTm = dwEndTm= 0;
					}
						
					iRet = ::ReadItem(IMG0, wPn, wID, rItemAcess, dwStartTm, dwEndTm, &m_pBank0Ctrl[wSect]);
					if (iRet != -ERR_ITEM)
					{
						if (rItemAcess.bType == DI_ACESS_INFO)	//ȡ��������Ϣ(���ȺͶ�)
							rItemAcess.pItemInfo->wSect = wSect;
						else if (rItemAcess.pdwTime!=0 && IsSectHaveSampleData(wSect) && !IsMeterPn(wPn) && IsPnValid(wPn)) 
							*rItemAcess.pdwTime = GetCurTime();

						return iRet;
					}
				}
			}
		}
	}
	else if (wBank<m_wBankNum && m_pBankCtrl[wBank].pItemDesc!=NULL)
	{
		return ::ReadItem(IMG0, wPn, wID, rItemAcess, dwStartTime, dwEndTime, &m_pBankCtrl[wBank]);
	}
	
	return -ERR_ITEM;
}


int CDataManager::WriteItemEx(WORD wBank, WORD wPn, WORD wID, TItemAcess& rItemAcess, BYTE bPerm, BYTE* pbPassword, DWORD dwTime)
{
	int iRet;
	
	if (IsDbLocked())
		return -ERR_ITEM;
		
	if (wBank == BANK0)
	{
		int iIndex;
		if (m_wImgNum>0 && m_iSectImg>=0 && dwTime!=0 && IsMeterPn(wPn) 
			&& (iIndex=BinarySearchIndex(m_pBank0Ctrl[m_iSectImg].pItemDesc, m_pBank0Ctrl[m_iSectImg].dwItemNum, wID))>=0)
		{	//��ʱ�̵�д���������Ǿ����е�������,��Ҫд������
			
			WORD wImg;
			for (wImg=0; wImg<m_wImgNum; wImg++)
			{
				if (dwTime == m_pImgCtrl[wImg].dwStartTime) //�и�ʱ�̵ľ���
					break;
			}
			
			//���ھ����е�������,�뾵���ͬʱҲ�뵱ǰ���SECT,��ǰ���ʱ����õ�ǰ��ʱ�������,
			//��Ϊ��ǰ���ʱ�걣�ָ��������ݳ�����ʱ��һ��,����Ǿ������ݵĻ�,�п������ʱ�걻������
			//������ͳһ�õ�ǰʱ���뵱ǰ��
			DWORD dwCurTime = GetCurTime();	
			iRet = ::WriteItem(IMG0, wPn, wID, rItemAcess, 
							   bPerm, pbPassword, dwCurTime, 
							   &m_pBank0Ctrl[m_pDbCtrl->wSectPnData]);
														//ͬʱ������д����ǰ��
			if (wImg < m_wImgNum) //�ҵ���ʱ��ľ���
			{
				return ::WriteItem(wImg, wPn, wID, rItemAcess, bPerm, pbPassword, dwTime, &m_pBank0Ctrl[m_iSectImg]);
			}
			else
			{
				return iRet;
			}
		}
		else //д����ǰ��
		{
			//�ٵ���ǰ������
			for (WORD wSect=0; wSect<m_wSectNum; wSect++)
			{
				if (wSect != m_iSectImg)
				{
					iRet = ::WriteItem(IMG0, wPn, wID, rItemAcess, bPerm, pbPassword, dwTime, &m_pBank0Ctrl[wSect]);
					if (iRet != -ERR_ITEM)
						return iRet;
				}
			}
		}
	}
	else if (wBank<m_wBankNum && m_pBankCtrl[wBank].pItemDesc!=NULL)
	{
		return ::WriteItem(IMG0, wPn, wID, rItemAcess, bPerm, pbPassword, dwTime, &m_pBankCtrl[wBank]);
	}

	return -ERR_ITEM;
}

//����:���汾�����õ�,������ͬʱȡ���ݿ��е�ʱ��
int CDataManager::UpgReadItem(WORD wBank, WORD wPn, WORD wID, BYTE* pbBuf, DWORD* pdwTime)
{
	if (m_pDbUpgCtrl==NULL || !m_fDbUpg)
		return -ERR_ITEM;

	if (wBank<m_pDbUpgCtrl->wBankNum && 	//��BANK�ľɰ汾�ļ�����
		m_pDbUpgCtrl->pBankCtrl[wBank].pbBankData!=NULL)
	{
		TItemAcess ItemAcess;
		memset(&ItemAcess, 0, sizeof(TItemAcess));
		ItemAcess.bType = DI_ACESS_BUF;
		ItemAcess.pbBuf = pbBuf;
		ItemAcess.pdwTime = pdwTime;

		return ::ReadItem(IMG0, wPn, wID, ItemAcess, INVALID_TIME, INVALID_TIME, &m_pDbUpgCtrl->pBankCtrl[wBank]);
	}

	return -ERR_ITEM;
}

//����:ȡ������ĵ�ַ,��Ҫע�ⲻҪֱ���øõ�ַ��������,
//     ����Ҫʹ�����ݿ��ṩ�ĺ���ReadItem()��WriteItem()
TDataItem CDataManager::GetItemEx(WORD wBank, WORD wPn, WORD wID)
{
	int iRet;
	TDataItem di;
	memset(&di, 0, sizeof(di));
	
	TItemAcess ItemAcess;
	memset(&ItemAcess, 0, sizeof(ItemAcess));
	ItemAcess.bType = DI_ACESS_GI;
	ItemAcess.pbBuf = (BYTE* )&di;
	
	if (wBank == BANK0)
	{
		//�ٵ���ǰ������
		for (WORD wSect=0; wSect<m_wSectNum; wSect++) //�ն˲�����
		{
			if (wSect != m_iSectImg)
			{
				iRet = ::ReadItem(IMG0, wPn, wID, ItemAcess, 
								  INVALID_TIME, INVALID_TIME, 
								  &m_pBank0Ctrl[wSect]);
								  
				if (iRet != -ERR_ITEM)
					break;
			}
		}
	}
	else if (wBank < m_wBankNum) // && wBank!=BANK0
	{
		iRet = ReadItem(IMG0, wPn, wID, ItemAcess, 
						INVALID_TIME, INVALID_TIME, 
						&m_pBankCtrl[wBank]);
	} 

	return di;
}

void CDataManager::ClearBank(TBankCtrl* pBankCtrl)
{	
	if (pBankCtrl->pszPathName != NULL)   //��BANK��Ҫ������
	{
		for (WORD i=0; i<pBankCtrl->wFileNum; i++)
		{
			char szPathName[128];
			sprintf(szPathName, pBankCtrl->pszPathName, i);
			unlink(szPathName);   //ɾ��

			if (pBankCtrl->pszBakPathName!=NULL && i==0)	//�����ļ���·���ļ�����Ϊ�ձ�ʾ�����ݣ�
			{												//�����ļ���֧�ֶ��������ļ��ı��ݣ�ֻ֧�ֵ��ļ�����֧��ʱ��ı���
				char szBakPathName[128];	//�����ļ�������
				sprintf(szBakPathName, pBankCtrl->pszBakPathName, i);
				unlink(szBakPathName);
			}

		} //for (i=0; i<pBankCtrl->wFileNum; i++)
	}
}

void CDataManager::ClearData()
{
}

void CDataManager::ClearPara()
{
}

void CDataManager::SaveVolatile()
{
}

//����:���Ҳ������Ӧ��ӳ���
//����:����ҵ��򷵻ض�Ӧ��ӳ���,���򷵻�-1
int CDataManager::SearchPnMap(BYTE bSch, WORD wPn)
{
	if (bSch==0 || bSch>m_wPnMapNum)	//������Ӧ����1~PNMAP_NUM��
		return -1;

	TPnMapCtrl* pPnMapCtrl = &m_pPnMapCtrl[bSch-1];
	if (wPn >= pPnMapCtrl->wMaxPn)
		return -1;

	WaitSemaphore(m_semPnMap);	
	WORD wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //�Ѿ�ӳ��ĸ���
	if (wMapNum==0 || wMapNum>pPnMapCtrl->wRealNum)
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}

	int little, big, mid;
	little = 0;
	big = (int )wMapNum-1;
	while (little <= big)
	{                               
		mid = (little + big) / 2;  //����

		if (wPn == pPnMapCtrl->pwPnToMemMap[2+mid*2])
		{
			SignalSemaphore(m_semPnMap);
			return pPnMapCtrl->pwPnToMemMap[2+mid*2+1];
		}
		else if (wPn > pPnMapCtrl->pwPnToMemMap[2+mid*2])
		{
			little = mid + 1;
		} 
		else  //if (wPn < pPnMapCtrl.pwPnToMemMap[2+mid*2])
		{
			big = mid - 1;
		}

		mid = (little + big) / 2;
	}

	SignalSemaphore(m_semPnMap);
	return -1;
}

//����:����ӳ��Ŷ�Ӧ�Ĳ�����
//����:����ҵ��򷵻ض�Ӧ�Ĳ�����,���򷵻�-1
int CDataManager::MapToPn(BYTE bSch, WORD wMn)
{
	if (bSch==0 || bSch>m_wPnMapNum)	//������Ӧ����1~PNMAP_NUM��
		return -1;

	TPnMapCtrl* pPnMapCtrl = &m_pPnMapCtrl[bSch-1];

	WaitSemaphore(m_semPnMap);	
	WORD wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //�Ѿ�ӳ��ĸ���
	if (wMapNum==0 || wMapNum>pPnMapCtrl->wRealNum)
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}

	int iPn = -1;
	for (WORD i=0; i<wMapNum; i++)
	{                               
		if (wMn == pPnMapCtrl->pwPnToMemMap[2+i*2+1])
		{
			iPn = pPnMapCtrl->pwPnToMemMap[2+i*2];
			break;
		}
	}

	SignalSemaphore(m_semPnMap);
	return iPn;
}

//����:���붯̬ӳ�������
//����:@bSch ӳ�䷽��
// 	   @wPn ��Ҫӳ��Ĳ������
//����:�����ȷ����ӳ���,���򷵻�-1
int CDataManager::NewPnMap(BYTE bSch, WORD wPn)
{
	WORD i, j;
	int iMN = SearchPnMap(bSch, wPn);
	if (iMN >= 0)	//�Ѿ����ڸò������ӳ����
		return iMN;

	if (bSch==0 || bSch>m_wPnMapNum)	//������Ӧ����1~PNMAP_NUM��
		return -1;

	bSch--;	//ת����g_PnMapCtrl�����ж�Ӧ������
	TPnMapCtrl* pPnMapCtrl = &m_pPnMapCtrl[bSch];
	if (wPn >= pPnMapCtrl->wMaxPn)
	{
		DTRACE(DB_DB, ("CDataManager::NewPnMap: err, wPn(%d)>=wMaxPn(%d)\n", 
					   wPn, pPnMapCtrl->wMaxPn));
		return -1;
	}

	WaitSemaphore(m_semPnMap);	//SearchPnMap()Ҳ�������ͷ�m_semPnMap,��������ǰ����ź���
	WORD wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //�Ѿ�ӳ��ĸ���
	if (wMapNum >= pPnMapCtrl->wRealNum)
	{
		SignalSemaphore(m_semPnMap);
		DTRACE(DB_DB, ("CDataManager::NewPnMap: there is no room for pn=%d\n", wPn));
		return -1;
	}

	//����洢��(���߽�ӳ���)
	for (i=0; i<pPnMapCtrl->wAllocSize; i++)
	{
		if (pPnMapCtrl->pbAllocTab[i] != 0xff)	//��ûռ�õĿռ�
			break;
	}

	if (i >= pPnMapCtrl->wAllocSize)
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}

	for (j=0; j<8; j++)
	{
		if ((pPnMapCtrl->pbAllocTab[i]&(1<<j)) == 0)
			break;
	}

	if (j >= 8)	//Ӧ�ò�����������Ĵ���,�Է���һ
	{
		SignalSemaphore(m_semPnMap);
		return -1;
	}

	WORD wMN = i*8 + j;
	pPnMapCtrl->pbAllocTab[i] |= 1<<j;	//��־�ô洢�ռ��Ѿ�������

	//��������Ŵ�С����,ȷ���²�������ӳ����е�λ��
	for (i=0; i<wMapNum; i++)
	{
		if (wPn < pPnMapCtrl->pwPnToMemMap[2+i*2])
			break;
	}

	//Ų��һ������ӳ�������Ŀռ�
	for (j=wMapNum; j>i; j--)
	{
		pPnMapCtrl->pwPnToMemMap[2+j*2] = pPnMapCtrl->pwPnToMemMap[2+(j-1)*2];
		pPnMapCtrl->pwPnToMemMap[2+j*2+1] = pPnMapCtrl->pwPnToMemMap[2+(j-1)*2+1];
	}

	pPnMapCtrl->pwPnToMemMap[2+i*2] = wPn;
	pPnMapCtrl->pwPnToMemMap[2+i*2+1] = wMN;
	pPnMapCtrl->pwPnToMemMap[0]++;

	m_dwPnMapFileFlg |= (DWORD )1<<bSch;
	SignalSemaphore(m_semPnMap);

	DTRACE(DB_DB, ("CDataManager::NewPnMap: new map=%d for pn=%d\n", wMN, wPn));
	return wMN;
}

//����:ɾ��ӳ�������
//����:@bSch ӳ�䷽��
// 	   @wPn �Ѿ�ӳ��Ĳ������
//����:�����ȷ����true,���򷵻�false
bool CDataManager::DeletePnMap(BYTE bSch, WORD wPn)
{
	WORD i;
	if (bSch==0 || bSch>m_wPnMapNum)	//������Ӧ����1~PNMAP_NUM��
		return false;

	bSch--;	//ת����g_PnMapCtrl�����ж�Ӧ������
	TPnMapCtrl* pPnMapCtrl = &m_pPnMapCtrl[bSch];
	if (wPn >= pPnMapCtrl->wMaxPn)
		return false;

	WaitSemaphore(m_semPnMap);
	WORD wMapNum = pPnMapCtrl->pwPnToMemMap[0]; //�Ѿ�ӳ��ĸ���
	if (wMapNum==0 || wMapNum>pPnMapCtrl->wRealNum)
	{	
		SignalSemaphore(m_semPnMap);
		return false;
	}
	
	//��������Ŵ�С����,ȷ���²�������ӳ����е�λ��
	for (i=0; i<wMapNum; i++)
	{
		if (wPn == pPnMapCtrl->pwPnToMemMap[2+i*2])
			break;
	}

	if (i >= wMapNum)
	{
		SignalSemaphore(m_semPnMap);
		return false;
	}

	//ɾ����ӳ����Դ��ռ��
	WORD wMN = pPnMapCtrl->pwPnToMemMap[2+i*2+1];
	pPnMapCtrl->pbAllocTab[wMN/8] &= ~(1<<(wMN%8));	//��־�ô洢�ռ��Ѿ�������

	//�����ӳ��������ǰ�ƶ�,ռ�ñ�ɾ��������Ŀռ�
	for (; i<wMapNum-1; i++)
	{
		pPnMapCtrl->pwPnToMemMap[2+i*2] = pPnMapCtrl->pwPnToMemMap[2+(i+1)*2];
		pPnMapCtrl->pwPnToMemMap[2+i*2+1] = pPnMapCtrl->pwPnToMemMap[2+(i+1)*2+1];
	}

	pPnMapCtrl->pwPnToMemMap[0]--;
	m_dwPnMapFileFlg |= (DWORD )1<<bSch;
	SignalSemaphore(m_semPnMap);
	return true;
}

//�����BANKʹ�ò����㶯̬ӳ��,��pBankCtrl->wPnNum����Ϊӳ�䷽�������õ�ʵ��֧�ֵĲ�������
int CDataManager::GetPnMapRealNum(BYTE bSch)
{
	if (bSch>0 && bSch<=m_wPnMapNum) //������Ӧ����1~m_wPnMapNum��
		return m_pPnMapCtrl[bSch-1].wRealNum;
	else
		return -1;
}


