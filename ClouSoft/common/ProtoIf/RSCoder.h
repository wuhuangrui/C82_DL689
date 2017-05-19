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
************************************************************************************************************/
#ifndef _RSCODER_H
#define _RSCODER_H

#define RSCODERLC 15

class CRSCoder
{
public:	
	CRSCoder(void);
	virtual ~CRSCoder(void);
	virtual int  Pack(unsigned char* ucSrc, int iSrcLen, unsigned char* ucDst) = 0;
	virtual int  UnPack(unsigned char* ucSrc, int iSrcLen, unsigned char* ucDst) = 0;
	
protected:
	static unsigned char m_bGF[16];
	static unsigned char m_bID[16];
	
	unsigned short GetParity(unsigned char *ucSrc, int iLen);
	void Coder(unsigned char* ucBuf);
	void Mod(unsigned char* ucBuf1, unsigned char* ucBuf2);
	virtual void Encode(unsigned char* ucSrc, unsigned char* ucDst) = 0;
	virtual void Decode(unsigned char* ucSrc, unsigned char* ucDst) = 0;
	void XCoder(unsigned char* ucBuf);
	unsigned char Mul(unsigned char uc1, unsigned char uc2);
	unsigned char Div(unsigned char uc1, unsigned char uc2);
};
#endif

