/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：ProIfConst.h
 * 摘    要：本文件主要用来定义通信接口的常量
 * 当前版本：1.0
 * 作    者：岑坚宇
 * 完成日期：2009年12月
 * 备    注：
 *********************************************************************************************************/
#ifndef PROIFCONST_H
#define PROIFCONST_H

//GPRS联网步骤
#define GPRS_STEP_IDLE		0	//空闲
#define GPRS_STEP_RST		1	//复位模块
#define GPRS_STEP_APN		2	//初始化APN
#define GPRS_STEP_SIM		3	//检测SIM
#define GPRS_STEP_REG		4	//注册网络
#define GPRS_STEP_SIGN		5	//更新信号强度
#define GPRS_STEP_DIAL		6	//拨号进行中
#define GPRS_STEP_AUTH		7	//PPP认证
#define GPRS_STEP_IP		8	//获取IP过程中
#define GPRS_STEP_PPP		9	//ppp协商
#define GPRS_STEP_ONLINE	10	//GPRS在线
#define GPRS_STEP_SMS		11	//短线在线

//错误状态
#define GPRS_ERR_UNK   	   -1	//还没有初始化
#define GPRS_ERR_OK			0	//没有错误
#define GPRS_ERR_RST		1	//复位模块失败
#define GPRS_ERR_SIM		2	//检测SIM卡失败
#define GPRS_ERR_REG		3	//注册网络失败
#define GPRS_ERR_PPP		4	//拨号失败
#define GPRS_ERR_AUTH		5	//PPP认证失败
#define GPRS_ERR_IP			6	//获取IP失败
#define GPRS_ERR_CON		7	//连接主站失败

#endif //PROIFCONST_H
