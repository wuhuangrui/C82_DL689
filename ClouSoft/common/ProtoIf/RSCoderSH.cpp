/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：RSCoder.h
 * 摘    要：本文件实现了上海负控RS(15, 11)纠错编码编码/解码
 *
 * 当前版本：1.0.1
 * 作    者：杨进
 * 完成日期：2007-08-02
 *
 * 取代版本：
 * 原 作 者：
 * 完成日期：
 * 备    注：上海负控RS编码和江苏的不同点：
 *		     1、江苏RS编码拆分字节的时候是低位在前，高位在后，而上海的是高位在前，低位在后。
 *			 2、江苏的RS编码是每15字节1组，11个有效字节，4个纠错字节；上海的是每15个字节1组，9个有效字节，
 *				2个校验字节（前9个字节的"奇偶校验"，如第1个字节为0x68（0b01101000）有奇数个高位，这该字节对应的
 *				校验位为1），4个纠错字节。
************************************************************************************************************/
#include "stdafx.h"
#include <string.h>
#include "RSCoderSH.h"

CRSCoderSH::CRSCoderSH()
{
}

CRSCoderSH::~CRSCoderSH()
{
}

void CRSCoderSH::Encode(unsigned char* ucSrc, unsigned char* ucDst)
{
	int i = 0;
	unsigned char b[15];
	unsigned char c[22];
	unsigned char d[15];

	memset(b, 0, sizeof(b));
	memset(c, 0, sizeof(c));
	memset(d, 0, sizeof(d));
	memcpy(b, ucSrc, 11);

	for (i=0; i<11; i++)
	{
		c[2*i+1] = b[i]&0x0f;
		c[2*i] = (b[i]>>4)&0x0f;
	}

	memcpy(d, c, 11);

    Coder(d);
	b[11] = d[12] + (d[11]<<4);
    b[12] = d[14] + (d[13]<<4);
    
	memcpy(d, &c[11], 11);

	Coder(d);
    
    b[13] = d[12] + (d[11]<<4);
    b[14] = d[14] + (d[13]<<4);

	memcpy(ucDst, b, 15);
}

int CRSCoderSH::Pack(unsigned char *ucSrc, int iSrcLen, unsigned char* ucDst)
{	
	int iLen = 0;
	int iDstLen = 0;
	int iTmpLen = 0;
	int i = 0;
	int iCodePos = 0;
	int iDataPos = 0;
	unsigned short usPar = 0;
	unsigned char ucBuf[2048];
	unsigned char ucBuf1[15];
	unsigned char ucBuf2[15];
	unsigned char *p = ucBuf;

	memset(ucBuf, 0, sizeof(ucBuf));
	memset(ucBuf1, 0, sizeof(ucBuf1));
	memset(ucBuf2, 0, sizeof(ucBuf2));
	if (iSrcLen <= 0)
		return -1;

	iTmpLen = iSrcLen;

	while(iTmpLen > 0)
	{
		if (iTmpLen >= 9)
		{
			memcpy(p, ucSrc, 9);
		}
		else
		{
			memcpy(p, ucSrc, iTmpLen);
			memset(p+iTmpLen, 0x77, 9-iTmpLen);
		}
		usPar = GetParity(p, 9);
		p += 9;		
		*p++ = usPar&0xff;
		*p++ = (usPar>>8)&0xff;
		iTmpLen -= 9;
		ucSrc += 9;
		iLen += 11;
	}

	iCodePos = iLen*15/11 - 15;
	iDataPos = iLen - 11;

	iDstLen = iLen;

	while(iDataPos >= 0)
	{
		memcpy(ucBuf1, &ucBuf[iDataPos], 11); //9个字节有效数据
		Encode(ucBuf1, ucBuf2);
		memcpy(&ucDst[iCodePos], ucBuf2, 15);
		iCodePos = iCodePos - 15;
		iDataPos = iDataPos - 11;
		iDstLen = iDstLen + 4;
	}

	return iDstLen;
}

int CRSCoderSH::UnPack(unsigned char *ucSrc, int iSrcLen, unsigned char *ucDst)
{
	int iDstLen = 0;
	int iLen = 0;
	int i = 0;
	int iCodePos = 0;
	int iDataPos = 0;
	int iCycle = 0;
	unsigned short usPar = 0;
	unsigned char ucBuf[2048];
    unsigned char ucBuf1[15];
	unsigned char ucBuf2[15];

	memset(ucBuf, 0, sizeof(ucBuf));
	memset(ucBuf1, 0, sizeof(ucBuf1));
	memset(ucBuf2, 0, sizeof(ucBuf2));
    
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
		usPar = ucBuf2[9]+(ucBuf2[10]<<8);
		if (usPar != GetParity(ucBuf2, 9))
			break;
//		memcpy(&ucDst[iDataPos], ucBuf2, 11);
		memcpy(&ucDst[iDataPos], ucBuf2, 9); //9个字节有效数据
		iCodePos = iCodePos + 15;
//		iDataPos = iDataPos + 11;
		iDataPos = iDataPos + 9;
//		iDstLen = iDstLen - 4;
		iDstLen = iDstLen - 4 - 2;
		iCycle++;
	}
	if (iCycle <= 0)
		return 0;
	return iDstLen;
}

void CRSCoderSH::Decode(unsigned char *ucSrc, unsigned char *ucDst)
{
	int i = 0;
	unsigned char b[15];
	unsigned char c[30];
	unsigned char d[15];
	unsigned char e[30];
	memset(b, 0, sizeof(b));
	memset(c, 0, sizeof(c));
	memset(d, 0, sizeof(d));
	memset(e, 0, sizeof(e));

	memcpy(b, ucSrc, 15);

	for (i=0; i<15; i++)
	{
		c[2*i + 1] = b[i]&0x0f;
		c[2*i] = (b[i]>>4)&0x0f;
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
		ucDst[i] = (e[2*i]<<4) + e[2*i+1];
}
