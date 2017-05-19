/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Lcd.h
 * 摘    要：驱动用到一些结构的定义
 *
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2009-07
 *
 * 取代版本：
 * 原 作 者：
 * 完成日期：
************************************************************************************************************/
#ifndef DRVSTRUCT_H
#define DRVSTRUCT_H
#include "apptypedef.h"

//遥信变位结构
typedef struct{
    WORD wNum;//记录数量
    BYTE bPtr;//记录指针
	DWORD dwNow;//当前时刻
    DWORD dwValue[256];//变位记录
    DWORD dwTicks[256];//变位时刻
} TYMX;

//按键变化记录
typedef struct{
    BYTE bPtr;//记录指针
    DWORD dwValue[256]; //变化记录
} TKeyValue;


typedef struct {
	WORD wYxFlag;	//有效标志位,某位置1表示该位有效
	WORD wYxPolar;
}TYxPara;	//遥信参数

//遥控结构
typedef struct{
	WORD  wMode;		//遥控输出方式:YK_MODE_LEVEL | YK_MODE_PULSE
	DWORD dwValidTurn;	//有效轮次
	WORD  wPulseWidth;	//脉冲宽度,单位100毫秒
	WORD  wSafeTime;	//上电保电时间,单位分钟,0表示不保电
	DWORD dwFastDist;	//快跳间隔,单位分钟,在此时间内,脉冲每分钟跳一次
						//超过此时间,脉冲每dwSlowInterv分钟跳一次
						//如果设置为0,则都按照每分钟跳一次
	DWORD dwSlowInterv; //慢跳间隔,单位分钟
}TYkPara;

#endif
