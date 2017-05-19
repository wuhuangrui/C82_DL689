/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�SFTP.cpp
 * ժ    Ҫ�����ļ���Ҫʵ���˼��ļ�����Э��SFTP����Э����Ϊ������Ƕ�뵽����
 *			 ͨѶЭ���У����������ƽ��շ��ͣ�ֻ�ṩ�ӿں���������ͨ��Э����
 *			 ֡����ʱ����
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��8��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�
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
