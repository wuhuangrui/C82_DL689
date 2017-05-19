/*********************************************************************************************************
 * Copyright (c) 2009,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：DbCfg.cpp
 * 摘    要：系统数据库的配置文件,主要用来配置系统库的数据项及控制结构
 * 当前版本：1.0
 * 作    者：孔成波
 * 完成日期：2016年8月
 *********************************************************************************************************/
#include "stdafx.h"
#include "apptypedef.h"
#include "FaCfg.h"
#include "DataManager.h"
#include "FaAPI.h"
#include "sysfs.h"
#include "DbAPI.h"
#include "DbConst.h"
#include "FrzTask.h"

//软件版本
BYTE g_bTermSoftVer[OOB_SOFT_VER_LEN] = {
						DT_STRUCT, 0x06, 
						DT_VIS_STR, 0x04, 'C', 'L', 'O', 'U',			//厂商代号 4
//						DT_VIS_STR, 0x04, 'Z', 'J', 'S', 'J',			//软件版本 4 ,过台子远程升级用
						DT_VIS_STR, 0x04, '0', '7', '9', 'g',			//软件版本 4 ,过台子远程升级用
//						DT_VIS_STR, 0x06, '1','7', '0','2','2','3',		//软件版本日期
						DT_VIS_STR, 0x06, '1','7', '0','5','0','8',		//软件版本日期
						DT_VIS_STR, 0x04, 'V', 'C', '8', '2',			//硬件版本 4 
						DT_VIS_STR, 0x06, '1','6', '1','0','1','3',		//硬件版本日期
						DT_VIS_STR, 0x08, '0x00', '0x00', '0x00', '0x00', '0x00', '0x00','0x00','0x00',
};	//电气设备——版本信息

//内部软件版本
BYTE g_bInnerSoftVer[INN_SOFT_VER_LEN] = 
{
	'6', '9', '8', '.', '4', '5', '-', 'Z', 'h', 'e', 'J', 'i', 'a', 'n',  'g', 0,  //地区 16个字节，通常为省份全拼，标准版则为Standard
	'0', '0', '0', '1',     //版本4个字节 x.xx A 主版本.副版本 测试版本，正式归档的版本测试版本号为0
	 0x08, 0x05, 0x17,        		 //日期3个字节 BCD码，终端软件发布日期。
};

TItemDesc g_EngDataDesc[] =   //电能量类对象
{
    {0x0000, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//版本控制
	//组合有功电能，带格式，02 05 06 ********
    {0x0001, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//组合有功电能，带格式，02 05 06 ********
	//正向有功电能，总，A/B/C
    {0x0010, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//正向有功电能，带格式，
    {0x0011, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A相正向有功电能，带格式，
    {0x0012, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B相正向有功电能，带格式，
    {0x0013, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C相正向有功电能，带格式，
	//反向有功电能，总，A/B/C
    {0x0020, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//反向有功电能，带格式，
    {0x0021, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A相反向有功电能，带格式，
    {0x0022, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B相反向有功电能，带格式，
    {0x0023, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C相反向有功电能，带格式，
	//组合无功1电能，总，A/B/C
    {0x0030, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//组合无功1电能，带格式，
    {0x0031, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A相组合无功1电能，带格式，
    {0x0032, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B相组合无功1电能，带格式，
    {0x0033, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C相组合无功1电能，带格式，
	//组合无功2电能，总，A/B/C
    {0x0040, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//组合无功2电能，带格式，
    {0x0041, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A相组合无功2电能，带格式，
    {0x0042, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B相组合无功2电能，带格式，
    {0x0043, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C相组合无功2电能，带格式，
	//第1象限无功电能，总，A/B/C
    {0x0050, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0051, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0052, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0053, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//第2象限无功电能，总，A/B/C
    {0x0060, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0061, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0062, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0063, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//第3象限无功电能，总，A/B/C
    {0x0070, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0071, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0072, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0073, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//第4象限无功电能，总，A/B/C
    {0x0080, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0081, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0082, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0083, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//正向视在电能，总，A/B/C
    {0x0090, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0091, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0092, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0093, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//反向视在电能，总，A/B/C
    {0x00A0, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x00A1, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x00A2, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x00A3, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//正向有功基波电能，总，A/B/C
    {0x0110, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0111, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0112, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0113, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//反向有功基波电能，总，A/B/C
    {0x0120, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0121, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0122, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0123, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//正向有功谐波电能，总，A/B/C
    {0x0210, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0211, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0212, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0213, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//反向有功谐波电能，总，A/B/C
    {0x0220, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0221, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0222, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0223, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//铜损有功总电能补偿量，总，A/B/C
    {0x0300, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0301, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0302, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0303, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//铁损有功总电能补偿量，总，A/B/C
    {0x0400, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0401, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0402, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0403, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//关联总电能，总，A/B/C
    {0x0500, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0501, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0502, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0503, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},

	//高精度组合有功电能，带格式，02 05 21 ********
    {0x0601, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//组合有功电能，带格式，02 05 06 ********
	//正向有功电能，总，A/B/C
    {0x0610, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//正向有功电能，带格式，
    {0x0611, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A相正向有功电能，带格式，
    {0x0612, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B相正向有功电能，带格式，
    {0x0613, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C相正向有功电能，带格式，
	//反向有功电能，总，A/B/C
    {0x0620, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//反向有功电能，带格式，
    {0x0621, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A相反向有功电能，带格式，
    {0x0622, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B相反向有功电能，带格式，
    {0x0623, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C相反向有功电能，带格式，
	//组合无功1电能，总，A/B/C
    {0x0630, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//组合无功1电能，带格式，
    {0x0631, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A相组合无功1电能，带格式，
    {0x0632, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B相组合无功1电能，带格式，
    {0x0633, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C相组合无功1电能，带格式，
	//组合无功2电能，总，A/B/C
    {0x0640, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//组合无功2电能，带格式，
    {0x0641, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A相组合无功2电能，带格式，
    {0x0642, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B相组合无功2电能，带格式，
    {0x0643, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C相组合无功2电能，带格式，
	//第1象限无功电能，总，A/B/C
    {0x0650, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0651, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0652, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0653, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//第2象限无功电能，总，A/B/C
    {0x0660, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0661, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0662, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0663, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//第3象限无功电能，总，A/B/C
    {0x0670, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0671, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0672, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0673, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//第4象限无功电能，总，A/B/C
    {0x0680, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0681, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0682, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0683, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//正向视在电能，总，A/B/C
    {0x0690, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0691, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0692, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0693, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//反向视在电能，总，A/B/C
    {0x06A0, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x06A1, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x06A2, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x06A3, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//正向有功基波电能，总，A/B/C
    {0x0710, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0711, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0712, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0713, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//反向有功基波电能，总，A/B/C
    {0x0720, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0721, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0722, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0723, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//正向有功谐波电能，总，A/B/C
    {0x0810, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0811, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0812, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0813, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//反向有功谐波电能，总，A/B/C
    {0x0820, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0821, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0822, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0823, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//铜损有功总电能补偿量，总，A/B/C
    {0x0900, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0901, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0902, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0903, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//铁损有功总电能补偿量，总，A/B/C
    {0x0A00, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0A01, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0A02, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0A03, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//关联总电能，总，A/B/C
    {0x0B00, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0B01, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0B02, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x0B03, 	47, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
};

TItemDesc g_DemDataDesc[] =   //最大需量类对象
{
    {0x1000, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//版本控制
	//正向有功最大需量总，A/B/C
    {0x1010, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//正向有功最大需量，带格式，02 05 06 ********
    {0x1011, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A相正向有功最大需量，带格式，
    {0x1012, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//B相正向有功最大需量，带格式，
    {0x1013, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C相正向有功最大需量，带格式，
	//反向有功最大需量总，A/B/C
    {0x1020, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1021, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1022, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1023, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//组合1无功最大需量总，A/B/C
    {0x1030, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1031, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1032, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1033, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//组合2无功最大需量总，A/B/C
    {0x1040, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1041, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1042, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1043, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//第1象限最大需量总，A/B/C
    {0x1050, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1051, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1052, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1053, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//第2象限最大需量总，A/B/C
    {0x1060, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1061, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1062, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1063, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//第3象限最大需量总，A/B/C
    {0x1070, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1071, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1072, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1073, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//第4象限最大需量总，A/B/C
    {0x1080, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1081, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1082, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1083, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//正向视在最大需量总，A/B/C
    {0x1090, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1091, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1092, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1093, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//反向视在最大需量总，A/B/C
    {0x10A0, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x10A1, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x10A2, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x10A3, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//冻结周期内正向有功最大需量总，A/B/C
    {0x1110, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1111, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1112, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1113, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//冻结周期内反向有功最大需量总，A/B/C
    {0x1120, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1121, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1122, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1123, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//冻结周期内组合无功1最大需量总，A/B/C
    {0x1130, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1131, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1132, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1133, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//冻结周期内组合无功2最大需量总，A/B/C
    {0x1140, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1141, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1142, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1143, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//冻结周期内第1象限最大需量总，A/B/C
    {0x1150, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1151, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1152, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1153, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//冻结周期内第2象限最大需量总，A/B/C
    {0x1160, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1161, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1162, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1163, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//冻结周期内第3象限最大需量总，A/B/C
    {0x1170, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1171, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1172, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1173, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//冻结周期内第4象限最大需量总，A/B/C
    {0x1180, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1181, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1182, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1183, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//冻结周期内正向视在最大需量总，A/B/C
    {0x1190, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1191, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1192, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x1193, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	//冻结周期内反向视在最大需量总，A/B/C
    {0x11A0, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x11A1, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x11A2, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
    {0x11A3, 	77, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},
};

TItemDesc g_VariableDesc[] =   //变量类对象
{
    {0x2000, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电压A/B/C，带格式，02 03 12 ********
    {0x2001, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电流A/B/C/零序，带格式，
    {0x2002, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电压相角A/B/C，带格式，
    {0x2003, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电压电流夹角A/B/C，带格式，
    {0x2004, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//有功功率总，A/B/C，带格式，
    {0x2005, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//无功功率总，A/B/C，带格式，
    {0x2006, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//视在功功率总，A/B/C，带格式，
    {0x2007, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//一分钟平均有功功率总，A/B/C，带格式，
    {0x2008, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//一分钟平均无功功率总，A/B/C，带格式，
    {0x2009, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//一分钟平均视在功率总，A/B/C，带格式，
    {0x200A, 	14, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//功率因素总，A/B/C，带格式，
    {0x200B, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电压波形失真度总，A/B/C，带格式，
    {0x200C, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电流波形失真度总，A/B/C，带格式，
    {0x200D, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电压谐波含有量，带格式，总及2~n次
    {0x200E, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电流谐波还有量，带格式，总及2~n次，
    {0x200F, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电网频率，带格式，
    {0x2010, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//表内温度
    {0x2011, 	3, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//时钟电池电压
    {0x2012, 	3, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//停电抄表电池电压
    {0x2013, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//时钟电池工作时间

    {0x2014, 	30, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表运行状态字
    {0x2015, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//对称密匙状态字
    {0x2016, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//证书状态字
    {0x2017, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当前有功需量
    {0x2018, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当前无功需量
    {0x2019, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当前视在需量
    {0x201A, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当前电价
    {0x201B, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当前费率电价
    {0x201C, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当前阶梯电价
    {0x201D, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//安全认证剩余时长
    {0x201E, 	8, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//事件发生时间
    {0x2020, 	8, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//事件结束时间
	{0x2021, 	8, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//数据冻结时间
    {0x2022, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//事件记录序号
    {0x2023, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//冻结记录序号
    {0x2024, 	EVT_SRC_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//事件发生源,格式不定
    {0x2025, 	12, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//事件当前值
    {0x2026, 	3, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电压不平衡
    {0x2027, 	3, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电流不平衡
    {0x2028, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//负载率
    {0x2029, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//安时值
	
	{0x202A, 	17, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//目的服务器地址

	{0x202c, 	12, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//（当前）钱包文件
	{0x202d, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//（当前）钱包文件
	{0x202e, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//累计购电金额
//统计类ID
//区间数据ID, 2+5+2+11*(2+5+5)=141,目前按11个区间值来考虑
	{0x2108, 	141*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//分钟区间统计
	{0x2109, 	141*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//小时区间统计
	{0x210a, 	141*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//日区间统计
	{0x210b, 	141*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//月区间统计
	{0x210c, 	141*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//年区间统计

//累加平均统计数据ID, 2+5+5+5=17
	{0x2118, 	17*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//分钟平均
	{0x2119, 	17*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//小时平均
	{0x211a, 	17*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//日平均
	{0x211b, 	17*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//月平均
	{0x211c, 	17*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//年平均
//极值统计数据ID,2+5+2+4=33
	{0x2128, 	33*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//分钟极值
	{0x2129, 	33*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//小时极值
	{0x212a, 	33*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//日极值
	{0x212b, 	33*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//月极值
	{0x212c, 	33*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//年极值
//电压合格率数据ID,2+2*(2+5+3+3+5+5)=48
	{0x2130, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//总电压合格率
	{0x2131, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当月A相电压合格率
	{0x2132, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当月B相电压合格率
	{0x2133, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当月C相电压合格率
//日最大有功功率及发生时间
	{0x2140, 	15, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//日最大有功功率及发生时间
	{0x2141, 	15, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//月最大有功功率及发生时间

	{0x2200, 	12, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//通信流量
	{0x2203, 	12, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//供电时间
	{0x2204, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//复位次数


//总加组ID
	{0x2301, 	102, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加配置表 支持4个测量点配置
	{0x2302, 	9, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加有功功率
	{0x2303, 	9, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加无功功率
	{0x2304, 	9, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加滑差时间内平均有功功率
	{0x2305, 	9, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加滑差时间内平均无功功率
	{0x2306, 	47, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加日正向有功电量
	{0x2307, 	47, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加日正向无功电量
	{0x2308, 	47, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加月正向有功电量
	{0x2309, 	47, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加月正向无功电量
	{0x230a, 	9, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加剩余电量（费）
	{0x230b, 	9, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//当前功率下浮控控后总加有功功率冻结值
	{0x230c, 	2, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加组滑差时间周期
	{0x230d, 	3,	 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加组功控轮次配置
	{0x230e, 	3,	 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加组电控轮次配置
	{0x230f, 	19, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加组控制设置状态
	{0x2310, 	28, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//总加组当前控制状态
	{0x2311, 	40, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,		},//换算及单位	
	
	{0x2404, 	5, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//有功功率  --- 脉冲数据
	{0x2405, 	5, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//无功功率
	{0x2406, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//当日正向有功电量
	{0x2407, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//当月正向有功电量
	{0x2408, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//当日反向有功电量
	{0x2409, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//当月反向有功电量
	{0x2410, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//当日正向无功电量
	{0x2411, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//当月正向无功电量
	{0x2412, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//当日反向无功电量
	{0x2413, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//当月反向无功电量
	{0x2414, 	47, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//正向有功电能示值
	{0x2415, 	47, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//正向无功电能示值
	{0x2416, 	47, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//反向有功电能示值
	{0x2417, 	47, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//反向无功电能示值
	{0x2418, 	44, 				DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//换算及单位(14*3+2)	--- 脉冲数据

	{0x2419, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//正向有功电能示值-低精度
	{0x241a, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//正向无功电能示值-低精度
	{0x241b, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//反向有功电能示值-低精度
	{0x241c, 	27, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//反向无功电能示值-低精度

	{0x2500, 	5, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//累计水（热）流量
	{0x2501, 	5, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//累计气流量
	{0x2502, 	5, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//累计热量
	{0x2503, 	5, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//热功率
	{0x2504, 	5, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//累计工作时间
	{0x2505, 	12, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//水温
	{0x2506, 	6, 			DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//（仪表）状态ST

    {0x2600, 	(2+(HARMONIC_NUM-1)*3), 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A相电压谐波含有量，带格式，总及2~n次
	{0x2601,	(2+(HARMONIC_NUM-1)*3), 		DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//B相电压谐波含有量，带格式，总及2~n次
	{0x2602,	(2+(HARMONIC_NUM-1)*3), 		DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//C相电压谐波含有量，带格式，总及2~n次
    {0x2603, 	(2+(HARMONIC_NUM-1)*3), 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//A相电流谐波还有量，带格式，总及2~n次，
	{0x2604,	(2+(HARMONIC_NUM-1)*3), 		DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//B相电流谐波还有量，带格式，总及2~n次，
	{0x2605,	(2+(HARMONIC_NUM-1)*3), 		DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//C相电流谐波还有量，带格式，总及2~n次，
	{0x2606,	(2+2), 		DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//谐波次数，



};

TItemDesc g_EventParaDesc[] =   //事件类对象参数
{
    {0x2FF0, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//版本控制
    
     //事件配置参数****************************************   
    {0x3000, 	15, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表失压事件，属性5，配置参数
    {0x3001, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表欠压事件，属性5，配置参数
    {0x3002, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表过压事件，属性5，配置参数
    {0x3003, 	12, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表断相事件，属性5，配置参数
    {0x3004, 	17, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表失流事件，属性5，配置参数
    {0x3005, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表过流事件，属性5，配置参数
    {0x3006, 	12, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表断流事件，属性5，配置参数
    {0x3007, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表潮流反向事件，属性5，配置参数
    {0x3008, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表过载事件，属性5，配置参数
    {0x3009, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表正向有功需量超限事件，属性6，配置参数
    {0x300A, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表反向有功需量超限事件，属性6，配置参数
    {0x300B, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表无功需量超限事件，属性5，配置参数
    {0x300C, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表功率因数超下限事件，属性6，配置参数
    //{0x300D, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表全失压事件，属性6，配置参数，无配置参数
    /* 不支持
    {0x300E, 	4, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表辅助电源掉电事件，属性6，配置参数，不支持
    */ 
    {0x300F, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表电压逆相序事件，属性6，配置参数
    {0x3010, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表电流逆相序事件，属性2，属性6
    /* 不支持
    {0x3011, 	4, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表掉电事件，属性6，配置参数，无配置参数
    {0x3012, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表编程事件，属性6，配置参数，无配置参数
    */
   //{0x3013, 	0, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表清零事件，属性6，配置参数，无配置参数
   //{0x3014, 	0, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表需量清零事件，属性6，配置参数，无配置参数
   //{0x3015, 	0, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表事件清零事件，属性6，配置参数，无配置参数
    /* 不支持
    {0x3016, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表校时事件，属性6，配置参数，无配置参数
    {0x3017, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表时段表编程事件，属性6，配置参数，无配置参数
    {0x3018, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表时区表编程事件，属性6，配置参数，无配置参数
    {0x3019, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表周休日编程事件，属性6，配置参数，无配置参数
    {0x301A, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表结算日编程事件，属性6，配置参数，无配置参数
    {0x301B, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表开盖事件，属性6，配置参数，无配置参数
    {0x301C, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表开端钮盒事件，属性6，配置参数，无配置参数
    */
    {0x301D, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表电压不平衡事件，属性2，属性6
    {0x301E, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表电流不平衡事件，属性2，属性6
    /* 不支持
    {0x301F, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表跳闸事件，属性6，配置参数，无配置参数
    {0x3020, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表合闸事件，属性6，配置参数，无配置参数
    {0x3021, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表节假日编程事件，属性6，配置参数，无配置参数
    {0x3022, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表有功组合方式编程事件，属性6，配置参数，无配置参数
    {0x3023, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表无功组合方式编程事件，属性6，配置参数，无配置参数
    {0x3024, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表费率参数表编程事件，属性6，配置参数，无配置参数
    {0x3025, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表阶梯表编程事件，属性6，配置参数，无配置参数
    {0x3026, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表密钥更新事件，属性6，配置参数，无配置参数
    {0x3027, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表异常插卡事件，属性6，配置参数，无配置参数
    {0x3028, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表购电记录，属性6，配置参数，无配置参数
    {0x3029, 	1, 					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表退费记录，属性6，配置参数，无配置参数
    {0x302A, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表恒定磁场干扰事件，属性2，属性6
    {0x302B, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表负荷开关误动作事件，属性2，属性6
    {0x302C, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表电源异常事件，属性2，属性6
    */
    {0x302D, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表电流严重不平衡事件，属性2，属性6
    {0x302E, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表时钟故障事件，属性2，属性6
    {0x302F, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表计量芯片故障事件，属性2，属性6
	/* 不支持
	{0x3030, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//通信模块变更事件，属性6
	*/

	//{0x3100, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端初始化事件，属性6，无配置参数
	//{0x3101, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端版本变更事件，属性6，无配置参数
	//{0x3104, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端状态量变位事件，属性6，无配置参数
#ifdef GW_OOB_DEBUG_0x31050600	
	{0x3105, 	5, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表时钟超差事件，属性6
#else
	{0x3105, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表时钟超差事件，属性6
#endif
	{0x3106, 	SAMPLE_CFG_ID_LEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端停/上电事件，属性6
	/* 不支持
	{0x3107, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端直流模拟量越上限事件，属性6
	{0x3108, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端直流模拟量越下限事件，属性6
	*/
	//{0x3109, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端消息认证错误事件，属性6，无配置参数
	//{0x310A, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端故障记录，属性6，无配置参数
	{0x310B, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表示度下降事件，属性6
#ifdef GW_OOB_DEBUG_0x310C0600
	{0x310C, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能量超差事件，属性6
#else
	{0x310C, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能量超差事件，属性6
#endif
#ifdef GW_OOB_DEBUG_0x310D0600
	{0x310D, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表飞走事件，属性6
#else
	{0x310D, 	9, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表飞走事件，属性6
#endif
#ifdef GW_OOB_DEBUG_0x310E0600
	{0x310E, 	6, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表停走事件，属性6
#else
	{0x310E, 	8, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表停走事件，属性6
#endif
	{0x310F, 	6, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端抄表失败事件，属性6
	{0x3110, 	7, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//月通信流量超限事件，属性6
	/* 不支持		
	//{0x3111, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//发现未知电能表事件，属性6，无配置参数
	//{0x3112, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//跨台区电能表事件，属性6，无配置参数
	*/
	//{0x3114, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端对时事件，属性6，无配置参数
	//{0x3115, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//遥控跳闸记录，属性6，无配置参数
	{0x3116, 	DIFF_COMP_CFG_ID_LEN, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	1,		},//有功总电能量差动越限事件记录，属性2，属性6	28*n+2 = 282, n = 10
	/* 不支持
	//{0x3117, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//输出回路接入状态变位事件记录，属性6，无配置参数
	*/
	//{0x3118, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端编程记录，属性6，无配置参数
	//{0x3119, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端电流回路异常事件，属性6，无配置参数
	/* 不支持
	{0x311A, 	5, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表在网状态切换事件，属性6
	//{0x311B, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端对电表校时记录，属性6，无配置参数
	*/
	{0x311C, 	4, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表数据变更监控记录，属性6

	//{0x3200, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//功控跳闸记录，属性6，无配置参数
	//{0x3201, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电控跳闸记录，属性6，无配置参数
	//{0x3202, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//购电参数设置记录，属性6，无配置参数
	//{0x3203, 	0, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电控告警事件记录，属性6，无配置参数

	{0x3300, 	CN_RPT_TOTAL_LEN, 		DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//事件上报状态
	{0x3301, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//标准事件记录单元，格式不定
	{0x3302, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//编程记录事件单元
	{0x3303, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//发现未知电能表事件单元
	{0x3304, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//跨台区电能表事件单元
	{0x3305, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//功控跳闸记录单元
	{0x3306, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//电控跳闸记录单元
	{0x3307, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//电控告警事件单元
	{0x3308, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表需量超限事件单元，格式不定
	{0x3309, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//停上电事件记录单元
	{0x330A, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//遥控事件记录单元
	{0x330B, 	1, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//有功总电能量差动越限事件记录单元
	{0x330C, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//事件清零事件记录单元，格式不定
	{0x330D, 	1, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//终端对电表校时记录单元
	{0x3320, 	EVT_ADDOAD_MAXLEN, 			DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//新增对象3320

	//事件其它参数****************************************
	{0x3600, 	EVT_ATTRTAB_LEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		INMTR_EVT_NUM,		},//IC24内表事件，属性2，关联对象属性表  IC7内表事件，属性3，关联对象属性表
	{0x3601, 	3, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		INMTR_EVT_NUM,		},//IC24内表事件，属性4，最大记录数   IC7内表事件，属性5，最大记录数
	{0x3602, 	2, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		INMTR_EVT_NUM,		},//IC24内表事件，属性11，上报标识  IC7内表事件，属性8，上报标识
	{0x3603, 	2, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		INMTR_EVT_NUM,		},//IC24内表事件，属性12，有效标识  IC7内表事件，属性9，有效标识

	{0x3700, 	EVT_ATTRTAB_LEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		TERM_EXC_NUM,		},//终端事件，属性3，关联对象属性表
	{0x3701, 	3, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		TERM_EXC_NUM,		},//终端事件，属性5，最大记录数 
	{0x3702, 	2, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		TERM_EXC_NUM,		},//终端事件，属性8，上报标识
	{0x3703, 	2, 						DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		TERM_EXC_NUM,		},//终端事件，属性9，有效标识

	{0x3704, 	EVT_ADDOI_MAXLEN, 		DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//新增对象3320 属性3（需上报事件对象列表，只读）∷= array OI
	//注:0x3AXX此些ID仅用于计算长度,不用于数据的存储.
	{0x3A00, 	TERM_PRG_LIST_LEN, 		DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//编程记录事件单元∷编程对象列表  array OAD	
	{0x3A01, 	600, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//发现未知电能表事件∷搜表结果      array 一个搜表结果
	//0x3A02按2+46*STEP_AREA_SAVE_REC_NUM计算,需要大于或等于此计算的值
	{0x3A02, 	500, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//跨台区电能表事件单元∷跨台区搜表结果  array  一个跨台区结果
	{0x3A03, 	9, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//功控跳闸记录单元∷事件发生后2分钟功率  long64(单位：W，换算-1)，
	{0x3A04, 	3, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//功控跳闸记录单元∷控制对象      OI，
	{0x3A05, 	2, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//功控跳闸记录单元∷跳闸轮次      bit-string(SIZE(8))，
	{0x3A06, 	9, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//功控跳闸记录单元∷功控定值      long64（单位：kWh，换算-4），
	{0x3A07, 	9, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//功控跳闸记录单元∷跳闸发生前总加有功功率    long64（单位：kW，换算-4），
	{0x3A08, 	3, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//电控跳闸记录单元∷控制对象      OI，
	{0x3A09, 	2, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//电控跳闸记录单元∷跳闸轮次      bit-string(SIZE(8))，
	{0x3A0A, 	9, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//电控跳闸记录单元∷电控定值      long64（单位：kWh，换算-4），
	{0x3A0B, 	9, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//电控跳闸记录单元∷跳闸发生时总加电能量  long64（单位：kwh/元，换算-4）
	{0x3A0C, 	3, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//电控告警事件单元∷控制对象      OI，
 	{0x3A0D, 	9, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//电控告警事件单元∷电控定值      long64（单位：kWh，换算-4），	
	{0x3A0E, 	5, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表需量超限事件单元∷超限期间需量最大值  double-long-unsigned
	{0x3A0F, 	8, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表需量超限事件单元∷超限期间需量最大值发生时间  date_time_s
	{0x3A10, 	3, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//停/上电事件记录单元∷属性标志     bit-string（SIZE(8)）
	{0x3A11, 	74, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//遥控事件记录单元∷控后2分钟总加组功率 array long64
 	{0x3A12, 	9, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//有功总电能量差动越限事件记录单元∷越限时对比总加组有功总电能量 long64（单位：kWh，换算：-4），
 	{0x3A13, 	9, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//有功总电能量差动越限事件记录单元∷越限时参照总加组有功总电能量 long64（单位：kWh，换算：-4），
 	{0x3A14, 	2, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//有功总电能量差动越限事件记录单元∷越限时差动越限相对偏差值 integer（单位：%，换算：0）	
 	{0x3A15, 	9, 						DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//有功总电能量差动越限事件记录单元∷越限时差动越限绝对偏差值 long64（单位：kWh，换算：-4）	
	{0x3A16, 	EVT_CLR_LIST_LEN, 		DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//事件清零事件记录单元∷事件清零事件记录单元，array OMD
	{0x3A17, 	8, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//终端对电表校时记录单元∷校时前时钟    date_time_s，
	{0x3A18, 	2, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//终端对电表校时记录单元∷时钟误差      integer（单位：秒，无换算）
	//{0x3A19, 	0, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表在网状态切换事件单元∷状态变迁事件  array structure 
	{0x3A1A, 	MTEDATACHG_CSD_LEN, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表数据变更监控记录单元∷监控数据对象  CSD，
	{0x3A1B, 	MTEDATACHG_DATA_LEN, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表数据变更监控记录单元∷变化前数据    Data，
	{0x3A1C, 	MTEDATACHG_DATA_LEN, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表数据变更监控记录单元∷变化后数据    Data
	//{0x3A1D, 	0, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//异常插卡事件记录单元∷卡序列号	  octet-string，
	//{0x3A1E, 	0, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//异常插卡事件记录单元∷插卡错误信息字	 unsigned，
	//{0x3A1F, 	0, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//异常插卡事件记录单元∷插卡操作命令头 	 octet-string，
	//{0x3A20, 	0, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//异常插卡事件记录单元∷插卡错误响应状态  long-unsigned，
	//{0x3A21, 	0, 						DI_HIGH_PERM, DI_READ, 			0, 	INFO_NONE,	FMT_UNK,		1,		},//退费事件记录单元∷退费金额      double-long-unsigned（单位：元，换算：-2），
};

BYTE g_EventParaDefault[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x2FF0,Ver(20)
	0x02,0x04,0x12,0x06,0xb4,0x12,0x07,0x4e,0x05,0x00,0x00,0x00,0xfa,0x11,0x3c,	// 0x3000,配置参数,失压
	0x02,0x02,0x12,0x06,0xb4,0x11,0x3c,			//0x3001,配置参数,欠压
	0x02,0x02,0x12,0x0a,0x50,0x11,0x3c,			//0x3002,配置参数,过压
	0x02,0x03,0x12,0x05,0x28,0x05,0x00,0x00,0x00,0xfa,0x11,0x3c,	//0x3003,配置参数,断相
	0x02,0x04,0x12,0x06,0x04,0x05,0x00,0x00,0x00,0xfa,0x05,0x00,0x00,0x09,0xc4,0x11,0x3c,	// 0x3004,配置参数,失流
	0x02,0x02,0x05,0x00,0x0a,0xfc,0x80,0x11,0x3c,	//0x3005,配置参数,过流
	0x02,0x03,0x12,0x05,0x28,0x05,0x00,0x00,0x00,0xfa,0x11,0x3c,	//0x3006,配置参数,断流
	0x02,0x02,0x05,0x00,0x00,0x00,0x37,0x11,0x3c,	//0x3007,配置参数,潮流反向 单位：W，换算：-1
	0x02,0x02,0x05,0x00,0x02,0x6a,0xc0,0x11,0x3c,	//0x3008,配置参数,过载 单位：W，换算：-1
	0x02,0x02,0x06,0x00,0x02,0x17,0xd6,0x11,0x3c,	//0x3009,配置参数,正向有功需量超限 1.2*Imax*Un*1.732/2/1000 按三相四线默认 单位：kW，换算：-4
	0x02,0x02,0x06,0x00,0x02,0x17,0xd6,0x11,0x3c,	//0x300A,配置参数,反向有功需量超限 1.2*Imax*Un*1.732/2/1000 按三相四线默认 单位：kW，换算：-4
	0x02,0x02,0x06,0x00,0x02,0x17,0xd6,0x11,0x3c,	//0x300B,配置参数,无功需量超限 1.2*Imax*Un*1.732/2/1000 按三相四线默认 单位：kW，换算：-4
	0x02,0x02,0x10,0x01,0x2c,0x11,0x3c,			//0x300C,配置参数,功率因数超下限
	0x02,0x01,0x11,0x3c,							//0x300F,配置参数,电压逆相序
	0x02,0x01,0x11,0x3c,							//0x3010,配置参数,电流逆相序
	0x02,0x02,0x10,0x0b,0xb8,0x11,0x3c,			//0x301D,配置参数,电压不平衡
	0x02,0x02,0x10,0x0b,0xb8,0x11,0x3c,			//0x301E,配置参数,电流不平衡
	0x02,0x02,0x10,0x23,0x28,0x11,0x3c,			//0x302D,配置参数,电流不平衡
	0x02,0x01,0x11,0x3c,							//0x302E,配置参数,时钟故障
	0x02,0x01,0x11,0x3c,							//0x302F,配置参数,计量芯片故障

#ifdef GW_OOB_DEBUG_0x31050600	
	DT_STRUCT,0x01,DT_LONG_U,0x00,0x00,			//0x3105 电能表时钟超差事件，属性6
#else
	DT_STRUCT,0x02,DT_LONG_U,0x00,0x00,DT_UNSIGN,0x00,		//0x3105 电能表时钟超差事件，属性6
#endif

	DT_STRUCT,0x02,
		DT_STRUCT,0x04,
			DT_BIT_STR,0x08,0x00,
			DT_UNSIGN,0x00,
			DT_UNSIGN,0x05,
			DT_ARRAY,0,	//12,按<适应于面向对象互操作数据交换协议的电能表及采集终端相关技术要求2016-09-09>默认参数
/*#ifdef MTREXC_ADDR_TPYE_TSA
				DT_TSA,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//30
				DT_TSA,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//48
				DT_TSA,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//66
				DT_TSA,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//84
#else
				DT_OCT_STR,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//30
				DT_OCT_STR,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//48
				DT_OCT_STR,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//66
				DT_OCT_STR,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//84
#endif*/
		DT_STRUCT,0x06,
			DT_LONG_U,0x00,0x01,
			DT_LONG_U,0x10,0xE0,
			DT_LONG_U,0x00,0x05,
			DT_LONG_U,0x00,0x01,
			DT_LONG_U,0x05,0x28,
			DT_LONG_U,0x06,0xE0,

		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3106 104 终端停/上电事件，属性6

	DT_STRUCT,0x01,DT_UNSIGN,0x00,				//0x310B 电能表示度下降事件，属性6

#ifdef GW_OOB_DEBUG_0x310C0600
	DT_STRUCT,0x01,DT_DB_LONG_U,0x00,0x00,0x00,0x00, //0x310C 9 电能量超差事件，属性6
#else
	DT_STRUCT,0x02,DT_DB_LONG_U,0x00,0x00,0x00,0x00,DT_UNSIGN,0x00, //0x310C 9 电能量超差事件，属性6
#endif
#ifdef GW_OOB_DEBUG_0x310D0600
	DT_STRUCT,0x01,DT_DB_LONG_U,0x00,0x00,0x00,0x00, //0x310D 9 电能表飞走事件，属性6
#else
	DT_STRUCT,0x02,DT_DB_LONG_U,0x00,0x00,0x00,0x00,DT_UNSIGN,0x00, //0x310D 9 电能表飞走事件，属性6
#endif
#ifdef GW_OOB_DEBUG_0x310E0600
	DT_STRUCT,0x01,DT_TI,0x00,0x00,0x00, //0x310E 电能表停走事件，属性6
#else
	DT_STRUCT,0x02,DT_TI,0x00,0x00,0x00,DT_UNSIGN,0x00, //0x310E 电能表停走事件，属性6
#endif
	DT_STRUCT,0x02,DT_UNSIGN,0x00,DT_UNSIGN,0x00,		//0x310F 终端抄表失败事件，属性6
	DT_STRUCT,0x01,DT_DB_LONG_U,0x00,0x00,0x00,0x00,	//0x3110 月通信流量超限事件，属性6

	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3116有功总电能量差动越限事件记录，属性2，属性6	28*n+2 = 282, n = 10
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//100
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//200
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//3//300

	DT_STRUCT,0x01,DT_UNSIGN,0x00,				//0x311C 电能表数据变更监控记录，属性6

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3300,事件上报状态
	0x00,										//0x3301
	0x00,										//0x3302
	0x00,										//0x3303
	0x00,										//0x3304	
	0x00,										//0x3305
	0x00,										//0x3306
	0x00,										//0x3307
	0x00,										//0x3308
	0x00,										//0x3309
	0x00,										//0x330A
	0x00,										//0x330B
	0x00,										//0x330C
	0x00,										//0x330D
	
	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3320 新增对象 632字节
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//100
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//200
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//3//300
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3320 新增对象 352字节
	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,								//0x3600, IC24内表事件，属性2，关联对象属性表
	0x12,0x00,0x0a,							//0x3601, IC24内表事件，属性4，最大记录数	
	0x16,0x00,								//0x3602, IC24内表事件，属性11，上报标识
	0x03,0x01,								//0x3603, IC24内表事件，属性12，有效标识

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,	//0x3700, 322 IC7属性2，关联对象属性表

	0x12,0x00,0x0f,	//0x3701 IC7属性5 最大记录数
	0x16,0x00, //0x3702 IC7属性8 上报标识
	0x03,0x01, //0x3703 IC7属性9 有效标识	//FOR TEST 默认有效测试用

	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3704 新增对象 212字节
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//100
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//200
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//0x3704 新增对象 212字节

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x3A00 编程对象列表  array OAD	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A01发现未知电能表事件∷搜表结果  500字节

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A02跨台区电能表事件单元∷跨台区搜表结果   500字节

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A03 事件发生后2分钟功率  long64
	0x00,0x00,0x00,	//0x3A04 控制对象      OI
	0x00,0x00,	//0x3A05 跳闸轮次      bit-string
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A06 功控定值      long64
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A07 跳闸发生前总加有功功率    long64

	0x00,0x00,0x00,	//0x3A08 控制对象      OI
	0x00,0x00,	//0x3A09 跳闸轮次      bit-string
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A0A 电控定值      long64
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A0B 跳闸发生时总加电能量    long64

	
	0x00,0x00,0x00,	//0x3A0C 控制对象      OI
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A0D 电控定值      long64
	0x00,0x00,0x00,0x00,0x00,						//0x3A0E,超限期间需量最大值  double-long-unsigned
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 		//0x3A0F,超限期间需量最大值发生时间  date_time_s
	0x00,0x00,0x00,	//0x3A10 属性标志     bit-string（SIZE(8)）
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A11 控后2分钟总加组功率 array long64

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A12 有功总电能量差动越限事件记录单元∷越限时对比总加组有功总电能量 long64（单位：kWh，换算：-4），
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A13 有功总电能量差动越限事件记录单元∷越限时参照总加组有功总电能量 long64（单位：kWh，换算：-4），
	0x00,0x00,	//0x3A14 有功总电能量差动越限事件记录单元∷越限时差动越限相对偏差值 integer（单位：%，换算：0）
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3A15 有功总电能量差动越限事件记录单元∷越限时差动越限绝对偏差值 long64（单位：kWh，换算：-4）	

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x3A16 事件清零事件记录单元，array OMD
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x3A17 终端对电表校时记录单元∷校时前时钟    date_time_s
	0x00,0x00, //0x3A18 终端对电表校时记录单元∷时钟误差      integer（单位：秒，无换算）


	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//电能表数据变更监控记录单元∷监控数据对象  CSD，60字节
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//电能表数据变更监控记录单元∷变化前数据    Data，200字节
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//100
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//200

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//电能表数据变更监控记录单元∷变化后数据    Data，200字节
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//100
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//200
};

TItemDesc g_EventDataDesc[] =   //事件类对象数据
{
	{0x3B00, 	14, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		INMTR_IC24EVT_NUM,	},//IC24内表事件，属性3，当前记录数
	{0x3B01, 	50, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		INMTR_IC24EVT_NUM,	},//IC24内表事件，属性10，当前值记录表
	{0x3B02, 	28, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,					},//电能表失压事件，属性13，失压统计

	{0x3B03, 	3, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		INMTR_EVT_NUM,		},//IC7内表源NULL事件，属性4，当前记录数
	{0x3B04, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		INMTR_EVT_NUM,		},//IC7内表源NULL事件，属性7，当前值记录表

	{0x3B10, 	3, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		TERM_EXC_NUM,		},//IC7终端抄表事件，属性4，当前记录数
	{0x3B11, 	34,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		MTR_EXC_NUM,				},//IC7终端抄表事件，属性7，当前值记录表(带事件发生源)
	{0x3B12, 	17,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		TERM_EXC_NUM,		},//IC7终端其他事件，属性7，当前值记录表
	{0x3B13, 	18,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,		},//0x310A 设备故障记录	enum，当前值记录表
	{0x3B14, 	21,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,		},//0x3115 遥控跳闸记录	OAD，当前值记录表
	{0x3B15, 	18,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,		},//0x3119 终端电流回路异常事件	enum，当前值记录表
	{0x3B16, 	19,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,		},//0x3202 购电参数设置记录	OI，当前值记录表	
	{0x3B17, 	18,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,		},//0x3106 停上电记录	enum，当前值记录表
	{0x3B18, 	35,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,		},//0x311b 终端对电表校时记录	tsa，当前值记录表
};

BYTE g_EventDataDefault[] = 
{
	0x02,0x04,0x12,0x00,0x00,0x12,0x00,0x00,0x12,0x00,0x00,0x12,0x00,0x00,				//0x3B00, IC24内表事件，属性3，当前记录数
	0x02,0x04,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,
   			0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,
   			0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,
   			0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,					//0x3B01, IC24内表事件，属性10，当前值记录表
	0x02,0x04,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,
			0x1c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x1c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,									//0x3B02, 电能表失压事件，属性13，失压统计
	0x12,0x00,0x00,																	//0x3B03, IC7内表事件，属性4，当前记录数
	0x01,0x01,0x02,0x02,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,		//0x3B04, IC7内表事件，属性7，当前值记录表

	0x12,0x00,0x00, //0x3B10, IC7终端抄表事件，属性4，当前记录数

	0x01,0x01,0x02,0x02,
#ifdef MTREXC_ADDR_TPYE_TSA		//GW_OOB_DEBUG_0x31050700
	DT_TSA,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
#else
	DT_OCT_STR,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
#endif
	0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00, //0x3B11, IC7终端抄表事件，属性7，当前值记录表（带事件发生源）	

	0x01,0x01,0x02,0x02,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00, //0x3B12, IC7终端其他事件，属性7，当前值记录表	
	0x01,0x01,0x02,0x02,0x22,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00, //0x3B13, IC7终端其他事件，属性7，当前值记录表	
	0x01,0x01,0x02,0x02,0x81,0x00,0x00,0x00,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00, //0x3B14, IC7终端其他事件，属性7，当前值记录表	
	0x01,0x01,0x02,0x02,0x22,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00, //0x3B15, IC7终端其他事件，属性7，当前值记录表	
	0x01,0x01,0x02,0x02,0x80,0x00,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00,  //0x3B16, IC7终端其他事件，属性7，当前值记录表	
	0x01,0x01,0x02,0x02,0x16,0x00,0x02,0x02,0x06,0x00,0x00,0x00,0x00,0x06,0x00,0x00,0x00,0x00, //0x3B17, IC7终端其他事件，属性7，当前值记录表	

	DT_ARRAY,0x01,
	DT_STRUCT,0x02,
	DT_TSA,0x00,	
	DT_STRUCT,0x02,DT_DB_LONG_U,0x00,0x00,0x00,0x00,DT_DB_LONG_U,0x00,0x00,0x00,0x00,	
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	//0x3B18, IC7终端其他事件，属性7，当前值记录表		
};


TItemDesc g_ParaDesc[] =   //参变量类对象
{
	{0x3FF0, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//版本控制
	{0x4000, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//日期时间，属性2,属性3,属性4,方法127
	{0x4001, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//通信地址，属性2, 带格式 TSA ********
	{0x4002, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//表号，属性2
	{0x4003, 	18, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//客户编号，属性2
	{0x4004, 	27, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//设备地理位置，属性2,
	{0x4005, 	82, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//组地址，属性2,
	{0x4006, 	6, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//时钟源，属性2，方法127启用（），128禁用（）
	{0x4007, 	20, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//LCD参数，属性2
	{0x4008, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//两套时区表切换时间，属性2
	{0x4009, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//两套日时段切换时间，属性2
	{0x400A, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//两套分时费率切换时间，属性2
	{0x400B, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//两套阶梯电价切换时间，属性2
	{0x400C, 	12, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//时区时段数，属性2
	{0x400D, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//阶梯数，属性2
	{0x400E, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//谐波分析次数，属性2
	{0x400F, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//密钥总条数，属性2

	{0x4010, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//计量元件数，属性2
	{0x4011, 	202, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//公共假日表，属性2
	{0x4012, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//周休日特征字，属性2
	{0x4013, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//周休日釆用的日时段表号，属性2
	{0x4014, 	114,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//当前套时区表，属性2
	{0x4015, 	114,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//备用套时区表，属性2
	{0x4016, 	530,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//当前套日时段表，属性2
	{0x4017, 	530,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_TZ_DC_PARACHG,	FMT_UNK,		1,		},//备用套日时段表，属性2
	{0x4018, 	162,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当前套费率电价，属性2
	{0x4019, 	162, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//备用套费率电价，属性2

	{0x401A, 	128, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当前套阶梯电价，属性2
	{0x401B, 	128, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//备用套阶梯电价，属性2

	{0x401C, 	5, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电流互感器变比，属性2
	{0x401D, 	5, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电压互感器变比，属性2
	{0x401E, 	12, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//报警金额限值，属性2
	{0x401F, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//其它金额限值，属性2

	{0x4020, 	12, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//报警电量限值，属性2
	{0x4021, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//其它电量限值，属性2
	{0x4022, 	4, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//插卡状态字，属性2
	{0x4023, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//认证有效时长，属性2
	{0x4024, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//剔除，属性2

	{0x4030, 	14, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_STAT_PARA,	FMT_UNK,		1,		},//电压合格率参数，属性2

	{0x4100, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_AC_PARA,	FMT_UNK,		1,		},//最大需量周期，属性2
	{0x4101, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_AC_PARA,	FMT_UNK,		1,		},//滑差时间，属性2
	{0x4102, 	2,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//校表脉冲宽度，属性2
	{0x4103, 	34, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//资产管理编码，属性2
	{0x4104, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//额定电压，属性2
	{0x4105, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//额定电流/基本电流，属性2
	{0x4106, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//最大电流，属性2
	{0x4107, 	6, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//有功准确度等级，属性2
	{0x4108, 	6, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//无功准确度等级，属性2
	{0x4109, 	5, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表有功常数，属性2
	{0x410A, 	5, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表无功常数，属性2
	{0x410B, 	34, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表型号，属性2
	{0x410C, 	11, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ABC各相电导系数，属性2
	{0x410D, 	11, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ABC各相电抗系数，属性2
	{0x410E, 	11, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ABC各相电阻系数，属性2
	{0x410F, 	11, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//ABC各相电纳系数，属性2

	{0x4110, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电能表运行特征字1，属性2
	{0x4111, 	18, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//软件备案号，属性2
	{0x4112, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_AC_PARA,	FMT_UNK,		1,		},//有功组合方式特征字，属性2
	{0x4113, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_AC_PARA,	FMT_UNK,		1,		},//无功组合方式1特征字，属性2
	{0x4114, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_AC_PARA,	FMT_UNK,		1,		},//无功组合方式2特征字，属性2
	{0x4115, 	4, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//IC卡
	{0x4116,	(BALANCE_DAY_NUM*6+2), 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_AC_PARA,	FMT_UNK,		1,	},//结算日，属性2 
	{0x4117, 	5, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//期间需量冻结周期，属性2

	{0x4200, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//路由表，属性2，方法127、128、129、134
	{0x4201, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//路由信息单元，属性2
	{0x4202, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//级联通信参数，属性2
	{0x4204, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端广播校时，属性2
	{0x4205,   10, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端单地址广播校时参数，属性3
	

	{0x4302, 	18, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电气设备——设备描述符
	{0x4303, 	46, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电气设备——版本信息
	{0x4304, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电气设备——生产日期
	{0x4305, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电气设备——子设备列表
	{0x4306,	13, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电气设备——支持规约列表
	{0x4307, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电气设备——允许跟随上报
	{0x4308, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电气设备——允许主动上报
	{0x4309, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电气设备——允许与主动通话
	{0x430a, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电气设备——上报通道


	{0x4400, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//应用连接
	{0x4401, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//应用连接认证密码，属性2

	//0x4500,0x4501两个参数格式一样，两个不同参数，映射到不同测量点
	{0x4500, 	153, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//公网通信模块---通讯配置
	{0x4501, 	24, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//公网通信模块---主站通信参数表
	{0x4502, 	204, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//公网通信模块---短信通信参数
	{0x4503, 	46, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//公网通信模块---版本信息
	{0x4504, 	1,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//公网通信模块---支持规约列表
	{0x4505, 	22, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//公网通信模块---SIM卡ICCID
	{0x4506, 	17, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//公网通信模块---IMSI
	{0x4507, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//公网通信模块---信号强度
	{0x4508, 	18, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//公网通信模块---SIM卡号码
	{0x4509, 	6,	 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		2,		},//公网通信模块---拨号IP

	//0x4510~0x4517 以太网通信模块,8个模块，映射到8个不同的测量点
	{0x4510, 	49, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		8,		},//以太网通信模块---属性2，通讯配置
	{0x4511, 	24, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		8,		},//以太网通信模块---属性3，主站通信参数表
	{0x4512, 	102, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		8,		},//以太网通信模块---属性4，网络配置
	{0x4513, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		8,		},//以太网通信模块---属性5，MAC地址

	{0x4520, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,			FMT_UNK,		1,		},//日期时间，属性3，校时模式
	{0x4521, 	12, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,			FMT_UNK,		1,		},//日期时间，属性4，精准校时参数
};

BYTE g_bParaDefault[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)

	//0x4000 DataTimeBCD--7byte+格式--8byte
	0x1c,0x14,0x10,0x01,0x01,0x00,0x00,0x00,//时间日期，实际抄读时需要取当前时间
	//0x4001， 通讯地址，默认01--8byte
	DT_OCT_STR,0x06,
		0x11,0x22,0x33,0x44,0x55,0x66,
		0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,
	//0x4002， 表号，默认01---8byte
	DT_OCT_STR,0x06,
		0x00,0x00,0x00,0x00,0x00,0x01,
		0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,
	//0x4003, 客户编码---8byte
	DT_OCT_STR,0x06,	//有效长度6
		0x00,0x00,0x00,0x00,0x00,0x01,
		0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,

	//0x4004, 设备地理位置---27byte
	DT_STRUCT, 0x03,
		DT_STRUCT, 0x04,	//经度
			DT_ENUM,0x00,	//方位
			DT_UNSIGN,0x00,	//度
			DT_UNSIGN,0x00, //分
			DT_UNSIGN,0x00,	//秒
		DT_STRUCT, 0x04,	//纬度
			DT_ENUM,0x00,	//方位
			DT_UNSIGN,0x00,	//度
			DT_UNSIGN,0x00,	//分
			DT_UNSIGN,0x00,	//秒
		DT_DB_LONG_U,0x00,0x00,0x00,0x00,	//高度（cm）	

	//0x4005, 组地址，先最多设定10个组地址---82byte
	0x01,0x01,
		DT_OCT_STR, 0x06,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	//0x4006,时钟源---6byte
	DT_STRUCT, 0x02,
		DT_ENUM, 0x01,
		DT_ENUM, 0x01,
	//0x4007,LCD参数--20byte
	DT_STRUCT, 0x07,
		DT_UNSIGN, 0x03,
		DT_LONG_U, 0x3c, 0x00,
		DT_LONG_U, 0x1e, 0x00,
		DT_LONG_U, 0x0a, 0x00,
		DT_UNSIGN, 0x05,
		DT_UNSIGN, 0x02,
		DT_UNSIGN, 0x04,
		0x00,
	//0x4008,两套时区表切换时间 DT_DATE_TIME_S,---8byte
	DT_DATE_TIME_S,0x07,0xe0,0x01,0x01,0x00,0x00,0x00,
	//0x4009,两套日时段切换时间 DT_DATE_TIME_S,---8byte
	DT_DATE_TIME_S,0x07,0xe0,0x01,0x01,0x00,0x00,0x00,
	//0x400A,两套分时费率切换时间 DT_DATE_TIME_S,---8byte
	DT_DATE_TIME_S,0x07,0xe0,0x01,0x01,0x00,0x00,0x00,
	//0x400B,两套阶梯电价切换时间 DT_DATE_TIME_S,---8byte
	DT_DATE_TIME_S,0x07,0xe0,0x01,0x01,0x00,0x00,0x00,
	//400C,时区时段数---12 byte
	0x02, 0x05,
		0x11,MAX_ZONE_NUM,
		0x11,MAX_DAY_CHART_NUM,
		0x11,RATE_PERIOD_NUM,
		0x11,RATE_NUM,
		0x11,MAX_HOLIDAY_NUM,
	//400D,阶梯数---2byte
	0x11,0x04,
	//400E,谐波分析次数---2byte
	0x11,0x15,
	//400F,密匙总条数---2byte
	0x11,0x04,
	//4010,计量元件数---2byte
	0x11,0x03,
	//4011,公共假日表，最多20个假日吧---182byte
	0x01,0x14,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
		DT_STRUCT,0x02,DT_DATE,0x07,0xe0,0x01,0x01,0x05,DT_UNSIGN,0x01,
	//4012,周休日特征字---3byte
	DT_BIT_STR,0x08,0x4E,
	//4013,周休日采用的日时段表号---2byte
	DT_UNSIGN,0x01,
	//4014,当前时区表---114byte
	0x01,MAX_ZONE_NUM,//最多14个时区
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
	//4015,备用套时区表----114byte
	0x01,MAX_ZONE_NUM,//最多14个时区
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
	//4016，当前套日时段表----530byte
	0x01,MAX_DAY_CHART_NUM,//最多日时段数暂定为8
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
	//4017,备用套日时段表---530byte
	0x01,MAX_DAY_CHART_NUM,//最多日时段数暂定为8
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
		0x01,RATE_PERIOD_NUM,//日时段表，最多8个
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
			0x02,0x03,DT_UNSIGN,0x01,DT_UNSIGN,0x01,DT_UNSIGN,0x01,
	//4018,当前套费率电价---162byte
	0x01,0x20,//最多支持32个费率电价--只读
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
	//4019,备用套费率电价---162byte
	0x01,0x20,//最多支持32个费率电价--只读
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
		0x06,0x0a,0x00,0x00,0x00,0x06,0x0a,0x00,0x00,0x00,
	//0x401A,当前套阶梯电价----128---只读

	0x02,0x03,
		0x01,0x08,
			0x06,0xc8,0x00,0x00,0x00,0x06,0x90,0x01,0x00,0x00,0x06,0x58,0x02,0x00,0x00,0x06,0x20,0x03,0x00,0x00,0x06,0xe8,0x03,0x00,0x00,0x06,0xdc,0x05,0x00,0x00,0x06,0xd0,0x07,0x00,0x00,0x06,0xb8,0x0b,0x00,0x00,//40
		0x01,0x08,
			0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,//40
		0x01,0x08,//结算日
			0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,
	//0x401B,备用套阶梯电价----128---只读
	0x02,0x03,
		0x01,0x08,
			0x06,0xc8,0x00,0x00,0x00,0x06,0x90,0x01,0x00,0x00,0x06,0x58,0x02,0x00,0x00,0x06,0x20,0x03,0x00,0x00,0x06,0xe8,0x03,0x00,0x00,0x06,0xdc,0x05,0x00,0x00,0x06,0xd0,0x07,0x00,0x00,0x06,0xb8,0x0b,0x00,0x00,//40
		0x01,0x08,
			0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,0x06,0x88,0x13,0x00,0x00,//40
		0x01,0x08,//结算日
			0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,0x02,0x03,0x01,0x01,0x00,
	//0x401C,电流互感器变比---5byte
	0x06,0x64,0x00,0x00,0x00,
	//0x401D,电压互感器变比---5byte
	0x06,0x64,0x00,0x00,0x00,
	//0x401E,报警金额限值---12byte
	0x02,0x02,0x06,0xe8,0x03,0x00,0x00,0x06,0x64,0x00,0x00,0x00,
	//0x401f,其他金额限值---17byte
	0x02,0x02,0x06,0xe8,0x03,0x00,0x00,0x06,0x64,0x72,0x77,0x16,0x06,0x64,0x00,0x00,0x00,

	//0x4020,报警电量限值--12byte
	0x02,0x02,0x06,0xe8,0x03,0x00,
	0x00,0x06,0x64,0x00,0x00,0x00,
	//0x4021,其他电量限值---17byte
	0x02,0x02,0x06,0xe8,0x03,0x00,
	0x00,0x06,0x64,0x72,0x77,0x16,
	0x06,0x64,0x00,0x00,0x00,
	//4022，插卡状态字----4byte
	0x04,0x10,0x00,0x00,
	//4023,认证有效时长---3byte
	0x12,0x05,0x00,
	//4024 剔除
	DT_ENUM, 0x00,
	//4030,电压合格率参数---18byte
	0x02,0x04,0x12,0x00,0x00,0x12,0x00,0x00,0x12,0x00,0x00,0x12,0x00,0x00,
///////////////////begin////////////////////

	//4100,最大需量周期---2byte
	0x11,0x05,
	//4101,划差时间---2byte
	0x11,0x05,
	//4102,校表脉冲宽度---2byte
	0x11,0x05,
	//4103,资产管理编码---34byte
	DT_VIS_STR,0x20,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	//4104,额定电压--8byte
	DT_VIS_STR, 0x06, 
	'2', '2', '0', 'V',	0x00,0x00,
	//4105,额定电流--8byte
	DT_VIS_STR, 0x06, 
	'1', '.', '5', 'A',	0x00,0x00,
	//4106,最大电流--8byte
	DT_VIS_STR, 0x06, 
	'6', '.', '0', 'A',	0x00,0x00,
	//4107,有功准确度等级--6byte
	DT_VIS_STR, 0x04, 
	'1', '.', '0', 0x00,
	//4108,无功准确度等级--6byte
	DT_VIS_STR, 0x04, 
	'2', '.', '0', 0x00,
	//4109,电能表有功常数---5byte
	DT_DB_LONG_U,0x00,0x19,0x00,0x00,
	//410A,电能表无功常数---5byte
	DT_DB_LONG_U,0x00,0x19,0x00,0x00,
	//410B,电能表型号---34byte
	DT_VIS_STR,32,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	//410C,ABC各相电导系数---11byte
	0x02,0x03,0x10,0x00,0x00,16,0x00,0x00,16,0x00,0x00,
	//410D,ABC各相电抗系数---11byte
	0x02,0x03,0x10,0x00,0x00,16,0x00,0x00,16,0x00,0x00,
	//410E,ABC各相电阻系数---11byte
	0x02,0x03,0x10,0x00,0x00,16,0x00,0x00,16,0x00,0x00,
	//410F,ABC各相电纳系数---11byte
	0x02,0x03,0x10,0x00,0x00,16,0x00,0x00,16,0x00,0x00,

	//4110,电能表运行特征子1---3byte
	0x04,0x08,0x00,
	//4111 软件备案号---18byte
	0x0a,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	//4112 有功组合方式特征字--3byte
	0x04,0x08,0x05,
	//4113 无功组合方式1特征字--3byte
	0x04,0x08,0x41,
	//4114 无功组合方式2特征字--3byte
	0x04,0x08,0x14,
	//4115 IC卡---4byte
	0x02,0x01,0x16,0x00,
	//4116 结算日--最多3个结算日，20byte
	0x01,BALANCE_DAY_NUM,
		0x02,0x02,0x11,0x01,0x11,0x00,
		0x02,0x02,0x11,0x0f,0x11,0x00,
		0x02,0x02,0x11,0x19,0x11,0x00,
	//4117 期间需量冻结周期---5byte
	84,0x16,0x01,0x12,0x0f,
	//4200,路由表
	0x00,
	//4201,路由信息单元
	0x00,
	//4202,级联通讯单元
	0x00,
	//4204,广播校时时间
	DT_STRUCT, 2,
		DT_TIME, 0x00, 0x00, 0x00,	//终端广播校时启动时间
		DT_BOOL, 0x00,	//是否启用
	//4205,终端单地址广播校时参数
	DT_STRUCT, 3,
		DT_UNSIGN, 0x01,	// 时钟误差阈值  
		DT_TIME, 0x00, 0x00, 0x00,	//终端广播校时启动时间
		DT_BOOL, 0x00,	//是否启用

	//4302 设备描述符
	DT_VIS_STR, 0x10,
		'0x00', '0x00', '0x00', '0x00', '0x00', '0x00','0x00','0x00',
		'0x00', '0x00', '0x00', '0x00', '0x00', '0x00','0x00','0x00',
	//4303 电气设备
	DT_STRUCT, 0x06, 
		DT_VIS_STR, 0x04, 'C', 'L', 'O', 'U',	//厂商代号 4
		DT_VIS_STR, 0x04, '0', '0', '0', '2',	//软件版本 4 
		DT_VIS_STR, 0x06, '1','6', '1','0','1','3',	//软件版本日期
		DT_VIS_STR, 0x04, 'V', '0', '1', '0',	//硬件版本 4 
		DT_VIS_STR, 0x06, '1','6', '1','0','1','3',	//硬件版本日期
		DT_VIS_STR, 0x08, '0x00', '0x00', '0x00', '0x00', '0x00', '0x00','0x00','0x00',
	//4304 电气设备——生产日期
	DT_DATE_TIME_S,
		0x07, 0xE0, 0x0A, 0x1B, 0x11, 0x2A, 0x1E,	//2016-10-27 17:42:30
	//4305 电气设备——子设备列表
	0x00,
	//4306 电气设备——支持规约列表
	DT_ARRAY, 0x01,
	DT_VIS_STR, 9, 'D','L','/','T','6','9','8','4','5',
	//4307 电气设备——允许跟随上报
	DT_BOOL, 0x00,
	//4308 电气设备——允许主动上报
	DT_BOOL, 0x01,
	//4309 电气设备——允许与主动通话
	DT_BOOL, 0x01,
	//430a 电气设备——上报通道
	0x01,0x02,
	0x51,0x45,0x00,0x00,0x00,
	0x51,0x45,0x10,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
	//4400，应用连接
	0x00,
	//4401，应用连接认证密码
	0x00,

	//4500 公网通信模块1——通讯配置
	DT_STRUCT, 0x0c,
		DT_ENUM, 0x01,	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
		DT_ENUM, 0x00,	//永久在线（0），被动激活（1）
		DT_ENUM, 0x00,	//连接方式 TCP（0），UDP（1）
		DT_ENUM, 0x00,	//连接应用方式 主备模式（0），多连接模式（1）
		DT_ARRAY, 0x01,	//帧听端口列表
			DT_LONG_U, 0x11, 0x22, 
		DT_VIS_STR, 0x05,		//APN
			'c', 'm', 'n',  'e', 't',
		DT_VIS_STR, 0x04,		///用户名
			'c', 'a', 'r', 'd',	
		DT_VIS_STR, 0x04,		///密码
			'c', 'a', 'r', 'd',	
		DT_OCT_STR, 4,	//代理服务器地址
			 0xC0, 0x00, 0x00, 0x01,
		DT_LONG_U,	//代理端口
			0x88, 0x88,
		DT_UNSIGN,	//超时时间及重发次数
			0x7B,	//超时时间30S，重试次数3次 	
		DT_LONG_U,	//心跳周期
			0x01, 0x2C,	//300s
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00,
		

	//4501 公网通信模块1——主站通信参数表
	DT_ARRAY, 2,
		DT_STRUCT, 0x02,
			DT_OCT_STR, 4, 
				0x3A, 0xFB, 0x4A, 0x65,	//58.251.74.101
			DT_LONG_U, 
				0x19, 0x22,	//6434
		DT_STRUCT, 0x02,
			DT_OCT_STR, 4, 
				0x00, 0x00, 0x00, 0x00,
			DT_LONG_U, 
				0x00, 0x00,


	//4502 公网通信模块1——短信通信参数
		DT_STRUCT, 0x03,
			DT_VIS_STR, 8, 	//短信中心号码
				0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			DT_ARRAY, 0x01,
				DT_VIS_STR, 8,	//主站号码
					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			DT_ARRAY, 0x01, 
				DT_VIS_STR, 8,	//短信通知目的号码
					0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,	
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		

	//4503 公网通信模块1——版本信息   备注：这里需要GPRS模块的信息，暂时用终端版本信息！！！
	DT_STRUCT, 0x06, 
		DT_VIS_STR, 0x04, 'C', 'L', 'O', 'U',	//厂商代号 4
		DT_VIS_STR, 0x04, 'V', '1', '.', '1',	//软件版本 4 
		DT_VIS_STR, 0x06, '1','6', '1','0','1','3',	//软件版本日期
		DT_VIS_STR, 0x04, 'V', '0', '1', '0',	//硬件版本 4 
		DT_VIS_STR, 0x06, '1','6', '1','0','1','3',	//硬件版本日期
		DT_VIS_STR, 0x08, '0x00', '0x00', '0x00', '0x00', '0x00', '0x00','0x00','0x00',
	//4504 公网通信模块1——支持规约列表
	0x00,
	//4505 公网通信模块1——SIM卡ICCID
	DT_VIS_STR,0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//4506 公网通信模块1——IMSI
	DT_VIS_STR,0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 
	//4507 公网通信模块1——信号强度
	DT_LONG, 0x00, 0x00,
	//4508 公网通信模块1——SIM卡号码
	DT_OCT_STR, 0x01, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//4509 公网通信模块1——拨号IP
	DT_OCT_STR, 0x04, 
	0x00, 0x00, 0x00, 0x00,
	//0x4510~0x4517 以太网模块属性2——通信配置
	DT_STRUCT, 0x08,
		DT_ENUM, 0x01,	//工作模式 混合模式（0），客户机模式（1），服务器模式（2）
		DT_ENUM, 0x00,	//连接方式 TCP（0），UDP（1）
		DT_ENUM, 0x00,	//连接应用方式 主备模式（0），多连接模式（1）
		DT_ARRAY, 0x01,	//帧听端口列表
			DT_LONG_U, 0x24, 0x54,	//9300
		DT_OCT_STR, 4,	//代理服务器地址
			0xC0, 0x00, 0x00, 0x01,
		DT_LONG_U,	//代理端口
			0x88, 0x88,
		DT_UNSIGN,	//超时时间及重发次数
			0x7B,	//超时时间30S，重试次数3次 	
		DT_LONG_U,	//心跳周期
			0x01, 0x2c,	//300s
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00, 
	0x00,

	//0x4510~0x4517 以太网模块属性3——主站通信参数表
	DT_ARRAY, 2,
		DT_STRUCT, 0x02,
			DT_OCT_STR, 0x04, 
				0xC0, 0xA8, 0x01, 0x64,	//192.168.1.100
			DT_LONG_U, 
				0x23, 0xF0,	//9200
		DT_STRUCT, 0x02,
			DT_OCT_STR, 0x04, 
				0x00, 0x00, 0x00, 0x00,
			DT_LONG_U, 
				0x00, 0x00,

	//0x4510~0x4517 以太网模块属性4——网络配置
	DT_STRUCT, 0x06,
		DT_ENUM, 0x01,	//IP配置方式 DHCP（0）,静态（1），PPPoE（2）
		DT_OCT_STR, 0x04,	//IP地址	192.168.1.200
			0xC0, 0xA8, 0x01, 0xC8, 
		DT_OCT_STR, 0x04,	//子网掩码	255.255.255.0
			0xFF, 0xFF, 0xFF, 0x00,
		DT_OCT_STR, 0x04,	//网关地址	192.168.1.1
			0xC0, 0xA8, 0x01, 0x01, 
		DT_VIS_STR, 0x04,	//PPPoE用户名
			'T',	'E',   'S',	'T',   
		DT_VIS_STR, 0x04,	//PPPoE密码
			'T',	'E',   'S',	'T',   
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 

	//0x4510~0x4517 以太网模块属性5——MAC地址
	DT_MAC, 0x06,
		0x11, 0x22, 0x33, 0x44, 0x55, 0x66,

	//0x4520 日期时间，属性3，校时模式
	DT_ENUM, 0x00,	//主站授时（0），终端精确校时（1），北斗/GPS（2），其它（255）
	//0x4521 日期时间，属性4，精准校时参数
	DT_STRUCT, 0x05,
		DT_UNSIGN, 0x00,	//最近心跳时间总个数
		DT_UNSIGN, 0x00,	//最大值剔除个数
		DT_UNSIGN, 0x00,	//最小值剔除个数
		DT_UNSIGN, 0x00,	//通讯延时阈值
		DT_UNSIGN, 0x00,	//最少有效个数
};

TItemDesc g_FrzDesc[] =   //冻结类对象标识定义
{
	{0x4FF0, 	BN_VER_LEN,				DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,			FMT_UNK,		1,				},//版本控制
	{0x5000, 	FRZRELA_ID_LEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_FRZPARA_CHG,	FMT_UNK,		FRZ_TYPE_NUM,	},//冻结关联属性表
};

BYTE g_FrzDescDefault[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)

	0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //300
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //600
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x5000 834Bytes
};



TItemDesc g_CollecMonitorDesc[] =   //采集监控类对象标识定义
{
    {0x5FF0, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//版本控制
	{0x6000, 	PNPARA_LEN, DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		POINT_NUM,		},//采集档案配置表，属性2，方法127~134
	{0x6001, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//采集档案配置单元，属性2，

	{0x6002, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//所有搜表结果
	{0x6003, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//跨台区搜表结果
	{0x6004, 	3, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//所有搜表结果记录数
	{0x6005, 	3, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//跨台区搜表结果记录数
	{0x6006, 	10, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//搜表参数
	{0x6007, 	38, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		MAX_TIME_SCH_MTR_NUM,},//每天周期搜表参数配置（定时搜表参数定时搜表参数）
	{0x6008, 	2, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//搜表状态
	{0x6009, 	3, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_SCH_MTR,FMT_UNK,		1,		},//搜表时长
	{0x600A, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//清空搜表结果
	{0x600B, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//清空跨台区搜表结果

	{0x6012, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//任务配置表，属性2~4，方法127~129
	{0x6013, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//任务配置单元，属性2
	{0x6014, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//普通采集方案集，属性2，方法127~130
	{0x6015, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//普通采集方案，属性2
	{0x6016, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//事件采集方案集，属性2，方法127~130
	{0x6017, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//事件采集方案，属性2，
	{0x6018, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//透明方案集，属性2，方法127~131
	{0x6019, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//透明方案，属性2，
	{0x601A, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//透明方案结果集，属性2，
	{0x601B, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//一个透明方案结果
	{0x601C, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//上报方案集，属性2，方法127~129
	{0x601D, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//上报方案，属性2
	
	{0x601E, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//采集规则库，属性2，方法127~129

	{0x6032, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//采集状态集，属性2
	{0x6033, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//一个采集状态
	{0x6034, 	34, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	TASK_NUM,	},//采集任务监控集，属性2
	{0x6035, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//采集任务监控单元

	{0x6040, 	7, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//采集启动时标，属性2
	{0x6041, 	7, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//采集成功时标，属性2
	{0x6042, 	7, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//采集存储时标，属性2

	{0x6051, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//实时监控采集方案集，属性2，方法127~130
	{0x6052, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//实时监控采集方案，属性2

	{0x6700, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//添加一个采集档案配置单元，配置单元的信息中基本信息必须是完整的
	{0x6701, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//批量添加采集档案配置单元
	{0x6702, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//更新配置单元的基本信息对象
	{0x6703, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//更新配置单元的扩展信息以及附属信息，对象为NULL表示不更新
	{0x6704, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//删除配置单元，通过配置序号删除
	{0x6705, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//删除配置单元，通过基本信息对象删除
	{0x6706, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//删除配置单元，通过通信地址及端口删除
	{0x6707, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//清空采集档案配置表

};

TItemDesc g_SetDesc[] = //集合类对象
{
    {0x6FF0, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//版本控制
	{0x7000, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//文件集合，方法127~129
	{0x7001, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//文件，属性2,

	{0x7010, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//脚本集合，方法127~130
	{0x7011, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//脚本，属性2
	{0x7012, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//脚本执行结果集，属性2
	{0x7013, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//一个脚本执行结果
	{0x7100, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//扩展变量对象集合，属性2
	{0x7101, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//扩展参变量对象集合，属性2
};

TItemDesc g_CtrlDesc[] = //控制类对象
{
    {0x7FF0, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//版本控制
	{0x8000, 	10, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//遥控，属性2
	{0x8001, 	2, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//保电，属性2
	{0x8002, 	2, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//催费告警，属性2
	{0x8003, 	216, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXCOMCHNNOTE,			},//一般中文信息，属性2
	{0x8004, 	216, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXIMPCHNNOTE,			},//重要中文信息，属性2

	{0x8100, 	9, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端保安定值
	{0x8101, 	26,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//终端功控时段，属性2
	{0x8102, 	18,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//功控告警时间，属性2 
	{0x8103, 	242,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//时段功控，属性2
	{0x8104, 	28,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//厂休控，属性2
	{0x8105, 	30,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//营业报停控，属性2
	//{0x8106, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//当前功率下浮控，属性2，方法127，
	{0x8107, 	43,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//购电控，属性2
	{0x8108, 	18,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//月电控，属性2
	{0x8109, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//时段功控配置单元，属性2
	{0x810A, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//厂休控配置单元，属性2
	{0x810B, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//营业报停控配置单元，属性2
	{0x810C, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//购电控配置单元，属性2
	{0x810D, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//月电控配置单元，属性2
	{0x810E, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//控制对象
	{0x810F, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//跳闸轮次
	{0x8110, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电控定值

	{0x8200, 	2, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//遥控，属性3（继电器输出状态，只读)
	{0x8201, 	2, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//遥控，属性4(告警状态，只读)
	{0x8202, 	2,	 	DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		1,		},//遥控，属性5（命令状态，只读)
	{0x8203, 	8,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXCONTROLTURN,			},//遥控命令

	{0x8210, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//保电，属性3,允许最大无通讯时间
	{0x8211, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//保电，属性4,上电自动保电时间
	{0x8212, 	146, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//保电，属性5,自动保电时段(最多24个时段)
	{0x8213, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//保电投入命令

	{0x8220, 	207, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//催费告警投入（参数）

	{0x8230, 	7,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//时段功控，属性3,投入状态
	{0x8231, 	8,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//时段功控，属性4,输出状态
	{0x8232, 	7, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//时段功控，属性5,越限告警状态
	{0x8233, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//时段功控投入命令

	{0x8240, 	7,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//厂休控，属性3,投入状态
	{0x8241, 	8,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//厂休控，属性4,输出状态
	{0x8242, 	7, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//厂休控，属性5,越限告警状态
	{0x8243, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//厂休控投入命令

	{0x8250, 	7,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//营业报停控，属性3,投入状态
	{0x8251, 	8,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//营业报停控，属性4,输出状态
	{0x8252, 	7, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//营业报停控，属性5,越限告警状态
	{0x8253, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//营业报停控投入命令

	{0x8260, 	7,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//当前功率下浮控，属性3,投入状态
	{0x8261, 	8,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//当前功率下浮控，属性4,输出状态
	{0x8262, 	7, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//当前功率下浮控，属性5,越限告警状态
	{0x8263, 	19,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//当前功率下浮控投入命令

	{0x8270, 	7,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//购电控，属性3,投入状态
	{0x8271, 	8,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//购电控，属性4,输出状态
	{0x8272, 	7, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//购电控，属性5,越限告警状态
	{0x8273, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//购电控投入命令

	{0x8280, 	7,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//月电控，属性3,投入状态
	{0x8281, 	8,		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//月电控，属性4,输出状态
	{0x8282, 	7, 		DI_HIGH_PERM, DI_READ,			0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//月电控，属性5,越限告警状态
	{0x8283, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GRP_NUM,},//月电控投入命令
};

TItemDesc g_FileTransDesc[] = //文件传输类对象标识定义
{
	{0xF000, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//文件分帧传输管理，属性4，属性5
	{0xF001, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//文件分块传输管理，属性4，方法7~10
	{0xF002, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//文件扩展传输管理，属性4，方法7，方法8

};

TItemDesc g_ESAMIfDesc[] = //EASM接口类对象
{
	{0xF102, 	18,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	{0xF103, 	18,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},
	{0xF104,	34,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF105,	38,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF106,	5,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF107,	5,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF108,	17,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF109,	34,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF10A,	2052,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF10B,	34,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF10C,	2052,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},

	{0xF112,	2,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF113,	130,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,		1,		},
	{0xF114, 	18,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PN_NUM,	}, //电表ESAM序列号
	{0xF115, 	16,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PN_NUM,	}, //电表随机数
};

TItemDesc g_InOutDevDesc[] = //输入输出设备类对象
{
    {0xF1F0, 	BN_VER_LEN, DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//版本控制
	{0xF200, 	28, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_RS232_PARACHG,	FMT_UNK,	MAX_232_PORT_NUM,		},//RS232，属性2，方法127
	{0xF201, 	28, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_RS485_PARACHG,	FMT_UNK,	MAX_485_PORT_NUM,		},//RS485-1, RS485-2, RS485-3, 属性2，方法127
	{0xF202, 	26, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_INFRA_PARACHG,	FMT_UNK,	MAX_HW_PORT_NUM,		},//红外，属性2，方法127
	{0xF203, 	6, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,			MAX_SW_PORT_NUM,		},//开关量输入，属性2
//		{0xF204, 	22, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,					MAX_DC_PORT_NUM,		},//直流模拟量，属性2，属性4
	{0xF205, 	26, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_RELAY_PARACHG,	FMT_UNK,			MAX_RLY_PORT_NUM,		},//继电器输出，属性2，方法127
	{0xF206, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,			MAX_ALRM_PORT_NUM,		},//告警输出，属性2，属性4
	{0xF207, 	2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,			MAX_MUL_PORT_NUM,		},//多功能端子，属性2，方法127
	{0xF208,	20,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_MULPORT_PARACHG,	FMT_UNK,			1,						},//交采接口，属性2，方法127
	{0xF209, 	45, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_PLC_PARACHG,	FMT_UNK,			MAX_PLC_PORT_NUM,		},//载波/微功率无线接口，属性2，方法127
	{0xF20A, 	18, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,			MAX_PLUS_PORT_NUM,		},//脉冲输入设备，属性2
//		{0xF20B, 	1, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,					1,		},//蓝牙，属性2
	{0xF800, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_YX_PARA,	FMT_UNK,		MAX_SW_PORT_NUM,		},//开关量输入，属性4
	{0xF801,	10, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0,	INFO_NONE,	FMT_UNK,			MAX_SW_PORT_NUM,		},//告警输出，属性4

};

BYTE g_InOutDevDefault[] = {	 //输入输出设备类对象
	//F1F0
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)
	
	//F200属性2--RS232端口
	DT_STRUCT, 0x03,
		DT_VIS_STR, 0x10,	//端口描述符
			'0', '0', '0', '0', '0', '0', '0', '0',
			'0', '0', '0', '0', '0', '0', '0', '0',
		DT_COMDCB,	//端口参数
			0x06,	//9600bps
			0x00,	//无校验
			0x08,	//8位数据位
			0x01,	//停止位
			0x00,	//流控
		DT_ENUM,	//端口功能
			0x00,	//上行通信（0），抄表（1），级联（2），停用（3）
	//F201属性2--RS485端口1\2\3
	DT_STRUCT, 0x03,
		DT_VIS_STR, 0x10,	//端口描述符
			'0', '0', '0', '0', '0', '0', '0', '0',
			'0', '0', '0', '0', '0', '0', '0', '0',
		DT_COMDCB,	//端口参数
			0x03,	//2400bps
			0x02,	//无校验
			0x08,	//8位数据位
			0x01,	//停止位
			0x00,	//流控
		DT_ENUM,	//端口功能
			0x01,	//上行通信（0），抄表（1），级联（2），停用（3）
	//F202属性2--红外
	DT_STRUCT, 0x02,
		DT_VIS_STR, 0x10,	//端口描述符
			'0', '0', '0', '0', '0', '0', '0', '0',
			'0', '0', '0', '0', '0', '0', '0', '0',
		DT_COMDCB,	//端口参数
			0x03,	//2400bps
			0x02,	//无校验
			0x08,	//8位数据位
			0x01,	//停止位
			0x00,	//流控
	//F203属性2--开关量
	DT_STRUCT, 0x02,
		DT_UNSIGN, 0x00,	//状态ST
		DT_UNSIGN, 0x00,	//变位CD 

	//F205属性2--继电器
	DT_STRUCT, 0x04,
		DT_VIS_STR, 0x10,	//端口描述符
			'0', '0', '0', '0', '0', '0', '0', '0',
			'0', '0', '0', '0', '0', '0', '0', '0',
		DT_ENUM,	//当前状态 
			0x01,	//未输出（0），输出（1）
		DT_ENUM,	//开关属性 
			0x01,	//脉冲式（0），保持式（1）
		DT_ENUM,	//接线状态
			0x01,	//接入（0），未接入（1)
	//F206属性2--告警输出
	DT_ENUM,	//告警输出
		0x00,	//未输出（0），输出（1）
	//F207属性2--多功能端子
	DT_ENUM,	//功能
		0x00,	//秒脉冲输出（0），需量周期  （1），时段投切  （2）
	//F208属性2--交采接口
	DT_STRUCT, 0x01,
		DT_VIS_STR, 0x10,	//交采描述符
			'0', '0', '0', '0', '0', '0', '0', '0',
			'0', '0', '0', '0', '0', '0', '0', '0',
	//F209属性2--载波/微功率无线接口
	DT_STRUCT, 0x03,
		DT_VIS_STR, 0x10,	//端口描述符
			'0', '0', '0', '0', '0', '0', '0', '0',
			'0', '0', '0', '0', '0', '0', '0', '0',
		DT_COMDCB,	//通信参数
			0x06,	//9600bps
			0x02,	//无校验
			0x08,	//8位数据位
			0x01,	//停止位
			0x00,	//流控
		DT_STRUCT, 0x04,//版本信息
			DT_VIS_STR, 0x02,	//厂商代码
				'0', '0',
			DT_VIS_STR, 0x02,	//芯片代码 
				'0', '0',
			DT_DATE, 	//版本日期
				0x00,0x00,0x00,0x00,0x00,
			DT_LONG_U,	//软件版本
				0x00,0x00,
	//F20a属性2--脉冲输入设备
	DT_VIS_STR, 0x10,	//脉冲输入端口描述符
		'0', '0', '0', '0', '0', '0', '0', '0',
		'0', '0', '0', '0', '0', '0', '0', '0',
	
	//F800属性4--开关量
	DT_STRUCT, 0x02,
		DT_BIT_STR, 0x08,	//开关量接入标志
			0x00,
		DT_BIT_STR, 0x08,	//开关量属性标志 
			0x00,
	
	//F801属性4--告警输出
	DT_STRUCT, 0x02,
		DT_TIME, 	//起始时间
			0x00,0x00,0x00,
		DT_TIME,	//结束时间
			0x00,0x00,0x00,
};



TItemDesc g_DisPlayDesc[] = //输入输出设备类对象
{
	{0xF300, 	15, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//自动轮显，属性2，显示对象列表
	{0xF301, 	15, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//按键轮显，属性2，显示对象列表
	{0xF900, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//自动轮显，属性3，显示时间
	{0xF901, 	3, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//按键轮显，属性3，显示时间
	{0xF902, 	6, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//自动轮显，属性4，显示参数
	{0xF903, 	6, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//按键轮显，属性4，显示参数
};


//保留原有的一些电量，需量数据
TItemDesc g_AcDataDesc[] = 
{//----标识-----长度------------权限-----------读写--------偏移----写操作-------格式----------Pn个数------
	//组合有功电能示值
	{0x9000,    ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
	{0x9001,    ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
	{0x9002,    ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
	{0x9003,    ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
	{0x9004,    ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
	{0x900f,    0,		    DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},

	//正向有功电能
    {0x9010,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9011,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9012,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9013,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9014,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x901f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    //反向有功电能
    {0x9020,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9021,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9022,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9023,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9024,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x902f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},

	//分相正向有功电能
    {0x9070,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9071,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9072,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x907f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    
	//分相反向有功电能
    {0x9080,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9081,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x9082,	ELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},
    {0x908f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		EFMT,		1},

	//正向无功电能
    {0x9110,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9111,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9112,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9113,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9114,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x911f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},

	//反向无功电能
    {0x9120,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9121,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9122,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9123,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9124,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x912f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
	//一象限无功电能
    {0x9130,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9131,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9132,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9133,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9134,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x913f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},

	//四象限无功电能
    {0x9140,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9141,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9142,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9143,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9144,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x914f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},

	//二象限无功电能
    {0x9150,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9151,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9152,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9153,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9154,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x915f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},

	//三象限无功电能
    {0x9160,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9161,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9162,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9163,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9164,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x916f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},

	//分相感性无功电能
    {0x9170,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9171,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9172,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x917f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    
	//分相容性无功电能
    {0x9180,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9181,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x9182,	RELEN,		DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},
    {0x918f,	0,			DI_LOW_PERM,	DI_READ,		0,	INFO_NONE,		REFMT,		1},	
};

//------------------------------------------------------------------------------------------------------
//测量点数据描述表：标识--长度--权限--读写--偏移
TItemDesc  g_PointDataDesc[] = 
{//----标识-----长度------------权限-----------读写--------偏移----写操作-------格式----------Pn个数------
	//组合有功电能，带格式，02 05 06 ********
	{0xa000, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//组合有功电能，带格式，02 05 06 ********
	//正向有功电能，总，1~4费率
	{0xa010, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//正向有功电能，带格式，
	//反向有功电能，总，1~4费率
	{0xa020, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//反向有功电能，带格式，
	//组合无功1电能，总，1~4费率
	{0xa030, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//组合无功1电能，带格式，
	//组合无功2电能，总，1~4费率
	{0xa040, 	27, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//组合无功2电能，带格式，

	{0xa050, 	11, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电压A/B/C，带格式，02 03 12 ********
	{0xa051, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//电流A/B/C/零序，带格式，
	{0xa052, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//有功功率总，A/B/C，带格式，
	{0xa053, 	22, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//无功功率总，A/B/C，带格式，
	{0xa054, 	14, 		DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//功率因素总，A/B/C，带格式，
};



//sect15 Ext-Variable-Para
TItemDesc g_ExtVarParaDesc[] = 
{
// 区间统计参数ID,2+5+(2+5*10)+2+4=65,按最大10个超限参数来考虑
	{0x2100, 	65*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS14_STAT_CHG,	FMT_UNK,		1,		},//分钟区间统计
	{0x2101, 	65*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS14_STAT_CHG,	FMT_UNK,		1,		},//小时区间统计
	{0x2102, 	65*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS14_STAT_CHG,	FMT_UNK,		1,		},//日区间统计
	{0x2103, 	65*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS14_STAT_CHG,	FMT_UNK,		1,		},//月区间统计
	{0x2104, 	65*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS14_STAT_CHG,	FMT_UNK,		1,		},//年区间统计
//累加平均统计参数ID, 2+5+2+4=13
	{0x2110, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS15_STAT_CHG,	FMT_UNK,		1,		},//分钟平均
	{0x2111, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS15_STAT_CHG,	FMT_UNK,		1,		},//小时平均
	{0x2112, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS15_STAT_CHG,	FMT_UNK,		1,		},//日平均
	{0x2113, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS15_STAT_CHG,	FMT_UNK,		1,		},//月平均
	{0x2114, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS15_STAT_CHG,	FMT_UNK,		1,		},//年平均
//极值统计参数ID,2+5+2+4=13
	{0x2120, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS16_STAT_CHG,	FMT_UNK,		1,		},//分钟极值
	{0x2121, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS16_STAT_CHG,	FMT_UNK,		1,		},//小时极值
	{0x2122, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS16_STAT_CHG,	FMT_UNK,		1,		},//日极值
	{0x2123, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS16_STAT_CHG,	FMT_UNK,		1,		},//月极值
	{0x2124, 	13*STAT_OAD_NUM+2, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_CLASS16_STAT_CHG,	FMT_UNK,		1,		},//年极值
	//脉冲接口类参数
	{0x2401, 	(TSA_LEN+1), 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//通信地址
	{0x2402, 	8,	 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,		},//互感器倍率
	{0x2403, 	PULSE_CFG_ID_LEN, 	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_PULSE,	FMT_UNK,		PULSE_PN_NUM,		},//脉冲配置	

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Bank1 数据库定义

//终端参数描述表：标识--长度--权限--读写--偏移
TItemDesc  g_Bank1Desc[] =   //标准版
{ 
	{0x1001, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //调试输出自动关断时间,单位分钟,0不自动关断
	{0x1002, 16,DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //调试输出开关
	{0x100f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},

	//0
	{0x2000, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},  //地方版本,01广东版,02江西,03承德
	{0x200f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},

	//1
    {0x2010, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //扩展协议中用到的本公司的厂商编号
    {0x2011, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_COMM_RLD},    //心跳间隔,单位分钟HEX
    {0x2012, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},  //GPRS模块型号,0表示GR47 1.3版,1表示SIM,2表示WAVECOM,3表示华为,4表示GR47 1.5版
	{0x2013, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_WK_PARA},    //GPRS连接等待时间 HEX 秒
	{0x201f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    

	//2
    {0x2020, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //功控来源
	{0x2021, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //电控来源
	{0x2022, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},  //继电器输出方式,0电平,1脉冲
	{0x2023, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //对比度 
	{0x2024, 32, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //任务使能屏蔽
	{0x202f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    

	{0x2030, 16,DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	  //校准参数
	{0x2031, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	  //三相三线的电压互感器数量,0或2:2个, 3:3个
	{0x2032, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},  //TCP/IP协议库：0模块自带,1终端
	{0x2033, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //CT一次侧短路高值，HEX
	{0x2034, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //CT二次侧短路高值，HEX
	{0x2035, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //CT二次侧开路低值，HEX
	{0x2036, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //CT一次侧短路不平衡值，HEX
	{0x2037, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //CT二次侧短路低值，HEX
	{0x2038, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //CT检测投退,0退出,1投入
	{0x2039, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //无压百分比
	{0x203a, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //无流百分比
	{0x203b, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //负荷过载相对变压器容量的比例
	{0x203c, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //负荷过载恢复相对变压器容量的比例
	{0x203d, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //电表异常的判断间隔,BCD,单位分钟
	{0x203e, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_COMM_RLD},    //重连方案等价于8a60,RRTT BCD RR重连次数,缺省3次,TT重连等待间隔,缺省30分钟	
	{0x203f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    
	
	{0x2040, 16, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //告警次数限制,定义与8033类似,0限定每天送一次,1不限定
	{0x2041, 48, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //PPP用户名
	{0x2042, 48, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //PPP密码
	{0x2043, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},     //通信错误复位终端时间,BCD,HHMM,0表示不复位
	{0x204f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    

	{0x2050, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //电表->645费率对照表0，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	{0x2051, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //电表->645费率对照表1，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	{0x2052, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //电表->645费率对照表2，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	{0x2053, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //电表->645费率对照表3，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	{0x2054, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //电表->645费率对照表4，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	{0x2055, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //电表->645费率对照表5，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	{0x2056, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //电表->645费率对照表6，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	{0x2057, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //电表->645费率对照表7，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	{0x2058, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //电表->645费率对照表8，N4N3N2N1,N1表示电表的第一个费率对应645的费率
    {0x205f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},    //
	
	{0x2060, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	    //抄表时间间隔,BCD,1-15,
	{0x2061, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //测量点0等价于892a判断负荷过载相对额定值的比例（缺省为1） 
	{0x2062, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //测量点1等价于892a判断负荷过载相对额定值的比例（缺省为1） 
	{0x2063, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //测量点2等价于892a判断负荷过载相对额定值的比例（缺省为1） 
	{0x2064, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //测量点3等价于892a判断负荷过载相对额定值的比例（缺省为1） 
	{0x2065, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //测量点4等价于892a判断负荷过载相对额定值的比例（缺省为1） 
	{0x2066, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //测量点5等价于892a判断负荷过载相对额定值的比例（缺省为1） 
	{0x2067, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //测量点6等价于892a判断负荷过载相对额定值的比例（缺省为1） 
	{0x2068, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //测量点7等价于892a判断负荷过载相对额定值的比例（缺省为1） 
	{0x2069, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //测量点8等价于892a判断负荷过载相对额定值的比例（缺省为1） 
	{0x206f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //
	
	{0x2070, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //等价于8038启动电压(NN)缺省为额定的70%，恢复电压(MM)缺省为额定的85%，启动电流(LL)缺省为额定的10% 
	{0x2071, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //8065 判断断相、缺相时间，缺省5分钟
	{0x2072, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //8066判断断相、缺相恢复时间，缺省5分钟
	{0x2073, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //8067判断电压逆向序时间，缺省1分钟
	{0x2074, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //8068判断电压逆向序恢复时间，缺省1分钟
	{0x2075, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //8069判断电流反极性时间，缺省值1分钟
	{0x2076, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //806a判断电流反极性恢复时间，缺省值1分钟
	{0x2077, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //806b时钟误差阈值，缺省10分钟
	{0x2078, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //806c电能表飞走阈值，缺省为10倍
	{0x2079, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //806d电能表停走阈值：电量示度停止增长时按照有功功率计算应走电量值，越此值则电表停走，缺省值为0.1kWh
	{0x207f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //	
	
	{0x2080, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //CT正常时电流值
	{0x2081, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //CT一次侧短路电流低值
	{0x2082, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //CT一次侧短路电流高值
	{0x2083, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //CT二次侧短路电流低值
	{0x2084, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //CT二次侧短路电流高值
	{0x2085, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //CT二次侧开路电流低值
	{0x2086, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},		//对应位置1则测量点1的瞬时功率值取自测量点1的15分钟的平均功率值
	{0x208f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //	
	
	{0x2090, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},		//终端电流互感器型号,0--5A,1--10A
	{0x2091, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_WK_PARA},   //休眠到模块掉电次数,0表示不允许给模块掉电,HEX
	{0x209f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},      //
	
	{0x2100, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_APP_RST},  //863终端功能：1:集中器，2:专变终端
	{0x210f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	
	{0x2110, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},  //控制模块电平控制方式
	{0x211f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},		
	//备用空间
	{0x2f01, 15, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //备用空间
	{0x2f0f,  0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},

	{0x3000,  2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//面向对象--服务器模式下的心跳周期次数
	
};

BYTE g_bBank1Default[] =   
{
	0x1e, 0x00,     //0x1001 2 调试输出自动关断时间,单位分钟,0不自动关断
	0x99, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 
	//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                //0x1002 16 各项的调试输出开关
	0x01,           //0x2000 1 地方版本,01广东版,02江西,03承德

	//1
	0x00,   //0x2010 1 厂商编号
	0x0f,   //0x2011 1 心跳间隔,单位分钟HEX
	0x16,	//0x2012 1 GPRS模块型号,ff表示自动识别,0表示GR47 1.3版,1表示SIM,2表示WAVECOM,3表示华为,4表示GR47 1.5版
	0x1e,   //0x2013 1 GPRS连接等待时间 HEX 秒
	        
	//2
	0x00,   //0x2020 1 功控来源
	0x00,   //0x2021 1 电控来源
	0x01,	//0x2022 1 继电器输出方式,0电平,1脉冲
	0x1a,   //0x2023 1 对比度 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //0x00~0x3f  //0x2024 32 任务使能屏蔽
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,  //0x40~0x7f	
  	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  //0x80~0xbf
  	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  //0xc0~0xff
  	
  	0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10,  //0x2030  校准参数，默认4096
	0x00, 0x10, 0x00, 0x10, 0x00, 0x10, 0x00, 0x10,  //0x2030  校准参数
	
	//后加参数
	0x02, //0x2031 1 三相三线的电压互感器数量,0或2:2个, 3:3个
	0x01, //0x2032 1 TCP/IP协议库：0模块自带,1终端
	0x28, 0x00,	  //0x2033 2 CT一次侧短路高值，HEX
	0x80, 0x00,   //0x2034 2 CT二次侧短路高值，HEX
	0x08, 0x00,	  //0x2035 2 CT二次侧开路低值，HEX
	0x07, 0x00,	  //0x2036 2 CT一次侧短路不平衡值，HEX
	0x0d, 0x00,   //0x2037 2 CT二次侧短路低值，HEX
	0x00, //0x2038 1 CT检测投退,0退出,1投入

	0x10,   //0x2039 1 无压百分比
	0x10,   //0x203a 1 无流百分比
	0x10,	//0x203b 1 负荷过载相对变压器容量的比例
	0x10,	//0x203c 1 负荷过载恢复相对变压器容量的比例
	0x01,	//0x203d 1 电表异常的判断间隔,BCD,单位分钟
	0x00, 0x05,  //0x203e 2 重连方案,RRTT BCD RR重连次数,缺省3次,TT重连等待间隔,缺省30分钟	
	
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, //0x2040 16 告警次数限制,定义与8033类似,0限定每天送一次,1不限定
	'C',  'A',  'R',  'D',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x2041 48 PPP用户名	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	'C',  'A',  'R',  'D',  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0x2042 48 PPP密码
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

	0x00, 0x00,	 //0x2043 2 通信错误复位终端时间,BCD,HHMM
	
	
	0x00, 0x00, 0x00, 0x00, //0x2050 4 电表->645费率对照表0，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	0x00, 0x00, 0x00, 0x00, //0x2051 4 电表->645费率对照表1，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	0x00, 0x00, 0x00, 0x00, //0x2052 4 电表->645费率对照表2，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	0x00, 0x00, 0x00, 0x00, //0x2053 4 电表->645费率对照表3，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	0x00, 0x00, 0x00, 0x00, //0x2054 4 电表->645费率对照表4，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	0x00, 0x00, 0x00, 0x00, //0x2055 4 电表->645费率对照表5，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	0x00, 0x00, 0x00, 0x00, //0x2056 4 电表->645费率对照表6，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	0x00, 0x00, 0x00, 0x00, //0x2057 4 电表->645费率对照表7，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	0x00, 0x00, 0x00, 0x00, //0x2058 4 电表->645费率对照表8，N4N3N2N1,N1表示电表的第一个费率对应645的费率
	
	0x01,//0x2060 抄表时间间隔
	
	0x20, 0x01, //0x2061 判断负荷过载相对额定值的比例 (缺省为1)
	0x20, 0x01, //0x2062 判断负荷过载相对额定值的比例 (缺省为1)
	0x20, 0x01, //0x2063 判断负荷过载相对额定值的比例 (缺省为1)
	0x20, 0x01, //0x2064 判断负荷过载相对额定值的比例 (缺省为1)
	0x20, 0x01, //0x2065 判断负荷过载相对额定值的比例 (缺省为1)
	0x20, 0x01, //0x2066 判断负荷过载相对额定值的比例 (缺省为1)
	0x20, 0x01, //0x2067 判断负荷过载相对额定值的比例 (缺省为1)
	0x20, 0x01, //0x2068 判断负荷过载相对额定值的比例 (缺省为1)
	0x20, 0x01, //0x2069 判断负荷过载相对额定值的比例 (缺省为1)
	
	0x85, 0x10, 0x70, //0x2070 3 启动电压 恢复电压 启动电流 等价于8038
	
	0x05,//2071
	0x05,//2072
	
	0x01,//2073
	0x01,//2074
	
	0x01,//2075
	0x01,//2076
	
	0x10,//2077
	
	0x10,//2078
	
	0x10, 0x00,//2079
	
	//表8
	0x00, 0x01,//2080
	0x05, 0x00,//2081
	0x20, 0x00,//2082
	0x05, 0x00,//2083
	0x20, 0x00,//2084
	0x40, 0x00,//2085
	
	0x00,//2086
	0x01, //0x2090 1 终端电流互感器型号,0--5A,1--10A
	0x02, //0x2091 1 休眠到模块掉电次数,HEX
	0x01, //0x2100， 863终端功能 1：集中器， 2：专变终端，其他：集中器
	0x00, // 0x2110控制模块电平控制方式1-脉冲方式,0-电平方式
	0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00,  //0x2fX1 16 密码

	//0x3000
	0x00, 0x10,	//1000次
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////
//Bank2 数据库定义

//终端参数描述表：标识--长度--权限--读写--偏移
TItemDesc  g_Bank2Desc[] =   //标准版
{ 
	//----------各版本公用的非保存数据放到0x2000前(驱动,交采等)-----------
	{0x1001, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A相CT值,HEX
	{0x1002, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A相CT值,HEX
	{0x1003, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A相CT值,HEX
    {0x1004, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	 //A相CT结果 0:NORMAL 1:SHORT1 2 :SHORT2 3:OPEN2
    {0x1005, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	 //B相CT结果
    {0x1006, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	 //C相CT结果
	{0x100f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	
	{0x1011, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A相电压相角,BCD
	{0x1012, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //B相电压相角,BCD
	{0x1013, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //C相电压相角,BCD
	{0x1014, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A相电流相角,BCD
	{0x1015, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //B相电流相角,BCD
	{0x1016, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //C相电流相角,BCD
	{0x101f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	
	{0x1021, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //终端复位次数,HEX
	{0x1022, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //最大AD间隔,HEX
	{0x1023, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //最小AD间隔,HEX
	{0x1024, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //当前AD间隔,HEX
	{0x1025, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //AD复位次数,HEX
	{0x1026, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //频率跟踪复位次数,HEX
	{0x1027, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //频率,HEX,实际频率*256
	{0x1028, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //电池电压 NN.NN V
	{0x1029, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //线程监控复位次数,HEX
	{0x102a, 32,DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //线程监控复位最后一次复位的线程名称
	{0x102f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	
	{0x1031, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ua NNNNN.N
	{0x1032, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ub
	{0x1033, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Uc
	{0x1034, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ia NNN.NNN
	{0x1035, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ib
	{0x1036, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ic
	{0x103f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ic

	{0x1041, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //实验室状态,1处于实验室状态,0不处于
	{0x1042, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //终端当前费率号
	{0x1043, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //终端当前时段表号
	{0x1044, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //终端时钟电池电压 NN.NN V
	{0x1045, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //终端内部温度
	{0x1046, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //第一路模拟量
	{0x1047, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //第二路模拟量
	
	{0x1051, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A相CT值,HEX
	{0x1052, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A相CT值,HEX
	{0x1053, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //A相CT值,HEX
	{0x1054, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //电网频率,HEX
	{0x1055, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //视在功率,NNNN.NNNN
	{0x1056, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //本地IP
	{0x1057, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //硬件版本
	{0x1058, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //GPRS信号强度

	{0x1060, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    //总相有功功率
	{0x1061, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // A相有功功率
	{0x1062, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // B相有功功率
	{0x1063, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // C相有功功率
	{0x1064, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    //总相无功功率
	{0x1065, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // A相无功功率
	{0x1066, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // B相无功功率
	{0x1067, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // C相无功功率
	{0x1068, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    //总相功率因素
	{0x1069, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // A相功率因素
	{0x106a, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // B相功率因素
	{0x106b, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    // C相功率因素
	{0x106f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE,	FMT_BIN,  	 1},    

	{0x1070, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//记录当前是否发生了通信故障
    
    {0x10d0, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 	//当前状态
	{0x10d1, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 	//上一次错误类型
	{0x10d2, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 	//休眠剩余时间
	{0x10d3, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 	//模块类型

	{0x1100, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //8路遥信，每位对应一路遥信(Hex)
	{0x1101, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉1(Hex)
	{0x1102, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉2(Hex)
	{0x1103, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉3(Hex)
	{0x1104, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉4(Hex)
	{0x1105, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉5(Hex)
	{0x1106, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉6(Hex)
	{0x1107, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉7(Hex)
	{0x1108, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉8(Hex)
	{0x110f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //以上数据项集合	

	{0x1110, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //8路遥信，每位对应一路遥信(Hex)
	{0x1111, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉9(Hex)
	{0x1112, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉10(Hex)
	{0x1113, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉11(Hex)
	{0x1114, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉12(Hex)
	{0x1115, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉13(Hex)
	{0x1116, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉14(Hex)
	{0x1117, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉15(Hex)
	{0x1118, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //遥脉16(Hex)
	{0x111f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //以上数据项集合	
	
	{0x1120, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //相序状态
	{0x1121, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //电表运行状态字
	{0x1122, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //时钟故障
	{0x1123, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //计量芯片故障
	
	//---------应用相关的非保存数据放到0x2000后----------------------
	{0x2001, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ua总谐波含有率,数据格式5
	{0x2002, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ub总谐波含有率,数据格式5
	{0x2003, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Uc总谐波含有率,数据格式5
	{0x2004, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ia总谐波含有率,数据格式5
	{0x2005, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ib总谐波含有率,数据格式5
	{0x2006, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ic总谐波含有率,数据格式5
	{0x200f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //以上数据项集合

	{0x2011, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ua总谐波有效值,数据格式7
	{0x2012, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ub总谐波有效值,数据格式7
	{0x2013, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Uc总谐波有效值,数据格式7
	{0x2014, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ia总谐波有效值,数据格式6
	{0x2015, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ib总谐波有效值,数据格式6
	{0x2016, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //Ic总谐波有效值,数据格式6
	{0x201f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //以上数据项集合
	
	{0x2020, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},    //状态量变位标志，供显示用；
	{0x2021, 1,	DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //连续无通讯超时后进入保电状态

	{0x2030, 21, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //集中器运行状态(显示用:工作正常/自检故障)
	{0x2031, 1,	DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //当前显示事件编号(显示用)
	
	{0x2032, 21, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //集中器运行状态(显示用：正在抄表、正在搜索表号、任务执行完毕)
	{0x2033, 21, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //集中器运行状态(显示用：空闲、通讯中、掉线)
	{0x2034, 21, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //集中器运行状态(显示用)
	
	{0x2040, 1,	DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //CT检测
	{0x2050, 1,	DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //模块连接类型，福建显示专用，
	{0x2051, 1,	DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //pIf->Connect()状态，1表示connectok福建显示专用，
	{0x2052, 1,	DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //ping结果

	{0x2101, 42,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //A相电压谐波含有率数据块
	{0x2102, 42,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //B相电压谐波含有率数据块
	{0x2103, 42,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //C相电压谐波含有率数据块
	{0x2104, 42,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //A相电流谐波含有率数据块
	{0x2105, 42,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //B相电流谐波含有率数据块
	{0x2106, 42,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //C相电流谐波含有率数据块
	{0x2107, 23,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //内部软件版本号
	{0x2108,  2,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE}, 	 //网络制式2: 2G 3: 3G  4: 4G，运营商:1--移动，2--联通，3--电信
	{0x210e, 1, DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //终端停上电状态
	{0x2201, 36,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //A相电压谐波有效值数据块, 2~19次谐波有效值
	{0x2202, 36,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //B相电压谐波有效值数据块，2~19次谐波有效值
	{0x2203, 36,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //C相电压谐波有效值数据块，2~19次谐波有效值
	{0x2204, 36,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //A相电流谐波有效值数据块，2~19次谐波有效值
	{0x2205, 36,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //B相电流谐波有效值数据块，2~19次谐波有效值
	{0x2206, 36,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //C相电流谐波有效值数据块，2~19次谐波有效值

	{0x2300, 1,DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE, FMT_BIN,  	 MTR_EXC_NUM},	 //抄表事件是否初始化

	//以下为当前实时需量入库的数据项ID，不是最大需量
    {0x3010, 	5, 			DI_HIGH_PERM, DI_READ, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当前正向有功需量
	{0x3011,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前A相正向有功需量
	{0x3012,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前B相正向有功需量
	{0x3013,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前C相正向有功需量
	{0x3020,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前反向有功需量
	{0x3021,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前A相反向有功需量
	{0x3022,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前B相反向有功需量
	{0x3023,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前C相反向有功需量
	{0x3030,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前组合无功1需量
	{0x3031,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前A相组合无功1需量
	{0x3032,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前B相组合无功1需量
	{0x3033,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前C相组合无功1需量
	{0x3040,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前组合无功2需量
	{0x3041,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前A相组合无功2需量
	{0x3042,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前B相组合无功2需量
	{0x3043,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前C相组合无功2需量
	{0x3050,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前一象限无功需量
	{0x3051,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前A相一象限无功需量
	{0x3052,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前B相一象限无功需量
	{0x3053,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前C相一象限无功需量
	{0x3060,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前二象限无功需量
	{0x3061,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前A相二象限无功需量
	{0x3062,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前B相二象限无功需量
	{0x3063,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前C相二象限无功需量
	{0x3070,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前三象限无功需量
	{0x3071,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前A相三象限无功需量
	{0x3072,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前B相三象限无功需量
	{0x3073,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前C相三象限无功需量
	{0x3080,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前四象限无功需量
	{0x3081,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前A相四象限无功需量
	{0x3082,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前B相四象限无功需量
	{0x3083,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前C相四象限无功需量

	{0x3117,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前有功需量
	{0x3118,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前无功需量
	{0x3119,	5,			DI_HIGH_PERM, DI_READ, 0,	INFO_NONE,	FMT_UNK,		1,		},//当前视在需量

	//{0x5001, 1, DI_HIGH_PERM, DI_READ|DI_WRITE,0, INFO_NONE},	 //协议切换
	{0x5039, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //BYTE	m_PulseRatio;	// 脉冲放大倍数 liuzhixing 20170225

};

//终端参数描述表：标识--长度--权限--读写--偏移
TItemDesc  g_Bank3Desc[] =   //标准版
{ 
	{0x3000, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //唤醒控制 HEX 0 不要唤醒 大于0表示唤醒字符数
	{0x3001, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3002, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3003, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3004, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3005, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3006, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3007, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3008, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x300f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
	
	{0x3010, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //电能小数位数 Hex 默认 2
	{0x3011, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3012, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3013, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3014, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3015, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3016, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3017, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3018, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x301f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
	
	{0x3020, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //需量小数位数 Hex 默认 4
	{0x3021, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3022, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3023, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3024, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3025, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3026, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3027, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3028, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x302f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 

	{0x3030, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //电压小数位数 Hex 默认 0
	{0x3031, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3032, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3033, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3034, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3035, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3036, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3037, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3038, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x303f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 

	{0x3040, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //电流小数位数 Hex 默认 2
	{0x3041, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3042, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3043, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3044, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3045, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3046, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3047, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3048, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x304f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 

	{0x3050, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //有功功率小数位数 默认4
	{0x3051, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3052, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3053, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3054, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3055, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3056, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3057, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3058, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x305f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
	
	{0x3060, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //无功功率小数位数 默认2
	{0x3061, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3062, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3063, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3064, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3065, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3066, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3067, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3068, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x306f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 

	{0x3070, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //功率因数小数位数 默认 3
	{0x3071, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3072, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3073, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3074, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3075, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3076, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3077, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x3078, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	{0x307f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 

	{0x3080, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //数据库瞬时量更新间隔
	{0x3081, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //数据库非瞬时量更新间隔
	{0x3082, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //电表飞走数据项是否需要对应量 0 不需要，1需要,2 部分需要
	{0x3083, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//电表子协议区分(目前只针对红相表) 1代表Mk3,非1代表Mk6
	{0x3084, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//符合过载恢复比例
	{0x308f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
	
	{0x3090,24, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //新的校正参数
	{0x309f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   
	
	{0x30a0, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //CT一次短路阈值
	{0x30a1, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //CT二次短路阈值
	{0x30a2, 4, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //CT二次开路阈值
	{0x30af, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   

	{0x30b0, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //视度下降终端走△
	{0x30b1, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //示度下降电表走△
	{0x30b2, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //压缩投退,0退出,1投入
	{0x30b3, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //协议类型,0 TCP,1 UDP
	{0x30b4, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //无功过补欠补投退, 0 反极性逆相序发生时无功过补欠补不判断,1 判断
	{0x30b5, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //电池关断电压值
	{0x30bf, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},

	{0x30c0, 72, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//7022校准参数	
	
	{0x30d0, 1,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//遥信脉冲是否共用 0：不公用 1：共用
	
	{0x30d1, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//第1路脉冲参数配置
	{0x30d2, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//第2路脉冲参数配置
	{0x30d3, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//第3路脉冲参数配置
	{0x30d4, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//第4路脉冲参数配置
	{0x30d5, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//第5路脉冲参数配置
	{0x30d6, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//第6路脉冲参数配置
	{0x30d7, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//第7路脉冲参数配置
	{0x30d8, 12,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_YX_PARA},	//第8路脉冲参数配置
	{0x30d9, 10,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//轮显的使能，每bit对应一屏显示
	{0x30e0, 4,   DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//第1路直流模拟量校准参数
	{0x30e1, 4,   DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//第1路直流模拟量校准参数
	{0x30e2, 4,   DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//第2路直流模拟量校准参数
	{0x30e3, 4,   DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//第2路直流模拟量校准参数

	{0x3100, 4,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//额定电压，额定电流，接线方式
	{0x3101, 7,  DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//额定电压NNNNN.N，额定电流NNN.NNN，接线方式NN	

	
	//剩余空间
	{0x3f01, 165, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},   //备用空间
	{0x3f0f,   0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},
	
	{0x5031, 8, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Iregion[2];	// 相位校准分区点，7022E只有2个相位校准分区点
	{0x5032, 12, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Pgain[3];			// 功率校准值A,B,C
	{0x5033, 12, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Phsreg[3];			// 三相校准点的相位补偿
	{0x5034, 12, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Poffset[3]; 		// 三相有功功率Offset校正值
	{0x5035, 6, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //WORD Ioffset[3];		// 电流有效值Offset校正值
	{0x5036, 18, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},//signed short	AngleOffset[3][3];//3个分区ABC三相相位误差补偿值，单位：0.01，带符号
	{0x5037, 6, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //signed short EngErrAdj[3];			// 人为误差调整值,单位0.0001%.
	{0x5038, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //BYTE	AdjustFlag; 		// 校准标志，校表后置位
	{0x503f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
  
};

BYTE g_bBank3Default[] = 
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //唤醒字符数
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, //电能小数位
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, //需量小数位
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //电压小数位
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, //电流小数位
	0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, //有功功率小数位
	0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, //无功功率小数位
	0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, //功率因数小数位
	0x09, 0x00, //3080
	0x3c, 0x00, //3081
	0x01, //3082 
	0x00, //3083
	0x95, 0x00,	//3084
	
	//0x3090 24 新的校正参数
	//0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	0x00, 0x00, 0x00, 0x00, 
	//0x00, 0x00, 0x00, 0x00, 
	0x8E, 0x32, 0x32, 0x00, 0x7D, 0x32, 0x40, 0x00, 0x7D, 0x32, 
	0x41, 0x00, 0x6B, 0x30, 0x00, 0x00, 0x62, 0x30, 0x00, 0x00,
	0x58, 0x30, 0x00, 0x00,	
	
	0xA0, 0x86, 0x01, 0x00, //30a0 CT一次短路阈值10w
	0xc0, 0x27, 0x09, 0x00,	//30a1 CT二次短路阈值60w
	0x58, 0x02, 0x00, 0x00, //30a2 CT二次开路阈值600
	
	0x05, 0x00, //30b0
	0x02, 0x00, //30b1
	
	0x00, //0x30b2 1 压缩投退,0退出,1投入
	0x01, //0x30b3 1 协议类型,0 TCP,1 UDP
	0x00, //0x30b4 1 无功过补欠补投退, 0 反极性逆相序发生时无功过补欠补不判断,1 判断
	
	0x40, 0x03, //0x30b5 2 电池关断电压值

	0xc4, 0x21, 0xac, 0x00, 0xfc, 0xe5, 0xab, 0x00, 0xc8, 0x83, 0xab, 0x00, 0xe3, 0x7f, 0x00, 0x00, 
	0xf4, 0x87, 0x00, 0x00, 0x96, 0xb0, 0x00, 0x00, 0xe1, 0x2d, 0x00, 0x00, 0x6e, 0x42, 0x00, 0x00, 
	0x0b, 0x01, 0x00, 0x00, 0xd0, 0xa2, 0x00, 0x00, 0xa4, 0x34, 0x00, 0x00, 0x7d, 0x73, 0x00, 0x00, 
	0x32, 0x50, 0xea, 0x00, 0xb0, 0xe0, 0xe9, 0x00, 0x52, 0x90, 0xe8, 0x00, 0x86, 0xcd, 0x9c, 0x00, 
	0x64, 0xc4, 0x9c, 0x00, 0x8e, 0xe0, 0x9c, 0x00, //0x30c0 ATT7022的校正参数
	
	0x00,	//0x30D0
	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 	//第1路脉冲参数配置0x30d1
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, //第2路脉冲参数配置0x30d2
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//第3路脉冲参数配置0x30d3
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //第4路脉冲参数配置0x30d4
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, //第5路脉冲参数配置0x30d5
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //第6路脉冲参数配置0x30d6
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //第7路脉冲参数配置0x30d7
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //第8路脉冲参数配置0x30d8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,//轮显的使能，每bit对应一屏显示0x30d9
	0x00, 0x00, 0x05, 0x00, //第1路直流模拟量校准参数
	0x00, 0x00, 0x15, 0x00, //第1路直流模拟量校准参数
	0x00, 0x00, 0x05, 0x00, //第2路直流模拟量校准参数
	0x00, 0x00, 0x15, 0x00, //第2路直流模拟量校准参数
	0x00,0x00,0x00,0x00, //额定电压，额定电流，接线方式0x3100
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //额定电压，额定电流，接线方式0x3101
	0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	
	
		//new 75
	//DWORD	Iregion[2];			// 相位校准分区点，7022E只有2个相位校准分区点
	0xC8, 0x00, 0x00, 0x00,
	0x0A, 0x00, 0x00, 0x00,
	//DWORD Pgain[3];			// 功率校准值A,B,C
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	//DWORD Phsreg[3];			// 三相校准点的相位补偿
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	//DWORD Poffset[3]; 		// 三相有功功率Offset校正值
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	//WORD	Ioffset[3];			// 电流有效值Offset校正值
	0x08, 0x00,
	0x08, 0x00,
	0x08, 0x00,
	//signed short	AngleOffset[3][3];	// 3个分区ABC三相相位误差补偿值，单位：0.01，带符号
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//signed short EngErrAdj[3];			// 人为误差调整值,单位0.0001%.
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00,
	//BYTE	AdjustFlag; 		// 校准标志，校表后置位
	0x00, 
	
};


//645参数
TItemDesc g_Bank4Desc[] =   //标准版
{//----标识-----长度------------权限-----------读写--------偏移----写操作-----------格式----------Pn个数------    
	{0x0001, 	BN_VER_LEN,	DI_HIGH_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,				},//Ver
	{0x0002, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //645协议号选择
        
    {0x0600, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //485端口的波特率//缺省值4即1200bps,端口通信波特率/300
	{0x0601, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BCD, 		1}, //485端口的校验位//缺省无校验0, 0-无,1-偶,2-奇
	{0x0602, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BCD, 		1}, //485端口的数据位//缺省数据位8
	{0x0603, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BCD, 		1}, //485端口的停止位//缺省值0,0-1位停止位,1-1.5停止位,2-2位停止位
	{0x060f, 	0,			DI_HIGH_PERM, 	DI_WRITE, 		  0,	INFO_NONE, 		FMT_UNK, 		GBC4_MTRPORTNUM},	
   	
	{0x0610, 	6, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BCD, 		1}, //通信地址
	{0x0611, 	32, 		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //资产管理编码
	{0x0612, 	6, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //额定电压(ASCII码)
	{0x0613, 	6, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //额定电流/基本电流(ASCII码)
	{0x0614, 	6,	 		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //最大电流(ASCII码)
	{0x0615, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //有功准确度等级(ASCII码)
	{0x0616, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //无功准确度等级(ASCII码)
	{0x0617, 	10,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //电表型号(ASCII码)
	{0x0618, 	10,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //生产日期(ASCII码)
	{0x0619, 	16,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //协议版本号(ASCII码)	
	{0x061a, 	30,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //645_97 10级密码	
	
	{0x0620, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_BCD, 		1}, //有功组合方式特征字
	{0x0621, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_BCD, 		1}, //无功组合方式1特征字
	{0x0622, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_BCD, 		1}, //无功组合方式2特征字	

	{0x0623, 	1,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //负荷记录模式字
	{0x0624, 	1,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //冻结数据模式字

	{0x0630, 	4,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1},	//负荷记录起始时间
	{0x0631, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //第1类负荷记录间隔时间
	{0x0632, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //第2类负荷记录间隔时间
	{0x0633, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //第3类负荷记录间隔时间
	{0x0634, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //第4类负荷记录间隔时间
	{0x0635, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //第5类负荷记录间隔时间
	{0x0636, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //第6类负荷记录间隔时间
	{0x063f, 	0,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, 

	{0x0640, 	2,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //校表脉冲宽度
	{0x0641, 	5,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //两套时区表切换时间
	{0x0642, 	5,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //两套日时段表切换时间
	{0x0643, 	32,			DI_LOW_PERM, 	DI_READ,		  0,	INFO_NONE, 		FMT_UNK, 		1}, //厂家软件版本号(ASCII码)
	{0x0644, 	32,			DI_LOW_PERM, 	DI_READ,		  0,	INFO_NONE, 		FMT_UNK, 		1}, //厂家硬件版本号(ASCII码)
	{0x0645, 	32,			DI_LOW_PERM, 	DI_READ,		  0,	INFO_NONE, 		FMT_UNK, 		1}, //厂家编号(ASCII码)

	{0x0650, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_BCD, 		1}, //每月第一结算日
	{0x0651, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_BCD, 		1}, //每月第二结算日
	{0x0652, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_BCD, 		1}, //每月第三结算日
	{0xc022,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,	1},	//周休日状态字
	
    //电表脉冲常数
    {0xc030,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //电表脉冲常数(有功) 
    {0xc031,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //电表脉冲常数(无功)
    
    //电表地址
	{0xc032,	6,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //表号(	645通讯地址 )
    {0xc033,	6,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //用户号
	{0xc034,	6,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //设备号
	{0xc03f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, 

    {0xc111,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,	FMT_BIN,	1}, //最大需量周期
    {0xc112,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,	FMT_BIN,	1}, //滑差时间
    {0xc113, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	0,	INFO_NONE, 		FMT_BCD,	1}, //循显时间
	{0xc114, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	0,	INFO_NONE, 		FMT_BCD,	1}, //停显时间
	{0xc115,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //显示电能小数位
    {0xc116,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_BCD,	1}, //显示功率(最大需量)小数位数 
    {0xc117,	2,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,	FMT_BCD,	1}, //自动抄表日
    {0xc118, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	0,	INFO_NONE, 		FMT_BCD,	1}, //负荷代表日
	{0xc119, 	4, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	0,	INFO_NONE, 		FMT_BCD,	1}, //有功电能表起始读数
	{0xc11a, 	4, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	0,	INFO_NONE, 		FMT_BCD,	1}, //无功电能表起始读数
	{0xc11f, 	0, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	0,	INFO_NONE, 		FMT_BCD,	1},

#ifdef VER_LIAONING
	//参数设置扩展
    {0xc148,	1, 			DI_LOW_PERM,DI_READ|DI_WRITE,	 0,	INFO_AC_PARA, 		FMT_BCD,	1}, //有功电能计量方式选择
    {0xc149,	1, 			DI_LOW_PERM,DI_READ|DI_WRITE,	 0,	INFO_AC_PARA, 		FMT_BCD,	1}, //无功电能计量方式选择
#endif    

    {0xc211,	2, 			DI_LOW_PERM,DI_READ|DI_WRITE,	 0,	INFO_NONE, 		FMT_BCD, 	1}, //输出脉冲宽度
    {0xc212,	4, 			DI_LOW_PERM,DI_WRITE,	 		 0,	INFO_NONE, 		FMT_BIN, 	1}, //密码权限及密码
    
    {0xc310,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //年时区数P
    {0xc311,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //日时段表数q
    {0xc312,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //日时段(每日切换数) m小于等于10
	{0xc313,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //费率数 k小于等于14
	{0xc314,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //公共假日数n
	{0xc31f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},

	//全年最多支持14个时区数
    {0xc321,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//1时区起始日期及日时段表号
    {0xc322,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//2时区起始日期及日时段表号 
	{0xc323,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//3时区起始日期及日时段表号 
	{0xc324,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//4时区起始日期及日时段表号 
	{0xc325,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//5时区起始日期及日时段表号 
    {0xc326,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//6时区起始日期及日时段表号 
	{0xc327,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//7时区起始日期及日时段表号 
	{0xc328,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//8时区起始日期及日时段表号 
	{0xc329,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//9时区起始日期及日时段表号 
    {0xc32a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//10时区起始日期及日时段表号 
	{0xc32b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//11时区起始日期及日时段表号 
	{0xc32c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//12时区起始日期及日时段表号 
	{0xc32d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//13时区起始日期及日时段表号 
	{0xc32e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1},	//14时区起始日期及日时段表号 
	{0xc32f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	
	//日时段表1	(最大8套日时段表)	//脉冲测量点费率同测量点0
	{0xc331,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc332,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc333,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc334,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc335,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc336,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc337,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc338,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc339,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc33a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc33b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc33c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc33d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc33e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc33f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 

	//日时段表2
	{0xc341,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc342,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc343,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc344,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc345,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc346,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc347,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc348,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc349,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc34a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc34b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc34c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc34d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc34e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc34f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
    
	//日时段表3
	{0xc351,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc352,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc353,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc354,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc355,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc356,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc357,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc358,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc359,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc35a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc35b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc35c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc35d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc35e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc35f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	
	//日时段表4
	{0xc361,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc362,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc363,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc364,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc365,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc366,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc367,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc368,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc369,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc36a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc36b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc36c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc36d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc36e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc36f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	
	//日时段表5
	{0xc371,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc372,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc373,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc374,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc375,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc376,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc377,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc378,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc379,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc37a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc37b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc37c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc37d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc37e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc37f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 

	//日时段表6
	{0xc381,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc382,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc383,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc384,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc385,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc386,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc387,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc388,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc389,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc38a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc38b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc38c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc38d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc38e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc38f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
    
	//日时段表7
	{0xc391,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc392,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc393,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc394,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc395,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc396,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc397,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc398,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc399,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc39a,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc39b,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc39c,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc39d,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc39e,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc39f,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	
	//日时段表8
	{0xc3a1,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a2,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a3,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a4,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a5,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a6,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a7,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a8,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3a9,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3aa,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3ab,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3ac,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3ad,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3ae,	3,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 
	{0xc3af,	0,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, 

	//公共假日
    {0xc411,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第一公共假日日期及日时段表号
    {0xc412,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第二公共假日日期及日时段表号
    {0xc413,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第三公共假日日期及日时段表号
    {0xc414,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第四公共假日日期及日时段表号
    {0xc415,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第五公共假日日期及日时段表号
    {0xc416,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第六公共假日日期及日时段表号
    {0xc417,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第七公共假日日期及日时段表号
    {0xc418,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第八公共假日日期及日时段表号
	{0xc419,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第九公共假日日期及日时段表号
    {0xc41a,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十公共假日日期及日时段表号
    {0xc41b,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十一公共假日日期及日时段表号
    {0xc41c,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十二公共假日日期及日时段表号
    {0xc41d,	3, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十三公共假日日期及日时段表号
    {0xc41e,	1, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //周休日采用的日时段表号
    {0xc41f,	0, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1},

	{0xc510,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE, 		FMT_BCD, 		1}, //负荷记录起始时间,月日时分
    {0xc511,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //负荷曲线1类数据记录间隔,分
       
    {0xc512,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //负荷曲线2类数据记录间隔,分
    {0xc513,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //负荷曲线3类数据记录间隔,分
    {0xc514,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //负荷曲线4类数据记录间隔,分
    {0xc515,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //负荷曲线5类数据记录间隔,分
    {0xc516,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //负荷曲线6类数据记录间隔,分
    {0xc517,	2, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //负荷曲线7类数据记录间隔,分
    {0xc51f,	0, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_CUR_PARA,	FMT_BCD, 		1}, //负荷曲线7类数据记录间隔,分
#ifdef VER_LIAONING     
    {0xc610, 	6,			DI_HIGH_PERM, 	DI_WRITE, 		  0,	INFO_NONE, 		FMT_UNK, 		1}, //手动复位密码
    //{0xc611, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BCD,		1},	//三相三线失流判定值
    //{0xc612, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BCD,		1},	//三相四线失流判定值

    {0xc720, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失压启动电压阀值
    {0xc721, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失压恢复电压阀值
    {0xc722, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失压启动电流阀值
    {0xc723, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失流启动平均值阀值
    {0xc724, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失流启动不平衡率阀值
    //{0xc725, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//
#endif

	{0xc810, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失压事件电压触发上限
	{0xc811, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失压事件电压恢复下限
	{0xc812, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失压事件电流触发下限
	{0xc813, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失压事件判定延时时间

	{0xc814, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//欠压事件电压触发上限
	{0xc815, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//欠压事件判定延时时间

	{0xc816, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//过压事件电压触发下限
	{0xc817, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//过压事件判定延时时间

	{0xc818, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//断相事件电压触发上限
	{0xc819, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//断相事件电流触发上限
	{0xc81a, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//断相事件判定延时时间

	{0xc81b, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//电压不平衡率限值
	{0xc81c, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//电压不平衡率判定延时时间

	{0xc81d, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//电流不平衡率限值
	{0xc81e, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//电流不平衡率判定延时时间

	{0xc820, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失流事件电压触发下限
	{0xc821, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失流事件电流触发上限
	{0xc822, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失流事件电流触发下限
	{0xc823, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//失流事件判定延时时间

	{0xc824, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//过流事件电流触发下限
	{0xc825, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//过流事件判定延时时间

	{0xc826, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//断流事件电压触发下限
	{0xc827, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//断流事件电流触发上限
	{0xc828, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//断流事件判定延时时间

	{0xc829, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//潮流反向事件有功功率触发下限
	{0xc82a, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//潮流反向事件判定延时时间

	{0xc82b, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//过载事件有功功率触发下限
	{0xc82c, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//过载事件判定延时时间

	{0xc82d, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//电压考核上限
	{0xc82e, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//电压考核下限

	{0xc830, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//有功需量超限事件需量触发下限
	{0xc831, 	3, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//无功需量超限事件需量触发下限
	{0xc832, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//需量超限事件判定延时时间

	{0xc833, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//总功率因数超下限阀值
	{0xc834, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//总功率因数超下限判定延时时间

	{0xc835, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//电流严重不平衡限值
	{0xc836, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//电流严重不平衡触发延时时间

	{0xc840, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//负荷记录模式字
	{0xc841, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//冻结数据模式字
	{0xc842, 	4, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//负荷记录起始时间
	{0xc843, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//第1类负荷记录间隔时间
	{0xc844, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//第2类负荷记录间隔时间
	{0xc845, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//第3类负荷记录间隔时间
	{0xc846, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//第4类负荷记录间隔时间
	{0xc847, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//第5类负荷记录间隔时间
	{0xc848, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//第6类负荷记录间隔时间

	{0xc849, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//定时冻结数据模式字
	{0xc84a, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//瞬时冻结数据模式字
	{0xc84b, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//约定冻结数据模式字
	{0xc84c, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//整点冻结数据模式字
	{0xc84d, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//日冻结数据模式字

	{0xc850, 	5, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//整点冻结起始时间
	{0xc851, 	1, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//整点冻结时间间隔
	{0xc852, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//日冻结时间

	{0xc853, 	4, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//冻结命令
	{0xc854, 	4, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//瞬时冻结命令

	{0xc855,	2,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //公共假日数n	2	//从0xc314映射来

	{0xc856,	1,			DI_LOW_PERM,DI_READ|DI_WRITE,	0,	INFO_AC_PARA,		FMT_BCD,	1}, //编程开关有效时间,单位分钟	BCD

	//公共假日
	{0xc861,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第一公共假日日期及日时段表号
	{0xc862,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第二公共假日日期及日时段表号
	{0xc863,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第三公共假日日期及日时段表号
	{0xc864,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第四公共假日日期及日时段表号
	{0xc865,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第五公共假日日期及日时段表号
	{0xc866,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第六公共假日日期及日时段表号
	{0xc867,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第七公共假日日期及日时段表号
	{0xc868,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第八公共假日日期及日时段表号
	{0xc869,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第九公共假日日期及日时段表号
	{0xc86a,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十公共假日日期及日时段表号
	{0xc86b,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十一公共假日日期及日时段表号
	{0xc86c,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十二公共假日日期及日时段表号
	{0xc86d,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十三公共假日日期及日时段表号
	{0xc86e,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十四公共假日日期及日时段表号

	{0xc871,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十五公共假日日期及日时段表号
	{0xc872,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十六公共假日日期及日时段表号
	{0xc873,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十七公共假日日期及日时段表号
	{0xc874,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十八公共假日日期及日时段表号
	{0xc875,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第十九公共假日日期及日时段表号
	{0xc876,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第二十公共假日日期及日时段表号
	{0xc877,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第二一公共假日日期及日时段表号
	{0xc878,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第二二公共假日日期及日时段表号
	{0xc879,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第二三公共假日日期及日时段表号
	{0xc87a,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第二四公共假日日期及日时段表号
	{0xc87b,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第二五公共假日日期及日时段表号
	{0xc87c,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第二六公共假日日期及日时段表号
	{0xc87d,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第二七公共假日日期及日时段表号
	{0xc87e,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第二八公共假日日期及日时段表号

	{0xc881,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第二九公共假日日期及日时段表号
	{0xc882,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第三十公共假日日期及日时段表号
	{0xc883,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第三一公共假日日期及日时段表号
	{0xc884,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第三二公共假日日期及日时段表号
	{0xc885,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第三三公共假日日期及日时段表号
	{0xc886,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第三四共假日日期及日时段表号
	{0xc887,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第三五共假日日期及日时段表号
	{0xc888,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第三六共假日日期及日时段表号
	{0xc889,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第三七共假日日期及日时段表号
	{0xc88a,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第三八共假日日期及日时段表号
	{0xc88b,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第三九公共假日日期及日时段表号
	{0xc88c,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第四十公共假日日期及日时段表号
	{0xc88d,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第四一公共假日日期及日时段表号
	{0xc88e,	4, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_AC_PARA, 		FMT_BCD, 		1}, //第四二公共假日日期及日时段表号

	{0xc890,	1, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE, 			FMT_BCD, 		1}, //是否返回实际个数费率参数
	{0xc900,	1, 			DI_LOW_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE, 			FMT_BCD, 		1}, //时段时区切换标志
};	//    

BYTE g_bBank4Default[] = {
    //0x05ee	 20
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

	0x00,	//0x0002 1 00:645/2007协议 01:645/1997协议

   	0x08,	//0x0600 1
	0x01,	//0x0601 1
	0x08,	//0x0602 1
	0x00,	//0x0603 1
	
	0x01,0x00,0x00,0x00,0x00,0x00,	//0x0610 6

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0611 32

	0x00,0x00,0x00,0x00,0x00,0x00,	//0x0612 6
	0x00,0x00,0x00,0x00,0x00,0x00,	//0x0613 6
	0x00,0x00,0x00,0x00,0x00,0x00,	//0x0614 6
	0x00,0x00,0x00,0x00, //0x0615 4
	0x00,0x00,0x00,0x00, //0x0616 4
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0617 10
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0618 10
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0619 16
	
	0x00,0x00,0x00,	//高级密码
	0x00,0x00,0x00,	//编程密码
	0x00,0x00,0x00,	//管理员密码
	0x00,0x00,0x00,
	0x11,0x11,0x11,	//操作员密码
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,	//0x061a 30	
	
	0x05, //0x0620
	0x41, //0x0621	//正向无功 默认I+IV
	0x14, //0x0622	//反向无功 默认II+III

	0x00, //0x0623
	0x00, //0x0624

	0x00,0x00,0x00,0x00, //0x0630 4
	0x00,0x00, //0x0631
	0x00,0x00, //0x0632
	0x00,0x00, //0x0633
	0x00,0x00, //0x0634
	0x00,0x00, //0x0635
	0x00,0x00, //0x0636
	
	0x00,0x00, //0x0640
	0x00,0x00,0x00,0x00,0x00, //0x0641 5
	0x00,0x00,0x00,0x00,0x00, //0x0642 5

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0643 32
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0644 32
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //0x0645 32
	
	0x00,0x01,//0x0650
	0xff,0xff,//0x0651
	0xff,0xff,//0x0652

	/*0x00,0x00, //0x0650
	0x00,0x00, //0x0651
	0x00,0x00, //0x0652
	0x00,0x00, //0x0653
	0x00,0x00, //0x0654
	0x00,0x00, //0x0655
	0x00,0x00, //0x0656*/	

	//0x00, //0x0660
	//0x00, //0x0661
	//0x00, //0x0662
	//0x00, //0x0663
	//0x00, //0x0664

	0xff, //0xc022 1
	
	0x00,0x64,0x00,	//0xc030 3
	0x00,0x64,0x00,	//0xc031 3
	
	0x01,0x00,0x00,0x00,0x00,0x00,	//0xc032 6
	0x01,0x00,0x00,0x00,0x00,0x00,	//0xc033 6
	0x01,0x00,0x00,0x00,0x00,0x00,	//0xc034 6
	
	0x15, //0xc111 需量周期
	0x01, //0xc112 滑差时间 
	0x08, //0xc113 循显时间 
	0x60, //0xc114 停显时间 
	0x02, //0xc115 显示电能小数位
	0x04, //0xc116
	0x00,0x01, //0xc117 自动抄表日
	0x01, //0xc118 负荷代表日
	0x00,0x00,0x00,0x00, //0xc119 有功电能表起始读数
	0x00,0x00,0x00,0x00, //0xc11a 无功电能表起始读数

#ifdef VER_LIAONING
	0x00, //0xc148 有功电能计量方式选择
	0xc3, //0xc149 无功电能计量方式选择
#endif

	0x00,0x00, //0xc211
	0x00,0x00,0x00,0x00, //0xc212
	
	0x01, //0xc310
	0x01, //0xc311
	0x07, //0xc312
	0x04, //0xc313
	0x00, //0xc314

	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc32f
	
	0x04,0x00,0x00,	//00:00~05:00 4 谷
	0x03,0x00,0x05,	//05:00~07:30 3 平
	0x02,0x30,0x07,	//07:30~11:30 2 峰
	0x03,0x30,0x11,	//11:30~17:00 3 平
	0x02,0x00,0x17, //17:00~21:00 2 峰
	0x03,0x00,0x21,	//21:00~22:00 3 平
	0x04,0x00,0x22, //22:00~24:00 4 谷
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc33f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc34f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc35f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc36f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc37f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc38f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc39f
	
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00, //0xc3af

	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00,0x00,0x00,
	0x00, //0xc41f
	
	0x00,0x00,0x01,0x01, //0xc510, 4, 负荷记录起始时间,分时日月
    0x15,0x00,		//0xc511, 2, 负荷曲线1类数据记录间隔   
    0x0,0x00,		//0xc512, 2, 负荷曲线2类数据记录间隔
    0x0,0x00,		//0xc513, 2, 负荷曲线3类数据记录间隔
    0x0,0x00,		//0xc514, 2, 负荷曲线4类数据记录间隔
    0x0,0x00,		//0xc515, 2, 负荷曲线5类数据记录间隔
    0x0,0x00,		//0xc516, 2, 负荷曲线6类数据记录间隔
    0x0,0x00,		//0xc517, 2, 负荷曲线7类数据记录间隔
#ifdef VER_LIAONING	
	0x31,0x31,0x31,0x31,0x00,0x00,	//0xc610, 6字节ASCII码字符, 手动复位密码
	//0x30,			//0xc611, 1, 三相三线失流判定值
	//0x50,			//0xc612, 1, 三相四线失流判定值

    0x40,0x15, 	//0xc720, 2, 失压启动电压阀值 %70Un 	154.0
    0x70,0x18,	//0xc721, 2, 失压恢复电压阀值 %85Un 	187.0
    0x00,0x01,	//0xc722, 2, 失压启动电流阀值 %2Ib  	0.1
    0x05,0x00,	//0xc723, 2, 失流启动平均值阀值(%Ib)	5
    0x50,0x00,	//0xc724, 2, 失流启动不平衡率阀值(%)	50
    //{0xc725, 	2, 			DI_LOW_PERM,DI_READ|DI_WRITE, 	 0, INFO_NONE,		FMT_BCD,	1},	//

#endif	
	0x16, 0x17, //0xc810 2 失压事件电压触发上限 NNN.N
	0x60, 0x17, //0xc811 2 失压事件电压恢复下限 NNN.N
	0x00, 0x05, 0x00, //0xc812 3 失压事件电流触发下限 NN.NNNN
	0x60, //0xc813 1 失压事件判定延时时间 秒

	0x16, 0x17, //0xc814 2 欠压事件电压触发上限 NNN.N
	0x60, //0xc815 1 欠压事件判定延时时间 秒

	0x00, 0x00, //0xc816 2 过压事件电压触发下限
	0x00, //0xc817 1 过压事件判定延时时间

	0x16, 0x17, //0xc818 2 断相事件电压触发上限
	0x00, 0x05, 0x00, //0xc819 3 断相事件电流触发上限
	0x60, //0xc81a 1 断相事件判定延时时间

	0x00, 0x00, //0xc81b 2 电压不平衡率限值
	0x60, //0xc81c 1 电压不平衡率判定延时时间

	0x00, 0x00, //0xc81d 2 电流不平衡率限值
	0x60, //0xc81e 1 电流不平衡率判定延时时间

	0x16, 0x17, //0xc820 2 失流事件电压触发下限
	0x00, 0x00, 0x00, //0xc821 3 失流事件电流触发上限
	0x00, 0x00, 0x00, //0xc822 3 失流事件电流触发下限
	0x60, //0xc823 1 失流事件判定延时时间

	0x00, 0x00, //0xc824 2 过流事件电流触发下限
	0x60, //0xc825 1 过流事件判定延时时间

	0x00, 0x00, //0xc826 2 断流事件电压触发下限
	0x00, 0x00, 0x00, //0xc827 3 断流事件电流触发上限
	0x60, //0xc828 1 断流事件判定延时时间

	0x00, 0x00, 0x00, //0xc829 3 潮流反向事件有功功率触发下限
	0x60, //0xc82a 1 潮流反向事件判定延时时间

	0x00, 0x32, 0x01, //0xc82b 3 过载事件有功功率触发下限 NN.NNNN
	0x60, //0xc82c 1 过载事件判定延时时间

	0x00, 0x00, //0xc82d 2 电压考核上限
	0x00, 0x00, //0xc82e 2 电压考核下限

	0x00, 0x00, 0x00, //0xc830 3 有功需量超限事件需量触发下限
	0x00, 0x00, 0x00, //0xc831 3 无功需量超限事件需量触发下限
	0x60, //0xc832 1 需量超限事件判定延时时间

	0x00, 0x00, //0xc833 2 总功率因数超下限阀值
	0x60, //0xc834 1 总功率因数超下限判定延时时间

	0x00, 0x00, //0xc835 2 电流严重不平衡限值
	0x60, //0xc836 1 电流严重不平衡触发延时时间

	0xff, //0xc840 1 负荷记录模式字
	0xff, //0xc841 1 冻结数据模式字
	0x00, 0x00, 0x01, 0x01, //0xc842 4 负荷记录起始时间
	0x15, 0x00, //0xc843 2 第1类负荷记录间隔时间
	0x15, 0x00, //0xc844 2 第2类负荷记录间隔时间
	0x15, 0x00, //0xc845 2 第3类负荷记录间隔时间
	0x15, 0x00, //0xc846 2 第4类负荷记录间隔时间
	0x15, 0x00, //0xc847 2 第5类负荷记录间隔时间
	0x15, 0x00, //0xc848 2 第6类负荷记录间隔时间

	0xff, //0xc849 1 定时冻结数据模式字
	0xff, //0xc84a 1 瞬时冻结数据模式字
	0xff, //0xc84b 1 约定冻结数据模式字
	0x03, //0xc84c 1 整点冻结数据模式字
	0xff, //0xc84d 1 日冻结数据模式字

	0x00, 0x00, 0x01, 0x01, 0x00,//0xc850 5 整点冻结起始时间
	0x60, //0xc851 1 整点冻结时间间隔
	0x00, 0x00, //0xc852 2 日冻结时间

	0x00, 0x00, 0x00, 0x00, //0xc853 4 冻结命令
	0x00, 0x00, 0x00, 0x00, //0xc854 4 瞬时冻结命令

	0x00, 0x00,	//0xc855 2 公共假日数n

	0x60, //0xc856 1 编程开关有效时间

	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00, //0xc861-0xc86e  第1至十四公共假日日期及日时段表号

	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00, //0xc871-0xc87e  第15至28公共假日日期及日时段表号

	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00, 
	0x00,0x00,0x00,0x00, //0xc881-0xc88e  第29至42公共假日日期及日时段表号

	0x01,	//0xc890 是否返回实际个数费率参数
	0x00,	//0xc900 时区时段切换标志
};

//07内表扩展数据
TItemDesc g_Bank5Desc[] =   //标准版
{//----标识-----长度------------权限-----------读写--------偏移----写操作-----------格式----------Pn个数------    
	{0x0001, 	BN_VER_LEN,	DI_HIGH_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1},//Ver
        
    {0x5001, 	195,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //A相失压临时记录
    {0x5002, 	195,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //B相失压临时记录
    {0x5003, 	195,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //C相失压临时记录

	{0x5004, 	15,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //全失压临时记录

	{0x5005, 	195,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //A相断相临时记录
	{0x5006, 	195,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //B相断相临时记录
	{0x5007, 	195,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //C相断相临时记录

	{0x5008, 	179,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //A相失流临时记录
	{0x5009, 	179,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //B相失流临时记录
	{0x500a, 	179,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //C相失流临时记录

	{0x500b, 	140,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //电压逆相序临时记录
	{0x500c, 	140,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //电流逆相序临时记录

	{0x500d, 	140,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //A相过载临时记录
	{0x500e, 	140,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //B相过载临时记录
	{0x500f, 	140,		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //C相过载临时记录

	{0x5010, 	8,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //最近一次编程时间 TTime格式	
};

TItemDesc g_Bank10Desc[] =
{//----标识-----长度------------权限-----------读写--------偏移----写操作--------------格式----------Pn个数------格式描述串------测量点映射号----
	{0xa000, 	BN_VER_LEN,	DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//Ver
	
	//电能计量
	{0xa010, 	3,			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT10,			1,	},//脉冲常数,BCD码
	{0xa011, 	1,			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	},//有功单双向计量模式:0,双向 1单向
	{0xa012, 	1,			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	},//终端无功电量累加标志,
																										  //低4位用于正向无功:D0,D1,D2,D3分别表示一,二,三,四象限无功电量,为1:相加,为0:不加;
																										  //高4位用于反向无功:D4,D5,D6,D7分别表示一,二,三,四象限无功电量,为1:相加,为0:不加;
	{0xa013, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	}, //编程开关有效持续时间 NN(1~99分)
	{0xa014, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	}, //防潜动脉冲最大间隔时间 NNNN(0~9999秒)
	{0xa015, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	}, //角度方向,0表示角度按照逆时针方向表示,Ua,Ub,Uc分别为0,240,120
																										   //	1表示角度按照顺时针方向表示,Ua,Ub,Uc分别为0,120,240	
	//冻结
	{0xa020, 	1,			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	},//日月冻结模式 BIN 0表示0点抄表再冻结,1表示提前抄表0点冻结
	
	//告警
	{0xa030, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,  	INFO_NONE, 	    FMT_BIN, 	GB_MAXMETER,	NULL,		MTRPNMAP}, //电表互感器数量
	{0xa031, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},   //电流/压不平衡发生时间限值 0xa031 HEX
	{0xa032, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},   //电流/压不平衡恢复时间限值 0xa032 HEX 谐波越限，电流，电压，功率越限同电流/压不平衡不平衡
	{0xa033, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},   //电压异常发生时间限值 0xa033 HEX
	{0xa034, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},   //电压异常恢复时间限值 0xa034 HEX
	{0xa035, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},   //相序异常延迟（发生/恢复）时间限值 0xa035 HEX
	{0xa036, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},	  //判断电压回路异常时电压断相(缺相）门限电压百分比 HEX
	{0xa037,    1,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},	  //判断断相缺相时的断相电压门限(V),HEX,-----本数据项去除不用
	{0xa038,    1,			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_BIN, 		1},	  //启动电流为最大电流的千分比；
	
	//终端扩展参数
	{0xa040, 	2, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_COMM_RLD, 		FMT_UNK, 		1}, //行政区划码
	{0xa041, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_COMM_RLD, 		FMT_UNK, 		1}, //终端地址
	{0xa042, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //厂商代码
	{0xa043, 	8, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //设备编号
	{0xa044, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //交采默认测量点号,0表示不默认
	{0xa045, 	6, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //系统设置密码
	{0xa046, 	11, 		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //SIM卡号码
	{0xa04f, 	0, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //终端扩展参数块数据
	
	//电容器配置参数
	{0xa05f, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //第一组电容器配置参数
	{0xa06f, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //第二组电容器配置参数
	{0xa07f, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //第三组电容器配置参数
	{0xa08f, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //第四组电容器配置参数
	{0xa09f, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //第五组电容器配置参数
	{0xa0af, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //第六组电容器配置参数
	{0xa0bf, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //第七组电容器配置参数
	{0xa0cf, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //第八组电容器配置参数
	{0xa0df, 	7, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 	GB_MAXMETER}, //第九组电容器配置参数
	
	//本地维护端口配置参数
	{0xa100, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //波特率
	{0xa101, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //校验位
	{0xa102, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //数据位
	{0xa103, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //停止位
	{0xa104, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //端口功能
	{0xa10f, 	0, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, 
	
	//级联端口配置参数
	{0xa110, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //波特率
	{0xa111, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //校验位
	{0xa112, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //数据位
	{0xa113, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //停止位
	{0xa114, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //端口功能
	{0xa11f, 	0, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, 
	
	//控制参数
	{0xa120, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //上电保电时间,HEX,单位分钟
	{0xa121, 	3, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //月电控告警时间,D0~D23分别对应0点~23点

	{0xa122, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_UNK, 		1}, //有功电能计量方式选择
	{0xa123, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_UNK, 		1}, //正向无功电量累加标志
	{0xa124, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_AC_PARA, 		FMT_UNK, 		1}, //反向无功电量累加标志
	
	{0xa130, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //直流模拟量检测类型,HEX, 01：PT100电阻， 其它值：直流电压量
	
	{0xa131, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 	FMT_UNK, 		1}, //右起往左第1个485口功能定义
	{0xa132, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 	FMT_UNK, 		1}, //右起往左第2个485口功能定义

	{0xa133, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //级联口校验方式 0:无校验，1：奇校验，2：偶校验
	//国标版本
	{0xa140, 	1,			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0, 	INFO_NONE,	   	FMT_UNK,		1,	},//国标版本选择 BIN 0表示国标2005版,1表示国标2004版
	//主动上报
	{0xa141, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	GB_MAXCOMMTHREAD,	},//二类定时上报模式：BIN 1表示华北模式；0表示普通模式    
	//GPRS通信
	{0xa142, 	2, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMM_RLD,	FMT_UNK,		1,	},//GPRS无通讯复位终端时间,单位分钟,HEX
	//总加组相关参数变更
	{0xa143, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//总加组相关参数变更，是否在原有日月累计电量的基础上累计：BIN 1表示不累计；0表示累计    
	//终端地址显示
	{0xa144, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//终端地址显示格式	BIN	1	缺省值0,0表示十进制显示，1表示十六进制显示
	//终端通信方式
	{0xa145, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMM_RLD,	FMT_UNK,		1,	},//终端网卡1通信方式BIN	1	缺省值0,0表示TCP方式，1表示UDP方式

	{0xa150, 	6, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 	0, 	   INFO_COMM_RLD,	FMT_BIN,	 1},   //本机网卡1物理地址
	{0xa151, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 	0, 	   INFO_COMM_RLD,	FMT_BIN,	 1},   //本机网卡1IP
	{0xa152, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 	0, 	   INFO_COMM_RLD,	FMT_BIN,	 1},   //子网掩码
	{0xa153, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 	0, 	   INFO_COMM_RLD,	FMT_BIN,	 1},   //网关
	{0xa15f, 	0, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 	0, 	   INFO_COMM_RLD,	FMT_BIN,	 1},

	{0xa160, 	4, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 		FMT_UNK, 		1}, //红外口波特率
	{0xa161, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 		FMT_UNK, 		1}, //红外口校验位	
	{0xa162, 	4, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 		FMT_UNK, 		1}, //232口波特率
	{0xa164, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 		FMT_UNK, 		1}, //232口校验位

	{0xa165, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 			FMT_UNK, 		1}, //主站通信密码校验的长度选择，0-2字节，1-16字节
	{0xa166, 	1, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 		FMT_UNK, 		1}, //485抄表口排列顺序,0:右->左分别是口1,口2...; 1:左->右分别是口1,口2...
  
  //显示专用
	{0xa170, 	21, 		DI_LOW_PERM,   	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //液晶屏欢迎界面第1行
	{0xa171, 	21, 		DI_LOW_PERM,   	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //液晶屏欢迎界面第2行
	{0xa172, 	21, 		DI_LOW_PERM,   	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //液晶屏欢迎界面第3行
	{0xa173, 	21, 		DI_LOW_PERM,   	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //液晶屏欢迎界面第4行
	{0xa174, 	21, 		DI_LOW_PERM,   	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //液晶屏欢迎界面第5行
	{0xa175, 	21, 		DI_LOW_PERM,   	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //液晶屏欢迎界面第6行
	{0xa176, 	21, 		DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //液晶屏欢迎界面第7行
	{0xa177, 	21, 		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //液晶屏欢迎界面第8行
	{0xa178, 	21, 		DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //液晶屏欢迎界面第9行
	{0xa179, 	2, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //记录轮显的测量点号

	{0xa180, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 	FMT_UNK, 		1}, //右起往左第3个485口功能定义

	{0xa190, 	1 , 		DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMTASK_PARA,		FMT_BIN,		GB_MAXMETER,},	//电表曲线冻结模式字,对97版不起作用  0:不读取电表冻结,终端自行冻结、1:读取电表自身冻结
	{0xa191, 	1 , 		DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMTASK_PARA,		FMT_BIN,		GB_MAXMETER,},	//电表日冻结模式字,对无日冻结功能的表不起作用 0:不读取电表冻结,终端自行冻结、1:读取电表自身日冻结

	{0xa1a0, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //设置主动上报为多帧时帧序号每帧均按单帧传输，还是按有续帧规则传输 0:每帧都是单帧 1:按有续帧规则
	{0xa1a1, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //设置是否根据电表返回电能数据的实际长度修改测量点的费率数，0:不修改费率数按照F10的配置 1:按照电表实际返回电能块长度修改

	{0xa1a5, 	2, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //用来配置模块是否需要进入Dormant。设置为0的时候则不使用Dormant，非0的时候则在无通信后wDormantInterv时间内进入Dormant。	
	{0xa1a6, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //遥信属性取反标志位 0-05 1-698
	{0xa1a7, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //越限阀值不带符号位
	{0xa1a8, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //终端工作模式

	{0xa1a9, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //计量方式选择，0x00表示以总功率计量电量，0x01表示以分相功率计量电量	

	{0xa1b0,	1,			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK,		GB_MAXMETER,}, //无功补偿模块  无功电能量计量模式：1为一次侧、2为二次侧、其他值无效,默认为二次侧

	{0xa1b1, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_COMTASK_PARA, 		FMT_UNK, 		1}, //日月冻结滞后0点开始执行的分钟数，缺省为0表示无需滞后执行

	{0xa1b2, 	1 , 		DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMTASK_PARA,	FMT_BIN,	GB_MAXMETER,},	//电表抄表日冻结模式字, 0:读实时数据、1:读取电表结算日

    {0xa1b3, 	12, 		DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMM_RLD,	FMT_BIN,		1},	//设备号 ascii字符  12 BYTE
	{0xa1b4, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1},	//重抄次数，0~7,0表示不进行垃圾队列的重抄,7表示一直进行垃圾队列的重抄,其它表示次数
	{0xa1b5, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1},	//第4个485口功能定义 0：维护口, 0xFF为调试输出
	{0xa1b6, 	1, 			DI_LOW_PERM,  	DI_READ|DI_WRITE, 0,	INFO_APP_RST, 	FMT_UNK, 		1}, //以太网口接入方式选择0：自动获取IP，1：pppoe拨号
	{0xa1b7, 	2, 			DI_HIGH_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_NONE, 		FMT_BIN, 		1},	//液晶背光点亮时间(单位：秒)
	{0xa1b8, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1},	//默认GPRS模块类型
	
    {0xa1b9, 	21, 		DI_HIGH_PERM,  	DI_READ|DI_WRITE, 0,	INFO_NONE, 		FMT_UNK, 		1}, //上次读取的配置文件名
	
	{0xa1ba, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMTASK_PARA,		FMT_BIN,		GB_MAXMETER,},	//电表月冻结模式字, 0:不读取电表冻结,终端自行冻结、1:读取电表自身日冻结; 2.读取电表结算日
	{0xa1bb, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1,},//设置读二类及主动上报二类数据中，如果没有冻结的历史数据，是否填无效上送: 0:不填无效（不上送）; 1:填无效

	{0xa1bc, 	4, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1,},//电表数据项间的抄读延时ms
	{0xa1bd, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1,},//第1个脉冲端口所对应的开关量
	{0xa1be, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1,},//调节LCD对比度

	{0xa1c0, 	2, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,		1,}, //TCP探测间隔，BIN,单位秒
	{0xa1c1, 	6, 			DI_LOW_PERM, 	DI_READ|DI_WRITE, 0, 	INFO_COMM_RLD,	FMT_BIN,		1},   //本机网卡2物理地址

	//终端通信方式
	{0xa1c2, 	1, 			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_COMM_RLD,	FMT_UNK,		1,	},//终端网卡2通信方式BIN	1	缺省值0,0表示TCP方式，1表示UDP方式
	{0xa323,	1,	  		DI_LOW_PERM,	DI_READ|DI_WRITE,0, 	INFO_NONE,		FMT_UNK,		1,	}, //是否保存调试信息到文件

};

BYTE g_bBank10Default[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)	
	
	//电能计量
	0x00, 0x64, 0x00, //0xa010 3 脉冲常数,BCD码
	0x00, //0xa011 1 有功单双向计量模式:0,双向 1单向
	0xc3, //0xa012 终端无功电量累加标志,
	
	0x30,	//0xa013 1 编程开关有效持续时间 NN(1~99分)
	0x60, 0x00,	//0xa014 2 防潜动脉冲最大间隔时间 NNNN(0~9999秒)
	0x01,	//0xa015 1	角度方向,0表示角度按照逆时针方向表示,Ua,Ub,Uc分别为0,240,120
			//					 1表示角度按照顺时针方向表示,Ua,Ub,Uc分别为0,120,240

	//冻结
	0x00, //0xa020 1 日月冻结模式 BIN 0表示0点抄表再冻结,1表示提前抄表0点冻结
	
	//告警
	0x03, //0xa030 1 电表互感器数量
	0x01, //0xa031 1 电流/压不平衡发生时间限值
	0x01, //0xa032 1 电流/压不平衡恢复时间限值
	0x01, //0xa033 1 电压异常发生时间限值 0xa033；
	0x01, //0xa034 1 电压异常恢复时间限值 0xa034；
	0x01, //0xa035 1 相序异常延迟（发生/恢复）时间限值 0xa035;
	0x4e,	//0xa036 1 判断电压回路异常时电压断相(缺相）门限电压百分比 HEX
	0x00,   //0xa037  1 判断断相缺相时的断相电压门限(V)， 默认为无效值；
	0x50,   //0xa038  1 启动电流为最大电流的千分比；默认为无效值；

	0x11, 0x22, //行政区划码
	0x33, 0x44, //终端地址
	'C', 'L', 'O', 'U',//厂商代码	
    #ifdef EN_AC
    'C', 'L', '8', '1', '8', 'C', 'G', 'B', //设备编号	
    0x01, 0x00, //0xa044 2 交采默认测量点号,0表示不默认
    #else
    'C', 'L', '8', '1', '8', 'C', 'J', 'C', //设备编号	
	0x00, 0x00, //0xa044 2 交采默认测量点号,0表示不默认
    #endif
	'0', '0', '0', '0', '0', '0', //系统设置密码
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //终端扩展参数块数据

	//电容器配置参数
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa05f 7 第一组
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa06f 7 第二组
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa07f 7 第三组
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa08f 7 第四组
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa09f 7 第五组
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa0af 7 第六组
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa0bf 7 第七组
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa0cf 7 第八组
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa0df 7 第九组
	                         
	//本地维护端口配置参数
	0x08,//波特率  缺省值8即2400bps,端口通信波特率/300
	0x00,//校验位  缺省无校验0, 0-无,1-偶,2-奇 
	0x08,//数据位  缺省数据位8
	0x00,//停止位  缺省值0,0-1位停止位,1-1.5停止位,2-2位停止位
	0x00,//端口功能 缺省值0
	 
	//级联维护端口配置参数
	0x06,//波特率  缺省值6即9600bps,端口通信波特率/300
	0x00,//校验位  缺省无校验0, 0-无,1-偶,2-奇 
	0x08,//数据位  缺省数据位8
	0x00,//停止位  缺省值0,0-1位停止位,1-1.5停止位,2-2位停止位
	0x01,//端口功能 缺省值0,0-多功能电能表抄表,1-485总线级联
	                              

	//控制参数
	0x00, //0xa120 1 上电保电时间,HEX,单位分钟
	0xff, 0xff, 0xff, //0xa121 3 月电控告警时间,D0~D23分别对应0点~23点
	
	0x00, //0xa122 1 有功电能计量方式选择
	0x0f, //0xa123 1 正向无功电量累加标志
	0xf0, //0xa124 1 反向无功电量累加标志

	0x00, //0xa130 1 直流模拟量类型,HEX, 01：PT100电阻， 其它值：直流电压模拟量
#ifdef SYS_WIN
    0x00,//0x04, //0xa131 1 右起往左第1个485口功能定义 0：抄表口， 1：被抄口，2：级联口，3：接无功补偿装置
#else
	0x00, //0xa131 1 右起往左第1个485口功能定义 0：抄表口， 1：被抄口，2：级联口，3：接无功补偿装置
#endif
	0x00, //0xa132 1 右起往左第2个485口功能定义 0：抄表口， 1：被抄口，2：级联口，3：接无功补偿装置
	0xc3, //0xa133 级联口通讯参数,默认9600,8位数据位,无校验
	
	//国标版本
	0x00, //国标版本选择 BIN 0表示国标2005版,1表示国标2004版
	//主动上报
	0x01, //二类定时上报模式：BIN 1表示华北模式；0表示普通模式
	//GPRS通信
	0x80, 0x16,	  //0xa142 2 GPRS无通讯复位终端时间,单位分钟,HEX,默认4天
	//总加组相关参数变更
	0x00, //总加组相关参数变更，是否在原有日月累计电量的基础上累计：BIN 1表示不累计；0表示累计    
	//总加组相关参数变更
	0x00, //终端地址显示格式 缺省值0, 0表示十进制显示，1表示十六进制显示   	
#ifdef SYS_WIN
	0x00,//终端网卡1通信方式BIN	1	缺省值0,0表示TCP方式，1表示UDP方式
#else
	0x01,//终端网卡1通信方式BIN	1	缺省值0,0表示TCP方式，1表示UDP方式
#endif

	//0xa150
	0x00, 0x87, 0xFC, 0x74, 0xA7, 0x00, //本机物理地址
	0xC8, 0x01, 0xA8, 0xC0,  //本机IP	test:192.168.1.200
	0x00, 0xFF, 0xFF, 0xFF, //子网掩码
	0x00, 0x00, 0x00, 0x00, //网关

	0x03, 0x00, 0x00, 0x00, //红外口波特率
	0x00,//红外口校验位	
	0x03, 0x00, 0x00, 0x00, //232口波特率
	0x00,//232口校验位

	16,//主站通信密码校验的长度选择，0-默认（2字节），其他为密码的字节数
	0x00,//0xa166 1 485抄表口排列顺序,0:右->左分别是口1,口2...; 1:左->右分别是口1,口2...
	
	//显示专用 
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
//	0xc9,0xee,0xdb,0xda,0xbf,0xc6,0xc2,0xbd,0xb5,0xe7,0xd7,0xd3,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
#ifdef EN_CONCENTRATOR
	#ifdef ENGLISH_DISP
		'L', 'o' ,'w' ,'-' ,'V' ,'o' ,'t' ,'-','C' ,'o' ,'n' ,'c' ,'e' ,'n' ,'t' ,'o' ,'r' ,0x00,0x00,0x00,0x00,//(21) 低压集抄集中器---英文显示
	#else
		0xB5,0xCD,0xD1,0xB9,0xBC,0xAF,0xB3,0xAD,0xBC,0xAF,0xD6,0xD0,0xC6,0xF7,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21) 低压集抄集中器
	#endif
#else
	#ifdef ENGLISH_DISP	//EN_SPECIAL_TRANSFORMER
		'S' ,'P' ,'E' ,'C' ,'I','A' ,'L' ,'-' ,'T' ,'R' ,'A' ,'N' ,'S' ,'F' ,'O','R','M','E','R',0x00,0x00,//(21) 低压集抄集中器---英文显示
	#else
		0xD6,0xC7,0xC4,0xDC,0xD7,0xA8,0xB1,0xE4,0xD6,0xD5,0xB6,0xCB,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21) 智能专变终端
	#endif
#endif
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//(21)
	0x00,0x00,
	//9*21

	0xFF, //0xa180 1 右起往左第3个485口功能定义 0：抄表口， 1：被抄口，2：级联口，3：接无功补偿装置, 0xFF为调试输出
	0x00, //0xa190
	0x00, //0xa191
	0x00, //0xa1a0
	0x00, //0xa1a1
	0x10, 0x00,//0xa1a5	用来配置模块是否需要进入Dormant。设置为0的时候则不使用Dormant，非0的时候则在无通信后wDormantInterv时间内进入Dormant。	
	0x00, //0xa1a6 遥信属性取反标志位0-05,1-697
	0x00,// 0xa1a7越限阀值不带符号位0标志1+系数，1直接使用系数
	0x00,// 0xa1a8 测试模式开关，0，非测试模式，1.测试模式
	0x00, //0xa1a9 0表示以总功率计量电量，1表示以分相功率计量电量
	0x02, //0xa1b0 1为一次侧、2为二次侧、其他值无效 默认为二次侧
	0x00, //0xa1b1	
    0x00, //0xa1b2

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //0xa1b3 设备号
	0x02, //0xa1b4 1 重抄次数，0~7,0表示不进行垃圾队列的重抄,7表示一直进行垃圾队列的重抄,其它表示次数
	0x00, //0xa1b5
	0x00, //0xa1b6 以太网口接入方式选择0：自动获取IP，1：pppoe拨号
	60, 0, //LCD点亮时间
	11,  //0xa1b8 默认GPRS模块类型 GC864==11
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,////0xa1b9(21)
	0x00, //0xa1ba	
	0x01, //0xa1bb	

	0xF4, 0x01, 0x00, 0x00, //0xa1bc
	0x03,//0xa1bd
	0xa0,//0xa1be LCD默认对比度

	0x3c, 0x00, //0xa1c0 2 TCP探测间隔，BIN,单位秒
	0x00, 0x87, 0xFD, 0x75, 0xA6, 0x00, //本机网卡2物理地址，其他网卡参数在标准参数中
	0x01,//终端网卡2通信方式BIN	1	缺省值0,0表示TCP方式，1表示UDP方式
	0x00,  //0xa323    //是否保存调试信息到文件
};

//------------------------------------------------------------------------------------------------------
//中间数据描述表
TItemDesc g_Bank11Desc[] =
{//----标识-----长度------------权限-----------读写--------偏移----写操作-------格式----------Pn个数------------格式描述数组--------------------------------------
	//增加版本控制，已经出去的终端版本注意不要轻易合成这个，否则会升级会造成数据丢失
	{0x0001, 	BN_VER_LEN,	DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//Ver  	
//统计合格率的合格时间与越限时间,高字节存前,合格日+合格月+越限日+越限月,不带格式字节符的
	{0x0010, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//总电压合格时间，
	{0x0011, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//A电压合格时间，高字节存前
	{0x0012, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//B电压合格时间，高字节存前
	{0x0013, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//C电压合格时间，高字节存前
//区间统计的,先周期间隔时间值后频率执行时间
	{0x0014, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//分钟区间统计周期间隔时间点保存,2组TTime格式
	{0x0015, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//小时区间统计周期间隔时间点保存
	{0x0016, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//日区间统计周期间隔时间点保存
	{0x0017, 16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//月区间统计周期间隔时间点保存
	{0x0018, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//年区间统计周期间隔时间点保存
//累加平均统计的,先周期间隔时间值后频率执行时间
	{0x0019, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//分钟累加平均统计周期间隔时间点保存,2组TTime格式
	{0x001a, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//小时累加平均统计周期间隔时间点保存
	{0x001b, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//日累加平均统计周期间隔时间点保存
	{0x001c, 16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//月累加平均统计周期间隔时间点保存
	{0x001d, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//年累加平均统计周期间隔时间点保存
//极值统计的,先周期间隔时间值后频率执行时间
	{0x001e, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//分钟极值统计周期间隔时间点保存,2组TTime格式
	{0x001f, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//小时极值统计周期间隔时间点保存
	{0x0020, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//日极值统计周期间隔时间点保存
	{0x0021, 16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//月极值统计周期间隔时间点保存
	{0x0022, 	16,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//年极值统计周期间隔时间点保存
//统计的冻结ID
//区间的日冻结
	{0x0023, 	141*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
	{0x0024, 	141*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
	{0x0025, 	141*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
	{0x0026, 	141*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
	{0x0027, 	141*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
//累加平均的日冻结
	{0x0028, 	17*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
	{0x0029, 	17*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
	{0x002a, 	17*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
	{0x002b, 	17*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
	{0x002c, 	17*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
//极值的日冻结
	{0x002d, 	33*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
	{0x002e, 33*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
	{0x002f, 	33*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
	{0x0030, 	33*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
	{0x0031, 	33*STAT_OAD_NUM+2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	},//分时日月年
//电压合格率数据ID,2+2*(2+5+3+3+5+5)=48
	{0x0032, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//总电压合格率
	{0x0033, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当月A相电压合格率
	{0x0034, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当月B相电压合格率
	{0x0035, 	48, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//当月C相电压合格率

//日最大有功功率及发生时间的日冻结
	{0x0036, 	15, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//日最大有功功率及发生时间
	{0x0037, 	15, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//月最大有功功率及发生时间

	{0x0038, 	12, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//通信流量
	{0x0039, 	12, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//供电时间
	{0x003a, 	8, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//复位次数
//累加平均的累计次数
	{0x003b, 	2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//分
	{0x003c, 	2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//时
	{0x003d, 2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//日
	{0x003e, 2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//月
	{0x003f, 2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		STAT_OAD_NUM,	},//年






#if 0	//tll 下边的这些ID如果确定都不要用了可以考虑去掉
    {0x00df, 	36, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F27+
    {0x00ef, 	36, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F35+ 
    {0x010f, 	32, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F25 
    {0x011f, 	24, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F26 
    {0x012f, 	66, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F27 

    {0x013f, 	14 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F28 
    {0x014f, 	38, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F29
	{0x015f, 	4 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F30 
    {0x017f, 	32, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F33
    {0x018f, 	24, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F34
    {0x019f, 	66, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F35
    {0x01af, 	16 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F36
    {0x01bf, 	38, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F37
    {0x01cf, 	4 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F38
    {0x01df, 	72, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F41 
    {0x01ef, 	8 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F42 

    {0x020f, 	6 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F43 
    {0x021f, 	6 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F44 
    {0x022f, 	4 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C2F49
    {0x023f, 	4 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C2F50
    {0x024f, 	4 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C2F51
    {0x025f, 	4 , 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,		},//C2F52

    {0x035f, 	114, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F113 
    {0x036f, 	114, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F114                                                                                                                     
    {0x037f, 	114, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F115 
    {0x038f, 	114, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F116 
    {0x039f, 	114, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F117 
    {0x03af, 	114, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F118 
    {0x03bf, 	77, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F121 
    {0x03cf, 	77, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F122 
    {0x03df, 	77, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,},//C2F123 
    {0x03ef, 	14, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMEASURE,},//C2F129 
    {0x040f, 	14, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMEASURE,},//C2F130  
	
    {0x052f, 	1, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXCOMMTHREAD,	},//事件读指针  
    {0x055f, 	7, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXCONTROLTURN,	},//遥控跳闸中间数据
    {0x056f, 	9, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXCONTROLTURN,	},//时段控跳闸中间数据
    {0x057f, 	9, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXCONTROLTURN,	},//厂休控跳闸中间数据
    {0x058f, 	9, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXCONTROLTURN,	},//营业报停控中间数据
    {0x059f, 	9, 			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXCONTROLTURN,	},//临时下浮控跳闸中间数据
    
    {0x0600, PN_MASK_SIZE,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	}, //测量点屏蔽位
	{0x0601, PN_MASK_SIZE,	DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	}, //测量点参数变更标志位
	{0x0602, 1,				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //脉冲测量点对应的参数变更标志(bit0~3对应正有、正无、反有、反无)

    {0x0610, 	4,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //最后一次日冻结时间
	{0x0611, 	4,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //最后一次月冻结时间

	{0x0b01, 	5,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //事件ERC31抄表时间
	{0x0b02, 	4,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //事件ERC31正向有功电能示值
	{0x0b03, 	4,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //事件ERC31正向无功示值
	{0x0b0f, 	0,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, 

#endif

	{0x0b10, 	6,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		FRZ_TASK_NUM,	}, //冻结任务触发冻结命令(延迟时间2 + 接收命令时标4)
	{0x0b11, 	2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		FRZ_TASK_NUM,	}, //复位冻结数据命令(冻结类型1 + 有效标志1)
	{0x0b12, 	1,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		EVT_TOTAL_NUM,	}, //事件清记录标识
	{0x0b13, 	EVT_TRIG_PARA_LEN,		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		EVT_TOTAL_NUM,	}, //事件触发冻结命令
	{0x0b14, 	2,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //复位脉冲数据命令(冻结类型1 + 有效标志1)	
	{0x0b15, 	1,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		MTR_EXC_NUM,	}, //抄表事件清记录标识

	{0x0b16, 	8,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		FRZ_TASK_NUM,	}, //冻结任务触发冻结命令(起始时间4 + 结束时间4)

	{0x0b20, 	9,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //3105触发冻结命令(延迟发生时间2 + 延迟发生时间2 + 接收命令时间4 + 状态机1)
	{0x0b21, 	9,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //310B触发冻结命令(延迟发生时间2 + 延迟发生时间2 + 接收命令时间4 + 状态机1)
	{0x0b22, 	9,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //310C触发冻结命令(延迟发生时间2 + 延迟发生时间2 + 接收命令时间4 + 状态机1)
	{0x0b23, 	9,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //310D触发冻结命令(延迟发生时间2 + 延迟发生时间2 + 接收命令时间4 + 状态机1)
	{0x0b24, 	9,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //310E触发冻结命令(延迟发生时间2 + 延迟发生时间2 + 接收命令时间4 + 状态机1)
	{0x0b25, 	9,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		GB_MAXMETER,	}, //310F触发冻结命令(延迟发生时间2 + 延迟发生时间2 + 接收命令时间4 + 状态机1)

#ifdef EN_SBJC_V2
	{0x0c00,	1,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		1,	}, //水气热表抄失败计数，总标志
	{0x0c01,	1,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		POINT_NUM,	}, //水气热表抄失败计数，抄失败次数超过3次后，不在去抄读
#endif

	{0x0d00,	5,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //最近1次脉冲统计时标
	{0x0d01,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //最近1次脉冲日正有统计起点值
	{0x0d02,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //最近1次脉冲月正有统计起点值
	{0x0d03,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //最近1次脉冲日反有统计起点值
	{0x0d04,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //最近1次脉冲月反有统计起点值
	{0x0d05,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //最近1次脉冲日正无统计起点值
	{0x0d06,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //最近1次脉冲月正无统计起点值
	{0x0d07,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //最近1次脉冲日反无统计起点值
	{0x0d08,	47,			DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,		PULSE_PN_NUM,	}, //最近1次脉冲月反无统计起点值

};

//------------------------------------------------------------------------------------------------------
TItemDesc  g_Bank16Desc[] =
{//----标识-----长度------------权限-----------读写--------偏移----写操作-------格式----------Pn个数------
	//Oob
	{0x6001,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //
	{0x6002,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //记录任务上报的执行时间
	{0x6003,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //记录任务上报的执行时间IF_GPRS通道
	{0x6004,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //记录任务上报的执行时间
	{0x6005,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //记录任务上报的执行时间IF_SOCKET
	{0x6006,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //记录任务上报的执行时间
	{0x6007,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //记录任务上报的执行时间
	{0x6008,	4,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //记录任务上报的执行时间(短信模式)
	{0x6010,	PN_MASK_SIZE,		DI_LOW_PERM,	DI_READ|DI_WRITE,	0,	INFO_NONE,		FMT_UNK,		TASK_NUM,	}, //载波发现未知电表告警事件屏蔽字
};

//------------------------------------------------------------------------------------------------------
TItemDesc  g_Bank17Desc[] =
{//----标识-----长度------------权限-----------读写--------偏移----写操作-------格式----------Pn个数------
	//Oob
	{0x6001,   PN_MASK_SIZE,DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //测量点屏蔽位,包括载波、485
	{0x6002,   PN_MASK_SIZE,DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //485测量点屏蔽位
	{0x6003,   PN_MASK_SIZE,DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //载波测量点屏蔽位
	{0x6004,   PN_MASK_SIZE,DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //载波测量点采集器屏蔽位
	
	{0x6010,	1,			DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //公网通信模块：0--公网通信模块1，1--公网通信模块2
	{0x6011,	1,			DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //每个公网通信模块存在多个主站IP地址，本参数用于区分不同IP地址
	{0x6012,	1,			DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //以太网通信模块：0--以太网通信模块1，1--以太网通信模块2...
	{0x6013,	1,			DI_LOW_PERM, DI_READ|DI_WRITE,	0, INFO_NONE,		FMT_UNK,		1,			}, //每个以太网信模块存在多个主站IP地址，本参数用于区分不同IP地址
};

//------------------------------------------------------------------------------------------------------
//中间数据描述表
TItemDesc g_Bank18Desc[] =
{//----标识-----长度------------权限-----------读写--------偏移----写操作-------格式----------Pn个数------------格式描述数组--------------------------------------
    {0x0001, 	BN_VER_LEN,			DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//Ver
    {0x003f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F41+
    {0x004f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F42+                                                                                                                    
    {0x005f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F43+
    {0x006f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F44+
    //{0x007f, 	41, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		g_bC1F21Fmt},//C1F21+
    //{0x008f, 	41, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		g_bC1F22Fmt},//C1F22+
    {0x009f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F45+
    {0x00af, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F46+
    {0x00bf, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F47+
    {0x00cf, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F48+
	{0x026f, 	12, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,	},//C2F57
	{0x029f, 	12, 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,	},//C2F60
	{0x02cf, 	6 , 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,	},//C2F65 
    {0x02df, 	6 , 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,	},//C2F66 
	
	{0x031f, 	8 , 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F97
    {0x032f, 	8 , 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F98
    {0x033f, 	8 , 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F99
    {0x034f, 	8 , 				DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C2F100

	//总加组日月起点及起点时的累计值，用于处理参数变更
	{0x035f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F19
    {0x036f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F20
	{0x037f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F21
    {0x038f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F22
	{0x039f, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F19+
    {0x03af, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F20+
	{0x03bf, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F21+
    {0x03cf, 	40+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXSUMGROUP,		},//C1F22+

	//测量点日月起点时的累计值,用于处理电表示度下降或满度,目前备用 
	{0x040f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F41
    {0x041f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F42
	{0x042f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F43
    {0x043f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F44
	{0x044f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F45
	{0x045f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F46
    {0x046f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F47
	{0x047f, 	20+RATELEN, 		DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	POINT_NUM,	},//C1F48
	
	{0x0610, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //最后一次日冻结时间
	{0x0611, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //最后一次月冻结时间
	{0x0612, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //最后一次月冻结时间
	{0x0613, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	GB_MAXMETER,}, //最后一次月冻结时间
	{0x0614, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	8,			}, //脉冲最后一次月冻结时间,每一路脉冲对应一个测量点
	{0x0615, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	8,			}, //脉冲最后一次月冻结时间,每一路脉冲对应一个测量点
	{0x0616, 	4,					DI_HIGH_PERM, DI_READ|DI_WRITE, 0, 	INFO_NONE,	FMT_UNK,	8,			}, //脉冲最后一次月冻结时间,每一路脉冲对应一个测量点

};

//终端增补规约参数：标识--长度--权限--读写--偏移
TItemDesc  g_Bank24Desc[] =   //
{
	{0x0001, 	BN_VER_LEN,	DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//Ver

	{0x001f,	5,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, GB_MAXCONTROLTURN}, //C4F91
	{0x002f,	12,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, GB_MAXMETER}, //C492
	{0x003f,	36,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //C493
	{0x0040,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //电台发送是否需要前导字节控制参数
	{0x0041,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //电台发送是否需要RS编码控制参数
	{0x0042,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //电台发送是否需要控制电台参数
	{0x0043,	4,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_APP_RST,	FMT_UNK, 1}, //GPRS或电台接口波特率
	{0x0044,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_APP_RST,	FMT_UNK, 1}, //GPRS或电台接口校验位
	{0x0045,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_230M_PARA, FMT_UNK, 1}, //GPRS或电台接口校验位
	{0x0046,	4,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //电台接口读数据延时ms
	{0x0047,	4,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //电台发送后切换延时ms
	{0x0048,	4,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_230M_PARA, FMT_UNK, 1}, //电台数传延时ms
	{0x0049,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //轮显时间间隔设置(未用)

	{0x004a,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, //0x004a语音模块类型0-ISD4002；1-tts6188；
	{0x004b,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1}, // 0x004b tts6188语音音量大小

	{0x0060,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_APP_RST,	FMT_UNK, 1}, //辽宁230终端地区版本选择 0:大连 1:沈阳 2:盘锦 3:本溪 4:其它
	{0x0061,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_APP_RST,	FMT_UNK, 1}, //是否带内置交采功能 0:不带 1:带
	{0x0062,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_APP_RST,	FMT_UNK, 1}, //外置交采装置历史数据生成方式 0:抄交采装置 1:终端本身冻结
	{0x4001,	140,	DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1},//FTP升级传输文件（下行）
	{0x4002,	25,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1},//FTP升级传输文件结果（上行）
	{0x4003,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK, 1},//FTP升级标志。1:刚升级，0:未升级 默认0
	{0x4102,	6,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN, GB_MAXMETER,},//记录最近一次收到主站回复的二类数据主动上报确认帧的时间
	
	{0x5001,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},//是否进行版本判断,0不进行版本判断，非0启动版本判断
	{0x5002,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},//是否启用加密模块,1启用加密模块，其他值不启动
	{0x5003,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_PORTSCH_PARA,	FMT_BIN,1},//是否启用启用自动搜表端口号,1启用，其他值不启动
	{0x5004,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},//是否开放ERC70事件,1开放，其他值关闭
	{0x5005,	1, 		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_UNK,	1},	//跳闸版本1.为云南97协议,2为黑龙江模式
	{0x5006,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},//有效轮次个数
	{0x5007,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},//默认兰吉尔电表子协议号	
	{0x5008,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},// 对电表的抄读数据命令的应答中对电表的数据是否需要减去0x33
	{0x5009,	1,		DI_LOW_PERM,		DI_READ|DI_WRITE, 0,	INFO_NONE,		FMT_BIN,	1},// 对3P3W接线方式的电表电压不平衡度计算方式 0:按三相计算 1:按二相计算
	{0x5010, 	64, 	DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_COMM_RLD,	FMT_UNK,	1,},//(海南05扩展)终端IP地址和端口
	{0x5011, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//F85(天津) 终端地址参数
	{0x5012, 	8, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//F243(天津)终端sim卡号码
	{0x5015,    520,    DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//(新国标1类数据)文件下装未收到数据段 
	{0x5016, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//补抄方式（0:按冻结时标 1:按冻结次数）
	{0x5017, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//补抄冻结数据开始的时间（小时）
	{0x5018,    10,     DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//参数设置F91
	{0x5020, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//补抄方式（0:按冻结时标 1:按冻结次数）
	{0x5021, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,	1,},//补抄开关:0-全部关闭 1-只补抄曲线 2-只补抄日月冻结 3-都补抄
	{0x5023, 	2, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BCD,	1,},//脉冲宽度 单位毫秒
	{0x5024, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,	1,},//随即延时登陆（D0:休眠时登陆 D1:失败时登陆）
	{0x5025, 	1, 		DI_HIGH_PERM,		DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_BIN,	1,},//是否保存调试信息（0：否 1：是）
};

BYTE g_bBank24Default[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)

	0x00, 0x00, 0x00, 0x00, 0x00, //C491(5) 下浮控的告警时间和控制时间
	'1', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //C492(12) 电能表局编号
	0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 
	0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 0x00, 0x05, 
	0x00, 0x05, 0x00, 0x05, //C493(36) 交流装置的各次谐波电压含有率上限参数
	0x01, //0x0040 1 电台发送是否需要前导字节控制参数
	0x00, //0x0041 1 电台发送是否需要RS编码控制参数
	0x01, //0x0042 1 电台发送是否需要控制电台参数
	0x02, 0x00, 0x00, 0x00, //0x0043 4 BCD GPRS或电台接口波特率
	0x02, //0x0044 1 GPRS或电台接口校验位
	0x02, //0x0045 1 电台接口发送数据时的校验位
	0x00, 0x03, 0x00, 0x00, //0x0046 4 BCD 电台接口读数据延时ms
	0x00, 0x00, 0x00, 0x00, //0x0047 4 BCD 电台发送后切换延时ms
	0x50, 0x02, 0x00, 0x00, //0x0048 4 BCD 电台数传延时ms
	0x10, //轮显时间间隔设置(未用)

	0x01, //0x004a语音模块类型0-ISD4002；1-tts6188；
	0x05, // 0x004b tts6188语音音量大小

	0x00, //0x0060 辽宁230终端地区版本选择 0:大连 1:沈阳 2:盘锦 3:本溪 4:其它
	0x00, //0x0061 是否带内置交采功能 0:不带 1:带
	0x01, //0x0062 外置交采装置历史数据生成方式 0:抄交采装置 1:终端本身冻结
	//0x4001 FTP升级传输文件（下行）
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	//0x4002 FTP升级传输文件结果（上行）
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,

	0x00,	//0x4003 FTP升级标志。1:刚升级，0:未升级 默认0

	0x00,0x00,0x00,0x00,0x00,0x00,//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,//是否进行版本判断 ,0不进行版本判断
	0x01,//是否启用加密模块,0不进行不使用加密模块
	0x01, //0x5003 是否启用启用自动搜表端口号,1启用，其他值不启动
	0x01, //0x5004 是否开放ERC70事件,1开放，其他值关闭
	0x01, //0x5005 跳闸版本1.为云南97协议,2为黑龙江模式	
	0x04,//默认控制模块有效轮次个数
	0x00,//默认兰吉尔电表子协议号	
	0x00, //0x5008  对电表的抄读数据命令的应答中对电表的数据是否需要减去0x33 1--不需要，其他值不需要
	0x00, //0x5009 对3P3W接线方式的电表电压不平衡度计算方式 0:按三相计算(台体测试方式) 1:按二相计算
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,//FN98 (64) 终端IP地址和端口
	0x00, //0x5011
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,//0x5012

	//0x5015 下装文件标志位，置1：未成功，置0：成功
	0x00,0x00,//组1
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,//组2
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,//组3
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,//组4
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0x01,//0x5016
	0x01,//0x5017
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,//0x5020
	0x00,//0x5021
	0x00,0x03,//0x5023
	0x00,//0x5024
	0x01,//0x5025
};

//终端校准参数描述表：标识--长度--权限--读写--偏移
TItemDesc  g_Bank25Desc[] =   //标准版
{
	{0x0001, 	BN_VER_LEN,	DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//Ver

	{0x5001, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //额定电压 BCD NNNN.NN
	{0x5002, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //额定电流 BCD NNN.NNN
	{0x5003, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //接线方式 BCD NN
	{0x5004, 3, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //脉冲常数 BCD NNNNNN
	{0x5005, 72, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},	//7022校准参数	
	{0x500f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},

  {0x5011, 96, DI_LOW_PERM,DI_READ|DI_WRITE, 	0, INFO_NONE,FMT_BIN,1},//直流校准系数
  {0x501f, 0,  DI_LOW_PERM,DI_READ|DI_WRITE, 	0, INFO_NONE},	 

//  {0x5020, 1,  DI_LOW_PERM,DI_READ|DI_WRITE, 	0, INFO_NONE},	//校准时的温度 
	{0x5021, 12, DI_LOW_PERM,DI_READ|DI_WRITE, 	0, INFO_NONE},//生产编号
	{0x5022, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //保存校准时的温度
	{0x5023, 4, DI_LOW_PERM,DI_READ|DI_WRITE, 	0, INFO_NONE}, // k值应是在额定电压电流下（信号对应50mv）经过7022增益放大后的电Vu电流Vi的乘积。即k=(Vu*增益)*（Vi*增益）
};

BYTE g_bBank25Default[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)

	0x00, 0x00, 0x00, //0x5001 额定电压 BCD NNNN.NN
	0x00, 0x00, 0x00, //0x5002 额定电流 BCD NNN.NNN
	0x00, //0x5003 接线方式 BCD NN
	0x00, 0x00, 0x00, //0x5004 脉冲常数 BCD NNNNNN
	0xDA, 0x0B, 0x00, 0x00, 0x45, 0x0C, 0x00, 0x00, 0xB9, 0x0E, 0x00, 0x00, 0x90, 0x00, 0x00, 0x00, 
	0xAA, 0x00, 0x00, 0x00, 0x1B, 0x00, 0x00, 0x00, 0x86, 0x00, 0x00, 0x00, 0x9B, 0x00, 0x00, 0x00, 
	0x81, 0x00, 0x00, 0x00, 0xEF, 0xFF, 0x00, 0x00, 0xED, 0xFF, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 
	0xF6, 0x11, 0x00, 0x00, 0x30, 0x12, 0x00, 0x00, 0xD2, 0x14, 0x00, 0x00, 0x4D, 0xA8, 0x00, 0x00, 
	0x61, 0xA8, 0x00, 0x00, 0x58, 0xA8, 0x00, 0x00, //0x5005 ATT7022的校正参数

    0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 
	0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 
    0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 
	0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 	
    0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 
	0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, 0x69, 0x59, 0x00, 0x00, //5011 直流校准系数

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //生产编号
	0x00,//保存校准时的温度
	0x52, 0x03, 0x00, 0x00, //5023 k值  0.05*0.17*100000
};

TItemDesc  g_Bank28Desc[] =   //标准版
{
	{0x0001, 	BN_VER_LEN,	DI_HIGH_PERM,	DI_READ|DI_WRITE, 0, 	INFO_NONE,		FMT_UNK,		1,	},//Ver

	{0x0002, 32, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
	{0x0003, 8, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE},

	{0x0011, 8, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Iregion[2]; 		// 相位校准分区点，7022E只有2个相位校准分区点
	{0x0012, 12, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Pgain[3];			// 功率校准值A,B,C
	{0x0013, 12, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Phsreg[3];			// 三相校准点的相位补偿
	{0x0014, 12, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //DWORD	Poffset[3]; 		// 三相有功功率Offset校正值
	{0x0015, 6, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //WORD Ioffset[3];		// 电流有效值Offset校正值
	{0x0016, 18, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //signed short	AngleOffset[3][3];	// 3个分区ABC三相相位误差补偿值，单位：0.01，带符号
	{0x0017, 6, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //signed short EngErrAdj[3];			// 人为误差调整值,单位0.0001%.
	{0x0018, 2, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //WORD	KDivFactor; 		// K值放大倍数 *100
	{0x0019, 1, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, //BYTE	AdjustFlag; 		// 校准标志，校表后置位
	{0x001f, 0, DI_LOW_PERM, DI_READ|DI_WRITE, 0, INFO_NONE}, 
};

BYTE g_bBank28Default[] = 
{
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //Ver(20)

	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, //0002 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, //0002 

	//new 77
	//DWORD Iregion[2]; 		// 相位校准分区点，7022E只有2个相位校准分区点
	0xC8, 0x00, 0x00, 0x00,
	0x0A, 0x00, 0x00, 0x00,
	//DWORD Pgain[3];			// 功率校准值A,B,C
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	//DWORD Phsreg[3];			// 三相校准点的相位补偿
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	//DWORD Poffset[3]; 		// 三相有功功率Offset校正值
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	//WORD	Ioffset[3]; 		// 电流有效值Offset校正值
	0x08, 0x00,
	0x08, 0x00,
	0x08, 0x00,
	//signed short	AngleOffset[3][3];	// 3个分区ABC三相相位误差补偿值，单位：0.01，带符号
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	//signed short EngErrAdj[3];			// 人为误差调整值,单位0.0001%.
	0x00, 0x00, 
	0x00, 0x00, 
	0x00, 0x00,
	//WORD	KDivFactor; 		// K值放大倍数 *100
	0x64, 0x00,
	//BYTE	AdjustFlag; 		// 校准标志，校表后置位
	0x00,
	
};

//数据库的组控制块
TBankCtrl g_Bank0Ctrl[SECT_NUM] = {
	//SECTION0
	{"sect0 Eng Data",								//本SECTION的名称
	 USER_DATA_PATH"EngData.dat",					//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_EngDataDesc,									//本SECTION数据项描述表
	 sizeof(g_EngDataDesc)/sizeof(TItemDesc),		//本SECTION数据项描述表的数据项个数
	 NULL,											//本SECTION数据库的默认值	
	 0x00,											//本SECTION数据库的默认值的大小
	 0x01,											//本SECTION数据库的当前版本,0表示没有版本管理
	 1,												//本SECTION数据的测量点个数
	 1,												//本SECTION数据的镜像个数
	 false,											//本BANK数据是否需要更新时间
	},

	//SECTION1
	{"sect1 Dem-Data",								//本SECTION的名称
	 USER_DATA_PATH"DemData.dat",					//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_DemDataDesc,									//本SECTION数据项描述表
	 sizeof(g_DemDataDesc)/sizeof(TItemDesc),		//本SECTION数据项描述表的数据项个数
	 NULL,											//本SECTION数据库的默认值	
	 0x00,											//本SECTION数据库的默认值的大小
	 0x01,											//本SECTION数据库的当前版本,0表示没有版本管理
	 1,												//本SECTION数据的测量点个数
	 1,												//本SECTION数据的镜像个数
	 false,											//本BANK数据是否需要更新时间
	},

	//SECTION2
	{"sect2 Variable-Data",							//本SECTION的名称
	 USER_DATA_PATH"VarData.dat",					//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_VariableDesc,								//本SECTION数据项描述表
	 sizeof(g_VariableDesc)/sizeof(TItemDesc),		//本SECTION数据项描述表的数据项个数
	 NULL,											//本SECTION数据库的默认值	
	 0x00,											//本SECTION数据库的默认值的大小
	 0x00,											//本SECTION数据库的当前版本,0表示没有版本管理
	 1,												//本SECTION数据的测量点个数
	 1,												//本SECTION数据的镜像个数
	 true,											//本BANK数据是否需要更新时间
	},

	//SECTION3
	{"sect3 Event-Data",							//本SECTION的名称
	 USER_PARA_PATH"EventData.cfg",					//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_EventParaDesc,									//本SECTION数据项描述表
	 sizeof(g_EventParaDesc)/sizeof(TItemDesc),			//本SECTION数据项描述表的数据项个数
	 g_EventParaDefault,											//本SECTION数据库的默认值	
	 sizeof(g_EventParaDefault),											//本SECTION数据库的默认值的大小
	 0x01,											//本SECTION数据库的当前版本,0表示没有版本管理
	 1,												//本SECTION数据的测量点个数
	 1,												//本SECTION数据的镜像个数
	 false,											//本BANK数据是否需要更新时间
	},

	//SECTION4
	{"sect4 para-varible",							//本SECTION的名称
	 USER_PARA_PATH"ParaVar.dat",					//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_ParaDesc,									//本SECTION数据项描述表
	 sizeof(g_ParaDesc)/sizeof(TItemDesc),			//本SECTION数据项描述表的数据项个数
	 g_bParaDefault,								//本SECTION数据库的默认值	
	 sizeof(g_bParaDefault),						//本SECTION数据库的默认值的大小
	 0x01,											//本SECTION数据库的当前版本,0表示没有版本管理
	 1,												//本SECTION数据的测量点个数
	 1,												//本SECTION数据的镜像个数
	 false,											//本BANK数据是否需要更新时间
	},

	//SECTION5
	{"sect5 Frz-data",							//本SECTION的名称
	 USER_PARA_PATH"Frz-Data.cfg",					//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_FrzDesc,									//本SECTION数据项描述表
	 sizeof(g_FrzDesc)/sizeof(TItemDesc),		//本SECTION数据项描述表的数据项个数
	 g_FrzDescDefault,							//本SECTION数据库的默认值	
	 sizeof(g_FrzDescDefault),					//本SECTION数据库的默认值的大小
	 0x02,										//本SECTION数据库的当前版本,0表示没有版本管理
	 1,											//本SECTION数据的测量点个数
	 1,											//本SECTION数据的镜像个数
	 false,										//本BANK数据是否需要更新时间
	},

	//SECTION6
	{"sect6 Acq-Moni-para",				//本SECTION的名称
	 USER_PARA_PATH"acq-moni.cfg",			//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_CollecMonitorDesc,				        //本SECTION数据项描述表
	 sizeof(g_CollecMonitorDesc)/sizeof(TItemDesc), //本SECTION数据项描述表的数据项个数
	 NULL,										//本SECTION数据库的默认值	
	 0,											//本SECTION数据库的默认值的大小
	 0x01,										//本SECTION数据库的当前版本,0表示没有版本管理
	 1,											//本SECTION数据的测量点个数
	 1,											//本SECTION数据的镜像个数
	 false,										//本BANK数据是否需要更新时间
	},

	//SECTION7
	{"sect7 Class-set object",					//本SECTION的名称
	 USER_DATA_PATH"Class-Set.dat",				//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_SetDesc,									//本SECTION数据项描述表
	 sizeof(g_SetDesc)/sizeof(TItemDesc),		//本SECTION数据项描述表的数据项个数
	 NULL,										//本SECTION数据库的默认值	
	 0,											//本SECTION数据库的默认值的大小
	 0x01,										//本SECTION数据库的当前版本,0表示没有版本管理
	 1,											//本SECTION数据的测量点个数
	 1,											//本SECTION数据的镜像个数
	 true,										//本BANK数据是否需要更新时间
	},

	//SECTION8
	{"sect8 Class-Ctrl object",					//本SECTION的名称
	 USER_PARA_PATH"Ctrl.cfg",				//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_CtrlDesc,								//本SECTION数据项描述表
	 sizeof(g_CtrlDesc)/sizeof(TItemDesc),		//本SECTION数据项描述表的数据项个数
	 NULL,										//本SECTION数据库的默认值
	 0,											//本SECTION数据库的默认值的大小
	 0x01,										//本BANK数据库的当前版本,0表示没有版本管理
	 1,											//本SECTION数据的测量点个数
	 1,											//本SECTION数据的镜像个数
	 true,										//本BANK数据是否需要更新时间
	 MTRPNMAP,									//本SECTION数据选用的动态测量点方案号,0x00表示不支持整个BANK地支持动态测量点
	},


	//SECTION9
	{"sect9 File-Tran object",					//本SECTION的名称
	 USER_DATA_PATH"FileTran.dat",					//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_FileTransDesc,							//本SECTION数据项描述表
	 sizeof(g_FileTransDesc)/sizeof(TItemDesc),	//本SECTION数据项描述表的数据项个数
	 NULL,										//本SECTION数据库的默认值
	 0,											//本SECTION数据库的默认值的大小
	 0x00,										//本BANK数据库的当前版本,0表示没有版本管理
	 1,											//本SECTION数据的测量点个数
	 1,											//本SECTION数据的镜像个数
	 true,										//本BANK数据是否需要更新时间
	 MTRPNMAP,									//本SECTION数据选用的动态测量点方案号,0x00表示不支持整个BANK地支持动态测量点
	},

	//SECTION10
	{"sect10 Esam-If object",					//本SECTION的名称
	 USER_PARA_PATH"Esam.cfg",					//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_ESAMIfDesc,								//本SECTION数据项描述表
	 sizeof(g_ESAMIfDesc)/sizeof(TItemDesc),	//本SECTION数据项描述表的数据项个数
	 NULL,										//本SECTION数据库的默认值
	 0x00,										//本SECTION数据库的默认值的大小
	 0x01,										//本BANK数据库的当前版本,0表示没有版本管理
	 1,											//本SECTION数据的测量点个数
	 1,											//本SECTION数据的镜像个数
	 false,										//本BANK数据是否需要更新时间
	},	

	//SECTION11
	{"sect11 In-Out-Dev object",				//本SECTION的名称
	 USER_PARA_PATH"In_Out_Dev.cfg",			//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_InOutDevDesc,					        //本SECTION数据项描述表
	 sizeof(g_InOutDevDesc)/sizeof(TItemDesc),	//本SECTION数据项描述表的数据项个数
	 g_InOutDevDefault,							//本SECTION数据库的默认值
	 sizeof(g_InOutDevDefault),					//本SECTION数据库的默认值的大小
	 0x01,										//本BANK数据库的当前版本,0表示没有版本管理
	 1,											//本SECTION数据的测量点个数
	 1,											//本SECTION数据的镜像个数
	 false,										//本BANK数据是否需要更新时间
	},	
	
	//SECTION12
	{"sect12 Display object",					//本SECTION的名称
	 USER_PARA_PATH"Display.cfg",				//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_DisPlayDesc,								//本SECTION数据项描述表
	 sizeof(g_DisPlayDesc)/sizeof(TItemDesc),	//本SECTION数据项描述表的数据项个数
	 NULL,										//本SECTION数据库的默认值
	 0x00,										//本SECTION数据库的默认值的大小
	 0x00,										//本BANK数据库的当前版本,0表示没有版本管理
	 1,											//本SECTION数据的测量点个数
	 1,											//本SECTION数据的镜像个数
	 false,										//本BANK数据是否需要更新时间
	},
	
	//SECTION13
	{"sect13 Ac-Eng-data",						//本SECTION的名称
	 USER_DATA_PATH"AcEngData.dat",				//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_AcDataDesc,								//本SECTION数据项描述表
	 sizeof(g_AcDataDesc)/sizeof(TItemDesc),	//本SECTION数据项描述表的数据项个数
	 NULL,										//本SECTION数据库的默认值
	 0x00,										//本SECTION数据库的默认值的大小
	 0x00,										//本BANK数据库的当前版本,0表示没有版本管理
	 1,											//本SECTION数据的测量点个数
	 1,											//本SECTION数据的镜像个数
	 false,										//本BANK数据是否需要更新时间
	},

	//SECTION14
	{"sect14 point data",						//本SECTION的名称
	 USER_DATA_PATH"point%d.dat",				//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_PointDataDesc,					        //本SECTION数据项描述表
	 sizeof(g_PointDataDesc)/sizeof(TItemDesc), //本SECTION数据项描述表的数据项个数
	 NULL,										//本SECTION数据库的默认值
	 0,											//本SECTION数据库的默认值的大小
	 0x00,										//本BANK数据库的当前版本,0表示没有版本管理
	 PN_NUM,									//本SECTION数据的测量点个数
	 1,											//本SECTION数据的镜像个数
	 true,										//本BANK数据是否需要更新时间
	 MTRPNMAP,									//本SECTION数据选用的动态测量点方案号,0x00表示不支持整个BANK地支持动态测量点
	},

	//SECTION15
	{"sect15 Ext-Variable-Para",				//本SECTION的名称
	USER_PARA_PATH"ExtVarPara.cfg",				//本SECTION数据保存的路径文件名
	NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	g_ExtVarParaDesc,							//本SECTION数据项描述表
	sizeof(g_ExtVarParaDesc)/sizeof(TItemDesc),	//本SECTION数据项描述表的数据项个数
	NULL,										//本SECTION数据库的默认值	g_ExtVarParaDefault
	0x00,										//本SECTION数据库的默认值的大小	sizeof(g_ExtVarParaDefault)
	0x00,										//本BANK数据库的当前版本,0表示没有版本管理
	1,											//本SECTION数据的测量点个数
	1,											//本SECTION数据的镜像个数
	false,										//本BANK数据是否需要更新时间
	},

	//SECTION16
	{"sect16 Ext-Event-Data",					//本SECTION的名称
	USER_DATA_PATH"ExtEventData.dat",			//本SECTION数据保存的路径文件名
	NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	g_EventDataDesc,							//本SECTION数据项描述表
	sizeof(g_EventDataDesc)/sizeof(TItemDesc),	//本SECTION数据项描述表的数据项个数
	g_EventDataDefault,							//本SECTION数据库的默认值
	sizeof(g_EventDataDefault),					//本SECTION数据库的默认值的大小
	0x01,										//本BANK数据库的当前版本,0表示没有版本管理
	1,											//本SECTION数据的测量点个数
	1,											//本SECTION数据的镜像个数
	false,										//本BANK数据是否需要更新时间
	},

	//SECTION17
	{"sect17 Ext-Frz-data",						//本SECTION的名称
	 USER_DATA_PATH"Ext-Frz-Data.dat",			//本SECTION数据保存的路径文件名
	 NULL,										//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,										//本SECTION数据项描述表
	 0x00,										//本SECTION数据项描述表的数据项个数
	 NULL,										//本SECTION数据库的默认值
	 0x00,										//本SECTION数据库的默认值的大小
	 0x00,										//本BANK数据库的当前版本,0表示没有版本管理
	 1,											//本SECTION数据的测量点个数
	 1,											//本SECTION数据的镜像个数
	 false,										//本BANK数据是否需要更新时间
	 },
};


//数据库的组控制块
TBankCtrl g_BankCtrl[BANK_NUM] = {

	//BANK0		浙江协议规定的基本数据标识不用本控制块来管理
	{"bank0 sys data",						//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},

	//BANK1
	{"bank1 expara",						//本BANK的名称
	 USER_PARA_PATH"bank1.cfg",				//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_Bank1Desc,					        //本BANK数据项描述表
	 sizeof(g_Bank1Desc)/sizeof(TItemDesc), //本BANK数据项描述表的数据项个数
	 g_bBank1Default,						//本BANK数据库的默认值	
	 sizeof(g_bBank1Default),				//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},
	 
	//BANK2
	{"bank2 runtime data",					//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名,不需要保存
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_Bank2Desc,					        //本BANK数据项描述表
	 sizeof(g_Bank2Desc)/sizeof(TItemDesc), //本BANK数据项描述表的数据项个数
	 NULL,						            //本BANK数据库的默认值,无默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},

	//BANK3
	{"bank3 expara",						//本BANK的名称
	 USER_PARA_PATH"bank3.cfg",             //本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_Bank3Desc,					        //本BANK数据项描述表
	 sizeof(g_Bank3Desc)/sizeof(TItemDesc), //本BANK数据项描述表的数据项个数
	 g_bBank3Default,						//本BANK数据库的默认值	
	 sizeof(g_bBank3Default),				//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},
	 
	//BANK4 --------------电表扩展参数-----------------
	{"bank4 net meter expara",				//本BANK的名称
	 USER_PARA_PATH"bank4.cfg",        		//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_Bank4Desc,					        //本BANK数据项描述表
	 sizeof(g_Bank4Desc)/sizeof(TItemDesc), //本BANK数据项描述表的数据项个数
	 g_bBank4Default,						//本BANK数据库的默认值	
	 sizeof(g_bBank4Default),				//本BANK数据库的默认值的大小
	 0x04,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},
	 
	//BANK5 -------------- 电表扩展数据 ---------------
	{"bank5 07 645 exdata",					//本BANK的名称
	 USER_PARA_PATH"bank5.dat",        		//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_Bank5Desc,					        //本BANK数据项描述表
	 sizeof(g_Bank5Desc)/sizeof(TItemDesc), //本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x02,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},

	//BANK6 -------------- 电表冻结数据 ---------------
	{"bank6 net meter frozen data",			//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},

	//BANK7 -------------- 广东配变扩展参数 ---------------
	{"bank7 gdpb expara",					//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},

	//BANK8 -------------- 扩展 ---------------
	 {"bank8",								//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	 },

	//BANK9 -------------- 扩展 ---------------
	{"bank9 sd expara",						//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},

	//BANK10 -------------- 扩展 ---------------
	{"bank10 GB expara",					//本BANK的名称
	 USER_PARA_PATH"bank10.cfg",           	//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_Bank10Desc,					        //本BANK数据项描述表
	 sizeof(g_Bank10Desc)/sizeof(TItemDesc),//本BANK数据项描述表的数据项个数
	 g_bBank10Default,						//本BANK数据库的默认值	
	 sizeof(g_bBank10Default),				//本BANK数据库的默认值的大小
	 0x11,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},

	//BANK11-------------- 国标版中间数据 ---------------
	{"bank11 GB tmp data",					//本BANK的名称
	 USER_DATA_PATH"bank11.dat",			//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_Bank11Desc,					        //本BANK数据项描述表
	 sizeof(g_Bank11Desc)/sizeof(TItemDesc),//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x02,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 true,									//本BANK数据是否需要更新时间
	},

	//BANK12 -------------- 扩展 ---------------
	{"bank12 sd expara",						//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},

	//BANK13 -------------- 扩展 ---------------
	{"bank13 sd expara",						//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},

	//BANK14 -------------- 扩展 ---------------
	{"bank14 sd expara",						//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},

	//BANK15 --------------  ---------------
	 {"bank15 ",								//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	 },

	 //BANK16 --------------  ---------------
	{"bank16 temp data",					//本BANK的名称
	USER_DATA_PATH"bank16.dat",            //本BANK数据保存的路径文件名
	NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	g_Bank16Desc,					        //本BANK数据项描述表
	sizeof(g_Bank16Desc)/sizeof(TItemDesc),//本BANK数据项描述表的数据项个数
	NULL,									//本BANK数据库的默认值
	0,										//本BANK数据库的默认值的大小
	0x01,									//本BANK数据库的当前版本,0表示没有版本管理
	1,										//本BANK数据的测量点个数
	1,										//本BANK数据的镜像个数
	false,									//本BANK数据是否需要更新时间
	},

	 //BANK17 ----------- 集抄非保存数据 ------------------------
	 {"bank17 cct unsave data",				//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_Bank17Desc,					        //本BANK数据项描述表
	 sizeof(g_Bank17Desc)/sizeof(TItemDesc),//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	 },
	
	//BANK18-------------- 测量点日月起点数据 单独分出来因为要频繁保存 ---------------
	{"bank18 GB tmp data",					//本BANK的名称
	 USER_DATA_PATH"bank18.dat",			//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_Bank18Desc,					        //本BANK数据项描述表
	 sizeof(g_Bank18Desc)/sizeof(TItemDesc),//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x01,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 true,									//本BANK数据是否需要更新时间
	},

	//BANK19 --------------  ---------------
	{"bank19 ",								//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},

	//BANK20 --------------  ---------------
	 {"bank20 ",								//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	 },

	 //BANK21 --------------  ---------------
	 {"bank21 ",								//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	 },

	 //BANK22 --------------  ---------------
	 {"bank22 ",								//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	 },

	 //BANK23 --------------  ---------------
	 {"bank23 ",								//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	 },

		//BANK24 -------------- 扩展 ---------------
	{"bank24 GB Temp expara",						//本BANK的名称
	 USER_PARA_PATH"bank24.cfg",        //本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_Bank24Desc,					        		//本BANK数据项描述表
	 sizeof(g_Bank24Desc)/sizeof(TItemDesc), 		//本BANK数据项描述表的数据项个数
	 g_bBank24Default,							//本BANK数据库的默认值	
	 sizeof(g_bBank24Default),					//本BANK数据库的默认值的大小
	 0x03,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},

	//BANK25 校准参数扩展
	{"bank25 att adj expara",						//本BANK的名称
	 USER_PARA_PATH"bank25.cfg",             //本BANK数据保存的路径文件名
	 USER_BAK_PATH"bank25.cfg",				//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_Bank25Desc,					        //本BANK数据项描述表
	 sizeof(g_Bank25Desc)/sizeof(TItemDesc), //本BANK数据项描述表的数据项个数
	 g_bBank25Default,						//本BANK数据库的默认值	
	 sizeof(g_bBank25Default),				//本BANK数据库的默认值的大小
	 0x04,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},
	
		//BANK26 ------------- 未用参数 ---------------
	{"bank26 no use para",				//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},
	
		//BANK27 ------------- 未用参数 ---------------
	{"bank27 no use para",				//本BANK的名称
	 NULL,           						//本BANK数据保存的路径文件名
	 NULL,									//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 NULL,					        		//本BANK数据项描述表
	 0, 									//本BANK数据项描述表的数据项个数
	 NULL,									//本BANK数据库的默认值	
	 0,										//本BANK数据库的默认值的大小
	 0x00,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},
	
	//BANK28 校准参数扩展
	{"bank28 exd expara",						//本BANK的名称
	 USER_PARA_PATH"bank28.cfg",             //本BANK数据保存的路径文件名
	 USER_BAK_PATH"bank28.cfg",				//备份文件的路径文件名，为空表示不备份，备份文件不支持多测量点多文件的备份，只支持单文件(比如只支持交采测量点0），不支持时标的备份,
	 g_Bank28Desc,					        //本BANK数据项描述表
	 sizeof(g_Bank28Desc)/sizeof(TItemDesc), //本BANK数据项描述表的数据项个数
	 g_bBank28Default,						//本BANK数据库的默认值	
	 sizeof(g_bBank28Default),				//本BANK数据库的默认值的大小
	 0x01,									//本BANK数据库的当前版本,0表示没有版本管理
	 1,										//本BANK数据的测量点个数
	 1,										//本BANK数据的镜像个数
	 false,									//本BANK数据是否需要更新时间
	},
};


TPnMapCtrl g_PnMapCtrl[PNMAP_NUM] = {	//测量点动态映射控制结构
//名义上支持测量点号----实际支持测量点数
	//{POINT_NUM,			MFM_NUM},		//集抄多功能表映射方案
	{POINT_NUM,			20},		//集抄重点户映射方案,
};


