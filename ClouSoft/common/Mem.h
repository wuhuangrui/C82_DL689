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
#ifndef MEM_H
#define MEM_H

#include "apptypedef.h"
#include "MeterStruct.h"

//���䶯̬�ڴ�ռ�
bool AllocMem(BYTE*pbGlobal, TAllocTab* pAllocTab,WORD wTabNum, BYTE bType, DWORD dwId,WORD wDataLen);
//�ͷŶ�̬�ڴ�ռ�
bool FreeMem(BYTE*pbGlobal, TAllocTab* pAllocTab, WORD wTabNum, BYTE bType, DWORD dwId);
//��ȡ��̬�ڴ�
int ReadMem(TAllocTab* pAllocTab, WORD wTabNum, BYTE* pbMem, BYTE bType, DWORD dwId, BYTE* pbData);
//д��̬�ڴ�
int WriteMem(TAllocTab* pAllocTab, WORD wTabNum, BYTE* pbMem, BYTE bType, DWORD dwId, BYTE* pbData);

//������ȡ�ö�̬�ڴ����Ч���ݳ���
int GetMemLen(TAllocTab* pAllocTab, WORD wTabNum, BYTE bType, DWORD dwId);
//��������bType��dwId�ϲ�Ϊ��
//		��1�ֽ�Ϊ���ͣ���3�ֽ�Ϊ�������ݹ�����dwId�ĵ�3�ֽ�
DWORD MemTypeIdToId(BYTE bType, DWORD dwId);
WORD SchIndexInAllocTab(BYTE *pbAllocTab, WORD wSect);

#endif //MEM_H
