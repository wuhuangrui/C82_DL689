/*********************************************************************************************************
* Copyright (c) 2011,���ڿ�½���ӿƼ��ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�DrvHook.cpp
* ժ    Ҫ�����ļ���Ҫʵ�ֱ�ϵͳ���豸��������ķ�װ
* ��ǰ�汾��1.0
* ��    �ߣ����
* ������ڣ�2011��5��
*
* ȡ���汾��
* ԭ����  ��
* ������ڣ�
*********************************************************************************************************/
#ifndef DRVHOOK_H
#define DRVHOOK_H
#include "apptypedef.h"
#include <string>

void CommWriteHook(WORD wPort, DWORD dwLen);

void CommReadHook(WORD wPort, DWORD dwLen);

#endif   

