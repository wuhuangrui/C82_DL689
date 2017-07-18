/*********************************************************************************************************
 * Copyright (c) 2016,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Mem.h
 * ժ    Ҫ�����ļ�ʵ�ֶ�̬�ڴ�ռ����
 * ��ǰ�汾��1.0
 * ��    �ߣ������
 * ������ڣ�2016��10��
 * ��ע��
 *********************************************************************************************************/
#include "stdafx.h"
#include "Mem.h"
#include "FaCfg.h"
#include "Trace.h"
#include "ComAPI.h"


//���������䶯̬�ڴ�ռ�
//������@pbGlobal ȫ���ڴ�����
//		@pAllocTab ��̬�ڴ�����
//		@wTabNum �����ռ䳤��
//		@bType ��̬�ڴ����Ͷ���
//		@dwId �����Ͷ�Ӧ�ľ���ID��
//			�����¼:����ID��
//			�����쳣:�¼�OI
//			�ն��ڲ��¼�:�¼�OI+��������
//			ȫ�¼��ɼ����ɼ��������
//		@wDataLen ��Ҫ������ڴ泤��
//���أ������ȷ���뵽�ռ䣬����true
//      �������ʧ�ܣ��᷵��false
bool AllocMem(BYTE*pbGlobal, TAllocTab* pAllocTab, WORD wTabNum, BYTE bType, DWORD dwId, WORD wDataLen)
{
	char str[30];
	int iByte, iBit, iIndex;
	WORD wAllocSects = 0;	//�Ѿ�����Ŀ���
	DWORD dwTmpId = MemTypeIdToId(bType, dwId);
	WORD wNeedBlk = (wDataLen+MEMORY_BLOCK_SIZE-1) / MEMORY_BLOCK_SIZE;

	for (iIndex=0; iIndex<wTabNum; iIndex++)
	{
		if (pAllocTab[iIndex].dwId == dwTmpId)
		{
			return true; //�Ѿ�����
		}
	}

	for (iIndex=0; iIndex<wTabNum; iIndex++)
	{
		if (pAllocTab[iIndex].dwId==0 && pAllocTab[iIndex].wDataLen==0)
		{
			break; //�ҵ�һ���ձ�
		}
	}

	if (iIndex == wTabNum)
		return false; //û�з���������

	//��ȫ�ַ�������Ѽ��ռ�
	for (iByte=0; iByte<ALLOC_TAB_LEN; iByte++)
	{
		if (pbGlobal[iByte] == 0xff)
			continue;

		for (iBit=0; iBit<8; iBit++)
		{
			if (wAllocSects == wNeedBlk) //�������㹻�Ŀռ䣬��¼������
				goto Malloc_end;	//�п��ܻ��������һ���ڴ�Ÿպ÷����꣬���������������ж���Ҫ�ŵ�������

			if ((pbGlobal[iByte] & (1<<iBit)) == 0) //���ڴ�δ��ʹ�ã�ռ����
			{
				pAllocTab[iIndex].bAllocTab[iByte] |= (0x01<<iBit);
				wAllocSects++;
			}
		}	//for (bBit=0; bBit<8; bBit++)
	}	//for (iByte=0; iByte<ALLOC_TAB_LEN; iByte++)

Malloc_end:
	if (wAllocSects == wNeedBlk)
	{
		for (iByte=0; iByte<ALLOC_TAB_LEN; iByte++)
			pbGlobal[iByte] |= pAllocTab[iIndex].bAllocTab[iByte];//��¼ȫ�ֱ���

		pAllocTab[iIndex].dwId = dwTmpId;
		pAllocTab[iIndex].wDataLen = wDataLen;
		DTRACE(DB_CRITICAL, ("AllocMem: bType=%s dwId=0x%x, Sucess\r\n", MemTypeToStr(bType, str), dwId));
		return true;
	}

	memset(pAllocTab[iIndex].bAllocTab, 0, ALLOC_TAB_LEN);
	DTRACE(DB_CRITICAL, ("AllocMem: bType=%s dwId=0x%x fail, Because buf is full\r\n", MemTypeToStr(bType, str), dwId));
	return false;
}

//�������ͷŶ�̬�ڴ�ռ�
//������@pbGlobal ȫ���ڴ�����
//		@pAllocTab ��̬�ڴ�����
//		@wTabNum �����ռ䳤��
//		@bType ��̬�ڴ����Ͷ���
//		@dwId �����Ͷ�Ӧ�ľ���ID��
//			�����¼:����ID��
//			�����쳣:�¼�OI
//			�ն��ڲ��¼�:�¼�OI+��������
//			ȫ�¼��ɼ����ɼ��������
//���أ������ȷ�ͷſռ䣬����true
//      ����ͷ�ʧ�ܣ��᷵��false
bool FreeMem(BYTE*pbGlobal, TAllocTab* pAllocTab, WORD wTabNum, BYTE bType, DWORD dwId)
{
	DWORD dwTmpId = MemTypeIdToId(bType, dwId);

	for (int i=0; i<wTabNum; i++)
	{
		if (pAllocTab[i].dwId == dwTmpId)
		{
			for (int iByte=0; iByte<ALLOC_TAB_LEN; iByte++)
				pbGlobal[iByte] &= (~pAllocTab[i].bAllocTab[iByte]);//�����¼ȫ�ֱ���
			memset((BYTE *)&pAllocTab[i], 0, sizeof(TAllocTab));
			return true;
		}
	}

	return false;
}

//��������ȡ��̬�ڴ���������
//������@pAllocTab ��̬�ڴ�����
//		@wTabNum �����ռ䳤��
//		@pbMem ��̬�ڴ�ָ��
//		@bType ��̬�ڴ����Ͷ���
//		@dwId �����Ͷ�Ӧ�ľ���ID��
//			�����¼:����ID��
//			�����쳣:�¼�OI
//			�ն��ڲ��¼�:�¼�OI+��������
//			ȫ�¼��ɼ����ɼ��������
//		@pbData ���յĻ�����
//���أ������ȡ��ȷ��������ȷ����
//      �����ȡ���󣬷���-1
int ReadMem(TAllocTab* pAllocTab, WORD wTabNum, BYTE* pbMem, BYTE bType, DWORD dwId, BYTE* pbData)
{
	WORD wIndex;
	DWORD dwTmpId = MemTypeIdToId(bType, dwId);

	for (int i=0; i<wTabNum; i++)
	{
		if (pAllocTab[i].dwId == dwTmpId)
		{
			WORD wAllocBlk = CalcuBitNum(pAllocTab[i].bAllocTab, ALLOC_TAB_LEN);
			for (WORD wSect=0; wSect<wAllocBlk; wSect++)
			{
				wIndex = SchIndexInAllocTab(pAllocTab[i].bAllocTab, wSect);
				if (wIndex != 0xffff)
				{
					if (wSect < wAllocBlk-1)
						memcpy(pbData+wSect*MEMORY_BLOCK_SIZE, &pbMem[wIndex*MEMORY_BLOCK_SIZE], MEMORY_BLOCK_SIZE);
					else
						memcpy(pbData+wSect*MEMORY_BLOCK_SIZE, &pbMem[wIndex*MEMORY_BLOCK_SIZE], pAllocTab[i].wDataLen - wSect*MEMORY_BLOCK_SIZE);
				}
				else
				{
					return -1;
				}
			}
			return pAllocTab[i].wDataLen;
		}
	}

	return -1;
}

//��������̬�ڴ�д������
//������@pAllocTab ��̬�ڴ�����
//		@wTabNum �����ռ䳤��
//		@pbMem ��̬�ڴ�ָ��
//		@bType ��̬�ڴ����Ͷ���
//		@dwId �����Ͷ�Ӧ�ľ���ID��
//			�����¼:����ID��
//			�����쳣:�¼�OI
//			�ն��ڲ��¼�:�¼�OI+��������
//			ȫ�¼��ɼ����ɼ��������
//		@pbData Ҫд��Ļ�����
//���أ����д����ȷ��������ȷ����
//      ���д����󣬷���-1
int WriteMem(TAllocTab* pAllocTab, WORD wTabNum, BYTE* pbMem, BYTE bType, DWORD dwId, BYTE* pbData)
{
	WORD wIndex;
	DWORD dwTmpId = MemTypeIdToId(bType, dwId);

	for (int i=0; i<wTabNum; i++)
	{
		if (pAllocTab[i].dwId == dwTmpId)
		{
			WORD wAllocBlk = CalcuBitNum(pAllocTab[i].bAllocTab, ALLOC_TAB_LEN);
			for (WORD wSect=0; wSect<wAllocBlk; wSect++)
			{
				wIndex = SchIndexInAllocTab(pAllocTab[i].bAllocTab, wSect);
				if (wIndex != 0xffff)
				{
					if (wSect < wAllocBlk-1)
						memcpy(&pbMem[wIndex*MEMORY_BLOCK_SIZE], pbData+wSect*MEMORY_BLOCK_SIZE, MEMORY_BLOCK_SIZE);
					else
						memcpy(&pbMem[wIndex*MEMORY_BLOCK_SIZE], pbData+wSect*MEMORY_BLOCK_SIZE, pAllocTab[i].wDataLen - wSect*MEMORY_BLOCK_SIZE);
				}
				else
				{
					return -1;
				}
			}
			return pAllocTab[i].wDataLen;
		}
	}

	return -1;
}

//������ȡ�ö�̬�ڴ����Ч���ݳ���
//���أ���ȷ�򷵻ض�̬�ڴ����Ч���ݳ��ȣ�����-1
int GetMemLen(TAllocTab* pAllocTab, WORD wTabNum, BYTE bType, DWORD dwId)
{
	DWORD dwTmpId = MemTypeIdToId(bType, dwId);

	for (int i=0; i<wTabNum; i++)
	{
		if (pAllocTab[i].dwId == dwTmpId)
		{
			return pAllocTab[i].wDataLen;
		}
	}

	return -1;
}

//��������bType��dwId�ϲ�Ϊ��
//		��1�ֽ�Ϊ���ͣ���3�ֽ�Ϊ�������ݹ�����dwId�ĵ�3�ֽ�
DWORD MemTypeIdToId(BYTE bType, DWORD dwId)
{
	DWORD dwTmpId = ((bType<<24) | (dwId&0x00ffffff));
	return dwTmpId;
}

//�������ҵ��ڴ�����pbAllocTab�׸���̬����ռ������ƫ��
//������@pbAllocTab �ڴ�����
//		@wSect ��Ҫ���ҵĵ�wSect����Ч�ڴ���
//���أ����ص�wSect����̬�����ڴ��ƫ�ƺ�,��1���ڴ��ƫ��Ϊ0
WORD SchIndexInAllocTab(BYTE *pbAllocTab, WORD wSect)
{
	WORD wIdx = 0;	//�ڴ���������
	WORD wByte;
	BYTE bBit;

	for (wByte=0; wByte<ALLOC_TAB_LEN; wByte++)
	{
		if (pbAllocTab[wByte] == 0)
			continue;

		for (bBit=0; bBit<8; bBit++)
		{
			if (pbAllocTab[wByte] & (0x01<<bBit))
			{
				wIdx++;		//�ڴ�����������1
				if (wIdx == (wSect+1))
					return (wByte<<3)+bBit;	//�ҵ��ͷ����ڴ���ƫ��
			}
		}
	}

	return 0xffff;
}