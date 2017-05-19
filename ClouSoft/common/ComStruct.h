/*********************************************************************************************************
 * Copyright (c) 2006,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�ComStruct.h
 * ժ    Ҫ�����ļ���Ҫ��������һЩ���õ����ݽṹ
 * ��ǰ�汾��1.0
 * ��    �ߣ�᯼���
 * ������ڣ�2006��12��
 * ��ע��
 *********************************************************************************************************/
#ifndef COMSTRUCT_H
#define COMSTRUCT_H
#include "apptypedef.h"

typedef struct{
	WORD  nYear;
	BYTE  nMonth;
	BYTE  nDay;
	BYTE  nHour;
	BYTE  nMinute;
	BYTE  nSecond;
	BYTE  nWeek;
}TTime;

typedef struct{
    WORD nYear;
    BYTE nMonth;
    BYTE nDayOfWeek;
    BYTE nDay;
    BYTE nHour;
    BYTE nMinute;
    BYTE nSecond;
    WORD nMilliseconds;
}TMillTime;


#endif //COMSTRUCT_H

