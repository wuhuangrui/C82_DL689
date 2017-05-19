/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：LoopBuf.h
 * 摘    要：本文件实现循环缓冲区类
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年12月
 * 备    注：
 *********************************************************************************************************/
#ifndef LOOPBUF_H
#define LOOPBUF_H
#include "apptypedef.h"

class CLoopBuf
{
public:
    CLoopBuf();
    virtual ~CLoopBuf();
    
    //变量定义
	
    //函数定义
	bool Init(WORD wBufSize);
	void PutToBuf(BYTE* pbBuf, WORD wLen);
	WORD RxFromBuf(BYTE* pbRxBuf, WORD wBufSize);
	void DeleteFromBuf(WORD wLen);
	WORD GetBufLen();
	void ClrBuf();

protected:
	//变量定义
	WORD  m_wBufSize;
	WORD  m_wRxHead;
	WORD  m_wRxTail;
	BYTE* m_pbLoopBuf;    //循环缓冲区
};
	


#endif //LOOPBUF_H
