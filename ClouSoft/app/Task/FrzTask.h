/*********************************************************************************************************
 * Copyright (c) 2016,深圳科陆电子科技股份有限公司
 * All rights reserved.
 *
 * 文件名称：FrzTask.h
 * 摘    要：本文件主要实现面向对象协议的冻结类
 * 当前版本：1.0
 * 作    者：
 * 完成日期：2016年9月
 *********************************************************************************************************/

#ifndef FRZTASK_H
#define FRZTASK_H

#include "DbStruct.h"

#define FRZ_TASK_NUM					200		//冻结任务数
#define CAP_OAD_NUM						64


#define IC9								9					//冻结类
#define OI_FRZ							0x5000

#define FRZ_TYPE_NUM					13

#define DT_FRZRELA_LEN					13
#define FRZRELA_ID_LEN					(DT_FRZRELA_LEN*CAP_OAD_NUM + 2)	//冻结类关联属性表ID长度

#define SUB_DATD_FIELD_LEN				7	//ARRAY 1 + 元素个数 1 + DT_OAD 1 + OAD 4 = 7


#define TASK_PATH_LEN					64
#define REC_TIME_OFFSET					6
#define REC_FRZ_DATA_OFFSET				13		//DATA数据类型的起始偏移
#define FRZ_REC_LEN						256		//冻结为单个ID一笔记录

#define MAX_GAP_SEC						120		//记录时间最大误差

#define FMT_FRZ_TASK_TABLE				"frz%04x_%04x_%08x_%04x.dat"

// 冻结接口类属性宏定义
//#define FRZ_LOGIC						1					//逻辑名 			static  	ctet-string
//#define FRZ_RECORDTAB					2					//冻结数据表 		dyn  		array
#define FRZ_ATTRTAB						3					//关联对象属性表 	static  	array

// 冻结接口类方法
#define FRZ_RESET						1					//复位
#define FRZ_RUN							2					//执行
#define FRZ_TRIG						3					//触发一次记录
#define FRZ_ADDATTR						4					//添加一个冻结关联对象属性 
#define FRZ_DELATTR						5					//删除一个冻结关联对象属性
#define FRZ_REFRZ						6					//触发补冻结
#define FRZ_BATADDATTR					7					//批量添加关联对象属性 
#define FRZ_CLRATTR						8					//清除关联对象属性表


//冻结类型定义
#define FRZ_OIB_INST					0x00				//00H：瞬时冻结
#define FRZ_OIB_SEC						0x01				//01H：秒冻结
#define FRZ_OIB_MIN						0x02				//02H：分钟冻结
#define FRZ_OIB_HOUR					0x03				//03H：小时冻结
#define FRZ_OIB_DAY						0x04				//04H：日冻结
#define FRZ_OIB_BALANCEDAY				0x05				//05H：结算日冻结
#define FRZ_OIB_MONTH					0x06				//06H：月冻结
#define FRZ_OIB_YEAR					0x07				//07H：年冻结
#define FRZ_OIB_TIMEZONE				0x08				//08H：时区表切换冻结
#define FRZ_OIB_DAYSTAGE				0x09				//09H：日时段表切换冻结
#define FRZ_OIB_TARIFF					0x0A				//0AH：费率电价切换冻结
#define FRZ_OIB_STAIR					0x0B				//0BH：阶梯切换冻结
//#define FRZ_OIB_VRATE					0x10				//10H：电压合格率月冻结
#define FRZ_OIB_CLRYEAR					0x11				//11H：阶梯结算冻结


//默认 冻结同期.存储周期仅对自动冻结类型有效,其它冻结存储周期无用
#define FRZ_DFTCYC_SEC					60					//01H：秒冻结,默认60秒
#define FRZ_DFTCYC_MIN					60					//02H：分钟冻结，默认60分钟
#define FRZ_DFTCYC_HOUR					1					//03H：小时冻结，默认1小时
#define FRZ_DFTCYC_DAY					1					//04H：日冻结，默认1日
#define FRZ_DFTCYC_MON					1					//06H：月冻结，默认1月
#define FRZ_DFTCYC_YEAR					1					//07H：年冻结，默认1年

#define FRZ_CLR_VALID					0xA5

//在添加1个关联对象属性中的偏移
#define OFFSET_FRZ_CYCLE				3	
#define OFFSET_FRZ_OAD					6
#define OFFSET_FRZ_REC_NUM				11

#define LAST_REC						1	//上1笔记录

#define FRZ_STAT_BASE_ID				0x0023		//BN11中上一周期结果起始ID

typedef struct{
	WORD	wCycle;					//冻结周期  long-unsigned
	DWORD	dwOA;					//关联对象属性描述符  OAD
	WORD	wMaxRecNum;				//存储深度  long-unsigned
}TFrzCfg;		//冻结对象属性

typedef struct{
	WORD		wOI;				//对象标识
	TFrzCfg		tCfg;				//冻结对象属性
	TTime		tmLastRec;			//上1笔记录时标	
	WORD		wDataFieldLen;		//数据字段长度
}TFrzCtrl;		//冻结控制


bool InitTask(bool fInit);
void DoFrzTasks();
void OnTrigFrzData(WORD wOI);
int ReadRecByPhyIdx(char* pszTbName, WORD wPhyIdx, BYTE* pbBuf, int iLen);
int ReadLastNRec(char* szTbName, BYTE bPtr, BYTE* pbBuf, int iLen);
int GetRecIdx(const int fd, int iRecNo);
int GetRecPhyIdx(char* szTbName, BYTE bPtr);
bool GetFrzTaskFileName(WORD wOI, DWORD dwROAD, char* pszFileName);
bool InitFrzTaskFieldParser(BYTE bTableIndex, BYTE* pbRCSD, TFieldParser* pFixFields, TFieldParser* pDataFields);
int ReadFrzData(DWORD dwOAD, BYTE* pbBuf, WORD wBufSize, int* piStart);
void FrzTaskOnRxFaResetCmd();

//方法接口函数
int OnResetFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnRunFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnRxTrigFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnAddFrzAttrCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnDelFrzAttrCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnRxTrigReFrzCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen=NULL);
int OnBatAddFrzAttrCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen);
int OnClrAttrTableCmd(WORD wOI, BYTE bMethod, BYTE bOpMode, BYTE* pbPara, int iParaLen, void* pvAddon, BYTE* pFmt, WORD wFmtLen, BYTE* pbRes, int* piRetLen);

void OnStatParaChg();
bool GetFrzTaskRecFieldParser(DWORD dwROAD, TFieldParser* pFixFields, TFieldParser* pDataFields, BYTE* pbAttrTabBuf, WORD wBufSize);

bool IsIntervMatch(DWORD dwOAD);
bool IsSpecFrzOAD(DWORD dwOAD);
DWORD GetLastCycleFrzMapID(DWORD dwOAD);
void DoFrzDataCmd(WORD wTaskIndex, TFrzCtrl* pFrzCtrl);
void DoReFrzDataCmd(WORD wTaskIndex, TFrzCtrl* pFrzCtrl);

#endif //FRZTASK_H
