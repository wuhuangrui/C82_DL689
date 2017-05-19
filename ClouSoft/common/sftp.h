/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：SFTP.cpp
 * 摘    要：本文件主要实现了简单文件传输协议SFTP，本协议作为数据区嵌入到其它
 *			 通讯协议中，它本身不控制接收发送，只提供接口函数供其它通信协议在
 *			 帧处理时调用
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年8月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：
*********************************************************************************************************/

#ifndef SFTP_H
#define SFTP_H

#include "apptypedef.h"
#include "ComStruct.h"

#define DIR_MAX_SIZE    4096
#define PATHNAME_LEN    512


class CSftp
{
public:
	CSftp();
	virtual ~CSftp();
	void Clear();
	int ReadFirst(BYTE* pbRx, BYTE* pbTx);
	int ReadNext(BYTE* pbRx, BYTE* pbTx);
	int WriteFirst(BYTE* pbRx, BYTE* pbTx);
	int WriteNext(BYTE* pbRx, BYTE* pbTx);
	int TransferFinish(BYTE* pbRx, BYTE* pbTx);
	int TransferCancel(BYTE* pbRx, BYTE* pbTx);
	bool HandleFrm(BYTE* pbRx, BYTE* pbTx);
	bool 	m_IsFinish;
	WORD	m_wTxLen;
	WORD	m_wRxLen;
	TTime 	m_tmLastRecv;
    
protected:
	WORD	m_wBlockSize;
	WORD	m_wPermission;
	BYTE	m_bTransType;
	BYTE  	m_bDir[DIR_MAX_SIZE];
	char  	m_szPathName[PATHNAME_LEN+1];
	DWORD 	m_dwFileID;
	DWORD	m_dwFileSize;
	BYTE*   m_pbDataBuf;
	
private:
	bool IsFwUpdate();
};


#endif  //SFTP_H
