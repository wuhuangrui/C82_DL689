/*********************************************************************************************************
 * Copyright (c) 2010,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ProIfAPI.h
 * ժ    Ҫ�����ļ�ʵ����ͨ�Žӿں�������
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2010��1��
 * ��    ע��
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