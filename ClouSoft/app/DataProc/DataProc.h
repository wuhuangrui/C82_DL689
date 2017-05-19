/*
**********************************************************************************************
* Copyright (c) 2008,深圳科陆电子科技股份有限公司
* All rights reserved.
* 
* 文件名称：DataProc.h
* 摘    要: 本文件提供一类数据及中间数据的实现
* 当前版本：1.0
* 作    者：潘香玲
* 完成日期：2008年3月
* 备    注：
**********************************************************************************************
*/
#ifndef   DATAPROC_INCLUDED
#define   DATAPROC_INCLUDED

#include  "ComStruct.h"
#include  "Trace.h"
#include  "DbConst.h"


#define BLOCK_ITEMNUM		(RATE_NUM+2)	//带费率数的电能块数据项目数

#define		INTVCAL		1	//计算间隔
#define		INTV15M  	2	//15分钟间隔
#define		INTVDAY		3	//日间隔
#define		INTVMON		4	//月间隔

#define		NOCHANGE	0
#define		DAYCHANGE	1
#define		MONCHANGE	2

typedef struct{
	DWORD dwS;	 //间隔起始时间
	DWORD dwEndS;//间隔结束时间	
}TIntvSec; //访问数据的时间起止间隔结构

//测量点的类
class CDataProc
{
public:
	CDataProc(void);
	virtual ~CDataProc(void);

	//测量点数据
	bool virtual Init(WORD  wPn) { return false; };
	void virtual DoDataProc(){};

	void TimeToIntvS(TTime time, BYTE bInterv, TIntvSec* pIntervS);
	int  DataDelta( int64* iResVal, int64 iSrcVal1, int64 iSrcVal2);
	bool DataSum( int64* iResVal, int64 iSrcVal1, int64 iSrcVal2, BYTE bFlag);
	void ClsBlockE(WORD wBank, WORD wID, int64* piValBuf, int64 iDstVal, DWORD dwSec);

public:	
	WORD m_wPn;      //测量点号（或总加组号） 	
	BYTE m_bPnProp;  //测量点属性
	BYTE m_bMtrIntv; //测量点的抄表间隔时间 		
	BYTE m_bRateNum; //实际费率数
	
};


#endif
