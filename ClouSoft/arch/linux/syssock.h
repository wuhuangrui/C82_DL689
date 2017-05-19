/*********************************************************************************************************
 * Copyright (c) 2007,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�syssock.h
 * ժ    Ҫ�����ļ���Ҫʵ�ֱ�ϵͳ��socketͨ�ŵ�ͷ�ļ�����������
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2007��4��
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
