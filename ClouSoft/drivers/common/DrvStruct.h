/*********************************************************************************************************
 * Copyright (c) 2009,���ڿ�½���ӿƼ��ɷ����޹�˾
 * All rights reserved.
 *
 * �ļ����ƣ�Lcd.h
 * ժ    Ҫ�������õ�һЩ�ṹ�Ķ���
 *
 * ��ǰ�汾��1.0
 * ��    �ߣ����
 * ������ڣ�2009-07
 *
 * ȡ���汾��
 * ԭ �� �ߣ�
 * ������ڣ�
************************************************************************************************************/
#ifndef DRVSTRUCT_H
#define DRVSTRUCT_H
#include "apptypedef.h"

//ң�ű�λ�ṹ
typedef struct{
    WORD wNum;//��¼����
    BYTE bPtr;//��¼ָ��
	DWORD dwNow;//��ǰʱ��
    DWORD dwValue[256];//��λ��¼
    DWORD dwTicks[256];//��λʱ��
} TYMX;

//�����仯��¼
typedef struct{
    BYTE bPtr;//��¼ָ��
    DWORD dwValue[256]; //�仯��¼
} TKeyValue;


typedef struct {
	WORD wYxFlag;	//��Ч��־λ,ĳλ��1��ʾ��λ��Ч
	WORD wYxPolar;
}TYxPara;	//ң�Ų���

//ң�ؽṹ
typedef struct{
	WORD  wMode;		//ң�������ʽ:YK_MODE_LEVEL | YK_MODE_PULSE
	DWORD dwValidTurn;	//��Ч�ִ�
	WORD  wPulseWidth;	//������,��λ100����
	WORD  wSafeTime;	//�ϵ籣��ʱ��,��λ����,0��ʾ������
	DWORD dwFastDist;	//�������,��λ����,�ڴ�ʱ����,����ÿ������һ��
						//������ʱ��,����ÿdwSlowInterv������һ��
						//�������Ϊ0,�򶼰���ÿ������һ��
	DWORD dwSlowInterv; //�������,��λ����
}TYkPara;

#endif
