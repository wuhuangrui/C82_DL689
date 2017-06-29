/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：syscfg.h
 * 摘    要：本文件用来定义本系统下的硬件分配
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：包括端口配置等
 *********************************************************************************************************/
#ifndef SYSCFG_H
#define SYSCFG_H

#include "apptypedef.h"

#define SYS_LINUX

#define USER_PATH  "/mnt/data/"
#define USER_DATA_PATH  "/mnt/data/data/"
#define USER_PARA_PATH  "/mnt/data/para/"
#define USER_CFG_PATH  "/mnt/data/cfg/"
#define USER_BAK_PATH  "/mnt/ext/bak/"
#define USER_TASKDATA_PATH	USER_DATA_PATH
   
#define USER_FTP_PATH	USER_PATH
#define USER_FTP_LOCAL_FILE	 "clou.tgz"


#define  COMM_TEST     6
#define  COMM_METER    2
#define  COMM_GPRS     3
#define  COMM_LINK     4
#define  COMM_PLC	   	 5
#define  COMM_LOCAL    1
#define  COMM_3GMODEM  7
#define  COMM_DEBUG	   8
#define  COMM_METER3	 8	//第3路485

#define  COMM_CCT_PLC COMM_PLC
#define  COMM_CCT_485 COMM_LINK

/*
#define  COMM_GPRS     0
#define  COMM_METER    1
#define  COMM_LOCAL    2
#define  COMM_TEST     3
#define  COMM_LINK     4
#define  COMM_PLC	   5
*/
#endif //SYSCFG_H
