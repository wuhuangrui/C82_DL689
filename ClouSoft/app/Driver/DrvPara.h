 /*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DrvPara.h
 * ժ    Ҫ��ʵ�ּ���������������Ҫ���û���������������Ĳ������ض���������ʵ��
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2009��7��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
*********************************************************************************************************/
#ifndef DRVPARA_H
#define DRVPARA_H
#include "DrvStruct.h"

#ifdef SYS_LINUX
	#ifndef BIG_LCD
		#define BIG_LCD				//��Һ��������ʾ�ο�������ʾ
	#endif
#endif

void InitLcd();
void InitYX();
void InitYK();
void YXLoadPara(TYxPara* pYxPara);
void TransferLidStatus();
void DcLoadAdjPara(BYTE* pbBuf);
bool SaveTrigerAdj(BYTE* pbBuf);
WORD GetTermCurrent(WORD wCtCn);
bool InitDrvPara();
extern WORD g_wLogFileAddr[512];
#endif

