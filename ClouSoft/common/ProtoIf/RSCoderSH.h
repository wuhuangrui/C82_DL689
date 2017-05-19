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
