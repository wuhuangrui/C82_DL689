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

