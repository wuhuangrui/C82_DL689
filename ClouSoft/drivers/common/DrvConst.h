/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：Lcd.h
 * 摘    要：驱动用到一些常量的定义
 *
 * 当前版本：1.0
 * 作    者：杨进
 * 完成日期：2009-07
 *
 * 取代版本：
 * 原 作 者：
 * 完成日期：
************************************************************************************************************/
#ifndef DRVCONST_H
#define DRVCONST_H

#define DRIVER_VER_STR	  "Ver1.3.9s"  //

#define YK_MODE_LEVEL	0   //电平输出方式
#define YK_MODE_PULSE	1	//脉冲输出方式


#define DC_VAL_CLKBAT	0
#define DC_VAL_GPRSBAT	1
#define DC_VAL_INNDC1	2
#define DC_VAL_INNDC2	3
#define DC_VAL_OUTDC1	4
#define DC_VAL_OUTDC2	5
#define DC_VAL_OUTDC3	6
#define DC_VAL_OUTDC4	7
#define DC_VAL_OUTDC5	8
#define DC_VAL_OUTDC6	9
#define DC_VAL_OUTDC7	10
#define DC_VAL_OUTDC8	11
//下面是ARM7(AT91SAM7SE32)片内的AD
#define DC_VAL_ARM7DC0	0
#define DC_VAL_ARM7DC1	1
#define DC_VAL_ARM7DC2	2
#define DC_VAL_ARM7DC3	3
#define DC_VAL_ARM7CLKBAT	4//时钟电池电压（在C2表上没有时钟电池，用作主电源电压检测）
#define DC_VAL_ARM7GPRSBAT	5//GPRS电池电压检测
#define DC_VAL_ARM7PWRAC	6
#define DC_VAL_ARM7DC7	7

//浙江版兼容的模块定义
#define MODULE_UNKNOWN 		0xff
#define MODULE_GR47			0
#define MODULE_SIM100		1
#define MODULE_WAVECOM		2
#define MODULE_HUAWEI		3
#define MODULE_GR47_15		4
#define MODULE_MC39			5
#define MODULE_MG815		6
#define MODULE_CX06833		7
#define MODULE_SOCKET		8
#define MODULE_MC39_NEW 	9
#define MODULE_HWDTU    	10
#define MODULE_GC864    	11
#define MODULE_ME3000   	12
#define MODULE_R230M		13
#define MODULE_EF0306		15
#define MODULE_M580I		16
#define MODULE_LC6311		17
#define MODULE_LC6311_2G 	18
#define MODULE_MC8331A 		19
#define MODULE_MC37I	 	20
#define MODULE_M580Z	 	21
#define MODULE_ME590	    22
#define MODULE_CX068332		23
#define MODULE_CM180		24
#define MODULE_GL868		25
#define MODULE_G600			26
#define MODULE_FC206		27//高速电台
#define MODULE_MC323		28//华为GPRS模块
#define MODULE_MC39_TEMPOFF	29//MC37/39关闭低温保护
#define MODULE_MC2106		30
#define MODULE_GM650		32
#define MODULE_GC864_2		253
#define MODULE_GC864_1		254

#define HW_CL790D82	0

#endif

