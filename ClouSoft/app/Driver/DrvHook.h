/*********************************************************************************************************
* Copyright (c) 2011,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：DrvHook.cpp
* 摘    要：本文件主要实现本系统下设备驱动程序的封装
* 当前版本：1.0
* 作    者：杨进
* 完成日期：2011年5月
*
* 取代版本：
* 原作者  ：
* 完成日期：
*********************************************************************************************************/
#ifndef DRVHOOK_H
#define DRVHOOK_H
#include "apptypedef.h"
#include <string>

void CommWriteHook(WORD wPort, DWORD dwLen);

void CommReadHook(WORD wPort, DWORD dwLen);

#endif   

