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

//#define SYS_WIN32
#define SYS_VDK

#define USER_PATH  "/root/user/"
#define USER_DATA_PATH  "/root/user/data/"
#define USER_PARA_PATH  "/root/user/para/"
#define USER_CFG_PATH   USER_PARA_PATH
#define USER_TASKDATA_PATH	USER_DATA_PATH

#define  COMM_LOCAL    0
#define  COMM_METER    1
#define  COMM_GPRS     2
#define  COMM_LINK     3 	
#define  COMM_TEST     4

#endif //SYSCFG_H
