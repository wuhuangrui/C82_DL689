/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：driver.h
 * 摘    要：本文件主要实现铁电存储器FM24CL64的驱动
 * 当前版本：1.0
 * 作    者：张凯
 * 完成日期：2007年1月
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

