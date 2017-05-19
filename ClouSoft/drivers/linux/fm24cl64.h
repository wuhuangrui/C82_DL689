/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：driver.h
 * 摘    要：本文件主要实现eeprom(AT24C02)的驱动接口
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2009年4月
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

