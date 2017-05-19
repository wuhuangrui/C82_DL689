 /*********************************************************************************************************
 * Copyright (c) 2005,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�DataLog.cpp
 * ժ    Ҫ�����ļ�ͨ����־�ķ�ʽʵ�ֵ������ݵĻָ�,��־������BANK�������л�,���ݱ��浽FM24CL64
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��6��
 *
 * ȡ���汾��
 * ԭ����  ��
 * ������ڣ�

 * ��ע:д����־���û����ݳ���Ŀǰ����趨Ϊ100
*********************************************************************************************************/

#ifndef DATALOG_H
#define DATALOG_H

#include "apptypedef.h"
#define LOG_VOLATILE	0
#define LOG_DEMAND		1
#define MAX_LOG_ID		1


class CDataLog{

public:
	CDataLog();
	virtual ~CDataLog();
	bool Init(WORD wID, WORD wDataSize);
	bool Recover(BYTE* pbBuf);
	bool WriteLog(BYTE* pbBuf);
	bool ClearLog();

private:
	WORD m_wID;
	WORD m_wDataSize;
	WORD m_wFileSize;
	BYTE m_bSN;
};


#endif //DATALOG_H