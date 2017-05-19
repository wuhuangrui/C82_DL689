/*********************************************************************************************************
 * Copyright (c) 2007,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：syssock.h
 * 摘    要：本文件主要实现本系统下socket通信的头文件包含及定义
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2007年4月
 *********************************************************************************************************/
#ifndef SYSSOCK_H
#define SYSSOCK_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

typedef unsigned int ULONG;

#define INVALID_SOCKET  -1
#define SOCKET_ERROR	-1

#define closesocket close

inline int SocketGetLastError(int s)
{
	return errno;
}

inline void SocketSetLastError(int s, int iError)
{
	errno = 0;
}

inline int ioctlsocket(int fd, int command, unsigned int* arg)
{
	return  ioctl(fd, command, arg);
}

#endif //SYSSOCK_H
