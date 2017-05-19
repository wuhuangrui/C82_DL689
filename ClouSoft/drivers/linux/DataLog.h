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

class CDataLog{

public:
	CDataLog();
	virtual ~CDataLog();
	bool Init(WORD wID, WORD wDataSize);
	bool Recover(BYTE* pbBuf);
	bool WriteLog(BYTE* pbBuf);
	bool ClearLog();
	bool ClearBlock(WORD wID);

private:
	WORD m_wID;
	WORD m_wDataSize;
	WORD m_wFileSize;
//  WORD *m_wLogFileAddr;
	BYTE m_bSN;
};

//������־�����  ��ƽ̨����Ҫ�޸�
extern void SetLogFileAddr(WORD *pwLogFileAddr, DWORD dwLen);

#endif //DATALOG_H