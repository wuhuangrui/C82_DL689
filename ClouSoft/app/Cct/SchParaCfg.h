#ifndef ACQSCHPARACFG_H
#define ACQSCHPARACFG_H

#include "TaskDB.h"
#include "CctTaskMangerOob.h"
#include "FaCfg.h"
#include "DbConst.h"



//任务库采集方案固定字段格式数据
static const BYTE g_bSchFixCfg[] = {	
	DT_ARRAY,	
	5,	//struct成员个数
	DT_OAD,
	0x20, 0x2A, 0x02, 0x00,	//目的服务器地址
	DT_OAD,
	0x60, 0x40, 0x02, 0x00,	//采集启动时标OAD
	DT_OAD,
	0x60, 0x41, 0x02, 0x00,	//采集成功时标OAD
	DT_OAD,
	0x60, 0x42, 0x02, 0x00,	//采集存储时标OAD
	DT_OAD,	
	0x40, 0x01, 0x02, 0x00,	//TSA
};

//任务库采集方案固定字段格式数据类型
static const BYTE g_bSchFixDataFmt[] = {
	DT_ARRAY,	
	5,	//struct 个数	
		DT_OAD,	//TSA
		DT_OAD,	//data_time_s
		DT_OAD, //data_time_s
		DT_OAD, //data_time_s
		DT_OAD,	//TSA	
};

//透明采集方案固定字段格式数据
static const BYTE g_TransSchFixCfg[] = {
	DT_ARRAY,
	5,
	DT_OAD, 
	0xFF, 0xFF, 0xFF, 0x01,	//方案编号（需扩展）
	DT_OAD,
	0x60, 0x40, 0x02, 0x00,	//方案启动时标OAD
	DT_OAD,
	0x40, 0x01, 0x02, 0x00,	//TSA
	DT_OAD,
	0xFF, 0xFF, 0xFF, 0x01,	//报文序号（需扩展）
	DT_OAD,
	0x60, 0x41, 0x02, 0x00,	//方案成功时标OAD
};

static const BYTE g_TransSchFixDataFmt[] = {
	DT_ARRAY,
	5,
		DT_OAD,
		DT_OAD,
		DT_OAD,
		DT_OAD,
		DT_OAD,
};

typedef struct 
{
	const BYTE *pbCfg;		//字段内容配置
	WORD wCfgLen;			//字段内容长度
	const BYTE *pbFmt;		//字段格式配置
	WORD wFmtLen;			//字段格式长度
}TFieldCfg;

typedef struct {
	const char *pszTableName;	//任务类型名字
	BYTE bSchType;				//采集类型
	TFieldCfg tTFixFieldCfg;	//固定字段配置
	TFieldCfg tTDataFieldCfg;	//数据字段配置
}TSchFieldCfg; 

const TSchFieldCfg g_TSchFieldCfg[] = {
	{"CommSch",	SCH_TYPE_COMM,		{g_bSchFixCfg,		sizeof(g_bSchFixCfg),		g_bSchFixDataFmt,		sizeof(g_bSchFixDataFmt)		}},
	{"EvtSch",	SCH_TYPE_EVENT,		{g_bSchFixCfg,		sizeof(g_bSchFixCfg),		g_bSchFixDataFmt,		sizeof(g_bSchFixDataFmt)		}},
	{"TransSch",	SCH_TYPE_TRANS,		{g_TransSchFixCfg,	sizeof(g_TransSchFixCfg),	g_TransSchFixDataFmt,	sizeof(g_TransSchFixDataFmt)	}},
	{"RptSch",	SCH_TYPE_REPORT,	{g_bSchFixCfg,		sizeof(g_bSchFixCfg),		g_bSchFixDataFmt,		sizeof(g_bSchFixDataFmt)		}},
	{"ScriptSch",	SCH_TYPE_SCRIPT,	{NULL,					0,								NULL,						0					}},
	{"MoniSch",	SCH_TYPE_REAL,		{g_bSchFixCfg,		sizeof(g_bSchFixCfg),		g_bSchFixDataFmt,		sizeof(g_bSchFixDataFmt)		}},
};

#endif