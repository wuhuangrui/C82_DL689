/*********************************************************************************************************
 * Copyright (c) 2006,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：syscfg.h
 * 摘    要：本文件用来定义本系统下的各种配置
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2006年12月
 * 备    注：包括端口、路径等配置
 *********************************************************************************************************/
#ifndef SYSCFG_H
#define SYSCFG_H

//#define SYS_LINUX
#define SYS_WIN
#include "apptypedef.h"

//#define VER_STD     //标准版

#define USER_PATH  "d:\\fafiles-45\\"
#define USER_PARA_PATH  "d:\\fafiles-45\\para\\"
#define USER_DATA_PATH  "d:\\fafiles-45\\data\\"
#define USER_CFG_PATH   USER_PARA_PATH
#define USER_BAK_PATH	"d:\\fafiles\\ext\\"
#define USER_TASKDATA_PATH	USER_DATA_PATH

#define USER_FTP_PATH	USER_PATH
#define USER_FTP_LOCAL_FILE	 "clou.tgz"
#define SERVPORT 10000 
#define METERPORT 7900

#define  COMM_GPRS     100
#define  COMM_METER    4//4	第2路485
#define  COMM_PLC	   2//2//5
#define  COMM_LOCAL    100
#define  COMM_LINK     9//8//4	第1路485
#define  COMM_3GMODEM  100
#define  COMM_DEBUG	   100
#define  COMM_TEST	   100

#define COMM_CCT_PLC COMM_PLC
#define COMM_CCT_485 COMM_LINK

#endif //SYSCFG_H
