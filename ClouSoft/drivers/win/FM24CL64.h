/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�driver.h
 * ժ    Ҫ�����ļ���Ҫʵ������洢��FM24CL64������
 * ��ǰ�汾��1.0
 * ��    �ߣ��ſ�
 * ������ڣ�2007��1��
 *********************************************************************************************************/
#ifndef FM24CL64_H
#define FM24CL64_H
#include "apptypedef.h"

class CFM24CL64
{
public:
	CFM24CL64();
	virtual ~CFM24CL64();

	bool Open();
	bool Close();
	bool Read(WORD addr, BYTE* data, WORD len);
	bool Write(WORD addr, BYTE* data, WORD len);

private:
	int m_fd;	
	int ReadByte(WORD addr);
	int WriteByte(WORD mem_addr, BYTE data);
	int i2c_write_3b(BYTE buf[3]);
	int i2c_write_2b(BYTE buf[2]);
};
extern CFM24CL64 g_FM24CL64;

#endif

