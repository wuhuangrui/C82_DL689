/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�syscfg.h
 * ժ    Ҫ�����ļ��������屾ϵͳ�µĸ�������
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��    ע�������˿ڡ�·��������
 *********************************************************************************************************/
#ifndef SYSCFG_H
#define SYSCFG_H

//#define SYS_LINUX
#define SYS_WIN
#include "apptypedef.h"

//#define VER_STD     //��׼��

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
#define  COMM_METER    4//4	��2·485
#define  COMM_PLC	   2//2//5
#define  COMM_LOCAL    100
#define  COMM_LINK     9//8//4	��1·485
#define  COMM_3GMODEM  100
#define  COMM_DEBUG	   100
#define  COMM_TEST	   100

#define COMM_CCT_PLC COMM_PLC
#define COMM_CCT_485 COMM_LINK

#endif //SYSCFG_H
