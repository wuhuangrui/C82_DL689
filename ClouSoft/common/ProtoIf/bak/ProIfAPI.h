/*********************************************************************************************************
 * Copyright (c) 2010,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProIfAPI.h
 * 摘    要：本文件实现了通信接口函数定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2010年1月
 * 备    注：
 *********************************************************************************************************/
#ifndef PROIFAPI_H
#define PROIFAPI_H

#include "apptypedef.h"
//#include <stdlib.h>
//#include <stdio.h>
//#include <sys/types.h>
//#include <dirent.h>
//#include <errno.h>
//#include <sys/stat.h>
//#include <fcntl.h>
//#include <string.h>
//#include <signal.h>
//#include <sys/wait.h>
//#include <unistd.h>

void InitProIf();
bool SetSockLed(bool fLight);
//int SafeRead(int fd, void *buf, int len);
//int FindPid(const char *prg_name);
//void CheckPppdProcess();

#endif //PROIFAPI_H