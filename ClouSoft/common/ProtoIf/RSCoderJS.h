/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：RSCoder.h
 * 摘    要：本文件实现了江苏负控RS(15, 11)纠错编码编码/解码
 *
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2007-01-16
 *
 * 取代版本：
 * 原 作 者：
 * 完成日期：
************************************************************************************************************/
#ifndef _RSCODERJS_H
#define _RSCODERJS_H
#include "RSCoder.h"

class CRSCoderJS : public CRSCoder
{
public:	
	CRSCoderJS(void);
	~CRSCoderJS(void);
	int  Pack(unsigned char* ucSrc, int iSrcLen, unsigned char* ucDst);
	int  UnPack(unsigned char* ucSrc, int iSrcLen, unsigned char* ucDst);
	
protected:
	void Encode(unsigned char* ucSrc, unsigned char* ucDst);
	void Decode(unsigned char* ucSrc, unsigned char* ucDst);
};
#endif
