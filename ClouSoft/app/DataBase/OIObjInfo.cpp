/*********************************************************************************************************
* Copyright (c) 2016,深圳科陆电子科技股份有限公司
* All rights reserved.
*
* 文件名称：OIObjInfo.h
* 摘    要：本文件主要实现所有数据OI数据的格式定义
* 当前版本：1.0
* 作    者：孔成波
* 完成日期：2016年9月
*********************************************************************************************************/
#include <fcntl.h>
#include <stdio.h>
#include "stdafx.h"
#include "OIObjInfo.h"
#include "DbConst.h"
#include "FaCfg.h"
#include "sysdebug.h"
#include "DbAPI.h"
#include "ComAPI.h"
#include "LibDbConst.h"
#include "TaskDB.h"
#include "SchParaCfg.h"
#include "CctSchMtr.h"
#include "TaskManager.h"
#include "DbOIAPI.h"
#include "sysfs.h"
#include "FrzTask.h"
#include "TransmitApi.h"
#include "Pulse.h"
#include "FileTran.h"
#include "LoadCtrl.h"
#include "MtrExc.h"
#include "MtrCtrl.h"
#include "CctAPI.h"
#include "Esam.h"
#include "StatMgr.h"
#include "ParaMgr.h"
#include "FaAPI.h"

#ifdef SYS_LINUX 
#include "segment_debug.h"
#else 
#define SYSTEM_DEBUG_STEP(x)  (0)
#endif

extern CStatMgr g_StatMgr;
BYTE g_bComEngDataFmt[3] = {0x01, 0x05, DT_DB_LONG};	//组合有功
BYTE g_bEngDataFmt[3] = {0x01, 0x05, DT_DB_LONG_U};
BYTE g_bComHiPreEngFmt[] = {0x01, 0x05, DT_LONG64};	//高精度组合电能量
BYTE g_bHiPreEngFmt[] = {0x01, 0x05, DT_LONG64_U};	//高精度电能量

BYTE g_bVoltDataFmt[3] = {0x01, 0x03, 0x12};
BYTE g_bCurDataFmt[3] = {0x01, 0x04, 0x05};
BYTE g_bPowerDataFmt[3] = {0x01, 0x04, 0x05};
BYTE g_bEngUnitFmt[] = {0x0f};
BYTE g_bMaxDemFmt[6] = {0x01,0x05,0x02,0x02,0x06,DT_DATE_TIME_S};
BYTE g_bComMaxDemFmt[] = {0x01,0x05,0x02,0x02,0x05,DT_DATE_TIME_S};
BYTE g_bCosDataFmt[3] = {0x01, 0x04, DT_LONG};
BYTE g_bFreqDataFmt[] = {0x01, 0x01, DT_LONG_U};
BYTE g_bVarDmdFmt[] = {DT_DB_LONG};							//需量
BYTE g_bPwrPrice[1] = {DT_DB_LONG_U};	//电价
BYTE g_bRtcVolFmt[] = {DT_LONG_U};
BYTE g_bMtrBlkRunStateFmt[5] = {DT_ARRAY, 0x07, DT_BIT_STR, 0x10,LRF};	//电表运行状态字
BYTE g_bMtrSubRunStateFmt[3] = {DT_BIT_STR, 0x10,LRF};	//电表运行状态字1~7

BYTE g_bVoltDistortionDataFmt[] = {0x01, 0x03, DT_LONG};		//电压波形失真度
BYTE g_bCurDistortionDataFmt[] = {0x01, 0x03, DT_LONG};			//电流波形失真度
BYTE g_bVoltHarPercentDataFmt[] = {0x01, HARMONIC_NUM-1, DT_LONG};		//电压谐波含量 总,2-21次
BYTE g_bCurHarPercentDataFmt[] = {0x01, HARMONIC_NUM-1, DT_LONG};			//电流谐波含量 总,2-21次

BYTE g_bUnsignedDataFmt[] = {0x01, 0x01, DT_UNSIGN};		//unsigned

//水气热表数据格式
BYTE g_b645ExtDataFmt[1] = {DT_DB_LONG_U};
BYTE g_b645ExtTempFmt[4] = {DT_STRUCT, 2, DT_DB_LONG_U, DT_DB_LONG_U};
BYTE g_b645ExtStaFmt[4] = {DT_STRUCT, 2, DT_ENUM, DT_ENUM};

// 基本格式
BYTE g_bUnSignTypeFmt[] = {DT_UNSIGN};						//unsigned
BYTE g_bBitStringTypeFmt[] = {DT_BIT_STR,0x08};				//BitString
BYTE g_bLongUTypeFmt[] = {DT_LONG_U};						//long-unsigned
BYTE g_bAhTypeFmt[] = {DT_ARRAY, 0x04, DT_DB_LONG_U};						//安时值


//资产管理编码
BYTE bAssetMagCode[] = {DT_VIS_STR, 32, RLV};

//额定电压 属性2（只读）∷=visible-string(SIZE(6))
BYTE gbUnString[] = {DT_VIS_STR, 6, RLV};
//额定电流/基本电流属性2（只读）∷=visible-string(SIZE(6))
BYTE gbIbString[] = {DT_VIS_STR, 6, RLV};
//最大电流属性2（只读）∷=visible-string(SIZE(6))
BYTE gbImaxString[] = {DT_VIS_STR, 6, RLV};
//有功准确度等级 属性2（只读）∷=visible-string(SIZE(4))
BYTE gbPClassString[] = {DT_VIS_STR, 4, RLV};
//无功准确度等级 属性2（只读）∷=visible-string(SIZE(4))
BYTE gbQClassString[] = {DT_VIS_STR, 4, RLV};
//电能表有功常数 属性2（只读）∷=double-long-unsigned
BYTE gbPImpConst[] = {DT_DB_LONG_U};
//电能表无功常数 属性2（只读）∷=double-long-unsigned
BYTE gbQImpConst[] = {DT_DB_LONG_U};
//电能表型号 属性2（只读）∷=visible-string(SIZE(32))
BYTE gbMeterTypeString[] = {DT_VIS_STR, 32, RLV};


//终端广播校时参数
BYTE bTermBroadTime[] = {DT_STRUCT, 2,
DT_TIME,	//终端广播校时启动时间
DT_BOOL,	//是否启用
};

//终端单地址广播校时参数
BYTE bSigAddrBroadTime[] = {DT_STRUCT, 3,
//DT_INT,	//时钟误差阈值 
DT_UNSIGN,	//时钟误差阈值 
DT_TIME,	//终端广播校时启动时间
DT_BOOL,	//是否启用
};


BYTE g_bEleDevDsc[] = {DT_VIS_STR, 0x10,RLV,};	//电气设备——设备描述符
BYTE g_bEleVerInfo[] = {DT_STRUCT, 0x06, 
DT_VIS_STR, 0x04,RLF, 
DT_VIS_STR, 0x04,RLF, 
DT_VIS_STR, 0x06,RLF, 
DT_VIS_STR, 0x04,RLF, 
DT_VIS_STR, 0x06,RLF, 
DT_VIS_STR, 0x08,RLF};	//电气设备——版本信息
BYTE g_bManufactureDate[] = {DT_DATE_TIME_S};	//电气设备——生产日期
BYTE g_bMastProType[] = {DT_ARRAY, 0x01,
DT_VIS_STR, 0x09, RLV,};	//电气设备——协议规约类型
BYTE g_bRptFlowFlg[] = {DT_BOOL};	//电气设备——允许跟随上报
BYTE g_bRptFlg[] = {DT_BOOL};	//电气设备——允许主动上报
BYTE g_bMastCall[] = {DT_BOOL};	//电气设备——允许与主站通话
BYTE g_bMasRptCn[] = {DT_ARRAY, CN_RPT_NUM, DT_OAD};	//电气设备——上报通道，hyl目前0x430a的空间是按CN_RPT_NUM开的，为17个字节

BYTE g_bSimCCID[] = {DT_VIS_STR, 0x14, RLV};	//SIM卡的ICCID
BYTE g_bIMSI[] = {DT_VIS_STR, 0x0F, RLV};	//IMSI
BYTE g_bSigStrenth[] = {DT_LONG};	//信号强度
BYTE g_bSimNo[] = {DT_VIS_STR, 0x10, RLV};	//SIM卡号码
BYTE g_DialIp[] = {DT_OCT_STR, 0x04, RLF};	//拨号IP

BYTE g_bAdjTimeModeFmt[] = {DT_ENUM};	//日期时间，属性3，校时模式
BYTE g_bPreAdjParaFmt[] = {DT_STRUCT, 0x05,
DT_UNSIGN, 
DT_UNSIGN, 
DT_UNSIGN, 
DT_UNSIGN, 
DT_UNSIGN,};//日期时间，属性4，精准校时参数	

// 事件相关格式
BYTE g_bEvtDmdFmt[] = {DT_DB_LONG};							//需量
BYTE g_bEvtTimeFmt[] = {DT_DATE_TIME_S};							//事件时间类型date_time_s
BYTE g_bEvtIndexFmt[] = {DT_DB_LONG_U};							//事件序号类型double-long-unsigned
BYTE g_bEvtCurValFmt[] = {DT_STRUCT, 0x02, DT_DB_LONG_U, DT_DB_LONG_U};					//事件当前值类型structure
BYTE g_bEvtChnRptFmt[] = {DT_STRUCT, 0x02, DT_OAD, DT_ENUM};				//通道上报状态structure
BYTE g_bEvtCapOADFmt[] = {DT_ARRAY, CAP_OAD_NUM, DT_OAD};		//关联对象属性表
BYTE g_bEvtMaxNumFmt[] = {DT_LONG_U};						//最大记录数
BYTE g_bEvtRptFlagFmt[] = {DT_ENUM};						//上报标识
BYTE g_bEvtValidFlagFmt[] = {DT_BOOL};						//有效标识
BYTE g_bEvtIc7RecNumFmt[] = {DT_LONG_U};					//当前记录数,IC7
BYTE g_bEvtIc7ValNumFmt[] = {DT_ARRAY, 0x01, DT_STRUCT, 0x02, DT_NULL, DT_STRUCT, 0x02, DT_DB_LONG_U, DT_DB_LONG_U};		//当前值记录表,IC7,无事件发生源
BYTE g_bEvtIc7ValEnumFmt[] = {DT_ARRAY, 0x01, DT_STRUCT, 0x02, DT_ENUM, DT_STRUCT, 0x02, DT_DB_LONG_U, DT_DB_LONG_U};		//当前值记录表,IC7,事件发生源ENUM
BYTE g_bEvtIc7ValOadFmt[] = {DT_ARRAY, 0x01, DT_STRUCT, 0x02, DT_OAD, DT_STRUCT, 0x02, DT_DB_LONG_U, DT_DB_LONG_U};		//当前值记录表,IC7,事件发生源OAD
BYTE g_bEvtIc7ValOiFmt[] = {DT_ARRAY, 0x01, DT_STRUCT, 0x02, DT_OI, DT_STRUCT, 0x02, DT_DB_LONG_U, DT_DB_LONG_U};		//当前值记录表,IC7,事件发生源OI
BYTE g_bEvtIc7ValTsaFmt[] = {DT_ARRAY, 0x01, DT_STRUCT, 0x02, DT_TSA, DT_STRUCT, 0x02, DT_DB_LONG_U, DT_DB_LONG_U};		//当前值记录表,IC7,事件发生源TSA
BYTE g_bEvtIc24RecNumFmt[] = {DT_STRUCT, 0x04, DT_LONG_U, DT_LONG_U, DT_LONG_U, DT_LONG_U};			//当前记录数,IC24
BYTE g_bEvtIc24ValNumFmt[] = {DT_STRUCT, 0x04, DT_STRUCT, 0x02, DT_DB_LONG_U, DT_DB_LONG_U, DT_STRUCT, 0x02, DT_DB_LONG_U, DT_DB_LONG_U, DT_STRUCT, 0x02, DT_DB_LONG_U, DT_DB_LONG_U, DT_STRUCT, 0x02, DT_DB_LONG_U, DT_DB_LONG_U};		//当前值记录表,IC24
BYTE g_bEvtVLostStaFmt[] = {DT_STRUCT, 0x04, DT_DB_LONG_U, DT_DB_LONG_U, DT_DATE_TIME_S, DT_DATE_TIME_S};			//失压统计
BYTE g_bEvtVlostParaFmt[] = {DT_STRUCT, 0x04, DT_LONG_U, DT_LONG_U, DT_DB_LONG, DT_UNSIGN};		//配置参数，失压					1
BYTE g_bEvtVUnderParaFmt[] = {DT_STRUCT, 0x02, DT_LONG_U, DT_UNSIGN};			//配置参数，欠压					2
BYTE g_bEvtVOverParaFmt[] = {DT_STRUCT, 0x02, DT_LONG_U, DT_UNSIGN};			//配置参数，过压					3
BYTE g_bEvtVBreakParaFmt[] = {DT_STRUCT, 0x03, DT_LONG_U, DT_DB_LONG, DT_UNSIGN};			//配置参数，断相					4
BYTE g_bEvtILostParaFmt[] = {DT_STRUCT, 0x04, DT_LONG_U, DT_DB_LONG, DT_DB_LONG, DT_UNSIGN};		//配置参数，失流					5
BYTE g_bEvtIOverParaFmt[] = {DT_STRUCT, 0x02,  DT_DB_LONG, DT_UNSIGN};				//配置参数，过流					6
BYTE g_bEvtIBreakParaFmt[] = {DT_STRUCT, 0x03, DT_LONG_U,  DT_DB_LONG, DT_UNSIGN};		//配置参数，断流					7
BYTE g_bEvtPReverseParaFmt[] = {DT_STRUCT, 0x02, DT_DB_LONG, DT_UNSIGN};			//配置参数，潮流反向				8
BYTE g_bEvtPOverParaFmt[] = {DT_STRUCT, 0x02, DT_DB_LONG, DT_UNSIGN};				//配置参数，过载					9
BYTE g_bEvtPDmdOverParaFmt[] = {DT_STRUCT, 0x02, DT_DB_LONG_U, DT_UNSIGN};			//配置参数，正向有功需量超限		10
BYTE g_bEvtRPDmdOverParaFmt[] = {DT_STRUCT, 0x02, DT_DB_LONG_U, DT_UNSIGN};		//配置参数，反向有功需量超限		11
BYTE g_bEvtQDmdOverParaFmt[] = {DT_STRUCT, 0x02, DT_DB_LONG_U, DT_UNSIGN};			//配置参数，无功需量超限			12
BYTE g_bEvtPfUnderParaFmt[] = {DT_STRUCT, 0x02, DT_LONG, DT_UNSIGN};			//配置参数，功率因数超下限			13
BYTE g_bEvtAllVlostParaFmt[] = {NULL};					//配置参数，全失压					14
BYTE g_bEvtVDisorderParaFmt[] = {DT_STRUCT, 0x01, DT_UNSIGN};			//配置参数，电压逆相序				15
BYTE g_bEvtIDisorderParaFmt[] = {DT_STRUCT, 0x01, DT_UNSIGN};			//配置参数，电流逆相序				16
BYTE g_bEvtMeterClrParaFmt[] = {NULL};					//配置参数，清零					17
BYTE g_bEvtDmdClrParaFmt[] = {NULL};					//配置参数，需量清零				18
BYTE g_bEvtEventClrParaFmt[] ={NULL};					//配置参数，事件清零				19
BYTE g_bEvtVUnbalanceParaFmt[] = {DT_STRUCT, 0x02, DT_LONG, DT_UNSIGN};		//配置参数，电压不平衡				20
BYTE g_bEvtIUnbalanceParaFmt[] = {DT_STRUCT, 0x02, DT_LONG, DT_UNSIGN};		//配置参数，电流不平衡				21
BYTE g_bEvtISUnbalanceParaFmt[] = {DT_STRUCT, 0x02, DT_LONG, DT_UNSIGN};		//配置参数，电流严重不平衡			22
BYTE g_bEvtClockErrParaFmt[] = {DT_STRUCT, 0x01, DT_UNSIGN};				//配置参数，时钟故障				23
BYTE g_bEvtMtrChipErrParaFmt[] = {DT_STRUCT, 0x01, DT_UNSIGN};			//配置参数，计量芯片故障			24
BYTE g_bEvtMaxDmdFmt[] = {DT_DB_LONG_U};			//超限期间正向有功需量最大值  double-long-unsigned
BYTE g_bEvtMaxDmdDateFmt[] = {DT_DATE_TIME_S};			//超限期间需量最大值发生时间  date_time_s
BYTE g_bEvtClearrOMDFmt[] = {DT_ARRAY, EVT_CLR_OMD_NUM, DT_OMD};			//超限期间需量最大值发生时间  date_time_s
BYTE g_bChnRptStateFmt[] = { DT_ARRAY, CN_RPT_NUM,
DT_STRUCT, 0x02, 
DT_OAD, 
DT_UNSIGN,
};
BYTE g_bAddEvtOadFmt[] = {DT_ARRAY, EVT_ADDOAD_MAXNUM,DT_OAD};
BYTE g_bAddEvtOIFmt[] = {DT_ARRAY, EVT_ADDOAD_MAXNUM,DT_OI};

BYTE g_bEvtTermPrgOADFmt[] = {DT_ARRAY, TERM_PRG_OAD_NUM, DT_OAD};			

//单笔记录搜表结果格式
BYTE g_bSchMtrRltPerRecFmt[] = {DT_ARRAY, SCH_MTR_SAVE_REC_NUM,
DT_TSA,	//通信地址
DT_TSA,	//所属采集器地址
DT_ENUM,	//规约类型
DT_ENUM,	//相位
DT_UNSIGN,	//信号品质 
DT_DATE_TIME_S,	//搜到的时间
DT_SCH_MTR_ANNEX,	//搜到的附加信息
};


BYTE g_bEvtStepAreaFmt[] = {DT_ARRAY, TERM_PRG_OAD_NUM, DT_TSA, DT_TSA, DT_DATE_TIME_S};	
BYTE g_bEvtLong64Fmt[] = {DT_LONG64};		
BYTE g_bEvtOIFmt[] = {DT_OI};			
BYTE g_bEvtBitStrFmt[] = {DT_BIT_STR, 0x08, RLF};		
BYTE g_bEvtIntFmt[] = {DT_INT};		
BYTE g_bEvtYKCtrlPEFmt[] = {DT_ARRAY, 8, DT_LONG64};		
BYTE g_bEvtMtrClkTimeFmt[] = {DT_DATE_TIME_S};		
BYTE g_bEvtMtrClkErrFmt[] = {DT_INT};		


BYTE g_bEvtYXParamFmt[] = {DT_ARRAY, 0x10,
DT_STRUCT, 0x04,	//备注：结构体中的成员个数需要更改
DT_LONG_U,	//事件记录序号
DT_DATE_TIME_S,	//事件发生时间
DT_DATE_TIME_S,	//事件结束时间
//无事件发生源
DT_ARRAY, 0x04,	//通道上报状态
DT_OAD,	
//具体的数据内容再添加
};	//终端状态量变位事件 属性2

BYTE g_bMtrExcIc7ValNumFmt[] = {DT_ARRAY, 0x01, 
DT_STRUCT, 0x02, 
#ifdef MTREXC_ADDR_TPYE_TSA
DT_TSA,
#else
DT_OCT_STR, TSA_LEN, RLV,//需要读取停电事件电能表---70	RLV		
#endif	
DT_STRUCT, 0x02, DT_DB_LONG_U, DT_DB_LONG_U};		//当前值记录表,IC7,有事件发生源

#ifdef GW_OOB_DEBUG_0x31050600
BYTE g_bMtrExcClockkErrFmt[] = {DT_STRUCT, 0x01, DT_LONG_U};//电能表时钟超差事件 属性6，配置参数
#else
BYTE g_bMtrExcClockkErrFmt[] = {DT_STRUCT, 0x02, DT_LONG_U, DT_UNSIGN};//电能表时钟超差事件 属性6，配置参数
#endif

BYTE g_bEvtTermPwrOffFmt[] = {DT_STRUCT, 0x02, 
DT_STRUCT, 0x04,	//停电数据采集配置参数	--18
DT_BIT_STR, 0x08, RLV,	//采集标志
DT_UNSIGN,	//停电事件抄读时间间隔（小时）
DT_UNSIGN,	//停电事件抄读时间限值（分钟）
DT_ARRAY, SAMPLE_MTR_NUM,	
#ifdef MTREXC_ADDR_TPYE_TSA
DT_TSA,
#else
DT_OCT_STR, TSA_LEN, RLV,//需要读取停电事件电能表---70	RLV		
#endif
DT_STRUCT, 0x06,	//停电事件甄别限值参数---------20
DT_LONG_U,	//停电时间最小有效间隔(分钟) 
DT_LONG_U,	//停电时间最大有效间隔(分钟)
DT_LONG_U,	//停电事件起止时间偏差限值(分钟)
DT_LONG_U,	//停电事件时间区段偏差限值(分钟) 
DT_LONG_U,	//停电发生电压限值 
DT_LONG_U,	//停电恢复电压限值
};	//终端停上电参数，属性6


BYTE g_bMtrExcEnergyDecFmt[] = { DT_STRUCT, 0x01, DT_UNSIGN };	//电能表示度下降，属性6
BYTE g_bMtrExcEnergyErrFmt[] = { DT_STRUCT, 
#ifdef GW_OOB_DEBUG_0x310C0600
0x01,DT_DB_LONG_U,
#else
0x02, DT_DB_LONG_U, DT_UNSIGN 
#endif
};  //电能量超差事件
BYTE g_bMtrExcEnergyFlowFmt[] = { DT_STRUCT, 
#ifdef GW_OOB_DEBUG_0x310D0600
0x01, DT_DB_LONG_U
#else
0x02, DT_DB_LONG_U, DT_UNSIGN 
#endif
};  //电能量飞走事件
BYTE g_bMtrExcEnergyStopFmt[] = { DT_STRUCT, 
#ifdef GW_OOB_DEBUG_0x310E0600
0x01, DT_TI
#else
0x02, DT_TI, DT_UNSIGN 
#endif
};  //电能量停走事件
BYTE g_bMtrExcMtrRdFailFmt[] = { DT_STRUCT, 
#ifdef GW_OOB_DEBUG_0x310F0600
0x01, DT_UNSIGN
#else
0x02, DT_UNSIGN, DT_UNSIGN 
#endif
};  //电能抄表失败事件
BYTE g_bEvtMthFluOverFmt[] = {DT_STRUCT, 0x01, DT_DB_LONG_U,};	//月通信流量超限事件

BYTE g_bMtrDataChgFmt[] = { DT_STRUCT, 0x01, DT_UNSIGN };	//电能表数据变更监控记录，属性6 配置参数


// 冻结相关格式
BYTE g_bFrzParaFmt[] = { DT_ARRAY, CAP_OAD_NUM,	//数组及元素个数
DT_STRUCT,0x03,		//结构及成员个数
DT_LONG_U,		//周期 long-unsigned
DT_OAD,		//OAD
DT_LONG_U,		//深度 long-unsigned
};
BYTE g_bFrzRecIndexFmt[] = {0x6};
BYTE g_bFrzRecTimeFmt[1] = {0x1c};

BYTE g_bPnParaFmt[] = {DT_STRUCT, 0x04, 	//添加一个采集档案配置单元
DT_LONG_U, 
DT_STRUCT, 0x0a, 
DT_TSA, //16, RLV, //TSA
DT_ENUM, //emu-----波特率
DT_ENUM, //emu-----规约类型
DT_OAD, //OAD----端口
DT_OCT_STR, 32, RLV, //密码，变长，最长32字节
DT_UNSIGN, //费率个数
DT_UNSIGN, //---------用户类型
DT_ENUM, //emu----接线方式
DT_LONG_U,	//额定电压
DT_LONG_U,	//额定电流
DT_STRUCT, 0x04, 
DT_TSA, //16, RLV, //TSA
DT_OCT_STR, 16, RLV, //octing
DT_LONG_U, //PT
DT_LONG_U, //CT
#if 0
DT_ARRAY, MAX_MTR_ANNEX,
DT_STRUCT, 0x02, //结构体
DT_OAD, //OAD
128, 100, RLV, 	//-----Data里面嵌套data，扩展一个数据类型128，做data
#else 
DT_MTR_ANNEX,
#endif
};

//搜表时长
BYTE g_bSchMtrTimeFmt[] = {DT_LONG_U};

//无线公网通讯接口配置参数
BYTE g_bGprsCommCfg[] = {DT_STRUCT, 0x0C, 
DT_ENUM, 		//工作模式
DT_ENUM, 	//在线方式
DT_ENUM, 	//连接方式
DT_ENUM, 	//连接应用方式
DT_ARRAY, 8, //帧听端口列表
DT_LONG_U,
DT_VIS_STR, 32, RLV,  //APN,最大32字节
DT_VIS_STR, 32, RLV, //用户名,最大32字节
DT_VIS_STR, 32, RLV, //密码,最大32字节
DT_OCT_STR, 4,	RLF,//代理服务器地址
DT_LONG_U, 		//代理端口
DT_UNSIGN,  //超时时间及重发次数
DT_LONG_U, 		//心跳周期
};

//以太网通信参数
BYTE g_bEthCommCfg[] = {DT_STRUCT, 0x08, 
DT_ENUM, 		//工作模式
DT_ENUM, 	//连接方式
DT_ENUM, 	//连接应用方式
DT_ARRAY, 8, //帧听端口列表
DT_LONG_U,
DT_OCT_STR, 4,	RLF,//代理服务器地址
DT_LONG_U, 		//代理端口
DT_UNSIGN,  //超时时间及重发次数
DT_LONG_U, 		//心跳周期
};

//以太网网络配置
BYTE g_bEthNetCfg[] = {DT_STRUCT, 0x06,
DT_ENUM,	//IP配置方式 DHCP（0）,静态（1），PPPoE（2）
DT_OCT_STR, 0x08, RLV,	//IP地址
DT_OCT_STR, 0x08, RLV,	//子网掩码
DT_OCT_STR, 0x08, RLV,	//网关地址
DT_VIS_STR, 0x20, RLV,	//PPPoE用户名
DT_VIS_STR, 0x20, RLV,	//PPPoE密码
};

//MAC地址
BYTE g_bEthMacCfg[] = {DT_MAC, 0x06, RLV};	


BYTE g_bMastCommPara[] = {DT_ARRAY, MAX_MAINIP_NUM,	//最多2个主站地址
DT_STRUCT, 0x02,
DT_OCT_STR, 4, RLF,	//IP地址
DT_LONG_U,	//端口 
};
BYTE g_bSmsCommPara[] = {DT_STRUCT, 0x03, 
DT_VIS_STR, 16, RLV, 	//短信中心号码
DT_ARRAY, MAX_SMS_MAIN_NUM,
DT_VIS_STR, 16, RLV, 	//主站号码
DT_ARRAY, MAX_SMS_SEND_NUM, 
DT_VIS_STR, 16, RLV,	//短信通知目的号码
};
BYTE g_bVerInfo[] = {0x02, 0x06,
10, 4,	//厂商代码
10, 4,	//软件版本号
10, 6,	//软件版本日期
10, 4,	//硬件版本号
10, 6,	//硬件版本日期
10, 8,	//厂家扩展信息
};

//服务器地址SA
BYTE g_bServAddrFmt[3] = {DT_OCT_STR, 0x10, RLV};  

//（当前）钱包文件
BYTE g_bCurWalletFile[] = {DT_STRUCT, 0x02,
DT_DB_LONG_U,
DT_DB_LONG_U,
};

//（当前）透支金额
BYTE g_bCurOverRate[] = {DT_DB_LONG_U};

//累计购电金额
BYTE g_bPurchaseRate[1] = {DT_DB_LONG_U};

//表号
BYTE g_bMtrAddr[] = {DT_OCT_STR, 0x10, RLV}; 

//客户编号
BYTE g_bCliCode[] = {DT_OCT_STR, 0x10, RLV};

//设备地理位置
BYTE g_bDevLocat[] = {DT_STRUCT, 3,
DT_STRUCT, 4,	//经度
DT_ENUM ,	//方位
DT_UNSIGN,	//度
DT_UNSIGN,	//分
DT_UNSIGN,	//秒
DT_STRUCT, 4,	//纬度
DT_ENUM ,	//方位
DT_UNSIGN,	//度
DT_UNSIGN,	//分
DT_UNSIGN,	//秒
DT_DB_LONG_U,	//高度（cm）
};

//组地址
BYTE g_bGroupAddr[] = {DT_ARRAY, 0x0a,
DT_OCT_STR, TSA_LEN, RLV,
};

//时钟源
BYTE g_bClkSrc[] = {DT_STRUCT, 2,
DT_ENUM,	//时钟源
DT_ENUM,	//状态
};

//LCD参数
BYTE g_bDispParam[] = {DT_STRUCT, 7,
DT_UNSIGN,	//上电全显时间
DT_LONG_U,	//背光点亮时间 
DT_LONG_U,	//显示查看背光点亮时间 
DT_LONG_U,	//有电按键屏幕驻留最大时间
DT_UNSIGN,	//无电按键屏幕驻留最大时间
DT_UNSIGN,	//显示电能小数位数
DT_UNSIGN,	//显示功率（最大需量）小数位数
};


//时区时段数
BYTE g_bTimeZoneDayChartParam[] = {DT_STRUCT, 5,
DT_UNSIGN,	//年时区数(p≤14)
DT_UNSIGN,	//日时段表数（q≤8） 
DT_UNSIGN,	//日时段数(每日切换数)（m≤14）
DT_UNSIGN,	//费率数（k≤63）
DT_UNSIGN,	//公共假日数（n≤254）
};

//两套时区表切换时间
BYTE g_bTimeZoneSwitchTimeParam[] = {DT_DATE_TIME_S};
//两套时段表切换时间
BYTE g_bDayChartSwitchTimeParam[] = {DT_DATE_TIME_S};

//公共假日表
BYTE g_bHolidayParam[] = {DT_ARRAY, MAX_HOLIDAY_NUM,
DT_STRUCT, 2,
DT_DATE,	//日期
DT_UNSIGN,	//日时段表号
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
DT_STRUCT,	2, DT_DATE,	DT_UNSIGN,
};

//周休日特征字
BYTE g_bRestStatusParam[] = {DT_BIT_STR,
8,	
};
//周休日釆用的日时段表号
BYTE g_bRestDayChartParam[] = {DT_UNSIGN};

//当前/备用套时区表
BYTE g_bZoneNumParam[] = {DT_ARRAY, MAX_ZONE_NUM,
DT_STRUCT, 3,
DT_UNSIGN,	//月
DT_UNSIGN,	//日 
DT_UNSIGN,	//日时段表号
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
};
//当前/备用套日时段表
BYTE g_bDayChartParam[] = {DT_ARRAY, MAX_DAY_CHART_NUM,
DT_ARRAY, RATE_PERIOD_NUM,
DT_STRUCT, 3,
DT_UNSIGN,	//时
DT_UNSIGN,	//分 
DT_UNSIGN,	//费率号
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,							
DT_ARRAY, RATE_PERIOD_NUM,
DT_STRUCT, 3,
DT_UNSIGN,	//时
DT_UNSIGN,	//分 
DT_UNSIGN,	//费率号
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,							
DT_ARRAY, RATE_PERIOD_NUM,
DT_STRUCT, 3,
DT_UNSIGN,	//时
DT_UNSIGN,	//分 
DT_UNSIGN,	//费率号
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,							
DT_ARRAY, RATE_PERIOD_NUM,
DT_STRUCT, 3,
DT_UNSIGN,	//时
DT_UNSIGN,	//分 
DT_UNSIGN,	//费率号
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,							
DT_ARRAY, RATE_PERIOD_NUM,
DT_STRUCT, 3,
DT_UNSIGN,	//时
DT_UNSIGN,	//分 
DT_UNSIGN,	//费率号
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,							
DT_ARRAY, RATE_PERIOD_NUM,
DT_STRUCT, 3,
DT_UNSIGN,	//时
DT_UNSIGN,	//分 
DT_UNSIGN,	//费率号
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,							
DT_ARRAY, RATE_PERIOD_NUM,
DT_STRUCT, 3,
DT_UNSIGN,	//时
DT_UNSIGN,	//分 
DT_UNSIGN,	//费率号
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,							
DT_ARRAY, RATE_PERIOD_NUM,
DT_STRUCT, 3,
DT_UNSIGN,	//时
DT_UNSIGN,	//分 
DT_UNSIGN,	//费率号
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,
DT_STRUCT,3,DT_UNSIGN,DT_UNSIGN,DT_UNSIGN,							
};


//无线公网属性2：通信配置
BYTE g_bGprs2Fmt[] = {0x02, 0x0C, 
0x16,      //工作模式
0x16,      //在线方式
0x16,      //连接方式
0x16,      //连接应用方式
0x12,      //侦听端口列表
0x0A, 0x20,//APN
0x0A, 0x20,//用户名
0x0A, 0x20,//密码
0x09, 0x04,//代理服务器地址
0x12,      //代理端口
0x04,      //超时时间及重发次数
0x12};     //心跳周期

//无线公网属性3：主站通信参数表
BYTE g_bGprs3Fmt[] = {0x01, 0x01, 0x02, 0x02, 
0x09, 0x04, //主站IP地址，长度为4
0x12};      //主站端口            
//无线公网属性4：短信通信参数
BYTE g_bGprs4Fmt[] = {0x02, 0x03, 
0x0a, 0x10,  //短信中心号码
0x0a, 0x10,  //主站号码
0x0a, 0x10}; //短信通知目的号码   

//以太网属性2：通信配置
BYTE g_bEth2Fmt[] = {0x02, 0x08, 
0x16,		//工作模式
0x16,		//连接方式
0x16,		//连接应用方式
0x12,		//侦听端口列表
0x09, 0x04,  //代理服务器地址 ,长度为4
0x12,		//代理端口
0x04,		//超时时间及重发次数
0x12};		//心跳周期	

//以太网属性3：主站通信参数表
BYTE g_bEth3Fmt[] = {0x01, 0x01, 
0x02, 0x02, 
0x09, 0x04,    //主站IP地址，长度为4
0x12};         //主站端口    

//以太网属性4：网络配置
BYTE g_bEth4Fmt[] = {0x02, 0x06, 
0x16,       //IP配置方式
0x09, 0x04, //IP地址
0x09, 0x04, //子网掩码
0x09, 0x04, //网关地址
0x09, 0x20, //PPPoE用户名
0x09, 0x20};//PPPoE密码 

//采集任务监控单元
BYTE g_bTaskMoniUnit[] = {DT_STRUCT, 0x08,
DT_UNSIGN,	//任务ID
DT_ENUM,	//任务执行状态
DT_DATE_TIME_S,	//任务执行开始时间 
DT_DATE_TIME_S,	//任务执行结束时间 
DT_LONG_U,	//采集总数量
DT_LONG_U,	//采集成功数量 
DT_LONG_U,	//已发送报文条数
DT_LONG_U,	//已接收报文条数 
} ;

/////////////////////////////////////////////////////////////////
//控制参数和数据格式描述
BYTE g_bYkParaFmt[] = {DT_STRUCT, 2,
DT_DB_LONG_U,
DT_LONG_U};
BYTE g_bCtrlStaFmt[] = {DT_BIT_STR, 1, RLF};

BYTE g_bGuarant02Fmt[] = {DT_ENUM};
BYTE g_bGuarant03Fmt[] = {DT_LONG_U};
BYTE g_bGuarant05Fmt[] = {DT_ARRAY, 24,
DT_STRUCT, 2,
DT_UNSIGN, DT_UNSIGN};

BYTE g_bChineseInfoFmt[] = {DT_ARRAY, GB_MAXCOMCHNNOTE,  
DT_STRUCT, 0x04,
DT_UNSIGN,	//序号
DT_DATE_TIME_S,	//发布时间
DT_BOOL,	//已阅读标识
DT_VIS_STR, 200, RLV 	//信息内容
};

BYTE g_bSafeLimitFmt[] = {DT_LONG64}; //保安定值
BYTE g_bCtrlPeriodFmt[] = {DT_ARRAY, 12,
DT_UNSIGN,							
}; //控制时段
BYTE g_bTurnAlrTimeFmt[] = {DT_ARRAY, 8,
DT_UNSIGN,							
}; //轮次告警时间
BYTE g_bPeriodCtrlParaFmt[] = {
	DT_ARRAY, 8,
	DT_STRUCT, 6,
	DT_OI,	//总加组对象
	DT_BIT_STR,	1, RLF, //方案标识
	DT_STRUCT, 9, //第一套定值
	DT_BIT_STR,	1, RLF, //时段号
	DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, //1~8个时段功控定值
	DT_STRUCT, 9, //第二套定值
	DT_BIT_STR,	1, RLF, //时段号
	DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, //1~8个时段功控定值
	DT_STRUCT, 9, //第三套定值
	DT_BIT_STR,	1, RLF, //时段号
	DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, //1~8个时段功控定值
	DT_INT, //时段功控定值浮动系数
}; //时段功控配置方案集
BYTE g_bRestCtrlParaFmt[] = {
	DT_ARRAY, 8,
	DT_STRUCT, 5,
	DT_OI,	//总加组对象
	DT_LONG64, //厂休控定值
	DT_DATE_TIME_S, //限电起始时间
	DT_LONG_U, //限电延续时间
	DT_BIT_STR,	1, RLF, //每周限电日
}; //厂休控配置方案集
BYTE g_bShutoutCtrlParaFmt[] = {
	DT_ARRAY, 8,
	DT_STRUCT, 4,
	DT_OI,	//总加组对象
	DT_DATE_TIME_S, //报停起始时间
	DT_DATE_TIME_S, //报停结束时间
	DT_LONG64, //报停控功率定值
}; //营业报停控配置方案集
BYTE g_bBuyCtrlParaFmt[] = {
	DT_ARRAY, 8,
	DT_STRUCT, 8,
	DT_OI,	//总加组对象
	DT_DB_LONG_U, //购电单号
	DT_ENUM, //追加/刷新标识
	DT_ENUM, //购电类型
	DT_LONG64, //购电量（费）值
	DT_LONG64, //报警门限值
	DT_LONG64, //跳闸门限值
	DT_ENUM, //购电控模式
}; //购电控配置方案集
BYTE g_bMonthCtrlParaFmt[] = {
	DT_ARRAY, 8,
	DT_STRUCT, 4,
	DT_OI,	//总加组对象
	DT_LONG64, //月电量控定值
	DT_UNSIGN, //报警门限值系数
	DT_INT, //月电量控定值浮动系数
}; //月电控配置方案集
BYTE g_bCtrlInputStatusFmt[] = {
	DT_ARRAY, 8,
	DT_STRUCT, 2,
	DT_OI,	//总加组对象
	DT_ENUM, //投入状态
}; //控制投入状态
BYTE g_bCtrlOutputStatusFmt[] = {
	DT_ARRAY, 8,
	DT_STRUCT, 2,
	DT_OI,	//总加组对象
	DT_BIT_STR, 1, RLF, //输出状态
}; //控制输出状态
BYTE g_bCtrlAlrStatusFmt[] = {
	DT_ARRAY, 8,
	DT_STRUCT, 2,
	DT_OI,	//总加组对象
	DT_ENUM, //告警状态
}; //越限告警状态

////ESAM////////////////////////////////////////////////////
BYTE g_bEsamSerialNumFmt[] = 			{DT_OCT_STR, 0x10, RLV}; // 18
BYTE g_bEsamVerisonFmt[] = 				{DT_OCT_STR, 0x10, RLV}; // 18
BYTE g_bEsamKeyVersionFmt[] = 			{DT_OCT_STR, 0x20, RLV}; // 34
BYTE g_bEsamCerVersionFmt[] = 			{DT_STRUCT, 2,
DT_OCT_STR, 0x10, RLV,
DT_OCT_STR, 0x10, RLV,};  // 38
BYTE g_bEsamSessionMaxTimeFmt[] = 		{DT_DB_LONG_U}; // 5
BYTE g_bEsamSessionRemainTimeFmt[] =	{DT_DB_LONG_U}; // 5
BYTE g_bEsamCounterFmt[] = 				{DT_STRUCT, 3,
DT_DB_LONG_U,
DT_DB_LONG_U,
DT_DB_LONG_U}; // 17
BYTE g_bEsamTermCerSerNumFmt[] = 		{DT_OCT_STR, 0x20, RLV}; // 34
BYTE g_bEsamMSCerSerNumFmt[] = 			{DT_OCT_STR, 0x20, RLV}; // 34
BYTE g_bEsamTermCertificateFmt[] = 		{DT_OCT_STR, 0x82, 0x08, 0x00, RLV}; // 2052
BYTE g_bEsamMSCertificateFmt[] = 		{DT_OCT_STR, 0x82, 0x08, 0x00, RLV}; // 2052
BYTE g_bEsamSercureModeChoiceFmt[] = 	{DT_ENUM};  // 1
BYTE g_bEsamSercureModeParamFmt[] = 	{DT_ARRAY, 0x10, //2+(8)*16=130
DT_STRUCT, 2, // 2
DT_OI, //1+2
DT_LONG_U}; //1+2

//RS232端口
BYTE g_bRS232PortFmt[] = {DT_ARRAY, MAX_232_PORT_NUM, 
DT_STRUCT, 0x03,
DT_VIS_STR,	0x10,RLV,
DT_COMDCB,
DT_ENUM,
};

//RS485端口
BYTE g_bRS485PortFmt[] = {DT_ARRAY, MAX_485_PORT_NUM, 
DT_STRUCT, 0x03,
DT_VIS_STR,	0x10,RLV,
DT_COMDCB,
DT_ENUM,
};

//红外端口
BYTE g_bInfraPortFmt[] = {DT_ARRAY, MAX_HW_PORT_NUM,
DT_STRUCT, 0x02,
DT_VIS_STR,	0x10,RLV,
DT_COMDCB,
};

//载波/微功率无线接口端口
BYTE g_bPlcPortFmt[] = {DT_ARRAY, MAX_PLC_PORT_NUM,
DT_STRUCT, 0x03,
DT_VIS_STR,	0x10,RLV,
DT_COMDCB,
DT_STRUCT,  0x04,
DT_VIS_STR,	0x02,RLF,
DT_VIS_STR,	0x02,RLF,
DT_DATE,
DT_LONG_U,
};


//开关量属性2
BYTE g_bSwitchInput2Fmt[] = {DT_ARRAY, MAX_SW_PORT_NUM,
DT_STRUCT, 0x02, 
DT_UNSIGN, //状态ST
DT_UNSIGN,//变位CD
};

//开关量属性4
BYTE g_bSwitchInput4Fmt[] = {DT_STRUCT, 0x02, 
DT_BIT_STR, 8, RLV, //开关量接入标志
DT_BIT_STR, 8, RLV,	//开关量属性标志 
};

//继电器输出属性2
BYTE g_bRelayOutput2Fmt[] = {DT_ARRAY, MAX_RLY_PORT_NUM,
DT_STRUCT, 0x04,
DT_VIS_STR, 0x10,RLV,	//端口描述符
DT_ENUM,	//当前状态 
DT_ENUM,	//开关属性 
DT_ENUM,	//接线状态
};

//告警输出属性2
BYTE g_bAlarmOutput2Fmt[] = {DT_ARRAY, MAX_ALRM_PORT_NUM,DT_ENUM};//告警输出

//告警输出属性4
BYTE g_bAlarmOutput4Fmt[] = {DT_ARRAY, MAX_ALRM_PORT_NUM,
DT_STRUCT, 0x02,
DT_TIME,	//起始时间
DT_TIME,	//结束时间
};

//多功能端子属性2
BYTE g_bMulFunction2Fmt[] = {DT_ARRAY, MAX_MUL_PORT_NUM,DT_ENUM};//功能

//交采接口属性2
BYTE g_bACInterface2Fmt[] = {DT_ARRAY, 1,
DT_STRUCT, 0x01,
DT_VIS_STR, 0x10,RLV,	//交采描述符
};

//脉冲输入设备属性2
BYTE g_bPulseInput2Fmt[] = {DT_ARRAY, MAX_PLUS_PORT_NUM,
DT_VIS_STR, 0x10,RLV,
};


//显示接口类属性2，显示对象列表
BYTE g_bDisplay2Fmt[] = {0x01, 0x32, 
0x02, 0x02,
0x5B, //显示对象
0x11};//屏序号

//显示接口类属性3，显示时间
BYTE g_bDisplay3Fmt[] = {0x12};

//显示接口类属性4，显示参数
BYTE g_bDisplay4Fmt[] = {0x02, 0x02, 
0x11, //当前总对象数
0x11};//允许最大对象数


#define MAX_MTR_ANNEX	3	//电表附属信息最大数组个数
#define	MAX_MTR_BATNUM	50	//一次批量配置最大个数50

#define EXT_DATATYPE_DATA	128	//扩展的数据类型128，用于data数据里面再包含了data，此时只解析为string就好


#if 0
BYTE g_bAddMeterFmt[] = {DT_STRUCT, 0x04, 	//添加一个采集档案配置单元
DT_LONG_U, 
DT_STRUCT, 0x0a, 
DT_TSA, //16, RLV, //TSA
DT_ENUM, //emu-----波特率
DT_ENUM, //emu-----规约类型
DT_OAD, //OAD----端口
DT_OCT_STR, 32, RLV, //密码，变长，最长32字节
DT_UNSIGN, //费率个数
DT_UNSIGN, //---------用户类型
DT_ENUM, //emu----接线方式
DT_LONG_U,	//额定电压
DT_LONG_U,	//额定电流
DT_STRUCT, 0x04, 
DT_TSA, //16, RLV, //TSA
DT_OCT_STR, 16, RLV, //octing
DT_LONG_U, //PT
DT_LONG_U, //CT
#if 0
DT_ARRAY, MAX_MTR_ANNEX,
DT_STRUCT, 0x02, //结构体
DT_OAD, //OAD
128, 100, RLV, 	//-----Data里面嵌套data，扩展一个数据类型128，做data
#else 
DT_MTR_ANNEX,
#endif
};
#endif


BYTE g_bBatchAddMeterFmt[] = {DT_ARRAY, MAX_MTR_BATNUM,  
DT_STRUCT, 0x04, 	//添加一个采集档案配置单元
DT_LONG_U, 
DT_STRUCT, 0x0a, 
DT_TSA, //16, RLV, //TSA
DT_ENUM, //emu-----波特率
DT_ENUM, //emu-----规约类型
DT_OAD, //OAD----端口
DT_OCT_STR, 32, RLV, //密码，变长，最长32字节
DT_UNSIGN, //费率个数
DT_UNSIGN, //---------用户类型
DT_ENUM, //emu----接线方式
DT_LONG_U,	//额定电压
DT_LONG_U,	//额定电流
DT_STRUCT, 0x04, 
DT_TSA, //16, RLV, //TSA
DT_OCT_STR, 16, RLV, //octing
DT_LONG_U, //PT
DT_LONG_U, //CT
#if 0
DT_ARRAY, MAX_MTR_ANNEX,
DT_STRUCT, 0x02, //结构体
DT_OAD, //OAD
128, 100, RLV, 	//-----Data里面嵌套data，扩展一个数据类型128，做data
#else 
DT_MTR_ANNEX,
#endif
};	//批量添加采集档案配置单元
//任务配置单元格式
BYTE g_bTskUnitFmtDesc[] = {
	1,	//array
	255,	//最大个数
	2,	//struct
	12,	//12个成员
	DT_UNSIGN,	//unsigned	
	DT_TI,	//TI
	DT_ENUM,	//enum
	DT_UNSIGN,	//unsigned
	DT_DATE_TIME_S,	//date_time_s
	DT_DATE_TIME_S,	//date_time_s
	DT_TI,	//TI
#ifdef GW_OOB_PROTO_UPDATA_20170406
	DT_UNSIGN,
#else
	DT_ENUM,	//enum
#endif
	DT_ENUM,	//enum
	DT_LONG_U,	//long-unsigned
	DT_LONG_U,	//long-unsigned
	2,	//struct
	2,	//struct 成员个数
	DT_ENUM,	//enum
	1,	//array
	24,	//24个时段
	2,	//struct
	4,	//4个成员
	DT_UNSIGN,	//unsigned
	DT_UNSIGN,	//unsigned
	DT_UNSIGN,	//unsigned
	DT_UNSIGN,	//unsigned
};

//普通采集方案格式
BYTE g_bCommFmtDesc[] = {
	1,	//array
	255,	//最大个数
	2,	//struct
	6,	//12个成员
	DT_UNSIGN,	//unsigned
	DT_LONG_U,	//long-unsigned
	DT_ACQ_TYPE,	//协议扩展格式
	1,	//array
	64,	//64个成员个数
	DT_CSD,	//CSD
	DT_MS,	//MS
	DT_ENUM,	//enum
};

//事件采集方案
BYTE g_bEvtFmtDesc[] = {
	1,	//array
	255,	//最大个数
	2,	//struct
	5,	
	DT_UNSIGN,	//unsigned
	//DT_STRUCT,
	//0x02,
	//DT_UNSIGN,
	//1,	//array
	//64,	
	//DT_ROAD,	//ROAD
	DT_EVTACQ_TYPE,	//协议扩展格式
	DT_MS,	//MS
	3,	//bool
	DT_LONG_U,	//long-unsigned
};

//事件采集方案，更新上报标识
BYTE g_bEvtAcqUpdRptFlg[] = {
	DT_STRUCT,
	0x02,
	DT_UNSIGN,
	DT_BOOL,
};

//更新任务状态
BYTE g_bUdpTaskState[] = {
	DT_STRUCT,
	0x02,
	DT_UNSIGN,	//任务ID
	DT_ENUM,	//状态，正常（1），停用（2）
}; 

//透明采集方案
BYTE g_bTranFmtDesc[] = {
	1,	//array
	255,	//最大个数
	2,	//struct
	2,
	8,	//unsigned
	1,	//array
	32,	
	2,	//struct
	5,	
	DT_TSA,	//TSA
	DT_UNSIGN,
	DT_LONG_U,	//long-unsigned
	DT_LONG_U,	//long-unsigned
	2,	//struct
	4,	
	3,	//bool
	DT_LONG_U,	//long-unsigned
	DT_ENUM,	//enum
	2,	//struct
	3,
	DT_UNSIGN,	//unsigned
	DT_LONG_U,	//long-unsigned
	DT_LONG_U,	//long-unsigned
	1,	//array
	16,
	2,	//struct
	2,	
	DT_UNSIGN,	//unsigned
	9,	//oct
	128,
};

//上报采集方案
BYTE g_bRptFmtDesc[] = {
	DT_ARRAY,	//array
	255,	//最大个数
	DT_STRUCT,	//struct
	5,
	DT_UNSIGN,	//unsigned
	1,	//array
	62,	
	DT_OAD,	//OAD
	DT_TI,	//TI
	DT_UNSIGN,	//unsigned
	DT_RPT_TYPE,	//
};

//采集规则库
BYTE g_bAddAcqRuleLib[] = {
	DT_ARRAY,	
	255,
	DT_STRUCT, 
	2,
	DT_CSD,	//数据列选择描述符  CSD
	DT_STRUCT,	//规则描述  structrue
	3,
	//------------------------------------------------------------------
	DT_STRUCT,	//AcqCmd_2007  structure
	3,
	DT_STRUCT,	//主用DI  array octet-string(SIZE(4))，
	2,
	DT_ARRAY, 
	20,
	DT_OCT_STR,
	4,
	DT_ARRAY, 
	20,
	DT_OCT_STR,
	4,	

	DT_STRUCT,	//备用DI  array octet-string(SIZE(4))，
	2,
	DT_ARRAY, 
	20,
	DT_OCT_STR,
	4,
	DT_ARRAY, 
	20,
	DT_OCT_STR,
	4,	
	//-----------------------------------------------------------------
	DT_STRUCT,	//AcqCmd_1997  structure
	3,
	DT_STRUCT,	//主用DI  array octet-string(SIZE(4))，
	2,
	DT_ARRAY, 
	20,
	DT_OCT_STR,
	4,
	DT_ARRAY, 
	20,
	DT_OCT_STR,
	4,	

	DT_STRUCT,	//备用DI  array octet-string(SIZE(4))，
	2,
	DT_ARRAY, 
	20,
	DT_OCT_STR,
	4,
	DT_ARRAY, 
	20,
	DT_OCT_STR,
	4,	
	//------------------------------------------------------------------
	DT_STRUCT,	//AcqCmd_Trans
	1,
	DT_OCT_STR,
	255,
};

//删除一组采集规则
BYTE g_bDelAcqRuleLib[] = {
	DT_ARRAY,
	64,
	DT_CSD,
};	


//实时监控方案格式描述
BYTE g_bRealFmtDesc[] = {
	1,	//array
	255,	//最大个数
	2,	//struct
	4,
	DT_UNSIGN, //unsigned
	1,	//array
	64,	
	DT_CSD,	//CSD
	DT_MS,	//MS
	3,	//bool
};

//重置方案列表
BYTE g_bResetCSDFmtDesc[] = {
	DT_STRUCT,	//struct
	2,	
	DT_UNSIGN,	//unsigned
	DT_ARRAY,	//array
	64,
	DT_CSD,	//CSD
};

//总加组相关格式
BYTE g_bGrpCfgFmt[] = {DT_ARRAY, 4,	//数组及元素个数
DT_STRUCT,0x03,		//结构及成员个数
DT_TSA,		//通信地址 Tsa
DT_ENUM,		//总加标志 enum
DT_ENUM,		//运算符标志 enum
};	//总加组配置
BYTE g_bGrpPowFmt[] = {DT_LONG64};	//总加组功率
BYTE g_bGrpEngFmt[] = {DT_ARRAY, 5, DT_LONG64};	//总加组电能量
BYTE g_bGrpSlipIntervFmt[] = {DT_UNSIGN};	//总加组滑差周期
BYTE g_bGrpTurnFmt[] = {DT_BIT_STR, 1, RLF};	//总加组轮次配置
BYTE g_bGrpCtrlStaCfgFmt[] = {DT_STRUCT, 6,		//结构及成员个数
DT_UNSIGN,	//时段控定值方案号
DT_BIT_STR, 1, RLF,	//功控时段有效标志位
DT_BIT_STR, 1, RLF,	//功控状态
DT_BIT_STR, 1, RLF,	//电控状态
DT_BIT_STR, 1, RLF,	//功控轮次状态
DT_BIT_STR, 1, RLF,	//电控轮次状态
};	//总加组控制设置状态
BYTE g_bGrpCtrlStaCurFmt[] = {DT_STRUCT, 7,		//结构及成员个数
DT_LONG64,	//当前功控定值
DT_INT,	//当前功率下浮控浮动系数
DT_BIT_STR, 1, RLF,	//功控跳闸输出状态
DT_BIT_STR, 1, RLF,	//月电控跳闸输出状态
DT_BIT_STR, 1, RLF,	//购电控跳闸输出状态
DT_BIT_STR, 1, RLF,	//功控越限告警状态
DT_BIT_STR, 1, RLF,	//电控越限告警状态
};	//总加组当前控制状态
BYTE g_bGrpDataUnitFmt[] = { DT_STRUCT, 10, 
DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT,  //属性3~属性7单位
DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT,  //属性8~属性12单位
};	//总加组换算及单位
//脉冲计量相关格式
BYTE g_bCommAddrFmt[] = { DT_OCT_STR, 16 };	//RLV

BYTE g_bPTCTFmt[] = { DT_STRUCT, 0x02,
DT_LONG_U,		//PT
DT_LONG_U,		//CT
};

BYTE g_bPulseCfgFmt[] = { DT_ARRAY, MAX_PULSE_TYPE,	//数组及元素个数
DT_STRUCT,0x03,		//结构及成员个数
DT_OAD,		//端口号 OAD
DT_ENUM,		//脉冲属性 enum
DT_LONG_U,		//脉冲常数 long-unsigned
};

BYTE g_bPulsePowerFmt[] = { DT_DB_LONG };	//脉冲功率

BYTE g_bPulseDataUnitFmt[] = { DT_STRUCT, 0x0e, 
DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, //属性5~属性11单位
DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, DT_SCALE_UNIT, //属性12~属性18单位
};
BYTE g_bBalanceDayFmt[] = { DT_ARRAY, BALANCE_DAY_NUM, DT_STRUCT, 02, DT_UNSIGN, DT_UNSIGN };

//搜表结果
BYTE g_bSchMtrRltFmt[] = {DT_ARRAY, POINT_NUM,
DT_TSA,	//通信地址
DT_TSA,	//所属采集器地址
DT_ENUM,	//规约类型
DT_ENUM,	//相位
DT_UNSIGN,	//信号品质 
DT_DATE_TIME_S,	//搜到的时间
DT_SCH_MTR_ANNEX,	//搜到的附加信息
};

//跨台区搜表结果
BYTE g_bCrossSchMtrRltFmt[] = {DT_ARRAY, POINT_NUM,
DT_TSA,	//通信地址
DT_TSA,	//主节点地址
DT_DATE_TIME_S,	//变更时间
};

//所有搜表结果记录数
BYTE g_bSchMtrCntFmt[] = {DT_LONG_U};

//跨台区搜表结果记录数
BYTE g_bCrossSchMtrCntFmt[] = {DT_LONG_U};

//搜表参数
BYTE g_bSchMtrParaFmt[] = {DT_STRUCT, 0x04,
DT_BOOL,	//是否启用每天周期搜表
DT_BOOL,	//自动更新采集档案 
DT_BOOL,	//是否产生搜表相关事件
DT_ENUM,	//清空搜表结果选项
};

//定时搜表参数
BYTE g_bTimeSchMtrParaFmt[] = {DT_ARRAY, MAX_TIME_SCH_MTR_NUM,
DT_STRUCT, 0x02,
DT_TIME,	//开始时间
DT_LONG_U,	//搜表时长（min）
};

//搜表实时状态
BYTE g_bSchMtrStateFmt[] = {DT_ENUM};
//有功总电能差动组配置参数格式描述串
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
BYTE g_bECompareCfgFmt[] = { DT_ARRAY, DIFF_COMP_CFG_NUM,
DT_STRUCT, 0x06, 
DT_UNSIGN, 
DT_OI,
DT_OI,
DT_UNSIGN, 
DT_INT,
DT_DB_LONG,
};

//冻结类参数格式
static BYTE g_bTrigFrzFmt[] = {
	DT_LONG_U,
};

static BYTE g_bAddFrzCfgFmt[] = {
	DT_STRUCT,	//struct
	3,	//成员个数
	DT_LONG_U,	//long unsigned
	DT_OAD,
	DT_LONG_U,
};

static BYTE g_bDelFrzCfgFmt[] = {
	DT_OAD,
};

static BYTE g_bTrigReFrzFmt[] = {
	DT_DATE_TIME_S,
	DT_DATE_TIME_S,
};	//方法6 补冻结



//批量添加关联属性表格式
static BYTE g_bBatAddFrzCfgFmt[] = { DT_ARRAY, CAP_OAD_NUM,	//数组及元素个数
DT_STRUCT,0x03,		//结构及成员个数
DT_LONG_U,		//周期 long-unsigned
DT_OAD,			//OAD
DT_LONG_U,		//深度 long-unsigned
};


static BYTE g_bAddPulseCfgFmt[] = {
	DT_STRUCT,	//struct
	3,	//成员个数
	DT_OAD,
	DT_ENUM,	//long unsigned
	DT_LONG_U,
};

static BYTE g_bDelPulseCfgFmt[] = {
	DT_OAD,
};

static BYTE g_bOADCfgFmt[] = {
	DT_OAD,
};
static BYTE g_bResetEvtFmt[] = {
	DT_INT,
};
static BYTE g_bRunEvtFmt[] = {
	DT_NULL,
};
static BYTE g_bIC24TrigEvtFmt[] = {
	DT_ENUM,
	DT_LONG_U,	
	DT_LONG_U,	
};
static BYTE g_bSrcNullTrigEvtFmt[] = {
	DT_NULL,
	DT_LONG_U,	
	DT_LONG_U,	
};
static BYTE g_bSrcOADTrigEvtFmt[] = {
	DT_OAD,
	DT_LONG_U,	
	DT_LONG_U,	
};
static BYTE g_bSrcEnumTrigEvtFmt[] = {
	DT_ENUM,
	DT_LONG_U,	
	DT_LONG_U,	
};
static BYTE g_bSrcOITrigEvtFmt[] = {
	DT_OI,
	DT_LONG_U,	
	DT_LONG_U,	
};

static BYTE g_bSrcTsaTrigEvtFmt[] = {
	DT_TSA,
	DT_LONG_U,	
	DT_LONG_U,	
};

//区间统计参数
BYTE g_bRangeStatFmt[] = {
	DT_ARRAY,	//array
	SPECIAL_NUM,//20,	//最大个数
	DT_STRUCT,	//struct
	4,
	DT_OAD,	//unsigned
	DT_OVER_PARA,	//越限判断参数array Data
	DT_UNSIGN,	//统计周期  unsigned
	DT_TI,	//TI
};
//区间统计数据
BYTE g_bRangeStatResFmt[] = {
	DT_ARRAY,	//array
	SPECIAL_NUM,//20,	//可以定义为0个么?
	DT_STRUCT,	//struct
	2,
	DT_OAD,	//unsigned
	DT_OVER_RES,	//区间统计值array 

};
//平均统计参数
BYTE g_bAvrStatFmt[] = {
	DT_ARRAY,	//array
	SPECIAL_NUM,//20,	//最大个数
	DT_STRUCT,	//struct
	3,
	DT_OAD,	//unsigned
	DT_UNSIGN,	//统计周期  unsigned
	DT_TI,	//TI
};

//平均统计结果
BYTE g_bAvrResFmt[] = {
	DT_ARRAY,	//array
	SPECIAL_NUM,//20,	//最大个数
	DT_STRUCT,	//struct
	3,
	DT_OAD,	//unsigned
	DT_INSTANCE,	//累加和
	DT_INSTANCE,	//平均值
};

//极值统计结果
BYTE g_bExtremResFmt[] = {
	DT_ARRAY,	//array
	SPECIAL_NUM,//20,	//最大个数
	DT_STRUCT,	//struct
	5,
	DT_OAD,	//unsigned
	DT_INSTANCE,	//累加和
	DT_DATE_TIME_S,
	DT_INSTANCE,	//平均值
	DT_DATE_TIME_S,
};

//电压合格率参数
BYTE g_bVolParaFmt[] = {
	DT_STRUCT,	//struct
	4,
	DT_LONG_U,//电压考核上限
	DT_LONG_U,//电压考核下限
	DT_LONG_U,//电压合格上限
	DT_LONG_U,//电压合格下限

};

//电压合格率数据
BYTE g_bVoltStatFmt[] = {
	DT_STRUCT,	//struct
	2,	//最大个数
	DT_STRUCT,	//struct
	5,
	DT_DB_LONG_U,	//电压监测时间
	DT_LONG_U,	//电压合格率
	DT_LONG_U,	//电压超限率
	DT_DB_LONG_U,	//电压超上限时间
	DT_DB_LONG_U,	//电压超下限时间
	DT_STRUCT,	//struct
	5,
	DT_DB_LONG_U,	//电压监测时间
	DT_LONG_U,	//电压合格率
	DT_LONG_U,	//电压超限率
	DT_DB_LONG_U,	//电压超上限时间
	DT_DB_LONG_U,	//电压超下限时间
};

//最大有功功率及发生时间
BYTE g_bMaxPowerFmt[] = {
	DT_STRUCT,	//struct
	2,	//最大个数
	DT_DB_LONG_U,	//最大功率值
	DT_DATE_TIME_S,	//发生时间
};

//复位次数
BYTE g_bResetTimesFmt[] = {
	DT_STRUCT,	//struct
	2,	//最大个数
	DT_LONG_U,	//日复位累计次数
	DT_LONG_U,	//月复位累计次数
};
//供电时间\通信流量
BYTE g_bOnTimeFmt[] = {
	DT_STRUCT,	//struct
	2,	//最大个数
	DT_DB_LONG_U,	//日供电累计时间
	DT_DB_LONG_U,	//月供电累计时间
};
#define SCA_UNIT_ENG		((WORD )0x21<<8) + 0xfe	//普通电量数据
#define SCA_UNIT_ENG_HIPRE	((WORD )0x21<<8) + 0xfc	//高精度电量数据
#define SCA_UNIT_COMENG		((WORD )0x23<<8) + 0xfe	//普通组合电量数据
#define SCA_UNIT_COMENG_HI	((WORD )0x23<<8) + 0xfc	//高精度组合电量数据

#define SCA_UNIT_DEM_P		((WORD )0x1c<<8) + 0xfc	//最大需量&有功功率
#define SCA_UNIT_DEM_Q		((WORD )0x20<<8) + 0xfc	//最大需量&无功功率
#define SCA_UNIT_DEM_S		((WORD )0x1E<<8) + 0xfc	//最大需量&视在功率

#define VAR_HARM_NUM		(21)	//谐波含量次数

ToaMap g_OIConvertClass[] = 
{
	{0x00000200,	1,		MAP_SYSDB,		0x0001,	  PN0,   0,					g_bComEngDataFmt,	sizeof(g_bComEngDataFmt),	NULL},	//组合有功电能示值
	{0x00000300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	//组合有功电能换算及单位
	{0x00000400,	1,		MAP_SYSDB,		0x0601,	  PN0,   0,					g_bComHiPreEngFmt,	sizeof(g_bComHiPreEngFmt),	NULL},	//高精度组合有功电能电量
	{0x00000500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	//组合有功电能高精度电量

	{0x00100200,	1,		MAP_SYSDB,		0x0010,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向有功电能
	{0x00100300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	//正向有功电能换算及单位
	{0x00100400,	1,		MAP_SYSDB,		0x0610,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	//高精度正向有功电能换算及单位
	{0x00100500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	//组合有功电能高精度电量

	{0x00110200,	1,		MAP_SYSDB,		0x0011,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//A相正向有功电能
	{0x00110300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	//A相正向有功电能换算及单位
	{0x00110400,	1,		MAP_SYSDB,		0x0611,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	//高精度正向有功电能换算及单位
	{0x00110500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	//组合有功电能高精度电量

	{0x00120200,	1,		MAP_SYSDB,		0x0012,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//B相正向有功电能
	{0x00120300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	//B相正向有功电能换算及单位
	{0x00120400,	1,		MAP_SYSDB,		0x0612,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	//高精度正向有功电能换算及单位
	{0x00120500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	//组合有功电能高精度电量

	{0x00130200,	1,		MAP_SYSDB,		0x0013,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//C相正向有功电能
	{0x00130300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	//C相正向有功电能换算及单位
	{0x00130400,	1,		MAP_SYSDB,		0x0613,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	//高精度正向有功电能换算及单位
	{0x00130500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	//组合有功电能高精度电量

	{0x00200200,	1,		MAP_SYSDB,		0x0020,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向有功电能
	{0x00200300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	//C相正向有功电能换算及单位
	{0x00200400,	1,		MAP_SYSDB,		0x0620,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	//高精度正向有功电能换算及单位
	{0x00200500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	//组合有功电能高精度电量

	{0x00210200,	1,		MAP_SYSDB,		0x0021,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},
	{0x00210300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	//C相正向有功电能换算及单位
	{0x00210400,	1,		MAP_SYSDB,		0x0621,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	//高精度正向有功电能换算及单位
	{0x00210500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	//组合有功电能高精度电量

	{0x00220200,	1,		MAP_SYSDB,		0x0022,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},
	{0x00220300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	//C相正向有功电能换算及单位
	{0x00220400,	1,		MAP_SYSDB,		0x0622,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	//高精度正向有功电能换算及单位
	{0x00220500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	//组合有功电能高精度电量

	{0x00230200,	1,		MAP_SYSDB,		0x0023,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},
	{0x00230300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	//C相正向有功电能换算及单位
	{0x00230400,	1,		MAP_SYSDB,		0x0623,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	//高精度正向有功电能换算及单位
	{0x00230500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	//组合有功电能高精度电量

	{0x00300200,	1,		MAP_SYSDB,		0x0030,	  PN0,   0,					g_bComEngDataFmt,	sizeof(g_bComEngDataFmt),	NULL},	//组合无功1电能
	{0x00300300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG,	NULL,				0,							NULL},	
	{0x00300400,	1,		MAP_SYSDB,		0x0630,	  PN0,   0,					g_bComHiPreEngFmt,	sizeof(g_bComHiPreEngFmt),	NULL},	
	{0x00300500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG_HI,NULL,				0,							NULL},	

	{0x00310200,	1,		MAP_SYSDB,		0x0031,	  PN0,   0,					g_bComEngDataFmt,	sizeof(g_bEngDataFmt),		NULL},	//A组合无功1电能
	{0x00310300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG,	NULL,				0,							NULL},	
	{0x00310400,	1,		MAP_SYSDB,		0x0631,	  PN0,   0,					g_bComHiPreEngFmt,	sizeof(g_bComHiPreEngFmt),	NULL},	
	{0x00310500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG_HI,NULL,				0,							NULL},	

	{0x00320200,	1,		MAP_SYSDB,		0x0032,	  PN0,   0,					g_bComEngDataFmt,	sizeof(g_bComEngDataFmt),	NULL},	//B组合无功1电能
	{0x00320300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG,	NULL,				0,							NULL},	
	{0x00320400,	1,		MAP_SYSDB,		0x0632,	  PN0,   0,					g_bComHiPreEngFmt,	sizeof(g_bComHiPreEngFmt),	NULL},	
	{0x00320500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG_HI,NULL,				0,							NULL},	

	{0x00330200,	1,		MAP_SYSDB,		0x0033,	  PN0,   0,					g_bComEngDataFmt,	sizeof(g_bComEngDataFmt),	NULL},	//C组合无功1电能
	{0x00330300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG,	NULL,				0,							NULL},	
	{0x00330400,	1,		MAP_SYSDB,		0x0633,	  PN0,   0,					g_bComHiPreEngFmt,	sizeof(g_bComHiPreEngFmt),	NULL},	
	{0x00330500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG_HI,NULL,				0,							NULL},	

	{0x00400200,	1,		MAP_SYSDB,		0x0040,	  PN0,   0,					g_bComEngDataFmt,	sizeof(g_bComEngDataFmt),	NULL},	//组合无功2电能
	{0x00400300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG,	NULL,				0,							NULL},	
	{0x00400400,	1,		MAP_SYSDB,		0x0640,	  PN0,   0,					g_bComHiPreEngFmt,	sizeof(g_bComHiPreEngFmt),	NULL},	
	{0x00400500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG_HI,NULL,				0,							NULL},	

	{0x00410200,	1,		MAP_SYSDB,		0x0041,	  PN0,   0,					g_bComEngDataFmt,	sizeof(g_bComEngDataFmt),	NULL},//A组合无功2电能
	{0x00410300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG,	NULL,				0,							NULL},	
	{0x00410400,	1,		MAP_SYSDB,		0x0641,	  PN0,   0,					g_bComHiPreEngFmt,	sizeof(g_bComHiPreEngFmt),	NULL},	
	{0x00410500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG_HI,NULL,				0,							NULL},	

	{0x00420200,	1,		MAP_SYSDB,		0x0042,	  PN0,   0,					g_bComEngDataFmt,	sizeof(g_bComEngDataFmt),	NULL},//B组合无功2电能
	{0x00420300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG,	NULL,				0,							NULL},	
	{0x00420400,	1,		MAP_SYSDB,		0x0642,	  PN0,   0,					g_bComHiPreEngFmt,	sizeof(g_bComHiPreEngFmt),	NULL},	
	{0x00420500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG_HI,NULL,				0,							NULL},	

	{0x00430200,	1,		MAP_SYSDB,		0x0043,	  PN0,   0,					g_bComEngDataFmt,	sizeof(g_bComEngDataFmt),	NULL},//C组合无功2电能
	{0x00430300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG,	NULL,				0,							NULL},	
	{0x00430400,	1,		MAP_SYSDB,		0x0643,	  PN0,   0,					g_bComHiPreEngFmt,	sizeof(g_bComHiPreEngFmt),	NULL},	
	{0x00430500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_COMENG_HI,NULL,				0,							NULL},	

	{0x00500200,	1,		MAP_SYSDB,		0x0050,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//第1象限无功电能
	{0x00500300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00500400,	1,		MAP_SYSDB,		0x0650,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00500500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00510200,	1,		MAP_SYSDB,		0x0051,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A第1象限无功电能
	{0x00510300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00510400,	1,		MAP_SYSDB,		0x0651,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00510500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00520200,	1,		MAP_SYSDB,		0x0052,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B第1象限无功电能
	{0x00520300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00520400,	1,		MAP_SYSDB,		0x0652,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00520500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00530200,	1,		MAP_SYSDB,		0x0053,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C第1象限无功电能
	{0x00530300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00530400,	1,		MAP_SYSDB,		0x0653,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00530500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00600200,	1,		MAP_SYSDB,		0x0060,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//第2象限无功电能
	{0x00600300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00600400,	1,		MAP_SYSDB,		0x0660,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00600500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00610200,	1,		MAP_SYSDB,		0x0061,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A第2象限无功电能
	{0x00610300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00610400,	1,		MAP_SYSDB,		0x0661,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00610500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00620200,	1,		MAP_SYSDB,		0x0062,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B第2象限无功电能
	{0x00620300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00620400,	1,		MAP_SYSDB,		0x0662,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00620500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00630200,	1,		MAP_SYSDB,		0x0063,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C第2象限无功电能
	{0x00630300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00630400,	1,		MAP_SYSDB,		0x0663,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00630500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00700200,	1,		MAP_SYSDB,		0x0070,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//第3象限无功电能
	{0x00700300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00700400,	1,		MAP_SYSDB,		0x0670,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00700500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00710200,	1,		MAP_SYSDB,		0x0071,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A第3象限无功电能
	{0x00710300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00710400,	1,		MAP_SYSDB,		0x0671,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00710500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00720200,	1,		MAP_SYSDB,		0x0072,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B第3象限无功电能
	{0x00720300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00720400,	1,		MAP_SYSDB,		0x0672,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00720500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00730200,	1,		MAP_SYSDB,		0x0073,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C第3象限无功电能
	{0x00730300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00730400,	1,		MAP_SYSDB,		0x0673,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00730500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00800200,	1,		MAP_SYSDB,		0x0080,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//第4象限无功电能
	{0x00800300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00800400,	1,		MAP_SYSDB,		0x0680,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00800500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00810200,	1,		MAP_SYSDB,		0x0081,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A第4象限无功电能
	{0x00810300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00810400,	1,		MAP_SYSDB,		0x0681,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00810500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00820200,	1,		MAP_SYSDB,		0x0082,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B第4象限无功电能
	{0x00820300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00820400,	1,		MAP_SYSDB,		0x0682,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00820500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00830200,	1,		MAP_SYSDB,		0x0083,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C第4象限无功电能
	{0x00830300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00830400,	1,		MAP_SYSDB,		0x0683,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00830500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00900200,	1,		MAP_SYSDB,		0x0090,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//正向视在电能
	{0x00900300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00900400,	1,		MAP_SYSDB,		0x0690,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00900500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00910200,	1,		MAP_SYSDB,		0x0091,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A正向视在电能
	{0x00910300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00910400,	1,		MAP_SYSDB,		0x0691,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00910500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00920200,	1,		MAP_SYSDB,		0x0092,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B正向视在电能
	{0x00920300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00920400,	1,		MAP_SYSDB,		0x0692,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00920500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00930200,	1,		MAP_SYSDB,		0x0093,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C正向视在电能
	{0x00930300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00930400,	1,		MAP_SYSDB,		0x0693,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00930500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00A00200,	1,		MAP_SYSDB,		0x00A0,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//反向视在电能
	{0x00A00300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00A00400,	1,		MAP_SYSDB,		0x06A0,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00A00500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00A10200,	1,		MAP_SYSDB,		0x00A1,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A反向视在电能
	{0x00A10300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00A10400,	1,		MAP_SYSDB,		0x06A1,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00A10500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00A20200,	1,		MAP_SYSDB,		0x00A2,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B反向视在电能
	{0x00A20300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00A20400,	1,		MAP_SYSDB,		0x06A2,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00A20500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x00A30200,	1,		MAP_SYSDB,		0x00A3,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C反向视在电能
	{0x00A30300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x00A30400,	1,		MAP_SYSDB,		0x06A3,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x00A30500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x01100200,	1,		MAP_SYSDB,		0x0110,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//正向有功基波电能
	{0x01100300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x01100400,	1,		MAP_SYSDB,		0x0710,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x01100500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x01110200,	1,		MAP_SYSDB,		0x0111,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A正向有功基波电能
	{0x01110300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x01110400,	1,		MAP_SYSDB,		0x0711,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x01110500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x01120200,	1,		MAP_SYSDB,		0x0112,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B正向有功基波电能
	{0x01120300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x01120400,	1,		MAP_SYSDB,		0x0712,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x01120500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x01130200,	1,		MAP_SYSDB,		0x0113,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C正向有功基波电能
	{0x01130300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x01130400,	1,		MAP_SYSDB,		0x0713,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x01130500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x01200200,	1,		MAP_SYSDB,		0x0120,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//反向有功基波电能
	{0x01200300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x01200400,	1,		MAP_SYSDB,		0x0720,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x01200500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x01210200,	1,		MAP_SYSDB,		0x0121,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A反向有功基波电能
	{0x01210300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x01210400,	1,		MAP_SYSDB,		0x0721,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x01210500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x01220200,	1,		MAP_SYSDB,		0x0122,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B反向有功基波电能
	{0x01220300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x01220400,	1,		MAP_SYSDB,		0x0722,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x01220500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x01230200,	1,		MAP_SYSDB,		0x0123,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C反向有功基波电能
	{0x01230300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x01230400,	1,		MAP_SYSDB,		0x0723,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x01230500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x02100200,	1,		MAP_SYSDB,		0x0210,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//正向有功谐波电能
	{0x02100300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x02100400,	1,		MAP_SYSDB,		0x0810,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x02100500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x02110200,	1,		MAP_SYSDB,		0x0211,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A正向有功谐波电能
	{0x02110300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x02110400,	1,		MAP_SYSDB,		0x0811,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x02110500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x02120200,	1,		MAP_SYSDB,		0x0212,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B正向有功谐波电能
	{0x02120300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x02120400,	1,		MAP_SYSDB,		0x0812,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x02120500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x02130200,	1,		MAP_SYSDB,		0x0213,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C正向有功谐波电能
	{0x02130300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x02130400,	1,		MAP_SYSDB,		0x0813,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x02130500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x02200200,	1,		MAP_SYSDB,		0x0220,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//反向有功谐波电能
	{0x02200300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x02200400,	1,		MAP_SYSDB,		0x0820,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x02200500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x02210200,	1,		MAP_SYSDB,		0x0221,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A反向有功谐波电能
	{0x02210300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x02210400,	1,		MAP_SYSDB,		0x0821,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x02210500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x02220200,	1,		MAP_SYSDB,		0x0222,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B反向有功谐波电能
	{0x02220300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x02220400,	1,		MAP_SYSDB,		0x0822,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x02220500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x02230200,	1,		MAP_SYSDB,		0x0223,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C反向有功谐波电能
	{0x02230300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x02230400,	1,		MAP_SYSDB,		0x0823,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x02230500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x03000200,	1,		MAP_SYSDB,		0x0300,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//铜损有功总电能补偿量
	{0x03000300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x03000400,	1,		MAP_SYSDB,		0x0900,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x03000500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x03010200,	1,		MAP_SYSDB,		0x0301,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A铜损有功总电能补偿量
	{0x03010300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x03010400,	1,		MAP_SYSDB,		0x0901,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x03010500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x03020200,	1,		MAP_SYSDB,		0x0302,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B铜损有功总电能补偿量
	{0x03020300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x03020400,	1,		MAP_SYSDB,		0x0902,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x03020500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x03030200,	1,		MAP_SYSDB,		0x0303,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C铜损有功总电能补偿量
	{0x03030300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x03030400,	1,		MAP_SYSDB,		0x0903,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x03030500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x04000200,	1,		MAP_SYSDB,		0x0400,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//铁损有功总电能补偿量
	{0x04000300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x04000400,	1,		MAP_SYSDB,		0x0A00,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x04000500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x04010200,	1,		MAP_SYSDB,		0x0401,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A铁损有功总电能补偿量
	{0x04010300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x04010400,	1,		MAP_SYSDB,		0x0A01,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x04010500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x04020200,	1,		MAP_SYSDB,		0x0402,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B铁损有功总电能补偿量
	{0x04020300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x04020400,	1,		MAP_SYSDB,		0x0A02,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x04020500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x04030200,	1,		MAP_SYSDB,		0x0403,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C铁损有功总电能补偿量
	{0x04030300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x04030400,	1,		MAP_SYSDB,		0x0A03,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x04030500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},

	{0x05000200,	1,		MAP_SYSDB,		0x0500,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//关联总电能
	{0x05000300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x05000400,	1,		MAP_SYSDB,		0x0B00,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x05000500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x05010200,	1,		MAP_SYSDB,		0x0501,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//A关联总电能
	{0x05010300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x05010400,	1,		MAP_SYSDB,		0x0B01,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x05010500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x05020200,	1,		MAP_SYSDB,		0x0502,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//B关联总电能
	{0x05020300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x05020400,	1,		MAP_SYSDB,		0x0B02,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x05020500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},	

	{0x05030200,	1,		MAP_SYSDB,		0x0503,	  PN0,   0,					g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},//C关联总电能
	{0x05030300,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG,		NULL,				0,							NULL},	
	{0x05030400,	1,		MAP_SYSDB,		0x0B03,	  PN0,   0,					g_bHiPreEngFmt,		sizeof(g_bHiPreEngFmt),		NULL},	
	{0x05030500,	1,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_ENG_HIPRE,NULL,				0,							NULL},

	///////////////////////////////////////////////////////////////////////////////////////////
	//最大需量类对象标识
	{0x10100200,	2,		MAP_SYSDB,		0x1010,	  PN0,   0,					g_bMaxDemFmt,		sizeof(g_bMaxDemFmt),		NULL},	//正向有功最大需量
	{0x10100300,	2,		MAP_VAL,		0x0000,	  PN0,   SCA_UNIT_DEM_P,	NULL,				0,							NULL},	//Scaler_Unit


	{0x10110200,	2,		MAP_SYSDB,		0x1011,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//A相正向有功最大需量
	{0x10120200,	2,		MAP_SYSDB,		0x1012,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//B相正向有功最大需量
	{0x10130200,	2,		MAP_SYSDB,		0x1013,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//C相正向有功最大需量

	{0x10200200,	2,		MAP_SYSDB,		0x1020,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//反向有功最大需量
	{0x10210200,	2,		MAP_SYSDB,		0x1021,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//A相反向有功最大需量
	{0x10220200,	2,		MAP_SYSDB,		0x1022,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//B相反向有功最大需量
	{0x10230200,	2,		MAP_SYSDB,		0x1023,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//C相反向有功最大需量

	{0x10300200,	2,		MAP_SYSDB,		0x1030,	  PN0,   0,		g_bComMaxDemFmt, sizeof(g_bComMaxDemFmt), NULL},	//反向有功最大需量
	{0x10310200,	2,		MAP_SYSDB,		0x1031,	  PN0,   0,		g_bComMaxDemFmt, sizeof(g_bComMaxDemFmt), NULL},	//A相反向有功最大需量
	{0x10320200,	2,		MAP_SYSDB,		0x1032,	  PN0,   0,		g_bComMaxDemFmt, sizeof(g_bComMaxDemFmt), NULL},	//B相反向有功最大需量
	{0x10330200,	2,		MAP_SYSDB,		0x1033,	  PN0,   0,		g_bComMaxDemFmt, sizeof(g_bComMaxDemFmt), NULL},	//C相反向有功最大需量

	{0x10400200,	2,		MAP_SYSDB,		0x1040,	  PN0,   0,		g_bComMaxDemFmt, sizeof(g_bComMaxDemFmt), NULL},	//反向有功最大需量
	{0x10410200,	2,		MAP_SYSDB,		0x1041,	  PN0,   0,		g_bComMaxDemFmt, sizeof(g_bComMaxDemFmt), NULL},	//A相反向有功最大需量
	{0x10420200,	2,		MAP_SYSDB,		0x1042,	  PN0,   0,		g_bComMaxDemFmt, sizeof(g_bComMaxDemFmt), NULL},	//B相反向有功最大需量
	{0x10430200,	2,		MAP_SYSDB,		0x1043,	  PN0,   0,		g_bComMaxDemFmt, sizeof(g_bComMaxDemFmt), NULL},	//C相反向有功最大需量

	{0x10500200,	2,		MAP_SYSDB,		0x1050,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//1象限无功最大需量
	{0x10510200,	2,		MAP_SYSDB,		0x1051,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//A相1象限无功最大需量
	{0x10520200,	2,		MAP_SYSDB,		0x1052,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//B相1象限无功最大需量
	{0x10530200,	2,		MAP_SYSDB,		0x1053,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//C相1象限无功最大需量

	{0x10600200,	2,		MAP_SYSDB,		0x1060,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//2象限无功最大需量
	{0x10610200,	2,		MAP_SYSDB,		0x1061,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//A相2象限无功最大需量
	{0x10620200,	2,		MAP_SYSDB,		0x1062,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//B相2象限无功最大需量
	{0x10630200,	2,		MAP_SYSDB,		0x1063,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//C相2象限无功最大需量

	{0x10700200,	2,		MAP_SYSDB,		0x1070,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//3象限无功最大需量
	{0x10710200,	2,		MAP_SYSDB,		0x1071,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//A相3象限无功最大需量
	{0x10720200,	2,		MAP_SYSDB,		0x1072,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//B相3象限无功最大需量
	{0x10730200,	2,		MAP_SYSDB,		0x1073,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//C相3象限无功最大需量

	{0x10800200,	2,		MAP_SYSDB,		0x1080,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//4象限无功最大需量
	{0x10810200,	2,		MAP_SYSDB,		0x1081,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//A相4象限无功最大需量
	{0x10820200,	2,		MAP_SYSDB,		0x1082,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//B相4象限无功最大需量
	{0x10830200,	2,		MAP_SYSDB,		0x1083,	  PN0,   0,		g_bMaxDemFmt, sizeof(g_bMaxDemFmt), NULL},	//C相4象限无功最大需量


	///////////////////////////////////////////////////////////////////////////////////////////
	//变量类对象标识
	{0x20000200,	3,		MAP_SYSDB,		0x2000,	  PN0,   0,		g_bVoltDataFmt, sizeof(g_bVoltDataFmt), NULL},	//电压配置
	{0x20010200,	3,		MAP_SYSDB,		0x2001,	  PN0,   0,		g_bCurDataFmt,  sizeof(g_bCurDataFmt), NULL},	//电流配置
	{0x20020200,	3,		MAP_SYSDB,		0x2002,	  PN0,   0,		g_bVoltDataFmt, sizeof(g_bVoltDataFmt), NULL},	//电压相角
	{0x20030200,	3,		MAP_SYSDB,		0x2003,	  PN0,   0,		g_bVoltDataFmt, sizeof(g_bVoltDataFmt), NULL},	//电压电流相角
	{0x20040200,	4,		MAP_SYSDB,		0x2004,	  PN0,   0,		g_bPowerDataFmt, sizeof(g_bPowerDataFmt), NULL},	//有功功率
	{0x20050200,	4,		MAP_SYSDB,		0x2005,	  PN0,   0,		g_bPowerDataFmt, sizeof(g_bPowerDataFmt), NULL},	//无功功率
	{0x20060200,	4,		MAP_SYSDB,		0x2006,	  PN0,   0,		g_bPowerDataFmt, sizeof(g_bPowerDataFmt), NULL},	//视在功率
	{0x20070200,	4,		MAP_SYSDB,		0x2007,	  PN0,   0,		g_bPowerDataFmt, sizeof(g_bPowerDataFmt), NULL},	//一分钟平均有功功率
	{0x20080200,	4,		MAP_SYSDB,		0x2008,	  PN0,   0,		g_bPowerDataFmt, sizeof(g_bPowerDataFmt), NULL},	//一分钟平均无功功率
	{0x20090200,	4,		MAP_SYSDB,		0x2009,	  PN0,   0,		g_bPowerDataFmt, sizeof(g_bPowerDataFmt), NULL},	//一分钟平均视在功率

	{0x200a0200,	4,		MAP_SYSDB,		0x200a,	  PN0,   0,		g_bCosDataFmt,	sizeof(g_bCosDataFmt),	NULL},	//功率因数
	{0x200b0200,	3,		MAP_SYSDB,		0x200b,	  PN0,   0,		g_bVoltDistortionDataFmt,	sizeof(g_bVoltDistortionDataFmt),	NULL},	//电压波形失真度
	{0x200c0200,	3,		MAP_SYSDB,		0x200c,	  PN0,   0,		g_bCurDistortionDataFmt,	sizeof(g_bCurDistortionDataFmt),	NULL},	//电流波形失真度
	{0x200d0200,	5,		MAP_SYSDB,		0x2600,	  PN0,   0,		g_bVoltHarPercentDataFmt,	sizeof(g_bVoltHarPercentDataFmt),	NULL},	//电压谐波含量
	{0x200d0300,	5,		MAP_SYSDB,		0x2601,	  PN0,   0,		g_bVoltHarPercentDataFmt,	sizeof(g_bVoltHarPercentDataFmt),	NULL},	//电压谐波含量
	{0x200d0400,	5,		MAP_SYSDB,		0x2602,	  PN0,   0,		g_bVoltHarPercentDataFmt,	sizeof(g_bVoltHarPercentDataFmt),	NULL},	//电压谐波含量
	{0x200d0500,	5,		MAP_SYSDB,		0x2606,	  PN0,   0,		g_bUnsignedDataFmt,	sizeof(g_bUnsignedDataFmt),	NULL},	//谐波次数
	{0x200e0200,	5,		MAP_SYSDB,		0x2603,	  PN0,   0,		g_bCurHarPercentDataFmt,	sizeof(g_bCurHarPercentDataFmt),	NULL},	//电流谐波含量
	{0x200e0300,	5,		MAP_SYSDB,		0x2604,	  PN0,   0,		g_bCurHarPercentDataFmt,	sizeof(g_bCurHarPercentDataFmt),	NULL},	//电流谐波含量
	{0x200e0400,	5,		MAP_SYSDB,		0x2605,	  PN0,   0,		g_bCurHarPercentDataFmt,	sizeof(g_bCurHarPercentDataFmt),	NULL},	//电流谐波含量
	{0x200e0500,	5,		MAP_SYSDB,		0x2606,   PN0,	 0, 	g_bUnsignedDataFmt, sizeof(g_bUnsignedDataFmt), NULL},	//谐波次数
	{0x200f0200,	6,		MAP_SYSDB,		0x200f,	  PN0,   0,		g_bFreqDataFmt,	sizeof(g_bFreqDataFmt),	NULL},	//电网频率

	{0x20110200,	4,		MAP_SYSDB,		0x2011,	  PN0,   0,		g_bRtcVolFmt, sizeof(g_bRtcVolFmt), NULL},	//时钟电池电压
	{0x20120200,	4,		MAP_SYSDB,		0x2012,   PN0,	 0, 	g_bRtcVolFmt, sizeof(g_bRtcVolFmt), NULL},	//停电抄表电池电压

	{0x20140200,	6,		MAP_SYSDB,		0x2014,   PN0,	 0, 	g_bMtrBlkRunStateFmt, sizeof(g_bMtrBlkRunStateFmt), NULL},	//电表运行状态字

	{0x20170200,	6,		MAP_SYSDB,		0x2017,	  PN0,   0,		g_bVarDmdFmt,				sizeof(g_bVarDmdFmt),		NULL},//当前有功需量
	{0x20180200,	6,		MAP_SYSDB,		0x2018,	  PN0,   0,		g_bVarDmdFmt,				sizeof(g_bVarDmdFmt),		NULL},//当前无功需量

	{0x201A0200,	6,		MAP_SYSDB,		0x201A,	  PN0,   0,		g_bPwrPrice,				sizeof(g_bPwrPrice),		NULL},//当前电价
	{0x201B0200,	6,		MAP_SYSDB,		0x201B,	  PN0,   0,		g_bPwrPrice,				sizeof(g_bPwrPrice),		NULL},//当前费率电价
	{0x201C0200,	6,		MAP_SYSDB,		0x201C,	  PN0,   0,		g_bPwrPrice,				sizeof(g_bPwrPrice),		NULL},//当前阶梯电价

	{0x201E0200,	8,		MAP_SYSDB,		0x201E,	  PN0,   0,		g_bEvtTimeFmt,				sizeof(g_bEvtTimeFmt),		NULL},//事件发生时间
	{0x20200200,	8,		MAP_SYSDB,		0x2020,	  PN0,   0,		g_bEvtTimeFmt,				sizeof(g_bEvtTimeFmt),		NULL},//事件结束时间
	{0x20210200,	8,		MAP_SYSDB,		0x2021,	  PN0,   0,		g_bFrzRecTimeFmt,			sizeof(g_bFrzRecTimeFmt),	NULL},//数据冻结时间
	{0x20220200,	8,		MAP_SYSDB,		0x2022,	  PN0,   0,		g_bEvtIndexFmt,				sizeof(g_bEvtIndexFmt),		NULL},//事件记录序号
	{0x20230200,	8,		MAP_SYSDB,		0x2023,	  PN0,   0,		g_bFrzRecIndexFmt,			sizeof(g_bFrzRecIndexFmt), 	 NULL},//冻结记录序号
	{0x20240200,	8,		MAP_SYSDB,		0x2024,	  PN0,   0,		NULL,						0,					  		NULL},//事件发生源,格式不定
	{0x20250200,	8,		MAP_SYSDB,		0x2025,	  PN0,   0,		g_bEvtCurValFmt,			sizeof(g_bEvtCurValFmt),	NULL},//事件当前值
	{0x20260200,	6,		MAP_SYSDB,		0x2026,	  PN0,   0,		g_bLongUTypeFmt,			sizeof(g_bLongUTypeFmt),					  		NULL},//电压不平衡率
	{0x20270200,	6,		MAP_SYSDB,		0x2027,	  PN0,   0,		g_bLongUTypeFmt,			sizeof(g_bLongUTypeFmt),					  		NULL},//电流不平衡率
	{0x20290200,	6,		MAP_SYSDB,		0x2029,	  PN0,   0,		g_bAhTypeFmt,			sizeof(g_bAhTypeFmt),					  		NULL},//安时值

	{0x202A0200,	8,		MAP_SYSDB,		0x202A,	  PN0,   0,		g_bServAddrFmt,				sizeof(g_bServAddrFmt),		NULL},//目的服务器地址


	{0x202C0200,	8,		MAP_SYSDB,		0x202C,	  PN0,   0,		g_bCurWalletFile,			sizeof(g_bCurWalletFile),	NULL},//（当前）钱包文件
	{0x202D0200,	6,		MAP_SYSDB,		0x202D,	  PN0,   0,		g_bCurOverRate,				sizeof(g_bCurOverRate),		NULL},//（当前）透支金额
	{0x202E0200,	6,		MAP_SYSDB,		0x202E,	  PN0,   0,		g_bPurchaseRate,			sizeof(g_bPurchaseRate),	NULL},//累计购电金额

	{0x21000200,	14,		MAP_SYSDB,		0x2108,	  PN0,	 0,		g_bRangeStatResFmt,		sizeof(g_bRangeStatResFmt),		NULL},//分钟区间统计数据
	{0x21000300,	14,		MAP_SYSDB,		0x2100,	  PN0,	 0,		g_bRangeStatFmt,		sizeof(g_bRangeStatFmt),		NULL},//分钟区间统计配置
	{0x21010200,	14,		MAP_SYSDB,		0x2109,	  PN0,	 0,		g_bRangeStatResFmt,		sizeof(g_bRangeStatResFmt),		NULL},//小时区间统计数据
	{0x21010300,	14,		MAP_SYSDB,		0x2101,	  PN0,	 0,		g_bRangeStatFmt,		sizeof(g_bRangeStatFmt),		NULL},//小时区间统计配置
	{0x21020200,	14,		MAP_SYSDB,		0x210a,	  PN0,	 0,		g_bRangeStatResFmt,		sizeof(g_bRangeStatResFmt),		NULL},//日区间统计数据
	{0x21020300,	14,		MAP_SYSDB,		0x2102,	  PN0,	 0,		g_bRangeStatFmt,		sizeof(g_bRangeStatFmt),		NULL},//日区间统计配置
	{0x21030200,	14,		MAP_SYSDB,		0x210b,	  PN0,	 0,		g_bRangeStatResFmt,		sizeof(g_bRangeStatResFmt),		NULL},//月区间统计数据
	{0x21030300,	14,		MAP_SYSDB,		0x2103,	  PN0,	 0,		g_bRangeStatFmt,		sizeof(g_bRangeStatFmt),		NULL},//月区间统计配置
	{0x21040200,	14,		MAP_SYSDB,		0x210c,	  PN0,	 0,		g_bRangeStatResFmt,		sizeof(g_bRangeStatResFmt),		NULL},//年区间统计数据
	{0x21040300,	14,		MAP_SYSDB,		0x2104,	  PN0,	 0,		g_bRangeStatFmt,		sizeof(g_bRangeStatFmt),		NULL},//年区间统计配置
	{0x21080200,	14,		MAP_SYSDB,		0x2108,	  PN0,	 0,		g_bRangeStatResFmt,		sizeof(g_bRangeStatResFmt),		NULL},//分钟区间统计数据
	{0x21090200,	14,		MAP_SYSDB,		0x2109,	  PN0,	 0,		g_bRangeStatResFmt,		sizeof(g_bRangeStatResFmt),		NULL},//分钟区间统计数据
	{0x210a0200,	14,		MAP_SYSDB,		0x210a,	  PN0,	 0,		g_bRangeStatResFmt,		sizeof(g_bRangeStatResFmt),		NULL},//日区间统计数据
	{0x210b0200,	14,		MAP_SYSDB,		0x210b,	  PN0,	 0,		g_bRangeStatResFmt,		sizeof(g_bRangeStatResFmt),		NULL},//月区间统计数据
	{0x210c0200,	14,		MAP_SYSDB,		0x210c,	  PN0,	 0,		g_bRangeStatResFmt,		sizeof(g_bRangeStatResFmt),		NULL},//年区间统计数据

	{0x21100200,	15,		MAP_SYSDB,		0x2118,	  PN0,	 0,		g_bAvrResFmt,			sizeof(g_bAvrResFmt),			NULL},//分钟平均统计数据
	{0x21100300,	15,		MAP_SYSDB,		0x2110,	  PN0,	 0,		g_bAvrStatFmt,			sizeof(g_bAvrStatFmt),			NULL},//分钟平均统计配置
	{0x21110200,	15,		MAP_SYSDB,		0x2119,	  PN0,	 0,		g_bAvrResFmt,			sizeof(g_bAvrResFmt),			NULL},//小时平均统计数据
	{0x21110300,	15,		MAP_SYSDB,		0x2111,	  PN0,	 0,		g_bAvrStatFmt,			sizeof(g_bAvrStatFmt),			NULL},//小时平均统计配置
	{0x21120200,	15,		MAP_SYSDB,		0x211a,	  PN0,	 0,		g_bAvrResFmt,			sizeof(g_bAvrResFmt),			NULL},//日平均统计数据
	{0x21120300,	15,		MAP_SYSDB,		0x2112,	  PN0,	 0,		g_bAvrStatFmt,			sizeof(g_bAvrStatFmt),			NULL},//日平均统计配置
	{0x21130200,	15,		MAP_SYSDB,		0x211b,	  PN0,	 0,		g_bAvrResFmt,			sizeof(g_bAvrResFmt),			NULL},//月平均统计数据
	{0x21130300,	15,		MAP_SYSDB,		0x2113,	  PN0,	 0,		g_bAvrStatFmt,			sizeof(g_bAvrStatFmt),			NULL},//月平均统计配置
	{0x21140200,	15,		MAP_SYSDB,		0x211c,	  PN0,	 0,		g_bAvrResFmt,			sizeof(g_bAvrResFmt),			NULL},//年平均统计数据
	{0x21140300,	15,		MAP_SYSDB,		0x2114,	  PN0,	 0,		g_bAvrStatFmt,			sizeof(g_bAvrStatFmt),			NULL},//年平均统计配置
	{0x21180200,	15,		MAP_SYSDB,		0x2118,	  PN0,	 0,		g_bAvrResFmt,			sizeof(g_bAvrResFmt),			NULL},//分钟平均统计数据
	{0x21190200,	15,		MAP_SYSDB,		0x2119,	  PN0,	 0,		g_bAvrResFmt,			sizeof(g_bAvrResFmt),			NULL},//小时平均统计数据
	{0x211a0200,	15,		MAP_SYSDB,		0x211a,	  PN0,	 0,		g_bAvrResFmt,			sizeof(g_bAvrResFmt),			NULL},//日平均统计数据
	{0x211b0200,	15,		MAP_SYSDB,		0x211b,	  PN0,	 0,		g_bAvrResFmt,			sizeof(g_bAvrResFmt),			NULL},//月平均统计数据
	{0x211c0200,	15,		MAP_SYSDB,		0x211c,	  PN0,	 0,		g_bAvrResFmt,			sizeof(g_bAvrResFmt),			NULL},//年平均统计数据

	{0x21200200,	15,		MAP_SYSDB,		0x2128,	  PN0,	 0,		g_bExtremResFmt,		sizeof(g_bExtremResFmt),		NULL},//分钟极值数据
	{0x21200300,	15,		MAP_SYSDB,		0x2120,	  PN0,	 0,		g_bAvrStatFmt,			sizeof(g_bAvrStatFmt),			NULL},//分钟极值配置
	{0x21210200,	15,		MAP_SYSDB,		0x2129,	  PN0,	 0,		g_bExtremResFmt,		sizeof(g_bExtremResFmt),		NULL},//小时极值数据
	{0x21210300,	15,		MAP_SYSDB,		0x2121,	  PN0,	 0,		g_bAvrStatFmt,			sizeof(g_bAvrStatFmt),			NULL},//小时极值配置
	{0x21220200,	15,		MAP_SYSDB,		0x212a,	  PN0,	 0,		g_bExtremResFmt,		sizeof(g_bExtremResFmt),		NULL},//日极值数据
	{0x21220300,	15,		MAP_SYSDB,		0x2122,	  PN0,	 0,		g_bAvrStatFmt,			sizeof(g_bAvrStatFmt),			NULL},//日极值配置
	{0x21230200,	15,		MAP_SYSDB,		0x212b,	  PN0,	 0,		g_bExtremResFmt,		sizeof(g_bExtremResFmt),		NULL},//月极值数据
	{0x21230300,	15,		MAP_SYSDB,		0x2123,	  PN0,	 0,		g_bAvrStatFmt,			sizeof(g_bAvrStatFmt),			NULL},//月极值配置
	{0x21240200,	15,		MAP_SYSDB,		0x212c,	  PN0,	 0,		g_bExtremResFmt,		sizeof(g_bExtremResFmt),		NULL},//年极值数据
	{0x21240300,	15,		MAP_SYSDB,		0x2124,	  PN0,	 0,		g_bAvrStatFmt,			sizeof(g_bAvrStatFmt),			NULL},//年极值配置
	{0x21280200,	15,		MAP_SYSDB,		0x2128,	  PN0,	 0,		g_bExtremResFmt,		sizeof(g_bExtremResFmt),		NULL},//分钟极值数据
	{0x21290200,	15,		MAP_SYSDB,		0x2129,	  PN0,	 0,		g_bExtremResFmt,		sizeof(g_bExtremResFmt),		NULL},//小时极值数据
	{0x212a0200,	15,		MAP_SYSDB,		0x212a,	  PN0,	 0,		g_bExtremResFmt,		sizeof(g_bExtremResFmt),		NULL},//日极值数据
	{0x212b0200,	15,		MAP_SYSDB,		0x212b,	  PN0,	 0,		g_bExtremResFmt,		sizeof(g_bExtremResFmt),		NULL},//月极值数据
	{0x212c0200,	15,		MAP_SYSDB,		0x212c,	  PN0,	 0,		g_bExtremResFmt,		sizeof(g_bExtremResFmt),		NULL},//年极值数据

	{0x21300200,	6,		MAP_SYSDB,		0x2130,	  PN0,	 0,		g_bVoltStatFmt,			sizeof(g_bVoltStatFmt),		NULL},//总电压合格率
	{0x21310200,	6,		MAP_SYSDB,		0x2131,	  PN0,	 0,		g_bVoltStatFmt,			sizeof(g_bVoltStatFmt),		NULL},//当月A相电压合格率
	{0x21320200,	6,		MAP_SYSDB,		0x2132,	  PN0,	 0,		g_bVoltStatFmt,			sizeof(g_bVoltStatFmt),		NULL},//当月B相电压合格率
	{0x21330200,	6,		MAP_SYSDB,		0x2133,	  PN0,	 0,		g_bVoltStatFmt,			sizeof(g_bVoltStatFmt),		NULL},//当月C相电压合格率

	{0x21400200,	6,		MAP_SYSDB,		0x2140,	  PN0,	 0,		g_bMaxPowerFmt,			sizeof(g_bMaxPowerFmt),		NULL},//日最大有功功率及发生时间
	{0x21410200,	6,		MAP_SYSDB,		0x2141,	  PN0,	 0,		g_bMaxPowerFmt,			sizeof(g_bMaxPowerFmt),		NULL},//月最大有功功率及发生时间

	{0x22000200,	6,		MAP_SYSDB,		0x2200,	  PN0,	 0,		g_bOnTimeFmt,			sizeof(g_bOnTimeFmt),		NULL},//通信流量
	{0x22030200,	6,		MAP_SYSDB,		0x2203,	  PN0,	 0,		g_bOnTimeFmt,			sizeof(g_bOnTimeFmt),		NULL},//供电时间
	{0x22040200,	6,		MAP_SYSDB,		0x2204,	  PN0,	 0,		g_bResetTimesFmt,		sizeof(g_bResetTimesFmt),	NULL},//复位次数

	{0x23010200,	23,		MAP_SYSDB,		0x2301,	  PN1,	 0,		g_bGrpCfgFmt,			sizeof(g_bGrpCfgFmt),		NULL},//总加配置表 支持4个测量点配置
	{0x23010300,	23,		MAP_SYSDB,		0x2302,	  PN1,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加有功功率
	{0x23010400,	23,		MAP_SYSDB,		0x2303,	  PN1,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加无功功率
	{0x23010500,	23,		MAP_SYSDB,		0x2304,	  PN1,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均有功功率
	{0x23010600,	23,		MAP_SYSDB,		0x2305,	  PN1,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均无功功率
	{0x23010700,	23,		MAP_SYSDB,		0x2306,	  PN1,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向有功电量
	{0x23010800,	23,		MAP_SYSDB,		0x2307,	  PN1,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向无功电量
	{0x23010900,	23,		MAP_SYSDB,		0x2308,	  PN1,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向有功电量
	{0x23010a00,	23,		MAP_SYSDB,		0x2309,	  PN1,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向无功电量
	{0x23010b00,	23,		MAP_SYSDB,		0x230a,	  PN1,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加剩余电量（费）
	{0x23010c00,	23,		MAP_SYSDB,		0x230b,	  PN1,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//当前功率下浮控控后总加有功功率冻结值
	{0x23010d00,	23,		MAP_SYSDB,		0x230c,	  PN1,	 0,		g_bGrpSlipIntervFmt,	sizeof(g_bGrpSlipIntervFmt),NULL},//总加组滑差时间周期
	{0x23010e00,	23,		MAP_SYSDB,		0x230d,	  PN1,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组功控轮次配置
	{0x23010f00,	23,		MAP_SYSDB,		0x230e,	  PN1,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组电控轮次配置
	{0x23011000,	23,		MAP_SYSDB,		0x230f,	  PN1,	 0,		g_bGrpCtrlStaCfgFmt,	sizeof(g_bGrpCtrlStaCfgFmt),NULL},//总加组控制设置状态
	{0x23011100,	23,		MAP_SYSDB,		0x2310,	  PN1,	 0,		g_bGrpCtrlStaCurFmt,	sizeof(g_bGrpCtrlStaCurFmt),NULL},//总加组当前控制状态
	{0x23011200,	23,		MAP_SYSDB,		0x2311,	  PN1,	 0,		g_bGrpDataUnitFmt,		sizeof(g_bGrpDataUnitFmt),	NULL},//换算及单位

	{0x23020200,	23,		MAP_SYSDB,		0x2301,	  PN2,	 0,		g_bGrpCfgFmt,			sizeof(g_bGrpCfgFmt),		NULL},//总加配置表 支持4个测量点配置
	{0x23020300,	23,		MAP_SYSDB,		0x2302,	  PN2,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加有功功率
	{0x23020400,	23,		MAP_SYSDB,		0x2303,	  PN2,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加无功功率
	{0x23020500,	23,		MAP_SYSDB,		0x2304,	  PN2,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均有功功率
	{0x23020600,	23,		MAP_SYSDB,		0x2305,	  PN2,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均无功功率
	{0x23020700,	23,		MAP_SYSDB,		0x2306,	  PN2,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向有功电量
	{0x23020800,	23,		MAP_SYSDB,		0x2307,	  PN2,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向无功电量
	{0x23020900,	23,		MAP_SYSDB,		0x2308,	  PN2,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向有功电量
	{0x23020a00,	23,		MAP_SYSDB,		0x2309,	  PN2,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向无功电量
	{0x23020b00,	23,		MAP_SYSDB,		0x230a,	  PN2,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加剩余电量（费）
	{0x23020c00,	23,		MAP_SYSDB,		0x230b,	  PN2,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//当前功率下浮控控后总加有功功率冻结值
	{0x23020d00,	23,		MAP_SYSDB,		0x230c,	  PN2,	 0,		g_bGrpSlipIntervFmt,	sizeof(g_bGrpSlipIntervFmt),NULL},//总加组滑差时间周期
	{0x23020e00,	23,		MAP_SYSDB,		0x230d,	  PN2,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组功控轮次配置
	{0x23020f00,	23,		MAP_SYSDB,		0x230e,	  PN2,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组电控轮次配置
	{0x23021000,	23,		MAP_SYSDB,		0x230f,	  PN2,	 0,		g_bGrpCtrlStaCfgFmt,	sizeof(g_bGrpCtrlStaCfgFmt),NULL},//总加组控制设置状态
	{0x23021100,	23,		MAP_SYSDB,		0x2310,	  PN2,	 0,		g_bGrpCtrlStaCurFmt,	sizeof(g_bGrpCtrlStaCurFmt),NULL},//总加组当前控制状态
	{0x23021200,	23,		MAP_SYSDB,		0x2311,	  PN2,	 0,		g_bGrpDataUnitFmt,		sizeof(g_bGrpDataUnitFmt),	NULL},//换算及单位

	{0x23030200,	23,		MAP_SYSDB,		0x2301,	  PN3,	 0,		g_bGrpCfgFmt,			sizeof(g_bGrpCfgFmt),		NULL},//总加配置表 支持4个测量点配置
	{0x23030300,	23,		MAP_SYSDB,		0x2302,	  PN3,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加有功功率
	{0x23030400,	23,		MAP_SYSDB,		0x2303,	  PN3,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加无功功率
	{0x23030500,	23,		MAP_SYSDB,		0x2304,	  PN3,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均有功功率
	{0x23030600,	23,		MAP_SYSDB,		0x2305,	  PN3,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均无功功率
	{0x23030700,	23,		MAP_SYSDB,		0x2306,	  PN3,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向有功电量
	{0x23030800,	23,		MAP_SYSDB,		0x2307,	  PN3,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向无功电量
	{0x23030900,	23,		MAP_SYSDB,		0x2308,	  PN3,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向有功电量
	{0x23030a00,	23,		MAP_SYSDB,		0x2309,	  PN3,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向无功电量
	{0x23030b00,	23,		MAP_SYSDB,		0x230a,	  PN3,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加剩余电量（费）
	{0x23030c00,	23,		MAP_SYSDB,		0x230b,	  PN3,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//当前功率下浮控控后总加有功功率冻结值
	{0x23030d00,	23,		MAP_SYSDB,		0x230c,	  PN3,	 0,		g_bGrpSlipIntervFmt,	sizeof(g_bGrpSlipIntervFmt),NULL},//总加组滑差时间周期
	{0x23030e00,	23,		MAP_SYSDB,		0x230d,	  PN3,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组功控轮次配置
	{0x23030f00,	23,		MAP_SYSDB,		0x230e,	  PN3,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组电控轮次配置
	{0x23031000,	23,		MAP_SYSDB,		0x230f,	  PN3,	 0,		g_bGrpCtrlStaCfgFmt,	sizeof(g_bGrpCtrlStaCfgFmt),NULL},//总加组控制设置状态
	{0x23031100,	23,		MAP_SYSDB,		0x2310,	  PN3,	 0,		g_bGrpCtrlStaCurFmt,	sizeof(g_bGrpCtrlStaCurFmt),NULL},//总加组当前控制状态
	{0x23031200,	23,		MAP_SYSDB,		0x2311,	  PN3,	 0,		g_bGrpDataUnitFmt,		sizeof(g_bGrpDataUnitFmt),	NULL},//换算及单位

	{0x23040200,	23,		MAP_SYSDB,		0x2301,	  PN4,	 0,		g_bGrpCfgFmt,			sizeof(g_bGrpCfgFmt),		NULL},//总加配置表 支持4个测量点配置
	{0x23040300,	23,		MAP_SYSDB,		0x2302,	  PN4,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加有功功率
	{0x23040400,	23,		MAP_SYSDB,		0x2303,	  PN4,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加无功功率
	{0x23040500,	23,		MAP_SYSDB,		0x2304,	  PN4,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均有功功率
	{0x23040600,	23,		MAP_SYSDB,		0x2305,	  PN4,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均无功功率
	{0x23040700,	23,		MAP_SYSDB,		0x2306,	  PN4,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向有功电量
	{0x23040800,	23,		MAP_SYSDB,		0x2307,	  PN4,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向无功电量
	{0x23040900,	23,		MAP_SYSDB,		0x2308,	  PN4,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向有功电量
	{0x23040a00,	23,		MAP_SYSDB,		0x2309,	  PN4,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向无功电量
	{0x23040b00,	23,		MAP_SYSDB,		0x230a,	  PN4,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加剩余电量（费）
	{0x23040c00,	23,		MAP_SYSDB,		0x230b,	  PN4,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//当前功率下浮控控后总加有功功率冻结值
	{0x23040d00,	23,		MAP_SYSDB,		0x230c,	  PN4,	 0,		g_bGrpSlipIntervFmt,	sizeof(g_bGrpSlipIntervFmt),NULL},//总加组滑差时间周期
	{0x23040e00,	23,		MAP_SYSDB,		0x230d,	  PN4,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组功控轮次配置
	{0x23040f00,	23,		MAP_SYSDB,		0x230e,	  PN4,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组电控轮次配置
	{0x23041000,	23,		MAP_SYSDB,		0x230f,	  PN4,	 0,		g_bGrpCtrlStaCfgFmt,	sizeof(g_bGrpCtrlStaCfgFmt),NULL},//总加组控制设置状态
	{0x23041100,	23,		MAP_SYSDB,		0x2310,	  PN4,	 0,		g_bGrpCtrlStaCurFmt,	sizeof(g_bGrpCtrlStaCurFmt),NULL},//总加组当前控制状态
	{0x23041200,	23,		MAP_SYSDB,		0x2311,	  PN4,	 0,		g_bGrpDataUnitFmt,		sizeof(g_bGrpDataUnitFmt),	NULL},//换算及单位

	{0x23050200,	23,		MAP_SYSDB,		0x2301,	  PN5,	 0,		g_bGrpCfgFmt,			sizeof(g_bGrpCfgFmt),		NULL},//总加配置表 支持4个测量点配置
	{0x23050300,	23,		MAP_SYSDB,		0x2302,	  PN5,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加有功功率
	{0x23050400,	23,		MAP_SYSDB,		0x2303,	  PN5,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加无功功率
	{0x23050500,	23,		MAP_SYSDB,		0x2304,	  PN5,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均有功功率
	{0x23050600,	23,		MAP_SYSDB,		0x2305,	  PN5,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均无功功率
	{0x23050700,	23,		MAP_SYSDB,		0x2306,	  PN5,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向有功电量
	{0x23050800,	23,		MAP_SYSDB,		0x2307,	  PN5,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向无功电量
	{0x23050900,	23,		MAP_SYSDB,		0x2308,	  PN5,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向有功电量
	{0x23050a00,	23,		MAP_SYSDB,		0x2309,	  PN5,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向无功电量
	{0x23050b00,	23,		MAP_SYSDB,		0x230a,	  PN5,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加剩余电量（费）
	{0x23050c00,	23,		MAP_SYSDB,		0x230b,	  PN5,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//当前功率下浮控控后总加有功功率冻结值
	{0x23050d00,	23,		MAP_SYSDB,		0x230c,	  PN5,	 0,		g_bGrpSlipIntervFmt,	sizeof(g_bGrpSlipIntervFmt),NULL},//总加组滑差时间周期
	{0x23050e00,	23,		MAP_SYSDB,		0x230d,	  PN5,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组功控轮次配置
	{0x23050f00,	23,		MAP_SYSDB,		0x230e,	  PN5,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组电控轮次配置
	{0x23051000,	23,		MAP_SYSDB,		0x230f,	  PN5,	 0,		g_bGrpCtrlStaCfgFmt,	sizeof(g_bGrpCtrlStaCfgFmt),NULL},//总加组控制设置状态
	{0x23051100,	23,		MAP_SYSDB,		0x2310,	  PN5,	 0,		g_bGrpCtrlStaCurFmt,	sizeof(g_bGrpCtrlStaCurFmt),NULL},//总加组当前控制状态
	{0x23051200,	23,		MAP_SYSDB,		0x2311,	  PN5,	 0,		g_bGrpDataUnitFmt,		sizeof(g_bGrpDataUnitFmt),	NULL},//换算及单位

	{0x23060200,	23,		MAP_SYSDB,		0x2301,	  PN6,	 0,		g_bGrpCfgFmt,			sizeof(g_bGrpCfgFmt),		NULL},//总加配置表 支持4个测量点配置
	{0x23060300,	23,		MAP_SYSDB,		0x2302,	  PN6,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加有功功率
	{0x23060400,	23,		MAP_SYSDB,		0x2303,	  PN6,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加无功功率
	{0x23060500,	23,		MAP_SYSDB,		0x2304,	  PN6,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均有功功率
	{0x23060600,	23,		MAP_SYSDB,		0x2305,	  PN6,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均无功功率
	{0x23060700,	23,		MAP_SYSDB,		0x2306,	  PN6,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向有功电量
	{0x23060800,	23,		MAP_SYSDB,		0x2307,	  PN6,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向无功电量
	{0x23060900,	23,		MAP_SYSDB,		0x2308,	  PN6,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向有功电量
	{0x23060a00,	23,		MAP_SYSDB,		0x2309,	  PN6,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向无功电量
	{0x23060b00,	23,		MAP_SYSDB,		0x230a,	  PN6,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加剩余电量（费）
	{0x23060c00,	23,		MAP_SYSDB,		0x230b,	  PN6,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//当前功率下浮控控后总加有功功率冻结值
	{0x23060d00,	23,		MAP_SYSDB,		0x230c,	  PN6,	 0,		g_bGrpSlipIntervFmt,	sizeof(g_bGrpSlipIntervFmt),NULL},//总加组滑差时间周期
	{0x23060e00,	23,		MAP_SYSDB,		0x230d,	  PN6,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组功控轮次配置
	{0x23060f00,	23,		MAP_SYSDB,		0x230e,	  PN6,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组电控轮次配置
	{0x23061000,	23,		MAP_SYSDB,		0x230f,	  PN6,	 0,		g_bGrpCtrlStaCfgFmt,	sizeof(g_bGrpCtrlStaCfgFmt),NULL},//总加组控制设置状态
	{0x23061100,	23,		MAP_SYSDB,		0x2310,	  PN6,	 0,		g_bGrpCtrlStaCurFmt,	sizeof(g_bGrpCtrlStaCurFmt),NULL},//总加组当前控制状态
	{0x23061200,	23,		MAP_SYSDB,		0x2311,	  PN6,	 0,		g_bGrpDataUnitFmt,		sizeof(g_bGrpDataUnitFmt),	NULL},//换算及单位

	{0x23070200,	23,		MAP_SYSDB,		0x2301,	  PN7,	 0,		g_bGrpCfgFmt,			sizeof(g_bGrpCfgFmt),		NULL},//总加配置表 支持4个测量点配置
	{0x23070300,	23,		MAP_SYSDB,		0x2302,	  PN7,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加有功功率
	{0x23070400,	23,		MAP_SYSDB,		0x2303,	  PN7,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加无功功率
	{0x23070500,	23,		MAP_SYSDB,		0x2304,	  PN7,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均有功功率
	{0x23070600,	23,		MAP_SYSDB,		0x2305,	  PN7,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均无功功率
	{0x23070700,	23,		MAP_SYSDB,		0x2306,	  PN7,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向有功电量
	{0x23070800,	23,		MAP_SYSDB,		0x2307,	  PN7,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向无功电量
	{0x23070900,	23,		MAP_SYSDB,		0x2308,	  PN7,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向有功电量
	{0x23070a00,	23,		MAP_SYSDB,		0x2309,	  PN7,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向无功电量
	{0x23070b00,	23,		MAP_SYSDB,		0x230a,	  PN7,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加剩余电量（费）
	{0x23070c00,	23,		MAP_SYSDB,		0x230b,	  PN7,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//当前功率下浮控控后总加有功功率冻结值
	{0x23070d00,	23,		MAP_SYSDB,		0x230c,	  PN7,	 0,		g_bGrpSlipIntervFmt,	sizeof(g_bGrpSlipIntervFmt),NULL},//总加组滑差时间周期
	{0x23070e00,	23,		MAP_SYSDB,		0x230d,	  PN7,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组功控轮次配置
	{0x23070f00,	23,		MAP_SYSDB,		0x230e,	  PN7,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组电控轮次配置
	{0x23071000,	23,		MAP_SYSDB,		0x230f,	  PN7,	 0,		g_bGrpCtrlStaCfgFmt,	sizeof(g_bGrpCtrlStaCfgFmt),NULL},//总加组控制设置状态
	{0x23071100,	23,		MAP_SYSDB,		0x2310,	  PN7,	 0,		g_bGrpCtrlStaCurFmt,	sizeof(g_bGrpCtrlStaCurFmt),NULL},//总加组当前控制状态
	{0x23071200,	23,		MAP_SYSDB,		0x2311,	  PN7,	 0,		g_bGrpDataUnitFmt,		sizeof(g_bGrpDataUnitFmt),	NULL},//换算及单位

	{0x23080200,	23,		MAP_SYSDB,		0x2301,	  PN8,	 0,		g_bGrpCfgFmt,			sizeof(g_bGrpCfgFmt),		NULL},//总加配置表 支持4个测量点配置
	{0x23080300,	23,		MAP_SYSDB,		0x2302,	  PN8,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加有功功率
	{0x23080400,	23,		MAP_SYSDB,		0x2303,	  PN8,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加无功功率
	{0x23080500,	23,		MAP_SYSDB,		0x2304,	  PN8,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均有功功率
	{0x23080600,	23,		MAP_SYSDB,		0x2305,	  PN8,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加滑差时间内平均无功功率
	{0x23080700,	23,		MAP_SYSDB,		0x2306,	  PN8,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向有功电量
	{0x23080800,	23,		MAP_SYSDB,		0x2307,	  PN8,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加日正向无功电量
	{0x23080900,	23,		MAP_SYSDB,		0x2308,	  PN8,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向有功电量
	{0x23080a00,	23,		MAP_SYSDB,		0x2309,	  PN8,	 0,		g_bGrpEngFmt,			sizeof(g_bGrpEngFmt),		NULL},//总加月正向无功电量
	{0x23080b00,	23,		MAP_SYSDB,		0x230a,	  PN8,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//总加剩余电量（费）
	{0x23080c00,	23,		MAP_SYSDB,		0x230b,	  PN8,	 0,		g_bGrpPowFmt,			sizeof(g_bGrpPowFmt),		NULL},//当前功率下浮控控后总加有功功率冻结值
	{0x23080d00,	23,		MAP_SYSDB,		0x230c,	  PN8,	 0,		g_bGrpSlipIntervFmt,	sizeof(g_bGrpSlipIntervFmt),NULL},//总加组滑差时间周期
	{0x23080e00,	23,		MAP_SYSDB,		0x230d,	  PN8,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组功控轮次配置
	{0x23080f00,	23,		MAP_SYSDB,		0x230e,	  PN8,	 0,		g_bGrpTurnFmt,			sizeof(g_bGrpTurnFmt),		NULL},//总加组电控轮次配置
	{0x23081000,	23,		MAP_SYSDB,		0x230f,	  PN8,	 0,		g_bGrpCtrlStaCfgFmt,	sizeof(g_bGrpCtrlStaCfgFmt),NULL},//总加组控制设置状态
	{0x23081100,	23,		MAP_SYSDB,		0x2310,	  PN8,	 0,		g_bGrpCtrlStaCurFmt,	sizeof(g_bGrpCtrlStaCurFmt),NULL},//总加组当前控制状态
	{0x23081200,	23,		MAP_SYSDB,		0x2311,	  PN8,	 0,		g_bGrpDataUnitFmt,		sizeof(g_bGrpDataUnitFmt),	NULL},//换算及单位


	{0x24010200,	12,		MAP_SYSDB,		0x2401,	  	  PN0,	 0,		g_bCommAddrFmt,		sizeof(g_bCommAddrFmt),		NULL},	//通信地址  ---- (脉冲计量接口类)
	{0x24010300,	12,		MAP_SYSDB,		0x2402,	  	  PN0,	 0,		g_bPTCTFmt,			sizeof(g_bPTCTFmt),			NULL},	//互感器倍率
	{0x24010400,	12,		MAP_SYSDB,		0x2403,	  	  PN0,	 0,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		NULL},	//脉冲配置
	{0x24010500,	12,		MAP_SYSDB,		0x2404,	  	  PN0,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//有功功率
	{0x24010600,	12,		MAP_SYSDB,		0x2405,	  	  PN0,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//无功功率
	{0x24010700,	12,		MAP_SYSDB,		0x2406,	  	  PN0,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向有功电量
	{0x24010800,	12,		MAP_SYSDB,		0x2407,	  	  PN0,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向有功电量
	{0x24010900,	12,		MAP_SYSDB,		0x2408,	  	  PN0,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向有功电量
	{0x24010a00,	12,		MAP_SYSDB,		0x2409,	  	  PN0,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向有功电量
	{0x24010b00,	12,		MAP_SYSDB,		0x2410,	  	  PN0,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向无功电量
	{0x24010c00,	12,		MAP_SYSDB,		0x2411,	  	  PN0,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向无功电量
	{0x24010d00,	12,		MAP_SYSDB,		0x2412,	  	  PN0,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向无功电量
	{0x24010e00,	12,		MAP_SYSDB,		0x2413,	  	  PN0,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向无功电量
	{0x24010f00,	12,		MAP_SYSDB,		0x2419,	  	  PN0,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向有功电能示值
	{0x24011000,	12,		MAP_SYSDB,		0x241a,	  	  PN0,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向无功电能示值
	{0x24011100,	12,		MAP_SYSDB,		0x241b,	  	  PN0,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向有功电能示值
	{0x24011200,	12,		MAP_SYSDB,		0x241c,	  	  PN0,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向无功电能示值
	{0x24011300,	12,		MAP_SYSDB,		0x2418,	  	  PN0,   0,		g_bPulseDataUnitFmt,	sizeof(g_bPulseDataUnitFmt),	NULL},	//换算及单位 ---- (脉冲计量点1)

	{0x24020200,	12,		MAP_SYSDB,		0x2401,	  	  PN1,	 0,		g_bCommAddrFmt,		sizeof(g_bCommAddrFmt),		NULL},	//通信地址  ---- (脉冲计量接口类)
	{0x24020300,	12,		MAP_SYSDB,		0x2402,	  	  PN1,	 0,		g_bPTCTFmt,			sizeof(g_bPTCTFmt),			NULL},	//互感器倍率
	{0x24020400,	12,		MAP_SYSDB,		0x2403,	  	  PN1,	 0,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		NULL},	//脉冲配置
	{0x24020500,	12,		MAP_SYSDB,		0x2404,	  	  PN1,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//有功功率
	{0x24020600,	12,		MAP_SYSDB,		0x2405,	  	  PN1,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//无功功率
	{0x24020700,	12,		MAP_SYSDB,		0x2406,	  	  PN1,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向有功电量
	{0x24020800,	12,		MAP_SYSDB,		0x2407,	  	  PN1,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向有功电量
	{0x24020900,	12,		MAP_SYSDB,		0x2408,	  	  PN1,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向有功电量
	{0x24020a00,	12,		MAP_SYSDB,		0x2409,	  	  PN1,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向有功电量
	{0x24020b00,	12,		MAP_SYSDB,		0x2410,	  	  PN1,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向无功电量
	{0x24020c00,	12,		MAP_SYSDB,		0x2411,	  	  PN1,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向无功电量
	{0x24020d00,	12,		MAP_SYSDB,		0x2412,	  	  PN1,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向无功电量
	{0x24020e00,	12,		MAP_SYSDB,		0x2413,	  	  PN1,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向无功电量
	{0x24020f00,	12,		MAP_SYSDB,		0x2419,	  	  PN1,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向有功电能示值
	{0x24021000,	12,		MAP_SYSDB,		0x241a,	  	  PN1,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向无功电能示值
	{0x24021100,	12,		MAP_SYSDB,		0x241b,	  	  PN1,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向有功电能示值
	{0x24021200,	12,		MAP_SYSDB,		0x241c,	  	  PN1,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向无功电能示值
	{0x24021300,	12,		MAP_SYSDB,		0x2418,	  	  PN1,   0,		g_bPulseDataUnitFmt,	sizeof(g_bPulseDataUnitFmt),	NULL},	//换算及单位 ---- (脉冲计量点2)

	{0x24030200,	12,		MAP_SYSDB,		0x2401,	  	  PN2,	 0,		g_bCommAddrFmt,		sizeof(g_bCommAddrFmt),		NULL},	//通信地址  ---- (脉冲计量接口类)
	{0x24030300,	12,		MAP_SYSDB,		0x2402,	  	  PN2,	 0,		g_bPTCTFmt,			sizeof(g_bPTCTFmt),			NULL},	//互感器倍率
	{0x24030400,	12,		MAP_SYSDB,		0x2403,	  	  PN2,	 0,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		NULL},	//脉冲配置
	{0x24030500,	12,		MAP_SYSDB,		0x2404,	  	  PN2,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//有功功率
	{0x24030600,	12,		MAP_SYSDB,		0x2405,	  	  PN2,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//无功功率
	{0x24030700,	12,		MAP_SYSDB,		0x2406,	  	  PN2,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向有功电量
	{0x24030800,	12,		MAP_SYSDB,		0x2407,	  	  PN2,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向有功电量
	{0x24030900,	12,		MAP_SYSDB,		0x2408,	  	  PN2,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向有功电量
	{0x24030a00,	12,		MAP_SYSDB,		0x2409,	  	  PN2,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向有功电量
	{0x24030b00,	12,		MAP_SYSDB,		0x2410,	  	  PN2,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向无功电量
	{0x24030c00,	12,		MAP_SYSDB,		0x2411,	  	  PN2,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向无功电量
	{0x24030d00,	12,		MAP_SYSDB,		0x2412,	  	  PN2,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向无功电量
	{0x24030e00,	12,		MAP_SYSDB,		0x2413,	  	  PN2,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向无功电量
	{0x24030f00,	12,		MAP_SYSDB,		0x2419,	  	  PN2,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向有功电能示值
	{0x24031000,	12,		MAP_SYSDB,		0x241a,	  	  PN2,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向无功电能示值
	{0x24031100,	12,		MAP_SYSDB,		0x241b,	  	  PN2,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向有功电能示值
	{0x24031200,	12,		MAP_SYSDB,		0x241c,	  	  PN2,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向无功电能示值
	{0x24031300,	12,		MAP_SYSDB,		0x2418,	  	  PN2,   0,		g_bPulseDataUnitFmt,	sizeof(g_bPulseDataUnitFmt),	NULL},	//换算及单位 ---- (脉冲计量点3)

	{0x24040200,	12,		MAP_SYSDB,		0x2401,	  	  PN3,	 0,		g_bCommAddrFmt,		sizeof(g_bCommAddrFmt),		NULL},	//通信地址  ---- (脉冲计量接口类)
	{0x24040300,	12,		MAP_SYSDB,		0x2402,	  	  PN3,	 0,		g_bPTCTFmt,			sizeof(g_bPTCTFmt),			NULL},	//互感器倍率
	{0x24040400,	12,		MAP_SYSDB,		0x2403,	  	  PN3,	 0,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		NULL},	//脉冲配置
	{0x24040500,	12,		MAP_SYSDB,		0x2404,	  	  PN3,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//有功功率
	{0x24040600,	12,		MAP_SYSDB,		0x2405,	  	  PN3,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//无功功率
	{0x24040700,	12,		MAP_SYSDB,		0x2406,	  	  PN3,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向有功电量
	{0x24040800,	12,		MAP_SYSDB,		0x2407,	  	  PN3,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向有功电量
	{0x24040900,	12,		MAP_SYSDB,		0x2408,	  	  PN3,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向有功电量
	{0x24040a00,	12,		MAP_SYSDB,		0x2409,	  	  PN3,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向有功电量
	{0x24040b00,	12,		MAP_SYSDB,		0x2410,	  	  PN3,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向无功电量
	{0x24040c00,	12,		MAP_SYSDB,		0x2411,	  	  PN3,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向无功电量
	{0x24040d00,	12,		MAP_SYSDB,		0x2412,	  	  PN3,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向无功电量
	{0x24040e00,	12,		MAP_SYSDB,		0x2413,	  	  PN3,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向无功电量
	{0x24040f00,	12,		MAP_SYSDB,		0x2419,	  	  PN3,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向有功电能示值
	{0x24041000,	12,		MAP_SYSDB,		0x241a,	  	  PN3,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向无功电能示值
	{0x24041100,	12,		MAP_SYSDB,		0x241b,	  	  PN3,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向有功电能示值
	{0x24041200,	12,		MAP_SYSDB,		0x241c,	  	  PN3,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向无功电能示值
	{0x24041300,	12,		MAP_SYSDB,		0x2418,	  	  PN3,   0,		g_bPulseDataUnitFmt,	sizeof(g_bPulseDataUnitFmt),	NULL},	//换算及单位 ---- (脉冲计量点4)

	{0x24050200,	12,		MAP_SYSDB,		0x2401,	  	  PN4,	 0,		g_bCommAddrFmt,		sizeof(g_bCommAddrFmt),		NULL},	//通信地址  ---- (脉冲计量接口类)
	{0x24050300,	12,		MAP_SYSDB,		0x2402,	  	  PN4,	 0,		g_bPTCTFmt,			sizeof(g_bPTCTFmt),			NULL},	//互感器倍率
	{0x24050400,	12,		MAP_SYSDB,		0x2403,	  	  PN4,	 0,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		NULL},	//脉冲配置
	{0x24050500,	12,		MAP_SYSDB,		0x2404,	  	  PN4,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//有功功率
	{0x24050600,	12,		MAP_SYSDB,		0x2405,	  	  PN4,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//无功功率
	{0x24050700,	12,		MAP_SYSDB,		0x2406,	  	  PN4,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向有功电量
	{0x24050800,	12,		MAP_SYSDB,		0x2407,	  	  PN4,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向有功电量
	{0x24050900,	12,		MAP_SYSDB,		0x2408,	  	  PN4,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向有功电量
	{0x24050a00,	12,		MAP_SYSDB,		0x2409,	  	  PN4,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向有功电量
	{0x24050b00,	12,		MAP_SYSDB,		0x2410,	  	  PN4,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向无功电量
	{0x24050c00,	12,		MAP_SYSDB,		0x2411,	  	  PN4,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向无功电量
	{0x24050d00,	12,		MAP_SYSDB,		0x2412,	  	  PN4,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向无功电量
	{0x24050e00,	12,		MAP_SYSDB,		0x2413,	  	  PN4,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向无功电量
	{0x24050f00,	12,		MAP_SYSDB,		0x2419,	  	  PN4,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向有功电能示值
	{0x24051000,	12,		MAP_SYSDB,		0x241a,	  	  PN4,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向无功电能示值
	{0x24051100,	12,		MAP_SYSDB,		0x241b,	  	  PN4,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向有功电能示值
	{0x24051200,	12,		MAP_SYSDB,		0x241c,	  	  PN4,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向无功电能示值
	{0x24051300,	12,		MAP_SYSDB,		0x2418,	  	  PN4,   0,		g_bPulseDataUnitFmt,	sizeof(g_bPulseDataUnitFmt),	NULL},	//换算及单位 ---- (脉冲计量点5)

	{0x24060200,	12,		MAP_SYSDB,		0x2401,	  	  PN5,	 0,		g_bCommAddrFmt,		sizeof(g_bCommAddrFmt),		NULL},	//通信地址  ---- (脉冲计量接口类)
	{0x24060300,	12,		MAP_SYSDB,		0x2402,	  	  PN5,	 0,		g_bPTCTFmt,			sizeof(g_bPTCTFmt),			NULL},	//互感器倍率
	{0x24060400,	12,		MAP_SYSDB,		0x2403,	  	  PN5,	 0,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		NULL},	//脉冲配置
	{0x24060500,	12,		MAP_SYSDB,		0x2404,	  	  PN5,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//有功功率
	{0x24060600,	12,		MAP_SYSDB,		0x2405,	  	  PN5,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//无功功率
	{0x24060700,	12,		MAP_SYSDB,		0x2406,	  	  PN5,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向有功电量
	{0x24060800,	12,		MAP_SYSDB,		0x2407,	  	  PN5,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向有功电量
	{0x24060900,	12,		MAP_SYSDB,		0x2408,	  	  PN5,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向有功电量
	{0x24060a00,	12,		MAP_SYSDB,		0x2409,	  	  PN5,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向有功电量
	{0x24060b00,	12,		MAP_SYSDB,		0x2410,	  	  PN5,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向无功电量
	{0x24060c00,	12,		MAP_SYSDB,		0x2411,	  	  PN5,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向无功电量
	{0x24060d00,	12,		MAP_SYSDB,		0x2412,	  	  PN5,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向无功电量
	{0x24060e00,	12,		MAP_SYSDB,		0x2413,	  	  PN5,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向无功电量
	{0x24060f00,	12,		MAP_SYSDB,		0x2419,	  	  PN5,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向有功电能示值
	{0x24061000,	12,		MAP_SYSDB,		0x241a,	  	  PN5,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向无功电能示值
	{0x24061100,	12,		MAP_SYSDB,		0x241b,	  	  PN5,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向有功电能示值
	{0x24061200,	12,		MAP_SYSDB,		0x241c,	  	  PN5,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向无功电能示值
	{0x24061300,	12,		MAP_SYSDB,		0x2418,	  	  PN5,   0,		g_bPulseDataUnitFmt,	sizeof(g_bPulseDataUnitFmt),	NULL},	//换算及单位 ---- (脉冲计量点6)

	{0x24070200,	12,		MAP_SYSDB,		0x2401,	  	  PN6,	 0,		g_bCommAddrFmt,		sizeof(g_bCommAddrFmt),		NULL},	//通信地址  ---- (脉冲计量接口类)
	{0x24070300,	12,		MAP_SYSDB,		0x2402,	  	  PN6,	 0,		g_bPTCTFmt,			sizeof(g_bPTCTFmt),			NULL},	//互感器倍率
	{0x24070400,	12,		MAP_SYSDB,		0x2403,	  	  PN6,	 0,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		NULL},	//脉冲配置
	{0x24070500,	12,		MAP_SYSDB,		0x2404,	  	  PN6,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//有功功率
	{0x24070600,	12,		MAP_SYSDB,		0x2405,	  	  PN6,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//无功功率
	{0x24070700,	12,		MAP_SYSDB,		0x2406,	  	  PN6,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向有功电量
	{0x24070800,	12,		MAP_SYSDB,		0x2407,	  	  PN6,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向有功电量
	{0x24070900,	12,		MAP_SYSDB,		0x2408,	  	  PN6,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向有功电量
	{0x24070a00,	12,		MAP_SYSDB,		0x2409,	  	  PN6,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向有功电量
	{0x24070b00,	12,		MAP_SYSDB,		0x2410,	  	  PN6,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向无功电量
	{0x24070c00,	12,		MAP_SYSDB,		0x2411,	  	  PN6,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向无功电量
	{0x24070d00,	12,		MAP_SYSDB,		0x2412,	  	  PN6,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向无功电量
	{0x24070e00,	12,		MAP_SYSDB,		0x2413,	  	  PN6,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向无功电量
	{0x24070f00,	12,		MAP_SYSDB,		0x2419,	  	  PN6,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向有功电能示值
	{0x24071000,	12,		MAP_SYSDB,		0x241a,	  	  PN6,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向无功电能示值
	{0x24071100,	12,		MAP_SYSDB,		0x241b,	  	  PN6,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向有功电能示值
	{0x24071200,	12,		MAP_SYSDB,		0x241c,	  	  PN6,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向无功电能示值
	{0x24071300,	12,		MAP_SYSDB,		0x2418,	  	  PN6,   0,		g_bPulseDataUnitFmt,	sizeof(g_bPulseDataUnitFmt),	NULL},	//换算及单位 ---- (脉冲计量点7)

	{0x24080200,	12,		MAP_SYSDB,		0x2401,	  	  PN7,	 0,		g_bCommAddrFmt,		sizeof(g_bCommAddrFmt),		NULL},	//通信地址  ---- (脉冲计量接口类)
	{0x24080300,	12,		MAP_SYSDB,		0x2402,	  	  PN7,	 0,		g_bPTCTFmt,			sizeof(g_bPTCTFmt),			NULL},	//互感器倍率
	{0x24080400,	12,		MAP_SYSDB,		0x2403,	  	  PN7,	 0,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		NULL},	//脉冲配置
	{0x24080500,	12,		MAP_SYSDB,		0x2404,	  	  PN7,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//有功功率
	{0x24080600,	12,		MAP_SYSDB,		0x2405,	  	  PN7,	 0,		g_bPulsePowerFmt,	sizeof(g_bPulsePowerFmt),	NULL},	//无功功率
	{0x24080700,	12,		MAP_SYSDB,		0x2406,	  	  PN7,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向有功电量
	{0x24080800,	12,		MAP_SYSDB,		0x2407,	  	  PN7,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向有功电量
	{0x24080900,	12,		MAP_SYSDB,		0x2408,	  	  PN7,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向有功电量
	{0x24080a00,	12,		MAP_SYSDB,		0x2409,	  	  PN7,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向有功电量
	{0x24080b00,	12,		MAP_SYSDB,		0x2410,	  	  PN7,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日正向无功电量
	{0x24080c00,	12,		MAP_SYSDB,		0x2411,	  	  PN7,	 0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月正向无功电量
	{0x24080d00,	12,		MAP_SYSDB,		0x2412,	  	  PN7,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当日反向无功电量
	{0x24080e00,	12,		MAP_SYSDB,		0x2413,	  	  PN7,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//当月反向无功电量
	{0x24080f00,	12,		MAP_SYSDB,		0x2419,	  	  PN7,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向有功电能示值
	{0x24081000,	12,		MAP_SYSDB,		0x241a,	  	  PN7,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//正向无功电能示值
	{0x24081100,	12,		MAP_SYSDB,		0x241b,	  	  PN7,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向有功电能示值
	{0x24081200,	12,		MAP_SYSDB,		0x241c,	  	  PN7,   0,		g_bEngDataFmt,		sizeof(g_bEngDataFmt),		NULL},	//反向无功电能示值
	{0x24081300,	12,		MAP_SYSDB,		0x2418,	  	  PN7,   0,		g_bPulseDataUnitFmt,	sizeof(g_bPulseDataUnitFmt),	NULL},	//换算及单位 ---- (脉冲计量点8)

	{0x25000200,	6,		MAP_SYSDB,		0x2500,	  	  PN0,	 0,		g_b645ExtDataFmt,		sizeof(g_b645ExtDataFmt),		NULL},//累计水（热）流量
	{0x25010200,	6,		MAP_SYSDB,		0x2501,	  	  PN0,	 0,		g_b645ExtDataFmt,		sizeof(g_b645ExtDataFmt),		NULL},//累计气流量
	{0x25020200,	6,		MAP_SYSDB,		0x2502,	  	  PN0,	 0,		g_b645ExtDataFmt,		sizeof(g_b645ExtDataFmt),		NULL},//累计热量
	{0x25030200,	6,		MAP_SYSDB,		0x2503,	  	  PN0,	 0,		g_b645ExtDataFmt,		sizeof(g_b645ExtDataFmt),		NULL},//热功率
	{0x25040200,	6,		MAP_SYSDB,		0x2504,	  	  PN0,	 0,		g_b645ExtDataFmt,		sizeof(g_b645ExtDataFmt),		NULL},//累计工作时间
	{0x25050200,	6,		MAP_SYSDB,		0x2505,	  	  PN0,	 0,		g_b645ExtTempFmt,		sizeof(g_b645ExtTempFmt),		NULL},//水温
	{0x25060200,	6,		MAP_SYSDB,		0x2506,	  	  PN0,	 0,		g_b645ExtStaFmt,		sizeof(g_b645ExtStaFmt),		NULL},//（仪表）状态ST
	///////////////////////////////////////////////////////////////////////////////////////////
	{0x30000200,	24,		MAP_SYSDB,		0x3600,		  PN0,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表失压事件，属性2，关联对象属性表
	{0x30000300,	24,		MAP_SYSDB,		0x3B00,		  PN0,   0,		g_bEvtIc24RecNumFmt,		sizeof(g_bEvtIc24RecNumFmt),					  NULL},//电能表失压事件，属性3，当前记录数
	{0x30000400,	24,		MAP_SYSDB,		0x3601,		  PN0,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表失压事件，属性4，最大记录数
	{0x30000500,	24,		MAP_SYSDB,		0x3000,	  	  PN0,   0,		g_bEvtVlostParaFmt,			sizeof(g_bEvtVlostParaFmt),					  NULL},//电能表失压事件，属性5，配置参数
	{0x30000600,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,					 			 NULL},//电能表失压事件，属性6，事件记录表1  
	{0x30000700,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,					  			"InMrtEvtVLoss-A.dat"},//电能表失压事件，属性7，事件记录表2  
	{0x30000800,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,					 			"InMrtEvtVLoss-B.dat"},//电能表失压事件，属性8，事件记录表3  
	{0x30000900,	24,		MAP_TASKDB,		0,		 	  PN0,   0,		NULL,						0,					  			"InMrtEvtVLoss-C.dat"},//电能表失压事件，属性9，事件记录表4  
	{0x30000A00,	24,		MAP_SYSDB,		0x3B01,		  PN0,   0,		g_bEvtIc24ValNumFmt,		sizeof(g_bEvtIc24ValNumFmt),					  NULL},//电能表失压事件，属性10，当前值记录表
	{0x30000B00,	24,		MAP_SYSDB,		0x3602,		  PN0,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表失压事件，属性11，上报标识
	{0x30000C00,	24,		MAP_SYSDB,		0x3603,		  PN0,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表失压事件，属性12，有效标识
	{0x30000D00,	24,		MAP_SYSDB,		0x3B02,		  PN0,   0,		g_bEvtVLostStaFmt,			sizeof(g_bEvtVLostStaFmt),					  NULL},//电能表失压事件，属性13，失压统计

	{0x30010200,	24,		MAP_SYSDB,		0x3600,		  PN1,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表欠压事件，属性2，关联对象属性表
	{0x30010300,	24,		MAP_SYSDB,		0x3B00,		  PN1,   0,		g_bEvtIc24RecNumFmt,		sizeof(g_bEvtIc24RecNumFmt),					  NULL},//电能表欠压事件，属性3，当前记录数
	{0x30010400,	24,		MAP_SYSDB,		0x3601,		  PN1,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表欠压事件，属性4，最大记录数
	{0x30010500,	24,		MAP_SYSDB,		0x3001,	  	  PN0,   0,		g_bEvtVUnderParaFmt,		sizeof(g_bEvtVUnderParaFmt),					  NULL},//电能表欠压事件，属性5，配置参数
	{0x30010600,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,					 			 NULL},//电能表欠压事件，属性6，事件记录表1  
	{0x30010700,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								 "InMrtEvtVLess-A.dat"},//电能表欠压事件，属性7，事件记录表2  
	{0x30010800,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,					 			 "InMrtEvtVLess-B.dat"},//电能表欠压事件，属性8，事件记录表3  
	{0x30010900,	24,		MAP_TASKDB,		0,		 	  PN0,   0,		NULL,						0,					 			 "InMrtEvtVLess-C.dat"},//电能表欠压事件，属性9，事件记录表4  
	{0x30010A00,	24,		MAP_SYSDB,		0x3B01,		  PN1,   0,		g_bEvtIc24ValNumFmt,		sizeof(g_bEvtIc24ValNumFmt),					  NULL},//电能表欠压事件，属性10，当前值记录表
	{0x30010B00,	24,		MAP_SYSDB,		0x3602,		  PN1,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表欠压事件，属性11，上报标识
	{0x30010C00,	24,		MAP_SYSDB,		0x3603,		  PN1,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表欠压事件，属性12，有效标识

	{0x30020200,	24,		MAP_SYSDB,		0x3600,		  PN2,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表过压事件，属性2，关联对象属性表
	{0x30020300,	24,		MAP_SYSDB,		0x3B00,		  PN2,   0,		g_bEvtIc24RecNumFmt,		sizeof(g_bEvtIc24RecNumFmt),					  NULL},//电能表过压事件，属性3，当前记录数
	{0x30020400,	24,		MAP_SYSDB,		0x3601,		  PN2,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表过压事件，属性4，最大记录数
	{0x30020500,	24,		MAP_SYSDB,		0x3002,	  	  PN0,   0,		g_bEvtVOverParaFmt,		sizeof(g_bEvtVOverParaFmt),					  NULL},//电能表过压事件，属性5，配置参数
	{0x30020600,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,					 			 NULL},//电能表过压事件，属性6，事件记录表1  
	{0x30020700,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,					 			 "InMrtEvtVOver-A.dat"},//电能表过压事件，属性7，事件记录表2  
	{0x30020800,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtVOver-B.dat"},//电能表过压事件，属性8，事件记录表3  
	{0x30020900,	24,		MAP_TASKDB,		0,		 	  PN0,   0,		NULL,						0,								  "InMrtEvtVOver-C.dat"},//电能表过压事件，属性9，事件记录表4  
	{0x30020A00,	24,		MAP_SYSDB,		0x3B01,		  PN2,   0,		g_bEvtIc24ValNumFmt,		sizeof(g_bEvtIc24ValNumFmt),					  NULL},//电能表过压事件，属性10，当前值记录表
	{0x30020B00,	24,		MAP_SYSDB,		0x3602,		  PN2,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表过压事件，属性11，上报标识
	{0x30020C00,	24,		MAP_SYSDB,		0x3603,		  PN2,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表过压事件，属性12，有效标识

	{0x30030200,	24,		MAP_SYSDB,		0x3600,		  PN3,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表断相事件，属性2，关联对象属性表
	{0x30030300,	24,		MAP_SYSDB,		0x3B00,		  PN3,   0,		g_bEvtIc24RecNumFmt,		sizeof(g_bEvtIc24RecNumFmt),					  NULL},//电能表断相事件，属性3，当前记录数
	{0x30030400,	24,		MAP_SYSDB,		0x3601,		  PN3,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表断相事件，属性4，最大记录数
	{0x30030500,	24,		MAP_SYSDB,		0x3003,	  	  PN0,   0,		g_bEvtVBreakParaFmt,		sizeof(g_bEvtVBreakParaFmt),					  NULL},//电能表断相事件，属性5，配置参数
	{0x30030600,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  NULL},//电能表断相事件，属性6，事件记录表1  
	{0x30030700,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtVBreak-A.dat"},//电能表断相事件，属性7，事件记录表2  
	{0x30030800,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtVBreak-B.dat"},//电能表断相事件，属性8，事件记录表3  
	{0x30030900,	24,		MAP_TASKDB,		0,		 	  PN0,   0,		NULL,						0,								  "InMrtEvtVBreak-C.dat"},//电能表断相事件，属性9，事件记录表4  
	{0x30030A00,	24,		MAP_SYSDB,		0x3B01,		  PN3,   0,		g_bEvtIc24ValNumFmt,		sizeof(g_bEvtIc24ValNumFmt),					  NULL},//电能表断相事件，属性10，当前值记录表
	{0x30030B00,	24,		MAP_SYSDB,		0x3602,		  PN3,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表断相事件，属性11，上报标识
	{0x30030C00,	24,		MAP_SYSDB,		0x3603,		  PN3,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表断相事件，属性12，有效标识

	{0x30040200,	24,		MAP_SYSDB,		0x3600,		  PN4,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),				  NULL},//电能表失流事件，属性2，关联对象属性表
	{0x30040300,	24,		MAP_SYSDB,		0x3B00,		  PN4,   0,		g_bEvtIc24RecNumFmt,		sizeof(g_bEvtIc24RecNumFmt),					  NULL},//电能表失流事件，属性3，当前记录数
	{0x30040400,	24,		MAP_SYSDB,		0x3601,		  PN4,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表失流事件，属性4，最大记录数
	{0x30040500,	24,		MAP_SYSDB,		0x3004,	  	  PN0,   0,		g_bEvtILostParaFmt,			sizeof(g_bEvtILostParaFmt),					  NULL},//电能表失流事件，属性5，配置参数
	{0x30040600,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  NULL},//电能表失流事件，属性6，事件记录表1  
	{0x30040700,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtILoss-A.dat"},//电能表失流事件，属性7，事件记录表2  
	{0x30040800,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtILoss-B.dat"},//电能表失流事件，属性8，事件记录表3  
	{0x30040900,	24,		MAP_TASKDB,		0,		 	  PN0,   0,		NULL,						0,								  "InMrtEvtILoss-C.dat"},//电能表失流事件，属性9，事件记录表4  
	{0x30040A00,	24,		MAP_SYSDB,		0x3B01,		  PN4,   0,		g_bEvtIc24ValNumFmt,		sizeof(g_bEvtIc24ValNumFmt),					  NULL},//电能表失流事件，属性10，当前值记录表
	{0x30040B00,	24,		MAP_SYSDB,		0x3602,		  PN4,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表失流事件，属性11，上报标识
	{0x30040C00,	24,		MAP_SYSDB,		0x3603,		  PN4,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表失流事件，属性12，有效标识

	{0x30050200,	24,		MAP_SYSDB,		0x3600,		  PN5,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表过流事件，属性2，关联对象属性表
	{0x30050300,	24,		MAP_SYSDB,		0x3B00,		  PN5,   0,		g_bEvtIc24RecNumFmt,		sizeof(g_bEvtIc24RecNumFmt),					  NULL},//电能表过流事件，属性3，当前记录数
	{0x30050400,	24,		MAP_SYSDB,		0x3601,		  PN5,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表过流事件，属性4，最大记录数
	{0x30050500,	24,		MAP_SYSDB,		0x3005,	  	  PN0,   0,		g_bEvtIOverParaFmt,			sizeof(g_bEvtIOverParaFmt),					  NULL},//电能表过流事件，属性5，配置参数
	{0x30050600,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  NULL},//电能表过流事件，属性6，事件记录表1  
	{0x30050700,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtIOver-A.dat"},//电能表过流事件，属性7，事件记录表2  
	{0x30050800,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtIOver-B.dat"},//电能表过流事件，属性8，事件记录表3  
	{0x30050900,	24,		MAP_TASKDB,		0,		 	  PN0,   0,		NULL,						0,								  "InMrtEvtIOver-C.dat"},//电能表过流事件，属性9，事件记录表4  
	{0x30050A00,	24,		MAP_SYSDB,		0x3B01,		  PN5,   0,		g_bEvtIc24ValNumFmt,		sizeof(g_bEvtIc24ValNumFmt),					  NULL},//电能表过流事件，属性10，当前值记录表
	{0x30050B00,	24,		MAP_SYSDB,		0x3602,		  PN5,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表过流事件，属性11，上报标识
	{0x30050C00,	24,		MAP_SYSDB,		0x3603,		  PN5,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表过流事件，属性12，有效标识

	{0x30060200,	24,		MAP_SYSDB,		0x3600,		  PN6,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表断流事件，属性2，关联对象属性表
	{0x30060300,	24,		MAP_SYSDB,		0x3B00,		  PN6,   0,		g_bEvtIc24RecNumFmt,		sizeof(g_bEvtIc24RecNumFmt),					  NULL},//电能表断流事件，属性3，当前记录数
	{0x30060400,	24,		MAP_SYSDB,		0x3601,		  PN6,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表断流事件，属性4，最大记录数
	{0x30060500,	24,		MAP_SYSDB,		0x3006,	  	  PN0,   0,		g_bEvtIBreakParaFmt,		sizeof(g_bEvtIBreakParaFmt),					  NULL},//电能表断流事件，属性5，配置参数
	{0x30060600,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  NULL},//电能表断流事件，属性6，事件记录表1  
	{0x30060700,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtIBreak-A.dat"},//电能表断流事件，属性7，事件记录表2  
	{0x30060800,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtIBreak-B.dat"},//电能表断流事件，属性8，事件记录表3  
	{0x30060900,	24,		MAP_TASKDB,		0,		 	  PN0,   0,		NULL,						0,								  "InMrtEvtIBreak-C.dat"},//电能表断流事件，属性9，事件记录表4  
	{0x30060A00,	24,		MAP_SYSDB,		0x3B01,		  PN6,   0,		g_bEvtIc24ValNumFmt,		sizeof(g_bEvtIc24ValNumFmt),					  NULL},//电能表断流事件，属性10，当前值记录表
	{0x30060B00,	24,		MAP_SYSDB,		0x3602,		  PN6,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表断流事件，属性11，上报标识
	{0x30060C00,	24,		MAP_SYSDB,		0x3603,		  PN6,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表断流事件，属性12，有效标识

	{0x30070200,	24,		MAP_SYSDB,		0x3600,		  PN7,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表潮流反向事件，属性2，关联对象属性表
	{0x30070300,	24,		MAP_SYSDB,		0x3B00,		  PN7,   0,		g_bEvtIc24RecNumFmt,		sizeof(g_bEvtIc24RecNumFmt),					  NULL},//电能表潮流反向事件，属性3，当前记录数
	{0x30070400,	24,		MAP_SYSDB,		0x3601,		  PN7,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表潮流反向事件，属性4，最大记录数
	{0x30070500,	24,		MAP_SYSDB,		0x3007,	  	  PN0,   0,		g_bEvtPReverseParaFmt,		sizeof(g_bEvtPReverseParaFmt),					  NULL},//电能表潮流反向事件，属性5，配置参数
	{0x30070600,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  NULL},//电能表潮流反向事件，属性6，事件记录表1  
	{0x30070700,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtPReverse-A.dat"},//电能表潮流反向事件，属性7，事件记录表2  
	{0x30070800,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtPReverse-B.dat"},//电能表潮流反向事件，属性8，事件记录表3  
	{0x30070900,	24,		MAP_TASKDB,		0,		 	  PN0,   0,		NULL,						0,								  "InMrtEvtPReverse-C.dat"},//电能表潮流反向事件，属性9，事件记录表4  
	{0x30070A00,	24,		MAP_SYSDB,		0x3B01,		  PN7,   0,		g_bEvtIc24ValNumFmt,		sizeof(g_bEvtIc24ValNumFmt),					  NULL},//电能表潮流反向事件，属性10，当前值记录表
	{0x30070B00,	24,		MAP_SYSDB,		0x3602,		  PN7,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表潮流反向事件，属性11，上报标识
	{0x30070C00,	24,		MAP_SYSDB,		0x3603,		  PN7,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表潮流反向事件，属性12，有效标识

	{0x30080200,	24,		MAP_SYSDB,		0x3600,		  PN8,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表过载事件，属性2，关联对象属性表
	{0x30080300,	24,		MAP_SYSDB,		0x3B00,		  PN8,   0,		g_bEvtIc24RecNumFmt,		sizeof(g_bEvtIc24RecNumFmt),					  NULL},//电能表过载事件，属性3，当前记录数
	{0x30080400,	24,		MAP_SYSDB,		0x3601,		  PN8,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表过载事件，属性4，最大记录数
	{0x30080500,	24,		MAP_SYSDB,		0x3008,	  	  PN0,   0,		g_bEvtPOverParaFmt,		sizeof(g_bEvtPOverParaFmt),					  NULL},//电能表过载事件，属性5，配置参数
	{0x30080600,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  NULL},//电能表过载事件，属性6，事件记录表1  
	{0x30080700,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtPOver-A.dat"},//电能表过载事件，属性7，事件记录表2  
	{0x30080800,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtPOver-B.dat"},//电能表过载事件，属性8，事件记录表3  
	{0x30080900,	24,		MAP_TASKDB,		0,		 	  PN0,   0,		NULL,						0,								  "InMrtEvtPOver-C.dat"},//电能表过载事件，属性9，事件记录表4  
	{0x30080A00,	24,		MAP_SYSDB,		0x3B01,		  PN8,   0,		g_bEvtIc24ValNumFmt,		sizeof(g_bEvtIc24ValNumFmt),					  NULL},//电能表过载事件，属性10，当前值记录表
	{0x30080B00,	24,		MAP_SYSDB,		0x3602,		  PN8,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表过载事件，属性11，上报标识
	{0x30080C00,	24,		MAP_SYSDB,		0x3603,		  PN8,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表过载事件，属性12，有效标识

	{0x30090200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtPDmdOver.dat"},//电能表正向有功需量超限事件，属性2，事件记录表
	{0x30090300,	7,		MAP_SYSDB,		0x3600,		  PN9,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表正向有功需量超限事件，属性3，关联对象属性表
	{0x30090400,	7,		MAP_SYSDB,		0x3B03,		  PN9,   0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表正向有功需量超限事件，属性4，当前记录数
	{0x30090500,	7,		MAP_SYSDB,		0x3601,	  	  PN9,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表正向有功需量超限事件，属性5，最大记录数
	{0x30090600,	7,		MAP_SYSDB,		0x3009,		  PN0,   0,		g_bEvtPDmdOverParaFmt,	sizeof(g_bEvtPDmdOverParaFmt),					  NULL},//电能表正向有功需量超限事件，属性6，配置参数  
	{0x30090700,	7,		MAP_SYSDB,		0x3B04,		  PN9,   0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表正向有功需量超限事件，属性7，当前值记录表 
	{0x30090800,	7,		MAP_SYSDB,		0x3602,		  PN9,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表正向有功需量超限事件，属性8，上报标识
	{0x30090900,	7,		MAP_SYSDB,		0x3603,		  PN9,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表正向有功需量超限事件，属性9，有效标识  

	{0x300A0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtRPDmdOver.dat"},//电能表反向有功需量超限事件，属性2，事件记录表
	{0x300A0300,	7,		MAP_SYSDB,		0x3600,		  PN10,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表反向有功需量超限事件，属性3，关联对象属性表
	{0x300A0400,	7,		MAP_SYSDB,		0x3B03,		  PN10,   0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),				  NULL},//电能表反向有功需量超限事件，属性4，当前记录数
	{0x300A0500,	7,		MAP_SYSDB,		0x3601,	  	  PN10,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表反向有功需量超限事件，属性5，最大记录数
	{0x300A0600,	7,		MAP_SYSDB,		0x300A,		  PN0,   0,		g_bEvtRPDmdOverParaFmt,	sizeof(g_bEvtRPDmdOverParaFmt),					  NULL},//电能表反向有功需量超限事件，属性6，配置参数  
	{0x300A0700,	7,		MAP_SYSDB,		0x3B04,		  PN10,   0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表反向有功需量超限事件，属性7，当前值记录表 
	{0x300A0800,	7,		MAP_SYSDB,		0x3602,		  PN10,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表反向有功需量超限事件，属性8，上报标识
	{0x300A0900,	7,		MAP_SYSDB,		0x3603,		  PN10,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表反向有功需量超限事件，属性9，有效标识  

	{0x300B0200,	24,		MAP_SYSDB,		0x3600,		  PN11,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表无功需量超限事件，属性2，关联对象属性表
	{0x300B0300,	24,		MAP_SYSDB,		0x3B00,		  PN11,   0,		g_bEvtIc24RecNumFmt,		sizeof(g_bEvtIc24RecNumFmt),					  NULL},//电能表无功需量超限事件，属性3，当前记录数
	{0x300B0400,	24,		MAP_SYSDB,		0x3601,		  PN11,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表无功需量超限事件，属性4，最大记录数
	{0x300B0500,	24,		MAP_SYSDB,		0x300B,	  	  PN0,   0,		g_bEvtQDmdOverParaFmt,	sizeof(g_bEvtQDmdOverParaFmt),					  NULL},//电能表无功需量超限事件，属性5，配置参数
	{0x300B0600,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,					 			 "InMrtEvtQDmdOver-1.dat"},//电能表无功需量超限事件，属性6，事件记录表1  
	{0x300B0700,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtQDmdOver-2.dat"},//电能表无功需量超限事件，属性7，事件记录表2  
	{0x300B0800,	24,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtQDmdOver-3.dat"},//电能表无功需量超限事件，属性8，事件记录表3  
	{0x300B0900,	24,		MAP_TASKDB,		0,		 	  PN0,   0,		NULL,						0,								  "InMrtEvtQDmdOver-4.dat"},//电能表无功需量超限事件，属性9，事件记录表4  
	{0x300B0A00,	24,		MAP_SYSDB,		0x3B01,		  PN11,   0,		g_bEvtIc24ValNumFmt,		sizeof(g_bEvtIc24ValNumFmt),					  NULL},//电能表无功需量超限事件，属性10，当前值记录表
	{0x300B0B00,	24,		MAP_SYSDB,		0x3602,		  PN11,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表无功需量超限事件，属性11，上报标识
	{0x300B0C00,	24,		MAP_SYSDB,		0x3603,		  PN11,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表无功需量超限事件，属性12，有效标识


	{0x300C0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtPfUnder.dat"},//电能表功率因数超下限事件，属性2，事件记录表
	{0x300C0300,	7,		MAP_SYSDB,		0x3600,		  PN12,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表功率因数超下限事件，属性3，关联对象属性表
	{0x300C0400,	7,		MAP_SYSDB,		0x3B03,		  PN12,   0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表功率因数超下限事件，属性4，当前记录数
	{0x300C0500,	7,		MAP_SYSDB,		0x3601,	  	  PN12,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表功率因数超下限事件，属性5，最大记录数
	{0x300C0600,	7,		MAP_SYSDB,		0x300C,		  PN0,   0,		g_bEvtPfUnderParaFmt,		sizeof(g_bEvtPfUnderParaFmt),					  NULL},//电能表功率因数超下限事件，属性6，配置参数  
	{0x300C0700,	7,		MAP_SYSDB,		0x3B04,		  PN12,   0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表功率因数超下限事件，属性7，当前值记录表 
	{0x300C0800,	7,		MAP_SYSDB,		0x3602,		  PN12,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表功率因数超下限事件，属性8，上报标识
	{0x300C0900,	7,		MAP_SYSDB,		0x3603,		  PN12,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表功率因数超下限事件，属性9，有效标识   

	{0x300D0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtAllVLoss.dat"},//电能表全失压事件，属性2，事件记录表
	{0x300D0300,	7,		MAP_SYSDB,		0x3600,		  PN13,   0,		g_bEvtCapOADFmt,			 sizeof(g_bEvtCapOADFmt),					  NULL},//电能表全失压事件，属性3，关联对象属性表
	{0x300D0400,	7,		MAP_SYSDB,		0x3B03,		  PN13,   0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表全失压事件，属性4，当前记录数
	{0x300D0500,	7,		MAP_SYSDB,		0x3601,	  	  PN13,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表全失压事件，属性5，最大记录数
	//{0x300D0600,	7,		MAP_SYSDB,		0x300D,		  PN0,   0,		g_bEvtAllVlostParaFmt,		sizeof(g_bEvtAllVlostParaFmt),					  NULL},//电能表全失压事件，属性6，全失压无配置参数  
	{0x300D0700,	7,		MAP_SYSDB,		0x3B04,		  PN13,   0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表全失压事件，属性7，当前值记录表 
	{0x300D0800,	7,		MAP_SYSDB,		0x3602,		  PN13,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表全失压事件，属性8，上报标识
	{0x300D0900,	7,		MAP_SYSDB,		0x3603,		  PN13,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表全失压事件，属性9，有效标识   

	{0x300F0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtVDisorder.dat"},//电能表电压逆向序事件，属性2，事件记录表
	{0x300F0300,	7,		MAP_SYSDB,		0x3600,		  PN15,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表电压逆向序事件，属性3，关联对象属性表
	{0x300F0400,	7,		MAP_SYSDB,		0x3B03,		  PN15,   0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表电压逆向序事件，属性4，当前记录数
	{0x300F0500,	7,		MAP_SYSDB,		0x3601,	  	  PN15,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表电压逆向序事件，属性5，最大记录数
	{0x300F0600,	7,		MAP_SYSDB,		0x300F,		  PN0,   0,		g_bEvtVDisorderParaFmt,		sizeof(g_bEvtVDisorderParaFmt),					  NULL},//电能表电压逆向序事件，属性6，配置参数  
	{0x300F0700,	7,		MAP_SYSDB,		0x3B04,		  PN15,   0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),						  NULL},//电能表电压逆向序事件，属性7，当前值记录表 
	{0x300F0800,	7,		MAP_SYSDB,		0x3602,		  PN15,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表电压逆向序事件，属性8，上报标识
	{0x300F0900,	7,		MAP_SYSDB,		0x3603,		  PN15,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表电压逆向序事件，属性9，有效标识   

	{0x30100200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtIDisorder.dat"},//电能表电流逆向序事件，属性2，事件记录表
	{0x30100300,	7,		MAP_SYSDB,		0x3600,		  PN16,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表电流逆向序事件，属性3，关联对象属性表
	{0x30100400,	7,		MAP_SYSDB,		0x3B03,		  PN16,   0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表电流逆向序事件，属性4，当前记录数
	{0x30100500,	7,		MAP_SYSDB,		0x3601,	  	  PN16,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表电流逆向序事件，属性5，最大记录数
	{0x30100600,	7,		MAP_SYSDB,		0x3010,		  PN0,   0,		g_bEvtIDisorderParaFmt,		sizeof(g_bEvtIDisorderParaFmt),					  NULL},//电能表电流逆向序事件，属性6，配置参数  
	{0x30100700,	7,		MAP_SYSDB,		0x3B04,		  PN16,   0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表电流逆向序事件，属性7，当前值记录表 
	{0x30100800,	7,		MAP_SYSDB,		0x3602,		  PN16,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表电流逆向序事件，属性8，上报标识
	{0x30100900,	7,		MAP_SYSDB,		0x3603,		  PN16,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表电流逆向序事件，属性9，有效标识   

	{0x30130200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtMeterClear.dat"},//电能表清零事件，属性2，事件记录表
	{0x30130300,	7,		MAP_SYSDB,		0x3600,		  PN19,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表清零事件，属性3，关联对象属性表
	{0x30130400,	7,		MAP_SYSDB,		0x3B03,		  PN19,   0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表清零事件，属性4，当前记录数
	{0x30130500,	7,		MAP_SYSDB,		0x3601,	  	  PN19,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),				  NULL},//电能表清零事件，属性5，最大记录数
	//{0x30130600,	7,		MAP_SYSDB,		0x3013,		  PN0,   0,		g_bEvtMeterClrParaFmt,		sizeof(g_bEvtMeterClrParaFmt),					  NULL},//电能表清零事件，属性6，无配置参数  
	{0x30130700,	7,		MAP_SYSDB,		0x3B04,		  PN19,   0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表清零事件，属性7，当前值记录表 
	{0x30130800,	7,		MAP_SYSDB,		0x3602,		  PN19,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表清零事件，属性8，上报标识
	{0x30130900,	7,		MAP_SYSDB,		0x3603,		  PN19,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表清零事件，属性9，有效标识   

	{0x30140200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtDmdClear.dat"},//电能表需量清零事件，属性2，事件记录表
	{0x30140300,	7,		MAP_SYSDB,		0x3600,		  PN20,   0,		g_bEvtCapOADFmt,			 sizeof(g_bEvtCapOADFmt),					  NULL},//电能表需量清零事件，属性3，关联对象属性表
	{0x30140400,	7,		MAP_SYSDB,		0x3B03,		  PN20,   0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表需量清零事件，属性4，当前记录数
	{0x30140500,	7,		MAP_SYSDB,		0x3601,	  	  PN20,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表需量清零事件，属性5，最大记录数
	//{0x30140600,	7,		MAP_SYSDB,		0x3014,		  PN0,   0,		g_bEvtDmdClrParaFmt,		sizeof(g_bEvtDmdClrParaFmt),					  NULL},//电能表需量清零事件，属性6，无配置参数  
	{0x30140700,	7,		MAP_SYSDB,		0x3B04,		  PN20,   0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表需量清零事件，属性7，当前值记录表 
	{0x30140800,	7,		MAP_SYSDB,		0x3602,		  PN20,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表需量清零事件，属性8，上报标识
	{0x30140900,	7,		MAP_SYSDB,		0x3603,		  PN20,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表需量清零事件，属性9，有效标识   

	{0x30150200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtEventClear.dat"},//电能表事件清零事件，属性2，事件记录表
	//{0x30150300,	7,		MAP_SYSDB,		0x3600,		  PN21,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表事件清零事件，属性3，关联对象属性表，无关联属性表
	{0x30150400,	7,		MAP_SYSDB,		0x3B03,		  PN21,   0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表事件清零事件，属性4，当前记录数
	{0x30150500,	7,		MAP_SYSDB,		0x3601,	  	  PN21,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表事件清零事件，属性5，最大记录数
	//{0x30150600,	7,		MAP_SYSDB,		0x3015,		  PN0,   0,		g_bEvtEventClrParaFmt,		sizeof(g_bEvtEventClrParaFmt),					  NULL},//电能表事件清零事件，属性6，无配置参数  
	{0x30150700,	7,		MAP_SYSDB,		0x3B04,		  PN21,   0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表事件清零事件，属性7，当前值记录表 
	{0x30150800,	7,		MAP_SYSDB,		0x3602,		  PN21,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表事件清零事件，属性8，上报标识
	{0x30150900,	7,		MAP_SYSDB,		0x3603,		  PN21,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表事件清零事件，属性9，有效标识   

	{0x301D0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtVUnbalance.dat"},//电能表电压不平衡，属性2，事件记录表
	{0x301D0300,	7,		MAP_SYSDB,		0x3600,		  PN29,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表电压不平衡，属性3，关联对象属性表
	{0x301D0400,	7,		MAP_SYSDB,		0x3B03,		  PN29,   0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表电压不平衡，属性4，当前记录数
	{0x301D0500,	7,		MAP_SYSDB,		0x3601,	  	  PN29,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表电压不平衡，属性5，最大记录数
	{0x301D0600,	7,		MAP_SYSDB,		0x301D,		  PN0,   0,		g_bEvtVUnbalanceParaFmt,	sizeof(g_bEvtVUnbalanceParaFmt),					  NULL},//电能表电压不平衡，属性6，配置参数  
	{0x301D0700,	7,		MAP_SYSDB,		0x3B04,		  PN29,   0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表电压不平衡，属性7，当前值记录表 
	{0x301D0800,	7,		MAP_SYSDB,		0x3602,		  PN29,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表电压不平衡，属性8，上报标识
	{0x301D0900,	7,		MAP_SYSDB,		0x3603,		  PN29,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表电压不平衡，属性9，有效标识   

	{0x301E0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtIUnbalance.dat"},//电能表电流不平衡事件，属性2，事件记录表
	{0x301E0300,	7,		MAP_SYSDB,		0x3600,		  PN30,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表电流不平衡事件，属性3，关联对象属性表
	{0x301E0400,	7,		MAP_SYSDB,		0x3B03,		  PN30,  0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表电流不平衡事件，属性4，当前记录数
	{0x301E0500,	7,		MAP_SYSDB,		0x3601,	  	  PN30,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表电流不平衡事件，属性5，最大记录数
	{0x301E0600,	7,		MAP_SYSDB,		0x301E,		  PN0,   0,		g_bEvtIUnbalanceParaFmt,	sizeof(g_bEvtIUnbalanceParaFmt),					  NULL},//电能表电流不平衡事件，属性6，配置参数  
	{0x301E0700,	7,		MAP_SYSDB,		0x3B04,		  PN30,  0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表电流不平衡事件，属性7，当前值记录表 
	{0x301E0800,	7,		MAP_SYSDB,		0x3602,		  PN30,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表电流不平衡事件，属性8，上报标识
	{0x301E0900,	7,		MAP_SYSDB,		0x3603,		  PN30,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表电流不平衡事件，属性9，有效标识   

	{0x302D0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtISUnbalance.dat"},//电能表电流严重不平衡事件，属性2，事件记录表
	{0x302D0300,	7,		MAP_SYSDB,		0x3600,		  PN45,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),				  NULL},//电能表电流严重不平衡事件，属性3，关联对象属性表
	{0x302D0400,	7,		MAP_SYSDB,		0x3B03,		  PN45,  0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表电流严重不平衡事件，属性4，当前记录数
	{0x302D0500,	7,		MAP_SYSDB,		0x3601,	  	  PN45,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表电流严重不平衡事件，属性5，最大记录数 
	{0x302D0600,	7,		MAP_SYSDB,		0x302D,		  PN0,   0,		g_bEvtISUnbalanceParaFmt,	sizeof(g_bEvtISUnbalanceParaFmt),					  NULL},//电能表电流严重不平衡事件，属性6，配置参数   
	{0x302D0700,	7,		MAP_SYSDB,		0x3B04,		  PN45,  0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表电流严重不平衡事件，属性7，当前值记录表 
	{0x302D0800,	7,		MAP_SYSDB,		0x3602,		  PN45,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表电流严重不平衡事件，属性8，上报标识
	{0x302D0900,	7,		MAP_SYSDB,		0x3603,		  PN45,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表电流严重不平衡事件，属性9，有效标识   

	{0x302E0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtClockErr.dat"},//电能表时钟故障事件，属性2，事件记录表
	{0x302E0300,	7,		MAP_SYSDB,		0x3600,		  PN46,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表时钟故障事件，属性3，关联对象属性表
	{0x302E0400,	7,		MAP_SYSDB,		0x3B03,		  PN46,  0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表时钟故障事件，属性4，当前记录数
	{0x302E0500,	7,		MAP_SYSDB,		0x3601,	  	  PN46,  0,		g_bEvtMaxNumFmt,			 sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表时钟故障事件，属性5，最大记录数 
	{0x302E0600,	7,		MAP_SYSDB,		0x302E,		  PN0,   0,		g_bEvtClockErrParaFmt,		sizeof(g_bEvtClockErrParaFmt),					  NULL},//电能表时钟故障事件，属性6，配置参数   
	{0x302E0700,	7,		MAP_SYSDB,		0x3B04,		  PN46,  0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表时钟故障事件，属性7，当前值记录表 
	{0x302E0800,	7,		MAP_SYSDB,		0x3602,		  PN46,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表时钟故障事件，属性8，上报标识
	{0x302E0900,	7,		MAP_SYSDB,		0x3603,		  PN46,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表时钟故障事件，属性9，有效标识   

	{0x302F0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "InMrtEvtMeterChipErr.dat"},//电能表计量芯片故障事件，属性2，事件记录表
	{0x302F0300,	7,		MAP_SYSDB,		0x3600,		  PN47,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL},//电能表计量芯片故障事件，属性3，关联对象属性表
	{0x302F0400,	7,		MAP_SYSDB,		0x3B03,		  PN47,  0,		g_bEvtIc7RecNumFmt,		sizeof(g_bEvtIc7RecNumFmt),					  NULL},//电能表计量芯片故障事件，属性4，当前记录数
	{0x302F0500,	7,		MAP_SYSDB,		0x3601,	  	  PN47,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL},//电能表计量芯片故障事件，属性5，最大记录数 
	{0x302F0600,	7,		MAP_SYSDB,		0x302F,		  PN0,   0,		g_bEvtMtrChipErrParaFmt,	sizeof(g_bEvtMtrChipErrParaFmt),					  NULL},//电能表计量芯片故障事件，属性6，配置参数   
	{0x302F0700,	7,		MAP_SYSDB,		0x3B04,		  PN47,  0,		g_bEvtIc7ValNumFmt,		sizeof(g_bEvtIc7ValNumFmt),					  NULL},//电能表计量芯片故障事件，属性7，当前值记录表 
	{0x302F0800,	7,		MAP_SYSDB,		0x3602,		  PN47,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL},//电能表计量芯片故障事件，属性8，上报标识
	{0x302F0900,	7,		MAP_SYSDB,		0x3603,		  PN47,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL},//电能表计量芯片故障事件，属性9，有效标识   

	{0x31000200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_Init.dat"},//终端初始化事件，属性2，事件记录表
	{0x31000300,	7,		MAP_SYSDB,		0x3700,		  PN0,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //终端初始化事件，属性3，关联对象属性表
	{0x31000400,	7,		MAP_SYSDB,		0x3B10,		  PN0,   0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //终端初始化事件，属性4，当前记录数
	{0x31000500,	7,		MAP_SYSDB,		0x3701,	  	  PN0,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //终端初始化事件，属性5，最大记录数 
	//{0x31000600,	7,		MAP_SYSDB,		0x3100,		  PN0,   0,		g_bEvtMtrChipErrParaFmt,	sizeof(g_bEvtMtrChipErrParaFmt),			  NULL}, //终端初始化事件，属性6，配置参数   
	{0x31000700,	7,		MAP_SYSDB,		0x3B12,		  PN0,   0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //终端初始化事件，属性7，当前值记录表 
	{0x31000800,	7,		MAP_SYSDB,		0x3702,		  PN0,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //终端初始化事件，属性8，上报标识
	{0x31000900,	7,		MAP_SYSDB,		0x3703,		  PN0,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //终端初始化事件，属性9，有效标识   

	{0x31010200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_VerChg.dat"},//终端初始化事件，属性2，事件记录表
	{0x31010300,	7,		MAP_SYSDB,		0x3700,		  PN1,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //终端初始化事件，属性3，关联对象属性表
	{0x31010400,	7,		MAP_SYSDB,		0x3B10,		  PN1,   0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //终端初始化事件，属性4，当前记录数
	{0x31010500,	7,		MAP_SYSDB,		0x3701,	  	  PN1,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //终端初始化事件，属性5，最大记录数 
	//{0x31010600,	7,		MAP_SYSDB,		0x3101,		  PN0,   0,		g_bEvtMtrChipErrParaFmt,	sizeof(g_bEvtMtrChipErrParaFmt),			  NULL}, //终端初始化事件，属性6，配置参数   
	{0x31010700,	7,		MAP_SYSDB,		0x3B12,		  PN1,   0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //终端初始化事件，属性7，当前值记录表 
	{0x31010800,	7,		MAP_SYSDB,		0x3702,		  PN1,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //终端初始化事件，属性8，上报标识
	{0x31010900,	7,		MAP_SYSDB,		0x3703,		  PN1,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //终端初始化事件，属性9，有效标识   

	{0x31040200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_YXChg.dat"},//终端初始化事件，属性2，事件记录表
	{0x31040300,	7,		MAP_SYSDB,		0x3700,		  PN4,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //终端初始化事件，属性3，关联对象属性表
	{0x31040400,	7,		MAP_SYSDB,		0x3B10,		  PN4,   0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //终端初始化事件，属性4，当前记录数
	{0x31040500,	7,		MAP_SYSDB,		0x3701,	  	  PN4,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //终端初始化事件，属性5，最大记录数 
	//{0x31040600,	7,		MAP_SYSDB,		0x3104,		  PN0,   0,		g_bEvtMtrChipErrParaFmt,	sizeof(g_bEvtMtrChipErrParaFmt),			  NULL}, //终端初始化事件，属性6，配置参数   
	{0x31040700,	7,		MAP_SYSDB,		0x3B12,		  PN4,   0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //终端初始化事件，属性7，当前值记录表 
	{0x31040800,	7,		MAP_SYSDB,		0x3702,		  PN4,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //终端初始化事件，属性8，上报标识
	{0x31040900,	7,		MAP_SYSDB,		0x3703,		  PN4,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //终端初始化事件，属性9，有效标识   	

	{0x31050200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "MtrExc_ClockErr.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x31050300,	7,		MAP_SYSDB,		0x3700,		  PN5,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x31050400,	7,		MAP_SYSDB,		0x3B10,		  PN5,   0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x31050500,	7,		MAP_SYSDB,		0x3701,	  	  PN5,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	{0x31050600,	7,		MAP_SYSDB,		0x3105,		  PN0,   0,		g_bMtrExcClockkErrFmt,		sizeof(g_bMtrExcClockkErrFmt),				  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x31050700,	7,		MAP_SYSDB,		0x3B11,		  PN0,   0,		g_bMtrExcIc7ValNumFmt,		sizeof(g_bMtrExcIc7ValNumFmt),				  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x31050800,	7,		MAP_SYSDB,		0x3702,		  PN5,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x31050900,	7,		MAP_SYSDB,		0x3703,		  PN5,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识

	{0x31060200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_PowerOff.dat"},//终端停上电事件，属性2，事件记录表
	{0x31060300,	7,		MAP_SYSDB,		0x3700,		  PN6,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //终端停上电事件，属性3，关联对象属性表
	{0x31060400,	7,		MAP_SYSDB,		0x3B10,		  PN6,   0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //终端停上电事件，属性4，当前记录数
	{0x31060500,	7,		MAP_SYSDB,		0x3701,	  	  PN6,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //终端停上电事件，属性5，最大记录数 
	{0x31060600,	7,		MAP_SYSDB,		0x3106,		  PN0,   0,		g_bEvtTermPwrOffFmt,		sizeof(g_bEvtTermPwrOffFmt),				  NULL}, //终端停上电事件，属性6，配置参数   
	{0x31060700,	7,		MAP_SYSDB,		0x3B17,		  PN0,   0,		g_bEvtIc7ValEnumFmt,			sizeof(g_bEvtIc7ValEnumFmt),					  NULL}, //终端停上电事件，属性7，当前值记录表 
	{0x31060800,	7,		MAP_SYSDB,		0x3702,		  PN6,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //终端停上电事件，属性8，上报标识
	{0x31060900,	7,		MAP_SYSDB,		0x3703,		  PN6,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //终端停上电事件，属性9，有效标识   

	/*{0x31070200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_DCOverUp.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x31070300,	7,		MAP_SYSDB,		0x3700,		  PN7,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x31070400,	7,		MAP_SYSDB,		0x3B10,		  PN7,   0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x31070500,	7,		MAP_SYSDB,		0x3701,	  	  PN7,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	{0x31070600,	7,		MAP_SYSDB,		0x3107,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x31070700,	7,		MAP_SYSDB,		0x3B12,		  PN7,   0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x31070800,	7,		MAP_SYSDB,		0x3702,		  PN7,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x31070900,	7,		MAP_SYSDB,		0x3703,		  PN7,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   

	{0x31080200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_DCOverDown.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x31080300,	7,		MAP_SYSDB,		0x3700,		  PN8,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x31080400,	7,		MAP_SYSDB,		0x3B10,		  PN8,   0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x31080500,	7,		MAP_SYSDB,		0x3701,	  	  PN8,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	{0x31080600,	7,		MAP_SYSDB,		0x3108,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x31080700,	7,		MAP_SYSDB,		0x3B12,		  PN8,   0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x31080800,	7,		MAP_SYSDB,		0x3702,		  PN8,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x31080900,	7,		MAP_SYSDB,		0x3703,		  PN8,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   */

	{0x31090200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_ESAM_Err.dat"},//终端消息认证错误事件，属性2，事件记录表
	{0x31090300,	7,		MAP_SYSDB,		0x3700,		  PN9,   0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //终端消息认证错误事件，属性3，关联对象属性表
	{0x31090400,	7,		MAP_SYSDB,		0x3B10,		  PN9,   0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //终端消息认证错误事件，属性4，当前记录数
	{0x31090500,	7,		MAP_SYSDB,		0x3701,	  	  PN9,   0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //终端消息认证错误事件，属性5，最大记录数 
	//{0x31090600,	7,		MAP_SYSDB,		0x3109,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //终端消息认证错误事件，属性6，配置参数   
	{0x31090700,	7,		MAP_SYSDB,		0x3B12,		  PN9,   0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //终端消息认证错误事件，属性7，当前值记录表 
	{0x31090800,	7,		MAP_SYSDB,		0x3702,		  PN9,   0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //终端消息认证错误事件，属性8，上报标识
	{0x31090900,	7,		MAP_SYSDB,		0x3703,		  PN9,   0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //终端消息认证错误事件，属性9，有效标识   

	{0x310A0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_Fault.dat"},//终端故障事件，属性2，事件记录表
	{0x310A0300,	7,		MAP_SYSDB,		0x3700,		  PN10,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //终端故障事件，属性3，关联对象属性表
	{0x310A0400,	7,		MAP_SYSDB,		0x3B10,		  PN10,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //终端故障事件，属性4，当前记录数
	{0x310A0500,	7,		MAP_SYSDB,		0x3701,	  	  PN10,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //终端故障事件，属性5，最大记录数 
	//{0x310A0600,	7,		MAP_SYSDB,		0x310A,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //终端故障事件，属性6，配置参数   
	{0x310A0700,	7,		MAP_SYSDB,		0x3B13,		  PN0,  0,		g_bEvtIc7ValEnumFmt,			sizeof(g_bEvtIc7ValEnumFmt),					  NULL}, //终端故障事件，属性7，当前值记录表 
	{0x310A0800,	7,		MAP_SYSDB,		0x3702,		  PN10,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //终端故障事件，属性8，上报标识
	{0x310A0900,	7,		MAP_SYSDB,		0x3703,		  PN10,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //终端故障事件，属性9，有效标识   

	{0x310B0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "MtrExc_EnergyDec.dat"},//电能表示度下降事件，属性2，事件记录表
	{0x310B0300,	7,		MAP_SYSDB,		0x3700,		  PN11,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表示度下降事件，属性3，关联对象属性表
	{0x310B0400,	7,		MAP_SYSDB,		0x3B10,		  PN11,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表示度下降事件，属性4，当前记录数
	{0x310B0500,	7,		MAP_SYSDB,		0x3701,	  	  PN11,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表示度下降事件，属性5，最大记录数 
	{0x310B0600,	7,		MAP_SYSDB,		0x310B,		  PN0,   0,		g_bMtrExcEnergyDecFmt,		sizeof(g_bMtrExcEnergyDecFmt),					  NULL}, //电能表示度下降事件，属性6，配置参数   
	{0x310B0700,	7,		MAP_SYSDB,		0x3B11,		  PN1,   0,		g_bMtrExcIc7ValNumFmt,		sizeof(g_bMtrExcIc7ValNumFmt),				  NULL}, //电能表示度下降事件，属性7，当前值记录表 
	{0x310B0800,	7,		MAP_SYSDB,		0x3702,		  PN11,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表示度下降事件，属性8，上报标识
	{0x310B0900,	7,		MAP_SYSDB,		0x3703,		  PN11,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表示度下降事件，属性9，有效标识   

	{0x310C0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "MtrExc_EnergyErr.dat"},//电能表电能量超差事件，属性2，事件记录表
	{0x310C0300,	7,		MAP_SYSDB,		0x3700,		  PN12,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表电能量超差事件，属性3，关联对象属性表
	{0x310C0400,	7,		MAP_SYSDB,		0x3B10,		  PN12,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表电能量超差事件，属性4，当前记录数
	{0x310C0500,	7,		MAP_SYSDB,		0x3701,	  	  PN12,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表电能量超差事件，属性5，最大记录数 
	{0x310C0600,	7,		MAP_SYSDB,		0x310C,		  PN0,   0,		g_bMtrExcEnergyErrFmt,		sizeof(g_bMtrExcEnergyErrFmt),				  NULL}, //电能表电能量超差事件，属性6，配置参数   
	{0x310C0700,	7,		MAP_SYSDB,		0x3B11,		  PN2,   0,		g_bMtrExcIc7ValNumFmt,		sizeof(g_bMtrExcIc7ValNumFmt),				  NULL}, //电能表电能量超差事件，属性7，当前值记录表 
	{0x310C0800,	7,		MAP_SYSDB,		0x3702,		  PN12,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表电能量超差事件，属性8，上报标识
	{0x310C0900,	7,		MAP_SYSDB,		0x3703,		  PN12,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表电能量超差事件，属性9，有效标识   

	{0x310D0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "MtrExc_EnergyFlow.dat"},//电能表飞走事件，属性2，事件记录表
	{0x310D0300,	7,		MAP_SYSDB,		0x3700,		  PN13,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表飞走事件，属性3，关联对象属性表
	{0x310D0400,	7,		MAP_SYSDB,		0x3B10,		  PN13,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表飞走事件，属性4，当前记录数
	{0x310D0500,	7,		MAP_SYSDB,		0x3701,	  	  PN13,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表飞走事件，属性5，最大记录数 
	{0x310D0600,	7,		MAP_SYSDB,		0x310D,		  PN0,   0,		g_bMtrExcEnergyFlowFmt,		sizeof(g_bMtrExcEnergyFlowFmt),				  NULL}, //电能表飞走事件，属性6，配置参数   
	{0x310D0700,	7,		MAP_SYSDB,		0x3B11,		  PN3,   0,		g_bMtrExcIc7ValNumFmt,		sizeof(g_bMtrExcIc7ValNumFmt),				  NULL}, //电能表飞走事件，属性7，当前值记录表 
	{0x310D0800,	7,		MAP_SYSDB,		0x3702,		  PN13,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表飞走事件，属性8，上报标识
	{0x310D0900,	7,		MAP_SYSDB,		0x3703,		  PN13,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表飞走事件，属性9，有效标识   

	{0x310E0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "MtrExc_EnergyStop.dat"},//电能表停走事件，属性2，事件记录表
	{0x310E0300,	7,		MAP_SYSDB,		0x3700,		  PN14,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表停走事件，属性3，关联对象属性表
	{0x310E0400,	7,		MAP_SYSDB,		0x3B10,		  PN14,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表停走事件，属性4，当前记录数
	{0x310E0500,	7,		MAP_SYSDB,		0x3701,	  	  PN14,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表停走事件，属性5，最大记录数 
	{0x310E0600,	7,		MAP_SYSDB,		0x310E,		  PN0,   0,		g_bMtrExcEnergyStopFmt,		sizeof(g_bMtrExcEnergyStopFmt),				  NULL}, //电能表停走事件，属性6，配置参数   
	{0x310E0700,	7,		MAP_SYSDB,		0x3B11,		  PN4,   0,		g_bMtrExcIc7ValNumFmt,		sizeof(g_bMtrExcIc7ValNumFmt),				  NULL}, //电能表停走事件，属性7，当前值记录表 
	{0x310E0800,	7,		MAP_SYSDB,		0x3702,		  PN14,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表停走事件，属性8，上报标识
	{0x310E0900,	7,		MAP_SYSDB,		0x3703,		  PN14,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表停走事件，属性9，有效标识   

	{0x310F0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "MtrExc_MtrRdFail.dat"},//电能表抄表失败事件，属性2，事件记录表
	{0x310F0300,	7,		MAP_SYSDB,		0x3700,		  PN15,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表抄表失败事件，属性3，关联对象属性表
	{0x310F0400,	7,		MAP_SYSDB,		0x3B10,		  PN15,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表抄表失败事件，属性4，当前记录数
	{0x310F0500,	7,		MAP_SYSDB,		0x3701,	  	  PN15,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表抄表失败事件，属性5，最大记录数 
	{0x310F0600,	7,		MAP_SYSDB,		0x310F,		  PN0,   0,		g_bMtrExcMtrRdFailFmt,		sizeof(g_bMtrExcMtrRdFailFmt),				  NULL}, //电能表抄表失败事件，属性6，配置参数   
	{0x310F0700,	7,		MAP_SYSDB,		0x3B11,		  PN5,   0,		g_bMtrExcIc7ValNumFmt,		sizeof(g_bMtrExcIc7ValNumFmt),				  NULL}, //电能表抄表失败事件，属性7，当前值记录表 
	{0x310F0800,	7,		MAP_SYSDB,		0x3702,		  PN15,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表抄表失败事件，属性8，上报标识
	{0x310F0900,	7,		MAP_SYSDB,		0x3703,		  PN15,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表抄表失败事件，属性9，有效标识   

	{0x31100200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_MonthFluxOver.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x31100300,	7,		MAP_SYSDB,		0x3700,		  PN16,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x31100400,	7,		MAP_SYSDB,		0x3B10,		  PN16,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x31100500,	7,		MAP_SYSDB,		0x3701,	  	  PN16,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	{0x31100600,	7,		MAP_SYSDB,		0x3110,		  PN0,   0,		g_bEvtMthFluOverFmt,		sizeof(g_bEvtMthFluOverFmt),				  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x31100700,	7,		MAP_SYSDB,		0x3B12,		  PN16,  0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x31100800,	7,		MAP_SYSDB,		0x3702,		  PN16,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x31100900,	7,		MAP_SYSDB,		0x3703,		  PN16,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   

	{0x31110200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_NewUnknownMtr.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x31110300,	7,		MAP_SYSDB,		0x3700,		  PN17,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x31110400,	7,		MAP_SYSDB,		0x3B10,		  PN17,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x31110500,	7,		MAP_SYSDB,		0x3701,	  	  PN17,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	//{0x31110600,	7,		MAP_SYSDB,		0x3111,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x31110700,	7,		MAP_SYSDB,		0x3B12,		  PN17,  0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x31110800,	7,		MAP_SYSDB,		0x3702,		  PN17,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x31110900,	7,		MAP_SYSDB,		0x3703,		  PN17,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   

	{0x31120200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_MtrOverArea.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x31120300,	7,		MAP_SYSDB,		0x3700,		  PN18,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x31120400,	7,		MAP_SYSDB,		0x3B10,		  PN18,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x31120500,	7,		MAP_SYSDB,		0x3701,	  	  PN18,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	//{0x31120600,	7,		MAP_SYSDB,		0x3112,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x31120700,	7,		MAP_SYSDB,		0x3B12,		  PN18,  0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x31120800,	7,		MAP_SYSDB,		0x3702,		  PN18,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x31120900,	7,		MAP_SYSDB,		0x3703,		  PN18,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   	

	{0x31140200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_AdjTermTime.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x31140300,	7,		MAP_SYSDB,		0x3700,		  PN20,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x31140400,	7,		MAP_SYSDB,		0x3B10,		  PN20,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x31140500,	7,		MAP_SYSDB,		0x3701,	  	  PN20,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	//{0x31140600,	7,		MAP_SYSDB,		0x3114,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x31140700,	7,		MAP_SYSDB,		0x3B12,		  PN20,  0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x31140800,	7,		MAP_SYSDB,		0x3702,		  PN20,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x31140900,	7,		MAP_SYSDB,		0x3703,		  PN20,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   

	{0x31150200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_YKRec.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x31150300,	7,		MAP_SYSDB,		0x3700,		  PN21,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x31150400,	7,		MAP_SYSDB,		0x3B10,		  PN21,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x31150500,	7,		MAP_SYSDB,		0x3701,	  	  PN21,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	//{0x31150600,	7,		MAP_SYSDB,		0x3115,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x31150700,	7,		MAP_SYSDB,		0x3B14,		  PN0,  0,		g_bEvtIc7ValOadFmt,			sizeof(g_bEvtIc7ValOadFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x31150800,	7,		MAP_SYSDB,		0x3702,		  PN21,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x31150900,	7,		MAP_SYSDB,		0x3703,		  PN21,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识

	{0x31160200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_EnergyDiffComp.dat"},//有功总电能量差动越限事件记录，属性2，事件记录表
	{0x31160300,	7,		MAP_SYSDB,		0x3700,		  PN22,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //有功总电能量差动越限事件记录，属性3，关联对象属性表
	{0x31160400,	7,		MAP_SYSDB,		0x3B10,		  PN22,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //有功总电能量差动越限事件记录，属性4，当前记录数
	{0x31160500,	7,		MAP_SYSDB,		0x3701,	  	  PN22,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //有功总电能量差动越限事件记录，属性5，最大记录数 
	{0x31160600,	7,		MAP_SYSDB,		0x3116,		  PN0,   0,		g_bECompareCfgFmt,			sizeof(g_bECompareCfgFmt),					  NULL}, //有功总电能量差动越限事件记录，属性6，配置参数   
	{0x31160700,	7,		MAP_SYSDB,		0x3B12,		  PN22,  0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //有功总电能量差动越限事件记录，属性7，当前值记录表 
	{0x31160800,	7,		MAP_SYSDB,		0x3702,		  PN22,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //有功总电能量差动越限事件记录，属性8，上报标识
	{0x31160900,	7,		MAP_SYSDB,		0x3703,		  PN22,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //有功总电能量差动越限事件记录，属性9，有效标识   

	/*不支持
	{0x31170200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_StatusChg.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x31170300,	7,		MAP_SYSDB,		0x3700,		  PN23,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x31170400,	7,		MAP_SYSDB,		0x3B10,		  PN23,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x31170500,	7,		MAP_SYSDB,		0x3701,	  	  PN23,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	//{0x31170600,	7,		MAP_SYSDB,		0x3117,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x31170700,	7,		MAP_SYSDB,		0x3B12,		  PN23,  0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x31170800,	7,		MAP_SYSDB,		0x3702,		  PN23,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x31170900,	7,		MAP_SYSDB,		0x3703,		  PN23,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   
	*/

	{0x31180200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_PrgRec.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x31180300,	7,		MAP_SYSDB,		0x3700,		  PN24,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x31180400,	7,		MAP_SYSDB,		0x3B10,		  PN24,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x31180500,	7,		MAP_SYSDB,		0x3701,	  	  PN24,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	//{0x31180600,	7,		MAP_SYSDB,		0x3118,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x31180700,	7,		MAP_SYSDB,		0x3B12,		  PN24,  0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x31180800,	7,		MAP_SYSDB,		0x3702,		  PN24,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x31180900,	7,		MAP_SYSDB,		0x3703,		  PN24,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   

	{0x31190200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_ILoopDisorder.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x31190300,	7,		MAP_SYSDB,		0x3700,		  PN25,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x31190400,	7,		MAP_SYSDB,		0x3B10,		  PN25,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x31190500,	7,		MAP_SYSDB,		0x3701,	  	  PN25,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	//{0x31190600,	7,		MAP_SYSDB,		0x3119,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x31190700,	7,		MAP_SYSDB,		0x3B15,		  PN0,  0,		g_bEvtIc7ValEnumFmt,			sizeof(g_bEvtIc7ValEnumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x31190800,	7,		MAP_SYSDB,		0x3702,		  PN25,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x31190900,	7,		MAP_SYSDB,		0x3703,		  PN25,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   
	/*不支持	
	{0x311A0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_MtrNetStateChg.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x311A0300,	7,		MAP_SYSDB,		0x3700,		  PN26,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x311A0400,	7,		MAP_SYSDB,		0x3B10,		  PN26,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x311A0500,	7,		MAP_SYSDB,		0x3701,	  	  PN26,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	{0x311A0600,	7,		MAP_SYSDB,		0x311A,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x311A0700,	7,		MAP_SYSDB,		0x3B12,		  PN26,  0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x311A0800,	7,		MAP_SYSDB,		0x3702,		  PN26,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x311A0900,	7,		MAP_SYSDB,		0x3703,		  PN26,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识
	*/
	{0x311B0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_AdjMtrTime.dat"},//电能表时钟超差事件，属性2，事件记录表
	//{0x311B0300,	7,		MAP_SYSDB,		0x3700,		  PN27,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表，不支持
	{0x311B0400,	7,		MAP_SYSDB,		0x3B10,		  PN27,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x311B0500,	7,		MAP_SYSDB,		0x3701,	  	  PN27,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	//{0x311B0600,	7,		MAP_SYSDB,		0x311B,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x311B0700,	7,		MAP_SYSDB,		0x3B18,		  PN0,  0,		g_bEvtIc7ValTsaFmt,			sizeof(g_bEvtIc7ValTsaFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x311B0800,	7,		MAP_SYSDB,		0x3702,		  PN27,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x311B0900,	7,		MAP_SYSDB,		0x3703,		  PN27,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   

	{0x311C0200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "MtrExc__MtrDataChg.dat"},//电能表数据变更监控记录，属性2，事件记录表
	{0x311C0300,	7,		MAP_SYSDB,		0x3700,		  PN28,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表数据变更监控记录，属性3，关联对象属性表
	{0x311C0400,	7,		MAP_SYSDB,		0x3B10,		  PN28,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表数据变更监控记录，属性4，当前记录数
	{0x311C0500,	7,		MAP_SYSDB,		0x3701,	  	  PN28,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表数据变更监控记录，属性5，最大记录数 
	{0x311C0600,	7,		MAP_SYSDB,		0x311C,		  PN0,   0,		g_bMtrDataChgFmt,				sizeof(g_bMtrDataChgFmt),						  NULL}, //电能表数据变更监控记录，属性6，配置参数   
	{0x311C0700,	7,		MAP_SYSDB,		0x3B11,		  PN6,  0,		g_bMtrExcIc7ValNumFmt,			sizeof(g_bMtrExcIc7ValNumFmt),					  NULL}, //电能表数据变更监控记录，属性7，当前值记录表 
	{0x311C0800,	7,		MAP_SYSDB,		0x3702,		  PN28,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表数据变更监控记录，属性8，上报标识
	{0x311C0900,	7,		MAP_SYSDB,		0x3703,		  PN28,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表数据变更监控记录，属性9，有效标识   

	{0x32000200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_PowerCtrlRec.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x32000300,	7,		MAP_SYSDB,		0x3700,		  PN29,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x32000400,	7,		MAP_SYSDB,		0x3B10,		  PN29,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x32000500,	7,		MAP_SYSDB,		0x3701,	  	  PN29,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	//{0x32000600,	7,		MAP_SYSDB,		0x3200,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x32000700,	7,		MAP_SYSDB,		0x3B12,		  PN29,  0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x32000800,	7,		MAP_SYSDB,		0x3702,		  PN29,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x32000900,	7,		MAP_SYSDB,		0x3703,		  PN29,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   

	{0x32010200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_EnergyCtrlRec.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x32010300,	7,		MAP_SYSDB,		0x3700,		  PN30,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x32010400,	7,		MAP_SYSDB,		0x3B10,		  PN30,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x32010500,	7,		MAP_SYSDB,		0x3701,	  	  PN30,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	//{0x32010600,	7,		MAP_SYSDB,		0x3201,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x32010700,	7,		MAP_SYSDB,		0x3B12,		  PN30,  0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x32010800,	7,		MAP_SYSDB,		0x3702,		  PN30,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x32010900,	7,		MAP_SYSDB,		0x3703,		  PN30,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   

	{0x32020200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_EnergyBuyParaChg.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x32020300,	7,		MAP_SYSDB,		0x3700,		  PN31,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x32020400,	7,		MAP_SYSDB,		0x3B10,		  PN31,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x32020500,	7,		MAP_SYSDB,		0x3701,	  	  PN31,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	//{0x32020600,	7,		MAP_SYSDB,		0x3202,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x32020700,	7,		MAP_SYSDB,		0x3B16,		  PN0,  0,		g_bEvtIc7ValOiFmt,			sizeof(g_bEvtIc7ValOiFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x32020800,	7,		MAP_SYSDB,		0x3702,		  PN31,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x32020900,	7,		MAP_SYSDB,		0x3703,		  PN31,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   

	{0x32030200,	7,		MAP_TASKDB,		0,		  	  PN0,   0,		NULL,						0,								  "TermExc_EnergyCtrlAlrRec.dat"},//电能表时钟超差事件，属性2，事件记录表
	{0x32030300,	7,		MAP_SYSDB,		0x3700,		  PN32,  0,		g_bEvtCapOADFmt,			sizeof(g_bEvtCapOADFmt),					  NULL}, //电能表时钟超差事件，属性3，关联对象属性表
	{0x32030400,	7,		MAP_SYSDB,		0x3B10,		  PN32,  0,		g_bEvtIc7RecNumFmt,			sizeof(g_bEvtIc7RecNumFmt),					  NULL}, //电能表时钟超差事件，属性4，当前记录数
	{0x32030500,	7,		MAP_SYSDB,		0x3701,	  	  PN32,  0,		g_bEvtMaxNumFmt,			sizeof(g_bEvtMaxNumFmt),					  NULL}, //电能表时钟超差事件，属性5，最大记录数 
	//{0x32030600,	7,		MAP_SYSDB,		0x3203,		  PN0,   0,		g_bDcOverUpFmt,				sizeof(g_bDcOverUpFmt),						  NULL}, //电能表时钟超差事件，属性6，配置参数   
	{0x32030700,	7,		MAP_SYSDB,		0x3B12,		  PN32,  0,		g_bEvtIc7ValNumFmt,			sizeof(g_bEvtIc7ValNumFmt),					  NULL}, //电能表时钟超差事件，属性7，当前值记录表 
	{0x32030800,	7,		MAP_SYSDB,		0x3702,		  PN32,  0,		g_bEvtRptFlagFmt,			sizeof(g_bEvtRptFlagFmt),					  NULL}, //电能表时钟超差事件，属性8，上报标识
	{0x32030900,	7,		MAP_SYSDB,		0x3703,		  PN32,  0,		g_bEvtValidFlagFmt,			sizeof(g_bEvtValidFlagFmt),					  NULL}, //电能表时钟超差事件，属性9，有效标识   

	{0x33000200,	8,		MAP_SYSDB,		0x3300,	 	 PN0,   0,		g_bChnRptStateFmt,				sizeof(g_bChnRptStateFmt),		NULL},//事件上报状态
	{0x33020206,	8,		MAP_SYSDB,		0x3A00,	 	 PN0,   0,		g_bEvtTermPrgOADFmt,				sizeof(g_bEvtTermPrgOADFmt),		NULL},//编程记录事件单元∷编程对象列表  array OAD	
	{0x33030206,	8,		MAP_SYSDB,		0x3A01, 	 PN0,	0,		g_bSchMtrRltPerRecFmt,				sizeof(g_bSchMtrRltPerRecFmt),		NULL},//搜表结果      array 一个搜表结果	
	{0x33040206,	8,		MAP_SYSDB,		0x3A02, 	 PN0,	0,		g_bEvtStepAreaFmt,				sizeof(g_bEvtStepAreaFmt),		NULL},//跨台区搜表结果  array  一个跨台区结果	
	{0x33050206,	8,		MAP_SYSDB,		0x3A03,	 	 PN0,   0,		g_bEvtLong64Fmt,					sizeof(g_bEvtLong64Fmt),		NULL},//功控跳闸记录单元∷事件发生后2分钟功率  long64(单位：W，换算-1)，
	{0x33050207,	8,		MAP_SYSDB,		0x3A04,	 	 PN0,   0,		g_bEvtOIFmt,						sizeof(g_bEvtOIFmt),		NULL},//功控跳闸记录单元∷控制对象      OI，
	{0x33050208,	8,		MAP_SYSDB,		0x3A05,	 	 PN0,   0,		g_bEvtBitStrFmt,					sizeof(g_bEvtBitStrFmt),		NULL},//功控跳闸记录单元∷跳闸轮次      bit-string(SIZE(8))，
	{0x33050209,	8,		MAP_SYSDB,		0x3A06,	 	 PN0,   0,		g_bEvtLong64Fmt,					sizeof(g_bEvtLong64Fmt),		NULL},//功控跳闸记录单元∷功控定值      long64（单位：kWh，换算-4），
	{0x3305020A,	8,		MAP_SYSDB,		0x3A07,	 	 PN0,   0,		g_bEvtLong64Fmt,					sizeof(g_bEvtLong64Fmt),		NULL},//功控跳闸记录单元∷跳闸发生前总加有功功率    long64（单位：kW，换算-4），
	{0x33060206,	8,		MAP_SYSDB,		0x3A08,	 	 PN0,   0,		g_bEvtOIFmt,						sizeof(g_bEvtOIFmt),		NULL},//电控跳闸记录单元∷控制对象      OI，
	{0x33060207,	8,		MAP_SYSDB,		0x3A09,	 	 PN0,   0,		g_bEvtBitStrFmt,					sizeof(g_bEvtBitStrFmt),		NULL},//电控跳闸记录单元∷跳闸轮次      bit-string(SIZE(8))，
	{0x33060208,	8,		MAP_SYSDB,		0x3A0A,	 	 PN0,   0,		g_bEvtLong64Fmt,					sizeof(g_bEvtLong64Fmt),		NULL},//电控跳闸记录单元∷电控定值      long64（单位：kWh，换算-4），
	{0x33060209,	8,		MAP_SYSDB,		0x3A0B,	 	 PN0,   0,		g_bEvtLong64Fmt,					sizeof(g_bEvtLong64Fmt),		NULL},//电控跳闸记录单元∷跳闸发生时总加电能量  long64（单位：kwh/元，换算-4）
	{0x33070206,	8,		MAP_SYSDB,		0x3A0C,	 	 PN0,   0,		g_bEvtOIFmt,						sizeof(g_bEvtOIFmt),		NULL},//电控告警事件单元∷控制对象      OI，
	{0x33070207,	8,		MAP_SYSDB,		0x3A0D,	 	 PN0,   0,		g_bEvtLong64Fmt,					sizeof(g_bEvtLong64Fmt),		NULL},//电控告警事件单元∷电控定值      long64（单位：kWh，换算-4），	
	{0x33080206,	8,		MAP_SYSDB,		0x3A0E,	 	 PN0,   0,		g_bEvtMaxDmdFmt,				sizeof(g_bEvtMaxDmdFmt),		NULL},//电能表需量超限事件单元∷超限期间需量最大值  double-long-unsigned
	{0x33080207,	8,		MAP_SYSDB,		0x3A0F,	 	 PN0,   0,		g_bEvtMaxDmdDateFmt,			sizeof(g_bEvtMaxDmdDateFmt),		NULL},//电能表需量超限事件单元∷超限期间需量最大值发生时间  date_time_s
	{0x33090206,	8,		MAP_SYSDB,		0x3A10,	 	 PN0,   0,		g_bEvtBitStrFmt,					sizeof(g_bEvtBitStrFmt),		NULL},//停/上电事件记录单元∷属性标志     bit-string（SIZE(8)）
	{0x330B0206,	8,		MAP_SYSDB,		0x3A12,	 	 PN0,   0,		g_bEvtLong64Fmt,					sizeof(g_bEvtLong64Fmt),		NULL},//有功总电能量差动越限事件记录单元∷越限时对比总加组有功总电能量 long64（单位：kWh，换算：-4），
	{0x330B0207,	8,		MAP_SYSDB,		0x3A13,	 	 PN0,   0,		g_bEvtLong64Fmt,						sizeof(g_bEvtLong64Fmt),		NULL},//有功总电能量差动越限事件记录单元∷越限时参照总加组有功总电能量 long64（单位：kWh，换算：-4），
	{0x330B0208,	8,		MAP_SYSDB,		0x3A14,	 	 PN0,   0,		g_bEvtIntFmt,					sizeof(g_bEvtIntFmt),		NULL},//有功总电能量差动越限事件记录单元∷越限时差动越限相对偏差值 integer（单位：%，换算：0）	
	{0x330B0209,	8,		MAP_SYSDB,		0x3A15,	 	 PN0,   0,		g_bEvtLong64Fmt,					sizeof(g_bEvtLong64Fmt),		NULL},//有功总电能量差动越限事件记录单元∷越限时差动越限绝对偏差值 long64（单位：kWh，换算：-4）	
	{0x330A0206,	8,		MAP_SYSDB,		0x3A11,	 	 PN0,   0,		g_bEvtYKCtrlPEFmt,				sizeof(g_bEvtYKCtrlPEFmt),		NULL},//遥控事件记录单元∷控后2分钟总加组功率 array long64
	{0x330C0206,	8,		MAP_SYSDB,		0x3A16,	 	 PN0,   0,		g_bEvtClearrOMDFmt,				sizeof(g_bEvtClearrOMDFmt),		NULL},//事件清零事件记录单元∷事件清零事件记录单元，array OMD
	{0x330D0206,	8,		MAP_SYSDB,		0x3A17,	 	 PN0,   0,		g_bEvtMtrClkTimeFmt,				sizeof(g_bEvtMtrClkTimeFmt),		NULL},//终端对电表校时记录单元∷校时前时钟    date_time_s
	{0x330D0207,	8,		MAP_SYSDB,		0x3A18,	 	 PN0,   0,		g_bEvtMtrClkErrFmt,				sizeof(g_bEvtMtrClkErrFmt),		NULL},//终端对电表校时记录单元∷时钟误差      integer（单位：秒，无换算）
	{0x330F0206,	8,		MAP_SYSDB,		0x3A1A,	 	 PN0,   0,		NULL,						NULL,		NULL},//电能表数据变更监控记录单元:: 监控数据对象  CSD，
	{0x330F0207,	8,		MAP_SYSDB,		0x3A1B,	 	 PN0,   0,		NULL,					NULL,		NULL},//电能表数据变更监控记录单元:: 变化前数据    Data，
	{0x330F0208,	8,		MAP_SYSDB,		0x3A1C,	 	 PN0,   0,		NULL,					NULL,		NULL},//电能表数据变更监控记录单元:: 变化后数据    Data，
	{0x33200200,	8,		MAP_SYSDB,		0x3320,	 	 PN0,   0,		g_bAddEvtOadFmt,				sizeof(g_bAddEvtOadFmt),		NULL},////新增对象3320，class_id = 8，属性2∷= array OI
	{0x33200300,	8,		MAP_SYSDB,		0x3704,	 	 PN0,   0,		g_bAddEvtOIFmt,				sizeof(g_bAddEvtOIFmt),		NULL},//属性3（需上报事件对象列表，只读）∷= array OI

	///////////////////////////////////////////////////////////////////////////////////////////

	{0x40000200,	8,		MAP_SYSDB,		0x4000,		PN0,   0,		g_bEvtTimeFmt,				sizeof(g_bEvtTimeFmt),			NULL},	//日期时间，属性2
	{0x40000300,	8,		MAP_SYSDB,		0x4520,		PN0,   0,		g_bAdjTimeModeFmt,			sizeof(g_bAdjTimeModeFmt),		NULL},	//日期时间，属性3，校时模式
	{0x40000400,	8,		MAP_SYSDB,		0x4521,		PN0,   0,		g_bPreAdjParaFmt,			sizeof(g_bPreAdjParaFmt),		NULL},	//日期时间，属性4，精准校时参数

	{0x40010200,	8,		MAP_SYSDB,		0x4001,		PN0,   0,		g_bServAddrFmt,				sizeof(g_bServAddrFmt),			NULL},	
	{0x40020200,	8,		MAP_SYSDB,		0x4002,		PN0,   0,		g_bMtrAddr,					sizeof(g_bMtrAddr),				NULL},	//表号
	{0x40030200,	8,		MAP_SYSDB,		0x4003,		PN0,   0,		g_bCliCode,					sizeof(g_bCliCode),				NULL},	//客户编号
	{0x40040200,	8,		MAP_SYSDB,		0x4004,		PN0,   0,		g_bDevLocat,				sizeof(g_bDevLocat),			NULL},	//设备地理位置

	{0x40050200,	8,		MAP_SYSDB,		0x4005,		PN0,   0,		g_bGroupAddr,				sizeof(g_bGroupAddr),			NULL},	//组地址

	{0x40060200,	8,		MAP_SYSDB,		0x4006,		PN0,   0,		g_bClkSrc,					sizeof(g_bClkSrc),				NULL},	//时钟源
	{0x40070200,	8,		MAP_SYSDB,		0x4007,		PN0,   0,		g_bDispParam,				sizeof(g_bDispParam),			NULL},	//显示参数
	{0x40080200,	8,		MAP_SYSDB,		0x4008, 	PN0,   0,		g_bTimeZoneSwitchTimeParam,	sizeof(g_bTimeZoneSwitchTimeParam),			NULL},	//两套时区表切换时间
	{0x40090200,	8,		MAP_SYSDB,		0x4009, 	PN0,   0,		g_bDayChartSwitchTimeParam, sizeof(g_bDayChartSwitchTimeParam), 		NULL},	//两套时段切换时间
	{0x400c0200,	8,		MAP_SYSDB,		0x400c, 	PN0,   0,		g_bTimeZoneDayChartParam, 	sizeof(g_bTimeZoneDayChartParam), 		NULL},	//时区时段数
	{0x400d0200,	8,		MAP_SYSDB,		0x400d, 	PN0,   0,		g_bUnSignTypeFmt, 	sizeof(g_bTimeZoneDayChartParam), 		NULL},	//阶梯数
	{0x400e0200,	8,		MAP_SYSDB,		0x400e, 	PN0,   0,		g_bUnSignTypeFmt,			sizeof(g_bUnSignTypeFmt),		NULL},	//谐波分析次数

	{0x40100200,	8,		MAP_SYSDB,		0x4010,		PN0,   0,		g_bUnSignTypeFmt,			sizeof(g_bUnSignTypeFmt),		NULL},	//计量元件数
	{0x40110200,	8,		MAP_SYSDB,		0x4011, 	PN0,   0,		g_bHolidayParam,			sizeof(g_bHolidayParam),		NULL},	//公共假日表
	{0x40120200,	8,		MAP_SYSDB,		0x4012, 	PN0,   0,		g_bRestStatusParam,			sizeof(g_bRestStatusParam),		NULL},	//周休日特征字
	{0x40130200,	8,		MAP_SYSDB,		0x4013, 	PN0,   0,		g_bRestDayChartParam, 		sizeof(g_bRestDayChartParam), 	NULL},	//周休日釆用的日时段表号
	{0x40140200,	8,		MAP_SYSDB,		0x4014, 	PN0,   0,		g_bZoneNumParam,		sizeof(g_bZoneNumParam),	NULL},	//当前套时区表
	{0x40150200,	8,		MAP_SYSDB,		0x4015, 	PN0,   0,		g_bZoneNumParam,		sizeof(g_bZoneNumParam),	NULL},	//备用套时区表
	{0x40160200,	8,		MAP_SYSDB,		0x4016, 	PN0,   0,		g_bDayChartParam,		sizeof(g_bDayChartParam),	NULL},	//当前套日时段表
	{0x40170200,	8,		MAP_SYSDB,		0x4017, 	PN0,   0,		g_bDayChartParam,		sizeof(g_bDayChartParam),	NULL},	//备用套日时段表
	{0x40300200,	8,		MAP_SYSDB,		0x4030,		PN0,   0,		g_bVolParaFmt,			sizeof(g_bVolParaFmt),		NULL},	//电压合格率参数
	{0x41000200,	8,		MAP_SYSDB,		0x4100,		PN0,   0,		g_bUnSignTypeFmt,			sizeof(g_bUnSignTypeFmt),		NULL},  //最大需量周期
	{0x41010200,	8,		MAP_SYSDB,		0x4101, 	PN0,   0,		g_bUnSignTypeFmt,			sizeof(g_bUnSignTypeFmt),		NULL},	//滑差时间
	{0x41020200,	8,		MAP_SYSDB,		0x4102, 	PN0,   0,		g_bUnSignTypeFmt,			sizeof(g_bUnSignTypeFmt),		NULL},	//校表脉冲宽度
	{0x41030200,	8,		MAP_SYSDB,		0x4103, 	PN0,   0,		bAssetMagCode,				sizeof(bAssetMagCode),			NULL},	//资产管理编码
	{0x41040200,	8,		MAP_SYSDB,		0x4104, 	PN0,   0,		gbUnString,					sizeof(gbUnString),				NULL},	//额定电压 
	{0x41050200,	8,		MAP_SYSDB,		0x4105, 	PN0,   0,		gbIbString, 				sizeof(gbIbString), 			NULL},	//额定电流
	{0x41060200,	8,		MAP_SYSDB,		0x4106, 	PN0,   0,		gbImaxString, 				sizeof(gbImaxString), 			NULL},	//最大电流
	{0x41070200,	8,		MAP_SYSDB,		0x4107, 	PN0,   0,		gbPClassString,				sizeof(gbPClassString),			NULL},	//有功准确度等级
	{0x41080200,	8,		MAP_SYSDB,		0x4108, 	PN0,   0,		gbQClassString, 			sizeof(gbQClassString), 		NULL},	//无功准确度等级
	{0x41090200,	8,		MAP_SYSDB,		0x4109, 	PN0,   0,		gbPImpConst, 				sizeof(gbPImpConst), 			NULL},	//电能表有功常数 
	{0x410a0200,	8,		MAP_SYSDB,		0x410a, 	PN0,   0,		gbQImpConst,				sizeof(gbQImpConst),			NULL},	//电能表无功常数 
	{0x410b0200,	8,		MAP_SYSDB,		0x410b, 	PN0,   0,		gbMeterTypeString,			sizeof(gbMeterTypeString),		NULL},	//电能表型号
	{0x41120200,	8,		MAP_SYSDB,		0x4112, 	PN0,   0,		g_bBitStringTypeFmt,		sizeof(g_bBitStringTypeFmt),	NULL},	//有功组合方式特征字
	{0x41130200,	8,		MAP_SYSDB,		0x4113, 	PN0,   0,		g_bBitStringTypeFmt,		sizeof(g_bBitStringTypeFmt),	NULL},	//无功组合方式1特征字
	{0x41140200,	8,		MAP_SYSDB,		0x4114, 	PN0,   0,		g_bBitStringTypeFmt,		sizeof(g_bBitStringTypeFmt),	NULL},	//无功组合方式2特征字
	{0x41160200,	8, 		MAP_SYSDB,		0x4116, 	PN0,   0,		g_bBalanceDayFmt,			sizeof(g_bBalanceDayFmt),		NULL},	//结算日参数
	{0x42040200,	8,		MAP_SYSDB,		0x4204,		PN0,   0,		bTermBroadTime,				sizeof(bTermBroadTime),			NULL},  //终端广播校时参数
	{0x42040300,	8,		MAP_SYSDB,		0x4205,		PN0,   0,		bSigAddrBroadTime,			sizeof(bSigAddrBroadTime),		NULL},  //终端单地址广播校时参数


	//电气设备
	{0x43000200,	9,		MAP_SYSDB,		0x4302,	  PN0,   0,		g_bEleDevDsc,		sizeof(g_bEleDevDsc),				NULL},	//电气设备——设备描述符
	{0x43000300,	9,		MAP_SYSDB,		0x4303,	  PN0,   0,		g_bEleVerInfo,		sizeof(g_bEleVerInfo),				NULL},	//电气设备——版本信息
	{0x43000400,	9,		MAP_SYSDB,		0x4304,	  PN0,   0,		g_bManufactureDate, sizeof(g_bManufactureDate),			NULL},	//电气设备——生产日期
	{0x43000500,	9,		MAP_SYSDB,		0x4305,	  PN0,   0,		NULL,								0,					NULL},	//电气设备——子设备列表
	{0x43000600,	9,		MAP_SYSDB,		0x4306,	  PN0,   0,		g_bMastProType,		sizeof(g_bMastProType),				NULL},	//电气设备——支持规约列表
	{0x43000700,	9,		MAP_SYSDB,		0x4307,	  PN0,   0,		g_bRptFlowFlg,		sizeof(g_bRptFlowFlg),				NULL},	//电气设备——允许跟随上报
	{0x43000800,	9,		MAP_SYSDB,		0x4308,	  PN0,   0,		g_bRptFlg,			sizeof(g_bRptFlg),					NULL},	//电气设备——允许主动上报
	{0x43000900,	9,		MAP_SYSDB,		0x4309,	  PN0,   0,		g_bMastCall,		sizeof(g_bMastCall),				NULL},	//电气设备——允许与主动通话
	{0x43000a00,	9,		MAP_SYSDB,		0x430a,	  PN0,   0,		g_bMasRptCn,			sizeof(g_bMasRptCn),					NULL},	//电气设备——上报通道

	//公网通信模块1
	{0x45000200,	25,		MAP_SYSDB,		0x4500,	  PN0,   0,		g_bGprsCommCfg,		sizeof(g_bGprsCommCfg),				NULL},	//通讯配置
	{0x45000300,	25,		MAP_SYSDB,		0x4501,	  PN0,   0,		g_bMastCommPara,	sizeof(g_bMastCommPara),			NULL},	//主站通信参数表
	{0x45000400,	25,		MAP_SYSDB,		0x4502,	  PN0,   0,		g_bSmsCommPara,		sizeof(g_bSmsCommPara),				NULL},	//短信通信参数
	{0x45000500,	25,		MAP_SYSDB,		0x4503,	  PN0,   0,		g_bEleVerInfo,		sizeof(g_bEleVerInfo),				NULL},	//版本信息
	{0x45000600,	25,		MAP_SYSDB,		0x4504,	  PN0,   0,		NULL,				0,									NULL},	//支持规约列表
	{0x45000700,	25,		MAP_SYSDB,		0x4505,	  PN0,   0,		g_bSimCCID,			sizeof(g_bSimCCID),					NULL},	//SIM卡ICCID
	{0x45000800,	25,		MAP_SYSDB,		0x4506,	  PN0,   0,		g_bIMSI,			sizeof(g_bIMSI),					NULL},	//IMSI
	{0x45000900,	25,		MAP_SYSDB,		0x4507,	  PN0,   0,		g_bSigStrenth,		sizeof(g_bSigStrenth),				NULL},	//信号强度
	{0x45000A00,	25,		MAP_SYSDB,		0x4508,	  PN0,   0,		g_bSimNo,			sizeof(g_bSimNo),					NULL},	//SIM卡号码
	{0x45000B00,	25,		MAP_SYSDB,		0x4509,	  PN0,   0,		g_DialIp,			sizeof(g_DialIp),					NULL},	//拨号IP

	//公网通信模块2
	{0x45010200,	25,		MAP_SYSDB,		0x4500,	  PN1,   0,		g_bGprsCommCfg,		sizeof(g_bGprsCommCfg),				NULL},	//通讯配置
	{0x45010300,	25,		MAP_SYSDB,		0x4501,	  PN1,   0,		g_bMastCommPara,	sizeof(g_bMastCommPara),			NULL},	//主站通信参数表
	{0x45010400,	25,		MAP_SYSDB,		0x4502,	  PN1,   0,		g_bSmsCommPara,		sizeof(g_bSmsCommPara),				NULL},	//短信通信参数
	{0x45010500,	25,		MAP_SYSDB,		0x4503,	  PN1,   0,		g_bEleVerInfo,		sizeof(g_bEleVerInfo),				NULL},	//版本信息
	{0x45010600,	25,		MAP_SYSDB,		0x4504,	  PN1,   0,		NULL,				0,									NULL},	//支持规约列表
	{0x45010700,	25,		MAP_SYSDB,		0x4505,	  PN1,   0,		g_bSimCCID,			sizeof(g_bSimCCID),					NULL},	//SIM卡ICCID
	{0x45010800,	25,		MAP_SYSDB,		0x4506,	  PN1,   0,		g_bIMSI,			sizeof(g_bIMSI),					NULL},	//IMSI
	{0x45010900,	25,		MAP_SYSDB,		0x4507,	  PN1,   0,		g_bSigStrenth,		sizeof(g_bSigStrenth),				NULL},	//信号强度
	{0x45010A00,	25,		MAP_SYSDB,		0x4508,	  PN1,   0,		g_bSimNo,			sizeof(g_bSimNo),					NULL},	//SIM卡号码
	{0x45010B00,	25,		MAP_SYSDB,		0x4509,	  PN1,   0,		g_DialIp,			sizeof(g_DialIp),					NULL},	//拨号IP

	//0x4510以太网通信模块1
	{0x45100200,	26,		MAP_SYSDB,		0x4510,	  PN0,   0,		g_bEthCommCfg,		sizeof(g_bEthCommCfg),				NULL},	//通讯配置
	{0x45100300,	26,		MAP_SYSDB,		0x4511,	  PN0,   0,		g_bMastCommPara,	sizeof(g_bMastCommPara),			NULL},	//主站通信参数表
	{0x45100400,	26,		MAP_SYSDB,		0x4512,	  PN0,   0,		g_bEthNetCfg,		sizeof(g_bEthNetCfg),				NULL},	//网络配置参数
	{0x45100500,	26,		MAP_SYSDB,		0x4513,	  PN0,   0,		g_bEthMacCfg,		sizeof(g_bEthMacCfg),				NULL},	//MAC地址

	//0x4510以太网通信模块2
	{0x45110200,	26,		MAP_SYSDB,		0x4510,	  PN1,   0,		g_bEthCommCfg,		sizeof(g_bEthCommCfg),				NULL},	//通讯配置
	{0x45110300,	26,		MAP_SYSDB,		0x4511,	  PN1,   0,		g_bMastCommPara,	sizeof(g_bMastCommPara),			NULL},	//主站通信参数表
	{0x45110400,	26,		MAP_SYSDB,		0x4512,	  PN1,   0,		g_bEthNetCfg,		sizeof(g_bEthNetCfg),				NULL},	//网络配置参数
	{0x45110500,	26,		MAP_SYSDB,		0x4513,	  PN1,   0,		g_bEthMacCfg,		sizeof(g_bEthMacCfg),				NULL},	//MAC地址

	//0x4510以太网通信模块3
	{0x45120200,	26,		MAP_SYSDB,		0x4510,	  PN2,   0,		g_bEthCommCfg,		sizeof(g_bEthCommCfg),				NULL},	//通讯配置
	{0x45120300,	26,		MAP_SYSDB,		0x4511,	  PN2,   0,		g_bMastCommPara,	sizeof(g_bMastCommPara),			NULL},	//主站通信参数表
	{0x45120400,	26,		MAP_SYSDB,		0x4512,	  PN2,   0,		g_bEthNetCfg,		sizeof(g_bEthNetCfg),				NULL},	//网络配置参数
	{0x45120500,	26,		MAP_SYSDB,		0x4513,	  PN2,   0,		g_bEthMacCfg,		sizeof(g_bEthMacCfg),				NULL},	//MAC地址

	//0x4510以太网通信模块4
	{0x45130200,	26,		MAP_SYSDB,		0x4510,	  PN3,   0,		g_bEthCommCfg,		sizeof(g_bEthCommCfg),				NULL},	//通讯配置
	{0x45130300,	26,		MAP_SYSDB,		0x4511,	  PN3,   0,		g_bMastCommPara,	sizeof(g_bMastCommPara),			NULL},	//主站通信参数表
	{0x45130400,	26,		MAP_SYSDB,		0x4512,	  PN3,   0,		g_bEthNetCfg,		sizeof(g_bEthNetCfg),				NULL},	//网络配置参数
	{0x45130500,	26,		MAP_SYSDB,		0x4513,	  PN3,   0,		g_bEthMacCfg,		sizeof(g_bEthMacCfg),				NULL},	//MAC地址

	//0x4510以太网通信模块5
	{0x45140200,	26,		MAP_SYSDB,		0x4510,	  PN4,   0,		g_bEthCommCfg,		sizeof(g_bEthCommCfg),				NULL},	//通讯配置
	{0x45140300,	26,		MAP_SYSDB,		0x4511,	  PN4,   0,		g_bMastCommPara,	sizeof(g_bMastCommPara),			NULL},	//主站通信参数表
	{0x45140400,	26,		MAP_SYSDB,		0x4512,	  PN4,   0,		g_bEthNetCfg,		sizeof(g_bEthNetCfg),				NULL},	//网络配置参数
	{0x45140500,	26,		MAP_SYSDB,		0x4513,	  PN4,   0,		g_bEthMacCfg,		sizeof(g_bEthMacCfg),				NULL},	//MAC地址

	//0x4510以太网通信模块6
	{0x45150200,	26,		MAP_SYSDB,		0x4510,	  PN5,   0,		g_bEthCommCfg,		sizeof(g_bEthCommCfg),				NULL},	//通讯配置
	{0x45150300,	26,		MAP_SYSDB,		0x4511,	  PN5,   0,		g_bMastCommPara,	sizeof(g_bMastCommPara),			NULL},	//主站通信参数表
	{0x45150400,	26,		MAP_SYSDB,		0x4512,	  PN5,   0,		g_bEthNetCfg,		sizeof(g_bEthNetCfg),				NULL},	//网络配置参数
	{0x45150500,	26,		MAP_SYSDB,		0x4513,	  PN5,   0,		g_bEthMacCfg,		sizeof(g_bEthMacCfg),				NULL},	//MAC地址

	//0x4510以太网通信模块7
	{0x45160200,	26,		MAP_SYSDB,		0x4510,	  PN6,   0,		g_bEthCommCfg,		sizeof(g_bEthCommCfg),				NULL},	//通讯配置
	{0x45160300,	26,		MAP_SYSDB,		0x4511,	  PN6,   0,		g_bMastCommPara,	sizeof(g_bMastCommPara),			NULL},	//主站通信参数表
	{0x45160400,	26,		MAP_SYSDB,		0x4512,	  PN6,   0,		g_bEthNetCfg,		sizeof(g_bEthNetCfg),				NULL},	//网络配置参数
	{0x45160500,	26,		MAP_SYSDB,		0x4513,	  PN6,   0,		g_bEthMacCfg,		sizeof(g_bEthMacCfg),				NULL},	//MAC地址

	//0x4510以太网通信模块8
	{0x45170200,	26,		MAP_SYSDB,		0x4510,	  PN7,   0,		g_bEthCommCfg,		sizeof(g_bEthCommCfg),				NULL},	//通讯配置
	{0x45170300,	26,		MAP_SYSDB,		0x4511,	  PN7,   0,		g_bMastCommPara,	sizeof(g_bMastCommPara),			NULL},	//主站通信参数表
	{0x45170400,	26,		MAP_SYSDB,		0x4512,	  PN7,   0,		g_bEthNetCfg,		sizeof(g_bEthNetCfg),				NULL},	//网络配置参数
	{0x45170500,	26,		MAP_SYSDB,		0x4513,	  PN7,   0,		g_bEthMacCfg,		sizeof(g_bEthMacCfg),				NULL},	//MAC地址


	{0x50000200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x50000300,	9,		MAP_SYSDB,		0x5000,	  PN0,   0,		g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},
	{0x50010200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x50010300,	9,		MAP_SYSDB,		0x5000,	  PN1,   0,		g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},
	{0x50020200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x50020300,	9,		MAP_SYSDB,		0x5000,	  PN2,   0,		g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},
	{0x50030200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x50030300,	9,		MAP_SYSDB,		0x5000,	  PN3,   0,		g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},
	{0x50040200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x50040300,	9,		MAP_SYSDB,		0x5000,	  PN4,   0,		g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},

	{0x50050200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x50050300,	9,		MAP_SYSDB,		0x5000,	  PN5,   0,		g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},
	{0x50060200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x50060300,	9,		MAP_SYSDB,		0x5000,	  PN6,   0,		g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},
	{0x50070200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x50070300,	9,		MAP_SYSDB,		0x5000,	  PN7,   0,		g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},

	{0x50080200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x50080300,	9,		MAP_SYSDB,		0x5000,	  PN8,   0,		g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},
	{0x50090200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x50090300,	9,		MAP_SYSDB,		0x5000,	  PN9,   0,		g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},

	{0x500A0200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x500A0300,	9,		MAP_SYSDB,		0x5000,	  PN10,   0,	g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},
	{0x500B0200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x500B0300,	9,		MAP_SYSDB,		0x5000,	  PN11,   0,	g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},

	{0x50110200,	9,		MAP_TASKDB,		0,		  PN0,   0,		NULL,		   0,					  FMT_FRZ_TASK_TABLE},
	{0x50110300,	9,		MAP_SYSDB,		0x5000,	  PN12,   0,	g_bFrzParaFmt, sizeof(g_bFrzParaFmt), NULL},

	{0x60000200,	1,		MAP_SYSDB,		0x6000,	  PN0,   0,		g_bPnParaFmt, sizeof(g_bPnParaFmt)},	//采集档案配置单元

	{0x60007F00,	1,		MAP_SYSDB,		0x6700,	  PN0,   0,		NULL,			0},	//添加一个采集档案配置单元，配置单元的信息中基本信息必须是完整的
	{0x60008000,	1,		MAP_SYSDB,		0x6701,	  PN0,   0,		NULL,			0},	//批量添加采集档案配置单元
	{0x60008100,	1,		MAP_SYSDB,		0x6702,	  PN0,   0,		NULL,			0},	//更新配置单元的基本信息对象
	{0x60008200,	1,		MAP_SYSDB,		0x6703,	  PN0,   0,		NULL,			0},	//更新配置单元的扩展信息以及附属信息，对象为NULL表示不更新
	{0x60008300,	1,		MAP_SYSDB,		0x6704,	  PN0,   0,		NULL,			0},	//删除配置单元，通过配置序号删除
	{0x60008400,	1,		MAP_SYSDB,		0x6705,	  PN0,   0,		NULL,			0},	//删除配置单元，通过基本信息对象删除
	{0x60008500,	1,		MAP_SYSDB,		0x6706,	  PN0,   0,		NULL,			0},	//删除配置单元，通过通信地址及端口删除
	{0x60008600,	1,		MAP_SYSDB,		0x6707,	  PN0,   0,		NULL,			0},	//清空采集档案配置表


	{0x60020200,	11,		MAP_TASKDB,		0x6002,	  PN0,   0,		g_bSchMtrRltFmt,			sizeof(g_bSchMtrRltFmt)},	//所有搜表结果
	{0x60020500,	11,		MAP_TASKDB,		0x6003,	  PN0,   0,		g_bCrossSchMtrRltFmt,		sizeof(g_bCrossSchMtrRltFmt)},	//跨台区搜表结果
	{0x60020600,	11,		MAP_SYSDB,		0x6004,	  PN0,   0,		g_bSchMtrCntFmt,			sizeof(g_bSchMtrCntFmt)},	//所有搜表结果记录数
	{0x60020700,	11,		MAP_SYSDB,		0x6005,	  PN0,   0,		g_bCrossSchMtrCntFmt,		sizeof(g_bCrossSchMtrCntFmt)},	//跨台区搜表结果记录数
	{0x60020800,	11,		MAP_SYSDB,		0x6006,	  PN0,   0,		g_bSchMtrParaFmt,			sizeof(g_bSchMtrParaFmt)},	//搜表参数
	{0x60020900,	11,		MAP_SYSDB,		0x6007,	  PN0,   0,		g_bTimeSchMtrParaFmt,		sizeof(g_bTimeSchMtrParaFmt)},	//每天周期搜表参数配置（定时搜表参数定时搜表参数）
	{0x60020A00,	11,		MAP_SYSDB,		0x6008,	  PN0,   0,		g_bSchMtrStateFmt,			sizeof(g_bSchMtrStateFmt)},	//搜表状态

	{0x60027f00,	11,		MAP_SYSDB,		0x6009,	  PN0,   0,		g_bSchMtrTimeFmt,			sizeof(g_bSchMtrTimeFmt)},	//实时启动搜表
	{0x60028000,	11,		MAP_SYSDB,		0x600A,	  PN0,   0,		NULL,												0},	//清空搜表结果
	{0x60028100,	11,		MAP_SYSDB,		0x600B,	  PN0,   0,		NULL,												0},	//清空跨台区搜表结果

	{0x60120200,	1,		MAP_TASKDB,		0x6012,	  PN0,   0,		g_bTskUnitFmtDesc/*g_bTskUnitFmtDesc+2*/,	sizeof(g_bTskUnitFmtDesc)/*sizeof(g_bTskUnitFmtDesc)-2*/},
	{0x60140200,	1,		MAP_TASKDB,		0x6014,	  PN0,   0,		g_bCommFmtDesc+2,		sizeof(g_bCommFmtDesc)-2},
	{0x60160200,	1,		MAP_TASKDB,		0x6016,	  PN0,   0,		g_bEvtFmtDesc+2,		sizeof(g_bEvtFmtDesc)-2},
	{0x60180200,	1,		MAP_TASKDB,		0x6018,	  PN0,   0,		g_bTranFmtDesc+2,		sizeof(g_bTranFmtDesc)-2},
	{0x60190200,	1,		MAP_TASKDB,		0x6019,	  PN0,   0,		NULL,								0},
	{0x601C0200,	1,		MAP_TASKDB,		0x601C,	  PN0,   0,		g_bRptFmtDesc+2,		sizeof(g_bRptFmtDesc)-2},

	{0x601E0200,	8,		MAP_TASKDB,		0x601E,	  PN0,   0,		g_bAddAcqRuleLib,			sizeof(g_bAddAcqRuleLib)},	//获取采集规则

	{0x60340200,	1,		MAP_SYSDB,		0x6034,	  PN0,   0,		g_bTaskMoniUnit,sizeof(g_bTaskMoniUnit)},	//采集任务监控单元
	{0x60400200,	7,		MAP_SYSDB,		0x6040,	  PN0,   0,		NULL,					0},
	{0x60410200,	7,		MAP_SYSDB,		0x6041,	  PN0,   0,		NULL,					0},
	{0x60420200,	7,		MAP_SYSDB,		0x6042,	  PN0,   0,		NULL,					0},
	{0x60510200,	1,		MAP_TASKDB,		0x6051,	  PN0,   0,		NULL,					0},

	{0x80000200,	8,		MAP_SYSDB,		0x8000,	  PN0,   0,		g_bYkParaFmt,  sizeof(g_bYkParaFmt), NULL},		//遥控，属性2（配置参数)
	{0x80000300,	8,		MAP_SYSDB,		0x8200,	  PN0,   0,		g_bCtrlStaFmt, sizeof(g_bCtrlStaFmt), NULL},	//遥控，属性3 继电器输出状态
	{0x80000400,	8,		MAP_SYSDB,		0x8201,	  PN0,   0,		g_bCtrlStaFmt, sizeof(g_bCtrlStaFmt), NULL},	//遥控，属性4 告警状态
	{0x80000500,	8,		MAP_SYSDB,		0x8202,	  PN0,   0,		g_bCtrlStaFmt, sizeof(g_bCtrlStaFmt), NULL},	//遥控，属性5 命令状态
	//保电参数属性2，3，4，5的配置
	{0x80010200,	8,		MAP_SYSDB,		0x8001,	  PN0,   0,		g_bGuarant02Fmt,		sizeof(g_bGuarant02Fmt),		NULL},	//保电配置，属性2
	{0x80010300,	8,		MAP_SYSDB,		0x8210,	  PN0,   0,		g_bGuarant03Fmt,		sizeof(g_bGuarant03Fmt),		NULL},	//保电配置，属性3
	{0x80010400,	8,		MAP_SYSDB,		0x8211,	  PN0,   0,		g_bGuarant03Fmt,		sizeof(g_bGuarant03Fmt),		NULL},	//保电配置，属性4
	{0x80010500,	8,		MAP_SYSDB,		0x8212,	  PN0,   0,		g_bGuarant05Fmt,		sizeof(g_bGuarant05Fmt),		NULL},	//保电配置，属性5

	{0x80020200,	8,		MAP_SYSDB,		0x8002,	  PN0,   0,		g_bGuarant02Fmt,		sizeof(g_bGuarant02Fmt),		NULL},	//催费告警配置，属性2

	{0x80030200,	8,		MAP_SYSDB,		0x8003,	  PN0,   0,		g_bChineseInfoFmt,		sizeof(g_bChineseInfoFmt),		NULL},	//一般中文信息
	{0x80040200,	8,		MAP_SYSDB,		0x8004,	  PN0,   0,		g_bChineseInfoFmt,		sizeof(g_bChineseInfoFmt),		NULL},	//重要中文信息

	{0x81000200,	8,		MAP_SYSDB,		0x8100,	  PN0,   0,		g_bSafeLimitFmt,		sizeof(g_bSafeLimitFmt),		NULL},	//保安定值，属性2
	{0x81010200,	8,		MAP_SYSDB,		0x8101,	  PN0,   0,		g_bCtrlPeriodFmt,		sizeof(g_bCtrlPeriodFmt),		NULL},	//终端功控时段，属性2
	{0x81020200,	8,		MAP_SYSDB,		0x8102,	  PN0,   0,		g_bTurnAlrTimeFmt,		sizeof(g_bTurnAlrTimeFmt),		NULL},	//终端功控轮次告警时间，属性2

	{0x81030200,	13,		MAP_SYSDB,		0x8103,	  PN0,   0,		g_bPeriodCtrlParaFmt,	sizeof(g_bPeriodCtrlParaFmt),	NULL},	//终端时段控，属性2
	{0x81030300,	13,		MAP_SYSDB,		0x8230,	  PN0,   0,		g_bCtrlInputStatusFmt,	sizeof(g_bCtrlInputStatusFmt),	NULL},	//终端时段控，属性3
	{0x81030400,	13,		MAP_SYSDB,		0x8231,	  PN0,   0,		g_bCtrlOutputStatusFmt,	sizeof(g_bCtrlOutputStatusFmt),	NULL},	//终端时段控，属性4
	{0x81030500,	13,		MAP_SYSDB,		0x8232,	  PN0,   0,		g_bCtrlAlrStatusFmt,	sizeof(g_bCtrlAlrStatusFmt),	NULL},	//终端时段控，属性5

	{0x81040200,	13,		MAP_SYSDB,		0x8104,	  PN0,   0,		g_bRestCtrlParaFmt,		sizeof(g_bRestCtrlParaFmt),		NULL},	//终端厂休控，属性2
	{0x81040300,	13,		MAP_SYSDB,		0x8240,	  PN0,   0,		g_bCtrlInputStatusFmt,	sizeof(g_bCtrlInputStatusFmt),	NULL},	//终端厂休控，属性3
	{0x81040400,	13,		MAP_SYSDB,		0x8241,	  PN0,   0,		g_bCtrlOutputStatusFmt,	sizeof(g_bCtrlOutputStatusFmt),	NULL},	//终端厂休控，属性4
	{0x81040500,	13,		MAP_SYSDB,		0x8242,	  PN0,   0,		g_bCtrlAlrStatusFmt,	sizeof(g_bCtrlAlrStatusFmt),	NULL},	//终端厂休控，属性5

	{0x81050200,	13,		MAP_SYSDB,		0x8105,	  PN0,   0,		g_bShutoutCtrlParaFmt,	sizeof(g_bShutoutCtrlParaFmt),	NULL},	//终端营业报停控，属性2
	{0x81050300,	13,		MAP_SYSDB,		0x8250,	  PN0,   0,		g_bCtrlInputStatusFmt,	sizeof(g_bCtrlInputStatusFmt),	NULL},	//终端营业报停控，属性3
	{0x81050400,	13,		MAP_SYSDB,		0x8251,	  PN0,   0,		g_bCtrlOutputStatusFmt,	sizeof(g_bCtrlOutputStatusFmt),	NULL},	//终端营业报停控，属性4
	{0x81050500,	13,		MAP_SYSDB,		0x8252,	  PN0,   0,		g_bCtrlAlrStatusFmt,	sizeof(g_bCtrlAlrStatusFmt),	NULL},	//终端营业报停控，属性5

	//{0x81060200,	13,		MAP_SYSDB,		0x8106,	  PN0,   0,		g_bTmpCtrlParaFmt,		sizeof(g_bTmpCtrlParaFmt),		NULL},	//终端当前功率下浮控，属性2
	{0x81060300,	13,		MAP_SYSDB,		0x8260,	  PN0,   0,		g_bCtrlInputStatusFmt,	sizeof(g_bCtrlInputStatusFmt),	NULL},	//终端当前功率下浮控，属性3
	{0x81060400,	13,		MAP_SYSDB,		0x8261,	  PN0,   0,		g_bCtrlOutputStatusFmt,	sizeof(g_bCtrlOutputStatusFmt),	NULL},	//终端当前功率下浮控，属性4
	{0x81060500,	13,		MAP_SYSDB,		0x8262,	  PN0,   0,		g_bCtrlAlrStatusFmt,	sizeof(g_bCtrlAlrStatusFmt),	NULL},	//终端当前功率下浮控，属性5

	{0x81070200,	13,		MAP_SYSDB,		0x8107,	  PN0,   0,		g_bBuyCtrlParaFmt,		sizeof(g_bBuyCtrlParaFmt),		NULL},	//终端购电控，属性2
	{0x81070300,	13,		MAP_SYSDB,		0x8270,	  PN0,   0,		g_bCtrlInputStatusFmt,	sizeof(g_bCtrlInputStatusFmt),	NULL},	//终端购电控，属性3
	{0x81070400,	13,		MAP_SYSDB,		0x8271,	  PN0,   0,		g_bCtrlOutputStatusFmt,	sizeof(g_bCtrlOutputStatusFmt),	NULL},	//终端购电控，属性4
	{0x81070500,	13,		MAP_SYSDB,		0x8272,	  PN0,   0,		g_bCtrlAlrStatusFmt,	sizeof(g_bCtrlAlrStatusFmt),	NULL},	//终端购电控，属性5

	{0x81080200,	13,		MAP_SYSDB,		0x8108,	  PN0,   0,		g_bMonthCtrlParaFmt,	sizeof(g_bMonthCtrlParaFmt),	NULL},	//终端月电控，属性2
	{0x81080300,	13,		MAP_SYSDB,		0x8280,	  PN0,   0,		g_bCtrlInputStatusFmt,	sizeof(g_bCtrlInputStatusFmt),	NULL},	//终端月电控，属性3
	{0x81080400,	13,		MAP_SYSDB,		0x8281,	  PN0,   0,		g_bCtrlOutputStatusFmt,	sizeof(g_bCtrlOutputStatusFmt),	NULL},	//终端月电控，属性4
	{0x81080500,	13,		MAP_SYSDB,		0x8282,	  PN0,   0,		g_bCtrlAlrStatusFmt,	sizeof(g_bCtrlAlrStatusFmt),	NULL},	//终端月电控，属性5

	//ESAM
	{0xF1000200,	21,		MAP_SYSDB,		0xF102,	  PN0,	0,	g_bEsamSerialNumFmt,			sizeof(g_bEsamSerialNumFmt), 			NULL}, //ESAM序列号
	{0xF1000300,	21,		MAP_SYSDB,		0xF103,	  PN0,	0,	g_bEsamVerisonFmt,				sizeof(g_bEsamVerisonFmt), 				NULL}, //ESAM版本号
	{0xF1000400,	21,		MAP_SYSDB,		0xF104,	  PN0,	0,	g_bEsamKeyVersionFmt,			sizeof(g_bEsamKeyVersionFmt), 			NULL}, //对称密钥版本
	{0xF1000500,	21, 	MAP_SYSDB,		0xF106,	  PN0,	0,	g_bEsamSessionMaxTimeFmt,		sizeof(g_bEsamSessionMaxTimeFmt),		NULL}, //证书版本
	{0xF1000600,	21,		MAP_SYSDB,		0xF107,	  PN0,	0,	g_bEsamSessionRemainTimeFmt,	sizeof(g_bEsamSessionRemainTimeFmt), 	NULL}, //会话时效门限
	{0xF1000700,	21,		MAP_SYSDB,		0xF108,	  PN0,	0,	g_bEsamCounterFmt,				sizeof(g_bEsamCounterFmt),				NULL}, //会话时效剩余时间
	{0xF1000800,	21,		MAP_SYSDB,		0xF105,	  PN0,	0,	g_bEsamCerVersionFmt,			sizeof(g_bEsamCerVersionFmt), 			NULL}, //当前计数器
	{0xF1000900,	21,		MAP_SYSDB,		0xF109,	  PN0,	0,	g_bEsamTermCerSerNumFmt,		sizeof(g_bEsamTermCerSerNumFmt), 		NULL}, //终端证书序列号
	{0xF1000A00,	21,		MAP_SYSDB,		0xF10A,	  PN0,	0,	g_bEsamTermCertificateFmt,		sizeof(g_bEsamTermCertificateFmt), 		NULL}, //主战证书序列号
	{0xF1000B00,	21,		MAP_SYSDB,		0xF10B,   PN0,	0,	g_bEsamMSCerSerNumFmt,			sizeof(g_bEsamMSCerSerNumFmt), 			NULL}, //终端证书
	{0xF1000C00,	21, 	MAP_SYSDB,		0xF10C,   PN0,	0,	g_bEsamMSCertificateFmt,		sizeof(g_bEsamMSCertificateFmt),		NULL}, //主站证书

	{0xF1010200,	8,		MAP_SYSDB,		0xF112,	  PN0,  0,  g_bEsamSercureModeChoiceFmt,	sizeof(g_bEsamSercureModeChoiceFmt),	NULL}, //安全模式选择
	{0xF1010300,	8,		MAP_SYSDB,		0xF113,	  PN0,  0,  g_bEsamSercureModeParamFmt,		sizeof(g_bEsamSercureModeParamFmt),		NULL}, //显式安全模式参数


	///
	{0xF2000200,	22,		MAP_SYSDB,		0xF200,	  PN0,   MAX_232_PORT_NUM,		g_bRS232PortFmt, sizeof(g_bRS232PortFmt)},    //RS232，属性2
	{0xF2000300,	22, 	MAP_BYTE,		0,   	  PN0,	 MAX_232_PORT_NUM, 	NULL, 			 NULL},	  			//RS232，属性3
	{0xF2010200,	22,		MAP_SYSDB,		0xF201,	  PN0,   MAX_485_PORT_NUM,		g_bRS485PortFmt, sizeof(g_bRS485PortFmt)},    //RS485，属性2
	{0xF2010300,	22, 	MAP_BYTE,		0,		  PN0,	 MAX_485_PORT_NUM,	NULL,			 NULL}, 			//RS485，属性3
	{0xF2020200,	22,		MAP_SYSDB,		0xF202,	  PN0,   MAX_HW_PORT_NUM,		g_bInfraPortFmt, sizeof(g_bInfraPortFmt)},    //红外，属性2
	{0xF2020300,	22, 	MAP_BYTE,		0,		  PN0,	 MAX_HW_PORT_NUM,	NULL,			 NULL}, 			//红外，属性3
	{0xF2030200,	22,		MAP_SYSDB,		0xF203,	  PN0,   MAX_SW_PORT_NUM,		g_bSwitchInput2Fmt, sizeof(g_bSwitchInput2Fmt)},    //开关量输入，属性2
	{0xF2030300,	22, 	MAP_BYTE,		0,		  PN0,	 MAX_SW_PORT_NUM,	NULL,			 NULL}, 			//红外，属性3
	{0xF2030400,	22,		MAP_SYSDB,		0xF800,	  PN0,   MAX_SW_PORT_NUM,		g_bSwitchInput4Fmt, sizeof(g_bSwitchInput4Fmt)},    //开关量输入，属性4
	{0xF2050200,	22, 	MAP_SYSDB,		0xF205,   PN0,	 MAX_RLY_PORT_NUM, 	g_bRelayOutput2Fmt, sizeof(g_bRelayOutput2Fmt)},	//继电器输出，属性2
	{0xF2050300,	22, 	MAP_BYTE,		0,		  PN0,	 MAX_RLY_PORT_NUM,	NULL,			 NULL}, 				//继电器输出，属性3
	{0xF2060200,	22, 	MAP_SYSDB,		0xF206,   PN0,	 MAX_ALRM_PORT_NUM, 	g_bAlarmOutput2Fmt, sizeof(g_bAlarmOutput2Fmt)},	//告警输出，属性2
	{0xF2060300,	22, 	MAP_BYTE,		0,		  PN0,	 MAX_ALRM_PORT_NUM,	NULL,			 NULL}, 				//告警输出，属性3
	{0xF2060400,	22, 	MAP_SYSDB,		0xF801,   PN0,	 MAX_ALRM_PORT_NUM, 	g_bAlarmOutput4Fmt, sizeof(g_bAlarmOutput4Fmt)},	//告警输出，属性4
	{0xF2070200,	22, 	MAP_SYSDB,		0xF207,   PN0,	 MAX_MUL_PORT_NUM, 	g_bMulFunction2Fmt, sizeof(g_bMulFunction2Fmt)},	//多功能端子，属性2
	{0xF2070300,	22, 	MAP_BYTE,		0,		  PN0,	 MAX_MUL_PORT_NUM, NULL,			 NULL}, 				//多功能端子，属性3
	{0xF2080200,	22, 	MAP_SYSDB,		0xF208,   PN0,	 1, 	g_bACInterface2Fmt, sizeof(g_bACInterface2Fmt)},	//交采接口，属性2
	{0xF2080300,	22, 	MAP_BYTE,		0,		  PN0,	 1, NULL,			 NULL}, 								//交采接口，属性3
	{0xF2090200,	22,		MAP_SYSDB,		0xF209,	  PN0,   MAX_PLC_PORT_NUM,		g_bPlcPortFmt,	sizeof(g_bPlcPortFmt)},    			//载波，属性2
	{0xF2090300,	22, 	MAP_BYTE,		0,		  PN0,	 MAX_PLC_PORT_NUM, NULL,			 NULL}, 				//载波，属性3
	{0xF20a0200,	22, 	MAP_SYSDB,		0xF20a,   PN0,	 MAX_PLUS_PORT_NUM, 	g_bPulseInput2Fmt,	sizeof(g_bPulseInput2Fmt)}, 	//脉冲输入设备，属性2
	{0xF20a0300,	22, 	MAP_BYTE,		0,		  PN0,	 MAX_PLUS_PORT_NUM, NULL,			 NULL}, 				//脉冲输入设备，属性3

	{0xF3000200, 	17,		MAP_SYSDB,		0xF300,	  PN0,	 0,		g_bDisplay2Fmt,		sizeof(g_bDisplay2Fmt)},	//自动轮显属性2，显示对象列表
	{0xF3000300, 	17,		MAP_SYSDB,		0xF900,	  PN0,	 0,		g_bDisplay3Fmt,		sizeof(g_bDisplay3Fmt)},    //自动轮显属性3，显示时间
	{0xF3000400, 	17,		MAP_SYSDB,		0xF902,	  PN0,	 0,		g_bDisplay4Fmt,		sizeof(g_bDisplay4Fmt)},	//自动轮显属性4，显示参数
	{0xF3010200, 	17,		MAP_SYSDB,		0xF301,	  PN0,	 0,		g_bDisplay2Fmt,		sizeof(g_bDisplay2Fmt)},	//按键轮显属性2，显示对象列表
	{0xF3010300, 	17,		MAP_SYSDB,		0xF901,	  PN0,	 0,		g_bDisplay3Fmt,		sizeof(g_bDisplay3Fmt)},	//按键轮显属性3，显示时间
	{0xF3010400, 	17,		MAP_SYSDB,		0xF903,	  PN0,	 0,		g_bDisplay4Fmt,		sizeof(g_bDisplay4Fmt)},	//按键轮显属性4，显示参数

};

#define OI_MAP_NUM sizeof(g_OIConvertClass)/sizeof(g_OIConvertClass[0])


//总加组参数和数据格式描述
static BYTE g_bAddGrpCfgFmt[] = {
	DT_STRUCT,	//struct
	3,	//成员个数
	DT_TSA,		//通信地址 Tsa
	DT_ENUM,		//总加标志 enum
	DT_ENUM,		//运算符标志 enum
};

static BYTE g_bDelGrpCfgFmt[] = {
	DT_TSA,		//通信地址 Tsa
};

/////////////////////////////////////////////////////////////////
//控制参数和数据格式描述
BYTE g_bYkCtrlOpenFmt[] = {
	DT_ARRAY,	//array
	4,	//最大个数
	DT_STRUCT,	//struct
	4,
	DT_OAD,
	DT_UNSIGN,
	DT_LONG_U,
	DT_BOOL,
};

BYTE g_bYkCtrlCloseFmt[] = {
	DT_ARRAY,	//array
	4,	//最大个数
	DT_STRUCT,	//struct
	2,
	DT_OAD,
	DT_ENUM,
};

BYTE g_bEsamKeyUpdateFmt[] = {
	DT_STRUCT, 2,
	DT_OCT_STR, 0x82, 0x08, 0x00, RLV,
	DT_SID_MAC,
};

BYTE g_bEsamCerUpdateFmt[] = {
	DT_STRUCT, 2,
	DT_OCT_STR, 0x82, 0x08, 0x00, RLV,
	DT_SID,
};

BYTE g_bEsamTimeBarUpdateFmt[] = {
	DT_STRUCT, 2,
	DT_OCT_STR, 0x82, 0x08, 0x00, RLV,
	DT_SID,
};

BYTE g_bUrgeParaFmt[] = {
	DT_STRUCT,	//struct
	2,
	DT_OCT_STR, 3,	RLF,
	DT_VIS_STR, 200, RLV,
};

BYTE g_bAddChineseInfoFmt[] = {
	DT_UNSIGN,	//序号
	DT_DATE_TIME_S,	//发布时间
	DT_VIS_STR, 200, RLV 	//信息内容
};

BYTE g_bDelChineseInfoFmt[] = {
	DT_UNSIGN,	//序号
};

BYTE g_bPeriodCtrlUnitFmt[] = {
	DT_STRUCT, 6,
	DT_OI,	//总加组对象
	DT_BIT_STR,	1, RLF, //方案标识
	DT_STRUCT, 9, //第一套定值
	DT_BIT_STR,	1, RLF, //时段号
	DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, //1~8个时段功控定值
	DT_STRUCT, 9, //第二套定值
	DT_BIT_STR,	1, RLF, //时段号
	DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, //1~8个时段功控定值
	DT_STRUCT, 9, //第三套定值
	DT_BIT_STR,	1, RLF, //时段号
	DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, DT_LONG64, //1~8个时段功控定值
	DT_INT, //时段功控定值浮动系数
}; //时段功控配置单元

BYTE g_bPeriodCtrlChgFmt[] = {
	DT_OI,	//总加组对象
	DT_STRUCT, 2,
	DT_BIT_STR,	1, RLF, //时段功控投入标识
	DT_UNSIGN, //时段功控定值方案号
}; //时段功控切换

BYTE g_bRestCtrlUnitFmt[] = {
	DT_STRUCT, 5,
	DT_OI,	//总加组对象
	DT_LONG64, //厂休控定值
	DT_DATE_TIME_S, //限电起始时间
	DT_LONG_U, //限电延续时间
	DT_BIT_STR,	1, RLF, //每周限电日
}; //厂休控配置单元

BYTE g_bShutoutCtrlUnitFmt[] = {
	DT_STRUCT, 4,
	DT_OI,	//总加组对象
	DT_DATE_TIME_S, //报停起始时间
	DT_DATE_TIME_S, //报停结束时间
	DT_LONG64, //报停控功率定值
}; //营业报停控配置单元

BYTE g_bInputTmpCtrlFmt[] = {
	DT_OI,	//总加组对象
	DT_STRUCT, 8,
	DT_UNSIGN, //当前功率下浮控定值滑差时间    unsigned（单位：分钟），
	DT_INT, //当前功率下浮控定值浮动系数    integer（单位：%），
	DT_UNSIGN, //控后总加有功功率冻结延时时间  unsigned（单位：分钟），
	DT_UNSIGN, //当前功率下浮控的控制时间      unsigned（单位：0.5小时），
	DT_UNSIGN, //当前功率下浮控第1轮告警时间  unsigned（单位：分钟），
	DT_UNSIGN, //当前功率下浮控第2轮告警时间  unsigned（单位：分钟），
	DT_UNSIGN, //当前功率下浮控第3轮告警时间  unsigned（单位：分钟），
	DT_UNSIGN, //当前功率下浮控第4轮告警时间  unsigned（单位：分钟）
}; //当前功率下浮控投入

BYTE g_bBuyCtrlUnitFmt[] = {
	DT_STRUCT, 8,
	DT_OI,	//总加组对象
	DT_DB_LONG_U, //购电单号
	DT_ENUM, //追加/刷新标识
	DT_ENUM, //购电类型
	DT_LONG64, //购电量（费）值
	DT_LONG64, //报警门限值
	DT_LONG64, //跳闸门限值
	DT_ENUM, //购电控模式
}; //购电控配置单元

BYTE g_bMonthCtrlUnitFmt[] = {
	DT_STRUCT, 4,
	DT_OI,	//总加组对象
	DT_LONG64, //月电量控定值
	DT_UNSIGN, //报警门限值系数
	DT_INT, //月电量控定值浮动系数
}; //月电控配置单元

BYTE g_bDelCtrlUnitFmt[] = {
	DT_OI,	//总加组对象
};

static BYTE g_bRS232PortParaCfgFmt[] = {
	//		DT_ARRAY,	//array
	//		MAX_232_PORT_NUM,	//最大个数
	DT_STRUCT,	//struct
	3,	//成员个数
	DT_OAD,
	DT_COMDCB,	
	DT_ENUM,
};

static BYTE g_bRS485PortParaCfgFmt[] = {
	//		DT_ARRAY,	//array
	//		MAX_485_PORT_NUM,	//最大个数
	DT_STRUCT,	//struct
	3,	//成员个数
	DT_OAD,
	DT_COMDCB,	
	DT_ENUM,
};




static BYTE g_bInfraPortParaCfgFmt[] = {
	//		DT_ARRAY,	//array
	//		MAX_HW_PORT_NUM,	//最大个数
	DT_STRUCT,	//struct
	2,	//成员个数
	DT_OAD,
	DT_COMDCB,	
};


static BYTE g_bRelayParaCfgFmt[] = {
	//		DT_ARRAY,	//array
	//		MAX_RLY_PORT_NUM,	//最大个数
	DT_STRUCT,	//struct
	2,	//成员个数
	DT_OAD,
	DT_ENUM,	
};

static BYTE g_bMulPortCfgFmt[] = {
	//		DT_ARRAY,	//array
	//		MAX_MUL_PORT_NUM,	//最大个数
	DT_STRUCT,	//struct
	2,	//成员个数
	DT_OAD,
	DT_ENUM,	
};

static BYTE g_bPlcTransCfgFmt[] = {//透明转发（参数）
	DT_STRUCT,	//struct
	3,	//成员个数
	DT_TSA,
	DT_LONG_U,
	DT_OCT_STR, 0x7f,RLV,
};
static BYTE g_bPlcPortParaCfgFmt[] = {
	//		DT_ARRAY,	//array
	//		MAX_PLC_PORT_NUM,	//最大个数
	DT_STRUCT,	//struct
	2,	//成员个数
	DT_OAD,
	DT_COMDCB,	
};




TOmMap g_OmMap[] = 
{
	//----dwOM----wClass-----------pFmt-------------wFmtLen-----------------DoMethod----------------------pvAddon-------
	{0x21000100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//复位区间统计
	{0x21010100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//
	{0x21020100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//
	{0x21030100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//
	{0x21040100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//

	{0x21100100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//复位平均统计
	{0x21110100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//
	{0x21120100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//
	{0x21130100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//
	{0x21140100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//

	{0x21200100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//复位极值统计
	{0x21210100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//
	{0x21220100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//
	{0x21230100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//
	{0x21240100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//

	{0x21300100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//复位合格率统计
	{0x21310100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//
	{0x21320100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//
	{0x21330100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//

	{0x21400100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//复位功率统计
	{0x21410100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//

	{0x22000100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//复位流量、供电时间、复位次数统计
	{0x22030100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//
	{0x22040100,	14,				NULL,		NULL,						ResetStatAll,				NULL},	//

	{0x23010100,	23,				NULL,		NULL,						ClrGrpCfgMethod1,				NULL},	//清空总加配置表
	{0x23010300,	23,		g_bAddGrpCfgFmt,	sizeof(g_bAddGrpCfgFmt),	AddGrpCfgMethod3,				NULL},	//添加一个总加配置单元
	{0x23010400,	23,		g_bGrpCfgFmt,		sizeof(g_bGrpCfgFmt),		BatAddGrpCfgMethod4,			NULL},	//批量添加总加配置单元
	{0x23010500,	23,		g_bDelGrpCfgFmt,	sizeof(g_bDelGrpCfgFmt),	DelGrpCfgMethod5,				NULL},	//删除一个总加配置单元

	{0x23020100,	23,				NULL,		NULL,						ClrGrpCfgMethod1,				NULL},	//清空总加配置表
	{0x23020300,	23,		g_bAddGrpCfgFmt,	sizeof(g_bAddGrpCfgFmt),	AddGrpCfgMethod3,				NULL},	//添加一个总加配置单元
	{0x23020400,	23,		g_bGrpCfgFmt,		sizeof(g_bGrpCfgFmt),		BatAddGrpCfgMethod4,			NULL},	//批量添加总加配置单元
	{0x23020500,	23,		g_bDelGrpCfgFmt,	sizeof(g_bDelGrpCfgFmt),	DelGrpCfgMethod5,				NULL},	//删除一个总加配置单元

	{0x23030100,	23,				NULL,		NULL,						ClrGrpCfgMethod1,				NULL},	//清空总加配置表
	{0x23030300,	23,		g_bAddGrpCfgFmt,	sizeof(g_bAddGrpCfgFmt),	AddGrpCfgMethod3,				NULL},	//添加一个总加配置单元
	{0x23030400,	23,		g_bGrpCfgFmt,		sizeof(g_bGrpCfgFmt),		BatAddGrpCfgMethod4,			NULL},	//批量添加总加配置单元
	{0x23030500,	23,		g_bDelGrpCfgFmt,	sizeof(g_bDelGrpCfgFmt),	DelGrpCfgMethod5,				NULL},	//删除一个总加配置单元

	{0x23040100,	23,				NULL,		NULL,						ClrGrpCfgMethod1,				NULL},	//清空总加配置表
	{0x23040300,	23,		g_bAddGrpCfgFmt,	sizeof(g_bAddGrpCfgFmt),	AddGrpCfgMethod3,				NULL},	//添加一个总加配置单元
	{0x23040400,	23,		g_bGrpCfgFmt,		sizeof(g_bGrpCfgFmt),		BatAddGrpCfgMethod4,			NULL},	//批量添加总加配置单元
	{0x23040500,	23,		g_bDelGrpCfgFmt,	sizeof(g_bDelGrpCfgFmt),	DelGrpCfgMethod5,				NULL},	//删除一个总加配置单元

	{0x23050100,	23,				NULL,		NULL,						ClrGrpCfgMethod1,				NULL},	//清空总加配置表
	{0x23050300,	23,		g_bAddGrpCfgFmt,	sizeof(g_bAddGrpCfgFmt),	AddGrpCfgMethod3,				NULL},	//添加一个总加配置单元
	{0x23050400,	23,		g_bGrpCfgFmt,		sizeof(g_bGrpCfgFmt),		BatAddGrpCfgMethod4,			NULL},	//批量添加总加配置单元
	{0x23050500,	23,		g_bDelGrpCfgFmt,	sizeof(g_bDelGrpCfgFmt),	DelGrpCfgMethod5,				NULL},	//删除一个总加配置单元

	{0x23060100,	23,				NULL,		NULL,						ClrGrpCfgMethod1,				NULL},	//清空总加配置表
	{0x23060300,	23,		g_bAddGrpCfgFmt,	sizeof(g_bAddGrpCfgFmt),	AddGrpCfgMethod3,				NULL},	//添加一个总加配置单元
	{0x23060400,	23,		g_bGrpCfgFmt,		sizeof(g_bGrpCfgFmt),		BatAddGrpCfgMethod4,			NULL},	//批量添加总加配置单元
	{0x23060500,	23,		g_bDelGrpCfgFmt,	sizeof(g_bDelGrpCfgFmt),	DelGrpCfgMethod5,				NULL},	//删除一个总加配置单元

	{0x23070100,	23,				NULL,		NULL,						ClrGrpCfgMethod1,				NULL},	//清空总加配置表
	{0x23070300,	23,		g_bAddGrpCfgFmt,	sizeof(g_bAddGrpCfgFmt),	AddGrpCfgMethod3,				NULL},	//添加一个总加配置单元
	{0x23070400,	23,		g_bGrpCfgFmt,		sizeof(g_bGrpCfgFmt),		BatAddGrpCfgMethod4,			NULL},	//批量添加总加配置单元
	{0x23070500,	23,		g_bDelGrpCfgFmt,	sizeof(g_bDelGrpCfgFmt),	DelGrpCfgMethod5,				NULL},	//删除一个总加配置单元

	{0x23080100,	23,				NULL,		NULL,						ClrGrpCfgMethod1,				NULL},	//清空总加配置表
	{0x23080300,	23,		g_bAddGrpCfgFmt,	sizeof(g_bAddGrpCfgFmt),	AddGrpCfgMethod3,				NULL},	//添加一个总加配置单元
	{0x23080400,	23,		g_bGrpCfgFmt,		sizeof(g_bGrpCfgFmt),		BatAddGrpCfgMethod4,			NULL},	//批量添加总加配置单元
	{0x23080500,	23,		g_bDelGrpCfgFmt,	sizeof(g_bDelGrpCfgFmt),	DelGrpCfgMethod5,				NULL},	//删除一个总加配置单元

#ifndef SYS_WIN
	//----dwOM----wClass-----------pFmt-------------wFmtLen-----------------DoMethod----------------------pvAddon-------
	{0x24010100,	12,				NULL,		NULL,						OnResePulseCmd,					NULL},	//复位  ---脉冲计量类
	{0x24010200,	12,				NULL,		NULL,						OnRunPulseCmd,					NULL},	//运行
	{0x24010300,	12,		g_bAddPulseCfgFmt,	sizeof(g_bAddPulseCfgFmt),	OnAddPulseCfgCmd,				NULL},	//添加一个冻结关联对象属性
	{0x24010400,	12,		g_bDelPulseCfgFmt,	sizeof(g_bDelPulseCfgFmt),	OnDelPulseCfgCmd,				NULL},	//添加一个冻结关联对象属性
	{0x24017f00,	12,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		BatAddPulseCfgCmd,				NULL}, //自定义 批量添加脉冲关联属性
	{0x24027f00,	12,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		BatAddPulseCfgCmd,				NULL}, //自定义 批量添加脉冲关联属性
	{0x24037f00,	12,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		BatAddPulseCfgCmd,				NULL}, //自定义 批量添加脉冲关联属性
	{0x24047f00,	12,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		BatAddPulseCfgCmd,				NULL}, //自定义 批量添加脉冲关联属性
	{0x24057f00,	12,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		BatAddPulseCfgCmd,				NULL}, //自定义 批量添加脉冲关联属性
	{0x24067f00,	12,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		BatAddPulseCfgCmd,				NULL}, //自定义 批量添加脉冲关联属性
	{0x24077f00,	12,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		BatAddPulseCfgCmd,				NULL}, //自定义 批量添加脉冲关联属性
	{0x24087f00,	12,		g_bPulseCfgFmt,		sizeof(g_bPulseCfgFmt),		BatAddPulseCfgCmd,				NULL}, //自定义 批量添加脉冲关联属性
#endif
	//----dwOM----wClass-----------pFmt-------------wFmtLen-----------------DoMethod----------------------pvAddon-------
	{0x30000100,	24,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30000200,	24,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30000300,	24,		g_bIC24TrigEvtFmt,		sizeof(g_bIC24TrigEvtFmt),						DoTermEvtIC24Method3,			NULL},	//触发一次记录
	{0x30000400,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30000500,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30010100,	24,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30010200,	24,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30010300,	24,		g_bIC24TrigEvtFmt,		sizeof(g_bIC24TrigEvtFmt),						DoTermEvtIC24Method3,			NULL},	//触发一次记录
	{0x30010400,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30010500,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30020100,	24,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30020200,	24,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30020300,	24,		g_bIC24TrigEvtFmt,		sizeof(g_bIC24TrigEvtFmt),						DoTermEvtIC24Method3,			NULL},	//触发一次记录
	{0x30020400,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30020500,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30030100,	24,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30030200,	24,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30030300,	24,		g_bIC24TrigEvtFmt,		sizeof(g_bIC24TrigEvtFmt),						DoTermEvtIC24Method3,			NULL},	//触发一次记录
	{0x30030400,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30030500,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30040100,	24,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30040200,	24,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30040300,	24,		g_bIC24TrigEvtFmt,		sizeof(g_bIC24TrigEvtFmt),						DoTermEvtIC24Method3,			NULL},	//触发一次记录
	{0x30040400,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30040500,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30050100,	24,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30050200,	24,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30050300,	24,		g_bIC24TrigEvtFmt,		sizeof(g_bIC24TrigEvtFmt),						DoTermEvtIC24Method3,			NULL},	//触发一次记录
	{0x30050400,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30050500,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30060100,	24,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30060200,	24,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30060300,	24,		g_bIC24TrigEvtFmt,		sizeof(g_bIC24TrigEvtFmt),						DoTermEvtIC24Method3,			NULL},	//触发一次记录
	{0x30060400,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30060500,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30070100,	24,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30070200,	24,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30070300,	24,		g_bIC24TrigEvtFmt,		sizeof(g_bIC24TrigEvtFmt),						DoTermEvtIC24Method3,			NULL},	//触发一次记录
	{0x30070400,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30070500,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30080100,	24,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30080200,	24,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30080300,	24,		g_bIC24TrigEvtFmt,		sizeof(g_bIC24TrigEvtFmt),						DoTermEvtIC24Method3,			NULL},	//触发一次记录
	{0x30080400,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30080500,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30090100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30090200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30090300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x30090400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30090500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x300A0100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x300A0200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x300A0300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x300A0400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x300A0500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x300B0100,	24,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x300B0200,	24,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x300B0300,	24,		g_bIC24TrigEvtFmt,		sizeof(g_bIC24TrigEvtFmt),						DoTermEvtIC24Method3,			NULL},	//触发一次记录
	{0x300B0400,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x300B0500,	24,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x300C0100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x300C0200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x300C0300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x300C0400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x300C0500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x300D0100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x300D0200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x300D0300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x300D0400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x300D0500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x300F0100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x300F0200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x300F0300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x300F0400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x300F0500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30100100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30100200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30100300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x30100400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30100500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30130100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30130200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30130300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x30130400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30130500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30140100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30140200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30140300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x30140400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30140500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x30150100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x30150200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x30150300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x30150400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x30150500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x301D0100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x301D0200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x301D0300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x301D0400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x301D0500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x301E0100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x301E0200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x301E0300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x301E0400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x301E0500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x302D0100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x302D0200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x302D0300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x302D0400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x302D0500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x302E0100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x302E0200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x302E0300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x302E0400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x302E0500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x302F0100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x302F0200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x302F0300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x302F0400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x302F0500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x31000100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31000200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31000300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31000400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31000500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x31010100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31010200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31010300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31010400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31010500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x31040100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31040200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31040300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31040400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31040500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x31050100,	9,				NULL,		0,							DoMtrExcMethod1,				NULL},	//复位
	{0x31050200,	9,				NULL,		0,							DoMtrExcMethod2,				NULL},	//运行
	//{0x31050300,	9,				NULL,		0,							DoMtrExcMethod3,				NULL},	//触发一次冻结
	{0x31050400,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod4,				NULL},	//添加一个冻结关联对象属性
	{0x31050500,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod5,				NULL},	//添加一个冻结关联对象属性

	{0x31060100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31060200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31060300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31060400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31060500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x31090100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31090200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31090300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31090400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31090500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x310A0100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x310A0200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x310A0300,	7,		g_bSrcEnumTrigEvtFmt,		sizeof(g_bSrcEnumTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x310A0400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x310A0500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x310B0100,	9,				NULL,		0,							DoMtrExcMethod1,				NULL},	//复位
	{0x310B0200,	9,				NULL,		0,							DoMtrExcMethod2,				NULL},	//运行
	//{0x310B0300,	9,				NULL,		0,							DoMtrExcMethod3,				NULL},	//触发一次冻结
	{0x310B0400,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod4,				NULL},	//添加一个冻结关联对象属性
	{0x310B0500,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod5,				NULL},	//添加一个冻结关联对象属性

	{0x310C0100,	9,				NULL,		0,							DoMtrExcMethod1,				NULL},	//复位
	{0x310C0200,	9,				NULL,		0,							DoMtrExcMethod2,				NULL},	//运行
	//{0x310C0300,	9,				NULL,		0,							DoMtrExcMethod3,				NULL},	//触发一次冻结
	{0x310C0400,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod4,				NULL},	//添加一个冻结关联对象属性
	{0x310C0500,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod5,				NULL},	//添加一个冻结关联对象属性

	{0x310D0100,	9,				NULL,		0,							DoMtrExcMethod1,				NULL},	//复位
	{0x310D0200,	9,				NULL,		0,							DoMtrExcMethod2,				NULL},	//运行
	//{0x310D0300,	9,				NULL,		0,							DoMtrExcMethod3,				NULL},	//触发一次冻结
	{0x310D0400,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod4,				NULL},	//添加一个冻结关联对象属性
	{0x310D0500,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod5,				NULL},	//添加一个冻结关联对象属性

	{0x310E0100,	9,				NULL,		0,							DoMtrExcMethod1,				NULL},	//复位
	{0x310E0200,	9,				NULL,		0,							DoMtrExcMethod2,				NULL},	//运行
	//{0x310E0300,	9,				NULL,		0,							DoMtrExcMethod3,				NULL},	//触发一次冻结
	{0x310E0400,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod4,				NULL},	//添加一个冻结关联对象属性
	{0x310E0500,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod5,				NULL},	//添加一个冻结关联对象属性

	{0x310F0100,	9,				NULL,		0,							DoMtrExcMethod1,				NULL},	//复位
	{0x310F0200,	9,				NULL,		0,							DoMtrExcMethod2,				NULL},	//运行
	//{0x310F0300,	9,				NULL,		0,							DoMtrExcMethod3,				NULL},	//触发一次冻结
	{0x310F0400,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod4,				NULL},	//添加一个冻结关联对象属性
	{0x310F0500,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod5,				NULL},	//添加一个冻结关联对象属性

	{0x31100100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31100200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31100300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31100400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31100500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x31110100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31110200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31110300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31110400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31110500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x31120100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31120200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31120300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31120400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31120500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x31140100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31140200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31140300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31140400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31140500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x31150100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31150200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31150300,	7,		g_bSrcOADTrigEvtFmt,		sizeof(g_bSrcOADTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31150400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31150500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x31160100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31160200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31160300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31160400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31160500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x31180100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31180200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31180300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31180400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31180500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x31190100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x31190200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x31190300,	7,		g_bSrcEnumTrigEvtFmt,		sizeof(g_bSrcEnumTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x31190400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x31190500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x311B0100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x311B0200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x311B0300,	7,		g_bSrcTsaTrigEvtFmt,		sizeof(g_bSrcTsaTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x311B0400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x311B0500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x311C0100,	9,				NULL,		0,							DoMtrExcMethod1,				NULL},	//复位
	{0x311C0200,	9,				NULL,		0,							DoMtrExcMethod2,				NULL},	//运行
	//{0x311C0300,	9,				NULL,		0,							DoMtrExcMethod3,				NULL},	//触发一次冻结
	{0x311C0400,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod4,				NULL},	//添加一个冻结关联对象属性
	{0x311C0500,	9,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),			DoMtrExcMethod5,				NULL},	//添加一个冻结关联对象属性

	{0x32200100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x32200200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x32200300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x32200400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x32200500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x32210100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x32210200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x32210300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x32210400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x32210500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x32220100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x32220200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x32220300,	7,		g_bSrcOITrigEvtFmt,		sizeof(g_bSrcOITrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x32220400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x32220500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x32230100,	7,		g_bResetEvtFmt,		sizeof(g_bResetEvtFmt),						DoTermEvtMethod1,			NULL},	//复位
	{0x32230200,	7,		g_bRunEvtFmt,		sizeof(g_bRunEvtFmt),						DoTermEvtMethod2,			NULL},	//执行
	//{0x32230300,	7,		g_bSrcNullTrigEvtFmt,		sizeof(g_bSrcNullTrigEvtFmt),						DoTermEvtIC7Method3,			NULL},	//触发一次记录
	{0x32230400,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod4,			NULL},	//添加一个事件关联对象属性
	{0x32230500,	7,		g_bOADCfgFmt,	sizeof(g_bOADCfgFmt),						DoTermEvtMethod5,			NULL},	//删除一个事件关联对象属性

	{0x43000100,	19,				NULL,		NULL,						DoDevInterfaceClass19,			NULL},	//复位
	{0x43000200,	19,				NULL,		NULL,						DoDevInterfaceClass19,			NULL},	//执行
	{0x43000300,	19,				NULL,		NULL,						DoDevInterfaceClass19,			NULL},	//数据初始化
	{0x43000400,	19,				NULL,		NULL,						DoDevInterfaceClass19,			NULL},	//恢复出厂参数
	{0x43000500,	19,				NULL,		NULL,						DoDevInterfaceClass19,			NULL},	//事件初始化
	{0x43000600,	19,				NULL,		NULL,						DoDevInterfaceClass19,			NULL},	//需量初始化

	{0x45000100,	25,				NULL,		NULL,						DoGprsInterfaceClass25,			NULL},	//公网设备初始化
	{0x45010100,	25,				NULL,		NULL,						DoGprsInterfaceClass25,			NULL},	//公网设备初始化
	{0x45100100,	26,				NULL,		NULL,						DoEthInterfaceClass26,			NULL},	//以太网设备初始化
	{0x45110100,	26,				NULL,		NULL,						DoEthInterfaceClass26,			NULL},	//以太网设备初始化
	{0x45120100,	26,				NULL,		NULL,						DoEthInterfaceClass26,			NULL},	//以太网设备初始化
	{0x45130100,	26,				NULL,		NULL,						DoEthInterfaceClass26,			NULL},	//以太网设备初始化
	{0x45140100,	26,				NULL,		NULL,						DoEthInterfaceClass26,			NULL},	//以太网设备初始化
	{0x45150100,	26,				NULL,		NULL,						DoEthInterfaceClass26,			NULL},	//以太网设备初始化
	{0x45160100,	26,				NULL,		NULL,						DoEthInterfaceClass26,			NULL},	//以太网设备初始化
	{0x45170100,	26,				NULL,		NULL,						DoEthInterfaceClass26,			NULL},	//以太网设备初始化

	{0x50000100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x50000200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x50000300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x50000400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50000500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//删除一个冻结关联对象属性
	{0x50000600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x50000700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x50000800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x50010100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x50010200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x50010300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x50010400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50010500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50010600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x50010700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x50010800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x50020100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x50020200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x50020300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x50020400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50020500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50020600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x50020700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x50020800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x50030100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x50030200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x50030300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x50030400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50030500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50030600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x50030700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x50030800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x50040100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x50040200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x50040300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x50040400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50040500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50040600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x50040700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x50040800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x50050100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x50050200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x50050300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x50050400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50050500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50050600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x50050700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x50050800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x50060100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x50060200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x50060300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x50060400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50060500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50060600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x50060700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x50060800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x50070100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x50070200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x50070300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x50070400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50070500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50070600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x50070700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x50070800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x50080100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x50080200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x50080300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x50080400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50080500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50080600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x50080700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x50080800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x50090100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x50090200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x50090300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x50090400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50090500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50090600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x50090700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x50090800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x500A0100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x500A0200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x500A0300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x500A0400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x500A0500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x500A0600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x500A0700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x500A0800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x500B0100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x500B0200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x500B0300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x500B0400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x500B0500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x500B0600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x500B0700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x500B0800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x50110100,	9,				NULL,		NULL,						OnResetFrzCmd,					NULL},	//复位
	{0x50110200,	9,				NULL,		NULL,						OnRunFrzCmd,					NULL},	//运行
	{0x50110300,	9,		g_bTrigFrzFmt,		sizeof(g_bTrigFrzFmt),		OnRxTrigFrzCmd,					NULL},	//触发一次冻结
	{0x50110400,	9,		g_bAddFrzCfgFmt,	sizeof(g_bAddFrzCfgFmt),	OnAddFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50110500,	9,		g_bDelFrzCfgFmt,	sizeof(g_bDelFrzCfgFmt),	OnDelFrzAttrCmd,				NULL},	//添加一个冻结关联对象属性
	{0x50110600,	9,		g_bTrigReFrzFmt,	sizeof(g_bTrigReFrzFmt),	OnRxTrigReFrzCmd,				NULL},	//触发补冻结
	{0x50110700,	9,		g_bBatAddFrzCfgFmt,	sizeof(g_bBatAddFrzCfgFmt),	OnBatAddFrzAttrCmd,				NULL},	//批量添加冻结对象属性
	{0x50110800,	9,				NULL,		NULL,						OnClrAttrTableCmd,				NULL},	//清除关联对象属性表

	{0x60007f00,	11,		g_bPnParaFmt,		sizeof(g_bPnParaFmt),		DoClass11Method127_AddMeter,	NULL},	//增加采集档案配置单元
	{0x60008000,	11,		g_bBatchAddMeterFmt,sizeof(g_bBatchAddMeterFmt),DoClass11Method128_AddMeter,	NULL},	//批量增加采集档案配置单元
	{0x60008100,	11,				NULL,		0,							DoClass11Method129_UpdataMeter,	NULL},	//更新配置单元的基本信息对象
	{0x60008200,	11,				NULL,		0,							DoClass11Method130_UpdataMeter,	NULL},	//更新配置单元的扩展信息以及附属信息，对象为NULL表示不更新
	{0x60008300,	11,				NULL,		0,							DoClass11Method131_DelMeter,	NULL},	//删除配置单元，通过配置序号删除
	{0x60008400,	11,				NULL,		0,							DoClass11Method132_DelMeter,	NULL},	//删除配置单元，通过基本信息对象删除
	{0x60008500,	11,				NULL,		0,							DoClass11Method133_DelMeter,	NULL},	//删除配置单元，通过通信地址及端口删除
	{0x60008600,	11,				NULL,		0,							DoClass11Method134_DelAllMtr,	NULL},	//清空采集档案配置表

	{0x60027f00,	11,		g_bSchMtrTimeFmt,sizeof(g_bSchMtrTimeFmt),		DoRightNowStartSchMtr	,		NULL},	//实时启动搜表
	{0x60028000,	11,				NULL,		0,							DoClearSchMtrFile,				NULL},	//清空搜表结果
	{0x60028100,	11,				NULL,		0,							DoClearCrossSchMtrFile,			NULL},	//清空跨台区搜表结果

	{0x60127F00,	10,		g_bTskUnitFmtDesc,	sizeof(g_bTskUnitFmtDesc),	AddCommonMethod127,		(void*)"TaskCfgUnit"},	//OI=0x6012 方法：127	添加或更新一组任务配置单元
	{0x60128000,	10,				NULL,		NULL,						DelCommonMethod128,		(void*)"TaskCfgUnit"},	//OI=0x6012 方法：128	删除一组配置单元			
	{0x60128100,	10,				NULL,		NULL,						ClrCommonMethod129,		(void*)"TaskCfgUnit"},	//OI=0x6012 方法：129	清空任务配置表
	{0x60128200,	10,		g_bUdpTaskState,	sizeof(g_bUdpTaskState),	UdpTaskState130,		(void*)"TaskCfgUnit"},	//OI=0x6012 方法：130	更新任务状态

	{0x60147F00,	11,		g_bCommFmtDesc,		sizeof(g_bCommFmtDesc),		AddCommonMethod127,		(void*)"CommSch"},	//OI=0x6014 方法：127	添加或更新一组普通采集方案
	{0x60148000,	11,				NULL,		NULL,						DelCommonMethod128,		(void*)"CommSch"},	//OI=0x6014 方法：128	删除一组普通采集方案
	{0x60148100,	11,				NULL,		NULL,						ClrCommonMethod129,		(void*)"CommSch"},	//OI=0x6014 方法：129	清空普通采集方案集
	{0x60148200,	11,		g_bResetCSDFmtDesc,	sizeof(g_bResetCSDFmtDesc),	ResetSchRecordCSD,		(void*)"CommSch"},	//OI=0x6014 方法：130	重置方案的记录列选择
	{0x60167F00,	11,		g_bEvtFmtDesc,		sizeof(g_bEvtFmtDesc),		AddCommonMethod127,		(void*)"EvtSch"},	//OI=0x6016 方法：127	添加或更新一组事件采集方案
	{0x60168000,	11,				NULL,		NULL,						DelCommonMethod128,		(void*)"EvtSch"},	//OI=0x6016 方法：128	删除一组事件采集方案
	{0x60168100,	11,				NULL,		NULL,						ClrCommonMethod129,		(void*)"EvtSch"},	//OI=0x6016 方法：129	清空事件采集方案集
	{0x60168200,	11,		g_bEvtAcqUpdRptFlg,	sizeof(g_bEvtAcqUpdRptFlg), UpdateRptFlgMethod130,	(void*)"EvtSch"},	//OI=0x6016 方法：130	UpdateReportFlag(方案编号，上报标识)
	{0x60187F00,	11,		g_bTranFmtDesc,		sizeof(g_bTranFmtDesc),		DoTransMethod127_Add,				(void*)"TranSch"},	//OI=0x6018 方法：127	添加更新一个透明方案或添加一组方案内容	
	{0x60188000,	11,				NULL,		NULL,						DoTransAddMeterFrameMethod128,		(void*)"TranSch"},	//OI=0x6018 方法：128	添加一组报文
	{0x60188100,	11,				NULL,		NULL,						DoTransDelSchMtrAddrMethod129,		(void*)"TranSch"},	//OI=0x6018 方法：129	删除一个方案的一组方案内容
	{0x60188200,	11,				NULL,		NULL,						DoTransDelGroupSchMethod130,		(void*)"TranSch"},	//OI=0x6018 方法：130	删除一组透明方案
	{0x60188300,	11,				NULL,		NULL,						DoTransClearMethod131,				(void*)"TranSch"},	//OI=0x6018 方法：131	清空透明方案集
	{0x601C7F00,	11,		g_bRptFmtDesc,		sizeof(g_bRptFmtDesc),		AddCommonMethod127,		(void*)"RptSch"},	//OI=0x601C 方法：127	添加或更新一组上报方案
	{0x601C8000,	11,				NULL,		NULL,						DelCommonMethod128,		(void*)"RptSch"},	//OI=0x601C 方法：128	删除一组上报方案
	{0x601C8100,	11,				NULL,		NULL,						ClrCommonMethod129,		(void*)"RptSch"},	//OI=0x601C 方法：129	清空上报方案集

	{0x601E7f00,	8,		g_bAddAcqRuleLib,	sizeof(g_bAddAcqRuleLib),	AddAcqRuleMethod129,		(void*)"AcqRule"},	//OI=0x601E 方法：129	添加或更新一组采集规则
	{0x601E8000,	8,		g_bDelAcqRuleLib,	sizeof(g_bDelAcqRuleLib),	DelAcqRuleMethod130,		(void*)"AcqRule"},	//OI=0x601E 方法：130	删除一组采集规则
	{0x601E8100,	8,				NULL,		NULL,						ClrAcqRuleMethod131,		(void*)"AcqRule"},	//OI=0x601E 方法：131	清空采集规则

	{0x60517F00,	11,		g_bRealFmtDesc,		sizeof(g_bRealFmtDesc),		AddCommonMethod127,		(void*)"RealSch"},	//OI=0x6051 方法：127	添加或更新一组实时监控采集方案
	{0x60518000,	11,				NULL,		NULL,						DelCommonMethod128,		(void*)"RealSch"},	//OI=0x6051 方法：128	删除一组实时监控采集方案
	{0x60518100,	11,				NULL,		NULL,						ClrCommonMethod129,		(void*)"RealSch"},	//OI=0x6051 方法：129	清空实时监控采集方案

	{0x80007F00,	8,				NULL,		NULL,						YkCtrlTriAlertMethod127,		NULL},	//OI=0x8000 方法：127	触发告警
	{0x80008000,	8,				NULL,		NULL,						YkCtrlDisAlertMethod128,		NULL},	//OI=0x8000 方法：128	解除报警
	{0x80008100,	8,		g_bYkCtrlOpenFmt,	sizeof(g_bYkCtrlOpenFmt),	YkCtrlOpenMethod129,			NULL},	//OI=0x8000 方法：129	跳闸
	{0x80008200,	8,		g_bYkCtrlCloseFmt,	sizeof(g_bYkCtrlCloseFmt),	YkCtrlCloseMethod130,			NULL},	//OI=0x8000 方法：129	合闸

	{0x80017F00,	8,				NULL,		NULL,						InputGuaranteeMethod127,		NULL},	//OI=0x8001 方法：127	投入保电
	{0x80018000,	8,				NULL,		NULL,						QuitGuaranteeMethod128,			NULL},	//OI=0x8001 方法：128	解除保电
	{0x80018100,	8,				NULL,		NULL,						QuitAutoGuaranteeMethod129,		NULL},	//OI=0x8001 方法：129	解除自动保电

	{0x80027F00,	8,		g_bUrgeParaFmt,		sizeof(g_bUrgeParaFmt),		InputUrgeFeeMethod127,			NULL},	//OI=0x8002 方法：127	催费告警投入

	{0x80028000,	8,				NULL,		NULL,						QuitUrgeFeeMethod128,			NULL},	//OI=0x8002 方法：128	取消催费告警
	{0x80037F00,	8,	g_bAddChineseInfoFmt,sizeof(g_bAddChineseInfoFmt),	AddChineseInfoMethod127,		NULL},	//OI=0x8003 方法：127	添加中文信息
	{0x80038000,	8,	g_bAddChineseInfoFmt,sizeof(g_bAddChineseInfoFmt),	DelChineseInfoMethod128,		NULL},	//OI=0x8003 方法：128	删除中文信息

	{0x81030300,	13,	g_bPeriodCtrlUnitFmt,sizeof(g_bPeriodCtrlUnitFmt),	AddCtrlUnitMethod3,				NULL},	//OI=0x8103 方法：3	添加控制单元
	{0x81030400,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		DelCtrlUnitMethod4,				NULL},	//OI=0x8103 方法：4	删除控制单元
	{0x81030500,	13,	g_bPeriodCtrlUnitFmt,sizeof(g_bPeriodCtrlUnitFmt),	AddCtrlUnitMethod3,				NULL},	//OI=0x8103 方法：5	更新控制单元
	//{0x81030600,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		InputCtrlMethod6,				NULL},	//OI=0x8103 方法：6	控制投入
	{0x81030700,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		QuitCtrlMethod7,				NULL},	//OI=0x8103 方法：7	控制解除
	{0x81037F00,	13,	g_bPeriodCtrlChgFmt, sizeof(g_bPeriodCtrlChgFmt),	InputCtrlMethod127,				NULL},	//OI=0x8103 方法：127 时段功控方案切换

	{0x81040300,	13,	g_bRestCtrlUnitFmt,	 sizeof(g_bRestCtrlUnitFmt),	AddCtrlUnitMethod3,				NULL},	//OI=0x8104 方法：3	添加控制单元
	{0x81040400,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		DelCtrlUnitMethod4,				NULL},	//OI=0x8104 方法：4	删除控制单元
	{0x81040500,	13,	g_bRestCtrlUnitFmt,	 sizeof(g_bRestCtrlUnitFmt),	AddCtrlUnitMethod3,				NULL},	//OI=0x8104 方法：5	更新控制单元
	{0x81040600,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		InputCtrlMethod6,				NULL},	//OI=0x8104 方法：6	控制投入
	{0x81040700,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		QuitCtrlMethod7,				NULL},	//OI=0x8104 方法：7	控制解除

	{0x81050300,	13,	g_bShutoutCtrlUnitFmt,sizeof(g_bShutoutCtrlUnitFmt),AddCtrlUnitMethod3,				NULL},	//OI=0x8105 方法：3	添加控制单元
	{0x81050400,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		DelCtrlUnitMethod4,				NULL},	//OI=0x8105 方法：4	删除控制单元
	{0x81050500,	13,	g_bShutoutCtrlUnitFmt,sizeof(g_bShutoutCtrlUnitFmt),AddCtrlUnitMethod3,				NULL},	//OI=0x8105 方法：5	更新控制单元
	{0x81050600,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		InputCtrlMethod6,				NULL},	//OI=0x8105 方法：6	控制投入
	{0x81050700,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		QuitCtrlMethod7,				NULL},	//OI=0x8105 方法：7	控制解除

	//{0x81060300,	13,	g_bTmpCtrlUnitFmt,	 sizeof(g_bTmpCtrlUnitFmt),		AddCtrlUnitMethod3,				NULL},	//OI=0x8106 方法：3	添加控制单元
	//{0x81060400,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		DelCtrlUnitMethod4,				NULL},	//OI=0x8106 方法：4	删除控制单元
	//{0x81060500,	13,	g_bTmpCtrlUnitFmt,	 sizeof(g_bTmpCtrlUnitFmt),		AddCtrlUnitMethod3,				NULL},	//OI=0x8106 方法：5	更新控制单元
	//{0x81060600,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		InputCtrlMethod6,				NULL},	//OI=0x8106 方法：6	控制投入
	{0x81060700,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		QuitCtrlMethod7,				NULL},	//OI=0x8106 方法：7	控制解除
	{0x81067F00,	13,	g_bInputTmpCtrlFmt, sizeof(g_bInputTmpCtrlFmt),		InputCtrlMethod127,				NULL},	//OI=0x8106 方法：127 当前功率下浮控投入

	{0x81070300,	13,	g_bBuyCtrlUnitFmt,	 sizeof(g_bBuyCtrlUnitFmt),		AddCtrlUnitMethod3,				NULL},	//OI=0x8107 方法：3	添加控制单元
	{0x81070400,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		DelCtrlUnitMethod4,				NULL},	//OI=0x8107 方法：4	删除控制单元
	{0x81070500,	13,	g_bBuyCtrlUnitFmt,	 sizeof(g_bBuyCtrlUnitFmt),		AddCtrlUnitMethod3,				NULL},	//OI=0x8107 方法：5	更新控制单元
	{0x81070600,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		InputCtrlMethod6,				NULL},	//OI=0x8107 方法：6	控制投入
	{0x81070700,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		QuitCtrlMethod7,				NULL},	//OI=0x8107 方法：7	控制解除

	{0x81080300,	13,	g_bMonthCtrlUnitFmt, sizeof(g_bMonthCtrlUnitFmt),	AddCtrlUnitMethod3,				NULL},	//OI=0x8108 方法：3	添加控制单元
	{0x81080400,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		DelCtrlUnitMethod4,				NULL},	//OI=0x8108 方法：4	删除控制单元
	{0x81080500,	13,	g_bMonthCtrlUnitFmt, sizeof(g_bMonthCtrlUnitFmt),	AddCtrlUnitMethod3,				NULL},	//OI=0x8108 方法：5	更新控制单元
	{0x81080600,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		InputCtrlMethod6,				NULL},	//OI=0x8108 方法：6	控制投入
	{0x81080700,	13,	g_bDelCtrlUnitFmt,	 sizeof(g_bDelCtrlUnitFmt),		QuitCtrlMethod7,				NULL},	//OI=0x8108 方法：7	控制解除

	{0xF0010700,	18,				NULL,		NULL,						FileBlkTransMethod7,		(void*)"FileTran"},	//OI=F001 方法：07	启动下载
	{0xF0010800,	18,				NULL,		NULL,						FileBlkTransMethod8,		(void*)"FileTran"},	//OI=F001 方法：08	下载文件
	{0xF0010900,	18,				NULL,		NULL,						FileBlkTransMethod9,		(void*)"FileTran"},	//OI=F001 方法：09	读文件（参数）
	{0xF0010A00,	18,				NULL,		NULL,						FileBlkTransMethod10,		(void*)"FileTran"},	//OI=F001 方法：0A	软件比对（参数）
	//{0xF0020700,	18,				NULL,		NULL,						FileExtTransmit,				(void*)"FileTran"},	//OI=F000 方法：07	启动下载
	//{0xF0020800,	18,				NULL,		NULL,						FileExtTransmit,		(void*)"FileTran"},	//OI=F000 方法：08	下载文件

	//ESAM接口类
	{0xF1000100,	21,		NULL,						NULL,								EsamResetMethod,				NULL},	//复位
	{0xF1000200,	21,		NULL,						NULL,								EsamExeMethod,					NULL},	//执行
	{0xF1000300,	21,		NULL,						NULL,								EsamDataReadMethod,				NULL},	//ESAM数据读取
	{0xF1000400,	21,		NULL,						NULL,								EsamDataUpdateMethod,			NULL},	//数据更新
	{0xF1000500,	21, 	NULL,						NULL,								EsamConsultFailMethod,			NULL},	//协商失效
	{0xF1000600,	21,		NULL,						NULL,								EsamWalletOpMethod,				NULL},	//钱包操作（开户、充值、退费）
	{0xF1000700,	21,		g_bEsamKeyUpdateFmt,		sizeof(g_bEsamKeyUpdateFmt),		EsamKeyUpdateMethod,			NULL},	//密钥更新
	{0xF1000800,	21,		g_bEsamCerUpdateFmt,		sizeof(g_bEsamCerUpdateFmt),		EsamCerUpdateMethod,			NULL},	//证书更新
	{0xF1000900,	21,		g_bEsamTimeBarUpdateFmt,	sizeof(g_bEsamTimeBarUpdateFmt),	EsamSetConsultTimeBarMethod,	NULL},	//设置协商时效
	{0xF1000A00,	21,		NULL,						NULL,								EsamWalletInitMethod,			NULL},	//钱包初始化

	///
	{0xF2007f00,	22, 	g_bRS232PortParaCfgFmt,		sizeof(g_bRS232PortParaCfgFmt),   					ComPortParaCfgMethod127,	 	(void*)"InputOutPut"},	  //OI=0xF200 方法：127	端口
	{0xF2017f00,	22, 	g_bRS485PortParaCfgFmt,		sizeof(g_bRS485PortParaCfgFmt),   					ComPortParaCfgMethod127,	 	(void*)"InputOutPut"},	  //OI=0xF201 方法：127	端口
	{0xF2027f00,	22, 	g_bInfraPortParaCfgFmt,		sizeof(g_bInfraPortParaCfgFmt),   					ComPortParaCfgMethod127,	 	(void*)"InputOutPut"},	  //OI=0xF202 方法：127	端口
	{0xF2057f00,	22, 	g_bRelayParaCfgFmt,			sizeof(g_bRelayParaCfgFmt),   						RelayParaCfgMethod127,	 		(void*)"InputOutPut"},	  //OI=0xF205 方法：127	端口
	{0xF2077f00,	22, 	g_bMulPortCfgFmt,			sizeof(g_bMulPortCfgFmt),   						MulPortCfgMethod127,	 		(void*)"InputOutPut"},	  //OI=0xF207 方法：127	端口
	{0xF2097f00,	22, 	g_bPlcTransCfgFmt,			sizeof(g_bPlcTransCfgFmt),   						CctTransmitMethod127,	 		(void*)"InputOutPut"},	  //OI=0xF207 方法：127	端口
	{0xF2098000,	22, 	g_bPlcPortParaCfgFmt,		sizeof(g_bPlcPortParaCfgFmt),   					ComPortParaCfgMethod127,	 	(void*)"InputOutPut"},	  //OI=0xF207 方法：127	端口

};

#define OM_MAP_NUM sizeof(g_OmMap)/sizeof(g_OmMap[0])

//描述：搜索对象方法对应的映射表
TOmMap* BinarySearchOM(TOmMap* pOmMap, WORD num, DWORD dwOIMethod)
{
	int little, big, mid;
	if (dwOIMethod < pOmMap[0].dwOM  || dwOIMethod > pOmMap[num-1].dwOM)
		return NULL;

	little = 0;
	big = num;
	while (little <= big)
	{                               
		mid = (little + big) / 2;       //二分

		if (pOmMap[mid].dwOM == dwOIMethod) 
		{
			return pOmMap + mid;
		}
		else if (dwOIMethod > pOmMap[mid].dwOM)
		{
			little = mid + 1;
		} 
		else  
		{
			big = mid - 1;
		}

		mid = (little + big) / 2;
	}

	return NULL;
}

//描述：使用二分法查找映射ID
ToaMap* BinarySearchProId(ToaMap* pOIMap, WORD num, DWORD dwID)
{
	int little, big, mid;

	if (dwID<pOIMap[0].dwOA  || dwID>pOIMap[num-1].dwOA)
		return NULL;

	little = 0;
	big = num - 1;
	while (little <= big)
	{                               
		mid = (little + big) / 2;       //二分

		if (pOIMap[mid].dwOA == dwID) 
		{
			return pOIMap + mid;
		}
		else if (dwID > pOIMap[mid].dwOA)
		{
			little = mid + 1;
		} 
		else  
		{
			big = mid - 1;
		}

		//mid = (little + big) / 2;
	}

	return NULL;
}

//描述:获得OI列表中各对象的各种信息，包括描述串,内部ID等
//		@wClass:类标识
//		@pbObis:对象标识 
//		@bAttr: 类属性 
//返回：找到相应对象信息则返回描述指针
const ToaMap* GetOIMap(DWORD dwOIAtt)
{
	ToaMap* pOAMap = NULL;
	pOAMap = BinarySearchProId(g_OIConvertClass, OI_MAP_NUM, dwOIAtt);
	return pOAMap;
}

//描述:获得Method列表中各对象的各种信息，包括描述串,内部ID等
//		@wClass:类标识
//		@pbObis:对象标识 
//		@bAttr: 类属性 
//返回：找到相应对象信息则返回描述指针
const TOmMap* GetOmMap(DWORD dwOIMethod)
{
	TOmMap* pOmMap = NULL;
	pOmMap = BinarySearchOM(g_OmMap, OM_MAP_NUM, dwOIMethod);
	return pOmMap;
}

//描述:获得OI列表中各对象的各种信息，包括描述串,内部ID等
//		@wOI:对象标识
//返回：正确返回wOI对应的类，否则返回0
BYTE GetOiClass(WORD wOI)
{
	DWORD dwOI = ((DWORD )wOI<<16) + 0x0200;	//所有ID必然有属性2，且属性2需要配置到映射表中
	ToaMap* pOAMap = NULL;
	pOAMap = BinarySearchProId(g_OIConvertClass, OI_MAP_NUM, dwOI);
	if (pOAMap != NULL)
		return pOAMap->wClass;
	else
		DTRACE(DB_DB, ("GetOiClass: wOI:%02x get class failed\n", wOI));

	return 0;
}

int ClrGrpCfgMethod1(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bBuf[GRPPARA_LEN];	

	WORD wGrpNo = wOI - 0x2301 + GRP_START_PN;

	memset(bBuf, 0, sizeof(bBuf));
	WriteItemEx(BN0, wGrpNo, 0x2301, bBuf);

	return 0;
}

int AddGrpCfgMethod3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	bool fIsExist = false;
	WORD wLen;
	const ToaMap *pOadMap;
	BYTE bType, *pbBuf;
	BYTE bPnNum, bBuf[GRPPARA_LEN];	

	WORD wGrpNo = wOI - 0x2301 + GRP_START_PN;

	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN0, wGrpNo, 0x2301, bBuf);

	bPnNum = bBuf[1];
	pOadMap = GetOIMap(0x23010200);
	for (BYTE i=0; i<bPnNum; i++)
	{
		pbBuf = OoGetField(bBuf, pOadMap->pFmt, pOadMap->wFmtLen, i, &wLen, &bType);
		if (memcmp(pbBuf, pbPara, iParaLen-4) == 0)
		{
			memcpy(pbBuf, pbPara, iParaLen);
			fIsExist = true;
			break;
		}
	}

	if (!fIsExist)
	{
		if (bPnNum < MAX_GRP_PN)
		{
			int iParaLen = OoDataFieldScan(bBuf, pOadMap->pFmt, pOadMap->wFmtLen);
			if (iParaLen > 0)
			{
				memcpy(&bBuf[iParaLen], pbPara, iParaLen);
				bPnNum++;
				bBuf[0] = DT_ARRAY;
				bBuf[1] = bPnNum;
			}
			else
			{
				return -1;
			}
		}
		else
		{
			return -1; //超过允许配置的总加测量点个数
		}
	}

	WriteItemEx(BN0, wGrpNo, 0x2301, bBuf);

	return 0;
}

int BatAddGrpCfgMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD wLen;
	const ToaMap *pOadMap;
	BYTE *pbFeildFmt;
	WORD wFeildLen;
	BYTE bPnNum, bType;
	int iTmpLen = iParaLen;
	BYTE* pbTmp = pbPara;

	pbTmp++; //数组类型
	bPnNum = *pbTmp++; //元素个数
	iTmpLen -= 2;
	pOadMap = GetOIMap(0x23010200);
	for (BYTE i=0; i<bPnNum; i++)
	{
		pbTmp = OoGetField(pbPara, pOadMap->pFmt, pOadMap->wFmtLen, i, &wLen, &bType, &pbFeildFmt, &wFeildLen);
		if (AddGrpCfgMethod3(wOI, bMethod, bOpMode, pbTmp, wLen, pvAddon, pbFeildFmt, wFeildLen, pbRes) == 0)
		{
			iTmpLen -= wLen;
		}
		else
		{
			return -1;
		}
	}

	if (iTmpLen == 0)
		return 0;
	else
		return -1;
}

int DelGrpCfgMethod5(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE *pbBuf;
	WORD wLen;
	BYTE bType;
	const ToaMap *pOadMap;
	BYTE bPnNum, bBuf[GRPPARA_LEN];	

	WORD wGrpNo = wOI - 0x2301 + GRP_START_PN;

	memset(bBuf, 0, sizeof(bBuf));
	ReadItemEx(BN0, wGrpNo, 0x2301, bBuf);

	bPnNum = bBuf[1];
	pOadMap = GetOIMap(0x23010200);
	for (BYTE i=0; i<bPnNum; i++)
	{
		pbBuf = OoGetField(bBuf, pOadMap->pFmt, pOadMap->wFmtLen, i, &wLen, &bType);
		if (memcmp(pbBuf, pbPara, iParaLen) == 0)
		{
			memset(pbBuf, 0, wLen);
			memmove(pbBuf, pbBuf+wLen, GRPPARA_LEN-(pbBuf-bBuf)-wLen);
			bBuf[1] = bPnNum - 1;
			WriteItemEx(BN0, wGrpNo, 0x2301, bBuf);

			return 0;
		}
	}

	return -1;
}

int DoDevInterfaceClass19(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	DWORD dwOMD = (wOI<<16) + (bMethod<<8) + bOpMode;

	switch(dwOMD)
	{
	case 0x43000100:	//设备接口类19--复位
		SetInfo(INFO_CLASS19_METHOD_RST);
		break;
	case 0x43000200:	//设备接口类19--执行
		SetInfo(INFO_CLASS19_METHOD_EXE);
		break;
	case 0x43000300:	//设备接口类19--数据初始化
		SetInfo(INFO_CLASS19_METHOD_DATA_INIT);
		break;
	case 0x43000400:	//设备接口类19--恢复出厂参数
		SetInfo(INFO_CLASS19_METHOD_RST_FACT_PARA);
		break;
	case 0x43000500:	//设备接口类19--事件初始化
		SetInfo(INFO_CLASS19_METHOD_EVT_INIT);
		break;
	case 0x43000600:	//设备接口类19--需量初始化
		SetInfo(INFO_CLASS19_METHOD_DEM_INIT);
		break;
	default:
		return -1;
	}

	return 0;
}

int DoGprsInterfaceClass25(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	DTRACE(DB_FAPROTO, ("Gprs parameter reload!\n"));
	SetInfo(INFO_COMM_GPRS_RLD);
	return 0;
}

int DoEthInterfaceClass26(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	DTRACE(DB_FAPROTO, ("Ethernet parameter reload!\n"));
	SetInfo(INFO_COMM_ETH_RLD);
	return 0;
}


int DoRightNowStartSchMtr(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{

	WriteItemEx(BANK0, PN0, 0x6009, pbPara);

	return iParaLen;
}

int DoClearSchMtrFile(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	if (g_CStdReader)
		g_CStdReader->DeleteSearchMtrFile();

	return 1;
}

int DoClearCrossSchMtrFile(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	if (g_CStdReader)
		g_CStdReader->DeleteCrossSearchMtrFile();

	return 1;
}

int DoClass11Method127_AddMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	int iRet;
	WORD wSn, wPn;
	BYTE bBuf[128];
	char bDar = 0;

	wSn = OoOiToWord(&pbPara[3]);
	if (wSn == 0) //无效配置
	{	
		bDar = 8;
		goto Do_ExMethod_Fail;
	}

	wPn = MtrSnToPn(wSn);
	if (wPn == INVALID_POINT)
	{
		wPn = GetEmptyPn();
		if (wPn == INVALID_POINT) //配满了
		{
			bDar = 14;
			goto Do_ExMethod_Fail;
		}
	}

	memset(bBuf, 0, sizeof(bBuf));
	bBuf[0] = iParaLen;
	memcpy(bBuf+1, pbPara, iParaLen);
	DTRACE(DB_FAPROTO, ("DoClass11Method127_AddMeter: wPn=%ld, bBuf[0]=0x%02x, bBuf[1]=0x%02x.\n", wPn, bBuf[0], bBuf[1]));
	SYSTEM_DEBUG_STEP(1);
	iRet = WriteItemEx(BN0, wPn, 0x6000, bBuf);
	SYSTEM_DEBUG_STEP(2);
	if (iRet > 0)
	{
		SetMtrSnToPn(wPn, wSn);
		TrigerSaveBank(BN0, SECT_ACQ_MONI, -1);
		SetInfo(INFO_MTR_UPDATE);
#ifdef EN_SBJC_V2_CVTEXTPRO
		BYTE bAddL = bBuf[9];
		if (bBuf[13+bAddL] == 4) //是水气热表
		{
			SetInfo(INFO_SYNC_T188PARA);
			StopMtrRd(0xffff); //停止抄表
		}
#endif
		*piRetLen = iParaLen;
		return 0;
	}
	else
	{
		bDar = 3;
	}

Do_ExMethod_Fail:
	pbRes[0] = bDar; //DAR
	pbRes[1] = 0; //Data OPTIONAL=0 表示没有数据
	return -bDar;
}

int DoClass11Method128_AddMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	int iScanLen;
	WORD wLen, wMtrNum;
	BYTE bType;
	BYTE *pbPara0 = pbPara;

	if (*pbPara++ == DT_ARRAY)
	{
		wMtrNum = *pbPara++;
		for (WORD i=0; i<wMtrNum; i++)
		{
			iScanLen = OoScanData(pbPara, g_bPnParaFmt, sizeof(g_bPnParaFmt), false, -1, &wLen, &bType);
			if (iScanLen < 0)
				return -1;

			if (DoClass11Method127_AddMeter(wOI, bMethod, bOpMode, pbPara, iScanLen, pvAddon, pFmt, wFmtLen, pbRes) < 0)
				return -1;

			pbPara += iScanLen;
		}
	}

	return 0;
}


//描述：通过配置序号，更新配置单元的基本信息对象
int DoClass11Method129_UpdataMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TOobMtrInfo tMtrInfo;
	WORD wMtrSn, wPn;
	BYTE *pbPara0 = pbPara;

	if (*pbPara++ != DT_STRUCT)
		return -1;
	pbPara++;

	if (*pbPara++ != DT_LONG_U)
		return -1;
	wMtrSn = OoOiToWord(pbPara);	pbPara += 2;
	wPn = MtrSnToPn(wMtrSn);
	memset((BYTE*)&tMtrInfo, 0, sizeof(tMtrInfo));
	if (!GetMeterInfo(wPn, &tMtrInfo))
		return -1;
	if ((*pbPara++ != DT_STRUCT) || (*pbPara++ != 0x0a))
		return -1;
	if (*pbPara++ != DT_TSA)
		return -1;
	tMtrInfo.bTsaLen = *pbPara++;
	memcpy(tMtrInfo.bTsa, pbPara, tMtrInfo.bTsaLen);
	pbPara += tMtrInfo.bTsaLen;
	if (*pbPara++ != DT_ENUM)
		return -1;
	tMtrInfo.bBps = *pbPara++;
	if (*pbPara++ != DT_ENUM)
		return -1;
	tMtrInfo.bProType = *pbPara++;
	if (*pbPara++ != DT_OAD)
		return -1;
	tMtrInfo.dwPortOAD = OoOadToDWord(pbPara);	pbPara += 4;
	if (*pbPara++ != DT_OCT_STR)
		return -1;
	tMtrInfo.bCodeLen = *pbPara++;
	memcpy(tMtrInfo.bCode, pbPara, tMtrInfo.bCodeLen);
	pbPara += tMtrInfo.bCodeLen;
	if (*pbPara++ != DT_UNSIGN)
		return -1;
	tMtrInfo.bRate = *pbPara++;
	if (*pbPara++ != DT_UNSIGN)
		return -1;
	tMtrInfo.bUserType = *pbPara++;
	if (*pbPara++ != DT_ENUM)
		return -1;
	tMtrInfo.bLine = *pbPara++;
	if (*pbPara++ != DT_LONG_U)
		return -1;
	tMtrInfo.wRateVol = OoOiToWord(pbPara);	pbPara += 2;
	if (*pbPara++ != DT_LONG_U)
		return -1;
	tMtrInfo.wRateCurr = OoOiToWord(pbPara);	pbPara += 2;

	if (SetMeterInfo(wPn, tMtrInfo))
	{
		TrigerSaveBank(BN0, SECT_ACQ_MONI, -1);
		DTRACE(DB_FAPROTO, ("Delete meter successful.\n"));
		SetDelayInfo(INFO_MTR_UPDATE);
		pbRes[0] = DAR_SUCC;
		return 1;
	}

	return -1;
}

//通过基本信息对象删除
int DoClass11Method130_UpdataMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TOobMtrInfo tMtrInfo;
	WORD wMtrSn, wPn;
	BYTE *pbPara0 = pbPara;

	if (*pbPara++ != DT_STRUCT)
		return -1;
	pbPara++;

	if (*pbPara++ != DT_LONG_U)
		return -1;
	wMtrSn = OoOiToWord(pbPara);	pbPara += 2;
	wPn = MtrSnToPn(wMtrSn);
	memset((BYTE*)&tMtrInfo, 0, sizeof(tMtrInfo));
	if (!GetMeterInfo(wPn, &tMtrInfo))
		return -1;
	//扩展信息
	if ((*pbPara++!=DT_STRUCT) || (*pbPara++!=0x04))
		return -1;
	if (*pbPara++ != DT_TSA)
		return -1;
	tMtrInfo.bAcqTsaLen = *pbPara++;
	memcpy(tMtrInfo.bAcqTsa, pbPara, tMtrInfo.bAcqTsaLen);
	pbPara += tMtrInfo.bAcqTsaLen;
	if (*pbPara++ != DT_OCT_STR)
		return -1;
	tMtrInfo.bAssetLen = *pbPara++;
	memcpy(tMtrInfo.bTsa, pbPara, tMtrInfo.bAssetLen);
	pbPara += tMtrInfo.bAssetLen;
	if (*pbPara++ != DT_LONG_U)
		return -1;
	tMtrInfo.wPT = OoLongUnsignedToWord(pbPara);
	pbPara += 2;
	if (*pbPara++ != DT_LONG_U)
		return -1;
	tMtrInfo.wCT = OoLongUnsignedToWord(pbPara);
	pbPara += 2;
	if (SetMeterInfo(wPn, tMtrInfo))
	{
		pbRes[0] = DAR_SUCC;
		return 1;
	}

	TrigerSaveBank(BN0, SECT_ACQ_MONI, -1);

	DTRACE(DB_FAPROTO, ("Delete meter successful.\n"));
	SetDelayInfo(INFO_MTR_UPDATE);

	return -1;
}

//描述：通过配置序号删除
int DoClass11Method131_DelMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD wMtrSn, wPn;
	BYTE *pbPara0 = pbPara;
	BYTE bBuf[256];

	*piRetLen = 3;

	if (*pbPara++ != DT_LONG_U)
		return -1;
	wMtrSn = OoLongUnsignedToWord(pbPara);
	pbPara += 2;
	wPn = MtrSnToPn(wMtrSn);
	if (wPn == 0)
		return -1;
	memset(bBuf, 0, sizeof(bBuf));
	WriteItemEx(BN0, wPn, 0x6000, bBuf);

	TrigerSaveBank(BN0, SECT_ACQ_MONI, -1);

	DTRACE(DB_FAPROTO, ("Delete meter successful.\n"));
	SetDelayInfo(INFO_MTR_UPDATE);

	pbRes[0] = DAR_SUCC;
	return 1;
}

//描述：通过基本信息对象删除
int DoClass11Method132_DelMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TOobMtrInfo tDelMtrInfo, tDbMtrInfo;
	int iRet = -1;
	BYTE bBuf[256];
	BYTE *pbPara0 = pbPara;
	bool fDelMtrFlg;

	memset((BYTE*)&tDelMtrInfo, 0, sizeof(tDelMtrInfo));
	if ((*pbPara++ != DT_STRUCT) || (*pbPara++ != 0x0a))
		return -1;
	if (*pbPara++ != DT_TSA)
		return -1;
	pbPara++;	//跳过TSA的长度
	tDelMtrInfo.bTsaLen = *pbPara++ + 1;	//表地址长度+1为实际地址长度
	memcpy(tDelMtrInfo.bTsa, pbPara, tDelMtrInfo.bTsaLen);
	pbPara += tDelMtrInfo.bTsaLen;
	if (*pbPara++ != DT_ENUM)
		return -1;
	tDelMtrInfo.bBps = *pbPara++;
	if (*pbPara++ != DT_ENUM)
		return -1;
	tDelMtrInfo.bProType = *pbPara++;
	if (*pbPara++ != DT_OAD)
		return -1;
	tDelMtrInfo.dwPortOAD = OoOadToDWord(pbPara);	pbPara += 4;
	if (*pbPara++ != DT_OCT_STR)
		return -1;
	tDelMtrInfo.bCodeLen = *pbPara++;
	memcpy(tDelMtrInfo.bCode, pbPara, tDelMtrInfo.bCodeLen);
	pbPara += tDelMtrInfo.bCodeLen;
	if (*pbPara++ != DT_UNSIGN)
		return -1;
	tDelMtrInfo.bRate = *pbPara++;
	if (*pbPara++ != DT_UNSIGN)
		return -1;
	tDelMtrInfo.bUserType = *pbPara++;
	if (*pbPara++ != DT_ENUM)
		return -1;
	tDelMtrInfo.bLine = *pbPara++;
	if (*pbPara++ != DT_LONG_U)
		return -1;
	tDelMtrInfo.wRateVol = OoOiToWord(pbPara);	pbPara += 2;
	if (*pbPara++ != DT_LONG_U)
		return -1;
	tDelMtrInfo.wRateCurr = OoOiToWord(pbPara);	pbPara += 2;

	for (WORD wPn=0; wPn<POINT_NUM; wPn++)
	{
		if (GetMeterInfo(wPn, &tDbMtrInfo))
		{
			if ((tDbMtrInfo.bTsaLen!=tDelMtrInfo.bTsaLen) || (memcmp(tDbMtrInfo.bTsa, tDelMtrInfo.bTsa, tDelMtrInfo.bTsaLen)!=0))
				continue;
			if (tDbMtrInfo.bBps != tDelMtrInfo.bBps)
				continue;
			if (tDbMtrInfo.bProType != tDelMtrInfo.bProType)
				continue;
			if (tDbMtrInfo.dwPortOAD != tDelMtrInfo.dwPortOAD)
				continue;
			if ((tDbMtrInfo.bCodeLen!=tDelMtrInfo.bCodeLen) || (memcmp(tDbMtrInfo.bCode, tDelMtrInfo.bCode, tDelMtrInfo.bCodeLen)!=0))
				continue;
			if (tDbMtrInfo.bRate != tDelMtrInfo.bRate)
				continue;
			if (tDbMtrInfo.bLine != tDelMtrInfo.bLine)
				continue;
			if (tDbMtrInfo.wRateVol != tDelMtrInfo.wRateVol)
				continue;
			if (tDbMtrInfo.wRateCurr != tDelMtrInfo.wRateCurr)
				continue;
			memset(bBuf, 0, sizeof(bBuf));
			WriteItemEx(BN0, wPn, 0x6000, bBuf);
			fDelMtrFlg  = true;
			break;
		}
	}

	if (fDelMtrFlg)
	{
		iRet = 1;
		pbRes[0] = DAR_SUCC;
		TrigerSaveBank(BN0, SECT_ACQ_MONI, -1);
		DTRACE(DB_FAPROTO, ("Delete meter successful.\n"));
	}
	else
	{
		pbRes[0] = DAR_RES_RW;
		DTRACE(DB_FAPROTO, ("Delete meter fail.\n"));
	}

	DTRACE(DB_FAPROTO, ("Delete meter successful.\n"));
	SetDelayInfo(INFO_MTR_UPDATE);

	*piRetLen = pbPara - pbPara0;

	return iRet;
}

//描述：通过通信地址及端口删除
int DoClass11Method133_DelMeter(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TOobMtrInfo tMtrInfo;
	DWORD dwPort;
	BYTE bTsaLen;
	BYTE bTsa[TSA_LEN]={0};
	BYTE *pbPara0 = pbPara;
	BYTE bBuf[256];
	bool fDelMtrFlg = false;

	if (*pbPara++ != DT_STRUCT)
		return -1;
	pbPara++;

	if (*pbPara++ != DT_TSA)
		return -1;
	pbPara++;	//跳过TSA的长度
	bTsaLen = *pbPara++ + 1;
	memcpy(bTsa, pbPara, bTsaLen);
	pbPara += bTsaLen;
	if (*pbPara++ != DT_OAD)
		return -1;
	dwPort = OoOadToDWord(pbPara);
	pbPara += 4;

	for (WORD wPn=0; wPn<POINT_NUM; wPn++)
	{
		if (GetMeterInfo(wPn, &tMtrInfo))
		{
			if ((tMtrInfo.bTsaLen==bTsaLen) && (memcmp(tMtrInfo.bTsa, bTsa, bTsaLen)==0) && (tMtrInfo.dwPortOAD==dwPort))
			{
				memset(bBuf, 0, sizeof(bBuf));
				WriteItemEx(BN0, wPn, 0x6000, bBuf);
				fDelMtrFlg = true;
				break;
			}
		}
	}

	if (fDelMtrFlg)
		TrigerSaveBank(BN0, SECT_ACQ_MONI, -1);

	DTRACE(DB_FAPROTO, ("Delete meter successful.\n"));
	SetDelayInfo(INFO_MTR_UPDATE);

	*piRetLen = pbPara - pbPara0;

	pbRes[0] = DAR_SUCC;
	return 1;
}

//描述：清空采集档案配置表
int DoClass11Method134_DelAllMtr(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	WORD wPn;
	BYTE bBuf[MTR_PARA_LEN];
	bool fDelMtrFlg = false;

	for (wPn=0; wPn<POINT_NUM; wPn++)
	{
		if (IsPnValid(wPn))
		{
			memset(bBuf, 0, sizeof(bBuf));	
			WriteItemEx(BN0, wPn, 0x6000, bBuf);
			fDelMtrFlg = true;
		}
	}

	if (fDelMtrFlg)
		TrigerSaveBank(BN0, SECT_ACQ_MONI, -1);

	DTRACE(DB_FAPROTO, ("Delete all meter successful.\n"));
	SetDelayInfo(INFO_MTR_UPDATE);

	pbRes[0] = DAR_SUCC;
	return 1;
}

int AddCommonMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TTabCtrl TabCtrl;
	TTdbSchRule tTdbSchRule[2];
	int fd, iRet, iLen;
	WORD wLen;
	char pszTabName[32];
	BYTE bArryNum;
	BYTE bId;
	BYTE bBuf[512];
	BYTE bType;
	BYTE *pbPara0 = pbPara;

	pbPara++;	//array
	bArryNum = *pbPara++;	//跳过 array
	for (BYTE wIndex = 0; wIndex < bArryNum; wIndex++)
	{
		iRet = OoScanData(pbPara, pFmt+2, wFmtLen-2, false, -1, &wLen, &bType);	//扫描一个结构体的长度,不能用OoGetField，解析不了
		if (iRet <= 0)	
			return -1;

		//iLen = iRet + 2;	//iRet返回的是结构的内容，不包含结构体、及成员个数2个字节
		iLen = iRet;	//iRet返回的是结构的内容，不包含结构体、及成员个数2个字节
		bId = pbPara[3];
		memset(pszTabName, 0, sizeof(pszTabName));
		sprintf(pszTabName, "%s_%03d.para", (char*)pvAddon, bId);

		if ((fd=TdbOpenTable(pszTabName, O_RDWR|O_BINARY)) < 0)
		{
			memset(&TabCtrl, 0, sizeof(TabCtrl));
			//任务ID
			TabCtrl.wField[0][0] = TDB_BYTE;
			TabCtrl.wField[0][1] = 1;
			//数据类容
			TabCtrl.wField[1][0] = TDB_BYTE;
			TabCtrl.wField[1][1] = 512;

			TabCtrl.bPublicNum = 0;	//公用字段的个数
			TabCtrl.bPrivateNum = 2;	//子字段个数
			TabCtrl.dwMaxRecPublicNum = 0;	//可记录公共个数
			TabCtrl.dwMaxRecPrivateNum = 1;//最大记录子个数
			TabCtrl.dwCurRecNum = 0;
			TabCtrl.dwCurRecOffset = 0;
			TabCtrl.bVer = 1;
			int iRet = TdbCreateTable(pszTabName, TabCtrl);
			if (iRet!=TDB_ERR_OK && iRet!=TDB_ERR_TBEXIST)
			{
				TdbCloseTable(fd);
				return -1;
			}
			fd=TdbOpenTable(pszTabName, O_RDWR|O_BINARY);
		}
		else	//任务存在，先删除该条记录
		{
			memset((BYTE*)&tTdbSchRule, 0, sizeof(tTdbSchRule));
			tTdbSchRule[0].wField = 0;
			tTdbSchRule[0].wOpNum = 1;
			tTdbSchRule[0].wOp[0] = TDB_OP_EQ;
			tTdbSchRule[0].bVal[0][0] = bId;
			if (TdbDeleteSchRec(fd, tTdbSchRule, 1) < 0)
			{
				TdbCloseTable(fd);
				return -1;
			}
		}

		if (fd > 0)
		{
			memset(bBuf, 0, sizeof(bBuf));
			bBuf[0] = bId;	//填充字段
			WordToByte(iLen, &bBuf[1]);
			memcpy(bBuf+3, pbPara, iLen);
			if (!TdbAppendRec(fd, bBuf))
			{	
				TdbCloseTable(fd);
				return -1;
			}
			TdbCloseTable(fd);

		}
		pbPara += iLen;
	}

	if (wOI==0x6012)
	{
		SetDelayInfo(INFO_TASK_CFG_UPDATE);
	}
	else if (wOI==0x6014 || wOI==0x6016 || wOI==0x601C|| wOI==0x6051)
	{
		SetDelayInfo(INFO_ACQ_SCH_UPDATE);
	}

	*piRetLen = pbPara - pbPara0;

	return 0;
}

//描述：添加透明采集参数,透明传输比较特别，参数不用任务库，直接写文件
int AddTransMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	return -1;
}

//描述：删除一组配置单元			
int DelCommonMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	char pszTabName[32];
	WORD wLen = 0;
	BYTE *pbPara0 = pbPara;
	BYTE bDelNum;
	BYTE bId;
	int fd;

	pbPara0++;	//array
	bDelNum = *pbPara0++;
	for (BYTE i = 0; i < bDelNum; i++)
	{
		TTdbSchRule tTdbSchRule[2];
		BYTE bBuf[512];

		pbPara0++;
		bId = *pbPara0++;
		memset(pszTabName, 0, sizeof(pszTabName));
		sprintf(pszTabName, "%s_%03d.para", (char*)pvAddon, bId);
		if ((fd=TdbOpenTable(pszTabName, O_RDWR|O_BINARY)) < 0)
		{
			TdbCloseTable(fd);
			return -1;
		}
		else	
		{
			memset((BYTE*)&tTdbSchRule, 0, sizeof(tTdbSchRule));
			tTdbSchRule[0].wField = 0;
			tTdbSchRule[0].wOpNum = 1;
			tTdbSchRule[0].wOp[0] = TDB_OP_EQ;
			tTdbSchRule[0].bVal[0][0] = bId;
			if (TdbDeleteSchRec(fd, tTdbSchRule, 1) < 0)
			{
				TdbCloseTable(fd);
				return -1;
			}
			TdbCloseTable(fd);
		}
		pbPara0 += wLen;
	}

	if (wOI==0x6012)
	{
		SetDelayInfo(INFO_TASK_CFG_UPDATE);
	}
	else if (wOI==0x6014 || wOI==0x6016 || wOI==0x6051 || wOI==0x601c)
	{
		SetDelayInfo(INFO_ACQ_SCH_UPDATE);
	}

	*piRetLen = pbPara - pbPara0;

	return 0;
}

//描述：清空配置单元			
int ClrCommonMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	char pszTabName[32];

	DelSchData();

	for (WORD wIndex=1; wIndex<TASK_NUM; wIndex++)
	{
		memset(pszTabName, 0, sizeof(pszTabName));
		sprintf(pszTabName, "%s_%03d.para", pvAddon, wIndex);
		TdbClearRec(pszTabName); 
		if (wOI > 0x6012)
		{
			sprintf(pszTabName, "%s_%03d.dat", pvAddon, wIndex);
			TdbClearRec(pszTabName); 
		}
		DTRACE(DB_FAPROTO, ("ClrCommonMethod129: wOI=0x%04x, pszTabName:%s.\n", wOI, pszTabName));
	}

	if (wOI==0x6012)
	{
		SetDelayInfo(INFO_TASK_CFG_UPDATE);
	}
	else if (wOI==0x6014 || wOI==0x6016 || wOI==0x6051)
	{
		SetDelayInfo(INFO_ACQ_SCH_UPDATE);
	}
	else if (wOI==0x601C)
	{
		SetDelayInfo(INFO_RP_SCH_UPDATE);
	}

	return 0;
}

//描述：添加或更新一组采集规则
int AddAcqRuleMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TTabCtrl TabCtrl;
	TAcqRuleTable tAcqRuleTable;
	TTdbSchRule tTdbSchRule[2];
	TAcqRuleInfo tAcqRuleInfo;
	DWORD dwOAD, dwROAD, dwLnkOAD;
	int fd, iRet;
	char szTabName[ACQRULE_TABLE_NAME_LEN];
	BYTE bArryNum;
	BYTE bBuf[1024];
	BYTE *pbPara0 = pbPara;
	BYTE *pOneRule;

	if (*pbPara++ == DT_ARRAY)
	{
		bArryNum = *pbPara++;
		pOneRule = pbPara;
		for (BYTE bRuleIdex=0; bRuleIdex<bArryNum; bRuleIdex++)
		{
			memset(szTabName, 0, sizeof(szTabName));
			memset((BYTE*)&tAcqRuleInfo, 0, sizeof(tAcqRuleInfo));
			iRet = GetOneAcqRuleInfo(pOneRule, szTabName, sizeof(szTabName), &tAcqRuleInfo);
			if (iRet < 0)
				goto ERR_RET;

			if ((fd=TdbOpenTable(szTabName, O_RDWR|O_BINARY)) < 0)
			{
				memset(&TabCtrl, 0, sizeof(TabCtrl));
				//主OAD
				TabCtrl.wField[0][0] = TDB_BYTE;
				TabCtrl.wField[0][1] = 4;
				//数据类容
				TabCtrl.wField[1][0] = TDB_BYTE;
				TabCtrl.wField[1][1] = 512;

				TabCtrl.bPublicNum = 0;	//公用字段的个数
				TabCtrl.bPrivateNum = 2;	//子字段个数
				TabCtrl.dwMaxRecPublicNum = 0;	//可记录公共个数
				TabCtrl.dwMaxRecPrivateNum = 1;//最大记录子个数
				TabCtrl.dwCurRecNum = 0;
				TabCtrl.dwCurRecOffset = 0;
				TabCtrl.bVer = 1;
				int iRet = TdbCreateTable(szTabName, TabCtrl);
				if (iRet!=TDB_ERR_OK && iRet!=TDB_ERR_TBEXIST)
				{
					TdbCloseTable(fd);
					goto ERR_RET;
				}
				fd=TdbOpenTable(szTabName, O_RDWR|O_BINARY);
			}
			else	//任务存在，先删除该条记录
			{
				memset((BYTE*)&tTdbSchRule, 0, sizeof(tTdbSchRule));
				tTdbSchRule[0].wField = 0;
				tTdbSchRule[0].wOpNum = 1;
				tTdbSchRule[0].wOp[0] = TDB_OP_EQ;
				memcpy(tTdbSchRule[0].bVal[0], tAcqRuleInfo.pCSD, 4);
				if (TdbDeleteSchRec(fd, tTdbSchRule, 1) < 0)
				{
					TdbCloseTable(fd);
					goto ERR_RET;
				}

				if (!DeleteAcqRuleTableName(szTabName))
					goto ERR_RET;
			}

			if (fd > 0)
			{
				BYTE *pbBuf = bBuf;

				if (!SaveAcqRuleTableName(szTabName))
					goto ERR_RET;

				memset(bBuf, 0, sizeof(bBuf));
				memcpy(pbBuf, tAcqRuleInfo.pCSD+1, 4);	//+1:跳过choice
				pbBuf += 4;
				WordToByte(iRet, pbBuf);
				pbBuf += 2;
				memcpy(pbBuf, pOneRule, iRet);
				pbBuf += iRet;
				if (!TdbAppendRec(fd, bBuf))
				{	
					TdbCloseTable(fd);
					goto ERR_RET;
				}
				TdbCloseTable(fd);	
			}

			pOneRule += iRet;
		}
	}

	DTRACE(DB_FAPROTO, ("AddAcqRuleMethod129() succ...\n"));
	*piRetLen = (pOneRule-pbPara)+2;
	return 0;

ERR_RET:
	DTRACE(DB_FAPROTO, ("AddAcqRuleMethod129() succ...\n"));
	return -1;
}

//描述：删除一组采集规则
int DelAcqRuleMethod130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TTabCtrl TabCtrl;
	TTdbSchRule tTdbSchRule[2];
	TAcqRuleInfo tAcqRuleInfo;
	int iRet, fd;
	BYTE *pbPara0 = pbPara;
	BYTE bRuleNum;
	char szTabName[128];

	if (*pbPara++ == DT_ARRAY)
	{
		bRuleNum = *pbPara++;
		for (BYTE bRuleIdx=0; bRuleIdx<bRuleNum; bRuleIdx++)
		{
			memset(szTabName, 0, sizeof(szTabName));
			memset((BYTE*)&tAcqRuleInfo, 0, sizeof(tAcqRuleInfo));
			iRet = GetAcqRuleTableName(pbPara, szTabName, sizeof(szTabName), &tAcqRuleInfo);
			if (iRet < 0)
				goto ERR_RET;

			if ((fd=TdbOpenTable(szTabName, O_RDWR|O_BINARY)) < 0)
				goto ERR_RET;

			memset((BYTE*)&tTdbSchRule, 0, sizeof(tTdbSchRule));
			tTdbSchRule[0].wField = 0;
			tTdbSchRule[0].wOpNum = 1;
			tTdbSchRule[0].wOp[0] = TDB_OP_EQ;
			memcpy(tTdbSchRule[0].bVal[0], tAcqRuleInfo.pCSD, 4);
			if (TdbDeleteSchRec(fd, tTdbSchRule, 1) < 0)
			{
				TdbCloseTable(fd);
				return -1;
			}

			TdbCloseTable(fd);

			pbPara += iRet;
		}
	}

	DTRACE(DB_FAPROTO, ("DelAcqRuleMethod130() succ...\n"));
	*piRetLen = pbPara - pbPara0;
	return 0;
ERR_RET:
	DTRACE(DB_FAPROTO, ("DelAcqRuleMethod130() fail...\n"));
	return -1;
}

//描述：清空采集规则库
int ClrAcqRuleMethod131(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
#ifndef SYS_WIN
	system("rm -rf /mnt/data/data/AcqRule*");
	system("rm -rf /mnt/data/para/AcqRule*");
#endif
	DTRACE(DB_FAPROTO, ("ClrAcqRuleMethod131() succ...\n"));
	return 0;
}


//描述：更新任务状态
int UdpTaskState130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	int iRet;
	WORD wTaskFmtLen = sizeof(g_bTskUnitFmtDesc);
	BYTE bDbBuf[512] = {0};
	BYTE *pDb = bDbBuf+2;
	BYTE bSetBuf[16] = {0};
	BYTE *pbSet = bSetBuf;
	BYTE bTaskId;
	BYTE *pbPara0 = pbPara;
	BYTE *pbTaskFmt = g_bTskUnitFmtDesc;
	BYTE bIndex;
	BYTE bSetTaskStateFlg;	//正常（1），停用（2）

	if (*pbPara++ != DT_STRUCT)
		return -1;
	if (*pbPara++ != 0x02)
		return -1;
	if (*pbPara++ != DT_UNSIGN)
		return -1;
	bTaskId = *pbPara++;
	if (*pbPara++ != DT_ENUM)
		return -1;
	bSetTaskStateFlg = *pbPara++;

	iRet = GetTaskConfigFromTaskDb(bTaskId, pDb);
	if (iRet < 0)
		return -1;

	bIndex = 8;	//任务状态在任务配置单元中的索引
	WORD wSubFieldLen;
	BYTE bSubFieldType;
	BYTE bSubTaskState;
	BYTE *pSubField = OoGetField(pDb, pbTaskFmt+2, wTaskFmtLen-2, bIndex, &wSubFieldLen, &bSubFieldType);
	if (*pSubField++ != DT_ENUM)
	{
		return -1;
	}
	bSubTaskState = *pSubField++;
	if (bSubTaskState == bSetTaskStateFlg)	//任务状态相同就没有必要在进行设置了
		return 0;

	*pbSet++ = DT_ENUM;
	*pbSet++ = bSetTaskStateFlg;

	iRet = OoWriteField(pDb, iRet, pbTaskFmt+2, wTaskFmtLen-2, bIndex, bSetBuf, pbSet-bSetBuf);	//-5: 4字节OAD + 1字节时间标签
	if (iRet < 0)
	{
		DTRACE(DB_FAPROTO, ("UdpTaskState130() error!\n"));
		return -1;
	}

	bDbBuf[0] = DT_ARRAY;
	bDbBuf[1] = 0x01;

	iRet += 2;
	if (AddCommonMethod127(wOI, 0, bOpMode,bDbBuf, iRet, pvAddon, pbTaskFmt, wTaskFmtLen, pbRes, piRetLen) < 0)
		return -1;
	return 0;
}

int YkCtrlTriAlertMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	g_LoadCtrl.TrigerAlr();

	return 0;
}

int YkCtrlDisAlertMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	g_LoadCtrl.DisAlr();

	return 0;
}

int YkCtrlOpenMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE *pbPtr = pbPara;
	BYTE bNum, bTurn, bBuf[2];
	int iLen = iParaLen;
	TTime t;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	pbPtr++; //array
	bNum = *pbPtr++;
	iLen -= 2;
	for (BYTE i=0; i<bNum; i++)
	{
		pbPtr += 2; //struct
		pbPtr += 4; //OAD
		iLen -= 5;
		bTurn = *pbPtr; //轮次号
		pbPtr[0] = 1; //投入
		WriteItemEx(BN0, bTurn, 0x8203, pbPtr, dwSecs);
		pbPtr += 8;

		ReadItemEx(BN0, PN0, 0x8202, bBuf);
		bBuf[0] = DT_BIT_STR;
		bBuf[1] |= (0x01<<(bTurn-1));		//保存当前轮次'遥控命令状态'.
		WriteItemEx(BN0, PN0, 0x8202, bBuf);	//写"终端当前控制状态".
	}

	return 0;
}

int YkCtrlCloseMethod130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE *pbPtr = pbPara;
	BYTE bNum, bTurn, bBuf[2];
	int iLen = iParaLen;
	TTime t;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	pbPtr++; //array
	bNum = *pbPtr++;
	iLen -= 2;
	for (BYTE i=0; i<bNum; i++)
	{
		pbPtr += 2; //struct
		pbPtr += 4; //OAD
		iLen -= 5;
		bTurn = *pbPtr; //轮次号
		pbPtr[0] = 2; //退出
		WriteItemEx(BN0, bTurn, 0x8203, pbPtr, dwSecs);
		pbPtr += 3;

		ReadItemEx(BN0, PN0, 0x8202, bBuf);
		bBuf[0] = DT_BIT_STR;
		bBuf[1] &= ~(0x01<<(bTurn-1));		//保存当前轮次'遥控命令状态'.
		WriteItemEx(BN0, PN0, 0x8202, bBuf);	//写"终端当前控制状态".
	}

	return 0;
}

int InputGuaranteeMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bBuf[2];
	TTime t;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	bBuf[0] = 1; //保电投入
	WriteItemEx(BN0, PN0, 0x8213, bBuf, dwSecs);
	return 0;
}

int QuitGuaranteeMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bBuf[2];
	TTime t;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	bBuf[0] = 2; //保电解除
	WriteItemEx(BN0, PN0, 0x8213, bBuf, dwSecs);
	return 0;
}

int QuitAutoGuaranteeMethod129(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bBuf[2];
	TTime t;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	bBuf[0] = 3; //自动保电解除
	WriteItemEx(BN0, PN0, 0x8213, bBuf, dwSecs);
	return 0;
}

int InputUrgeFeeMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bBuf[210];
	TTime t;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	bBuf[0] = 1; //催费告警投入
	memcpy(&bBuf[1], pbPara, iParaLen);
	WriteItemEx(BN0, PN0, 0x8220, bBuf, dwSecs);
	return 0;
}

int QuitUrgeFeeMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bBuf[210];
	TTime t;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	memset(bBuf, 0, sizeof(bBuf));
	bBuf[0] = 2; //取消催费告警
	WriteItemEx(BN0, PN0, 0x8220, bBuf, dwSecs);
	return 0;
}

int AddChineseInfoMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bBuf[256];
	BYTE i, bNo, bSerialNo;
	TTime t;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	bNo = pbPara[1]; //中文信息编号
	bSerialNo = 0;
	for (i=0; i<GB_MAXCOMCHNNOTE; i++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		if (ReadItemEx(BN0, i, wOI, bBuf) > 0)
		{
			if (bNo == bBuf[4])
			{
				bSerialNo = i+1;
				break;
			}
		}
	}

	if ( bSerialNo!=0 && bSerialNo<=GB_MAXCOMCHNNOTE)
	{
		memset(bBuf, 0, sizeof(bBuf));
		bBuf[0] = iParaLen + 4;
		bBuf[1] = DT_STRUCT;
		bBuf[2] = 4;
		memcpy(&bBuf[3], pbPara, 10); //序号+发布时间
		bBuf[13] = DT_BOOL;
		bBuf[14] = false; //阅读标志
		memcpy(&bBuf[15], pbPara+10, iParaLen-10); //内容
		WriteItemEx(BN0, bSerialNo-1, wOI, bBuf, dwSecs); //替换原来的短信
	}
	else //压桟方式存储
	{				
		for (i=0; i<GB_MAXCOMCHNNOTE; i++)
		{
			if (i == (GB_MAXCOMCHNNOTE-1))//写入最新的一条
			{
				memset(bBuf, 0, sizeof(bBuf));
				bBuf[0] = iParaLen + 4;
				bBuf[1] = DT_STRUCT;
				bBuf[2] = 4;
				memcpy(&bBuf[3], pbPara, 10); //序号+发布时间
				bBuf[13] = DT_BOOL;
				bBuf[14] = false; //阅读标志
				memcpy(&bBuf[15], pbPara+10, iParaLen-10); //内容
				WriteItemEx(BN0, 0, wOI, bBuf);//从0开始
			}
			else //调整顺序
			{
				ReadItemEx(BN0, GB_MAXCOMCHNNOTE-i-2, wOI, bBuf);
				WriteItemEx(BN0, GB_MAXCOMCHNNOTE-i-1, wOI, bBuf);
			}
		}
	}
	return 0;
}

int DelChineseInfoMethod128(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bBuf[256];
	BYTE i, bNo, bSerialNo;

	bNo = pbPara[1]; //中文信息编号
	bSerialNo = 0;
	for (i=0; i<GB_MAXCOMCHNNOTE; i++)
	{
		memset(bBuf, 0, sizeof(bBuf));
		if (ReadItemEx(BN0, i, wOI, bBuf) > 0)
		{
			if (bNo == bBuf[4])
			{
				bSerialNo = i+1;
				break;
			}
		}
	}

	if ( bSerialNo!=0 && bSerialNo<=GB_MAXCOMCHNNOTE)
	{
		memset(bBuf, 0, sizeof(bBuf));
		WriteItemEx(BN0, bSerialNo-1, wOI, bBuf); //清除
	}

	return 0;
}

int AddCtrlUnitMethod3(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bBuf[256];
	WORD wGrp, wGrpOI;

	wGrpOI = OoOiToWord(pbPara+3);
	if ((wGrpOI&0xfff0) == 0x2300)
	{
		wGrp = wGrpOI - 0x2300;
		memset(bBuf, 0, sizeof(bBuf));
		WriteItemEx(BN0, wGrp, wOI, bBuf);
		WriteItemEx(BN0, wGrp, wOI, pbPara);
		return 0;
	}

	return -1;
}

int DelCtrlUnitMethod4(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE bBuf[256];
	WORD wGrp, wGrpOI;

	wGrpOI = OoOiToWord(pbPara+1);
	if ((wGrpOI&0xfff0) == 0x2300)
	{
		wGrp = wGrpOI - 0x2300;
		memset(bBuf, 0, sizeof(bBuf));
		WriteItemEx(BN0, wGrp, wOI, bBuf);
		return 0;
	}

	return -1;
}

int InputCtrlMethod6(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TTime t;
	BYTE bAct = 1; //投入
	WORD wInID, wGrp, wGrpOI;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	if (wOI>=0x8103 && wOI<=0x8108)
	{
		wInID = 0x8203 + ((wOI&0x000f)<<8);
		wGrpOI = OoOiToWord(pbPara+1);
		if ((wGrpOI&0xfff0) == 0x2300)
		{
			wGrp = wGrpOI - 0x2300;
			WriteItemEx(BN0, wGrp, wInID, &bAct, dwSecs);
			return 0;
		}
	}

	return -1;
}

int QuitCtrlMethod7(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TTime t;
	BYTE bBuf[20]; //按下浮控投入命令最大长度
	WORD wInID, wGrp, wGrpOI;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	if (wOI>=0x8103 && wOI<=0x8108)
	{
		wInID = 0x8203 + ((wOI&0x000f)<<8);
		wGrpOI = OoOiToWord(pbPara+1);
		if ((wGrpOI&0xfff0) == 0x2300)
		{
			wGrp = wGrpOI - 0x2300;
			memset(bBuf, 0, sizeof(bBuf));
			bBuf[0] = 2; //退出
			WriteItemEx(BN0, wGrp, wInID, bBuf, dwSecs);
			return 0;
		}
	}

	return -1;
}

int InputCtrlMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TTime t;
	BYTE bBuf[20]; //按下浮控投入命令最大长度
	WORD wInID, wGrp, wGrpOI;
	GetCurTime( &t );
	DWORD dwSecs = TimeToSeconds( t );

	if (wOI>=0x8103 && wOI<=0x8108)
	{
		wInID = 0x8203 + ((wOI&0x000f)<<8);
		wGrpOI = OoOiToWord(pbPara+1);
		if ((wGrpOI&0xfff0) == 0x2300)
		{
			wGrp = wGrpOI - 0x2300;
			memset(bBuf, 0, sizeof(bBuf));
			bBuf[0] = 1; //投入
			memcpy(&bBuf[1], pbPara+3, iParaLen);
			WriteItemEx(BN0, wGrp, wInID, bBuf, dwSecs);
			return 0;
		}
	}

	return -1;
}

//0x6014 方法：130	重置方案的记录列选择
int ResetSchRecordCSD(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TTdbSchRule	SchRule;
	TTdbReadCtrl TdbReadCtrl;
	TTdbSchCtrl	SchCtrl; 
	char pszTabName[32];
	int fd, iRet, schID, iSchRecs;
	BYTE bSchNo, bRdDbBuf[512] = {0};
	WORD wLen;
	BYTE *pbPara0 = pbPara;

	if (*pbPara++ != DT_STRUCT)
		return -1;
	pbPara++;

	if (*pbPara++ != DT_UNSIGN)
		return -1;

	bSchNo = *pbPara++;
	sprintf(pszTabName, "%s_%03d.para", pvAddon, bSchNo);
	if ((fd=TdbOpenTable(pszTabName, O_RDWR|O_BINARY)) >= 0)
	{
		//初始化搜索规则字段
		memset(&SchRule, 0, sizeof(SchRule));
		SchRule.wField = 0;	
		SchRule.wOpNum = 1;	
		SchRule.wOp[0] = TDB_OP_EQ;
		SchRule.bVal[0][0] = bSchNo;

		//排序规则
		memset(&SchCtrl, 0, sizeof(SchCtrl));
		SchCtrl.wSortNum = 1;		//排序规则个数
		SchCtrl.wSortOp[0] = TDB_OP_EQ; 
		SchCtrl.wSortFild[0] = 0;
		SchCtrl.wRecsToSch = 1;
		SchCtrl.iPrivateRecStart = -1;
		SchCtrl.iPublicRecStart = -1;
		SchCtrl.wRecsFound = 0;

		schID =	TdbOpenSch(fd, &SchRule, 1, SchCtrl);
		if (schID<0	|| SchCtrl.wRecsFound==0)
		{
			TdbCloseTable(fd);
			return -1;
		}
		else	
		{
			memset((BYTE*)&TdbReadCtrl, 0, sizeof(TdbReadCtrl));
			TdbReadCtrl.dwFiledNeed	= 0x02;
			TdbReadCtrl.iRecStart =	-1;	
			if (TdbReadRec(schID, &SchRule, 1, TdbReadCtrl, bRdDbBuf+1) > 0)
			{
				pFmt = g_bCommFmtDesc;
				wFmtLen = sizeof(g_bCommFmtDesc);
				wLen = ByteToWord(&bRdDbBuf[1]);
				if (iParaLen < 6)
					return -1;
				iParaLen -= 4;	//-4: 方法130的 = struct(1) + 结构成员个数（1） + 方案编号格式（1） + 方案编号（1）
				iRet = OoWriteField(&bRdDbBuf[3], wLen, pFmt+2, wFmtLen-2, 3, pbPara, iParaLen);
				if (iRet < 0)
					return -1;

				bRdDbBuf[0] = bSchNo;	//填充字段
				WordToByte(iRet, bRdDbBuf+1);

				if (!TdbAppendRec(fd, bRdDbBuf))
				{
					TdbCloseSch(schID);
					TdbCloseTable(fd);
					return -1;
				}
				TdbCloseSch(schID);
				TdbCloseTable(fd);

				SetDelayInfo(INFO_ACQ_SCH_UPDATE);
			}
		}
	}

	return 0;
}

//0x6016 方法：130	更新上报方案标识
int UpdateRptFlgMethod130(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	TTdbSchRule	SchRule;
	TTdbReadCtrl TdbReadCtrl;
	TTdbSchCtrl	SchCtrl; 
	char pszTabName[32];
	int fd, iRet, schID, iSchRecs;
	BYTE bSchNo, bRdDbBuf[512] = {0};
	WORD wLen;
	BYTE *pbPara0 = pbPara;

	if (*pbPara++ != DT_STRUCT)
		return -1;
	pbPara++;

	if (*pbPara++ != DT_UNSIGN)
		return -1;

	bSchNo = *pbPara++;
	sprintf(pszTabName, "%s_%03d.para", pvAddon, bSchNo);
	if ((fd=TdbOpenTable(pszTabName, O_RDWR|O_BINARY)) >= 0)
	{
		//初始化搜索规则字段
		memset(&SchRule, 0, sizeof(SchRule));
		SchRule.wField = 0;	
		SchRule.wOpNum = 1;	
		SchRule.wOp[0] = TDB_OP_EQ;
		SchRule.bVal[0][0] = bSchNo;

		//排序规则
		memset(&SchCtrl, 0, sizeof(SchCtrl));
		SchCtrl.wSortNum = 1;		//排序规则个数
		SchCtrl.wSortOp[0] = TDB_OP_EQ; 
		SchCtrl.wSortFild[0] = 0;
		SchCtrl.wRecsToSch = 1;
		SchCtrl.iPrivateRecStart = -1;
		SchCtrl.iPublicRecStart = -1;
		SchCtrl.wRecsFound = 0;

		schID =	TdbOpenSch(fd, &SchRule, 1, SchCtrl);
		if (schID<0	|| SchCtrl.wRecsFound==0)
		{
			TdbCloseTable(fd);
			return -1;
		}
		else	
		{
			memset((BYTE*)&TdbReadCtrl, 0, sizeof(TdbReadCtrl));
			TdbReadCtrl.dwFiledNeed	= 0x02;
			TdbReadCtrl.iRecStart =	-1;	
			if (TdbReadRec(schID, &SchRule, 1, TdbReadCtrl, bRdDbBuf+1) > 0)
			{
				pFmt = g_bEvtFmtDesc;
				wFmtLen = sizeof(g_bEvtFmtDesc);
				wLen = ByteToWord(&bRdDbBuf[1]);
				if (iParaLen < 4)
					return -1;
				iParaLen -= 4;	//-4: 方法130的 = struct(1) + 结构成员个数（1） + 方案编号格式（1） + 方案编号（1）
				iRet = OoWriteField(&bRdDbBuf[3], wLen, pFmt+2, wFmtLen-2, 3, pbPara, iParaLen);
				if (iRet < 0)
					return -1;

				bRdDbBuf[0] = bSchNo;	//填充字段
				WordToByte(iRet, bRdDbBuf+1);

				if (!TdbAppendRec(fd, bRdDbBuf))
				{
					TdbCloseSch(schID);
					TdbCloseTable(fd);
					return -1;
				}
				TdbCloseSch(schID);
				TdbCloseTable(fd);

				SetDelayInfo(INFO_ACQ_SCH_UPDATE);
			}
		}
	}

	*piRetLen = pbPara - pbPara0;

	return 0;
}


//OI=0x6012 属性：02	任务配置单元
int GetTaskConfigFromTaskDb(BYTE bTaskId, BYTE *pbRespBuf)
{
	char *pszTableName = "TaskCfgUnit";
	char pszTabName[32];
	TTdbReadCtrl TdbReadCtrl;
	TTdbSchRule	SchRule;
	TTdbSchCtrl	SchCtrl; 
	int	schID = -1;
	int fd;
	int iRet = -1;

	sprintf(pszTabName, "%s_%03d.para", pszTableName, bTaskId);
	if ((fd=TdbOpenTable(pszTabName, O_RDWR|O_BINARY)) >= 0)
	{
		//初始化搜索规则字段
		memset(&SchRule, 0, sizeof(SchRule));
		SchRule.wField = 0;	
		SchRule.wOpNum = 1;	
		SchRule.wOp[0] = TDB_OP_EQ;
		SchRule.bVal[0][0] = bTaskId;

		//排序规则
		memset(&SchCtrl, 0, sizeof(SchCtrl));
		SchCtrl.wSortNum = 1;		//排序规则个数
		SchCtrl.wSortOp[0] = TDB_OP_EQ; 
		SchCtrl.wSortFild[0] = 0;
		SchCtrl.wRecsToSch = 1;
		SchCtrl.iPrivateRecStart = -1;
		SchCtrl.iPublicRecStart = -1;
		SchCtrl.wRecsFound = 0;


		schID =	TdbOpenSch(fd, &SchRule, 1, SchCtrl);
		if (schID<0	|| SchCtrl.wRecsFound==0)
		{
			if (schID >= 0)
				TdbCloseSch(schID);
			TdbCloseTable(fd);
		}
		else
		{
			BYTE bBuf[512] = {0};
			memset(&TdbReadCtrl, 0, sizeof(TdbReadCtrl));
			//TdbReadCtrl.dwFiledNeed	= TDB_ALL_FIELD;
			TdbReadCtrl.dwFiledNeed	= 0x02;
			TdbReadCtrl.iRecStart =	-1;	
			iRet = TdbReadRec(schID, &SchRule, 1, TdbReadCtrl, bBuf);
			if (iRet > 0)
			{
				iRet = ByteToWord(&bBuf[0]);
				memcpy(pbRespBuf, &bBuf[2], iRet);
			}
			TdbCloseSch(schID);
			TdbCloseTable(fd);
		}
	}

	return iRet;
}

//描述：获取采集方案参数采集方案
int GetSchFromTaskDb(BYTE bSchNo, BYTE bSchType, BYTE *pbRespBuf)
{
	char pszTableName[32] = {0};
	TTdbReadCtrl TdbReadCtrl;
	TTdbSchRule	SchRule;
	TTdbSchCtrl	SchCtrl; 
	int	schID = -1;
	int fd;
	int iRet = -1;

	const TSchFieldCfg *p = &g_TSchFieldCfg[bSchType-1];
	if (p == NULL)
		return iRet;

	sprintf(pszTableName, "%s_%03d.para", p->pszTableName, bSchNo);
	if ((fd=TdbOpenTable(pszTableName, O_RDWR|O_BINARY)) >= 0)
	{
		memset(&SchRule, 0, sizeof(SchRule));
		SchRule.wField = 0;	
		SchRule.wOpNum = 1;	
		SchRule.wOp[0] = TDB_OP_EQ;
		SchRule.bVal[0][0] = bSchNo;

		//排序规则
		memset(&SchCtrl, 0, sizeof(SchCtrl));
		SchCtrl.wSortNum = 1;		//排序规则个数
		SchCtrl.wSortOp[0] = TDB_OP_EQ; 
		SchCtrl.wSortFild[0] = 0;
		SchCtrl.wRecsToSch = 1;
		SchCtrl.iPrivateRecStart = -1;
		SchCtrl.iPublicRecStart = -1;
		SchCtrl.wRecsFound = 0;

		schID =	TdbOpenSch(fd, &SchRule, 1, SchCtrl);
		if (schID<0	|| SchCtrl.wRecsFound==0)
		{
			if (schID >= 0)
				TdbCloseSch(schID);
			TdbCloseTable(fd);
		}
		else
		{
			BYTE bBuf[512] = {0};
			memset(&TdbReadCtrl, 0, sizeof(TdbReadCtrl));
			//TdbReadCtrl.dwFiledNeed	= TDB_ALL_FIELD;
			TdbReadCtrl.dwFiledNeed	= 0x02;
			TdbReadCtrl.iRecStart =	-1;	
			iRet = TdbReadRec(schID, &SchRule, 1, TdbReadCtrl, bBuf);
			if (iRet > 0)
			{
				iRet = ByteToWord(bBuf);
				memcpy(pbRespBuf, &bBuf[2], iRet);
			}
			TdbCloseSch(schID);
			TdbCloseTable(fd);
		}
	}
	return iRet;
}

BYTE* GetSchFmt(BYTE bSchType, WORD* pwFmtLen)
{
	if (bSchType == SCH_TYPE_COMM)
	{
		*pwFmtLen = sizeof(g_bCommFmtDesc) - 2;
		return g_bCommFmtDesc + 2;
	}
	else if (bSchType == SCH_TYPE_EVENT)
	{
		*pwFmtLen = sizeof(g_bEvtFmtDesc) - 2;
		return g_bEvtFmtDesc + 2;
	}
	else if (bSchType == SCH_TYPE_TRANS)
	{
		*pwFmtLen = sizeof(g_bTranFmtDesc) - 2;
		return g_bTranFmtDesc + 2;
	}
	else if (bSchType == SCH_TYPE_REPORT)
	{
		*pwFmtLen = sizeof(g_bRptFmtDesc) - 2;
		return g_bRptFmtDesc + 2;
	}
	else if (bSchType == SCH_TYPE_SCRIPT)
	{
		return NULL;
	}
	else if (bSchType == SCH_TYPE_REAL)
	{
		*pwFmtLen = sizeof(g_bRealFmtDesc) - 2;
		return g_bRealFmtDesc + 2;
	}

	return NULL;
}

char* GetSchTableName(BYTE bSchType)
{
	if (bSchType>=SCH_TYPE_COMM && bSchType<=SCH_TYPE_REAL)
		return (char*)g_TSchFieldCfg[bSchType-1].pszTableName;

	return NULL;
}

int GetEvtTaskTableName(BYTE bSchNo, BYTE bCSDIndex, char* pszTableName)
{
	return sprintf(pszTableName, "%s_%03d_%02d.dat", GetSchTableName(SCH_TYPE_EVENT), bSchNo, bCSDIndex);
}

int ComPortParaCfgMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE *pbPtr = pbPara;
	BYTE bPn, bNum, bTurn, bBuf[32];
	int iLen = iParaLen;
	WORD wDI;
	BYTE *pbDat =bBuf;


#if 0
	g_bRS232PortFmt
		//F200属性2--RS232端口
		DT_STRUCT, 0x03,
		DT_OAD,F2 00 02 01 //oad
		DT_COMDCB,	//端口参数
		0x06,	//9600bps
		0x00,	//无校验
		0x08,	//8位数据位
		0x01,	//停止位
		0x00,	//流控
		DT_ENUM,	//端口功能
		0x00,	//上行通信（0），抄表（1），级联（2），停用（3）
#endif

		//一次只会设置一个端口
		pbPtr += 2; //DT_STRUCT
	pbPtr += 1; //oad
	wDI = *pbPtr++;
	wDI = (wDI<<8)|*pbPtr;
	pbPtr += 2;
	bPn = *pbPtr-1;//端口号
	pbPtr += 2;//DT_COMDCB

	if(ReadItemEx(BN0, bPn, wDI, pbDat)<=0)
	{
		return -1;
	}


#if 0
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
#endif
		pbDat += 2;//DT_STRUCT
	pbDat += 1;//DT_VIS_STR
	pbDat += (*pbDat+1);// string length
	pbDat += 1;//DT_COMDCB

	//波特率
	if(*pbPtr<=10 || *pbPtr==255)
	{
		*pbDat++ = *pbPtr++;
	}
	else
	{
		return -1;
	}

	//校验
	if(*pbPtr<=2)
	{
		*pbDat++ = *pbPtr++;
	}
	else
	{
		return -1;
	}

	//数据位
	if(*pbPtr>=5 && *pbPtr<=8)
	{
		*pbDat++ = *pbPtr++;
	}
	else
	{
		return -1;
	}

	//停止位
	if(*pbPtr==1 || *pbPtr==2)
	{
		*pbDat++ = *pbPtr++;
	}
	else
	{
		return -1;
	}

	//流控
	if(*pbPtr<=2)
	{
		*pbDat++ = *pbPtr++;
	}
	else
	{
		return -1;
	}
	if(wDI==0xf200 || wDI==0xf201)
	{//红外,载波、没有端口功能
		pbPtr++;//DT_ENUM
		pbDat++;//DT_ENUM
		//端口功能
		if(*pbPtr<=3)
		{
			*pbDat = *pbPtr++;
		}
		else
		{
			return -1;
		}
	}

	WriteItemEx(BN0, bPn, wDI, bBuf);

	return 0;
}


int RelayParaCfgMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE *pbPtr = pbPara;
	BYTE bPn, bNum, bTurn, bBuf[32];
	int iLen = iParaLen;
	WORD wDI;
	BYTE *pbDat =bBuf;


#if 0
	g_bRelayParaCfgFmt
		//F205属性2--继电器输出
		DT_STRUCT, 0x02,
		DT_OAD,F2 05 02 01 //oad
		DT_ENUM,	//开关属性 
		0x00,	//脉冲式（0），保持式（1）
#endif

		//一次只会设置一个端口
		pbPtr += 2; //DT_STRUCT
	pbPtr += 1; //oad
	wDI = *pbPtr++;
	wDI = (wDI<<8)|*pbPtr;
	pbPtr += 2;
	bPn = *pbPtr-1;//端口号
	pbPtr += 2;//DT_ENUM

	if(ReadItemEx(BN0, bPn, wDI, bBuf)<=0)
	{
		return -1;
	}

#if 0
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
#endif
		pbDat += 2;//DT_STRUCT
	pbDat += 1;//DT_VIS_STR
	pbDat += (*pbDat+1);// string length
	pbDat += 3;//DT_ENUM 当前状态 

	//开关属性 
	if(*pbPtr<=1)
	{
		*pbDat = *pbPtr++;
	}
	else
	{
		return -1;
	}

	WriteItemEx(BN0, bPn, wDI, bBuf);

	return 0;
}

int MulPortCfgMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	BYTE *pbPtr = pbPara;
	BYTE bPn, bNum, bTurn, bBuf[32];
	int iLen = iParaLen;
	WORD wDI;
	BYTE *pbDat =bBuf;


#if 0
	g_bMulPortCfgFmt
		//F207属性2--继电器输出
		DT_STRUCT, 0x02,
		DT_OAD,F2 07 02 01 //oad
		DT_ENUM,	//工作模式 
		0x00,	//秒脉冲输出（0），需量周期  （1），时段投切  （2）
#endif

		//一次只会设置一个端口
		pbPtr += 2; //DT_STRUCT
	pbPtr += 1; //oad
	wDI = *pbPtr++;
	wDI = (wDI<<8)|*pbPtr;
	pbPtr += 2;
	bPn = *pbPtr-1;//端口号
	pbPtr += 2;//DT_ENUM

	if(ReadItemEx(BN0, bPn, wDI, bBuf)<=0)
	{
		return -1;
	}
#if 0	
	DT_ENUM,	//功能
		0x00,	//秒脉冲输出（0），需量周期  （1），时段投切  （2）
#endif

		pbDat += 1;//DT_ENUM  

	//功能
	if(*pbPtr<=2)
	{
		*pbDat = *pbPtr++;
	}
	else
	{
		return -1;
	}

	WriteItemEx(BN0, bPn, wDI, bBuf);

	return 0;
}

int CctTransmitMethod127(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	DWORD wTxLen;
	WORD wTimeOut;
	BYTE bTsa[TSA_LEN];
	BYTE bTsaLen;
	int iRet;

	if (*pbPara++ == DT_STRUCT)
	{
		if (*pbPara++ == 0x03)
		{
			if (*pbPara++ == DT_TSA)
			{
				*pbPara++;
				bTsaLen = *pbPara++ + 1;
				memset(bTsa, 0, sizeof(bTsa));
				memcpy(bTsa, pbPara, bTsaLen);
				pbPara += bTsaLen;

				pbPara++;	//long-unsigned
				wTimeOut = OoOiToWord(pbPara);
				pbPara += 2;

				pbPara++;	//octet-string
				pbPara += DecodeLength(pbPara, &wTxLen);

				iRet = CctTransmit(bTsa, bTsaLen, pbPara, wTxLen, wTimeOut, pbRes+2);
				if (iRet < 10)
				{
					pbRes[0] = DT_OCT_STR;
					pbRes[1] = 0x00;
					return 2;
				}
				else
				{
					pbRes[0] = DT_OCT_STR;
					pbRes[1] = iRet;
					return iRet+2;
				}
			}
		}
	}

	return -1;
}

int ResetStatAll(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen)
{
	g_StatMgr.ResetStat(wOI);
	return 0;
}

