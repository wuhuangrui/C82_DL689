/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：MeterConst.h
 * 摘    要：本文件主要实现抄表基本常量定义及配置常量定义
 * 当前版本：1.0
 * 作    者：李锦仙
 * 完成日期：2016年9月
 *********************************************************************************************************/
#ifndef METERCONST_H
#define METERCONST_H
#include "syscfg.h"

//抄表需求的优先级，共4档
#define MTR_PRIO_FIRST			1	//首要
#define MTR_PRIO_SECOND			2	//必要
#define MTR_PRIO_THREE			3	//需要
#define MTR_PRIO_FOUR			4	//可能

//抄读错误定义
#define RD_ERR_UNKNOWN		-3		//未知错识
#define RD_ERR_HALT			-2		//暂停抄读
#define RD_ERR_UNTIME		-1		//时间未到
#define RD_ERR_OK			0		//无错误，完全抄完
#define RD_ERR_UNFIN		1		//没抄完
#define RD_ERR_PWROFF		2		//停电
#define RD_ERR_485			3		//485抄表故障
#define RD_ERR_PARACHG		4		//电表参数变更
#define RD_ERR_INTVCHG		5		//抄表间隔变更
#define RD_ERR_DIR			6		//正在直抄
#define RD_ERR_STOPRD		7		//停止抄表
#define RD_ERR_RDFAIL		8		//抄读失败
#define RD_ERR_CHKTSK		9		//是否是全任务检索的返回值

const static int g_iInSnToPhyPort[] = {COMM_LINK, COMM_METER};	

//注意:本数组的定义就不要再改了,否则引起混乱,
// 	   我们就固定认为逻辑端口根据老的习惯,是从右->左的顺序开始编号1,2...
#define LOGIC_PORT_NUM	(sizeof(g_iInSnToPhyPort)/sizeof(int))

#define LOGIC_PORT_MIN	1									//最小的逻辑端口定义
#define LOGIC_PORT_MAX	(LOGIC_PORT_MIN+LOGIC_PORT_NUM-1)	//最大的逻辑端口定义

#endif //METERCONST_H
