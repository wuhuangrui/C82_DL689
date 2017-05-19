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
#include "lwipopts.h"
#include "lwip/sockets.h"
#include "lwip/inet.h"
#include "lwip/arch.h"

typedef unsigned int ULONG;

#define  closesocket lwip_close 

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1

/*
#include <stdio.h>
#include "winsock2.h"
//#include <WinSock.h>

#define EWOULDBLOCK WSAEWOULDBLOCK

inline int SocketGetLastError(void)
{
	return WSAGetLastError();
}

inline void SocketSetLastError(int iError)
{
	return WSASetLastError(iError);
}
*/

u32_t GetPppLoacalAddr(int unit);

#endif //SYSSOCK_H