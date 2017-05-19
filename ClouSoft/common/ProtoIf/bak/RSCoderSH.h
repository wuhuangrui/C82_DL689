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
#ifndef _RSCODERSH_H
#define _RSCODERSH_H
#include "RSCoder.h"

class CRSCoderSH : public CRSCoder
{
public:	
	CRSCoderSH(void);
	~CRSCoderSH(void);
	int  Pack(unsigned char* ucSrc, int iSrcLen, unsigned char* ucDst);
	int  UnPack(unsigned char* ucSrc, int iSrcLen, unsigned char* ucDst);
	
protected:	
	void Encode(unsigned char* ucSrc, unsigned char* ucDst);
	void Decode(unsigned char* ucSrc, unsigned char* ucDst);
};
#endif
