#ifndef ACQSCHPARACFG_H
#define ACQSCHPARACFG_H

#include "TaskDB.h"
#include "CctTaskMangerOob.h"
#include "FaCfg.h"
#include "DbConst.h"



//�����ɼ������̶��ֶθ�ʽ����
static const BYTE g_bSchFixCfg[] = {	
	DT_ARRAY,	
	5,	//struct��Ա����
	DT_OAD,
	0x20, 0x2A, 0x02, 0x00,	//Ŀ�ķ�������ַ
	DT_OAD,
	0x60, 0x40, 0x02, 0x00,	//�ɼ�����ʱ��OAD
	DT_OAD,
	0x60, 0x41, 0x02, 0x00,	//�ɼ��ɹ�ʱ��OAD
	DT_OAD,
	0x60, 0x42, 0x02, 0x00,	//�ɼ��洢ʱ��OAD
	DT_OAD,	
	0x40, 0x01, 0x02, 0x00,	//TSA
};

//�����ɼ������̶��ֶθ�ʽ��������
static const BYTE g_bSchFixDataFmt[] = {
	DT_ARRAY,	
	5,	//struct ����	
		DT_OAD,	//TSA
		DT_OAD,	//data_time_s
		DT_OAD, //data_time_s
		DT_OAD, //data_time_s
		DT_OAD,	//TSA	
};

//͸���ɼ������̶��ֶθ�ʽ����
static const BYTE g_TransSchFixCfg[] = {
	DT_ARRAY,
	5,
	DT_OAD, 
	0xFF, 0xFF, 0xFF, 0x01,	//������ţ�����չ��
	DT_OAD,
	0x60, 0x40, 0x02, 0x00,	//��������ʱ��OAD
	DT_OAD,
	0x40, 0x01, 0x02, 0x00,	//TSA
	DT_OAD,
	0xFF, 0xFF, 0xFF, 0x01,	//������ţ�����չ��
	DT_OAD,
	0x60, 0x41, 0x02, 0x00,	//�����ɹ�ʱ��OAD
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
	const BYTE *pbCfg;		//�ֶ���������
	WORD wCfgLen;			//�ֶ����ݳ���
	const BYTE *pbFmt;		//�ֶθ�ʽ����
	WORD wFmtLen;			//�ֶθ�ʽ����
}TFieldCfg;

typedef struct {
	const char *pszTableName;	//������������
	BYTE bSchType;				//�ɼ�����
	TFieldCfg tTFixFieldCfg;	//�̶��ֶ�����
	TFieldCfg tTDataFieldCfg;	//�����ֶ�����
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