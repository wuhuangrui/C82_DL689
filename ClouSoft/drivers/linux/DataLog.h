 /*********************************************************************************************************
 * Copyright (c) 2005,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DataLog.cpp
 * 摘    要：本文件通过日志的方式实现掉电数据的恢复,日志在两个BANK间轮流切换,数据保存到FM24CL64
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年6月
 *
 * 取代版本：
 * 原作者  ：
 * 完成日期：

 * 备注:写入日志的用户数据长度目前最大设定为100
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

//铁电日志保存表  各平台按需要修改
extern void SetLogFileAddr(WORD *pwLogFileAddr, DWORD dwLen);

#endif //DATALOG_H