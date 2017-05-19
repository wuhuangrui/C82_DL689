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
