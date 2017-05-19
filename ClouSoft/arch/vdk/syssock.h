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