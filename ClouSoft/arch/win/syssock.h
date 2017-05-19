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
#include "winsock2.h"
//#include <WinSock.h>
#include "Ws2tcpip.h"

typedef u_long ULONG;

#define EWOULDBLOCK WSAEWOULDBLOCK

inline int SocketGetLastError(int s)
{
	return WSAGetLastError();
}

inline void SocketSetLastError(int s, int iError)
{
	return WSASetLastError(iError);
}



#endif //SYSSOCK_H