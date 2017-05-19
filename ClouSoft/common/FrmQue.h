/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FrmQue.h
 * 摘    要：本文件主要用来定义类CFrmQue(帧队列)
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年11月
 * 备    注：CFrmQue(帧队列类)的功能类似于CQueue(队列类),为异步的线程间的
 *           数据交换提供方便
 *           主要的区别在于:
 *			 CQueue为进入队列的每份数据提供的是一个指针
 *           CFrmQue为进入队列的每份数据提供一个固定大小的缓冲区(frame),
 *			 这也就是类命名为CFrmQue的来由,这个frame并不一定是指报文,而是
 *			 消息、数据等更广泛的内容
 *********************************************************************************************************/
#ifndef FRMQUE_H
#define FRMQUE_H
#include "apptypedef.h"
#include "sysarch.h"

class CFrmQue
{
public:
	CFrmQue();
	virtual ~CFrmQue();
	bool Init(WORD wMaxFrms, WORD wFrmSize);
    bool Append(BYTE* pbFrm, WORD wLen, DWORD dwMilliseconds);
    WORD Remove(BYTE* pbFrm, DWORD dwMilliseconds);
	void RemoveAll();
	int GetMsgNum() { return m_wFrmNum; };
	bool IsFull() { return m_wFrmNum==m_wMaxFrms; };
		
protected:
    TSem m_hsemMail;
    TSem m_hsemSpace;
    TSem m_hmtxQ;

	WORD m_wFrmSize;		//每帧最大能缓存的字节数
	WORD m_wMaxFrms;		//最大能缓存的帧数量
	WORD m_wFrmNum;			//实际缓存的帧数量
    WORD m_wFirst;
	WORD m_wLast;
	
    BYTE*  m_pbFrms;		//用来存放一个个的帧
    WORD*  m_pwFrmBytes;	//每个帧的实际字节数
};

#endif //FRMQUE_H

