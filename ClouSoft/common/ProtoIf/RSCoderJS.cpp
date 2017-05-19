/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�RSCoder.h
 * ժ    Ҫ�����ļ�ʵ���˽��ո���RS(15, 11)����������/����
 *
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2007-01-16
 *
 * ȡ���汾��
 * ԭ �� �ߣ�
 * ������ڣ�
************************************************************************************************************/
#include "stdafx.h"
#include "RSCoderJS.h"
#include <string.h>

CRSCoderJS::CRSCoderJS()
{
}

CRSCoderJS::~CRSCoderJS()
{
}

void CRSCoderJS::Encode(unsigned char* ucSrc, unsigned char* ucDst)
{
	int i = 0;
	unsigned char b[15];
	unsigned char c[22];
	unsigned char d[15];

	memcpy(b, ucSrc, 11);

	for (i=0; i<11; i++)
	{
		c[2*i] = b[i] % 16;
		c[2*i+1] = b[i] / 16;
	}

	memcpy(d, c, 11);

    Coder(d);
	b[11] = d[12] * 16 + d[11];
    b[12] = d[14] * 16 + d[13];
    
	memcpy(d, &c[11], 11);

	Coder(d);
    
    b[13] = d[12] * 16 + d[11];
    b[14] = d[14] * 16 + d[13];

	memcpy(ucDst, b, 15);
}

int CRSCoderJS::Pack(unsigned char *ucSrc, int iSrcLen, unsigned char* ucDst)
{	
	int iLen = 0;
	int iDstLen = 0;
	int i = 0;
	int iCodePos = 0;
	int iDataPos = 0;
	unsigned char ucBuf[2048];
	unsigned char ucBuf1[15];
	unsigned char ucBuf2[15];

	if (iSrcLen <= 0)
		return -1;

	memcpy(ucBuf, ucSrc, iSrcLen);

	iLen = iSrcLen;

    while(iLen%11 != 0)
	{
		ucBuf[iLen] = 0x77;
		iLen++;
	}

	iCodePos = iLen*15/11 - 15;
	iDataPos = iLen - 11;

	iDstLen = iLen;

	while(iDataPos >= 0)
	{
		memcpy(ucBuf1, &ucBuf[iDataPos], 11);
		Encode(ucBuf1, ucBuf2);
		memcpy(&ucDst[iCodePos], ucBuf2, 15);
		iCodePos = iCodePos - 15;
		iDataPos = iDataPos - 11;
		iDstLen = iDstLen + 4;
	}

	return iDstLen;
}

int CRSCoderJS::UnPack(unsigned char *ucSrc, int iSrcLen, unsigned char *ucDst)
{
	int iDstLen = 0;
	int iLen = 0;
	int i = 0;
	int iCodePos = 0;
	int iDataPos = 0;

	unsigned char ucBuf[2048];
    unsigned char ucBuf1[15];
	unsigned char ucBuf2[15];
    
	if (iSrcLen <= 0)
		return -1;

    memcpy(ucBuf, ucSrc, iSrcLen);
	iLen = iSrcLen;

	while (iLen%15 != 0)
	{
		ucBuf[iLen] = 0x77;
		iLen++;
	}

    iDstLen = iLen;

	while (iCodePos < iLen)
	{
		memcpy(ucBuf1, &ucBuf[iCodePos], 15);
		Decode(ucBuf1, ucBuf2);
		memcpy(&ucDst[iDataPos], ucBuf2, 11);
		iCodePos = iCodePos + 15;
		iDataPos = iDataPos + 11;
		iDstLen = iDstLen - 4;
	}
	return iDstLen;
}

void CRSCoderJS::Decode(unsigned char *ucSrc, unsigned char *ucDst)
{
	int i = 0;
	unsigned char b[15];
	unsigned char c[30];
	unsigned char d[15];
	unsigned char e[30];

	memcpy(b, ucSrc, 15);

	for (i=0; i<15; i++)
	{
		c[2*i] = b[i] % 16;
		c[2*i + 1] = b[i] / 16;
	}

	memcpy(d, c, 11);
	memcpy(&d[11], &c[22], 4);
  
    XCoder(d);

	memcpy(e, d, 11);
	memcpy(d, &c[11], 11);
	memcpy(&d[11], &c[26], 4);
    
    XCoder(d);

	memcpy(&e[11], d, 11);

	for (i=0; i<11; i++)
		ucDst[i] = e[2*i] + e[2*i+1]*16;
}
