#include "stdafx.h"
#include "apptypedef.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "FaAPI.h"
#include "sysfs.h"
#include "DbAPI.h"
#include "DL645Fmt.h"
//#include "V07Task.h"
#include "AcConst.h"

#ifdef EN_INMTR

DWORD g_dwAmpMS[3]={0, 0, 0};


T07To97IDCfg g_07to97IdMap[]= 
{
	//电能
	{	BN_645_DATA,		0x00000000,		0xe000,		NULL},

	{	BN_645_DATA,		0x00010000,		0x9010,		NULL},
	{	BN_645_DATA,		0x00020000,		0x9020,		NULL},
	{	BN_645_DATA,		0x00030000,		0x9110,		NULL},
	{	BN_645_DATA,		0x00040000,		0x9120,		NULL},

	{	BN_645_DATA,		0x00050000,		0x9130,		NULL},
	{	BN_645_DATA,		0x00060000,		0x9150,		NULL},
	{	BN_645_DATA,		0x00070000,		0x9160,		NULL},
	{	BN_645_DATA,		0x00080000,		0x9140,		NULL},

	//上X结算日电能
	{	BN_645_DATA,		0x00000001,		0x6080,		MONTH_ENERGY_FILE},

	{	BN_645_DATA,		0x00010001,		0x6000,		MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00020001,		0x6020,		MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00030001,		0x6010,		MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00040001,		0x6030,		MONTH_ENERGY_FILE},

	{	BN_645_DATA,		0x00050001,		0x6040,		MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00060001,		0x6060,		MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00070001,		0x6070,		MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00080001,		0x6050,		MONTH_ENERGY_FILE},
	
	//A相电能
	{	BN_645_DATA,		0x00150000,		0x9070,		NULL},
	{	BN_645_DATA,		0x00160000,		0x9080,		NULL},
	{	BN_645_DATA,		0x00170000,		0xe112,		NULL},
	{	BN_645_DATA,		0x00180000,		0xe113,		NULL},
	{	BN_645_DATA,		0x00190000,		0xe114,		NULL},
	{	BN_645_DATA,		0x001a0000,		0xe115,		NULL},
	{	BN_645_DATA,		0x001b0000,		0xe116,		NULL},
	{	BN_645_DATA,		0x001c0000,		0xe117,		NULL},
	
	//上X结算日A相电能
	{	BN_645_DATA,		0x00150001,		0x4000,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00160001,		0x4020,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00170001,		0x4010,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00180001,		0x4030,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00190001,		0x4040,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x001a0001,		0x4060,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x001b0001,		0x4070,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x001c0001,		0x4050,		PHASE_MONTH_ENERGY_FILE},

	//B相电能
	{	BN_645_DATA,		0x00290000,		0x9071,		NULL},
	{	BN_645_DATA,		0x002a0000,		0x9081,		NULL},
	{	BN_645_DATA,		0x002b0000,		0xe122,		NULL},
	{	BN_645_DATA,		0x002c0000,		0xe123,		NULL},
	{	BN_645_DATA,		0x002d0000,		0xe124,		NULL},
	{	BN_645_DATA,		0x002e0000,		0xe125,		NULL},
	{	BN_645_DATA,		0x002f0000,		0xe126,		NULL},
	{	BN_645_DATA,		0x00300000,		0xe127,		NULL},

	//上X结算日B相电能
	{	BN_645_DATA,		0x00290001,		0x4001,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x002a0001,		0x4021,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x002b0001,		0x4011,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x002c0001,		0x4031,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x002d0001,		0x4041,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x002e0001,		0x4061,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x002f0001,		0x4071,		PHASE_MONTH_ENERGY_FILE},
	{	BN_645_DATA,		0x00300001,		0x4051,		PHASE_MONTH_ENERGY_FILE},

	//C相电能
	{	BN_645_DATA,		0x003d0000,		0x9072,		NULL},
	{	BN_645_DATA,		0x003e0000,		0x9082,		NULL},
	{	BN_645_DATA,		0x003f0000,		0xe132,		NULL},
	{	BN_645_DATA,		0x00400000,		0xe133,		NULL},
	{	BN_645_DATA,		0x00410000,		0xe134,		NULL},
	{	BN_645_DATA,		0x00420000,		0xe135,		NULL},
	{	BN_645_DATA,		0x00430000,		0xe136,		NULL},
	{	BN_645_DATA,		0x00440000,		0xe137,		NULL},

	//上X结算日C相电能
	{	BN_645_DATA,		0x003d0001,		0x4002,		PHASE_MONTH_ENERGY_FILE},	//C相正向有功电能  0x4002
	{	BN_645_DATA,		0x003e0001,		0x4022,		PHASE_MONTH_ENERGY_FILE},	//C相反向有功电能  0x4022
	{	BN_645_DATA,		0x003f0001,		0x4012,		PHASE_MONTH_ENERGY_FILE},	//C相组合无功1电能 0x4012
	{	BN_645_DATA,		0x00400001,		0x4032,		PHASE_MONTH_ENERGY_FILE},	//C相组合无功2电能 0x4032
	{	BN_645_DATA,		0x00410001,		0x4042,		PHASE_MONTH_ENERGY_FILE},	//C相第一象限无功电能 0x4042
	{	BN_645_DATA,		0x00420001,		0x4062,		PHASE_MONTH_ENERGY_FILE},	//C相第二象限无功电能 0x4062
	{	BN_645_DATA,		0x00430001,		0x4072,		PHASE_MONTH_ENERGY_FILE},	//C相第三象限无功电能 0x4072
	{	BN_645_DATA,		0x00440001,		0x4052,		PHASE_MONTH_ENERGY_FILE},	//C相第四象限无功电能 0x4052
	
	//需量及发生时间
	{	BN_645_DATA,		0x01010000,		0xa010,		NULL},
	{	BN_645_DATA,		0x01020000,		0xa020,		NULL},
	{	BN_645_DATA,		0x01030000,		0xa110,		NULL},
	{	BN_645_DATA,		0x01040000,		0xa120,		NULL},

	{	BN_645_DATA,		0x01050000,		0xa130,		NULL},
	{	BN_645_DATA,		0x01060000,		0xa150,		NULL},
	{	BN_645_DATA,		0x01070000,		0xa160,		NULL},
	{	BN_645_DATA,		0x01080000,		0xa140,		NULL},

	//上X结算日需量及发生时间
	{	BN_645_DATA,		0x01010001,		0x7000,		MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01020001,		0x7020,		MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01030001,		0x7010,		MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01040001,		0x7030,		MONTH_DEMAND_FILE},

	{	BN_645_DATA,		0x01050001,		0x7040,		MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01060001,		0x7060,		MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01070001,		0x7070,		MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01080001,		0x7050,		MONTH_DEMAND_FILE},

	//A相需量及发生时间
	{	BN_645_DATA,		0x01150000,		0xe810,		NULL},
	{	BN_645_DATA,		0x01160000,		0xe811,		NULL},
	{	BN_645_DATA,		0x01170000,		0xe812,		NULL},
	{	BN_645_DATA,		0x01180000,		0xe813,		NULL},

	{	BN_645_DATA,		0x01190000,		0xe814,		NULL},
	{	BN_645_DATA,		0x011a0000,		0xe815,		NULL},
	{	BN_645_DATA,		0x011b0000,		0xe816,		NULL},
	{	BN_645_DATA,		0x011c0000,		0xe817,		NULL},

	//B相需量及发生时间
	{	BN_645_DATA,		0x01290000,		0xe820,		NULL},
	{	BN_645_DATA,		0x012a0000,		0xe821,		NULL},
	{	BN_645_DATA,		0x012b0000,		0xe822,		NULL},
	{	BN_645_DATA,		0x012c0000,		0xe823,		NULL},

	{	BN_645_DATA,		0x012d0000,		0xe824,		NULL},
	{	BN_645_DATA,		0x012e0000,		0xe825,		NULL},
	{	BN_645_DATA,		0x012f0000,		0xe826,		NULL},
	{	BN_645_DATA,		0x01300000,		0xe827,		NULL},

	//C相需量及发生时间
	{	BN_645_DATA,		0x013d0000,		0xe830,		NULL},
	{	BN_645_DATA,		0x013e0000,		0xe831,		NULL},
	{	BN_645_DATA,		0x013f0000,		0xe832,		NULL},
	{	BN_645_DATA,		0x01400000,		0xe833,		NULL},

	{	BN_645_DATA,		0x01410000,		0xe834,		NULL},
	{	BN_645_DATA,		0x01420000,		0xe835,		NULL},
	{	BN_645_DATA,		0x01430000,		0xe836,		NULL},
	{	BN_645_DATA,		0x01440000,		0xe837,		NULL},


	//上月A相需量及发生时间
	{	BN_645_DATA,		0x01150001,		0x2000,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01160001,		0x2020,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01170001,		0x2010,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01180001,		0x2030,		PHASE_MONTH_DEMAND_FILE},

	{	BN_645_DATA,		0x01190001,		0x2040,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x011a0001,		0x2060,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x011b0001,		0x2070,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x011c0001,		0x2050,		PHASE_MONTH_DEMAND_FILE},

	//上月B相需量及发生时间
	{	BN_645_DATA,		0x01290001,		0x2001,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x012a0001,		0x2021,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x012b0001,		0x2011,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x012c0001,		0x2031,		PHASE_MONTH_DEMAND_FILE},

	{	BN_645_DATA,		0x012d0001,		0x2041,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x012e0001,		0x2061,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x012f0001,		0x2071,		PHASE_MONTH_DEMAND_FILE},
	{	BN_645_DATA,		0x01300001,		0x2051,		PHASE_MONTH_DEMAND_FILE},

	//上月C相需量及发生时间
	{	BN_645_DATA,		0x013d0001,		0x2002,		NULL},
	{	BN_645_DATA,		0x013e0001,		0x2022,		NULL},
	{	BN_645_DATA,		0x013f0001,		0x2012,		NULL},
	{	BN_645_DATA,		0x01400001,		0x2032,		NULL},

	{	BN_645_DATA,		0x01410001,		0x2042,		NULL},
	{	BN_645_DATA,		0x01420001,		0x2062,		NULL},
	{	BN_645_DATA,		0x01430001,		0x2072,		NULL},
	{	BN_645_DATA,		0x01440001,		0x2052,		NULL},

	//电压
	{	BN_645_DATA,		0x02010100,		0xb611,		NULL},
	{	BN_645_DATA,		0x02010200,		0xb612,		NULL},
	{	BN_645_DATA,		0x02010300,		0xb613,		NULL},
	{	BN_645_DATA,		0x0201FF00,		0xb61f,		NULL},

	//电流
	{	BN_645_DATA,		0x02020100,		0xb621,		NULL},
	{	BN_645_DATA,		0x02020200,		0xb622,		NULL},
	{	BN_645_DATA,		0x02020300,		0xb623,		NULL},
	{	BN_645_DATA,		0x0202FF00,		0xb62f,		NULL},

	//有功功率
	{	BN_645_DATA,		0x02030000,		0xb630,		NULL},
	{	BN_645_DATA,		0x02030100,		0xb631,		NULL},
	{	BN_645_DATA,		0x02030200,		0xb632,		NULL},
	{	BN_645_DATA,		0x02030300,		0xb633,		NULL},
	{	BN_645_DATA,		0x0203FF00,		0xb63f,		NULL},

	//无功功率
	{	BN_645_DATA,		0x02040000,		0xb640,		NULL},
	{	BN_645_DATA,		0x02040100,		0xb641,		NULL},
	{	BN_645_DATA,		0x02040200,		0xb642,		NULL},
	{	BN_645_DATA,		0x02040300,		0xb643,		NULL},
	{	BN_645_DATA,		0x0204FF00,		0xb64f,		NULL},

	//视在功率
	{	BN_645_DATA,		0x02050000,		0xb670,		NULL},
	{	BN_645_DATA,		0x02050100,		0xb671,		NULL},
	{	BN_645_DATA,		0x02050200,		0xb672,		NULL},
	{	BN_645_DATA,		0x02050300,		0xb673,		NULL},
	{	BN_645_DATA,		0x0205FF00,		0xb67f,		NULL},

	//功率因数
	{	BN_645_DATA,		0x02060000,		0xb650,		NULL},
	{	BN_645_DATA,		0x02060100,		0xb651,		NULL},
	{	BN_645_DATA,		0x02060200,		0xb652,		NULL},
	{	BN_645_DATA,		0x02060300,		0xb653,		NULL},
	{	BN_645_DATA,		0x0206FF00,		0xb65f,		NULL},

	{	BN2,				0x02800002,		0x1054,		NULL},	//电网频率

	/////////////////////////////////////////////////////////////
	//事件统计
	{	BN_645_DATA, 		0x10000001,		0xea10,		NULL}, //失压总次数
	{	BN_645_DATA, 		0x10000002,		0xea11,		NULL}, //失压总累计时间
	{	BN_645_DATA, 		0x10000101,		0xea12,		NULL}, //最近1次失压发生时刻
	{	BN_645_DATA, 		0x10000201,		0xea13,		NULL}, //最近1次失压结束时刻

	{	BN_645_DATA, 		0x10010001,		0xea14,		NULL}, //A相失压总次数
	{	BN_645_DATA, 		0x10010002,		0xea15,		NULL}, //A相失压总累计时间
	{	BN_645_DATA, 		0x10020001,		0xea16,		NULL}, //B相失压总次数
	{	BN_645_DATA, 		0x10020002,		0xea17,		NULL}, //B相失压总累计时间
	{	BN_645_DATA, 		0x10030001,		0xea18,		NULL}, //C相失压总次数
	{	BN_645_DATA, 		0x10030002,		0xea19,		NULL}, //C相失压总累计时间

	{	BN_645_DATA, 		0x03050000,		0xea20,		NULL}, //全失压总次数，总累计时间 XXXXXX，XXXXXX
	{	BN_645_DATA, 		0x14000001,		0xea21,		NULL}, //电压逆相序总次数
	{	BN_645_DATA, 		0x14000002,		0xea22,		NULL}, //电压逆相序总次数															   
	{	BN_645_DATA, 		0x15000001,		0xea23,		NULL}, //电流逆相序总次数
	{	BN_645_DATA, 		0x15000002,		0xea24,		NULL}, //电流逆相序总累计时间
	{	BN_645_DATA, 		0x03300200,		0xea25,		NULL}, //需量清零总次数
	{	BN_645_DATA, 		0x03300100,		0xea26,		NULL}, //电表清零总次数
	{	BN_645_DATA, 		0x03300000,		0xea27,		NULL}, //编程总次数
	{	BN_645_DATA, 		0x03300400,		0xea28,		NULL}, //校时总次数

	{	BN_645_DATA, 		0x13010001,		0xea30,		NULL}, //A相断相总次数
	{	BN_645_DATA, 		0x13010002,		0xea31,		NULL}, //A相断相总累计时间
	{	BN_645_DATA, 		0x13020001,		0xea32,		NULL}, //B相断相总次数
	{	BN_645_DATA, 		0x13020002,		0xea33,		NULL}, //B相断相总累计时间
	{	BN_645_DATA, 		0x13030001,		0xea34,		NULL}, //C相断相总次数
	{	BN_645_DATA, 		0x13030002,		0xea35,		NULL}, //C相断相总累计时间

	{	BN_645_DATA, 		0x18010001,		0xea40,		NULL}, //A相失流总次数
	{	BN_645_DATA, 		0x18010002,		0xea41,		NULL}, //A相失流总累计时间
	{	BN_645_DATA, 		0x18020001,		0xea42,		NULL}, //B相失流总次数
	{	BN_645_DATA, 		0x18020002,		0xea43,		NULL}, //B相失流总累计时间
	{	BN_645_DATA, 		0x18030001,		0xea44,		NULL}, //C相失流总次数
	{	BN_645_DATA, 		0x18030002,		0xea45,		NULL}, //C相失流总累计时间

	{	BN_645_DATA, 		0x1c010001,		0xea50,		NULL}, //A相过载总次数
	{	BN_645_DATA, 		0x1c010002,		0xea51,		NULL}, //A相过载总累计时间
	{	BN_645_DATA, 		0x1c020001,		0xea52,		NULL}, //B相过载总次数
	{	BN_645_DATA, 		0x1c020002,		0xea53,		NULL}, //B相过载总累计时间
	{	BN_645_DATA, 		0x1c030001,		0xea54,		NULL}, //C相过载总次数
	{	BN_645_DATA, 		0x1c030002,		0xea55,		NULL}, //C相过载总累计时间

	///////////////////////////////////////////////////////////////////////////
	//参变量
	{	BN_645_DATA,		0x04000101,		0xc010,		NULL},//日期
	{	BN_645_DATA,		0x04000102,		0xc011,		NULL},//时间
	{	BN_645_PARA,		0x04000103,		0xc111,		NULL},//最大需量周期
	{	BN_645_PARA,		0x04000104,		0xc112,		NULL},//滑差时间
	{	BN_645_PARA,		0x04000105,		0x0640,		NULL},//校表脉冲宽度
	{	BN_645_PARA,		0x04000106,		0x0641,		NULL},//两套时区表切换时间
	{	BN_645_PARA,		0x04000107,		0x0642,		NULL}, //两套日时段表切换时间

	{	BN_645_PARA,		0x04000201,		0xc310,		NULL},//年时区数
	{	BN_645_PARA,		0x04000202,		0xc311,		NULL},//日时段表叔
	{	BN_645_PARA,		0x04000203,		0xc312,		NULL},//日时段
	{	BN_645_PARA,		0x04000204,		0xc313,		NULL},//费率数
	{	BN_645_PARA,		0x04000205,		0xc855,		NULL},//公共假日数
	
	{	BN_645_PARA,		0x04000302,		0xc113,		NULL},//循显每屏显示时间
	{	BN_645_PARA,		0x04000303,		0xc115,		NULL},//显示电能小数位	
	{	BN_645_PARA,		0x04000304,		0xc116,		NULL},//显示功率(最大需量)小数位数 

	{	BN_645_PARA,		0x04000401,		0x0610,		NULL},//通信地址	//0xc033
	{	BN_645_PARA,		0x04000402,		0xc032,		NULL},//表号
	{	BN_645_PARA,		0x04000403,		0x0611,		NULL},//资产管理编码(ASCII码)
	{	BN_645_PARA,		0x04000404,		0x0612,		NULL},//额定电压(ASCII码)
	{	BN_645_PARA,		0x04000405,		0x0613,		NULL},//额定电流/基本电流(ASCII码)
	{	BN_645_PARA,		0x04000406,		0x0614,		NULL},//最大电流(ASCII码)
	{	BN_645_PARA,		0x04000407,		0x0615,		NULL},//有功准确度等级(ASCII码)
	{	BN_645_PARA,		0x04000408,		0x0616,		NULL},//无功准确度等级(ASCII码)
	{	BN_645_PARA,		0x04000409,		0xc030,		NULL},//电表有功常数
	{	BN_645_PARA,		0x0400040a,		0xc031,		NULL},//电表无功常数
	{	BN_645_PARA,		0x0400040b,		0x0617,		NULL},//电表型号(ASCII码)
	{	BN_645_PARA,		0x0400040c,		0x0618,		NULL},//生产日期(ASCII码)
	{	BN_645_PARA,		0x0400040d,		0x0619,		NULL},//协议版本号(ASCII码)

	{	BN_645_DATA,		0x04000501,		0xc860,		NULL},//电表状态字1
	{	BN_645_DATA,		0x04000502,		0xc861,		NULL},//电表状态字2
	{	BN_645_DATA,		0x04000503,		0xc862,		NULL},//电表状态字3
	{	BN_645_DATA,		0x04000504,		0xc863,		NULL},//电表状态字4
	{	BN_645_DATA,		0x04000505,		0xc864,		NULL},//电表状态字5
	{	BN_645_DATA,		0x04000506,		0xc865,		NULL},//电表状态字6
	{	BN_645_DATA,		0x04000507,		0xc866,		NULL},//电表状态字7

	{	BN_645_PARA,		0x04000601,		0x0620,		NULL},//有功组合方式
	{	BN_645_PARA,		0x04000602,		0x0621,		NULL},//无功组合方式1
	{	BN_645_PARA,		0x04000603,		0x0622,		NULL},//无功组合方式2

	{	BN_645_DATA,		0x04000701,		0x0660,		NULL},//调制型红外光口通信速率特征字
	{	BN_645_DATA,		0x04000702,		0x0661,		NULL},//接触式红外光口通信速率特征字
	{	BN_645_DATA,		0x04000703,		0x0662,		NULL},//通信口1通信速率特征字
	{	BN_645_DATA,		0x04000704,		0x0663,		NULL},//通信口2通信速率特征字
	{	BN_645_DATA,		0x04000705,		0x0664,		NULL},//通信口3通信速率特征字

	{	BN_645_PARA,		0x04000801,		0xc022,		NULL},//周休日状态字
	{	BN_645_PARA,		0x04000802,		0xc41e,		NULL},//周休日采用的日时段表号

	/*{	BN_645_PARA,		0x04000901,		0x0623,		NULL},//负荷记录模式字
	{	BN_645_PARA,		0x04000902,		0x0624,		NULL},//冻结数据模式字

	{	BN_645_PARA,		0x04000a01,		0x0630,		NULL},//负荷记录起始时间
	{	BN_645_PARA,		0x04000a02,		0x0631,		NULL},//第1类负荷记录间隔时间
	{	BN_645_PARA,		0x04000a03,		0x0632,		NULL},//第2类负荷记录间隔时间
	{	BN_645_PARA,		0x04000a04,		0x0633,		NULL},//第3类负荷记录间隔时间
	{	BN_645_PARA,		0x04000a05,		0x0634,		NULL},//第4类负荷记录间隔时间
	{	BN_645_PARA,		0x04000a06,		0x0635,		NULL},//第5类负荷记录间隔时间
	{	BN_645_PARA,		0x04000a07,		0x0636,		NULL},//第6类负荷记录间隔时间
*/
	{	BN_645_PARA,		0x04000b01,		0x0650,		NULL},//每月第1结算日  //0xc117
	{	BN_645_PARA,		0x04000b02,		0x0651,		NULL},//每月第2结算日
	{	BN_645_PARA,		0x04000b03,		0x0652,		NULL},//每月第3结算日	

	{	BN_645_PARA,		0x04010000,		0xc32f,		NULL},//第一套第时区表起始日期及时段表号
	{	BN_645_PARA,		0x04010001,		0xc33f,		NULL},//第一套第一日时段表第1时段起始时间及费率号	
	{	BN_645_PARA,		0x04010002,		0xc34f,		NULL},//第一套第二日时段表第1时段起始时间及费率号	
	{	BN_645_PARA,		0x04010003,		0xc35f,		NULL},//第一套第三日时段表第1时段起始时间及费率号	
	{	BN_645_PARA,		0x04010004,		0xc36f,		NULL},//第一套第四日时段表第1时段起始时间及费率号	
	{	BN_645_PARA,		0x04010005,		0xc37f,		NULL},//第一套第五日时段表第1时段起始时间及费率号	
	{	BN_645_PARA,		0x04010006,		0xc38f,		NULL},//第一套第六日时段表第1时段起始时间及费率号	
	{	BN_645_PARA,		0x04010007,		0xc39f,		NULL},//第一套第七日时段表第1时段起始时间及费率号	
	{	BN_645_PARA,		0x04010008,		0xc3af,		NULL},//第一套第八日时段表第1时段起始时间及费率号

	{	BN_645_PARA,		0x04030001,		0xc861,		NULL},//第1公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030002,		0xc862,		NULL},//第2公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030003,		0xc863,		NULL},//第3公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030004,		0xc864,		NULL},//第4公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030005,		0xc865,		NULL},//第5公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030006,		0xc866,		NULL},//第6公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030007,		0xc867,		NULL},//第7公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030008,		0xc868,		NULL},//第8公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030009,		0xc869,		NULL},//第9公共假日日期及日时段表号
	{	BN_645_PARA,		0x0403000a,		0xc86a,		NULL},//第10公共假日日期及日时段表号
	{	BN_645_PARA,		0x0403000b,		0xc86b,		NULL},//第11公共假日日期及日时段表号
	{	BN_645_PARA,		0x0403000c,		0xc86c,		NULL},//第12公共假日日期及日时段表号
	{	BN_645_PARA,		0x0403000d,		0xc86d,		NULL},//第13公共假日日期及日时段表号
	{	BN_645_PARA,		0x0403000e,		0xc86e,		NULL},//第14公共假日日期及日时段表号

	{	BN_645_PARA,		0x0403000f,		0xc871,		NULL},//第15公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030010,		0xc872,		NULL},//第16公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030011,		0xc873,		NULL},//第17公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030012,		0xc874,		NULL},//第18公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030013,		0xc875,		NULL},//第19公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030014,		0xc876,		NULL},//第20公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030015,		0xc877,		NULL},//第21公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030016,		0xc878,		NULL},//第22公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030017,		0xc879,		NULL},//第23公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030018,		0xc87a,		NULL},//第24公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030019,		0xc87b,		NULL},//第25公共假日日期及日时段表号
	{	BN_645_PARA,		0x0403001a,		0xc87c,		NULL},//第26公共假日日期及日时段表号
	{	BN_645_PARA,		0x0403001b,		0xc87d,		NULL},//第27公共假日日期及日时段表号
	{	BN_645_PARA,		0x0403001c,		0xc87e,		NULL},//第28公共假日日期及日时段表号

	{	BN_645_PARA,		0x0403001d,		0xc881,		NULL},//第29公共假日日期及日时段表号
	{	BN_645_PARA,		0x0403001e,		0xc882,		NULL},//第30公共假日日期及日时段表号
	{	BN_645_PARA,		0x0403001f,		0xc883,		NULL},//第31公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030020,		0xc884,		NULL},//第32公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030021,		0xc885,		NULL},//第33公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030022,		0xc886,		NULL},//第34公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030023,		0xc887,		NULL},//第35公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030024,		0xc888,		NULL},//第36公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030025,		0xc889,		NULL},//第37公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030026,		0xc88a,		NULL},//第38公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030027,		0xc88b,		NULL},//第39公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030028,		0xc88c,		NULL},//第40公共假日日期及日时段表号
	{	BN_645_PARA,		0x04030029,		0xc88d,		NULL},//第41公共假日日期及日时段表号
	{	BN_645_PARA,		0x0403002a,		0xc88e,		NULL},//第42公共假日日期及日时段表号

	{	BN_645_PARA,		0x04800001,		0x0643,		NULL},//厂家软件版本号(ASCII码)
	{	BN_645_PARA,		0x04800002,		0x0644,		NULL},//厂家硬件版本号(ASCII码)
	{	BN_645_PARA,		0x04800003,		0x0645,		NULL},//厂家编号(ASCII码)

	{	BN_645_PARA,		0x04090101,		0xc810,		NULL},//失压事件电压触发上限
	{	BN_645_PARA,		0x04090102,		0xc811,		NULL},//失压事件电压恢复下限
	{	BN_645_PARA,		0x04090103,		0xc812,		NULL},//失压事件电流触发下限
	{	BN_645_PARA,		0x04090104,		0xc813,		NULL},//失压事件判定延时时间

	{	BN_645_PARA,		0x04090201,		0xc814,		NULL},//欠压事件电压触发上限
	{	BN_645_PARA,		0x04090202,		0xc815,		NULL},//欠压事件判定延时时间

	{	BN_645_PARA,		0x04090301,		0xc816,		NULL},//过压事件电压触发下限
	{	BN_645_PARA,		0x04090302,		0xc817,		NULL},//过压事件判定延时时间

	{	BN_645_PARA,		0x04090401,		0xc818,		NULL},//断相事件电压触发上限          
	{	BN_645_PARA,		0x04090402,		0xc819,		NULL},//断相事件电流触发上限          
	{	BN_645_PARA,		0x04090403,		0xc81a,		NULL},//断相事件判定延时时间          
                                                                                  
	{	BN_645_PARA,		0x04090501,		0xc81b,		NULL},//电压不平衡率限值              
	{	BN_645_PARA,		0x04090502,		0xc81c,		NULL},//电压不平衡率判定延时时间      
                                                                                  
	{	BN_645_PARA,		0x04090601,		0xc81d,		NULL},//电流不平衡率限值              
	{	BN_645_PARA,		0x04090602,		0xc81e,		NULL},//电流不平衡率判定延时时间      
                                                                                  
	{	BN_645_PARA,		0x04090701,		0xc820,		NULL},//失流事件电压触发下限          
	{	BN_645_PARA,		0x04090702,		0xc821,		NULL},//失流事件电流触发上限          
	{	BN_645_PARA,		0x04090703,		0xc822,		NULL},//失流事件电流触发下限          
	{	BN_645_PARA,		0x04090704,		0xc823,		NULL},//失流事件判定延时时间          
                                                                                  
	{	BN_645_PARA,		0x04090801,		0xc824,		NULL},//过流事件电流触发下限          
	{	BN_645_PARA,		0x04090802,		0xc825,		NULL},//过流事件判定延时时间          
                                                                                  
	{	BN_645_PARA,		0x04090901,		0xc826,		NULL},//断流事件电压触发下限          
	{	BN_645_PARA,		0x04090902,		0xc827,		NULL},//断流事件电流触发上限          
	{	BN_645_PARA,		0x04090903,		0xc828,		NULL},//断流事件判定延时时间          
                                                                                  
	{	BN_645_PARA,		0x04090a01,		0xc829,		NULL},//潮流反向事件有功功率触发下限  
	{	BN_645_PARA,		0x04090a02,		0xc82a,		NULL},//潮流反向事件判定延时时间      
                                                                                  
	{	BN_645_PARA,		0x04090b01,		0xc82b,		NULL},//过载事件有功功率触发下限      
	{	BN_645_PARA,		0x04090b02,		0xc82c,		NULL},//过载事件判定延时时间          
                                                                                  
	{	BN_645_PARA,		0x04090c01,		0xc82d,		NULL},//电压考核上限                  
	{	BN_645_PARA,		0x04090c02,		0xc82e,		NULL},//电压考核下限                  
                                                                                  
	{	BN_645_PARA,		0x04090d01,		0xc830,		NULL},//有功需量超限事件需量触发下限  
	{	BN_645_PARA,		0x04090d02,		0xc831,		NULL},//无功需量超限事件需量触发下限  
	{	BN_645_PARA,		0x04090d03,		0xc832,		NULL},//需量超限事件判定延时时间      
                                                                                  
	{	BN_645_PARA,		0x04090e01,		0xc833,		NULL},//总功率因数超下限阀值          
	{	BN_645_PARA,		0x04090e02,		0xc834,		NULL},//总功率因数超下限判定延时时间  
                                                                                  
	{	BN_645_PARA,		0x04090f01,		0xc835,		NULL},//电流严重不平衡限值            
	{	BN_645_PARA,		0x04090f02,		0xc836,		NULL},//电流严重不平衡触发延时时间    

	{	BN_645_PARA,		0x04000901,		0xc840,		NULL},//负荷记录模式字
	//{	BN_645_PARA,		0x04000902,		0xc841,		NULL},//冻结数据模式字
	{	BN_645_PARA,		0x04000a01,		0xc842,		NULL},//负荷记录起始时间
	{	BN_645_PARA,		0x04000a02,		0xc843,		NULL},//第1类负荷记录间隔时间
	{	BN_645_PARA,		0x04000a03,		0xc844,		NULL},//第2类负荷记录间隔时间
	{	BN_645_PARA,		0x04000a04,		0xc845,		NULL},//第3类负荷记录间隔时间
	{	BN_645_PARA,		0x04000a05,		0xc846,		NULL},//第4类负荷记录间隔时间
	{	BN_645_PARA,		0x04000a06,		0xc847,		NULL},//第5类负荷记录间隔时间
	{	BN_645_PARA,		0x04000a07,		0xc848,		NULL},//第6类负荷记录间隔时间

	{	BN_645_PARA,		0x04000902,		0xc849,		NULL},//定时冻结数据模式字
	{	BN_645_PARA,		0x04000903,		0xc84a,		NULL},//瞬时冻结数据模式字
	{	BN_645_PARA,		0x04000904,		0xc84b,		NULL},//约定冻结数据模式字
	{	BN_645_PARA,		0x04000905,		0xc84c,		NULL},//整点冻结数据模式字
	{	BN_645_PARA,		0x04000906,		0xc84d,		NULL},//日冻结数据模式字

	{	BN_645_PARA,		0x04001201,		0xc850,		NULL},//整点冻结起始时间
	{	BN_645_PARA,		0x04001202,		0xc851,		NULL},//整点冻结时间间隔
	{	BN_645_PARA,		0x04001203,		0xc852,		NULL},//日冻结时间

	{	BN_645_PARA,		0x0280000b,		0xea60,		NULL},//当前阶梯电价
	{	BN_645_PARA,		0x02800020,		0xea61,		NULL},//当前电价
	{	BN_645_PARA,		0x02800021,		0xea62,		NULL},//当前费率电价
};



bool Map07To97ID(DWORD dw07Id, T07To97IDCfg* pIdCfg)
{
	for (WORD i=0; i<sizeof(g_07to97IdMap)/sizeof(T07To97IDCfg); i++)
	{
		if (dw07Id == g_07to97IdMap[i].dw07Id)
		{
			*pIdCfg = g_07to97IdMap[i];
			return true;
		}
	}

	return false;
}

WORD Map97ToBank(DWORD dw97Id)
{
	for (WORD i=0; i<sizeof(g_07to97IdMap)/sizeof(T07To97IDCfg); i++)
	{
		if (dw97Id == g_07to97IdMap[i].w97Id)
			return g_07to97IdMap[i].bBank;	
	}

	return 0;
}

//返回值：>0则返回645格式长度；<=0失败
WORD FmtGBTo645(WORD wBn, WORD wID, BYTE* pbIn, BYTE* pbOut)
{
	WORD wRead=0;
	WORD i, wGBLen, w645Len, wIdNum=1;
	BYTE bTmpBuf[80];
	DWORD dwVal=0;
	bool fBlockID = ((wID&0x000f) == 0x000f);
	if ((wID & 0xf000) == 0x9000 || (wID & 0xf000) == 0xe000)
	{		
		if ((wID >= 0x9000 && wID < 0x9110) || (wID > 0x9400 && wID < 0x9510) || (wID > 0x9800 && wID < 0x9910) || 
			(wID >= 0xe000 && wID < 0xe110) || (wID >= 0xe200 && wID < 0xe210))	//有功
		{
			wGBLen = 5;
			w645Len = 4;
			if (fBlockID == true)
				wIdNum = TOTAL_RATE_NUM;			

			for (i=0; i<wIdNum; i++)		//RATE_NUM+1
			{		    	    
				memcpy(bTmpBuf, pbIn+i*wGBLen+1, w645Len);
				memcpy(pbOut+i*w645Len, bTmpBuf, w645Len);			    	
			}
			wRead = w645Len*wIdNum;		//RATE_NUM+1	    	
		}
		else if ((wID >= 0x9110 && wID <= 0x916f) || (wID >= 0x9510 && wID <= 0x956f) || (wID >= 0x9910 && wID <= 0x996f)
				|| (wID&0xff00) == 0xe100 || (wID&0xff00) == 0xe300)	//无功
		{
			w645Len = 4;
			if (fBlockID)
				wIdNum = TOTAL_RATE_NUM;	//RATE_NUM+1

			memcpy(pbOut, pbIn, w645Len*wIdNum);
			wRead = w645Len*wIdNum;
		}		
	}	
	else if ((wID & 0xff00) == 0xb600)
	{
		if (wID < 0xb620)	//U
		{
			if (fBlockID)
				wGBLen = 2*3;
			else
				wGBLen = 2;

			memcpy(pbOut, pbIn, wGBLen);
			wRead = wGBLen;
		}
		else if (wID < 0xb650)	//I, P
		{		
			wGBLen = 3;
			if (fBlockID == true)
			{
				if (wID < 0xb630)	//I
					wIdNum = 3;
				else
					wIdNum = 4;
			}

			memcpy(pbOut, pbIn, wGBLen*wIdNum);
			for (i=0; i<wIdNum; i++)
				pbOut[wGBLen*i+2] &= 0x7f;

			wRead = wGBLen*wIdNum;
		}
		else if(wID < 0xb660)	//COS
		{
			wGBLen = 2;
			w645Len = 2;
			if (fBlockID)
				wIdNum = 4;

			memcpy(pbOut, pbIn, wGBLen*wIdNum);
			for (i=0; i<wIdNum; i++)
				pbOut[wGBLen*i+1] &= 0x7f;

			wRead = w645Len*wIdNum;
		}
		else if (wID>=0xb670 && wID < 0xb680)	//S
		{
			wGBLen = 3;
			if (fBlockID == true)
				wIdNum = 4;			

			memcpy(pbOut, pbIn, wGBLen*wIdNum);
			for (i=0; i<wIdNum; i++)
				pbOut[wGBLen*i+2] &= 0x7f;

			wRead = wGBLen*wIdNum;
		}
	}

	if (wRead  <= 0)	//格式不需要转换，直接拷贝
	{
		int iLen = GetItemLen(wBn, wID);
		if (iLen > 0)
		{
			memcpy(pbOut, pbIn, iLen);
			wRead = iLen;
		}
	}

	return wRead;
}



/*说明: 获得一条月冻结记录
 *@szTbName:    需打开的文件名;
 *@bPtr: 要获得上次哪个月的冻结数据,从1开始;
 *@pbBuf:       存冻结数据缓冲区;
 *@iLen:        数据缓冲区大小;
 返回值: <=0错误; >0获得数据的大小
*/
int ReadLastNRec(char* szTbName, BYTE bPtr, BYTE* pbBuf, int iLen)
{
	if (iLen <= 0)
		return -1;
		
	int fd = TdbOpenTable(szTbName, O_RDONLY);
	if (fd < 0)
	{
		DTRACE(DB_TASK, ("ReadFrzRec: fail to open table:%s.\n", szTbName));
   		//return -2;
		memset(pbBuf, 0, iLen);	//INVALID_645_DATA
		return iLen;
	}
	
    int iRet, iIdx;
	iIdx = GetRecIdx(fd, bPtr);
	if (iIdx < 0)
		memset(pbBuf, 0, iLen);	//INVALID_645_DATA
	else
	{
		iRet = TdbReadRec(fd, iIdx, pbBuf, iLen);
		if (iRet <= 0)
		{
			DTRACE(DB_TASK, ("ReadFrzRec: TdbReadRec fail! bPtr:%d, iIdx:%d, iRet:%d.\r\n", bPtr, iIdx, iRet));
			memset(pbBuf, INVALID_645_DATA, iLen);
		}
	}	
	TdbCloseTable(fd);
	
	return iLen;
}


/*说明:获取一条记录的物理地址
 *@szTbName:    需打开的文件名;
 *@bPtr: 要获得上次哪个月的冻结数据,从1开始;
 返回值: <=0错误; >0获得的物理地址
*/
int GetRecPhyIdx(char* szTbName, BYTE bPtr)
{	
	int iIdx;
	int fd = TdbOpenTable(szTbName, O_RDONLY);
	
	if (fd < 0)
	{
		DTRACE(DB_TASK, ("ReadFrzRec: fail to open table:%s.\n", szTbName));
		TdbCloseTable(fd);
		return -1;
	}
	
	iIdx = GetRecIdx(fd, bPtr);
	TdbCloseTable(fd);
	return iIdx;
}
int Read97Id(WORD wBn, WORD wID, BYTE* pbTx)
{
	int iLen = -1;
	BYTE bVer=0;
	BYTE bTmpBuf[30];
	BYTE bBuf[200];

	/*if (IsMonthFrzID(wID, bTmpBuf, 4))	//上第几结算日
	{
		iLen = GetMonthFrzData(bTmpBuf, 4, pbTx);
	}
	else*/
	{
		memset(bBuf, 0, sizeof(bBuf));
		iLen = ReadItemEx(wBn, PN0, wID, bBuf);
		if (iLen > 0)
		{
			iLen = FmtGBTo645(wBn, wID, bBuf, pbTx);	//转为07-645格式
			
			int iRet = ReadItemEx(BN_645_PARA, MTRPN, 0xc890, (BYTE* )&bVer);	//实际时段数
			if (iRet > 0 && bVer == 1)	//天津要求返回实际个数时段费率参数
			{
				if (wBn == BN_645_PARA)
				{
					if ((wID&0xff0f) == 0xc30f && wID > 0xc330)
					{
						BYTE bRateNum=0;
						ReadItemEx(BN_645_PARA, MTRPN, 0xc312, (BYTE* )&bRateNum);	//实际时段数
						bRateNum = BcdToByte(bRateNum);
						if (bRateNum>RATE_PERIOD_NUM || bRateNum==0)
							bRateNum = RATE_PERIOD_NUM;

						iLen = 3*bRateNum;		//天津要求返回实际的时段数 3为1个时段的长度
					}
					else if (wID == 0xc32f)
					{
						BYTE bZoneNum=0;
						ReadItemEx(BN_645_PARA, MTRPN, 0xc310, (BYTE* )&bZoneNum);	//实际年时区数
						//年时区数
						bZoneNum = BcdToByte(bZoneNum);      
						if (bZoneNum > MAX_ZONE_NUM)
							bZoneNum = MAX_ZONE_NUM;

						iLen = 3*bZoneNum;		//天津要求返回实际年时区数
					}
				}	
			}			
		}
		else
		{
			int iItemLen = GetItemLen(wBn, wID);
			if (iItemLen > 0)
			{
				iLen = iItemLen;
				memset(pbTx, INVALID_645_DATA, iItemLen);
			}
		}
	}

	return iLen;
}

//根据描述表中传入的ID
bool IsDemandId(WORD wDesID)
{
	WORD wDmdID[] = { 0xa010, 0xa020, 0xa110, 0xa120, 0xa130, 0xa140, 0xa150, 0xa160,	//当前需量总
					  0x7000, 0x7010, 0x7020, 0x7030, 0x7040, 0x7050, 0x7060, 0x7070,	//上月需量总
					  0xe810, 0xe812, 0xe811, 0xe813, 0xe814, 0xe817, 0xe815, 0xe816, 
					  0xe820, 0xe822, 0xe821, 0xe823, 0xe824, 0xe827, 0xe825, 0xe826, 
					  0xe830, 0xe832, 0xe831, 0xe833, 0xe834, 0xe837, 0xe835, 0xe836,	//当前A/B/C分相需量
					  0x2000, 0x2010, 0x2020, 0x2030, 0x2040, 0x2050, 0x2060, 0x2070,
					  0x2001, 0x2011, 0x2021, 0x2031, 0x2041, 0x2051, 0x2061, 0x2071,
					  0x2002, 0x2012, 0x2022, 0x2032, 0x2042, 0x2052, 0x2062, 0x2072,	//上月A/B/C分相需量
	};

	WORD i;
	for (i=0; i<sizeof(wDmdID)/sizeof(WORD); i++)
	{
		if (wDmdID[i] == wDesID)
			return true;
	}

	return false;
}


//返回值：>0则返回645-1997格式长度 <=0:失败
int Read07Id(DWORD dw07Id, BYTE* pbTx)
{
	int iLen, n645Len=-1;
	BYTE b07Id[4];	
	BYTE bBuf[200];	//
	BYTE bTmpBuf[60];

	if (dw07Id>=0x020a0101 && dw07Id<=0x020b03ff)
	{
		WORD wHarId = 0;
		BYTE n;
		BYTE bPhase = (dw07Id>>8) & 0xff;
		if (bPhase>=1 && bPhase<=3)
		{
			if ((dw07Id&0xffff0000) == 0x020a0000)
				wHarId = 0x2101 + bPhase - 1;
			else if ((dw07Id&0xffff0000) == 0x020b0000)
				wHarId = 0x2104 + bPhase - 1;
		}

		if (wHarId != 0)
		{
			n = (BYTE )(dw07Id&0xff);	//谐波次数
			if (n == 0xff)	//块ID
			{
				return ReadItemEx(BN2, PN0, wHarId, pbTx);
			}
			else if (n>=1 && n<=21)	//块ID
			{
				ReadItemEx(BN2, PN0, wHarId, bTmpBuf);
				memcpy(pbTx, &bTmpBuf[(n-1)*2], 2); 
				return 2;
			}
		}

		//ID不符合让它继续流下去
	}

	iLen = Read07TaskId(dw07Id, pbTx); //如果没有匹配的ID则返回-4
	if (iLen != -4) //有匹配ID，但是没读到数据
		return iLen;

	for (WORD i=0; i<4; i++)
	{
		b07Id[i] = (BYTE ) (dw07Id >> (i<<3)) & 0xff;
	}

	if (b07Id[0] == 0xff || b07Id[2] == 0xff)	//DI[0]和DI[2]块数据暂不支持
		return -1;
	
	if (b07Id[3] < 2 )	//电能和需量ID  //处理分费率及块ID
	{
		dw07Id &= 0xffff00ff;	//分费率及块ID转换后再查映射表
		if (b07Id[0] > 1 && b07Id[0] <= 0xc)	//上X结算日ID转成上一结算日ID查映射表
			dw07Id = (dw07Id & 0xffffff00) + 1;
	}

	T07To97IDCfg IdCfg;
	bool fValidId = Map07To97ID(dw07Id, &IdCfg);
	if (fValidId == false)
		return -1;	//失败处理

	TTime tmNow;
	WORD wBaseID, wIdNum;
	WORD wBn = IdCfg.bBank;
	WORD w97Id = IdCfg.w97Id;
	char* psTdbName = IdCfg.psTdbName;
	BYTE bRateNo = b07Id[1];	//费率号
	BYTE bLastDayPtr = 0;		//第几结算日
	BYTE bVal = 0;
	WORD wDmdTimeId=0;

	if (b07Id[3] < 2 )	//电能和需量ID 分费率及块ID处理
	{
		if (b07Id[2] <= 0x0a)
		{
			if (bRateNo == 0xff)
				w97Id |= 0xf;
			else
				w97Id = (w97Id&0xfff0) + bRateNo;
		}

		bLastDayPtr = b07Id[0];		//第几结算日
		w97Id |= ((WORD ) (bLastDayPtr&0xf) <<8 );
	}

	if (!IsDemandId(IdCfg.w97Id))	//非需量ID
	{
		n645Len = Read97Id(wBn, w97Id, pbTx);
	}
	else	// 需量ID为（需量值＋需量发生时间）组合
	{
		n645Len = 0;
		if (bRateNo == 0xff)
		{
			wIdNum = 5;
			wBaseID = (w97Id&0xfff0);
		}
		else
		{
			wIdNum = 1;
			wBaseID = w97Id;
		}

		for (WORD j=0; j<wIdNum; j++)
		{
			w97Id = wBaseID + j;
			Read97Id(wBn, w97Id, pbTx);
			pbTx += 3;
			n645Len += 3;

			if ((w97Id&0xf000) == 0xe000)
				wDmdTimeId = w97Id+0x100;
			else
				wDmdTimeId = w97Id+0x1000;
			Read97Id(wBn, wDmdTimeId, pbTx);	//需量发生时间(年月日时分) 5Bytes
			if (IsAllAByte(pbTx, 0, 4))	//原来是全0，年继续补0
			{
				pbTx[4] = 0;	//年
			}
			else
			{
				if (!bLastDayPtr)	//当前需量时间数据，补当前年  //上Ｘ结算日需量时间不用补年，在保存记录的时候已经补好年
				{
					GetCurTime(&tmNow);
					pbTx[4] = ByteToBcd(tmNow.nYear%100);	//年
				}
			}

			pbTx += 5;
			n645Len += 5;
		}
	}

	return n645Len;
}


//描述: 根据抄表ID返回数据项长度
int GetItemLength( WORD wBank, const WORD wID)
{
	switch(wID)
	{
	case 0x901f:
		return GetItemLen(BN0, 0x941f);
	default :
		return GetItemLen(BN0, wID);
	}
}

//返回值：-1则转换失败；>0表示转换后所有ID的数据长度
int FmtGBTo645(WORD wBn, WORD* pwID, WORD wNum, BYTE* pbIn, BYTE* pbOut)
{
	int i, iGBLen, i645Len;
	BYTE* pbBase = pbOut;
	for (i=0; i<wNum; i++)
	{
		iGBLen = GetItemLength(wBn, pwID[i]);
		
		if (iGBLen <= 0)
			return -1;
			
		i645Len = (int)FmtGBTo645(wBn, pwID[i], pbIn, pbOut);

		pbOut += i645Len;		
		pbIn += iGBLen;
	}
	
	return (pbOut-pbBase);
}


//描述：根据当前表的记录号得到其在表中的记录索引
//参数：@fd 数据库表的句柄; @iRecNo 表的记录号(从1开始)
//返回：当前表记录号的记录索引,小于0表示错误
int GetRecIdx(const int fd, int iRecNo)
{
	int iRecNum = TdbGetRecNum(fd);	//TDB_ERR_DBLOCKED:-11, TDB_ERR_TBNOEXIST:-3
	if (iRecNum < 0)
		return iRecNum;
		
	if (iRecNo > iRecNum)
		return ERR_OVER_RECNUM;		//ERR_OVER_RECNUM:-20
		
	int iRecPtr = TdbGetRecPtr(fd);	//TDB_ERR_DBLOCKED:-11, TDB_ERR_TBNOEXIST:-3
	if (iRecPtr < 0)
		return iRecPtr;
	
	int iRecIdx, iTmpIdx;
	iTmpIdx = iRecPtr - iRecNo;
	if ( iTmpIdx >= 0 )
		iRecIdx = iTmpIdx;
	else
		iRecIdx = iRecNum + iTmpIdx;
	
	return iRecIdx;
}

//#define AMP_MS_MAX 	(10*1000*3600)	//单位:0.1安培时,因Fmt6ToVal放大了100倍所以为10*
#define AMP_MS_MAX 	(10*1000*3600)	//单位:1安培时,因Fmt6ToVal放大了100倍所以为100*
									//1000*3600把时间都转换成毫秒
void DoAmpHour()
{
	static DWORD dwPreMS = 0;
	
	if (GetClick() < 10)
		return;

	if (dwPreMS == 0)
	{
		dwPreMS = GetTick();
		return;
	}
	
	BYTE bTmpBuf[6];
	BYTE buf[12];
	if (ReadItemEx(BN0, PN0, 0xb62f, buf)<= 0)//相电流数据块
		return;
	
	bool fNewVal = false;
	BYTE bI;
	WORD wI[3];
	int  iIntervalMS, iAdd;
	DWORD dwAmpMS, dwCurMS;

	dwCurMS = GetTick();
	iIntervalMS = dwCurMS - dwPreMS;
	if (iIntervalMS <= 0)
		return;
		
	dwPreMS = dwCurMS;
	for (bI=0; bI<3; bI++)
	{
		wI[bI]= (WORD)ABS(Fmt25ToVal(buf+bI*3, 3));
		dwAmpMS = wI[bI] * iIntervalMS;
		g_dwAmpMS[bI] += dwAmpMS;
		
		if (g_dwAmpMS[bI] >= AMP_MS_MAX)
		{
			iAdd = g_dwAmpMS[bI] / AMP_MS_MAX;
			g_dwAmpMS[bI] = g_dwAmpMS[bI] - iAdd * AMP_MS_MAX;	
			
			//0xea01, 0xea02, 0xea03--A、B、C相的总安培乘时间, 4字节
			if (ReadItemEx(BN0, PN0, 0xea01+bI, bTmpBuf)<= 0)
				continue;
			
			DWORD dwTmpVal = BcdToDWORD(bTmpBuf, 4) + iAdd;
			DWORDToBCD(dwTmpVal, bTmpBuf, 4);
			WriteItemEx(BN0, PN0, 0xea01+bI, bTmpBuf);

			fNewVal = true;
		}
	}

	//更新总安时
	if (fNewVal)	
	{
		DWORD dwTotal = 0;
		for (bI=0; bI<3; bI++)
		{
			if (ReadItemEx(BN0, PN0, 0xea01+bI, bTmpBuf)<= 0)
				continue;

			dwTotal += BcdToDWORD(bTmpBuf, 4);
		}

		DWORDToBCD(dwTotal, bTmpBuf, 4);
		WriteItemEx(BN0, PN0, 0xea00, bTmpBuf);
	}
}


//是否有权限
bool HaveProgPermit(WORD wID, BYTE bPswPerm)
{
	if (IsInProgState() == false)
		return false;

	if (bPswPerm == 2)	//管理员权限
	{
		return true;
	}
	else if (bPswPerm == 4)	//操作员权限
	{
		if (wID == 0x19)	//最大需量清零命令
			return true;
	}
	
	return false;
}
#endif

