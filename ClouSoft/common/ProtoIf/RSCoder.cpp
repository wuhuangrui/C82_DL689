/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�RSCoder.h
 * ժ    Ҫ�����ļ�ʵ�����Ϻ�����RS(15, 11)����������/����
 *
 * ��ǰ�汾��1.0.1
 * ��    �ߣ����
 * ������ڣ�2007-08-02
 *
 * ȡ���汾��
 * ԭ �� �ߣ�
 * ������ڣ�
 * ��    ע���Ϻ�����RS����ͽ��յĲ�ͬ�㣺
 *		     1������RS�������ֽڵ�ʱ���ǵ�λ��ǰ����λ�ں󣬶��Ϻ����Ǹ�λ��ǰ����λ�ں�
 *			 2�����յ�RS������ÿ15�ֽ�1�飬11����Ч�ֽڣ�4�������ֽڣ��Ϻ�����ÿ15���ֽ�1�飬9����Ч�ֽڣ�
 *				2��У���ֽڣ�ǰ9���ֽڵ�"��żУ��"�����1���ֽ�Ϊ0x68��0b01101000������������λ������ֽڶ�Ӧ��
 *				У��λΪ1����4�������ֽڡ�
************************************************************************************************************/
#include "stdafx.h"
#include <string.h>
#include "RSCoder.h"

unsigned char CRSCoder::m_bGF[] = {0, 1, 2, 4, 8, 3, 6, 12, 11, 5, 10, 7, 14, 15, 13, 9};
unsigned char CRSCoder::m_bID[] = {0, 1, 2, 5, 3, 9, 6, 11, 4, 15, 10, 8, 7, 14, 12, 13};

CRSCoder::CRSCoder()
{
}

CRSCoder::~CRSCoder()
{
}

void CRSCoder::Coder(unsigned char *ucBuf)
{
	unsigned char b[] = {1, 13, 12, 8, 7};
	unsigned char c[15];

	memcpy(c, ucBuf, 11);
	memset(&c[11], 0, 4);
	Mod(c, b);
	memcpy(&ucBuf[11], &c[11], 4);
}

void CRSCoder::Mod(unsigned char *ucBuf1, unsigned char *ucBuf2)
{
	int i = 0;
	int j = 0;
	int k = 0;
	int n = 0;

	for (i=0; i<11; i++)
	{
		if (ucBuf1[i] != 0)
		{
			if (m_bID[ucBuf1[i]] == m_bID[ucBuf2[0]])
				k = 0;
			else if (m_bID[ucBuf1[i]] > m_bID[ucBuf2[0]])
				k = 1 + m_bID[ucBuf1[i]] - m_bID[ucBuf2[0]];
			else
				k = 1 + RSCODERLC - (m_bID[ucBuf1[i]] - m_bID[ucBuf2[0]]);

			for (j=0; j<5; j++)
			{
				if (k==0 || m_bID[ucBuf2[j]]==0)
					n = m_bID[ucBuf2[j]];
				else
				{
					n = (k - 1 + m_bID[ucBuf2[j]] - 1) % RSCODERLC;
					if (n < 0) n = n + RSCODERLC;
					n++;
				}
				ucBuf1[i + j] = ucBuf1[i + j] ^ m_bGF[n];
			}
		}
	}
}

unsigned short CRSCoder::GetParity(unsigned char *ucSrc, int iLen)
{
	if (iLen > 16)
		return 0;
	unsigned short usPar = 0;
	for (int i=0; i<iLen; i++)
	{
		int iBit1Num = 0;
		for (int j=0; j<8; j++)
		{
			if (((*ucSrc)&(1<<j)) != 0)
				iBit1Num++;
		}
		ucSrc++;
		if (iBit1Num%2 != 0)
			usPar |= (1<<i);
	}
	return usPar;
}


void CRSCoder::XCoder(unsigned char *ucBuf)
{
	int k = 0;
	unsigned char i = 0;
	unsigned char j = 0;
	unsigned char s[4];
	unsigned char dz[3];
	unsigned char ew[2];
	unsigned char x[2];

	memset(s, 0, sizeof(s));
	memset(dz, 0, sizeof(dz));
	memset(ew, 0, sizeof(ew));
    memset(x, 0, sizeof(x));

	for (i=0; i<4; i++)
	{
		s[i] = 0;
		for (j=0; j<15; j++)
		{
			if (ucBuf[j] != 0)
			{
				k = m_bID[ucBuf[j]] - 1 + ((i + 1) * (RSCODERLC - 1 - j));
				k = k % RSCODERLC;
				if (k < 0) k = k + RSCODERLC;
				k++;
				s[i] = s[i] ^ m_bGF[k];
			}
		}
	}

	j = 0;

	for (i=0; i<4; i++)
	{
		if (s[i] != 0)
		{
			j = 1;
			break;
		}
	}

	dz[2] = Mul(s[0], s[2]) ^ Mul(s[1], s[1]);
	dz[1] = Mul(s[0], s[3]) ^ Mul(s[1], s[2]);
	dz[0] = Mul(s[2], s[2]) ^ Mul(s[1], s[3]);

	if (dz[2] == 0)
	{
		if (dz[1]!=0 || s[0]==0 || s[1]==0 || s[2]==0 || s[3]==0)
		{
		}
		else
		{
			x[0] = Div(s[1], s[0]);
			ew[0] = Mul(s[0], s[0]);
			ew[0] = Div(ew[0], s[1]);
			i = m_bID[x[0]] - 1;
			ucBuf[RSCODERLC - 1 - i] = ucBuf[RSCODERLC - 1 - i] ^ ew[0];
		}
	}
	else
	{
		if (dz[1]==0 || dz[0]==0)
		{
		}
		else
		{
			for (i=0; i<16; i++)
			{
				j = Mul(dz[2], Mul(m_bGF[i], m_bGF[i]));
				j = j ^ Mul(dz[1], m_bGF[i]);
				j = j ^ dz[0];
				if (j == 0)
				{
					x[0] = m_bGF[i + 1];
					break;
				}
			}
			x[1] = Div(dz[1], dz[2]) ^ x[0];
		}
		i = Mul(x[0], x[1]) ^ Mul(x[0], x[0]);
		ew[0] = Mul(s[0], x[1]) ^ s[1];
		ew[0] = Div(ew[0], i);
		i = Mul(x[0], x[1]) ^ Mul(x[1], x[1]);
		ew[1] = Mul(s[0], x[0]) ^ s[1];
		ew[1] = Div(ew[1], i);       

		if (m_bID[x[0]] < 1) i = 0;
		else i = m_bID[x[0]] - 1;
		ucBuf[RSCODERLC - 1 - i] = ucBuf[RSCODERLC - 1 - i] ^ ew[0];
		if (m_bID[x[1]] < 1) i = 0;
		else i = m_bID[x[1]] - 1;
		ucBuf[RSCODERLC - 1 - i] = ucBuf[RSCODERLC - 1 - i] ^ ew[1];
	}
}

unsigned char CRSCoder::Mul(unsigned char uc1, unsigned char uc2)
{
	int k = 0;

	if (m_bID[uc1]==0 || m_bID[uc2]==0)
		k = 0;
	else
	{
		k = (m_bID[uc1] - 1 + m_bID[uc2] - 1) % RSCODERLC;
		if (k < 0) k = k + RSCODERLC;
		k++;
	}
	return m_bGF[k];
}

unsigned char CRSCoder::Div(unsigned char uc1, unsigned char uc2)
{
	int k = 0;
	
	if (m_bID[uc1]==0 || m_bID[uc2]==0)
		k = 0;
	else
	{
		k = (m_bID[uc1] - 1 - (m_bID[uc2] - 1)) % RSCODERLC;
		if (k < 0) k = k + RSCODERLC;
		k++;
	}
	return m_bGF[k];
}

