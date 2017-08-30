/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：TermEvtTask.cpp
 * 摘    要：本文件主要实现DL/T 698.45 的事件
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2016年10月
 *********************************************************************************************************/
#include "stdafx.h"
#include "syscfg.h"
#include "FaCfg.h"
#include "Mem.h"
#include <fcntl.h>
#include "FrzTask.h"
#include "FaAPI.h"
#include "DbOIAPI.h"
#include "OIObjInfo.h"
#include "CctTaskMangerOob.h"
#include "TermEvtTask.h"
#include "DL69845.h"
#include "MeterAPI.h"
#include "MtrHook.h"
#include "MtrCtrl.h"
#include "StdReader.h"
#include "CctSchMtr.h"


//事件参数默认
//终端类型---不同类型的终端需要支持的事件不同
#if FA_TYPE == FA_TYPE_C82
const WORD g_wValidCfg[] = {
	MTR_VLOSS,					//失压					1
	MTR_VLESS,					//欠压					2
	MTR_VOVER,					//过压					3
	MTR_VBREAK,					//断相					4
	MTR_ILOSS,					//失流					5
	MTR_IOVER,					//过流					6
	MTR_IBREAK,					//断流					7
	MTR_PREVERSE,				//潮流反向				8
	MTR_POVER,					//过载					9
	MTR_PDMDOVER,				//正向有功需量超限		10
	MTR_RPDMDOVER,				//反向有功需量超限		11
	MTR_QDMDOVER,				//无功需量超限			12
	MTR_PFUNDER,				//功率因数超下限		13
	MTR_ALLVLOSS,				//全失压				14
	MTR_VDISORDER,				//电压逆相序			15
	MTR_IDISORDER,				//电流逆相序			16
	MTR_MTRCLEAR,				//电表清零				17
	MTR_DMDCLEAR,				//需量清零				18
	MTR_EVTCLEAR,				//事件清零				19
	MTR_VUNBALANCE,				//电压不平衡			20
	MTR_IUNBALANCE,				//电流不平衡			21
	MTR_ISUNBALANCE,			//电流严重不平衡		22
	MTR_CLKERR,					//时钟故障				23
	MTR_MTRCHIPERR,				//计量芯片故障			24

	TERM_INIT,					//终端初始化事件		1
	TERM_VERCHG,				//终端版本变更事件		2
	TERM_YXCHG,					//终端状态量变位事件	3
	TERM_POWOFF,				//终端停/上电事件		4
	TERM_MSGAUTH,				//终端消息认证错误事件	5
	TERM_DEVICEERR,				//设备故障记录			6			enum
	TERM_FLUXOVER,				//月通信流量超限事件	7	
	TERM_UNKNOWNMTR,			//发现未知电能表事件	8
	TERM_STEPAREA,				//跨台区电能表事件	9
	TERM_CLOCKPRG,				//终端对时事件			10
	TERM_TERMPRG,				//终端编程记录			12
	TERM_MTRCLKPRG,				//终端对电表校时记录	14			TSA
};
#elif FA_TYPE == FA_TYPE_K32
const WORD g_wValidCfg[] = {
	MTR_MTRCLEAR,				//电表清零				17
	MTR_EVTCLEAR,				//事件清零				19
	MTR_CLKERR,					//时钟故障				23
	MTR_MTRCHIPERR,				//计量芯片故障			24

	TERM_INIT,					//终端初始化事件		1
	TERM_VERCHG,				//终端版本变更事件		2
	TERM_YXCHG,					//终端状态量变位事件	3
	TERM_POWOFF,				//终端停/上电事件		4
	TERM_MSGAUTH,				//终端消息认证错误事件	5
	TERM_DEVICEERR,				//设备故障记录			6			enum
	TERM_FLUXOVER,				//月通信流量超限事件	7	
	TERM_CLOCKPRG,				//终端对时事件			8
	TERM_TERMPRG,				//终端编程记录			10
	TERM_MTRCLKPRG,				//终端对电表校时记录	12			TSA
};
#elif FA_TYPE == FA_TYPE_D82
const WORD g_wValidCfg[] = {
	MTR_VLOSS,					//失压					1
	MTR_VLESS,					//欠压					2
	MTR_VOVER,					//过压					3
	MTR_VBREAK,					//断相					4
	MTR_ILOSS,					//失流					5
	MTR_IOVER,					//过流					6
	MTR_IBREAK,					//断流					7
	MTR_PREVERSE,				//潮流反向				8
	MTR_POVER,					//过载					9
	MTR_PDMDOVER,				//正向有功需量超限		10
	MTR_RPDMDOVER,				//反向有功需量超限		11
	MTR_QDMDOVER,				//无功需量超限			12
	MTR_PFUNDER,				//功率因数超下限		13
	MTR_ALLVLOSS,				//全失压				14
	MTR_VDISORDER,				//电压逆相序			15
	MTR_IDISORDER,				//电流逆相序			16
	MTR_MTRCLEAR,				//电表清零				17
	MTR_DMDCLEAR,				//需量清零				18
	MTR_EVTCLEAR,				//事件清零				19
	MTR_VUNBALANCE,				//电压不平衡			20
	MTR_IUNBALANCE,				//电流不平衡			21
	MTR_ISUNBALANCE,			//电流严重不平衡		22
	MTR_CLKERR,					//时钟故障				23
	MTR_MTRCHIPERR,				//计量芯片故障			24

	TERM_INIT,					//终端初始化事件		1
	TERM_VERCHG,				//终端版本变更事件		2
	TERM_YXCHG,					//终端状态量变位事件	3
	TERM_POWOFF,				//终端停/上电事件		4
	TERM_MSGAUTH,				//终端消息认证错误事件	5
	TERM_DEVICEERR,				//设备故障记录			6			enum
	TERM_FLUXOVER,				//月通信流量超限事件	7	
	TERM_UNKNOWNMTR,			//发现未知电能表事件	8
	TERM_STEPAREA,				//跨台区电能表事件	9	
	TERM_CLOCKPRG,				//终端对时事件			10
	TERM_YKCTRLBREAK,			//遥控跳闸记录			11			OAD
	TERM_EPOVER,				//有功总电能量差动越限事件记录		12
	TERM_TERMPRG,				//终端编程记录			13
	TERM_CURCIRC,				//终端电流回路异常事件	11			enum
	TERM_MTRCLKPRG,				//终端对电表校时记录	12			TSA
	TERM_POWCTRLBREAK,			//功控跳闸记录			13			OI
	TERM_ELECTRLBREAK,			//电控跳闸记录			14			OI
	TERM_PURCHPARACHG,			//购电参数设置记录		15			OI 				
	TERM_ELECTRLALARM,			//电控告警事件记录		16			OI
};
#endif

//关联属性表参数默认
//0x3000 电能表失压事件
const BYTE g_bVLoCfg[] = {
	0x01,0x26,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,0x51,0x20,0x29,0x62,0x00,	
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3001 电能表欠压事件
const BYTE g_bVLeCfg[] = {
	0x01,0x26,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,0x51,0x20,0x29,0x62,0x00,	
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3002 电能表过压事件
const BYTE g_bVOCfg[] = {
	0x01,0x26,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,0x51,0x20,0x29,0x62,0x00,	
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3003 电能表断相事件
const BYTE g_bVBCfg[] = {
	0x01,0x26,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,0x51,0x20,0x29,0x62,0x00,	
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3004 电能表失流事件
const BYTE g_bILoCfg[] = {
	0x01,0x25,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3005 电能表过流事件
const BYTE g_bIOCfg[] = {
	0x01,0x25,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3006 电能表断流事件
const BYTE g_bIBCfg[] = {
	0x01,0x25,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x00,0x22,0x00,0x51,0x20,0x01,0x22,0x00,0x51,0x20,0x04,0x22,0x00,0x51,0x20,0x05,0x22,0x00,	
	0x51,0x20,0x0A,0x22,0x00,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3007 电能表过载事件
const BYTE g_bPRCfg[] = {
	0x01,0x20,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3008 电能表过载事件
const BYTE g_bPOCfg[] = {
	0x01,0x20,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3009 电能表正向有功需量超限事件
const BYTE g_bPDCfg[] = {
	0x01,0x00,
};
//0x300A 电能表反向有功需量超限事件
const BYTE g_bRPDCfg[] = {
	0x01,0x00,
};
//0x300B 电能表无功需量超限事件
const BYTE g_bQDCfg[] = {
	0x01,0x00,
};
//0x300C 电能表功率因数超下限事件
const BYTE g_bPfOCfg[] = {
	0x01,0x08,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
};
//0x300D 电能表全失压事件
const BYTE g_bAVLCfg[] = {
	0x01,0x01,
	0x51,0x20,0x01,0x22,0x00,
};
//0x300F 电能表电压逆相序事件
const BYTE g_bVDCfg[] = {
	0x01,0x20,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3010 电能表电流逆相序事件
const BYTE g_bIDCfg[] = {
	0x01,0x20,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x3013 电表清零事件
const BYTE g_bMCCfg[] = {
	0x01,0x18,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x50,0x22,0x01,0x51,0x00,0x60,0x22,0x01,0x51,0x00,0x70,0x22,0x01,0x51,0x00,0x80,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x51,0x22,0x01,0x51,0x00,0x61,0x22,0x01,0x51,0x00,0x71,0x22,0x01,0x51,0x00,0x81,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x52,0x22,0x01,0x51,0x00,0x62,0x22,0x01,0x51,0x00,0x72,0x22,0x01,0x51,0x00,0x82,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x53,0x22,0x01,0x51,0x00,0x63,0x22,0x01,0x51,0x00,0x73,0x22,0x01,0x51,0x00,0x83,0x22,0x01,
};

//0x3014 需量清零事件
const BYTE g_bDCCfg[] = {
	0x01,0x18,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x50,0x22,0x01,0x51,0x00,0x60,0x22,0x01,0x51,0x00,0x70,0x22,0x01,0x51,0x00,0x80,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x51,0x22,0x01,0x51,0x00,0x61,0x22,0x01,0x51,0x00,0x71,0x22,0x01,0x51,0x00,0x81,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x52,0x22,0x01,0x51,0x00,0x62,0x22,0x01,0x51,0x00,0x72,0x22,0x01,0x51,0x00,0x82,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x53,0x22,0x01,0x51,0x00,0x63,0x22,0x01,0x51,0x00,0x73,0x22,0x01,0x51,0x00,0x83,0x22,0x01,
};
//0x3015 电能表事件清零事件
const BYTE g_bECCfg[] = {
	0x01,0x00,
};
//0x301D 电能表电压不平衡事件
const BYTE g_bVNCfg[] = {
	0x01,0x21,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x26,0x62,0x00,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x301E 电能表电流不平衡事件
const BYTE g_bINCfg[] = {
	0x01,0x21,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x27,0x62,0x00,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x302D 电能表电流严重不平衡事件
const BYTE g_bISNCfg[] = {
	0x01,0x21,
	0x51,0x00,0x10,0x22,0x01,0x51,0x00,0x20,0x22,0x01,0x51,0x00,0x30,0x22,0x01,0x51,0x00,0x40,0x22,0x01,
	0x51,0x00,0x11,0x22,0x01,0x51,0x00,0x21,0x22,0x01,0x51,0x00,0x31,0x22,0x01,0x51,0x00,0x41,0x22,0x01,
	0x51,0x00,0x12,0x22,0x01,0x51,0x00,0x22,0x22,0x01,0x51,0x00,0x32,0x22,0x01,0x51,0x00,0x42,0x22,0x01,
	0x51,0x00,0x13,0x22,0x01,0x51,0x00,0x23,0x22,0x01,0x51,0x00,0x33,0x22,0x01,0x51,0x00,0x43,0x22,0x01,
	0x51,0x20,0x27,0x62,0x00,
	0x51,0x00,0x10,0x82,0x01,0x51,0x00,0x20,0x82,0x01,0x51,0x00,0x30,0x82,0x01,0x51,0x00,0x40,0x82,0x01,
	0x51,0x00,0x11,0x82,0x01,0x51,0x00,0x21,0x82,0x01,0x51,0x00,0x31,0x82,0x01,0x51,0x00,0x41,0x82,0x01,
	0x51,0x00,0x12,0x82,0x01,0x51,0x00,0x22,0x82,0x01,0x51,0x00,0x32,0x82,0x01,0x51,0x00,0x42,0x82,0x01,
	0x51,0x00,0x13,0x82,0x01,0x51,0x00,0x23,0x82,0x01,0x51,0x00,0x33,0x82,0x01,0x51,0x00,0x43,0x82,0x01,	
};
//0x302E 时钟故障
const BYTE g_bCECfg[] = {
	0x01,0x00,
};
//0x302F 计量芯片故障
const BYTE g_bMCEDCfg[] = {
	0x01,0x00,
};
//0x3100 终端初始化事件
const BYTE g_bITCfg[] = {
	0x01,0x00,
};
//0x3101 终端版本变更事件
const BYTE g_bVChDCfg[] = {
	0x01,0x02,
	0x51,0x43,0x00,0x23,0x02,
	0x51,0x43,0x00,0x43,0x02, 
};
//0x3104 终端状态量变位事件
const BYTE g_bYXCfg[] = {
	0x01,0x05,
	0x51,0x20,0x1E,0x42,0x00,
	0x51,0xF2,0x03,0x42,0x01, 
	0x51,0xF2,0x03,0x42,0x02, 
	0x51,0xF2,0x03,0x42,0x03, 
	0x51,0xF2,0x03,0x42,0x04, 
};
//0X3106终端停/上电事件 终端停/上电事件
const BYTE g_bPowCfg[] = {
	0x01,0x00,
};
//0x3109 终端消息认证错误事件
const BYTE g_bMsgCfg[] = {
	//0x01,0x01,
	//0x51,0x44,0x01,0x22,0x00,	//程序目前不支持此ID，暂时按空处理
	0x01,0x00,
};
//0x310A 设备故障记录
const BYTE g_bDevCfg[] = {
	0x01,0x00,
};
//0x3110 月通信流量超限事件
const BYTE g_bFluxCfg[] = {
	0x01,0x02,
	0x51,0x22,0x00,0x42,0x02,
	0x51,0x31,0x10,0x06,0x01, 
};
//0x3111 发现未知电能表事件
const BYTE g_bUKnMtrCfg[] = {
	0x01,0x00,
};
//0x3112 跨台区电能表事件
const BYTE g_bStepACfg[] = {
	0x01,0x00,
};
//0x3114 终端对时事件
const BYTE g_bClkCfg[] = {
	0x01,0x02,
	0x51,0x40,0x00,0x22,0x00,
	0x51,0x40,0x00,0x82,0x00,
};
//0x3115 遥控跳闸记录
const BYTE g_bYKCfg[] = {
	0x01,0x00,
};
//0x3116 有功总电能量差动越限事件记录
const BYTE g_bEpOCfg[] = {
	0x01,0x00,
};
//0x3118 终端编程记录
const BYTE g_bPrgCfg[] = {
	0x01,0x00,
};
//0x3119 终端电流回路异常事件
const BYTE g_bCurCCfg[] = {
	0x01,0x02,
	0x51,0x20,0x03,0x42,0x00,
	0x51,0x00,0x10,0x42,0x00,	
};
//0x311B 终端对电表校时记录
const BYTE g_bMtrClkPrgCfg[] = {
	0x01,0x00,
};
//0x3200 功控跳闸记录
const BYTE g_bPCtCfg[] = {
	0x01,0x01,
	0x51,0x23,0x01,0x23,0x00,
};
//0x3201 电控跳闸记录
const BYTE g_bECtBCfg[] = {
	0x01,0x01,
	0x51,0x23,0x01,0x49,0x00,
};
//0x3202 购电参数设置记录
const BYTE g_bPChCfg[] = {
	0x01,0x00,
	//0x51,0x81,0x0C,0x22,0x01,
};
//0x3203 电控告警事件记录
const BYTE g_bECtCfg[] = {
	0x01,0x00,
};


//终端所有事件，包括支持的和不支持的
//用于EVT_CLR_ID(事件清零)、和EVT_TRIG_IDID(触发一个事件)的分配。
const WORD g_wTermEvt[EVT_TOTAL_NUM] = {
	MTR_VLOSS,MTR_VLESS,MTR_VOVER,MTR_VBREAK,MTR_ILOSS,MTR_IOVER,MTR_IBREAK,MTR_PREVERSE,MTR_POVER,MTR_PDMDOVER,
	MTR_RPDMDOVER,MTR_QDMDOVER,MTR_PFUNDER,MTR_ALLVLOSS,MTR_SUPLYPOWDOWN,MTR_VDISORDER,MTR_IDISORDER,MTR_POWERDOWN,MTR_PROGRAM,MTR_MTRCLEAR,
	MTR_DMDCLEAR,MTR_EVTCLEAR,MTR_SETCLOCK,MTR_DAYSTAGE,MTR_TIMEZONE,MTR_WEEKREST,MTR_ACOUNTDAY,MTR_OPENCOVER,MTR_OPENTAILOVER,MTR_VUNBALANCE,
	MTR_IUNBALANCE,MTR_RELAYLAZHA,MTR_RELAYHEZHA,MTR_HOLIDAY,MTR_MIXDPEXP,MTR_MIXDQEXP,MTR_TARIFFPRICE,MTR_STAIRPRICE,MTR_UPDATEKEY,MTR_CARDABNAORMAL,
	MTR_PURCHASE,MTR_DECREASEPURSE,MTR_MAGNTEITCINT,MTR_SWITCHABNORMAL,MTR_POWERABNORMAL,MTR_ISUNBALANCE,MTR_CLKERR,MTR_MTRCHIPERR,MTR_MODULECHANGE,TERM_INIT,
	TERM_VERCHG,TERM_YXCHG,TERM_POWOFF,TERM_DIGITOVER,TERM_DIGITUNDER,TERM_MSGAUTH,TERM_DEVICEERR,TERM_FLUXOVER,TERM_UNKNOWNMTR,TERM_STEPAREA,
	TERM_CLOCKPRG,TERM_YKCTRLBREAK,TERM_EPOVER,TERM_OUTPUTSTACHG,TERM_TERMPRG,TERM_CURCIRC,TERM_ONLINESTACHG,TERM_MTRCLKPRG,TERM_POWCTRLBREAK,TERM_ELECTRLBREAK,
	TERM_PURCHPARACHG,TERM_ELECTRLALARM,
};
//描述：获取事件Sn(序号)，用于事件清零和触发一个事件的wPn的获取
//参数：@wOI 对象标识
//返回：正确则返回wOI对应的序号，否则返回-1
int GetEvtSn(WORD wOI)
{
	for(BYTE i=0; i<EVT_TOTAL_NUM; i++)
	{
		if (g_wTermEvt[i] == wOI) 
			return i;
	}
	return -1;	
}


//事件对象属性实例，Class7
const TEvtAttr g_tIC7EvtAttr = {
	IC7_ATTRTAB,
	IC7_CURRECNUM,
	IC7_MAXNUM,
	IC7_PARA,
	IC7_RECORDTAB,
	IC7_CURRECLIST,
	IC7_REPROTFLAG,
	IC7_VALIDFLAG,
};
//事件对象属性实例，Class24，总ABC或无功1234
const TEvtAttr g_tIC24EvtAttr4Item = {
	IC24_ATTRTAB,
	IC24_CURRECNUM,
	IC24_MAXNUM,
	IC24_PARA,
	IC24_RECORDTAB1,
	IC24_CURRECLIST,
	IC24_REPROTFLAG,
	IC24_VALIDFLAG,
};
//事件对象属性实例，Class24，只有ABC
const TEvtAttr g_tIC24EvtAttr3Item = {
	IC24_ATTRTAB,
	IC24_CURRECNUM,
	IC24_MAXNUM,
	IC24_PARA,
	IC24_RECORDTAB2,
	IC24_CURRECLIST,
	IC24_REPROTFLAG,
	IC24_VALIDFLAG,
};

//事件固定字段，属性特征配置为0值
//IC24 分项事件记录单元
BYTE g_bIC24EvtFixList[] = {
	DT_ARRAY,
	0x04,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
};
// 3301 8 标准事件记录单元
BYTE g_bStdEvtFixList[] = {
	DT_ARRAY,
	0x05,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	//事件记录序号  double-long-unsigned	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	//事件发生时间  date_time_s		
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	//事件结束时间  date_time_s		
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	//事件发生源    instance-specific		
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	//事件上报状态  array 通道上报状态		
};
// 3303 8 发现未知电能表事件单元
BYTE g_bUnKnMtrFixList[] = {
	DT_ARRAY,
	0x06,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x03, 0x02, 0x06,	//搜表结果      array 一个搜表结果
};

// 3304 8 跨台区电能表事件单元
BYTE g_bStepAreaFixList[] = {
	DT_ARRAY,
	0x06,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x04, 0x02, 0x06,	//跨台区搜表结果  array  一个跨台区结果	
};

// 3302 8 编程记录事件单元
BYTE g_bTermPrgEvtFixList[] = {
	DT_ARRAY,
	0x06,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x02, 0x02, 0x06,	//编程对象列表  array OAD	
};



// 3305 8 功控跳闸记录单元
BYTE g_bPowCtrlEvtFixList[] = {
	DT_ARRAY,
	0x0A,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x05, 0x02, 0x06,	//事件发生后2分钟功率  long64(单位：W，换算-1)
	DT_OAD, 0x33, 0x05, 0x02, 0x07,	//控制对象  OI
	DT_OAD, 0x33, 0x05, 0x02, 0x08,	//跳闸轮次  bit-string(SIZE(8))
	DT_OAD, 0x33, 0x05, 0x02, 0x09,	//功控定值  long64（单位：kW，换算-4）
	DT_OAD, 0x33, 0x05, 0x02, 0x0A,	//跳闸发生前总加有功功率    long64（单位：kW，换算-4）
};
// 3306 8 电控跳闸记录单元
BYTE g_bEleCtrlEvtFixList[] = {
	DT_ARRAY,
	0x09,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x06, 0x02, 0x06,	//控制对象  OI
	DT_OAD, 0x33, 0x06, 0x02, 0x07,	//跳闸轮次  bit-string(SIZE(8))
	DT_OAD, 0x33, 0x06, 0x02, 0x08,	//功控定值  long64（单位：kW，换算-4）
	DT_OAD, 0x33, 0x06, 0x02, 0x09,	//跳闸发生前总加有功功率 long64（单位：kW，换算-4）
};
// 3307 8 电控告警事件单元
BYTE g_bEleAlarmEvtFixList[] = {
	DT_ARRAY,
	0x07,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x07, 0x02, 0x06,	//控制对象  OI	
	DT_OAD, 0x33, 0x07, 0x02, 0x07,	//电控定值  long64（单位：kWh，换算-4）	
};
// 3308 8 电能表需量超限事件单元
BYTE g_bDmdEvtFixList[] = {
	DT_ARRAY,
	0x07,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x08, 0x02, 0x06,	//超限期间需量最大值  double-long-unsigned
	DT_OAD, 0x33, 0x08, 0x02, 0x07,	//超限期间需量最大值发生时间  date_time_s
};
// 3309 8 停/上电事件记录单元
BYTE g_bPowOffEvtFixList[] = {
	DT_ARRAY,
	0x06,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x09, 0x02, 0x06,	//属性标志	bit-string（SIZE(8)）
};
// 330A 8 遥控事件记录单元
BYTE g_bYKCtrlEvtFixList[] = {
	DT_ARRAY,
	0x06,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x0A, 0x02, 0x06,	//控后2分钟总加组功率 array long64
};
// 330B 8 有功总电能量差动越限事件记录单元
BYTE g_bEpOverlEvtFixList[] = {
	DT_ARRAY,
	0x09,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x0B, 0x02, 0x06,	//越限时对比总加组有功总电能量 long64（单位：kWh，换算：-4），
	DT_OAD, 0x33, 0x0B, 0x02, 0x07,	//越限时参照总加组有功总电能量 long64（单位：kWh，换算：-4），
	DT_OAD, 0x33, 0x0B, 0x02, 0x08,	//越限时差动越限相对偏差值 integer（单位：%，换算：0），
	DT_OAD, 0x33, 0x0B, 0x02, 0x09,	//越限时差动越限绝对偏差值 long64（单位：kWh，换算：-4）
};

// 330C 8 事件清零事件记录单元
BYTE g_bEvtClrEvtFixList[] = {
	DT_ARRAY,
	0x06,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x0C, 0x02, 0x06,	//事件清零列表  array OMD
};

// 330D 8 终端对电表校时记录单元
BYTE g_bMtrClkPrgFixList[] = {
	DT_ARRAY,
	0x07,
	DT_OAD, 0x20, 0x22, 0x02, 0x00,	
	DT_OAD, 0x20, 0x1E, 0x02, 0x00,	
	DT_OAD, 0x20, 0x20, 0x02, 0x00,	
	DT_OAD, 0x20, 0x24, 0x02, 0x00,	
	DT_OAD, 0x33, 0x00, 0x02, 0x00,	
	DT_OAD, 0x33, 0x0D, 0x02, 0x06,	//校时前时钟    date_time_s
	DT_OAD, 0x33, 0x0D, 0x02, 0x07,	//时钟误差      integer（单位：秒，无换算）
};
#define FIXLIST_ELEMENT_MAXNUM	0x0a//0x3301~0x3311记录单元对象中固定字段元素的最大个数


//IC7 当前记录数数据结构
const BYTE g_bIC7CurNum[3] = {
	DT_LONG_U,0x00,0x00,	
};
//IC24 当前记录数数据结构
const BYTE g_bIC24CurNum[14] = {
	DT_STRUCT,	
	0x04,
	DT_LONG_U,0x00,0x00,
	DT_LONG_U,0x00,0x00,
	DT_LONG_U,0x00,0x00,
	DT_LONG_U,0x00,0x00,
};

//事件发生源
BYTE g_bEvtSrcNullFmt[] = {DT_NULL};	
BYTE g_bEvtSrcOADFmt[] = {DT_OAD};	
BYTE g_bEvtSrcEnumFmt[] = {DT_ENUM};	
BYTE g_bEvtSrcOIFmt[] = {DT_OI};	
BYTE g_bEvtSrcTSAFmt[] = {DT_TSA};	

//IC7内表事件当前值记录表数据结构
//与g_bEvtSrcNullFmt匹配
const BYTE g_bSrcNullCurRecList[17] = {
	DT_ARRAY,	
	0x01,
	DT_STRUCT,	
	0x02,
	DT_NULL,	
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
};
//与g_bEvtSrcOADFmt匹配
const BYTE g_bSrcOADCurRecList[21] = {
	DT_ARRAY,	
	0x01,
	DT_STRUCT,	
	0x02,
	DT_OAD,	0x00,0x00,0x00,0x00,	
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
};
//与g_bEvtSrcEnumFmt匹配
const BYTE g_bSrcEnumCurRecList[18] = {
	DT_ARRAY,	
	0x01,
	DT_STRUCT,	
	0x02,
	DT_ENUM, 0x00,
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
};
//与g_bEvtSrcOIFmt匹配
const BYTE g_bSrcOICurRecList[19] = {
	DT_ARRAY,	
	0x01,
	DT_STRUCT,	
	0x02,
	DT_OI,0x00,0x00,	
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
};
//与g_bEvtSrcTSAFmt匹配
const BYTE g_bSrcTSACurRecList[35] = {
	DT_ARRAY,	
	0x01,
	DT_STRUCT,	
	0x02,
	0x00,//DT_TSA,0x00,	
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//事件发生源多余字节
};

//IC24当前值记录表数据结构
const BYTE g_bIC24CurRecList[50] = {
	DT_STRUCT,	
	0x04,
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
	DT_STRUCT,	
	0x02,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
};
//失压统计数据结构
const BYTE g_bAllVLossSta[28] = {
	DT_STRUCT,	
	0x04,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,
	DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
	DT_DATE_TIME_S,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	DT_DATE_TIME_S,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
};
//通道上报状态结构
const BYTE g_bCnRptState[CN_RPT_TOTAL_LEN] = {
	DT_STRUCT,	
	0x02,
	DT_OAD, 0x00, 0x00, 0x00, 0x00,	
	DT_UNSIGN, 0x00,		
};


#define TERM_EVT_NUM	64			//需要与g_EvtCtrl[]里的定义匹配
TEvtBase g_EvtBase[TERM_EVT_NUM];	//事件基类数据结构
TTermMem g_TermMem;					//事件临时空间
TVLoss g_VLoss;
TDmd g_PDmd;
TDmd g_RPDmd;
TDmd g_QDmd;
TUnKnMtr g_UnKnMtr;
TStepArea g_StepArea;
TEvtClr g_EvtClr;
TYXChgCtrl g_YXChgCtrl;
TPowOff g_PowOff;
TDeviceErr g_DeviceErr;
TYKCtrl g_YKCtrl;
TEpOver g_EpOver;
TTermPrg g_TermPrg;
TCurCirc g_CurCirc;
TMtrClkPrg g_MtrClkPrg;
TPowCtrl g_PowCtrl;
TEleCtrl g_EleCtrl;
TPurchParaChg g_PurchParaChg;
TEleAlram g_EleAlram;
TAdjTermTime g_AdjTermTime = { 0 };

TTermEvtCtrl g_EvtCtrl[] = 	//事件控制结构
{
	//wOI-bClass-分项数-延时时间- 固定字段------- 固定字段长度--------- 发生源格式--------  发生源格式长度---------------关联属性表默认配置及其长度--------基类- ----- 私有数据-----------------初始化------------判断-------------执行--
	{0x3000, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL, 				0,							g_bVLoCfg,	sizeof(g_bVLoCfg),	&g_EvtBase[0], 	&g_VLoss, 				InitVLoss, 		VLossJudge, 		DoVLoss},	//失压
	{0x3001, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL, 				0,							g_bVLeCfg,	sizeof(g_bVLeCfg),	&g_EvtBase[3], 	NULL, 					InitEvt, 		VLessJudge, 		DoEvt},		//欠压
	{0x3002, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bVOCfg,	sizeof(g_bVOCfg),	&g_EvtBase[6], 	NULL, 					InitEvt, 		VOverJudge, 		DoEvt},		//过压
	{0x3003, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bVBCfg, 	sizeof(g_bVBCfg),	&g_EvtBase[9], 	NULL, 					InitEvt, 		VBreakJudge, 		DoEvt},		//断相
	{0x3004, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bILoCfg, 	sizeof(g_bILoCfg),	&g_EvtBase[12],	NULL, 					InitEvt, 		ILossJudge, 		DoEvt},		//失流
	{0x3005, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bIOCfg, 	sizeof(g_bIOCfg),	&g_EvtBase[15],	NULL, 					InitEvt, 		IOverJudge, 		DoEvt},		//过流
	{0x3006, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bIBCfg, 	sizeof(g_bIBCfg),	&g_EvtBase[18],	NULL, 					InitEvt, 		IBreakJudge, 		DoEvt},		//断流
	{0x3007, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bPRCfg, 	sizeof(g_bPRCfg),	&g_EvtBase[21],	NULL, 					InitEvt, 		PReverseJudge, 		DoEvt},		//潮流反向
	{0x3008, 24, 3, 0, g_bIC24EvtFixList, 	sizeof(g_bIC24EvtFixList), 	NULL,				0,							g_bPOCfg, 	sizeof(g_bPOCfg),	&g_EvtBase[24],	NULL, 					InitEvt, 		POverJudge, 		DoEvt},		//过载
	{0x3009, 7, 1, 0, g_bDmdEvtFixList, 	sizeof(g_bDmdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bPDCfg, 	sizeof(g_bPDCfg),	&g_EvtBase[27],	&g_PDmd, 				InitDmd, 		PDmdOverJudge, 		DoDmd},		//正向有功需量超限
	{0x300A, 7, 1, 0, g_bDmdEvtFixList, 	sizeof(g_bDmdEvtFixList),	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bRPDCfg, 	sizeof(g_bRPDCfg),	&g_EvtBase[28],	&g_RPDmd,			 	InitDmd, 		RPDmdOverJudge, 	DoDmd},		//反向有功需量超限
	{0x300B, 24, 4, 0, g_bDmdEvtFixList, 	sizeof(g_bDmdEvtFixList), 	NULL, 				0,							g_bQDCfg, 	sizeof(g_bQDCfg),	&g_EvtBase[29],	&g_QDmd, 				InitDmd, 		QDmdOverJudge, 		DoDmd},		//无功需量超限
	{0x300C, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bPfOCfg, 	sizeof(g_bPfOCfg),	&g_EvtBase[33], NULL, 					InitEvt, 		PfUnderJudge, 		DoEvt},		//功率因数超下限
	{0x300D, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bAVLCfg, 	sizeof(g_bAVLCfg),	&g_EvtBase[34],	&g_PowerOffTmp.tAllVLoss, InitAVLoss, 	AVLossJudge, 		DoAVLoss},	//全失压
	{0x300F, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bVDCfg, 	sizeof(g_bVDCfg),	&g_EvtBase[35],	NULL, 					InitEvt, 		DisOrderJudge, 		DoEvt},		//电压逆相序
	{0x3010, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bIDCfg, 	sizeof(g_bIDCfg),	&g_EvtBase[36],	NULL, 					InitEvt, 		DisOrderJudge, 		DoEvt},		//电流逆相序
	{0x3013, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bMCCfg, 	sizeof(g_bMCCfg),	&g_EvtBase[37],	NULL, 					InitEvt, 		MtrClrJudge, 		DoNullEvt},	//电表清零
	{0x3014, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bDCCfg, 	sizeof(g_bDCCfg),	&g_EvtBase[38],	NULL, 					InitEvt, 		DmdClrJudge, 		DoNullEvt},	//需量清零
	{0x3015, 7, 1, 0, g_bEvtClrEvtFixList,	sizeof(g_bEvtClrEvtFixList), g_bEvtSrcNullFmt, sizeof(g_bEvtSrcNullFmt),	g_bECCfg, 	sizeof(g_bECCfg),	&g_EvtBase[39],	&g_EvtClr, 				InitEvtClr, 	EvtClrJudge, 		DoEvt},		//事件清零
	{0x301D, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bVNCfg, 	sizeof(g_bVNCfg),	&g_EvtBase[40],	NULL, 					InitEvt, 		VUnBalanceJudge, 	DoEvt},		//电压不平衡
	{0x301E, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bINCfg, 	sizeof(g_bINCfg),	&g_EvtBase[41],	NULL, 					InitEvt, 		IUnBalanceJudge, 	DoEvt},		//电流不平衡
	{0x302D, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bISNCfg, 	sizeof(g_bISNCfg),	&g_EvtBase[42],	NULL, 					InitEvt, 		IUnBalanceJudge, 	DoEvt},		//电流严重不平衡
	{0x302E, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bCECfg, 	sizeof(g_bCECfg),	&g_EvtBase[43],	NULL, 					InitEvt, 		TermErrJudge, 		DoEvt},		//时钟故障
	{0x302F, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bMCEDCfg, sizeof(g_bMCEDCfg),	&g_EvtBase[44],	NULL, 					InitEvt, 		TermErrJudge, 		DoEvt},		//计量芯片故障
	{0x3100, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt,	sizeof(g_bEvtSrcNullFmt),	g_bITCfg, 	sizeof(g_bITCfg),	&g_EvtBase[45],	NULL, 					InitEvt, 		TermInitJudge, 		DoNullEvt},	//终端初始化事件
	{0x3101, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bVChDCfg, sizeof(g_bVChDCfg),	&g_EvtBase[46],	NULL, 					InitEvt, 		TermVerChgJudge, 	DoEvt},		//终端版本变更事件
	{0x3104, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList),	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bYXCfg, 	sizeof(g_bYXCfg),	&g_EvtBase[47],	&g_YXChgCtrl,			InitYXEvtCtrl,  DoYXChgJudge, 		DoEvt},		//终端状态量变位事件
	{0x3106, 7, 1, 0, g_bPowOffEvtFixList,	sizeof(g_bPowOffEvtFixList),g_bEvtSrcEnumFmt, 	sizeof(g_bEvtSrcEnumFmt),	g_bPowCfg, 	sizeof(g_bPowCfg),	&g_EvtBase[48],	&g_PowOff, 				InitPowOff, 	PowOffJudge, 		DoEvt},		//终端停/上电事件
	{0x3109, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bMsgCfg, 	sizeof(g_bMsgCfg),	&g_EvtBase[49], NULL, 					InitEvt, 		GsgQAuthJudge, 		DoEvt},		//终端消息认证错误事件
	{0x310A, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcEnumFmt, 	sizeof(g_bEvtSrcEnumFmt),	g_bDevCfg, 	sizeof(g_bDevCfg),	&g_EvtBase[50],	&g_DeviceErr,			InitDeviceErr, 	DeviceErrJudge, 	DoEvt},		//设备故障记录
	{0x3110, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bFluxCfg, sizeof(g_bFluxCfg),	&g_EvtBase[51],	NULL, 					InitEvt, 		FluxOverJudge, 		DoEvt},		//月通信流量超限事件
	{0x3111, 7, 1, 0, g_bUnKnMtrFixList, 	sizeof(g_bUnKnMtrFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bUKnMtrCfg, sizeof(g_bUKnMtrCfg),	&g_EvtBase[52],	&g_UnKnMtr, 		InitUnKnMtr, 	UnKnMtrJudge, 		DoEvt},		//月通信流量超限事件
	{0x3112, 7, 1, 0, g_bStepAreaFixList, sizeof(g_bStepAreaFixList),	g_bEvtSrcNullFmt,	sizeof(g_bEvtSrcNullFmt),	g_bStepACfg, sizeof(g_bStepACfg), &g_EvtBase[53], &g_StepArea,			InitStepArea,	StepAreaJudge,		DoEvt}, 	//月通信流量超限事件
	{0x3114, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bClkCfg, 	sizeof(g_bClkCfg),	&g_EvtBase[54],	&g_AdjTermTime, 		InitEvt, 		TermClockPrgJudge, 	DoNullEvt},		//终端对时事件
	{0x3115, 7, 1, 0, g_bYKCtrlEvtFixList,	sizeof(g_bYKCtrlEvtFixList),g_bEvtSrcOADFmt, 	sizeof(g_bEvtSrcOADFmt),	g_bYKCfg, 	sizeof(g_bYKCfg),	&g_EvtBase[55],	&g_YKCtrl, 				InitYKCtrl, 	YKCtrlBreakJudge,	DoEvt},		//遥控跳闸记录
	{0x3116, 7, 1, 0, g_bEpOverlEvtFixList, 	sizeof(g_bEpOverlEvtFixList), 	g_bEvtSrcNullFmt, 	sizeof(g_bEvtSrcNullFmt),	g_bEpOCfg, 	sizeof(g_bEpOCfg),	&g_EvtBase[56],	&g_EpOver, 		InitEpOver, 		EpOverJudge, 	DoEvt},		//有功总电能量差动越限事件记录
	{0x3118, 7, 1, 0, g_bTermPrgEvtFixList,	sizeof(g_bTermPrgEvtFixList),g_bEvtSrcNullFmt,	sizeof(g_bEvtSrcNullFmt),	g_bPrgCfg, 	sizeof(g_bPrgCfg),	&g_EvtBase[57],	&g_TermPrg, 			InitTermPrg, 	TermPrgJudge, 		DoNullEvt},		//终端编程记录	
	{0x3119, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcEnumFmt, 	sizeof(g_bEvtSrcEnumFmt),	g_bCurCCfg, sizeof(g_bCurCCfg),	&g_EvtBase[58],	&g_CurCirc, 			InitCurCirc, 	CurCircJudge, 		DoEvt},		//终端电流回路异常事件
	{0x311B, 7, 1, 0, g_bMtrClkPrgFixList, 	sizeof(g_bMtrClkPrgFixList), g_bEvtSrcTSAFmt, 	sizeof(g_bEvtSrcTSAFmt),	g_bMtrClkPrgCfg, sizeof(g_bMtrClkPrgCfg),&g_EvtBase[59],&g_MtrClkPrg, 	InitMtrClkPrg, 	MtrClkPrgJudge, 	DoEvt},		//终端对电表校时记录
	{0x3200, 7, 1, 0, g_bPowCtrlEvtFixList,	sizeof(g_bPowCtrlEvtFixList),g_bEvtSrcOIFmt,	sizeof(g_bEvtSrcOIFmt),		g_bPCtCfg, 	sizeof(g_bPCtCfg),	&g_EvtBase[60],	&g_PowCtrl, 			InitPowCtrl, 	PowCtrlBreakJudge, 	DoEvt},		//功控跳闸记录
	{0x3201, 7, 1, 0, g_bEleCtrlEvtFixList,	sizeof(g_bEleCtrlEvtFixList),g_bEvtSrcOIFmt,	sizeof(g_bEvtSrcOIFmt),		g_bECtBCfg, sizeof(g_bECtBCfg),	&g_EvtBase[61],	&g_EleCtrl, 			InitEleCtrl, 	EleCtrlBreakJudge, 	DoEvt},		//电控跳闸记录
	{0x3202, 7, 1, 0, g_bStdEvtFixList, 	sizeof(g_bStdEvtFixList), 	g_bEvtSrcOIFmt,	 	sizeof(g_bEvtSrcOIFmt),		g_bPChCfg, 	sizeof(g_bPChCfg),	&g_EvtBase[62],	&g_PurchParaChg, 		InitPurchParaChg,PurChParaChgJudge, DoEvt},		//购电参数设置记录
	{0x3203, 7, 1, 0, g_bEleAlarmEvtFixList,sizeof(g_bEleAlarmEvtFixList),g_bEvtSrcOIFmt,	sizeof(g_bEvtSrcOIFmt),		g_bECtCfg, 	sizeof(g_bECtCfg),	&g_EvtBase[63],	&g_EleAlram, 			InitEleAlram, 	EleCtrlAlarmJudge, 	DoEvt},		//电控告警事件记录
};
#define EVT_NUM (sizeof(g_EvtCtrl)/sizeof(TTermEvtCtrl))


//描述：设置默认参数。包括关联属性表、最大记录数、有效标识
//参数：@ pEvtCtrl事件控制
//返回：无
void SetTermEvtOadDefCfg(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bBuf[EVT_ATTRTAB_LEN];
	BYTE bValidBuf[2] = {DT_BOOL, 0x01};
	BYTE bINValidBuf[2] = {DT_BOOL, 0x00};
	BYTE bINValidMaxNumBuf[3] = {DT_LONG_U, 0x00, 0x00};
	BYTE bRptBuf[2] = {DT_ENUM, 0x02};
	BYTE bPwrOffRptBuf[2] = {DT_ENUM, 0x03};
	WORD wOI;
	DWORD dwOAD;
	int iLen;
	const TEvtAttr* pEvtAttr; 
	const ToaMap* pOI;
	BYTE i;
	bool fIsValidEvt = false;

	if (pEvtCtrl == NULL)	//入参不合法
		return;
	
	wOI = pEvtCtrl->wOI;
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return;
	dwOAD = GetOAD(wOI, pEvtAttr->bRela, 0);
	pOI = GetOIMap(dwOAD);
	if (pOI == NULL)
		return;

	memset(bBuf, 0, sizeof(bBuf));	
	iLen = ReadItemEx(BN0, pOI->wPn, pOI->wID, bBuf);
	if (iLen>0 && IsAllAByte(bBuf, 0, sizeof(bBuf)))
	{
		memcpy(bBuf, pEvtCtrl->pbDefCfg, pEvtCtrl->wDefCfgLen);
		WriteItemEx(BN0, pOI->wPn, pOI->wID, bBuf);
		for (i = 0; i < sizeof(g_wValidCfg)/sizeof(WORD); i++)
		{
			if (wOI == g_wValidCfg[i])
			{
				fIsValidEvt = true;
				break;
			}
		}

		if (fIsValidEvt)
		{	
			OoWriteAttr(wOI, pEvtAttr->bValidFlg, bValidBuf);
		}
		else
		{	
			OoWriteAttr(wOI, pEvtAttr->bValidFlg, bINValidBuf);
			OoWriteAttr(wOI, pEvtAttr->bMaxRecNum, bINValidMaxNumBuf);
		}

#if FA_TYPE == FA_TYPE_D82
		if (wOI==TERM_POWOFF || wOI==TERM_DEVICEERR || wOI==TERM_CLOCKPRG || wOI==TERM_CURCIRC || wOI==TERM_MTRCLKPRG)
		{
			if (wOI == TERM_POWOFF)
				OoWriteAttr(wOI, pEvtAttr->bRepFlg, bPwrOffRptBuf);
			else
				OoWriteAttr(wOI, pEvtAttr->bRepFlg, bRptBuf);
		}
#endif	
		TrigerSaveBank(BN0, SECT3, -1);
	}
	
	return;
}

//描述：获取事件控制结构
//参数：@wOI 对象标识
//返回：正确则返回wOI对应的事件控制结构，否则返回NULL
TTermEvtCtrl* GetTermEvtCtrl(WORD wOI)
{
	for(BYTE i=0; i<EVT_NUM; i++)
	{
		if (g_EvtCtrl[i].wOI == wOI) 
			return &g_EvtCtrl[i];
	}
	return NULL;	
}
//描述：根据事件的类，取得事件属性的定义
//参数：@ pEvtCtrl事件控制
//返回:如果正确则返回事件属性的定义，否则返回NULL
const TEvtAttr* GetEvtAttr(TEvtCtrl* pEvtCtrl)
{
	if (pEvtCtrl == NULL)	//入参不合法
		return NULL;
	
	if (pEvtCtrl->bClass == IC7)
		return &g_tIC7EvtAttr;
	else if ((pEvtCtrl->bClass==IC24) && (pEvtCtrl->bItemNum==4))
		return &g_tIC24EvtAttr4Item;
	else if ((pEvtCtrl->bClass==IC24) && (pEvtCtrl->bItemNum==3))
		return &g_tIC24EvtAttr3Item;	
	else
		return NULL;
}

//描述：获取得到事件发生源实例数据
//参数：@ pEvtCtrl 事件控制
//		@pbSrcBuf 返回事件发生源实例数据
//		@bType = 0 只返回 长度；bType = 1 直接拷贝；bType=2当前值记录表拷贝，需要移位再取值，针对有TSA的情况
//返回：正确返回事件发生源长度，否则返回-1
int GetEvtSrcData(TEvtCtrl* pEvtCtrl, BYTE* pbSrcBuf, BYTE bType)
{
	int iRet = -1;
	int iTsaLen;
	BYTE bBuf[12] = {0};

	if ((pEvtCtrl==NULL) || (pbSrcBuf==NULL) || (bType > 3))		//入参不合法
		return -1;
	if (pEvtCtrl->pbSrcFmt == NULL)	//无源，且不需要记录事件发生源
		return 0;

	if (bType)	//需要拷贝的先获取类型
		*pbSrcBuf++ = pEvtCtrl->pbSrcFmt[0];
	else
		pbSrcBuf++;
	
	if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcNullFmt[0])
	{
		return 1;	//源为空，固定一个字节00	
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcOADFmt[0])
	{	
		iRet = 5;	
		if (bType)	//需要拷贝
		{
			if (pEvtCtrl->wOI == TERM_YKCTRLBREAK)
			{	
				TYKCtrl* pEvtPriv = (TYKCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				if (pEvtPriv->bEvtSrcOAD[0] != DT_OAD)	//私有数据赋值是否合法
					return -1;
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcOAD, iRet);
			}
			else 
				return -1;
		}	
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcEnumFmt[0])
	{	
		iRet = 2;	
		if (bType)	//需要拷贝
		{
			if (pEvtCtrl->wOI == TERM_DEVICEERR)
			{	
				TDeviceErr* pEvtPriv = (TDeviceErr* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				*pbSrcBuf =  pEvtPriv->bEvtSrcEnum;
			}
			else if (pEvtCtrl->wOI == TERM_CURCIRC)
			{
				TCurCirc* pEvtPriv = (TCurCirc* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				*pbSrcBuf =  pEvtPriv->bEvtSrcEnum;
			}
			else if (pEvtCtrl->wOI == TERM_POWOFF)
			{
				TPowOff* pEvtPriv = (TPowOff* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;

				DTRACE(DB_FAPROTO, ("GetEvtFieldParser: wOI=%u PowOnoff EvtSrcEnum = %d.\r\n", pEvtCtrl->wOI, pEvtPriv->bEvtSrcEnum));
				*pbSrcBuf =  pEvtPriv->bEvtSrcEnum;
			}
			else 
				return -1;
		}	
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcOIFmt[0])
	{		
		iRet = 3;	
		if (bType)	//需要拷贝
		{		
			if (pEvtCtrl->wOI == TERM_POWCTRLBREAK)
			{	
				TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				if (pEvtPriv->bEvtSrcOI[0] != DT_OI)	//私有数据赋值是否合法
					return -1;
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcOI, iRet);
			}
			else if (pEvtCtrl->wOI == TERM_ELECTRLBREAK)
			{	
				TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				if (pEvtPriv->bEvtSrcOI[0] != DT_OI)	//私有数据赋值是否合法
					return -1;
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcOI, iRet);
			}
			else if (pEvtCtrl->wOI == TERM_PURCHPARACHG)
			{	
				TPurchParaChg* pEvtPriv = (TPurchParaChg* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				if (pEvtPriv->bEvtSrcOI[0] != DT_OI)	//私有数据赋值是否合法
					return -1;
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcOI, iRet);
			}
			else if (pEvtCtrl->wOI == TERM_ELECTRLALARM)
			{	
				TEleAlram* pEvtPriv = (TEleAlram* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				if (pEvtPriv->bEvtSrcOI[0] != DT_OI)	//私有数据赋值是否合法
					return -1;
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcOI, iRet);
			}
			else 
				return -1;
			
		}
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcTSAFmt[0])
	{	
		iTsaLen = *pbSrcBuf;
		
		if (bType == 0)
			iRet = iTsaLen+2;
		else if (bType == 1)
		{
			if (pEvtCtrl->wOI == TERM_MTRCLKPRG)
			{	
				TMtrClkPrg* pEvtPriv = (TMtrClkPrg* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				iRet =  pEvtPriv->bEvtSrcTSA[1] + 2;	//补充类型及长度
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcTSA, iRet);	//只拷贝有效数据部分
			}
			else 
				return -1;
		}
		else  if (bType == 2)
		{
			if (pEvtCtrl->wOI == TERM_MTRCLKPRG)
			{	
				TMtrClkPrg* pEvtPriv = (TMtrClkPrg* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return -1;
				
				memcpy(bBuf, pbSrcBuf+1+iTsaLen, sizeof(bBuf));//备份后续数据
				iRet =  pEvtPriv->bEvtSrcTSA[1] + 2;	//补充类型及长度
				memcpy(pbSrcBuf-1, pEvtPriv->bEvtSrcTSA, iRet);	//只拷贝有效数据部分
				memcpy(pbSrcBuf-1+iRet, bBuf,sizeof(bBuf));	//拷贝后续数据				
			}
			else
				return -1;
		}
		else
			return -1;
	}	
	else
		return -1;
	
	return iRet;	
}
	
//描述：获取得到事件发生源对应的当前值记录表结构
//参数：@ pEvtCtrl 事件控制
//返回：正确返回事件当前值记录表结构的长度，否则返回-1
int  EvtSrctoCurRecList(TEvtCtrl* pEvtCtrl, BYTE* bCurRecList)
{
	if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcNullFmt[0])
	{	
		memcpy(bCurRecList, g_bSrcNullCurRecList, sizeof(g_bSrcNullCurRecList));
		return sizeof(g_bSrcNullCurRecList);
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcOADFmt[0])
	{	
		memcpy(bCurRecList, g_bSrcOADCurRecList, sizeof(g_bSrcOADCurRecList));
		return sizeof(g_bSrcOADCurRecList);
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcEnumFmt[0])
	{	
		memcpy(bCurRecList, g_bSrcEnumCurRecList, sizeof(g_bSrcEnumCurRecList));
		return sizeof(g_bSrcEnumCurRecList);
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcOIFmt[0])
	{	
		memcpy(bCurRecList, g_bSrcOICurRecList, sizeof(g_bSrcOICurRecList));
		return sizeof(g_bSrcOICurRecList);
	}
	else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcTSAFmt[0])
	{	
		memcpy(bCurRecList, g_bSrcTSACurRecList, sizeof(g_bSrcTSACurRecList));
		return sizeof(g_bSrcTSACurRecList);
	}
	else
	{	
		bCurRecList = NULL;
		return -1;
	}
}

//描述：根据OAD获取对象/属性/元素
//参数：@dwOAD 数据标识
//		@pwOI 返回的对象标识
//		@pbAttr 返回的属性(已去掉了属性特征)
//		@pbIndex 返回的属性内元素索引
//返回：无返回
void GetOIAttrIndex(DWORD dwOAD, WORD* pwOI, BYTE* pbAttr, BYTE* pbIndex)
{
	if (pwOI != NULL) *pwOI = (WORD)(dwOAD>>16);
	if (pbAttr != NULL) *pbAttr = (BYTE)((dwOAD&0x00001f00)>>8);
	if (pbIndex != NULL) *pbIndex = (BYTE)(dwOAD&0x000000ff);	
}

//描述：获取事件固定字段/数据字段
//参数：@pEvtCtrl 事件控制
//		@pFixFields 返回的固定字段
//		@pDataFields 返回的数据字段
//		@pbAtrrTabBuf 关联属性表缓冲区
//		@wBufSize pbDataCfg缓冲区的大小
//返回：正确获取到固定字段/数据字段返回true，否则返回false
bool GetEvtFieldParser(struct TEvtCtrl* pEvtCtrl, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAtrrTabBuf, WORD wBufSize)
{
	BYTE bFixListFmt[] = {DT_ARRAY, FIXLIST_ELEMENT_MAXNUM, DT_OAD};
	BYTE* pbFmt;
	WORD wOI, wFmtLen = 0;
	int iLen;
	const TEvtAttr* pEvtAttr; 
	
	wOI = pEvtCtrl->wOI;
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return false;
	
	// 固定字段
	if (pFixFields != NULL)
	{
		pFixFields->pbCfg = pEvtCtrl->pbFixField;
		pFixFields->wCfgLen = pEvtCtrl->wFixFieldLen;
		if (OoParseField(pFixFields, bFixListFmt, sizeof(bFixListFmt), true) == false)	//必有固定字段
		{	
			//DTRACE(DB_INMTR, ("GetEvtFieldParser: wOI=%u OoParseField() pFixFields fail.\r\n", wOI));
			return false;
		}	
	}

	//数据字段，关联属性表可以为NULL
	if (pDataFields != NULL)
	{
		if (wBufSize < EVT_ATTRTAB_LEN)
			return false;
		memset(pbAtrrTabBuf, 0x00, wBufSize);
		pDataFields->pbCfg = pbAtrrTabBuf;
		iLen= OoReadAttr(wOI, pEvtAttr->bRela, pDataFields->pbCfg, &pbFmt, &wFmtLen);		
		if (iLen > 0)
		{
			pDataFields->wCfgLen = iLen;
			if (OoParseField(pDataFields, pbFmt, wFmtLen, true) == false)
			{	
				//DTRACE(DB_INMTR, ("GetEvtFieldParser: wOI=%u OoParseField() pDataFields fail.\r\n", wOI));
				return false;
			}
		}
		else
		{	
			memset(pbAtrrTabBuf, 0x00, wBufSize);
			pDataFields->pbCfg = 0;
			pDataFields->wNum= 0;
		}
	}
	
	return true;
}

struct TEvtCtrl* GetEvtCtrl(DWORD dwOAD)
{
	WORD i, wOI, wNum;
	wOI = dwOAD>>16;
	wNum = sizeof(g_EvtCtrl)/sizeof(TEvtCtrl);
	struct TEvtCtrl* pEvtCtrl = NULL;

	for (i=0; i<wNum; i++)
	{
		if (wOI == g_EvtCtrl[i].wOI)
		{
			pEvtCtrl = &g_EvtCtrl[i];
			break;
		}
	}

	if (i==wNum && wOI>0x3008 && wOI<=0x3030)
	{
		pEvtCtrl = &g_EvtCtrl[12];
	}

	return pEvtCtrl;
}

//描述：获取事件ROAD的主OAD的配置长度
//参数：@dwOAD 事件OAD
//返回：存在返回实际长度，否则返回0
DWORD GetEvtMainOadDataLen(DWORD dwOAD)
{
	BYTE bBuf[EVT_ATTRTAB_LEN];
	WORD wOI;
	BYTE bAttr, bIndex;
	DWORD dwDataLen = 0;
	const TEvtAttr* pEvtAttr; 
	TFieldParser tFixFields;
	TFieldParser tDataFields;

	struct TEvtCtrl* pEvtCtrl = GetEvtCtrl(dwOAD);
	if (pEvtCtrl == NULL)
		return 0;

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));

	//获取事件属性
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return 0;

	GetOIAttrIndex(dwOAD, &wOI, &bAttr, &bIndex);
	if ((pEvtCtrl->bClass==7 && bAttr!=ATTR2) || (pEvtCtrl->bClass==24 && bAttr!=ATTR6 && bAttr!=ATTR7 && bAttr!=ATTR8 && bAttr!=ATTR9))
		return 0;

	//获取固定字段和数据字段
	if (GetEvtFieldParser(pEvtCtrl, &tFixFields, &tDataFields, bBuf, sizeof(bBuf)) == false)
	{	
		return 0;
	}

	//if (pEvtCtrl->pbSrcFmt != NULL)
	//{
	//	if (tFixFields.wTotalLen >= EVT_SRC_LEN)
	//	{
	//		dwDataLen = tFixFields.wTotalLen - EVT_SRC_LEN;
	//		switch (pEvtCtrl->pbSrcFmt[0])
	//		{
	//		case DT_NULL:
	//			dwDataLen += 3;
	//			break;
	//		case DT_ENUM:
	//			dwDataLen += 4;
	//			break;
	//		case DT_OI:
	//			dwDataLen += 5;
	//			break;
	//		case DT_OAD:
	//			dwDataLen += 7;
	//			break;
	//		case DT_TSA:
	//			dwDataLen += (EVT_SRC_LEN+2);
	//			break;
	//		}
	//	}
	//	else
	//		dwDataLen = tFixFields.wTotalLen;
	//}

	dwDataLen = 2+5; //事件ROAD的次数长度 +2字节记录数据实际长度
	return dwDataLen;
}

//描述：是否为事件发生前/事件结束前的OAD
//参数：@dwOAD数据标识
//返回：true/false
//注：此类OAD需要分配全局空间
bool IsEvtBeforeOAD(DWORD dwOAD)
{
	BYTE bFeat;
	bFeat = (BYTE)((dwOAD&~OAD_FEAT_MASK) >> OAD_FEAT_BIT_OFFSET);

	if ((bFeat!=EVT_S_BF_HP) && (bFeat!=EVT_S_AFT_HP) && (bFeat!=EVT_S_BF_END)  &&  (bFeat!=EVT_S_AFT_END))
		bFeat = EVT_S_AFT_HP;

	if ((bFeat==EVT_S_BF_HP) ||(bFeat==EVT_S_BF_END))
		return true;
	else
		return false;
}

//描述：本OAD当前是否需要采集数据
//参数：@dwOAD数据标识
//		@bState当前事件状态
//返回：true/false
bool IsOADNeedAcqData(DWORD dwOAD, BYTE bState)
{
	BYTE bFeat;
	bFeat = (BYTE)((dwOAD&~OAD_FEAT_MASK) >> OAD_FEAT_BIT_OFFSET);

	if ((bFeat!=EVT_S_BF_HP) && (bFeat!=EVT_S_AFT_HP) && (bFeat!=EVT_S_BF_END)  &&  (bFeat!=EVT_S_AFT_END))
		bFeat = EVT_S_AFT_HP;

	if ((bState==bFeat) ||((bState!=EVT_S_BF_HP)&&(bFeat==EVT_S_BF_END)))	//状态一致，或者事件发生后读结束前EVT_S_BF_END		
		return true;
	else
		return false;	
}

//描述：本OAD当前是否需要保存到任务库的事件记录表
//参数：@dwOAD数据标识
//		@bState当前事件状态
//返回：true/false
bool IsOADNeedSaveData(DWORD dwOAD, BYTE bState)
{
	BYTE bFeat;
	bFeat = (BYTE)((dwOAD&~OAD_FEAT_MASK) >> OAD_FEAT_BIT_OFFSET);

	if ((bFeat!=EVT_S_BF_HP) && (bFeat!=EVT_S_AFT_HP) && (bFeat!=EVT_S_BF_END)  &&  (bFeat!=EVT_S_AFT_END))
		bFeat = EVT_S_AFT_HP;

	//事件发生后，存储发生前和发生后的数据
	//事件结束后，存储结束前和结束后的数据
	if (((bState==EVT_S_AFT_HP) && ((bFeat==EVT_S_BF_HP)||(bFeat==EVT_S_AFT_HP)))
		|| ((bState==EVT_S_AFT_END) && ((bFeat==EVT_S_BF_END)||(bFeat==EVT_S_AFT_END))))		
		return true;
	else
		return false;	
}

//描述：初始化临时空间。关联属性表有事件发生前/事件结束前的OAD需要申请临时全局变量空间
//参数：@pEvtCtrl 事件控制
//		@pDataFields 数据字段
//返回：处理正确返回true，否则返回false
bool InitTmpMem(struct TEvtCtrl* pEvtCtrl, TFieldParser* pDataFields)
{
	BYTE bAttrTab, bOadNum, bIndex, bType, bItem, bOadBuf[10], bBuf[SYSDB_ITEM_MAXSIZE];
	WORD wItemOffset, wItemLen, wDataLen, wTotalLen;	
	DWORD dwOAD, dwROAD;
	int iLen;
	const TEvtAttr* pEvtAttr; 

	//无数据字段，不需要分配临时空间
	if ((pDataFields==NULL) || (pDataFields->wCfgLen==0) || (pDataFields->wNum==0))
		return true;

	//获取事件类属性
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return false;
	bAttrTab = pEvtAttr->bRecTabStart;

	//计算临时空间
	bOadNum = 0;	//临时空间OAD个数
	wDataLen = 0;	//临时空间数据长度
	wTotalLen = 0;	//临时空间所需总长度
	memset(bBuf, 0, sizeof(bBuf));
	for (bIndex=0; bIndex<pDataFields->wNum; bIndex++)
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(pDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			//DTRACE(DB_INMTR, ("InitTmpMem: ReadParserField() fail.\r\n"));
			return false;
		}		
		dwOAD = OoOadToDWord(bOadBuf+1);
		if (IsEvtBeforeOAD(dwOAD))
		{
			OoDWordToOad(dwOAD, &bBuf[1+bOadNum*5]);
			bBuf[5+bOadNum*5] = (BYTE)wItemLen;	//目前数据项最大长度小于255
			wDataLen += wItemLen;
			bOadNum++;	//个数自加
			bBuf[0] = bOadNum;
		}
	}
	if (wDataLen)
		wTotalLen = wDataLen+1+bOadNum*5;//个数+个数*（OAD+LEN）+数据
	else
		return true;	//不需要分配临时空间

	//分配临时空间
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		dwROAD = GetOAD(pEvtCtrl->wOI, bAttrTab, 0);
		
		//初始化时需要保证此值正确
		if (pEvtCtrl->pEvtBase[bItem].bMemType != MEM_TYPE_NONE)
			return false;

		if (wTotalLen > 64)
		{
			pEvtCtrl->pEvtBase[bItem].bMemType = MEM_TYPE_TERM_EVTREC;
			if (EvtAllocRecMem(dwROAD, &g_TermMem, pDataFields->wTotalLen) == false)
				return false;
		}
		else if (wTotalLen != 0)
		{
			pEvtCtrl->pEvtBase[bItem].bMemType = MEM_TYPE_TERM_EVTITEM;
			if (EvtAllocItemMem(dwROAD, &g_TermMem, wTotalLen))
			{
				if (EvtWriteItemMem(dwROAD, &g_TermMem, bBuf) <= 0)
					return false;
			}
			else
				return false;
		}
		else
			return false;

		//下一个表
		bAttrTab++;
	}
	return true;
}

//描述：通用事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitEvt(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bAttrTab, bItem, bBuf[EVT_ATTRTAB_LEN];
	WORD wOI, wMaxNum; 
	DWORD  dwROAD;
	int iLen;
	const TEvtAttr* pEvtAttr; 
	TFieldParser tFixFields;
	TFieldParser tDataFields;
	const ToaMap* pOaMap = NULL;
	
	//DTRACE(DB_INMTR, ("InitEvt: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));

	//全局变量初始化
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)	
		memset((BYTE*)&pEvtCtrl->pEvtBase[bItem], 0x00, sizeof(TEvtBase));
	pEvtCtrl->dwLastClick = 0;
	pEvtCtrl->dwNewClick = 0;

	//获取事件属性
	wOI = pEvtCtrl->wOI;
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return false;
	bAttrTab = pEvtAttr->bRecTabStart;

	SetTermEvtOadDefCfg(pEvtCtrl);

	//获取最大记录数
	iLen = OoReadAttr(wOI, pEvtAttr->bMaxRecNum, bBuf, NULL, NULL);
	if (iLen <= 0)
	{	
		DTRACE(DB_INMTR, ("InitEvt: wOI=%u Init fail because Read wMaxNum fail.\r\n", wOI));
		return false;
	}
	wMaxNum = OoLongUnsignedToWord(bBuf+1);	
	DTRACE(DB_INMTR, ("InitEvt: wOI=%u & wMaxNum=%u.\r\n", wOI, wMaxNum));
	if (wMaxNum == 0)	//最大记录数为0，初始化失败
		return false;

	//获取固定字段和数据字段
	if (GetEvtFieldParser(pEvtCtrl, &tFixFields, &tDataFields, bBuf, sizeof(bBuf)) == false)
	{	
		DTRACE(DB_INMTR, ("InitEvt: wOI=%u GetEvtFieldParser() fail.\r\n", wOI));
		return false;
	}

	//给数据字段申请整笔/临时空间
	if (InitTmpMem(pEvtCtrl, &tDataFields) == false)
	{	
		DTRACE(DB_INMTR, ("InitEvt: wOI=%u InitTmpMem() fail.\r\n", wOI));
		return false;
	}

	//建表
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		dwROAD = GetOAD(wOI, bAttrTab, 0);
		pOaMap = GetOIMap(dwROAD);
		iLen = CreateTable(pOaMap->pszTableName, &tFixFields, &tDataFields, (DWORD)wMaxNum);
		if (iLen <= 0)	
		{	
			DTRACE(DB_INMTR, ("InitEvt: wOI=%u bItem=%d CreateTable fail.\r\n", wOI, bItem));
			return false;
		}
		//置初始化标识
		pEvtCtrl->pEvtBase[bItem].fInitOk = true;
		pEvtCtrl->pEvtBase[bItem].bState= EVT_S_BF_HP;
		//下一个表
		bAttrTab++;
	}

	//初始化后强制刷新数据
	UpdateRecMem(pEvtCtrl, 1);
	UpdateItemMem(pEvtCtrl, 1);
	
	DTRACE(DB_INMTR, ("InitEvt: wOI=%u Init sucess.\r\n", wOI));
	return true;
}

//描述：失压事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitVLoss(struct TEvtCtrl* pEvtCtrl)
{
	TVLoss* pEvtPriv = (TVLoss* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;
	memset((BYTE*)pEvtPriv, 0x00, sizeof(TVLoss));
	return InitEvt(pEvtCtrl);
}

//描述：需量事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitDmd(struct TEvtCtrl* pEvtCtrl)
{
	TDmd* pEvtPriv = (TDmd* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;
	memset((BYTE*)pEvtPriv, 0x00, sizeof(TDmd));	
	return InitEvt(pEvtCtrl);
}

//描述：全失压事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitAVLoss(struct TEvtCtrl* pEvtCtrl)
{
	TAllVLoss* pEvtPriv = (TAllVLoss* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;
	if (InitEvt(pEvtCtrl) == false)
		return false;
	if (pEvtPriv->tEvtBase.fInitOk)
		memcpy((BYTE*)&pEvtCtrl->pEvtBase[0]+1, (BYTE*)pEvtPriv+1, sizeof(TAllVLoss)-1);	//为掉电变量，不更改初始化标识
	return true;
}

//描述：事件清零初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitEvtClr(struct TEvtCtrl* pEvtCtrl)
{
	TEvtClr* pEvtPriv = (TEvtClr* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;
	memset((BYTE*)pEvtPriv, 0x00, sizeof(TEvtClr));
	return InitEvt(pEvtCtrl);
}

//描述：事件接口函数初始化。由主线程调用
//参数：无
//返回：无
void InitTermEvt()
{
	for(BYTE i=0; i<EVT_NUM; i++)
		g_EvtCtrl[i].pfnInitEvt(&g_EvtCtrl[i]);
}

//描述：失压事件判断函数
//在三相供电系统中，某相电流大于设定的失压事件电流触发下限，
//同时该相电压低于设定的失压事件电压触发上限，
//且持续时间大于设定的失压事件判定延时时间，此中工况称为该相失压。
//注1: 三相三线情况下，不判断B相失压。
//注2: 全失压发生时，分相失压事件记录结束。
//注3: 当"失压事件触发上限"设定为"0"时，表示不启用。
//@属性5（配置参数）∷=structure
//{
//	电压触发上限  long-unsigned（单位：V，换算：-1），
//	电压恢复下限  long-unsigned（单位：V，换算：-1），
//	电流触发下限  double-long（单位：A，换算：-4），
//	判定延时时间  unsigned（单位：s，换算：0）
//}
int VLossJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		WORD wVUp;			
		WORD wVDown;			
		int iIDown;			
		BYTE bDelaySec;		
	}tPara;
	WORD wVol[3];	//当前电压		
	int iIval[3];		//当前电流			
	TTermEvtCtrl* pAllVLossEvtCtrl;
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_VLOSS)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("VLossJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//获取全失压事件控制结构
	pAllVLossEvtCtrl = GetTermEvtCtrl(MTR_ALLVLOSS);
	if (pAllVLossEvtCtrl == NULL)
		return 0;	
	if  (pAllVLossEvtCtrl->pEvtBase[0].fExcValid)
	{
		DTRACE(DB_INMTR, ("VLossJudge: AllVLoss is valid.\r\n"));
		return 0;
	}

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("VLossJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("VLossJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.wVUp = OoLongUnsignedToWord(bBuf+3);
		tPara.wVDown = OoLongUnsignedToWord(bBuf+6);
		tPara.iIDown = OoDoubleLongToInt(bBuf+9);
		tPara.bDelaySec = *(bBuf+14);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.wVUp == 0)
	{	
		DTRACE(DB_INMTR, ("VLossJudge: para wVUp=%u.\r\n", tPara.wVUp));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	// 2000 3 电压 数据类型：long-unsigned，单位：V，换算：-1，A相、B相、C相
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//2001 3 电流数据类型：double-long，单位：A换算：-3，A相、B相、C相、N线
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5)*10;	//小数位调整与事件配置参数一致，即4位小数 XXXX.XXXX A
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	//状态判断
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
#if 0	//调试使用								
		if ((g_TestFlag == 1)&&(bItem == 0))	
		{	if ((wVol[bItem]<tPara.wVUp) && (labs(iIval[bItem])> labs(tPara.iIDown)))			
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
			else if ((wVol[bItem]>=tPara.wVDown) || labs(iIval[bItem])<= labs(tPara.iIDown))	
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		}	
		else if ((g_TestFlag == 2)&&(bItem == 1))	
		{	if ((wVol[bItem]<tPara.wVUp) && (labs(iIval[bItem])> labs(tPara.iIDown)))			
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
			else if ((wVol[bItem]>=tPara.wVDown) || labs(iIval[bItem])<= labs(tPara.iIDown))	
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		}	
		else if ((g_TestFlag == 3)&&(bItem == 2))	
		{	if ((wVol[bItem]<tPara.wVUp) && (labs(iIval[bItem])> labs(tPara.iIDown)))			
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
			else if ((wVol[bItem]>=tPara.wVDown) || labs(iIval[bItem])<= labs(tPara.iIDown))	
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		}
		else
		{	if ((wVol[bItem]<tPara.wVUp) && (labs(iIval[bItem])> labs(tPara.iIDown)))			
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
			else if ((wVol[bItem]>=tPara.wVDown) || labs(iIval[bItem])<= labs(tPara.iIDown))	
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		}
#else
		if ((wVol[bItem]<tPara.wVUp) && (labs(iIval[bItem])> labs(tPara.iIDown)))			
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else if ((wVol[bItem]>=tPara.wVDown) || labs(iIval[bItem])<= labs(tPara.iIDown))	
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
#endif	
		//DTRACE(DB_INMTR, ("************************VLossJudge: wVol=%u, tPara.wVUp=%u, tPara.wVDown=%u,iIval=%u, tPara.iIDown=%u, bItem=%d, bJudgeState=%d.\r\n",  wVol[bItem], tPara.wVUp, tPara.wVDown, iIval[bItem], tPara.iIDown, bItem, pEvtCtrl->pEvtBase[bItem].bJudgeState));	

	}	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//描述：欠压事件判断函数
//在三相(或单相)供电系统中，某相电压小于设定的欠压事件电压触发上限，
//且持续时间大于设定的欠压事件判定延时时间此种工况称为欠压。
//注:当"欠压事件电压触发上限"设定为"0"时，表示"欠压事件"不启用。
//注:三相三线情况下，不判断B相。
//属性5（配置参数）∷=structure
//{
//	电压触发上限  long-unsigned（单位：V，换算：-1），
//	判定延时时间  unsigned（单位：s，换算：0）
//}
int VLessJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		WORD wVUp;			
		BYTE bDelaySec;		
	}tPara;
	WORD wVol[3];	//当前电压		
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_VLESS)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("VLessJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("VLessJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("VLessJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.wVUp = OoLongToInt16(bBuf+3);
		tPara.bDelaySec = *(bBuf+6);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.wVUp == 0)
	{	
		DTRACE(DB_INMTR, ("VLessJudge: para wVUp=%u.\r\n", tPara.wVUp));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	// 2000 3 电压 数据类型：long-unsigned，单位：V，换算：-1，A相、B相、C相
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//状态判断
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
#if 0	//调试使用								
		if ((g_TestFlag == 1)&&(bItem == 0))	
		{	
			if (wVol[bItem]<tPara.wVUp)		
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		}	
		else if ((g_TestFlag == 2)&&(bItem == 1))	
		{	
			if (wVol[bItem]<tPara.wVUp)		
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		}	
		else if ((g_TestFlag == 3)&&(bItem == 2))	
		{	
			if (wVol[bItem]<tPara.wVUp)		
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		}
		else
		{
			if (wVol[bItem]<tPara.wVUp)		
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
			else
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		}
#else
		if (wVol[bItem]<tPara.wVUp)		
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
#endif	
		
		//DTRACE(DB_INMTR, ("************************VLessJudge: wVol=%u, tPara.wVUp=%u, bItem=%d, bJudgeState=%d.\r\n",  wVol[bItem], tPara.wVUp, bItem, pEvtCtrl->pEvtBase[bItem].bJudgeState));	
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}


//描述：过压事件判断函数
//在三相(或单相)供电系统中，某相电压大于设定的过压事件电压触发下限，
//且持续时间大于设定的过压事件判定延时时间此种工况称为过压。
//注:当"过压事件电压触发下限"设定为"0"时，表示"过压事件"不启用。
//注:三相三线情况下，不判断B相。
//属性5（配置参数）∷=structure
//{
//	电压触发下限  long-unsigned（单位：V，换算：-1），
//	判定延时时间  unsigned（单位：s，换算：0）
//}
int VOverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		WORD wVDown;			
		BYTE bDelaySec;		
	}tPara;
	WORD wVol[3];	//当前电压		
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_VOVER)	
		return -1;

	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("VOverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("VOverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("VOverJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.wVDown = OoLongToInt16(bBuf+3);
		tPara.bDelaySec = *(bBuf+6);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.wVDown == 0)
	{	
		DTRACE(DB_INMTR, ("VOverJudge: para wVDown=%u.\r\n", tPara.wVDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	// 2000 3 电压 数据类型：long-unsigned，单位：V，换算：-1，A相、B相、C相
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//状态判断
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;		
		if (wVol[bItem]>tPara.wVDown)		
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		//DTRACE(DB_INMTR, ("************************VOverJudge: wVol=%u, tPara.wVDown=%u, bItem=%d, bJudgeState=%d.\r\n",  wVol[bItem], tPara.wVDown, bItem, pEvtCtrl->pEvtBase[bItem].bJudgeState));	
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//描述：断相事件判断函数
//在三相供电系统中，当某相电压低于设定的断相事件电压触发上限，
//同时该相电流小于设定的断相事件电流触发上限，
//且持续时间大于设定的断相事件判定延时时间，此种 工况称为断相。
//注1:三相三线情况下，不判用B相。
//注2:当"断相事件电压触发上限"设定为"0"时，表示不启用。
//@属性5（配置参数）∷=structure
//{
//	电压触发上限  long-unsigned（单位：V，换算：-1），
//	电流触发上限  double-long（单位：A，换算：-4），
//	判定延时时间  unsigned（单位：s，换算：0）
//}
int VBreakJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		WORD wVUp;		
		int iIUp;			
		BYTE bDelaySec;	
	}tPara;
	WORD wVol[3];	//当前电压		
	int iIval[3];		//当前电流			
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_VBREAK)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("VBreakJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("VBreakJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("VBreakJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.wVUp = OoLongUnsignedToWord(bBuf+3);
		tPara.iIUp= OoDoubleLongToInt(bBuf+6);
		tPara.bDelaySec = *(bBuf+11);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.wVUp == 0)
	{	
		DTRACE(DB_INMTR, ("VBreakJudge: para wVUp=%u.\r\n", tPara.wVUp));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	// 2000 3 电压 数据类型：long-unsigned，单位：V，换算：-1，A相、B相、C相
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//2001 3 电流数据类型：double-long，单位：A换算：-3，A相、B相、C相、N线
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5)*10;	//小数位调整与事件配置参数一致，即4位小数 XXXX.XXXX A
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	//状态判断
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;		
		if ((wVol[bItem]<tPara.wVUp) && (labs(iIval[bItem])<labs(tPara.iIUp)))			
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		//DTRACE(DB_INMTR, ("************************VBreakJudge: wVol=%u, tPara.wVUp=%u, iIval=%u, tPara.iIUp=%u, bItem=%d, bJudgeState=%d.\r\n",  wVol[bItem], tPara.wVUp, iIval[bItem], tPara.iIUp, bItem, pEvtCtrl->pEvtBase[bItem].bJudgeState));	
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}


//描述：失流事件判断函数
//在三相供电系统中，三相中至少有一相负荷电流大于失流事件电流触发下限，
//某相电压大于设定的失流事件电压触发下限，同时该相电流小于设定的失流事件电流触发上限时，
//且持续时间大于设定的失流事件判定延时时间，此种工况称为该相失流。
//注:当"失流事件电流触发上限"设定为"0"时，表求不启用。
//注:三相三线情况下，不判断B相。
//@属性5（配置参数）∷=structure
//{
//	电压触发下限  long-unsigned（单位：V，换算：-1），
//	电流触发上限  double-long（单位：A，换算：-4），
//	电流恢复下限  double-long（单位：A，换算：-4），
//	判定延时时间  unsigned（单位：s，换算：0）
//}
int ILossJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		WORD wVDowm;		
		int iIUp;				
		int iIDown;		
		BYTE bDelaySec;		
	}tPara;
	WORD wVol[3];	//当前电压		
	int iIval[3];		//当前电流			
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_ILOSS)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("ILossJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("ILossJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("ILossJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.wVDowm = OoLongUnsignedToWord(bBuf+3);
		tPara.iIUp = OoDoubleLongToInt(bBuf+6);
		tPara.iIDown = OoDoubleLongToInt(bBuf+11);
		tPara.bDelaySec = *(bBuf+15);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iIUp == 0)
	{	
		DTRACE(DB_INMTR, ("ILossJudge: para iIUp=%u.\r\n", tPara.iIUp));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	// 2000 3 电压 数据类型：long-unsigned，单位：V，换算：-1，A相、B相、C相
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//2001 3 电流数据类型：double-long，单位：A换算：-3，A相、B相、C相、N线
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5)*10;	//小数位调整与事件配置参数一致，即4位小数 XXXX.XXXX A
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	//状态判断
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
		if (bPhase==2)
		{
			if (((labs(iIval[0])>labs(tPara.iIDown))||(labs(iIval[2])>labs(tPara.iIDown))) 
				&& (wVol[bItem]>tPara.wVDowm) && (labs(iIval[bItem])<labs(tPara.iIUp)))	
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		}
		else if (bPhase==3)
		{
			if (((labs(iIval[0])>labs(tPara.iIDown))||(labs(iIval[1])>labs(tPara.iIDown))||(labs(iIval[2])>labs(tPara.iIDown))) 
				&& (wVol[bItem]>tPara.wVDowm) && (labs(iIval[bItem])<labs(tPara.iIUp)))	
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		}
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//描述：过流事件判断函数
//在三相(或单相)供电系统中，某相负荷电流大于设定的过流事件电流触发下限，
//且持续时间大于设定的过流事件判定延时时间，此种工况称为过流。
//注: 当"过流电流触发下限"设定为"0"时，表求不启用。
//注:三相三线情况下，不判断B相。
//属性5（配置参数）∷=structure
//{
//	电流触发下限  double-long（单位：A，换算：-4），
//	判定延时时间  unsigned（单位：s，换算：0）
//}
int IOverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		int iDown;		
		BYTE bDelaySec;		
	}tPara;
	int iIval[3];		//当前电流			
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_IOVER)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("IOverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("IOverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("IOverJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.iDown= OoDoubleLongToInt(bBuf+3);
		tPara.bDelaySec = *(bBuf+8);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iDown == 0)
	{	
		DTRACE(DB_INMTR, ("IOverJudge: para iDown=%u.\r\n", tPara.iDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//2001 3 电流数据类型：double-long，单位：A换算：-3，A相、B相、C相、N线
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5)*10;	//小数位调整与事件配置参数一致，即4位小数 XXXX.XXXX A
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	//状态判断
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
		if (labs(iIval[bItem])>labs(tPara.iDown))	
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		//DTRACE(DB_INMTR, ("************************IOverJudge: iIval=%u,  iDown=%u, bItem=%d, bJudgeState=%d.\r\n",iIval[bItem],  tPara.iDown, bItem, pEvtCtrl->pEvtBase[bItem].bJudgeState));	
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//描述：断流事件判断函数
//在三相(或单相)供电系统中，某相电压大于断流事件触发下限，
//同时相电流小于设定的断流事件电流触发上限，
//且持续时间大于设定的断流事件判定延时时间，此种工况称为断流。
//注:当"断流事件电流触发上限"设定为"0"时，表示不启用。
//注:三相三线情况下，不判断B相。
//@属性5（配置参数）∷=structure
//{
//	电压触发下限  long-unsigned（单位：V，换算：-1），
//	电流触发上限  double-long（单位：A，换算：-4），
//	判定延时时间  unsigned（单位：s，换算：0）
//}
int IBreakJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		WORD wVDowm;		
		int iIUp;				
		BYTE bDelaySec;	
	}tPara;
	WORD wVol[3];	//当前电压		
	int iIval[3];		//当前电流			
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_IBREAK)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("IBreakJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("IBreakJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("IBreakJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.wVDowm = OoLongUnsignedToWord(bBuf+3);
		tPara.iIUp = OoDoubleLongToInt(bBuf+6);
		tPara.bDelaySec = *(bBuf+11);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iIUp == 0)
	{	
		DTRACE(DB_INMTR, ("IBreakJudge: para iIUp=%u.\r\n", tPara.iIUp));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	// 2000 3 电压 数据类型：long-unsigned，单位：V，换算：-1，A相、B相、C相
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//2001 3 电流数据类型：double-long，单位：A换算：-3，A相、B相、C相、N线
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5)*10;	//小数位调整与事件配置参数一致，即4位小数 XXXX.XXXX A
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	//状态判断
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
		if ((wVol[bItem]>tPara.wVDowm) && (labs(iIval[bItem])<labs(tPara.iIUp)))		
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
		//DTRACE(DB_INMTR, ("************************IBreakJudge: wVol=%u, tPara.wVDowm=%u, iIval=%u, tPara.iIUp=%u, bItem=%d, bJudgeState=%d.\r\n",  wVol[bItem], tPara.wVDowm, iIval[bItem], tPara.iIUp, bItem, pEvtCtrl->pEvtBase[bItem].bJudgeState));	
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//描述：潮流反向事件判断函数
//在三相供电系统中，当总有功功率方向改变方向时，同时有功功率大于
//设定的潮流反向事件有功功率触发下限，且持续时间大于设定的潮流反向延时时间，
//此种式况称为潮流反向。
//注:当'潮流反向事件有功功率触发下限'设置为0时，表示不启用。
//注:三相三线情况下，不判断B相。
//@属性5（配置参数）∷=structure
//{
//	有功功率触发下限  double-long（单位：W，换算：-1），
//	判定延时时间  unsigned（单位：s，换算：0）
//}
int PReverseJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		int iPowDown;		
		BYTE bDelaySec;		
	}tPara;
	int iPow[4];	//当前功率			
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_PREVERSE)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("PReverseJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("PReverseJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("PReverseJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.iPowDown= OoDoubleLongToInt(bBuf+3);
		tPara.bDelaySec = *(bBuf+8);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iPowDown == 0)
	{	
		DTRACE(DB_INMTR, ("PReverseJudge: para iPowDown=%u.\r\n", tPara.iPowDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//2004 4 有功功率 数据类型：double-long，单位：W，换算：-1，总、A相、B相、C相
	if (OoReadAttr(0x2004, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<4; i++)
			iPow[i] = OoDoubleLongToInt(bBuf+3+i*5);
	}	
	else
		memset((BYTE*)&iPow, 0x00, sizeof(iPow));

	//状态判断
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
		if ((iPow[bItem+1]<0) && (labs(iPow[bItem+1])>labs(tPara.iPowDown)))	
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//描述：过载事件判断函数
//在三相(或单相)供电系统中，某相功率大于设定的过载事件有功功率触发下限，
//且持续时间大于设定的过载事件判定延时时间，此种工况称为过载。
//注:当'过载事件有功功率触发下限'设置为0时，表示不启用。
//注:三相三线情况下，不判断B相。
//@属性5（配置参数）∷=structure
//{
//	有功功率触发下限  double-long（单位：W，换算：-1），
//	判定延时时间  unsigned（单位：s，换算：0）
//}
int POverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		int iPowDown;		
		BYTE bDelaySec;		
	}tPara;
	int iPow[4];	//当前功率			
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_POVER)	
		return -1;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("POverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("POverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("POverJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.iPowDown= OoDoubleLongToInt(bBuf+3);
		tPara.bDelaySec = *(bBuf+8);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iPowDown == 0)
	{	
		DTRACE(DB_INMTR, ("POverJudge: para iPowDown=%u.\r\n", tPara.iPowDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//2004 4 有功功率 数据类型：double-long，单位：W，换算：-1，总、A相、B相、C相
	if (OoReadAttr(0x2004, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<4; i++)
			iPow[i] = OoDoubleLongToInt(bBuf+3+i*5);
	}	
	else
		memset((BYTE*)&iPow, 0x00, sizeof(iPow));

	//状态判断
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if ((bPhase==2) && (bItem==1))
			continue;	
		if (labs(iPow[bItem+1]) > labs(tPara.iPowDown))	
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}


//描述：需量超限判断函数
//在三相(或单相)供电系统中，当总有功需量大于设定的有功需量超限事件触发下限，
//且技术时间大于设定的需量超限事件判定延时时间，此种工况称为有功需量越限。
//当触发下限值为0时表示不启用。
//@属性6（配置参数）∷=structure
//{
//	触发下限  double-long-unsigned（单位：kW，换算：-4），
//	判定延时时间  unsigned（单位：s，换算：0）
//}
int PDmdOverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[60];	
	struct
	{	
		DWORD dwDown;		
		BYTE bDelaySec;			
	}tPara;
	DWORD dwDmd;	//当前正向有功总需量	
	
	if (pEvtCtrl->wOI != MTR_PDMDOVER)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("PDmdOverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("PDmdOverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("PDmdOverJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.dwDown= OoDoubleLongUnsignedToDWord(bBuf+3);
		tPara.bDelaySec = *(bBuf+8);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.dwDown == 0)
	{	
		DTRACE(DB_INMTR, ("PDmdOverJudge: para dwDown=%u.\r\n", tPara.dwDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//当前正向有功总需要
	if (ReadItemEx(BN2, PN0, 0x3010, bBuf) > 0)
		dwDmd = OoDoubleLongUnsignedToDWord(bBuf+1);
	else
		dwDmd = 0;

	//状态判断
	if (dwDmd > tPara.dwDown)
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
	else
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;

	DTRACE(DB_INMTR, ("************************PDmdOverJudge: dwDmd=%x,  dwDown=%x, bJudgeState=%d.\r\n",dwDmd, tPara.dwDown, pEvtCtrl->pEvtBase[0].bJudgeState));	
	
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//描述：需量超限判断函数
//在三相(或单相)供电系统中，当总有功需量大于设定的有功需量超限事件触发下限，
//且技术时间大于设定的需量超限事件判定延时时间，此种工况称为有功需量越限。
//当触发下限值为0时表示不启用。
//@属性6（配置参数）∷=structure
//{
//	触发下限  double-long-unsigned（单位：kW，换算：-4），
//	判定延时时间  unsigned（单位：s，换算：0）
//}
int RPDmdOverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[60];	
	struct
	{	
		DWORD dwDown;		
		BYTE bDelaySec;			
	}tPara;
	DWORD dwDmd;	//当前反向有功总需量	
	
	if (pEvtCtrl->wOI != MTR_RPDMDOVER)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("RPDmdOverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("RPDmdOverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("RPDmdOverJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.dwDown= OoDoubleLongUnsignedToDWord(bBuf+3);
		tPara.bDelaySec = *(bBuf+8);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.dwDown == 0)
	{	
		DTRACE(DB_INMTR, ("RPDmdOverJudge: para dwDown=%u.\r\n", tPara.dwDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//当前反向有功总需要
	if (ReadItemEx(BN2, PN0, 0x3020, bBuf) > 0)
		dwDmd = OoDoubleLongUnsignedToDWord(bBuf+1);
	else
		dwDmd = 0;

	//状态判断
	if (dwDmd > tPara.dwDown)
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
	else
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;

	DTRACE(DB_INMTR, ("************************RPDmdOverJudge: dwDmd=%x,  dwDown=%x, bJudgeState=%d.\r\n",dwDmd, tPara.dwDown, pEvtCtrl->pEvtBase[0].bJudgeState));	

	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//描述：需量超限判断函数
//在三相(或单相)供电系统中，当总无功需量大于设定的无功需量超限事件触发下限，
//且持续时间大于设定的需量超限事件判定延时时间，此种工况称为有功需量越限。
//当触发下限值为0时表示不启用?
//@属性6（配置参数）∷=structure
//{
//	触发下限  double-long-unsigned（单位：kW，换算：-4），
//	判定延时时间  unsigned（单位：s，换算：0）
//}
int QDmdOverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bItem, bBuf[180];	
	struct
	{	
		DWORD dwDown;		
		BYTE bDelaySec;			
	}tPara;
	DWORD dwDmd[4];	//当前需量		
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_QDMDOVER)	
		return -1;
		
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("QDmdOverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC24_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("QDmdOverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("QDmdOverJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC24_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.dwDown= OoDoubleLongUnsignedToDWord(bBuf+3);
		tPara.bDelaySec = *(bBuf+8);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.dwDown == 0)
	{	
		DTRACE(DB_INMTR, ("QDmdOverJudge: para dwDown=%u.\r\n", tPara.dwDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//当前无功需量
	for(i=0; i<4; i++)
	{
		if (ReadItemEx(BN2, PN0, (0x3030+i), bBuf) > 0)
			dwDmd[i] = OoDoubleLongUnsignedToDWord(bBuf+1);
		else
			dwDmd[i] = 0;
	}

	//状态判断
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)	
	{	
		if (dwDmd[bItem] > tPara.dwDown)
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[bItem].bJudgeState = EVT_JS_END;

		DTRACE(DB_INMTR, ("************************QDmdOverJudge: dwDmd[%d]=%x,  dwDown=%x, bJudgeState=%d.\r\n",bItem, dwDmd[bItem], tPara.dwDown, pEvtCtrl->pEvtBase[0].bJudgeState));
	}
	
	return pEvtCtrl->pEvtBase[bItem].bJudgeState;
}

//描述：功率因数 超下限判断函数
//在三相供电系统中，当总功率因数小于设定的功率因数超下限阀值，
//同时任意一相电流大于5%基本电流，
//且持续时间大于设定的功率因数超下限判定时间，此种工况称为总功率因数超下限。
//注:当"功率因数超下限阀值"设定为"0"时，表示"总功率因数超下限事件不启用"
//单相表不要求总功率因数超下限。
//@属性6（配置参数）∷=structure
//{
//  下限阀值  long（单位：%，换算：-1），
//  判定延时时间  unsigned（单位：s，换算：0）
//}
int PfUnderJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		int16 iDown;			//XXX.X (如30%对应300)
		BYTE bDelaySec;			
	}tPara;
	int16 iPf;
	int iIval[3];	
	DWORD dwIb;	//基本电流
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_PFUNDER)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("PfUnderJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("PfUnderJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("PfUnderJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.iDown= OoLongToInt16(bBuf+3);
		tPara.bDelaySec = *(bBuf+6);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iDown == 0)
	{	
		DTRACE(DB_INMTR, ("PfUnderJudge: para iDown=%u.\r\n", tPara.iDown));
		return 0;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;	//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;

	//2001 3 电流 数据类型：double-long，单位：A换算：-3，A相、B相、C相、N线
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5);	
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	
	
	//基本电流
	ReadItemEx(BN25, PN0, 0x5002, bBuf);
	dwIb= BcdToDWORD(bBuf, 3)*0.05;	//NNN.NNNA, 5%Ib
	
	// 200A 4 功率因数 数据类型：long，单位：无，换算：-3， 总、A相、B相、C相，只获取总
	if (OoReadAttr(0x200A, 0x02, bBuf, NULL, NULL) > 0)
		iPf = OoLongToInt16(bBuf+3);
	else
		iPf = -1000;		

	//状态判断
	if (bPhase == 2)
	{
		if ((iPf<tPara.iDown) && ((labs(iIval[0])>dwIb)||(labs(iIval[2])>dwIb)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}
	else if(bPhase == 3)
	{
		if ((iPf<tPara.iDown) && ((labs(iIval[0])>dwIb)||(labs(iIval[1])>dwIb)||(labs(iIval[2])>dwIb)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}
	//DTRACE(DB_INMTR, ("************************PfUnderJudge: iPf=%04x, tPara.iDown=%04x, iIval=%u-%u-%u,  dwIb=%u, bJudgeState=%d.\r\n",iPf, tPara.iDown, iIval[0], iIval[1], iIval[2], dwIb, pEvtCtrl->pEvtBase[0].bJudgeState));	
	
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//描述：全失压事件判断函数
//在三相供电系统中，若三相电压均低于电能表的临界电压，
//且有任意一相或我相负荷电流大于5%基本电流，
//且持续时间大于60s，此种工况秒钟为全失压。
//全失压时，电压降低直到电能表不能工作时，不记录全失压结束，直到电压恢复至
//电能表启动工作时，再进行全失压事件的判断。
//电表停止工作后，在停止工作60s时检测且仅检测电流一次，进行全失压事件记录的判断，此后不再检测电流。
//属性6（配置参数）∷=structure
//{
//}
int AVLossJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[EVT_ATRR_MAXLEN];	
	WORD wVol[3], wUn;		//电压 XXX.X v
	int iIval[3];				//ABCN 电流 XXXXX.XXX A
	DWORD dwIb;
	BYTE i;
	
	if (pEvtCtrl->wOI != MTR_ALLVLOSS)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("AVLossJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("AVLossJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("AVLossJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	pEvtCtrl->bDelaySec = 60;	//全失压固定判定延时为60秒

	// 2000 3 电压 数据类型：long-unsigned，单位：V，换算：-1，A相、B相、C相
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//2001 3 电流 数据类型：double-long，单位：A换算：-3，A相、B相、C相、N线
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5);	
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	// 基本电压
	ReadItemEx(BN25, PN0, 0x5001, bBuf);
	wUn = (WORD)(BcdToDWORD(bBuf, 3)/10); //NNNN.NN转为XXX.XV
	wUn *= 0.6;	//临界电压
	
	//基本电流
	ReadItemEx(BN25, PN0, 0x5002, bBuf);
	dwIb= BcdToDWORD(bBuf, 3)*0.05;	//NNN.NNNA, 5%Ib

	//状态判断
	if (bPhase == 2)
	{
		if ((wVol[0]<wUn) && (wVol[2]<wUn) && ((labs(iIval[0])>dwIb)||(labs(iIval[2])>dwIb)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}
	else if(bPhase == 3)
	{
		if ((wVol[0]<wUn) && (wVol[1]<wUn) && (wVol[2]<wUn) && ((labs(iIval[0])>dwIb)||(labs(iIval[1])>dwIb)||(labs(iIval[2])>dwIb)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}
	//DTRACE(DB_INMTR, ("************************AVLossJudge: wVol=%u-%u-%u, wUn=%u,  iIval=%u-%u-%u, dwIb=%u, bJudgeState=%d.\r\n",wVol[0], wVol[1], wVol[2], wUn,  iIval[0], iIval[1], iIval[2], dwIb, pEvtCtrl->pEvtBase[0].bJudgeState));	
	
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//描述：逆相序判断函数，包括电压逆向序和电流逆相序
//在三相供电系统中，三相电压均大于电能表的临界电压，
//三相电压逆相序，且持续时间大于60秒，记录为电压逆相序事件。
//在三相供电系统中，三相电压均大于电能表的临界电压，
//三相电流均大于5%额定电流，三相电流逆相序，且持续时间大于60秒，记录为电流逆相序事件
//@属性6（配置参数）∷=structure
//{
//  判定延时  unsigned（单位：s，换算：0）
//}
int DisOrderJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		BYTE bDelaySec;		
	}tPara;
	WORD wVol[3], wUn;		//电压 XXX.X v
	int iIval[3];				//ABCN 电流 XXXXX.XXX A
	DWORD dwIb;
	BYTE bPolar, i;
	
	if ((pEvtCtrl->wOI!=MTR_VDISORDER) && (pEvtCtrl->wOI!=MTR_IDISORDER))
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("DisOrderJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("DisOrderJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("DisOrderJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
		tPara.bDelaySec = *(bBuf+3);
	else
		tPara.bDelaySec = 0;
	if (tPara.bDelaySec == 0)
		tPara.bDelaySec = DEFAULT_DELAY_SEC;//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;
	
	// 2000 3 电压 数据类型：long-unsigned，单位：V，换算：-1，A相、B相、C相
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	//2001 3 电流数据类型：double-long，单位：A换算：-3，A相、B相、C相、N线
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5);	//小数位调整与事件配置参数一致，即4位小数 XXXX.XXXX A
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	// 基本电压
	ReadItemEx(BN25, PN0, 0x5001, bBuf);
	wUn = (WORD)(BcdToDWORD(bBuf, 3)/10); //NNNN.NN转为XXX.XV
	wUn *= 0.6;	//临界电压
	
	//基本电流
	ReadItemEx(BN25, PN0, 0x5002, bBuf);
	dwIb= BcdToDWORD(bBuf, 3)*0.05;	//NNN.NNNA, 5%Ib

	// 逆相序，扩展ID
	ReadItemEx(BN2, PN0, 0x1120, &bPolar);

	//状态判断
	if (pEvtCtrl->wOI == MTR_VDISORDER)
	{
		if (bPhase == 2)
		{
			if ((bPolar==1) && (wVol[0]>wUn) && (wVol[2]>wUn))
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
		}
		else if(bPhase == 3)
		{
			if ((bPolar==1) && (wVol[0]>wUn) && (wVol[1]>wUn) && (wVol[2]>wUn))
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
		}
		//DTRACE(DB_INMTR, ("************************UDisOrderJudge: bPolar=%d,  wVol=%u-%u-%u, wUn=%u, bJudgeState=%d.\r\n",bPolar,  wVol[0], wVol[1],wVol[2],wUn, pEvtCtrl->pEvtBase[0].bJudgeState));	
	
	}
	else if (pEvtCtrl->wOI == MTR_IDISORDER)
	{
		if (bPhase == 2)
		{
			if ((bPolar==2) && (wVol[0]>wUn) && (wVol[2]>wUn) && (labs(iIval[0])>dwIb) && (labs(iIval[2])>dwIb))
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
		}
		else if(bPhase == 3)
		{
			if ((bPolar==2) && (wVol[0]>wUn) && (wVol[1]>wUn) && (wVol[2]>wUn) && (labs(iIval[0])>dwIb) && (labs(iIval[1])>dwIb) && (labs(iIval[2])>dwIb))
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			else
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
		}
		//DTRACE(DB_INMTR, ("************************iDisOrderJudge: bPolar=%d,  wVol=%u-%u-%u, iIval=%u, wVol=%u-%u-%u, dwIb=%u, bJudgeState=%d.\r\n",bPolar,  wVol[0], wVol[1],wVol[2],wUn, iIval[0], iIval[1],iIval[2],dwIb, pEvtCtrl->pEvtBase[0].bJudgeState));	
	}
	
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}


//描述: 电表清零判断函数
int MtrClrJudge(struct TEvtCtrl* pEvtCtrl)
{	
	return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_EVT_MTRCLR);
}

//描述: 需量清零判断函数
int DmdClrJudge(struct TEvtCtrl* pEvtCtrl)
{
	return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_EVT_DMDCLR);	
}

//描述: 事件清零判断函数
int EvtClrJudge(struct TEvtCtrl* pEvtCtrl)
{	
	return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_EVT_EVTCLR);
}

//描述：电压不平衡判断函数
//当三相电压有的任一相大于电能表的临界电压，
//电压不平衡率大于设定的电压不平衡率限值，
//且持续时间大于设定的电压不平衡率判定延时时间，此种工况为电压不平衡。
//注:当"当电不平衡率限值"设定为"0"时，表示"电压不平衡事件"不启用
//@属性6（配置参数）∷=structure
//{
//  限值  long（单位：%，换算：-2），
//  判定延时时间  unsigned（单位：s，换算：0）
//}
int VUnBalanceJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		int16 iRate;			//XX.XX (如30%对应3000)
		BYTE bDelaySec;		//XX s	
	}tPara;
	WORD wVol[3], wUn;		//电压 XXX.X v
	WORD wRate = 0;	
	BYTE i;
	
	if (pEvtCtrl->wOI!=MTR_VUNBALANCE)
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("VUnBalanceJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("VUnBalanceJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("VUnBalanceJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.iRate = OoLongToInt16(bBuf+3);
		tPara.bDelaySec = *(bBuf+6);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iRate == 0)
	{	
		DTRACE(DB_INMTR, ("VUnBalanceJudge: para iRate=%x.\r\n", tPara.iRate));
		return false;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;
		
	// 2000 3 电压 数据类型：long-unsigned，单位：V，换算：-1，A相、B相、C相
	if (OoReadAttr(0x2000, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			wVol[i] = OoLongUnsignedToWord(bBuf+3+i*3);
	}
	else
		memset((BYTE*)&wVol, 0x00, sizeof(wVol));	

	// 基本电压
	ReadItemEx(BN25, PN0, 0x5001, bBuf);
	wUn = (WORD)(BcdToDWORD(bBuf, 3)/10); //NNNN.NN转为XXX.XV
	wUn *= 0.6;	//临界电压
	
	//2026 6 电压不平衡率 数据类型：long-unsigned，单位：%，换算：-2
	if (OoReadAttr(0x2026, 0x02, bBuf, NULL, NULL) > 0)
		wRate = OoLongUnsignedToWord(bBuf+1);
	else
		wRate = 0;

	//状态判断
	if (bPhase == 2)
	{
		if ((wRate>labs(tPara.iRate)) && ((wVol[0]>wUn)||(wVol[2]>wUn)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}
	else if (bPhase == 3)
	{
		if ((wRate>labs(tPara.iRate)) && ((wVol[0]>wUn)||(wVol[1]>wUn)||(wVol[2]>wUn)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}

	DTRACE(DB_INMTR, ("************************VUnBalanceJudge: wRate=%u,  tPara.iRate=%u, wVol=%u-%u-%u, wUn=%u, bJudgeState=%d.\r\n",wRate, tPara.iRate, wVol[0], wVol[1], wVol[2], wUn, pEvtCtrl->pEvtBase[0].bJudgeState));	
	
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//描述：电流不平衡事件判断函数，包括电流不平衡和电流严重不平衡
//当三相电流中的任一相电流大于5%基本电流，
//电流不平衡率大于设定的电流(严重)不平衡率限值，
//且持续时间大于设定的电流(严重)不平衡判定延时时间，此种工况为电流(严重)不平衡。
//当"电流(严重)不平衡率限值"设定为"0"时，表示电流(严重)不平衡事件不启用
//@属性6（配置参数）∷=structure
//{
//  限值  long（单位：%，换算：-2）
//  判定延时时间  unsigned（单位：s，换算：0）
//}
int IUnBalanceJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		int16 iRate;			//XX.XX (如30%对应3000)
		BYTE bDelaySec;		//XX s	
	}tPara;
	int iIval[3];				//ABCN 电流 XXXXX.XXX A
	DWORD dwIb;
	WORD wRate = 0;	
	BYTE i;
	
	if ((pEvtCtrl->wOI!=MTR_IUNBALANCE) && (pEvtCtrl->wOI!=MTR_ISUNBALANCE))
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("IUnBalanceJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("IUnBalanceJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("IUnBalanceJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数，值为0表示不启用
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
	{
		tPara.iRate = OoLongToInt16(bBuf+3);
		tPara.bDelaySec = *(bBuf+6);
	}
	else
		memset((BYTE*)&tPara, 0x00, sizeof(tPara));
	if (tPara.iRate == 0)
	{	
		DTRACE(DB_INMTR, ("IUnBalanceJudge: para iRate=%x.\r\n", tPara.iRate));
		return false;
	}		
	if (tPara.bDelaySec == 0)	
		tPara.bDelaySec = DEFAULT_DELAY_SEC;//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;
		
	//2001 3 电流 数据类型：double-long，单位：A换算：-3
	//A相、B相、C相、N线
	if (OoReadAttr(0x2001, 0x02, bBuf, NULL, NULL) > 0)
	{
		for(i=0; i<3; i++)
			iIval[i] = OoDoubleLongToInt(bBuf+3+i*5);	
	}
	else
		memset((BYTE*)&iIval, 0x00, sizeof(iIval));	

	//基本电流
	ReadItemEx(BN25, PN0, 0x5002, bBuf);
	dwIb= BcdToDWORD(bBuf, 3)*0.05;	//NNN.NNNA, 5%Ib
 
	//2027 6 电流不平衡率数据类型：long-unsigned，单位：%，换算：-2
	if (OoReadAttr(0x2027, 0x02, bBuf, NULL, NULL) > 0)
		wRate = OoLongUnsignedToWord(bBuf+1);
	else
		wRate = 0;

	//状态判断
	if (bPhase == 2)
	{
		if ((wRate>labs(tPara.iRate)) &&((labs(iIval[0])>dwIb)||(labs(iIval[2])>dwIb)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}
	else if(bPhase == 3)
	{
		if ((wRate>labs(tPara.iRate)) && ((labs(iIval[0])>dwIb)||(labs(iIval[1])>dwIb)||(labs(iIval[2])>dwIb)))
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}

	//DTRACE(DB_INMTR, ("************************IUnBalanceJudge:  wOI=%04x,  wRate=%u,  tPara.iRate=%u, iIval=%u-%u-%u, dwIb=%u, bJudgeState=%d.\r\n", pEvtCtrl->wOI, wRate, tPara.iRate, iIval[0], iIval[1], iIval[2], dwIb, pEvtCtrl->pEvtBase[0].bJudgeState));	
	
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//描述：终端故障判断函数，包括时钟故障和计量芯片故障
//@属性6（配置参数）∷=structure
//{
//  判定延时  unsigned（单位：s，换算：0）
//}
int TermErrJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[EVT_ATRR_MAXLEN];	
	struct
	{	
		BYTE bDelaySec;		
	}tPara;
	BYTE bErr = 0;
	BYTE i;
	
	if ((pEvtCtrl->wOI!=MTR_CLKERR) && (pEvtCtrl->wOI!=MTR_MTRCHIPERR))
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("TermErrJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("TermErrJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//4010 8 计量元件数, 单相表为1，三相三线表为2，三相四线表为3。
	if (OoReadAttr(0x4010, 0x02, bBuf, NULL, NULL) > 0)
		bPhase = bBuf[1];
	else
		bPhase = 0;
	if ((bPhase!=2) && (bPhase!=3))		
	{	
		DTRACE(DB_INMTR, ("TermErrJudge: bPhase=%d.\r\n", bPhase));
		return 0;
	}

	//配置参数
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
		tPara.bDelaySec = *(bBuf+3);
	else
		tPara.bDelaySec = 0;
	if (tPara.bDelaySec == 0)
		tPara.bDelaySec = DEFAULT_DELAY_SEC;//延时时间为0默认为60秒
	pEvtCtrl->bDelaySec = tPara.bDelaySec;
	
	if (pEvtCtrl->wOI == MTR_CLKERR)
		ReadItemEx(BN2, PN0, 0x1122, &bErr);
	else if (pEvtCtrl->wOI == MTR_MTRCHIPERR)
		ReadItemEx(BN2, PN0, 0x1123, &bErr);

	//状态判断
	if(bErr)	
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
	else
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;		

	//DTRACE(DB_INMTR, ("************************TermErrJudge:  wOI=%04x,  bErr=%d, bJudgeState=%d.\r\n", pEvtCtrl->wOI, bErr, pEvtCtrl->pEvtBase[0].bJudgeState));	

	return pEvtCtrl->pEvtBase[0].bJudgeState;
}


//描述：从系统库读出OAD的数据内容
//参数：@dwOAD 数据标识
//		@pbBuf 数据缓冲区
//		@wDataLen OAD的数据长度
//		@wBufSize OAD的数据缓冲区大小
//返回：true/false
bool OoReadOad(DWORD dwOAD, BYTE* pbBuf, WORD wDataLen, WORD wBufSize)
{
	int iStart = -1;
	BYTE bAttr, bIndex, bType;
	BYTE bBuf[4096];	//tll 统计那边如果没有数据将返回分配的最大长度 SYSDB_ITEM_MAXSIZE
	BYTE bBufIdx[50];	//单个元素数据长度不会大于50
	BYTE* pbFmt;
	WORD wOI, wFmtLen, wID;
	int iLen;
	TTime now;
	BYTE bOadBuf[4];

	if (wDataLen > wBufSize)
		return false;
	
	GetOIAttrIndex(dwOAD, &wOI, &bAttr, &bIndex);

	// 时间需要特殊处理
	if (wOI==0x4000 || wOI==0x201e || wOI==0x2020)		//时间或事件发生时间或事件结束时间
	{	
		if (8 != wDataLen)
			return false;

		GetCurTime(&now);
		*pbBuf++ = DT_DATE_TIME_S;
		*pbBuf++ = now.nYear/256;
		*pbBuf++ = now.nYear%256;
		*pbBuf++ = now.nMonth;
		*pbBuf++ = now.nDay;
		*pbBuf++ = now.nHour;
		*pbBuf++ = now.nMinute;
		*pbBuf++ = now.nSecond;
		return true;
	}

	const ToaMap* pOI = GetOIMap(dwOAD&0xffff1f00);
	if (pOI == NULL)
		return false;

	if (IsNeedRdSpec(pOI) && bIndex==0)		//特殊OI的读取，如测量点参数，时间等
	{
		iStart = -1;
		iLen = OIRead_Spec((ToaMap *)pOI, bBuf, sizeof(bBuf), &iStart);
		pbFmt = pOI->pFmt;
		wFmtLen = pOI->wFmtLen;
	}
	else
	{
		if (IsNeedRdSpec(pOI))
		{
			if(pOI->wMode == MAP_SYSDB)		//
				iLen = ReadItemEx(BN0, bIndex-1, pOI->wID, bBuf);		//特殊OAD的第bIndex个属性对应第bIndex-1个测量点
			else
				iLen = OoReadAttr(wOI, bAttr, bBuf, &pbFmt, &wFmtLen);

			bIndex = 0;	//特殊OI都是人为拆分成子OAD，这里直接读子OAD的内容，等同于读特殊OAD的第几个属性
		}
		else
		{
			memset(bOadBuf, 0, sizeof(bOadBuf));
			OoDWordToOad(dwOAD, bOadBuf);
			if (sizeof(bBuf) < OoGetDataLen(DT_OAD, bOadBuf))
			{
				DTRACE(DB_INMTR, ("OoReadOad: dwOAD=%08x Buf Not Enough, return!\r\n", dwOAD));
				return false;
			}

			iLen = OoReadAttr(wOI, bAttr, bBuf, &pbFmt, &wFmtLen);
			if (IsSpecFrzOAD(dwOAD) && IsIntervMatch(dwOAD))	//统计OAD,读取上一周期统计结果
			{
				wID = GetLastCycleFrzMapID(dwOAD);
				if (wID > 0)
					iLen = ReadItemEx(BN11, PN0, wID, bBuf);	//读取上一周期统计结果ID
			}
		}
	}

	if (iLen > 0)
	{	
		if (bIndex > 0)	//获取索引的数据
		{	
			iLen = OoReadField(bBuf, pbFmt, wFmtLen, (WORD)bIndex-1, bBufIdx, &bType);
			if (iLen > 0)
				memcpy(bBuf, bBufIdx, iLen);
		}
	}

	if (iLen>0 && iLen==wDataLen)
	{	
		memcpy(pbBuf, bBuf, wDataLen);
		return true;
	}
	
	return false;
}


//描述：根据字段OAD填充事件的特殊字段
//参数：@ dwROAD 事件记录表OAD
//		@ dwFieldOAD 字段OAD
//		@ pbField 字段缓冲区，传入旧值，按条件导出新值
//		@ wFieldLen 字段缓冲区的长度
//		@ wFieldSize 字段缓冲区的大小
//返回：如果有对特殊字段进行了正确处理则返回true，否则返回false
bool MakeEvtSpecField(DWORD dwROAD, DWORD dwFieldOAD, BYTE* pbField, WORD wFieldLen, WORD wFieldSize)
{
	BYTE bByte = 0, bBit = 0, bSaveMtrNum = 0;
	BYTE bAttr, bItem, bState, bCnNum;
	BYTE bBuf[50];
	BYTE* pbData = pbField;
	BYTE* pbData0 = pbData; 
	WORD wOI, wOldVal, wNewVal, wIndex = 0;
	DWORD dwRecSN, dwOldVal,dwNewVal;
	int iLen, iRet = 0;
	TTime tmCurRec;
	TTermEvtCtrl* pEvtCtrl;
	const TEvtAttr* pEvtAttr; 
	BYTE i;
	TDmd* pDmd = NULL;
	if (wFieldLen > wFieldSize)
		return false;

	GetOIAttrIndex(dwROAD, &wOI, &bAttr, NULL);
	//获取事件控制结构
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return -1;	
	//获取事件属性
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return -1;
	//获取事件分项号
	if (bAttr >= pEvtAttr->bRecTabStart)
		bItem = bAttr-pEvtAttr->bRecTabStart;
	else 
		return -1;

 	bState = pEvtCtrl->pEvtBase[bItem].bState;	//当前状态
 	
	switch(dwFieldOAD)
	{
		case 0x20220200:		//事件记录序号
			if (wFieldLen != 5)
				return false;
			if (bState == EVT_S_AFT_HP)
			{
				dwRecSN = OoDoubleLongUnsignedToDWord(pbField+1);
				dwRecSN++;
				*pbData++ = DT_DB_LONG_U;
				OoDWordToDoubleLongUnsigned(dwRecSN, pbData);
			}
			return true;
		case 0x201E0200:		//事件发生时间
			if (wFieldLen != 8)
				return false;
			if (wFieldSize < 16)
				return false;

			if (bState == EVT_S_AFT_HP)
			{
				if (wOI == TERM_CLOCKPRG)	//取校时前终端时间
				{
					TAdjTermTime* pEvtPriv = (TAdjTermTime* )pEvtCtrl->pEvtPriv;
					if (pEvtPriv == NULL)
						return false;

					memcpy(pbData, pEvtPriv->bClock, 8);
					return true;
				}

				if (wOI==TERM_POWOFF && !IsInvalidTime(g_PowerOffTmp.tPoweroff))		//终端停上电事件
					tmCurRec = g_PowerOffTmp.tPoweroff;		//事件发生时间取最近一次停电时间
				else
					GetCurTime(&tmCurRec);

				*pbData++ = DT_DATE_TIME_S;
				OoTimeToDateTimeS(&tmCurRec, pbData);pbData+=sizeof(TDTime);
				if (wOI == TERM_TERMPRG)	 //过台子直接赋值
					memcpy(pbData, pbData-sizeof(TDTime)-1, sizeof(TDTime)+1);
				else
					memset(pbData, 0x00, sizeof(TDTime)+1);	//包括数据类型
			}	
			return true;
		case 0x20200200:		//事件结束时间
			if (wFieldLen != 8)
				return false;
			if (bState == EVT_S_AFT_END)
			{
				if (wOI == TERM_TERMPRG)	//已经获取
					return true;
				else if (wOI==TERM_POWOFF )
				{
					//获取私有变量
					TPowOff* pCtrl = (TPowOff* )pEvtCtrl->pEvtPriv;
					if (pCtrl == NULL)
						return false;
					if ((pCtrl->bRptFlag==2) && (!IsInvalidTime(g_tPowerOn)))		//终端上电事件
						tmCurRec = g_tPowerOn;		//事件发生时间取最近一次上电时间
					else
						return true;
				}			
				else
					GetCurTime(&tmCurRec);

				*pbData++ = DT_DATE_TIME_S;
				OoTimeToDateTimeS(&tmCurRec, pbData);
			}
			return true;
		case 0x20240200:		//事件发生源
			//if (wOI == TERM_POWOFF)
			//{
			//	if (bState==EVT_S_AFT_HP || bState==EVT_S_AFT_END)
			//	{
			//		 if (GetEvtSrcData(pEvtCtrl, pbData, 1) < 0)	//直接拷贝
			//	 		return false;
			//	}
			//}
			//else
			//{
				if (bState == EVT_S_AFT_HP)
				{
					 if (GetEvtSrcData(pEvtCtrl, pbData, 1) < 0)	//直接拷贝
				 		return false;			
				}
			//}
			return true;
		case 0x33080206:		//超限期间正向有功需量最大值，发生时间也在此处理
			pDmd = (TDmd* )pEvtCtrl->pEvtPriv;
			if (pDmd == NULL)
				return false;
			if (wFieldLen != 5)
				return false;
			if (wFieldSize < 13)
				return false;
			if (bState >= EVT_S_AFT_HP)	
			{
				*pbData++ = DT_DB_LONG_U;
				OoDWordToDoubleLongUnsigned(pDmd->tDmd[bItem].dwDmdVal, pbData);pbData+=sizeof(DWORD);
				*pbData++ = DT_DATE_TIME_S;
				memcpy(pbData, (BYTE*)&pDmd->tDmd[bItem].tTime, sizeof(TDTime));
			}
			return true;
		case 0x33080207:		//超限期间需量最大值发生时间
			if (wFieldLen != 8)
				return false;
			return true;
		case 0x330C0206:		//事件清零列表
			if (bState == EVT_S_AFT_HP)
			{	
				TEvtClr* pEvtClr = (TEvtClr* )pEvtCtrl->pEvtPriv;
				if (pEvtClr == NULL)
					return false;
				memcpy(pbData, pEvtClr,sizeof(TEvtClr));
				memset((BYTE*)pEvtClr, 0x00, sizeof(TEvtClr));	//使用后清零
			}		
			return true;
		case 0x33000200:		//事件上报状态
			if (wFieldLen != CN_RPT_TOTAL_LEN)
				return false;
			if (bState == EVT_S_AFT_HP)
			{	
				memset(pbData, 0x00, CN_RPT_TOTAL_LEN);	//默认为空
				if (OoReadAttr(0x4300, 10, bBuf, NULL, NULL) > 0)	//上报通道
				{
					if (bBuf[1])
					{
						bCnNum = bBuf[1];
						if (bCnNum > CN_RPT_NUM)
							bCnNum = CN_RPT_NUM;
						*pbData = DT_ARRAY;
						*(pbData+1) = bCnNum;
						for (i=0; i<bCnNum; i++)	
						{
							memcpy(pbData+2+i*CN_RPT_STATE_LEN, g_bCnRptState,sizeof(g_bCnRptState));
							memcpy(pbData+5+i*CN_RPT_STATE_LEN, bBuf+3+5*i,sizeof(DWORD));
						}
					}
				}
			}	
			return true;
		case 0x20296200:		//安时值
			if (bState == EVT_S_BF_HP)
				return false;
			if (OoReadOad(dwFieldOAD, bBuf, wFieldLen, wFieldSize) == false)
				return false;
			if (bState == EVT_S_AFT_HP)
				memcpy(pbData, bBuf, wFieldLen);	
			else if (bState == EVT_S_AFT_END)
			{
				dwOldVal = OoDoubleLongUnsignedToDWord(pbData+1);
				dwNewVal = OoDoubleLongUnsignedToDWord(bBuf+1);
				if (dwOldVal < dwNewVal)
					dwNewVal -= dwOldVal;
				OoDWordToDoubleLongUnsigned(dwNewVal, pbData+1);	
			}
			return true;			
		case 0x20266200:		//不平衡率
		case 0x20276200:		//不平衡率	
			if (bState == EVT_S_BF_HP)
				return false;
			if (OoReadOad(dwFieldOAD, bBuf, wFieldLen, wFieldSize) == false)
				return false;
			if (bState == EVT_S_AFT_HP)
				memcpy(pbData, bBuf, wFieldLen);	
			else if ((bState==EVT_S_BF_END)||(bState==EVT_S_AFT_END))	//不平衡率取大值
			{
				wOldVal= OoLongUnsignedToWord(pbData+1);
				wNewVal= OoLongUnsignedToWord(bBuf+1);
				if (wOldVal < wNewVal)
					memcpy(pbData, bBuf, wFieldLen);	
			}
			return true;	
		case 0x33020206:		//编程对象列表  array OAD	
			if (bState == EVT_S_AFT_HP)
			{	
				TTermPrg* pEvtPriv = (TTermPrg* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				memcpy(pbData, pEvtPriv->bOAD, TERM_PRG_LIST_LEN);
				*pbData = DT_ARRAY;	//确保为ARRAY类型数据
				memset((BYTE*)pEvtPriv, 0x00, TERM_PRG_LIST_LEN);	//使用后清零
			}		
			return true;	

		case 0x33030206:		//搜表结果      array 一个搜表结果	
			if (bState == EVT_S_AFT_HP)
			{	
				TUnKnMtr* pEvtPriv = (TUnKnMtr* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				//张强，按pEvtPriv->bSaveFlag置起的位获取相数的搜表数据
				//组帧出array 一个搜表结果至pbData
				//*pbData++ = DT_ARRAY;…………………………
				//使用完后清零

				bSaveMtrNum = 0;
				*pbData++ = DT_ARRAY;

				pbData0 = pbData;	//记录数组元素个数指针位置
				pbData++;

				for (bByte=0; bByte<SCH_MTR_SAVE_LEN; bByte++)
				{
					if (pEvtPriv->bSaveFlag[bByte] == 0x00)	//此字节不需要记录
						continue;

					for (bBit=0; bBit<8; bBit++)
					{
						if ((pEvtPriv->bSaveFlag[bByte] & (1<<bBit)) != 0) //此序号搜到表了，需要记录
						{
							pEvtPriv->bSaveFlag[bByte] &= ~(0x01<<bBit);

							wIndex = bByte*8 + bBit;
							iRet = GetSchMtrEvtData(wIndex, pbData);	//
							if (iRet > 0)
							{
								pbData += iRet;
								bSaveMtrNum++;
							}							
						}

						if (bSaveMtrNum == SCH_MTR_SAVE_REC_NUM) //凑齐条数
						{
							*pbData0 = bSaveMtrNum;		//更新数组元素个数
							memset((BYTE*)pEvtPriv->bSaveFlag, 0x00, SCH_MTR_SAVE_LEN);
							return true;	
						}
					}	
				}

				*pbData0 = bSaveMtrNum;		//更新数组元素个数
				memset((BYTE*)pEvtPriv->bSaveFlag, 0x00, SCH_MTR_SAVE_LEN);
			}		
			return true;	
		case 0x33040206:		// 跨台区搜表结果  array  一个跨台区结果	
			if (bState == EVT_S_AFT_HP)
			{	
				TUnKnMtr* pEvtPriv = (TUnKnMtr* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				//张强，按pEvtPriv->bSaveFlag置起的位获取相数的搜表数据
				//组帧出array  一个跨台区结果至pbData
				//*pbData++ = DT_ARRAY;…………………………
				//使用完后清零
				memset((BYTE*)pEvtPriv->bSaveFlag, 0x00, SCH_MTR_SAVE_LEN);
			}		
			return true;	
		case 0x33050206:		//事件发生后2分钟功率  long64(单位：W，换算-1)
			if (bState == EVT_S_AFT_HP)
			{
				memset(pbData, 0x00, 9);
				*pbData++  = DT_LONG64;
			}
			else if (bState == EVT_S_BF_END)
			{	
				TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bHpAfPow[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bHpAfPow, 9);
				memset(pEvtPriv->bHpAfPow, 0x00, 9);//使用后清零
			}		
			return true;	
		case 0x33050207:		//控制对象  OI
			if (bState == EVT_S_AFT_HP)
			{	
				TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bCtrlOI[0] != DT_OI)
					return false;
				memcpy(pbData, pEvtPriv->bCtrlOI, 3);
				memset(pEvtPriv->bCtrlOI, 0x00, 3);//使用后清零
			}		
			return true;	
		case 0x33050208:		//跳闸轮次  bit-string(SIZE(8))
			if (bState == EVT_S_AFT_HP)
			{	
				TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bBreakCnt[0] != DT_BIT_STR)
					return false;
				memcpy(pbData, pEvtPriv->bBreakCnt, 3);
				memset(pEvtPriv->bBreakCnt, 0x00, 3);//使用后清零
			}		
			return true;	
		case 0x33050209:		//功控定值  long64（单位：kW，换算-4）
			if (bState == EVT_S_AFT_HP)
			{	
				TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bPowCtrlVal[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bPowCtrlVal, 9);
				memset(pEvtPriv->bPowCtrlVal, 0x00, 9);//使用后清零
			}		
			return true;	
		case 0x3305020A:		//跳闸发生前总加有功功率    long64（单位：kW，换算-4）
			if (bState == EVT_S_AFT_HP)
			{	
				TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bHpBfPow[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bHpBfPow, 9);
				memset(pEvtPriv->bHpBfPow, 0x00, 9);//使用后清零
			}		
			return true;	
		case 0x33060206:		//控制对象  OI
			if (bState == EVT_S_AFT_HP)
			{	
				TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bCtrlOI[0] != DT_OI)
					return false;
				memcpy(pbData, pEvtPriv->bCtrlOI, 3);
				memset(pEvtPriv->bCtrlOI, 0x00, 3);//使用后清零
			}		
			return true;	
		case 0x33060207:		//跳闸轮次  bit-string(SIZE(8))
			if (bState == EVT_S_AFT_HP)
			{	
				TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bBreakCnt[0] != DT_BIT_STR)
					return false;
				memcpy(pbData, pEvtPriv->bBreakCnt, 3);
				memset(pEvtPriv->bBreakCnt, 0x00, 3);//使用后清零
			}		
			return true;	
		case 0x33060208:		//功控定值  long64（单位：kW，换算-4）
			if (bState == EVT_S_AFT_HP)
			{	
				TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bEleCtrlVal[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bEleCtrlVal, 9);
				memset(pEvtPriv->bEleCtrlVal, 0x00, 9);//使用后清零
			}		
			return true;	
		case 0x33060209:		//跳闸发生前总加有功功率 long64（单位：kW，换算-4）
			if (bState == EVT_S_AFT_HP)
			{	
				TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bHpEng[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bHpEng, 9);
				memset(pEvtPriv->bHpEng, 0x00, 9);//使用后清零
			}		
			return true;	
		case 0x33070206:		//控制对象  OI
			if (bState == EVT_S_AFT_HP)
			{	
				TEleAlram* pEvtPriv = (TEleAlram* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bCtrlOI[0] != DT_OI)
					return false;
				memcpy(pbData, pEvtPriv->bCtrlOI, 3);
				memset(pEvtPriv->bCtrlOI, 0x00, 3);//使用后清零
			}		
			return true;	
		case 0x33070207:		//电控定值  long64（单位：kWh，换算-4）
			if (bState == EVT_S_AFT_HP)
			{	
				TEleAlram* pEvtPriv = (TEleAlram* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bEleAlrCtrlVal[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bEleAlrCtrlVal, 9);
				memset(pEvtPriv->bEleAlrCtrlVal, 0x00, 9);//使用后清零
			}		
			return true;	
		case 0x33090206:		//属性标志	bit-string（SIZE(8)）
			if ((bState == EVT_S_AFT_HP) || (bState == EVT_S_AFT_END))
			{	
				TPowOff* pEvtPriv = (TPowOff* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				*pbData++ = DT_BIT_STR;
				*pbData++ = 0x08;
				*pbData = pEvtPriv->bAttr;
				//pEvtPriv->bAttr = 0;	//使用后清零
			}		
			return true;	
		case 0x330A0206:		//控后2分钟总加组功率 array long64
			if (bState == EVT_S_AFT_HP)
			{
				memset(pbData, 0x00, 74);
				*pbData++ = DT_ARRAY;
			}
			else if (bState == EVT_S_AFT_END)
			{	
				TYKCtrl* pEvtPriv = (TYKCtrl* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bArrayPow[0] != DT_ARRAY)
					return false;
				memcpy(pbData, pEvtPriv->bArrayPow, 74);
				memset(pEvtPriv->bArrayPow, 0x00, 74);//使用后清零
			}		
			return true;	
		case 0x330B0206:		//有功总电能量差动越限事件记录单元∷越限时对比总加组有功总电能量 long64（单位：kWh，换算：-4），
			if (bState == EVT_S_AFT_HP)
			{	
				TEpOver* pEvtPriv = (TEpOver* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bCompEng[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bCompEng, 9);
				memset(pEvtPriv->bCompEng, 0x00, 9);//使用后清零
			}		
			return true;	
		case 0x330B0207:		//有功总电能量差动越限事件记录单元∷越限时参照总加组有功总电能量 long64（单位：kWh，换算：-4），
			if (bState == EVT_S_AFT_HP)
			{	
				TEpOver* pEvtPriv = (TEpOver* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bReferEng[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bReferEng, 9);
				memset(pEvtPriv->bReferEng, 0x00, 9);//使用后清零
			}		
			return true;	
		case 0x330B0208:		//有功总电能量差动越限事件记录单元∷越限时差动越限相对偏差值 integer（单位：%，换算：0）	
			if (bState == EVT_S_AFT_HP) 
			{	
				TEpOver* pEvtPriv = (TEpOver* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				memcpy(pbData, pEvtPriv->bRelaErr, 2);
				memset(pEvtPriv->bRelaErr, 0x00, 2);//使用后清零
			}		
			return true;			
		case 0x330B0209:		//有功总电能量差动越限事件记录单元∷越限时差动越限绝对偏差值 long64（单位：kWh，换算：-4）	
			if (bState == EVT_S_AFT_HP)
			{	
				TEpOver* pEvtPriv = (TEpOver* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				if (pEvtPriv->bAbsoErr[0] != DT_LONG64)
					return false;
				memcpy(pbData, pEvtPriv->bAbsoErr, 9);
				memset(pEvtPriv->bAbsoErr, 0x00, 9);//使用后清零
			}		
			return true;				
		case 0x330D0206:		//校时前时钟    date_time_s
			if (bState == EVT_S_AFT_HP) 
			{	
				TMtrClkPrg* pEvtPriv = (TMtrClkPrg* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				memcpy(pbData, pEvtPriv->bClock, 8);
				memset(pEvtPriv->bClock, 0x00, 8);//使用后清零
			}		
			return true;	
		case 0x330D0207:		//时钟误差      integer（单位：秒，无换算）
			if (bState == EVT_S_AFT_HP) 
			{	
				TMtrClkPrg* pEvtPriv = (TMtrClkPrg* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;
				memcpy(pbData, pEvtPriv->bClkErr, 2);
				memset(pEvtPriv->bClkErr, 0x00, 2);//使用后清零
			}		
			return true;
		/*case 0x40002200:	//校时前终端时间
			if (bState == EVT_S_AFT_HP)
			{
				TAdjTermTime* pEvtPriv = (TAdjTermTime* )pEvtCtrl->pEvtPriv;
				if (pEvtPriv == NULL)
					return false;

				memcpy(pbData, pEvtPriv->bClock, 8);
				memset(pEvtPriv->bClock, 0, 8);//使用后清零
			}
			return true;*/	//校时前数据由中间数据更新
		default:
			return false;
	}

	return false;
}



//描述：组成一条记录的数据字段内容用于存储
//参数：@ dwROAD 事件记录表OAD
//		@ pRecBuf 缓冲区
//		@ wBufSize 缓冲区的大小
//返回：正确获取到数据返回数据长度，否则返回负数
int EvtGetRecData(DWORD dwROAD, BYTE* pRecBuf, WORD wBufSize)
{
	BYTE bAttr, bItem, bType;
	BYTE bOadBuf[10];
	BYTE bAttrBuf[EVT_ATTRTAB_LEN];		//关联属性表OAD
	BYTE bMemBuf[EVTREC_MAXSIZE];
	BYTE* pbMem = bMemBuf;
 	WORD wOI, wItemOffset, wItemLen;
	DWORD dwOAD;
	TFieldParser tDataFields;
	TTermEvtCtrl* pEvtCtrl;
	BYTE bIndex;
	const TEvtAttr* pEvtAttr; 
	
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));
	GetOIAttrIndex(dwROAD, &wOI, &bAttr, NULL);
	
	//获取事件控制结构
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return -1;	

	//获取事件属性
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return -1;
	
	//获取事件分项号
	if (bAttr >= pEvtAttr->bRecTabStart)
		bItem = bAttr-pEvtAttr->bRecTabStart;
	else 
		return -1;

	//获取数据字段
	if (GetEvtFieldParser(pEvtCtrl, NULL, &tDataFields, bAttrBuf, sizeof(bAttrBuf)) == false)
		return -1;
	if (tDataFields.wNum == 0)	//关联属性表可以为空，返回0值表示无数据字段数据
		return 0;
	if(tDataFields.wTotalLen==0)	//无数据
		return -1;	
	if (wBufSize < tDataFields.wTotalLen)	//超范围了
		return -1;
	
	//读申请的临时空间全局变量数据至bMemBuf
	memset((BYTE*)&bMemBuf, 0x00, sizeof(bMemBuf));
	 if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTREC)	//申请了整笔临时记录的情况
	{	
		if (EvtReadRecMem(dwROAD, &g_TermMem, bMemBuf) <= 0)
			return -1;
	}
	else if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTITEM)	//申请了整笔临时记录的情况
	{	
		if (EvtReadItemMem(dwROAD, &g_TermMem, bMemBuf) <= 0)
			return -1;
	}

	//获取数据字段的数据
	for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
			return -1;
		if (bType != DT_OAD) 
			return -1;
		if (wItemLen == 0) 
			return -1;
		dwOAD = OoOadToDWord(bOadBuf+1);	
		if (IsOADNeedSaveData(dwOAD, pEvtCtrl->pEvtBase[bItem].bState))
		{
			if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_NONE)	//从系统库直接获取值
			{
				if (OoReadOad(dwOAD, bMemBuf, wItemLen, sizeof(bMemBuf)) == false)
				{
					pEvtCtrl->pEvtBase[bItem].fInitOk = false;	//需要重新初始化
					continue;
				}
				memcpy(pRecBuf, bMemBuf, wItemLen);
			}
			else if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTREC)
			{
				memcpy(pRecBuf, pbMem, wItemLen);
			}
			else if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTITEM)	//从临时空间获取值
			{
				if (EvtReadOneItem (dwOAD, bMemBuf, bMemBuf+100) <= 0)	//先读临时数据
				{
					if (OoReadOad(dwOAD, bMemBuf+100, wItemLen, sizeof(bMemBuf)) == false)	//读不出直接从系统库获取
					{
						pEvtCtrl->pEvtBase[bItem].fInitOk = false;	//需要重新初始化
						continue;
					}
				}
				memcpy(pRecBuf, bMemBuf+100, wItemLen);				
			}
			else
				return -1;		
		}
		if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTREC)
			pbMem += wItemLen;
		pRecBuf += wItemLen;
	}
	
	return tDataFields.wTotalLen;
}


//描述：滤波延时，更新状态机，维护pEvtCtrl->pEvtBase[bItem].bState
//参数：@pEvtCtrl 事件控制
//返回：无
void UpdateState(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bItem;
	WORD wDelaySec = 0;
	TEvtBase* pEvtBase;
	bool fUpDataFlag = false;
	TTime time;

	wDelaySec = pEvtCtrl->bDelaySec;

	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		pEvtBase = &pEvtCtrl->pEvtBase[bItem];
		if (!pEvtBase->fInitOk)	//未初始化
			continue;

		if (pEvtCtrl->pEvtBase[bItem].wTrigEvtDelaySec != 0)
			wDelaySec = pEvtCtrl->pEvtBase[bItem].wTrigEvtDelaySec;		//若是有触发类事件，修正延时时间

		//状态机的变化流程必须是如下：发生前--->发生后--->结束前--->结束后--->发生前
		if (pEvtBase->bState == EVT_S_BF_HP)
		{	
			if (pEvtBase->bJudgeState == EVT_JS_HP)	//发生前到发生需要滤波延时
			{
				pEvtBase->dwRecvClick = 0;
				if (wDelaySec == 0)
				{
					pEvtBase->dwEstClick = GetClick();
					if (!pEvtBase->fExcValid)   //新产生一条事件
					{
						pEvtBase->bState = EVT_S_AFT_HP;	
						pEvtBase->fExcValid = true;
						fUpDataFlag = true;
					}  
				}
				else
				{
					if(pEvtBase->dwEstClick == 0)
					{
						GetCurTime(&time);
						DTRACE(DB_METER_EXC, ("UpdateState-EVT_S_AFT_HP-s time= %02d %02d:%02d:%02d, pEvtCtrl->wOI=0x%02x, bItem=%d, pEvtBase->dwEstClick=%ld, GetClick()=%ld.\r\n", time.nDay, time.nHour, time.nMinute, time.nSecond, pEvtCtrl->wOI, bItem, pEvtBase->dwEstClick, GetClick()));	
						pEvtBase->dwEstClick = GetClick();
					}
					else
					{
						if (GetClick()-pEvtBase->dwEstClick < wDelaySec) 
							continue;
						if (!pEvtBase->fExcValid)   //新产生一条事件
						{
							GetCurTime(&time);
							DTRACE(DB_METER_EXC, ("UpdateState-EVT_S_AFT_HP-s time= %02d %02d:%02d:%02d, pEvtCtrl->wOI=0x%02x, bItem=%d, pEvtBase->dwEstClick=%ld, GetClick()=%ld.\r\n", time.nDay, time.nHour, time.nMinute, time.nSecond, pEvtCtrl->wOI, bItem, pEvtBase->dwEstClick, GetClick()));	
							pEvtBase->bState = EVT_S_AFT_HP;	
							pEvtBase->fExcValid = true;
							fUpDataFlag = true;
						}  
					}
				}
			}			
		}
		else if (pEvtBase->bState == EVT_S_AFT_HP)
		{	
			pEvtBase->bState = EVT_S_BF_END;
			fUpDataFlag = true;
			if (pEvtBase->bJudgeState == EVT_JS_FORCE_END)	//掉电等情况会引起强制结束
			{
				if (pEvtBase->fExcValid)
				{	
					pEvtBase->bState = EVT_S_AFT_END;	
					pEvtBase->fExcValid = false;
				}
			}
		}
		else if (pEvtBase->bState == EVT_S_BF_END)
		{
			if (pEvtBase->bJudgeState == EVT_JS_FORCE_END)	//掉电等情况会引起强制结束
			{
				if (pEvtBase->fExcValid)
				{	
					pEvtBase->bState = EVT_S_AFT_END;	
					pEvtBase->fExcValid = false;
					fUpDataFlag = true;
				}
			}
			else if (pEvtBase->bJudgeState == EVT_JS_END)	//正常运行中结束前到结束需要滤波延时
			{
				pEvtBase->dwEstClick = 0;
				if (wDelaySec == 0)
				{
					pEvtBase->dwRecvClick = GetClick();
					if (pEvtBase->fExcValid)  //一条事件恢复了
					{
						pEvtBase->bState = EVT_S_AFT_END;	
						pEvtBase->fExcValid = false;
						fUpDataFlag = true;
					}
				}
				else
				{
					if(pEvtBase->dwRecvClick == 0)
					{
						pEvtBase->dwRecvClick = GetClick();
						GetCurTime(&time);
						DTRACE(DB_METER_EXC, ("UpdateState-EVT_S_AFT_END-s time= %02d %02d:%02d:%02d, pEvtCtrl->wOI=0x%02x, bItem=%d, pEvtBase->dwEstClick=%ld, GetClick()=%ld.\r\n", time.nDay, time.nHour, time.nMinute, time.nSecond, pEvtCtrl->wOI, bItem, pEvtBase->dwEstClick, GetClick()));	
					}
					else
					{
						if (GetClick()-pEvtBase->dwRecvClick < wDelaySec) 
							continue;

						if (pEvtBase->fExcValid)  //一条事件恢复了
						{
							GetCurTime(&time);
							DTRACE(DB_METER_EXC, ("UpdateState-EVT_S_AFT_END-s time= %02d %02d:%02d:%02d, pEvtCtrl->wOI=0x%02x, bItem=%d, pEvtBase->dwEstClick=%ld, GetClick()=%ld.\r\n", time.nDay, time.nHour, time.nMinute, time.nSecond, pEvtCtrl->wOI, bItem, pEvtBase->dwEstClick, GetClick()));	
							pEvtBase->bState = EVT_S_AFT_END;	
							pEvtBase->fExcValid = false;
							fUpDataFlag = true;
						}
					}
				}
			}
		}
		else if (pEvtBase->bState == EVT_S_AFT_END)
		{	
			pEvtBase->bState = EVT_S_BF_HP;
			fUpDataFlag = true;
		}
		else
		{
			pEvtBase->fInitOk = false;	//数据不正确重新初始化。
		}
	}

	 if (fUpDataFlag)
	{	
		UpdateRecMem(pEvtCtrl, 1);	//状态刚刚切换时强制刷新数据，后续按周期刷新
		UpdateItemMem(pEvtCtrl, 1);
	}
	
	return;
}

//描述：申请的整笔记录空间的处理，有发生/结束时立即处理，结束前每2秒更新一次
//参数：@pEvtCtrl 事件控制
//	@bSaveType 强制刷新数据
//返回：无
void UpdateRecMem(struct TEvtCtrl* pEvtCtrl, BYTE bSaveType)
{
	BYTE bAttrTab, bType, bItem, bIndex;
	BYTE bBuf[EVT_ATTRTAB_LEN];						//关联属性表OAD
	BYTE bOadBuf[10] = {0};				//关联属性表获取到的OAD数据BUF
	BYTE bRecMemBuf[EVTREC_MAXSIZE];	//用读/写临时空间的缓冲区
	BYTE* pbRec = bRecMemBuf;
	WORD wItemOffset, wItemLen;
	DWORD dwROAD, dwOAD;
	int iLen;
	TFieldParser tDataFields;
	const TEvtAttr* pEvtAttr; 
	TEvtBase* pEvtBase;

	//获取数据字段等
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return;
	bAttrTab = pEvtAttr->bRecTabStart;
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));
	if (GetEvtFieldParser(pEvtCtrl, NULL, &tDataFields, bBuf, sizeof(bBuf)) == false)
		return;
	if ((tDataFields.wCfgLen==0) ||(tDataFields.wTotalLen==0) ||(tDataFields.wNum==0)) 
		return;

	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		pbRec = bRecMemBuf;
		pEvtBase = &pEvtCtrl->pEvtBase[bItem];
		if (!pEvtBase->fInitOk)	//未初始化
			{bAttrTab++;continue;}
		if (pEvtBase->bMemType != MEM_TYPE_TERM_EVTREC)	//未申请临时空间直接退出
			{bAttrTab++;continue;}

		if ((bSaveType == 1) || (pEvtCtrl->dwNewClick -pEvtCtrl->dwLastClick > EVT_UPDATE_CYC))
		{
			//获取记录表OAD
			dwROAD = GetOAD(pEvtCtrl->wOI, bAttrTab, 0);	

			//Step :	先读出旧值
			if (EvtReadRecMem(dwROAD, &g_TermMem, bRecMemBuf) <= 0)
			{
				pEvtBase->fInitOk = false;	
				{bAttrTab++;continue;}
			}
			
			//Step :	bRecMemBuf按条件重新取值
			for(bIndex=0; bIndex<tDataFields.wNum; bIndex++)
			{
				memset(bOadBuf, 0, sizeof(bOadBuf));
				if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
				{	
					pEvtBase->fInitOk = false;	
					return;	//直接退出
				}	
				if ((bType!=DT_OAD) || (wItemLen==0))
				{	
					pEvtBase->fInitOk = false;
					return;
				}	
				dwOAD = OoOadToDWord(bOadBuf+1);
				if (IsOADNeedAcqData(dwOAD, pEvtBase->bState))
				{
					if (MakeEvtSpecField(dwROAD, dwOAD, pbRec, wItemLen, sizeof(bRecMemBuf)) == false)
					{	
						if (OoReadOad(dwOAD, pbRec, wItemLen, sizeof(bRecMemBuf)) == false)
						{	
							pEvtBase->fInitOk = false;	
							return;
						}
					}
				}
				pbRec += wItemLen;
			}

			//Step :	获取完所有数据之后刷新新值
			iLen = EvtWriteRecMem(dwROAD, &g_TermMem, bRecMemBuf);
			if ((iLen<=0) || (iLen!= tDataFields.wTotalLen))
			{
				pEvtBase->fInitOk = false;	//需要重新初始化
				{bAttrTab++;continue;}
			}	
		}

		//下一个记录表
		bAttrTab++;
	}
	
	return;
}


//描述：申请的临时空间的处理，每2秒更新一次
//	@bSaveType 强制刷新数据
void UpdateItemMem(struct TEvtCtrl* pEvtCtrl, BYTE bSaveType)
{
	BYTE bAttrTab, bItemLen,bItem, bIndex;
	BYTE bItemMemBuf[MEMORY_BLOCK_SIZE] ;	//用读/写临时空间的缓冲区
	BYTE bBuf[SYSDB_ITEM_MAXSIZE];			//系统库读出的数据
	DWORD dwROAD, dwOAD;
	int iLen;
	const TEvtAttr* pEvtAttr; 
	TEvtBase* pEvtBase;

	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return;
	bAttrTab = pEvtAttr->bRecTabStart;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		pEvtBase = &pEvtCtrl->pEvtBase[bItem];
		if (!pEvtBase->fInitOk)	//未初始化
			{bAttrTab++;continue;}
		if (pEvtBase->bMemType != MEM_TYPE_TERM_EVTITEM)	//未申请临时空间直接退出
			{bAttrTab++;continue;}

		if ((bSaveType == 1) || (pEvtCtrl->dwNewClick -pEvtCtrl->dwLastClick > EVT_UPDATE_CYC))
		{
			//获取记录表OAD
			dwROAD = GetOAD(pEvtCtrl->wOI, bAttrTab, 0);	

			//Step :	先读出旧值
			if (EvtReadItemMem(dwROAD, &g_TermMem, bItemMemBuf) <= 0)
			{
				pEvtBase->fInitOk = false;	//需要重新初始化
				{bAttrTab++;continue;}
			}
			if (bItemMemBuf[0] == 0)	//OAD个数为0
			{
				pEvtBase->fInitOk = false;	
				{bAttrTab++;continue;}
			}

			//Step : 按条件重新取值，获取到新值的要直接刷新
			for(bIndex=0; bIndex<bItemMemBuf[0]; bIndex++)
			{
				dwOAD = OoOadToDWord(bItemMemBuf+1+bIndex*5);
				bItemLen = *(bItemMemBuf+5+bIndex*5);
				if (IsEvtBeforeOAD(dwOAD) == false)
				{
					pEvtBase->fInitOk = false;	
					return;
				}
				if (IsOADNeedAcqData(dwOAD, pEvtBase->bState))
				{
					if (EvtReadOneItem(dwOAD, bItemMemBuf, bBuf) != bItemLen)	//读出旧值
					{
						pEvtBase->fInitOk = false;	
						return;
					}
					if (MakeEvtSpecField(dwROAD, dwOAD, bBuf, (WORD)bItemLen, sizeof(bBuf)) == false)
					{	
						if (OoReadOad(dwOAD, bBuf, (WORD)bItemLen, sizeof(bBuf)) == false)	
						{
							pEvtBase->fInitOk = false;	
							return;
						}
					}
					EvtWriteOneItem(dwOAD, bItemMemBuf, bBuf);
				}
			}
			if (EvtWriteItemMem(dwROAD, &g_TermMem, bItemMemBuf) <= 0)
				return;
		}

		//下一个记录表
		bAttrTab++;
	}

	return;
}


//描述:	处理事件系统库中DYN属性数据，包括记录数属性、当前值记录表属性
bool UpdateEvtStaData(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bClass, bItem, bItemNo, bState, bOffset;
	BYTE bBuf[EVT_ATRR_MAXLEN];
	WORD wOI, wMaxNum, wCurRecNum;
	DWORD dwTimes, dwClick, dwSec;
	int iLen;
	TTime tmCurRec;
	TEvtBase* pEvtBase;
	const TEvtAttr* pEvtAttr; 
	BYTE bCurRecList[30] = {0};

	wOI = pEvtCtrl->wOI;
	bClass = pEvtCtrl->bClass;
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return false;

	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		pEvtBase = &pEvtCtrl->pEvtBase[bItem];
		if (!pEvtBase->fInitOk)	//未初始化
			continue;
		bState = pEvtBase->bState;
		bItemNo = bItem;
		if (pEvtCtrl->bItemNum == 3)
			bItemNo++;	//总ABC，略过总

		//事件发生时累计次数
		if (bState == EVT_S_AFT_HP)
		{
			//最大记录数
			if (OoReadAttr(wOI, pEvtAttr->bMaxRecNum, bBuf, NULL, NULL) > 0)
				wMaxNum = OoLongUnsignedToWord(bBuf+1);	
			else
				wMaxNum = 0;
			if (wMaxNum == 0)
			{
				pEvtBase->fInitOk = false;
				return false;
			}

			//当前记录数累加
			iLen = OoReadAttr(wOI, pEvtAttr->bCurRecNum, bBuf, NULL, NULL);
			if (pEvtCtrl->bClass == IC7)
			{
				if (iLen <= 0)	
					memcpy(bBuf, g_bIC7CurNum, sizeof(g_bIC7CurNum));
				bOffset = 1;
			}
			else if (bClass == IC24)
			{
				if (iLen <= 0)	
					memcpy(bBuf, g_bIC24CurNum, sizeof(g_bIC24CurNum));
				bOffset = 3+bItemNo*3;
			}
			else
				return false;
			wCurRecNum = OoLongUnsignedToWord(bBuf+bOffset);
			if (wCurRecNum < wMaxNum)	//当前记录数加1，直到记录数等于最大记录数
			{
				wCurRecNum++;
				OoWordToLongUnsigned(wCurRecNum, bBuf+bOffset);
				OoWriteAttr(wOI, pEvtAttr->bCurRecNum, bBuf);
			}
	
			//当前值记录表累计次数累加，IC7获取事件发生源
			iLen =OoReadAttr(wOI, pEvtAttr->bCurVal, bBuf, NULL, NULL);
			if (pEvtCtrl->bClass == IC7)
			{
				//当前值记录表读取有错时修正
				if (iLen <= 0)
				{	
					iLen = EvtSrctoCurRecList(pEvtCtrl, bCurRecList);
					if (iLen > 0)
						memcpy(bBuf, bCurRecList, iLen);
					else
						return false;
				}
				//获取事件发生源
				iLen =  GetEvtSrcData(pEvtCtrl, bBuf+4, 2);	//需要移数据
				if (iLen <= 0)
					return false;
				//累计次数偏移
				bOffset = 7+iLen;	
			}
			else if (bClass == IC24)
			{
				if (iLen <= 0)	
					memcpy(bBuf, g_bIC24CurRecList, sizeof(g_bIC24CurRecList));
				bOffset = 5+bItemNo*12;
			}
			else
				return false;	
			dwTimes = OoDoubleLongUnsignedToDWord(bBuf+bOffset);
			dwTimes++;
			OoDWordToDoubleLongUnsigned(dwTimes, bBuf+bOffset);
			OoWriteAttr(wOI, pEvtAttr->bCurVal, bBuf);
			pEvtBase->dwStaClick = GetClick();
			if (pEvtCtrl->wOI == MTR_VLOSS)	//放这里使统计数据更准确
			{
				TVLoss* pVLoss = (TVLoss* )pEvtCtrl->pEvtPriv;
				if (pVLoss->dwVLossStaClick == 0)	//还未开始统计时
					pVLoss->dwVLossStaClick = pEvtBase->dwStaClick;
			}
		}		
	}	

	//累计时间每2秒更新一次
	if (pEvtCtrl->dwNewClick -pEvtCtrl->dwLastClick <= EVT_UPDATE_CYC)
		return true;
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		pEvtBase = &pEvtCtrl->pEvtBase[bItem];
		if (!pEvtBase->fInitOk)	//未初始化
			continue;
		bState = pEvtBase->bState;
		bItemNo = bItem;
		if (pEvtCtrl->bItemNum == 3)
			bItemNo++;	//总ABC，略过总

		//事件发生期间累计时间
		if ((bState==EVT_S_BF_END) || (bState==EVT_S_AFT_END))
		{
			iLen =OoReadAttr(wOI, pEvtAttr->bCurVal, bBuf, NULL, NULL);
			if (iLen <= 0)	//统计之前必有数据了
			{
				pEvtBase->fInitOk = false;
				return false;
			}
			if (pEvtCtrl->bClass == IC7)
			{
				//获取事件发生源长度
				iLen =  GetEvtSrcData(pEvtCtrl, bBuf+4, 0);	//只获取长度
				if (iLen <= 0)
					return false;
				//累计次数偏移
				bOffset = 12+iLen;	
			}
			else if (bClass == IC24)
				bOffset = 10+bItemNo*12;
			else
				return false;	
			dwClick =pEvtCtrl->dwNewClick;
			dwSec = OoDoubleLongUnsignedToDWord(bBuf+bOffset);
			if ((pEvtBase->dwStaClick) && ((dwClick-pEvtBase->dwStaClick)>0))	
			{	
				if (dwClick > pEvtBase->dwStaClick)
					dwSec += dwClick-pEvtBase->dwStaClick;
				OoDWordToDoubleLongUnsigned(dwSec, bBuf+bOffset);
				OoWriteAttr(wOI, pEvtAttr->bCurVal, bBuf);		
				pEvtBase->dwStaClick = dwClick;
			}
			if (bState==EVT_S_AFT_END)
				pEvtBase->dwStaClick = 0;
		}	
	}	
	
	return true; 
}

//描述：失压事件私有函数
void UpdateVLossPriv(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bItem, bIsVLoss=0, bBuf[EVT_ATRR_MAXLEN];
	DWORD dwCnt, dwSec, dwNewClick =pEvtCtrl->dwNewClick;
	TTime tmCurRec;
	TVLoss* pVLoss = (TVLoss* )pEvtCtrl->pEvtPriv;
	if (pVLoss == NULL)
		return;

	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)	//未初始化
			continue;

		//私有变量实时维护
		if (pEvtCtrl->pEvtBase[bItem].bState == EVT_S_AFT_HP) 	//有新事件产生
		{	
			//if (pVLoss->dwVLossStaClick == 0)	//还未开始统计时
			//	pVLoss->dwVLossStaClick = GetClick();
			
			memset(bBuf, 0x00, sizeof(bBuf));
			if (OoReadAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, bBuf, NULL, NULL) <= 0)
				memcpy(bBuf, g_bAllVLossSta, sizeof(g_bAllVLossSta));
			dwCnt = OoDoubleLongUnsignedToDWord(bBuf+3);
			dwCnt++;
			OoDWordToDoubleLongUnsigned(dwCnt, bBuf+3);
			GetCurTime(&tmCurRec);
			OoTimeToDateTimeS(&tmCurRec, bBuf+13);
			OoWriteAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, bBuf);
			bIsVLoss = 1;			
		}		
		else if (pEvtCtrl->pEvtBase[bItem].bState == EVT_S_BF_END)
			bIsVLoss = 1;
	}	

	if (pVLoss->dwVLossStaClick == 0)
		return;

	//发生或结束需要统计，累计时间每2秒统计一次
	if ((dwNewClick -pEvtCtrl->dwLastClick >EVT_UPDATE_CYC) && (dwNewClick-pVLoss->dwVLossStaClick>0))
	{		
		memset(bBuf, 0x00, sizeof(bBuf));
		if (OoReadAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, bBuf, NULL, NULL) <= 0)
			memcpy(bBuf, g_bAllVLossSta, sizeof(g_bAllVLossSta));
		dwSec = OoDoubleLongUnsignedToDWord(bBuf+8);
		if (dwNewClick > pVLoss->dwVLossStaClick)
			dwSec += dwNewClick-pVLoss->dwVLossStaClick;
		OoDWordToDoubleLongUnsigned(dwSec, bBuf+8);
		pVLoss->dwVLossStaClick = dwNewClick;
		OoWriteAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, bBuf);			
	}
	
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (pEvtCtrl->pEvtBase[bItem].bState == EVT_S_AFT_END)	//事件结束
		{
			memset(bBuf, 0x00, sizeof(bBuf));
			if (OoReadAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, bBuf, NULL, NULL) <= 0)
				memcpy(bBuf, g_bAllVLossSta, sizeof(g_bAllVLossSta));
			GetCurTime(&tmCurRec);
			OoTimeToDateTimeS(&tmCurRec, bBuf+21);		
			OoWriteAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, bBuf);	
			if (bIsVLoss == 0)	pVLoss->dwVLossStaClick = 0;
		}
	}
}

//描述：需量事件私有函数
//超限期间正向有功需量最大值  double-long-unsigned，
//超限期间需量最大值发生时间  date_time_s，
void UpdateDmdPriv(struct TEvtCtrl* pEvtCtrl)
{
	TDmd* pDmd = (TDmd* )pEvtCtrl->pEvtPriv;
	int iLen;
	BYTE bItem, bBuf[200] = {0};
	WORD wDmdID0, wDmdID;
	DWORD dwDmd[4] = {0};
	TTime tmCurRec;

	if (pDmd == NULL)
		return;
	
	if (pEvtCtrl->wOI == MTR_PDMDOVER)
		wDmdID0 = 0x3010;
	else if (pEvtCtrl->wOI == MTR_RPDMDOVER)
		wDmdID0 = 0x3020;
	else if (pEvtCtrl->wOI == MTR_QDMDOVER)
		wDmdID0 = 0x3030;
	else
		return;

	wDmdID = wDmdID0;
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk) //未初始化
			{wDmdID++;continue;}
		if (pEvtCtrl->pEvtBase[bItem].bState == EVT_S_BF_HP)	//未发生事件不需要刷新
		{	
			if (pDmd->tDmd[bItem].dwDmdVal != 0)
				pDmd->tDmd[bItem].dwDmdVal = 0;
			{wDmdID++;continue;}
		}
		else if (pEvtCtrl->pEvtBase[bItem].bState==EVT_S_AFT_HP) //刷新新值
		{
			if (ReadItemEx(BN2, PN0, wDmdID, bBuf) > 0)
				dwDmd[bItem] = OoDoubleLongUnsignedToDWord(bBuf+1);
			else
				dwDmd[bItem] = 0;
			
			pDmd->tDmd[bItem].dwDmdVal = dwDmd[bItem];
			//获取当前时间
			GetCurTime(&tmCurRec);
			OoTimeToDateTimeS(&tmCurRec, (BYTE*)&pDmd->tDmd[bItem].tTime);
		}
		wDmdID++;
	}

	//每1秒更新一次
	if (pEvtCtrl->dwNewClick -pEvtCtrl->dwLastClick <= EVT_UPDATE_CYC)
		return;

	wDmdID = wDmdID0;
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk) //未初始化
			{wDmdID++;continue;}
		if (pEvtCtrl->pEvtBase[bItem].bState <= EVT_S_AFT_HP)	//未发生事件不需要刷新
			{wDmdID++;continue;}

		if (ReadItemEx(BN2, PN0, wDmdID, bBuf) > 0)
			dwDmd[bItem] = OoDoubleLongUnsignedToDWord(bBuf+1);
		else
			dwDmd[bItem] = 0;

		//刷新新值
		if (pDmd->tDmd[bItem].dwDmdVal<dwDmd[bItem]) 
		{
			pDmd->tDmd[bItem].dwDmdVal = dwDmd[bItem];
			//获取当前时间
			GetCurTime(&tmCurRec);
			OoTimeToDateTimeS(&tmCurRec, (BYTE*)&pDmd->tDmd[bItem].tTime);
		}
		wDmdID++;
	}
}

//描述：全失压事件私有函数
//掉电变量及时刷新
void UpdateAVLossPriv(struct TEvtCtrl* pEvtCtrl)
{	
	TAllVLoss* pAVLoss = (TAllVLoss* )pEvtCtrl->pEvtPriv;
	if (pAVLoss == NULL)
		return;
	memcpy((BYTE*)pAVLoss, pEvtCtrl->pEvtBase, sizeof(TAllVLoss));
}


//描述：根据事件的状态，保存记录到临时记录区或任务库
//参数：@pEvtCtrl事件控制
//返回：如果正确则返回true，否则返回false
bool SaveTermEvtRec(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bAttrTab, bRptFlag, bItem, bState, bIndex, bType, bOffset, bSendRptFlag = EVT_STAGE_UNCARE;
	BYTE bOadBuf[10];
	BYTE bBuf[EVT_ATTRTAB_LEN];	//关联属性表信缓冲区
	BYTE bRecBuf[EVTREC_MAXSIZE];	//一条记录缓冲区
	BYTE pvDataBuf[20];				//附加缓冲区
	BYTE* pbRec = bRecBuf;
	WORD wOI, wItemOffset, wItemLen, wRecIdx;
	int iLen;
	const ToaMap* pOaMap = NULL;
	TTime tmCurRec;
	TFieldParser tFixFields;
	TFieldParser tDataFields;
	const TEvtAttr* pEvtAttr; 
	BYTE i, bCnNum;
	DWORD dwROAD, dwOAD, dwCnOAD[CN_RPT_NUM] = {0};
	
	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));

	//获取事件类属性
	wOI = pEvtCtrl->wOI;
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return false;
	bAttrTab = pEvtAttr->bRecTabStart;

	//上报标识，不上报（0），事件发生上报（1），事件恢复上报（2），事件发生恢复均上报（3）
	if (OoReadAttr(pEvtCtrl->wOI,  pEvtAttr->bRepFlg, bBuf, NULL, NULL) > 0)
		bRptFlag = bBuf[1];
	else
		bRptFlag = 0;

	//获取固定字段和数据字段
	if (GetEvtFieldParser(pEvtCtrl, &tFixFields, &tDataFields, bBuf, sizeof(bBuf)) == false)
	{	
		DTRACE(DB_INMTR, ("SaveTermEvtRec: wOI=%u GetEvtFieldParser() fail.\r\n", wOI));
		return false;
	}
		
	//记录
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		bSendRptFlag = EVT_STAGE_UNCARE;
		pbRec = bRecBuf;	
		bState = pEvtCtrl->pEvtBase[bItem].bState;
		if ((bState!=EVT_S_AFT_HP) && (bState!=EVT_S_AFT_END))	//只有发生和结束时才需要记录
			{bAttrTab++;continue;}
		
		dwROAD = GetOAD(wOI, bAttrTab, 0);
		pOaMap = GetOIMap(dwROAD);
		if (pOaMap==NULL || pOaMap->pszTableName==NULL)
		{
			DTRACE(DB_TASK, ("SaveTermEvtRec: Read dwOA :%x failed Or Tdb Table is null!!\r\n", dwROAD));
			{bAttrTab++;continue;}
		}
		//先读出上1笔记录
		memset(bRecBuf, 0, sizeof(bRecBuf));
		iLen = ReadLastNRec(pOaMap->pszTableName, LAST_REC, bRecBuf, sizeof(bRecBuf));
		if (iLen <= 0)
		{	
			if (bState == EVT_S_AFT_END)	//若当前状态为恢复但读不出上1笔记录直接返回false
				return false;
			memset(bRecBuf, 0x00, sizeof(bRecBuf));
		}
		else
		{
			if (bState == EVT_S_AFT_HP)
				memset(bRecBuf+5, 0x00, sizeof(bRecBuf)-5);	//新产生一条记录需要先清除了记录序号外的数据
		}

		//处理固定字段
		for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	
		{
			memset(bOadBuf, 0, sizeof(bOadBuf));
			if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
			{	
				pEvtCtrl->pEvtBase[bItem].fInitOk = false;	//需要重新初始化
				return false;	//直接返回，固定字段不应答有问题
			}	
			if ((bType!=DT_OAD) || (wItemLen==0))
			{	
				pEvtCtrl->pEvtBase[bItem].fInitOk = false;	//需要重新初始化
				return false;
			}		
			dwOAD = OoOadToDWord(bOadBuf+1);
			if (MakeEvtSpecField(dwROAD, dwOAD, pbRec, wItemLen, sizeof(bRecBuf)) == false)
			{	
				pEvtCtrl->pEvtBase[bItem].fInitOk = false;	//需要重新初始化
				return false;
			}	
			if (dwOAD == 0x33000200)	//处理上报消息
			{
				bCnNum = *(pbRec+1);
				if (bCnNum > CN_RPT_NUM)
					bCnNum = CN_RPT_NUM;
				for(i=0; i<bCnNum; i++)
					dwCnOAD[i] = OoOadToDWord(pbRec+5+CN_RPT_STATE_LEN*i);
			}
			pbRec += wItemLen;	
		}
	
		//处理数据字段，关联属性表可以为空
		if (tDataFields.wNum != 0)
		{
			iLen = sizeof(bRecBuf)-(pbRec - bRecBuf);
			if (iLen <= 0)
				return false;
			iLen = EvtGetRecData(dwROAD, pbRec, iLen);
			if (iLen <= 0)
				return false;
		}

		//保存记录到任务库
		if (bState == EVT_S_AFT_HP)
		{	
			//DTRACE(DB_FAPROTO, ("SaveTermEvtRec: wOI = %04x evt happen!\r\n", wOI));
			SaveRecord(pOaMap->pszTableName, bRecBuf);	
			//TrigerSaveBank(BN0, SECT3, -1);
			//TrigerSaveBank(BN0, SECT16, -1);
		}
		else if (bState == EVT_S_AFT_END)
		{	
			DTRACE(DB_FAPROTO, ("SaveTermEvtRec: wOI = %04x evt reover!\r\n", wOI));
			SaveHisRecord(pOaMap->pszTableName, 1, bRecBuf);
			memset(bRecBuf, 0, sizeof(bRecBuf));
			if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTREC)
				EvtWriteRecMem(dwROAD, &g_TermMem, bRecBuf);
			else if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTITEM)
			{
				EvtReadItemMem(dwROAD, &g_TermMem, bRecBuf);
				bOffset = 1+5*bRecBuf[0];
				memset(bRecBuf+bOffset, 0, sizeof(bRecBuf)-bOffset);
				EvtWriteItemMem(dwROAD, &g_TermMem, bRecBuf);
			}
			//TrigerSaveBank(BN0, SECT3, -1);
			//TrigerSaveBank(BN0, SECT16, -1);
		}
		else 
			return false;
		
		//发送上报消息		
		if ((bState==EVT_S_AFT_HP) && ((bRptFlag&0x01)==0x01))
			bSendRptFlag = EVT_STAGE_HP;
		else if ((bState==EVT_S_AFT_END) && ((bRptFlag&0x02)==0x02))
			bSendRptFlag = EVT_STAGE_END;

		if  (wOI == TERM_POWOFF)	//停上电事件需根据事件上报标志来判断是否上报
		{
			TPowOff* pEvtPriv = (TPowOff* )pEvtCtrl->pEvtPriv;
			if (pEvtPriv != NULL)
			{
				if (pEvtPriv->bRptFlag==1 && bSendRptFlag==EVT_STAGE_HP)		//上报发生
					bSendRptFlag = EVT_STAGE_HP;
				else if (pEvtPriv->bRptFlag==2 && bSendRptFlag==EVT_STAGE_END)	//上报恢复
					bSendRptFlag = EVT_STAGE_END;
				else
					bSendRptFlag = EVT_STAGE_UNCARE;

				DTRACE(DB_FAPROTO, ("SaveTermEvtRec: wOI = %04x,  bSendRptFlag=%d, pEvtPriv->bRptFlag=%d.\r\n", wOI, bSendRptFlag, pEvtPriv->bRptFlag));
			}
		}

		if ((bState==EVT_S_AFT_HP) || (bState==EVT_S_AFT_END))
			AddEvtOad(dwROAD, bSendRptFlag);

		DTRACE(DB_FAPROTO, ("SaveTermEvtRec: wOI = %04x,  bSendRptFlag=%d.\r\n", wOI, bSendRptFlag));
		if (bSendRptFlag != EVT_STAGE_UNCARE)	//需要上报
		{
			wRecIdx = GetRecPhyIdx(pOaMap->pszTableName, 1);
			if (wRecIdx >= 0)
			{	
				for(i=0; i<bCnNum; i++)
					SendEvtMsg(dwCnOAD[i], dwROAD, wRecIdx, bSendRptFlag);
			}	
		}
		bAttrTab++;
	}

	return true;
}
/*bool DoTrigeEvt(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bItem, bItemNo, bOffse = 0;
	BYTE bBuf[EVT_TRIG_PARA_LEN] = {0};
	int iLen;
	bool fParaChgFlag = false;
	WORD wPn, wHpDelaySec, wEndDelaySec;
	TEvtBase* pEvtBase;
	TTime tmCurRec;
	
	if (GetEvtSn(pEvtCtrl->wOI) >= 0)
		wPn = GetEvtSn(pEvtCtrl->wOI);
	else
		return false;
	
	if (ReadItemEx(BN11, wPn, EVT_TRIG_ID, bBuf) <= 0)
		return false;
	if (IsAllAByte(bBuf, 0x00, sizeof(bBuf)))
		return false;	

	if (pEvtCtrl->bClass == IC7)
	{
		pEvtBase = &pEvtCtrl->pEvtBase[0];
		//触发一次记录（事件发生源，触发延时时间，恢复延时时间）
		//事件发生源∷=instance-specific
		//触发延时时间∷=long-unsigned 
		//恢复延时时间∷=long-unsigned
		iLen =  GetEvtSrcData(pEvtCtrl, bBuf, 0);	//只获取长度 
		if (iLen <= 0)
		{	
			memset(bBuf, 0x00, sizeof(bBuf));
			fParaChgFlag = true;
			goto End;
		}
		//判断数据是否有效
		if ( (bBuf[0]!=pEvtCtrl->pbSrcFmt[0]) 
			|| ((bBuf[iLen]!=DT_LONG_U) && (bBuf[iLen]!=0xfe) && (bBuf[iLen]!=0xff))
			|| (bBuf[iLen+3]!=DT_LONG_U))	
		{	
			memset(bBuf, 0x00, sizeof(bBuf));
			fParaChgFlag = true;
			goto End;
		}
		//触发延时时间
		if (bBuf[iLen] == DT_LONG_U)
		{	
			pEvtBase->bJudgeState = 0;	//先强制结束事件
			bBuf[iLen] = 0xfe;						//状态改为可以开始触发事件了
			fParaChgFlag = true;
		}
		else if (bBuf[iLen] == 0xfe)
		{
			pEvtBase->bJudgeState = 1;	//事件发生
			pEvtBase->wTrigEvtDelaySec = OoLongUnsignedToWord(&bBuf[iLen+1]);	//获取触发延时时间
			if (pEvtBase->wTrigEvtDelaySec == 0)
				pEvtBase->wTrigEvtDelaySec  = 0x02;
			if (iLen != 1)	//IC7事件发生源的处理
			{
				if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcOADFmt[0])
				{	
					if (pEvtCtrl->wOI == TERM_YKCTRLBREAK)
					{	
						TYKCtrl* pEvtPriv = (TYKCtrl* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						memset(pEvtPriv, 0x00, sizeof(TYKCtrl));
						memcpy(pEvtPriv->bEvtSrcOAD, bBuf, 5);
						pEvtPriv->bArrayPow[0] = DT_ARRAY;						
					}
				}
				else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcEnumFmt[0])
				{	
					if (pEvtCtrl->wOI == TERM_DEVICEERR)
					{	
						TDeviceErr* pEvtPriv = (TDeviceErr* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						 pEvtPriv->bEvtSrcEnum = bBuf[1];
					}
					else if (pEvtCtrl->wOI == TERM_CURCIRC)
					{
						TCurCirc* pEvtPriv = (TCurCirc* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						 pEvtPriv->bEvtSrcEnum = bBuf[1];
					}
					else if (pEvtCtrl->wOI == TERM_POWOFF)
					{
						TPowOff* pEvtPriv = (TPowOff* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						pEvtPriv->bEvtSrcEnum = bBuf[1];
					}
				}
				else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcOIFmt[0])
				{
					if (pEvtCtrl->wOI == TERM_POWCTRLBREAK)
					{	
						TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						memset(pEvtPriv, 0x00, sizeof(TPowCtrl));
						memcpy(pEvtPriv->bEvtSrcOI, bBuf, 3);
						pEvtPriv->bHpAfPow[0] = DT_LONG64;
						pEvtPriv->bCtrlOI[0] = DT_OI;
						pEvtPriv->bBreakCnt[0] = DT_BIT_STR;
						pEvtPriv->bPowCtrlVal[0] = DT_LONG64;
						pEvtPriv->bHpAfPow[0] = DT_LONG64;						
					}
					else if (pEvtCtrl->wOI == TERM_ELECTRLBREAK)
					{
						TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						memset(pEvtPriv, 0x00, sizeof(TEleCtrl));
						memcpy(pEvtPriv->bEvtSrcOI, bBuf, 3);
						pEvtPriv->bCtrlOI[0] = DT_OI;
						pEvtPriv->bBreakCnt[0] = DT_BIT_STR;
						pEvtPriv->bEleCtrlVal[0] = DT_LONG64;
						pEvtPriv->bHpEng[0] = DT_LONG64;	
						
					}
					else if (pEvtCtrl->wOI == TERM_PURCHPARACHG)
					{
						TPurchParaChg* pEvtPriv = (TPurchParaChg* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						memset(pEvtPriv, 0x00, sizeof(TPurchParaChg));
						memcpy(pEvtPriv->bEvtSrcOI, bBuf, 3);
					}
					else if (pEvtCtrl->wOI == TERM_ELECTRLALARM)
					{
						TEleAlram* pEvtPriv = (TEleAlram* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						memset(pEvtPriv, 0x00, sizeof(TEleAlram));
						memcpy(pEvtPriv->bEvtSrcOI, bBuf, 3);
						pEvtPriv->bCtrlOI[0] = DT_OI;
						pEvtPriv->bEleAlrCtrlVal[0] = DT_LONG64;
					}
				}
				else if (pEvtCtrl->pbSrcFmt[0] == g_bEvtSrcTSAFmt[0])
				{		
					if (pEvtCtrl->wOI == TERM_MTRCLKPRG)
					{	
						TMtrClkPrg* pEvtPriv = (TMtrClkPrg* )pEvtCtrl->pEvtPriv;
						if (pEvtPriv == NULL)
						{	
							memset(bBuf, 0x00, sizeof(bBuf));
							fParaChgFlag = true;
							goto End;
						}
						memcpy(pEvtPriv->bEvtSrcTSA, bBuf, bBuf[1]+2);
						GetCurTime(&tmCurRec);
						pEvtPriv->bClock[0] = DT_DATE_TIME_S;
						OoTimeToDateTimeS(&tmCurRec, &pEvtPriv->bClock[1]);
						pEvtPriv->bClkErr[0] = DT_INT;
						pEvtPriv->bClkErr[1] = 0;
					}
				}
			}
			if (pEvtBase->bState == EVT_S_AFT_HP) 
			{
				bBuf[iLen] = 0xff;						//状态转为触发事件存储后可以准备恢复事件
				fParaChgFlag = true;
			}
		}

		if (bBuf[iLen] == 0xff)
		{
			if ((bBuf[iLen+3]==DT_LONG_U) && ((pEvtBase->bState == EVT_S_AFT_HP) || (pEvtBase->bState == EVT_S_BF_END)))
			{
				pEvtBase->bJudgeState = 2;	//事件恢复
				pEvtBase->wTrigEvtDelaySec = OoLongUnsignedToWord(&bBuf[iLen+4]);	//获取触发延时时间
				if (pEvtBase->wTrigEvtDelaySec == 0)
					pEvtBase->wTrigEvtDelaySec  = 0x02;
			}
			else if ((bBuf[iLen+3]==DT_LONG_U) && (pEvtBase->bState == EVT_S_AFT_END))
			{
				memset(bBuf, 0x00, sizeof(bBuf));
				pEvtBase->wTrigEvtDelaySec = 0;
				fParaChgFlag = true;
			}			
		}		
	}
	else if (pEvtCtrl->bClass == IC24)
	{
		//触发一次记录（事件类别，触发延时时间，恢复延时时间）
		//事件类别∷=enum
		//{
		//	事件记录1（0），
		//	事件记录2（1），
		//	事件记录3（2），
		//	事件记录4（3）
		//}
		//触发延时时间∷=long-unsigned 
		//恢复延时时间∷=long-unsigned
		for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)	
		{	
			pEvtBase = &pEvtCtrl->pEvtBase[bItem];
			bItemNo = bItem;
			if (pEvtCtrl->bItemNum == 3)
				bItemNo++;
			bOffse = bItemNo*8;

			//判断数据是否有效
			if ((bBuf[bOffse]!=DT_ENUM) || (bBuf[bOffse+1]!=bItemNo)
				|| ((bBuf[bOffse+2]!=DT_LONG_U) && (bBuf[bOffse+2]!=0xfe) && (bBuf[bOffse+2]!=0xff))
				|| (bBuf[bOffse+5]!=DT_LONG_U))		
			{	
				memset(bBuf+bOffse, 0x00, 8);
				fParaChgFlag = true;
				continue;
			}
			//触发延时时间
			if (bBuf[bOffse+2] == DT_LONG_U)
			{	
				pEvtBase->bJudgeState = 0;	//先强制结束事件
				bBuf[bOffse+2] = 0xfe;						//状态改为可以开始触发事件了
				fParaChgFlag = true;
			}
			else if (bBuf[bOffse+2] == 0xfe)
			{
				pEvtBase->bJudgeState = 1;	//事件发生
				pEvtBase->wTrigEvtDelaySec = OoLongUnsignedToWord(&bBuf[bOffse+3]);	//获取触发延时时间
				if (pEvtBase->wTrigEvtDelaySec == 0)
					pEvtBase->wTrigEvtDelaySec  = 0x02;
				if (pEvtBase->bState == EVT_S_AFT_HP) 
				{
					bBuf[bOffse+2] = 0xff;						//状态转为触发事件存储后可以准备恢复事件
					fParaChgFlag = true;
				}
			}
			
			if (bBuf[bOffse+2] == 0xff)
			{
				if ((bBuf[bOffse+5]==DT_LONG_U) && ((pEvtBase->bState == EVT_S_AFT_HP) || (pEvtBase->bState == EVT_S_BF_END)))
				{
					pEvtBase->bJudgeState = 2;	//事件恢复
					pEvtBase->wTrigEvtDelaySec = OoLongUnsignedToWord(&bBuf[bOffse+6]);	//获取触发延时时间
					if (pEvtBase->wTrigEvtDelaySec == 0)
						pEvtBase->wTrigEvtDelaySec  = 0x02;
				}
				else if ((bBuf[bOffse+5]==DT_LONG_U) && (pEvtBase->bState == EVT_S_AFT_END))
				{
					memset(bBuf+bOffse, 0x00, 8);
					pEvtBase->wTrigEvtDelaySec = 0;
					fParaChgFlag = true;
					continue;
				}
			}		
		}
	}

End:
	if (fParaChgFlag == true)
	{
		WriteItemEx(BN11, wPn, EVT_TRIG_ID, bBuf);
		TrigerSaveBank(BN11, 0, -1);
	}
	return true;
}*/
bool DoNullEvt(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bItem;
	BYTE bFlag = false;
	
	//重初始化
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
		{	
			DTRACE(DB_INMTR, ("DoNullEvt: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
			pEvtCtrl->pfnInitEvt(pEvtCtrl);
		}
	}
	//重初始化后仍有失败项，直接返回
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
			return false;
	}

	//事件当前时标
	pEvtCtrl->dwNewClick = GetClick();

	if (pEvtCtrl->pEvtBase[0].bState == EVT_S_BF_HP)
		bFlag = true;
	//if (DoTrigeEvt(pEvtCtrl) == true)
	//	bFlag = true;

	if (bFlag)
	{
		//滤波延时,更新状态机
		UpdateState(pEvtCtrl);

		//申请的临时空间
		UpdateRecMem(pEvtCtrl, 0);
		UpdateItemMem(pEvtCtrl, 0);

		//记录数及当前值记录表处理
		UpdateEvtStaData(pEvtCtrl);

		//根据状态机保存记录表
		SaveTermEvtRec(pEvtCtrl);
	}
	//事件清零处理
	ClearTermEvt(pEvtCtrl);

	//刷新事件时标
	if (pEvtCtrl->dwNewClick - pEvtCtrl->dwLastClick > EVT_UPDATE_CYC)
		pEvtCtrl->dwLastClick = pEvtCtrl->dwNewClick;
	
	return true;
}

//描述：通用的事件执行函数
bool DoEvt(struct TEvtCtrl* pEvtCtrl)
{	
	BYTE bItem;
	
	//重初始化
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
		{	
			DTRACE(DB_INMTR, ("DoEvt: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
			pEvtCtrl->pfnInitEvt(pEvtCtrl);
		}
	}
	//重初始化后仍有失败项，直接返回
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
			return false;
	}

	//事件当前时标
	pEvtCtrl->dwNewClick = GetClick();

	//判断状态
	pEvtCtrl->pfnJudgeEvt(pEvtCtrl);

	//触发事件
	//DoTrigeEvt(pEvtCtrl);

	//滤波延时,更新状态机
	UpdateState(pEvtCtrl);

	//申请的临时空间
	UpdateRecMem(pEvtCtrl, 0);
	UpdateItemMem(pEvtCtrl, 0);

	//记录数及当前值记录表处理
	UpdateEvtStaData(pEvtCtrl);

	//根据状态机保存记录表
	SaveTermEvtRec(pEvtCtrl);

	//事件清零处理
	ClearTermEvt(pEvtCtrl);

	//刷新事件时标
	if (pEvtCtrl->dwNewClick - pEvtCtrl->dwLastClick > EVT_UPDATE_CYC)
		pEvtCtrl->dwLastClick = pEvtCtrl->dwNewClick;

	return true;
}

//描述：失压事件执行函数
bool DoVLoss(struct TEvtCtrl* pEvtCtrl)
{	
	BYTE bItem;
	
	//重初始化
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
		{	
			DTRACE(DB_INMTR, ("DoVLoss: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
			pEvtCtrl->pfnInitEvt(pEvtCtrl);
		}
	}
	//重初始化后仍有失败项，直接返回
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
			return false;
	}
	
	//事件当前时标
	pEvtCtrl->dwNewClick = GetClick();

	//判断状态
	pEvtCtrl->pfnJudgeEvt(pEvtCtrl);

	//触发事件
	//DoTrigeEvt(pEvtCtrl);

	//滤波延时,更新状态机
	UpdateState(pEvtCtrl);

	//申请的临时空间
	UpdateRecMem(pEvtCtrl, 0);
	UpdateItemMem(pEvtCtrl, 0);

	//记录数及当前值记录表处理
	UpdateEvtStaData(pEvtCtrl);

	//刷新私有变量，刷新失压统计数据
	UpdateVLossPriv(pEvtCtrl);

	//根据状态机保存记录表
	SaveTermEvtRec(pEvtCtrl);

	//事件清零处理
	ClearTermEvt(pEvtCtrl);

	//刷新事件时标
	if (pEvtCtrl->dwNewClick - pEvtCtrl->dwLastClick > EVT_UPDATE_CYC)
		pEvtCtrl->dwLastClick = pEvtCtrl->dwNewClick;

	return true;
}


//描述：需量超限事件执行函数
bool DoDmd(struct TEvtCtrl* pEvtCtrl)
{	
	BYTE bItem;
	
	//重初始化
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
		{	
			DTRACE(DB_INMTR, ("DoDmd: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
			pEvtCtrl->pfnInitEvt(pEvtCtrl);
		}
	}
	//重初始化后仍有失败项，直接返回
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
			return false;
	}

	//事件当前时标
	pEvtCtrl->dwNewClick = GetClick();

	//判断状态
	pEvtCtrl->pfnJudgeEvt(pEvtCtrl);

	//触发事件
	//DoTrigeEvt(pEvtCtrl);

	//滤波延时,更新状态机
	UpdateState(pEvtCtrl);

	//刷新私有变量，事件发生期间取需量最大值
	UpdateDmdPriv(pEvtCtrl);

	//申请的临时空间
	UpdateRecMem(pEvtCtrl, 0);
	UpdateItemMem(pEvtCtrl, 0);

	//记录数及当前值记录表处理
	UpdateEvtStaData(pEvtCtrl);

	//根据状态机保存记录表
	SaveTermEvtRec(pEvtCtrl);

	//事件清零处理
	ClearTermEvt(pEvtCtrl);

	//刷新事件时标
	if (pEvtCtrl->dwNewClick - pEvtCtrl->dwLastClick > EVT_UPDATE_CYC)
		pEvtCtrl->dwLastClick = pEvtCtrl->dwNewClick;

	return true;
}


//描述：全失压事件执行函数
bool DoAVLoss(struct TEvtCtrl* pEvtCtrl)
{	
	BYTE bItem;
	
	//重初始化
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
		{	
			DTRACE(DB_INMTR, ("DoAVLoss: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
			pEvtCtrl->pfnInitEvt(pEvtCtrl);
		}
	}
	//重初始化后仍有失败项，直接返回
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
			return false;
	}

	//事件当前时标
	pEvtCtrl->dwNewClick = GetClick();

	//判断状态
	pEvtCtrl->pfnJudgeEvt(pEvtCtrl);

	//触发事件
	//DoTrigeEvt(pEvtCtrl);

	//滤波延时,更新状态机
	UpdateState(pEvtCtrl);

	//刷新私有变量，更新掉电变量
	UpdateVLossPriv(pEvtCtrl);

	//申请的临时空间
	UpdateRecMem(pEvtCtrl, 0);
	UpdateItemMem(pEvtCtrl, 0);

	//记录数及当前值记录表处理
	UpdateEvtStaData(pEvtCtrl);

	//根据状态机保存记录表
	SaveTermEvtRec(pEvtCtrl);

	//事件清零处理
	ClearTermEvt(pEvtCtrl);

	//刷新事件时标
	if (pEvtCtrl->dwNewClick - pEvtCtrl->dwLastClick > EVT_UPDATE_CYC)
		pEvtCtrl->dwLastClick = pEvtCtrl->dwNewClick;

	return true;
}


//描述：事件接口函数执行
/*void DoTermEvt()
{
	static DWORD dwLastClick = GetClick();

	if (GetClick() - dwLastClick > 10)
	{
		dwLastClick = GetClick();

		WaitSemaphore(g_semTermEvt);
		for(BYTE i=0; i<EVT_NUM; i++)
			g_EvtCtrl[i].pfnDoEvt(&g_EvtCtrl[i]);
		SignalSemaphore(g_semTermEvt);
	}	
}*/

TThreadRet  DoTermEvt(void* pvPara)
{
	int iMonitorID = ReqThreadMonitorID("DoTermEvt-thrd", 60*60);	//申请线程监控ID,更新间隔为60秒
	DTRACE(DB_CRITICAL, ("DoTermEvt : started!\n"));

	while(1)
	{
		WaitSemaphore(g_semTermEvt);
		
	 	for(BYTE i=0; i<EVT_NUM; i++)
	 		g_EvtCtrl[i].pfnDoEvt(&g_EvtCtrl[i]);
		
		SignalSemaphore(g_semTermEvt);

		Sleep(100);
		UpdThreadRunClick(iMonitorID);
	}

	ReleaseThreadMonitorID(iMonitorID);
}


//**************************事件清零功能*********************************************
//描述：清统计数据，包括当前记录数和当前值记录表
//参数：@pEvtCtrl事件控制
//返回：无
void ClearEvtStaData(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bCurRecList[50] = {0};
	const TEvtAttr* pEvtAttr; 

	//获取事件类属性
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return;

	if (pEvtCtrl->bClass == IC24)
	{
		OoWriteAttr(pEvtCtrl->wOI, pEvtAttr->bCurRecNum, (BYTE*)g_bIC24CurNum);	
		OoWriteAttr(pEvtCtrl->wOI, pEvtAttr->bCurVal, (BYTE*)g_bIC24CurRecList);	
	}
	else if (pEvtCtrl->bClass == IC7)
	{
		OoWriteAttr(pEvtCtrl->wOI, pEvtAttr->bCurRecNum, (BYTE*)g_bIC7CurNum);
		if (EvtSrctoCurRecList(pEvtCtrl, bCurRecList))
			OoWriteAttr(pEvtCtrl->wOI, pEvtAttr->bCurVal, bCurRecList);
	}

	return;
}

//失压事件私有数据清零
void ClearVLossPriv(struct TEvtCtrl* pEvtCtrl)
{
	if (pEvtCtrl->wOI == MTR_VLOSS)
	{	
		TVLoss* pVLoss = (TVLoss* )pEvtCtrl->pEvtPriv;
		if (pVLoss == NULL)
			return;
		memset((BYTE*)pVLoss, 0x00, sizeof(TVLoss));
		OoWriteAttr(pEvtCtrl->wOI, IC24_VLOSSSTA, (BYTE*)g_bAllVLossSta);	//清失压统计
	}	
}

//全失压事件私有数据清零
void ClearVAllLossPriv(struct TEvtCtrl* pEvtCtrl)
{
	if (pEvtCtrl->wOI == MTR_ALLVLOSS)
	{	
		TAllVLoss* pAVLoss = (TAllVLoss* )pEvtCtrl->pEvtPriv;
		if (pAVLoss == NULL)
			return;
		memset((BYTE*)pAVLoss, 0x00, sizeof(pAVLoss));
	}	
}


//描述：一个事件记录清零
//参数：@pEvtCtrl事件控制
//返回：无
void ClearOneEvt(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bAttrTab, bItem;
	DWORD dwROAD;	
	const ToaMap* pOaMap = NULL;
	const TEvtAttr* pEvtAttr; 

	//获取事件类属性
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return;
	bAttrTab = pEvtAttr->bRecTabStart;

	//清当前值
	ClearEvtStaData(pEvtCtrl);

	//特殊事件的私有数据清零。
	ClearVLossPriv(pEvtCtrl);
	ClearVAllLossPriv(pEvtCtrl);
	if (pEvtCtrl->wOI == TERM_POWOFF)
		UpdateTermPowerOffTime();
	
	//清临时空间及清记录表
	for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
	{
		dwROAD = GetOAD(pEvtCtrl->wOI, bAttrTab, 0);

		if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTITEM)
			EvtFreeItemMem(dwROAD, &g_TermMem);
		else if (pEvtCtrl->pEvtBase[bItem].bMemType == MEM_TYPE_TERM_EVTREC)
			EvtFreeRecMem(dwROAD, &g_TermMem);

		//删除记录表
		pOaMap = GetOIMap(dwROAD);
		TdbDeleteTable(pOaMap->pszTableName);

		//置初始化标识等
		memset((BYTE*)&pEvtCtrl->pEvtBase[bItem], 0x00, sizeof(TEvtBase));

		//下一个表
		bAttrTab++;
	}

	DTRACE(DB_INMTR, ("ClearOneEvt: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
	pEvtCtrl->pfnInitEvt(pEvtCtrl);

	if (GetInfo(INFO_EVT_EVTRESET))	//先清零，再做事件清零事件记录
		SetInfo(INFO_EVT_EVTCLR);
	if (GetInfo(INFO_EVT_CLREVTRESET))	//先清零，再做事件清零事件记录
		GetEvtClearOMD(MTR_EVTCLEAR, 0x01, 0x00);	
}

//描述：正常运行时的数据清零处理
//参数：@pEvtCtrl事件控制
//返回：无
//注：以下情况会引起一条事件记录的清零
// 1. 关联属性表变更---设置参数/添加删除OAD方法
// 2. 最大记录数据变更
// 3. 清零操作---复位方法/电表清零/事件总清零/需量清零
void ClearTermEvt(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bAttrTab, bItem, bBuf[EVT_TRIG_PARA_LEN];
	WORD wPn;
	BYTE bClrFlag;
	DWORD dwROAD;	
	const ToaMap* pOaMap = NULL;
	const TEvtAttr* pEvtAttr; 
	
	if (GetEvtSn(pEvtCtrl->wOI) >= 0)
		wPn = GetEvtSn(pEvtCtrl->wOI);
	else
		return;
	
	if (ReadItemEx(BN11, wPn, EVT_CLR_ID, &bClrFlag) <= 0)
		return;
	if (bClrFlag != EVT_CLR_VALID) return;	//不需要清零

	//将事件清零
	ClearOneEvt(pEvtCtrl);

	//将清零标识清除
	bClrFlag = 0;
	WriteItemEx(BN11, wPn, EVT_CLR_ID, &bClrFlag);	//清除完所有的数据再将标识清零

	//清除触发标识
	memset(bBuf, 0x00, sizeof(bBuf));
	WriteItemEx(BN11, wPn, EVT_TRIG_ID, bBuf);	//清除完所有的数据再将标识清零
	TrigerSaveBank(BN11, 0, -1);	
}

//描述：记录特殊触发类事件
bool RecordSpecTrigerEvt(struct TEvtCtrl* pEvtCtrl)
{	
	BYTE i;
	BYTE bItem;

	 for(i=0; i<6; i++)
 	{
		//重初始化
		for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		{
			if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
			{	
				DTRACE(DB_INMTR, ("RecordSpecTrigerEvt: wOI=%u at Click=%d.\r\n", pEvtCtrl->wOI, GetClick()));
				pEvtCtrl->pfnInitEvt(pEvtCtrl);
			}
		}
		//重初始化后仍有失败项，直接返回
		for(bItem=0; bItem<pEvtCtrl->bItemNum; bItem++)
		{
			if (!pEvtCtrl->pEvtBase[bItem].fInitOk)
				return false;
		}
			
		//事件当前时标
		pEvtCtrl->dwNewClick = GetClick();
		pEvtCtrl->dwLastClick = 0;

		//判断状态
		pEvtCtrl->pfnJudgeEvt(pEvtCtrl);

		//滤波延时,更新状态机
		UpdateState(pEvtCtrl);

		//申请的临时空间
		UpdateRecMem(pEvtCtrl, 0);
		UpdateItemMem(pEvtCtrl, 0);

		//记录数及当前值记录表处理
		UpdateEvtStaData(pEvtCtrl);

		//根据状态机保存记录表
		SaveTermEvtRec(pEvtCtrl);
	 }

	//刷新事件时标
	if (pEvtCtrl->dwNewClick - pEvtCtrl->dwLastClick > EVT_UPDATE_CYC)
		pEvtCtrl->dwLastClick = pEvtCtrl->dwNewClick;
}

//描述：处理触发类事件，直接做记录提供给上行接口
//参数：@wOI 对象标识
//返回：无
void DealSpecTrigerEvt(WORD wOI)
{
	BYTE i;
	TTermEvtCtrl* pEvtCtrl;

	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return;
	if (pEvtCtrl->bClass != IC7)	//仅IC7有触发类	
		return;

	//WaitSemaphore(g_semTermEvt);
	//电表清零/事件清零
	if ((wOI==MTR_MTRCLEAR) ||(wOI==MTR_EVTCLEAR))
	{
		for(i=0; i<EVT_NUM; i++)
		{	
			if (g_EvtCtrl[i].wOI == wOI)	//不清自身事件记录
				continue;
			//if (g_EvtCtrl[i].wOI==TERM_INIT)	//不清终端初始化事件，需要清除
			//	continue;
			else if ((wOI==MTR_EVTCLEAR) && (g_EvtCtrl[i].wOI==MTR_MTRCLEAR))//0x43000500事件初始化，即事件总清零时不清电表清零事件记录和事件清零事件记录
				continue;
			ClearOneEvt(&g_EvtCtrl[i]);	
		}
	}

	switch(wOI)
	{
		case MTR_MTRCLEAR:
			SetInfo(INFO_EVT_MTRCLR);//电表清零
			break;
		case MTR_DMDCLEAR:		//需量清零
			SetInfo(INFO_EVT_DMDCLR);			
			break;	
		case MTR_EVTCLEAR:		//事件清零
			SetInfo(INFO_EVT_EVTCLR);
			GetEvtClearOMD(0x4300, 0x05, 0x00);
			//SignalSemaphore(g_semTermEvt);
			return;
		case TERM_INIT:		//终端初始化
			SetInfo(INFO_TERM_INIT);
			break;
		case TERM_CLOCKPRG:		//终端对时事件
			SetInfo(INFO_ADJ_TERM_TIME);
			break;	
		case TERM_TERMPRG:		//终端编程记录	
			SetInfo(INFO_TERM_PROG);
			break;	
		default:
			//SignalSemaphore(g_semTermEvt);
			return;
	}

	RecordSpecTrigerEvt(pEvtCtrl);
	//SignalSemaphore(g_semTermEvt);
}


//描述：获取事件清零OMD。上行通讯调用
//参数：@wOI 对象标识
//	   @ bMethod 对象方法编号
//	   @ bOpMode 操作模式
//返回：无
void GetEvtClearOMD(WORD wOI, BYTE bMethod, BYTE bOpMode)
{	
	BYTE bClrNum;	//清零个数
	DWORD dwOMD;
	TTermEvtCtrl* pEvtCtrl;
	TTermEvtCtrl* pEvtClrEvtCtrl;
	TEvtClr* pEvtClr;

	//确认是否为事件清零OAD，包括事件初始化0x43000500和各事件的复位操作
	dwOMD =  GetOAD(wOI, bMethod, bOpMode);	
	if (dwOMD != 0x43000500)	
	{
		if (bMethod != EVT_RESET)	//事件分项清零
		{	
			if (wOI!=MTR_EVTCLEAR)
				return;
		}
		//获取事件控制结构
		pEvtCtrl = GetTermEvtCtrl(wOI);
		if (pEvtCtrl == NULL)
			return;	
	}

	//获取事件清零事件控制结构
	pEvtClrEvtCtrl = GetTermEvtCtrl(MTR_EVTCLEAR);
	if (pEvtClrEvtCtrl == NULL)
		return;

	pEvtClr = (TEvtClr* )pEvtClrEvtCtrl->pEvtPriv;
	if (pEvtClr == NULL)
		return;
	pEvtClr->bOMD[0] = DT_ARRAY;
	if (pEvtClr->bOMD[1] < EVT_CLR_OMD_NUM)	//最多10个
		pEvtClr->bOMD[1]++;
	else
		return;
	bClrNum = pEvtClr->bOMD[1];
	
	pEvtClr->bOMD[2+5*(bClrNum-1)] = DT_OMD;
	OoDWordToOad(dwOMD, &pEvtClr->bOMD[3+5*(bClrNum-1)]);
}


//描述：获取编程记录事件编程对象列表OAD。上行通讯调用
//参数：@dwOAD 对象标识
//	   @ bAttr 对象属性
//	   @ bIndex 
//返回：无
void GetTermPrgOAD(WORD wOI, BYTE bAttr, BYTE Index)
{	
	BYTE bPrgNum;	//编程个数
	DWORD dwOAD;
	TTermEvtCtrl* pTermPrgCtrl;
	TTermPrg* pEvtPriv;

	dwOAD =  GetOAD(wOI, bAttr, Index);	
	//获取终端编程记录事件控制结构
	pTermPrgCtrl = GetTermEvtCtrl(TERM_TERMPRG);
	if (pTermPrgCtrl == NULL)
		return;

	pEvtPriv = (TTermPrg* )pTermPrgCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return;

	pEvtPriv->bOAD[0] = DT_ARRAY;
	if (pEvtPriv->bOAD[1] < TERM_PRG_OAD_NUM)	//最多10个
		pEvtPriv->bOAD[1]++;
	else
		return;
	
	bPrgNum = pEvtPriv->bOAD[1];
	pEvtPriv->bOAD[2+5*(bPrgNum-1)] = DT_OAD;
	OoDWordToOad(dwOAD, &pEvtPriv->bOAD[3+5*(bPrgNum-1)]);

	DTRACE(DB_FAPROTO, ("GetTermPrgOAD: ******************************************************************bPrgNum=%d, dwOAD = %x.\r\n", bPrgNum, dwOAD));
}


//**************************事件记录读取接口*********************************************
int GetEvtSpecField(struct TEvtCtrl* pEvtCtrl, DWORD dwFieldOAD, BYTE* pbField, WORD wFieldLen)
{
	WORD i = 0;
	BYTE bArrayNum, bTsaAddrLen = 0;
	int iRet = -1, iSchMtrLen = 0;
	 	
	switch(dwFieldOAD)
	{
		case 0x20200200:		//事件结束时间
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_DATE_TIME_S)
				iRet = 8;		//时间长度，已加类型
			break;	
		case 0x20240200:		//事件发生源
			iRet = GetEvtSrcData(pEvtCtrl, pbField, 0);		//只获取长度 	
			break;
		case 0x330C0206:		//事件清零列表
		case 0x33020206:		//编程对象列表  array OAD	
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_ARRAY)
			{
				bArrayNum = pbField[1];
				iRet = 2+5*bArrayNum;
			}			
			break;	
		case 0x33030206:		//搜表结果    array 一个搜表结果读取	
			if (pbField[0] == DT_NULL)
			{
				iRet = 1;
			}
			else if (pbField[0] == DT_ARRAY)
			{
				bArrayNum = pbField[1];

				pbField += 2;	//指针基址 指到第一个搜表结果02 07 ...的02位置

				iRet = 0;
				iSchMtrLen = 0;
				bTsaAddrLen = 0;
				for (i=0; i<bArrayNum; i++)
				{	
					iSchMtrLen = 0;

					iSchMtrLen += 2;		//02 07 

					iSchMtrLen += 2;		//55 pbField[3] 
					bTsaAddrLen = pbField[3];	//通信地址实际长度
					iSchMtrLen += bTsaAddrLen;	//通信地址实际长度					

					iSchMtrLen += 2;		//55 pbField[3+bTsaAddrLen+2]
					iSchMtrLen += pbField[3+bTsaAddrLen+2];	//通信地址实际长度

					iSchMtrLen += 16;		//剩余部分长度之和为16

					pbField += iSchMtrLen;	//指到第N个搜表结果02 07 ...的02位置

					iRet += iSchMtrLen;
				}

				iRet += 2;	//DT_ARRAY bArrayNum

				DTRACE(DB_FAPROTO, ("GetEvtSpecField: bArrayNum=%d, iRet = %ld.\r\n", bArrayNum, iRet));

				//iRet = 2+ONE_SCH_MTR_RLT_LEN*bArrayNum;	//一个搜表结果长度为56,不能按最大长度计算，应按电表地址实际长度计算！
			}			
			break;	
		case 0x33040206:		//跨台区搜表结果  array  一个跨台区结果	
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_ARRAY)
			{
				bArrayNum = pbField[1];
				//iRet = 2+Len*bArrayNum;	//张强将Len修改为具体的值
			}			
			break;		
		case 0x33000200:		//事件上报状态
			if (pbField[0] == DT_NULL)
				iRet = 1;
			else if (pbField[0] == DT_ARRAY)
			{
				bArrayNum = pbField[1];
				iRet = 2+CN_RPT_STATE_LEN*bArrayNum;
			}
			break;	
		default:
			if (pbField[0] == DT_NULL)	//无效数据返回NULL
				iRet = 1;
			else
				return wFieldLen;
	}

	if ((iRet>0) && (iRet<=wFieldLen))
		return iRet;
	else
		return 1;
}

int OoProReadEvtRecord(WORD wOI, BYTE bAttr, BYTE* pbRecBuf, WORD wRecLen, WORD wBufSize)
{
	BYTE bType, bOadBuf[10], bBuf[EVT_ATTRTAB_LEN];
	BYTE bTmpRecBuf[EVTREC_MAXSIZE];	//一条记录缓冲区
	BYTE* pbTmpRec = bTmpRecBuf;
	BYTE* pbRec = pbRecBuf;
	WORD wItemOffset, wItemLen, wTotalLen;
	DWORD dwOAD;
	int iLen;
	TFieldParser tFixFields;
	TFieldParser tDataFields;
	TTermEvtCtrl* pEvtCtrl;
	const ToaMap* pOaMap = NULL;
	BYTE bIndex;

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));
	
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return -1;

	//获取固定字段和数据字段
	if (GetEvtFieldParser(pEvtCtrl, &tFixFields, &tDataFields, bBuf, sizeof(bBuf)) == false)
	{	
		DTRACE(DB_INMTR, ("InitEvt: wOI=%u GetEvtFieldParser() fail.\r\n", wOI));
		return -1;
	}

	//跳过数据类型和元素个数
	wTotalLen = 0;
	*pbTmpRec++ = DT_STRUCT;
	wTotalLen++;
	*pbTmpRec++ = tFixFields.wNum+tDataFields.wNum;
	wTotalLen++;

	//处理固定字段的事件发生源、上报信息、事件清零列表等特殊数据
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;	//直接返回，固定字段不应答有问题
		}	
		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;
		}		
		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);

		//调整数据
		iLen = GetEvtSpecField(pEvtCtrl, dwOAD, pbRec, wItemLen);
		if ((iLen>0) && (iLen<=wItemLen))
			memcpy(pbTmpRec, pbRec, iLen);
		else 
			return -1;
		
		pbRec += wItemLen;	
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}
	//处理数据字段
	for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;	//直接返回，固定字段不应答有问题
		}	
		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;
		}		
		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);

		//调整数据
		iLen = GetEvtSpecField(pEvtCtrl, dwOAD, pbRec, wItemLen);
		if ((iLen>0) && (iLen<=wItemLen))
			memcpy(pbTmpRec, pbRec, iLen);
		else 
			return -1;
		
		pbRec += wItemLen;	
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}
	//memcpy(pbTmpRec, pbRec, tDataFields.wTotalLen);
	//wTotalLen +=  tDataFields.wTotalLen;
	if (wTotalLen <= wBufSize)
	{	
		memcpy(pbRecBuf, bTmpRecBuf, wTotalLen);
		//TrigerSaveBank(BN0, SECT3, -1);
		return wTotalLen;
	}			

	return -1;
}


int OoProRptEvtRecord(WORD wOI, BYTE bAttr, BYTE* pbRecBuf, WORD wRecLen, WORD wBufSize)
{
	BYTE bType, bOadBuf[10], bBuf[EVT_ATTRTAB_LEN];
	BYTE bTmpRecBuf[EVTREC_MAXSIZE];	//一条记录缓冲区
	BYTE* pbTmpRec = bTmpRecBuf;
	BYTE* pbRec = pbRecBuf;
	WORD wItemOffset, wItemLen, wTotalLen;
	DWORD dwOAD;
	int iLen;
	TFieldParser tFixFields;
	TFieldParser tDataFields;
	TTermEvtCtrl* pEvtCtrl;
	const ToaMap* pOaMap = NULL;
	BYTE bIndex;

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	memset((BYTE*)&tDataFields, 0x00, sizeof(TFieldParser));
	
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return -1;

	//获取固定字段和数据字段
	if (GetEvtFieldParser(pEvtCtrl, &tFixFields, &tDataFields, bBuf, sizeof(bBuf)) == false)
	{	
		DTRACE(DB_INMTR, ("InitEvt: wOI=%u GetEvtFieldParser() fail.\r\n", wOI));
		return false;
	}

	wTotalLen = 0;
	
	// 4个字节的事件记录OAD
	OoDWordToOad(GetOAD(wOI, bAttr, 0), pbTmpRec);
	pbTmpRec += 4;wTotalLen+= 4;
	
	// 1字节元素个数
	*pbTmpRec++ = tFixFields.wNum+tDataFields.wNum;wTotalLen++;
	
	// 5字节每个元素类型OAD*元素个数
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	
	{
		//选择OAD类型
		*pbTmpRec++ = 0;wTotalLen++;
		//OAD
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;	//直接返回，固定字段不应答有问题
		}	
		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;
		}		
		memcpy(pbTmpRec, bOadBuf+1, 4);
		pbTmpRec += 4;
		wTotalLen += 4;
	}	
	for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	
	{
		//选择OAD类型
		*pbTmpRec++ = 0;wTotalLen++;
		//OAD
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;	//直接返回，固定字段不应答有问题
		}	
		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;
		}		
		memcpy(pbTmpRec, bOadBuf+1, 4);
		pbTmpRec += 4;
		wTotalLen += 4;	
	}

	// 1字节结果
	*pbTmpRec++ = 1;wTotalLen++;
	// 1字节结果条数，1条
	*pbTmpRec++ = 1;wTotalLen++;

	//具体数据	
	//处理固定字段的事件发生源、上报信息、事件清零列表等特殊数据
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;	//直接返回，固定字段不应答有问题
		}	
		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;
		}		
		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);

		//调整数据
		iLen = GetEvtSpecField(pEvtCtrl, dwOAD, pbRec, wItemLen);
		if ((iLen>0) && (iLen<=wItemLen))
			memcpy(pbTmpRec, pbRec, iLen);
		else 
			return -1;
		
		pbRec += wItemLen;	
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}
	//处理数据字段
	for (bIndex=0; bIndex<tDataFields.wNum; bIndex++)	
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tDataFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;	//直接返回，固定字段不应答有问题
		}	
		if ((bType!=DT_OAD) || (wItemLen==0))
		{	
			pEvtCtrl->pEvtBase[0].fInitOk = false;	//需要重新初始化
			return -1;
		}		
		dwOAD = OoOadToDWord(bOadBuf+1);
		pOaMap = GetOIMap(dwOAD);

		//调整数据
		iLen = GetEvtSpecField(pEvtCtrl, dwOAD, pbRec, wItemLen);
		if ((iLen>0) && (iLen<=wItemLen))
			memcpy(pbTmpRec, pbRec, iLen);
		else 
			return -1;
		
		pbRec += wItemLen;	
		pbTmpRec += iLen;
		wTotalLen += iLen;
	}
	//memcpy(pbTmpRec, pbRec, tDataFields.wTotalLen);
	//wTotalLen +=  tDataFields.wTotalLen;
	if (wTotalLen <= wBufSize)
	{	
		memcpy(pbRecBuf, bTmpRecBuf, wTotalLen);
		//TrigerSaveBank(BN0, SECT3, -1);
		return wTotalLen;
	}			

	return -1;
}



//描述：获取事件记录表的表名，提供给上行接口
char* GetEvtRecFileName(DWORD dwROAD)
{
	const ToaMap* pOaMap = NULL;
	pOaMap = GetOIMap(dwROAD);

	if (pOaMap == NULL)
		return NULL;
	
	return pOaMap->pszTableName;
}

//描述：获取事件记录表的固定字段和数据字段，提供给上行接口
bool GetEvtRecFieldParser(DWORD dwROAD, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAtrrTabBuf, WORD wBufSize)
{
	WORD wOI;
	TTermEvtCtrl* pEvtCtrl;
	
	GetOIAttrIndex(dwROAD, &wOI, NULL, NULL);
	
	//获取事件控制结构
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return false;	
	
	//获取固定字段和数据字段
	if (GetEvtFieldParser(pEvtCtrl, pFixFields, pDataFields, pbAtrrTabBuf, wBufSize) == false)
	{	
		DTRACE(DB_INMTR, ("GetEvtFieldParser: wOI=%u GetEvtFieldParser() fail.\r\n", wOI));
		return false;
	}

	DelEvtOad(dwROAD, 0);
	TrigerSaveBank(BN0, SECT3, -1);
	return true;
}

//描述：读出一条事件记录，提供给上行接口
//参数：@wOI 	对象标识
//		@bAtrr	属性标识及其特征 bit-string（SIZE（8））
//		@bIndex属性内元素索引
//		@pbRecBuf记录接收缓冲区
//		@wBufSize记录接收缓冲区的大小
//返回：正确返回记录的长度，否则返回负数
int GetEvtRecord(WORD wOI, BYTE bAttr, BYTE bIndex, BYTE* pbRecBuf, WORD wBufSize)
{
	DWORD dwROAD;
	int iLen;
	char* pszFileName;
	BYTE bBuf[5];
	WORD wMaxNum, wTotalLen = 0;
	const TEvtAttr* pEvtAttr; 
	TTermEvtCtrl* pEvtCtrl;
	
	dwROAD= GetOAD(wOI, bAttr, bIndex);
	
	pszFileName = GetEvtRecFileName(dwROAD&0xffff1f00);
	if (pszFileName == NULL)
		return -1;

	DelEvtOad(dwROAD, 0);

	if (bIndex == 0)	//读出全部已有记录
	{
		pEvtCtrl = GetTermEvtCtrl(wOI);
		if (pEvtCtrl == NULL)
			return -1;

		//获取事件属性
		pEvtAttr = GetEvtAttr(pEvtCtrl);
		if (pEvtAttr == NULL)
			return -1;

		//获取最大记录数
		if (OoReadAttr(wOI, pEvtAttr->bMaxRecNum, bBuf, NULL, NULL) <= 0)
			return -1;
		wMaxNum = OoLongUnsignedToWord(bBuf+1);	
		if (wMaxNum == 0)	//最大记录数为0,无数据可读取
			return -1;

		//跳过数据类型和元素个数
		pbRecBuf[0] = DT_ARRAY;
		wTotalLen = 2;

 		for(bIndex=1; bIndex<=wMaxNum; bIndex++)
 		{
			// 读取记录
			iLen = ReadLastNRec(pszFileName, bIndex, pbRecBuf+wTotalLen, wBufSize);
			if (iLen<=0)	
			{	
				if (bIndex==1) //无记录
					return iLen;
				else
					break;	//已读不出数据退出
			}
			if ((wTotalLen+iLen) > wBufSize)	//数据已超
				return -1;
			iLen = OoProReadEvtRecord(wOI, bAttr, pbRecBuf+wTotalLen, iLen, wBufSize);
			if (iLen<=0)
				return iLen;
			else
				wTotalLen += iLen;
		}
		pbRecBuf[1] = bIndex-1;	//修正个数
		return wTotalLen;
	}
	else
	{
		// 读取记录
		iLen = ReadLastNRec(pszFileName, bIndex, pbRecBuf, wBufSize);
		if (iLen <= 0)
			return iLen;
		return OoProReadEvtRecord(wOI, bAttr, pbRecBuf, iLen, wBufSize);
	}
}

//描述：事件参数变更需要重新初始化事件。提供给上行接口
//参数：@dwOAD数据标识
//返回：无
void ReInitMrtEvtPara(DWORD dwOAD)
{
	BYTE bAttr, bClrFlag;
	WORD wOI, wPn;
	TTermEvtCtrl* pEvtCtrl;
	const TEvtAttr* pEvtAttr; 

	GetOIAttrIndex(dwOAD, &wOI, &bAttr, NULL);
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		return;		
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return;		

	//最大记录数及关联属性表有变更时需要清事件后重新初始化
	if ((bAttr == pEvtAttr->bMaxRecNum) || (bAttr == pEvtAttr->bRela))
	{
		if (GetEvtSn(wOI) >= 0)
		{	
			wPn = GetEvtSn(wOI);
			bClrFlag = EVT_CLR_VALID;
			WriteItemEx(BN11, wPn, EVT_CLR_ID, &bClrFlag);		//写清零标识
			TrigerSaveBank(BN11, 0, -1);
		}
	}
}

//**************************事件方法操作*********************************************
//方法1：复位
int DoTermEvtMethod1(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bClrFlag;
	WORD wPn;
	TTermEvtCtrl* pEvtCtrl;

	if ((bMethod!=EVT_RESET) || (iParaLen != 0x02))	
		goto END_ERR;
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		goto END_ERR;
	
	if ((pbPara[0] == DT_INT) &&(pbPara[1] == 0x00))
	{
		if (GetEvtSn(wOI) >= 0)
		{	
			wPn = GetEvtSn(wOI);
			SetInfo(INFO_EVT_EVTRESET);
			if (wOI == MTR_EVTCLEAR)
				SetInfo(INFO_EVT_CLREVTRESET);	
			bClrFlag = EVT_CLR_VALID;
			WriteItemEx(BN11, wPn, EVT_CLR_ID, &bClrFlag);	
			TrigerSaveBank(BN11, 0, -1);
			GetEvtClearOMD(wOI, bMethod, bOpMode);
			*pbRes = 0;	//成功  （0） 
			return 0;
		}
	}
	
END_ERR:
	*pbRes = 3;	
	return -1;
}

//方法2：执行
//空函数
int DoTermEvtMethod2(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TTermEvtCtrl* pEvtCtrl;

	if (bMethod != EVT_RUN)	
		goto END_ERR;
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		goto END_ERR;

	//nothing to do

	*pbRes = 0;	//成功  （0） 返回结果
	return 0;

END_ERR:
	*pbRes = 3;	
	return -1;	
}



//方法3：触发一次记录
//触发一次记录（事件发生源，触发延时时间，恢复延时时间）
//事件发生源∷=instance-specific
//触发延时时间∷=long-unsigned 
//恢复延时时间∷=long-unsigned
int DoTermEvtIC7Method3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
/*	BYTE bBuf[EVT_TRIG_PARA_LEN];
	int iLen;
	WORD wPn;
	TTermEvtCtrl* pEvtCtrl;

	if (bMethod!=EVT_TRIG) 	
		goto END_ERR;
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		goto END_ERR;
	if (pEvtCtrl->bClass != IC7	)
		goto END_ERR;
	iLen =  GetEvtSrcData(pEvtCtrl, pbPara, 0);	//只获取长度
	if (iLen <= 0)
		goto END_ERR;
	//if ( (iParaLen!=(6+iLen)) || (pbPara[0]!=pEvtCtrl->pbSrcFmt[0]) || (pbPara[iLen]!=DT_LONG_U) || (pbPara[iLen+3]!=DT_LONG_U))	
	//	goto END_ERR;

	if ((pbPara[0]!=pEvtCtrl->pbSrcFmt[0]) || (pbPara[iLen]!=DT_LONG_U) || (pbPara[iLen+3]!=DT_LONG_U))	
		goto END_ERR;
	
	memset(bBuf, 0x00, sizeof(bBuf));
	memcpy(bBuf, pbPara, (6+iLen));

	if (GetEvtSn(wOI) >= 0)
	{	
		wPn = GetEvtSn(wOI);	
		WriteItemEx(BN11, wPn, EVT_TRIG_ID, bBuf);		//写清零标识
		TrigerSaveBank(BN11, 0, -1);
		if (wOI == MTR_EVTCLEAR)	
			GetEvtClearOMD(MTR_EVTCLEAR, 0x03, 0x00);	
	}

	*pbRes = 0;	//成功  （0） 返回结果
	return 0;
	
END_ERR:*/
	*pbRes = 3;	
	return -1;	
}

//方法3：触发一次记录
//触发一次记录（事件类别，触发延时时间，恢复延时时间）
//事件类别∷=enum
//{
//	事件记录1（0），
//	事件记录2（1），
//	事件记录3（2），
//	事件记录4（3）
//}
//触发延时时间∷=long-unsigned 
//恢复延时时间∷=long-unsigned
int DoTermEvtIC24Method3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
/*	BYTE bItem, bBuf[EVT_TRIG_PARA_LEN];
	WORD wPn;
	TTermEvtCtrl* pEvtCtrl;

	//if ((bMethod!=EVT_TRIG) || (iParaLen!=8) || (pbPara[0]!=DT_ENUM) || (pbPara[2]!=DT_LONG_U) || (pbPara[5]!=DT_LONG_U))		
	//	goto END_ERR;

	if ((bMethod!=EVT_TRIG) || (pbPara[0]!=DT_ENUM) || (pbPara[2]!=DT_LONG_U) || (pbPara[5]!=DT_LONG_U))		
		goto END_ERR;
	
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		goto END_ERR;
	if (pEvtCtrl->bClass != IC24)
		goto END_ERR;
	bItem = pbPara[1];
	if (bItem > 3)
		goto END_ERR;
	if (pEvtCtrl->bItemNum == 3)
	{
		if (bItem == 0)
			goto END_ERR;
	}
	
	if (GetEvtSn(wOI) >= 0)
	{	
		wPn = GetEvtSn(wOI);	
		memset(bBuf, 0x00, sizeof(bBuf));
		ReadItemEx(BN11, wPn, EVT_TRIG_ID, bBuf);	
		memcpy(bBuf+bItem*8, pbPara, 8);
		WriteItemEx(BN11, wPn, EVT_TRIG_ID, bBuf);		//写清零标识
		TrigerSaveBank(BN11, 0, -1);
	}

	*pbRes = 0;	//成功  （0） 返回结果
	return 0;
	
END_ERR:*/
	*pbRes = 3;	
	return -1;	
}


//方法4：添加一个冻结关联对象属性（参数）
//参数∷=FRZRELA 对象属性描述符
int DoTermEvtMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bAttrBuf[EVT_ATTRTAB_LEN];	//关联属性表缓冲区
	BYTE bAttrOADNum, bIndex;
	BYTE bClrFlag;
	WORD wPn;
	int iLen;
	TTermEvtCtrl* pEvtCtrl;
	const TEvtAttr* pEvtAttr; 

	if ((bMethod!=EVT_ADDATTR) || (iParaLen!=5) || (pbPara[0]!=DT_OAD))
		goto END_ERR;
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		goto END_ERR;
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		goto END_ERR;	

	//处理添加OAD
	iLen = OoReadAttr(wOI, pEvtAttr->bRela, bAttrBuf, NULL, NULL);
	if ((iLen<=0) || (iLen > sizeof(bAttrBuf)) || (bAttrBuf[0]!=DT_ARRAY) || (bAttrBuf[1]==0x00))	//读数据有问题或读出为空，清零后添加OAD
	{
		memset(bAttrBuf, 0x00, sizeof(bAttrBuf));
		bAttrBuf[0] = DT_ARRAY;
		bAttrBuf[1] = 1;
		memcpy(bAttrBuf+2, pbPara, 5);
		goto END_OK;
	}
	else if (bAttrBuf[1] ==CAP_OAD_NUM)	//已存满
		goto END_ERR;	

	bAttrOADNum = bAttrBuf[1];
	for (bIndex=0; bIndex<bAttrOADNum; bIndex++)
	{
		if (memcmp(&bAttrBuf[2+bIndex*5], pbPara, 5) == 0)	//已存在OAD
			goto END_OK_OK;	
	}
	if (bIndex == bAttrOADNum)
	{
		if (iLen+5 > sizeof(bAttrBuf))
			goto END_ERR;	
		bAttrBuf[1]++;
		memcpy(&bAttrBuf[iLen], pbPara, 5);
		goto END_OK;
	}

END_ERR:
	*pbRes = 3;	
	return -1;	
	
END_OK:
	if (OoWriteAttr(wOI, pEvtAttr->bRela, bAttrBuf) < 0)
	{
		*pbRes = 3;	
		return -1;	
	}
	if (GetEvtSn(wOI) >= 0)
	{	
		wPn = GetEvtSn(wOI);	
		bClrFlag = EVT_CLR_VALID;
		WriteItemEx(BN11, wPn, EVT_CLR_ID, &bClrFlag);		//写清零标识
		TrigerSaveBank(BN11, 0, -1);
	}
END_OK_OK:
	*pbRes = 0;	//成功  （0） 返回结果
	return 0;
}

//方法5：删除一个冻结对象属性（参数）
//参数∷=OAD 对象属性描述符
int DoTermEvtMethod5(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bAttrBuf[EVT_ATTRTAB_LEN];	//关联属性表缓冲区
	BYTE bAttrOADNum, bIndex;
	BYTE bClrFlag;
	WORD wPn;
	int iLen;
	bool fDelFlag = false;
	TTermEvtCtrl* pEvtCtrl;
	const TEvtAttr* pEvtAttr; 

	if ((bMethod!=EVT_DELATTR)	|| (iParaLen!=5) || (pbPara[0]!=DT_OAD))
		goto END_ERR;	
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
		goto END_ERR;		
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		goto END_ERR;	

	//处理删除
	iLen = OoReadAttr(wOI, pEvtAttr->bRela, bAttrBuf, NULL, NULL);
	if ((iLen<=0) || (bAttrBuf[0]!=DT_ARRAY) || (bAttrBuf[1]==0x00))	//读数据有问题或读出为空，无法删除
		goto END_ERR;	

	bAttrOADNum = bAttrBuf[1];
	for (bIndex=0; bIndex<bAttrOADNum; bIndex++)
	{
		if (memcmp(&bAttrBuf[2+bIndex*5], pbPara, 5) == 0)	//遍历所有，已存在的OAD都删除
		{
			memcpy(&bAttrBuf[2+bIndex*5], &bAttrBuf[2+(bIndex+1)*5], 5*(bAttrOADNum-bIndex));
			memset(&bAttrBuf[2+bAttrOADNum*5], 0x00, 5);
			bAttrBuf[1]--;bAttrOADNum--;bIndex--;
			fDelFlag = true;
		}	
	}
	if (fDelFlag)
		goto END_OK;
	else
		goto END_OK_OK;	//未查到也回复确认帧，表示正确完成处理

END_ERR:
	*pbRes = 3;	
	return -1;	
	
END_OK:
	OoWriteAttr(wOI, pEvtAttr->bRela, bAttrBuf);
	if (GetEvtSn(wOI) >= 0)
	{	
		wPn = GetEvtSn(wOI);
		bClrFlag = EVT_CLR_VALID;
		WriteItemEx(BN11, wPn, EVT_CLR_ID, &bClrFlag);		//写清零标识
		TrigerSaveBank(BN11, 0, -1);
	}
END_OK_OK:
	*pbRes = 0;	//成功  （0） 返回结果
	return 0;
}


//**************************主动上报功能*********************************************
//描述：向某个通道发送事件上报消息
//参数：@dwCnOAD 通道OAD
//		@dwOAD事件OAD
//		@wRecIdx任务库中的记录索引
//		@bStage发生阶段
//返回：正确返回true，否则返回false
bool SendEvtMsg(DWORD dwCnOAD, DWORD dwEvtOAD,WORD wRecIdx, BYTE bStage, BYTE bSchNo, WORD wIdex, BYTE* pbROAD, WORD wRoadLen)
{
	BYTE bBuf[2] = {0};
	TEvtMsg tEvtMsg;

	if (OoReadAttr(0x4300, 8, bBuf, NULL, NULL) > 0)	
	{
		if ((bBuf[0]==DT_BOOL) && (bBuf[1]==1))		//上报开启才发消息
		{
		}		
		else
		{
			return false;
		}
	}
	else
		return false;

	memset(&tEvtMsg, 0, sizeof(tEvtMsg));

	tEvtMsg.dwOAD = dwEvtOAD|1;	//调整为上1笔
	tEvtMsg.wRecIdx = wRecIdx;
	tEvtMsg.bStage = bStage;

	if (bStage==EVT_STAGE_TASK || (bStage==EVT_STAGE_ERCRPT && (dwEvtOAD&0xFF000000)==0x30000000))
	{
		tEvtMsg.bSchNo = bSchNo;
		tEvtMsg.wIdex = wIdex;
		DWORD dwOAD = 0x60120300;
		BYTE* pbPtr = tEvtMsg.bRcsd;
		pbPtr += OoDWordToOad(dwOAD, pbPtr);
		*pbPtr++ = 2; //RCSD数量
		*pbPtr++ = 0; //OAD
		pbPtr += OoDWordToOad(0x202A0200, pbPtr);
		*pbPtr++ = 1; //ROAD
		if (sizeof(tEvtMsg.bRcsd) - (pbPtr - tEvtMsg.bRcsd) >= wRoadLen)
		{
			memcpy(pbPtr, pbROAD, wRoadLen);
			pbPtr += wRoadLen;
		}
		tEvtMsg.wRcsdLen = pbPtr - tEvtMsg.bRcsd;
	}
	else if (bStage == EVT_STAGE_ERCRPT)
	{
		tEvtMsg.bSchNo = bSchNo;
		tEvtMsg.wIdex = wIdex;
		BYTE* pbPtr = tEvtMsg.bRcsd;
		*pbPtr++ = DT_OAD;
		pbPtr += OoDWordToOad(dwEvtOAD, pbPtr);
		*pbPtr++ = DT_OCT_STR;
		pbPtr += EncodeLength(wRoadLen, pbPtr);
		if (sizeof(tEvtMsg.bRcsd) - (pbPtr - tEvtMsg.bRcsd) >= wRoadLen)
		{
			memcpy(pbPtr, pbROAD, wRoadLen);
			pbPtr += wRoadLen;
		}

		tEvtMsg.wRcsdLen = pbPtr - tEvtMsg.bRcsd;
	}

	if ((dwCnOAD&0xfff00000) == 0x45000000)	//GPRS通道
	{
#ifndef SYS_WIN		
		g_pGprsFaProto.AppendEvtMsg(&tEvtMsg);
#else
		g_pEthFaProto.AppendEvtMsg(&tEvtMsg);
#endif
	}
	else if ((dwCnOAD&0xfff00000) == 0x45100000)	//以太网通道
		g_pEthFaProto.AppendEvtMsg(&tEvtMsg);
	else if ((dwCnOAD&0xffff0000) == 0xF2010000)
		g_FapTest.AppendEvtMsg(&tEvtMsg);
	else if ((dwCnOAD&0xffff0000) == 0xF2020000)
		g_FapLocal.AppendEvtMsg(&tEvtMsg);
	else
		return false;

	DTRACE(DB_INMTR, ("***EVTRPT****EVTRPT****EVTRPT***SendEvtMsg: dwOAD=%x,  wRecIdx=%u, bStage=%d.\r\n", tEvtMsg.dwOAD,  tEvtMsg.wRecIdx, tEvtMsg.bStage));	
	
	return true;
}

//描述：取上报事件的记录
//参数：@pEvtMsg事件上报消息
//		@pbRecBuf记录接收缓冲区
//		@wBufSize记录接收缓冲区的大小
//		@bType bType = 0对存储的数据做调整使符合上行协议，bType != 0不调整存储数据
//返回：正确返回记录的长度，否则返回负数
int GetEvtRec(TEvtMsg* pEvtMsg, BYTE* pbRecBuf, WORD wBufSize, BYTE bType)
{
	BYTE bAttr;
	WORD wOI;
	int iLen;
	char* pszFileName;
	char szTableName[32];
	TTermEvtCtrl* pEvtCtrl;

	if (pEvtMsg->bStage == 0)
		return -1;

	if (pEvtMsg->bStage == EVT_STAGE_TASK)
	{
		memset(szTableName, 0, sizeof(szTableName));
		GetEvtTaskTableName(pEvtMsg->bSchNo, pEvtMsg->wIdex, szTableName);
		pszFileName = szTableName;
	}
	else if (pEvtMsg->bStage == EVT_STAGE_ERCRPT)
	{
		if (wBufSize < pEvtMsg->wRcsdLen)
		{
			DTRACE(DB_FAPROTO, ("GetEvtRec: no space.\r\n"));
			return -1;
		}
		memcpy(pbRecBuf, pEvtMsg->bRcsd, pEvtMsg->wRcsdLen);

		return pEvtMsg->wRcsdLen;
	}
	else
	{
		pszFileName = GetEvtRecFileName(pEvtMsg->dwOAD&0xffff1f00);
	}

	if (pszFileName == NULL)
		return -1;

	if (bType) //非上行通讯使用，不需要重新组数据
		return  ReadRecByPhyIdx(pszFileName, pEvtMsg->wRecIdx, pbRecBuf, wBufSize);
	
	iLen = ReadRecByPhyIdx(pszFileName, pEvtMsg->wRecIdx, pbRecBuf, wBufSize);
	if (iLen <= 0)
		return iLen;

	if (pEvtMsg->bStage == EVT_STAGE_TASK)
	{
		WORD wOff = GetEvtTaskTableFixFieldLen();
		if (iLen > (wOff+7))
		{
			iLen -= wOff;
			WORD wDataLen;
			memcpy(&wDataLen, &pbRecBuf[wOff], 2);
			wOff += 2;
			iLen -= 2;
			if (pbRecBuf[wOff] == DT_DB_LONG_U) //全事件次数
			{
				wOff += 5;
				iLen -= 5;
				wDataLen -= 5;
			}

			BYTE bAddr[17];
			BYTE bAddrLen = pbRecBuf[0];
			BYTE bAddrLenBak = bAddrLen;
			memcpy(bAddr, pbRecBuf+1, bAddrLen);
			pbRecBuf[0] = DT_TSA;
			pbRecBuf[1] = bAddrLen + 1;
			pbRecBuf[2] = bAddrLen - 1;
			memcpy(pbRecBuf+3, bAddr, bAddrLen);
			bAddrLen += 3;
			pbRecBuf[bAddrLen++] = DT_ARRAY;
			pbRecBuf[bAddrLen++] = pEvtMsg->bRcsd[15];
			if (wDataLen > iLen)
				wDataLen = 1;
#if 1
			TOobMtrInfo tMtrInfo;
			GetMeterInfo(bAddr, bAddrLenBak, &tMtrInfo);
			if (tMtrInfo.bProType != PRO_TYPE_69845)	//645表数据需特殊处理
			{
				BYTE *pbSrc = pbRecBuf+wOff;
				BYTE *pbDst = pbRecBuf+bAddrLen;
				BYTE bRcsd[256];
				BYTE *pRcsd = pEvtMsg->bRcsd+11;
				WORD wRcsdLen = pEvtMsg->wRcsdLen-11;
				int iRet;

				bRcsd[0] = DT_ROAD;
				memcpy(bRcsd+1, pRcsd, wRcsdLen);
				iRet = OoFormatSrcData(pbSrc, wDataLen, bRcsd, wRcsdLen+1, pbDst);
				if (iRet < 0)
				{
					*pbDst = 0x00;
					iRet = 1;
				}
				iLen = iRet + bAddrLen;
			}
			else
			{
				memmove(pbRecBuf+bAddrLen, pbRecBuf+wOff, wDataLen);
				iLen = wDataLen + bAddrLen;
			}
#else
			
			memmove(pbRecBuf+bAddrLen, pbRecBuf+wOff, wDataLen);
			iLen = wDataLen + bAddrLen;
#endif
		}
		return iLen;
	}

	GetOIAttrIndex(pEvtMsg->dwOAD, &wOI, &bAttr, NULL);

	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl != NULL)
		return OoProRptEvtRecord(wOI, bAttr, pbRecBuf, iLen , wBufSize);
	else
		return OoProRptMtrExcRecord(wOI, bAttr, pbRecBuf, iLen , wBufSize);		//抄表事件组帧处理
}

//描述：更新事件记录的上报状态
//参数：@dwCnOAD 通道OAD
//		@pEvtMsg事件上报消息
//		@bRptState 要置的标志位，记录中的通道上报状态的原有值会或上这个值
//返回：如果正确则返回true,否则返回false
bool UpdateEvtRptState(DWORD dwCnOAD, TEvtMsg* pEvtMsg, BYTE bRptState)
{
	BYTE bAttr, bItem, bIndex, bType, bCnNum;
	BYTE bOadBuf[10];
	BYTE pbRecBuf[EVTREC_MAXSIZE];
	BYTE* pbRec = pbRecBuf;
	WORD wOI, wItemOffset, wItemLen;
	DWORD dwOAD, dwRecCnOAD;
	int iLen, nIndex = 0;
	TTermEvtCtrl* pEvtCtrl;
	const TEvtAttr* pEvtAttr; 
	TFieldParser tFixFields;
	BYTE i;
	char* pszFileName;

	if (pEvtMsg->bStage == EVT_STAGE_TASK)
		return false;

	memset((BYTE*)&tFixFields, 0x00, sizeof(TFieldParser));
	GetOIAttrIndex(pEvtMsg->dwOAD, &wOI, &bAttr, NULL);

	DelEvtOad(0, wOI);
	
	//获取事件控制结构
	pEvtCtrl = GetTermEvtCtrl(wOI);
	if (pEvtCtrl == NULL)
	{	
		nIndex = GetMtrExcIndex(wOI);
		if (nIndex < 0)
			return false;	
		else
			return UpdateMtrExcRptState(dwCnOAD, pEvtMsg, bRptState);
	}
	
	//获取事件属性
	pEvtAttr = GetEvtAttr(pEvtCtrl);
	if (pEvtAttr == NULL)
		return false;

	//获取事件分项号
	if (bAttr >= pEvtAttr->bRecTabStart)
		bItem = bAttr-pEvtAttr->bRecTabStart;
	else 
		return false;
	
	//取出上报事件的记录
	memset(pbRecBuf, 0, sizeof(pbRecBuf));
	iLen = GetEvtRec(pEvtMsg, pbRecBuf, sizeof(pbRecBuf), 1);
	if (iLen <= 0)
		return false;
	
	//获取固定字段
	if (GetEvtFieldParser(pEvtCtrl, &tFixFields, NULL, NULL, 0) == false)
		return false;
	if (tFixFields.wNum == 0)
		return false;

	//重新刷新通道上报状态
	for (bIndex=0; bIndex<tFixFields.wNum; bIndex++)	//固定字段个数
	{
		memset(bOadBuf, 0, sizeof(bOadBuf));
		if (ReadParserField(&tFixFields, bIndex, bOadBuf, &bType, &wItemOffset, &wItemLen) <= 0)	
			return false;
		if (bType != DT_OAD) 
			return false;
		if (wItemLen == 0) 
			return false;
		dwOAD = OoOadToDWord(bOadBuf+1);
		if (dwOAD == 0x33000200)	//通道上报状态刷新
		{
			if (pbRec != DT_NULL)
			{
				bCnNum = *(pbRec+1);
				if (bCnNum >= CN_RPT_NUM)
					bCnNum = CN_RPT_NUM;
				for(i=0; i<bCnNum; i++)
				{	
					dwRecCnOAD = OoDoubleLongUnsignedToDWord(pbRec+5+i*9);
					if ((dwCnOAD&0xfff00000) == 0x45000000)	//要与函数SendEvtMsg()相匹配
					{
						dwRecCnOAD &=0xfff00000;
						dwCnOAD &=0xfff00000;
					}
					else if	((dwCnOAD&0xfff00000) == 0x45100000)
					{	
						dwRecCnOAD &=0xfff00000;
						dwCnOAD &=0xfff00000;
					}
					else
					{	
						dwRecCnOAD &=0xffff0000;
						dwCnOAD &=0xffff0000;
					}
					if (dwCnOAD == dwRecCnOAD)
						*(pbRec+10+i*9) |= bRptState;
				}
			}
		}
		pbRec += wItemLen;
	}
	pszFileName = GetEvtRecFileName(pEvtMsg->dwOAD&0xffff1f00);
	if (pszFileName == NULL)
		return false;

	//if (bRptState&0x0a)
	//	AddEvtOad(pEvtMsg->dwOAD, 1);	//已上报

	//修正记录
	SaveRecordByPhyIdx(pszFileName, pEvtMsg->wRecIdx, pbRecBuf);
	return true;
}



//描述：遥信状态量变位事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitYXEvtCtrl(struct TEvtCtrl* pEvtCtrl)
{
	TYXChgCtrl* pCtrl = (TYXChgCtrl* )pEvtCtrl->pEvtPriv;
	if (pCtrl == NULL)
		return false;

	memset((BYTE*)pCtrl, 0, sizeof(TYXChgCtrl));
	return InitEvt(pEvtCtrl);
}



//描述：遥信状态量变位判断；
int DoYXChgJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bIsValid, bValidFlag;
	BYTE bStaByte = 0;//遥信状态标志
	BYTE bPreByte = 0; //遥信状态旧标志
	BYTE bChgByte = 0;//状态变位标志；
	BYTE bBuf[20];
	BYTE bDoorStat = 0x10;//门节点状态
	TTime time;
	TYXChgCtrl* pCtrl = (TYXChgCtrl* )pEvtCtrl->pEvtPriv;
	if (pCtrl == NULL)
		return 0;

    memset(&time, 0, sizeof(time));
	if (IsAcPowerOff(&bIsValid) || bIsValid == 1)
		return 0;

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)	//有效标志
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;

	if (!bValidFlag)
	{
		DTRACE(DB_INMTR, ("MtrClrJudge: ValidFlag=0.\r\n"));
		return 0;
	}

#ifdef SYS_LINUX
	bDoorStat = GetDoorStatus(); //门节点分状态为高电平
#endif

	int nRead = ReadItemEx(BN2, PN0, 0x1100, &bStaByte);
    if (nRead <= 0)
    	return -1;

	if(bDoorStat > 0)	//增加门节点状态量变位事件
		bDoorStat = 0x00;
	else
		bDoorStat = 0x10;

	bStaByte = (bStaByte&0xef) | bDoorStat;
    	
    //bStaByte >>= 4; //取遥信状态（高四位）；

	if (!pCtrl->fInit)	//第一次仅仅读取遥信状态位，不判断；
    {
		DTRACE(DB_TASK, ("DoYXChgJudge: YX Power On Init, bStaByte=%x.\r\n", bStaByte));
        pCtrl->bStaByte = bStaByte;
        pCtrl->fInit = true;
        return 0;
    }
 
	bChgByte = bStaByte ^ pCtrl->bStaByte;
	pCtrl->bStaByte = bStaByte;

	if (bChgByte != 0)	//状态改变产生变位事件
	{ 
		if (pEvtCtrl->pEvtBase[0].bJudgeState != EVT_JS_HP)		//产生事件
		{
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			pEvtCtrl->bDelaySec = 2;
			DTRACE(DB_TASK, ("DoYXChgJudge: YX Power On Init, bJudgeState=%d, bState=%d, Click=%x.\r\n", pEvtCtrl->pEvtBase[0].bJudgeState, pEvtCtrl->pEvtBase[0].bState, GetClick()));	
		}
	}
	else
	{	
		if (pEvtCtrl->pEvtBase[0].bState == EVT_S_AFT_HP)	//发生事件还没有记录保留原bJudgeState状态 
		{
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
			pEvtCtrl->bDelaySec = 0;	
			DTRACE(DB_TASK, ("DoYXChgJudge: YX Power On Init, bJudgeState=%d, bState=%d, Click=%x.\r\n", pEvtCtrl->pEvtBase[0].bJudgeState, pEvtCtrl->pEvtBase[0].bState, GetClick()));
		}
	}
	//DTRACE(DB_TASK, ("DoYXChgJudge: YX Power On Init, bJudgeState=%d, bState=%d, Click=%x.\r\n", pEvtCtrl->pEvtBase[0].bJudgeState, pEvtCtrl->pEvtBase[0].bState, GetClick()));
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}


//各初始化函数若有特殊私有变量强哥特别处理。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
//描述：设备故障记录事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitDeviceErr(struct TEvtCtrl* pEvtCtrl)
{
	TDeviceErr* pEvtPriv = (TDeviceErr* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TDeviceErr));
	return InitEvt(pEvtCtrl);
}

//描述：发现未知电能表事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitUnKnMtr(struct TEvtCtrl* pEvtCtrl)
{
	TUnKnMtr* pEvtPriv = (TUnKnMtr* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TUnKnMtr));
	return InitEvt(pEvtCtrl);
}

//描述：跨台区电能表事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitStepArea(struct TEvtCtrl* pEvtCtrl)
{
	TStepArea* pEvtPriv = (TStepArea* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TStepArea));
	return InitEvt(pEvtCtrl);
}

//描述：遥控跳闸记录事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitYKCtrl(struct TEvtCtrl* pEvtCtrl)
{
	TYKCtrl* pEvtPriv = (TYKCtrl* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TYKCtrl));
	return InitEvt(pEvtCtrl);
}

//描述：有功总电能量差动越限事件记录
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitEpOver(struct TEvtCtrl* pEvtCtrl)
{
	TEpOver* pEvtPriv = (TEpOver* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TEpOver));
	return InitEvt(pEvtCtrl);
}

//描述：终端编程记录事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitTermPrg(struct TEvtCtrl* pEvtCtrl)
{
	TTermPrg* pEvtPriv = (TTermPrg* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TTermPrg));
	return InitEvt(pEvtCtrl);
}
//描述：终端电流回路异常事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitCurCirc(struct TEvtCtrl* pEvtCtrl)
{
	TCurCirc* pEvtPriv = (TCurCirc* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TCurCirc));
	return InitEvt(pEvtCtrl);
}

//描述：终端对电表校时记录初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitMtrClkPrg(struct TEvtCtrl* pEvtCtrl)
{
	TMtrClkPrg* pEvtPriv = (TMtrClkPrg* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TMtrClkPrg));
	return InitEvt(pEvtCtrl);
}

//描述：功控跳闸记录件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitPowCtrl(struct TEvtCtrl* pEvtCtrl)
{
	TPowCtrl* pEvtPriv = (TPowCtrl* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TPowCtrl));
	return InitEvt(pEvtCtrl);
}
//描述：电控跳闸记录事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitEleCtrl(struct TEvtCtrl* pEvtCtrl)
{
	TEleCtrl* pEvtPriv = (TEleCtrl* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TEleCtrl));
	return InitEvt(pEvtCtrl);
}
//描述：购电参数设置记录事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitPurchParaChg(struct TEvtCtrl* pEvtCtrl)
{
	TPurchParaChg* pEvtPriv = (TPurchParaChg* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;	
	memset((BYTE*)pEvtPriv, 0, sizeof(TPurchParaChg));
	return InitEvt(pEvtCtrl);
}
//描述：电控告警事件记录事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitEleAlram(struct TEvtCtrl* pEvtCtrl)
{
	TEleAlram* pEvtPriv = (TEleAlram* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;
	memset((BYTE*)pEvtPriv, 0, sizeof(TEleAlram));
	return InitEvt(pEvtCtrl);
}

//各判断函数实现强哥添加。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
//描述：终端初始化事件判断；
int TermInitJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_TERM_INIT);
}
//描述：终端版本变更事件判断；
int TermVerChgJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_TERM_VER_CHG);
}

//描述：终端消息认证错误事件判断；
int GsgQAuthJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_ESAM_AUTH_FAIL);
}
//描述：设备故障记录判断；
int DeviceErrJudge(struct TEvtCtrl* pEvtCtrl)
{
	int nRet = 0;

	nRet = OnInfoTrigerEvtJudge(pEvtCtrl, INFO_DEVICE_485_ERR);
	if (nRet != 1)	//有1个事件发生，先不判断其他事件
		nRet = OnInfoTrigerEvtJudge(pEvtCtrl, INFO_DEVICE_CCT_ERR);

    return nRet;
}
//描述：月通信流量超限事件判断；
int FluxOverJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag;
	BYTE bBuf[60];
	DWORD dwMonthFluxLimit = 0;
	DWORD dwCurMonthFlux = 0;

	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)		//有效标志
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;

	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("TermClockPrgJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)		//配置参数
	{
		if (bBuf[0]==DT_STRUCT && bBuf[1]==1 && bBuf[2]==DT_DB_LONG_U)
			dwMonthFluxLimit = OoDoubleLongUnsignedToDWord(bBuf+3);
	}

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(OI_FLUX, ATTR2, bBuf, NULL, NULL) > 0)		//当日当月流量值 属性2
	{
		if (bBuf[0]==DT_STRUCT && bBuf[1]==2)
			dwCurMonthFlux = OoDoubleLongUnsignedToDWord(bBuf+8);
	}

	if (dwMonthFluxLimit != 0)	//不为0参数才有效
	{
		if (dwCurMonthFlux > dwMonthFluxLimit)
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		else
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}

    return pEvtCtrl->pEvtBase[0].bJudgeState;
}

#if 0	//调试使用
	BYTE bMtrShTest = 0;
#endif
bool IsNeedSaveShMtrEvt(BYTE *pShMtrFlag, BYTE *pSaveFlag)
{
	BYTE bByte, bBit, bSaveMtrNum = 0;

#if 0	//调试使用
	BYTE bBuf[PN_MASK_SIZE] = {0};
	bBuf[0] = 0xff;
	bBuf[1] = 0xff;
	bBuf[2] = 0x0f;
	if (bMtrShTest == 1)
	{
		WriteItemEx(BANK16, PN0, 0x6010, bBuf);
		bMtrShTest = 0;
	}
#endif

	//获取搜表标志
	memset(pShMtrFlag, 0x00, SCH_MTR_SAVE_LEN);
	GetSchMtrEvtMask(pShMtrFlag, SCH_MTR_SAVE_LEN);
	
	if (IsAllAByte(pShMtrFlag, 0x00, SCH_MTR_SAVE_LEN))
		return false;
	memset(pSaveFlag, 0x00, SCH_MTR_SAVE_LEN);

	for (bByte=0; bByte<SCH_MTR_SAVE_LEN; bByte++)
	{
		if (pShMtrFlag[bByte] == 0x00)	//此字节不需要记录
			continue;

		for (bBit=0; bBit<8; bBit++)
		{
			if ((pShMtrFlag[bByte] & (1<<bBit)) != 0) //此序号搜到表了，需要记录
			{
				pShMtrFlag[bByte] &= ~(0x01<<bBit);
				pSaveFlag[bByte] |= (0x01<<bBit);
				bSaveMtrNum++;
				
			}
	
			if (bSaveMtrNum == SCH_MTR_SAVE_REC_NUM) //凑齐条数
			{	
				UpdataSchMtrEvtMask(pShMtrFlag, SCH_MTR_SAVE_LEN);
				return true;	
			}
		}	
	}
	UpdataSchMtrEvtMask(pShMtrFlag, SCH_MTR_SAVE_LEN);
	return true;
}

//描述：发现未知电能表事件判断；
int UnKnMtrJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[10] = {0};	
	TUnKnMtr* pEvtPriv = (TUnKnMtr* )pEvtCtrl->pEvtPriv;
	
	if (pEvtPriv == NULL)
		return false;	
	
	if (pEvtCtrl->wOI != TERM_UNKNOWNMTR)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("UnKnMtrJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("UnKnMtrJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	if (pEvtPriv->bRunStep == 0)
	{	
		if (IsNeedSaveShMtrEvt(pEvtPriv->bShMtrFlag, pEvtPriv->bSaveFlag))	//维护bShMtrFlag和bSaveFlag
		{	
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			pEvtPriv->bRunStep = 1;
		}
	}
	else
	{
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
		pEvtPriv->bRunStep++;
		if (pEvtPriv->bRunStep > 6)	//多几轮没关系
			pEvtPriv->bRunStep = 0;

	}

	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//描述：跨台区电能表事件；
int StepAreaJudge(struct TEvtCtrl* pEvtCtrl)
{
	BYTE bValidFlag, bPhase, bBuf[10] = {0};	
	TStepArea* pEvtPriv = (TStepArea* )pEvtCtrl->pEvtPriv;
	
	if (pEvtPriv == NULL)
		return false;	
	
	if (pEvtCtrl->wOI != TERM_STEPAREA)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("StepAreaJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("StepAreaJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	if (pEvtPriv->bRunStep == 0)
	{
		//台区搜表还不知道陈亮接口,后面再单独做
		//if (IsNeedSaveShMtrEvt(pEvtPriv->bShMtrFlag, pEvtPriv->bSaveFlag, SCH_MTR_SAVE_LEN, STEP_AREA_SAVE_REC_NUM))	//维护bShMtrFlag和bSaveFlag
		//{	
		//	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
		//	pEvtPriv->bRunStep = 1;
		//}
	}
	else
	{
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
		pEvtPriv->bRunStep++;
		if (pEvtPriv->bRunStep > 6)	//多几轮没关系
			pEvtPriv->bRunStep = 0;

	}

	return pEvtCtrl->pEvtBase[0].bJudgeState;

}


//描述：终端对时事件判断；
int TermClockPrgJudge(struct TEvtCtrl* pEvtCtrl)
{
	return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_ADJ_TERM_TIME);
}

//描述：遥控跳闸记录判断；
int YKCtrlBreakJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_YK_REC);
}

//#if 1 //差动事件调试使用
#if 0
BYTE bEpOverTestFlag = 0;
#endif

//描述：有功总电能量差动越限事件记录判断；
int EpOverJudge(struct TEvtCtrl* pEvtCtrl)
{

	BYTE bValidFlag, bBuf[DIFF_COMP_CFG_ID_LEN];	
	
	if (pEvtCtrl->wOI != TERM_EPOVER)	
		return -1;
	
	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
	
	//掉电
	if (IsPowerOff())
	{	
		DTRACE(DB_INMTR, ("EpOverJudge: PowerOff.\r\n"));		
		return 0;
	}	

	//有效标志
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;
	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("EpOverJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	//配置参数
	//属性6（配置参数）∷=array 有功总电能量差动组配置
	//有功总电能量差动组配置∷=structure
	//{
	//  有功总电能量差动组序号 unsigned，
	//  对比的总加组           OI，
	//  参照的总加组           OI，
	//  参与差动的电能量的时间区间及对比方法标志 unsigned，
	//  差动越限相对偏差值 integer（单位：%，换算：0），
	//  差动越限绝对偏差值 long64（单位：kWh，换算：-4）
	//}
	if (OoReadAttr(pEvtCtrl->wOI, IC7_PARA, bBuf, NULL, NULL) > 0)
	{
		//配置参数的预处理，罗敏添加。。。。。。。。。。。。。

	}
	else
	{
		//配置参数的预处理，罗敏添加。。。。。。。。。。。。。。。
	}

	//状态判断
	//基于配置参数与终端当前电量值判断状态，罗敏添加。。。。。。。。。。。。。。。
	//if (产生差动事件)
	//{
	//	给g_EpOver.bCompEng赋值
	//	给g_EpOver.bReferEng赋值
	//	给g_EpOver.bRelaErr赋值
	//	给g_EpOver.bAbsoErr赋值
	//	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
	//}
	//else if(未产生差动事件)
	//{
	//	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	//}

//#if 1	//差动事件调试使用
#if 0
	BYTE bBuf1[] = {DT_LONG64, 1,2,3,4,5,6,7,8};
	BYTE bBuf2[] = {DT_LONG64, 2,2,3,4,5,6,7,8};
	BYTE bBuf3[] = {DT_INT, 0x03,};
	BYTE bBuf4[] = {DT_LONG64, 4,2,3,4,5,6,7,8};

	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	if (bEpOverTestFlag)
	{
		//bEpOverTestFlag = 0;
		memcpy(&g_EpOver.bCompEng[0], bBuf1, 9);
		memcpy(&g_EpOver.bReferEng[0], bBuf2, 9);
		memcpy(&g_EpOver.bRelaErr[0], bBuf3, 2);
		memcpy(&g_EpOver.bAbsoErr[0], bBuf4, 9);
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
	}
#endif

	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//描述：终端编程记录判断；
int TermPrgJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_TERM_PROG);
}
//描述：终端电流回路异常事件判断；
int CurCircJudge(struct TEvtCtrl* pEvtCtrl)
{
    return pEvtCtrl->pEvtBase[0].bJudgeState;
}

#if 0
BYTE bClkPrgTestFlag = 0;
#endif
//描述：终端对电表校时记录判断；
int MtrClkPrgJudge(struct TEvtCtrl* pEvtCtrl)
{
#if 0
	BYTE bBuf1[] = {DT_TSA, 0x07, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0,0,0,0,0,0,0,0,};
	BYTE bBuf2[] = {DT_DATE_TIME_S, 0x07, 0xE0, 0x01, 0x02, 0x03, 0x04, 0x05,};
	BYTE bBuf3[] = {DT_INT, 0x03,};

	if (bClkPrgTestFlag)
	{
		bClkPrgTestFlag = 0;
		memcpy(&g_MtrClkPrg.bEvtSrcTSA[0], bBuf1, 18);
		memcpy(&g_MtrClkPrg.bClock[0], bBuf2, 8);
		memcpy(&g_MtrClkPrg.bClkErr[0], bBuf3, 2);
		SetInfo(INFO_TERM_MTRCLKPRG);
	}
#endif


    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_TERM_MTRCLKPRG);
}

//描述：功控跳闸记录判断；
int PowCtrlBreakJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_POWERCTRL_REC);
}
//描述：电控跳闸记录判断；
int EleCtrlBreakJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_ENERGYCTRL_REC);
}
//描述：购电参数设置记录判断；
int PurChParaChgJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_ENERGYBUY_PARACHG);
}
//描述：电控告警事件记录判断；
int EleCtrlAlarmJudge(struct TEvtCtrl* pEvtCtrl)
{
    return OnInfoTrigerEvtJudge(pEvtCtrl, INFO_ENERGYCTRL_ALARM);
}


//通知消息触发类事件
int OnInfoTrigerEvtJudge(struct TEvtCtrl* pEvtCtrl, BYTE bInfoType)
{
	BYTE bValidFlag;
	BYTE bBuf[10];
	TDeviceErr* pEvtPriv = NULL;

	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)		//有效标志
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;

	if (!bValidFlag)	
	{	
		DTRACE(DB_INMTR, ("TermClockPrgJudge: ValidFlag=0.\r\n"));
		return 0;
	}

	if (GetInfo(bInfoType))
	{
		if (bInfoType==INFO_DEVICE_485_ERR )
		{
			pEvtPriv = (TDeviceErr* )pEvtCtrl->pEvtPriv;
			if (pEvtPriv == NULL)
				return 0;

			pEvtPriv->bEvtSrcEnum = 3;	//485抄表故障
		}
		else if (bInfoType==INFO_DEVICE_CCT_ERR)
		{
			pEvtPriv = (TDeviceErr* )pEvtCtrl->pEvtPriv;
			if (pEvtPriv == NULL)
				return 0;

			pEvtPriv->bEvtSrcEnum = 5;	//载波通道故障
		}

		if ((pEvtCtrl->wOI==TERM_YKCTRLBREAK) || (pEvtCtrl->wOI==TERM_POWCTRLBREAK)) 
			pEvtCtrl->bDelaySec = 0;	//特殊处理，遥控和功控需要做控后2分钟数据，延时130秒再结束事件
			
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
	}
	else
	{
		if ((pEvtCtrl->wOI==TERM_YKCTRLBREAK) || (pEvtCtrl->wOI==TERM_POWCTRLBREAK)) 
			pEvtCtrl->bDelaySec = 130;	//特殊处理，遥控和功控需要做控后2分钟数据，延时130秒再结束事件

		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;
	}

    return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//返回APDU长度
//组698.45协议读取最近一次停上电事件记录时标报文
int MakeEvtAPDUFrm(BYTE* pbBuf)
{
	BYTE* pbBuf0 = pbBuf;

	*pbBuf++ = 0x05;	//GET-Request
	*pbBuf++ = 0x03;	//GetRequestRecord
	*pbBuf++ = 0x03;	//PIID

	*pbBuf++ = 0x30;
	*pbBuf++ = 0x11;
	*pbBuf++ = 0x02;
	*pbBuf++ = 0x00;	//停上电事件0AD

	*pbBuf++ = 0x09;	//RSD
	*pbBuf++ = 0x01;	//上1笔

	*pbBuf++ = 0x02;	//RCSD，SEQUENCE OF个数=2

	*pbBuf++ = 0x00;	//OAD

	*pbBuf++ = 0x20;
	*pbBuf++ = 0x1E;
	*pbBuf++ = 0x02;
	*pbBuf++ = 0x00;	//停电发生时间0AD

	*pbBuf++ = 0x00;	//OAD

	*pbBuf++ = 0x20;
	*pbBuf++ = 0x20;
	*pbBuf++ = 0x02;
	*pbBuf++ = 0x00;	//上电发生（停电结束）时间0AD

	*pbBuf++ = 0x00;	//没有时间标签

	return pbBuf - pbBuf0;
}

//转发抄读698.45电表停上电事件，调用太频繁会影响抄表
int FwdRead69845PwrEvt(TMtrPara* pMtrPara, BYTE* pbBuf)
{
	int i = 0, iRet = 0;	
	WORD wTxLen, wAPDULen;
	BYTE bCs = 0, bProId = 0, bDarOffset = 0;
	TTime tmTime;
	BYTE bBuf[128];
	BYTE bCmdFrm[256];
	BYTE bFrmHead = pMtrPara->bAddr[0] + 8;

	if (pMtrPara->CommPara.dwBaudRate == 0)
		pMtrPara->CommPara.dwBaudRate = CBR_2400;

	wAPDULen = MakeEvtAPDUFrm(bCmdFrm+bFrmHead);	//组抄读698.45电表停上电事件帧
	wTxLen = DL69845MakeFrm(pMtrPara->wPn, pMtrPara->bAddr, bCmdFrm, bCmdFrm+bFrmHead, wAPDULen, false);

	for (i=0; i<3; i++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		iRet = MtrDoFwd(pMtrPara->CommPara, bCmdFrm, wTxLen, bBuf, sizeof(bBuf), 9000, 10);
		if (iRet > 0)
			break;
	}

	if (iRet<=0 || i>=3)
		return 0;

	for (i=0; i<iRet; i++)
	{
		if (bBuf[i] == 0x68)
			break;
	}

	if (iRet > i)
		memcpy(bBuf, &bBuf[i], iRet-i);

	//这里是否要增加桢校验判断

	bDarOffset = bFrmHead + 18;		//定位到响应数据    CHOICE
	if (bBuf[bDarOffset]==1 && bBuf[bDarOffset+1]==1)	//响应数据判断
	{
		if (bBuf[bDarOffset+2]==DT_DATE_TIME_S && bBuf[bDarOffset+10]==DT_DATE_TIME_S)
		{
			OoDateTimeSToTime(bBuf+bDarOffset+3, &tmTime);
			TimeToYMDHMS(tmTime, pbBuf);		//转成07表停上电事件时间格式 6字节BCD
			pbBuf += 6;

			OoDateTimeSToTime(bBuf+bDarOffset+11, &tmTime);
			TimeToYMDHMS(tmTime, pbBuf);
			pbBuf += 6;

			return 12;
		}
	}

	return 0;
}


int Make07PwrEvtFrm(BYTE* pbAddr, BYTE* pbBuf)
{
	WORD i;
	BYTE bCs = 0;
	BYTE* p = pbBuf; 

	*pbBuf++ = 0x68;

	//memcpy(pbBuf, pbAddr+1, 6);	//表地址 倒序???
	memrcpy(pbBuf, pbAddr+1, 6);	//07表地址需倒序!!!
	pbBuf += 6;

	*pbBuf++ = 0x68;

	*pbBuf++ = 0x11;	//功能码
	*pbBuf++ = 0x04;	//数据长度

	*pbBuf++ = (0x01+0x33);	//数据标识
	*pbBuf++ = (0x00+0x33);
	*pbBuf++ = (0x11+0x33);
	*pbBuf++ = (0x03+0x33);	//上一次停电记录时间ID

	bCs = 0;
	for(i=0; i<pbBuf-p; i++)	//14
	{
		bCs += p[i];
	}

	*pbBuf++ = bCs;		//上一次停电记录时间ID
	*pbBuf++ = 0x16;

	return pbBuf-p;		//16
}

//转发抄读07表停上电事件，调用太频繁会影响抄表
int FwdReadV07PwrEvt(TMtrPara* pMtrPara, BYTE* pbBuf)
{
	int i = 0, iRet = 0;
	WORD wTxLen;
	BYTE bCs = 0, bProId, bAddrLen;
	BYTE bBuf[128];
	BYTE bCmdFrm[256];

	if (pMtrPara->CommPara.dwBaudRate == 0)
		pMtrPara->CommPara.dwBaudRate = CBR_2400;

	wTxLen = Make07PwrEvtFrm(pMtrPara->bAddr, bCmdFrm);	//组07协议停上电事件帧

	for (i=0; i<3; i++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		iRet = MtrDoFwd(pMtrPara->CommPara, bCmdFrm, wTxLen, bBuf, sizeof(bBuf), 9000, 10);
		if (iRet > 0)
			break;
	}

	if (iRet<=0 || i>=3)
		return 0;

	for (i=0; i<iRet; i++)
	{
		if (bBuf[i] == 0x68)
			break;
	}

	if (i+12 > iRet)
		return 0;

	if (iRet > i)
		memcpy(bBuf, &bBuf[i], iRet-i);

	//接收帧的校验和判断
	bCs = 0x00;
	for(i=0; i<bBuf[9]+10; i++)
		bCs += bBuf[i];

	if(bCs != bBuf[i])
		return 0;

	for (i=0; i<bBuf[9]; i++)
	{
		bBuf[10+i] -= 0x33;
	}

	if(bBuf[8]==0x91 || bBuf[8]==0xB1)
	{
		memcpy(pbBuf, bBuf+14, 12);
		return 12;
	}

	return 0;
}

////////////////////////////////终端停电事件(0x3106)///////////////////////////
int GetPnPwrOff(WORD wPn, BYTE* pbBuf)
{
	int iRet = 0;
	TMtrPara tMtrPara;

	if (!GetMeterPara(wPn, &tMtrPara))
		return 0;
	
	tMtrPara.CommPara.wPort = GetPnPort(wPn);
	if (tMtrPara.bProId == PROTOCOLNO_DLT645_V07)	//07表
		iRet = FwdReadV07PwrEvt(&tMtrPara, pbBuf);
	else if (tMtrPara.bProId == PROTOCOLNO_DLT69845)	//698.45表
		iRet = FwdRead69845PwrEvt(&tMtrPara, pbBuf);

	return iRet;
}

DWORD GetDiffVal(DWORD dwVal1, DWORD dwVal2)
{
	DWORD dwVal = 0;
	if(dwVal1 >= dwVal2)
		dwVal = dwVal1 - dwVal2;
	else
		dwVal = dwVal2 - dwVal1;
	return dwVal;
}

bool IsMtrPwrOff(WORD wPn, DWORD dwPwrOffSec, BYTE* pbBuf, const TTime time, bool* fMtrOrTerm)
{
	int iLen;
	TTime tm;
	DWORD dwPwrOn=0, dwPwrOff=0, dwSec=0;
	WORD i, wLimt =0, wLimtSec=0;
	BYTE bBuf[SAMPLE_CFG_ID_LEN];
	bool fGetPnData = false;
	TMtrPara tMtrPara;
	WORD wBaseOffset = 0;
	BYTE* p;

	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(TERM_POWOFF, ATTR6, bBuf, NULL, NULL); //读取属性6配置参数-停电数据采集配置参数
	if (iLen<=0 || bBuf[0]!=DT_STRUCT)
		return 0;

	p = &bBuf[12];	//指向TSA个数
	for (i=0; i<bBuf[12]; i++)
	{
		wBaseOffset += 2;	//TSA数据类型1 + TSA长度1
		wBaseOffset += p[wBaseOffset];
	}

	wBaseOffset += 10;	//停电事件甄别限值-停电事件起止时间偏差限值（分钟）参数偏移
	wBaseOffset += 12;	//从bBuf开始，需加上前面的12个字节
	if (wBaseOffset >= sizeof(bBuf)-3)
		return 0;

	wLimt = OoLongUnsignedToWord(bBuf+wBaseOffset) * 60;		//换算成秒
	wLimtSec = OoLongUnsignedToWord(bBuf+wBaseOffset+3) * 60;	//换算成秒

	*fMtrOrTerm = false;

	if (!GetMeterPara(wPn, &tMtrPara))
		return false;

	if (tMtrPara.bProId!=PROTOCOLNO_DLT645_V07 && tMtrPara.bProId!=PROTOCOLNO_DLT69845)	//既非07表，也非698.45表
		return false;

	DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step000 coming init false!\r\n"));
	for (int i=0; i<10; i++)
	{
		if(IsPnValid(wPn) && GetPnPwrOff(wPn, pbBuf)>0
			&& (!IsAllAByte(pbBuf, 0, 6)) && (!IsAllAByte(pbBuf, 0xff, 6)) //掉电发生时刻有效
			&& (!IsAllAByte(pbBuf+6, 0, 6)) && (!IsAllAByte(pbBuf+6, 0xff, 6))) //掉电结束时刻有效，真实表有的产生较慢，可能先回全ff了，多抄2次
		{
			fGetPnData = true;
			*fMtrOrTerm = true; //置有效
			DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step001 coming init true! wPn=%d.\r\n", wPn));
			if(IsBcdCode(pbBuf, sizeof(pbBuf)))
			{
				DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step222 coming init! wPn=%d.\r\n", wPn));
				memset(&tm, 0x00, sizeof(&tm));
				Fmt15ToTime(pbBuf+1, tm);
				tm.nSecond = BcdToByte(pbBuf[0]);
				dwPwrOff = TimeToSeconds(tm);
				memset(&tm, 0x00, sizeof(&tm));
				Fmt15ToTime(pbBuf+7, tm);
				tm.nSecond = BcdToByte(pbBuf[6]);
				dwPwrOn  = TimeToSeconds(tm);

				if(dwPwrOn > dwPwrOff)
				{
					dwSec = dwPwrOn - dwPwrOff;
					if(GetDiffVal(dwPwrOffSec, dwSec) > wLimtSec)//停电时长
					{
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step1 dwPwrOffSec=%ld, dwSec=%ld, delta=%ld > wLimtSec=%ld.\r\n", dwPwrOffSec, dwSec, GetDiffVal(dwPwrOffSec, dwSec), wLimtSec));
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step1 term PwrOn time= %02d %02d:%02d:%02d.\r\n", time.nDay, time.nHour, time.nMinute, time.nSecond));
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step1 term PwrOff time= %02d %02d:%02d:%02d.\r\n", g_PowerOffTmp.tPoweroff.nDay, g_PowerOffTmp.tPoweroff.nHour, g_PowerOffTmp.tPoweroff.nMinute, g_PowerOffTmp.tPoweroff.nSecond));
						*fMtrOrTerm = false;
					}

					if((GetDiffVal(TimeToSeconds(time), dwPwrOn)>wLimt)||//停上电时间点
						(GetDiffVal(TimeToSeconds(g_PowerOffTmp.tPoweroff), dwPwrOff)>wLimt))
					{
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step2 Pwr On or Off exceed, Pwr On delta=%ld , wLimt=%ld.\r\n", GetDiffVal(TimeToSeconds(time), dwPwrOn), wLimt));			
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step2 Pwr On or Off exceed, Pwr Off delta=%ld , wLimt=%ld.\r\n", GetDiffVal(TimeToSeconds(g_PowerOffTmp.tPoweroff), dwPwrOff), wLimt));
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step2 term PwrOn time= %02d %02d:%02d:%02d.\r\n", time.nDay, time.nHour, time.nMinute, time.nSecond));			
						DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step2 term PwrOff time= %02d %02d:%02d:%02d.\r\n", g_PowerOffTmp.tPoweroff.nDay, g_PowerOffTmp.tPoweroff.nHour, g_PowerOffTmp.tPoweroff.nMinute, g_PowerOffTmp.tPoweroff.nSecond));			

						*fMtrOrTerm = false;
					}
				}
				else
				{
					DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step3 dwPwrOn=%ld, smaller than dwPwrOff=%ld.\r\n", dwPwrOn, dwPwrOff));			
					*fMtrOrTerm = false;
				}
			}
			else
			{
				DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step4 Para Invalid, wPn=%d!\r\n", wPn));
				*fMtrOrTerm = false;
			}

			break;
		}
		else
		{
			DTRACE(DB_METER_EXC, ("IsMtrPwrOff--step009 no way to go! wPn=%d, i=%d.\r\n", wPn, i));
		}

		if (IsMtrErr(wPn))
			break;
		else
			Sleep(3000); //由于设计原理限制，暂时只能这样一直去抄读一段时间了，因为真实表产生上电事件需要60秒防抖时间 终端启动时间20多秒+10*3000ms差不多一分钟，阻塞太长时间会有可能影响其他告警事件判断
	}
	return fGetPnData;
}


//描述：终端停/上电事件初始化
//参数：@pEvtCtrl 事件控制
//返回：true/false
bool InitPowOff(struct TEvtCtrl* pEvtCtrl)
{
	TPowOff* pEvtPriv = (TPowOff* )pEvtCtrl->pEvtPriv;
	if (pEvtPriv == NULL)
		return false;
	memset((BYTE*)pEvtPriv, 0x00, sizeof(TPowOff));	
	return InitEvt(pEvtCtrl);
}

//描述：终端停/上电事件；
//描述：停、上电事件判断；
//#define POWOFF_DEBUG_TEST
#ifdef POWOFF_DEBUG_TEST	//调试使用
BYTE g_PowOffTestFlag = 0;	
#endif
int PowOffJudge(struct TEvtCtrl* pEvtCtrl)
{
	int iLen;
	TTime time;
	BYTE bIsValid = 0, bValidFlag = 0;
	DWORD dwSec = 0, dwPwrOffSec = 0;
	WORD i = 0, wPn = 0, wCnt = 0, wMaxSec = 0, wMinSec = 0;
	bool fPowerOff = false, fMtrOrTerm	= false, fPwrOffValid = true;
	char cTime[20];
	BYTE bF98Buf[13];
	BYTE bPnPwrOff[12];
	BYTE bBuf[SAMPLE_CFG_ID_LEN];
	BYTE bMtrAddr[MTR_ADDR_LEN];
	const WORD wOI = pEvtCtrl->wOI;
	WORD wBaseOffset = 0;
	BYTE* pbBuf = NULL;


	// 1 浙江停上电
	//*ZJ1*停电即记录停电事件。
	//*ZJ2*上电时，若停电没正常记录停电事件，那么上电即补做停电记录。
	//*ZJ3*上电后，立即上报第1条上电。若配置电表，抄到表数据做判断后视情况做第2条上电记录。
	//*ZJ4*各条记录上报与否与配置的上报参数有关。
	
	// 2 国网停上电
	//*GW1*停电时，若配置电表，不需要做停电记录。
	//*GW2*停电时，若未配置电表，需要做停电记录。
	//*GW3*上电时，若停电时需要做停电记录但未来的及做，那么上电即补做停电记录。
	//*GW4*上电后，若配置电表，待判断完后做停电记录；抄完表后做上电记录。
	//*GW5*上电后，若未配置电表，待判断完后做上电记录。
	//*GW6*各条记录上报与否与配置的上报参数有关。

	pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_END;		//默认正常结束

	//获取私有变量
	TPowOff* pCtrl = (TPowOff* )pEvtCtrl->pEvtPriv;
	if (pCtrl == NULL)
		return 0;
		
#ifdef POWOFF_DEBUG_TEST	//调试使用
	if (g_PowOffTestFlag == 0)	//上电
		fPowerOff = 0;
	else
		fPowerOff = 1;
#else
	fPowerOff = IsAcPowerOff(&bIsValid);
#endif

	if (bIsValid == 1)	//上电9秒后再处理停上电事件，否则直接退出
	{	
		return 0;
	}

	memset(bBuf, 0, sizeof(bBuf));
	if (OoReadAttr(pEvtCtrl->wOI, IC7_VALIDFLAG, bBuf, NULL, NULL) > 0)	//有效标志
		bValidFlag = bBuf[1];
	else
		bValidFlag = 0;

	memset(bBuf, 0, sizeof(bBuf));
	iLen = OoReadAttr(wOI, ATTR6, bBuf, NULL, NULL); 	//读取属性6配置参数-停电数据采集配置参数
	if (iLen<=0 || bBuf[0]!=DT_STRUCT)		//配置参数有异常直接退出，强制结束事件
	{	
		pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
		return 0;
	}

	//停上电初始化处理。若停电未做过停电记录，上电时补上
	if (!pCtrl->fInit)	
	{
		if (fPowerOff)
			pCtrl->fPowerOff = true;
		else
			pCtrl->fPowerOff = false;

		pCtrl->bStep = PWR_OFF_RUN_CNT;	//上电时默认不需要空跑

		DTRACE(DB_METER_EXC, ("PowOffJudge: Init g_tPowerOn=%s \r\n", TimeToStr(g_tPowerOn, cTime)));
		
#ifdef POWOFF_DEBUG_TEST	//调试使用
		g_PowerOffTmp.tPoweroff= g_tPowerOn;
#endif

		dwSec = SecondsPast(g_PowerOffTmp.tPoweroff, g_tPowerOn);
		DTRACE(DB_METER_EXC, ("PowOffJudge: Init dwSec = %d, fAlrPowerOff = %d\r\n", dwSec, g_PowerOffTmp.fAlrPowerOff));

		if (!g_PowerOffTmp.fAlrPowerOff && (SecondsPast(g_PowerOffTmp.tPoweroff, g_tPowerOn) > 60))	//停电时没有做过停电记录，上电时补上
		{
#ifdef VER_ZJ
			if (bValidFlag)	//浙江，不受采集标志影响，停电固定上报.	//*ZJ2*上电时，若停电没正常记录停电事件，那么上电即补做停电记录。			
#else
			if (bValidFlag && (bBuf[6]&0x80)==0x00)		//国网标准，采集标志无效时，停电不上报 	//*GW3*上电时，若停电时需要做停电记录但未来的及做，那么上电即补做停电记录。
#endif		
			{
				pCtrl->bAttr = 0x80;	//正常无效停电事件
				pCtrl->bEvtSrcEnum = 0;		//停电事件
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
				pCtrl->bStep = 1;		//第一笔停电事件发生状态
				pCtrl->bRptFlag = 1;	//需要上报发生	
				DTRACE(DB_METER_EXC, ("PowOffJudge: Init save powoff event ########## Terminal __Bat Break__ is power off.\r\n"));	//不带电池的停电事件
			}
			
			g_PowerOffTmp.fAlrPowerOff = true;	//掉电前上报了停电告警
			SavePoweroffTmp();
		}
		
		pCtrl->fIsUp = false;	
		pCtrl->fIsUpRec = false;	
		pCtrl->fMtrOrTerm = false;	
		pCtrl->fOldPowerOff = g_PowerOffTmp.fAlrPowerOff;
		pCtrl->fInit = true;
		
		return pEvtCtrl->pEvtBase[0].bJudgeState;
	}

	//停上电消抖处理，获取当前停上电状态
	if (fPowerOff)
	{
		pCtrl->wLastPwrOnClick = 0;
		if(pCtrl->wLastPwrOffClick == 0)
		{
			pCtrl->wLastPwrOffClick = GetClick();
			return pEvtCtrl->pEvtBase[0].bJudgeState;
		}
		else if(GetClick() - pCtrl->wLastPwrOffClick >= 5)////持续停电5秒以上才报
		{
			pCtrl->fPowerOff = true;
			WriteItemEx(BN2, PN0, 0x210e, (BYTE* )&fPowerOff);
		}
		else
		{
			return pEvtCtrl->pEvtBase[0].bJudgeState;
		}
	}
	else
	{
		pCtrl->wLastPwrOffClick = 0;
		if(pCtrl->wLastPwrOnClick == 0)
		{
			pCtrl->wLastPwrOnClick = GetClick();
			return pEvtCtrl->pEvtBase[0].bJudgeState;
		}
		else if(GetClick() - pCtrl->wLastPwrOnClick >= 5)////持续上电5秒以上才报
		{
			pCtrl->fPowerOff = false;
		}
		else
		{
			return pEvtCtrl->pEvtBase[0].bJudgeState;
		}		
	}

	if (pCtrl->bStep < PWR_OFF_RUN_CNT)	//空跑使能正常存储
	{
		pCtrl->bStep++;
		return pEvtCtrl->pEvtBase[0].bJudgeState;
	}

	//DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask pCtrl->fPowerOff=%d, pCtrl->fOldPowerOff=%d, fAlrPowerOff = %d.\r\n", pCtrl->fPowerOff, pCtrl->fOldPowerOff, g_PowerOffTmp.fAlrPowerOff));
	//停上电状态发生改变时，需要做记录
	if (pCtrl->fPowerOff != pCtrl->fOldPowerOff)   //发生改变
	{
		pCtrl->fOldPowerOff = pCtrl->fPowerOff;	
		GetCurTime(&time);

		if (pCtrl->fPowerOff)   //终端停电
		{
#ifdef VER_ZJ
			if (bValidFlag)	//浙江，不受采集标志影响，停电固定上报.	//*ZJ1*停电即记录停电事件。
#else
			if (bValidFlag && (bBuf[6]&0x80)==0x00)		//国网标准，采集标志无效时，停电不上报	//*GW1*停电时，若配置电表，不需要做停电记录。//*GW2*停电时，若未配置电表，需要做停电记录。
#endif
			{
				pCtrl->bAttr = 0x80;	//正常无效停电事件
				pCtrl->bEvtSrcEnum = 0;		//停电事件
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
				pCtrl->bStep = 1;		//第一笔停电事件发生状态
				pCtrl->bRptFlag = 1;	//需要上报发生	
				DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask save powoff event  ########## Terminal is power off. \r\n"));	
			}
			g_PowerOffTmp.fAlrPowerOff = true;	//掉电前上报了停电告警
			memcpy(&g_PowerOffTmp.tPoweroff, &time,sizeof(time)); 
			SavePoweroffTmp();

			TrigerSaveBank(BN0, SECT3, -1);
			TrigerSaveBank(BN0, SECT16, -1);	//停电前保存一下事件相关数据

			pCtrl->fIsUp = false;	
			pCtrl->fIsUpRec = false;	
			pCtrl->fMtrOrTerm = false;
			SetInfo(INFO_PWROFF);
		}
		else if (!pCtrl->fPowerOff)		//终端来电
		{
			GetCurTime(&g_tPowerOn);	//刷新上电时间
			if (!bValidFlag)	//事件无效不判断了,强制结束
			{
				DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask ValidFlag=0.\r\n"));
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_FORCE_END;
				return pEvtCtrl->pEvtBase[0].bJudgeState;
			}
			
			pCtrl->fIsUp = true;	//上电
	
#ifdef VER_ZJ
			if (bValidFlag)		//*ZJ3*上电后，立即上报第1条上电。若配置电表，抄到表数据做判断后视情况做第2条上电记录。
			{	
				pCtrl->bAttr = 0x80;	//浙江要求，上电后立即上报正常无效的上电事件
				pCtrl->bEvtSrcEnum = 0x01;		//上电事件
				pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
				pCtrl->bStep = 1;
				pCtrl->bRptFlag = 2;	//需要上报恢复
		
				DTRACE(DB_METER_EXC, ("PowOffJudge: Right now Save Power On Rec bJudgeState = %d Click=%ld .\r\n", pEvtCtrl->pEvtBase[0].bJudgeState, GetClick()));
				return pEvtCtrl->pEvtBase[0].bJudgeState;
			}
#endif
		}	
	}

	if (pCtrl->fIsUp && (pCtrl->bStep>=PWR_OFF_RUN_CNT))
	{
		pCtrl->fIsUp = false;
		
		//////////////////////////事件有效性的判断↓////////////////////////////////////
		if (g_PowerOffTmp.fAlrPowerOff)
		{
			if (!IsInvalidTime(g_tPowerOn) && !IsInvalidTime(g_PowerOffTmp.tPoweroff))
			{
				if(TimeToSeconds(g_tPowerOn) > TimeToSeconds(g_PowerOffTmp.tPoweroff))
				{
					DTRACE(DB_METER_EXC, ("PowOffJudge1: RunTask fPwrOffValid = %d.\r\n", fPwrOffValid));
					wBaseOffset = 0;
					pbBuf = &bBuf[12];	//指向TSA个数
					for (i=0; i<bBuf[12]; i++)
					{
						wBaseOffset += 2;	//TSA数据类型1 + TSA长度1
						wBaseOffset += pbBuf[wBaseOffset];
					}

					wBaseOffset += 4;	//停电事件甄别限值-停电时间最小有效间隔（分钟）参数偏移
					wBaseOffset += 12;	//从bBuf开始，需加上前面的12个字节
					if (wBaseOffset >= sizeof(bBuf)-3)
						return pEvtCtrl->pEvtBase[0].bJudgeState;

					wMinSec = OoLongUnsignedToWord(bBuf+wBaseOffset) * 60;	//换算成秒
					wMaxSec = OoLongUnsignedToWord(bBuf+wBaseOffset+3) * 60;	//换算成秒
					dwPwrOffSec = TimeToSeconds(g_tPowerOn) - TimeToSeconds(g_PowerOffTmp.tPoweroff);		//来电时间 - 停电时间

					if(dwPwrOffSec>wMaxSec || dwPwrOffSec<wMinSec)
						fPwrOffValid = false;	//超过了甄别值
				}
				else
				{
					DTRACE(DB_METER_EXC, ("PowOffJudge2: RunTask fPwrOffValid = %d.\r\n", fPwrOffValid));
					fPwrOffValid = false;	//来电时间早于停电时间
				}
			}
			else
			{
				DTRACE(DB_METER_EXC, ("PowOffJudge3: RunTask fPwrOffValid = %d.\r\n", fPwrOffValid));
				fPwrOffValid = false;	//时间为空
			}
		}
		else
		{
			DTRACE(DB_METER_EXC, ("PowOffJudge4: RunTask fPwrOffValid = %d.\r\n", fPwrOffValid));
			fPwrOffValid = false;
		}

		//DTRACE(DB_METER_EXC, ("PowOffJudge5: RunTask fPwrOffValid = %d.\r\n", fPwrOffValid));

		if(bBuf[6] & 0x80)	//采集标志
		{
			DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask MtrRead --step00000.\r\n"));
			//////////////////////////////台区表或终端事件判断↓////////////////////////////////

			wCnt = bBuf[12];
			if (wCnt > SAMPLE_MTR_NUM)
				wCnt = SAMPLE_MTR_NUM;

			if (wCnt>0 && !(bBuf[6]&0x40))	//按照设置的测量点采
			{
				DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask MtrRead--sample given mtr addr.\r\n"));
				wBaseOffset = 0;
				pbBuf = &bBuf[12];	//指向TSA个数

				for (i=0; i<wCnt; i++)
				{
					wBaseOffset += 2;	//TSA数据类型1 + TSA长度1

					bMtrAddr[0] = pbBuf[wBaseOffset] - 1;	//地址长度
					if (bMtrAddr[0] > 16)
						bMtrAddr[0] = 16;

					memset(&bMtrAddr[1], 0, 16);
					memcpy(&bMtrAddr[1], &pbBuf[wBaseOffset+2], bMtrAddr[0]);		//地址内容
					wBaseOffset += pbBuf[wBaseOffset];

					//wPn = MtrAddrToPn(bMtrAddr, bMtrAddr[0]);
					wPn = GetMeterPn(bMtrAddr+1, bMtrAddr[0]);
					if (wPn == 0)
						continue;

					memset(bPnPwrOff, 0, sizeof(bPnPwrOff));
					if (IsMtrPwrOff(wPn, dwPwrOffSec, bPnPwrOff, g_tPowerOn, &fMtrOrTerm) && fMtrOrTerm)
						break;
				}
			}
			else if (bBuf[6] & 0x40)	//随机采
			{
				DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask MtrRead--random sample.\r\n"));
				for(i=0,wCnt=0; i<POINT_NUM; i++)
				{
					if (i == 0)
						DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask MtrRead--step33333.\r\n"));

					if (!IsPnValid(i))
						continue;

					if(wCnt>=3)
						break;

					memset(bPnPwrOff, 0x00, sizeof(bPnPwrOff));	
					if(IsMtrPwrOff(i, dwPwrOffSec, bPnPwrOff, g_tPowerOn, &fMtrOrTerm))
						wCnt++;

					if (fMtrOrTerm)
						break;
				}
			}
		}
		else
		{
			DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask MtrRead--step11111 fMtrOrTerm=false, sample flag is false.\r\n"));
			fMtrOrTerm = false;
		}

		if (fPwrOffValid)	//事件是否正常
			pCtrl->bAttr = 0x80;
		else
			pCtrl->bAttr = 0x00;

#ifndef VER_ZJ
		//产生停电事件
		if (g_PowerOffTmp.fAlrPowerOff && (bBuf[6]&0x80))		//采集标志不判断
		{
			DTRACE(DB_METER_EXC, ("PowOffJudge1: RunTask pCtrl->bAttr = %d.\r\n", pCtrl->bAttr));
			pCtrl->bEvtSrcEnum = 0;		//停电事件
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			pCtrl->bStep = 1;		//第一笔停电事件发生状态
			pCtrl->bRptFlag = 1;	//需要上报发生
			DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask save powoff event  ########## Terminal __Bat Break__ is power off.\r\n"));
		}
#endif

		/////////////////////////产生上电事件-------	
		g_PowerOffTmp.fAlrPowerOff = false;
		pCtrl->fIsUpRec = true;	//需要做上电记录
		pCtrl->fMtrOrTerm = fMtrOrTerm;
		DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask fMtrOrTerm=%d.\r\n", fMtrOrTerm));
	}
		
	//产生上电事件
	if (pCtrl->fIsUpRec && (pCtrl->bStep>=PWR_OFF_RUN_CNT))
	{
		pCtrl->fIsUpRec = false;

		DTRACE(DB_METER_EXC, ("PowOffJudge2: RunTask pCtrl->bAttr = %d.\r\n", pCtrl->bAttr));
		if (pCtrl->fMtrOrTerm)	 //事件是否有效
			pCtrl->bAttr |= 0x40;
		else
			DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask--step5 fMtrOrTerm=false, invalid event, pCtrl->bAttr=%02x.\r\n", pCtrl->bAttr));

#ifdef VER_ZJ
		if ((pCtrl->bAttr&0xc0) == 0xc0)	//浙江正常且有效事件，才上报
#endif	
		{
			pCtrl->bEvtSrcEnum = 0x01;		//上电事件
			pEvtCtrl->pEvtBase[0].bJudgeState = EVT_JS_HP;
			pCtrl->bStep = 1;		//第一笔停电事件发生状态
			pCtrl->bRptFlag = 2;	//需要上报上电恢复事件
		}

		DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask save powon event  ########## Terminal is power on.\r\n"));				
	}

	//DTRACE(DB_METER_EXC, ("PowOffJudge: RunTask pEvtCtrl->pEvtBase[0].bJudgeState=%d.\r\n", pEvtCtrl->pEvtBase[0].bJudgeState));
	return pEvtCtrl->pEvtBase[0].bJudgeState;
}

//新增一个事件对象
//属性2（新增事件列表，只读）∷= array OAD
//属性3（需上报事件对象列表，只读）∷= array OI
//注：新增事件列表对象针对每通信通道（OAD），当该列表中的事件记录通过"当前"通道被读取后，自动从该列表中删除此OI对象。
void AddEvtOad(DWORD dwOAD, bool fRptFlag)
{
	BYTE bBuf[EVT_ADDOAD_MAXLEN], bOadBuf[4];	
	BYTE bArrayNum, bIndex;
	WORD wOI;
	int iLen;

	//新增事件列表array OAD
	dwOAD &=0xffffff00;	//清底字节
	OoDWordToOad(dwOAD, bOadBuf);

	memset(bBuf, 0x00, sizeof(bBuf));
	iLen = OoReadAttr(0x3320, 0x02, bBuf, NULL, NULL);
	if ((iLen<=0) || (iLen > sizeof(bBuf)) || (bBuf[0]!=DT_ARRAY) || (bBuf[1]==0x00) || (bBuf[1]>EVT_ADDOAD_MAXNUM))	//读数据有问题或读出为空，清零后添加OI
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = DT_ARRAY;
		bBuf[1] = 1;
		bBuf[2] = DT_OAD;
		memcpy(bBuf+3, bOadBuf, 4);
		OoWriteAttr(0x3320, 0x02, bBuf);
		TrigerSaveBank(BN0, SECT3, -1);
		return;	
	}

	bArrayNum = bBuf[1];
	for (bIndex=0; bIndex<bArrayNum; bIndex++)
	{
		if (memcmp(&bBuf[3+bIndex*5], bOadBuf, 4) == 0)	//已存在OAD,不需要添加
			return;
	}

	if ((bIndex==bArrayNum) && (bArrayNum<EVT_ADDOAD_MAXNUM))
	{
		if (iLen+5 > sizeof(bBuf))
			return;	
		bBuf[1]++;
		bBuf[iLen] = DT_OAD;
		memcpy(&bBuf[iLen+1] , bOadBuf, 4);
		OoWriteAttr(0x3320, 0x02, bBuf);
		TrigerSaveBank(BN0, SECT3, -1);
		return;	
	}

	if (!fRptFlag)	
		return;

	//需要上报，添加上报事件对象列表array OI
	GetOIAttrIndex(dwOAD, &wOI, NULL, NULL);
	OoWordToOi(wOI, bOadBuf);

	memset(bBuf, 0x00, sizeof(bBuf));
	iLen = OoReadAttr(0x3320, 0x03, bBuf, NULL, NULL);

	if ((iLen<=0) || (iLen > sizeof(bBuf)) || (bBuf[0]!=DT_ARRAY) || (bBuf[1]==0x00) || (bBuf[1]>EVT_ADDOAD_MAXNUM))	//读数据有问题或读出为空，清零后添加OI
	{
		memset(bBuf, 0x00, sizeof(bBuf));
		bBuf[0] = DT_ARRAY;
		bBuf[1] = 1;
		bBuf[2] = DT_OI;
		memcpy(bBuf+3, bOadBuf, 2);
		OoWriteAttr(0x3320, 0x03, bBuf);
		TrigerSaveBank(BN0, SECT3, -1);
		return;	
	}

	bArrayNum = bBuf[1];
	for (bIndex=0; bIndex<bArrayNum; bIndex++)
	{
		if (memcmp(&bBuf[3+bIndex*3], bOadBuf, 2) == 0)	//已存在OAD,不需要添加
			return;
	}

	if ((bIndex==bArrayNum) && (bArrayNum<EVT_ADDOAD_MAXNUM))
	{
		if (iLen+3 > sizeof(bBuf))
			return;	
		bBuf[1]++;
		bBuf[iLen] = DT_OI;
		memcpy(&bBuf[iLen+1] , bOadBuf, 2);
		OoWriteAttr(0x3320, 0x03, bBuf);
		TrigerSaveBank(BN0, SECT3, -1);
		return;	
	}	
		
	return;	
}



//删除一个事件对象
//dwOAD 及wOI 入参为0时不删除
void DelEvtOad(DWORD dwOAD, WORD wOI)
{
	BYTE bBuf[EVT_ADDOAD_MAXLEN], bOadBuf[4];	
	BYTE bArrayNum, bIndex;
	int iLen;
	bool fDelFlag = false;

	if (dwOAD)
	{
		dwOAD &=0xffffff00;	//清底字节
		OoDWordToOad(dwOAD, bOadBuf);
		
		memset(bBuf, 0x00, sizeof(bBuf));	
		iLen = OoReadAttr(0x3320, 0x02, bBuf, NULL, NULL);
		if ((iLen<=0) || (bBuf[0]!=DT_ARRAY))	//读数据有问题,修正
		{
			memset(bBuf, 0x00, sizeof(bBuf));
			bBuf[0] = DT_ARRAY;
			OoWriteAttr(0x3320, 0x02, bBuf);
			TrigerSaveBank(BN0, SECT3, -1);
			return;
		}	
		if (bBuf[1]==0x00)	//读出为空，无法删除
			return;	
		
		bArrayNum = bBuf[1];
		for (bIndex=0; bIndex<bArrayNum; bIndex++)
		{
			if (memcmp(&bBuf[3+bIndex*5], bOadBuf, 4) == 0)	//遍历所有，已存在的OAD都删除
			{
				memcpy(&bBuf[2+bIndex*5], &bBuf[2+(bIndex+1)*5], 5*(bArrayNum-bIndex));
				memset(&bBuf[2+bArrayNum*5], 0x00, 5);
				bBuf[1]--;bArrayNum--;bIndex--;
				fDelFlag = true;
			}	
		}
		if (fDelFlag)
		{	
			OoWriteAttr(0x3320, 0x02, bBuf);
			TrigerSaveBank(BN0, SECT3, -1);
		}
	}
	
	if (wOI)
	{
		OoWordToOi(wOI, bOadBuf);
		
		memset(bBuf, 0x00, sizeof(bBuf));	
		iLen = OoReadAttr(0x3320, 0x03, bBuf, NULL, NULL);
		if ((iLen<=0) || (bBuf[0]!=DT_ARRAY))	//读数据有问题,修正
		{
			memset(bBuf, 0x00, sizeof(bBuf));
			bBuf[0] = DT_ARRAY;
			OoWriteAttr(0x3320, 0x03, bBuf);
			TrigerSaveBank(BN0, SECT3, -1);
			return;
		}	
		if (bBuf[1]==0x00)	//读出为空，无法删除
			return;	
		
		bArrayNum = bBuf[1];
		for (bIndex=0; bIndex<bArrayNum; bIndex++)
		{
			if (memcmp(&bBuf[3+bIndex*3], bOadBuf, 2) == 0)	//遍历所有，已存在的OAD都删除
			{
				memcpy(&bBuf[2+bIndex*3], &bBuf[2+(bIndex+1)*3], 3*(bArrayNum-bIndex));
				memset(&bBuf[2+bArrayNum*3], 0x00, 3);
				bBuf[1]--;bArrayNum--;bIndex--;
				fDelFlag = true;
			}	
		}
		if (fDelFlag)
		{	
			OoWriteAttr(0x3320, 0x03, bBuf);
			TrigerSaveBank(BN0, SECT3, -1);
		}
	}
		
	return;	
}


