/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�driver.h
 * ժ    Ҫ�����ļ���Ҫʵ��eeprom(AT24C02)�������ӿ�
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2009��4��
 *********************************************************************************************************/
#ifndef FM24CL64_H
#define FM24CL64_H
#include "apptypedef.h"

class CFm24cl64
{
public:
	CFm24cl64();
	virtual ~CFm24cl64();

public:
	bool Open();
	bool Close();
	int Read(WORD addr, BYTE* data, WORD len);
	int Write(WORD addr, BYTE* data, WORD len);
	
private:
    int m_iHandle;	
};

extern CFm24cl64 g_FM24CL64;

#endif

